/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "taihe_bluetooth_ble_utils.h"
#include "bluetooth_utils.h"
#include "bluetooth_log.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "bundle_mgr_proxy.h"

namespace OHOS {
namespace Bluetooth {

void Util::TaiheToBleAdvertiserSettings(BleAdvertiserSettings &advertiserSettings,
                                        ohos::bluetooth::ble::AdvertiseSetting advertiseSettingsTaihe)
{
    if (advertiseSettingsTaihe.interval.has_value()) {
        advertiserSettings.SetInterval(static_cast<uint16_t>(advertiseSettingsTaihe.interval.value()));
    }

    if (advertiseSettingsTaihe.txPower.has_value()) {
        advertiserSettings.SetTxPower(static_cast<int8_t>(advertiseSettingsTaihe.txPower.value()));
    }

    if (advertiseSettingsTaihe.connectable.has_value()) {
        advertiserSettings.SetConnectable(static_cast<bool>(advertiseSettingsTaihe.connectable.value()));
    }
}

void Util::TaiheToBleAdvertiserData(BleAdvertiserData &advertiserData,
                                    ohos::bluetooth::ble::AdvertiseData advertiseDataTaihe)
{
    std::vector<std::string> valueInner(
        advertiseDataTaihe.serviceUuids.begin(), advertiseDataTaihe.serviceUuids.end());
    for (auto it = valueInner.begin(); it != valueInner.end(); it++) {
        UUID serviceUuid = UUID::FromString(static_cast<std::string>(*it));
        advertiserData.AddServiceUuid(serviceUuid);
    }

    std::map<uint16_t, std::string> manufacturData;
    TaiheToBleManufactureData(manufacturData, advertiseDataTaihe.manufactureData);
    for (auto iter = manufacturData.begin(); iter != manufacturData.end(); iter++) {
        advertiserData.AddManufacturerData(iter->first, iter->second);
    }

    std::map<ParcelUuid, std::string> serviceData;
    TaiheToBleServiceData(serviceData, advertiseDataTaihe.serviceData);
    for (auto iter = serviceData.begin(); iter != serviceData.end(); iter++) {
        advertiserData.AddServiceData(iter->first, iter->second);
    }
    if (advertiseDataTaihe.includeDeviceName.has_value()) {
        advertiserData.SetIncludeDeviceName(static_cast<bool>(advertiseDataTaihe.includeDeviceName.value()));
    }
    if (advertiseDataTaihe.includeTxPower.has_value()) {
        advertiserData.SetIncludeTxPower(static_cast<bool>(advertiseDataTaihe.includeTxPower.value()));
    }
}

void Util::TaiheToBleManufactureData(std::map<uint16_t, std::string> &manufactureData,
                                     taihe::array<ohos::bluetooth::ble::ManufactureData> manufactureDataTaihe)
{
    std::vector<const ohos::bluetooth::ble::ManufactureData> manufactureDataTaiheVec(
        manufactureDataTaihe.begin(), manufactureDataTaihe.end());
    for (auto dataTaihe : manufactureDataTaiheVec) {
        uint16_t manufactureId = static_cast<uint16_t>(dataTaihe.manufactureId);
        std::string manufactureValue;
        std::vector<uint8_t> valueInner(
            dataTaihe.manufactureValue.begin(), dataTaihe.manufactureValue.end());
        for (auto value : valueInner) {
            manufactureValue.push_back(static_cast<char>(value));
        }
        manufactureData.insert(std::make_pair(manufactureId, manufactureValue));
    }
}

void Util::TaiheToBleServiceData(std::map<ParcelUuid, std::string> &serviceData,
                                 taihe::array<ohos::bluetooth::ble::ServiceData> serviceDataTaihe)
{
    std::vector<const ohos::bluetooth::ble::ServiceData> serviceDataTaiheVec(
        serviceDataTaihe.begin(), serviceDataTaihe.end());
    for (auto dataTaihe : serviceDataTaiheVec) {
        UUID serviceUuid = UUID::FromString(static_cast<std::string>(dataTaihe.serviceUuid));
        std::string serviceValue;
        std::vector<uint8_t> valueInner(
            dataTaihe.serviceValue.begin(), dataTaihe.serviceValue.end());
        for (auto value : valueInner) {
            serviceValue.push_back(static_cast<char>(value));
        }
        serviceData.insert(std::make_pair(serviceUuid, serviceValue));
    }
}

int GetCurrentSdkVersion(void)
{
    int version = SDK_VERSION_20;  // default sdk version is api 20

    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        HILOGE("fail to get system ability mgr.");
        return version;
    }
    auto remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!remoteObject) {
        HILOGE("fail to get bundle manager proxy.");
        return version;
    }
    sptr<AppExecFwk::BundleMgrProxy> bundleMgrProxy = iface_cast<AppExecFwk::BundleMgrProxy>(remoteObject);
    if (bundleMgrProxy == nullptr) {
        HILOGE("Failed to get bundle manager proxy.");
        return version;
    }
    AppExecFwk::BundleInfo bundleInfo;
    auto flags = AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION;
    auto ret = bundleMgrProxy->GetBundleInfoForSelf(static_cast<int32_t>(flags), bundleInfo);
    if (ret != ERR_OK) {
        HILOGE("GetBundleInfoForSelf: get fail.");
        return version;
    }

    version = static_cast<int>(bundleInfo.targetVersion % 100); // %100 to get the real version
    return version;
}

int GetSDKAdaptedStatusCode(int status)
{
    std::set<int> statusCodeForNewSdk = {
        BT_ERR_MAX_RESOURCES,
        BT_ERR_OPERATION_BUSY,
        BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED,
        BT_ERR_GATT_CONNECTION_CONGESTED,
        BT_ERR_GATT_CONNECTION_NOT_ENCRYPTED,
        BT_ERR_GATT_CONNECTION_NOT_AUTHENTICATED,
        BT_ERR_GATT_CONNECTION_NOT_AUTHORIZED,
        BT_ERR_BLE_ADV_DATA_EXCEED_LIMIT,
        BT_ERR_BLE_INVALID_ADV_ID,
    };
    bool isNewStatusCode = false;
    if (statusCodeForNewSdk.find(status) != statusCodeForNewSdk.end()) {
        isNewStatusCode = true;
    }
    // If sdk version is lower than SDK_VERSION_20, the status code is changed to old version.
    if (GetCurrentSdkVersion() < SDK_VERSION_20 && isNewStatusCode) {
        return BT_ERR_INTERNAL_ERROR;
    }
    return status;
}
}
}