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
#ifndef BLUETOOTH_AUDIO_MGR_INFO_H
#define BLUETOOTH_AUDIO_MGR_INFO_H

#pragma once

#include "bluetooth_audio_manager.h"


namespace OHOS {
namespace bluetooth {
class BluetoothAudioManagerInfo {
public:
    static std::string GetActionString(int action);
    static std::string GetPauseReasonString(int pauseReason);
    static int GetWearAction(BluetoothAudioManager::WearingStateRecord record, bool leftIn, bool rightIn);
    static int GetWearActionRecordWearNone(bool leftIn, bool rightIn);
    static int GetWearActionRecordWearDouble(bool leftIn, bool rightIn);
    static int GetWearActionRecordWearLeft(bool leftIn, bool rightIn);
    static int GetWearActionRecordWearRight(bool leftIn, bool rightIn);

    static constexpr int ACTION_NONE = 0;
    static constexpr int ACTION_ADD_DOUBLE = 1;
    static constexpr int ACTION_ADD_LEFT = 2;
    static constexpr int ACTION_ADD_RIGHT = 3;
    static constexpr int ACTION_REMOVE_DOUBLE = 4;
    static constexpr int ACTION_REMOVE_LEFT = 5;
    static constexpr int ACTION_REMOVE_RIGHT = 6;
    static constexpr int ACTION_SWITCH = 7;

    static constexpr int PAUSE_NO_NEED = -1;
    static constexpr int PAUSE_REASON_LEFT_REMOVE_LEFT = 1;
    static constexpr int PAUSE_REASON_RIGHT_REMOVE_RIGHT = 2;
    static constexpr int PAUSE_REASON_DOUBLE_REMOVE_LEFT = 3;
    static constexpr int PAUSE_REASON_DOUBLE_REMOVE_RIGHT = 4;
    static constexpr int PAUSE_REASON_DOUBLE_REMOVE_DOUBLE = 5;
private:
};
}  // namespace bluetooth
}  // namespace OHOS
#endif
