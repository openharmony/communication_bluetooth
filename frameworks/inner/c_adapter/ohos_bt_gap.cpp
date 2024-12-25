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
#define LOG_TAG "bt_c_adapter_gap"
#endif

#include "ohos_bt_gap.h"

#include <string.h>
#include "__config"
#include "bluetooth_def.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_utils.h"
#include "iosfwd"
#include "ohos_bt_adapter_utils.h"
#include "ohos_bt_def.h"
#include "string"

#ifdef __cplusplus
extern "C" {
#endif

using namespace std;

namespace OHOS {
namespace Bluetooth {
static BluetoothHost *g_BluetoothHost;
static BtGapCallBacks *g_GapCallback;

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
    void OnStateChanged(const int transport, const int status) override
    {
        int cvtTransport = OHOS_BT_TRANSPORT_LE;
        if (transport == BTTransport::ADAPTER_BREDR) {
            cvtTransport = OHOS_BT_TRANSPORT_BR_EDR;
        }
        HILOGI("transport: %{public}d, status: %{public}d", cvtTransport, status);
        if (g_GapCallback != nullptr && g_GapCallback->stateChangeCallback != nullptr) {
            g_GapCallback->stateChangeCallback(cvtTransport, status);
        } else {
            HILOGW("callback func is null!");
        }
    }

    /**
     * @brief Discovery state changed observer.
     *
     * @param status Device discovery status.
     * @since 6
     */
    void OnDiscoveryStateChanged(int status) override
    {
        return;
    }

    /**
     * @brief Discovery result observer.
     *
     * @param device Remote device.
     * @param rssi Rssi of device.
     * @param deviceName Name of device.
     * @param deviceClass Class of device.
     * @since 6
     */
    void OnDiscoveryResult(
        const BluetoothRemoteDevice &device, int rssi, const std::string deviceName, int deviceClass) override
    {
        return;
    }

    /**
     * @brief Pair request observer.
     *
     * @param device Remote device.
     * @since 6
     */
    void OnPairRequested(const BluetoothRemoteDevice &device) override
    {
        return;
    }

    /**
     * @brief Pair confirmed observer.
     *
     * @param device Remote device.
     * @param reqType Pair type.
     * @param number Paired passkey.
     * @since 6
     */
    void OnPairConfirmed(const BluetoothRemoteDevice &device, int reqType, int number) override
    {
        return;
    }

    /**
     * @brief Scan mode changed observer.
     *
     * @param mode Device scan mode.
     * @since 6
     */
    void OnScanModeChanged(int mode) override
    {
        HILOGI("mode: %{public}d", mode);
        if (g_GapCallback != nullptr && g_GapCallback->scanModeChangedCallback != nullptr) {
            g_GapCallback->scanModeChangedCallback(mode);
        } else {
            HILOGW("mode: %{public}d, but callback is null!", mode);
        }
    };

    /**
     * @brief Device name changed observer.
     *
     * @param deviceName Device name.
     * @since 6
     */
    void OnDeviceNameChanged(const std::string &deviceName) override
    {
        return;
    }

    /**
     * @brief Device address changed observer.
     *
     * @param address Device address.
     * @since 6
     */
    void OnDeviceAddrChanged(const std::string &address) override
    {
        return;
    }
};

class BluetoothRemoteDeviceObserverWapper : public BluetoothRemoteDeviceObserver {
public:
    /**
     * @brief Acl state changed observer.
     *
     * @param device Remote device.
     * @param state Remote device acl state.
     * @param reason Remote device reason.
     * @since 6
     */
    void OnAclStateChanged(const BluetoothRemoteDevice &device, int state, unsigned int reason) override
    {
        if (g_GapCallback == nullptr || g_GapCallback->aclStateChangedCallbak == nullptr) {
            HILOGW("callback func is null!");
            return;
        }
        std::string stateStr;
        GetAclStateName(device.GetTransportType(), state, stateStr);
        HILOGD("device: %{public}s, state: %{public}s, reason: %{public}u",
            GetEncryptAddr(device.GetDeviceAddr()).c_str(), stateStr.c_str(), reason);
        BdAddr remoteAddr;
        GetAddrFromString(device.GetDeviceAddr(), remoteAddr.addr);
        g_GapCallback->aclStateChangedCallbak(&remoteAddr, ConvertAclState(device.GetTransportType(), state), reason);
    }

    /**
     * @brief Pair status changed observer.
     *
     * @param device Remote device.
     * @param status Remote device pair status.
     * @param cause Pair fail cause.
     * @since 12
     */
    void OnPairStatusChanged(const BluetoothRemoteDevice &device, int status, int cause) override
    {
        return;
    }

    /**
     * @brief Remote uuid changed observer.
     *
     * @param device Remote device.
     * @param uuids Remote device uuids.
     * @since 6
     */
    void OnRemoteUuidChanged(const BluetoothRemoteDevice &device, const std::vector<ParcelUuid> &uuids) override
    {
        return;
    }

    /**
     * @brief Remote name changed observer.
     *
     * @param device Remote device.
     * @param deviceName Remote device name.
     * @since 6
     */
    void OnRemoteNameChanged(const BluetoothRemoteDevice &device, const std::string &deviceName) override
    {
        return;
    }

    /**
     * @brief Remote alias changed observer.
     *
     * @param device Remote device.
     * @param alias Remote device alias.
     * @since 6
     */
    void OnRemoteAliasChanged(const BluetoothRemoteDevice &device, const std::string &alias) override
    {
        return;
    }

    /**
     * @brief Remote cod changed observer.
     *
     * @param device Remote device.
     * @param cod Remote device cod.
     * @since 6
     */
    void OnRemoteCodChanged(const BluetoothRemoteDevice &device, const BluetoothDeviceClass &cod) override
    {
        return;
    }

    /**
     * @brief Remote battery level changed observer.
     *
     * @param device Remote device.
     * @param cod Remote device battery Level.
     * @since 6
     */
    void OnRemoteBatteryLevelChanged(const BluetoothRemoteDevice &device, int batteryLevel) override
    {
        return;
    }

    /**
     * @brief Remote rssi event observer.
     *
     * @param device Remote device.
     * @param rssi Remote device rssi.
     * @param status Read status.
     * @since 6
     */
    void OnReadRemoteRssiEvent(const BluetoothRemoteDevice &device, int rssi, int status) override
    {
        return;
    }
};

static std::shared_ptr<BluetoothHostObserverWapper> g_hostObserver = nullptr;
static std::shared_ptr<BluetoothRemoteDeviceObserverWapper> g_remoteDeviceObserver = nullptr;

bool EnableBle(void)
{
    HILOGI("enter");
    if (g_BluetoothHost == nullptr) {
        g_BluetoothHost = &BluetoothHost::GetDefaultHost();
    }

    if (g_BluetoothHost->IsBleEnabled()) {
        HILOGI("ble enabled already");
        return true;
    }
    bool  isEnabled = false;
    int32_t ret = g_BluetoothHost->EnableBle();
    HILOGI("result: %{public}d", ret);
    if (ret == BT_NO_ERROR) {
        isEnabled = true;
    }
    return isEnabled;
}

bool DisableBle(void)
{
    HILOGI("enter");
    if (g_BluetoothHost == nullptr) {
        g_BluetoothHost = &BluetoothHost::GetDefaultHost();
    }

    if (!g_BluetoothHost->IsBleEnabled()) {
        HILOGI("ble disabled already");
        return true;
    }
    bool  isEnabled = false;
    int ret = g_BluetoothHost->DisableBle();
    HILOGI("result: %{public}d", ret);
    if (ret == BT_NO_ERROR) {
        isEnabled = true;
    }
    return isEnabled;
}

bool EnableBt(void)
{
    HILOGI("enter");
    if (g_BluetoothHost == nullptr) {
        g_BluetoothHost = &BluetoothHost::GetDefaultHost();
    }

    int state;
    g_BluetoothHost->GetBtState(state);
    if (state == BTStateID::STATE_TURNING_ON ||
        state == BTStateID::STATE_TURN_ON) {
        HILOGI("br state is %{public}d", state);
        return true;
    }
    bool  isEnabled = false;
    int ret = g_BluetoothHost->EnableBt();
    HILOGI("result: %{public}d", ret);
    if (ret == BT_NO_ERROR) {
        isEnabled = true;
    }
    return isEnabled;
}

bool DisableBt(void)
{
    HILOGI("enter");
    if (g_BluetoothHost == nullptr) {
        g_BluetoothHost = &BluetoothHost::GetDefaultHost();
    }

    int state;
    g_BluetoothHost->GetBtState(state);
    if (state == BTStateID::STATE_TURNING_OFF ||
        state == BTStateID::STATE_TURN_OFF) {
        HILOGI("br state is %{public}d", state);
        return true;
    }
    bool  isDisabled = false;
    int ret = g_BluetoothHost->DisableBt();
    HILOGI("result: %{public}d", ret);
    if (ret == BT_NO_ERROR) {
        isDisabled = true;
    }
    return isDisabled;
}

int GetBtState()
{
    HILOGI("enter");
    if (g_BluetoothHost == nullptr) {
        g_BluetoothHost = &BluetoothHost::GetDefaultHost();
    }

    int state;
    g_BluetoothHost->GetBtState(state);
    HILOGI("br state: %{public}d", state);
    return state;
}

bool IsBleEnabled()
{
    if (g_BluetoothHost == nullptr) {
        g_BluetoothHost = &BluetoothHost::GetDefaultHost();
    }

    bool ret = g_BluetoothHost->IsBleEnabled();
    HILOGD("ble enable: %{public}d", ret);
    return ret;
}

bool GetLocalAddr(unsigned char *mac, unsigned int len)
{
    HILOGD("enter");
    if (mac == nullptr || len < OHOS_BD_ADDR_LEN) {
        HILOGE("invalid param, len:%{public}d", len);
        return false;
    }
    if (g_BluetoothHost == nullptr) {
        g_BluetoothHost = &BluetoothHost::GetDefaultHost();
    }

    std::string localAddress = INVALID_MAC_ADDRESS;
    int32_t err = g_BluetoothHost->GetLocalAddress(localAddress);
    if (err != BT_NO_ERROR) {
        HILOGE("error %{public}d", err);
        return false;
    }
    GetAddrFromString(localAddress, mac);
    HILOGI("device: %{public}s", GetEncryptAddr(localAddress).c_str());
    return true;
}

bool SetLocalName(unsigned char *localName, unsigned char length)
{
    HILOGI("enter");
    if (localName == nullptr) {
        HILOGE("localName is null");
        return false;
    }

    if (g_BluetoothHost == nullptr) {
        g_BluetoothHost = &BluetoothHost::GetDefaultHost();
    }

    string newName(reinterpret_cast<const char *>(localName));
    bool isSuccess = false;
    int ret = g_BluetoothHost->SetLocalName(newName);
    if (ret == BT_NO_ERROR) {
        isSuccess = true;
    }
    HILOGI("result %{public}d: LocalName : %{public}s", ret, g_BluetoothHost->GetLocalName().c_str());
    return isSuccess;
}

bool SetBtScanMode(int mode, int duration)
{
    HILOGI("mode: %{public}d, duration: %{public}d", mode, duration);
    if (g_BluetoothHost == nullptr) {
        g_BluetoothHost = &BluetoothHost::GetDefaultHost();
    }
    bool isSuccess = false;
    int ret = g_BluetoothHost->SetBtScanMode(mode, duration);
    if (ret == BT_NO_ERROR) {
        isSuccess = true;
    }
    g_BluetoothHost->SetBondableMode(BT_TRANSPORT_BREDR, BONDABLE_MODE_ON);
    return isSuccess;
}

bool PairRequestReply(const BdAddr *bdAddr, int transport, bool accept)
{
    string strAddress;
    ConvertAddr(bdAddr->addr, strAddress);
    HILOGI("device: %{public}s", GetEncryptAddr(strAddress).c_str());
    BluetoothRemoteDevice remoteDevice;
    if (transport == OHOS_BT_TRANSPORT_BR_EDR) {
        remoteDevice = g_BluetoothHost->GetRemoteDevice(strAddress, BT_TRANSPORT_BREDR);
    } else if (transport == OHOS_BT_TRANSPORT_LE) {
        remoteDevice = g_BluetoothHost->GetRemoteDevice(strAddress, BT_TRANSPORT_BLE);
    } else {
        HILOGE("transport: %{public}d is invalid", transport);
        return false;
    }
    bool ret = remoteDevice.PairRequestReply(accept);
    HILOGI("transport: %{public}d, accept: %{public}d, ret: %{public}d", transport, accept, ret);
    return ret;
}

bool SetDevicePairingConfirmation(const BdAddr *bdAddr, int transport, bool accept)
{
    string strAddress;
    ConvertAddr(bdAddr->addr, strAddress);
    HILOGI("device: %{public}s, accept: %{public}d", GetEncryptAddr(strAddress).c_str(), accept);
    BluetoothRemoteDevice remoteDevice;
    if (transport == OHOS_BT_TRANSPORT_BR_EDR) {
        remoteDevice = g_BluetoothHost->GetRemoteDevice(strAddress, BT_TRANSPORT_BREDR);
    } else if (transport == OHOS_BT_TRANSPORT_LE) {
        remoteDevice = g_BluetoothHost->GetRemoteDevice(strAddress, BT_TRANSPORT_BLE);
    } else {
        HILOGE("transport: %{public}d is invalid", transport);
        return false;
    }
    bool isSuccess = false;
    int ret = remoteDevice.SetDevicePairingConfirmation(accept);
    HILOGI("ret: %{public}d", ret);
    if (ret == BT_NO_ERROR) {
        isSuccess = true;
    }
    return isSuccess;
}

/**
 * explain: This function does not support dynamic registration;
 */
int GapRegisterCallbacks(BtGapCallBacks *func)
{
    HILOGI("enter");
    if (func == nullptr) {
        HILOGE("func is null.");
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    if (g_BluetoothHost == nullptr) {
        g_BluetoothHost = &BluetoothHost::GetDefaultHost();
    }
    g_GapCallback = func;
    g_hostObserver = std::make_shared<BluetoothHostObserverWapper>();
    g_remoteDeviceObserver = std::make_shared<BluetoothRemoteDeviceObserverWapper>();
    g_BluetoothHost->RegisterObserver(g_hostObserver);
    g_BluetoothHost->RegisterRemoteDeviceObserver(g_remoteDeviceObserver);
    return OHOS_BT_STATUS_SUCCESS;
}

bool SetFastScan(bool isEnable)
{
    HILOGI("isEnable: %{public}d", isEnable);
    if (g_BluetoothHost == nullptr) {
        g_BluetoothHost = &BluetoothHost::GetDefaultHost();
    }
    bool isSuccess = false;
    int ret = g_BluetoothHost->SetFastScan(isEnable);
    if (ret == BT_NO_ERROR) {
        isSuccess = true;
    }

    return isSuccess;
}
}  // namespace Bluetooth
}  // namespace OHOS
#ifdef __cplusplus
}
#endif
