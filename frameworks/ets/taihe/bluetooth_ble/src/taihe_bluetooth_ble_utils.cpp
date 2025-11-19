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
    ani_object write = {};
    ani_boolean value = static_cast<ani_boolean>(HasProperty(properties, GattCharacteristic::WRITE));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &write, value);

    ani_object writeNoResponse = {};
    value = static_cast<ani_boolean>(HasProperty(properties, GattCharacteristic::WRITE_WITHOUT_RESPONSE));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &writeNoResponse, value);

    ani_object read = {};
    value = static_cast<ani_boolean>(HasProperty(properties, GattCharacteristic::READ));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &read, value);

    ani_object notify = {};
    value = static_cast<ani_boolean>(HasProperty(properties, GattCharacteristic::NOTIFY));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &notify, value);

    ani_object indicate = {};
    value = static_cast<ani_boolean>(HasProperty(properties, GattCharacteristic::INDICATE));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &indicate, value);

    ani_object broadcast = {};
    value = static_cast<ani_boolean>(HasProperty(properties, GattCharacteristic::BROADCAST));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &broadcast, value);

    ani_object authenticatedSignedWrite = {};
    value = static_cast<ani_boolean>(HasProperty(properties, GattCharacteristic::AUTHENTICATED_SIGNED_WRITES));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &authenticatedSignedWrite, value);

    ani_object extendedProperties = {};
    value = static_cast<ani_boolean>(HasProperty(properties, GattCharacteristic::EXTENDED_PROPERTIES));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &extendedProperties, value);

    ani_object object = {};
    env->Object_New(TH_ANI_FIND_CLASS(env, "@ohos.bluetooth.ble.ble.GattPropertiesInfo"),
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.GattPropertiesInfo", "<ctor>", nullptr),
        &object, write, writeNoResponse, read, notify, indicate, broadcast,
        authenticatedSignedWrite, extendedProperties);
    return object;
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
    ani_object read = {};
    ani_boolean value = static_cast<ani_boolean>(HasPermission(permissions, GattCharacteristic::READABLE));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &read, value);

    ani_object readEncrypted = {};
    value = static_cast<ani_boolean>(HasPermission(permissions, GattCharacteristic::READ_ENCRYPTED));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &readEncrypted, value);

    ani_object readEncryptedMitm = {};
    value = static_cast<ani_boolean>(HasPermission(permissions, GattCharacteristic::READ_ENCRYPTED_MITM));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &readEncryptedMitm, value);

    ani_object write = {};
    value = static_cast<ani_boolean>(HasPermission(permissions, GattCharacteristic::WRITEABLE));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &write, value);

    ani_object writeEncrypted = {};
    value = static_cast<ani_boolean>(HasPermission(permissions, GattCharacteristic::WRITE_ENCRYPTED));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &writeEncrypted, value);

    ani_object writeEncryptedMitm = {};
    value = static_cast<ani_boolean>(HasPermission(permissions, GattCharacteristic::WRITE_ENCRYPTED_MITM));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &writeEncryptedMitm, value);

    ani_object writeSigned = {};
    value = static_cast<ani_boolean>(HasPermission(permissions, GattCharacteristic::WRITE_SIGNED));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &writeSigned, value);

    ani_object writeSignedMitm = {};
    value = static_cast<ani_boolean>(HasPermission(permissions, GattCharacteristic::WRITE_SIGNED_MITM));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Boolean"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "<ctor>", "z:"), &writeSignedMitm, value);

    ani_object object = {};
    env->Object_New(TH_ANI_FIND_CLASS(env, "@ohos.bluetooth.ble.ble.GattPermissionsInfo"),
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.GattPermissionsInfo", "<ctor>", nullptr),
        &object, read, readEncrypted, readEncryptedMitm, write, writeEncrypted, writeEncryptedMitm,
        writeSigned, writeSignedMitm);
    return object;
}

ani_object ConvertBLECharacteristicToJS(ani_env *env, GattCharacteristic &characteristic)
{
    ani_string characteristicUuid = {};
    HILOGI("uuid: %{public}s", characteristic.GetUuid().ToString().c_str());
    env->String_NewUTF8(characteristic.GetUuid().ToString().c_str(),
        characteristic.GetUuid().ToString().size(), &characteristicUuid);

    ani_string serviceUuid = {};
    if (characteristic.GetService() != nullptr) {
        env->String_NewUTF8(characteristic.GetService()->GetUuid().ToString().c_str(),
            characteristic.GetService()->GetUuid().ToString().size(), &serviceUuid);
    }

    size_t valueSize = 0;
    uint8_t* valueData = characteristic.GetValue(&valueSize).get();
    ani_arraybuffer characteristicValue = {};
    {
        void* characteristicValueData = {};
        env->CreateArrayBuffer(valueSize,
            &characteristicValueData, &characteristicValue);
        if (memcpy_s(reinterpret_cast<uint8_t*>(characteristicValueData), valueSize, valueData, valueSize) != EOK) {
            HILOGE("memcpy_s error");
            return {};
        }
    }

    ani_ref properties = {};
    ani_object propertiesObject = ConvertGattPropertiesToJs(env, characteristic.GetProperties());
    properties = propertiesObject;

    ani_ref permissions = {};
    ani_object permissionsObject = ConvertGattPermissionsToJs(env, characteristic.GetPermissions());
    permissions = permissionsObject;

    ani_array descriptors {};
    ani_object descriptorsObject = ConvertBLEDescriptorVectorToJS(env, characteristic.GetDescriptors());
    descriptors = reinterpret_cast<ani_array>(descriptorsObject);

    ani_ref characteristicValueHandle = {};
    ani_object characteristicValueHandleObject = {};
    ani_int handle = static_cast<ani_int>(characteristic.GetHandle());
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Int"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Int", "<ctor>", "i:"),
        &characteristicValueHandleObject, handle);
    characteristicValueHandle = characteristicValueHandleObject;

    ani_object object = {};
    env->Object_New(TH_ANI_FIND_CLASS(env, "@ohos.bluetooth.ble.ble.BLECharacteristicInfo"),
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.BLECharacteristicInfo", "<ctor>", nullptr),
        &object, serviceUuid, characteristicUuid, characteristicValue,
        descriptors, properties, characteristicValueHandle, permissions);
    return object;
}

ani_object ConvertBLEDescriptorToJS(ani_env *env, GattDescriptor& descriptor)
{
    HILOGI("uuid: %{public}s", descriptor.GetUuid().ToString().c_str());
    ani_string descriptorUuid = {};
    env->String_NewUTF8(descriptor.GetUuid().ToString().c_str(),
        descriptor.GetUuid().ToString().size(), &descriptorUuid);

    ani_string characteristicUuid = {};
    ani_string serviceUuid = {};
    if (descriptor.GetCharacteristic() != nullptr) {
        env->String_NewUTF8(descriptor.GetCharacteristic()->GetUuid().ToString().c_str(),
            descriptor.GetCharacteristic()->GetUuid().ToString().size(), &characteristicUuid);

        if (descriptor.GetCharacteristic()->GetService() != nullptr) {
            env->String_NewUTF8(descriptor.GetCharacteristic()->GetService()->GetUuid().ToString().c_str(),
                descriptor.GetCharacteristic()->GetService()->GetUuid().ToString().size(), &serviceUuid);
        }
    }

    size_t valueSize;
    uint8_t* valueData = descriptor.GetValue(&valueSize).get();
    void* descriptorValueData = {};
    ani_arraybuffer descriptorValue = {};
    env->CreateArrayBuffer(valueSize * (sizeof(uint8_t) / sizeof(char)), &descriptorValueData, &descriptorValue);
    if (memcpy_s(reinterpret_cast<uint8_t*>(descriptorValueData), valueSize, valueData, valueSize) != EOK) {
        HILOGE("memcpy_s error");
    }

    ani_ref descriptorHandle = {};
    ani_object descriptorHandleObject = {};
    ani_int handle = static_cast<ani_int>(static_cast<uint32_t>(descriptor.GetHandle()));
    env->Object_New(TH_ANI_FIND_CLASS(env, "std.core.Int"),
        TH_ANI_FIND_CLASS_METHOD(env, "std.core.Int", "<ctor>", "i:"), &descriptorHandleObject, handle);
    descriptorHandle = descriptorHandleObject;

    ani_ref permissions = {};
    ani_object permissionsObject = ConvertGattPermissionsToJs(env, descriptor.GetPermissions());
    permissions = permissionsObject;

    ani_object object = {};
    env->Object_New(TH_ANI_FIND_CLASS(env, "@ohos.bluetooth.ble.ble.BLEDescriptorInfo"),
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.BLEDescriptorInfo", "<ctor>", nullptr),
        &object, serviceUuid, characteristicUuid, descriptorUuid, descriptorValue, descriptorHandle, permissions);
    return object;
}

ani_object ConvertBLEDescriptorVectorToJS(ani_env *env, std::vector<GattDescriptor> &descriptors)
{
    HILOGI("size: %{public}zu", descriptors.size());

    if (descriptors.empty()) {
        return nullptr;
    }

    ani_array descriptorsObject = {};
    ani_ref descriptorsNone = {};
    env->GetUndefined(&descriptorsNone);
    env->Array_New(descriptors.size(), descriptorsNone, &descriptorsObject);
    for (size_t iterator = 0; iterator < descriptors.size(); iterator++) {
        ani_object item = ConvertBLEDescriptorToJS(env, descriptors[iterator]);
        env->Array_Set(descriptorsObject, iterator, item);
    }
    return descriptorsObject;
}
}  // namespace Bluetooth
}  // namespace OHOS