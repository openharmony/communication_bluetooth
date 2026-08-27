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
#define LOG_TAG "bt_service_adapter_properties"
#endif

#include "adapter_properties.h"

#include "bluetooth_common_event_helper.h"
#include "bluetooth_datashare_utils.h"
#include "bluetooth_os_account.h"
#include "bt_chr_ue_manager.h"
#include "common_util.h"
#include "hal_util.h"
#include "log.h"
#include "permission_manager.h"
#include "securec.h"
#include "service_util.h"
#include "thread_util.h"
#include "adapter_manager.h"
#if (defined(DEVICE_MANAGER) && !defined(BLUETOOTH_WATCH_ENABLE))
#include "device_manager_mini.h"
#endif

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;

sptr<BluetoothObserver> g_nameChangeObserver;
AdapterProperties::AdapterProperties() : config_(ClassicConfig::GetInstance())
{}

AdapterProperties::~AdapterProperties()
{}

AdapterProperties* AdapterProperties::GetInstance(void)
{
    static AdapterProperties singleton;
    return &singleton;
}

bool AdapterProperties::LoadConfigInfo()
{
    if (!LoadLocalName()) {
        HILOGE("load settings data to get devive name failed");
    }
    LoadHostInfo();
    return true;
}

bool AdapterProperties::LoadLocalName()
{
    if (!RegisterNameChangeObserver()) {
        HILOGE("RegisterNameChangeObserver failed");
    };
#if (defined(DEVICE_MANAGER) && !defined(BLUETOOTH_WATCH_ENABLE) && !defined(BT_MCU_PROXY_ENABLE))
    int32_t ret = DistributedHardware::DeviceManagerMini::GetInstance().InitDeviceManager("bluetooth_service");
    if (ret != DM_OK) {
        HILOGE("InitDeviceManager error: %{public}d", ret);
    }
#endif

    std::string value = "";
    if (!GetSettingDeviceName(value)) {
        HILOGI(":Query device name fail");
        StartQueryDeviceNameTimer();
        return false;
    } else {
        if (value.empty()) {
            HILOGE("deviceName empty");
            StartQueryDeviceNameTimer();
            return false;
        }
        HILOGI("set deviceName: %{public}s", GET_ENCRYPT_DEVICE_NAME(value));
        SetDeviceName(value);
    }
    return true;
}

bool AdapterProperties::RegisterNameChangeObserver()
{
    auto dataShareHelperUtils = std::make_unique<BluetoothDataShareHelperUtils>();
    if (dataShareHelperUtils == nullptr) {
        HILOGI("settingHelper is null");
        return false;
    }
    int32_t osAccountId = BluetoothOsAccount::GetActiveOsAccountId();
    std::string osaccountIdStr = "";
    if (osAccountId == INVALID_ID) {
        HILOGE("Invalid osAccountId");
    } else {
        osaccountIdStr = std::to_string(osAccountId);
    }
    const std::string proxySuffix = "?Proxy=true";
    Uri uri = Uri(SETTINGS_DATASHARE_URL_DEVICE_NAME + osaccountIdStr +
        proxySuffix + "&key=" + SETTINGS_DATASHARE_KEY_DEVICE_NAME);
    auto onChange = []() {
        AdapterProperties::GetInstance()->HandleNameChange();
    };
    g_nameChangeObserver = new BluetoothObserver(onChange);
    if (!dataShareHelperUtils->RegisterObserver(uri, g_nameChangeObserver)) {
        HILOGI("RegisterObserver failed");
        return false;
    }
    return true;
}

void AdapterProperties::HandleNameChange()
{
    std::string displayName = "";
    if (!GetSettingDeviceName(displayName)) {
        HILOGE("GetSettingDeviceName failed");
    }
    if (displayName == deviceName_) {
        HILOGE("same deviceName");
    } else {
        AdapterProperties::GetInstance()->SetDeviceName(displayName);
    }
}

void AdapterProperties::UnregisterNameChangeObserver()
{
    auto dataShareHelperUtils = std::make_unique<BluetoothDataShareHelperUtils>();
    if (dataShareHelperUtils == nullptr) {
        HILOGI("settingHelper is null");
        return;
    }
    int32_t osAccountId = BluetoothOsAccount::GetActiveOsAccountId();
    std::string osaccountIdStr = "";
    if (osAccountId == INVALID_ID) {
        HILOGE("Invalid osAccountId");
    } else {
        osaccountIdStr = std::to_string(osAccountId);
    }
    const std::string proxySuffix = "?Proxy=true";
    Uri uri = Uri(SETTINGS_DATASHARE_URL_DEVICE_NAME + osaccountIdStr +
        proxySuffix + "&key=" + SETTINGS_DATASHARE_KEY_DEVICE_NAME);
    if (!dataShareHelperUtils->UnregisterObserver(uri, g_nameChangeObserver)) {
        HILOGI("UnregisterObserver failed");
    }
    g_nameChangeObserver = nullptr;
}

void AdapterProperties::LoadHostInfo()
{
    std::string deviceName;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        deviceName = deviceName_;
    }
    if (deviceName.empty()) {
        HILOGE("deviceName empty");
    } else {
        HILOGI("set deviceName");
        SetDeviceName(deviceName);
    }

    int configCod = DEFAULT_CLASS_OF_DEVICE;
    configCod = (config_.GetLocalDeviceClass() & CLASS_OF_DEVICE_RANGE);
    if (configCod != DEFAULT_CLASS_OF_DEVICE && configCod != cod_.load()) {
        cod_.store(configCod);
    }
    if (cod_.load() < INVALID_VALUE) {
        cod_.store(DEFAULT_CLASS_OF_DEVICE);
    }

    int configIoCapability = config_.GetIoCapability();
    HILOGI("configIoCapability = %d", configIoCapability);
    if (configIoCapability != GAP_IO_DISPLAYYESNO && configIoCapability != ioCapability_.load()) {
        ioCapability_.store(configIoCapability);
    }
    if ((ioCapability_.load() < GAP_IO_DISPLAYONLY) || (ioCapability_.load() > GAP_IO_NOINPUTNOOUTPUT)) {
        ioCapability_.store(GAP_IO_DISPLAYYESNO);
    }

    HILOGI("Get Host info:");
    HILOGI("Class of device is: %{public}d", cod_.load());
    HILOGI("IoCapability is: %{public}d", ioCapability_.load());
    SetIoCapability(ioCapability_.load());
}

bool AdapterProperties::GetSettingDeviceName(std::string &value)
{
    auto dataShareHelperUtils = std::make_unique<BluetoothDataShareHelperUtils>();
    if (dataShareHelperUtils == nullptr) {
        HILOGI("settingHelper is null");
        return false;
    }
    int32_t osAccountId = BluetoothOsAccount::GetActiveOsAccountId();
    Uri uri("");
    std::string key = "";
    if (osAccountId == INVALID_ID) {
        // 拿不到OSAccountID的时候去获取默认设备名称
        uri = Uri(SETTINGS_DATASHARE_URI_DEFAULT_DEVICE_NAME);
        key = SETTINGS_DATASHARE_KEY_DEFAULT_DEVICE_NAME;
    } else {
        const std::string proxySuffix = "?Proxy=true";
        uri = Uri(SETTINGS_DATASHARE_URL_DEVICE_NAME + std::to_string(osAccountId) + proxySuffix);
        key = SETTINGS_DATASHARE_KEY_DEVICE_NAME;
    }
    if (dataShareHelperUtils->Query(uri, key, value) != RET_NO_ERROR) {
        return false;
    }
    return true;
}

bool AdapterProperties::SetDeviceName(std::string deviceName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (deviceName.empty()) {
        HILOGE("failed, because of deviceName is NULL!");
        return false;
    }
    if (deviceName_ == deviceName) {
        HILOGW("same deviceName!");
    }

    std::string callingName = PermissionManager::GetCallingName();
    BtChrUeManager::GetInstance()->WriteSetLocalNameUe(deviceName, callingName);
    if (deviceName.length() > MAX_LOC_BT_SERVICE_NAME_LEN) {
        deviceName_ = deviceName.substr(0, MAX_LOC_BT_SERVICE_NAME_LEN);
    } else {
        deviceName_ = deviceName;
    }
    return SetBroadcastName(deviceName);
}

bool AdapterProperties::SetBroadcastName(std::string deviceName)
{
    std::string name = "";
#if (defined(DEVICE_MANAGER) && !defined(BLUETOOTH_WATCH_ENABLE))
    int32_t ret = DistributedHardware::DeviceManagerMini::GetInstance().GetLocalDisplayDeviceName(
        "bluetooth_service", MAX_LOC_BT_NAME_LEN, name);
    if (ret != DM_OK) {
        HILOGW("GetLocalDisplayDeviceName error: %{public}d, fallback to deviceName", ret);
        name = GetTruncationName(deviceName);
    }
#else
    name = GetTruncationName(deviceName);
#endif
    unsigned int length = name.length();
    bt_property_t property;
    property.type = static_cast<bt_property_type_t>(STACK::BT_PROPERTY_BDNAME);
    property.len = static_cast<int>(length);
    property.val = name.data();
    const bt_interface_t *btInterface = nullptr;
    int status = hal_util_load_bt_library(&btInterface);
    if (status) {
        HILOGE("[ClassicAdapter] Failed to open the Bluetooth module");
        return false;
    }
    int result = btInterface->set_adapter_property(&property);
    if (result != BT_STATUS_SUCCESS) {
        HILOGE("SetBroadcastName fail");
        return false;
    }
    return true;
}

std::string AdapterProperties::GetDeviceName(void) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return deviceName_;
}

void AdapterProperties::SetDeviceAddress(std::string deviceAddress)
{
    std::lock_guard<std::mutex> lock(mutex_);
    macAddr_ = deviceAddress;
}

std::string AdapterProperties::GetDeviceAddress(void) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return macAddr_;
}

void AdapterProperties::SetServiceUuids(std::vector<Uuid> uuids)
{
    std::lock_guard<std::mutex> lock(mutex_);
    uuids_.clear();
    uuids_ = uuids;
}

std::vector<Uuid> AdapterProperties::GetServiceUuids(void) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return uuids_;
}

bool AdapterProperties::SetDeviceClass(int deviceClass)
{
    bt_property_t property;
    property.len = sizeof(int);
    property.type = (bt_property_type_t)STACK::BT_PROPERTY_CLASS_OF_DEVICE;
    property.val = &deviceClass;
    HILOGI("deviceClass = %d", deviceClass);
    const bt_interface_t *btInterface = nullptr;
    int status = hal_util_load_bt_library(&btInterface);
    if (status) {
        HILOGE("[ClassicAdapterProperties] Failed to open the Bluetooth module");
        return false;
    }
    int result = btInterface->set_adapter_property(&property);
    if (result != BT_STATUS_SUCCESS) {
        HILOGE("SetLocalDeviceClass fail");
        return false;
    }
    cod_.store(deviceClass);
    return true;
}

int AdapterProperties::GetDeviceClass(void) const
{
    return cod_.load();
}

void AdapterProperties::SetDeviceType(uint32_t deviceType)
{
    std::lock_guard<std::mutex> lock(mutex_);
    deviceType_ = deviceType;
}

uint32_t AdapterProperties::GetDeviceType(void) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return deviceType_;
}

std::vector<std::string> AdapterProperties::GetPairedAddrList(void) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return pairedAddrList_;
}

std::vector<std::string> AdapterProperties::AddPairedAddrList(RawAddress &device)
{
    HILOGI("addr = %{public}s ", GetEncryptAddr(device.GetAddress()).c_str());
    std::string addr = device.GetAddress();
    std::lock_guard<std::mutex> lock(mutex_);
    pairedAddrList_.push_back(addr);
    return pairedAddrList_;
}

void AdapterProperties::RemovePairedDeviceList(std::string addr)
{
    HILOGI("addr: %{public}s", GetEncryptAddr(addr).c_str());
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = pairedAddrList_.begin(); it != pairedAddrList_.end(); ++it) {
        if (*it == addr) {
            pairedAddrList_.erase(it);
            break;
        }
    }
}

bool AdapterProperties::SetIoCapability(int ioCapability)
{
    bt_property_t property;
    property.len = sizeof(int);
    property.type = (bt_property_type_t)STACK::BT_PROPERTY_LOCAL_IO_CAPS;
    property.val = &ioCapability;
    HILOGI("ioCapability: %{public}d", ioCapability);
    const bt_interface_t *btInterface = nullptr;
    int status = hal_util_load_bt_library(&btInterface);
    if (status) {
        HILOGE("[ClassicAdapterProperties] Failed to open the Bluetooth module");
        return false;
    }
    int result = btInterface->set_adapter_property(&property);
    if (result != BT_STATUS_SUCCESS) {
        HILOGE("SetBtScanMode fail");
        return false;
    }
    ioCapability_.store(ioCapability);
    return true;
}

uint8_t AdapterProperties::GetIoCapability(void) const
{
    return ioCapability_.load();
}

uint16_t AdapterProperties::GetTotalNumOfTrackableAdvertisements(void) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return localLeFeatures_.total_trackable_advertisers;
}

uint16_t AdapterProperties::GetBleMaxAdvertisingDataLength(void) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return localLeFeatures_.le_maximum_advertising_data_length;
}

bool AdapterProperties::GetLeExtendedAdvertisingSupported(void) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return localLeFeatures_.le_extended_advertising_supported;
}

bool AdapterProperties::GetLe2mPhySupported(void) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return localLeFeatures_.le_2m_phy_supported;
}

void AdapterProperties::HandlePropertyLocalAddress(bt_property_t* property)
{
    RawAddress address = ParseDeviceAddr(property);
    std::string localAddress = address.GetAddress();
    HILOGD("addr = %{public}s", GET_ENCRYPT_STR_ADDR(localAddress));
    std::lock_guard<std::mutex> lock(mutex_);
    macAddr_ = localAddress;
}

void AdapterProperties::HandlePropertyScanMode(bt_property_t* property)
{
    int curScanMode = static_cast<int>(BT_SCAN_MODE_NONE);
    if (ParseScanMode(property, curScanMode)) {
        BluetoothHelper::BluetoothCommonEventHelper::PublishScanModeChangeEvent(curScanMode);
        auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
        if (classicAdapter) {
            HILOGI("classicAdapter save scanMode");
            classicAdapter->SendScanModeChanged(curScanMode);
        }
    }
}

void AdapterProperties::ParseAdapterProps(bt_status_t status, int numProperties, bt_property_t* properties)
{
    if (status != BT_STATUS_SUCCESS || properties == nullptr) {
        HILOGE("ParseAdapterProps fail, status is %{public}d", status);
        return;
    }

    for (int i = 0; i < numProperties; i++) {
        bt_property_t* property = properties + i;
        switch (property->type) {
            case BT_PROPERTY_BDADDR: {
                HandlePropertyLocalAddress(property);
                break;
            }
            case BT_PROPERTY_UUIDS: {
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    uuids_ = ParseDeviceUuid(property);
                }
                break;
            }
            case STACK::BT_PROPERTY_CLASS_OF_DEVICE: {
                cod_.store((int)ParseDeviceCod(property));
                break;
            }
            case BT_PROPERTY_LOCAL_IO_CAPS: {
                ioCapability_.store(ParseDeviceIoCapability(property));
                break;
            }
            case BT_PROPERTY_ADAPTER_SCAN_MODE: {
                HandlePropertyScanMode(property);
                break;
            }
            case BT_PROPERTY_ADAPTER_BONDED_DEVICES: {
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    pairedAddrList_ = ParseBondedDevices(property);
                }
                break;
            }
            case BT_PROPERTY_LOCAL_LE_FEATURES:
                ParseLocalLeFeatures(property);
                break;
            default:
                HILOGD("Unhandled adapter property, case = %{public}x", property->type);
                break;
        }
    }
}

std::string AdapterProperties::ParseDeviceName(bt_property_t* property)
{
    if (property->len < 0) {
        HILOGE("invalid name");
        return INVALID_NAME;
    }
    if (property->val == nullptr) {
        HILOGE("Invalid pointer");
        return INVALID_NAME;
    }
    int len = property->len;
    if (len > MAX_REMOTE_BT_NAME_LEN) {
        HILOGI("name is too long");
        len = MAX_REMOTE_BT_NAME_LEN;
    }
    bt_bdname_t* hal_name = reinterpret_cast<bt_bdname_t*>(property->val);
    std::string name(hal_name->name, hal_name->name + len);
    return name;
}

RawAddress AdapterProperties::ParseDeviceAddr(bt_property_t* property)
{
    if (property->len != sizeof(STACK::RawAddress)) {
        HILOGE("Invalid length for BT_PROPERTY_BDADDR");
        return RawAddress(INVALID_MAC_ADDRESS);
    }
    if (property->val != nullptr) {
        RawAddress address =  ServiceUtil::AddrFromStack(*reinterpret_cast<STACK::RawAddress*>(property->val));
        return address;
    } else {
        HILOGE("Invalid pointer");
        return RawAddress(INVALID_MAC_ADDRESS);
    }
}

std::vector<Uuid> AdapterProperties::ParseDeviceUuid(bt_property_t* property)
{
    std::vector<Uuid> serviceUuids;
    if (property->val == nullptr) {
        HILOGE("Invalid pointer");
        return serviceUuids;
    }
    if (property->len < 0) {
        HILOGE("Negative length on BT_PROPERTY_UUIDS");
        return serviceUuids;
    }
    if (property->len % sizeof(STACK::bluetooth::Uuid) != 0) {
        HILOGE("Trailing bytes on BT_PROPERTY_UUIDS");
        return serviceUuids;
    }
    auto bluedriodUuids = static_cast<const STACK::bluetooth::Uuid*>(property->val);

    for (size_t i = 0; i < property->len / sizeof(STACK::bluetooth::Uuid); ++i) {
        Uuid uuid = ServiceUtil::UuidFromStack(bluedriodUuids[i]);
        serviceUuids.push_back(uuid);
        HILOGI("uuid_%{public}d: %{public}s", i, uuid.ToString().c_str());
    }
    return serviceUuids;
}

uint32_t AdapterProperties::ConvertDeviceTypeFromBluetdroid(uint32_t deviceType)
{
    if (deviceType == BT_DEVICE_TYPE_BREDR) {
        return DEVICE_TYPE_BREDR;
    }
    if (deviceType == BT_DEVICE_TYPE_BLE) {
        return DEVICE_TYPE_LE;
    }
    if (deviceType == BT_DEVICE_TYPE_DUMO) {
        return DEVICE_TYPE_DUAL_MONO;
    }
    return DEVICE_TYPE_UNKNOWN;
}

uint32_t AdapterProperties::ParseDeviceType(bt_property_t* property)
{
    if (property->len != sizeof(uint32_t)) {
        HILOGE("Invalid length for BT_PROPERTY_TYPE_OF_DEVICE");
        return INVALID_VALUE;
    }
    if (property->val != nullptr) {
        uint32_t deviceType = *reinterpret_cast<const uint32_t*>(property->val);
        deviceType = ConvertDeviceTypeFromBluetdroid(deviceType);
        return deviceType;
    } else {
        HILOGE("Invalid pointer");
        return INVALID_VALUE;
    }
}

uint32_t AdapterProperties::ParseDeviceCod(bt_property_t* property)
{
    if (property->len != sizeof(uint32_t)) {
        HILOGE("Invalid length for BT_PROPERTY_CLASS_OF_DEVICE");
        return INVALID_VALUE;
    }
    if (property->val != nullptr) {
        uint32_t deviceClass = *reinterpret_cast<const uint32_t*>(property->val);
        return deviceClass;
    } else {
        HILOGE("Invalid pointer");
        return INVALID_VALUE;
    }
}

int8_t AdapterProperties::ParseDeviceRssi(bt_property_t* property)
{
    if (property->len != sizeof(int8_t) && property->len != sizeof(uint8_t)) {
        HILOGE("Invalid length %{public}d for BT_PROPERTY_REMOTE_RSSI", property->len);
        return INVALID_VALUE;
    }
    if (property->val != nullptr) {
        int8_t rssi = *reinterpret_cast<const int8_t*>(property->val);
        return rssi;
    } else {
        HILOGE("Invalid pointer");
        return INVALID_VALUE;
    }
}

int AdapterProperties::ParseDeviceIoCapability(bt_property_t* property)
{
    if (property->len != sizeof(int)) {
        HILOGE("Invalid length for BT_PROPERTY_CLBT_PROPERTY_REMOTE_RSSIASS_OF_DEVICE");
        return INVALID_VALUE;
    }
    if (property->val != nullptr) {
        int ioCapability = *reinterpret_cast<const int*>(property->val);
        return ioCapability;
    } else {
        HILOGE("Invalid pointer");
        return INVALID_VALUE;
    }
}

bool AdapterProperties::ParseScanMode(bt_property_t* property, int &scanMode)
{
    if (property->len != sizeof(bt_scan_mode_t) || property->val == nullptr) {
        HILOGE("Invalid length or value for BT_PROPERTY_ADAPTER_SCAN_MODE");
        return false;
    }
    int modeFromStack = *reinterpret_cast<const int*>(property->val);
    switch (modeFromStack) {
        case BT_SCAN_MODE_CONNECTABLE:
            scanMode =  SCAN_MODE_CONNECTABLE;
            break;
        case BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE:
            scanMode = SCAN_MODE_CONNECTABLE_GENERAL_DISCOVERABLE;
            break;
        case BT_SCAN_MODE_NONE:
        default:
            scanMode = SCAN_MODE_NONE;
            break;
    }
    HILOGD("scanMode: %{public}d", scanMode);
    return true;
}

std::vector<std::string> AdapterProperties::ParseBondedDevices(bt_property_t* property)
{
    HILOGD("BT_PROPERTY_ADAPTER_BONDED_DEVICES");
    if (property->len < 0 || property->len % BD_ADDR_LEN != 0) {
        HILOGE("wrong len !");
        return pairedAddrList_;
    }
    int num = property->len / BD_ADDR_LEN;
    auto addr = reinterpret_cast<const STACK::RawAddress*>(property->val);
    std::vector<std::string> pairedAddrList;

    if (property->val == nullptr) {
        HILOGE("Invalid pointer");
        return pairedAddrList_;
    }
    for (int i = 0; i < num; ++i) {
        RawAddress address = ServiceUtil::AddrFromStack(addr[i]);
        HILOGD("BondedDevices = %{public}s", GetEncryptAddr(address.GetAddress()).c_str());
        pairedAddrList.push_back(address.GetAddress());
    }
    return pairedAddrList;
}

int32_t AdapterProperties::ParseAbsVolumeAbility(bt_property_t* property)
{
    if (property->len < static_cast<int>(sizeof(int32_t))) {
        HILOGE("Invalid length %{public}d for HW_BT_PROPERTY_ABS_VOLUM_KEY", property->len);
        return DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT;
    }
    if (property->val == nullptr) {
        HILOGE("Invalid pointer");
        return DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT;
    }
    // Unknown ability are treated as unsupported
    int32_t absVolumeAbility = *reinterpret_cast<const int32_t*>(property->val);
    if (absVolumeAbility != DeviceAbsVolumeAbility::DEVICE_ABSVOL_OPEN &&
        absVolumeAbility != DeviceAbsVolumeAbility::DEVICE_ABSVOL_CLOSE) {
        absVolumeAbility = DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT;
    }
    return absVolumeAbility;
}

int32_t AdapterProperties::ParseDeviceCustomType(bt_property_t* property)
{
    if (property->len < static_cast<int>(sizeof(int32_t))) {
        HILOGE("Invalid length %{public}d for DeviceCustomType", property->len);
        return 0;
    }
    if (property->val == nullptr) {
        HILOGE("Invalid pointer");
        return 0;
    }
    int32_t customType = *reinterpret_cast<const int32_t*>(property->val);
    HILOGI("customType is %{public}d", customType);
    return customType;
}

int64_t AdapterProperties::ParseDeviceConnectionTime(bt_property_t* property)
{
    if (property->len < INVALID_VALUE) {
        HILOGE("Invalid length for HW_BT_PROPERTY_RMT_IO_CAP_KEY");
        return INVALID_VALUE;
    }
    if (property->val == nullptr) {
        HILOGE("Invalid pointer");
        return 0;
    }
    int64_t connectionTime = *reinterpret_cast<const int64_t*>(property->val);
    HILOGI("connectionTime is %{public}ld", connectionTime);
    return connectionTime;
}

void AdapterProperties::ParseLocalLeFeatures(bt_property_t* property)
{
    if (property->len != sizeof(bt_local_le_features_t) || property->val == nullptr) {
        HILOGW("Malformed value received for property: BT_PROPERTY_LOCAL_LE_FEATURES");
        return;
    }
    bt_local_le_features_t* features = reinterpret_cast<bt_local_le_features_t*>(property->val);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        memcpy_s(&localLeFeatures_, sizeof(localLeFeatures_), features, sizeof(*features));
    }
    HILOGI("Supported LE features updated");
}

std::string AdapterProperties::ParseAlias(bt_property_t* property)
{
    if (property->len < 0) {
        HILOGE("invalid name");
        return INVALID_NAME;
    }
    if (property->val == nullptr) {
        HILOGE("Invalid pointer");
        return INVALID_NAME;
    }
    int len = property->len;
    if (len > MAX_ALIAS_LENGTH) {
        HILOGI("name is too long");
        len = MAX_ALIAS_LENGTH;
    }
    bt_bdname_t* hal_name = reinterpret_cast<bt_bdname_t*>(property->val);
    std::string alias(hal_name->name, hal_name->name + len);
    return alias;
}

int32_t AdapterProperties::ParseRemoteDeviceIoCapability(bt_property_t* property)
{
    if (property->len < INVALID_VALUE) {
        HILOGE("Invalid length for HW_BT_PROPERTY_RMT_IO_CAP_KEY");
        return INVALID_VALUE;
    }
    if (property->val != nullptr) {
        int32_t ioCapability = *reinterpret_cast<const int32_t*>(property->val);
        return ioCapability;
    } else {
        HILOGE("Invalid pointer");
        return INVALID_VALUE;
    }
}

int32_t AdapterProperties::ParseDeviceVendorId(bt_property_t* property)
{
    if (property->len != sizeof(int32_t)) {
        HILOGE("Invalid length for BT_PROPERTY_VENDOR_ID");
        return INVALID_VALUE;
    }
    if (property->val != nullptr) {
        int32_t vendorId = *reinterpret_cast<const int32_t*>(property->val);
        return vendorId;
    } else {
        HILOGE("Invalid pointer");
        return INVALID_VALUE;
    }
}

int32_t AdapterProperties::ParseDeviceProductId(bt_property_t* property)
{
    if (property->len != sizeof(int32_t)) {
        HILOGE("Invalid length for BT_PROPERTY_PRODUCT_ID");
        return INVALID_VALUE;
    }
    if (property->val != nullptr) {
        int32_t productId = *reinterpret_cast<const int32_t*>(property->val);
        return productId;
    } else {
        HILOGE("Invalid pointer");
        return INVALID_VALUE;
    }
}

int32_t AdapterProperties::ParseDeviceAutoConnSwitch(bt_property_t* property)
{
    if (property->len != sizeof(int32_t)) {
        HILOGE("Invalid length for BT_PROPERTY_AUTO_CONNECT_SWITCH");
        return INVALID_VALUE;
    }
    if (property->val != nullptr) {
        int32_t autoConnSwitch = *reinterpret_cast<const int32_t*>(property->val);
        return autoConnSwitch;
    } else {
        HILOGE("Invalid pointer");
        return INVALID_VALUE;
    }
}

std::string AdapterProperties::GetTruncationName(const std::string &deviceName)
{
    unsigned int length = deviceName.length();
    if (length <= MAX_LOC_BT_NAME_LEN) {
        return deviceName;
    }

    // 字符串长度超过30字节后，截取完整的且小于30个字节的UTF-8的字符串,截断后不满30个字节时用'.'补齐30字节
    std::string ellipsis = ".";
    int limitLength = GetValidUTF8StringLength(deviceName);
    HILOGI("limitLength = %{public}d", limitLength);
    std::string newDeviceName =  deviceName.substr(0, limitLength);
    for (int i = (MAX_LOC_BT_NAME_LEN - limitLength); i > 0; i--) {
        newDeviceName += ellipsis;
    }
    return newDeviceName;
}

int AdapterProperties::GetUTF8StringLength(const char firstByte)
{
    // 根据首字节获取UTF8字符占用的字节数量
    int length = UTF8_SINGLE_BYTE_LENGTH;
    if ((firstByte & 0x80) == 0) {
        length = UTF8_SINGLE_BYTE_LENGTH;
    } else if ((firstByte & 0xE0) == 0xC0) {
        length = UTF8_DOUBLE_BYTE_LENGTH;
    } else if ((firstByte & 0xF0) == 0xE0) {
        length = UTF8_TRIPLE_BYTE_LENGTH;
    } else if ((firstByte & 0xF8) == 0xF0) {
        length = UTF8_QUADRUPLE_BYTE_LENGTH;
    }
    return length;
}

int AdapterProperties::GetValidUTF8StringLength(const std::string &name)
{
    int byteCount = 0;
    size_t stringSize = name.size();
    for (size_t i = 0; i < stringSize;) {
        int utf8Length = GetUTF8StringLength(name[i]);
        if (byteCount + utf8Length > MAX_LOC_BT_NAME_LEN) {
            break;
        }
        byteCount += utf8Length;
        if (byteCount == MAX_LOC_BT_NAME_LEN) {
            return byteCount;
        }
        i += static_cast<size_t>(utf8Length);
    }
    return byteCount;
}

void QueryDeviceNameTimeoutCallback()
{
    AdapterProperties *adapterProperties = AdapterProperties::GetInstance();
    adapterProperties->LoadLocalName();
}

void AdapterProperties::StartQueryDeviceNameTimer()
{
#ifndef BT_MCU_PROXY_ENABLE
    HILOGI("Start Query DeviceName Timer");
    queryDeviceNameTimeout_ = std::make_shared<utility::Timer>(&QueryDeviceNameTimeoutCallback);
    queryDeviceNameTimeout_->Start(delayQueryTime);
#else
    HILOGW("Glassess Product not support.");
#endif
}
}  // namespace bluetooth
}  // namespace OHOS
