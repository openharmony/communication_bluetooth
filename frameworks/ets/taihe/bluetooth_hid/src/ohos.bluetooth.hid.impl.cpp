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
#include "bluetooth_errorcode.h"
#include "bluetooth_hid_host.h"
#include "bluetooth_log.h"
#include "taihe_bluetooth_hid_host_observer.h"
#include "taihe_bluetooth_utils.h"

namespace OHOS {
namespace Bluetooth {
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

    void Connect(taihe::string_view deviceId)
    {
        HILOGD("enter");
        std::string remoteAddr = std::string(deviceId);
        bool checkRet = CheckDeivceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

        HidHost *profile = HidHost::GetProfile();
        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t errorCode = profile->Connect(device);
        TAIHE_BT_ASSERT_RETURN_VOID(errorCode == BT_NO_ERROR, errorCode);
    }

    void Disconnect(taihe::string_view deviceId)
    {
        HILOGD("enter");
        std::string remoteAddr = std::string(deviceId);
        bool checkRet = CheckDeivceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

        HidHost *profile = HidHost::GetProfile();
        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t errorCode = profile->Disconnect(device);
        TAIHE_BT_ASSERT_RETURN_VOID(errorCode == BT_NO_ERROR, errorCode);
    }

    ohos::bluetooth::constant::ProfileConnectionState GetConnectionState(taihe::string_view deviceId)
    {
        HILOGD("enter");
        std::string remoteAddr = std::string(deviceId);
        bool checkRet = CheckDeivceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM,
            ohos::bluetooth::constant::ProfileConnectionState::from_value(0));

        HidHost *profile = HidHost::GetProfile();
        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t state = static_cast<int32_t>(BTConnectState::DISCONNECTED);
        int32_t errorCode = profile->GetDeviceState(device, state);
        int32_t profileState = TaiheUtils::GetProfileConnectionState(state);
        TAIHE_BT_ASSERT_RETURN(errorCode == BT_NO_ERROR, errorCode,
            ohos::bluetooth::constant::ProfileConnectionState::from_value(profileState));

        return ohos::bluetooth::constant::ProfileConnectionState::from_value(profileState);
    }

    taihe::array<taihe::string> GetConnectedDevices()
    {
        HILOGD("enter");
        HidHost *profile = HidHost::GetProfile();
        std::vector<int> states = { static_cast<int>(BTConnectState::CONNECTED) };
        std::vector<BluetoothRemoteDevice> devices;
        int errorCode = profile->GetDevicesByStates(states, devices);
        TAIHE_BT_ASSERT_RETURN(errorCode == BT_NO_ERROR, errorCode, taihe::array<taihe::string>{});

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

::ohos::bluetooth::hid::HidHostProfile CreateHidHostProfile()
{
    // The parameters in the make_holder function should be of the same type
    // as the parameters in the constructor of the actual implementation class.
    return taihe::make_holder<HidHostProfileImpl, ::ohos::bluetooth::hid::HidHostProfile>();
}
}  // namespace Bluetooth
}  // namespace OHOS

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_CreateHidHostProfile(OHOS::Bluetooth::CreateHidHostProfile);
// NOLINTEND
