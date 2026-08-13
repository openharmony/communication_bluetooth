/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_pbap_pce_connecting_state"
#endif

#include "pbap_pce_connecting_state.h"
#include <cstring>
#include <vector>
#include "bt_def.h"
#include "pbap_pce_service_impl.h"
#include "power_manager.h"
#include "securec.h"

namespace OHOS {
namespace bluetooth {
PceConnectingState::PceConnectingState(
    const std::string &name, PbapPceStateMachine &sm, std::shared_ptr<IPbapPceObserver> observer)
    : PceBaseState(name, sm, std::move(observer)),
      authDescription_(),
      authNonce_(),
      authUserCharset_(0),
      authFullAccess_(false),
      authNeedUser_(false),
      authChallenge_(false),
      header_(nullptr)
{}

void PceConnectingState::Entry()
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);

    PBAP_PCE_LOG_INFO("observer_ -> CONNECTING");
    auto device = sm_.GetDevice();
    if (observer_) {
        observer_->OnConnectionStateChanged(device, static_cast<int>(BTConnectState::CONNECTING), 0);
    } else {
        PBAP_PCE_LOG_ERROR("%{public}s observer_ is null!", __PRETTY_FUNCTION__);
    }
    PBAP_PCE_LOG_INFO("%{public}s calling SdpSearch, addr=%{public}s", __PRETTY_FUNCTION__,
        GetEncryptAddr(sm_.GetDevice().GetAddress()).c_str());
    int retVal = sm_.GetSdp().SdpSearch(sm_.GetDevice().GetAddress());
    PBAP_PCE_LOG_INFO("%{public}s SdpSearch returned=%{public}d", __PRETTY_FUNCTION__, retVal);
    if (retVal != Bluetooth::BT_NO_ERROR) {
        PBAP_PCE_LOG_ERROR("%{public}s end, pce client Call SDP_ServiceSearchAttribute Error", __PRETTY_FUNCTION__);
        Transition(PCE_DISCONNECTING_STATE);
        Transition(PCE_DISCONNECTED_STATE);
    }
    const int CONNECT_TIMEOUT_MS = 10000;
    connectTimer_ = std::make_shared<utility::Timer>([this]() { OnConnectTimeout(); });
    connectTimer_->Start(CONNECT_TIMEOUT_MS);
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
}

void PceConnectingState::Exit()
{
    if (connectTimer_ != nullptr) {
        connectTimer_->Stop();
        connectTimer_ = nullptr;
    }
}

int PceConnectingState::CreateObexClient(const PbapPceHeaderSdpMsg &sdpMsg)
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);

    // set version, repository, feature to sm
    sm_.SetVersionNumber(sdpMsg.GetVersionNumber());
    sm_.SetSupportedRes(sdpMsg.GetSupportedRes());
    sm_.SetSupportedFeature(sdpMsg.GetSupportedFeature());
    sm_.SetFeatureFlag(sdpMsg.GetFeatureFlag());

    obexConfig_ = sdpMsg.GetObexClientConfig();
    SetObexClientConfigDetail(obexConfig_);
    sm_.CreatePceObexClient(obexConfig_);
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
    return true;
}

void PceConnectingState::InitAuth()
{
    authDescription_.clear();
    authChallenge_ = true;
    authFullAccess_ = true;
    authNeedUser_ = false;
    authNonce_.clear();
}


void PceConnectingState::SetObexClientConfigDetail(PbapPceObexConfig &obexConfig) const
{
    obexConfig.isSupportReliableSession = false;
    if (obexConfig.isL2capPSM) {
        obexConfig.mtu = sm_.GetPceService().GetPceConfig().l2capMtu_;
    } else {
        obexConfig.mtu = sm_.GetPceService().GetPceConfig().rfcommMtu_;
    }
    const int len = PBAP_PCE_SERVICE_UUID_LEN;
}

void PceConnectingState::ProcessObexConnected(const utility::Message &msg)
{
    PBAP_PCE_LOG_INFO("%{public}s start, msg.what_=[%{public}d]", __PRETTY_FUNCTION__, msg.what_);
    auto obexMsg = static_cast<PbapPceObexMessage *>(msg.arg2_);
    if (obexMsg != nullptr) {
        auto connectId = obexMsg->GetObexHeader().GetConnectionId();
        if (connectId != -1) {
            sm_.SetConnectId(connectId);
        }
    }
    // PbapPceObexMessage* obexMsg = static_cast<PbapPceObexMessage*>(msg.arg2_);
    Transition(PCE_CONNECTED_STATE);
    PBAP_PCE_LOG_INFO("%{public}s end, msg.what_=[%{public}d]", __PRETTY_FUNCTION__, msg.what_);
}

void PceConnectingState::ProcessSdpFinish(const utility::Message &msg)
{
    PBAP_PCE_LOG_INFO("%{public}s start, msg.what_=[%{public}d], arg2=%{public}p", __PRETTY_FUNCTION__, msg.what_,
        msg.arg2_);
    std::unique_ptr<PbapPceHeaderSdpMsg> sdpMsg(static_cast<PbapPceHeaderSdpMsg *>(msg.arg2_));
    if (sdpMsg == nullptr) {
        PBAP_PCE_LOG_ERROR("sdpMsg is null, %{public}s end, msg.what_=[%{public}d], arg2=%{public}p", __PRETTY_FUNCTION__,
            msg.what_, msg.arg2_);
        return;
    }
    PBAP_PCE_LOG_INFO("%{public}s sdpMsg valid, version=%{public}u, supportedRes=%{public}u, "
        "supportedFeature=%{public}u, featureFlag=%{public}d", __PRETTY_FUNCTION__,
        sdpMsg->GetVersionNumber(), sdpMsg->GetSupportedRes(), sdpMsg->GetSupportedFeature(),
        sdpMsg->GetFeatureFlag());
    int createRet = CreateObexClient(*sdpMsg);
    PBAP_PCE_LOG_INFO("%{public}s CreateObexClient returned=%{public}d", __PRETTY_FUNCTION__, createRet);
    if (!createRet) {
        PBAP_PCE_LOG_ERROR("%{public}s CreateObexClient failed, transitioning to DISCONNECTING", __PRETTY_FUNCTION__);
        Transition(PCE_DISCONNECTING_STATE);
        // Transition(PCE_DISCONNECTED_STATE);
    } else {
        auto self = sm_.GetObexClientPtr();
        PBAP_PCE_LOG_INFO("%{public}s obexClient=%{public}p", __PRETTY_FUNCTION__, self.get());
        if (self != nullptr) {
            PBAP_PCE_LOG_INFO("%{public}s calling obexClient->Connect, featureFlag=%{public}d", __PRETTY_FUNCTION__,
                sm_.GetFeatureFlag());
            self->Connect(self, sm_.GetFeatureFlag());
            PBAP_PCE_LOG_INFO("%{public}s obexClient->Connect returned", __PRETTY_FUNCTION__);
        } else {
            PBAP_PCE_LOG_ERROR("%{public}s obexClient is null! cannot proceed with Connect", __PRETTY_FUNCTION__);
        }
    }
    PBAP_PCE_LOG_INFO("%{public}s end, msg.what_=[%{public}d]", __PRETTY_FUNCTION__, msg.what_);
}

void PceConnectingState::ProcessObexConnectFailed(const utility::Message &msg)
{
    PBAP_PCE_LOG_INFO("%{public}s start, msg.what_=[%{public}d]", __PRETTY_FUNCTION__, msg.what_);
    auto obexMsg = static_cast<PbapPceObexMessage *>(msg.arg2_);
    // if ((obexMsg != nullptr) && ObexReconnect(*obexMsg) != BT_NO_ERROR) {
    //     Transition(PCE_DISCONNECTING_STATE);
    // }
    Transition(PCE_DISCONNECTING_STATE);
    PBAP_PCE_LOG_INFO("%{public}s end, msg.what_=[%{public}d]", __PRETTY_FUNCTION__, msg.what_);
}

void PceConnectingState::ProcessObexTransportFailed(const utility::Message &msg)
{
    PBAP_PCE_LOG_INFO("%{public}s start, msg.what_=[%{public}d]", __PRETTY_FUNCTION__, msg.what_);
    // if (msg.what_ == PCE_TRANSPORT_FAILED) {
    //     if (msg.arg1_ == CONNECT_COLLISION) {
    //         // re gap request security and reconnect
    //         // sm_.GetGap()->RequestSecurity();
    //     } else {
    //         Transition(PCE_DISCONNECTING_STATE);
    //         Transition(PCE_DISCONNECTED_STATE);
    //     }
    // }
    PBAP_PCE_LOG_INFO("%{public}s end, msg.what_=[%{public}d]", __PRETTY_FUNCTION__, msg.what_);
}

void PceConnectingState::OnConnectTimeout()
{
    PBAP_PCE_LOG_WARN("%{public}s connect timeout, closing obexClient", __PRETTY_FUNCTION__);
    auto obexClient = sm_.GetObexClientPtr();
    if (obexClient != nullptr) {
        obexClient->ObexConnectFailed();
    } else {
        Transition(PCE_DISCONNECTING_STATE);
    }
}

bool PceConnectingState::Dispatch(const utility::Message &msg)
{
    PBAP_PCE_LOG_INFO("%{public}s start, state=CONNECTING, msg.what_=[%{public}d]", __PRETTY_FUNCTION__, msg.what_);
    switch (msg.what_) {
        case PCE_SDP_FINISH:
            PBAP_PCE_LOG_INFO("%{public}s handling PCE_SDP_FINISH", __PRETTY_FUNCTION__);
            ProcessSdpFinish(msg);
            break;
        case PCE_REQ_DISCONNECTED: {
            PBAP_PCE_LOG_INFO("%{public}s handling PCE_REQ_DISCONNECTED", __PRETTY_FUNCTION__);
            auto obexClient = sm_.GetObexClientPtr();
            if (obexClient != nullptr) {
                obexClient->Disconnect(false);
            }
            Transition(PCE_DISCONNECTING_STATE);
            break;
        }
        case PCE_OBEX_CONNECTED:
            PBAP_PCE_LOG_INFO("%{public}s handling PCE_OBEX_CONNECTED", __PRETTY_FUNCTION__);
            ProcessObexConnected(msg);
            break;
        case PCE_SDP_FAILED:
            PBAP_PCE_LOG_ERROR("%{public}s handling PCE_SDP_FAILED, transitioning to DISCONNECTING/DISCONNECTED",
                __PRETTY_FUNCTION__);
            Transition(PCE_DISCONNECTING_STATE);
            Transition(PCE_DISCONNECTED_STATE);
            break;
        case PCE_OBEX_CONNECT_FAILED:
            PBAP_PCE_LOG_INFO("%{public}s handling PCE_OBEX_CONNECT_FAILED", __PRETTY_FUNCTION__);
            ProcessObexConnectFailed(msg);
            break;
        case PCE_REQ_SET_TARGET_STATE:
            PBAP_PCE_LOG_INFO("%{public}s handling PCE_REQ_SET_TARGET_STATE, arg1=%{public}d", __PRETTY_FUNCTION__,
                msg.arg1_);
            sm_.SetTargetState(msg.arg1_);
            break;
        case PCE_REQ_TRANSIT_TARGET_STATE:
            PBAP_PCE_LOG_INFO("%{public}s handling PCE_REQ_TRANSIT_TARGET_STATE", __PRETTY_FUNCTION__);
            sm_.TransitTargetState();
            break;
        case PCE_OBEX_DISCONNECTED:
            PBAP_PCE_LOG_INFO("%{public}s handling PCE_OBEX_DISCONNECTED", __PRETTY_FUNCTION__);
            Transition(PCE_DISCONNECTING_STATE);
            Transition(PCE_DISCONNECTED_STATE);
            break;
        case PCE_TRANSPORT_FAILED:
            PBAP_PCE_LOG_INFO("%{public}s handling PCE_TRANSPORT_FAILED", __PRETTY_FUNCTION__);
            ProcessObexTransportFailed(msg);
            break;
        case PCE_CONNECT_TIMEOUT:
            PBAP_PCE_LOG_INFO("%{public}s handling PCE_CONNECT_TIMEOUT", __PRETTY_FUNCTION__);
            OnConnectTimeout();
            break;
        default:
            PBAP_PCE_LOG_ERROR("%{public}s unhandled msg.what_=[%{public}d], returning false", __PRETTY_FUNCTION__,
                msg.what_);
            return false;
    }

    PBAP_PCE_LOG_INFO("%{public}s end, msg.what_=[%{public}d]", __PRETTY_FUNCTION__, msg.what_);
    return true;
}
}  // namespace bluetooth
}  // namespace OHOS