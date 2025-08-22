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
#define LOG_TAG "bt_hid_impl_ohbluetooth"
#endif

#include "ohos.bluetooth.hid.proj.hpp"
#include "ohos.bluetooth.hid.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "bluetooth_hid_host.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "taihe_bluetooth_hid_host_observer.h"

namespace OHOS {
namespace Bluetooth {

using namespace taihe;
using namespace ohos::bluetooth::hid;

class HidHostProfileImpl {
public:
    HidHostProfileImpl()
    {
        observer_ = std::make_shared<TaiheBluetoothHidHostObserver>();
        HidHost *profile = HidHost::GetProfile();
        profile->RegisterObserver(observer_);
    }

    void On(::taihe::string_view type, ::taihe::callback_view<void(
        ::ohos::bluetooth::baseProfile::StateChangeParam const& data)> callback)
    {
        if (observer_) {
            observer_->eventSubscribe_.RegisterEvent(callback);
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
        HidHost *profile = HidHost::GetProfile();
        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t errorCode = profile->Connect(device);
        if (errorCode != BT_NO_ERROR) {
            taihe::set_business_error(errorCode, "Connect return error");
        }
    }

    void disconnect(taihe::string_view deviceId)
    {
        std::string remoteAddr = std::string(deviceId);
        HidHost *profile = HidHost::GetProfile();
        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t errorCode = profile->Disconnect(device);
        if (errorCode != BT_NO_ERROR) {
            taihe::set_business_error(errorCode, "Disconnect return error");
        }
    }

    ohos::bluetooth::constant::ProfileConnectionState GetConnectionState(taihe::string_view deviceId)
    {
        std::string remoteAddr = std::string(deviceId);
        HidHost *profile = HidHost::GetProfile();
        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t state = static_cast<int32_t>(BTConnectState::DISCONNECTED);
        int32_t errorCode = profile->GetDeviceState(device, state);
        if (errorCode != BT_NO_ERROR) {
            taihe::set_business_error(errorCode, "Disconnect return error");
        }

        int profileState = TaiheUtils::GetProfileConnectionState(state);
        return ohos::bluetooth::constant::ProfileConnectionState::from_value(profileState);
    }

    taihe::array<taihe::string> GetConnectedDevices()
    {
        HidHost *profile = HidHost::GetProfile();
        std::vector<int> states = { static_cast<int>(BTConnectState::CONNECTED) };
        std::vector<BluetoothRemoteDevice> devices;
        int errorCode = profile->GetDevicesByStates(states, devices);
        if (errorCode != BT_NO_ERROR) {
            // taihe::set_business_error(errorCode, "GetConnectedDevices return error");
        }

        std::vector<std::string> deviceVector;
        for (auto &device: devices) {
            deviceVector.push_back(device.GetDeviceAddr());
        }

        return deviceVector.empty() ? taihe::array<taihe::string>{}
            : taihe::array<::taihe::string>(taihe::copy_data_t{}, deviceVector.data(), deviceVector.size());
    }
private:
    std::shared_ptr<TaiheBluetoothHidHostObserver> observer_ = nullptr;
};

HidHostProfile CreateHidHostProfile()
{
    // The parameters in the make_holder function should be of the same type
    // as the parameters in the constructor of the actual implementation class.
    return make_holder<HidHostProfileImpl, ::ohos::bluetooth::hid::HidHostProfile>();
}
}  // namespace Bluetooth
}  // namespace OHOS

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_CreateHidHostProfile(OHOS::Bluetooth::CreateHidHostProfile);
// NOLINTEND
