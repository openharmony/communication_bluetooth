/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_map_mse_service"
#endif

#include "map_mse_service_impl.h"

#include "adapter_device_config.h"
#include "bluetooth_errorcode.h"
#include "log.h"
#include "interface_profile.h"

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;
const int32_t MAS_ID_SMS_MMS = 0;

extern "C" MapMseServiceImplInterface *CreateMapMseServiceImplInterface(void)
{
    return new MapMseServiceImpl();
}

extern "C" void DestroyMapMseServiceImplInterface(MapMseServiceImplInterface *interface)
{
    if (interface == nullptr) {
        HILOGE("map mse service impl Interface is nullptr");
        return;
    }
    delete interface;
}

void MapMseServiceImpl::StartUp(const MapMnsSdpFoundRecord &record,
    std::shared_ptr<ObexSocketDevice> &socketDevice, const bool isReconnClient)
{
    std::shared_ptr<MapMseMasInstance> smsInstance = std::make_shared<MapMseMasInstance>(MAS_ID_SMS_MMS, this);
    masInstanceMap_.Insert(MAS_ID_SMS_MMS, smsInstance);

    HILOGI("mapclient startup deviceAddr %{public}s, uuid %{public}s, l2capPsm %{public}d, channel %{public}d.",
        GET_ENCRYPT_STR_ADDR(record.deviceAddr), record.uuid.c_str(), record.l2capPsm, record.channel);

    if (!mnsClient_) {
        mnsClient_ = std::make_shared<MapMseMnsClient>(record, this);
    } else if (isReconnClient && (record.l2capPsm != 0 || record.channel != 0)) {
        HILOGI("mnsClient_ is exist clear and reconnect it");
        mnsClient_ = std::make_shared<MapMseMnsClient>(record, this);
        mnsClient_->Connect();
        masInstanceMap_.Iterate([this](int32_t key, std::shared_ptr<MapMseMasInstance> &instance) {
            instance->UpdateMnsMseClient(mnsClient_);
        });
        HILOGI("mnsClient_ reconnect end");
        return;
    }

    {
        std::lock_guard<std::mutex> lock(socketDeviceLock_);
        socketDevice_ = socketDevice;
        masInstanceMap_.Iterate([this](int32_t key, std::shared_ptr<MapMseMasInstance> &instance) {
            instance->StartObexServerSession(socketDevice_, mnsClient_);
        });
    }
    connectState_ = static_cast<int32_t>(BTConnectState::CONNECTED);
}

void MapMseServiceImpl::RegisterObserver(std::shared_ptr<IMapMseObserver> &observer)
{
    HILOGI("RegisterObserver");
    mapMseConnectionObserver_ = observer;
}

void MapMseServiceImpl::DeregisterObserver(std::shared_ptr<IMapMseObserver> &observer)
{
    HILOGI("DeregisterObserver");
    mapMseConnectionObserver_ = nullptr;
}

int MapMseServiceImpl::Disconnect(const RawAddress &device)
{
    HILOGI("device %{public}s.", GET_ENCRYPT_ADDR(device));
    std::lock_guard<std::mutex> lock(socketDeviceLock_);
    {
        CHECK_AND_RETURN_LOG_RET(socketDevice_ != nullptr, Bluetooth::BT_ERR_INTERNAL_ERROR, "socketDevice_ is null");
        if (device.GetAddress() != socketDevice_->mDeviceAddress ||
            connectState_ != static_cast<int32_t>(BTConnectState::CONNECTED)) {
            return RET_NO_ERROR;
        }
    }
    masInstanceMap_.Iterate([](int32_t key, std::shared_ptr<MapMseMasInstance> &instance) {
        instance->StopObexServerSession();
    });
    if (mnsClient_) {
        mnsClient_->DisConnect();
    }
    return RET_NO_ERROR;
}

void MapMseServiceImpl::OnClose(int32_t masInstanceId)
{
    DoInMapThread([this, masInstanceId]() { this->DealOnClose(masInstanceId); });
}

void MapMseServiceImpl::DealOnClose(int32_t masInstanceId)
{
    std::shared_ptr<MapMseMasInstance> tmpInstance;
    if (!masInstanceMap_.Find(masInstanceId, tmpInstance)) {
        return;
    }
    connectState_ = static_cast<int32_t>(BTConnectState::DISCONNECTED);
    tmpInstance->StopObexServerSession();

    std::lock_guard<std::mutex> lock(socketDeviceLock_);
    CHECK_AND_RETURN_LOG(socketDevice_ != nullptr, "socketDevice_ is null");
    NotifyStateChanged(RawAddress(socketDevice_->mDeviceAddress), connectState_);
    socketDevice_ = nullptr;
}

void MapMseServiceImpl::NotifyStateChanged(RawAddress address, int32_t state)
{
    CHECK_AND_RETURN_LOG(mapMseConnectionObserver_, "mapMseConnectionObserver_ is null");
    mapMseConnectionObserver_->OnConnectionStateChanged(address, state,
        static_cast<int>(ConnChangeCause::CONNECT_CHANGE_COMMON_CAUSE));
}

std::list<RawAddress> MapMseServiceImpl::GetConnectDevices()
{
    std::list<RawAddress> result;
    std::lock_guard<std::mutex> lock(socketDeviceLock_);
    CHECK_AND_RETURN_LOG_RET(socketDevice_ != nullptr, result, "socketDevice_ is null");
    if (connectState_ == static_cast<int32_t>(BTConnectState::CONNECTED) && socketDevice_->mDeviceAddress != "") {
        result.push_back(RawAddress(socketDevice_->mDeviceAddress));
    }
    return result;
}

int32_t MapMseServiceImpl::GetDeviceState(const RawAddress &device)
{
    std::lock_guard<std::mutex> lock(socketDeviceLock_);
    CHECK_AND_RETURN_LOG_RET(socketDevice_ != nullptr, static_cast<int32_t>(BTConnectState::DISCONNECTED),
        "socketDevice_ is null");
    if (device.GetAddress() == socketDevice_->mDeviceAddress) {
        return connectState_;
    }
    return static_cast<int32_t>(BTConnectState::DISCONNECTED);
}

int32_t MapMseServiceImpl::GetConnectState(void)
{
    if (connectState_ == static_cast<int32_t>(BTConnectState::CONNECTED)) {
        return PROFILE_STATE_CONNECTED;
    }
    return PROFILE_STATE_DISCONNECTED;
}

void MapMseServiceImpl::DllRegisterConnectSocketFunc(const std::function<int(const std::string &addr, const Uuid &uuid,
        int securityFlag, int type, int psm)> &connectSocketFunc)
{
    connectSocketFunc_ = connectSocketFunc;
}

void MapMseServiceImpl::DllRegisterGetDbIndetifierFunc(const std::function<int()> &func)
{
    getDbIdentifierFunc_ = func;
}

void MapMseServiceImpl::DllSetIsNeedReconnClientFunc(const std::function<void(const bool isNeedReconnClient)> &func)
{
    setIsNeedReconnClientFunc_ = func;
}
}  // namespace bluetooth
}  // namespace OHOS
