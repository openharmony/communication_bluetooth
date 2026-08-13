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

#ifndef HDF_DYNAMIC_LIBRARY_LOADER
#define HDF_DYNAMIC_LIBRARY_LOADER

#include "btcommon/dynamic_library_loader.h"
#include "thread_util.h"

#include "idevmgr_hdi.h"
#include "hdf_remote_service.h"
#include "servmgr_hdi.h"
#include "hdf_device_class.h"

const uint32_t DEFAULT_UNLOAD_TIMER_MS = 5000;

namespace OHOS {
namespace bluetooth {
class HdfDynamicLibraryLoader : public DynamicLibraryLoader {
private:
    explicit HdfDynamicLibraryLoader()
        : DynamicLibraryLoader("", DEFAULT_UNLOAD_TIMER_MS, GetDefaultThreadFunc())
    {}

    static std::function<void(const ThreadUtilFunc &func)> GetDefaultThreadFunc()
    {
        return [](const ThreadUtilFunc &func) { DoInA2dpThread(func); };
    }

public:
    static HdfDynamicLibraryLoader& GetInstance();

    bool LoadTask() override;
    bool UnloadTask() override;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif