<<<<<<< HEAD
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
=======
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
>>>>>>> 399c6857 (update time)
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
<<<<<<< HEAD

=======

>>>>>>> 399c6857 (update time)
#include <uv.h>
#include "napi_bluetooth_bt_weardetection.h"
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
const int WEARDETECTION_SUPPORT_ON = 1;
const int WEARDETECTION_SUPPORT_OFF = 0;

void NapiBtWearDetection::DefineSystemWearDetectionInterface(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("enableWearDetection", EnableWearDetection),
        DECLARE_NAPI_FUNCTION("disableWearDetection", DisableWearDetection),
        DECLARE_NAPI_FUNCTION("isWearDetectionEnabled", IsWearDetectionEnabled),
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    HILOGI("DefineSystemWearDetectionInterface init");
}
 
napi_value NapiBtWearDetection::EnableWearDetection(napi_env env, napi_callback_info info)
{
    HILOGD("start");
    std::string remoteAddr{};
    auto status = CheckDeviceAddressParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
 
    auto func = [remoteAddr]() {
        int32_t ability = WEARDETECTION_SUPPORT_ON;
        BtAudioManager &wd = BtAudioManager::GetInstance();
        int32_t err = wd.EnableBtAudioManager(remoteAddr, ability);
        return NapiAsyncWorkRet(err);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}
 
napi_value NapiBtWearDetection::DisableWearDetection(napi_env env, napi_callback_info info)
{
    HILOGD("start");
    std::string remoteAddr{};
    auto status = CheckDeviceAddressParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
 
    auto func = [remoteAddr]() {
        int32_t ability = WEARDETECTION_SUPPORT_OFF;
        BtAudioManager &wd = BtAudioManager::GetInstance();
        int32_t err = wd.DisableBtAudioManager(remoteAddr, ability);
        return NapiAsyncWorkRet(err);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
 
}
 
napi_value NapiBtWearDetection::IsWearDetectionEnabled(napi_env env, napi_callback_info info)
{
    HILOGD("start");
    std::string remoteAddr{};
    auto status = CheckDeviceAddressParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
 
    auto func = [remoteAddr]() {
        BtAudioManager &wd = BtAudioManager::GetInstance();
        int err = wd.IsBtAudioManagerEnabled(remoteAddr);
        return NapiAsyncWorkRet(err, std::make_shared<NapiNativeInt>(err));
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
 
}
 
} // namespace Bluetooth
} // namespace OHOS