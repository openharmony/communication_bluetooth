/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_BLUETOOTH_AUDIO_MANAGER_INTERFACE_H
#define OHOS_BLUETOOTH_AUDIO_MANAGER_INTERFACE_H

#include "bluetooth_raw_address.h"
#include "bluetooth_service_ipc_interface_code.h"
#include "parcel_bt_uuid.h"
#include "i_bluetooth_ble_peripheral_observer.h"
#include "i_bluetooth_host_observer.h"
#include "i_bluetooth_remote_device_observer.h"
#include "iremote_broker.h"

namespace OHOS {
namespace Bluetooth {

class IBluetoothAudioManager : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.ipc.IBluetoothAudioManager");

    virtual int EnableWearDetection(const std::string &deviceId, int32_t supportVal) = 0;
    virtual int DisableWearDetection(const std::string &deviceId, int32_t supportVal) = 0;
    virtual int IsWearDetectionEnabled(const std::string &deviceId, int32_t &ability) = 0;
    virtual int32_t IsWearDetectionSupported(const std::string &address, bool &isSupported) = 0;
    virtual int32_t SendDeviceSelection(const std::string &address, int useA2dp, int useHfp, int userSelection) = 0;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_AUDIO_MANAGER_INTERFACE_H
