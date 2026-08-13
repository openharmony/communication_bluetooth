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

#ifndef BLUETOOTH_FASTSCAN_MANAGER_LOADER_H
#define BLUETOOTH_FASTSCAN_MANAGER_LOADER_H

#include <mutex>
#include "bluetooth_hw_interface_hook.h"

namespace OHOS {
namespace bluetooth {

class BluetoothFastScanManagerLoader {
public:
    using LIBSETFASTSCANENABLE = void (*)(bool);
    static BluetoothFastScanManagerLoader& GetInstance();
    void Init();
    LIBSETFASTSCANENABLE getSetFastScanEnabledHandle();

private:
    BluetoothFastScanManagerLoader() = default;
    ~BluetoothFastScanManagerLoader();
    void CleanUp();
    
    std::mutex load_mutex;
    void* handle_;
    HfpAgServiceFuncs hfpAgServiceFuncs_;
    A2dpServiceFuncs a2dpServiceFuncs_;
    using VOIDLIBFUC = void (*) ();
    using LIBINITFUNC = void (*)(HfpAgServiceFuncs* hfpFuncs, A2dpServiceFuncs* a2dpFuncs);
    bool isLoaded_ = false;
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // BLUETOOTH_FASTSCAN_MANAGER_LOADER_H
