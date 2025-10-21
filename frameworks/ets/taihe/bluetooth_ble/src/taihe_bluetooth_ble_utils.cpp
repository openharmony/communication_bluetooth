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

#include "taihe_bluetooth_ble_utils.h"
#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "bundle_mgr_proxy.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
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
    env->Object_New(TH_ANI_FIND_CLASS(env, "@ohos.bluetooth.ble.ble._taihe_GattPermissions_inner"),
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble._taihe_GattPermissions_inner", "<ctor>", nullptr),
        &ani_obj, ani_field_read, ani_field_readEncrypted, ani_field_readEncryptedMitm,
            ani_field_write, ani_field_writeEncrypted, ani_field_writeEncryptedMitm,
            ani_field_writeSigned, ani_field_writeSignedMitm);
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
    std::copy(valueData, valueData + valueSize, reinterpret_cast<uint8_t*>(ani_field_descriptorValue_ani_data));

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
    env->Object_New(TH_ANI_FIND_CLASS(env, "@ohos.bluetooth.ble.ble._taihe_BLEDescriptor_inner"),
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble._taihe_BLEDescriptor_inner", "<ctor>", nullptr),
        &ani_obj, ani_field_serviceUuid, ani_field_characteristicUuid, ani_field_descriptorUuid,
        ani_field_descriptorValue, ani_field_descriptorHandle, ani_field_permissions);
    return ani_obj;
}
}  // namespace Bluetooth
}  // namespace OHOS