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

#ifndef LOG_TAG
#define LOG_TAG "bt_taihe_ble_utils"
#endif

#include "taihe_bluetooth_utils.h"

#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "bundle_mgr_proxy.h"
#include "iservice_registry.h"
#include "securec.h"
#include "system_ability_definition.h"
#include "taihe_bluetooth_ble_utils.h"
#include "taihe/array.hpp"
#include "taihe/optional.hpp"
#include "taihe/platform/ani.hpp"

namespace OHOS {
namespace Bluetooth {

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

bool HasProperty(int properties, int propertyMask)
{
    if (properties < 0 || propertyMask < 0) {
        HILOGE("properties or propertyMask is less than 0");
        return false;
    }
    return (static_cast<unsigned int>(properties) & static_cast<unsigned int>(propertyMask)) != 0;
}

ani_object ConvertGattPropertiesToJs(ani_env *env, int properties)
{
    ani_ref ani_field_write = {};
    ani_object ani_field_write_ani_temp = {};
    ani_boolean ani_field_write_ani_temp_ani_after =
        static_cast<ani_boolean>(HasProperty(properties, GattCharacteristic::WRITE));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"),
        &ani_field_write_ani_temp, ani_field_write_ani_temp_ani_after);
    ani_field_write = ani_field_write_ani_temp;

    ani_ref ani_field_writeNoResponse = {};
    ani_object ani_field_writeNoResponse_ani_temp = {};
    ani_boolean ani_field_writeNoResponse_ani_temp_ani_after =
        static_cast<ani_boolean>(HasProperty(properties, GattCharacteristic::WRITE_WITHOUT_RESPONSE));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"),
        &ani_field_writeNoResponse_ani_temp, ani_field_writeNoResponse_ani_temp_ani_after);
    ani_field_writeNoResponse = ani_field_writeNoResponse_ani_temp;

    ani_ref ani_field_read = {};
    ani_object ani_field_read_ani_temp = {};
    ani_boolean ani_field_read_ani_temp_ani_after =
        static_cast<ani_boolean>(HasProperty(properties, GattCharacteristic::READ));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"),
        &ani_field_read_ani_temp, ani_field_read_ani_temp_ani_after);
    ani_field_read = ani_field_read_ani_temp;

    ani_ref ani_field_notify = {};
    ani_object ani_field_notify_ani_temp = {};
    ani_boolean ani_field_notify_ani_temp_ani_after =
        static_cast<ani_boolean>(HasProperty(properties, GattCharacteristic::NOTIFY));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"),
        &ani_field_notify_ani_temp, ani_field_notify_ani_temp_ani_after);
    ani_field_notify = ani_field_notify_ani_temp;

    ani_ref ani_field_indicate = {};
    ani_object ani_field_indicate_ani_temp = {};
    ani_boolean ani_field_indicate_ani_temp_ani_after =
        static_cast<ani_boolean>(HasProperty(properties, GattCharacteristic::INDICATE));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"),
        &ani_field_indicate_ani_temp, ani_field_indicate_ani_temp_ani_after);
    ani_field_indicate = ani_field_indicate_ani_temp;

    ani_ref ani_field_broadcast = {};
    ani_object ani_field_broadcast_ani_temp = {};
    ani_boolean ani_field_broadcast_ani_temp_ani_after =
        static_cast<ani_boolean>(HasProperty(properties, GattCharacteristic::BROADCAST));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"),
        &ani_field_broadcast_ani_temp, ani_field_broadcast_ani_temp_ani_after);
    ani_field_broadcast = ani_field_broadcast_ani_temp;

    ani_ref ani_field_authenticatedSignedWrite = {};
    ani_object ani_field_authenticatedSignedWrite_ani_temp = {};
    ani_boolean ani_field_authenticatedSignedWrite_ani_temp_ani_after =
        static_cast<ani_boolean>(HasProperty(properties, GattCharacteristic::AUTHENTICATED_SIGNED_WRITES));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"),
        &ani_field_authenticatedSignedWrite_ani_temp, ani_field_authenticatedSignedWrite_ani_temp_ani_after);
    ani_field_authenticatedSignedWrite = ani_field_authenticatedSignedWrite_ani_temp;

    ani_ref ani_field_extendedProperties = {};
    ani_object ani_field_extendedProperties_ani_temp = {};
    ani_boolean ani_field_extendedProperties_ani_temp_ani_after =
        static_cast<ani_boolean>(HasProperty(properties, GattCharacteristic::EXTENDED_PROPERTIES));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"),
        &ani_field_extendedProperties_ani_temp, ani_field_extendedProperties_ani_temp_ani_after);
    ani_field_extendedProperties = ani_field_extendedProperties_ani_temp;

    ani_object ani_obj = {};
    env->Object_New(TH_ANI_FIND_CLASS(env, "@ohos.bluetooth.ble.ble.GattPropertiesInfo"),
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.GattPropertiesInfo", "<ctor>", nullptr),
        &ani_obj, ani_field_write, ani_field_writeNoResponse, ani_field_read, ani_field_notify, ani_field_indicate,
        ani_field_broadcast, ani_field_authenticatedSignedWrite, ani_field_extendedProperties);
    return ani_obj;
}

bool HasPermission(int permissions, int permissionMask)
{
    if (permissions < 0 || permissionMask < 0) {
        HILOGE("permissions or permissionMask is less than 0");
        return false;
    }
    return (static_cast<unsigned int>(permissions) & static_cast<unsigned int>(permissionMask)) != 0;
}

ani_object ConvertGattPermissionsToJs(ani_env *env, int permissions)
{
    ani_ref ani_field_read = {};
    ani_object ani_field_read_ani_temp = {};
    ani_boolean ani_field_read_ani_temp_ani_after =
        static_cast<ani_boolean>(HasPermission(permissions, GattCharacteristic::READABLE));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"),
        &ani_field_read_ani_temp, ani_field_read_ani_temp_ani_after);
    ani_field_read = ani_field_read_ani_temp;

    ani_ref ani_field_readEncrypted = {};
    ani_object ani_field_readEncrypted_ani_temp = {};
    ani_boolean ani_field_readEncrypted_ani_temp_ani_after =
        static_cast<ani_boolean>(HasPermission(permissions, GattCharacteristic::READ_ENCRYPTED));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"),
        &ani_field_readEncrypted_ani_temp, ani_field_readEncrypted_ani_temp_ani_after);
    ani_field_readEncrypted = ani_field_readEncrypted_ani_temp;

    ani_ref ani_field_readEncryptedMitm = {};
    ani_object ani_field_readEncryptedMitm_ani_temp = {};
    ani_boolean ani_field_readEncryptedMitm_ani_temp_ani_after =
        static_cast<ani_boolean>(HasPermission(permissions, GattCharacteristic::READ_ENCRYPTED_MITM));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"),
        &ani_field_readEncryptedMitm_ani_temp, ani_field_readEncryptedMitm_ani_temp_ani_after);
    ani_field_readEncryptedMitm = ani_field_readEncryptedMitm_ani_temp;

    ani_ref ani_field_write = {};
    ani_object ani_field_write_ani_temp = {};
    ani_boolean ani_field_write_ani_temp_ani_after =
        static_cast<ani_boolean>(HasPermission(permissions, GattCharacteristic::WRITEABLE));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"),
        &ani_field_write_ani_temp, ani_field_write_ani_temp_ani_after);
    ani_field_write = ani_field_write_ani_temp;

    ani_ref ani_field_writeEncrypted = {};
    ani_object ani_field_writeEncrypted_ani_temp = {};
    ani_boolean ani_field_writeEncrypted_ani_temp_ani_after =
        static_cast<ani_boolean>(HasPermission(permissions, GattCharacteristic::WRITE_ENCRYPTED));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"),
        &ani_field_writeEncrypted_ani_temp, ani_field_writeEncrypted_ani_temp_ani_after);
    ani_field_writeEncrypted = ani_field_writeEncrypted_ani_temp;

    ani_ref ani_field_writeEncryptedMitm = {};
    ani_object ani_field_writeEncryptedMitm_ani_temp = {};
    ani_boolean ani_field_writeEncryptedMitm_ani_temp_ani_after =
        static_cast<ani_boolean>(HasPermission(permissions, GattCharacteristic::WRITE_ENCRYPTED_MITM));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"),
        &ani_field_writeEncryptedMitm_ani_temp, ani_field_writeEncryptedMitm_ani_temp_ani_after);
    ani_field_writeEncryptedMitm = ani_field_writeEncryptedMitm_ani_temp;

    ani_ref ani_field_writeSigned = {};
    ani_object ani_field_writeSigned_ani_temp = {};
    ani_boolean ani_field_writeSigned_ani_temp_ani_after =
        static_cast<ani_boolean>(HasPermission(permissions, GattCharacteristic::WRITE_SIGNED));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"),
        &ani_field_writeSigned_ani_temp, ani_field_writeSigned_ani_temp_ani_after);
    ani_field_writeSigned = ani_field_writeSigned_ani_temp;

    ani_ref ani_field_writeSignedMitm = {};
    ani_object ani_field_writeSignedMitm_ani_temp = {};
    ani_boolean ani_field_writeSignedMitm_ani_temp_ani_after =
        static_cast<ani_boolean>(HasPermission(permissions, GattCharacteristic::WRITE_SIGNED_MITM));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"),
        &ani_field_writeSignedMitm_ani_temp, ani_field_writeSignedMitm_ani_temp_ani_after);
    ani_field_writeSignedMitm = ani_field_writeSignedMitm_ani_temp;

    ani_object ani_obj = {};
    env->Object_New(TH_ANI_FIND_CLASS(env, "@ohos.bluetooth.ble.ble.GattPermissionsInfo"),
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.GattPermissionsInfo", "<ctor>", nullptr),
        &ani_obj, ani_field_read, ani_field_readEncrypted, ani_field_readEncryptedMitm,
        ani_field_write, ani_field_writeEncrypted, ani_field_writeEncryptedMitm,
        ani_field_writeSigned, ani_field_writeSignedMitm);
    return ani_obj;
}

ani_object ConvertBLECharacteristicToJS(ani_env *env, GattCharacteristic &characteristic)
{
    ani_string ani_field_characteristicUuid = {};
    env->String_NewUTF8(characteristic.GetUuid().ToString().c_str(),
        characteristic.GetUuid().ToString().size(), &ani_field_characteristicUuid);

    ani_string ani_field_serviceUuid = {};
    if (characteristic.GetService() != nullptr) {
        env->String_NewUTF8(characteristic.GetService()->GetUuid().ToString().c_str(),
            characteristic.GetService()->GetUuid().ToString().size(), &ani_field_serviceUuid);
    }

    size_t valueSize = 0;
    uint8_t* valueData = characteristic.GetValue(&valueSize).get();
    ani_arraybuffer ani_field_characteristicValue = {};
    {
        void* ani_field_characteristicValue_ani_data = {};
        env->CreateArrayBuffer(valueSize * (sizeof(uint8_t) / sizeof(char)),
            &ani_field_characteristicValue_ani_data, &ani_field_characteristicValue);
        if (memcpy_s(reinterpret_cast<uint8_t*>(ani_field_characteristicValue_ani_data), valueSize,
            valueData, valueSize) != EOK) {
            HILOGE("memcpy_s error");
        }
    }

    ani_ref ani_field_properties = {};
    ani_object ani_field_properties_ani_temp = ConvertGattPropertiesToJs(env, characteristic.GetProperties());
    ani_field_properties = ani_field_properties_ani_temp;

    ani_ref ani_field_permissions = {};
    ani_object ani_field_permissions_ani_temp = ConvertGattPermissionsToJs(env, characteristic.GetPermissions());
    ani_field_permissions = ani_field_permissions_ani_temp;

    ani_array ani_field_descriptors {};
    ani_object ani_field_descriptors_ani_temp = ConvertBLEDescriptorVectorToJS(env, characteristic.GetDescriptors());
    ani_field_descriptors = reinterpret_cast<ani_array>(ani_field_descriptors_ani_temp);

    ani_ref ani_field_characteristicValueHandle = {};
    ani_object ani_field_characteristicValueHandle_ani_temp = {};
    ani_int ani_field_characteristicValueHandle_ani_temp_ani_after = static_cast<ani_int>(characteristic.GetHandle());
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Int"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Int", "<ctor>", "i:"),
        &ani_field_characteristicValueHandle_ani_temp, ani_field_characteristicValueHandle_ani_temp_ani_after);
    ani_field_characteristicValueHandle = ani_field_characteristicValueHandle_ani_temp;

    ani_object ani_obj = {};
    env->Object_New(TH_ANI_FIND_CLASS(env, "@ohos.bluetooth.ble.ble.BLECharacteristicInfo"),
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.BLECharacteristicInfo", "<ctor>", nullptr),
        &ani_obj, ani_field_serviceUuid, ani_field_characteristicUuid, ani_field_characteristicValue,
        ani_field_descriptors, ani_field_properties, ani_field_characteristicValueHandle, ani_field_permissions);
    return ani_obj;
}

ani_object ConvertBLEDescriptorToJS(ani_env *env, GattDescriptor& descriptor)
{
    HILOGI("uuid: %{public}s", descriptor.GetUuid().ToString().c_str());
    ani_string ani_field_descriptorUuid = {};
    env->String_NewUTF8(descriptor.GetUuid().ToString().c_str(),
        descriptor.GetUuid().ToString().size(), &ani_field_descriptorUuid);

    ani_string ani_field_characteristicUuid = {};
    ani_string ani_field_serviceUuid = {};
    if (descriptor.GetCharacteristic() != nullptr) {
        ani_string ani_field_characteristicUuid = {};
        env->String_NewUTF8(descriptor.GetCharacteristic()->GetUuid().ToString().c_str(),
            descriptor.GetCharacteristic()->GetUuid().ToString().size(), &ani_field_characteristicUuid);

        if (descriptor.GetCharacteristic()->GetService() != nullptr) {
            env->String_NewUTF8(descriptor.GetCharacteristic()->GetService()->GetUuid().ToString().c_str(),
                descriptor.GetCharacteristic()->GetService()->GetUuid().ToString().size(),
                &ani_field_serviceUuid);
        }
    }

    size_t valueSize;
    uint8_t* valueData = descriptor.GetValue(&valueSize).get();
    void* ani_field_descriptorValue_ani_data = {};
    ani_arraybuffer ani_field_descriptorValue = {};
    env->CreateArrayBuffer(valueSize * (sizeof(uint8_t) / sizeof(char)),
        &ani_field_descriptorValue_ani_data, &ani_field_descriptorValue);
    if (memcpy_s(reinterpret_cast<uint8_t*>(ani_field_descriptorValue_ani_data), valueSize,
        valueData, valueSize) != EOK) {
        HILOGE("memcpy_s error");
    }

    ani_ref ani_field_descriptorHandle = {};
    ani_object ani_field_descriptorHandle_ani_temp = {};
    ani_int ani_field_descriptorHandle_ani_temp_ani_after =
        static_cast<ani_int>(static_cast<uint32_t>(descriptor.GetHandle()));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Int"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Int", "<ctor>", "i:"),
        &ani_field_descriptorHandle_ani_temp, ani_field_descriptorHandle_ani_temp_ani_after);
    ani_field_descriptorHandle = ani_field_descriptorHandle_ani_temp;

    ani_ref ani_field_permissions = {};
    ani_object ani_field_permissions_ani_temp = ConvertGattPermissionsToJs(env, descriptor.GetPermissions());
    ani_field_permissions = ani_field_permissions_ani_temp;

    ani_object ani_obj = {};
    env->Object_New(TH_ANI_FIND_CLASS(env, "@ohos.bluetooth.ble.ble.BLEDescriptorInfo"),
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.BLEDescriptorInfo", "<ctor>", nullptr),
        &ani_obj, ani_field_serviceUuid, ani_field_characteristicUuid, ani_field_descriptorUuid,
        ani_field_descriptorValue, ani_field_descriptorHandle, ani_field_permissions);
    return ani_obj;
}

ani_object ConvertBLEDescriptorVectorToJS(ani_env *env, std::vector<GattDescriptor> &descriptors)
{
    HILOGI("size: %{public}zu", descriptors.size());

    if (descriptors.empty()) {
        return nullptr;
    }

    ani_array ani_field_descriptors = {};
    ani_ref ani_field_descriptors_ani_none = {};
    env->GetUndefined(&ani_field_descriptors_ani_none);
    env->Array_New(descriptors.size(), ani_field_descriptors_ani_none, &ani_field_descriptors);
    for (size_t ani_field_descriptors_iterator = 0; ani_field_descriptors_iterator < descriptors.size();
        ani_field_descriptors_iterator++) {
        ani_object ani_field_descriptors_ani_item =
            ConvertBLEDescriptorToJS(env, descriptors[ani_field_descriptors_iterator]);
        env->Array_Set(ani_field_descriptors, ani_field_descriptors_iterator, ani_field_descriptors_ani_item);
    }
    return ani_field_descriptors;
}
}  // namespace Bluetooth
}  // namespace OHOS