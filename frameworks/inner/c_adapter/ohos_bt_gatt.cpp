/*
 * Copyright (C) 2021-2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_c_adapter_gatt"
#endif

#include "ohos_bt_gatt.h"

#include <unistd.h>
#include <cstdint>
#include <sstream>
#include <vector>
#include "__config"
#include "bluetooth_ble_advertiser.h"
#include "bluetooth_ble_central_manager.h"
#include "bluetooth_log.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_utils.h"
#include "cstdint"
#include "iosfwd"
#include "istream"
#include "new"
#include "ohos_bt_adapter_utils.h"
#include "ohos_bt_def.h"
#include "ostream"
#include "securec.h"
#include "streambuf"
#include "string"
#include "uuid.h"
#include "bluetooth_object_map.h"
#include "ohos_bt_gatt_utils.h"
#include "bluetooth_timer.h"
#include "ffrt.h"
#include <functional>
#include "ohos_bt_gap.h"


#ifdef __cplusplus
extern "C" {
#endif

using namespace std;

namespace OHOS {
namespace Bluetooth {
#define MAX_BLE_ADV_NUM 7

class BleCentralManagerCallbackWapper;
class BleAdvCallback;

static BtGattCallbacks *g_AppCallback;

static std::shared_ptr<BleAdvCallback> g_bleAdvCallbacks[MAX_BLE_ADV_NUM];
static std::shared_ptr<BleAdvertiser> g_BleAdvertiser = nullptr;
static mutex g_advMutex;

constexpr int32_t MAX_BLE_SCAN_NUM = 5;
static BluetoothObjectMap<std::shared_ptr<BleCentralManager>, (MAX_BLE_SCAN_NUM + 1)> g_bleCentralManagerMap;

static uint32_t g_advAddrTimerIds[MAX_BLE_ADV_NUM];
static mutex g_advTimerMutex;
// ffrt queue
static ffrt::queue startAdvQueue("startAdv_Queue");

class BleCentralManagerCallbackWapper : public BleCentralManagerCallback {
public:
    /**
     * @brief Scan result callback.
     *
     * @param result Scan result.
     * @since 6
     */
    void OnScanCallback(const BleScanResult &result) override
    {
        BtScanResultData scanResult;
        if (result.IsConnectable()) {
            scanResult.eventType = OHOS_BLE_EVT_LEGACY_CONNECTABLE;
        } else {
            scanResult.eventType = OHOS_BLE_EVT_LEGACY_NON_CONNECTABLE;
        }
        scanResult.dataStatus = OHOS_BLE_DATA_COMPLETE;
        scanResult.addrType = OHOS_BLE_RANDOM_DEVICE_ADDRESS;
        scanResult.primaryPhy = OHOS_BLE_SCAN_PHY_1M;
        scanResult.secondaryPhy = OHOS_BLE_SCAN_PHY_1M;
        scanResult.advSid = 1;
        scanResult.txPower = 1;
        scanResult.rssi = result.GetRssi();
        scanResult.directAddrType = OHOS_BLE_RANDOM_DEVICE_ADDRESS;
        GetAddrFromString(result.GetPeripheralDevice().GetDeviceAddr(), scanResult.addr.addr);
        vector<uint8_t> vec = result.GetPayload();
        scanResult.advData = vec.data();
        scanResult.advLen = vec.size();

        std::string strs;
        std::stringstream strStream;
        for (int i = 0; i < scanResult.advLen; i++) {
            char token[3] = {0}; // The length of token is 3.
            (void)sprintf_s(token, sizeof(token), "%02X", scanResult.advData[i]);
            strStream << token;
        }
        strStream >> strs;
        string address = result.GetPeripheralDevice().GetDeviceAddr();
        HILOGD("device: %{public}s, len: %{public}d, scan data: %{public}s",
            GetEncryptAddr(address).c_str(), scanResult.advLen, strs.c_str());
        if (appCallback != nullptr && appCallback->scanResultCb != nullptr) {
            appCallback->scanResultCb(&scanResult);
        } else {
            HILOGW("call back is null.");
        }
    }

    /**
     * @brief Scan result for found or lost callback type.
     *
     * @param result Scan result.
     * @param callbackType callback Type.
     * @since 12
     */
    void OnFoundOrLostCallback(const BleScanResult &result, uint8_t callbackType) override {};

    /**
     * @brief Batch scan results event callback.
     *
     * @param results Scan results.
     * @since 6
     */
    void OnBleBatchScanResultsEvent(const std::vector<BleScanResult> &results) override {}

    /**
     * @brief Start or Stop scan event callback.
     *
     * @param resultCode Scan result code.
     * @param isStartScan true->start scan, false->stop scan.
     * @since 6
     */
    void OnStartOrStopScanEvent(int32_t resultCode, bool isStartScan) override
    {
        HILOGD("resultCode: %{public}d, isStartScan: %{public}d", resultCode, isStartScan);
        if (appCallback != nullptr && appCallback->scanStateChangeCb != nullptr) {
            appCallback->scanStateChangeCb(resultCode, isStartScan);
        } else {
            HILOGE("call back is null.");
        }
    }

    /**
     * @brief Notify low power device msg callback.
     *
     * @param btUuid uuid.
     * @param msgType notify msgType.
     * @param value notify msg value.
     * @since 6
     */
    void OnNotifyMsgReportFromLpDevice(const UUID &btUuid, int msgType, const std::vector<uint8_t> &value) override
    {
        if (appCallback != nullptr && appCallback->lpDeviceInfoCb != nullptr) {
            BtUuid retUuid;
            string strUuid = btUuid.ToString();
            retUuid.uuid = (char *)strUuid.c_str();
            retUuid.uuidLen = strUuid.size();
            appCallback->lpDeviceInfoCb(&retUuid, msgType, const_cast<uint8_t*>(value.data()), value.size());
        }
    }

public:
    BleScanCallbacks *appCallback = nullptr;
};

class BleAdvCallback : public BleAdvertiseCallback {
public:
    explicit BleAdvCallback(int advId)
    {
        advId_ = advId;
    }

    void OnStartResultEvent(int32_t result, int32_t advHandle) override
    {
        HILOGD("advId: %{public}d, advHandle: %{public}d, ret: %{public}d", advId_, advHandle, result);
        int ret = (result == 0) ? OHOS_BT_STATUS_SUCCESS : OHOS_BT_STATUS_FAIL;
        advHandle_ = advHandle;
        if (g_AppCallback != nullptr && g_AppCallback->advEnableCb != nullptr) {
            g_AppCallback->advEnableCb(advId_, ret);
        } else {
            HILOGW("call back is null.");
        }
    }

    void OnEnableResultEvent(int result, int advHandle) override
    {
        HILOGI("advId:%{public}d, advHandle:%{public}d, ret:%{public}d", advId_, advHandle, result);
        int ret = (result == 0) ? OHOS_BT_STATUS_SUCCESS : OHOS_BT_STATUS_FAIL;
        if (g_AppCallback != nullptr && g_AppCallback->onEnableExCb != nullptr) {
            g_AppCallback->onEnableExCb(advId_, ret);
        }
    }

    void OnDisableResultEvent(int result, int advHandle) override
    {
        HILOGI("advId:%{public}d, advHandle:%{public}d, ret:%{public}d", advId_, advHandle, result);
        int ret = (result == 0) ? OHOS_BT_STATUS_SUCCESS : OHOS_BT_STATUS_FAIL;
        if (g_AppCallback != nullptr && g_AppCallback->onDisableExCb != nullptr) {
            g_AppCallback->onDisableExCb(advId_, ret);
        }
    }

    void OnStopResultEvent(int result, int advHandle) override
    {
        HILOGD("advId: %{public}d, advHandle: %{public}d, ret: %{public}d", advId_, advHandle, result);
        int ret = (result == 0) ? OHOS_BT_STATUS_SUCCESS : OHOS_BT_STATUS_FAIL;
        advHandle_ = 0xFF; // restore invalid advHandle
        if (g_AppCallback != nullptr && g_AppCallback->advDisableCb != nullptr) {
            g_AppCallback->advDisableCb(advId_, ret);
        } else {
            HILOGW("call back is null.");
        }
        {
            lock_guard<mutex> lock(g_advTimerMutex);
            if (g_advAddrTimerIds[advId_] != 0) {
                BluetoothTimer::GetInstance()->UnRegister(g_advAddrTimerIds[advId_]);
                g_advAddrTimerIds[advId_] = 0;
            } else {
                HILOGD("TimerId no registered, is 0.");
            }
        }
        lock_guard<mutex> lock(g_advMutex);
        g_bleAdvCallbacks[advId_] = nullptr;
        int i = 0;
        for (; i < MAX_BLE_ADV_NUM; i++) {
            if (g_bleAdvCallbacks[i] != nullptr) {
                break;
            }
        }
        if (i == MAX_BLE_ADV_NUM) {
            g_BleAdvertiser = nullptr;
        }
    }

    void OnSetAdvDataEvent(int result) override
    {
        HILOGD("advId: %{public}d, ret: %{public}d", advId_, result);
        int ret = (result == 0) ? OHOS_BT_STATUS_SUCCESS : OHOS_BT_STATUS_FAIL;
        if (g_AppCallback != nullptr && g_AppCallback->advDataCb  != nullptr) {
            g_AppCallback->advDataCb(advId_, ret);
        }
    }

    void OnGetAdvHandleEvent(int result, int advHandle) override
    {}

    void OnChangeAdvResultEvent(int result, int advHandle) override
    {
        HILOGI("advId:%{public}d, advHandle:%{public}d, ret:%{public}d", advId_, advHandle, result);
        int ret = (result == 0) ? OHOS_BT_STATUS_SUCCESS : OHOS_BT_STATUS_FAIL;
        if (g_AppCallback != nullptr && g_AppCallback->advChangeCb != nullptr) {
            g_AppCallback->advChangeCb(advId_, ret);
        }
    }

    int GetAdvHandle()
    {
        return advHandle_;
    }

protected:
    BleAdvertiserData *advData = nullptr;
    BleAdvertiserData *advResponseData = nullptr;
    BleAdvertiserSettings *advSetting = nullptr;

private:
    int advId_;
    int32_t advHandle_ = 0xFF;
};

/**
 * @brief Initializes the Bluetooth protocol stack.
 *
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the Bluetooth protocol stack is initialized;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int InitBtStack(void)
{
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Enables the Bluetooth protocol stack.
 *
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the Bluetooth protocol stack is enabled;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int EnableBtStack(void)
{
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Disables the Bluetooth protocol stack.
 *
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the Bluetooth protocol stack is disabled;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int DisableBtStack(void)
{
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Sets the Bluetooth device name.
 *
 * @param name Indicates the pointer to the name to set.
 * @param len Indicates the length of the name to set.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the Bluetooth device name is set;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int SetDeviceName(const char *name, unsigned int len)
{
    return OHOS_BT_STATUS_UNSUPPORTED;
}

static vector<uint8_t> ConvertDataToVec(uint8_t *data, unsigned int len)
{
    if (data == nullptr || len == 0) {
        return {};
    }

    return vector<uint8_t>(data, data + len);
}

/**
 * @brief Sets advertising data.
 *
 * @param advId Indicates the advertisement ID, which is allocated by the upper layer of the advertiser.
 * @param data Indicates the pointer to the advertising data. For details, see {@link StartAdvRawData}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if advertising data is set;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleSetAdvData(int advId, const StartAdvRawData data)
{
    if (advId < 0 || advId >= MAX_BLE_ADV_NUM) {
        HILOGE("Invalid advId (%{public}d)", advId);
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    lock_guard<mutex> lock(g_advMutex);
    if (g_BleAdvertiser == nullptr || g_bleAdvCallbacks[advId] == nullptr) {
        HILOGE("Adv is not started, need call 'BleStartAdvEx' first.");
        return OHOS_BT_STATUS_FAIL;
    }
    HILOGD("advId: %{public}d, advLen: %{public}u, scanRspLen: %{public}u", advId, data.advDataLen, data.rspDataLen);

    auto advData = ConvertDataToVec(data.advData, data.advDataLen);
    auto rspData = ConvertDataToVec(data.rspData, data.rspDataLen);
    g_BleAdvertiser->SetAdvertisingData(advData, rspData, g_bleAdvCallbacks[advId]);
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Starts advertising.
 *
 * @param advId Indicates the advertisement ID, which is allocated by the upper layer of the advertiser.
 * @param param Indicates the pointer to the advertising parameters. For details, see {@link BleAdvParams}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if advertising is started;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleStartAdv(int advId, const BleAdvParams *param)
{
    return OHOS_BT_STATUS_UNSUPPORTED;
}

/**
 * @brief Stops advertising.
 *
 * @param advId Indicates the advertisement ID, which is allocated by the upper layer of the advertiser.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if advertising is stopped;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleStopAdv(int advId)
{
    HILOGD("BleStopAdv, advId: %{public}d.", advId);
    if (advId < 0 || advId >= MAX_BLE_ADV_NUM) {
        HILOGE("BleStopAdv fail, advId is invalid.");
        return OHOS_BT_STATUS_FAIL;
    }
    lock_guard<mutex> lock(g_advMutex);
    if (g_BleAdvertiser == nullptr || g_bleAdvCallbacks[advId] == nullptr) {
        HILOGE("BleStopAdv fail, the current adv is not started.");
        return OHOS_BT_STATUS_FAIL;
    }
    {
        lock_guard<mutex> lock(g_advTimerMutex);
        BluetoothTimer::GetInstance()->UnRegister(g_advAddrTimerIds[advId]);
        g_advAddrTimerIds[advId] = 0;
    }

    std::function stopAdvFunc = [advId]() {
        HILOGI("stop adv in adv_Queue thread, advId = %{public}d", advId);
        lock_guard<mutex> lock(g_advMutex);
        int ret = g_BleAdvertiser->StopAdvertising(g_bleAdvCallbacks[advId]);
        if (ret != BT_NO_ERROR) {
            HILOGE("fail, advId: %{public}d, ret: %{public}d", advId, ret);
        }
    };
    startAdvQueue.submit(stopAdvFunc);

    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Updates advertising parameters.
 *
 * @param advId Indicates the advertisement ID, which is allocated by the upper layer of the advertiser.
 * @param param Indicates the pointer to the advertising parameters. For details, see {@link BleAdvParams}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if advertising parameters are updated;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleUpdateAdv(int advId, const BleAdvParams *param)
{
    return OHOS_BT_STATUS_UNSUPPORTED;
}

/**
 * @brief Sets the secure I/O capability mode.
 *
 * @param mode Indicates the capability mode to set. For details, see {@link BleIoCapMode}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the capability mode is set;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleSetSecurityIoCap(BleIoCapMode mode)
{
    return OHOS_BT_STATUS_UNSUPPORTED;
}

/**
 * @brief Sets the authentication mode for secure connection requests.
 *
 * @param mode Indicates the authentication mode to set. For details, see {@link BleAuthReqMode}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the authentication mode is set;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleSetSecurityAuthReq(BleAuthReqMode mode)
{
    return OHOS_BT_STATUS_UNSUPPORTED;
}

/**
 * @brief Responds to a secure connection request.
 *
 * @param bdAddr Indicates the address of the device that sends the request.
 * @param accept Specifies whether to accept the request. The value <b>true</b> means to accept the request,
 * and <b>false</b> means to reject the request.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the request is responded to;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattSecurityRsp(BdAddr bdAddr, bool accept)
{
    return OHOS_BT_STATUS_UNSUPPORTED;
}

/**
 * @brief Obtains the device MAC address.
 *
 * @param mac Indicates the pointer to the device MAC address.
 * @param len Indicates the length of the device MAC address.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the device MAC address is obtained;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int ReadBtMacAddr(unsigned char *mac, unsigned int len)
{
    return OHOS_BT_STATUS_UNSUPPORTED;
}

/**
 * @brief Sets scan parameters.
 *
 * @param clientId Indicates the client ID, which is obtained during client registration.
 * @param param Indicates the pointer to the scan parameters. For details, see {@link BleScanParams}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the scan parameters are set;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleSetScanParameters(int clientId, BleScanParams *param)
{
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Starts a scan.
 *
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the scan is started;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleStartScan(void)
{
    HILOGE("This interface is deprecated. BleStartScanEx is recommended.");
    return OHOS_BT_STATUS_UNSUPPORTED;
}

/**
 * @brief Stops a scan.
 *
 * @param scannerId Indicates the scanner id.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the scan is stopped;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleStopScan(int32_t scannerId)
{
    HILOGD("BleStopScan enter scannerId: %{public}d", scannerId);
    std::shared_ptr<BleCentralManager> bleCentralManager = g_bleCentralManagerMap.GetObject(scannerId);
    if (bleCentralManager == nullptr) {
        HILOGE("BleStopScan fail, scannerId is invalid.");
        return OHOS_BT_STATUS_FAIL;
    }

    bleCentralManager->StopScan();
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Registers GATT callbacks.
 * explain: This function does not support dynamic registration;
 * @param func Indicates the pointer to the callbacks to register. For details, see {@link BtGattCallbacks}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the GATT callbacks are registered;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattRegisterCallbacks(BtGattCallbacks *func)
{
    HILOGI("BleGattRegisterCallbacks enter");
    if (func == nullptr) {
        HILOGE("func is null.");
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    g_AppCallback = func;
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Register ble scan callbacks.
 *
 * @param func Indicates the pointer to the callbacks to register. For details, see {@link BleScanCallbacks}.
 * @param scannerId Indicates the pointer to the scannerId, identify one scan.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the BLE callbacks are registered;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleRegisterScanCallbacks(BleScanCallbacks *func, int32_t *scannerId)
{
    HILOGI("BleRegisterScanCallbacks enter");
    if (scannerId == nullptr || func == nullptr) {
        HILOGE("BleRegisterScanCallbacks scannerId or func is null");
        return OHOS_BT_STATUS_PARM_INVALID;
    }

    std::shared_ptr<BleCentralManagerCallbackWapper> callback = std::make_shared<BleCentralManagerCallbackWapper>();
    callback->appCallback = func;
    std::shared_ptr<BleCentralManager> bleCentralManager = std::make_shared<BleCentralManager>(callback);
    int id = g_bleCentralManagerMap.AddLimitedObject(bleCentralManager);
    if (id == -1) {
        HILOGE("BleRegisterScanCallbacks fail.");
        return OHOS_BT_STATUS_FAIL;
    }
    *scannerId = id;
    HILOGI("BleRegisterScanCallbacks success scannerId: %{public}d", *scannerId);
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Deregister ble scan callbacks.
 *
 * @param scannerId Indicates the scanner id.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the BLE callbacks are deregistered;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleDeregisterScanCallbacks(int32_t scannerId)
{
    HILOGI("BleDeregisterScanCallbacks enter. scannerId: %{public}d", scannerId);
    if (g_bleCentralManagerMap.GetObject(scannerId) == nullptr) {
        HILOGE("BleDeregisterScanCallbacks fail, scannerId is invalid.");
        return OHOS_BT_STATUS_FAIL;
    }
    g_bleCentralManagerMap.RemoveObject(scannerId);
    return OHOS_BT_STATUS_SUCCESS;
}

/*
 * RPA: The two highest bits of the broadcast address are 01, and address type is random.
 */
static bool IsRpa(const AdvOwnAddrParams *ownAddrParams)
{
    if (ownAddrParams != nullptr && ((ownAddrParams->addr[0] & 0xC0) ^ 0x40) == 0 &&
        ownAddrParams->addrType == BLE_ADDR_RANDOM) {
            return true;
        }
    return false;
}

/**
 * @brief Sets own address, own address type, advertising data and parameters, and then starts advertising.
 *
 * This function is available for softbus only.
 *
 * @param advId Indicates the pointer to the advertisement ID.
 * @param rawData Indicates the advertising data. For details, see {@link StartAdvRawData}.
 * @param advParam Indicates the advertising parameters. For details, see {@link BleAdvParams}.
 * @param ownAddrParams Indicates the own address(little endian) and own address type.
 * For details, see {@link AdvOwnAddrParams}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the operation is successful;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleStartAdvWithAddr(int *advId, const StartAdvRawData *rawData, const BleAdvParams *advParam,
    const AdvOwnAddrParams *ownAddrParams)
{
    HILOGD("BleStartAdvWithAddr enter");
    if (advId == nullptr || rawData == nullptr || advParam == nullptr || !IsRpa(ownAddrParams)) {
        HILOGW("params are invalid");
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    if (!IsBleEnabled()) {
        HILOGE("bluetooth is off.");
        *advId = -1;
        return OHOS_BT_STATUS_NOT_READY;
    }
    lock_guard<mutex> lock(g_advMutex);
    int i = AllocateAdvHandle();
    if (i == MAX_BLE_ADV_NUM) {
        HILOGW("reach the max num of adv");
        return OHOS_BT_STATUS_UNHANDLED;
    }
    *advId = i;

    if (!StartAdvAddrTimer(i, ownAddrParams)) {
        HILOGE("Duplicate addresses after 15 minutes are not allowed to be broadcast");
        g_bleAdvCallbacks[i] = nullptr;
        *advId = -1;
        return OHOS_BT_STATUS_DUPLICATED_ADDR;
    }

    BleAdvertiserSettings settings;
    settings.SetInterval(advParam->minInterval);
    if (advParam->advType == OHOS_BLE_ADV_SCAN_IND || advParam->advType == OHOS_BLE_ADV_NONCONN_IND) {
        settings.SetConnectable(false);
    }
    std::array<uint8_t, OHOS_BD_ADDR_LEN> addr;
    std::copy(std::begin(ownAddrParams->addr), std::end(ownAddrParams->addr), std::begin(addr));
    settings.SetOwnAddr(addr);
    settings.SetOwnAddrType(ownAddrParams->addrType);

    auto advData = ConvertDataToVec(rawData->advData, rawData->advDataLen);
    auto scanResponse = ConvertDataToVec(rawData->rspData, rawData->rspDataLen);
    if (g_BleAdvertiser == nullptr) {
        g_BleAdvertiser = BleAdvertiser::CreateInstance();
    }

    std::function startAdvFunc = [i, advData, scanResponse, settings]() {
        HILOGI("start adv in startAdv_Queue thread, handle = %{public}d", i);
        lock_guard<mutex> lock(g_advMutex);
        int ret = g_BleAdvertiser->StartAdvertising(settings, advData, scanResponse, 0, g_bleAdvCallbacks[i]);
        if (ret != BT_NO_ERROR) {
            HILOGE("fail, ret: %{public}d", ret);
            //StartAdvertise fail, return default handle -1 to softbus
            g_bleAdvCallbacks[i] = nullptr;
            {
                lock_guard<mutex> lock(g_advTimerMutex);
                BluetoothTimer::GetInstance()->UnRegister(g_advAddrTimerIds[i]);
                g_advAddrTimerIds[i] = 0;
            }
        }
    };
    startAdvQueue.submit(startAdvFunc);
    HILOGI("ret advId: %{public}d.", *advId);
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Sets advertising data and parameters and starts advertising.
 *
 * This function is available for system applications only. \n
 *
 * @param advId Indicates the pointer to the advertisement ID.
 * @param rawData Indicates the advertising data. For details, see {@link StartAdvRawData}.
 * @param advParam Indicates the advertising parameters. For details, see {@link BleAdvParams}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the operation is successful;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleStartAdvEx(int *advId, const StartAdvRawData rawData, BleAdvParams advParam)
{
    HILOGD("BleStartAdvEx enter");
    if (!IsBleEnabled()) {
        HILOGE("bluetooth is off.");
        *advId = -1;
        return OHOS_BT_STATUS_NOT_READY;
    }

    lock_guard<mutex> lock(g_advMutex);
    if (g_BleAdvertiser == nullptr) {
        g_BleAdvertiser = BleAdvertiser::CreateInstance();
    }
    int i = 0;
    for (i = 0; i < MAX_BLE_ADV_NUM; i++) {
        if (g_bleAdvCallbacks[i] == nullptr) {
            g_bleAdvCallbacks[i] = std::make_shared<BleAdvCallback>(i);
            break;
        }
    }

    if (i == MAX_BLE_ADV_NUM) {
        HILOGW("reach the max num of adv");
        return OHOS_BT_STATUS_UNHANDLED;
    }
    *advId = i;
    BleAdvertiserSettings settings;
    settings.SetInterval(advParam.minInterval);
    if (advParam.advType == OHOS_BLE_ADV_SCAN_IND || advParam.advType == OHOS_BLE_ADV_NONCONN_IND) {
        settings.SetConnectable(false);
    }
    auto advData = ConvertDataToVec(rawData.advData, rawData.advDataLen);
    auto scanResponse = ConvertDataToVec(rawData.rspData, rawData.rspDataLen);

    std::function startAdvFunc = [i, advData, scanResponse, settings]() {
        HILOGI("start adv in startAdv_Queue thread, handle = %{public}d", i);
        lock_guard<mutex> lock(g_advMutex);
        int ret = g_BleAdvertiser->StartAdvertising(settings, advData, scanResponse, 0, g_bleAdvCallbacks[i]);
        if (ret != BT_NO_ERROR) {
            HILOGE("fail, ret: %{public}d", ret);
            //StartAdvertise fail, return default handle -1 to softbus
            g_bleAdvCallbacks[i] = nullptr;
        }
    };
    startAdvQueue.submit(startAdvFunc);
    HILOGI("ret advId: %{public}d.", *advId);
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Enable advertising.
 *
 * This function is available for system applications only. \n
 *
 * @param advId Indicates the pointer to the advertisement ID.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the operation is successful;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 11
 */
int BleEnableAdvEx(int advId)
{
    HILOGI("BleEnableAdv, advId: %{public}d.", advId);
    if (advId < 0 || advId >= MAX_BLE_ADV_NUM) {
        HILOGE("BleEnableAdv fail, advId is invalid.");
        return OHOS_BT_STATUS_FAIL;
    }
    lock_guard<mutex> lock(g_advMutex);
    if (g_BleAdvertiser == nullptr || g_bleAdvCallbacks[advId] == nullptr) {
        HILOGE("BleEnableAdv fail, the current adv is not started.");
        return OHOS_BT_STATUS_FAIL;
    }

    int ret = g_BleAdvertiser->EnableAdvertising(g_bleAdvCallbacks[advId]->GetAdvHandle(), 0,
        g_bleAdvCallbacks[advId]);
    if (ret != BT_NO_ERROR) {
        HILOGE("fail, advId: %{public}d, ret: %{public}d", advId, ret);
        return OHOS_BT_STATUS_FAIL;
    }
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Disable advertising.
 *
 * This function is available for system applications only. \n
 *
 * @param advId Indicates the pointer to the advertisement ID.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the operation is successful;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 11
 */
int BleDisableAdvEx(int advId)
{
    HILOGI("BleDisableAdv, advId: %{public}d.", advId);
    if (advId < 0 || advId >= MAX_BLE_ADV_NUM) {
        HILOGE("BleDisableAdv fail, advId is invalid.");
        return OHOS_BT_STATUS_FAIL;
    }
    lock_guard<mutex> lock(g_advMutex);
    if (g_BleAdvertiser == nullptr || g_bleAdvCallbacks[advId] == nullptr) {
        HILOGE("BleDisableAdv fail, the current adv is not started.");
        return OHOS_BT_STATUS_FAIL;
    }

    int ret = g_BleAdvertiser->DisableAdvertising(g_bleAdvCallbacks[advId]->GetAdvHandle(), g_bleAdvCallbacks[advId]);
    if (ret != BT_NO_ERROR) {
        HILOGE("fail, advId: %{public}d, ret: %{public}d", advId, ret);
        return OHOS_BT_STATUS_FAIL;
    }
    return OHOS_BT_STATUS_SUCCESS;
}

static bool SetServiceUuidParameter(BleScanFilter &scanFilter, BleScanNativeFilter *nativeScanFilter)
{
    HILOGD("SetServiceUuidParameter enter");
    if (nativeScanFilter->serviceUuidLength != 0 && nativeScanFilter->serviceUuid != nullptr) {
        if (!IsValidUuid(std::string(reinterpret_cast<char *>(nativeScanFilter->serviceUuid)))) {
            HILOGE("match the UUID faild.");
            return false;
        }
        UUID serviceUuid = UUID::FromString((char *)nativeScanFilter->serviceUuid);
        scanFilter.SetServiceUuid(serviceUuid);
        if (nativeScanFilter->serviceUuidMask != nullptr) {
            if (!IsValidUuid(std::string(reinterpret_cast<char *>(nativeScanFilter->serviceUuidMask)))) {
                HILOGE("match the UUID faild.");
                return false;
            }
            UUID serviceUuidMask = UUID::FromString((char *)nativeScanFilter->serviceUuidMask);
            scanFilter.SetServiceUuidMask(serviceUuidMask);
        }
    }
    return true;
}

/**
 * @brief Sets one scan filter config.
 *
 * @param scanFilter Indicates the framework object of scan filter, see {@link BleScanFilter}.
 * @param nativeScanFilter Indicates the pointer to the scan filter. For details, see {@link BleScanNativeFilter}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if set onr scan filter config success;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
static int SetOneScanFilter(BleScanFilter &scanFilter, BleScanNativeFilter *nativeScanFilter)
{
    HILOGD("SetOneScanFilter enter");
    if (nativeScanFilter->address != nullptr) {
        scanFilter.SetDeviceId(nativeScanFilter->address);
    }
    if (nativeScanFilter->deviceName != nullptr) {
        scanFilter.SetName(nativeScanFilter->deviceName);
    }
    bool isSuccess = SetServiceUuidParameter(scanFilter, nativeScanFilter);
    if (!isSuccess) {
        HILOGE("SetServiceUuidParameter faild.");
        return OHOS_BT_STATUS_PARM_INVALID;
    }

    if (nativeScanFilter->serviceData != nullptr) {
        std::vector<uint8_t> serviceData;
        for (unsigned int i = 0; i < nativeScanFilter->serviceDataLength; i++) {
            serviceData.push_back(nativeScanFilter->serviceData[i]);
        }
        scanFilter.SetServiceData(serviceData);

        if (nativeScanFilter->serviceDataMask != nullptr) {
            std::vector<uint8_t> serviceDataMask;
            for (unsigned int i = 0; i < nativeScanFilter->serviceDataLength; i++) {
                serviceDataMask.push_back(nativeScanFilter->serviceDataMask[i]);
            }
            scanFilter.SetServiceDataMask(serviceDataMask);
        }
    }

    if (nativeScanFilter->manufactureData != nullptr) {
        std::vector<uint8_t> manufactureData;
        for (unsigned int i = 0; i < nativeScanFilter->manufactureDataLength; i++) {
            manufactureData.push_back(nativeScanFilter->manufactureData[i]);
        }
        scanFilter.SetManufactureData(manufactureData);

        if (nativeScanFilter->manufactureDataMask != nullptr) {
            std::vector<uint8_t> manufactureDataMask;
            for (unsigned int i = 0; i < nativeScanFilter->manufactureDataLength; i++) {
                manufactureDataMask.push_back(nativeScanFilter->manufactureDataMask[i]);
            }
            scanFilter.SetManufactureDataMask(manufactureDataMask);
        }

        if (nativeScanFilter->manufactureId != 0) {
            scanFilter.SetManufacturerId(nativeScanFilter->manufactureId);
        }
    }
    scanFilter.SetAdvIndReportFlag(nativeScanFilter->advIndReport);
    scanFilter.SetFilterIndex(nativeScanFilter->filterIndex);
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Sets scan filter configs.
 *
 * @param scannerId Indicates the scanner id.
 * @param filter Indicates the pointer to the scan filter. For details, see {@link BleScanNativeFilter}.
 * @param filterSize Indicates the number of the scan filter.
 * @param outScanFilters expected scan filters.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if set scan filter configs success;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
static int SetConfigScanFilter(int32_t scannerId, const BleScanNativeFilter *filter, uint32_t filterSize,
    vector<BleScanFilter> &outScanFilters)
{
    HILOGD("SetConfigScanFilter enter");
    for (uint32_t i = 0; i < filterSize; i++) {
        BleScanNativeFilter nativeScanFilter = filter[i];
        BleScanFilter scanFilter;
        int result = SetOneScanFilter(scanFilter, &nativeScanFilter);
        if (result != OHOS_BT_STATUS_SUCCESS) {
            HILOGE("SetOneScanFilter faild, result: %{public}d", result);
            return OHOS_BT_STATUS_PARM_INVALID;
        }
        outScanFilters.push_back(scanFilter);
    }
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Starts a scan with BleScanConfigs.
 * If don't need ble scan filter, set BleScanNativeFilter to NULL or filterSize to zero.
 * If one of the ble scan filtering rules is not required, set it to NULL.
 * For example, set the address to NULL when you don't need it.
 * Don't support only using manufactureId as filter conditions, need to use it with manufactureData.
 * The manufactureId need to be set a related number when you need a filtering condition of manufactureData.
 *
 * @param scannerId Indicates the scanner id.
 * @param configs Indicates the pointer to the scan filter. For details, see {@link BleScanConfigs}.
 * @param filter Indicates the pointer to the scan filter. For details, see {@link BleScanNativeFilter}.
 * @param filterSize Indicates the number of the scan filter.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the scan is started;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleStartScanEx(int32_t scannerId, const BleScanConfigs *configs, const BleScanNativeFilter *filter,
    uint32_t filterSize)
{
    if (configs == nullptr) {
        HILOGE("BleStartScanEx fail, configs is null.");
        return OHOS_BT_STATUS_FAIL;
    }
    HILOGD("BleStartScanEx enter, scannerId: %{public}d, filterSize %{public}u, scanMode: %{public}d",
        scannerId, filterSize, configs->scanMode);
    std::shared_ptr<BleCentralManager> bleCentralManager = g_bleCentralManagerMap.GetObject(scannerId);
    if (bleCentralManager == nullptr) {
        HILOGE("BleStartScanEx fail, scannerId is invalid.");
        return OHOS_BT_STATUS_FAIL;
    }

    vector<BleScanFilter> scanFilters;
    if (filter != nullptr && filterSize != 0) {
        int result = SetConfigScanFilter(scannerId, filter, filterSize, scanFilters);
        if (result != OHOS_BT_STATUS_SUCCESS) {
            HILOGE("SetConfigScanFilter faild, result: %{public}d", result);
            return OHOS_BT_STATUS_PARM_INVALID;
        }
    }

    BleScanSettings settings;
    settings.SetScanMode(configs->scanMode);
    bleCentralManager->StartScan(settings, scanFilters);
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Get BleCentralManager object.
 * get one from existing objects, if not exist, create one.
 * used to operate by BleCentralManager object, but not related to one scan.
 *
 * @param bleCentralManager the pointer of BleCentralManager.
 * @since 6
*/
void GetBleCentralManagerObject(std::shared_ptr<BleCentralManager> &bleCentralManager)
{
    bleCentralManager = g_bleCentralManagerMap.GetObject();
    if (bleCentralManager == nullptr) {
        HILOGI("no exist BleCentralManager");
        std::shared_ptr<BleCentralManagerCallbackWapper> callback = std::make_shared<BleCentralManagerCallbackWapper>();
        bleCentralManager = std::make_shared<BleCentralManager>(callback);
    }
}

/**
 * @brief set low power device adv param.
 *
 * @param duration advertise duration.
 * @param maxExtAdvEvents maximum number of extended advertising events.
 * @param window work window.
 * @param interval work interval.
 * @param advHandle Indicates the advertise handle.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if set success;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
*/
int SetLpDeviceAdvParam(int duration, int maxExtAdvEvents, int window, int interval, int advHandle)
{
    HILOGI("SetLpDeviceAdvParam enter. duration: %{public}d, maxExtAdvEvents: %{public}d, window: %{public}d \
        interval: %{public}d, advHandle: %{public}d", duration, maxExtAdvEvents, window, interval, advHandle);
    if (duration < LPDEVICE_ADVERTISING_DURATION_MIN || duration > LPDEVICE_ADVERTISING_DURATION_MAX) {
        HILOGE("duration out of range:, duration: %{public}d", duration);
        return OHOS_BT_STATUS_PARM_INVALID;
    }

    if (maxExtAdvEvents < LPDEVICE_ADVERTISING_EXTADVEVENT_MIN
        || maxExtAdvEvents > LPDEVICE_ADVERTISING_EXTADVEVENT_MAX) {
        HILOGE("maxExtAdvEvents out of range:, maxExtAdvEvents: %{public}d", maxExtAdvEvents);
        return OHOS_BT_STATUS_PARM_INVALID;
    }

    if (window < LPDEVICE_ADVERTISING_WINDOW_MIN || window > LPDEVICE_ADVERTISING_WINDOW_MAX) {
        HILOGE("window out of range:, window: %{public}d", window);
        return OHOS_BT_STATUS_PARM_INVALID;
    }

    if (interval < LPDEVICE_ADVERTISING_INTERVAL_MIN || interval > LPDEVICE_ADVERTISING_INTERVAL_MAX) {
        HILOGE("interval out of range:, interval: %{public}d", interval);
        return OHOS_BT_STATUS_PARM_INVALID;
    }

    if (interval < window) {
        HILOGE("interval must bigger than window, interval: %{public}d, window: %{public}d", interval, window);
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    std::shared_ptr<BleCentralManager> bleCentralManager = nullptr;
    GetBleCentralManagerObject(bleCentralManager);
    if (bleCentralManager == nullptr) {
        HILOGE("get BleCentralManager fail.");
        return OHOS_BT_STATUS_FAIL;
    }
    int ret = bleCentralManager->SetLpDeviceAdvParam(duration, maxExtAdvEvents, window, interval, advHandle);
    if (ret != BT_NO_ERROR) {
        HILOGE("fail, advHandle: %{public}d,ret: %{public}d", advHandle, ret);
        return OHOS_BT_STATUS_FAIL;
    }
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Set scan report channel.
 *
 * @param scannerId Indicates the scanner id.
 * @param enable true：report to low power device; false：not report to low power device.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if set report channel success;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int SetScanReportChannelToLpDevice(int32_t scannerId, bool enable)
{
    HILOGD("SetScanReportChannelToLpDevice enter. scannerId: %{public}d, isToAp: %{public}d", scannerId, enable);
    std::shared_ptr<BleCentralManager> bleCentralManager = g_bleCentralManagerMap.GetObject(scannerId);
    if (bleCentralManager == nullptr) {
        HILOGE("SetScanReportChannelToLpDevice fail, ble centra manager is null.");
        return OHOS_BT_STATUS_FAIL;
    }
    int ret = bleCentralManager->SetScanReportChannelToLpDevice(enable);
    if (ret != BT_NO_ERROR) {
        HILOGE("fail, scannerId: %{public}d, ret: %{public}d", scannerId, ret);
        return OHOS_BT_STATUS_FAIL;
    }
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Enable synchronizing data to low power device.
 *
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if enable sync success;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int EnableSyncDataToLpDevice(void)
{
    HILOGI("EnableSyncDataToLpDevice enter");
    std::shared_ptr<BleCentralManager> bleCentralManager = nullptr;
    GetBleCentralManagerObject(bleCentralManager);
    if (bleCentralManager == nullptr) {
        HILOGE("get BleCentralManager fail.");
        return OHOS_BT_STATUS_FAIL;
    }
    int ret = bleCentralManager->EnableSyncDataToLpDevice();
    if (ret != BT_NO_ERROR) {
        HILOGE("fail, ret: %{public}d", ret);
        return OHOS_BT_STATUS_FAIL;
    }
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Disable synchronizing data to the low power device.
 *
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if disable sync success;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int DisableSyncDataToLpDevice(void)
{
    HILOGD("DisableSyncDataToLpDevice enter");
    std::shared_ptr<BleCentralManager> bleCentralManager = nullptr;
    GetBleCentralManagerObject(bleCentralManager);
    if (bleCentralManager == nullptr) {
        HILOGE("get BleCentralManager fail.");
        return OHOS_BT_STATUS_FAIL;
    }
    int ret = bleCentralManager->DisableSyncDataToLpDevice();
    if (ret != BT_NO_ERROR) {
        HILOGE("fail, ret: %{public}d", ret);
        return OHOS_BT_STATUS_FAIL;
    }
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Get advertiser handle.
 *
 * @param advId Indicates the advertisement ID.
 * @param advHandle Indicates the pointer to the advertiser handle.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if get success;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int GetAdvHandle(int advId, int *advHandle)
{
    HILOGI("GetAdvHandle enter. advId: %{public}d", advId);
    if (advHandle == nullptr) {
        HILOGE("GetAdvHandle fail, advHandle is nullptr.");
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    if (advId < 0 || advId >= MAX_BLE_ADV_NUM) {
        HILOGE("GetAdvHandle fail, advId is invalid.advId: %{public}d.", advId);
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    lock_guard<mutex> lock(g_advMutex);
    if (g_BleAdvertiser == nullptr || g_bleAdvCallbacks[advId] == nullptr) {
        HILOGE("GetAdvHandle fail, the current adv is not started.");
        return OHOS_BT_STATUS_FAIL;
    }
    *advHandle = g_BleAdvertiser->GetAdvHandle(g_bleAdvCallbacks[advId]);
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Translate ParamData to low power device.
 *
 * @param data Indicates the pointer to the data.
 * @param dataSize Indicates the data size.
 * @param type Indicates the data type.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if set param to low power device success;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int SendParamsToLpDevice(const uint8_t *data, uint32_t dataSize, int32_t type)
{
    HILOGI("SendParamsToLpDevice enter. type: %{public}d, dataSize: %{public}d", type, dataSize);
    if (data == nullptr || dataSize <= 0) {
        HILOGE("SendParamsToLpDevice fail, data is nullptr or dataSize is wrong.");
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    std::shared_ptr<BleCentralManager> bleCentralManager = nullptr;
    GetBleCentralManagerObject(bleCentralManager);
    if (bleCentralManager == nullptr) {
        HILOGE("get BleCentralManager fail.");
        return OHOS_BT_STATUS_FAIL;
    }
    std::vector<uint8_t> dataValue(data, data + dataSize);
    int ret = bleCentralManager->SendParamsToLpDevice(std::move(dataValue), type);
    if (ret != BT_NO_ERROR) {
        HILOGE("fail, ret: %{public}d", ret);
        return OHOS_BT_STATUS_FAIL;
    }
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Get whether low power device available.
 *
 * @return true: available; false: not available.
 * @since 6
 */
bool IsLpDeviceAvailable(void)
{
    HILOGI("IsLpDeviceAvailable enter.");
    std::shared_ptr<BleCentralManager> bleCentralManager = nullptr;
    GetBleCentralManagerObject(bleCentralManager);
    if (bleCentralManager == nullptr) {
        HILOGE("get BleCentralManager fail.");
        return false;
    }

    return bleCentralManager->IsLpDeviceAvailable();
}

void BleScanNativeFilterLog(BleScanNativeFilter &filter)
{
    if (filter.address != nullptr) {
        std::string address(filter.address);
        HILOGI("address: %{public}s", GetEncryptAddr(filter.address).c_str());
    }
    if (filter.deviceName != nullptr) {
        HILOGI("deviceName: %{public}s", filter.deviceName);
    }
    if (filter.serviceUuidLength != 0) {
        if (filter.serviceUuid != nullptr) {
            HILOGI("serviceUuid: %{public}s", reinterpret_cast<char *>(filter.serviceUuid));
        }
        if (filter.serviceUuidMask != nullptr) {
            HILOGI("serviceUuidMask: %{public}s", reinterpret_cast<char *>(filter.serviceUuidMask));
        }
    }
    std::string dataStr;
    if (filter.serviceDataLength != 0) {
        if (filter.serviceData != nullptr) {
            dataStr = "";
            ConvertDataToHex(filter.serviceData, filter.serviceDataLength, dataStr);
            HILOGI("serviceData: %{public}s", dataStr.c_str());
        }
        if (filter.serviceDataMask != nullptr) {
            dataStr = "";
            ConvertDataToHex(filter.serviceDataMask, filter.serviceDataLength, dataStr);
            HILOGI("serviceDataMask: %{public}s", dataStr.c_str());
        }
    }
    if (filter.manufactureDataLength != 0) {
        if (filter.manufactureData != nullptr) {
            dataStr = "";
            ConvertDataToHex(filter.manufactureData, filter.manufactureDataLength, dataStr);
            HILOGI("manufactureData: %{public}s", dataStr.c_str());
        }
        if (filter.manufactureDataMask != nullptr) {
            dataStr = "";
            ConvertDataToHex(filter.manufactureDataMask, filter.manufactureDataLength, dataStr);
            HILOGI("manufactureDataMask: %{public}s", dataStr.c_str());
        }
        HILOGI("manufactureId: %{public}d", filter.manufactureId);
    }
}

bool ConvertScanFilterParam(const BtLpDeviceParam *param, std::vector<BleScanFilter> &filter)
{
    unsigned int maxFilterSize = 10;  // max filter size
    if (param->filterSize > maxFilterSize) {
        HILOGE("filterSize(%{public}u) is larger than %{public}u", param->filterSize, maxFilterSize);
        return false;
    }
    for (unsigned int i = 0; i < param->filterSize; i++) {
        BleScanNativeFilter nativeScanFilter = param->filter[i];
        BleScanNativeFilterLog(nativeScanFilter);
        BleScanFilter scanFilter;
        int result = SetOneScanFilter(scanFilter, &nativeScanFilter);
        if (result != OHOS_BT_STATUS_SUCCESS) {
            HILOGE("SetOneScanFilter faild, result: %{public}d", result);
            return false;
        }
        filter.push_back(scanFilter);
    }
    return true;
}

void ConvertAdvSettingParam(const BtLpDeviceParam *param, BleAdvertiserSettings &advSettings)
{
    HILOGI("BleAdvParams: minInterval: %{public}d, advType: %{public}d",
        param->advParam.minInterval, param->advParam.advType);
    advSettings.SetInterval(param->advParam.minInterval);
    if (param->advParam.advType == OHOS_BLE_ADV_SCAN_IND ||
        param->advParam.advType == OHOS_BLE_ADV_NONCONN_IND) {
        advSettings.SetConnectable(false);
    }
}

bool ConvertAdvDeviceInfo(const BtLpDeviceParam *param, std::vector<BleActiveDeviceInfo> &activeDeviceInfos)
{
    if (param->activeDeviceInfo == nullptr || param->activeDeviceSize == 0) {
        return true;
    }
    unsigned int maxActiveDevice = 10;  // max active device
    if (param->activeDeviceSize > maxActiveDevice) {
        HILOGE("activeDeviceSize(%{public}u) is larger than %{public}u", param->activeDeviceSize, maxActiveDevice);
        return false;
    }
    for (unsigned int i = 0; i < param->activeDeviceSize; i++) {
        HILOGI("ActiveDeviceInfo: status: %{public}d, timeOut: %{public}d",
            param->activeDeviceInfo[i].status, param->activeDeviceInfo[i].timeOut);
        BleActiveDeviceInfo deviceInfo;
        std::vector<int8_t> value(param->activeDeviceInfo[i].deviceId,
            param->activeDeviceInfo[i].deviceId + OHOS_ACTIVE_DEVICE_ID_LEN);
        deviceInfo.deviceId = value;
        deviceInfo.status = param->activeDeviceInfo[i].status;
        deviceInfo.timeOut = param->activeDeviceInfo[i].timeOut;
        activeDeviceInfos.push_back(deviceInfo);
    }
    return true;
}

bool ConvertBtUuid(const BtUuid &inUuid, UUID &outUuid)
{
    if (inUuid.uuid == nullptr || inUuid.uuidLen == 0) {
        HILOGE("uuid is NULL or len is 0.");
        return false;
    }
    string strUuid(inUuid.uuid);
    HILOGD("UUID: %{public}s", strUuid.c_str());
    if (!IsValidUuid(strUuid)) {
        HILOGE("match the UUID faild.");
        return false;
    }
    outUuid = UUID::FromString(strUuid);
    return true;
}

int ConvertLpDeviceParamData(const BtLpDeviceParam *inParam, BleLpDeviceParamSet &outParam)
{
    outParam.fieldValidFlagBit = 0;
    if (!ConvertBtUuid(inParam->uuid, outParam.uuid)) {
        return OHOS_BT_STATUS_PARM_INVALID;
    }

    if (inParam->scanConfig != nullptr) {
        outParam.scanSettings.SetScanMode(inParam->scanConfig->scanMode);
        outParam.fieldValidFlagBit |= BLE_LPDEVICE_SCAN_SETTING_VALID_BIT;
    }

    if (inParam->filter != nullptr && inParam->filterSize > 0) {
        if (!ConvertScanFilterParam(inParam, outParam.scanFilters)) {
            return OHOS_BT_STATUS_PARM_INVALID;
        }
        outParam.fieldValidFlagBit |= BLE_LPDEVICE_SCAN_FILTER_VALID_BIT;
    }

    outParam.fieldValidFlagBit |= BLE_LPDEVICE_ADV_SETTING_VALID_BIT;
    ConvertAdvSettingParam(inParam, outParam.advSettings);

    if (inParam->rawData.advData != nullptr && inParam->rawData.advDataLen > 0) {
        outParam.advData = ConvertDataToVec(inParam->rawData.advData,
            inParam->rawData.advDataLen);
        outParam.fieldValidFlagBit |= BLE_LPDEVICE_ADVDATA_VALID_BIT;
    }

    if (inParam->rawData.rspData != nullptr && inParam->rawData.rspDataLen > 0) {
        outParam.respData = ConvertDataToVec(inParam->rawData.rspData,
            inParam->rawData.rspDataLen);
        outParam.fieldValidFlagBit |= BLE_LPDEVICE_RESPDATA_VALID_BIT;
    }

    if (inParam->activeDeviceInfo != nullptr && inParam->activeDeviceSize > 0) {
        if (!ConvertAdvDeviceInfo(inParam, outParam.activeDeviceInfos)) {
            return OHOS_BT_STATUS_PARM_INVALID;
        }
        outParam.fieldValidFlagBit |= BLE_LPDEVICE_ADV_DEVICEINFO_VALID_BIT;
    }

    outParam.advHandle = inParam->advHandle;
    outParam.duration = inParam->duration;
    outParam.deliveryMode = inParam->deliveryMode;
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Set low power device Param.
 *
 * @param lpDeviceParam the param set to low power device.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if set lpDeviceParam success;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int SetLpDeviceParam(const BtLpDeviceParam *lpDeviceParam)
{
    HILOGI("SetLpDeviceParam enter.");
    if (lpDeviceParam == nullptr) {
        HILOGE("SetLpDeviceParam fail, lpDeviceParam is null.");
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    BleLpDeviceParamSet paramSet;
    int ret = ConvertLpDeviceParamData(lpDeviceParam, paramSet);
    if (ret != OHOS_BT_STATUS_SUCCESS) {
        return ret;
    }
    std::shared_ptr<BleCentralManager> bleCentralManager = nullptr;
    GetBleCentralManagerObject(bleCentralManager);
    if (bleCentralManager == nullptr) {
        HILOGE("get BleCentralManager fail.");
        return OHOS_BT_STATUS_FAIL;
    }

    HILOGI("SetLpDeviceParam fieldValidFlagBit: %{public}u", paramSet.fieldValidFlagBit);
    ret = bleCentralManager->SetLpDeviceParam(paramSet);
    if (ret != BT_NO_ERROR) {
        HILOGE("fail, advHandle: %{public}d, ret: %{public}d", paramSet.advHandle, ret);
        return OHOS_BT_STATUS_FAIL;
    }
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Remove low power device Param.
 *
 * @param uuid Uuid.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if remove success;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int RemoveLpDeviceParam(BtUuid uuid)
{
    HILOGI("RemoveLpDeviceParam enter.");
    std::shared_ptr<BleCentralManager> bleCentralManager = nullptr;
    GetBleCentralManagerObject(bleCentralManager);
    if (bleCentralManager == nullptr) {
        HILOGE("get BleCentralManager fail.");
        return OHOS_BT_STATUS_FAIL;
    }
    UUID srvUuid;
    if (!ConvertBtUuid(uuid, srvUuid)) {
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    int ret = bleCentralManager->RemoveLpDeviceParam(srvUuid);
    if (ret != BT_NO_ERROR) {
        HILOGE("fail, ret: %{public}d", ret);
        return OHOS_BT_STATUS_FAIL;
    }
    return OHOS_BT_STATUS_SUCCESS;
}

void ClearGlobalResource(void)
{
    int i = 0;
    lock_guard<mutex> lock(g_advMutex);
    for (i = 0; i < MAX_BLE_ADV_NUM; i++) {
        if (g_bleAdvCallbacks[i] != nullptr) {
            g_bleAdvCallbacks[i] = nullptr;
        }
    }
    HILOGD("clear all g_bleAdvCallbacks when ble turn on or bluetooth_serivce unload");
}

bool StartAdvAddrTimer(int advHandle, const AdvOwnAddrParams *ownAddrParams)
{
    RemoveTimeoutAdvAddr();
    string addrStr;
    ConvertAddr(ownAddrParams->addr, addrStr);
    if (!CanStartAdv(addrStr)) {
        g_bleAdvCallbacks[advHandle] = nullptr;
        return false;
    }
    // adv must stop after {@link ADV_ADDR_TIME_THRESHOLD}
    auto timerCallback = [advHandle]() {
        HILOGI("Stop adv : %{public}d.", advHandle);
        BleStopAdv(advHandle);
    };
    uint32_t timerId = 0;
    BluetoothTimer::GetInstance()->Register(timerCallback, timerId, ADV_ADDR_TIME_THRESHOLD);
    {
        lock_guard<mutex> lock(g_advTimerMutex);
        g_advAddrTimerIds[advHandle] = timerId;
    }
    return true;
}

/**
 * @brief Change Advertising parameters when adv is disabled. If advertising started, an error will be returned.
 *
 * @param advId Indicates the advertisement ID.
 * @param advParam Indicates the advertising parameters. For details, see {@link BleAdvParams}. If you don't want
 * to cahnge current advertising address, please set ownAddr to 0xFF.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the advertising is started.
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 16
 */
int BleChangeAdvParams(int advId, const BleAdvParams advParam)
{
    HILOGI("advId:%{public}d", advId);
    if (advId < 0 || advId >= MAX_BLE_ADV_NUM) {
        HILOGW("Invaild advId:%{public}d", advId);
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    lock_guard<mutex> lock(g_advMutex);
    if (g_BleAdvertiser == nullptr || g_bleAdvCallbacks[advId] == nullptr) {
        HILOGE("Adv is not started, need call BleStartAdvEx first.");
        return OHOS_BT_STATUS_FAIL;
    }
    BleAdvertiserSettings settings;
    settings.SetInterval(advParam.minInterval);
    if ((advParam.advType == OHOS_BLE_ADV_SCAN_IND) || (advParam.advType == OHOS_BLE_ADV_NONCONN_IND)) {
        settings.SetConnectable(false);
    }
    return g_BleAdvertiser->ChangeAdvertisingParams(g_bleAdvCallbacks[advId]->GetAdvHandle(), settings);
}

/**
 * @brief Change a scan with BleScanConfigs and filter. Please make sure scan is started.
 * If don't change ble scan filter, set BleScanNativeFilter to nullptr or filterSzie to zero.
 * Don't support only using manufactureId as filter conditions, need to use it with manufactureData.
 * The manufactureId need to be set a related number when you need a filtering confition of manufactureData.
 *
 * @param scannerId Indicates the scanner id.
 * @param configs Indicates the pointer to the scan filter. For details, see {@link BleScanConfigs}.
 * @param filter Indicates the pointer to the scan filter. For details, see {@link BleScanNativeFilter}.
 * @param filterSize Indicates the number of the scan filter.
 * @param filterAction Indicates change filter behavior. See {@link BleScanUpdateFilterAction}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the scan is started;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 16
 */
int BleChangeScanParams(int32_t scannerId, const BleScanConfigs *config, const BleScanNativeFilter *filter,
    uint32_t filterSize, uint32_t filterAction)
{
    if (config == nullptr) {
        HILOGE("config invaild action:%{public}d", filterAction);
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    std::shared_ptr<BleCentralManager> bleCentralManager = g_bleCentralManagerMap.GetObject(scannerId);
    if (bleCentralManager == nullptr) {
        HILOGE("invaild scannerId:%{public}d", scannerId);
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    std::vector<BleScanFilter> scanFilters;
    if (filter != nullptr && filterSize != 0) {
        int result = SetConfigScanFilter(scannerId, filter, filterSize, scanFilters);
        if (result != OHOS_BT_STATUS_SUCCESS) {
            HILOGE("SetConfigScanFilter failed result:%{public}d", result);
            return OHOS_BT_STATUS_FAIL;
        }
    }
    BleScanSettings settings;
    settings.SetScanMode(config->scanMode);
    return bleCentralManager->ChangeScanParams(settings, scanFilters, filterAction);
}

int AllocateAdvHandle(void)
{
    int i = 0;
    for (; i < MAX_BLE_ADV_NUM; i++) {
        if (g_bleAdvCallbacks[i] == nullptr) {
            g_bleAdvCallbacks[i] = make_shared<BleAdvCallback>(i);
            break;
        }
    }
    return i;
}

}  // namespace Bluetooth
}  // namespace OHOS
#ifdef __cplusplus
}
#endif
/** @} */
