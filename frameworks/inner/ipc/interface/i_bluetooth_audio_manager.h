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

#include <string>
#include "../parcel/bluetooth_raw_address.h"
#include "iremote_broker.h"

namespace OHOS {
namespace Bluetooth {

class IBluetoothAudioManager : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.ipc.IBluetoothAudioManager");

    virtual int EnableWearDetection(const std::string &deviceId) = 0;
    virtual int DisableWearDetection(const std::string &deviceId) = 0;
    virtual int GetWearDetectionState(const std::string &deviceId, int32_t &ability) = 0;
    virtual int32_t IsWearDetectionSupported(const BluetoothRawAddress &device, bool &isSupported) = 0;
    virtual int32_t SendDeviceSelection(const BluetoothRawAddress &device,
        int useA2dp, int useHfp, int userSelection) = 0;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_AUDIO_MANAGER_INTERFACE_H