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

#ifndef PBAP_PCE_SERVICE_IMPL_H
#define PBAP_PCE_SERVICE_IMPL_H

#include <cstring>
#include <list>
#include <map>
#include <vector>
#include "base_def.h"
#include "btcommon/message.h"
#include "pbap_pce_def.h"
#include "pbap_pce_message.h"
#include "pbap_pce_state_machine.h"
#include "raw_address.h"
#include "pbap_pce_service_impl_interface.h"
#include "pbap_pce_sdp.h"
#include "pbap_pce_utils.h"
#include "common_util.h"
#include "thread_util.h"
#include "bt_recursive_mutex.h"


namespace OHOS {
namespace bluetooth {

class PbapPceServiceImpl : public PbapPceServiceImplInterface {
public:
    PbapPceServiceImpl();
    ~PbapPceServiceImpl() override;

    void RegisterObserver(std::shared_ptr<IPbapPceObserver> &observer) override;
    void DeregisterObserver(std::shared_ptr<IPbapPceObserver> &observer) override;

    int32_t Connect(const RawAddress &device) override;
    int32_t Disconnect(const RawAddress &device) override;
    std::vector<RawAddress> GetDevicesByStates(const std::vector<int32_t> &states) override;
    int32_t GetDeviceState(const RawAddress &device) override;
    int32_t GetConnectState() override;
    bool IsDownloading(const RawAddress &device) override;
    int32_t SetConnectionStrategy(const RawAddress &device, int strategy) override;
    int32_t GetConnectionStrategy(const RawAddress &device) override;

    void Enable() override;
    void Disable() override;
    int32_t GetPhoneBookSyncState(const RawAddress &device) override;

    void SetTryShutDownCallback(const std::function<void(bool)> &callback);

    const PbapPceConfig &GetPceConfig() const;
    void OnObexResponse(const RawAddress &device, const utility::Message &msg);
    void ProcessPhonebookMessage(const RawAddress &device, const utility::Message &msg);
    void SetPowerStatusBusy(const RawAddress &device, bool busy);
    bool IsBusy(const RawAddress &device);
    bool IsConnected(const RawAddress &device);
    bool SetBusy(const RawAddress &device, bool busy);
    void ForwardMsgTosm(const RawAddress &device, const utility::Message &msg);

private:
    void PostConnectEvent(const PbapPceMessage &event);
    void ProcessConnectEvent(const PbapPceMessage &event);
    int ConnectInternal(const RawAddress &device);
    int DisconnectInternal(const RawAddress &device);
    int GetMaxConnectNum();
    std::vector<RawAddress> GetConnectedDevices();
    bool SaveConnectPolicy(const std::string &addr, int strategy);
    bool LoadConnectPolicy(const std::string &addr, int &strategy);
    void ProcessObexRespMessage(const RawAddress &device, const utility::Message &msg);
    void ProcessObexConnected(const RawAddress &device, const utility::Message &msg);
    void ProcessObexDisconnected(const RawAddress &device, const utility::Message &msg);
    void ProcessPhoneBookActionCompleted(const RawAddress &device, const utility::Message &msg);
    void OnSdpResponse(const std::string &addr, const utility::Message &msg);
    void ProcessSdpMessage(const RawAddress &device, const utility::Message &msg);
    void LoadPceConfig();

    std::recursive_mutex machineMapMutex_;
    std::map<std::string, std::unique_ptr<utility::StateMachine>> machineMap_;
    std::shared_ptr<IPbapPceObserver> pceConnectionObserver_;
    std::unique_ptr<PbapPceSdp> pbapPceSdp_;
    PbapPceServiceStateType serviceState_ = PBAP_PCE_STATE_SHUTDOWN;
    PbapPceConfig pbapPceConfig_;
    std::function<void(bool)> tryShutDownCallback_;
    BtRecursiveMutex mutex_;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // PBAP_PCE_SERVICE_IMPL_H
