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

#ifndef LOG_TAG
#define LOG_TAG "bt_chr_switch"
#endif

#include "bt_chr_switch.h"
#include <map>
#include <vector>
#include "hisysevent.h"
#include "common_util.h"
#include "bluetooth_log.h"
#include "bt_config.h"
#include "parameter.h"
#include "parameters.h"

namespace OHOS {
namespace bluetooth {

namespace {
constexpr const char *SYS_PARAM_LAST_BLUETOOTH_SWITCH_ACTION = "persist.bluetooth.last_bluetooth_switch_action";
constexpr const char *SYS_PARAM_NO_ACTION = "0";
constexpr const char *SYS_PARAM_ENABLE_ACTION = "1";
constexpr const char *SYS_PARAM_DISABLE_ACTION = "2";

const int BLUETOOTH_SWITCH_STATE_VALUE_HALF = 2;
const char *BLUETOOTH_SWITCH_STATE_PROP = "persist.bluetooth.switch_enable";

std::string GetCurrentFormatTime();

struct ChrSwitchInfo {
    int64_t timestamp;  // 本次操作的时间戳
    std::string switchTime;  // 本次操作时间的格式化字符串
    std::string callingName;
    enum SWITCH_ACTION {
        ACTION_INVALID = 0,
        ACTION_ENABLE_BLUETOOTH,
        ACTION_ENABLE_BLUETOOTH_FROM_OFF_TO_HALF,
        ACTION_ENABLE_BLUETOOTH_FROM_HALF_TO_ON,
        ACTION_DISABLE_BLUETOOTH,
    } action;

    void Init(const std::string &name, SWITCH_ACTION actionEnum)
    {
        timestamp = GetTimeStamp();
        switchTime = GetCurrentFormatTime();
        callingName = name;
        action = actionEnum;
        if (callingName != "") {
            // 应用打开蓝牙时（非重启蓝牙自启动场景），将本次开关操作缓存进系统参数，供蓝牙进程重启后使用
            const char *paramValue =
                (actionEnum != ACTION_DISABLE_BLUETOOTH ? SYS_PARAM_ENABLE_ACTION : SYS_PARAM_DISABLE_ACTION);
            SetParameter(SYS_PARAM_LAST_BLUETOOTH_SWITCH_ACTION, paramValue);
        }
    }

    void Clear()
    {
        timestamp = 0;
        switchTime = "";
        callingName = "";
        action = ACTION_INVALID;
        SetParameter(SYS_PARAM_LAST_BLUETOOTH_SWITCH_ACTION, SYS_PARAM_NO_ACTION);
    }
};

std::mutex g_switchInfoMutex {};
bool g_needCheckCrash = true;  // 蓝牙进程重启后，需要检查本次重启是否是因为Crash导致
ChrSwitchInfo g_curSwitchInfo;

std::string GetCurrentFormatTime()
{
    const int FORMAT_TIME_STR_LEN = 20;

    time_t now = time(nullptr);
    char currentTime[FORMAT_TIME_STR_LEN];
    struct tm *localTime = localtime(&now);
    if (localTime == nullptr) {
        return "";
    }
    strftime(currentTime, sizeof(currentTime), "%Y-%m-%d %H:%M:%S", localTime);
    return currentTime;
}

extern "C" BT_FUNC_HOOK void BtChrWriteHiSysEvent(
    int errCode, const std::string &switchTime, const std::string &callingName, int action, int64_t actionTimeStamp)
{
    const char *durationName = (action != ChrSwitchInfo::ACTION_DISABLE_BLUETOOTH ? "BTONDURA" : "BTOFFDURA");
    HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::BT_SERVICE, CHR_BT_SWITCH_EVENT,
        HiviewDFX::HiSysEvent::EventType::FAULT,
        "ERRCODE", errCode,
        "BTSWITCHTIME", switchTime,
        "BTSWITCHAPP", callingName,
        "BTMANASERVICEFLOWSTATE", action,
        durationName, GetTimeStamp() - actionTimeStamp);
}

void BtChrWriteHiSysEvent(int errCode, int action)
{
    BtChrWriteHiSysEvent(
        errCode, g_curSwitchInfo.switchTime, g_curSwitchInfo.callingName, action, g_curSwitchInfo.timestamp);
    g_curSwitchInfo.Clear();
}

const char *EventTypeToStr(int event)
{
    static std::map<int, const char *> eventMap = {
        {EVENT_TYPE_BT_ENABLE, "EVENT_TYPE_BT_ENABLE"},
        {EVENT_TYPE_BT_DISABLE, "EVENT_TYPE_BT_DISABLE"},
        {EVENT_TYPE_BT_ENABLE_SUCCESS, "EVENT_TYPE_BT_ENABLE_SUCCESS"},
        {EVENT_TYPE_BT_DISABLE_SUCCESS, "EVENT_TYPE_BT_DISABLE_SUCCESS"},
        {EVENT_TYPE_BT_ENABLE_FAILED_OPEN_CHIPSET, "EVENT_TYPE_BT_ENABLE_FAILED_OPEN_CHIPSET"},
        {EVENT_TYPE_BT_ENABLE_FROM_HALF_TO_ON, "EVENT_TYPE_BT_ENABLE_FROM_HALF_TO_ON"},
    };

    const char *p = "UNDEFINED_EVENT";
    auto it = eventMap.find(event);
    if (it != eventMap.end()) {
        p = it->second;
    }
    return p;
}

void CheckBluetoothCrash()
{
    int action = GetIntParameter(SYS_PARAM_LAST_BLUETOOTH_SWITCH_ACTION, 0);
    if (action != ChrSwitchInfo::ACTION_INVALID) {
        HILOGW("bluetooth action(%{public}d) failed due to bluetooth crash", action);
        BtChrWriteHiSysEvent(BT_CHR_SWITCH_FAILED_FREEZE_CRASH, action);
    }
    g_curSwitchInfo.Clear();
}

void CheckBluetoothSwitchActionStart()
{
    if (g_curSwitchInfo.action != ChrSwitchInfo::ACTION_INVALID) {
        HILOGW("bluetooth action(%{public}d) failed due to unknown reason", g_curSwitchInfo.action);
        BtChrWriteHiSysEvent(BT_CHR_SWITCH_FAILED, g_curSwitchInfo.action);
        g_curSwitchInfo.Clear();
    }
}

bool IsBluetoothInHalfState(void)
{
    return (GetIntParameter(BLUETOOTH_SWITCH_STATE_PROP, 0) == BLUETOOTH_SWITCH_STATE_VALUE_HALF);
}
}  // namespace

void BtChrWriteSwitchEvent(ChrSwitchEvent event)
{
    std::lock_guard<std::mutex> lock(g_switchInfoMutex);
    // 进程重启首次调用时，检查是否是因为Crash导致的蓝牙打开失败
    if (g_needCheckCrash) {
        CheckBluetoothCrash();
        g_needCheckCrash = false;
    }

    HILOGI("recv event %{public}s", EventTypeToStr(event.eventType));
    switch (event.eventType) {
        case EVENT_TYPE_BT_ENABLE: {
            CheckBluetoothSwitchActionStart();
            g_curSwitchInfo.Init(event.callingName, ChrSwitchInfo::ACTION_ENABLE_BLUETOOTH);
            break;
        }
        case EVENT_TYPE_BT_ENABLE_SUCCESS: {
            // 上一次动作为打开蓝牙，认为成功
            int errCode = (g_curSwitchInfo.action == ChrSwitchInfo::ACTION_ENABLE_BLUETOOTH)
                ? BT_CHR_SWITCH_SUCCESS : BT_CHR_SWITCH_FAILED;
            auto action = IsBluetoothInHalfState() ?
                ChrSwitchInfo::ACTION_ENABLE_BLUETOOTH_FROM_OFF_TO_HALF :
                ChrSwitchInfo::ACTION_ENABLE_BLUETOOTH;
            BtChrWriteHiSysEvent(errCode, action);
            break;
        }
        case EVENT_TYPE_BT_DISABLE: {
            CheckBluetoothSwitchActionStart();
            g_curSwitchInfo.Init(event.callingName, ChrSwitchInfo::ACTION_DISABLE_BLUETOOTH);
            break;
        }
        case EVENT_TYPE_BT_DISABLE_SUCCESS: {
            // 上一次动作为关蓝牙，认为成功
            int errCode = (g_curSwitchInfo.action == ChrSwitchInfo::ACTION_DISABLE_BLUETOOTH)
                ? BT_CHR_SWITCH_SUCCESS : BT_CHR_SWITCH_FAILED;
            BtChrWriteHiSysEvent(errCode, ChrSwitchInfo::ACTION_DISABLE_BLUETOOTH);
            break;
        }
        case EVENT_TYPE_BT_ENABLE_FAILED_OPEN_CHIPSET: {
            // 重启场景，芯片会有开机校准流程，会先开蓝牙芯片，再关蓝牙芯片，期间BTH打开蓝牙，会有一次蓝牙芯片打开失败，属于误报场景，需屏蔽。
            if (g_curSwitchInfo.callingName != "bluetooth_reboot") {
                BtChrWriteHiSysEvent(BT_CHR_SWITCH_FAILED_OPEN_CHIPSET, ChrSwitchInfo::ACTION_ENABLE_BLUETOOTH);
            }
            g_curSwitchInfo.Clear();
            break;
        }
        case EVENT_TYPE_BT_ENABLE_FROM_HALF_TO_ON: {
            auto action = ChrSwitchInfo::ACTION_ENABLE_BLUETOOTH_FROM_HALF_TO_ON;
            g_curSwitchInfo.Init(event.callingName, action);
            BtChrWriteHiSysEvent(BT_CHR_SWITCH_SUCCESS, action);
            break;
        }
        default:
            break;
    }
}
}  // namespace Bluetooth
}  // namespace OHOS
