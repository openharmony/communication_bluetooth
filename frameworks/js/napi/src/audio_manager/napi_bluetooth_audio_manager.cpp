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

#include <uv.h>
#include "napi_bluetooth_audio_manager.h"
#include "bluetooth_audio_manager.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "napi_bluetooth_error.h"
#include "napi_async_work.h"
#include "napi_bluetooth_utils.h"
#include "parser/napi_parser_utils.h"
#include "napi_bluetooth_utils.h"

namespace OHOS {
namespace Bluetooth {

const int WEARDETECTION_ENABLED = 1;
const int ENABLE_WEARDETECTION_UNSUPPORT = -1;

void NapiBluetoothAudioManager::DefineSystemWearDetectionInterface(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("enableWearDetection", EnableWearDetection),
        DECLARE_NAPI_FUNCTION("disableWearDetection", DisableWearDetection),
        DECLARE_NAPI_FUNCTION("isWearDetectionEnabled", IsWearDetectionEnabled),
        DECLARE_NAPI_FUNCTION("isWearDetectionSupported", IsWearDetectionSupported),
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    HILOGI("DefineSystemWearDetectionInterface init");
}

napi_value NapiBluetoothAudioManager::EnableWearDetection(napi_env env, napi_callback_info info)
{
    HILOGD("start");
    std::string remoteAddr{};
    auto status = CheckDeviceAddressParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr]() {
        BluetoothAudioManager &wd = BluetoothAudioManager::GetInstance();
        int32_t err = wd.EnableWearDetection(remoteAddr);
        return NapiAsyncWorkRet(err);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiBluetoothAudioManager::DisableWearDetection(napi_env env, napi_callback_info info)
{
    HILOGD("start");
    std::string remoteAddr{};
    auto status = CheckDeviceAddressParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr]() {
        BluetoothAudioManager &wd = BluetoothAudioManager::GetInstance();
        int32_t err = wd.DisableWearDetection(remoteAddr);
        return NapiAsyncWorkRet(err);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiBluetoothAudioManager::IsWearDetectionEnabled(napi_env env, napi_callback_info info)
{
    HILOGD("start");
    std::string remoteAddr{};
    auto status = CheckDeviceAddressParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr]() {
        int32_t ability = ENABLE_WEARDETECTION_UNSUPPORT;
        BluetoothAudioManager &wd = BluetoothAudioManager::GetInstance();
        int32_t err = wd.GetWearDetectionState(remoteAddr, ability);
        if (ability == WEARDETECTION_ENABLED) {
            return NapiAsyncWorkRet(err, std::make_shared<NapiNativeBool>(true));
        }
        return NapiAsyncWorkRet(err, std::make_shared<NapiNativeBool>(false));
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiBluetoothAudioManager::IsWearDetectionSupported(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::string remoteAddr{};
    auto status = CheckDeviceAddressParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr]() {
        BluetoothAudioManager &audioManager = BluetoothAudioManager::GetInstance();
        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        bool isSupported = false;
        int32_t err = audioManager.IsWearDetectionSupported(device, isSupported);
        HILOGI("isSupported: %{public}d", isSupported);
        return NapiAsyncWorkRet(err, std::make_shared<NapiNativeBool>(isSupported));
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

} // namespace Bluetooth
} // namespace OHOS
