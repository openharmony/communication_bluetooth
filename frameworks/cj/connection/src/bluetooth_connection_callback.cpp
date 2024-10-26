/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef LOG_TAG
#define LOG_TAG "bt_cj_connection_callback"
#endif

#include "bluetooth_connection_impl.h"
#include "bluetooth_connection_ffi.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_connection_common.h"
#include "bluetooth_log.h"
#include "cj_lambda.h"

namespace OHOS {
namespace CJSystemapi {
namespace CJBluetoothConnection {
using Bluetooth::BluetoothHost;
using Bluetooth::BT_ERR_INTERNAL_ERROR;

std::shared_ptr<CjBluetoothConnectionObserver> g_connectionObserver =
    std::make_shared<CjBluetoothConnectionObserver>();
std::shared_ptr<CjBluetoothRemoteDeviceObserver> g_remoteDeviceObserver =
    std::make_shared<CjBluetoothRemoteDeviceObserver>();
std::mutex deviceMutex;
bool g_flag = false;

CjBluetoothConnectionObserver::CjBluetoothConnectionObserver()
{}

void CjBluetoothConnectionObserver::OnDiscoveryResult(const BluetoothRemoteDevice &device,
    int rssi, const std::string deviceName, int deviceClass)
{
    if (deviceFindFunc == nullptr) {
        HILOGD("not register bluetoothDeviceFind event failed");
        return;
    }
    CArrString array{0};
    std::shared_ptr<BluetoothRemoteDevice> remoteDevice = std::make_shared<BluetoothRemoteDevice>(device);
    array.size = 1;
    char **retValue = static_cast<char**>(malloc(sizeof(char*) * array.size));
    if (retValue == nullptr) {
        return;
    }

    for (int i = 0; i < array.size; i++) {
        retValue[i] = MallocCString(remoteDevice->GetDeviceAddr());
    }
    array.head = retValue;

    deviceFindFunc(array);

    for (int i = 0; i < array.size; i++) {
        free(array.head[i]);
        array.head[i] = nullptr;
    }
    free(retValue);
    retValue = nullptr;
}

void CjBluetoothConnectionObserver::OnPairConfirmed(const BluetoothRemoteDevice &device, int reqType, int number)
{
    if (pinRequestFunc == nullptr) {
        HILOGD("not register pinRequired event failed");
        return;
    }
    CPinRequiredParam cPinRequiredParam{0};
    char* deviceAddr = MallocCString(device.GetDeviceAddr());
    cPinRequiredParam.deviceId = deviceAddr;

    char* pinCodeNative = MallocCString(GetFormatPinCode(reqType, number));
    cPinRequiredParam.pinCode = pinCodeNative;

    pinRequestFunc(cPinRequiredParam);

    free(deviceAddr);
    deviceAddr = nullptr;
    free(pinCodeNative);
    pinCodeNative = nullptr;
}

void CjBluetoothConnectionObserver::RegisterDeviceFindFunc(std::function<void(CArrString)> cjCallback)
{
    deviceFindFunc = cjCallback;
}

void CjBluetoothConnectionObserver::RegisterPinRequestFunc(std::function<void(CPinRequiredParam)> cjCallback)
{
    pinRequestFunc = cjCallback;
}

CjBluetoothRemoteDeviceObserver::CjBluetoothRemoteDeviceObserver()
{}

void CjBluetoothRemoteDeviceObserver::OnPairStatusChanged(const BluetoothRemoteDevice &device,
    int status, int cause)
{
    if (bondStateFunc == nullptr) {
        HILOGD("not register bondStateChange event failed");
        return;
    }
    CBondStateParam cBondStateParam{0};
    int bondStatus = 0;
    DealPairStatus(status, bondStatus);

    char* deviceAddr = MallocCString(device.GetDeviceAddr());

    cBondStateParam.deviceId = deviceAddr;
    cBondStateParam.state = bondStatus;
    cBondStateParam.cause = cause;

    bondStateFunc(cBondStateParam);

    free(deviceAddr);
    deviceAddr = nullptr;
}

void CjBluetoothRemoteDeviceObserver::OnRemoteBatteryChanged(const BluetoothRemoteDevice &device,
    const DeviceBatteryInfo &batteryInfo)
{
    if (batteryChangeFunc == nullptr) {
        HILOGD("not register batteryChange event failed");
        return;
    }
    CBatteryInfo cBatteryInfo{0};
    cBatteryInfo.batteryLevel = batteryInfo.batteryLevel_;
    cBatteryInfo.leftEarBatteryLevel = batteryInfo.leftEarBatteryLevel_;
    cBatteryInfo.leftEarChargeState = static_cast<int32_t>(batteryInfo.leftEarChargeState_);
    cBatteryInfo.rightEarBatteryLevel = batteryInfo.rightEarBatteryLevel_;
    cBatteryInfo.rightEarChargeState = static_cast<int32_t>(batteryInfo.rightEarChargeState_);
    cBatteryInfo.boxBatteryLevel = batteryInfo.boxBatteryLevel_;
    cBatteryInfo.boxChargeState = static_cast<int32_t>(batteryInfo.boxChargeState_);

    batteryChangeFunc(cBatteryInfo);
}

void CjBluetoothRemoteDeviceObserver::RegisterBondStateFunc(std::function<void(CBondStateParam)> cjCallback)
{
    bondStateFunc = cjCallback;
}

void CjBluetoothRemoteDeviceObserver::RegisterBatteryChangeFunc(std::function<void(CBatteryInfo)> cjCallback)
{
    batteryChangeFunc = cjCallback;
}

static void RegisterObserverToHost()
{
    BluetoothHost &host = BluetoothHost::GetDefaultHost();
    host.RegisterObserver(g_connectionObserver);
    host.RegisterRemoteDeviceObserver(g_remoteDeviceObserver);
}

void ConnectionImpl::RegisterConnectionObserver(int32_t callbackType, void (*callback)(), int32_t* errCode)
{
    if (!g_flag) {
        RegisterObserverToHost();
        g_flag = true;
    }

    if (callbackType == REGISTER_DEVICE_FIND_TYPE) {
        auto connectionObserverFunc = CJLambda::Create(reinterpret_cast<void (*)(CArrString)>(callback));
        if (!connectionObserverFunc) {
            HILOGD("Register bluetoothDeviceFind event failed");
            *errCode = BT_ERR_INTERNAL_ERROR;
            return;
        }
        g_connectionObserver->RegisterDeviceFindFunc(connectionObserverFunc);
    }

    if (callbackType == REGISTER_PIN_REQUEST_TYPE) {
        auto connectionObserverFunc = CJLambda::Create(reinterpret_cast<void (*)(CPinRequiredParam)>(callback));
        if (!connectionObserverFunc) {
            HILOGD("Register pinRequired event failed");
            *errCode = BT_ERR_INTERNAL_ERROR;
            return;
        }
        g_connectionObserver->RegisterPinRequestFunc(connectionObserverFunc);
    }

    if (callbackType == REGISTER_BOND_STATE_TYPE) {
        auto remoteDeviceObserverFunc = CJLambda::Create(reinterpret_cast<void (*)(CBondStateParam)>(callback));
        if (!remoteDeviceObserverFunc) {
            HILOGD("Register bondStateChange event failed");
            *errCode = BT_ERR_INTERNAL_ERROR;
            return;
        }
        g_remoteDeviceObserver->RegisterBondStateFunc(remoteDeviceObserverFunc);
    }

    if (callbackType == REGISTER_BATTERY_CHANGE_TYPE) {
        auto remoteDeviceObserverFunc = CJLambda::Create(reinterpret_cast<void (*)(CBatteryInfo)>(callback));
        if (!remoteDeviceObserverFunc) {
            HILOGD("Register batteryChange event failed");
            *errCode = BT_ERR_INTERNAL_ERROR;
            return;
        }
        g_remoteDeviceObserver->RegisterBatteryChangeFunc(remoteDeviceObserverFunc);
    }
}
} // namespace BluetoothConnection
} // namespace CJSystemapi
} // namespace OHOS