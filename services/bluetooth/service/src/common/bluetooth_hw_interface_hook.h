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

#ifndef BLUETOOTH_HW_INTERFACE_HOOK_H
#define BLUETOOTH_HW_INTERFACE_HOOK_H

#include <functional>

namespace OHOS {
namespace bluetooth {


struct HfpAgServiceFuncs {
    using SendAtCmdCb = void (*)(int type);

    SendAtCmdCb sendAtCmdFunc;
    using HasHfpConnectedDevicesForCurrentMobilecb = bool(*)();
    HasHfpConnectedDevicesForCurrentMobilecb hasHfpConnectedDevicesForCurrentMobilefunc;
};

struct A2dpServiceFuncs {
    using IsA2dpPlayingCb = bool (*)();
    using SetA2dpPlayingStateCb = void (*)(std::function<void(int)>);
    using HasA2dpConnectedDevicesForCurrentMobilecb = bool(*)();
    IsA2dpPlayingCb isA2dpPlayingCb = nullptr;
    SetA2dpPlayingStateCb setA2dpPlayingCb = nullptr;
    HasA2dpConnectedDevicesForCurrentMobilecb hasA2dpConnectedDevicesForCurrentMobilefunc;
};

void GetHfpAgServiceFuncs(HfpAgServiceFuncs& funcs);
void GetA2dpServiceFuncs(A2dpServiceFuncs& funcs);

}
}
# endif //BLUETOOTH_HW_INTERFACE_HOOK_H