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
#ifndef BLE_SCANNER_STATE_MACHINE_H
#define BLE_SCANNER_STATE_MACHINE_H

#include <algorithm>
#include <atomic>
#include <list>
#include <functional>
#include <memory>
#include "ble_defs.h"
#include "ble_service_data.h"
#include "btcommon/message.h"
#include "btcommon/state_machine.h"
#include "btcommon/timer_manager.h"

namespace OHOS {
namespace Bluetooth {
const std::string BLE_SCAN_STOPPED = "BleScanStopped";
const std::string BLE_SCAN_STARTING = "BleScanStarting";
const std::string BLE_SCAN_STARTED = "BleScanStarted";
const std::string BLE_SCAN_STOPPING = "BleScanStopping";
const std::string BLE_SCAN_INNER_STOPPING = "BleScanInnerStopping";  // Used with update scan parameters
#ifdef CONTEXTHUB_BLE_V3
const std::string BLE_SCAN_SH_COLLABORATION = "BleScanShCollaboration";
#endif

struct BleScannerMessage;
class BleScannerStateMachine : public utility::StateMachine,
                               public std::enable_shared_from_this<BleScannerStateMachine> {
public:
    enum STATE_MACHINE_EVENT {
        INVALID_EVENT = 0,
        START_EVENT = 1,
        STOP_EVENT,
        START_SUCCESS_EVENT,
        START_FAILED_EVENT,
        STOP_SUCCESS_EVENT,
        STOP_FAILED_EVENT,

#ifdef CONTEXTHUB_BLE_V3
        SH_COLLABORATION_EVENT,
#endif
    };
    static constexpr size_t PRIORITY_Q_MAX_EVENT = 10;

    static constexpr int MAX_VALUE = 65535;
    static constexpr int MIN_VALUE = -1;

    static constexpr double MAX_DUTY_CYCLE = 1.0;

#ifdef CONTEXTHUB_BLE_V3
    enum SH_COLLABORATION_ARG {
        SH_COLLABORATION_REMOVE_LP_DEVICE_SCAN,
        SH_COLLABORATION_NOTIFY_LPDEVICE_STATUS,
        SH_COLLABORATION_ENABLED,
    };
#endif

    struct ScannerParameters {
        ScannerParameters() = default;
        ScannerParameters(long delay, int scanMode, bool legacy, int phy)
            : reportDelayMillis(delay), scanMode(scanMode), legacy(legacy), phy(phy)
        {
            OHOS::bluetooth::ConvertBleScanMode(scanMode, scanWindow, scanInterval);
        }

        explicit ScannerParameters(const bluetooth::BleScanSettingsImpl &settings)
        {
            reportDelayMillis = settings.GetReportDelayMillisValue();
            scanMode = settings.GetScanMode();
            legacy = settings.GetLegacy();
            phy = settings.GetPhy();
            callbackType = settings.GetCallbackType();
            matchTrackAdvType = settings.GetMatchTrackAdvType();
            matchMode = settings.GetMatchMode();
            OHOS::bluetooth::ConvertBleScanMode(scanMode, scanWindow, scanInterval);
        }

        long reportDelayMillis = 0;
        int scanMode = bluetooth::SCAN_MODE_LOW_POWER;
        bool legacy = true;
        int phy = 0;
        uint16_t scanInterval = 0;
        uint16_t scanWindow = 0;
        uint8_t callbackType = bluetooth::BLE_SCAN_CALLBACK_TYPE_ALL_MATCH;
        uint8_t matchTrackAdvType = bluetooth::MAX_MATCH_TRACK_ADV;
        uint8_t matchMode = bluetooth::MATCH_MODE_AGGRESSIVE;
    };

    struct ScannerAppInfo {
        ScannerAppInfo() = default;
        ScannerAppInfo(int pid, int uid, int scannerId) : pid(pid), uid(uid), scannerId(scannerId) {}
        ScannerAppInfo(int pid, int uid, int scannerId, const ScannerParameters &params)
            : pid(pid), uid(uid), scannerId(scannerId), params(params) {}
        std::string ToString(void) const;

        int pid = 0;
        int uid = 0;
        int scannerId = 0;
        ScannerParameters params;
    };

    BleScannerStateMachine() = default;
    ~BleScannerStateMachine() = default;

    using PostTaskFunc = std::function<void(std::function<void(void)>)>;
    using StartScanFunc = std::function<void (const bluetooth::BleScanSettingsImpl &setting)>;
    using StopScanFunc = std::function<void (void)>;
    using ReportStartedEventFunc = std::function<void(int, int, int, int)>;
    using ReportStoppedEventFunc = std::function<void(int, int, int, int)>;
#ifdef CONTEXTHUB_BLE_V3
    using SendCollaborationFunc = std::function<void(int type, const std::vector<ScannerAppInfo> &appInfos)>;
    using ScanCollaborationEnabledFunc = std::function<bool(void)>;
    struct CollabrationFunc {
        CollabrationFunc(SendCollaborationFunc sendFunc, ScanCollaborationEnabledFunc checkEnabled)
            : sendCollaborationFunc(sendFunc), scanCollaborationEnabledFunc(checkEnabled) {}
        SendCollaborationFunc sendCollaborationFunc;
        ScanCollaborationEnabledFunc scanCollaborationEnabledFunc;
    };
    void Init(PostTaskFunc postTaskFunc, StartScanFunc startScanFunc, StopScanFunc stopScanFunc,
        ReportStartedEventFunc reportStartedEventFunc, ReportStoppedEventFunc reportStoppedEventFunc,
        const CollabrationFunc &collaboratioFunc);
#else
    void Init(PostTaskFunc postTaskFunc, StartScanFunc startScanFunc, StopScanFunc stopScanFunc,
        ReportStartedEventFunc reportStartedEventFunc, ReportStoppedEventFunc reportStoppedEventFunc);
#endif
    void Reset(void);
    bool SendMessage(const BleScannerMessage &msg);

    // Used inner
    std::list<ScannerAppInfo>::iterator GetAppIter(const ScannerAppInfo &appInfo);
    std::list<ScannerAppInfo>::iterator GetMaxAppIter(void);
    static std::list<ScannerAppInfo>::iterator GetOptimalParam(std::list<ScannerAppInfo> &appQueue,
        ScannerParameters &optimalParam);
    void RemoveApp(const ScannerAppInfo &appInfo);
    bool IsExistApp(const ScannerAppInfo &appInfo);
    // Used for test
    void SetTimeoutMs(int ms)
    {
        timeoutMs_ = ms;
    }

private:
    void PushPriorityEvent(const BleScannerMessage &event, bool fronted);
    void PushPriorityStartEventFront(void);
    void PushPriorityEventFront(const BleScannerMessage &event);
    void PushPriorityEventTail(const BleScannerMessage &event);

    std::atomic_bool isStarted_ {false};  // Used for init check
    std::shared_ptr<ScannerAppInfo> reqApp_ {nullptr};
    ScannerParameters reqParam_ {};
    ScannerParameters optimalParam_ {};
    std::shared_ptr<ScannerAppInfo> currentApp_ {nullptr};
    std::list<ScannerAppInfo> appQueue_ {};
    std::list<BleScannerMessage> priorityEventQueue_ {};

    int timeoutMs_ = 3000;  // 3000ms
    std::shared_ptr<utility::Timer> timeoutTimer_ {nullptr};
    PostTaskFunc postTaskFuc_ {};
    StartScanFunc startScanFunc_ {};
    StopScanFunc stopScanFunc_ {};
    ReportStartedEventFunc reportStartedEventFunc_ {};
    ReportStoppedEventFunc reportStoppedEventFunc_ {};

    bool isInnerEvent_ {false};  // used for log

    static constexpr int LP_DEVICE_SCANNER_ID = 255;

#ifdef CONTEXTHUB_BLE_V3
    void InitCollaborationTimer();
    void SendCollaborationResult(bool started);
    void RemoveLpDeviceScanner();
    void HandlerCollaborationNotifyStatusEvent(const ScannerAppInfo &app);
    void HandlerCollaborationEvent(int subtype);

    bool IsNeedCollboration()
    {
        return scanCollaborationEnabledFunc_() && !isCollaborating_;
    }

    int collaborationTimeoutMs_ = 200;
    bool collaborationFromStarted_ = false;
    bool isCollaborating_ = false;
    int ownedScannerId_ = bluetooth::BLE_SCAN_INVALID_ID;
    SendCollaborationFunc sendCollaborationFunc_;
    ScanCollaborationEnabledFunc scanCollaborationEnabledFunc_;
    std::shared_ptr<utility::Timer> collaborationTimeoutTimer_ {nullptr};

    friend class BleScannerShCollaboration;
#endif

    // used for check StartScan by the StateMachine.
    // scene1: Failed to StartScan
    // scene2: StopScan
    std::atomic_bool isStartByStateMachine_ {false}; //a scan started by the statemachine, not by the application.
    friend class BleScannerStoppedState;
    friend class BleScannerStartingState;
    friend class BleScannerStartedState;
    friend class BleScannerStoppingState;
    friend class BleScannerInnerStoppingState;
    friend class BleScannerState;
};

struct BleScannerMessage : public utility::Message {
    BleScannerMessage() = delete;
    // what is STATE_MACHINE_EVENT
    explicit BleScannerMessage(int what, BleScannerStateMachine::ScannerAppInfo appInfo,
        BleScannerStateMachine::ScannerParameters scanParams)
        : utility::Message(what), appInfo(appInfo), scanParams(scanParams) {}
    
    explicit BleScannerMessage(int what, BleScannerStateMachine::ScannerAppInfo appInfo)
        : utility::Message(what), appInfo(appInfo)
    {
        scanParams = appInfo.params;
    }
    explicit BleScannerMessage(int what) : utility::Message(what) {}
#ifdef CONTEXTHUB_BLE_V3
    explicit BleScannerMessage(int what, int arg1) : utility::Message(what, arg1) {}
    explicit BleScannerMessage(int what, int arg1, BleScannerStateMachine::ScannerAppInfo appInfo)
        : utility::Message(what, arg1), appInfo(appInfo) {}
#endif
    ~BleScannerMessage() override = default;

    // used for START_EVENT, STOP_EVENT
    BleScannerStateMachine::ScannerAppInfo appInfo;
    BleScannerStateMachine::ScannerParameters scanParams;
};

class BleScannerState : public utility::StateMachine::State {
public:
    BleScannerState(const std::string &name, BleScannerStateMachine &stateMachine)
        : State(name, stateMachine), sm_(stateMachine) {}
    virtual ~BleScannerState() = default;

#ifdef CONTEXTHUB_BLE_V3
    bool Dispatch(const utility::Message &utilMsg) override;
    void StartCollobration(const BleScannerMessage &msg);
#endif

protected:
    BleScannerStateMachine &sm_;
};

class BleScannerStoppedState : public BleScannerState {
public:
    explicit BleScannerStoppedState(BleScannerStateMachine &stateMachine)
        : BleScannerState(BLE_SCAN_STOPPED, stateMachine) {}
    ~BleScannerStoppedState() override = default;

    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &utilMsg) override;
};

class BleScannerStartingState : public BleScannerState {
public:
    explicit BleScannerStartingState(BleScannerStateMachine &stateMachine)
        : BleScannerState(BLE_SCAN_STARTING, stateMachine) {}
    ~BleScannerStartingState() override = default;

    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &utilMsg) override;
};

class BleScannerStartedState : public BleScannerState {
public:
    explicit BleScannerStartedState(BleScannerStateMachine &stateMachine)
        : BleScannerState(BLE_SCAN_STARTED, stateMachine) {}
    ~BleScannerStartedState() override = default;

    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &utilMsg) override;
    bool HandleStartEvent(const BleScannerMessage &msg);
    bool HandleStopEvent(const BleScannerMessage &msg);
    void HandlePriorityEvent(void);
    std::list<BleScannerStateMachine::ScannerAppInfo>::iterator GetOptimalParamInStopEvent(
        std::list<BleScannerStateMachine::ScannerAppInfo> &appQueue,
        BleScannerStateMachine::ScannerParameters &optimalParam);
};

class BleScannerStoppingState : public BleScannerState {
public:
    explicit BleScannerStoppingState(BleScannerStateMachine &stateMachine)
        : BleScannerState(BLE_SCAN_STOPPING, stateMachine) {}
    ~BleScannerStoppingState() override = default;

    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &utilMsg) override;
};

class BleScannerInnerStoppingState : public BleScannerState {
public:
    explicit BleScannerInnerStoppingState(BleScannerStateMachine &stateMachine)
        : BleScannerState(BLE_SCAN_INNER_STOPPING, stateMachine) {}
    ~BleScannerInnerStoppingState() override = default;

    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &utilMsg) override;
};

#ifdef CONTEXTHUB_BLE_V3
class BleScannerShCollaboration : public BleScannerState {
public:
    explicit BleScannerShCollaboration(BleScannerStateMachine &stateMachine)
        : BleScannerState(BLE_SCAN_SH_COLLABORATION, stateMachine) {}
    ~BleScannerShCollaboration() override = default;

    void HandlePendingEvent(const BleScannerMessage &msg);
    void HandlerCollaborationNotifyStatusEvent(const BleScannerMessage &msg);

    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &utilMsg) override;
};
#endif

}  // namespace OHOS
}  // namespace Bluetooth
#endif  // BLE_SCANNER_STATE_MACHINE_H
