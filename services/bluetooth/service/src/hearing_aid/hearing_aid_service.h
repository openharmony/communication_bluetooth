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

#ifndef HEARING_AID_SERVICE_H
#define HEARING_AID_SERVICE_H

#include <cmath>
#include <cstring>
#include <list>
#include <memory.h>
#include <shared_mutex>
#include <vector>

#include "base_def.h"
#include "context.h"
#include "service_util.h"
#include "hearing_aid_message.h"
#include "interface_profile_hearing_aid.h"
#include "hearing_aid_service_impl_interface.h"

namespace OHOS {
namespace bluetooth {

class OHHearingAidCallbacks : public ::bluetooth::hearing_aid::HearingAidCallbacks {
    /** Callback for profile connection state change */
    void OnConnectionState(::bluetooth::hearing_aid::ConnectionState state, const BLUEDROID::RawAddress& address);

    /** Callback for device being available. Is executed when devices are loaded
    * from storage on stack bringup, and when new device is connected to profile.
    * Main purpose of this callback is to keep its users informed of device
    * capabilities and hiSyncId.
    */
    void OnDeviceAvailable(uint8_t capabilities, uint64_t hiSyncId, const BLUEDROID::RawAddress& address);
};

class HearingAidService : public IProfileHearingAid, public utility::Context {
public:
    static constexpr char const *DEFAULT_LIB_NAME = "libbthearing_aid_impl.z.so";
    static constexpr char const *DEFAULT_LIB_CREATE_FUNC_NAME = "CreateHearingAidServiceImplInterface";
    static constexpr char const *DEFAULT_LIB_DESTROY_FUNC_NAME = "DestroyHearingAidServiceImplInterface";
    static constexpr uint32_t DEFAULT_UNLOAD_TIMER_MS = 600000;
    /**
     * @brief Get the instance of the HearingAid Service object.
     *
     * @return Returns the instance of the HearingAid Service object.
     */
    static HearingAidService *GetService();
    /**
     * @brief Construct a new HearingAid Service object
     *
     */
    HearingAidService();
    /**
     * @brief Destroy the HearingAid Service object
     *
     */
    ~HearingAidService() override;

    // 覆写iprofile
    utility::Context *GetContext() override;
    int Connect(const RawAddress &device) override;
    int Disconnect(const RawAddress &device) override;
    std::list<RawAddress> GetConnectDevices() override;
    int GetConnectState(void) override;
    int GetMaxConnectNum(void) override;

    // 覆写IProfileHearingAid
    std::vector<RawAddress> GetDevicesByStates(std::vector<int> &states) override;
    int GetDeviceState(const RawAddress &device) override;
    int SetActiveDevice(const RawAddress &device) override;
    void GetActiveDevice(RawAddress &addr, std::string &name) override;
    int SetVolumeByAddr(const RawAddress &device, int volume) override;
    int GetHearingAidDeviceInfo(const RawAddress &device, int& side, int mode, int64_t& hiSyncId) override;
    int SetConnectStrategy(const RawAddress &device, int strategy) override;
    int GetConnectStrategy(const RawAddress &device) override;
    void RegisterObserver(IHearingAidObserver &HearingAidObserver) override;
    void DeregisterObserver(IHearingAidObserver &HearingAidObserver) override;
    bool IsLe2MPhySupported() override;

    // 覆写context
    void Enable(void) override;
    void Disable(void) override;

    ::bluetooth::hearing_aid::HearingAidInterface *getBluetoothHearingAidInterface() const;

    void RemoveStateMachine(const std::string &device);

    bool AllowConnect(const RawAddress &device);

    void PostEvent(const HearingAidMessage &event);

    void ProcessEvent(const HearingAidMessage &event);

    void ShutDownDone(bool isAllDisconnected);

    int ConnectAllDevice(const std::string &address);

    void AddHearingAidDeviceInfo(const RawAddress &device, uint8_t capabilities, uint64_t hiSyncId);

    bool GetVolume(const std::string &addr, int &volume);

    bool IsLeftDevice(bool &isLeft, const std::string &addr);

    void StartStackProfile(void) override;

    static constexpr uint64_t HI_SYNC_ID_INVALID = 0;

private:
    std::string ifname_ {"bt-hearing-aid"};
    HearingAidServiceData serviceData_;
    std::shared_ptr<HearingAidServiceImplInterface> serviceImpl_ = nullptr;
    CxxDynamicLibraryLoader<HearingAidServiceImplInterface> loader_;
    std::unique_ptr<OHHearingAidCallbacks> bluetoothHearingAidCallbacks_;
    std::mutex initMutex_;
    void StartUp();

    void ShutDown();

    void LoadServiceImpl(const char *func);

    int DisconnectImpl(const RawAddress &device, const int eventAction);

    bool GetVolumeFromSettingsData(bool isLeft, std::string& volumePercent);
    // msg process func
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // HEARING_AID_SERVICE_H
