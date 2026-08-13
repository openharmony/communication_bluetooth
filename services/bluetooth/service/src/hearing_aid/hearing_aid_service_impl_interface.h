/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#ifndef HEARING_AID_SERVICE_IMPL_INTERFACE_H
#define HEARING_AID_SERVICE_IMPL_INTERFACE_H

#include "context.h"
#include "hearing_aid_message.h"
#include "hearing_aid_service_data.h"

namespace OHOS {
namespace bluetooth {
class HearingAidServiceImplInterface {
public:
    HearingAidServiceImplInterface() = default;
    virtual ~HearingAidServiceImplInterface() = default;

    virtual void SetGetDeviceNameFunc(std::function<std::string(const RawAddress &device)> func) = 0;

    virtual void SetGetAllowConnectFunc(std::function<bool(const RawAddress &device)> func) = 0;

    virtual void SetServiceDataFunc(std::function<HearingAidServiceData&()> func) = 0;

    virtual void SetServicePosteventFunc(std::function<void(const HearingAidMessage &event)> func) = 0;

    virtual void SetGetStorageVolumeFunc(std::function<bool(const std::string &addr, int &volume)> func) = 0;

    virtual int SetActiveDevice(const RawAddress &device, HearingAidServiceData &data) = 0;

    virtual void ProcessEvent(const HearingAidMessage &event, HearingAidServiceData &data) = 0;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // HEARING_AID_SERVICE_IMPL_INTERFACE_H