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

#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "iremote_broker.h"
#include "refbase.h"
#include "bluetooth_def.h"
#include "bluetooth_types.h"

namespace OHOS {
namespace Bluetooth {

class BtAudioManagerCallback {
    virtual ~BtAudioManagerCallback() = default;

    virtual void OnAudioManagerEnabled(int ret);
};

class BLUETOOTH_API BluetoothAudioManager {
public:
    // common
    /**
     * @brief Get default host device.
     *
     * @return Returns the singleton instance.
     * @since 6
     */
    static BluetoothAudioManager &GetInstance();

    void RegisterCallback(BtAudioManagerCallback &btAudioManagerCallback);

    int EnableWearDetection(const std::string &deviceId, int32_t supportVal);
    int DisableWearDetection(const std::string &deviceId, int32_t supportVal);
    int IsWearDetectionEnabled(const std::string &deviceId, int32_t &ability);

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
