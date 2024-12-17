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
#define LOG_TAG "bt_cj_connection_impl"
#endif

#include "bluetooth_connection_impl.h"

#include "bluetooth_connection_common.h"
#include "bluetooth_connection_ffi.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_host.h"
#include "napi_bluetooth_utils.h"
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"

namespace OHOS {
namespace CJSystemapi {
namespace CJBluetoothConnection {
using OHOS::Bluetooth::BluetoothRemoteDevice;
using Bluetooth::BT_ERR_INVALID_PARAM;
using Bluetooth::BT_NO_ERROR;
using Bluetooth::INVALID_NAME;
using Bluetooth::MajorClass;
using Bluetooth::BluetoothHost;
using Bluetooth::BT_TRANSPORT_BREDR;
using Bluetooth::PAIR_NONE;
using Bluetooth::GetProfileConnectionState;
using Bluetooth::BTConnectState;
using Bluetooth::DeviceBatteryInfo;

void ConnectionImpl::PairDevice(std::string deviceId, int32_t* errCode)
{
    if (!IsValidAddress(deviceId)) {
        *errCode = BT_ERR_INVALID_PARAM;
        return;
    }
    BluetoothRemoteDevice remoteDevice = Bluetooth::BluetoothRemoteDevice(deviceId);
    *errCode = remoteDevice.StartPair();
    return;
}

char* ConnectionImpl::GetRemoteDeviceName(std::string deviceId, int32_t* errCode)
{
    std::string name = INVALID_NAME;
    if (!IsValidAddress(deviceId)) {
        *errCode = BT_ERR_INVALID_PARAM;
        return nullptr;
    }
    BluetoothRemoteDevice remoteDevice = Bluetooth::BluetoothRemoteDevice(deviceId);
    *errCode = remoteDevice.GetDeviceName(name);
    return MallocCString(name);
}

DeviceClass ConnectionImpl::GetRemoteDeviceClass(std::string deviceId, int32_t* errCode)
{
    DeviceClass ret{0};
    if (!IsValidAddress(deviceId)) {
        *errCode = BT_ERR_INVALID_PARAM;
        return ret;
    }
    BluetoothRemoteDevice remoteDevice = Bluetooth::BluetoothRemoteDevice(deviceId);
    int tmpCod = MajorClass::MAJOR_UNCATEGORIZED;
    int tmpMajorClass = MajorClass::MAJOR_UNCATEGORIZED;
    int tmpMajorMinorClass = MajorClass::MAJOR_UNCATEGORIZED;
    int timerId = HiviewDFX::XCollie::GetInstance().SetTimer(
        "GetDeviceProductType", 10, nullptr, nullptr, HiviewDFX::XCOLLIE_FLAG_LOG); // 10 表示超时时间为10s
    *errCode = remoteDevice.GetDeviceProductType(tmpCod, tmpMajorClass, tmpMajorMinorClass);
    HiviewDFX::XCollie::GetInstance().CancelTimer(timerId);
    ret.majorClass = tmpMajorClass;
    ret.majorMinorClass = tmpMajorMinorClass;
    ret.classOfDevice = tmpCod;
    return ret;
}

CArrString ConnectionImpl::GetRemoteProfileUuids(std::string deviceId, int32_t* errCode)
{
    CArrString ret{0};
    if (!IsValidAddress(deviceId)) {
        *errCode = BT_ERR_INVALID_PARAM;
        return ret;
    }
    std::vector<std::string> uuids{};
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(deviceId);
    *errCode = remoteDevice.GetDeviceUuids(uuids);
    ret = Convert2CArrString(uuids);
    return ret;
}

char* ConnectionImpl::GetLocalName(int32_t* errCode)
{
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    std::string localName = INVALID_NAME;
    *errCode = host->GetLocalName(localName);
    return MallocCString(localName);
}

CArrString ConnectionImpl::GetPairedDevices(int32_t* errCode)
{
    CArrString ret{0};
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    std::vector<BluetoothRemoteDevice> remoteDeviceLists;
    *errCode = host->GetPairedDevices(BT_TRANSPORT_BREDR, remoteDeviceLists);
    std::vector<std::string> deviceAddr{};
    size_t size = remoteDeviceLists.size();
    if (size == 0 ||
        size > std::numeric_limits<size_t>::max() / sizeof(char *)) {
        return ret;
    }
    ret.head = static_cast<char **>(malloc(sizeof(char *) * size));
    if (!ret.head) {
        return ret;
    }

    size_t i = 0;
    for (; i < size; ++i) {
        ret.head[i] = MallocCString(remoteDeviceLists[i].GetDeviceAddr());
    }
    ret.size = static_cast<int64_t>(i);
    return ret;
}

int32_t ConnectionImpl::GetPairState(std::string deviceId, int32_t* errCode)
{
    if (!IsValidAddress(deviceId)) {
        *errCode = BT_ERR_INVALID_PARAM;
        return PAIR_NONE;
    }
    BluetoothRemoteDevice remoteDevice = Bluetooth::BluetoothRemoteDevice(deviceId);
    int state = PAIR_NONE;
    *errCode = remoteDevice.GetPairState(state);
    int pairState = static_cast<int>(BondState::BOND_STATE_INVALID);
    DealPairStatus(state, pairState);
    return pairState;
}

int32_t ConnectionImpl::GetProfileConnectionState(int32_t profileId, int32_t* errCode)
{
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int state = static_cast<int>(BTConnectState::DISCONNECTED);
    *errCode = host->GetBtProfileConnState(Bluetooth::GetProfileId(profileId), state);
    int status = Bluetooth::GetProfileConnectionState(state);
    return status;
}

void ConnectionImpl::SetDevicePairingConfirmation(std::string deviceId, bool accept, int32_t* errCode)
{
    if (!IsValidAddress(deviceId)) {
        *errCode = BT_ERR_INVALID_PARAM;
        return;
    }
    BluetoothRemoteDevice remoteDevice = Bluetooth::BluetoothRemoteDevice(deviceId);
    if (accept) {
        *errCode = remoteDevice.SetDevicePairingConfirmation(accept);
    } else {
        *errCode = remoteDevice.CancelPairing();
    }
    return;
}

void ConnectionImpl::SetDevicePinCode(std::string deviceId, std::string code, int32_t* errCode)
{
    BluetoothRemoteDevice remoteDevice = Bluetooth::BluetoothRemoteDevice(deviceId);
    *errCode = remoteDevice.SetDevicePin(code);
    return;
}

void ConnectionImpl::SetLocalName(std::string localName, int32_t* errCode)
{
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    *errCode = host->SetLocalName(localName);
    return;
}

void ConnectionImpl::SetBluetoothScanMode(int32_t mode, int32_t duration, int32_t* errCode)
{
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    *errCode = host->SetBtScanMode(mode, duration);
    if (*errCode == BT_NO_ERROR) {
        return;
    }
    host->SetBondableMode(BT_TRANSPORT_BREDR, 1);
    return;
}

int32_t ConnectionImpl::GetBluetoothScanMode(int32_t* errCode)
{
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int32_t scanMode = 0;
    *errCode = host->GetBtScanMode(scanMode);
    return scanMode;
}

void ConnectionImpl::StartBluetoothDiscovery(int32_t* errCode)
{
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    *errCode = host->StartBtDiscovery();
    return;
}

void ConnectionImpl::StoptBluetoothDiscovery(int32_t* errCode)
{
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    *errCode = host->CancelBtDiscovery();
    return;
}

bool ConnectionImpl::IsBluetoothDiscovering(int32_t* errCode)
{
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    bool isDiscovering = false;
    *errCode = host->IsBtDiscovering(isDiscovering);
    return isDiscovering;
}

void ConnectionImpl::SetRemoteDeviceName(std::string deviceId, std::string name, int32_t* errCode)
{
    if (!IsValidAddress(deviceId)) {
        *errCode = BT_ERR_INVALID_PARAM;
        return;
    }
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(deviceId);
    *errCode = remoteDevice.SetDeviceAlias(name);
    return;
}

CBatteryInfo ConnectionImpl::GetRemoteDeviceBatteryInfo(std::string deviceId, int32_t* errCode)
{
    CBatteryInfo info{0};
    if (!IsValidAddress(deviceId)) {
        *errCode = BT_ERR_INVALID_PARAM;
        return info;
    }
    DeviceBatteryInfo batteryInfo;
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(deviceId);
    int32_t err = remoteDevice.GetRemoteDeviceBatteryInfo(batteryInfo);
    HILOGI("err: %{public}d", err);
    info.batteryLevel = batteryInfo.batteryLevel_;
    info.leftEarBatteryLevel = batteryInfo.leftEarBatteryLevel_;
    info.leftEarChargeState = static_cast<int>(batteryInfo.leftEarChargeState_);
    info.rightEarBatteryLevel = batteryInfo.rightEarBatteryLevel_;
    info.rightEarChargeState = static_cast<int>(batteryInfo.rightEarChargeState_);
    info.boxBatteryLevel = batteryInfo.boxBatteryLevel_;
    info.boxChargeState = static_cast<int>(batteryInfo.boxChargeState_);
    return info;
}

} // namespace BluetoothConnection
} // namespace CJSystemapi
} // namespace OHOS