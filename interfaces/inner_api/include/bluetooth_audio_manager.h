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

#ifndef BLUETOOTH_AUDIO_MANAGER_H
#define BLUETOOTH_AUDIO_MANAGER_H

#include <cstdint>
#include <string>

#include "iremote_broker.h"
#include "refbase.h"
#include "bluetooth_def.h"
#include "bluetooth_types.h"
#include "bluetooth_remote_device.h"

namespace OHOS {
namespace Bluetooth {

class BLUETOOTH_API BluetoothAudioManager {
public:
    /**
     * @brief Get audio manager.
     *
     * @return Returns the singleton instance.
     * @since 6
     */
    static BluetoothAudioManager &GetInstance();

    int EnableWearDetection(const std::string &deviceId);
    int DisableWearDetection(const std::string &deviceId);
    int GetWearDetectionState(const std::string &deviceId, int32_t &ability);

    /**
     * @brief Whether wear detection is supported
     *
     * @param device Remote device.
     * @param isSupported Headset wearing detection capability.
     * @return Returns {@link BT_NO_ERROR} if the operation fails.
     *         returns an error code defined in {@link BtErrCode} otherwise.
     * @since 11
     */
    int IsWearDetectionSupported(const BluetoothRemoteDevice &device, bool &isSupported);

    /**
    * @brief Report to the remote device that the user has selected the device.
    * @param device Remote device.
    * @param useA2dp whether use A2dp of this device.
    * @param useHfp whether use Hfp of this device.
    * @param userSelection whether this send command by user selection.
    * @return Returns {@link BT_NO_ERROR} if send selection success;
    * returns an error code defined in {@link BtErrCode} otherwise.
    */
    int SendDeviceSelection(const BluetoothRemoteDevice &device, int useA2dp, int useHfp, int userSelection) const;

private:
    /**
     * @brief A constructor used to create a <b>BluetoothHost</b> instance.
     *
     * @since 6
     */
    BluetoothAudioManager();

    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothAudioManager);
    BLUETOOTH_DECLARE_IMPL();

};
}  // namespace Bluetooth
}  // namespace OHOS
#endif
