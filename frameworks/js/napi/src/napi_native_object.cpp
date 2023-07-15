/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "napi_native_object.h"

namespace OHOS {
namespace Bluetooth {
napi_value NapiNativeInt::ToNapiValue(napi_env env) const
{
    napi_value value = nullptr;
    napi_status status = napi_create_int32(env, value_, &value);
    if (status != napi_ok) {
        HILOGE("napi_create_int32 failed");
    }
    return value;
}

napi_value NapiNativeString::ToNapiValue(napi_env env) const
{
    napi_value value = nullptr;
    napi_status status = napi_create_string_utf8(env, value_.c_str(), NAPI_AUTO_LENGTH, &value);
    if (status != napi_ok) {
        HILOGE("napi_create_string_utf8 failed");
    }
    return value;
}

napi_value NapiNativeBleCharacteristic::ToNapiValue(napi_env env) const
{
    napi_value object;
    napi_create_object(env, &object);
    ConvertBLECharacteristicToJS(env, object, const_cast<GattCharacteristic &>(character_));
    return object;
}

napi_value NapiNativeBleDescriptor::ToNapiValue(napi_env env) const
{
    napi_value object;
    napi_create_object(env, &object);
    ConvertBLEDescriptorToJS(env, object, const_cast<GattDescriptor &>(descriptor_));
    return object;
}

napi_value NapiNativeUuidsArray::ToNapiValue(napi_env env) const
{
    napi_value array;
    napi_create_array(env, &array);
    ConvertUuidsVectorToJS(env, array, uuids_);
    return array;
}
}  // namespace Bluetooth
}  // namespace OHOS