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
#ifndef LOG_TAG
#define LOG_TAG "bt_server_ble_scanner"
#endif

#include "ble_scanner_state_machine.h"
#include <algorithm>
#include <cmath>
#include <memory>

#include "log.h"
#include "hitrace_meter.h"
#include "bt_def.h"

using namespace utility;

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;

bool operator < (const BleScannerStateMachine::ScannerParameters &lhs,
    const BleScannerStateMachine::ScannerParameters &rhs);
bool operator <= (const BleScannerStateMachine::ScannerParameters &lhs,
    const BleScannerStateMachine::ScannerParameters &rhs);
bool operator < (const BleScannerStateMachine::ScannerAppInfo &lhs, const BleScannerStateMachine::ScannerAppInfo &rhs)
{
    return operator < (lhs.params, rhs.params);
}
bool operator <= (const BleScannerStateMachine::ScannerAppInfo &lhs, const BleScannerStateMachine::ScannerAppInfo &rhs)
{
    return operator <= (lhs.params, rhs.params);
}

#define CHECK_PTR_RETURN(ptr, stateStr, ...) \
do { \
    if (!(ptr)) {   \
        HILOGE("" #ptr " is null in " stateStr); \
        sm_.Reset(); \
        return __VA_ARGS__; \
    } \
} while (0)

namespace {
bluetooth::BleScanSettingsImpl ConvertScanSetting(BleScannerMessage msg)
{
    BleScannerStateMachine::ScannerParameters params = msg.appInfo.params;
    bluetooth::BleScanSettingsImpl setting;
    setting.SetReportDelay(params.reportDelayMillis);
    setting.SetScanMode(params.scanMode);
    setting.SetLegacy(params.legacy);
    setting.SetPhy(params.phy);
    setting.SetCallbackType(params.callbackType);
    setting.SetMatchTrackAdvType(params.matchTrackAdvType);
    setting.SetMatchMode(params.matchMode);
    setting.SetScanInterval(msg.scanParams.scanInterval);
    setting.SetScanWindow(msg.scanParams.scanWindow);
    return setting;
}

const char *EventToStr(int event, bool autoTrigger = false)
{
    // eventId <-> (event_str, auto_trigger_event_str)
    static std::map<int, std::pair<const char *, const char *>> table = {
        {BleScannerStateMachine::START_EVENT, {"START_EVENT", "INNER_START_EVENT"}},
        {BleScannerStateMachine::STOP_EVENT, {"STOP_EVENT", "INNER_STOP_EVENT"}},
        {BleScannerStateMachine::START_SUCCESS_EVENT, {"START_SUCCESS_EVENT", "INNER_START_SUCCESS_EVENT"}},
        {BleScannerStateMachine::START_FAILED_EVENT, {"START_FAILED_EVENT", "INNER_START_FAILED_EVENT"}},
        {BleScannerStateMachine::STOP_SUCCESS_EVENT, {"STOP_SUCCESS_EVENT", "INNER_STOP_SUCCESS_EVENT"}},
        {BleScannerStateMachine::STOP_FAILED_EVENT, {"STOP_FAILED_EVENT", "INNER_STOP_FAILED_EVENT"}},
#ifdef CONTEXTHUB_BLE_V3
        {BleScannerStateMachine::SH_COLLABORATION_EVENT, {"SH_COLLABORATION_EVENT", "INNER_SH_COLLABORATION_EVENT"}},
#endif
    };

    const char *p = "UNDEFINED";
    auto it = table.find(event);
    if (it != table.end()) {
        p = (!autoTrigger ? it->second.first : it->second.second);
    }
    return p;
}
}  // namespace {}

#ifdef CONTEXTHUB_BLE_V3
void BleScannerStateMachine::InitCollaborationTimer()
{
    auto timeoutFunc = [wptr = weak_from_this()]() {
        HILOGI("Collaboration timeout");
        auto sptr = wptr.lock();
        if (!sptr) {
            HILOGE("BleScannerStateMachine is not exist");
            return;
        }
        sptr->postTaskFuc_([sptr]() {
            const utility::StateMachine::State *currentState = sptr->GetState();
            if (currentState != nullptr && currentState->Name() == BLE_SCAN_SH_COLLABORATION) {
                HILOGI("sh collaboration timeout");
                sptr->SendMessage(BleScannerMessage(SH_COLLABORATION_EVENT, SH_COLLABORATION_NOTIFY_LPDEVICE_STATUS));
            } else {
                HILOGW("not in collaboration state, current state: %{public}s",
                    (currentState != nullptr) ? currentState->Name().c_str() : "unknown");
            }
        });
    };

    collaborationTimeoutTimer_ = std::make_shared<utility::Timer>(timeoutFunc);
}

void BleScannerStateMachine::Init(PostTaskFunc postTaskFunc, StartScanFunc startScanFunc, StopScanFunc stopScanFunc,
    ReportStartedEventFunc reportStartedEventFunc, ReportStoppedEventFunc reportStoppedEventFunc,
    const CollabrationFunc &collaboratioFunc)
#else
void BleScannerStateMachine::Init(PostTaskFunc postTaskFunc, StartScanFunc startScanFunc, StopScanFunc stopScanFunc,
    ReportStartedEventFunc reportStartedEventFunc, ReportStoppedEventFunc reportStoppedEventFunc)
#endif
{
    postTaskFuc_ = postTaskFunc;
    startScanFunc_ = startScanFunc;
    stopScanFunc_ = stopScanFunc;
    reportStartedEventFunc_ = reportStartedEventFunc;
    reportStoppedEventFunc_ = reportStoppedEventFunc;
#ifdef CONTEXTHUB_BLE_V3
    sendCollaborationFunc_ = collaboratioFunc.sendCollaborationFunc;
    scanCollaborationEnabledFunc_ = collaboratioFunc.scanCollaborationEnabledFunc;
#endif

    auto timeoutFunc = [wptr = weak_from_this()]() {
        HITRACE_METER_NAME(BT_TRACE_TAG, "BleScannerStateMachine::Init");
        HILOGI("Timeout");
        auto sptr = wptr.lock();
        if (!sptr) {
            HILOGE("BleScannerStateMachine is not exist");
            return;
        }
        sptr->postTaskFuc_([sptr]() {
            sptr->Reset();
        });
    };
    timeoutTimer_ = std::make_shared<utility::Timer>(timeoutFunc);

    std::unique_ptr<utility::StateMachine::State> stopped = std::make_unique<BleScannerStoppedState>(*this);
    StateMachine::Move(stopped);
    std::unique_ptr<utility::StateMachine::State> starting = std::make_unique<BleScannerStartingState>(*this);
    StateMachine::Move(starting);
    std::unique_ptr<utility::StateMachine::State> started = std::make_unique<BleScannerStartedState>(*this);
    StateMachine::Move(started);
    std::unique_ptr<utility::StateMachine::State> stopping = std::make_unique<BleScannerStoppingState>(*this);
    StateMachine::Move(stopping);
    std::unique_ptr<utility::StateMachine::State> innerStopping = std::make_unique<BleScannerInnerStoppingState>(*this);
    StateMachine::Move(innerStopping);
#ifdef CONTEXTHUB_BLE_V3
    std::unique_ptr<utility::StateMachine::State> shCollaboration = std::make_unique<BleScannerShCollaboration>(*this);
    StateMachine::Move(shCollaboration);

    InitCollaborationTimer();
#endif

    isStarted_ = true;
    StateMachine::InitState(BLE_SCAN_STOPPED);
}

void BleScannerStateMachine::Reset(void)
{
    HILOGI("[Reset]");
    reqApp_ = nullptr;
    currentApp_ = nullptr;
    appQueue_.clear();
    priorityEventQueue_.clear();
    StateMachine::InitState(BLE_SCAN_STOPPED);
    if (timeoutTimer_ != nullptr) {
        timeoutTimer_->Stop();
    }

#ifdef CONTEXTHUB_BLE_V3
    isCollaborating_ = false;
    collaborationFromStarted_ = false;
#endif
}

bool BleScannerStateMachine::SendMessage(const BleScannerMessage &msg)
{
    if (!isStarted_.load()) {
        HILOGE("scanner state machine not started, need call Init() first");
        return false;
    }
    postTaskFuc_([wptr = weak_from_this(), msg]() {
        auto sptr = wptr.lock();
        if (!sptr) {
            HILOGE("BleScannerStateMachine is not exist");
            return;
        }
        sptr->ProcessMessage(msg);
    });
    return true;
}

void BleScannerStoppedState::Entry()
{
    HILOGD("STOPPED");

#ifdef CONTEXTHUB_BLE_V3
    bool started = false;
#endif

    // If START_EVENT or STOP_EVENT trigger in this function, it will be Inner Event.
    sm_.isInnerEvent_ = true;
    // Try trigger start event
    while (!sm_.priorityEventQueue_.empty()) {
        auto event = sm_.priorityEventQueue_.front();
        if (event.what_ == BleScannerStateMachine::START_EVENT) {
            sm_.priorityEventQueue_.pop_front();
            Dispatch(event);
#ifdef CONTEXTHUB_BLE_V3
            started = true;
#endif
            break;
        } else {
            HILOGI("priorityEventQueue_ unknown [%{public}s] in StateStopped, ignore",
                EventToStr(event.what_));
            sm_.priorityEventQueue_.pop_front();
        }
    }
    sm_.isInnerEvent_ = false;

#ifdef CONTEXTHUB_BLE_V3
    if (!started) {
        sm_.SendCollaborationResult(false);
    }
#endif
}

void BleScannerStoppedState::Exit()
{
    HILOGD("STOPPED");
}

bool BleScannerStoppedState::Dispatch(const utility::Message &utilMsg)
{
    auto msg = static_cast<const BleScannerMessage &>(utilMsg);

    switch (msg.what_) {
        case BleScannerStateMachine::START_EVENT:
#ifdef CONTEXTHUB_BLE_V3
            if (!sm_.isInnerEvent_ && sm_.IsNeedCollboration()) {
                StartCollobration(msg);
                break;
            }
#endif
            HILOGI("[STOPPED] -> [STARTING] after event [%{public}s], %{public}s",
                EventToStr(msg.what_, sm_.isInnerEvent_), msg.appInfo.ToString().c_str());
            Transition(BLE_SCAN_STARTING);
            sm_.startScanFunc_(ConvertScanSetting(msg));
            sm_.reqApp_ = std::make_shared<BleScannerStateMachine::ScannerAppInfo>(msg.appInfo);
            sm_.reqParam_ = msg.scanParams;
            break;
        case BleScannerStateMachine::STOP_EVENT:
            HILOGE("Unsupported [%{public}s] in StateStopped.", EventToStr(msg.what_));
            sm_.reportStoppedEventFunc_(msg.appInfo.pid, msg.appInfo.uid, msg.appInfo.scannerId,
                bluetooth::SCAN_NOT_STARTED);
            break;
        default:
#ifndef CONTEXTHUB_BLE_V3
            HILOGE("Unsupported [%{public}s] in StateStopped.", EventToStr(msg.what_));
            return false;
#else
            return BleScannerState::Dispatch(utilMsg);
#endif
    }
    return true;
}

void BleScannerStateMachine::PushPriorityEventFront(const BleScannerMessage &event)
{
    if (priorityEventQueue_.size() >= PRIORITY_Q_MAX_EVENT) {
        HILOGE("priority queue is full, ignore [%{public}s]", EventToStr(event.what_));
        return;
    }
    priorityEventQueue_.push_front(event);
}

void BleScannerStateMachine::PushPriorityEventTail(const BleScannerMessage &event)
{
    if (priorityEventQueue_.size() >= PRIORITY_Q_MAX_EVENT) {
        HILOGE("priority queue is full, ignore [%{public}s]", EventToStr(event.what_));
        return;
    }
    priorityEventQueue_.push_back(event);
}

void BleScannerStateMachine::PushPriorityStartEventFront(void)
{
    if (!appQueue_.empty()) {
        BleScannerStateMachine::ScannerParameters optimalParam;
        auto maxDutyCycleIter = GetOptimalParam(appQueue_, optimalParam);
        if (maxDutyCycleIter != appQueue_.end()) {
            auto event = BleScannerMessage(BleScannerStateMachine::START_EVENT, *maxDutyCycleIter, optimalParam);
            PushPriorityEventFront(event);
            isStartByStateMachine_ = true;
        }
    }
}

#ifdef CONTEXTHUB_BLE_V3
void BleScannerStateMachine::SendCollaborationResult(bool started)
{
    isCollaborating_ = false;
    ownedScannerId_ = BLE_SCAN_INVALID_ID;

    std::vector<ScannerAppInfo> appInfos;
    if (!started || currentApp_ == nullptr) {
        sendCollaborationFunc_(BLE_LPDEVICE_MSG_SCAN_COLLABORATION_RESULT, appInfos);
        return;
    }

    appInfos.emplace_back(*currentApp_);

    ScannerAppInfo maxApp;
    if (currentApp_->scannerId == LP_DEVICE_SCANNER_ID) {
        for (auto it = appQueue_.begin(); it != appQueue_.end(); ++it) {
            if (it->scannerId != LP_DEVICE_SCANNER_ID && (maxApp.params.scanInterval == 0 || maxApp < *it)) {
                maxApp = *it;
            }
        }

        if (maxApp.params.scanInterval != 0) {
            HILOGI("notify max app w/o lpdevice scan app %{public}s", maxApp.ToString().c_str());
            appInfos.emplace_back(maxApp);
            ownedScannerId_ = maxApp.scannerId;
        }
    }

    sendCollaborationFunc_(BLE_LPDEVICE_MSG_SCAN_COLLABORATION_RESULT, appInfos);
}

void BleScannerStateMachine::RemoveLpDeviceScanner()
{
    auto it = GetAppIter(ScannerAppInfo(0, 0, LP_DEVICE_SCANNER_ID));
    if (it != appQueue_.end()) { // lp device scanner id existed
        SendMessage(BleScannerMessage(BleScannerStateMachine::STOP_EVENT, *it));
    }
}

void BleScannerStateMachine::HandlerCollaborationNotifyStatusEvent(const ScannerAppInfo &app)
{
    if (app.params.scanInterval == 0 || app.params.scanWindow == 0) {
        HILOGI("not started");
        RemoveLpDeviceScanner();
        return;
    }

    ScannerAppInfo lpDeviceApp(0, 0, LP_DEVICE_SCANNER_ID, app.params);
    auto it = GetAppIter(lpDeviceApp);
    if (it == appQueue_.end()) { // sensorhub scanner id not existed
        HILOGI("add lp device scanner %{public}s", lpDeviceApp.ToString().c_str());
        PushPriorityEventTail(BleScannerMessage(BleScannerStateMachine::START_EVENT, lpDeviceApp));
    } else {
        if (!(lpDeviceApp < *it) && !(*it < lpDeviceApp)) {
            HILOGI("lp device scan parameter %{public}s not changed", it->ToString().c_str());
            return;
        }

        HILOGI("lp device scanner parameter changed from %{public}s to %{public}s",
            it->ToString().c_str(), lpDeviceApp.ToString().c_str());
        PushPriorityEventTail(BleScannerMessage(BleScannerStateMachine::STOP_EVENT, *it));
        PushPriorityEventTail(BleScannerMessage(BleScannerStateMachine::START_EVENT, lpDeviceApp));
    }
}

void BleScannerStateMachine::HandlerCollaborationEvent(int subtype)
{
    switch (subtype) {
        case SH_COLLABORATION_REMOVE_LP_DEVICE_SCAN:
            HILOGI("lp device scan is revmoed");
            RemoveLpDeviceScanner();
            break;
        default:
            HILOGE("unknown subtype");
            break;
    }
}

void BleScannerState::StartCollobration(const BleScannerMessage &msg)
{
    HILOGI("[%{public}s] -> [SH_COLLABORAION] in event [%{public}s], %{public}s",
        Name().c_str(), EventToStr(msg.what_, sm_.isInnerEvent_), msg.appInfo.ToString().c_str());
    sm_.PushPriorityEventFront(msg);
    sm_.collaborationFromStarted_ = (Name() == BLE_SCAN_STARTED);
    Transition(BLE_SCAN_SH_COLLABORATION);
}

bool BleScannerState::Dispatch(const utility::Message &utilMsg)
{
    auto msg = static_cast<const BleScannerMessage &>(utilMsg);
    switch (msg.what_) {
        case BleScannerStateMachine::START_EVENT:
        case BleScannerStateMachine::STOP_EVENT:
            HILOGI("pending [%{public}s] in [%{public}s], %{public}s", EventToStr(msg.what_),
                Name().c_str(), msg.appInfo.ToString().c_str());
            sm_.PushPriorityEventTail(msg);
            break;
        case BleScannerStateMachine::SH_COLLABORATION_EVENT:
            HILOGI("handle [%{public}s] in [%{public}s], sub event: %{public}d",
                EventToStr(msg.what_), Name().c_str(), msg.arg1_);
            sm_.HandlerCollaborationEvent(msg.arg1_);
            break;
        default:
            HILOGE("Unsupported [%{public}s] in State [%{public}s].",
                EventToStr(msg.what_), Name().c_str());
            return false;
    }

    return true;
}
#endif

void BleScannerStartingState::Entry()
{
    HILOGD("STARTING");
    sm_.timeoutTimer_->Start(sm_.timeoutMs_);
}

void BleScannerStartingState::Exit()
{
    HILOGD("STARTING");
    sm_.timeoutTimer_->Stop();
}

bool BleScannerStartingState::Dispatch(const utility::Message &utilMsg)
{
    CHECK_PTR_RETURN(sm_.reqApp_, "StateStarting", false);

    auto msg = static_cast<const BleScannerMessage &>(utilMsg);
    switch (msg.what_) {
        case BleScannerStateMachine::START_SUCCESS_EVENT:
            HILOGD("[STARTING] -> [STARTED] after event [START_SUCCESS_EVENT]");
            Transition(BLE_SCAN_STARTED);
            break;
        case BleScannerStateMachine::START_FAILED_EVENT: {
            if (!sm_.isStartByStateMachine_.load()) {
                sm_.reportStartedEventFunc_(sm_.reqApp_->pid, sm_.reqApp_->uid, sm_.reqApp_->scannerId,
                    bluetooth::SCAN_FAILED_INTERNAL_ERROR);
            }
            sm_.isStartByStateMachine_ = false;
            sm_.RemoveApp(*sm_.reqApp_);
            sm_.PushPriorityStartEventFront();
            HILOGI("[STARTING] -> [STOPPED] after event [START_FAILED_EVENT]");
            Transition(BLE_SCAN_STOPPED);
            break;
        }
#ifndef CONTEXTHUB_BLE_V3
        case BleScannerStateMachine::START_EVENT:
        case BleScannerStateMachine::STOP_EVENT:
            HILOGI("pending [%{public}s] in [STARTING], %{public}s", EventToStr(msg.what_),
                msg.appInfo.ToString().c_str());
            sm_.PushPriorityEventTail(msg);
            break;
        default:
            HILOGE("Unsupported [%{public}s] in StateStarting.", EventToStr(msg.what_));
            return false;
#else
        default:
            return BleScannerState::Dispatch(utilMsg);
#endif
    };
    return true;
}

void BleScannerStartedState::HandlePriorityEvent(void)
{
    if (!sm_.priorityEventQueue_.empty()) {
        auto event = sm_.priorityEventQueue_.front();
        sm_.priorityEventQueue_.pop_front();
        Dispatch(event);
    }
}

void BleScannerStartedState::Entry()
{
    HILOGD("STARTED");
    CHECK_PTR_RETURN(sm_.reqApp_, "StateStarted");

    // After the application with the highest scanning duty cycle stops scanning, the application that automatically
    // starts scanning next time is still in the appQueue. Delete the application and add it again for updating.
    // In other scenarios, RemoveApp does not take effect.
    sm_.RemoveApp(*sm_.reqApp_);
    sm_.appQueue_.push_back(*sm_.reqApp_);
    sm_.currentApp_ = sm_.reqApp_;
    sm_.optimalParam_ = sm_.reqParam_;
    sm_.reqApp_ = nullptr;
    if (!sm_.isStartByStateMachine_.load()) {
        sm_.reportStartedEventFunc_(sm_.currentApp_->pid, sm_.currentApp_->uid, sm_.currentApp_->scannerId,
            bluetooth::SCAN_SUCCESS);
    }
    sm_.isStartByStateMachine_ = false;
    HILOGI("current app:%{public}s, window:%{public}d, interval:%{public}d",
        sm_.currentApp_->ToString().c_str(), sm_.optimalParam_.scanWindow, sm_.optimalParam_.scanInterval);
    // If START_EVENT or STOP_EVENT trigger in this function, it will be Inner Event.
    sm_.isInnerEvent_ = true;
    HandlePriorityEvent();
    sm_.isInnerEvent_ = false;

#ifdef CONTEXTHUB_BLE_V3
    if (sm_.priorityEventQueue_.empty()) {
        sm_.SendCollaborationResult(true);
    }
#endif
}

void BleScannerStartedState::Exit()
{
    HILOGD("STARTED");
}

bool BleScannerStartedState::HandleStartEvent(const BleScannerMessage &msg)
{
    if (sm_.IsExistApp(msg.appInfo)) {
        HILOGE("app is exist, %{public}s", msg.appInfo.ToString().c_str());
        sm_.reportStartedEventFunc_(msg.appInfo.pid, msg.appInfo.uid, msg.appInfo.scannerId,
            bluetooth::SCAN_FAILED_ALREADY_STARTED);
        HandlePriorityEvent();
        return false;
    }

    // Assuming app is added to the appQueue for calculating the optimal parameter.
    sm_.appQueue_.push_back(msg.appInfo);
    BleScannerStateMachine::ScannerParameters optimalParam;
    sm_.GetOptimalParam(sm_.appQueue_, optimalParam);
    if (optimalParam.scanInterval == sm_.optimalParam_.scanInterval &&
        optimalParam.scanWindow == sm_.optimalParam_.scanWindow) {
        HILOGI("Add app %{public}s", msg.appInfo.ToString().c_str());
        sm_.reportStartedEventFunc_(msg.appInfo.pid, msg.appInfo.uid, msg.appInfo.scannerId, bluetooth::SCAN_SUCCESS);
        HandlePriorityEvent();
        return true;
    }

#ifdef CONTEXTHUB_BLE_V3
    if (sm_.IsNeedCollboration()) {
        sm_.appQueue_.pop_back();
        StartCollobration(BleScannerMessage(BleScannerStateMachine::START_EVENT, msg.appInfo, optimalParam));
        return true;
    }
#endif

    // If the optimal param is changed, stop the current scan and restart with the new param.
    // The app is removed and added later.
    sm_.isStartByStateMachine_ = false;
    sm_.appQueue_.pop_back();
    sm_.stopScanFunc_();
    BleScannerMessage innerStartMsg(BleScannerStateMachine::START_EVENT, msg.appInfo, optimalParam);
    sm_.PushPriorityEventFront(innerStartMsg);
    HILOGI("[STARTED] -> [INNER_STOPPING] after event [%{public}s], %{public}s",
        EventToStr(msg.what_, sm_.isInnerEvent_), msg.appInfo.ToString().c_str());
    Transition(BLE_SCAN_INNER_STOPPING);
    return true;
}

bool BleScannerStartedState::HandleStopEvent(const BleScannerMessage &msg)
{
    auto it = sm_.GetAppIter(msg.appInfo);
    if (it == sm_.appQueue_.end()) {
        HILOGE("Unknown msg in StateStarted: %{public}s", msg.appInfo.ToString().c_str());
        sm_.reportStoppedEventFunc_(msg.appInfo.pid, msg.appInfo.uid, msg.appInfo.scannerId,
            bluetooth::SCAN_NOT_STARTED);
        HandlePriorityEvent();
        return false;
    }
    // Is not the current application (max scan duty cycle)
    // Assuming app is erased for calculating the optimal parameter.
    sm_.reportStoppedEventFunc_(msg.appInfo.pid, msg.appInfo.uid, msg.appInfo.scannerId, bluetooth::SCAN_SUCCESS);
    sm_.appQueue_.erase(it);
    // Considering frequent stop and start case, next message should be involved in calculating optimal parameter
    BleScannerStateMachine::ScannerParameters optimalParam;
    GetOptimalParamInStopEvent(sm_.appQueue_, optimalParam);
    if (optimalParam.scanInterval == sm_.optimalParam_.scanInterval &&
        optimalParam.scanWindow == sm_.optimalParam_.scanWindow) {
        HILOGI("Remove app %{public}s", msg.appInfo.ToString().c_str());
#ifdef CONTEXTHUB_BLE_V3
        if (msg.appInfo.scannerId == sm_.ownedScannerId_) {
            HILOGI("notify lpdevice ap owned scannerId in lpdevice is removed");
            sm_.SendCollaborationResult(true);
        }
#endif
        HandlePriorityEvent();
        return true;
    }

#ifdef CONTEXTHUB_BLE_V3
    if (sm_.IsNeedCollboration()) {
        StartCollobration(msg);
        return true;
    }
#endif

    sm_.stopScanFunc_();
    sm_.PushPriorityStartEventFront();
    HILOGI("[STARTED] -> [STOPPING] after event [%{public}s], %{public}s",
        EventToStr(msg.what_, sm_.isInnerEvent_), msg.appInfo.ToString().c_str());
    Transition(BLE_SCAN_STOPPING);
    return true;
}

bool BleScannerStartedState::Dispatch(const utility::Message &utilMsg)
{
    CHECK_PTR_RETURN(sm_.currentApp_, "StateStarted", false);

    auto msg = static_cast<const BleScannerMessage &>(utilMsg);
    switch (msg.what_) {
        case BleScannerStateMachine::START_EVENT:
            if (!HandleStartEvent(msg)) {
                return false;
            }
            break;
        case BleScannerStateMachine::STOP_EVENT:
            if (!HandleStopEvent(msg)) {
                return false;
            }
            break;
        case BleScannerStateMachine::STOP_SUCCESS_EVENT:
            HILOGI("May trigger disable ble, reset state machine");
            sm_.Reset();
            break;
        default:
#ifndef CONTEXTHUB_BLE_V3
            HILOGE("Unsupported [%{public}s] in StateStarted.", EventToStr(msg.what_));
            return false;
#else
            return BleScannerState::Dispatch(utilMsg);
#endif
    };
    return true;
}

std::list<BleScannerStateMachine::ScannerAppInfo>::iterator BleScannerStartedState::GetOptimalParamInStopEvent(
    std::list<BleScannerStateMachine::ScannerAppInfo> &appQueue,
    BleScannerStateMachine::ScannerParameters &optimalParam)
{
    BleScannerMessage nextMessage(BleScannerStateMachine::INVALID_EVENT);
    if (!sm_.priorityEventQueue_.empty()) {
        nextMessage = sm_.priorityEventQueue_.front();
    }
    if (nextMessage.appInfo.scannerId != BleScannerStateMachine::LP_DEVICE_SCANNER_ID &&
        nextMessage.what_ == BleScannerStateMachine::START_EVENT) {
        appQueue.push_back(nextMessage.appInfo);
    }
    std::list<BleScannerStateMachine::ScannerAppInfo>::iterator it = sm_.GetOptimalParam(appQueue, optimalParam);
    if (nextMessage.appInfo.scannerId != BleScannerStateMachine::LP_DEVICE_SCANNER_ID &&
        nextMessage.what_ == BleScannerStateMachine::START_EVENT) {
        appQueue.pop_back();
    }
    return it;
}

void BleScannerStoppingState::Entry()
{
    HILOGD("STOPPING");
    sm_.timeoutTimer_->Start(sm_.timeoutMs_);
}

void BleScannerStoppingState::Exit()
{
    HILOGD("STOPPING");
    sm_.timeoutTimer_->Stop();
}

bool BleScannerStoppingState::Dispatch(const utility::Message &utilMsg)
{
    auto msg = static_cast<const BleScannerMessage &>(utilMsg);

    switch (msg.what_) {
        case BleScannerStateMachine::STOP_SUCCESS_EVENT:
            HILOGD("[STOPPING] -> [STOPPED] after event [STOP_SUCCESS_EVENT]");
            Transition(BLE_SCAN_STOPPED);
            break;
#ifndef CONTEXTHUB_BLE_V3
        case BleScannerStateMachine::START_EVENT:
        case BleScannerStateMachine::STOP_EVENT:
            HILOGI("pending [%{public}s] in [STOPPING], %{public}s", EventToStr(msg.what_),
                msg.appInfo.ToString().c_str());
            sm_.PushPriorityEventTail(msg);
            break;
        default:
            HILOGE("Unsupported [%{public}s] in StateStoping.", EventToStr(msg.what_));
            return false;
#else
        default:
            return BleScannerState::Dispatch(utilMsg);
#endif
    };
    return true;
}

void BleScannerInnerStoppingState::Entry()
{
    HILOGD("INNER_STOPPING");
    sm_.timeoutTimer_->Start(sm_.timeoutMs_);
}

void BleScannerInnerStoppingState::Exit()
{
    HILOGD("INNER_STOPPING");
    sm_.timeoutTimer_->Stop();
}

bool BleScannerInnerStoppingState::Dispatch(const utility::Message &utilMsg)
{
    auto msg = static_cast<const BleScannerMessage &>(utilMsg);

    switch (msg.what_) {
        case BleScannerStateMachine::STOP_SUCCESS_EVENT:
            HILOGI("[INNER_STOPPING] -> [STOPPED]");
            Transition(BLE_SCAN_STOPPED);
            break;
#ifndef CONTEXTHUB_BLE_V3
        case BleScannerStateMachine::START_EVENT:
        case BleScannerStateMachine::STOP_EVENT:
            HILOGI("pending [%{public}s] in [INNER_STOPPING], %{public}s", EventToStr(msg.what_),
                msg.appInfo.ToString().c_str());
            sm_.PushPriorityEventTail(msg);
            break;
        default:
            HILOGE("Unsupported [%{public}s] in StateInnerStoping.", EventToStr(msg.what_));
            return false;
#else
        default:
            return BleScannerState::Dispatch(utilMsg);
#endif
    };
    return true;
}

#ifdef CONTEXTHUB_BLE_V3
void BleScannerShCollaboration::Entry()
{
    HILOGD("SH_COLLABORATION");
    sm_.sendCollaborationFunc_(BLE_LPDEVICE_MSG_SCAN_COLLABORATION_QUERY, {});
    sm_.collaborationTimeoutTimer_->Start(sm_.collaborationTimeoutMs_);
    sm_.isCollaborating_ = true;
}

void BleScannerShCollaboration::Exit()
{
    HILOGD("SH_COLLABORATION");
    sm_.collaborationTimeoutTimer_->Stop();
}

void BleScannerShCollaboration::HandlePendingEvent(const BleScannerMessage &msg)
{
    if (sm_.collaborationFromStarted_) {
        if (msg.what_ == BleScannerStateMachine::START_EVENT) {
            sm_.isStartByStateMachine_ = false;
            sm_.stopScanFunc_();
            sm_.PushPriorityEventFront(msg);
            HILOGI("[SH_COLLABORATION] -> [INNER_STOPPING]");
            Transition(BLE_SCAN_INNER_STOPPING);
        } else {
            sm_.stopScanFunc_();
            sm_.PushPriorityStartEventFront();
            HILOGI("[SH_COLLABORATION] -> [STOPPING]");
            Transition(BLE_SCAN_STOPPING);
        }
    } else {
        if (msg.what_ == BleScannerStateMachine::START_EVENT) {
            HILOGI("[SH_COLLABORATION] -> [STARTING]");
            Transition(BLE_SCAN_STARTING);
            sm_.startScanFunc_(ConvertScanSetting(msg));
            sm_.reqApp_ = std::make_shared<BleScannerStateMachine::ScannerAppInfo>(msg.appInfo);
            sm_.reqParam_ = msg.scanParams;
        } else {
            HILOGI("[SH_COLLABORATION] -> [STOPPED]");
            Transition(BLE_SCAN_STOPPED);
        }
    }
}

void BleScannerShCollaboration::HandlerCollaborationNotifyStatusEvent(const BleScannerMessage &msg)
{
    sm_.HandlerCollaborationNotifyStatusEvent(msg.appInfo);
    if (sm_.priorityEventQueue_.empty()) {
        HILOGE("event queue is empty!!!"); // it will not happen
        if (sm_.currentApp_ == nullptr) {
            HILOGE("current app is nullptr");
            Transition(BLE_SCAN_STOPPED);
        } else {
            sm_.reqApp_ = sm_.currentApp_;
            Transition(BLE_SCAN_STARTED);
        }
    } else {
        BleScannerMessage pendingMsg = sm_.priorityEventQueue_.front();
        sm_.priorityEventQueue_.pop_front();
        HandlePendingEvent(pendingMsg);
    }
}

bool BleScannerShCollaboration::Dispatch(const utility::Message &utilMsg)
{
    auto msg = static_cast<const BleScannerMessage &>(utilMsg);

    switch (msg.what_) {
        case BleScannerStateMachine::SH_COLLABORATION_EVENT:
            if (msg.arg1_ == BleScannerStateMachine::SH_COLLABORATION_NOTIFY_LPDEVICE_STATUS) {
                HandlerCollaborationNotifyStatusEvent(msg);
            } else {
                HILOGI("[SH_COLLABORATION] recv collaboration event, subtype: %{public}d", msg.arg1_);
                sm_.HandlerCollaborationEvent(msg.arg1_);
                if (msg.arg1_ == BleScannerStateMachine::SH_COLLABORATION_REMOVE_LP_DEVICE_SCAN) {
                    HILOGW("[SH_COLLABORATION] collaboration is disabled in collaboration");
                    HandlerCollaborationNotifyStatusEvent(BleScannerMessage(
                        BleScannerStateMachine::SH_COLLABORATION_EVENT,
                        BleScannerStateMachine::SH_COLLABORATION_NOTIFY_LPDEVICE_STATUS));
                }
            }
            break;
        default:
            return BleScannerState::Dispatch(utilMsg);
    };

    return true;
}
#endif

bool operator < (const BleScannerStateMachine::ScannerParameters &lhs,
    const BleScannerStateMachine::ScannerParameters &rhs)
{
    if (lhs.scanInterval == 0 || rhs.scanInterval == 0) {
        HILOGE("scanInterval is 0");
        return false;  // will not come here, just protect divided by 0.
    }
    // If dutyCycle larger, or if dutyCycle equal and scanInterval lower
    double lDutyCycle = 1.0 * lhs.scanWindow / lhs.scanInterval;
    double rDutyCycle = 1.0 * rhs.scanWindow / rhs.scanInterval;
    double precision = 0.00001;
    // scan duty cycle is not same
    if (fabs(lDutyCycle - rDutyCycle) > precision) {
        return lDutyCycle < rDutyCycle;
    }
    if (lhs.scanInterval != rhs.scanInterval) {
        return lhs.scanInterval > rhs.scanInterval;
    }
    return false;
}

bool operator <= (const BleScannerStateMachine::ScannerParameters &lhs,
    const BleScannerStateMachine::ScannerParameters &rhs)
{
    bool l = lhs < rhs;
    bool r = rhs < lhs;
    // lhs == rhs
    if (!l && !r) {
        return true;
    }
    // lhs < rhs
    if (l && !r) {
        return true;
    }
    // lhs > rhs
    if (!l && r) {
        return false;
    }
    HILOGE("ScannerParameters undefined operator <= behavior");
    return true;
}

std::list<BleScannerStateMachine::ScannerAppInfo>::iterator BleScannerStateMachine::GetAppIter(
    const ScannerAppInfo &appInfo)
{
    auto checkFunc = [appInfo](const ScannerAppInfo &app) -> bool {
        return app.pid == appInfo.pid && app.uid == appInfo.uid && app.scannerId == appInfo.scannerId;
    };
    return std::find_if(appQueue_.begin(), appQueue_.end(), checkFunc);
}

std::list<BleScannerStateMachine::ScannerAppInfo>::iterator BleScannerStateMachine::GetMaxAppIter(void)
{
    auto maxIter = appQueue_.begin();
    for (auto it = appQueue_.begin(); it != appQueue_.end(); ++it) {
        if (*maxIter < *it) {
            maxIter = it;
        }
    }
    return maxIter;
}

std::list<BleScannerStateMachine::ScannerAppInfo>::iterator BleScannerStateMachine::GetOptimalParam(
    std::list<BleScannerStateMachine::ScannerAppInfo> &appQueue,
    BleScannerStateMachine::ScannerParameters &optimalParam)
{
    if (appQueue.empty()) {
        HILOGI("appQueue is empty.");
        optimalParam.scanInterval = 0;
        optimalParam.scanWindow = 0;
        return appQueue.end();
    }
    int minScanInterval = MAX_VALUE;
    double maxDutyCycle = MIN_VALUE;
    int minIntervalOfMaxCycle = MAX_VALUE; // interval of app which has max duty cycle

    auto maxIter = appQueue.begin();
    optimalParam = maxIter->params;

    for (auto it = appQueue.begin(); it != appQueue.end(); ++it) {
        int clientWindow = it->params.scanWindow;
        int clientInterval = it->params.scanInterval;
        if (clientInterval == 0) {
            HILOGE("clientInterval is zero.");
            continue;
        }
        double clientDutyCycle = 1.0 * clientWindow / clientInterval;
        double precision = 0.00001;
        double diff = fabs(clientDutyCycle - maxDutyCycle);
        if (diff > precision && clientDutyCycle > maxDutyCycle) {
            maxIter = it;
            maxDutyCycle = clientDutyCycle;
            minIntervalOfMaxCycle = clientInterval;
        } else if (diff <= precision) {
            // when two apps have the same duty cycle, choose the one with the smallest interval.
            if (clientInterval < minIntervalOfMaxCycle) {
                maxIter = it;
                maxDutyCycle = clientDutyCycle;
                minIntervalOfMaxCycle = clientInterval;
            }
        }
        if (clientInterval < minScanInterval) {
            minScanInterval = clientInterval;
        }
        // If the scan duty cycle is 100%, it will be selected as the optimal param,
        // even if there are other scan params with the same duty cycle but a shorter scan interval.
        if (fabs(maxDutyCycle - MAX_DUTY_CYCLE) <= precision) {
            minScanInterval = clientInterval;
            break;
        }
    }
    // Reverse calculation operation, get the optimal param.
    if (minScanInterval < maxIter->params.scanInterval) {
        // Get the minScanInterval, and reverse calculation.
        optimalParam.scanInterval = minScanInterval;
        optimalParam.scanWindow = static_cast<int>(minScanInterval * maxDutyCycle);
    } else {
        optimalParam.scanInterval = maxIter->params.scanInterval;
        optimalParam.scanWindow = maxIter->params.scanWindow;
    }
    return maxIter;
}

void BleScannerStateMachine::RemoveApp(const ScannerAppInfo &appInfo)
{
    auto it = GetAppIter(appInfo);
    if (it != appQueue_.end()) {
        appQueue_.erase(it);
    }
}

bool BleScannerStateMachine::IsExistApp(const ScannerAppInfo &appInfo)
{
    auto it = GetAppIter(appInfo);
    return it != appQueue_.end();
}

std::string BleScannerStateMachine::ScannerAppInfo::ToString(void) const
{
    std::string s = "pid:" + std::to_string(pid) + ",uid:" + std::to_string(uid) +
        ",scannerId:" + std::to_string(scannerId);
    // not use scanMode
    if (params.scanMode != -1) {
        s += ",scanMode:" + std::to_string(params.scanMode) +
             ",window:" + std::to_string(params.scanWindow) +
             ",interval:" + std::to_string(params.scanInterval);
    }
    return s;
}

}  // namespace OHOS
}  // namespace Bluetooth
