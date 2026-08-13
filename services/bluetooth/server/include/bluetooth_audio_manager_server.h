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

#ifndef BLUETOOTH_AUDIO_MANAGER_SERVER_H
#define BLUETOOTH_AUDIO_MANAGER_SERVER_H

#include <string>
#include "bluetooth_types.h"
#include "bluetooth_audio_manager_stub.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothAudioManagerServer : public BluetoothAudioManagerStub {
public:
    BluetoothAudioManagerServer();
    ~BluetoothAudioManagerServer() override;

    int EnableWearDetection(const std::string &deviceId) override;
    int DisableWearDetection(const std::string &deviceId) override;
    int GetWearDetectionState(const std::string &deviceId, int32_t &ability) override;
    int32_t IsDeviceWearing(const BluetoothRawAddress &device) override;
    int32_t SendDeviceSelection(const BluetoothRawAddress &device, int useA2dp, int useHfp, int userSelect) override;
    int32_t IsWearDetectionSupported(const BluetoothRawAddress &device, bool &isSupported) override;
    int32_t GetProfileStatus(const BluetoothRawAddress &device, uint8_t &a2dpState, uint8_t &hfpState) override;

private:
    BLUETOOTH_DECLARE_IMPL();
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothAudioManagerServer);
};

}  // namespace Bluetooth
}  // namespace OHOS
#endif  // BLUETOOTH_AUDIO_MANAGER_SERVER_H
