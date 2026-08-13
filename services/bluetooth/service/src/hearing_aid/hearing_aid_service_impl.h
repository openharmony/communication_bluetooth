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

#ifndef HEARING_AID_SERVICE_IMPL_H
#define HEARING_AID_SERVICE_IMPL_H

#include <cmath>
#include <cstring>
#include <list>
#include <shared_mutex>
#include <vector>

#include "base_def.h"
#include "log.h"

#include "hearing_aid_service_impl_interface.h"
#include "hearing_aid_service_data.h"

namespace OHOS {
namespace bluetooth {

class HearingAidServiceImpl : public HearingAidServiceImplInterface {
public:
    static HearingAidServiceImpl *GetServiceImpl();

    void SetGetDeviceNameFunc(std::function<std::string(const RawAddress &device)> func) override;

    void SetGetAllowConnectFunc(std::function<bool(const RawAddress &device)> func) override;

    void SetServiceDataFunc(std::function<HearingAidServiceData&()> func) override;

    void SetServicePosteventFunc(std::function<void(const HearingAidMessage &event)> func) override;

    void SetGetStorageVolumeFunc(std::function<bool(const std::string &addr, int &volume)> func) override;

    int SetActiveDevice(const RawAddress &device, HearingAidServiceData &data) override;

    void ProcessEvent(const HearingAidMessage &event, HearingAidServiceData &data) override;

    std::list<RawAddress> GetConnectDevices(HearingAidServiceData &data);

    void NotifyStateChanged(
        const RawAddress &device, int fromState, int toState, int reason, HearingAidServiceData &data);

    void NotifyActiveDeviceChanged(const RawAddress &device, HearingAidServiceData &data);

    static const char* GetHearingAidEventName(int what);

    bool GetServiceAllowConnect(const RawAddress &device);
    HearingAidServiceData &GetServiceData();
    void PostEventToService(const HearingAidMessage &event);
    std::string GetDeviceName(const RawAddress &device);
    bool IsConnectedPeerDevices(const RawAddress &device, HearingAidServiceData &data);
    static void SetServiceImpl(HearingAidServiceImpl *serviceImpl);

private:
    std::function<std::string(const RawAddress &device)> getDeviceName_{nullptr};
    std::function<bool(const RawAddress &device)> getServiceAllowConnect_{nullptr};
    std::function<HearingAidServiceData&()> getServiceData_{nullptr};
    std::function<void(const HearingAidMessage &event)> servicePostFunc_{nullptr};
    std::function<bool(const std::string &addr, int &volume)> serviceGetVolumeFunc_{nullptr};
    int GetProcessingDeviceNum(HearingAidServiceData &data);
    // msg process func
    void ProcessDisconnectAll();
    void ProcessRemoveStateMachine(const std::string &address, HearingAidServiceData &data);
    void ProcessStateMessage(const HearingAidMessage &event, HearingAidServiceData &data);
    void ProcessVolumeSet(const HearingAidMessage &event, HearingAidServiceData &data);
    void ProcessConnectionTimeout(const HearingAidMessage &event, HearingAidServiceData &data);
    bool IsTheSameIdWithActiveDevice(const RawAddress &device, HearingAidServiceData &data);
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // HEARING_AID_SERVICE_H
