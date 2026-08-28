/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_sh_collaboration"
#endif

#include "sensorhub_collaboration_service.h"

#include <algorithm>
#include <functional>
#include <list>
#include <map>
#include <mutex>
#include <set>
#include <chrono>
#include <sstream>

#include "bluetooth_hw_interface.h"
#include "bluetooth_common_event_subscriber.h"
#include "base_observer_list.h"
#include "btcommon/timer_manager.h"
#include "bt_chr_ue_manager.h"
#include "log_utils.h"

namespace OHOS {
namespace bluetooth {

namespace {
static const uint32_t LPDEVICE_CAPABILITY_SCAN_COLLABORATION_ENABLED = 0x00000001U;
static const uint32_t LPDEVICE_CAPABILITY_GATT_COLLABORATION_ENABLED = 0x00000002U;

void SetLpDeviceBleBufferSizeAndCount()
{
    const BthwifInterface *hwif = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    if (hwif == nullptr) {
        HILOGE("bthwif is null");
        return;
    }

    HILOGD("start to set buffer size and count of lp device");
    HwConnAttr attr = { .type = HW_CONN_ATTR_TYPE_SET_LP_DEVICE_BUFFER_SIZE_AND_COUNT };
    hwif->hwConnAttrSet(BT_TRANSPORT_LE, STACK::RawAddress::kEmpty, &attr);
}

bool SendParamsToLpDevice(const std::vector<uint8_t> &dataValue, int32_t type)
{
    const BthwifInterface *hwif = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    if (hwif == nullptr) {
        HILOGE("bthwInterface_ is null.");
        return false;
    }
    HILOGI_TIME_LIMIT(std::to_string(type), "SendParamsToLpDevice: type:%{public}d, dataLen:%{public}zu",
        type, dataValue.size());
    hwif->sendParamsToSensorhub(reinterpret_cast<const char*>(dataValue.data()), type,
        dataValue.size());
    return true;
}
}

#define UCHAR4_TO_UINT32_LE(buf) (static_cast<uint32_t>((buf)[0]) | (static_cast<uint32_t>((buf)[1]) << 8) | \
    (static_cast<uint32_t>((buf)[2]) << 16) | (static_cast<uint32_t>((buf)[3]) << 24))
#define UCHAR2_TO_UINT16_LE(buf) (static_cast<uint16_t>((buf)[0]) | (static_cast<uint16_t>((buf)[1]) << 8))
#define UINT16_TO_VECTOR_LE(vec, data) do { \
    (vec).emplace_back(static_cast<uint8_t>(data)); \
    (vec).emplace_back(static_cast<uint8_t>((data) >> 8)); \
} while (0)

struct SensorhubCollaborationService::Stats {
    static constexpr int64_t UPLOAD_INTERVAL_MS = 24 * 60 * 60 * 1000; /* 24h */
    void CopyUe(SensorhubCollaborationUe &shColl);
    void CheckDfxUpload();
    void RecordSensorhubReset();
    void RecordScanCollaborationStart();
    void RecordScanCollaborationShResult(uint16_t interval, uint16_t window);
    void RecordConnCollaborationStart();
    void RecordConnCollaborationShResult(bool timeout);

    struct CollaborationStats {
        void Reset()
        {
            totalCount_ = 0;
            lastTs_ = {};
            costMs_ = 0;
            maxCostMs_ = 0;
            failedCnt_ = 0;
        }

        void RecordStart()
        {
            if (lastTs_ != std::chrono::system_clock::time_point{}) {
                failedCnt_++;
                HILOGE("record collaboration failed, failed cnt: %{public}u", failedCnt_);
            }
            lastTs_ = std::chrono::system_clock::now();
            totalCount_++;
        }

        void RecordTimeout()
        {
            lastTs_ = std::chrono::system_clock::time_point{};
            failedCnt_++;
            HILOGE("record collaboration timeout, failed cnt: %{public}u", failedCnt_);
        }

        void RecordComplete()
        {
            if (lastTs_ == std::chrono::system_clock::time_point{}) {
                HILOGE("ignore sh msg after timeout");
                return;
            }

            auto duration = std::chrono::system_clock::now() - lastTs_;
            int64_t costMs = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
            if (static_cast<uint32_t>(costMs) > maxCostMs_) {
                maxCostMs_ = static_cast<uint32_t>(costMs);
            }
            costMs_ += static_cast<uint32_t>(costMs);
            lastTs_ = std::chrono::system_clock::time_point{};
        }

        uint32_t totalCount_ = 0;
        std::chrono::time_point<std::chrono::system_clock> lastTs_ {};
        uint64_t costMs_ = 0;
        uint32_t maxCostMs_ = 0;
        uint32_t failedCnt_ = 0;
    };

    struct ScanCollaborationStats : public CollaborationStats {
        void Reset()
        {
            CollaborationStats::Reset();
            shScanStartedCnt_ = 0;
            shScanInfo_.clear();
        }
        std::vector<SensorhubScannerInfo> shScanInfo_;
        uint32_t shScanStartedCnt_ = 0;
    };

    std::mutex lock_;
    ScanCollaborationStats scanStats = {};
    CollaborationStats connStats = {};
    std::chrono::time_point<std::chrono::system_clock> startTs {};
    uint32_t resetCnt = 0;
};

void SensorhubCollaborationService::Stats::CopyUe(SensorhubCollaborationUe &shColl)
{
    shColl.sensorhubResetCnt = resetCnt;

    shColl.scanCollCnt = scanStats.totalCount_;
    shColl.scanCollFailedCnt = scanStats.failedCnt_;
    shColl.scanCollStartCnt = scanStats.shScanStartedCnt_;

    if (scanStats.totalCount_ > 0) {
        shColl.scanCollAvgMs = static_cast<uint32_t>(scanStats.costMs_ / scanStats.totalCount_);
    } else {
        shColl.scanCollAvgMs = 0;
    }

    shColl.scanCollMaxMs = scanStats.maxCostMs_;

    std::ostringstream oss;
    for (const auto& info : scanStats.shScanInfo_) {
        oss << info.scanInterval << "/" << info.scanWindow << ";";
    }
    shColl.shScanParam = oss.str();

    shColl.connCollCnt = connStats.totalCount_;
    shColl.connCollFailedCnt = connStats.failedCnt_;

    if (connStats.totalCount_ > 0) {
        shColl.connCollAvgMs = static_cast<uint32_t>(connStats.costMs_ / connStats.totalCount_);
    } else {
        shColl.connCollAvgMs = 0;
    }

    shColl.connCollMaxMs = connStats.maxCostMs_;
}

void SensorhubCollaborationService::Stats::CheckDfxUpload()
{
    SensorhubCollaborationUe ue = {};
    {
        std::lock_guard<std::mutex> lock(lock_);
        if (startTs == std::chrono::system_clock::time_point{}) {
            startTs = std::chrono::system_clock::now();
            return;
        }

        auto duration = std::chrono::system_clock::now() - startTs;
        if (std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() < UPLOAD_INTERVAL_MS) {
            return;
        }

        HILOGI("upload dfx, sensorhub reset count: %{public}u,"
            " scan collaboration: {cnt: %{public}u, avg cost ms: %{public}lu, max cost ms: %{public}u"
            " failed: %{public}u, shscan start cnt: %{public}u, shscan param cnt: %{public}zu},"
            " conn collaboration: {cnt: %{public}u, avg cost ms: %{public}lu,, max cost ms: %{public}u,"
            " failed: %{public}u",
            resetCnt, scanStats.totalCount_,
            scanStats.totalCount_ == 0 ? 0 : scanStats.costMs_ / scanStats.totalCount_,
            scanStats.maxCostMs_,
            scanStats.failedCnt_, scanStats.shScanStartedCnt_,
            scanStats.shScanInfo_.size(),
            connStats.totalCount_,
            connStats.totalCount_ == 0 ? 0 : connStats.costMs_ / connStats.totalCount_,
            connStats.maxCostMs_,
            connStats.failedCnt_);

        CopyUe(ue);

        startTs = {};
        connStats.Reset();
        scanStats.Reset();
        resetCnt = 0;
    }

    BtChrUeManager::GetInstance()->WriteSensorhubCollaborationUe(CHR_UE_SENSORHUB_COLLABORATION_EVENT, ue);
}

void SensorhubCollaborationService::Stats::RecordSensorhubReset()
{
    std::lock_guard<std::mutex> lock(lock_);
    resetCnt++;
}

void SensorhubCollaborationService::Stats::RecordScanCollaborationStart()
{
    std::lock_guard<std::mutex> lock(lock_);
    scanStats.RecordStart();
}

void SensorhubCollaborationService::Stats::RecordScanCollaborationShResult(uint16_t interval, uint16_t window)
{
    std::lock_guard<std::mutex> lock(lock_);
    scanStats.RecordComplete();
    if (interval != 0 && window != 0) {
        scanStats.shScanStartedCnt_++;
        for (SensorhubScannerInfo &info : scanStats.shScanInfo_) {
            if (info.scanInterval == interval && info.scanWindow == window) {
                return;
            }
        }

        scanStats.shScanInfo_.emplace_back(interval, window);
    }
}

void SensorhubCollaborationService::Stats::RecordConnCollaborationStart()
{
    std::lock_guard<std::mutex> lock(lock_);
    connStats.RecordStart();
}

void SensorhubCollaborationService::Stats::RecordConnCollaborationShResult(bool timeout)
{
    std::lock_guard<std::mutex> lock(lock_);
    if (timeout) {
        connStats.RecordTimeout();
    } else {
        connStats.RecordComplete();
    }
}

struct SensorhubCollaborationService::impl : public BluetoothHwInterface::SensorhubObserver {
    explicit impl();
    ~impl() override;

    void QueryCapability();

    void OnScreenStateChanged(ScreenState state);

    bool IsScanCollborationEnabled();
    void SetScanReportChannelToLpDevice(int32_t scannerId, bool enable);

    bool StartConnCollaboration(const std::string &connName);
    void AllConnectionComplete(const std::string &connName);
    void OnConnCollaborationCompleted(bool timeout);

    void ProcessScanCollaborationMsg(std::vector<uint8_t> &payload);
    void ProcessGattCollaborationMsg(std::vector<uint8_t> &payload);
    void ProcessCollaborationCapability(std::vector<uint8_t> &payload);

    void SensorhubResetCallback(uint32_t state) override;
    void SensorhubCollaborationCallback(const std::vector<uint8_t> &notifyValue) override;

    std::shared_ptr<BluetoothCommonEventSubscriberEx> screenOnSubscriber_ { nullptr };
    std::shared_ptr<BluetoothCommonEventSubscriberEx> screenOffSubscriber_ { nullptr };

    BaseObserverList<ISensorhubCollaborationObserver> shCollaborationStateObservers_ = {};

    std::mutex lock_;
    std::set<int> scannerIdSet_;

    bool capabilityIsQueryed_ = false;
    bool capabilityIsRecved_ = false;
    uint32_t capability_ = 0;

    SensorhubConnState connCollaborationState_ = SensorhubConnState::NONE;
    bool isScreenOn = true;
    const int COLLABORATION_TIMEOUT_MS = 200;
    std::shared_ptr<utility::Timer> collaborationTimer {nullptr};
    std::set<std::string> connTask_;

    SensorhubCollaborationService::Stats stats = {};

    BT_DISALLOW_COPY_AND_ASSIGN(impl);
};

SensorhubCollaborationService::impl::impl()
{
}

SensorhubCollaborationService::impl::~impl()
{
    if (collaborationTimer) {
        collaborationTimer->Stop();
        collaborationTimer = nullptr;
    }

    if (screenOnSubscriber_) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(screenOnSubscriber_);
        screenOnSubscriber_ = nullptr;
    }

    if (screenOffSubscriber_) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(screenOffSubscriber_);
        screenOffSubscriber_ = nullptr;
    }
}

void SensorhubCollaborationService::impl::SensorhubResetCallback(uint32_t state)
{
    HILOGI("sensorhub is reset, notify the scan in sensorhub is removed");
    shCollaborationStateObservers_.ForEach(
        [](ISensorhubCollaborationObserver &observer) {
            observer.OnSensorhubStateChanged(SensorhubState::SENSORHUB_SCAN_REVMOED);
        });

    bool notifyConn = false;
    {
        std::lock_guard<std::mutex> lock(lock_);
        scannerIdSet_.clear();
        if (connCollaborationState_ == SensorhubConnState::WAITING_FOR_RESULT) {
            HILOGI("sensorhub reset in waiting for conn collaboration result");
            collaborationTimer->Stop();
            notifyConn = true;
        }
        connCollaborationState_ = SensorhubConnState::NONE;
    }

    if (notifyConn) {
        shCollaborationStateObservers_.ForEach(
            [](ISensorhubCollaborationObserver &observer) { observer.OnSensorhubConnCollaborationCompleted(); });
    }
}

void SensorhubCollaborationService::impl::ProcessScanCollaborationMsg(std::vector<uint8_t> &payload)
{
    if (payload.size() < sizeof(uint16_t) + sizeof(uint16_t)) { // scanInterval(2bytes) + scanWindow(2bytes)
        HILOGE("invalid len of scan collbaoration");
        return;
    }

    uint16_t scanInterval = UCHAR2_TO_UINT16_LE(payload.data());
    uint16_t scanWindow = UCHAR2_TO_UINT16_LE(payload.data() + sizeof(scanInterval));
    HILOGI("scanInterval:%{public}hu, scanWindow:%{public}hu", scanInterval, scanWindow);
    shCollaborationStateObservers_.ForEach(
        [scanInterval, scanWindow](ISensorhubCollaborationObserver &observer) {
            observer.OnSensorhubScanCollaborationMsgRecved(SensorhubScannerInfo(scanInterval, scanWindow));
        });

    stats.RecordScanCollaborationShResult(scanInterval, scanWindow);
}

void SensorhubCollaborationService::impl::ProcessGattCollaborationMsg(std::vector<uint8_t> &payload)
{
    {
        std::lock_guard<std::mutex> lock(lock_);
        if (collaborationTimer) {
            collaborationTimer->Stop();
        }
    }
    OnConnCollaborationCompleted(false);
}

void SensorhubCollaborationService::impl::ProcessCollaborationCapability(std::vector<uint8_t> &payload)
{
    if (payload.size() < sizeof(uint32_t)) { // capability(4bytes)
        HILOGE("invalid capability msg length %{public}zu", payload.size());
        return;
    }

    uint32_t cap = UCHAR4_TO_UINT32_LE(payload.data());
    if (cap & LPDEVICE_CAPABILITY_GATT_COLLABORATION_ENABLED) {
        SetLpDeviceBleBufferSizeAndCount();
    }

    {
        std::lock_guard<std::mutex> lock(lock_);
        if (capabilityIsRecved_) {
            HILOGI("lpdevice capability already recved");
            return;
        }

        HILOGI("lpdevice capability: 0x%{public}08x", cap);
        capabilityIsRecved_ = true;
        capability_ = cap;
    }
}

void SensorhubCollaborationService::impl::SensorhubCollaborationCallback(const std::vector<uint8_t> &notifyValue)
{
    if (notifyValue.size() == 0) {
        return;
    }

    uint8_t msgType = notifyValue[0];
    HILOGI("msgType:%{public}hhu, dataLen:%{public}lu", msgType, notifyValue.size());

    std::vector<uint8_t> payload(notifyValue.begin() + 1, notifyValue.end());
    if (msgType == BLE_LPDEVICE_MSG_SCAN_COLLABORATION_NOTIFY_STATUS) {
        ProcessScanCollaborationMsg(payload);
    } else if (msgType == BLE_LPDEVICE_MSG_GATT_CONN_COLLABORATION_NOTIFY_STATUS) {
        ProcessGattCollaborationMsg(payload);
    } else if (msgType == BLE_LPDEVICE_MSG_CAPABILITY) {
        ProcessCollaborationCapability(payload);
    } else {
        HILOGE("unsupported msg");
    }
}

void SensorhubCollaborationService::impl::QueryCapability()
{
    {
        std::lock_guard<std::mutex> lock(lock_);
        if (capabilityIsQueryed_ || capabilityIsRecved_) {
            return;
        }
        capabilityIsQueryed_ = true;
    }

    HILOGI("start query capablity");
    std::vector<uint8_t> dataValue = {0};
    if (!SendParamsToLpDevice(dataValue, BLE_LPDEVICE_MSG_CAPABILITY_QUERY)) {
        std::lock_guard<std::mutex> lock(lock_);
        capabilityIsQueryed_ = false;
    }
}

void SensorhubCollaborationService::impl::OnScreenStateChanged(ScreenState state)
{
    std::lock_guard<std::mutex> lock(lock_);
    isScreenOn = (state == ScreenState::SCREEN_ON);
}

bool SensorhubCollaborationService::impl::IsScanCollborationEnabled()
{
    std::lock_guard<std::mutex> lock(lock_);
    return !isScreenOn && !scannerIdSet_.empty() && (capability_ | LPDEVICE_CAPABILITY_SCAN_COLLABORATION_ENABLED);
}

void SensorhubCollaborationService::impl::SetScanReportChannelToLpDevice(int32_t scannerId, bool enable)
{
    std::lock_guard<std::mutex> lock(lock_);
    if (enable) {
        scannerIdSet_.insert(scannerId);
    } else {
        scannerIdSet_.erase(scannerId);
    }
    HILOGI("scannerid=%{public}d, enabled=%{public}d, lp device scanner cnt=%{public}zu",
        scannerId, enable, scannerIdSet_.size());
}

bool SensorhubCollaborationService::impl::StartConnCollaboration(const std::string &connName)
{
    std::lock_guard<std::mutex> lock(lock_);
    if ((capability_ & LPDEVICE_CAPABILITY_GATT_COLLABORATION_ENABLED) == 0) {
        HILOGD("gatt connection in sensorhub not supported");
        return false;
    }

    HILOGI("state: %{public}d, sh scanner count: %{public}zu, screen state: %{public}d, conn task: %{public}s",
        connCollaborationState_, scannerIdSet_.size(), isScreenOn, connName.c_str());

    stats.CheckDfxUpload();
    connTask_.insert(connName);

    // notify ap connecting to sensorhub, no matter scann in sensorhub is started or screen on
    std::vector<uint8_t> dataValue;
    dataValue.emplace_back(1);
    SendParamsToLpDevice(dataValue, BLE_LPDEVICE_MSG_GATT_CONN_COLLABORATION_QUERY);

    if (connCollaborationState_ != SensorhubConnState::NONE) {
        bool waiting = (connCollaborationState_ == SensorhubConnState::WAITING_FOR_RESULT);
        HILOGI("already in conn collaboration, waiting = %{public}d", waiting);
        return waiting;
    }

    stats.RecordConnCollaborationStart();

    if (!collaborationTimer) {
        collaborationTimer = std::make_shared<utility::Timer>([this]() {
            OnConnCollaborationCompleted(true);
        });
        if (!collaborationTimer) {
            HILOGE("alloc timer failed");
            connCollaborationState_ = SensorhubConnState::COLLABORATED;
            return false;
        }
    }

    collaborationTimer->Start(COLLABORATION_TIMEOUT_MS);
    connCollaborationState_ = SensorhubConnState::WAITING_FOR_RESULT;
    return true;
}

void SensorhubCollaborationService::impl::AllConnectionComplete(const std::string &connName)
{
    std::lock_guard<std::mutex> lock(lock_);
    if ((capability_ & LPDEVICE_CAPABILITY_GATT_COLLABORATION_ENABLED) == 0) {
        HILOGD("gatt connection in sensorhub not supported");
        return;
    }

    HILOGI("conn task %{public}s completed", connName.c_str());
    connTask_.erase(connName);
    if (connTask_.empty()) {
        HILOGI("all conn task completed, now notify sensorhub");
        std::vector<uint8_t> dataValue;
        dataValue.emplace_back(1);
        SendParamsToLpDevice(dataValue, BLE_LPDEVICE_MSG_GATT_CONN_COLLABORATION_RESULT);
        connCollaborationState_ = SensorhubConnState::NONE;
    }
}

void SensorhubCollaborationService::impl::OnConnCollaborationCompleted(bool timeout)
{
    stats.RecordConnCollaborationShResult(timeout);
    {
        std::lock_guard<std::mutex> lock(lock_);
        HILOGI("sensorhub collaboration completed, state: %{public}d, timeout: %{public}d",
            connCollaborationState_, timeout);
        if (timeout) {
            connCollaborationState_ = SensorhubConnState::NONE;
        } else {
            connCollaborationState_ = SensorhubConnState::COLLABORATED;
        }
    }

    shCollaborationStateObservers_.ForEach(
        [](ISensorhubCollaborationObserver &observer) { observer.OnSensorhubConnCollaborationCompleted(); });
}

SensorhubCollaborationService::SensorhubCollaborationService()
{
    pimpl = std::make_unique<impl>();
}

SensorhubCollaborationService::~SensorhubCollaborationService()
{
    pimpl = nullptr;
}

bool SensorhubCollaborationService::IsScanCollborationEnabled()
{
    return pimpl->IsScanCollborationEnabled();
}

void SensorhubCollaborationService::SendScanCollaborationMsg(uint8_t type,
    const std::vector<SensorhubScannerInfo> &appInfos) const
{
    std::vector<uint8_t> dataValue;
    if (type == BLE_LPDEVICE_MSG_SCAN_COLLABORATION_QUERY) {
        pimpl->stats.CheckDfxUpload();
        pimpl->stats.RecordScanCollaborationStart();

        dataValue.emplace_back(0);
        HILOGI("send scan collaboration query, count: %{public}u", pimpl->stats.scanStats.totalCount_);
        SendParamsToLpDevice(dataValue, type);
        return;
    }

    if (!appInfos.empty()) {
        dataValue.emplace_back(1);
        for (uint32_t i = 0; i < appInfos.size(); i++) {
            const SensorhubScannerInfo &appInfo = appInfos[i];
            // little endian byte order
            UINT16_TO_VECTOR_LE(dataValue, static_cast<uint32_t>(appInfo.scannerId));
            UINT16_TO_VECTOR_LE(dataValue, appInfo.scanInterval);
            UINT16_TO_VECTOR_LE(dataValue, appInfo.scanWindow);
        }
        HILOGI("send scan started result, scannerId: %{public}d,"
            " scanInterval: %{public}d, scanWindow: %{public}d, count: %{public}zu",
            appInfos[0].scannerId, appInfos[0].scanInterval, appInfos[0].scanWindow, appInfos.size());
    } else {
        dataValue.emplace_back(0);
        HILOGI("send scan stopped result");
    }
    SendParamsToLpDevice(dataValue, type);
}

static std::shared_ptr<BluetoothCommonEventSubscriberEx> RegisterBluetoothCommonEvent(
    const std::string &event, BluetoothCommonEventSubscriberEx::CommonEventCallback callback)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(event);
    EventFwk::CommonEventSubscribeInfo info(matchingSkills);
    std::shared_ptr<BluetoothCommonEventSubscriberEx> subscriber = std::make_shared<BluetoothCommonEventSubscriberEx>(
        info, event, callback);
    auto subRet = EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber);
    HILOGI("Subscriber %{public}s end, SubscribeResult = %{public}d", event.c_str(), subRet);
    return subscriber;
}

void SensorhubCollaborationService::RegisterScreenStateCallback() const
{
    if (pimpl->screenOnSubscriber_) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(pimpl->screenOnSubscriber_);
    }
    pimpl->screenOnSubscriber_ = RegisterBluetoothCommonEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON,
        [this](const OHOS::EventFwk::CommonEventData &data) {
            HILOGI("recv screen on event");
            OnScreenStateChanged(ScreenState::SCREEN_ON);
        });

    if (pimpl->screenOffSubscriber_) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(pimpl->screenOffSubscriber_);
    }
    pimpl->screenOffSubscriber_ = RegisterBluetoothCommonEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF,
        [this](const OHOS::EventFwk::CommonEventData &data) {
            HILOGI("recv screen off event");
            OnScreenStateChanged(ScreenState::SCREEN_OFF);
        });
}

void SensorhubCollaborationService::OnScreenStateChanged(ScreenState state) const
{
    pimpl->OnScreenStateChanged(state);

    if (state == ScreenState::SCREEN_ON) {
        pimpl->shCollaborationStateObservers_.ForEach(
            [](ISensorhubCollaborationObserver &observer) {
                observer.OnSensorhubStateChanged(SensorhubState::SENSORHUB_SCAN_REVMOED);
            });
    }
}

void SensorhubCollaborationService::Init()
{
    RegisterScreenStateCallback();
    BluetoothHwInterface::GetInstance()->AddSensorhubObserver(*pimpl.get());
}

bool SensorhubCollaborationService::RegisterShCollaborationObserver(
    ISensorhubCollaborationObserver &observer) const
{
    return pimpl->shCollaborationStateObservers_.Register(observer);
}

bool SensorhubCollaborationService::DeregisterShCollaborationObserver(
    ISensorhubCollaborationObserver &observer) const
{
    if (pimpl == nullptr) {
        return false;
    }
    return pimpl->shCollaborationStateObservers_.Deregister(observer);
}

void SensorhubCollaborationService::SetScanReportChannelToLpDevice(int32_t scannerId, bool enable)
{
    pimpl->SetScanReportChannelToLpDevice(scannerId, enable);
}

void SensorhubCollaborationService::QueryCapability()
{
    pimpl->QueryCapability();
}

void SensorhubCollaborationService::AllConnectionComplete(const std::string &connName)
{
    pimpl->AllConnectionComplete(connName);
}

bool SensorhubCollaborationService::StartConnCollaboration(const std::string &connName)
{
    return pimpl->StartConnCollaboration(connName);
}

ISensorhubCollaboration *ISensorhubCollaboration::GetInstance()
{
    return SensorhubCollaborationService::GetInstance();
}

SensorhubCollaborationService *SensorhubCollaborationService::GetInstance()
{
    static SensorhubCollaborationService instance;
    return &instance;
}

}  // namespace bluetooth
}  // namespace OHOS
