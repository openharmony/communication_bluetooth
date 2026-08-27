/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_ble_advertiser"
#endif

#include "ble_advertiser_impl.h"

#include <algorithm>
#include <functional>
#include <future>
#include <sstream>
#include <vector>

#include "ble_adapter.h"
#include "ble_defs.h"
#include "btif_gatt.h"
#include "log.h"
#include "securec.h"
#include "thread_util.h"
#include "bluetooth_channel_rssi_manager.h"

namespace OHOS {
namespace bluetooth {
BleAdvertiserImpl::BleAdvertiserImpl(IBleAdvertiserCallback &callback, IAdapterBle &bleAdapter)
    : mutex_(), advInstances_(),
      callback_(&callback),
      bleAdapter_(&bleAdapter)
{
    btifBleAdvertiser_ = get_ble_advertiser_instance();
}

BleAdvertiserImpl::~BleAdvertiserImpl()
{
    // Clear advertiser handle
    advInstances_.Iterate([this](uint8_t id, BleAdvertiserInstance &instance) {
        if (btifBleAdvertiser_) {
            btifBleAdvertiser_->Unregister(id);
        }
    });
}

// Called in server thread
uint8_t BleAdvertiserImpl::CreateAdvertiserSetHandle(int &advStatus)
{
    std::lock_guard<std::mutex> lock(mutex_);

    uint8_t handle = BLE_INVALID_ADVERTISING_HANDLE;
    if (btifBleAdvertiser_) {
        auto promise = std::make_shared<std::promise<AdvInfo>>();
        std::future<AdvInfo> future = promise->get_future();
        std::weak_ptr<std::promise<AdvInfo>> promiseWptr(promise);
        btifBleAdvertiser_->RegisterAdvertiser(
            // callback in other thread
            [promiseWptr, btifBleAdvertiser = btifBleAdvertiser_](uint8_t advertiserId, uint8_t status) {
                // status code see: bluedroid/system/stack/include/ble_advertiser.h - BTM_BLE_MULTI_ADV_SUCCESS
                HILOGI("advHandle: %{public}u, status: %{public}u", advertiserId, status);
                AdvInfo res;
                res.advHandle = ((status == 0) ? advertiserId : BLE_INVALID_ADVERTISING_HANDLE);
                res.advStatus = status;
                auto ptr = promiseWptr.lock();
                if (ptr) {
                    ptr->set_value(res);
                } else {
                    HILOGE("advHandle timeout: %{public}u", advertiserId);
                    btifBleAdvertiser->Unregister(advertiserId);
                }
            });

        // wait for callback
        using namespace std::chrono_literals;
        if (future.wait_for(std::chrono::seconds(BLUETOOTH_BLE_CREATE_ADV_TIMEOUT)) != std::future_status::ready) {
            HILOGE("timeout");
            return handle;
        }
        AdvInfo advInfo = future.get();
        handle = advInfo.advHandle;
        advStatus = advInfo.advStatus;
    }

    if (handle != BLE_INVALID_ADVERTISING_HANDLE) {
        BleAdvertiserInstance newInstance(handle);
        newInstance.advHandle = handle;
        advInstances_.EnsureInsert(handle, newInstance);
    }
    return handle;
}

void BleAdvertiserImpl::UnregisterAdvertisingHandle(int advHandle)
{
    std::lock_guard<std::mutex> lock(mutex_);
    HILOGI("advHandle: %{public}d", advHandle);
    if (IsValidAdvHandle(advHandle)) {
        HILOGI("remove advInstance: %{public}d", advHandle);
        advInstances_.Erase(advHandle);
    }
    if (btifBleAdvertiser_) {
        btifBleAdvertiser_->Unregister(advHandle);
    }
}

// call in service thread
void BleAdvertiserImpl::OnAdvStartedEvent(uint8_t advHandle, uint8_t status)
{
    std::lock_guard<std::mutex> lock(mutex_);

    HILOGI("advHandle: %{public}u, status: %{public}u", advHandle, status);

    if (status != BT_STATUS_SUCCESS) {
        ReportStartResultEvent(ADVERTISE_FAILED_INTERNAL_ERROR, advHandle, BLE_ADV_START_FAILED_OP_CODE);
        // This handle is no longer used.
        RemoveAdvHandle(advHandle);
        return;
    }

    ReportStartResultEvent(ADVERTISE_SUCCESS, advHandle);
    if (bleAdapter_) {
        (static_cast<BleAdapter *>(bleAdapter_))->OnStartAdvertisingEvt();
    }

#ifdef COMM_PROTOCOL_RESOURCE_MGR_ENABLE
    BluetoothChannelRssiManager::GetInstance()->SendChannelRssiReadCommand();
#endif
}

void BleAdvertiserImpl::OnAdvEnabledEvent(uint8_t advHandle, uint8_t status)
{
    std::lock_guard<std::mutex> lock(mutex_);

    HILOGI("advHandle: %{public}u, status: %{public}u", advHandle, status);
    if (status != BT_STATUS_SUCCESS) {
        ReportEnableResultEvent(ADVERTISE_FAILED_INTERNAL_ERROR, advHandle);
        return;
    }

    ReportEnableResultEvent(ADVERTISE_SUCCESS, advHandle);
}

void BleAdvertiserImpl::OnAdvDisabledEvent(uint8_t advHandle, uint8_t status)
{
    std::lock_guard<std::mutex> lock(mutex_);

    HILOGI("advHandle: %{public}u, status: %{public}u", advHandle, status);
    if (status != 0x3C && status != BT_STATUS_SUCCESS) { // 0x3C mean enable adv duration timeout from btc
        ReportDisableResultEvent(ADVERTISE_FAILED_INTERNAL_ERROR, advHandle);
        return;
    }

    ReportDisableResultEvent(ADVERTISE_SUCCESS, advHandle);
}

// call in service thread
void BleAdvertiserImpl::OnAdvStoppedEvent(uint8_t advHandle, uint8_t status)
{
    HILOGD("advHandle: %{public}u, status: %{public}u", advHandle, status);

    if (status != BT_STATUS_SUCCESS) {
        ChangeAdvStatus(advHandle, ADVERTISE_FAILED_ALREADY_STARTED);
        ReportStopResultEvent(ADVERTISE_FAILED_INTERNAL_ERROR, advHandle);
        return;
    }

    ReportStopResultEvent(ADVERTISE_SUCCESS, advHandle);
    if (bleAdapter_) {
        (static_cast<BleAdapter *>(bleAdapter_))->OnStopAdvertisingEvt();
    }
}

// Callback in jni thread
void BleAdvertiserImpl::OnAdvStartedEventFromStack(uint8_t advHandle, uint8_t status)
{
    DoInBleThread([this, advHandle, status]() { this->OnAdvStartedEvent(advHandle, status); });
}

// Callback in jni thread
void BleAdvertiserImpl::OnAdvEnabledEventFromStack(uint8_t advHandle, uint8_t status)
{
    DoInBleThread([this, advHandle, status]() { this->OnAdvEnabledEvent(advHandle, status); });
}

// Callback in jni thread
void BleAdvertiserImpl::OnAdvDisabledEventFromStack(uint8_t advHandle, uint8_t status)
{
    DoInBleThread([this, advHandle, status]() { this->OnAdvDisabledEvent(advHandle, status); });
}

void BleAdvertiserImpl::OnAdvChangeParamsEventFromStack(uint8_t advHandle, uint8_t status, int8_t txPower)
{
    DoInBleThread([this, advHandle, status, txPower]() { this->OnAdvChangeParamsEvent(advHandle, status, txPower); });
}

bool BleAdvertiserImpl::IsValidAdvHandle(uint8_t advHandle)
{
    BleAdvertiserInstance instance(advHandle);
    return advInstances_.Find(advHandle, instance);
}

bool BleAdvertiserImpl::IsAdvStarted(uint8_t advHandle)
{
    BleAdvertiserInstance instance(advHandle);
    if (advInstances_.Find(advHandle, instance)) {
        return instance.advStatus == ADVERTISE_FAILED_ALREADY_STARTED;
    }
    return false;
}

void BleAdvertiserImpl::ChangeAdvStatus(uint8_t advHandle, int advStatus)
{
    advInstances_.Iterate([&advHandle, &advStatus](uint8_t advHandleIn, BleAdvertiserInstance &instance) {
        if (instance.advHandle == advHandle) {
            instance.advStatus = advStatus;
        }
    });
}

void BleAdvertiserImpl::ParseSettings(const BleAdvertiserSettingsImpl &settings, bool hasScanRsp,
    AdvertiseParameters *outParams)  const
{
    outParams->min_interval = settings.GetInterval();
    outParams->max_interval = settings.GetInterval();

    bool isLegacyMode = settings.IsLegacyMode();
    bool isConnectable = settings.IsConnectable();
    bool isScannable = hasScanRsp;

    if (isConnectable) {
        outParams->advertising_event_properties = isLegacyMode ?
            ADVERTISING_EVENT_PROP_LEGACY_CONNECTABLE : ADVERTISING_EVENT_PROP_EXTENDED_CONNECTABLE;
    } else if (isScannable) {
        outParams->advertising_event_properties = isLegacyMode ?
            ADVERTISING_EVENT_PROP_LEGACY_SCANNABLE : ADVERTISING_EVENT_PROP_EXTENDED_SCANNABLE;
    } else {
        outParams->advertising_event_properties = isLegacyMode ?
            ADVERTISING_EVENT_PROP_LEGACY_NON_CONNECTABLE : ADVERTISING_EVENT_PROP_EXTENDED_NON_CONNECTABLE;
    }

    outParams->channel_map = ADV_CHNL_ALL;
    outParams->tx_power = static_cast<int8_t>(settings.GetTxPower());
    outParams->primary_advertising_phy = settings.GetPrimaryPhy();
    outParams->secondary_advertising_phy = settings.GetSecondaryPhy();
    outParams->scan_request_notification_enable = 0;
    std::array<uint8_t, RawAddress::BT_ADDRESS_BYTE_LEN> addr = settings.GetOwnAddr();
    std::copy(addr.begin(), addr.end(), outParams->own_addr);
    outParams->own_addr_type = settings.GetOwnAddrType();
}

static std::string StringToHex(const std::string &str)
{
    const std::string hex = "0123456789ABCDEF";
    const uint8_t sizeFour = 4;
    std::stringstream ss;

    ss << "0x";
    for (size_t i = 0; i < str.size(); ++i) {
        uint8_t n = static_cast<uint8_t>(str[i]);
        ss << hex[n >> sizeFour] << hex[n & 0xF];
    }
    return ss.str();
}

void BleAdvertiserImpl::BleAdvertiserDataLog(const std::string &data, bool isScanRsp) const
{
    if (!data.empty()) {
        std::string prefix = isScanRsp ? "ScanRsp: " : "AdvData: ";
        auto str = StringToHex(data);
        HILOGD("%{public}s%{public}s", prefix.c_str(), str.c_str());
    }
}

std::pair<std::vector<uint8_t>, std::vector<uint8_t>> BleAdvertiserImpl::ConvertAndLogData(
    const BleAdvertiserDataImpl &advData, const BleAdvertiserDataImpl &scanRsp) const
{
    auto advPayload = advData.GetPayload();
    auto rspPayload = scanRsp.GetPayload();

    BleAdvertiserDataLog(advPayload, false);
    BleAdvertiserDataLog(rspPayload, true);

    auto adv = std::vector<uint8_t>(advPayload.begin(), advPayload.end());
    auto rsp = std::vector<uint8_t>(rspPayload.begin(), rspPayload.end());
    // bluedroid will fill the 'AD Flag' field, so we shall remove it if the application has set it.
    RemoveAdFlag(adv);

    return {std::move(adv), std::move(rsp)};
}

void BleAdvertiserImpl::RemoveAdFlag(std::vector<uint8_t> &data) const
{
    const uint8_t expectType = 0x01;
    const uint8_t expectLen = 0x03;
    if (data.size() >= expectLen && data[1] == expectType) {
        data.erase(data.begin(), data.begin() + expectLen);
    }
}

void BleAdvertiserImpl::StartAdvertising(const BleAdvertiserSettingsImpl &settings,
    const BleAdvertiserDataImpl &advData, const BleAdvertiserDataImpl &scanResponse,
    uint8_t advHandle, uint16_t duration)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (!IsValidAdvHandle(advHandle)) {
        HILOGE("Invalid advertiser handle");
        ReportStartResultEvent(ADVERTISE_FAILED_INTERNAL_ERROR, advHandle);
        return;
    }
    if (IsAdvStarted(advHandle)) {
        HILOGE("Already started");
        ReportStartResultEvent(ADVERTISE_FAILED_INTERNAL_ERROR, advHandle);
        return;
    }
    // Change the advertiser status in advance. If an error occurs, the advertiser status will be changed later.
    ChangeAdvStatus(advHandle, ADVERTISE_FAILED_ALREADY_STARTED);

    auto resourceMgr = BluetoothResourceManager::GetInstance();
    if (resourceMgr) {
        resourceMgr->AddAdvRecord(settings, advData, advHandle);
    }
    
    auto [advertiseData, scanResponseData] = ConvertAndLogData(advData, scanResponse);
    AdvertiseParameters params;

    if (!settings.IsLegacyMode() && settings.IsConnectable() && !scanResponseData.empty()) {
        HILOGE("Extended PDU mode: cannot be both connectable and scannable");
        ReportStartResultEvent(ADVERTISE_FAILED_INTERNAL_ERROR, advHandle);
        ChangeAdvStatus(advHandle, ADVERTISE_FAILED_INTERNAL_ERROR);
        return;
    }
    ParseSettings(settings, !scanResponseData.empty(), &params);

    if (btifBleAdvertiser_) {
        btifBleAdvertiser_->StartAdvertising(
            advHandle,
            [this, advHandle](uint8_t status) { OnAdvStartedEventFromStack(advHandle, status); },
            std::move(params), std::move(advertiseData), std::move(scanResponseData),
            duration,
            [this, advHandle](uint8_t status) { OnAdvDisabledEventFromStack(advHandle, status); });
    }
}

void BleAdvertiserImpl::EnableAdvertising(uint8_t advHandle, uint16_t duration)
{
    HILOGI("advHandle: %{public}u duration: %{public}u", advHandle, duration);

    std::lock_guard<std::mutex> lock(mutex_);

    if (!IsValidAdvHandle(advHandle)) {
        HILOGE("Invalid advertiser handle");
        ReportEnableResultEvent(ADVERTISE_FAILED_INTERNAL_ERROR, advHandle);
        return;
    }
    if (!IsAdvStarted(advHandle)) {
        HILOGE("advertiser not started");
        ReportEnableResultEvent(ADVERTISE_FAILED_INTERNAL_ERROR, advHandle);
        return;
    }

    if (btifBleAdvertiser_) {
        btifBleAdvertiser_->Enable(advHandle, true,
            [this, advHandle](uint8_t status) { OnAdvEnabledEventFromStack(advHandle, status); },
            duration, 0,
            [this, advHandle](uint8_t status) { OnAdvDisabledEventFromStack(advHandle, status); });
    }
}

void BleAdvertiserImpl::DisableAdvertising(uint8_t advHandle)
{
    HILOGI("advHandle: %{public}u", advHandle);

    std::lock_guard<std::mutex> lock(mutex_);

    if (!IsValidAdvHandle(advHandle)) {
        HILOGE("Invalid advertiser handle");
        ReportDisableResultEvent(ADVERTISE_FAILED_INTERNAL_ERROR, advHandle);
        return;
    }
    if (!IsAdvStarted(advHandle)) {
        HILOGE("advertiser not started");
        ReportDisableResultEvent(ADVERTISE_FAILED_INTERNAL_ERROR, advHandle);
        return;
    }

    if (btifBleAdvertiser_) {
        btifBleAdvertiser_->Enable(advHandle, false,
            [this, advHandle](uint8_t status) { OnAdvDisabledEventFromStack(advHandle, status); },
            0, 0,
            [](uint8_t status) {});
    }
}

void BleAdvertiserImpl::StopAdvertising(uint8_t advHandle)
{
    HILOGD("advHandle: %{public}u", advHandle);

    std::lock_guard<std::mutex> lock(mutex_);

    if (IsValidAdvHandle(advHandle)) {
        HILOGD("remove advInstance: %{public}d", advHandle);
        advInstances_.Erase(advHandle);
    }
    // Change the advertiser status in advance. If an error occurs, the advertiser status will be changed later.
    ChangeAdvStatus(advHandle, ADVERTISE_NOT_STARTED);

    if (btifBleAdvertiser_) {
        btifBleAdvertiser_->Unregister(advHandle);
    }
    OnAdvStoppedEvent(advHandle, BT_STATUS_SUCCESS);
}

void BleAdvertiserImpl::StopAdvertisingAll(void)
{
    std::vector<uint8_t> advIds;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        advInstances_.Iterate([&advIds](uint8_t id, BleAdvertiserInstance &instance) {
            advIds.push_back(id);
        });
    }

    for (uint8_t advId : advIds) {
        StopAdvertising(advId);
    }
}

void BleAdvertiserImpl::Close(uint8_t advHandle)
{
    StopAdvertising(advHandle);
}

int BleAdvertiserImpl::GetAdvertisingStatus()
{
    std::lock_guard<std::mutex> lock(mutex_);

    int status = ADVERTISE_NOT_STARTED;

    advInstances_.Iterate([&status](uint8_t id, BleAdvertiserInstance &instance) {
        if (instance.advStatus == ADVERTISE_FAILED_ALREADY_STARTED) {
            status = ADVERTISE_FAILED_ALREADY_STARTED;
        }
    });
    HILOGI("status: %{public}d", status);
    return status;
}

void BleAdvertiserImpl::OnSetAdvertisingDataEvent(uint8_t advHandle, SetAdvDataState state,
    std::vector<uint8_t> rspData, uint8_t status) const
{
    if (status != BT_STATUS_SUCCESS) {
        HILOGI("failed, advHandle: %{public}d status: %{public}d, state: %{public}d",
            advHandle, status, state);
        ReportSetAdvDataEvent(ADVERTISE_FAILED_INTERNAL_ERROR, advHandle);
        return;
    }

    if (state == SetAdvDataState::SCAN_RSP && btifBleAdvertiser_) {
        btifBleAdvertiser_->SetData(advHandle, true, std::move(rspData),
            [this, advHandle](uint8_t status) {
                OnSetAdvertisingDataEvent(advHandle, SetAdvDataState::COMPLETE, {}, status);
            });
    }

    if (state == SetAdvDataState::COMPLETE) {
        HILOGD("success");
        ReportSetAdvDataEvent(ADVERTISE_SUCCESS, advHandle);
    }
}

void BleAdvertiserImpl::OnAdvChangeParamsEvent(uint8_t advHandle, uint8_t status, int8_t txPower)
{
    HILOGI("advHandle:%{public}d status:%{public}d, txPower:%{public}d", advHandle, status, txPower);
    if (status != BT_STATUS_SUCCESS) {
        ReportChangeAdvResultEvent(ADVERTISE_FAILED_INTERNAL_ERROR, advHandle);
        return;
    }
    ReportChangeAdvResultEvent(ADVERTISE_SUCCESS, advHandle);
}

void BleAdvertiserImpl::SetAdvertisingData(const BleAdvertiserDataImpl &advData,
    const BleAdvertiserDataImpl &scanResponse, uint8_t advHandle)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (!IsValidAdvHandle(advHandle)) {
        HILOGE("Invalid advertiser handle");
        ReportSetAdvDataEvent(ADVERTISE_FAILED_INTERNAL_ERROR, advHandle);
        return;
    }
    if (!IsAdvStarted(advHandle)) {
        HILOGE("advertiser not started");
        ReportSetAdvDataEvent(ADVERTISE_NOT_STARTED, advHandle);
        return;
    }

    auto [advertiseData, scanResponseData] = ConvertAndLogData(advData, scanResponse);

    if (btifBleAdvertiser_) {
        btifBleAdvertiser_->SetData(advHandle, false, std::move(advertiseData),
            [this, advHandle, scanResponseData = std::move(scanResponseData)](uint8_t status) {
                OnSetAdvertisingDataEvent(advHandle, SetAdvDataState::SCAN_RSP, scanResponseData, status);
            });
    }
}

void BleAdvertiserImpl::SetAdvOrRspData(const BleAdvertiserDataImpl &data,
    bool isAdv, uint8_t advHandle)
{
    std::lock_guard<std::mutex> lock(mutex_);
    SetAdvDataType type = isAdv ? SET_ADV_DATA_ONLY_ADV : SET_ADV_DATA_ONLY_RSP;
    if (!IsValidAdvHandle(advHandle)) {
        HILOGE("Invalid advertiser handle");
        ReportSetAdvDataEvent(ADVERTISE_FAILED_INTERNAL_ERROR, advHandle, type);
        return;
    }
    if (!IsAdvStarted(advHandle)) {
        HILOGE("advertiser not started");
        ReportSetAdvDataEvent(ADVERTISE_NOT_STARTED, advHandle, type);
        return;
    }
    std::vector<uint8_t> sendData;
    if (isAdv) {
        auto advPayload = data.GetPayload();
        BleAdvertiserDataLog(advPayload, false);
        sendData = std::vector<uint8_t>(advPayload.begin(), advPayload.end());
        // bluedroid will fill the 'AD Flag' field, so we shall remove it if the application has set it.
        RemoveAdFlag(sendData);
    } else {
        auto rspPayload = data.GetPayload();
        BleAdvertiserDataLog(rspPayload, true);
        sendData = std::vector<uint8_t>(rspPayload.begin(), rspPayload.end());
    }
    if (btifBleAdvertiser_) {
        HILOGI("advHandle: %{public}d, type: %{public}d.", advHandle, type);
        btifBleAdvertiser_->SetData(advHandle, !isAdv, std::move(sendData),
            [this, advHandle, type](uint8_t status) { OnSetAdvDataEventByType(advHandle, type, status); });
    }
}

void BleAdvertiserImpl::OnSetAdvDataEventByType(uint8_t advHandle, SetAdvDataType type, uint8_t status) const
{
    HILOGI("advHandle: %{public}d, type: %{public}d.", advHandle, type);
    if (status != BT_STATUS_SUCCESS) {
        HILOGE("failed, advHandle: %{public}d status: %{public}d, type: %{public}d",
            advHandle, status, type);
        this->ReportSetAdvDataEvent(ADVERTISE_FAILED_INTERNAL_ERROR, advHandle, type);
    } else {
        this->ReportSetAdvDataEvent(ADVERTISE_SUCCESS, advHandle, type);
    }
}


void BleAdvertiserImpl::ChangeAdvertisingParams(uint8_t advHandle, const BleAdvertiserSettingsImpl &settings)
{
    HILOGI("advHandle: %{public}u", advHandle);
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsValidAdvHandle(advHandle)) {
        HILOGE("Invalid advertiser handle");
        ReportChangeAdvResultEvent(ADVERTISE_FAILED_INTERNAL_ERROR, advHandle);
        return;
    }
    if (!IsAdvStarted(advHandle)) {
        HILOGE("adv started not allow set parameters advHandle:%{public}d", advHandle);
        ReportChangeAdvResultEvent(ADVERTISE_NOT_STARTED, advHandle);
        return;
    }

    if (!settings.IsLegacyMode() && settings.IsConnectable()) {
        HILOGE("Extended PDU mode: cannot be both connectable and scannable");
        ReportChangeAdvResultEvent(ADVERTISE_FAILED_INTERNAL_ERROR, advHandle);
        ChangeAdvStatus(advHandle, ADVERTISE_FAILED_INTERNAL_ERROR);
        return;
    }

    AdvertiseParameters params;
    ParseSettings(settings, false, &params);
    if (btifBleAdvertiser_ != nullptr) {
        btifBleAdvertiser_->SetParameters(advHandle, std::move(params),
            [this, advHandle](uint8_t status, int8_t txPower) {
                OnAdvChangeParamsEventFromStack(advHandle, status, txPower);
            });
    } else {
        OnAdvChangeParamsEventFromStack(advHandle, ADVERTISE_FAILED_INTERNAL_ERROR, 0);
    }
}

void BleAdvertiserImpl::SaveBurstAdvertisingParam(int window, int interval, int advHandle)
{
    advInstances_.Iterate([&window, &interval, &advHandle](uint8_t advHandleIn, BleAdvertiserInstance &instance) {
        if (instance.advHandle == advHandle) {
            instance.burstWindow = window;
            instance.burstInterval = interval;
        }
    });
}

bool BleAdvertiserImpl::CheckBurstAdvertise(int advHandle)
{
    BleAdvertiserInstance instance(advHandle);
    if (advInstances_.Find(advHandle, instance)) {
        if (IsBurstAdvertise(instance.burstWindow, instance.burstInterval)) {
            return true;
        }
    }
    return false;
}

bool BleAdvertiserImpl::IsBurstAdvertise(int window, int interval)
{
    if ((window != 0) || (interval != 0)) {
        HILOGI("Is IsBurst Advertise, need to clear param");
        return true;
    }
    return false;
}

}  // namespace bluetooth
}  // namespace OHOS