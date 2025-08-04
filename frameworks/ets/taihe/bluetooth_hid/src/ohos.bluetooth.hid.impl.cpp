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
#include <iostream>
#include <string>
#include "taihe_bluetooth_hid_host_observer.h"

namespace OHOS {
namespace Bluetooth {

using namespace taihe;
using namespace ohos::bluetooth::hid;

class HidHostProfileImpl {
public:
    HidHostProfileImpl()
    {
        std::shared_ptr<TaiheBluetoothHidHostObserver> observer_ =
            std::make_shared<TaiheBluetoothHidHostObserver>();
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
        HILOGD("ohos::bluetooth::hid connect enter");
        std::string remoteAddr = std::string(deviceId);
        OHOS::Bluetooth::HidHost *profile = OHOS::Bluetooth::HidHost::GetProfile();
        OHOS::Bluetooth::BluetoothRemoteDevice device(remoteAddr, OHOS::Bluetooth::BT_TRANSPORT_BREDR);
        int32_t errorCode = profile->Connect(device);
        HILOGD("connect errorCode:%{public}d", errorCode);
        if (errorCode != OHOS::Bluetooth::BT_NO_ERROR) {
            taihe::set_business_error(errorCode, "Connect return error");
        }
    }

    void disconnect(taihe::string_view deviceId)
    {
        HILOGD("ohos::bluetooth::hid disconnect enter");
        std::string remoteAddr = std::string(deviceId);
        OHOS::Bluetooth::HidHost *profile = OHOS::Bluetooth::HidHost::GetProfile();
        OHOS::Bluetooth::BluetoothRemoteDevice device(remoteAddr, OHOS::Bluetooth::BT_TRANSPORT_BREDR);
        int32_t errorCode = profile->Disconnect(device);
        HILOGD("disconnect errorCode:%{public}d", errorCode);
        if (errorCode != OHOS::Bluetooth::BT_NO_ERROR) {
            taihe::set_business_error(errorCode, "Disconnect return error");
        }
    }

    ohos::bluetooth::constant::ProfileConnectionState GetConnectionState(taihe::string_view deviceId)
    {
        std::string remoteAddr = std::string(deviceId);
        OHOS::Bluetooth::HidHost *profile = OHOS::Bluetooth::HidHost::GetProfile();
        OHOS::Bluetooth::BluetoothRemoteDevice device(remoteAddr, OHOS::Bluetooth::BT_TRANSPORT_BREDR);
        int32_t state = static_cast<int32_t>(OHOS::Bluetooth::BTConnectState::DISCONNECTED);
        int32_t errorCode = profile->GetDeviceState(device, state);
        if (errorCode != OHOS::Bluetooth::BT_NO_ERROR) {
            taihe::set_business_error(errorCode, "Disconnect return error");
        }

        int profileState = GetProfileConnectionState(state);
        return ohos::bluetooth::constant::ProfileConnectionState::from_value(profileState);
    }

    void SetConnectionStrategySync(string_view deviceId, ohos::bluetooth::baseProfile::ConnectionStrategy strategy) {}
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
