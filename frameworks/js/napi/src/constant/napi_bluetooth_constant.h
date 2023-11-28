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

#ifndef NAPI_BLUETOOTH_CONSTANT_H_
#define NAPI_BLUETOOTH_CONSTANT_H_

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace Bluetooth {
class NapiConstant {
public:
    static napi_value DefineJSConstant(napi_env env, napi_value exports);

private:
    static napi_value ConstantPropertyValueInit(napi_env env, napi_value exports);
    static napi_value ProfileIdInit(napi_env env);
    static napi_value ProfileUuidsInit(napi_env env);
    static napi_value ProfileStateInit(napi_env env);
    static napi_value MajorClassOfDeviceInit(napi_env env);
    static napi_value MajorMinorClassOfDeviceInit(napi_env env);
    static napi_value AccessAuthorizationInit(napi_env env);
    static napi_value ComputerMajorClassOfDeviceInit(napi_env env, napi_value majorMinorClass);
    static napi_value PhoneMajorClassOfDeviceInit(napi_env env, napi_value majorMinorClass);
    static napi_value NetworkMajorClassOfDeviceInit(napi_env env, napi_value majorMinorClass);
    static napi_value AudioVideoMajorClassOfDeviceInit(napi_env env, napi_value majorMinorClass);
    static napi_value PeripheralMajorClassOfDeviceInit(napi_env env, napi_value majorMinorClass);
    static napi_value ImagingMajorClassOfDeviceInit(napi_env env, napi_value majorMinorClass);
    static napi_value WearableMajorClassOfDeviceInit(napi_env env, napi_value majorMinorClass);
    static napi_value ToyMajorClassOfDeviceInit(napi_env env, napi_value majorMinorClass);
    static napi_value HealthMajorClassOfDeviceInit(napi_env env, napi_value majorMinorClass);
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif /* NAPI_BLUETOOTH_CONSTANT_H_ */
