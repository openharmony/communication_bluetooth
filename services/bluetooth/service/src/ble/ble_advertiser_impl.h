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

#ifndef BLE_ADVERTISER_IMPL_H
#define BLE_ADVERTISER_IMPL_H

#include <atomic>
#include <map>
#include <mutex>
#include <vector>
#include "safe_map.h"

#include "base_def.h"
#include "ble_defs.h"
#include "bt_ble_interface.h"
#include "interface_adapter_ble.h"
#include "bluetooth_resource_manager.h"

namespace OHOS {
namespace bluetooth {
enum class SetAdvDataState {
    ADV = 0,
    SCAN_RSP,
    COMPLETE,
};
struct AdvInfo {
    uint8_t advHandle = 0xFF;
    int advStatus = -1;
};

class BleAdvertiserImpl {
public:
    static constexpr uint16_t ADVERTISING_EVENT_PROP_LEGACY_CONNECTABLE = 0x0013;
    static constexpr uint16_t ADVERTISING_EVENT_PROP_LEGACY_SCANNABLE = 0x0012;
    static constexpr uint16_t ADVERTISING_EVENT_PROP_LEGACY_NON_CONNECTABLE = 0x0010;
    // Extended advertising event properties (Bit 4 = 0)
    // Extended PDU mode constraints:
    // - Bit 0 (Connectable) and Bit 1 (Scannable) cannot both be 1
    // - Bit 3 (High Duty Cycle Directed Connectable) must be 0
    static constexpr uint16_t ADVERTISING_EVENT_PROP_EXTENDED_CONNECTABLE = 0x0001;
    static constexpr uint16_t ADVERTISING_EVENT_PROP_EXTENDED_SCANNABLE = 0x0002;
    static constexpr uint16_t ADVERTISING_EVENT_PROP_EXTENDED_NON_CONNECTABLE = 0x0000;

    struct BleAdvertiserInstance {
        explicit BleAdvertiserInstance(uint8_t advHandle)
            : advHandle(advHandle), advStatus(ADVERTISE_NOT_STARTED), burstWindow(0), burstInterval(0)
        {}

        uint8_t advHandle;
        int advStatus;
        int burstWindow;
        int burstInterval;
    };

    BleAdvertiserImpl(IBleAdvertiserCallback &callback, IAdapterBle &bleAdapter);
    ~BleAdvertiserImpl();

    /**
     * @brief Start Bluetooth LE Advertising.
     *
     * @param [in] Advertising parameters.
     * @param [in] Advertising data.
     * @param [in] Scan response data.
     * @param [in] Advertising handle.
     * @param [in] Advertising duration.
     */
    void StartAdvertising(const BleAdvertiserSettingsImpl &settings, const BleAdvertiserDataImpl &advData,
        const BleAdvertiserDataImpl &scanResponse, uint8_t advHandle, uint16_t duration);
    // Set advertising data when advertising is started.
    void SetAdvertisingData(const BleAdvertiserDataImpl &advData, const BleAdvertiserDataImpl &scanResponse,
            uint8_t advHandle);
    // Set advertising data or Scan response data based on isAdv when advertising is started.
    void SetAdvOrRspData(const BleAdvertiserDataImpl &data, bool isAdv, uint8_t advHandle);

    /**
     * @brief Stop Bluetooth LE advertising.
     *
     * @param [in] Advertise handle.
     * @param [in] Advertise duration.
     */
    void EnableAdvertising(uint8_t advHandle, uint16_t duration);

     /**
     * @brief Stop Bluetooth LE advertising.
     *
     * @param [in] Advertise handle.
     */
    void DisableAdvertising(uint8_t advHandle);

    /**
     * @brief Stop Bluetooth LE advertising.
     *
     * @param [in] Advertise handle.
     */
    void StopAdvertising(uint8_t advHandle);
    // Stop all advertiser, only used in "ble disable".
    void StopAdvertisingAll(void);

    /**
     * @brief Cleans up advertisers.
     */
    void Close(uint8_t advHandle);

    /**
     * @brief Create advertising set handle.
     *
     * @return @c  Advertising handle.
     */
    uint8_t CreateAdvertiserSetHandle(int &advStatus);

    /**
     * @brief Unregister advertising handle.
     */
    void UnregisterAdvertisingHandle(int advHandle);

    /**
     * @brief Get advertising status
     *
     * @return @c advertiser status.
     */
    int GetAdvertisingStatus(void);

    /**
     * @brief Save burst advertising param
     *
     * @param [in] BurstAdvertise window.
     * @param [in] BurstAdvertise interval.
     * @param [in] Advertise handle.
     */
    void SaveBurstAdvertisingParam(int window, int interval, int advHandle);

    /**
     * @brief Create advertising set handle.
     *
     * @param [in] Advertise handle.
     * @return @c Check result
     */
    bool CheckBurstAdvertise(int advHandle);

    /**
     * @brief Start Bluetooth LE Advertising.
     *
     * @param [in] Advertise handle.
     * @param [in] settings advertising setting parameters.
     */
    void ChangeAdvertisingParams(uint8_t advHandle, const BleAdvertiserSettingsImpl &settings);

private:
    bool IsValidAdvHandle(uint8_t advHandle);
    bool IsAdvStarted(uint8_t advHandle);
    bool IsBurstAdvertise(int window, int interval);
    void ChangeAdvStatus(uint8_t advHandle, int advStatus);
    void OnAdvStartedEventFromStack(uint8_t advHandle, uint8_t status);
    void OnAdvEnabledEventFromStack(uint8_t advHandle, uint8_t status);
    void OnAdvDisabledEventFromStack(uint8_t advHandle, uint8_t status);
    void OnAdvChangeParamsEventFromStack(uint8_t advHandle, uint8_t status, int8_t txPower);
    void OnAdvStartedEvent(uint8_t advHandle, uint8_t status);
    void OnAdvEnabledEvent(uint8_t advHandle, uint8_t status);
    void OnAdvDisabledEvent(uint8_t advHandle, uint8_t status);
    void OnAdvStoppedEvent(uint8_t advHandle, uint8_t status);
    void ParseSettings(const BleAdvertiserSettingsImpl &settings, bool hasScanRsp,
        AdvertiseParameters *outParams) const;
    void RemoveAdFlag(std::vector<uint8_t> &data) const;
    void BleAdvertiserDataLog(const std::string &data, bool isScanRsp) const;
    std::pair<std::vector<uint8_t>, std::vector<uint8_t>> ConvertAndLogData(
        const BleAdvertiserDataImpl &advData, const BleAdvertiserDataImpl &scanRsp) const;
    void OnSetAdvertisingDataEvent(
        uint8_t advHandle, SetAdvDataState state, std::vector<uint8_t> rspData, uint8_t status) const;
    void OnAdvChangeParamsEvent(uint8_t advHandle, uint8_t status, int8_t txPower);
    void OnSetAdvDataEventByType(uint8_t advHandle, SetAdvDataType type, uint8_t status) const;

    // 一旦上报 BLE_ADV_START_FAILED_OP_CODE，意味着该 advHandle 将不再使用
    void ReportStartResultEvent(int result, uint8_t advHandle, int opcode = BLE_ADV_DEFAULT_OP_CODE) const
    {
        if (callback_ != nullptr) {
            callback_->OnStartResultEvent(result, advHandle, opcode);
            auto resourceMgr = BluetoothResourceManager::GetInstance();
            if (resourceMgr) {
                resourceMgr->SendAdvStartedEvent(result, advHandle);
            }
        }
    }

    void ReportEnableResultEvent(int result, uint8_t advHandle) const
    {
        if (callback_ != nullptr) {
            callback_->OnEnableResultEvent(result, advHandle);
        }
    }

    void ReportDisableResultEvent(int result, uint8_t advHandle) const
    {
        if (callback_ != nullptr) {
            callback_->OnDisableResultEvent(result, advHandle);
        }
    }

    // 该 Event 仅在 BLE disable 时自动关广播的回调中上报
    void ReportAutoStopAdvEvent(uint8_t advHandle) const
    {}

    void ReportStopResultEvent(int result, uint8_t advHandle) const
    {
        if (callback_ != nullptr) {
            callback_->OnStopResultEvent(result, advHandle);
            auto resourceMgr = BluetoothResourceManager::GetInstance();
            if (resourceMgr) {
                resourceMgr->SendAdvStoppedEvent(result, advHandle);
            }
        }
    }

    void ReportSetAdvDataEvent(int result, uint8_t advHandle, SetAdvDataType type = SET_ADV_DATA_BOTH) const
    {
        if (callback_) {
            callback_->OnSetAdvDataEvent(result, advHandle, type);
        }
    }

    void ReportChangeAdvResultEvent(int result, uint8_t advHandle) const
    {
        if (callback_ != nullptr) {
            callback_->OnChangeAdvResultEvent(result, advHandle);
        }
    }

    void RemoveAdvHandle(uint8_t advHandle)
    {
        if (btBleAdvertiser_) {
            btBleAdvertiser_->Unregister(advHandle);
        }
        advInstances_.Erase(advHandle);
    }

    // global mutex
    std::mutex mutex_ {};
    // store created advHandle
    SafeMap<uint8_t, BleAdvertiserInstance> advInstances_ {};

    // Advertising callback
    IBleAdvertiserCallback *callback_ = nullptr;
    IAdapterBle *bleAdapter_ = nullptr;
    BleAdvertiserInterface* btBleAdvertiser_ = nullptr;

    BT_DISALLOW_COPY_AND_ASSIGN(BleAdvertiserImpl);
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // BLE_ADVERTISER_IMPL_H
