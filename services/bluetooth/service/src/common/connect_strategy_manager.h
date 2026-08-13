/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef CONNECT_STRATEGY_MANAGER_H
#define CONNECT_STRATEGY_MANAGER_H

#include <map>
#include "remote_device_properties.h"
#include "btcommon/timer_manager.h"
#include "safe_map.h"
#include "bluetooth_common_event_helper.h"

namespace OHOS {
namespace bluetooth {

constexpr const char* NEARLINK_RECONN_DEVICE_BT_ADDR = "persist.nearlink.reconn_device_bluetooth_address";

class ConnectStrategyManager {
public:
    static ConnectStrategyManager* GetInstance();

    void AutoConnect();
    void InitConnectStrategy(const std::string &addr);
    int QueryConnectStrategy(const RawAddress &device, const std::string &profile);
    bool SaveConnectStrategy(const RawAddress &device, const std::string &profile, int strategy);
    bool SaveLastActiveDevice(const std::string &addr);
    bool SaveLastConnectedDevice(const std::string &addr);
    void UpdateAutoConnectDeivce(const std::string &addr);
    void ProfileConnectionStateChange(const std::string &addr, const std::string &profileName);
    void EraseDevicePreviouslyConnected(const std::string &addr);
    bool QueryAutoConnectDevice(std::string &addr);
private:
    RemoteDeviceProperties *remoteDeviceProperties_;
    ConnectStrategyManager();
    bool IsA2dpNeedInit(const RawAddress &device, std::vector<Uuid> &uuids);
    bool IsHfpNeedInit(const RawAddress &device, std::vector<Uuid> &uuids);
    bool IsHidNeedInit(const RawAddress &device, std::vector<Uuid> &uuids);
    bool IsHidDeviceNeedInit(const RawAddress &device, std::vector<Uuid> &uuids);
    bool IsHearingAidNeedInit(const RawAddress &device, std::vector<Uuid> &uuids);
    bool IsPanNeedInit(const RawAddress &device, std::vector<Uuid> &uuids);
#ifdef BLUETOOTH_BAS_FEATURE_ENABLE
    bool IsBasNeedInit(const RawAddress &device, std::vector<Uuid> &uuids);
#endif
#ifdef BLUETOOTH_HFP_HF_ENABLE
    bool IsHfpHfNeedInit(const RawAddress &device, std::vector<Uuid> &uuids);
#endif
    bool IsProfileNeedDelayConnect(const RawAddress &device);
    void StrategyHandleProfileWaitConnect(const std::string addr, const std::string profileName);
    void AutoConnectProfile(std::string autoConnectDeviceAddr);
    void ReportAutoConnectResultChr(const std::string& peerAddr,
        bool isHfpResult, bool isA2dpResult, bool isHearingAidResult);
    bool IsProfileConnectFail(int profileStatus, bool profileAllowed);
    bool IsAllProfilesDisconnect(const std::string addr);
    bool IsAutoConnectingAllowed(const RawAddress &device);
    bool PublishAutoConnDevice(std::function<bool(std::string &)> func, const std::string &eventType);
    static bool QueryLastActiveDevice(std::string &addr);
    static bool QueryLastConnectedDevice(std::string &addr);
    bool StrategyConnectProfile(const std::string addr, const std::string profileName);
    void StopProfileDelayConnectTimer(const std::string addr);
    void BtChrWriteConnectStrategy(const std::string &addr, const std::string &profileName, const int strategyValue);

    std::shared_ptr<utility::Timer> delayReconnectTimer {};
    SafeMap<std::string, std::shared_ptr<utility::Timer>> profileDelayConnect_ {};
    SafeMap<std::string, bool> DevicePreviouslyConnectedMap_ {};
};
} // namespace bluetooth
} // namespace OHOS
#endif