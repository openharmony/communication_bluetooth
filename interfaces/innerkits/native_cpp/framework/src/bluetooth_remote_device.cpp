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

#include <string>

#include "bluetooth_raw_address.h"
#include "bluetooth_def.h"
#include "bluetooth_host.h"
#include "bluetooth_host_proxy.h"
#include "bluetooth_log.h"
#include "bluetooth_remote_device.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Bluetooth {

sptr<BluetoothHostProxy> GetHostProxy()
{

    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> remote = samgr->GetSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);

    sptr<BluetoothHostProxy> hostProxy = new BluetoothHostProxy(remote);
    return hostProxy;
}

BluetoothRemoteDevice::BluetoothRemoteDevice(const std::string &addr, const int transport)
{
    address_ = addr;
    transport_ = transport;
}

int BluetoothRemoteDevice::GetDeviceType() const
{
    int type = 0;
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGI("Invalid remote device.");
        return type;
    }

    sptr<BluetoothHostProxy> proxy = GetHostProxy();
    if (proxy == nullptr) {
        HILOGI("BT host server not created.");
        return type;
    }

    return proxy->GetDeviceType(transport_, address_);
}

bool BluetoothRemoteDevice::IsValidBluetoothRemoteDevice() const
{
    if (!BluetoothHost::IsValidBluetoothAddr(address_)) {
        HILOGI("invalid bluetooth addr.");
        return false;
    }

    if ((BT_TRANSPORT_BREDR != transport_) && (BT_TRANSPORT_BLE != transport_)) {
        HILOGI("invalid transport type.");
        return false;
    }
    return true;
}
int BluetoothRemoteDevice::GetTransportType() const
{
    return transport_;
}

int BluetoothRemoteDevice::GetPhonebookPermission() const
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return INVALID_VALUE;
    }
    return GetHostProxy()->GetPhonebookPermission(address_);
}

bool BluetoothRemoteDevice::SetPhonebookPermission(int permission)
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return false;
    }
    return GetHostProxy()->SetPhonebookPermission(address_, permission);
}

int BluetoothRemoteDevice::GetMessagePermission() const
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return INVALID_VALUE;
    }
    return GetHostProxy()->GetMessagePermission(address_);
}

bool BluetoothRemoteDevice::SetMessagePermission(int permission)
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return false;
    }
    return GetHostProxy()->SetMessagePermission(address_, permission);
}

int BluetoothRemoteDevice::GetPowerMode(void) const
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return INVALID_VALUE;
    }
    return GetHostProxy()->GetPowerMode(address_);
}

std::string BluetoothRemoteDevice::GetDeviceName() const
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return INVALID_NAME;
    }
    return GetHostProxy()->GetDeviceName(transport_, address_);
}

std::string BluetoothRemoteDevice::GetDeviceAlias() const
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return INVALID_NAME;
    }
    return GetHostProxy()->GetDeviceAlias(address_);
}

bool BluetoothRemoteDevice::SetDeviceAlias(const std::string &aliasName)
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return false;
    }
    return GetHostProxy()->SetDeviceAlias(address_, aliasName);
}

int BluetoothRemoteDevice::GetDeviceBatteryLevel() const
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return INVALID_VALUE;
    }
    return GetHostProxy()->GetDeviceBatteryLevel(address_);
}

int BluetoothRemoteDevice::GetPairState() const
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return INVALID_VALUE;
    }
    return GetHostProxy()->GetPairState(transport_, address_);
}

bool BluetoothRemoteDevice::StartPair()
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return false;
    }
    return GetHostProxy()->StartPair(transport_, address_);
}

bool BluetoothRemoteDevice::CancelPairing()
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return false;
    }
    return GetHostProxy()->CancelPairing(transport_, address_);
}

bool BluetoothRemoteDevice::IsBondedFromLocal() const
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return false;
    }
    return GetHostProxy()->IsBondedFromLocal(transport_, address_);
}

bool BluetoothRemoteDevice::IsAclConnected() const
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return INVALID_VALUE;
    }
    return GetHostProxy()->IsAclConnected(transport_, address_);
}

bool BluetoothRemoteDevice::IsAclEncrypted() const
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return INVALID_VALUE;
    }
    return GetHostProxy()->IsAclEncrypted(transport_, address_);
}

BluetoothDeviceClass BluetoothRemoteDevice::GetDeviceClass() const
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return BluetoothDeviceClass(0);
    }
    int cod = GetHostProxy()->GetDeviceClass(address_);
    return BluetoothDeviceClass(cod);
}

std::vector<ParcelUuid> BluetoothRemoteDevice::GetDeviceUuids() const
{
    std::vector<ParcelUuid> parcelUuids;
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return parcelUuids;
    }

    std::vector<bluetooth::Uuid> uuids = GetHostProxy()->GetDeviceUuids(transport_, address_);
    for (auto iter : uuids) {
        ParcelUuid parcelUuid = UUID::ConvertFrom128Bits(iter.ConvertTo128Bits());
        parcelUuids.push_back(parcelUuid);
    }
    return parcelUuids;
}

bool BluetoothRemoteDevice::SetDevicePin(const std::string &pin)
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return INVALID_VALUE;
    }
    return GetHostProxy()->SetDevicePin(address_, pin);
}

bool BluetoothRemoteDevice::SetDevicePairingConfirmation(bool accept)
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return INVALID_VALUE;
    }
    return GetHostProxy()->SetDevicePairingConfirmation(transport_, address_, accept);
}

bool BluetoothRemoteDevice::SetDevicePasskey(int passkey, bool accept)
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return INVALID_VALUE;
    }
    return GetHostProxy()->SetDevicePasskey(transport_, address_, passkey, accept);
}

bool BluetoothRemoteDevice::PairRequestReply(bool accept)
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return INVALID_VALUE;
    }
    return GetHostProxy()->PairRequestReply(transport_, address_, accept);
}

bool BluetoothRemoteDevice::ReadRemoteRssiValue()
{
    if (!IsValidBluetoothRemoteDevice()) {
        HILOGW("%s::Invalid remote device.", __FUNCTION__);
        return INVALID_VALUE;
    }
    return GetHostProxy()->ReadRemoteRssiValue(address_);
}

}  // namespace Bluetooth
}  // namespace OHOS
