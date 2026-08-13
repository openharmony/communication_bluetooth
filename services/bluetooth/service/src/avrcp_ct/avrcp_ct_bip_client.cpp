/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LOG_TAG
#define LOG_TAG "avrcp_ct_bip_client"
#endif

#include "avrcp_ct_bip_client.h"
#include "profile_service_manager.h"
#include "socket_service.h"
#include "bt_uuid.h"
#include "obex_def.h"

namespace OHOS {
namespace bluetooth {

static const uint8_t BIP_UUID_AVRCP_COVER_ART[] = {0x71, 0x63, 0xDD, 0x54, 0x4A, 0x7E, 0x11, 0xE2,
                                                   0xB4, 0x7C, 0x00, 0x50, 0xC2, 0x49, 0x00, 0x48};

AvrcpCtBipClient::AvrcpCtBipClient(const std::string &deviceAddr, uint16_t psm, std::shared_ptr<Callback> callback)
    : deviceAddr_(deviceAddr),
      psm_(psm),
      state_(STATE_DISCONNECTED),
      callback_(callback),
      socketFd_(-1)
{
    HILOGI("AvrcpCtBipClient created for device: %{public}s, psm: %{public}d", deviceAddr.c_str(), psm);
}

AvrcpCtBipClient::~AvrcpCtBipClient()
{
    HILOGI("AvrcpCtBipClient destroying");
}

bool AvrcpCtBipClient::Connect()
{
    // 对齐双框架：构造后由调用方触发 CONNECT，投递到 BIP 线程异步执行。
    // 此时对象已被 shared_ptr 管理（CoverArtManager::Connect 中 make_shared 后存入 clients_），
    // 可安全使用 shared_from_this() 保证 lambda 执行时对象生命周期。
    auto self = shared_from_this();
    PostToBipThread([self]() { self->ProcessConnect(); });
    return true;
}

void AvrcpCtBipClient::Disconnect()
{
    HILOGI("Posting DISCONNECT to BIP thread");
    auto self = shared_from_this();
    PostToBipThread([self]() { self->ProcessDisconnect(); });
}

void AvrcpCtBipClient::Shutdown()
{
    HILOGI("Shutdown: posting DISCONNECT to BIP thread");
    // 对齐双框架 shutdown()：投递 DISCONNECT 到 BIP 线程异步执行。
    // 用 shared_from_this 保证 lambda 执行时对象生命周期，
    // CoverArtManager::Disconnect 会在 Shutdown 后 erase clients_，
    // 回调 OnConnectionStateChanged(STATE_DISCONNECTED) 时 GetClient 返回 null，不会误重连。
    auto self = shared_from_this();
    PostToBipThread([self]() { self->ProcessDisconnect(); });
}

int AvrcpCtBipClient::GetState() const
{
    return state_;
}

bool AvrcpCtBipClient::IsConnected() const
{
    return state_ == STATE_CONNECTED && clientSession_ != nullptr;
}

uint16_t AvrcpCtBipClient::GetPsm() const
{
    return psm_;
}

bool AvrcpCtBipClient::GetImageProperties(const std::string &imageHandle)
{
    if (!IsConnected()) {
        HILOGE("Not connected");
        return false;
    }

    auto self = shared_from_this();
    auto request = std::make_shared<RequestGetImageProperties>(imageHandle);
    PostToBipThread([self, request]() { self->ProcessRequest(request); });
    return true;
}

bool AvrcpCtBipClient::GetImage(const std::string &imageHandle, const BipImageDescriptor &descriptor)
{
    if (!IsConnected()) {
        HILOGE("Not connected");
        return false;
    }

    auto self = shared_from_this();
    auto request = std::make_shared<RequestGetImage>(imageHandle, descriptor);
    PostToBipThread([self, request]() { self->ProcessRequest(request); });
    return true;
}

void AvrcpCtBipClient::PostToBipThread(const ThreadUtilFunc &func)
{
    DoInBipThread(func);
}

void AvrcpCtBipClient::ProcessConnect()
{
    HILOGI("Processing CONNECT in BIP thread");

    if (state_ == STATE_CONNECTED) {
        HILOGW("Already connected");
        return;
    }

    SetConnectionState(STATE_CONNECTING);

    if (!ConnectL2cap()) {
        HILOGE("Failed to connect L2CAP");
        SetConnectionState(STATE_DISCONNECTED);
        return;
    }

    if (!CreateObexSession()) {
        HILOGE("Failed to create OBEX session");
        DestroyObexSession();
        SetConnectionState(STATE_DISCONNECTED);
        return;
    }

    SetConnectionState(STATE_CONNECTED);
    HILOGI("BIP connection established");
}

void AvrcpCtBipClient::ProcessDisconnect()
{
    HILOGI("Processing DISCONNECT in BIP thread");

    if (state_ == STATE_DISCONNECTED) {
        return;
    }

    SetConnectionState(STATE_DISCONNECTING);
    DestroyObexSession();
    SetConnectionState(STATE_DISCONNECTED);
    HILOGI("BIP disconnected");
}

void AvrcpCtBipClient::ProcessRequest(std::shared_ptr<BipRequest> request)
{
    if (!request || !callback_) {
        return;
    }

    if (!IsConnected()) {
        HILOGE("Not connected, cannot process request");
        NotifyCaller(request);
        return;
    }

    HILOGI("Executing request: %{public}s", request->ToString().c_str());
    // 对齐双框架 executeRequest：请求失败时销毁 OBEX 会话（不触发重连，避免持续失败循环）
    if (!request->Execute(clientSession_)) {
        HILOGE("Request execute failed, destroying OBEX session");
        NotifyCaller(request);
        DestroyObexSession();
        return;
    }
    NotifyCaller(request);
}

void AvrcpCtBipClient::NotifyCaller(std::shared_ptr<BipRequest> request)
{
    if (!callback_ || !request) {
        return;
    }

    std::string imageHandle = request->GetImageHandle();
    int type = request->GetType();

    switch (type) {
        case BipRequest::TYPE_GET_IMAGE_PROPERTIES: {
            auto req = std::static_pointer_cast<RequestGetImageProperties>(request);
            callback_->OnGetImagePropertiesComplete(req->GetResponseCode(), imageHandle, req->GetProperties());
            break;
        }
        case BipRequest::TYPE_GET_IMAGE: {
            auto req = std::static_pointer_cast<RequestGetImage>(request);
            callback_->OnGetImageComplete(req->GetResponseCode(), imageHandle, req->GetImage());
            break;
        }
        default: {
            HILOGE("Unknown request type: %{public}d", type);
            break;
        }
    }
}

bool AvrcpCtBipClient::ConnectL2cap()
{
    HILOGI("Connecting L2CAP via SocketService, device: %{public}s, psm: %{public}d", deviceAddr_.c_str(), psm_);

    auto *profile = IProfileManager::GetInstance()->GetProfileService("SocketService");
    auto *socketService = static_cast<SocketService *>(profile);
    if (!socketService) {
        HILOGE("Failed to get SocketService");
        return false;
    }

    Uuid uuid = Uuid();
    int securityFlags = 0;
    int socketType = 1;

    socketFd_ = socketService->Connect(deviceAddr_, uuid, securityFlags, socketType, psm_);
    if (socketFd_ < 0) {
        HILOGE("SocketService::Connect failed, fd=%{public}d", socketFd_);
        return false;
    }

    HILOGI("L2CAP connected via SocketService, fd=%{public}d", socketFd_);

    socketDevice_ = std::make_shared<ObexSocketDevice>(socketFd_, 0xFFFF, 0xFFFF, SocketType::TYPE_L2CAP, deviceAddr_);

    return true;
}

namespace {
class BipObexResponseHandler final : public ObexResponseHandler {
public:
    BipObexResponseHandler() = default;
    ~BipObexResponseHandler() = default;

    void OnConnectResp(ObexHeader &reply, uint8_t responseCode) override {}
    void OnPutDateAvailableResp(ObexHeader &reply, uint8_t responseCode) override {}
    void OnDisConnectResp(ObexHeader &reply, uint8_t responseCode) override {}
    void OnAbortResp() override {}
};
}  // namespace

bool AvrcpCtBipClient::CreateObexSession()
{
    HILOGI("Creating OBEX session using existing OBEX API");

    if (!socketDevice_) {
        HILOGE("socketDevice_ is null");
        return false;
    }

    transport_ = std::make_shared<ObexServerTransport>(socketDevice_);
    responseHandler_ = std::make_shared<BipObexResponseHandler>();
    clientSession_ = std::make_shared<ObexClientSession>(transport_, responseHandler_);

    ObexHeader header;
    header.SetHeader(TARGET, std::vector<uint8_t>(BIP_UUID_AVRCP_COVER_ART,
                                                  BIP_UUID_AVRCP_COVER_ART + sizeof(BIP_UUID_AVRCP_COVER_ART)));

    ObexHeader outHeader;
    clientSession_->Connect(header, connectHeader_);

    uint8_t lastResp = clientSession_->GetLastRespCd();
    if (lastResp != 0xA0) {
        HILOGE("OBEX Connect failed, response code=%{public}d", lastResp);
        clientSession_ = nullptr;
        return false;
    }

    HILOGI("OBEX session created successfully");
    return true;
}

void AvrcpCtBipClient::DestroyObexSession()
{
    HILOGI("Destroying OBEX session");

    if (clientSession_) {
        ObexHeader header;
        header.SetConnectionId(connectHeader_.GetConnectionId());
        clientSession_->DisConnect(header, connectHeader_);
        clientSession_->CloseTransport();
        clientSession_ = nullptr;
    }

    transport_ = nullptr;
    responseHandler_ = nullptr;

    if (socketFd_ >= 0) {
        close(socketFd_);
        socketFd_ = -1;
    }
    socketDevice_ = nullptr;
}

void AvrcpCtBipClient::SetConnectionState(int state)
{
    int oldState = state_;
    state_ = state;
    if (callback_ && oldState != state) {
        callback_->OnConnectionStateChanged(oldState, state);
    }
}

}  // namespace bluetooth
}  // namespace OHOS
