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

#ifndef PCE_SERVICE_H
#define PCE_SERVICE_H

#include <cstdint>
#include <cstring>
#include <list>
#include <map>
#include <memory>
#include "base_observer_list.h"
#include "context.h"
#include "interface_profile.h"
#include "interface_profile_pbap_pce.h"
#include "interface_profile_manager.h"
#include "pbap_pce_service_impl_interface.h"
#include "hitrace_meter.h"
#include "btcommon/dynamic_library_loader.h"
#include "bt_def.h"
#include "pbap_pce_app_params.h"

namespace OHOS {
namespace bluetooth {

class PbapPceServiceImpl;

class PbapPceService : public IProfilePbapPce, public utility::Context {
public:
    static constexpr char const *DEFAULT_LIB_NAME = "libbtpbap_pce.z.so";
    static constexpr char const *DEFAULT_LIB_CREATE_FUNC_NAME = "CreatePbapPceServiceImplInterface";
    static constexpr char const *DEFAULT_LIB_DESTROY_FUNC_NAME = "DestroyPbapPceServiceImplInterface";
    static constexpr uint32_t DEFAULT_UNLOAD_TIMER_MS = 600000;

    explicit PbapPceService();
    ~PbapPceService();

    utility::Context *GetContext() override;

    int Connect(const RawAddress &device) override;
    int Disconnect(const RawAddress &device) override;
    void RegisterObserver(IPbapPceObserver &observer) override;
    void DeregisterObserver(IPbapPceObserver &observer) override;
    void SetPhoneBookSyncState(const std::string &addr, PbapPcePhoneBookStateType state);
    int32_t GetPhoneBookSyncState(const RawAddress &device) override;
    std::vector<RawAddress> GetDevicesByStates(const std::vector<int> &states) override;
    int GetDeviceState(const RawAddress &device) override;
    int SetConnectionStrategy(const RawAddress &device, int strategy) override;
    int GetConnectionStrategy(const RawAddress &device) override;

    void Enable() override;
    void Disable() override;
    std::list<RawAddress> GetConnectDevices() override;
    int GetConnectState() override;
    int PullPhoneBook(const RawAddress &device) override;
    bool IsDownloading(const RawAddress &device) override;
    int AbortDownloading(const RawAddress &device) override;

    static PbapPceService *GetService();
    void NotifyStateChanged(const RawAddress &device, int state, int cause);
    void NotifyActionCompleted(const RawAddress &device, int respCode, int actionType);

private:
    int GetMaxConnectNum() override;
    int SetDevicePassword(const RawAddress &device, const std::string &password, const std::string &userId = "") override;

    void EnableService();
    void DisableService();
    bool TryShutDown(bool ret);

    BaseObserverList<IPbapPceObserver> pbapPceObservers_;
    std::shared_ptr<IPbapPceObserver> connectionObserver_;
    std::map<std::string, PbapPcePhoneBookStateType> phoneBookStateMap_;
    CxxDynamicLibraryLoader<PbapPceServiceImplInterface> loader_;
    BT_DISALLOW_COPY_AND_ASSIGN(PbapPceService);
};

class PbapPceObserver : public IPbapPceObserver {
public:
    ~PbapPceObserver() override = default;
    void OnConnectionStateChanged(const RawAddress &device, int32_t state, int32_t cause) override;
    void OnActionCompleted(const RawAddress &device, int respCode, int actionType) override;
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // PCE_SERVICE_H
