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

#ifndef BLUETOOTH_AUDIO_MANAGER_INTERFACE
#define BLUETOOTH_AUDIO_MANAGER_INTERFACE
#include <string>
#include "raw_address.h"


namespace OHOS {
namespace bluetooth {

/** status for disable a2dp */
const int NOT_USE_A2DP = 1;
/** status for enable a2dp */
const int USE_A2DP = 2;
/** status for disable hfp */
const int NOT_USE_HFP = 1;
/** status for enable hfp */
const int USE_HFP = 2;
/** status for a2dp or hfp not change */
const int NO_CHANGE = -1;
/** whether a2dp or hfp status changed by user */
const int USER_SELECT = 1;

class BluetoothAudioManagerInterface {
public:
    static int32_t BtAudioManagerSetSupport(std::string addr, bool enable);
    static int32_t BtAudioManagerGetWearState(std::string addr, int32_t &ability);
    static int32_t IsDeviceWearing(const RawAddress &address);
    static int32_t SendDeviceSelection(const RawAddress &address, int useA2dp, int useHfp, int userSelect);
    static int32_t IsWearDetectionSupported(const RawAddress &address, bool &isSupported);
    static int32_t GetProfileStatus(const RawAddress &address, uint8_t &a2dpState, uint8_t &hfpState);
};

class IProfileAudioManager {
public:
    virtual void DealWithNewPlayingStatus() = 0;
};

}
}

#endif
