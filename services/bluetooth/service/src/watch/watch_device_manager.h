/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

#ifndef WATCH_DEVICE_MANAGER_H
#define WATCH_DEVICE_MANAGER_H

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include "safe_map.h"
#include "btcommon/timer_manager.h"
#include "hfp_hf_defines.h"
#include "watch_define.h"
namespace OHOS {
namespace bluetooth {
class BondedDeviceManager {
public:
    BondedDeviceManager() = default;
    explicit BondedDeviceManager(const std::string& addr);

    std::string GetAddressLocked() const;

    void resetAllStatus();

    bool IsAllowAgCallLocked();
    bool IsAllowConnectScoLocked();
    void AllHfCallFinishedLocked(bool isHfDisconnected);
    void ProcessCallStateActiveLocked(long currentTimeStamp);
    void ProcessCallStateFinishedLocked();
    void ControlScoTimeoutLocked();

    void SetCallIDLocked(int callId);
    void SetAllowHfScoConnectLocked(bool isAllow);
    void SetDialingByWatchLocked();
    bool IsDialingByWatchLocked();
    void SetIsHfOnCallLocked(bool isOnCall);
    bool IsHfOnCallLocked();
    void HandleVoipStateLocked(int voipState);
    bool ProcessCallStateCheckLocked(int callId);
    void SetPhoneConnectStateLocked(bool connected);
    void SetHfConnectedLocked(bool connected);
    void SetOnHfCallFalseLocked();
    void SetDisconnectHfpOnDemandHfCallLocked(bool hasCalls);
    bool isOnVoip_Locked();
    bool IsAcceptVoipByWatchLocked() const;
    bool IsWatchHealthConnectedLocked();
    bool IsHfConnectedLocked();
    void SetPhoneCreateConnectLocked(bool isPhoneCreate);
    void SetHfConnectionStateLocked(int state);
    int GetHfConnectionStateLocked();
    void SetIsConnectedLocked(bool connected);
    bool IsConnectedLocked();
    void SetIsSaisDeviceLocked(bool isSais);
    bool IsSaisDeviceLocked();
    bool IsJustConnectAclLocked();
    void SetJustConnectAclLocked(bool isJustConnect);
    bool IsRemovePairLocked();
    void SetRemovePairLocked(bool isRemovePair);
    bool IsRouteScoRunningLocked();
    bool IsPhoneConnectedLocked() const;

    void SetPhoneConnHfpLocked(bool isPhoneConn);
    bool IsPhoneConnHfpLocked() const;

    void SetNeedDialAfterHFConnectedLocked(bool isNeed);
    bool IsNeedDialAfterHFConnectedLocked() const;

    void SetWatchHealthConnectedLocked(bool connected);
    void SetHfConnectionStateDirectLocked(int state);

    void StartBtDisconnectTimerLocked(std::unique_ptr<utility::Timer> timer);
    void StopBtDisconnectTimerLocked();
    bool IsBtDisconnectTimerNullLocked() const;

    void StartConnectHfpOnDemandTimerLocked(std::unique_ptr<utility::Timer> timer);
    void StopConnectHfpOnDemandTimerLocked();
    bool IsConnectHfpOnDemandTimerNullLocked() const;

    void StartConnectHfpTimerLocked(std::unique_ptr<utility::Timer> timer);
    void StopConnectHfpTimerLocked();
    bool IsConnectHfpTimerNullLocked() const;

    void StartControlScoConnectTimerLocked(std::unique_ptr<utility::Timer> timer);
    void StopControlScoConnectTimerLocked();
    bool IsControlScoConnectTimerNullLocked() const;

    void StopAllTimers();

    bool ShouldDisconnectHfpOnDemand()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return isHfConnected_ && isWatchHealthConnected_ && !isOnHfCall_;
    }

    bool ShouldConnectHfpOnDemand()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return !isHfConnected_ && !isWatchHealthConnected_ && isConnected;
    }

    int SetAclConnectedAndGetDelay()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        isConnected = true;
        return isPhoneConnHfp_ ? DELAY_TIME_CONNECT_HF_PHONE_CC : DELAY_TIME_CONNECT_HF_WATCH_CC;
    }

    bool CheckAndUpdateScoAllow(long currentTimeStamp, int delayTimeRouteSco)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!isOnHfCall_ && isAcceptVoipByWatch) {
            return true;
        }
        if (!isAllowHfScoConnect_ && callActiveTime_ != 0 &&
            (currentTimeStamp - callActiveTime_) >= delayTimeRouteSco) {
            isAllowHfScoConnect_ = true;
        }
        if (isAllowHfScoConnect_) {
            return true;
        }
        return false;
    }

private:
    std::string address;
    bool isConnected{false};
    bool isSaisDevice{false};
    long callActiveTime_{0};

    int callID_{0};
    bool isOnVoip_{false};
    bool isAcceptVoipByWatch{false};
    bool isDialingByWatch_{false};

    bool isRouteScoRunning_{false};
    bool isAllowHfScoConnect_{false};
    bool isNeedDialAfterHFConnected_{false};
    bool isPhoneConnected_{false};
    bool isJustConnectAcl_{true};
    bool isPhoneConnHfp_{false};
    bool isHfConnected_{false};
    bool isOnHfCall_{false};
    bool isRemovePair_{false};
    bool isWatchHealthConnected_{false};
    bool isPhoneCreateConnect_{false};
    int hfConnectionState_{HFP_HF_STATE_DISCONNECTED};

    std::unique_ptr<utility::Timer> btDisconnectTimer_;
    std::unique_ptr<utility::Timer> connectHfpOnDemandTimer_;
    std::unique_ptr<utility::Timer> connectHfpTimer_;
    std::unique_ptr<utility::Timer> controlScoConnectTimer_;

    mutable std::mutex mutex_;
};

// 全局设备管理类
class GlobalDeviceManager {
private:
    SafeMap<std::string, std::shared_ptr<BondedDeviceManager>> deviceManagers_;

    GlobalDeviceManager() = default;
    ~GlobalDeviceManager() = default;

public:
    // 单例模式
    static GlobalDeviceManager& getInstance();

    // 禁止拷贝
    GlobalDeviceManager(const GlobalDeviceManager&) = delete;
    GlobalDeviceManager& operator=(const GlobalDeviceManager&) = delete;

    //=== 设备管理接口 ===
    bool AddDevice(const std::string& address);
    bool RemoveDevice(const std::string& address);
    std::shared_ptr<BondedDeviceManager> GetDevice(const std::string& address);
    std::shared_ptr<BondedDeviceManager> GetSaisDevice();

    // 批量操作
    std::vector<std::string> GetAllAddresses();
    size_t GetDeviceCount();
    bool DeviceExists(const std::string& address);
    bool IsAllowConnectHfpAgSco();

    // 便捷操作函数
    bool ResetDeviceStatus(const std::string& address);
};

// 全局便捷访问函数
std::shared_ptr<BondedDeviceManager> GetDeviceManager(const std::string& address);
bool RemoveDeviceManager(const std::string& address);
bool AddDeviceManager(const std::string& address);
}
}
#endif // WATCH_DEVICE_MANAGER_H