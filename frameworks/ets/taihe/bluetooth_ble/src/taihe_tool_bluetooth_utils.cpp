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
#define LOG_TAG "bt_taihe_tool_utils"
#endif

#include "taihe_tool_bluetooth_utils.h"
#include "bluetooth_log.h"
#include "taihe/platform/ani.hpp"
#include "taihe/optional.hpp"
#include "taihe/array.hpp"

namespace OHOS {
namespace Bluetooth {
static ::ohos::bluetooth::ble::ServiceData TaiheParseServiceData(ani_env *env, ani_object ani_obj)
{
    ani_string ani_field_serviceUuid;
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.ServiceData", "<get>serviceUuid", nullptr),
        reinterpret_cast<ani_ref*>(&ani_field_serviceUuid));
    size_t cpp_field_serviceUuid_len;
    env->String_GetUTF8Size(ani_field_serviceUuid, &cpp_field_serviceUuid_len);
    TString cpp_field_serviceUuid_tstr;
    char* cpp_field_serviceUuid_buf = tstr_initialize(&cpp_field_serviceUuid_tstr, cpp_field_serviceUuid_len + 1);
    env->String_GetUTF8(ani_field_serviceUuid, cpp_field_serviceUuid_buf,
        cpp_field_serviceUuid_len + 1, &cpp_field_serviceUuid_len);
    cpp_field_serviceUuid_buf[cpp_field_serviceUuid_len] = '\0';
    cpp_field_serviceUuid_tstr.length = cpp_field_serviceUuid_len;
    ::taihe::string cpp_field_serviceUuid = ::taihe::string(cpp_field_serviceUuid_tstr);
    ani_arraybuffer ani_field_serviceValue;
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.ServiceData", "<get>serviceValue", nullptr),
        reinterpret_cast<ani_ref*>(&ani_field_serviceValue));
    char* cpp_field_serviceValue_data = nullptr;
    size_t cpp_field_serviceValue_length = 0;
    env->ArrayBuffer_GetInfo(ani_field_serviceValue, reinterpret_cast<void **>(&cpp_field_serviceValue_data),
        &cpp_field_serviceValue_length);
    ::taihe::array_view<uint8_t> cpp_field_serviceValue(reinterpret_cast<uint8_t*>(cpp_field_serviceValue_data),
        cpp_field_serviceValue_length);
    return ::ohos::bluetooth::ble::ServiceData { std::move(cpp_field_serviceUuid),
                                                 std::move(cpp_field_serviceValue) };
}

static ::ohos::bluetooth::ble::ManufactureData TaiheParseManufactureData(ani_env *env, ani_object ani_obj)
{
    ani_int ani_field_manufactureId;
    env->Object_CallMethod_Int(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.ManufactureData", "<get>manufactureId", nullptr),
        reinterpret_cast<ani_int*>(&ani_field_manufactureId));
    int cpp_field_manufactureId = (int)ani_field_manufactureId;
    ani_arraybuffer ani_field_manufactureValue;
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.ManufactureData", "<get>manufactureValue", nullptr),
        reinterpret_cast<ani_ref*>(&ani_field_manufactureValue));
    char* cpp_field_manufactureValue_data = nullptr;
    size_t cpp_field_manufactureValue_length = 0;
    env->ArrayBuffer_GetInfo(ani_field_manufactureValue, reinterpret_cast<void **>(&cpp_field_manufactureValue_data),
        &cpp_field_manufactureValue_length);
    ::taihe::array_view<uint8_t> cpp_field_manufactureValue(reinterpret_cast<uint8_t*>(cpp_field_manufactureValue_data),
        cpp_field_manufactureValue_length);
    return ::ohos::bluetooth::ble::ManufactureData { std::move(cpp_field_manufactureId),
                                                     std::move(cpp_field_manufactureValue) };
}

static ::ohos::bluetooth::ble::AdvertiseData TaiheParseAdvertisingData(ani_env *env, ani_object ani_obj)
{
    ani_array ani_field_serviceUuids;
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.AdvertiseData", "<get>serviceUuids", nullptr),
        reinterpret_cast<ani_ref*>(&ani_field_serviceUuids));
    size_t cpp_field_serviceUuids_size;
    env->Array_GetLength(ani_field_serviceUuids, &cpp_field_serviceUuids_size);
    ::taihe::string* cpp_field_serviceUuids_buffer =
        reinterpret_cast<::taihe::string*>(malloc(cpp_field_serviceUuids_size * sizeof(::taihe::string)));
    for (size_t cpp_field_serviceUuids_buffer_i = 0;
         cpp_field_serviceUuids_buffer_i < cpp_field_serviceUuids_size;
         cpp_field_serviceUuids_buffer_i++) {
        ani_object cpp_field_serviceUuids_buffer_ani_item;
        env->Array_Get(ani_field_serviceUuids, cpp_field_serviceUuids_buffer_i,
            reinterpret_cast<ani_ref*>(&cpp_field_serviceUuids_buffer_ani_item));
        ani_size cpp_field_serviceUuids_buffer_cpp_item_len;
        env->String_GetUTF8Size(static_cast<ani_string>(cpp_field_serviceUuids_buffer_ani_item),
            &cpp_field_serviceUuids_buffer_cpp_item_len);
        TString cpp_field_serviceUuids_buffer_cpp_item_tstr;
        char *cpp_field_serviceUuids_buffer_cpp_item_buf = tstr_initialize(&cpp_field_serviceUuids_buffer_cpp_item_tstr,
            cpp_field_serviceUuids_buffer_cpp_item_len + 1);
        env->String_GetUTF8(static_cast<ani_string>(cpp_field_serviceUuids_buffer_ani_item),
            cpp_field_serviceUuids_buffer_cpp_item_buf,
            cpp_field_serviceUuids_buffer_cpp_item_len + 1,
            &cpp_field_serviceUuids_buffer_cpp_item_len);
        cpp_field_serviceUuids_buffer_cpp_item_buf[cpp_field_serviceUuids_buffer_cpp_item_len] = '\0';
        cpp_field_serviceUuids_buffer_cpp_item_tstr.length = cpp_field_serviceUuids_buffer_cpp_item_len;
        ::taihe::string cpp_field_serviceUuids_buffer_cpp_item =
            ::taihe::string(cpp_field_serviceUuids_buffer_cpp_item_tstr);
        new (&cpp_field_serviceUuids_buffer[cpp_field_serviceUuids_buffer_i]) ::taihe::string(std::move(cpp_field_serviceUuids_buffer_cpp_item));
    }
    ::taihe::array<::taihe::string> cpp_field_serviceUuids(cpp_field_serviceUuids_buffer, cpp_field_serviceUuids_size);
    ani_array ani_field_manufactureData;
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.AdvertiseData", "<get>manufactureData", nullptr),
        reinterpret_cast<ani_ref*>(&ani_field_manufactureData));
    size_t cpp_field_manufactureData_size;
    env->Array_GetLength(ani_field_manufactureData, &cpp_field_manufactureData_size);
    ::ohos::bluetooth::ble::ManufactureData* cpp_field_manufactureData_buffer =
        reinterpret_cast<::ohos::bluetooth::ble::ManufactureData*>(malloc(
            cpp_field_manufactureData_size * sizeof(::ohos::bluetooth::ble::ManufactureData)));
    for (size_t cpp_field_manufactureData_buffer_i = 0;
         cpp_field_manufactureData_buffer_i < cpp_field_manufactureData_size;
         cpp_field_manufactureData_buffer_i++) {
        ani_object cpp_field_manufactureData_buffer_ani_item;
        env->Array_Get(ani_field_manufactureData, cpp_field_manufactureData_buffer_i,
            reinterpret_cast<ani_ref*>(&cpp_field_manufactureData_buffer_ani_item));
        ::ohos::bluetooth::ble::ManufactureData cpp_field_manufactureData_buffer_cpp_item =
            TaiheParseManufactureData(env, static_cast<ani_object>(cpp_field_manufactureData_buffer_ani_item));
        new (&cpp_field_manufactureData_buffer[cpp_field_manufactureData_buffer_i]) ::ohos::bluetooth::ble::ManufactureData(std::move(cpp_field_manufactureData_buffer_cpp_item));
    }
    ::taihe::array<::ohos::bluetooth::ble::ManufactureData> cpp_field_manufactureData(cpp_field_manufactureData_buffer,
        cpp_field_manufactureData_size);
    ani_array ani_field_serviceData;
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.AdvertiseData", "<get>serviceData", nullptr),
        reinterpret_cast<ani_ref*>(&ani_field_serviceData));
    size_t cpp_field_serviceData_size;
    env->Array_GetLength(ani_field_serviceData, &cpp_field_serviceData_size);
    ::ohos::bluetooth::ble::ServiceData* cpp_field_serviceData_buffer =
        reinterpret_cast<::ohos::bluetooth::ble::ServiceData*>(malloc(
            cpp_field_serviceData_size * sizeof(::ohos::bluetooth::ble::ServiceData)));
    for (size_t cpp_field_serviceData_buffer_i = 0;
         cpp_field_serviceData_buffer_i < cpp_field_serviceData_size;
         cpp_field_serviceData_buffer_i++) {
        ani_object cpp_field_serviceData_buffer_ani_item;
        env->Array_Get(ani_field_serviceData, cpp_field_serviceData_buffer_i,
            reinterpret_cast<ani_ref*>(&cpp_field_serviceData_buffer_ani_item));
        ::ohos::bluetooth::ble::ServiceData cpp_field_serviceData_buffer_cpp_item =
            TaiheParseServiceData(env, static_cast<ani_object>(cpp_field_serviceData_buffer_ani_item));
        new (&cpp_field_serviceData_buffer[cpp_field_serviceData_buffer_i]) ::ohos::bluetooth::ble::ServiceData(std::move(cpp_field_serviceData_buffer_cpp_item));
    }
    ::taihe::array<::ohos::bluetooth::ble::ServiceData> cpp_field_serviceData(cpp_field_serviceData_buffer,
        cpp_field_serviceData_size);
    ani_ref ani_field_includeDeviceName;
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.AdvertiseData", "<get>includeDeviceName", nullptr),
        reinterpret_cast<ani_ref*>(&ani_field_includeDeviceName));
    ani_boolean cpp_field_includeDeviceName_flag;
    bool* cpp_field_includeDeviceName_ptr = nullptr;
    env->Reference_IsUndefined(ani_field_includeDeviceName, &cpp_field_includeDeviceName_flag);
    if (!cpp_field_includeDeviceName_flag) {
        ani_boolean cpp_field_includeDeviceName_spec_ani;
        env->Object_CallMethod_Boolean((ani_object)ani_field_includeDeviceName,
            TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "unboxed", ":z"),
            &cpp_field_includeDeviceName_spec_ani);
        bool cpp_field_includeDeviceName_spec = (bool)cpp_field_includeDeviceName_spec_ani;
        cpp_field_includeDeviceName_ptr = new bool(std::move(cpp_field_includeDeviceName_spec));
    }
    ::taihe::optional<bool> cpp_field_includeDeviceName(cpp_field_includeDeviceName_ptr);
    ani_ref ani_field_includeTxPower;
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.AdvertiseData", "<get>includeTxPower", nullptr),
        reinterpret_cast<ani_ref*>(&ani_field_includeTxPower));
    ani_boolean cpp_field_includeTxPower_flag;
    bool* cpp_field_includeTxPower_ptr = nullptr;
    env->Reference_IsUndefined(ani_field_includeTxPower, &cpp_field_includeTxPower_flag);
    if (!cpp_field_includeTxPower_flag) {
        ani_boolean cpp_field_includeTxPower_spec_ani;
        env->Object_CallMethod_Boolean((ani_object)ani_field_includeTxPower,
            TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "unboxed", ":z"),
            &cpp_field_includeTxPower_spec_ani);
        bool cpp_field_includeTxPower_spec = (bool)cpp_field_includeTxPower_spec_ani;
        cpp_field_includeTxPower_ptr = new bool(std::move(cpp_field_includeTxPower_spec));
    }
    ::taihe::optional<bool> cpp_field_includeTxPower(cpp_field_includeTxPower_ptr);
    return ::ohos::bluetooth::ble::AdvertiseData{std::move(cpp_field_serviceUuids),
                                                 std::move(cpp_field_manufactureData),
                                                 std::move(cpp_field_serviceData),
                                                 std::move(cpp_field_includeDeviceName),
                                                 std::move(cpp_field_includeTxPower)};
}

static ::ohos::bluetooth::ble::AdvertiseSetting TaiheParseAdvertiseSetting(ani_env *env, ani_object ani_obj)
{
    ani_ref ani_field_interval;
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.AdvertiseSetting", "<get>interval", nullptr),
        reinterpret_cast<ani_ref*>(&ani_field_interval));
    ani_boolean cpp_field_interval_flag;
    int* cpp_field_interval_ptr = nullptr;
    env->Reference_IsUndefined(ani_field_interval, &cpp_field_interval_flag);
    if (!cpp_field_interval_flag) {
        ani_int cpp_field_interval_spec_ani;
        env->Object_CallMethod_Int((ani_object)ani_field_interval,
            TH_ANI_FIND_CLASS_METHOD(env, "std.core.Int", "unboxed", ":i"),
            &cpp_field_interval_spec_ani);
        int cpp_field_interval_spec = (int)cpp_field_interval_spec_ani;
        cpp_field_interval_ptr = new int(std::move(cpp_field_interval_spec));
    }
    ::taihe::optional<int> cpp_field_interval(cpp_field_interval_ptr);
    ani_ref ani_field_txPower;
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.AdvertiseSetting", "<get>txPower", nullptr),
        reinterpret_cast<ani_ref*>(&ani_field_txPower));
    ani_boolean cpp_field_txPower_flag;
    int* cpp_field_txPower_ptr = nullptr;
    env->Reference_IsUndefined(ani_field_txPower, &cpp_field_txPower_flag);
    if (!cpp_field_txPower_flag) {
        ani_int cpp_field_txPower_spec_ani;
        env->Object_CallMethod_Int((ani_object)ani_field_txPower,
            TH_ANI_FIND_CLASS_METHOD(env, "std.core.Int", "unboxed", ":i"),
            &cpp_field_txPower_spec_ani);
        int cpp_field_txPower_spec = (int)cpp_field_txPower_spec_ani;
        cpp_field_txPower_ptr = new int(std::move(cpp_field_txPower_spec));
    }
    ::taihe::optional<int> cpp_field_txPower(cpp_field_txPower_ptr);
    ani_ref ani_field_connectable;
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.AdvertiseSetting", "<get>connectable", nullptr),
        reinterpret_cast<ani_ref*>(&ani_field_connectable));
    ani_boolean cpp_field_connectable_flag;
    bool* cpp_field_connectable_ptr = nullptr;
    env->Reference_IsUndefined(ani_field_connectable, &cpp_field_connectable_flag);
    if (!cpp_field_connectable_flag) {
        ani_boolean cpp_field_connectable_spec_ani;
        env->Object_CallMethod_Boolean(
            (ani_object)ani_field_connectable,
            TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "unboxed", ":z"),
            &cpp_field_connectable_spec_ani);
        bool cpp_field_connectable_spec = (bool)cpp_field_connectable_spec_ani;
        cpp_field_connectable_ptr = new bool(std::move(cpp_field_connectable_spec));
    }
    ::taihe::optional<bool> cpp_field_connectable(cpp_field_connectable_ptr);
    return ::ohos::bluetooth::ble::AdvertiseSetting{std::move(cpp_field_interval),
                                                    std::move(cpp_field_txPower),
                                                    std::move(cpp_field_connectable)};
}

::ohos::bluetooth::ble::AdvertisingParams TaiheParseAdvertisingParams(ani_env *env, ani_object ani_obj)
{
    ani_object ani_field_advertisingSettings;
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.AdvertisingParams", "<get>advertisingSettings", nullptr),
        reinterpret_cast<ani_ref *>(&ani_field_advertisingSettings));
    ::ohos::bluetooth::ble::AdvertiseSetting cpp_field_advertisingSettings =
        TaiheParseAdvertiseSetting(env, ani_field_advertisingSettings);
    ani_object ani_field_advertisingData;
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.AdvertisingParams", "<get>advertisingData", nullptr),
        reinterpret_cast<ani_ref *>(&ani_field_advertisingData));
    ::ohos::bluetooth::ble::AdvertiseData cpp_field_advertisingData =
        TaiheParseAdvertisingData(env, ani_field_advertisingData);
    ani_ref ani_field_advertisingResponse;
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.AdvertisingParams", "<get>advertisingResponse", nullptr),
        reinterpret_cast<ani_ref *>(&ani_field_advertisingResponse));
    ani_boolean cpp_field_advertisingResponse_flag;
    ::ohos::bluetooth::ble::AdvertiseData* cpp_field_advertisingResponse_ptr = nullptr;
    env->Reference_IsUndefined(ani_field_advertisingResponse, &cpp_field_advertisingResponse_flag);
    if (!cpp_field_advertisingResponse_flag) {
        ::ohos::bluetooth::ble::AdvertiseData cpp_field_advertisingResponse_spec =
            TaiheParseAdvertisingData(env, static_cast<ani_object>(ani_field_advertisingResponse));
        cpp_field_advertisingResponse_ptr =
            new ::ohos::bluetooth::ble::AdvertiseData(cpp_field_advertisingResponse_spec);
    }
    ::taihe::optional<::ohos::bluetooth::ble::AdvertiseData> cpp_field_advertisingResponse(
        cpp_field_advertisingResponse_ptr);
    ani_ref ani_field_duration;
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.AdvertisingParams", "<get>duration", nullptr),
        reinterpret_cast<ani_ref *>(&ani_field_duration));
    ani_boolean cpp_field_duration_flag;
    int* cpp_field_duration_ptr = nullptr;
    env->Reference_IsUndefined(ani_field_duration, &cpp_field_duration_flag);
    if (!cpp_field_duration_flag) {
        ani_int cpp_field_duration_spec_ani;
        env->Object_CallMethod_Int((ani_object)ani_field_duration,
            TH_ANI_FIND_CLASS_METHOD(env, "std.core.Int", "unboxed", ":i"),
            &cpp_field_duration_spec_ani);
        int cpp_field_duration_spec = (int)cpp_field_duration_spec_ani;
        cpp_field_duration_ptr = new int(std::move(cpp_field_duration_spec));
    }
    ::taihe::optional<int> cpp_field_duration(cpp_field_duration_ptr);
    return ::ohos::bluetooth::ble::AdvertisingParams{std::move(cpp_field_advertisingSettings),
                                                     std::move(cpp_field_advertisingData),
                                                     std::move(cpp_field_advertisingResponse),
                                                     std::move(cpp_field_duration)};
}

::ohos::bluetooth::ble::AdvertisingDisableParams TaiheParseAdvertisingDisableParams(ani_env *env, ani_object ani_obj)
{
    ani_int ani_field_advertisingId = {};
    env->Object_CallMethod_Int(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.AdvertisingDisableParams", "<get>advertisingId", nullptr),
        reinterpret_cast<ani_int*>(&ani_field_advertisingId));
    int32_t cpp_field_advertisingId = static_cast<int32_t>(ani_field_advertisingId);
    return ::ohos::bluetooth::ble::AdvertisingDisableParams{std::move(cpp_field_advertisingId)};
}

::ohos::bluetooth::ble::AdvertisingEnableParams TaiheParseAdvertisingEnableParams(ani_env* env, ani_object ani_obj)
{
    ani_int ani_field_advertisingId = {};
    env->Object_CallMethod_Int(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.AdvertisingEnableParams", "<get>advertisingId", nullptr),
        reinterpret_cast<ani_int*>(&ani_field_advertisingId));
    int32_t cpp_field_advertisingId = static_cast<int32_t>(ani_field_advertisingId);
    ani_ref ani_field_duration = {};
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.AdvertisingEnableParams", "<get>duration", nullptr),
        reinterpret_cast<ani_ref*>(&ani_field_duration));
    ::taihe::optional<int32_t> cpp_field_duration;
    ani_boolean cpp_field_duration_ani_flag = {};
    env->Reference_IsUndefined(ani_field_duration, &cpp_field_duration_ani_flag);
    if (!cpp_field_duration_ani_flag) {
        ani_int cpp_field_duration_cpp_temp_ani_value = {};
        env->Object_CallMethod_Int(static_cast<ani_object>(ani_field_duration),
            TH_ANI_FIND_CLASS_METHOD(env, "std.core.Int", "unboxed", ":i"), &cpp_field_duration_cpp_temp_ani_value);
        int32_t cpp_field_duration_cpp_temp = static_cast<int32_t>(cpp_field_duration_cpp_temp_ani_value);
        cpp_field_duration.emplace(std::move(cpp_field_duration_cpp_temp));
    };
    return ::ohos::bluetooth::ble::AdvertisingEnableParams { std::move(cpp_field_advertisingId),
                                                             std::move(cpp_field_duration) };
}
}  // namespace Bluetooth
}  // namespace OHOS
