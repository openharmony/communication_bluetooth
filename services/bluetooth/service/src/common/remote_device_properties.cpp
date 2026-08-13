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
#define LOG_TAG "bt_service_remote_device"
#endif

#include "remote_device_properties.h"

#include "adapter_manager.h"
#include "bluetooth_hw_interface.h"
#include "ble_adapter.h"
#include "bluetooth_common_event_helper.h"
#include "classic_adapter.h"
#include "connect_strategy_manager.h"
#include "hal_util.h"
#include "log.h"
#include "common_util.h"
#include "securec.h"
#include "service_util.h"
#include "bt_chr_dft_exception.h"
#include "bt_chr_ue_manager.h"
#include "permission_manager.h"
#include "thread_util.h"
#include "hitrace_meter.h"
#include "cloud_device_manager.h"

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;

constexpr int PAIR_REJECT = 0;
constexpr int PAIR_ACCEPT = 1;
constexpr int MAX_PIN_CODE_LENGTH = 16;
const std::map<int32_t, DeviceType> hwDeviceTypeToCustomType = {
    {static_cast<int32_t>(HW_ECHO_PRODUCT::HEADSET_TWS), DeviceType::DEVICE_TYPE_HEADSET},
    {static_cast<int32_t>(HW_ECHO_PRODUCT::HEADSET_HEADPHONE), DeviceType::DEVICE_TYPE_HEADSET},
    {static_cast<int32_t>(HW_ECHO_PRODUCT::HEADSET_WERABLE), DeviceType::DEVICE_TYPE_HEADSET},
    {static_cast<int32_t>(HW_ECHO_PRODUCT::GLASSES), DeviceType::DEVICE_TYPE_GLASSES},
    {static_cast<int32_t>(HW_ECHO_PRODUCT::CAR), DeviceType::DEVICE_TYPE_CAR},
};

RemoteDeviceProperties::RemoteDeviceProperties() : adapterProperties_(AdapterProperties::GetInstance())
{}

RemoteDeviceProperties::~RemoteDeviceProperties()
{}

RemoteDeviceProperties* RemoteDeviceProperties::GetInstance(void)
{
    static RemoteDeviceProperties singleton;
    return &singleton;
}

void RemoteDeviceProperties::FreeMemory()
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    for (auto it = remoteDevicesMap_.begin(); it != remoteDevicesMap_.end();) {
        if (it->second->IsPairNone() == true) {
            it = remoteDevicesMap_.erase(it);
        } else {
            ++it;
        }
    }
    std::lock_guard<BtRecursiveMutex> lks(remoteDevicesQueueMutex_);
    while (!remoteDeviceQueue.empty()) {
        remoteDeviceQueue.pop();
    }
}

bool RemoteDeviceProperties::IsRemoteDeviceMapEmpty()
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    return remoteDevicesMap_.empty();
}

std::shared_ptr<BluetoothDevice> RemoteDeviceProperties::FindRemoteDevice(const RawAddress &device)
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    std::shared_ptr<BluetoothDevice> remoteDevice;
    auto it = remoteDevicesMap_.find(device.GetAddress());
    if (it != remoteDevicesMap_.end()) {
        remoteDevice = it->second;
    } else {
        remoteDevice = std::make_shared<BluetoothDevice>(device.GetAddress());
        remoteDevicesMap_.insert(std::make_pair(device.GetAddress(), remoteDevice));
        std::lock_guard<BtRecursiveMutex> lks(remoteDevicesQueueMutex_);
        remoteDeviceQueue.push(device.GetAddress());
        while (remoteDeviceQueue.size() > MAX_DEVICE_QUEUE_SIZE) {
            std::string deleteAddress = remoteDeviceQueue.front();
            remoteDeviceQueue.pop(); // 保证pop，不要在这之前break
            auto it = remoteDevicesMap_.find(deleteAddress);
            if (it == remoteDevicesMap_.end()) {
                break;
            }
            int32_t pairState = it->second->GetPairedStatus();
            if (pairState != PAIR_PAIRED && pairState != PAIR_PAIRING) {
                remoteDevicesMap_.erase(it);
            }
            if (pairState == PAIR_PAIRING || pairState == PAIR_PAIRED) {
                HILOGI("The device at the head of the queue is not pairnone. Add the device to the end of the queue.");
                remoteDeviceQueue.push(deleteAddress);
            }
        }
    }
    return remoteDevice;
}

std::string RemoteDeviceProperties::GetDeviceName(const RawAddress &device, bool alias) const
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    std::string remoteName = "";
    auto it = remoteDevicesMap_.find(device.GetAddress());
    if (it != remoteDevicesMap_.end()) {
        remoteName = it->second->GetRemoteName(alias);
    }
    HILOGD("addr: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
    return remoteName;
}

std::vector<Uuid> RemoteDeviceProperties::GetDeviceUuids(const RawAddress &device) const
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    std::vector<Uuid> uuids;
    auto it = remoteDevicesMap_.find(device.GetAddress());
    if (it != remoteDevicesMap_.end()) {
        uuids = it->second->GetDeviceUuids();
    }
    return uuids;
}

bool RemoteDeviceProperties::IsBondedFromLocal(const RawAddress &device) const
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    bool isBondedFromLocal = false;
    auto it = remoteDevicesMap_.find(device.GetAddress());
    if (it != remoteDevicesMap_.end()) {
        isBondedFromLocal = it->second->IsBondedFromLocal();
    }
    return isBondedFromLocal;
}

std::vector<RawAddress> RemoteDeviceProperties::GetBtPairedDevices() const
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    std::vector<RawAddress> pairedList;
    if (remoteDevicesMap_.empty()) {
        return pairedList;
    }
    for (auto &device : remoteDevicesMap_) {
        if (device.second != nullptr && device.second->IsPaired()) {
            RawAddress rawAddr(device.second->GetAddress());
            pairedList.push_back(rawAddr);
        }
    }
    return pairedList;
}

std::shared_ptr<BluetoothDevice> RemoteDeviceProperties::GetBluetoothDeviceFromMap(const RawAddress &device) const
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    auto it = remoteDevicesMap_.find(device.GetAddress());
    if (it == remoteDevicesMap_.end()) {
        return nullptr;
    }
    return it->second;
}

bool RemoteDeviceProperties::CancelPairing(const RawAddress &device) const
{
    const bt_interface_t *btInterface = nullptr;
    std::string callingName = PermissionManager::GetCallingName();
    int status = hal_util_load_bt_library(&btInterface);
    if (status != 0) {
        HILOGE("[ClassicAdapter] Failed to open the Bluetooth module");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_CANCEL_PAIR, device, UE_COMMON_SCENE_CASE2, callingName);
        return false;
    }
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    auto it = remoteDevicesMap_.find(device.GetAddress());
    if (it == remoteDevicesMap_.end() || (it->second->GetPairedStatus() != PAIR_PAIRING)) {
        HILOGE("failed, because of not in PAIR_PAIRING!");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_CANCEL_PAIR, device, UE_COMMON_SCENE_CASE3, callingName);
        return false;
    }

    it->second->SetPairedStatus(PAIR_CANCELING);
    BLUEDROID::RawAddress btAddr = ServiceUtil::AddrToBluedroid(device);
    HILOGI("btAddr: %{public}s", GetEncryptAddr(btAddr.ToLogString()).c_str());
    bool ret = (btInterface->cancel_bond(&btAddr) == BT_STATUS_SUCCESS);
    if (ret) {
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_CANCEL_PAIR, device, UE_COMMON_SCENE_CASE1, callingName);
    } else {
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_CANCEL_PAIR, device, UE_COMMON_SCENE_CASE4, callingName);
    }
    return ret;
}

bool RemoteDeviceProperties::RemovePair(const RawAddress &device)
{
    BLUEDROID::RawAddress btAddr = ServiceUtil::AddrToBluedroid(device);
    const bt_interface_t *btInterface = nullptr;
    std::string callingName = PermissionManager::GetCallingName();
    BtChrEventWriteStr(CHR_BT_WATCH_REMOVE_PAIR, device.GetAddress(), "PKGNAME", callingName);

    int status = hal_util_load_bt_library(&btInterface);
    if (status != 0) {
        HILOGE("[ClassicAdapter] Failed to open the Bluetooth module");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_REMOVE_PAIR, device, UE_COMMON_SCENE_CASE2, callingName);
#ifdef BLUETOOTH_WATCH_ENABLE
        BtChrEventWriteInt(CHR_BT_WATCH_REMOVE_PAIR, device.GetAddress(), "REMOVEPAIRRESULT", CHR_LOAD_BLUETOOTH_FAIL);
#endif
        return false;
    }
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    auto it = remoteDevicesMap_.find(device.GetAddress());
    if ((it == remoteDevicesMap_.end())) {
        HILOGW("RemovePair failed, because of not find the paired device!");
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_REMOVE_PAIR, device, UE_COMMON_SCENE_CASE3, callingName);
#ifdef BLUETOOTH_WATCH_ENABLE
        BtChrEventWriteInt(CHR_BT_WATCH_REMOVE_PAIR, device.GetAddress(), "REMOVEPAIRRESULT", CHR_DEVICE_NOT_PAIR);
#endif
        return false;
    }
    std::shared_ptr<BluetoothDevice> remoteDevice = FindRemoteDevice(device);
    if (remoteDevice->IsCloudDevice()) {
        HILOGI("[CLOUD_PAIR] set cloud bond state: CLOUD_BOND_REMOVING.");
        remoteDevice->SetCloudBondState(CLOUD_BOND_REMOVING);
    }
    remoteDevice->SetAutoConnSwitch(0);

    it->second->SetAliasName(INVALID_NAME);
    SetCustomType(device, DEVICE_TYPE_DEFAULT);
    it->second->SetPairedStatus(PAIR_NONE);
    DeleteLinkKey(it->second);
    adapterProperties_->RemovePairedDeviceList(it->second->GetAddress());
    bool ret = (btInterface->remove_bond(&btAddr) == BT_STATUS_SUCCESS);
    if (ret) {
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_REMOVE_PAIR, device, UE_COMMON_SCENE_CASE1, callingName);
    } else {
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_REMOVE_PAIR, device, UE_COMMON_SCENE_CASE4, callingName);
    }
    BluetoothHelper::BluetoothCommonEventHelper::PublishRemovePairEvent(device.GetAddress());
    std::vector<std::string> pairedAddrList = adapterProperties_->GetPairedAddrList();
    BtChrRemovePairHandle(device.GetAddress(), pairedAddrList.size());

    return ret;
}

void RemoteDeviceProperties::DeleteLinkKey(std::shared_ptr<BluetoothDevice> remoteDevice) const
{
    int keyType = remoteDevice->GetLinkKeyType();
    if (keyType != PAIR_INVALID_LINK_KEY_TYPE) {
        remoteDevice->DeleteLinkKey();
        remoteDevice->SetLinkKeyType(PAIR_INVALID_LINK_KEY_TYPE);
    }
}

std::vector<RawAddress> RemoteDeviceProperties::removeAllDevicesFromMap()
{
    const bt_interface_t *btInterface = nullptr;
    std::vector<RawAddress> removeDevices;
    int status = hal_util_load_bt_library(&btInterface);
    if (status) {
        return removeDevices;
    }
    std::vector<std::string> removeAddrs;
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    for (auto it = remoteDevicesMap_.begin(); it != remoteDevicesMap_.end();) {
        if (it->second->IsPaired() == true) {
            it->second->SetPairedStatus(PAIR_NONE);
            DeleteLinkKey(it->second);
            adapterProperties_->RemovePairedDeviceList(it->second->GetAddress());
            RawAddress device = RawAddress(it->second->GetAddress());
            it = remoteDevicesMap_.erase(it);
            BLUEDROID::RawAddress btAddr = ServiceUtil::AddrToBluedroid(device);
            removeDevices.push_back(device);
            btInterface->remove_bond(&btAddr);
            removeAddrs.push_back(btAddr.ToString());
        } else {
            ++it;
        }
    }
    BtChrRemoveAllPairHandle(removeAddrs);
    return removeDevices;
}

int RemoteDeviceProperties::GetPairState(const RawAddress &device) const
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    int pairState = PAIR_NONE;
    auto it = remoteDevicesMap_.find(device.GetAddress());
    if (it == remoteDevicesMap_.end()) {
        return pairState;
    } else {
        pairState = it->second->GetPairedStatus();
    }
    return pairState;
}

bool RemoteDeviceProperties::SetDevicePairingConfirmation(const RawAddress &device, bool accept) const
{
    const bt_interface_t *btInterface = nullptr;
    int status = hal_util_load_bt_library(&btInterface);
    if (status) {
        HILOGE("[ClassicAdapter] Failed to open the Bluetooth module");
        return false;
    }
    HILOGI("SetDevicePairingConfirmation device.address: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
    bool ret = false;
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    auto it = remoteDevicesMap_.find(device.GetAddress());
    if ((it == remoteDevicesMap_.end()) || (it->second->GetPairedStatus() == PAIR_PAIRED) ||
        (it->second->GetPairedStatus() == PAIR_NONE)) {
        HILOGE("failed, not in pairing state.");
        return ret;
    }

    it->second->SetPairConfirmState(PAIR_CONFIRM_STATE_USER_CONFIRM_REPLY);
    it->second->SetPairConfirmType(PAIR_CONFIRM_TYPE_INVALID);

    int passKey = it->second->GetPasskey();
    bt_ssp_variant_t pairingVariant = static_cast<bt_ssp_variant_t> (it->second->GetSspVariant());
    BLUEDROID::RawAddress address = ServiceUtil::AddrToBluedroid(device);

    if (it->second->GetPairedStatus() == PAIR_CANCELING || accept == false) {
        ret = (btInterface->ssp_reply(&address, pairingVariant, PAIR_REJECT, passKey) == BT_STATUS_SUCCESS);
    } else {
        ret = (btInterface->ssp_reply(&address, pairingVariant, PAIR_ACCEPT, passKey) == BT_STATUS_SUCCESS);
    }
    return ret;
}

int RemoteDeviceProperties::GetDeviceType(const RawAddress &device) const
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    int type = INVALID_TYPE;
    auto it = remoteDevicesMap_.find(device.GetAddress());
    if (it != remoteDevicesMap_.end()) {
        type = it->second->GetDeviceType();
    }
    return type;
}

bool RemoteDeviceProperties::IsBrDeviceType(std::string address) const
{
    RawAddress device(address);
    int deviceType = GetDeviceType(device);
    if (deviceType == DEVICE_TYPE_BREDR || deviceType == DEVICE_TYPE_DUAL_MONO) {
        return true;
    }
    return false;
}

int RemoteDeviceProperties::GetDeviceClass(const RawAddress &device) const
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    int cod = INVALID_VALUE;
    auto it = remoteDevicesMap_.find(device.GetAddress());
    if (it != remoteDevicesMap_.end()) {
        cod = it->second->GetDeviceClass();
    }
    return cod;
}

std::string RemoteDeviceProperties::GetAliasName(const RawAddress &device) const
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    std::string alias = INVALID_NAME;
    auto it = remoteDevicesMap_.find(device.GetAddress());
    if (it != remoteDevicesMap_.end()) {
        alias = it->second->GetAliasName();
    }
    return alias;
}

int RemoteDeviceProperties::GetHwRemoteDeviceType(const RawAddress &device) const
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    int hwRemoteDeviceType = HW_INVALID_DEVICE_TYPE;
    auto it = remoteDevicesMap_.find(device.GetAddress());
    if (it != remoteDevicesMap_.end()) {
        hwRemoteDeviceType = it->second->GetHwRemoteDeviceType();
    }
    HILOGD("GetHwRemoteDeviceType is %{public}d", hwRemoteDeviceType);
    return hwRemoteDeviceType;
}

bool RemoteDeviceProperties::SetAliasName(const RawAddress &device, const std::string &name)
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    bool ret = true;
    auto it = remoteDevicesMap_.find(device.GetAddress());
    if (it != remoteDevicesMap_.end() && it->second->GetPairedStatus() == PAIR_PAIRED) {
        ret = it->second->SetAliasName(name);
    } else {
        HILOGE("device not found or not paired");
        return false;
    }
    bt_property_t prop;
    prop.type = static_cast<bt_property_type_t>(BT_PROPERTY_REMOTE_FRIENDLY_NAME);
    int len = name.size();
    std::string saveName = name;
    if (name.size() > MAX_ALIAS_LENGTH) {
        len = MAX_ALIAS_LENGTH;
        saveName = name.substr(0, MAX_ALIAS_LENGTH); // 0代表从字符串开始位置开始截断
    }
    size_t size = static_cast<size_t>(len);
    prop.len = size;
    prop.val = const_cast<void*>(static_cast<const void*>(saveName.c_str()));
    BLUEDROID::RawAddress addr;
    BLUEDROID::RawAddress::FromString(device.GetAddress(), addr);
    SetRemoteDeviceProperty(addr, prop);
    BluetoothHelper::BluetoothCommonEventHelper::PublishRemoteNameChangedEvent(device.GetAddress(), name);
    return ret;
}

void RemoteDeviceProperties::UpdateRemoteDeviceName(const RawAddress &device, std::string newRemoteName)
{
    std::shared_ptr<BluetoothDevice> remoteDevice = FindRemoteDevice(device);
    std::string currentRemoteName = remoteDevice->GetRemoteName();
    if (newRemoteName == currentRemoteName) {
        HILOGD("Skip name update for device: %{public}s, Remote Device name is: %{public}s",
            GET_ENCRYPT_ADDR(device), GET_ENCRYPT_DEVICE_NAME(currentRemoteName));
        return;
    }
    remoteDevice->SetRemoteName(newRemoteName);
    if (remoteDevice->GetAliasName() == INVALID_NAME) {
        BluetoothHelper::BluetoothCommonEventHelper::PublishRemoteNameChangedEvent(device.GetAddress(), newRemoteName);
    }
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter) {
        classicAdapter->SendRemoteNameChanged(device, newRemoteName);
        HILOGD("Remote Device name is: %{public}s",  GET_ENCRYPT_DEVICE_NAME(newRemoteName));
    }
    return;
}

void RemoteDeviceProperties::UpdateRemoteDeviceCod(const RawAddress &device, int newCod)
{
    std::shared_ptr<BluetoothDevice> remoteDevice = FindRemoteDevice(device);
    int currentCod = remoteDevice->GetDeviceClass();
    if (newCod == currentCod) {
        HILOGD("Skip cod update for device: %{public}s, Remote Device cod is: %{public}d",
            GET_ENCRYPT_ADDR(device), currentCod);
        return;
    }
    remoteDevice->SetDeviceClass(newCod);
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter) {
        classicAdapter->SendRemoteCodChanged(device, newCod);
        HILOGD("Remote Device cod is: %{public}d", newCod);
    }
    return;
}

void RemoteDeviceProperties::UpdateRemoteHwDeviceType(const RawAddress &device, int newDeviceType)
{
    std::shared_ptr<BluetoothDevice> remoteDevice = FindRemoteDevice(device);
    int currentDeviceType = remoteDevice->GetHwRemoteDeviceType();
    if (newDeviceType == currentDeviceType) {
        HILOGD("Skip deviceType update for device: %{public}s, Remote Device type is: %{public}d",
            GET_ENCRYPT_ADDR(device), currentDeviceType);
        return;
    }
    remoteDevice->SetHwRemoteDeviceType(newDeviceType);
    return;
}

void RemoteDeviceProperties::HandlePropertyRemoteDeviceName(bt_property_t* property, RawAddress device)
{
    std::string newRemoteName = adapterProperties_->ParseDeviceName(property);
    UpdateRemoteDeviceName(device, newRemoteName);
}

void RemoteDeviceProperties::HandlePropertyRemoteDeviceUuid(bt_property_t* property,
    std::shared_ptr<BluetoothDevice> remoteDevice)
{
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter) {
        HILOGD("Save_Uuids");
        classicAdapter->SaveRemoteDeviceUuids(remoteDevice, adapterProperties_->ParseDeviceUuid(property));
    }
}

void RemoteDeviceProperties::HandlePropertyRemoteDeviceCod(bt_property_t* property, RawAddress device)
{
    int newCod = static_cast<int>(adapterProperties_->ParseDeviceCod(property) & CLASS_OF_DEVICE_RANGE);
    UpdateRemoteDeviceCod(device, newCod);
}

void RemoteDeviceProperties::HandlePropertyRemoteDeviceType(bt_property_t* property,
    std::shared_ptr<BluetoothDevice> remoteDevice)
{
    unsigned int newDeviceType = adapterProperties_->ParseDeviceType(property);
    remoteDevice->SetDeviceType(newDeviceType);
}

void RemoteDeviceProperties::HandlePropertyRemoteDeviceAbsVolumKey(bt_property_t* property,
    std::shared_ptr<BluetoothDevice> remoteDevice)
{
    int32_t absVolumeAbility = adapterProperties_->ParseAbsVolumeAbility(property);
    remoteDevice->SetDeviceAbsVolumeAbility(absVolumeAbility);
}

void RemoteDeviceProperties::HandlePropertyRemoteDeviceRssi(bt_property_t* property,
    std::shared_ptr<BluetoothDevice> remoteDevice)
{
    uint32_t rssi = adapterProperties_->ParseDeviceRssi(property);
    remoteDevice->SetRssi(rssi);
}

void RemoteDeviceProperties::HandlePropertyRemoteDeviceAlias(bt_property_t* property,
    std::shared_ptr<BluetoothDevice> remoteDevice)
{
    std::string alias = adapterProperties_->ParseAlias(property);
    if (alias != INVALID_NAME) {
        remoteDevice->SetAliasName(alias);
        BluetoothHelper::BluetoothCommonEventHelper::PublishRemoteNameChangedEvent(remoteDevice->GetAddress(), alias);
    }
}

void RemoteDeviceProperties::HandlePropertyRemoteDeviceCustomType(bt_property_t* property,
    std::shared_ptr<BluetoothDevice> remoteDevice)
{
    int32_t customType = adapterProperties_->ParseDeviceCustomType(property);
    remoteDevice->SetCustomType(customType);
}

void RemoteDeviceProperties::HandlePropertyRemoteDeviceIoCapability(bt_property_t* property,
    std::shared_ptr<BluetoothDevice> remoteDevice)
{
    int32_t ioCapability = adapterProperties_->ParseRemoteDeviceIoCapability(property);
    remoteDevice->SetIoCapability(ioCapability);
}

void RemoteDeviceProperties::HandlePropertyRemoteDeviceVendorId(bt_property_t* property,
    std::shared_ptr<BluetoothDevice> remoteDevice)
{
    int32_t vendorId = adapterProperties_->ParseDeviceVendorId(property);
    remoteDevice->SetVendorId(vendorId);
}

void RemoteDeviceProperties::HandlePropertyRemoteDeviceProductId(bt_property_t* property,
    std::shared_ptr<BluetoothDevice> remoteDevice)
{
    int32_t productId = adapterProperties_->ParseDeviceProductId(property);
    remoteDevice->SetProductId(productId);
}

void RemoteDeviceProperties::HandlePropertyRemoteDeviceManuSpecData(bt_property_t* property,
    std::shared_ptr<BluetoothDevice> remoteDevice)
{
    if (property->len > 0 && property->val != nullptr) {
        std::vector<uint8_t> manuData(static_cast<uint8_t*>(property->val),
                                      static_cast<uint8_t*>(property->val) + property->len);
        remoteDevice->SetManufacturerSpecificData(manuData);
    }
}

void RemoteDeviceProperties::HandlePropertyRemoteDeviceChipInfo(bt_property_t* property,
    std::shared_ptr<BluetoothDevice> remoteDevice)
{
    if (property->len < static_cast<int>(sizeof(bt_remote_version_t))) {
        HILOGE("Invalid length %{public}d for DeviceCustomType", property->len);
        return;
    }
    if (property->val == nullptr) {
        HILOGE("Invalid pointer");
        return;
    }
    int32_t version = reinterpret_cast<const bt_remote_version_t*>(property->val)->version;
    int32_t subVer = reinterpret_cast<const bt_remote_version_t*>(property->val)->sub_ver;
    int32_t manufacturer = reinterpret_cast<const bt_remote_version_t*>(property->val)->manufacturer;
    BtChrUpdateChipInfo(remoteDevice->GetAddress(), version, subVer, manufacturer);
}

void RemoteDeviceProperties::HandlePropertyRemoteDeviceAutoConnSwitch(bt_property_t* property,
    std::shared_ptr<BluetoothDevice> remoteDevice)
{
    int32_t autoConnSwitch = adapterProperties_->ParseDeviceAutoConnSwitch(property);
    remoteDevice->SetAutoConnSwitch(autoConnSwitch);
}

void RemoteDeviceProperties::HandlePropertyRemoteDeviceMessage(bt_property_t* property,
    std::shared_ptr<BluetoothDevice> remoteDevice)
{
    switch (property->type) {
        case BT_PROPERTY_REMOTE_VERSION_INFO:
            HandlePropertyRemoteDeviceChipInfo(property, remoteDevice);
            break;
        case BT_PROPERTY_AUTO_CONNECT_SWITCH:
            HandlePropertyRemoteDeviceAutoConnSwitch(property, remoteDevice);
            break;
        case BT_PROPERTY_ClOUD_CAP:
            HandlePropertyRemoteDeviceCloudCap(property, remoteDevice);
            break;
        default:
            break;
    }
}

void RemoteDeviceProperties::StackErrnoCallback(const RawAddress &device, int status, int errNum)
{
    std::shared_ptr<BluetoothDevice> remoteDevice = FindRemoteDevice(device);
    remoteDevice->SetAutoConnSwitch(errNum);
}

void RemoteDeviceProperties::GetRemoteDevicePropsCallBack(bt_status_t status, BLUEDROID::RawAddress* bd_addr,
    int numProperties, bt_property_t* properties)
{
    if (bd_addr == nullptr) {
        return;
    }
    if (status != BT_STATUS_SUCCESS) {
        return;
    }
    
    // The callback of abs_volume does not switch threads to prevent timing problems.
    if (properties->type == HW_BT_PROPERTY_ABS_VOLUM_KEY) {
        bt_property_t* prop = PropertyDeepCopy(numProperties, properties);
        GetRemoteDevicePropsCallbackInner(*bd_addr, numProperties, prop);
        if (prop) {
            free(prop);
        }
        return;
    }

    DoInAdapterManagerThread(std::bind(
        [this](BLUEDROID::RawAddress addr, int numProperties, bt_property_t* prop) {
            GetRemoteDevicePropsCallbackInner(addr, numProperties, prop);
            if (prop) {
                free(prop);
            }
        },
        *bd_addr, numProperties, PropertyDeepCopy(numProperties, properties)
    ));
}

void RemoteDeviceProperties::HandlePropertyByType(bt_property_t* property, const RawAddress &device,
    std::shared_ptr<BluetoothDevice> remoteDevice)
{
    switch (property->type) {
        case BT_PROPERTY_BDNAME:
            HandlePropertyRemoteDeviceName(property, device);
            break;
        case BT_PROPERTY_UUIDS:
            HandlePropertyRemoteDeviceUuid(property, remoteDevice);
            break;
        case BLUEDROID::BT_PROPERTY_CLASS_OF_DEVICE:
            HandlePropertyRemoteDeviceCod(property, device);
            break;
        case BT_PROPERTY_TYPE_OF_DEVICE:
            HandlePropertyRemoteDeviceType(property, remoteDevice);
            break;
        case HW_BT_PROPERTY_ABS_VOLUM_KEY:
            HandlePropertyRemoteDeviceAbsVolumKey(property, remoteDevice);
            break;
        case BT_PROPERTY_REMOTE_RSSI:
            HandlePropertyRemoteDeviceRssi(property, remoteDevice);
            break;
        case BT_PROPERTY_REMOTE_FRIENDLY_NAME:
            HandlePropertyRemoteDeviceAlias(property, remoteDevice);
            break;
        case HW_BT_PROPERTY_CUSTOM_TYPE:
            HandlePropertyRemoteDeviceCustomType(property, remoteDevice);
            break;
        case HW_BT_PROPERTY_RMT_IO_CAP_KEY:
            HandlePropertyRemoteDeviceIoCapability(property, remoteDevice);
            break;
        case BT_PROPERTY_VENDOR_ID:
            HandlePropertyRemoteDeviceVendorId(property, remoteDevice);
            break;
        case BT_PROPERTY_PRODUCT_ID:
            HandlePropertyRemoteDeviceProductId(property, remoteDevice);
            break;
        case BT_PROPERTY_REMOTE_MANUFACTURE_HW:
            HandlePropertyRemoteDeviceManuSpecData(property, remoteDevice);
            break;
        case BT_PROPERTY_CONNECTION_TIME:
            HandlePropertyRemoteDeviceConnectionTime(property, remoteDevice);
            break;
        default:
            HandlePropertyRemoteDeviceMessage(property, remoteDevice);
            break;
    }
}

void RemoteDeviceProperties::GetRemoteDevicePropsCallbackInner(BLUEDROID::RawAddress bd_addr,
    int numProperties, bt_property_t* properties)
{
    CHECK_AND_RETURN_LOG(properties != nullptr, "Wrong pointer !");
    RawAddress device = ServiceUtil::AddrFromBluedroid(bd_addr);
    std::shared_ptr<BluetoothDevice> remoteDevice = FindRemoteDevice(device);
    
    for (int i = 0; i < numProperties; i++) {
        bt_property_t* property = properties + i;
        HandlePropertyByType(property, device, remoteDevice);
    }
}

void RemoteDeviceProperties::DeviceFoundCallBack(int numProperties, bt_property_t* properties)
{
    if (properties == nullptr) {
        HILOGE("wrong properties");
        return;
    }
    DoInAdapterManagerThread(std::bind(
        [this](int numProperties, bt_property_t* prop) {
            DeviceFoundInner(numProperties, prop);
            if (prop) {
                free(prop);
            }
        },
        numProperties,
        PropertyDeepCopy(numProperties, properties)
    ));
}

void RemoteDeviceProperties::DeviceFoundInner(int numProperties, bt_property_t* properties)
{
    if (properties == nullptr) {
        HILOGE("wrong properties");
        return;
    }
    RawAddress address = RawAddress(INVALID_MAC_ADDRESS);
    for (int i = 0; i < numProperties; i++) {
        bt_property_t* property = properties + i;
        if (property->type == BT_PROPERTY_BDADDR) {
            address = adapterProperties_->ParseDeviceAddr(property);
            break;
        }
    }
    if (address == RawAddress(INVALID_MAC_ADDRESS)) {
        HILOGE("Invaild addr");
        return;
    }
    BLUEDROID::RawAddress rawAddr = ServiceUtil::AddrToBluedroid(address);
    GetRemoteDevicePropsCallbackInner(rawAddr, numProperties, properties);
    std::shared_ptr<BluetoothDevice> remoteDevice = FindRemoteDevice(address);

    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter && IsBrDeviceType(address.GetAddress())) {
        classicAdapter->SendDiscoveryResult(
            address, remoteDevice->GetRssi(), remoteDevice->GetRemoteName(), remoteDevice->GetDeviceClass());
        return;
    }
    auto bleAdapter = AdapterManager::GetInstance()->GetBleAdapter();
    if (bleAdapter) {
        bleAdapter->SendDiscoveryResult(
            address, remoteDevice->GetRssi(), remoteDevice->GetRemoteName(), remoteDevice->GetDeviceClass());
    }
}

bt_property_t* RemoteDeviceProperties::PropertyDeepCopy(int numProperties, bt_property_t* properties)
{
    bt_property_t* copy = nullptr;
    if (numProperties == 0 || properties == nullptr) {
        HILOGE("wrong properties");
        return copy;
    }
    if (numProperties > 0) {
        size_t contentLen = 0;
        for (int i = 0; i < numProperties; i++) {
            auto len = properties[i].len;
            if (len > 0) {
                contentLen += len;
            }
        }
        unsigned int length = static_cast<unsigned int>(sizeof(bt_property_t) * numProperties) + contentLen;
        copy = static_cast<bt_property_t*>(malloc((sizeof(bt_property_t) * numProperties) + contentLen));
        if (copy == nullptr) {
            HILOGE("malloc failed!!");
            return copy;
        }
        (void)memset_s(copy, length, 0, length);
        uint8_t* content = reinterpret_cast<uint8_t*>(copy + numProperties);
        for (int i = 0; i < numProperties; i++) {
            auto len = properties[i].len;
            copy[i].type = properties[i].type;
            copy[i].len = len;
            if (len <= 0) {
                continue;
            }
            copy[i].val = content;
            memcpy_s(content, len, properties[i].val, len);
            content += len;
        }
    }
    return copy;
}

bool RemoteDeviceProperties::SetDevicePin(const RawAddress &device, const std::string &pinCode) const
{
    const bt_interface_t *btInterface = nullptr;
    int status = hal_util_load_bt_library(&btInterface);
    if (status) {
        HILOGE("[ClassicAdapter] Failed to open the Bluetooth module");
        return false;
    }
    HILOGI("SetDevicePin device.address: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
    bool ret = false;
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    auto it = remoteDevicesMap_.find(device.GetAddress());
    if ((it == remoteDevicesMap_.end()) || (it->second->GetPairedStatus() == PAIR_PAIRED) ||
        (it->second->GetPairedStatus() == PAIR_NONE)) {
        HILOGE("failed, not in pairing state.");
        return ret;
    }

    it->second->SetPairConfirmState(PAIR_CONFIRM_STATE_USER_CONFIRM_REPLY);
    it->second->SetPairConfirmType(PAIR_CONFIRM_TYPE_INVALID);
    BLUEDROID::RawAddress address = ServiceUtil::AddrToBluedroid(device);
    bt_pin_code_t code;
    uint8_t pinLen = pinCode.length();
    if (pinLen > MAX_PIN_CODE_LENGTH) {
        HILOGE("failed, pinLen out of size");
        CancelPairing(device);
        return false;
    }
    for (int i = 0; i < pinLen; i++) {
        code.pin[i] = static_cast<uint8_t>(pinCode[i]);
    }
    if (it->second->GetPairedStatus() == PAIR_CANCELING) {
        ret = (btInterface->pin_reply(&address, PAIR_REJECT, pinLen, &code) == BT_STATUS_SUCCESS);
    } else {
        ret = (btInterface->pin_reply(&address, PAIR_ACCEPT, pinLen, &code) == BT_STATUS_SUCCESS);
    }
    return ret;
}

void RemoteDeviceProperties::AddPairWhiteList(const RawAddress &device)
{
    HILOGI("device address: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
    std::lock_guard<BtRecursiveMutex> lk(whiteListMapMutex_);

    auto it = whiteListMap_.find(device.GetAddress());
    if (it != whiteListMap_.end()) {
        it = whiteListMap_.erase(it);
    }
    std::shared_ptr<utility::Timer> whiteListTimeout = std::make_shared<utility::Timer>(
        [this, device] {this->WhiltListCallTimeOut(device);});
    whiteListTimeout->Start(WHITELIST_TIMEOUT_TIME);
    whiteListMap_.insert(std::make_pair(device.GetAddress(), whiteListTimeout));
}

bool RemoteDeviceProperties::CheckPairWhiteList(const RawAddress &device)
{
    HILOGI("device.address: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
    std::lock_guard<BtRecursiveMutex> lk(whiteListMapMutex_);
    auto it = whiteListMap_.find(device.GetAddress());
    if (it != whiteListMap_.end()) {
        it->second->Stop();
        it->second = nullptr;
        return true;
    }
    HILOGW("not find the device!");
    return false;
}

void RemoteDeviceProperties::RemovePairWhiteList(const RawAddress &device)
{
    HILOGI("device.address: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
    std::lock_guard<BtRecursiveMutex> lk(whiteListMapMutex_);
    if (whiteListMap_.empty()) {
        return;
    }

    auto it = whiteListMap_.find(device.GetAddress());
    if ((it == whiteListMap_.end())) {
        HILOGW("not find the device!");
        return;
    }
    it = whiteListMap_.erase(it);
}

void RemoteDeviceProperties::WhiltListCallTimeOut(const RawAddress &device)
{
    HITRACE_METER(BT_TRACE_TAG);
    HILOGE("begin");
    RemovePairWhiteList(device);
}

int32_t RemoteDeviceProperties::GetDeviceAbsVolumeAbility(const RawAddress &device)
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    if (remoteDevicesMap_.count(device.GetAddress())) {
        return remoteDevicesMap_[device.GetAddress()]->GetDeviceAbsVolumeAbility();
    } else {
        return DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT;
    }
}

int32_t RemoteDeviceProperties::GetRemoteDeviceProperty(const BLUEDROID::RawAddress &addr, bt_property_type_t type)
{
    const bt_interface_t *btInterface = nullptr;
    int32_t status = hal_util_load_bt_library(&btInterface);
    if (status != 0) {
        HILOGE("Failed to open the Bluetooth module, status = %{public}d. ", status);
        return BT_STATUS_FAIL;
    }
    auto rawAddr = addr;
    status = btInterface->get_remote_device_property(&rawAddr, type);
    if (status != BT_STATUS_SUCCESS) {
        HILOGE("Failed to get_remote_device_property, status = %{public}d. ", status);
        return BT_STATUS_FAIL;
    }
    return BT_STATUS_SUCCESS;
}

void RemoteDeviceProperties::SetRemoteDeviceProperty(const BLUEDROID::RawAddress &addr, const bt_property_t &prop)
{
    const bt_interface_t *btInterface = nullptr;
    int32_t status = hal_util_load_bt_library(&btInterface);
    if (status != 0) {
        HILOGE("Failed to open the Bluetooth module, status = %{public}d. ", status);
        return;
    }
    auto rawAddr = addr;
    status = btInterface->set_remote_device_property(&rawAddr, &prop);
    HILOGI("status = %{public}d, type: 0x%{public}x. ", status, prop.type);
}

bool RemoteDeviceProperties::IsAclConnected(const RawAddress &device)
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    bool ret = false;
    auto it = remoteDevicesMap_.find(device.GetAddress());
    if (it != remoteDevicesMap_.end()) {
        ret = it->second->IsAclConnected();
    }
    return ret;
}

bool RemoteDeviceProperties::IsAclEncrypted(const RawAddress &device)
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    bool ret = false;
    auto it = remoteDevicesMap_.find(device.GetAddress());
    if (it != remoteDevicesMap_.end()) {
        ret = it->second->IsAclEncrypted();
    }
    return ret;
}

std::vector<std::string> RemoteDeviceProperties::GetNotPairNoneDevices() const
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    std::vector<std::string> notPairNoneDevices;
    if (remoteDevicesMap_.empty()) {
        return notPairNoneDevices;
    }
    for (auto &device : remoteDevicesMap_) {
        if (device.second != nullptr && !device.second->IsPairNone()) {
            std::string addr(device.second->GetAddress());
            std::transform(addr.begin(), addr.end(), addr.begin(), ::tolower);
            notPairNoneDevices.push_back(addr);
        }
    }
    return notPairNoneDevices;
}

int32_t RemoteDeviceProperties::GetCustomType(const RawAddress &device) const
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    int32_t customType = INVALID_TYPE;
    auto it = remoteDevicesMap_.find(device.GetAddress());
    if (it != remoteDevicesMap_.end() && it->second->GetPairedStatus() == PAIR_PAIRED) {
        customType = it->second->GetCustomType();
    }
    return customType;
}

int32_t RemoteDeviceProperties::SetCustomType(const RawAddress &device, int32_t customType)
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    auto it = remoteDevicesMap_.find(device.GetAddress());
    std::string deviceName = "";
    int32_t preCustomType = DeviceType::DEVICE_TYPE_DEFAULT;
    if (it != remoteDevicesMap_.end() && it->second->GetPairedStatus() == PAIR_PAIRED) {
        preCustomType = it->second->GetCustomType();
        deviceName = it->second->GetRemoteName();
        it->second->SetCustomType(customType);
    } else {
        HILOGE("device not found or not paired");
        return BT_ERR_INVALID_PARAM;
    }
    void *type = &customType;
    bt_property_t prop;
    prop.type = static_cast<bt_property_type_t>(HW_BT_PROPERTY_CUSTOM_TYPE);
    prop.len = sizeof(customType);
    prop.val = type;
    BLUEDROID::RawAddress addr;
    BLUEDROID::RawAddress::FromString(device.GetAddress(), addr);
    SetRemoteDeviceProperty(addr, prop);
    BtChrUeManager::GetInstance()->WriteCustomTypeChangeUe(device, deviceName, preCustomType, customType);
    return BT_NO_ERROR;
}

bool RemoteDeviceProperties::GetDeviceBatteryInfo(const std::string &addr, BatteryDetailInfo &batteryInfo) const
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    auto it = remoteDevicesMap_.find(addr);
    if (it == remoteDevicesMap_.end()) {
        HILOGE("Failed to find addr: %{public}s", GET_ENCRYPT_STR_ADDR(addr));
        return false;
    }
    batteryInfo = it->second->GetDeviceBatteryInfo();
    return true;
}

void RemoteDeviceProperties::SetDeviceBatteryInfo(const std::string &addr, const BatteryDetailInfo &batteryInfo)
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    auto it = remoteDevicesMap_.find(addr);
    if (it == remoteDevicesMap_.end()) {
        HILOGE("Failed to find addr: %{public}s", GET_ENCRYPT_STR_ADDR(addr));
        return;
    }
    it->second->SetDeviceBatteryInfo(batteryInfo);
}

void RemoteDeviceProperties::ClearDeviceBatteryInfo(const std::string &addr)
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    auto it = remoteDevicesMap_.find(addr);
    if (it == remoteDevicesMap_.end()) {
        HILOGE("Failed to find addr: %{public}s", GET_ENCRYPT_STR_ADDR(addr));
        return;
    }
    it->second->ClearDeviceBatteryInfo();
}

void RemoteDeviceProperties::SetDevProactiveDisconnectFlag(const std::string &addr,
    uint32_t profileId, bool isProactive)
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    auto it = remoteDevicesMap_.find(addr);
    if (it == remoteDevicesMap_.end()) {
        HILOGE("Failed to find addr: %{public}s", GET_ENCRYPT_STR_ADDR(addr));
        return;
    }
    it->second->SetDevProactiveDisconnectFlag(profileId, isProactive);
}

bool RemoteDeviceProperties::IsDevProactiveDisconnect(const std::string &addr, uint32_t profileId) const
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    auto it = remoteDevicesMap_.find(addr);
    if (it == remoteDevicesMap_.end()) {
        HILOGE("Failed to find addr: %{public}s", GET_ENCRYPT_STR_ADDR(addr));
        return false;
    }
    return it->second->IsDevProactiveDisconnect(profileId);
}

int RemoteDeviceProperties::GetDevConnStateChangeCause(const RawAddress &device, uint32_t profileId, int state)
{
    if (state != static_cast<int>(BTConnectState::DISCONNECTED)) {
        return static_cast<int>(ConnChangeCause::CONNECT_CHANGE_COMMON_CAUSE);
    }
    if (IsDevProactiveDisconnect(device.GetAddress(), profileId)) {
        HILOGI("ProactiveDisconnect device: %{public}s", GET_ENCRYPT_ADDR(device));
        SetDevProactiveDisconnectFlag(device.GetAddress(), profileId, false);
        return static_cast<int>(ConnChangeCause::DISCONNECT_CAUSE_USER_DISCONNECT);
    }

    std::shared_ptr<BluetoothDevice> remoteDevice = FindRemoteDevice(device);
    int deviceClass = remoteDevice->GetMajorMinorClass();
    switch (deviceClass) {
        case BluetoothDevice::PERIPHERAL_KEYBOARD:
            return static_cast<int>(ConnChangeCause::DISCONNECT_CAUSE_CONNECT_FROM_KEYBOARD);
        case BluetoothDevice::PERIPHERAL_POINTING_DEVICE:
            return static_cast<int>(ConnChangeCause::DISCONNECT_CAUSE_CONNECT_FROM_MOUSE);
        case BluetoothDevice::AUDIO_VIDEO_CAR_AUDIO:
            return static_cast<int>(ConnChangeCause::DISCONNECT_CAUSE_CONNECT_FROM_CAR);
        default:
            return static_cast<int>(ConnChangeCause::DISCONNECT_CAUSE_CONNECT_FAIL_INTERNAL);
    }
}

static int32_t GetPropertyIntValue(const std::string &propertyValue)
{
    int32_t propertyIntValue = INVALID_VALUE;
    if (!ConvertStrToDigit(propertyValue, propertyIntValue, HEX_STRING_TO_INT)) {
        return INVALID_VALUE;
    }
    return propertyIntValue;
}

bool RemoteDeviceProperties::SetRemoteDevicePropertyInfo(const RawAddress &device, bt_property_type_t type,
    const std::string &propertyValue)
{
    HILOGD("device: %{public}s, type: 0x%{public}x, propertyValue: %{public}s ", GET_ENCRYPT_ADDR(device), type,
        propertyValue.c_str());

    bool isIntProperty = true;
    int32_t propertyIntValue = INVALID_VALUE;
    std::shared_ptr<BluetoothDevice> remoteDevice = FindRemoteDevice(device);
    switch (type) {
        case bt_property_type_t::BT_PROPERTY_BDNAME:
            remoteDevice->SetRemoteName(propertyValue);
            isIntProperty = false;
            break;
        case BT_PROPERTY_REMOTE_FRIENDLY_NAME:
            return SetAliasName(device, propertyValue);
        case bt_property_type_t::BT_PROPERTY_VENDOR_ID:
            propertyIntValue = GetPropertyIntValue(propertyValue);
            remoteDevice->SetVendorId(propertyIntValue);
            break;
        case bt_property_type_t::BT_PROPERTY_PRODUCT_ID:
            propertyIntValue = GetPropertyIntValue(propertyValue);
            remoteDevice->SetProductId(propertyIntValue);
            break;
        case bt_property_type_t::BT_PROPERTY_NAME_CHANGE_VERSION:
            propertyIntValue = GetPropertyIntValue(propertyValue);
            remoteDevice->SetVersion(propertyIntValue);
            break;
        case bt_property_type_t::BT_PROPERTY_REMOTE_NEW_MODEL_ID:
            remoteDevice->SetNewModelId(propertyValue);
            isIntProperty = false;
            break;
        case bt_property_type_t::BT_PROPERTY_REMOTE_MODEL_ID:
            propertyIntValue = GetPropertyIntValue(propertyValue);
            remoteDevice->SetModelId(propertyIntValue);
            break;
        case bt_property_type_t::BT_PROPERTY_CLASS_OF_DEVICE:
            propertyIntValue = GetPropertyIntValue(propertyValue);
            UpdateRemoteDeviceCod(device, propertyIntValue);
            break;
        case bt_property_type_t::BT_PROPERTY_DEVICE_INFO:
            propertyIntValue = GetPropertyIntValue(propertyValue);
            remoteDevice->SetIcon(propertyIntValue);
            break;
        default:
            return SetOtherPropertyInfo(device, type, propertyValue);
    }
    return SetDeviceProperty(device, type, propertyValue, isIntProperty, propertyIntValue);
}

bool RemoteDeviceProperties::SetOtherPropertyInfo(const RawAddress &device, bt_property_type_t type,
    const std::string &propertyValue)
{
    bool isIntProperty = true;
    int32_t propertyIntValue = INVALID_VALUE;
    std::shared_ptr<BluetoothDevice> remoteDevice = FindRemoteDevice(device);
    int32_t customType = DeviceType::DEVICE_TYPE_DEFAULT;
    switch (type) {
        case bt_property_type_t::BT_PROPERTY_TIMESTAMP:
            propertyIntValue = GetPropertyIntValue(propertyValue);
            remoteDevice->SetTimeStamp(propertyIntValue);
            break;
        case bt_property_type_t::BT_PROPERTY_DEVICE_TYPE_ID:
            propertyIntValue = GetPropertyIntValue(propertyValue);
            remoteDevice->SetHwRemoteDeviceType(propertyIntValue);
            if (hwDeviceTypeToCustomType.find(propertyIntValue) != hwDeviceTypeToCustomType.end()) {
                customType = hwDeviceTypeToCustomType.at(propertyIntValue);
                SetCustomType(device, customType);
            }
            break;
        default:
            HILOGE("Invalid property type: 0x%{public}x, addr: %{public}s", type, GET_ENCRYPT_ADDR(device));
            return false;
    }
    return SetDeviceProperty(device, type, propertyValue, isIntProperty, propertyIntValue);
}

bool RemoteDeviceProperties::SetDeviceProperty(const RawAddress &device, bt_property_type_t type,
    const std::string &propertyValue, bool isIntProperty, int32_t intPropertyValue)
{
    if (isIntProperty && intPropertyValue == INVALID_VALUE) {
        HILOGE("Invalid intPropertyValue, type: 0x%{public}x, addr: %{public}s", type, GET_ENCRYPT_ADDR(device));
        return false;
    }
    
    bt_property_t property;
    property.type = type;
    if (isIntProperty) {
        property.len = sizeof(intPropertyValue);
        property.val = &intPropertyValue;
    } else {
        property.len = static_cast<int>(propertyValue.size());
        property.val = const_cast<void*>(static_cast<const void*>(propertyValue.c_str()));
    }
    BLUEDROID::RawAddress rawAddr = ServiceUtil::AddrToBluedroid(device);
    SetRemoteDeviceProperty(rawAddr, property);
    return true;
}

bool RemoteDeviceProperties::IsNeedDelayConnect(const RawAddress &device)
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    bool isNeedDelayConnect = false;
    auto it = remoteDevicesMap_.find(device.GetAddress());
    if (it != remoteDevicesMap_.end()) {
        isNeedDelayConnect = it->second->IsNeedDelayConnect();
    }
    return isNeedDelayConnect;
}

void RemoteDeviceProperties::HandlePropertyRemoteDeviceConnectionTime(bt_property_t* property,
    std::shared_ptr<BluetoothDevice> remoteDevice)
{
    int64_t connectionTime = adapterProperties_->ParseDeviceConnectionTime(property);
    remoteDevice->SetConnectionTime(connectionTime);
    HILOGI("RemoteDeviceProperties HandlePropertyRemoteDeviceConnectionTime %{public}ld",  connectionTime);
}
 
int64_t RemoteDeviceProperties::GetLastConnectionTime(const RawAddress &device) const
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    int64_t connectionTime = 0;
    auto it = remoteDevicesMap_.find(device.GetAddress());
    if (it != remoteDevicesMap_.end()) {
        connectionTime = it->second->GetLastConnectionTime();
    }
    return connectionTime;
}
 
int32_t RemoteDeviceProperties::SetConnectionTime(const RawAddress &device, int64_t connectionTime)
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    auto it = remoteDevicesMap_.find(device.GetAddress());
    if (it != remoteDevicesMap_.end()) {
        it->second->SetConnectionTime(connectionTime);
    } else {
        HILOGE("device not found or not paired");
        return BT_ERR_INVALID_PARAM;
    }
    void *type = &connectionTime;
    bt_property_t prop;
    prop.type = static_cast<bt_property_type_t>(BT_PROPERTY_CONNECTION_TIME);
    prop.len = sizeof(connectionTime);
    prop.val = type;
	BLUEDROID::RawAddress addr = ServiceUtil::AddrToBluedroid(device);
    SetRemoteDeviceProperty(addr, prop);
    HILOGI("RemoteDeviceProperties SetConnectionTime: addr: %{public}s connectionTime:%{public}ld",
        GET_ENCRYPT_ADDR(device), connectionTime);
    return BT_NO_ERROR;
}

void RemoteDeviceProperties::HandlePropertyRemoteDeviceCloudCap(bt_property_t* property,
    std::shared_ptr<BluetoothDevice> remoteDevice)
{
    if (CloudDeviceManager::GetInstance() == nullptr) {
        return;
    }
    int32_t isCloudDevice = adapterProperties_->ParseDeviceCustomType(property);
    if (isCloudDevice != 0) {
        CloudDeviceManager::GetInstance()->AddCloudDeviceProperties(remoteDevice);
    }
    HILOGI("[CLOUD_PAIR]:HandlePropertyRemoteDeviceCloudCap customType:%{public}d", isCloudDevice);
}

bool RemoteDeviceProperties::IsCloudDevice(const RawAddress &device) const
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    auto it = remoteDevicesMap_.find(device.GetAddress());
    if (it == remoteDevicesMap_.end()) {
        HILOGI("Failed to find addr: %{public}s", GET_ENCRYPT_STR_ADDR(device.GetAddress()));
        return false;
    }
    return it->second->IsCloudDevice();
}

void RemoteDeviceProperties::SetCloudDevice(const RawAddress &device, bool isCloudDevice)
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    auto it = remoteDevicesMap_.find(device.GetAddress());
    if (it == remoteDevicesMap_.end()) {
        HILOGE("Failed to find addr: %{public}s", GET_ENCRYPT_STR_ADDR(device.GetAddress()));
        return;
    }
    it->second->SetCloudDevice(isCloudDevice);
}

int32_t RemoteDeviceProperties::GetCloudBondState(const RawAddress &device) const
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    auto it = remoteDevicesMap_.find(device.GetAddress());
    if (it == remoteDevicesMap_.end()) {
        HILOGI("Failed to find addr: %{public}s", GET_ENCRYPT_STR_ADDR(device.GetAddress()));
        return 0;
    }
    return it->second->GetCloudBondState();
}

void RemoteDeviceProperties::SetCloudBondState(const RawAddress &device, int32_t cloudBondState)
{
    std::lock_guard<BtRecursiveMutex> lk(remoteDevicesMapMutex_);
    auto it = remoteDevicesMap_.find(device.GetAddress());
    if (it == remoteDevicesMap_.end()) {
        HILOGE("Failed to find addr: %{public}s", GET_ENCRYPT_STR_ADDR(device.GetAddress()));
        return;
    }
    return it->second->SetCloudBondState(cloudBondState);
}

bool RemoteDeviceProperties::IsStartCloudPair(const RawAddress &device)
{
    return IsCloudDevice(device) && (GetCloudBondState(device) == CLOUD_BOND_NONE);
}
}  // namespace bluetooth
}  // namespace OHOS
