/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_cloud_device_manager"
#endif

#include "cloud_device_manager.h"
#include "classic_adapter.h"
#include "common/bluetooth_hw_interface.h"
#include "common_util.h"
#include "adapter_manager.h"
#include "bluetooth_audio_manager.h"
#include "bt_chr_ue_manager.h"
#include <iomanip>
#include <sstream>
#include "bluetooth_common_event_helper.h"

using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {

constexpr size_t PRODUCT_ID_PARTS_SIZE = 3;
constexpr size_t MODEL_ID_INDEX = 0;
constexpr size_t SUB_MODEL_ID_INDEX = 1;
constexpr size_t ICON_ID_INDEX = 2;
constexpr uint32_t DEFAULT_ICON_ID = 0xFFFF;

CloudDeviceManager::CloudDeviceManager()
{}

CloudDeviceManager::~CloudDeviceManager()
{}

CloudDeviceManager *CloudDeviceManager::GetInstance()
{
    static CloudDeviceManager instance;
    return &instance;
}

bool IsAdvIrkInvalid(const std::vector<uint8_t>& advInfo)
{
    if (advInfo.size() != ADV_INFO_BYTE_ARRY_LEN_49) {
        HILOGW("adv key invalid, size:%{public}d", advInfo.size());
        return true;
    }
    /* Structure of advInfo: total length is BYTE_ARRY_LEN_49
     * irk: [1~16] (index 0~15)
     */
    std::vector<uint8_t> irk(ADV_INFO_BYTE_ARRY_LEN_16, 0);
    std::copy(advInfo.begin(), advInfo.begin() + ADV_INFO_BYTE_ARRY_LEN_16, irk.begin());

    if (std::all_of(irk.begin(), irk.end(), [](uint8_t k) { return k == 0; })) {
        HILOGD("irk is empty");
        return true;
    }

    return false;
}

bool IsAdvIrkChanged(const std::vector<uint8_t>& advInfo, const std::vector<uint8_t>& currentAdvIrk)
{
    // bluetooth stack will not update irk in this condition
    if (advInfo.size() != ADV_INFO_BYTE_ARRY_LEN_49) {
        HILOGW("adv key invalid, size:%{public}d", advInfo.size());
        return false;
    }

    std::vector<uint8_t> newIrk(ADV_INFO_BYTE_ARRY_LEN_16, 0);
    std::copy(advInfo.begin(), advInfo.begin() + ADV_INFO_BYTE_ARRY_LEN_16, newIrk.begin());

    if (newIrk.size() != currentAdvIrk.size()) {
        return true;
    }

    return !std::equal(newIrk.begin(), newIrk.end(), currentAdvIrk.begin());
}

std::string CloudTokenToLogString(const std::vector<uint8_t> &tokens)
{
    std::stringstream ss;
    int hexWidth = 2;
    for (uint8_t byte : tokens) {
        ss << std::hex << std::setw(hexWidth) << std::setfill('0') << static_cast<int>(byte);
    }
    std::string tokenHexStr = ss.str();

    // 匿名化处理, 保留前后3位, 其余用*代替; 不足6位的则全部匿名
    size_t maxRemainLen = 6; // 总体保留不匿名的长度
    size_t endLen = 3; //两端保留不匿名的长度
    if (tokenHexStr.length() > maxRemainLen) {
        std::string anonymized = tokenHexStr.substr(0, endLen);
        anonymized += std::string(tokenHexStr.length() - maxRemainLen, '*');
        anonymized += tokenHexStr.substr(tokenHexStr.length() - endLen, endLen);
        return anonymized;
    } else {
        return std::string(tokenHexStr.length(), '*');
    }
}

bool IsUuidChanged(const std::vector<Uuid>& newUuids, const std::vector<Uuid>& currentUuids)
{
    if ((newUuids.empty() && !currentUuids.empty()) || (!newUuids.empty() && currentUuids.empty())) {
        HILOGI("all uuid change, new size:%{public}d, current size:%{public}d",
            newUuids.size(), currentUuids.size());
        return true;
    }

    bool isUuidUpdated = false;

    for (const auto& uuid : newUuids) {
        if (std::find(currentUuids.begin(), currentUuids.end(), uuid) == currentUuids.end()) {
            isUuidUpdated = true;
            HILOGI("add uuid:%{public}s", uuid.ToString().c_str());
        }
    }

    for (const auto& uuid : currentUuids) {
        if (std::find(newUuids.begin(), newUuids.end(), uuid) == newUuids.end()) {
            isUuidUpdated = true;
            HILOGI("delete uuid:%{public}s", uuid.ToString().c_str());
        }
    }

    return isUuidUpdated;
}


static void ChrCloudPairCheckIfNameChange(
    std::shared_ptr<BluetoothDevice> device, Bluetooth::BluetoothTrustPairDevice &element)
{
    if (device == nullptr) {
        return;
    }

    RawAddress addr(device->GetAddress());
    std::string newDevName = element.GetDeviceName();
    std::string currentDevName = device->GetRemoteName();
    if (newDevName != currentDevName) {
        HILOGI("%{public}s name change", GET_ENCRYPT_STR_ADDR(device->GetAddress()));
        BtChrUpdateDeviceInfo(addr.GetAddress(), newDevName, device->GetRssi(), device->GetDeviceClass());
        BtChrUeManager::GetInstance()->HwChrReportFinishCloudPair(addr,
            static_cast<int>(UeCloudPairSceneCode::UPDATE_DOWN_CLOUD_DEV),
            static_cast<int>(UeUpdateCloudPairSubSceneCode::UPDATE_DEV_NAME));
    }
}

static void ChrCloudPairCheckUpdatingProp(
    std::shared_ptr<BluetoothDevice> device, Bluetooth::BluetoothTrustPairDevice &element)
{
    if (device == nullptr) {
        return;
    }

    RawAddress addr(device->GetAddress());
    std::vector<Uuid> newUuids = element.GetParseUuid();
    std::vector<Uuid> currentUuids = device->GetDeviceUuids();
    if (IsUuidChanged(newUuids, currentUuids)) {
        HILOGI("%{public}s uuid change", GET_ENCRYPT_STR_ADDR(device->GetAddress()));
        BtChrUeManager::GetInstance()->HwChrReportFinishCloudPair(addr,
            static_cast<int>(UeCloudPairSceneCode::UPDATE_DOWN_CLOUD_DEV),
            static_cast<int>(UeUpdateCloudPairSubSceneCode::UPDATE_DEV_UUID));
    }

    std::vector<uint8_t> advIrk(ADV_INFO_BYTE_ARRY_LEN_16, 0);
    BluetoothHwInterface::GetInstance()->HwGetAdvIrk(addr, advIrk);

    if (IsAdvIrkChanged(element.GetSecureAdvertisingInfo(), advIrk)) {
        HILOGI("%{public}s irk change", GET_ENCRYPT_STR_ADDR(device->GetAddress()));
        BtChrUeManager::GetInstance()->HwChrReportFinishCloudPair(addr,
            static_cast<int>(UeCloudPairSceneCode::UPDATE_DOWN_CLOUD_DEV),
            static_cast<int>(UeUpdateCloudPairSubSceneCode::UPDATE_DEV_IRK));
    }
}

static void ChrCloudPairCheckAddingProp(
    std::shared_ptr<BluetoothDevice> device, Bluetooth::BluetoothTrustPairDevice &element)
{
    if (device == nullptr) {
        return;
    }
    RawAddress addr(device->GetAddress());
    if (element.GetDeviceName().empty()) {
        HILOGW("%{public}s name is empty", GET_ENCRYPT_STR_ADDR(device->GetAddress()));
        BtChrUeManager::GetInstance()->HwChrReportFinishCloudPair(addr,
            static_cast<int>(UeCloudPairSceneCode::ADD_DOWN_CLOUD_DEV),
            static_cast<int>(UeAddCloudPairSubSceneCode::DOWN_CLOUD_DEV_NAME_EMPTY));
    }
    if (element.GetParseUuid().empty()) {
        HILOGW("%{public}s uuid is empty", GET_ENCRYPT_STR_ADDR(device->GetAddress()));
        BtChrUeManager::GetInstance()->HwChrReportFinishCloudPair(addr,
            static_cast<int>(UeCloudPairSceneCode::ADD_DOWN_CLOUD_DEV),
            static_cast<int>(UeAddCloudPairSubSceneCode::DOWN_CLOUD_DEV_UUID_EMPTY));
    }
    if (element.GetToken().empty()) {
        HILOGW("%{public}s token is empty", GET_ENCRYPT_STR_ADDR(device->GetAddress()));
        BtChrUeManager::GetInstance()->HwChrReportFinishCloudPair(addr,
            static_cast<int>(UeCloudPairSceneCode::ADD_DOWN_CLOUD_DEV),
            static_cast<int>(UeAddCloudPairSubSceneCode::DOWN_CLOUD_DEV_TOKEN_INVALID));
    }

    if (IsAdvIrkInvalid(element.GetSecureAdvertisingInfo())) {
        HILOGW("%{public}s irk is empty", GET_ENCRYPT_STR_ADDR(device->GetAddress()));
        BtChrUeManager::GetInstance()->HwChrReportFinishCloudPair(addr,
            static_cast<int>(UeCloudPairSceneCode::ADD_DOWN_CLOUD_DEV),
            static_cast<int>(UeAddCloudPairSubSceneCode::DOWN_CLOUD_DEV_IRK_EMPTY));
    }
}

void CloudDeviceManager::SetDeviceClassProp(std::shared_ptr<BluetoothDevice> device, int32_t deviceClass)
{
    if (device == nullptr || RemoteDeviceProperties::GetInstance() == nullptr) {
        return;
    }
    device->SetDeviceClass(deviceClass);
    SetRemoteDevicePropertyInt(
        RawAddress(device->GetAddress()), bt_property_type_t::BT_PROPERTY_CLASS_OF_DEVICE, deviceClass);
}

void CloudDeviceManager::SetDeviceNameProp(std::shared_ptr<BluetoothDevice> device, const std::string &deviceName)
{
    if (device == nullptr || deviceName.empty() || RemoteDeviceProperties::GetInstance() == nullptr) {
        return;
    }
    device->SetRemoteName(deviceName);
    bt_property_t prop;
    prop.type = static_cast<bt_property_type_t>(BT_PROPERTY_BDNAME);
    std::string saveName = deviceName;
    prop.len = static_cast<int32_t>(deviceName.size());
    prop.val = const_cast<void *>(static_cast<const void *>(saveName.c_str()));
    BLUEDROID::RawAddress addr = ServiceUtil::AddrToBluedroid(RawAddress(device->GetAddress()));
    RemoteDeviceProperties::GetInstance()->SetRemoteDeviceProperty(addr, prop);
}

void CloudDeviceManager::SetCloudDeviceProp(std::shared_ptr<BluetoothDevice> device, bool isCloudDev)
{
    if (device == nullptr) {
        return;
    }
    device->SetCloudDevice(isCloudDev);
    if (isCloudDev) {
        int32_t cloudDevType = isCloudDev ? 1 : 0;
        SetRemoteDevicePropertyInt(
            RawAddress(device->GetAddress()), bt_property_type_t::BT_PROPERTY_ClOUD_CAP, cloudDevType);
    }
}

void CloudDeviceManager::SetDeviceTypeProp(std::shared_ptr<BluetoothDevice> device, int32_t devType)
{
    if (device == nullptr || RemoteDeviceProperties::GetInstance() == nullptr) {
        return;
    }
    device->SetDeviceType(devType);
    SetRemoteDevicePropertyInt(
        RawAddress(device->GetAddress()), bt_property_type_t::BT_PROPERTY_TYPE_OF_DEVICE, BT_DEVICE_TYPE_DUMO);
}

void CloudDeviceManager::SetDeviceUuidsProp(
    std::shared_ptr<BluetoothDevice> device, const std::vector<Uuid> &uuids)
{
    if (device == nullptr || uuids.empty() || RemoteDeviceProperties::GetInstance() == nullptr) {
        return;
    }
    device->SetDeviceUuids(uuids);
    std::vector<BLUEDROID::bluetooth::Uuid> uuidsBlueDroid;
    for (auto &uuid : uuids) {
        uuidsBlueDroid.push_back(ServiceUtil::UuidToBluedroid(uuid));
    }
    bt_property_t prop;
    prop.type = static_cast<bt_property_type_t>(BT_PROPERTY_UUIDS);
    int32_t len = static_cast<int32_t>(uuidsBlueDroid.size() * sizeof(BLUEDROID::bluetooth::Uuid));
    prop.len = len;
    prop.val = const_cast<void *>(static_cast<const void *>(uuidsBlueDroid.data()));
    BLUEDROID::RawAddress addr = ServiceUtil::AddrToBluedroid(RawAddress(device->GetAddress()));
    RemoteDeviceProperties::GetInstance()->SetRemoteDeviceProperty(addr, prop);
}

void CloudDeviceManager::SetRemoteDevicePropertyInt(const RawAddress &device, bt_property_type_t type, int32_t value)
{
    void *val = &value;
    bt_property_t prop;
    prop.type = type;
    prop.len = sizeof(value);
    prop.val = val;
    BLUEDROID::RawAddress addr = ServiceUtil::AddrToBluedroid(device);
    RemoteDeviceProperties::GetInstance()->SetRemoteDeviceProperty(addr, prop);
}

void CloudDeviceManager::SetDeviceProductIdProp(std::shared_ptr<BluetoothDevice> device, const std::string &productId)
{
    if (device == nullptr || !device->IsCloudDevice() || productId.empty() ||
        RemoteDeviceProperties::GetInstance() == nullptr) {
        HILOGW("SetDeviceProductIdProp invalid param, device:%{public}d, productId empty:%{public}d", device == nullptr,
               productId.empty());
        return;
    }
    RawAddress addr(device->GetAddress());
    std::vector<std::string> parts = SplitValueByDelim(productId, "_");
    if (parts.size() != PRODUCT_ID_PARTS_SIZE) {
        HILOGE("SetDeviceProductIdProp invalid productId format, parts size:%{public}zu", parts.size());
        return;
    }
    for (size_t i = 0; i < parts.size(); i++) {
        if (parts[i].empty()) {
            HILOGE("SetDeviceProductIdProp empty part at index:%{public}zu", i);
            return;
        }
    }
    const std::string &iconId = parts[ICON_ID_INDEX];
    HILOGI("SetDeviceProductIdProp addr:%{public}s, iconId:%{public}s",
        GET_ENCRYPT_STR_ADDR(device->GetAddress()), iconId.c_str());

    uint32_t iconIdValue = 0;
    CHECK_AND_RETURN_LOG(ConvertStrToDigit<uint32_t>(iconId, iconIdValue, HEX_STRING_TO_INT),
        "SetDeviceProductIdProp parse iconId failed:%{public}s", iconId.c_str());

    if (iconIdValue < DEFAULT_ICON_ID) {
        HILOGI("SetDeviceProductIdProp [DEVICE_INFO/icon] oldValue: %{public}d, newValue: %{public}d",
            device->GetIcon(), static_cast<int32_t>(iconIdValue));
        device->SetIcon(static_cast<int32_t>(iconIdValue));
        SetRemoteDevicePropertyInt(addr, bt_property_type_t::BT_PROPERTY_DEVICE_INFO,
            static_cast<int32_t>(iconIdValue));
    }
}

void CloudDeviceManager::SendCloudDevState(const RawAddress &device)
{
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter != nullptr) {
        BLUEDROID::RawAddress rawAddr = ServiceUtil::AddrToBluedroid(device);
        classicAdapter->SetBondState(BT_STATUS_SUCCESS, rawAddr, BT_BOND_STATE_BONDED);
        HILOGI("%{public}s, set to bonded", rawAddr.ToLogString().c_str());
    }
}
 
std::shared_ptr<BluetoothDevice> CloudDeviceManager::GetCloudBluetoothDeviceFromMap(const RawAddress &device)
{
    std::shared_ptr<BluetoothDevice> remoteDevice = nullptr;
    cloudDevicesMap_.Find(device.GetAddress(), remoteDevice);
    return remoteDevice;
}
 
std::shared_ptr<BluetoothDevice> CloudDeviceManager::AddCloudRemoteDevice(const RawAddress &device)
{
    std::shared_ptr<BluetoothDevice> remoteDevice = nullptr;
    if (cloudDevicesMap_.Find(device.GetAddress(), remoteDevice)) {
        return remoteDevice;
    }
    remoteDevice = std::make_shared<BluetoothDevice>(device.GetAddress());
    cloudDevicesMap_.EnsureInsert(device.GetAddress(), remoteDevice);
    return remoteDevice;
}
 
void CloudDeviceManager::DelCloudRemoteDevice(const std::string &addr)
{
    cloudDevicesMap_.Erase(addr);
}
 
void CloudDeviceManager::AddCloudDeviceProperties(std::shared_ptr<BluetoothDevice> device)
{
    if (device == nullptr) {
        return;
    }
    RawAddress addr(device->GetAddress());
    auto cloudDev = GetCloudBluetoothDeviceFromMap(addr);
    if (cloudDev != nullptr) {
        HILOGI("already cloud device %{public}s",
            GET_ENCRYPT_STR_ADDR(device->GetAddress()));
        return;
    }
    AddCloudRemoteDevice(addr);
    device->SetCloudDevice(true);
    if (IsDeviceHasLinkkey(device)) {
        device->SetCloudBondState(CLOUD_BOND_BONDED);
        HILOGI("%{public}s local link key found, no need cloud pair again.",
            GET_ENCRYPT_STR_ADDR(device->GetAddress()));
    } else {
        device->SetCloudBondState(CLOUD_BOND_NONE);
        device->SetBondedFromLocal(false);
        BluetoothAudioManager::GetInstance().SetSharedPreVirtualAutoConnSwitch(device->GetAddress(), true);
    }
    SetDeviceTypeProp(device, DEVICE_TYPE_DUAL_MONO);
    HILOGI("%{public}s", GET_ENCRYPT_STR_ADDR(device->GetAddress()));
}

void CloudDeviceManager::AddDeviceCloudProp(
    std::shared_ptr<BluetoothDevice> device, Bluetooth::BluetoothTrustPairDevice &element)
{
    if (device == nullptr) {
        return;
    }
    SetDeviceNameProp(device, element.GetDeviceName());
    SetDeviceClassProp(device, element.GetBluetoothClass());
    SetDeviceUuidsProp(device, element.GetParseUuid());
    if (!element.GetToken().empty()) {
        device->SetToken(element.GetToken());
    }
    SetAdvKey(element.GetSecureAdvertisingInfo(), device);
    SetCloudDeviceProp(device, true);
    SetDeviceProductIdProp(device, element.GetProductId());
    ChrCloudPairCheckAddingProp(device, element);
}

void CloudDeviceManager::UpdateCloudDeviceProp(
    std::shared_ptr<BluetoothDevice> device, Bluetooth::BluetoothTrustPairDevice &element)
{
    if (device == nullptr) {
        return;
    }
    RawAddress addr(device->GetAddress());
    bool isTokenUpdated = false;
    ChrCloudPairCheckIfNameChange(device, element);
    if (!device->GetToken().empty() && (device->GetToken() != element.GetToken())) {
        ChrCloudPairCheckUpdatingProp(device, element);
        RemoveAdvKey(device);
        if (device->GetCloudBondState() != CLOUD_BOND_BONDING &&
            device->GetCloudBondState() != CLOUD_BOND_CREATE_BOND) {
            device->SetCloudBondState(CLOUD_BOND_NONE);
            device->SetBondedFromLocal(false);
            BluetoothAudioManager::GetInstance().SetSharedPreVirtualAutoConnSwitch(device->GetAddress(), true);
            BtChrUeManager::GetInstance()->HwChrReportFinishCloudPair(addr,
                static_cast<int>(UeCloudPairSceneCode::UPDATE_DOWN_CLOUD_DEV),
                static_cast<int>(UeUpdateCloudPairSubSceneCode::RESET_DEV_CLOUD_PAIR_STATE));
        }
        SetDeviceNameProp(device, element.GetDeviceName());
        SetAdvKey(element.GetSecureAdvertisingInfo(), device);
        auto oldDeviceToken = device->GetToken();
        device->SetToken(element.GetToken());
        SetDeviceUuidsProp(device, element.GetParseUuid());
        SetDeviceTypeProp(device, DEVICE_TYPE_DUAL_MONO);
        isTokenUpdated = true;
        HILOGI("%{public}s token change, old:%{public}s, new:%{public}s",
            GET_ENCRYPT_STR_ADDR(device->GetAddress()), CloudTokenToLogString(oldDeviceToken).c_str(),
            CloudTokenToLogString(element.GetToken()).c_str());
    }
    if (device->GetToken().empty() && (!element.GetToken().empty())) {
        device->SetToken(element.GetToken());
        SetDeviceTypeProp(device, DEVICE_TYPE_DUAL_MONO);
        HILOGI("%{public}s token is empty in cache, add again from cloud.",
            GET_ENCRYPT_STR_ADDR(device->GetAddress()));
    }
    if (isTokenUpdated) {
        BtChrUeManager::GetInstance()->HwChrReportFinishCloudPair(addr,
            static_cast<int>(UeCloudPairSceneCode::UPDATE_DOWN_CLOUD_DEV),
            static_cast<int>(UeUpdateCloudPairSubSceneCode::UPDATE_DEV_TOKEN));
    }
    SetDeviceProductIdProp(device, element.GetProductId());
}
 
void CloudDeviceManager::DelCloudDeviceProperties(std::shared_ptr<BluetoothDevice> device)
{
    if (device == nullptr || RemoteDeviceProperties::GetInstance() == nullptr) {
        return;
    }
    RawAddress addr(device->GetAddress());
    auto cloudDev = GetCloudBluetoothDeviceFromMap(addr);
    if (cloudDev == nullptr) {
        HILOGI("not cloud device %{public}s", GET_ENCRYPT_STR_ADDR(device->GetAddress()));
        return;
    }
    DelCloudRemoteDevice(device->GetAddress());
    auto dev = RemoteDeviceProperties::GetInstance()->GetBluetoothDeviceFromMap(addr);
    if (dev == nullptr) {
        return;
    }
    SetCloudDeviceProp(dev, false);
    dev->SetCloudBondState(CLOUD_BOND_NONE);
    HILOGI("%{public}s", GET_ENCRYPT_STR_ADDR(device->GetAddress()));
}
 
void CloudDeviceManager::AddCloudBluetoothDev(std::vector<Bluetooth::BluetoothTrustPairDevice> &cloudList)
{
    if (cloudList.empty() || RemoteDeviceProperties::GetInstance() == nullptr) {
        HILOGE("cloudList is empty:%{public}d", cloudList.empty());
        return;
    }
    HILOGI("device num:%{public}d.", cloudList.size());
    for (auto &element : cloudList) {
        if (element.GetMacAddress().empty() || element.GetUuid().empty()) {
            HILOGE("cloud device mac or uuid is empty mac:%{public}d uuid:%{public}d.",
                element.GetMacAddress().empty(), element.GetUuid().empty());
            continue;
        }
        RawAddress addr(element.GetMacAddress());
        auto dev = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(addr);
        if (dev == nullptr) {
            continue;
        }
        if (dev->IsCloudDevice()) {
            UpdateCloudDeviceProp(dev, element);
            continue;
        }
        if (!dev->IsPairNone()) {
            continue;
        }
        AddCloudDeviceProperties(dev);
        AddDeviceCloudProp(dev, element);
        SendCloudDevState(addr);
        SendCloudUuidIntent(dev);
        BtChrUpdateDeviceInfo(addr.GetAddress(), dev->GetRemoteName(), dev->GetRssi(), dev->GetDeviceClass());
        BtChrUeManager::GetInstance()->HwChrReportFinishCloudPair(addr,
            static_cast<int>(UeCloudPairSceneCode::ADD_DOWN_CLOUD_DEV),
            static_cast<int>(UeAddCloudPairSubSceneCode::ADD_DOWN_CLOUD_DEV_CMP));
        HILOGI("%{public}s.", GET_ENCRYPT_STR_ADDR(dev->GetAddress()));
    }
}
 
void CloudDeviceManager::SendCloudUuidIntent(std::shared_ptr<BluetoothDevice> device)
{
    if (device == nullptr || device->GetDeviceUuids().empty()) {
        HILOGE("device is null or uuid empty, device:%{public}d", device == nullptr);
        return;
    }
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter != nullptr) {
        classicAdapter->SendRemoteUuidChanged(RawAddress(device->GetAddress()), device->GetDeviceUuids());
        HILOGI("%{public}s", GET_ENCRYPT_STR_ADDR(device->GetAddress()));
    }
    BluetoothHelper::BluetoothCommonEventHelper::PublishRemoteUuidsChangedEvent(device->GetAddress(),
        ServiceUtil::UuidsToUuidStrs(device->GetDeviceUuids()));
}
 
bool CloudDeviceManager::IsStartCloudPair(const RawAddress &device)
{
    return IsCloudDevice(device) && (GetCloudBondState(device) == CLOUD_BOND_NONE);
}
 
void CloudDeviceManager::DelCloudBluetoothDev(std::vector<Bluetooth::BluetoothTrustPairDevice> &cloudList)
{
    auto delCloudDeviceCallback = [this, cloudList](
        const std::string &address, std::shared_ptr<BluetoothDevice> device) {
        std::string mac;
        if (device == nullptr) {
            return;
        }
        bool isHas = false;
        for (auto &cloudElement : cloudList) {
            std::string mac = cloudElement.GetMacAddress();
            if (address.empty() || mac.empty()) {
                return;
            }
            if (address == mac) {
                isHas = true;
                break;
            }
        }
        if (isHas) {
            return;
        }
        DeleteCloudBtDevice(device, address);
    };
    cloudDevicesMap_.Iterate(delCloudDeviceCallback);
}
 
void CloudDeviceManager::DeleteCloudBtDevice(
    std::shared_ptr<BluetoothDevice> cloudDevice, const std::string &address)
{
    if (cloudDevice == nullptr || RemoteDeviceProperties::GetInstance() == nullptr) {
        return;
    }
    RawAddress addr(address);
    auto dev = RemoteDeviceProperties::GetInstance()->GetBluetoothDeviceFromMap(addr);
    if (dev == nullptr) {
        return;
    }
    if (IsDeviceHasLinkkey(dev) || cloudDevice->GetPairedStatus() == PAIR_PAIRING) {
        HILOGI("cloudDevice has been paired %{public}s,paired status: %{public}d", GET_ENCRYPT_STR_ADDR(address),
            static_cast<int>(cloudDevice->GetPairedStatus()));
        return;
    }
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter != nullptr) {
        classicAdapter->RemovePair(BT_TRANSPORT_BR_EDR, RawAddress(dev->GetAddress()));
    }
    RemoveAdvKey(cloudDevice);
    HILOGI("%{public}s", GET_ENCRYPT_STR_ADDR(address));
    BtChrUeManager::GetInstance()->HwChrReportFinishCloudPair(addr,
        static_cast<int>(UeCloudPairSceneCode::UPDATE_DOWN_CLOUD_DEV),
        static_cast<int>(UeUpdateCloudPairSubSceneCode::DELETE_CLOUD_DEV));
}
 
bool CloudDeviceManager::IsDeviceHasLinkkey(std::shared_ptr<BluetoothDevice> device)
{
    if (device == nullptr) {
        return false;
    }
    return BluetoothHwInterface::GetInstance()->CheckDeviceBonded(RawAddress(device->GetAddress()));
}
 
void CloudDeviceManager::SetAdvKey(std::vector<uint8_t> advInfo, std::shared_ptr<BluetoothDevice> device)
{
    if (device == nullptr) {
        return;
    }
    if (advInfo.size() != ADV_INFO_BYTE_ARRY_LEN_49) {
        HILOGW("adv key invalid, %{public}s", GET_ENCRYPT_STR_ADDR(device->GetAddress()));
        return;
    }
    /* Structure of advInfo: total length is BYTE_ARRY_LEN_49
     * irk: [1~16] (index 0~15)
     * hbk: [17~32] (index 16~31)
     * version: [33] (index 32)
     * feature: [34~49] (index 33~48)
     */
    std::vector<char> irk(ADV_INFO_BYTE_ARRY_LEN_16, 0);
    std::copy(advInfo.begin(), advInfo.begin() + ADV_INFO_BYTE_ARRY_LEN_16, irk.begin());
    const int32_t secondIndex = 2;
    const int32_t thirdIndex = 3;
    std::vector<char> hbk(ADV_INFO_BYTE_ARRY_LEN_16, 0);
    std::copy(advInfo.begin() + ADV_INFO_BYTE_ARRY_LEN_16,
        advInfo.begin() + secondIndex * ADV_INFO_BYTE_ARRY_LEN_16,
        hbk.begin());
    uint8_t version = advInfo[ADV_INFO_BYTE_ARRY_LEN_16 * 2];
    std::vector<char> feature(ADV_INFO_BYTE_ARRY_LEN_16, 0);
    std::copy(advInfo.begin() + secondIndex * ADV_INFO_BYTE_ARRY_LEN_16 + 1,
        advInfo.begin() + thirdIndex * ADV_INFO_BYTE_ARRY_LEN_16 + 1,
        feature.begin());
    BluetoothHwInterface::GetInstance()->HwSetAdvKey(
        RawAddress(device->GetAddress()), irk[0], hbk[0], version, feature[0]);
}
 
void CloudDeviceManager::RemoveAdvKey(std::shared_ptr<BluetoothDevice> device)
{
    if (device == nullptr) {
        return;
    }
    BluetoothHwInterface::GetInstance()->HwRemoveAdvKey(RawAddress(device->GetAddress()));
}

void CloudDeviceManager::HandleCloudPairAclConnectTimeOut(const RawAddress &device)
{
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter != nullptr && classicAdapter->IsCloudDevice(device) &&
        classicAdapter->GetCloudBondState(device) == CLOUD_BOND_BONDING) {
        classicAdapter->SetCloudBondState(device, CLOUD_BOND_NONE);
        classicAdapter->NotifyDisconnectState(
            device, static_cast<int>(ConnChangeCause::DISCONNECT_CAUSE_CONNECT_FAIL_INTERNAL));
        BtChrUeManager::GetInstance()->HwChrReportFinishCloudPair(device,
            static_cast<int>(UeCloudPairSceneCode::CONNECT_DOWN_CLOUD_DEV),
            static_cast<int>(UeConnCloudPairSubSceneCode::ACL_CONN_TIMEOUT));
    }
    HILOGI("%{public}s", GET_ENCRYPT_STR_ADDR(device.GetAddress()));
}

bool CloudDeviceManager::IsValidAddress(const std::string &addr)
{
    if (addr.empty() || addr.length() != ADDRESS_LENGTH) {
        return false;
    }
    for (size_t i = 0; i < ADDRESS_LENGTH; i++) {
        char c = addr[i];
        switch (i % ADDRESS_SEPARATOR_UNIT) {
            case 0:
            case 1:
                if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
                    break;
                }
                return false;
            case ADDRESS_COLON_INDEX:
            default:
                if (c == ':') {
                    break;
                }
                return false;
        }
    }
    return true;
}

void CloudDeviceManager::StartBtAclTimer(const RawAddress &device)
{
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter == nullptr) {
        HILOGE("classicAdapter is nullptr");
        return;
    }
    if (!IsValidAddress(device.GetAddress()) || !classicAdapter->IsCloudDevice(device)) {
        HILOGE("device is invalid, device:%{public}s, IsCloudDevice:%{public}d",
            GET_ENCRYPT_STR_ADDR(device.GetAddress()), classicAdapter->IsCloudDevice(device));
        return;
    }
    std::lock_guard<std::mutex> lock(btAclTimeoutMutex_);
    auto it = btAclTimeoutMap_.find(device.GetAddress());
    if (it != btAclTimeoutMap_.end()) {
        it = btAclTimeoutMap_.erase(it);
    }
    std::shared_ptr<utility::Timer> btAclTimeout = std::make_shared<utility::Timer>(
        [this, device] {this->HandleCloudPairAclConnectTimeOut(device);});
    btAclTimeout->Start(ACL_TIMEOUT_TIME);
    btAclTimeoutMap_.insert(std::make_pair(device.GetAddress(), btAclTimeout));
    HILOGI("%{public}s, time:%{public}d ms",
        GET_ENCRYPT_STR_ADDR(device.GetAddress()), ACL_TIMEOUT_TIME);
}

void CloudDeviceManager::StopBtAclTimer(const RawAddress &device)
{
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter == nullptr) {
        HILOGE("classicAdapter is nullptr");
        return;
    }
    if (!IsValidAddress(device.GetAddress()) || !classicAdapter->IsCloudDevice(device)) {
        HILOGI("device is invalid, device:%{public}s, IsCloudDevice:%{public}d",
            GET_ENCRYPT_STR_ADDR(device.GetAddress()), classicAdapter->IsCloudDevice(device));
        return;
    }
    std::lock_guard<std::mutex> lock(btAclTimeoutMutex_);
    auto it = btAclTimeoutMap_.find(device.GetAddress());
    if (it != btAclTimeoutMap_.end()) {
        if (it->second != nullptr) {
            it->second->Stop();
            it->second = nullptr;
        }
        it = btAclTimeoutMap_.erase(it);
        HILOGI("[CLOUD_PAIR] stop acl timer device address: %{public}s", GET_ENCRYPT_STR_ADDR(device.GetAddress()));
        return;
    }
    HILOGW("not find the device! device:%{public}s", GET_ENCRYPT_STR_ADDR(device.GetAddress()));
}

bool CloudDeviceManager::IsAnyDeviceInCloudPairing()
{
    auto classicProperties = AdapterProperties::GetInstance();
    CHECK_AND_RETURN_LOG_RET(classicProperties != nullptr, false, "classicProperties is null");
    auto remoteDeviceProperties = RemoteDeviceProperties::GetInstance();
    CHECK_AND_RETURN_LOG_RET(remoteDeviceProperties != nullptr, false, "remoteDeviceProperties is null");
    std::vector<std::string> pairedAddrList = classicProperties->GetPairedAddrList();
    for (auto &pairedAddr : pairedAddrList) {
        std::string addr = pairedAddr;
        if (addr.empty() || addr == INVALID_MAC_ADDRESS) {
            continue;
        } else {
            RawAddress device = RawAddress(addr);
            std::shared_ptr<BluetoothDevice> remoteDevice = remoteDeviceProperties->GetBluetoothDeviceFromMap(device);
            if (remoteDevice == nullptr) {
                HILOGW("can not find remoteDevice, device:%{public}s", GET_ENCRYPT_STR_ADDR(addr));
                continue;
            }
            int pairStatus = remoteDevice->GetPairedStatus();
            int32_t cloudBondState = remoteDeviceProperties->GetCloudBondState(device);

            if (cloudBondState == CLOUD_BOND_BONDING || (pairStatus == PAIR_PAIRING &&
                (cloudBondState == CLOUD_BOND_CREATE_BOND || cloudBondState == CLOUD_BOND_BONDED))) {
                return true; 
            }
        }
    }
    return false;  
}

} // namespace bluetooth
} // namespace OHOS
