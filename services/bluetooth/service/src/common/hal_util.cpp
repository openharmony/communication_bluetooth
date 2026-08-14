/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_hal_util"
#endif

#include "hal_util.h"

#include <dlfcn.h>
#include <mutex>

#include "bt_dm_interface.h"
#include "log.h"

namespace {
std::once_flag g_loadOnceFlag;
const bt_interface_t *g_btInterface = nullptr;
int g_loadResult = 0;

/* Stack library selection: the loader entry stays the same, only the .so
 * name is chosen at build time. The closed-source vendor stack keeps the
 * historical name; the open stack is the btstack DM in libbtstack.z.so. */
const char *GetStackLibraryName()
{
#ifdef BT_USE_OPEN_STACK
    return "libbtstack.z.so";
#else
    return "libbluetooth_vendor.z.so";
#endif
}

/* Both stacks must export the same entry symbol (bluedroid HAL contract). */
using GetInterfaceFn = const bt_interface_t *(*)();

void LoadBluetoothLibrary()
{
    const char *libName = GetStackLibraryName();
    void *handle = dlopen(libName, RTLD_NOW);
    if (handle == nullptr) {
        HILOGE("Failed to dlopen %{public}s: %{public}s", libName, dlerror());
        g_loadResult = -1;
        return;
    }
    GetInterfaceFn getter = reinterpret_cast<GetInterfaceFn>(dlsym(handle, BT_DM_GET_INTERFACE_SYMBOL));
    if (getter == nullptr) {
        HILOGE("Failed to dlsym %{public}s from %{public}s: %{public}s", BT_DM_GET_INTERFACE_SYMBOL, libName,
            dlerror());
        g_loadResult = -1;
        return;
    }
    g_btInterface = getter();
    if (g_btInterface == nullptr) {
        HILOGE("%{public}s returned a null bt_interface_t", libName);
        g_loadResult = -1;
        return;
    }
    HILOGI("Bluetooth stack loaded: %{public}s", libName);
    g_loadResult = 0;
}
}  // namespace

int hal_util_load_bt_library(const bt_interface_t **btInterface)
{
    if (btInterface == nullptr) {
        return -1;
    }
    std::call_once(g_loadOnceFlag, LoadBluetoothLibrary);
    *btInterface = g_btInterface;
    return g_loadResult;
}
