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
#include <queue>
#include <set>


#ifdef __cplusplus
extern "C" {
#endif

using namespace std;

namespace OHOS {
namespace Bluetooth {
#define MAX_BLE_ADV_NUM 7
#define MAX_ADV_ADDR_MAP_SIZE 128
#define ADV_ADDR_TIMEOUT (60 * 60 * 1000) // 1 hour
#define ADV_ADDR_TIME_THRESHOLD (15 * 60 * 1000) // 15 mins
#define MS_PER_SECOND 1000
#define NS_PER_MS 1000000

class BleCentralManagerCallbackWapper;
class BleAdvCallback;

static BtGattCallbacks *g_AppCallback;

static std::shared_ptr<BleAdvCallback> g_bleAdvCallbacks[MAX_BLE_ADV_NUM];
static std::shared_ptr<BleAdvertiser> g_BleAdvertiser = nullptr;

constexpr int32_t MAX_BLE_SCAN_NUM = 5;
static BluetoothObjectMap<std::shared_ptr<BleCentralManager>, (MAX_BLE_SCAN_NUM + 1)> g_bleCentralManagerMap;

static mutex g_advMutex;
static set<string> g_advAddrs;
static queue<pair<string, uint64_t>> g_advTimeQueue; // pair<addr, time>

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
        scanResult.eventType = OHOS_BLE_EVT_LEGACY_CONNECTABLE; // result.IsConnectable();
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
            char token[3] = {0};
            (void)sprintf_s(token, 3, "%02X", scanResult.advData[i]);
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

    void OnStartResultEvent(int result, int advHandle) override
    {
        HILOGI("advId: %{public}d, ret: %{public}d", advId_, result);
        int ret = (result == 0) ? OHOS_BT_STATUS_SUCCESS : OHOS_BT_STATUS_FAIL;
        advHandle_ = advHandle;
        if (g_AppCallback != nullptr && g_AppCallback->advEnableCb != nullptr) {
            g_AppCallback->advEnableCb(advId_, ret);
        } else {
            HILOGW("call back is null.");
        }
    }

    void OnEnableResultEvent(int result, int advHandle) override
    {}

    void OnDisableResultEvent(int result, int advHandle) override
    {}

    void OnStopResultEvent(int result, int advHandle) override
    {}

    void OnSetAdvDataEvent(int result) override
    {
        HILOGI("advId: %{public}d, ret: %{public}d", advId_, result);
        int ret = (result == 0) ? OHOS_BT_STATUS_SUCCESS : OHOS_BT_STATUS_FAIL;
        if (g_AppCallback != nullptr && g_AppCallback->advDataCb  != nullptr) {
            g_AppCallback->advDataCb(advId_, ret);
        }
    }

    void OnGetAdvHandleEvent(int result, int advHandle) override
    {}

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
    int advHandle_ = 0xFF;
};

/**
 * @brief Initializes the Bluetooth protocol stack.
 *
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the Bluetooth protocol stack is initialized;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int InitBtStack(void) {
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Enables the Bluetooth protocol stack.
 *
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the Bluetooth protocol stack is enabled;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int EnableBtStack(void) {
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Disables the Bluetooth protocol stack.
 *
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the Bluetooth protocol stack is disabled;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int DisableBtStack(void) {
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
int SetDeviceName(const char *name, unsigned int len) {
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
    HILOGI("advId: %{public}d, advLen: %{public}u, scanRspLen: %{public}u", advId, data.advDataLen, data.rspDataLen);

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

static bool IsAllAdvStopped()
{
    for (int i = 0; i < MAX_BLE_ADV_NUM; i++) {
        if (g_bleAdvCallbacks[i] != nullptr) {
            return false;
        }
    }
    return true;
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
    HILOGI("BleStopAdv, advId: %{public}d.", advId);
    if (advId < 0 || advId >= MAX_BLE_ADV_NUM) {
        HILOGE("BleStopAdv fail, advId is invalid.");
        return OHOS_BT_STATUS_FAIL;
    }
    lock_guard<mutex> lock(g_advMutex);
    if (g_BleAdvertiser == nullptr || g_bleAdvCallbacks[advId] == nullptr) {
        HILOGE("BleStopAdv fail, the current adv is not started.");
        return OHOS_BT_STATUS_FAIL;
    }

    g_BleAdvertiser->StopAdvertising(g_bleAdvCallbacks[advId]);

    usleep(100);
    if (g_AppCallback != nullptr && g_AppCallback->advDisableCb != nullptr) {
        HILOGI("adv stopped advId: %{public}d.", advId);
        g_AppCallback->advDisableCb(advId, 0);
    }
    g_bleAdvCallbacks[advId] = nullptr;

    if (IsAllAdvStopped()) {
        HILOGI("All adv have been stopped.");
    }
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
int BleUpdateAdv(int advId, const BleAdvParams *param) {
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
int BleSetSecurityIoCap(BleIoCapMode mode) {
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
int BleSetSecurityAuthReq(BleAuthReqMode mode) {
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
int BleGattSecurityRsp(BdAddr bdAddr, bool accept) {
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
int ReadBtMacAddr(unsigned char *mac, unsigned int len) {
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
    HILOGI("BleStopScan enter scannerId: %{public}d", scannerId);
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
 *
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

static uint64_t GetBootMillis()
{
    struct timespec ts = {};
    clock_gettime(CLOCK_BOOTTIME, &ts);
    return ts.tv_sec * MS_PER_SECOND + ts.tv_nsec / NS_PER_MS;
}

static void RemoveTimeoutAdvAddr()
{
    uint64_t now = GetBootMillis();
    while (!g_advTimeQueue.empty() && now >= g_advTimeQueue.front().second + ADV_ADDR_TIMEOUT) {
        g_advAddrs.erase(g_advTimeQueue.front().first);
        g_advTimeQueue.pop();
    }
}

static bool IsAdvAddrSetFull()
{
    if (g_advTimeQueue.size() >= MAX_ADV_ADDR_MAP_SIZE) {
        if (GetBootMillis() >= g_advTimeQueue.front().second + ADV_ADDR_TIME_THRESHOLD) {
            g_advAddrs.erase(g_advTimeQueue.front().first);
            g_advTimeQueue.pop();
            HILOGI("remove oldest adv addr");
        } else {
            HILOGI("reached the max num of advs in 15 minutes");
            return true;
        }
    }
    return false;
}

static bool IsAddrValid(const AdvOwnAddrParams *ownAddrParams)
{
    if (ownAddrParams != nullptr && ((ownAddrParams->addr[0] & 0xC0) ^ 0x40) == 0) {
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
 * @param ownAddrParams Indicates the own address and own address type. For details, see {@link AdvOwnAddrParams}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the operation is successful;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleStartAdvWithAddr(int *advId, const StartAdvRawData *rawData, const BleAdvParams *advParam,
    const AdvOwnAddrParams *ownAddrParams)
{
    HILOGI("BleStartAdvWithAddr enter");
    if (rawData == nullptr || advParam == nullptr || !IsAddrValid(ownAddrParams)) {
        HILOGW("params are invalid");
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    lock_guard<mutex> lock(g_advMutex);
    int i = 0;
    for (; i < MAX_BLE_ADV_NUM; i++) {
        if (g_bleAdvCallbacks[i] == nullptr) {
            g_bleAdvCallbacks[i] = make_shared<BleAdvCallback>(i);
            break;
        }
    }

    if (i == MAX_BLE_ADV_NUM) {
        HILOGW("reach the max num of adv");
        return OHOS_BT_STATUS_UNHANDLED;
    }

    RemoveTimeoutAdvAddr();
    string addrStr(reinterpret_cast<const char*>(ownAddrParams->addr), sizeof(ownAddrParams->addr));
    if (g_advAddrs.find(addrStr) == g_advAddrs.end() && !IsAdvAddrSetFull()) {
        g_advTimeQueue.push(pair<string, uint64_t>(addrStr, GetBootMillis()));
        g_advAddrs.insert(addrStr);
    } else {
        HILOGW("already has the same adv addr");
        g_bleAdvCallbacks[i] = nullptr;
        return OHOS_BT_STATUS_UNHANDLED;
    }

    *advId = i;
    HILOGI("ret advId: %{public}d.", *advId);

    BleAdvertiserSettings settings;
    settings.SetInterval(advParam->minInterval);
    if (advParam->advType == OHOS_BLE_ADV_SCAN_IND ||
        advParam->advType == OHOS_BLE_ADV_NONCONN_IND) {
        settings.SetConnectable(false);
    }
    if (ownAddrParams != nullptr) {
        settings.SetOwnAddr(ownAddrParams->addr);
        settings.SetOwnAddrType(ownAddrParams->addrType);
    }

    auto advData = ConvertDataToVec(rawData->advData, rawData->advDataLen);
    auto scanResponse = ConvertDataToVec(rawData->rspData, rawData->rspDataLen);
    if (g_BleAdvertiser == nullptr) {
        g_BleAdvertiser = make_shared<BleAdvertiser>();
    }
    g_BleAdvertiser->StartAdvertising(settings, advData, scanResponse, 0, g_bleAdvCallbacks[i]);
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
    HILOGI("BleStartAdvEx enter");
    lock_guard<mutex> lock(g_advMutex);
    if (g_BleAdvertiser == nullptr) {
        g_BleAdvertiser = std::make_shared<BleAdvertiser>();
    }
    int i = 0;
    for (i = 0; i < MAX_BLE_ADV_NUM; i++) {
        if (g_bleAdvCallbacks[i] == nullptr) {
            g_bleAdvCallbacks[i] = std::make_shared<BleAdvCallback>(i);
            break;
        }
    }

    if (i == MAX_BLE_ADV_NUM) {
        return OHOS_BT_STATUS_UNHANDLED;
    }

    *advId = i;
    HILOGI("ret advId: %{public}d.", *advId);

    BleAdvertiserSettings settings;
    settings.SetInterval(advParam.minInterval);
    if (advParam.advType == OHOS_BLE_ADV_SCAN_IND ||
        advParam.advType == OHOS_BLE_ADV_NONCONN_IND) {
        settings.SetConnectable(false);
    }

    auto advData = ConvertDataToVec(rawData.advData, rawData.advDataLen);
    auto scanResponse = ConvertDataToVec(rawData.rspData, rawData.rspDataLen);
    g_BleAdvertiser->StartAdvertising(settings, advData, scanResponse, 0, g_bleAdvCallbacks[i]);
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

    g_BleAdvertiser->EnableAdvertising(g_bleAdvCallbacks[advId]->GetAdvHandle(), 0, g_bleAdvCallbacks[advId]);

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

    g_BleAdvertiser->DisableAdvertising(g_bleAdvCallbacks[advId]->GetAdvHandle(), g_bleAdvCallbacks[advId]);

    return OHOS_BT_STATUS_SUCCESS;
}

static bool SetServiceUuidParameter(BleScanFilter &scanFilter, BleScanNativeFilter *nativeScanFilter)
{
    HILOGD("SetServiceUuidParameter enter");
    if (nativeScanFilter->serviceUuidLength != 0 && nativeScanFilter->serviceUuid != nullptr) {
        if (!regex_match(std::string(reinterpret_cast<char *>(nativeScanFilter->serviceUuid)), uuidRegex)) {
            HILOGE("match the UUID faild.");
            return false;
        }
        UUID serviceUuid = UUID::FromString((char *)nativeScanFilter->serviceUuid);
        scanFilter.SetServiceUuid(serviceUuid);
        if (nativeScanFilter->serviceUuidMask != nullptr) {
            if (!regex_match(std::string(reinterpret_cast<char *>(nativeScanFilter->serviceUuidMask)), uuidRegex)) {
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
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Sets scan filter configs.
 *
 * @param scannerId Indicates the scanner id.
 * @param filter Indicates the pointer to the scan filter. For details, see {@link BleScanNativeFilter}.
 * @param filterSize Indicates the number of the scan filter.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if set scan filter configs success;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
static int SetConfigScanFilter(int32_t scannerId, const BleScanNativeFilter *filter, uint32_t filterSize)
{
    HILOGD("SetConfigScanFilter enter");
    vector<BleScanFilter> scanFilters;
    for (uint32_t i = 0; i < filterSize; i++) {
        BleScanNativeFilter nativeScanFilter = filter[i];
        BleScanFilter scanFilter;
        int result = SetOneScanFilter(scanFilter, &nativeScanFilter);
        if (result != OHOS_BT_STATUS_SUCCESS) {
            HILOGE("SetOneScanFilter faild, result: %{public}d", result);
            return OHOS_BT_STATUS_PARM_INVALID;
        }
        scanFilters.push_back(scanFilter);
    }
    std::shared_ptr<BleCentralManager> bleCentralManager = g_bleCentralManagerMap.GetObject(scannerId);
    if (bleCentralManager == nullptr) {
        HILOGE("SetConfigScanFilter fail, ble centra manager is null.");
        return OHOS_BT_STATUS_FAIL;
    }
    bleCentralManager->ConfigScanFilter(scanFilters);
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
    HILOGD("BleStartScanEx enter, scannerId: %{public}d, filterSize %{public}u", scannerId, filterSize);
    if (configs == nullptr) {
        HILOGE("BleStartScanEx fail, configs is null.");
        return OHOS_BT_STATUS_FAIL;
    }
    std::shared_ptr<BleCentralManager> bleCentralManager = g_bleCentralManagerMap.GetObject(scannerId);
    if (bleCentralManager == nullptr) {
        HILOGE("BleStartScanEx fail, scannerId is invalid.");
        return OHOS_BT_STATUS_FAIL;
    }

    if (filter != nullptr && filterSize != 0) {
        int result = SetConfigScanFilter(scannerId, filter, filterSize);
        if (result != OHOS_BT_STATUS_SUCCESS) {
            HILOGE("SetConfigScanFilter faild, result: %{public}d", result);
            return OHOS_BT_STATUS_PARM_INVALID;
        }
    }

    HILOGI("scanMode: %{public}d", configs->scanMode);
    BleScanSettings settings;
    settings.SetScanMode(configs->scanMode);
    bleCentralManager->StartScan(settings);
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
    if (bleCentralManager->SetLpDeviceAdvParam(duration, maxExtAdvEvents, window, interval, advHandle) !=
        BT_NO_ERROR) {
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
    HILOGI("SetScanReportChannelToLpDevice enter. scannerId: %{public}d, isToAp: %{public}d", scannerId, enable);
    std::shared_ptr<BleCentralManager> bleCentralManager = g_bleCentralManagerMap.GetObject(scannerId);
    if (bleCentralManager == nullptr) {
        HILOGE("SetScanReportChannelToLpDevice fail, ble centra manager is null.");
        return OHOS_BT_STATUS_FAIL;
    }
    if (bleCentralManager->SetScanReportChannelToLpDevice(enable) != BT_NO_ERROR) {
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
    if (bleCentralManager->EnableSyncDataToLpDevice() != BT_NO_ERROR) {
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
    HILOGI("DisableSyncDataToLpDevice enter");
    std::shared_ptr<BleCentralManager> bleCentralManager = nullptr;
    GetBleCentralManagerObject(bleCentralManager);
    if (bleCentralManager == nullptr) {
        HILOGE("get BleCentralManager fail.");
        return OHOS_BT_STATUS_FAIL;
    }
    if (bleCentralManager->DisableSyncDataToLpDevice() != BT_NO_ERROR) {
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
    if (bleCentralManager->SendParamsToLpDevice(std::move(dataValue), type) != BT_NO_ERROR) {
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
        return OHOS_BT_STATUS_FAIL;
    }

    return bleCentralManager->IsLpDeviceAvailable();
}

void BleScanNativeFilterLog(BleScanNativeFilter &filter)
{
    if (filter.address != nullptr) {
        HILOGI("address: %{public}s", filter.address);
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
    if (!regex_match(strUuid, uuidRegex)) {
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
    if (bleCentralManager->SetLpDeviceParam(paramSet) != BT_NO_ERROR) {
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
    if (bleCentralManager->RemoveLpDeviceParam(srvUuid) != BT_NO_ERROR) {
        return OHOS_BT_STATUS_FAIL;
    }
    return OHOS_BT_STATUS_SUCCESS;
}

}  // namespace Bluetooth
}  // namespace OHOS
#ifdef __cplusplus
}
#endif
/** @} */
