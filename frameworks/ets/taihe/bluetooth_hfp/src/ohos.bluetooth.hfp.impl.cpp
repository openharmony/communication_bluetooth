/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_hfp_impl_ohbluetooth"
#endif

#include "ohos.bluetooth.hfp.proj.hpp"
#include "ohos.bluetooth.hfp.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "bluetooth_hfp_ag.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "taihe_bluetooth_hfp_ag_observer.h"
namespace OHOS {
namespace Bluetooth {
    
using namespace taihe;
using namespace ohos::bluetooth::hfp;

class HandsFreeAudioGatewayProfileImpl {
public:
    HandsFreeAudioGatewayProfileImpl()
    {
        std::shared_ptr<TaiheHandsFreeAudioGatewayObserver> observer_ =
            std::make_shared<TaiheHandsFreeAudioGatewayObserver>();
    }

    void On(::taihe::string_view type, ::taihe::callback_view<void(
        ::ohos::bluetooth::baseProfile::StateChangeParam const& data)> callback)
    {
        if (observer_) {
            observer_->eventSubscribe_.RegisterEvent(callback);
        }
        if (!isRegistered_) {
            HandsFreeAudioGateway *profile = HandsFreeAudioGateway::GetProfile();
            profile->RegisterObserver(observer_);
            isRegistered_ = true;
        }
    }
    
    void Off(::taihe::string_view type, ::taihe::optional_view<::taihe::callback<void(
        ::ohos::bluetooth::baseProfile::StateChangeParam const& data)>> callback)
    {
        if (observer_) {
            observer_->eventSubscribe_.DeregisterEvent(callback);
        }
    }

    void connect(taihe::string_view deviceId)
    {
        HILOGD("ohos::bluetooth::hfp connect enter");
        std::string remoteAddr = std::string(deviceId);
        OHOS::Bluetooth::HandsFreeAudioGateway *profile = OHOS::Bluetooth::HandsFreeAudioGateway::GetProfile();
        OHOS::Bluetooth::BluetoothRemoteDevice device(remoteAddr, OHOS::Bluetooth::BT_TRANSPORT_BREDR);
        int32_t errorCode = profile->Connect(device);
        HILOGD("connect errorCode:%{public}d", errorCode);
        if (errorCode != OHOS::Bluetooth::BT_NO_ERROR) {
            taihe::set_business_error(errorCode, "Connect return error");
        }
    }

    void disconnect(taihe::string_view deviceId)
    {
        HILOGD("ohos::bluetooth::hfp disconnect enter");
        std::string remoteAddr = std::string(deviceId);
        OHOS::Bluetooth::HandsFreeAudioGateway *profile = OHOS::Bluetooth::HandsFreeAudioGateway::GetProfile();
        OHOS::Bluetooth::BluetoothRemoteDevice device(remoteAddr, OHOS::Bluetooth::BT_TRANSPORT_BREDR);
        int32_t errorCode = profile->Disconnect(device);
        HILOGD("disconnect errorCode:%{public}d", errorCode);
        if (errorCode != OHOS::Bluetooth::BT_NO_ERROR) {
            taihe::set_business_error(errorCode, "Connect return error");
        }
    }

    ohos::bluetooth::constant::ProfileConnectionState GetConnectionState(taihe::string_view deviceId)
    {
        std::string remoteAddr = std::string(deviceId);
        OHOS::Bluetooth::HandsFreeAudioGateway *profile = OHOS::Bluetooth::HandsFreeAudioGateway::GetProfile();
        OHOS::Bluetooth::BluetoothRemoteDevice device(remoteAddr, OHOS::Bluetooth::BT_TRANSPORT_BREDR);
        int32_t state = static_cast<int32_t>(OHOS::Bluetooth::BTConnectState::DISCONNECTED);
        int32_t errorCode = profile->GetDeviceState(device, state);
        if (errorCode != OHOS::Bluetooth::BT_NO_ERROR) {
            taihe::set_business_error(errorCode, "Disconnect return error");
        }

        int32_t profileState = GetProfileConnectionState(state);
        return ohos::bluetooth::constant::ProfileConnectionState::from_value(profileState);
    }

    array<string> GetConnectedDevices()
    {
        OHOS::Bluetooth::HandsFreeAudioGateway *profile = OHOS::Bluetooth::HandsFreeAudioGateway::GetProfile();
        std::vector<OHOS::Bluetooth::BluetoothRemoteDevice> devices;
        int errorCode = profile->GetConnectedDevices(devices);
        if (errorCode != OHOS::Bluetooth::BT_NO_ERROR) {
            taihe::set_business_error(errorCode, "GetConnectedDevices return error");
        }

        std::vector<std::string> deviceVector;
        for (auto &device: devices) {
            deviceVector.push_back(device.GetDeviceAddr());
        }

        return deviceVector.empty() ? taihe::array<taihe::string>{}
            : taihe::array<::taihe::string>(taihe::copy_data_t{}, deviceVector.data(), deviceVector.size());
    }
private:
    int GetProfileConnectionState(int state)
    {
        int32_t profileConnectionState = ohos::bluetooth::constant::ProfileConnectionState(
            ohos::bluetooth::constant::ProfileConnectionState::key_t::STATE_DISCONNECTED).get_value();
        switch (state) {
            case static_cast<int32_t>(OHOS::Bluetooth::BTConnectState::CONNECTING):
                HILOGD("STATE_CONNECTING(1)");
                profileConnectionState = ohos::bluetooth::constant::ProfileConnectionState(
                    ohos::bluetooth::constant::ProfileConnectionState::key_t::STATE_CONNECTING).get_value();
                break;
            case static_cast<int32_t>(OHOS::Bluetooth::BTConnectState::CONNECTED):
                HILOGD("STATE_CONNECTED(2)");
                profileConnectionState = ohos::bluetooth::constant::ProfileConnectionState(
                    ohos::bluetooth::constant::ProfileConnectionState::key_t::STATE_CONNECTED).get_value();
                break;
            case static_cast<int32_t>(OHOS::Bluetooth::BTConnectState::DISCONNECTING):
                HILOGD("STATE_DISCONNECTING(3)");
                profileConnectionState = ohos::bluetooth::constant::ProfileConnectionState(
                    ohos::bluetooth::constant::ProfileConnectionState::key_t::STATE_DISCONNECTING).get_value();
                break;
            case static_cast<int32_t>(OHOS::Bluetooth::BTConnectState::DISCONNECTED):
                HILOGD("STATE_DISCONNECTED(0)");
                profileConnectionState = ohos::bluetooth::constant::ProfileConnectionState(
                    ohos::bluetooth::constant::ProfileConnectionState::key_t::STATE_DISCONNECTED).get_value();
                break;
            default:
                break;
        }
        return profileConnectionState;
    }

    void SetConnectionStrategySync(string_view deviceId, ohos::bluetooth::baseProfile::ConnectionStrategy strategy) {}
private:
    std::shared_ptr<TaiheHandsFreeAudioGatewayObserver> observer_ = nullptr;
    bool isRegistered_ = false;
};

HandsFreeAudioGatewayProfile CreateHfpAgProfile()
{
    return make_holder<HandsFreeAudioGatewayProfileImpl, ohos::bluetooth::hfp::HandsFreeAudioGatewayProfile>();
}
}  // namespace Bluetooth
}  // namespace OHOS

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_CreateHfpAgProfile(OHOS::Bluetooth::CreateHfpAgProfile);
// NOLINTEND
