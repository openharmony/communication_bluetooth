/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "ohos_bt_gap.h"

#include <iostream>
#include <string.h>
#include <vector>

#include "bluetooth_host.h"
#include "bluetooth_def.h"
#include "bluetooth_log.h"

#ifdef __cplusplus
extern "C" {
#endif

using namespace std;

namespace OHOS {
namespace Bluetooth {
static BluetoothHost *g_BluetoothHost;
static BtGapCallBacks *g_GapCallback;

extern void GetAddrFromString(std::string in, unsigned char out[6]);
extern void GetAddrFromByte(unsigned char in[6], std::string &out);

class BluetoothHostObserverWapper : public BluetoothHostObserver {
public:
    /**
     * @brief Adapter state change function.
     *
     * @param transport Transport type when state change.
     *        BTTransport::ADAPTER_BREDR : classic;
     *        BTTransport::ADAPTER_BLE : ble.
     * @param state Change to the new state.
     *        BTStateID::STATE_TURNING_ON;
     *        BTStateID::STATE_TURN_ON;
     *        BTStateID::STATE_TURNING_OFF;
     *        BTStateID::STATE_TURN_OFF.
     * @since 6
     */
    void OnStateChanged(const int transport, const int status) {
        int cvtTransport = OHOS_BT_TRANSPORT_LE;
        if (transport == BTTransport::ADAPTER_BREDR) {
            cvtTransport = OHOS_BT_TRANSPORT_BR_EDR;
        }
        if (g_GapCallback != NULL && g_GapCallback->stateChangeCallback != NULL) {
            HILOGI("transport: %{public}d, status: %{public}d", cvtTransport, status);
            g_GapCallback->stateChangeCallback(cvtTransport, status);
        } else {
            HILOGW("transport: %{public}d, status: %{public}d, but callback is null!", cvtTransport, status);
        }
    }

    /**
     * @brief Discovery state changed observer.
     *
     * @param status Device discovery status.
     * @since 6
     */
    void OnDiscoveryStateChanged(int status) {}

    /**
     * @brief Discovery result observer.
     *
     * @param device Remote device.
     * @since 6
     */
    void OnDiscoveryResult(const BluetoothRemoteDevice &device) {}

    /**
     * @brief Pair request observer.
     *
     * @param device Remote device.
     * @since 6
     */
    void OnPairRequested(const BluetoothRemoteDevice &device) {}

    /**
     * @brief Pair confirmed observer.
     *
     * @param device Remote device.
     * @param reqType Pair type.
     * @param number Paired passkey.
     * @since 6
     */
    void OnPairConfirmed(const BluetoothRemoteDevice &device, int reqType, int number) {};

    /**
     * @brief Scan mode changed observer.
     *
     * @param mode Device scan mode.
     * @since 6
     */
    void OnScanModeChanged(int mode) {};

    /**
     * @brief Device name changed observer.
     *
     * @param deviceName Device name.
     * @since 6
     */
    void OnDeviceNameChanged(const std::string &deviceName) {};

    /**
     * @brief Device address changed observer.
     *
     * @param address Device address.
     * @since 6
     */
    void OnDeviceAddrChanged(const std::string &address) {};
};

static BluetoothHostObserverWapper g_HostObserver;

bool EnableBle(void)
{
    if (g_BluetoothHost == NULL) {
        g_BluetoothHost = &BluetoothHost::GetDefaultHost();
    }

    if (g_BluetoothHost->IsBleEnabled()) {
        HILOGI("ble enabled already");
        return true;
    }
    
    bool ret = g_BluetoothHost->EnableBle();
    HILOGI("result: %{public}d", ret);
    return ret;
}

bool DisableBle(void)
{
    if (g_BluetoothHost == NULL) {
        g_BluetoothHost = &BluetoothHost::GetDefaultHost();
    }

    if (!g_BluetoothHost->IsBleEnabled()) {
        HILOGI("ble disabled already");
        return true;
    }

    bool ret = g_BluetoothHost->DisableBle();
    HILOGI("result: %{public}d", ret);
    return ret;
}

bool EnableBt(void)
{
    if (g_BluetoothHost == NULL) {
        g_BluetoothHost = &BluetoothHost::GetDefaultHost();
    }

    int state = g_BluetoothHost->GetBtState();
    if (state == BTStateID::STATE_TURNING_ON ||
        state == BTStateID::STATE_TURN_ON) {
        HILOGI("br state is %{public}d", state);
        return true;
    }
    bool ret = g_BluetoothHost->EnableBt();
    HILOGI("result: %{public}d", ret);
    return ret;
}

bool DisableBt(void)
{
    if (g_BluetoothHost == NULL) {
        g_BluetoothHost = &BluetoothHost::GetDefaultHost();
    }

    int state = g_BluetoothHost->GetBtState();
    if (state == BTStateID::STATE_TURNING_OFF ||
        state == BTStateID::STATE_TURN_OFF) {
        HILOGI("br state is %{public}d", state);
        return true;
    }
    bool ret = g_BluetoothHost->DisableBt();
    HILOGI("result: %{public}d", ret);
    return ret;
}

int GetBtState()
{
    if (g_BluetoothHost == NULL) {
        g_BluetoothHost = &BluetoothHost::GetDefaultHost();
    }

    int state = g_BluetoothHost->GetBtState();
    HILOGI("br state: %{public}d", state);
    return state;
}

bool IsBleEnabled()
{
    if (g_BluetoothHost == NULL) {
        g_BluetoothHost = &BluetoothHost::GetDefaultHost();
    }

    bool ret = g_BluetoothHost->IsBleEnabled();
    HILOGI("ble enable: %{public}d", ret);
    return ret;
}

/**
 *  @brief Get local host bluetooth address
 *  @return @c Local host bluetooth address
 */
bool GetLocalAddr(unsigned char *mac, unsigned int len)
{
    if (mac == NULL || len < OHOS_BD_ADDR_LEN) {
        HILOGE("invalid param : mac:%{public}p, len:%{public}d", mac, len);
        return false;
    }
    if (g_BluetoothHost == NULL) {
        g_BluetoothHost = &BluetoothHost::GetDefaultHost();
    }

    string localAddress = g_BluetoothHost->GetLocalAddress();
    GetAddrFromString(localAddress, mac);
    HILOGI("adress: %02X:%02X:*:*:*:%02X", mac[0], mac[1], mac[5]);
    return true;
}

/**
 *  @brief Get local host bluetooth name
 *  @param localName actual receiving device name
           length - localName length, initail set length to zero, and call this func to set real length
 *  @return Local host bluetooth name
 */
bool GetLocalName(unsigned char *localName, unsigned char *length)
{
    return false;
}

/**
 * @brief Set local device name.
 * @param localName Device name.
          length device name length
 * @return Returns <b>true</b> if the operation is successful;
 *         returns <b>false</b> if the operation fails.
 */
bool SetLocalName(unsigned char *localName, unsigned char length)
{
    if (localName == NULL) {
        HILOGE("invalid param: localName(%{public}p)", localName);
        return false;
    }

    if (g_BluetoothHost == NULL) {
        g_BluetoothHost = &BluetoothHost::GetDefaultHost();
    }

    string newName((const char *)localName);
    bool ret = g_BluetoothHost->SetLocalName(newName);
    HILOGI("result %{public}d: LocalName : %{public}s", ret, g_BluetoothHost->GetLocalName().c_str());
    return ret;
}

/**
 * @brief Factory reset bluetooth service.
 * @return Returns <b>true</b> if the operation is successful;
 *         returns <b>false</b> if the operation fails.
 */
bool BluetoothFactoryReset(void)
{
    return false;
}

/**
 * @brief Set device scan mode.
 * @param mode Scan mode.
 * @param duration Scan time, see details {@link GapBtScanMode}
 * @return special mode
 */
int GetBtScanMode(void)
{
    return OHOS_GAP_SCAN_MODE_NONE;
}

/**
 * @brief Set device scan mode.
 * @param mode Scan mode, see details {@link GapBtScanMode}.
 * @param duration Scan time.
 * @return Returns <b>true</b> if the operation is successful;
 *         returns <b>false</b> if the operation fails.
 */
bool SetBtScanMode(int mode, int duration)
{
    return false;
}

/*
 * @brief Get paired devices.
 * @param pairList - 按照maxPairNums申请的设备列表数组
          maxPairNums - 指定需要获取的设备列表最大个数
          realPairNums - 实际的配对设备列表个数
 * @return Returns <b>true</b> if the operation is successful;
 *         returns <b>false</b> if the operation fails.
 */
bool GetPariedDevicesNum(unsigned int *number)
{
    return false;
}

bool GetPairedDevicesList(PairedDeviceInfo *devInfo, int *number)
{
    return false;
}

/**
 * @brief Get device pair state.
 * @param device Remote device.
 * @return Returns device pair state. see detail {@link GapBtPairedState}
 */
int GetPairState()
{
    return false;
}

/**
 * @brief Remove pair.
 * @param BdAddr Remote device address.
 * @return Returns <b>true</b> if the operation is successful;
 *         returns <b>false</b> if the operation fails.
 */
bool RemovePair(const BdAddr addr)
{
    return false;
}

/**
 * @brief Remove all pairs.
 * @return Returns <b>true</b> if the operation is successful;
 *         returns <b>false</b> if the operation fails.
 */
bool RemoveAllPairs(void)
{
    return false;
}

/**
 * @brief Check if device acl connected.
 * @param addr device address.
 * @return Returns <b>true</b> if device acl connected;
 *         returns <b>false</b> if device does not acl connect.
 */
bool IsAclConnected(BdAddr addr)
{
    return false;
}

/**
 * @brief disconnect remote device acl connected.
 * @param addr device address.
 * @return Returns <b>true</b> if device acl connected;
 *         returns <b>false</b> if device does not acl connect.
 */
bool DisconnectRemoteDevice(BdAddr *addr)
{
    return false;
}

/**
 * @brief 注册GAP回调函数。
 * @param func 回调函数原型，参考{@link BtGapCallBacks}。
 * @return 成功返回{@link OHOS_BT_STATUS_SUCCESS}，失败返回错误码，具体错误码含义参考{@link BtStatus}定义
 */
int GapRegisterCallbacks(BtGapCallBacks *func)
{
    HILOGI();
    if (g_BluetoothHost == NULL) {
        g_BluetoothHost = &BluetoothHost::GetDefaultHost();
    }
    g_GapCallback = func;
    g_BluetoothHost->RegisterObserver(g_HostObserver);
    return OHOS_BT_STATUS_SUCCESS;
}
}  // namespace Bluetooth
}  // namespace OHOS
#ifdef __cplusplus
}
#endif
