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

#ifndef PBAP_PCE_UTILS_H
#define PBAP_PCE_UTILS_H

#include <cstdint>
#include <cstring>
#include "base_def.h"
#include "../obex/obex_def.h"
#include "pbap_pce_def.h"

namespace OHOS {
namespace bluetooth {

struct PbapPceConfig {
    bool srmEnable_ = true;
    int rfcommMtu_ = OBEX_DEFAULT_MTU;
    int l2capMtu_ = OBEX_DEFAULT_MTU;
    int pceMaxDevices_ = PCE_MAXIMUM_DEVICES;
};

enum PbapPceServiceStateType {
    PBAP_PCE_STATE_STARTUP,
    PBAP_PCE_STATE_STARTUPING,
    PBAP_PCE_STATE_SHUTDOWN,
    PBAP_PCE_STATE_SHUTDOWNING,
};

enum PbapPceTargetStateType {
    PCE_TARGET_STATE_UNKNOWN,
    PCE_TARGET_STATE_CONNECTED,
    PCE_TARGET_STATE_DISCONNECTED,
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // PBAP_PCE_UTILS_H
