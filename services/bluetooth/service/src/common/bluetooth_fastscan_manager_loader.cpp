/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#undef LOG_TAG
#define LOG_TAG "bt_fastscan_manager_loader"
#include "bluetooth_fastscan_manager_loader.h"
#include <dlfcn.h>
#include "log.h"

namespace OHOS {
namespace bluetooth {
static constexpr char const *DEFAULT_LIB_NAME = "libbtfast_scan.z.so";
static constexpr char const *DEFAULT_LIB_CREATE_FUNC_NAME = "InitFastScanAndSetCallback";
static constexpr char const *GET_SETFASTSCANENABLE_FUNC_NAME = "SetFastScanEnabled";

BluetoothFastScanManagerLoader& BluetoothFastScanManagerLoader::GetInstance()
{
    static BluetoothFastScanManagerLoader instance;
    return instance;
}

void BluetoothFastScanManagerLoader::Init()
{
    std::lock_guard<std::mutex> lock(load_mutex);
    if (isLoaded_) {
        HILOGI("libbtfast_scan already Loaded.");
        return;
    }
    
    handle_ = dlopen(DEFAULT_LIB_NAME, RTLD_NOW);
    if (handle_ == nullptr) {
        HILOGI("can not open libbtfast_scan libarary");
        return;
    }
    isLoaded_ = true;
    LIBINITFUNC fastScanInit = (LIBINITFUNC) dlsym(handle_, DEFAULT_LIB_CREATE_FUNC_NAME);
    if (fastScanInit == nullptr) {
        HILOGI("can not find function %{public}s.", DEFAULT_LIB_CREATE_FUNC_NAME);
        CleanUp();
        return;
    }
    HILOGI("begin fastscan init.");
    GetHfpAgServiceFuncs(hfpAgServiceFuncs_);
    GetA2dpServiceFuncs(a2dpServiceFuncs_);
    fastScanInit(&hfpAgServiceFuncs_, &a2dpServiceFuncs_);
    return;
}

BluetoothFastScanManagerLoader::LIBSETFASTSCANENABLE BluetoothFastScanManagerLoader::getSetFastScanEnabledHandle()
{
    if (!isLoaded_) {
        return nullptr;
    }
    return (BluetoothFastScanManagerLoader::LIBSETFASTSCANENABLE)dlsym(handle_, GET_SETFASTSCANENABLE_FUNC_NAME);
}

void BluetoothFastScanManagerLoader::CleanUp()
{
    if (handle_ == nullptr) {
        return;
    }
    isLoaded_ = false;
    dlclose(handle_);
    return;
}

BluetoothFastScanManagerLoader::~BluetoothFastScanManagerLoader()
{
    CleanUp();
}
}  // namespace bluetooth
}  // namespace OHOS
