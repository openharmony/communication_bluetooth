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

    void Connect(taihe::string_view deviceId)
    {
        std::string remoteAddr = std::string(deviceId);
        HandsFreeAudioGateway *profile = HandsFreeAudioGateway::GetProfile();
        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t errorCode = profile->Connect(device);
        if (errorCode != BT_NO_ERROR) {
            taihe::set_business_error(errorCode, "Connect return error");
        }
    }

    void Disconnect(taihe::string_view deviceId)
    {
        std::string remoteAddr = std::string(deviceId);
        HandsFreeAudioGateway *profile = HandsFreeAudioGateway::GetProfile();
        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t errorCode = profile->Disconnect(device);
        if (errorCode != BT_NO_ERROR) {
            taihe::set_business_error(errorCode, "Disconnect return error");
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
            taihe::set_business_error(errorCode, "GetConnectionState return error");
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
