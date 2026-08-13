/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef BT_CHR_SWITCH_H
#define BT_CHR_SWITCH_H

#include "bt_chr_dft_exception.h"

namespace OHOS {
namespace bluetooth {
enum BtChrSwitchError {
    BT_CHR_SWITCH_SUCCESS = 0,
    BT_CHR_SWITCH_FAILED = 1,
    BT_CHR_SWITCH_FAILED_OPEN_CHIPSET = 2,  // 开蓝牙过程中，打开芯片驱动节点失败
    BT_CHR_SWITCH_FAILED_FREEZE_CRASH = 3,  // 开蓝牙过程中，蓝牙异常Freeze或Crash，导致本次开蓝牙失败
};

struct ChrSwitchEvent {
    explicit ChrSwitchEvent(int type) : callingName(""), eventType(type) {}
    ChrSwitchEvent(std::string name, int type) : callingName(name), eventType(type) {}
    ~ChrSwitchEvent() = default;

    std::string callingName;
    int eventType;
};

void BtChrWriteSwitchEvent(ChrSwitchEvent event);

#ifdef __cplusplus
extern "C" {
#endif
// Used for DT mock
void BtChrWriteHiSysEvent(
    int errCode, const std::string &switchTime, const std::string &callingName, int action, int64_t actionTimeStamp);
#ifdef __cplusplus
}
#endif

}  // namespace Bluetooth
}  // namespace OHOS
#endif /* BT_CHR_SWITCH_H */