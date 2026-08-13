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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_audio_manager_info"
#endif

#include "bluetooth_audio_manager_info.h"
#include <string>

namespace OHOS {
namespace bluetooth {

std::string BluetoothAudioManagerInfo::GetActionString(int action)
{
    std::string str;
    switch (action) {
        case ACTION_NONE:
            str = "ACTION_NONE";
            break;
        case ACTION_ADD_DOUBLE:
            str = "ACTION_ADD_DOUBLE";
            break;
        case ACTION_ADD_LEFT:
            str = "ACTION_ADD_LEFT";
            break;
        case ACTION_ADD_RIGHT:
            str = "ACTION_ADD_RIGHT";
            break;
        case ACTION_REMOVE_DOUBLE:
            str = "ACTION_REMOVE_DOUBLE";
            break;
        case ACTION_REMOVE_LEFT:
            str = "ACTION_REMOVE_LEFT";
            break;
        case ACTION_REMOVE_RIGHT:
            str = "ACTION_REMOVE_RIGHT";
            break;
        case ACTION_SWITCH:
            str = "ACTION_SWITCH";
            break;
        default:
            str = "UNKNOWN";
            break;
    }
    return str;
}

std::string BluetoothAudioManagerInfo::GetPauseReasonString(int pauseReason)
{
    std::string str;
    switch (pauseReason) {
        case PAUSE_NO_NEED:
            str = "PAUSE_NO_NEED";
            break;
        case PAUSE_REASON_LEFT_REMOVE_LEFT:
            str = "PAUSE_REASON_LEFT_REMOVE_LEFT";
            break;
        case PAUSE_REASON_RIGHT_REMOVE_RIGHT:
            str = "PAUSE_REASON_RIGHT_REMOVE_RIGHT";
            break;
        case PAUSE_REASON_DOUBLE_REMOVE_LEFT:
            str = "PAUSE_REASON_DOUBLE_REMOVE_LEFT";
            break;
        case PAUSE_REASON_DOUBLE_REMOVE_RIGHT:
            str = "PAUSE_REASON_DOUBLE_REMOVE_RIGHT";
            break;
        case PAUSE_REASON_DOUBLE_REMOVE_DOUBLE:
            str = "PAUSE_REASON_DOUBLE_REMOVE_DOUBLE";
            break;
        default:
            str = "UNKNOWN";
            break;
    }
    return str;
}
int BluetoothAudioManagerInfo::GetWearActionRecordWearNone(bool leftIn, bool rightIn)
{
    if (leftIn && rightIn) {
        return ACTION_ADD_DOUBLE; // 00->11
    } else if (leftIn) {
        return ACTION_ADD_LEFT; // 00->10
    } else if (rightIn) {
        return ACTION_ADD_RIGHT; // 00->01
    } else {
        return ACTION_NONE;
    }
}

int BluetoothAudioManagerInfo::GetWearActionRecordWearDouble(bool leftIn, bool rightIn)
{
    if (!leftIn && !rightIn) {
        return ACTION_REMOVE_DOUBLE; // 11->00
    } else if (!leftIn) {
        return ACTION_REMOVE_LEFT; // 11->01
    } else if (!rightIn) {
        return ACTION_REMOVE_RIGHT; // 11->10
    } else {
        return ACTION_NONE;
    }
}

int BluetoothAudioManagerInfo::GetWearActionRecordWearLeft(bool leftIn, bool rightIn)
{
    if (!leftIn && !rightIn) {
        return ACTION_REMOVE_LEFT; // 10->00
    } else if (leftIn && rightIn) {
        return ACTION_ADD_RIGHT; // 10->11
    } else if (!leftIn) {
        return ACTION_SWITCH; // 10->01
    } else {
        return ACTION_NONE;
    }
}

int BluetoothAudioManagerInfo::GetWearActionRecordWearRight(bool leftIn, bool rightIn)
{
    if (!leftIn && !rightIn) {
        return ACTION_REMOVE_RIGHT; // 01->00
    } else if (leftIn && rightIn) {
        return ACTION_ADD_LEFT; // 01->11
    } else if (leftIn) {
        return ACTION_SWITCH; // 01->10
    } else {
        return ACTION_NONE;
    }
}

int BluetoothAudioManagerInfo::GetWearAction(BluetoothAudioManager::WearingStateRecord record,
                                             bool leftIn, bool rightIn)
{
    if ((record.leftIn_ == leftIn) && (record.rightIn_ == rightIn)) {
        return ACTION_NONE;
    }
    if (!record.leftIn_ && !record.rightIn_) {
        return GetWearActionRecordWearNone(leftIn, rightIn);
    } else if (record.leftIn_ && record.rightIn_) {
        return GetWearActionRecordWearDouble(leftIn, rightIn);
    } else if (record.leftIn_ && !record.rightIn_) {
        return GetWearActionRecordWearLeft(leftIn, rightIn);
    } else {
        return GetWearActionRecordWearRight(leftIn, rightIn);
    }
}
}
}

