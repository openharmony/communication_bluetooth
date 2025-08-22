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
        observer_ = std::make_shared<TaiheHandsFreeAudioGatewayObserver>();
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
        std::string remoteAddr = std::string(deviceId);
        HandsFreeAudioGateway *profile = HandsFreeAudioGateway::GetProfile();
        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t errorCode = profile->Connect(device);
        if (errorCode != BT_NO_ERROR) {
            taihe::set_business_error(errorCode, "Connect return error");
        }
    }

    void disconnect(taihe::string_view deviceId)
    {
        std::string remoteAddr = std::string(deviceId);
        HandsFreeAudioGateway *profile = HandsFreeAudioGateway::GetProfile();
        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t errorCode = profile->Disconnect(device);
        if (errorCode != BT_NO_ERROR) {
            taihe::set_business_error(errorCode, "Connect return error");
        }
    }

    ohos::bluetooth::constant::ProfileConnectionState GetConnectionState(taihe::string_view deviceId)
    {
        std::string remoteAddr = std::string(deviceId);
        HandsFreeAudioGateway *profile = HandsFreeAudioGateway::GetProfile();
        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t state = static_cast<int32_t>(BTConnectState::DISCONNECTED);
        int32_t errorCode = profile->GetDeviceState(device, state);
        if (errorCode != BT_NO_ERROR) {
            taihe::set_business_error(errorCode, "Disconnect return error");
        }

        int32_t profileState = TaiheUtils::GetProfileConnectionState(state);
        return ohos::bluetooth::constant::ProfileConnectionState::from_value(profileState);
    }

    array<string> GetConnectedDevices()
    {
        HandsFreeAudioGateway *profile = HandsFreeAudioGateway::GetProfile();
        std::vector<BluetoothRemoteDevice> devices;
        int errorCode = profile->GetConnectedDevices(devices);
        if (errorCode != BT_NO_ERROR) {
            taihe::set_business_error(errorCode, "GetConnectedDevices return error");
        }

        std::vector<std::string> deviceVector;
        for (auto &device: devices) {
            deviceVector.push_back(device.GetDeviceAddr());
        }

        return deviceVector.empty() ? taihe::array<taihe::string>{}
            : taihe::array<::taihe::string>(taihe::copy_data_t{}, deviceVector.data(), deviceVector.size());
    }

    ohos::bluetooth::baseProfile::ConnectionStrategy GetConnectionStrategySync(taihe::string_view deviceId)
    {
        std::string remoteAddr = std::string(deviceId);
        int strategy = 0;
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        HandsFreeAudioGateway *profile = HandsFreeAudioGateway::GetProfile();
        int32_t err = profile->GetConnectStrategy(remoteDevice, strategy);
        if (err != BT_NO_ERROR) {
            taihe::set_business_error(err, "GetConnectionStrategy return error");
        }

        int maxStrategy = ohos::bluetooth::baseProfile::ConnectionStrategy(
            ohos::bluetooth::baseProfile::ConnectionStrategy::key_t::CONNECTION_STRATEGY_FORBIDDEN). get_value();
        int minStrategy = ohos::bluetooth::baseProfile::ConnectionStrategy(
            ohos::bluetooth::baseProfile::ConnectionStrategy::key_t::CONNECTION_STRATEGY_UNSUPPORTED). get_value();
        if (strategy < minStrategy || strategy > maxStrategy) {
            taihe::set_business_error(BT_ERR_INVALID_PARAM, "GetConnectionStrategy return error");
        }

        return ohos::bluetooth::baseProfile::ConnectionStrategy::from_value(strategy);
    }

    ohos::bluetooth::baseProfile::ConnectionStrategy GetConnectionStrategyWithCallback(string_view deviceId)
    {
        return GetConnectionStrategySync(deviceId);
    }

    ohos::bluetooth::baseProfile::ConnectionStrategy GetConnectionStrategyReturnsPromise(string_view deviceId)
    {
        return GetConnectionStrategySync(deviceId);
    }

    void SetConnectionStrategySync(string_view deviceId, ohos::bluetooth::baseProfile::ConnectionStrategy strategy)
    {
        std::string remoteAddr = std::string(deviceId);
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        HandsFreeAudioGateway *profile = HandsFreeAudioGateway::GetProfile();
        int32_t err = profile->SetConnectStrategy(remoteDevice, strategy.get_value());
        if (err != BT_NO_ERROR) {
            taihe::set_business_error(err, "SetConnectionStrategy return error");
        }
    }

    void SetConnectionStrategyWithCallback(taihe::string_view deviceId,
                                           ohos::bluetooth::baseProfile::ConnectionStrategy strategy)
    {
        SetConnectionStrategySync(deviceId, strategy);
    }

    void SetConnectionStrategyReturnsPromise(taihe::string_view deviceId,
                                             ohos::bluetooth::baseProfile::ConnectionStrategy strategy)
    {
        SetConnectionStrategySync(deviceId, strategy);
    }
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
