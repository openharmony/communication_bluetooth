/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_fwk_switch_module"
#endif

#include "bluetooth_switch_module.h"

#include <algorithm>
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
static const char *ToString(BluetoothSwitchEvent event)
{
    switch (event) {
        case BluetoothSwitchEvent::ENABLE_BLUETOOTH: return "ENABLE_BLUETOOTH";
        case BluetoothSwitchEvent::DISABLE_BLUETOOTH: return "DISABLE_BLUETOOTH";
        case BluetoothSwitchEvent::ENABLE_BLUETOOTH_TO_RESTRICE_MODE: return "ENABLE_BLUETOOTH_TO_RESTRICE_MODE";
        case BluetoothSwitchEvent::BLUETOOTH_ON: return "BLUETOOTH_ON";
        case BluetoothSwitchEvent::BLUETOOTH_OFF: return "BLUETOOTH_OFF";
        case BluetoothSwitchEvent::BLUETOOTH_HALF: return "BLUETOOTH_HALF";
        default: break;
    }
    return "Unknown";
}

void BluetoothSwitchModule::LogBluetoothSwitchEvent(BluetoothSwitchEvent event)
{
    bool needLog = (event == BluetoothSwitchEvent::BLUETOOTH_ON ||
        event == BluetoothSwitchEvent::BLUETOOTH_OFF ||
        event == BluetoothSwitchEvent::BLUETOOTH_HALF) ? isBtSwitchProcessing_.load() : true;
    if (needLog) {
        HILOGI("Process Event: %{public}s", ToString(event));
    }
}

int BluetoothSwitchModule::ProcessBluetoothSwitchEvent(BluetoothSwitchEvent event)
{
    CHECK_AND_RETURN_LOG_RET(switchAction_, BT_ERR_INTERNAL_ERROR, "switchAction is nullptr");

    std::lock_guard<std::mutex> lock(bluetoothSwitchEventMutex_);
    LogBluetoothSwitchEvent(event);
    switch (event) {
        case BluetoothSwitchEvent::ENABLE_BLUETOOTH:
            return ProcessEnableBluetoothEvent();
        case BluetoothSwitchEvent::DISABLE_BLUETOOTH:
            return ProcessDisableBluetoothEvent();
        case BluetoothSwitchEvent::ENABLE_BLUETOOTH_TO_RESTRICE_MODE:
            return ProcessEnableBluetoothToRestrictModeEvent();
        case BluetoothSwitchEvent::BLUETOOTH_ON:
            return ProcessBluetoothOnEvent();
        case BluetoothSwitchEvent::BLUETOOTH_OFF:
            return ProcessBluetoothOffEvent();
        case BluetoothSwitchEvent::BLUETOOTH_HALF:
            return ProcessBluetoothHalfEvent();
        default: break;
    }
    HILOGI("Invalid event: %{public}s", ToString(event));
    return BT_ERR_INTERNAL_ERROR;
}

void BluetoothSwitchModule::OnTaskTimeout(void)
{
    HILOGW("Bluetooth switch action timeout, clear resources");
    std::lock_guard<std::mutex> lock(bluetoothSwitchEventMutex_);
    isBtSwitchProcessing_ = false;
    cachedEventVec_.clear();
}

int BluetoothSwitchModule::ProcessBluetoothSwitchAction(
    std::function<int(void)> action, BluetoothSwitchEvent cachedEvent)
{
    if (isBtSwitchProcessing_.load()) {
        cachedEventVec_.push_back(cachedEvent);
        HILOGW("BtSwich action is processing, cache the %{public}s event", ToString(cachedEvent));
        return BT_NO_ERROR;
    }

    ffrt::task_attr taskAttr;
    taskAttr.name("bt_switch").delay(taskTimeout_);
    taskTimeoutHandle_ = ffrtQueue_.submit_h([switchWptr = weak_from_this()]() {
        auto switchSptr = switchWptr.lock();
        if (switchSptr == nullptr) {
            HILOGE("switchSptr is nullptr");
            return;
        }
        switchSptr->OnTaskTimeout();
    }, taskAttr);

    isBtSwitchProcessing_ = true;
    int ret = action();
    if (ret != BT_NO_ERROR) {
        isBtSwitchProcessing_ = false;
        ffrtQueue_.cancel(taskTimeoutHandle_);
    }
    // Considering interface compatibility, when a thiry party app invokes the Bluetooth switch interface,
    // a dialog box is displayed, indicating that the call is success.
    if (ret == BT_ERR_DIALOG_FOR_USER_CONFIRM) {
        ret = BT_NO_ERROR;
    }
    return ret;
}

int BluetoothSwitchModule::ProcessEnableBluetoothEvent(void)
{
    return ProcessBluetoothSwitchAction(
        [this]() {
            bool noAutoConnect = noAutoConnect_.load();
            if (noAutoConnect) {
                SetNoAutoConnect(false);
            }
            return switchAction_->EnableBluetooth(noAutoConnect);
        },
        BluetoothSwitchEvent::ENABLE_BLUETOOTH);
}

int BluetoothSwitchModule::ProcessDisableBluetoothEvent(void)
{
    return ProcessBluetoothSwitchAction(
        [this]() { return switchAction_->DisableBluetooth(); },
        BluetoothSwitchEvent::DISABLE_BLUETOOTH);
}

int BluetoothSwitchModule::ProcessEnableBluetoothToRestrictModeEvent(void)
{
    return ProcessBluetoothSwitchAction(
        [this]() { return switchAction_->EnableBluetoothToRestrictMode(); },
        BluetoothSwitchEvent::ENABLE_BLUETOOTH_TO_RESTRICE_MODE);
}

int BluetoothSwitchModule::ProcessBluetoothOnEvent(void)
{
    return ProcessBluetoothSwitchActionEnd(
        BluetoothSwitchEvent::ENABLE_BLUETOOTH,
        {BluetoothSwitchEvent::DISABLE_BLUETOOTH});
}

int BluetoothSwitchModule::ProcessBluetoothOffEvent(void)
{
    return ProcessBluetoothSwitchActionEnd(
        BluetoothSwitchEvent::DISABLE_BLUETOOTH,
        {BluetoothSwitchEvent::ENABLE_BLUETOOTH, BluetoothSwitchEvent::ENABLE_BLUETOOTH_TO_RESTRICE_MODE});
}

int BluetoothSwitchModule::ProcessBluetoothHalfEvent(void)
{
    return ProcessBluetoothSwitchActionEnd(
        BluetoothSwitchEvent::ENABLE_BLUETOOTH_TO_RESTRICE_MODE,
        {BluetoothSwitchEvent::ENABLE_BLUETOOTH, BluetoothSwitchEvent::DISABLE_BLUETOOTH});
}

int BluetoothSwitchModule::ProcessBluetoothSwitchActionEnd(
    BluetoothSwitchEvent curSwitchActionEvent, std::vector<BluetoothSwitchEvent> expectedEventVec)
{
    isBtSwitchProcessing_ = false;
    ffrtQueue_.cancel(taskTimeoutHandle_);
    DeduplicateCacheEvent(curSwitchActionEvent);

    // Expect process the next event is in 'expectedProcessEventVec'
    auto it = std::find_if(cachedEventVec_.begin(), cachedEventVec_.end(), [&expectedEventVec](auto event) {
        return std::find(expectedEventVec.begin(), expectedEventVec.end(), event) != expectedEventVec.end();
    });
    if (it != cachedEventVec_.end()) {
        if (it != cachedEventVec_.begin()) {
            LogCacheEventIgnored(std::vector<BluetoothSwitchEvent>(cachedEventVec_.begin(), it));
        }
        // Ignore the cached event before 'expectedEventVec'
        BluetoothSwitchEvent event = *it;
        cachedEventVec_.erase(cachedEventVec_.begin(), it + 1);
        return ProcessBluetoothSwitchCachedEvent(event);
    }

    cachedEventVec_.clear();
    return BT_NO_ERROR;
}

int BluetoothSwitchModule::ProcessBluetoothSwitchCachedEvent(BluetoothSwitchEvent event)
{
    HILOGI("Auto process cached %{public}s event", ToString(event));
    ffrtQueue_.submit([switchWptr = weak_from_this(), event]() {
        auto switchSptr = switchWptr.lock();
        if (switchSptr == nullptr) {
            HILOGE("switchSptr is nullptr");
            return;
        }
        switchSptr->ProcessBluetoothSwitchEvent(event);
    });
    return BT_NO_ERROR;
}

void BluetoothSwitchModule::DeduplicateCacheEvent(BluetoothSwitchEvent curEvent)
{
    // 从缓存事件列表里，找到最后一个 curEvent，保留该事件之后的缓存事件
    auto it = std::find(cachedEventVec_.rbegin(), cachedEventVec_.rend(), curEvent);
    if (it != cachedEventVec_.rend()) {
        // The it.base() is greater than cachedEventVec_.begin(), so std::distance > 0.
        size_t pos = static_cast<size_t>(std::distance(cachedEventVec_.begin(), it.base())) - 1;

        LogCacheEventIgnored(
            std::vector<BluetoothSwitchEvent>(cachedEventVec_.begin(), cachedEventVec_.begin() + pos + 1));
        cachedEventVec_.erase(cachedEventVec_.begin(), cachedEventVec_.begin() + pos + 1);
    }
}

void BluetoothSwitchModule::LogCacheEventIgnored(std::vector<BluetoothSwitchEvent> eventVec)
{
    std::string log = "";
    for (size_t i = 0; i < eventVec.size(); i++) {
        // The last event current process event, not ignored
        log += ToString(eventVec[i]);
        log += " ";
    }
    if (!log.empty()) {
        HILOGW("Ignore cache event: %{public}s", log.c_str());
    }
}

void BluetoothSwitchModule::SetNoAutoConnect(bool noAutoConnect)
{
    noAutoConnect_ = noAutoConnect;
}
}  // namespace Bluetooth
}  // namespace OHOS
