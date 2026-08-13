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

#ifndef PBAP_PCE_SERVICE_IMPL_INTERFACE_H
#define PBAP_PCE_SERVICE_IMPL_INTERFACE_H

#include <functional>
#include <memory>
#include <vector>
#include "raw_address.h"
#include "interface_profile_pbap_pce.h"
#include "bluetooth_errorcode.h"

namespace OHOS {
namespace bluetooth {

class PbapPceServiceImplInterface {
public:
    PbapPceServiceImplInterface() = default;
    virtual ~PbapPceServiceImplInterface() = default;

    virtual void RegisterObserver(std::shared_ptr<IPbapPceObserver> &observer) = 0;
    virtual void DeregisterObserver(std::shared_ptr<IPbapPceObserver> &observer) = 0;

    virtual int32_t Connect(const RawAddress &device) = 0;
    virtual int32_t Disconnect(const RawAddress &device) = 0;
    virtual std::vector<RawAddress> GetDevicesByStates(const std::vector<int32_t> &states) = 0;
    virtual int32_t GetDeviceState(const RawAddress &device) = 0;
    virtual int32_t GetConnectState() = 0;
    virtual bool IsDownloading(const RawAddress &device) = 0;
    virtual int32_t SetConnectionStrategy(const RawAddress &device, int strategy) = 0;
    virtual int32_t GetConnectionStrategy(const RawAddress &device) = 0;

    virtual void Enable() = 0;
    virtual void Disable() = 0;
    virtual int32_t GetPhoneBookSyncState(const RawAddress &device) = 0;
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // PBAP_PCE_SERVICE_IMPL_INTERFACE_H
