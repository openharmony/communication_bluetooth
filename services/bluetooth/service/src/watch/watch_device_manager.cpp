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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_device"
#endif

#include "watch_device_manager.h"
#include <chrono>
#include <iostream>
#include "bt_chr_base.h"
#include "bt_chr_dft_exception.h"
#include "hfp_hf_service.h"
#include "connect_strategy_manager.h"
#include "watch_define.h"
#include "profile_config.h"
#include "common_util.h"

namespace OHOS {
namespace bluetooth {
// BondedDeviceManager 成员函数实现
BondedDeviceManager::BondedDeviceManager(const std::string &addr) : address(addr)
{}

std::string BondedDeviceManager::GetAddressLocked() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return address;
}

void BondedDeviceManager::resetAllStatus()
{
    std::lock_guard<std::mutex> lock(mutex_);
    isConnected = false;
    callActiveTime_ = 0;
    callID_ = 0;
    isOnVoip_ = false;
    isAcceptVoipByWatch = false;
    isDialingByWatch_ = false;
    isRouteScoRunning_ = false;
    isAllowHfScoConnect_ = false;
    isNeedDialAfterHFConnected_ = false;
    isPhoneConnected_ = false;
    isJustConnectAcl_ = false;
    isPhoneConnHfp_ = false;
    isWatchHealthConnected_ = false;
    isPhoneCreateConnect_ = false;
}

// GlobalDeviceManager 单例实现
GlobalDeviceManager &GlobalDeviceManager::getInstance()
{
    static GlobalDeviceManager instance;
    return instance;
}

bool GlobalDeviceManager::AddDevice(const std::string &address)
{
    std::shared_ptr<BondedDeviceManager> existing;
    if (deviceManagers_.Find(address, existing)) {
        return false;
    }

    auto newDevice = std::make_shared<BondedDeviceManager>(address);
    deviceManagers_.Insert(address, newDevice);

    HILOGI("AddDevice device = %{public}s", GetEncryptAddr(address).c_str());
    return true;
}

std::shared_ptr<BondedDeviceManager> GlobalDeviceManager::GetDevice(const std::string &address)
{
    std::shared_ptr<BondedDeviceManager> device;
    deviceManagers_.Find(address, device);
    return device;
}

std::shared_ptr<BondedDeviceManager> GlobalDeviceManager::GetSaisDevice()
{
    std::shared_ptr<BondedDeviceManager> result;
    deviceManagers_.Iterate([&result](const std::string&, std::shared_ptr<BondedDeviceManager>& device) {
        if (device && device->IsSaisDeviceLocked() && !result) {
            result = device;
        }
    });
    return result;
}

std::vector<std::string> GlobalDeviceManager::GetAllAddresses()
{
    std::vector<std::string> addresses;
    deviceManagers_.Iterate([&addresses](const std::string& addr, std::shared_ptr<BondedDeviceManager>&) {
        addresses.push_back(addr);
    });
    return addresses;
}

bool GlobalDeviceManager::DeviceExists(const std::string &address)
{
    std::shared_ptr<BondedDeviceManager> device;
    return deviceManagers_.Find(address, device);
}

bool BondedDeviceManager::IsAllowAgCallLocked()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return !(isOnHfCall_ || isDialingByWatch_);
}

bool BondedDeviceManager::IsAllowConnectScoLocked()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return isOnHfCall_ || isDialingByWatch_;
}

void BondedDeviceManager::AllHfCallFinishedLocked(bool isHfDisconnected)
{
    std::lock_guard<std::mutex> lock(mutex_);
    isRouteScoRunning_ = false;
    callActiveTime_ = 0;
    isDialingByWatch_ = false;
}

void BondedDeviceManager::ProcessCallStateActiveLocked(long currentTimeStamp)
{
    std::lock_guard<std::mutex> lock(mutex_);
    callActiveTime_ = currentTimeStamp;
    isRouteScoRunning_ = true;
}

void BondedDeviceManager::ProcessCallStateFinishedLocked()
{
    std::lock_guard<std::mutex> lock(mutex_);
    callActiveTime_ = 0;
    isAllowHfScoConnect_ = true;
    isRouteScoRunning_ = true;
}

void BondedDeviceManager::ControlScoTimeoutLocked()
{
    std::lock_guard<std::mutex> lock(mutex_);
    callActiveTime_ = 0;
    isAllowHfScoConnect_ = true;
}

void BondedDeviceManager::SetPhoneConnHfpLocked(bool isPhoneConn)
{
    std::lock_guard<std::mutex> lock(mutex_);
    isPhoneConnHfp_ = isPhoneConn;
}

bool BondedDeviceManager::IsPhoneConnHfpLocked() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return isPhoneConnHfp_;
}

void BondedDeviceManager::SetCallIDLocked(int callId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    isRouteScoRunning_ = false;
    callID_ = callId;
}

void BondedDeviceManager::SetAllowHfScoConnectLocked(bool isAllow)
{
    std::lock_guard<std::mutex> lock(mutex_);
    callActiveTime_ = 0;
    isAllowHfScoConnect_ = isAllow;
}

void BondedDeviceManager::SetDialingByWatchLocked()
{
    std::lock_guard<std::mutex> lock(mutex_);
    isDialingByWatch_ = true;
}

bool BondedDeviceManager::IsDialingByWatchLocked()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return isDialingByWatch_;
}

void BondedDeviceManager::SetIsHfOnCallLocked(bool isOnCall)
{
    std::lock_guard<std::mutex> lock(mutex_);
    isOnHfCall_ = isOnCall;
}

bool BondedDeviceManager::IsHfOnCallLocked()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return isOnHfCall_;
}

void BondedDeviceManager::HandleVoipStateLocked(int voipState)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (voipState == VOIP_CALL_INCOMMING) {
        isOnVoip_ = true;
    } else if (voipState == VOIP_CALL_ACCEPT) {
        isAcceptVoipByWatch = true;
    } else {
        isAcceptVoipByWatch = false;
        isOnVoip_ = false;
    }
}

bool BondedDeviceManager::ProcessCallStateCheckLocked(int callId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return !isRouteScoRunning_ && callId == callID_;
}

void BondedDeviceManager::SetPhoneConnectStateLocked(bool connected)
{
    std::lock_guard<std::mutex> lock(mutex_);
    isPhoneConnected_ = connected;
}

bool BondedDeviceManager::IsPhoneConnectedLocked() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return isPhoneConnected_;
}

void BondedDeviceManager::SetHfConnectedLocked(bool connected)
{
    std::lock_guard<std::mutex> lock(mutex_);
    isHfConnected_ = connected;
}

void BondedDeviceManager::SetOnHfCallFalseLocked()
{
    std::lock_guard<std::mutex> lock(mutex_);
    isOnHfCall_ = false;
}

void BondedDeviceManager::SetDisconnectHfpOnDemandHfCallLocked(bool hasCalls)
{
    std::lock_guard<std::mutex> lock(mutex_);
    isOnHfCall_ = hasCalls;
}

bool BondedDeviceManager::isOnVoip_Locked()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return isOnVoip_;
}

bool BondedDeviceManager::IsWatchHealthConnectedLocked()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return isWatchHealthConnected_;
}

bool BondedDeviceManager::IsHfConnectedLocked()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return isHfConnected_;
}

void BondedDeviceManager::SetPhoneCreateConnectLocked(bool isPhoneCreate)
{
    std::lock_guard<std::mutex> lock(mutex_);
    isPhoneCreateConnect_ = isPhoneCreate;
}

void BondedDeviceManager::SetHfConnectionStateLocked(int state)
{
    std::lock_guard<std::mutex> lock(mutex_);
    hfConnectionState_ = state;
}

int BondedDeviceManager::GetHfConnectionStateLocked()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return hfConnectionState_;
}

void BondedDeviceManager::SetIsConnectedLocked(bool connected)
{
    std::lock_guard<std::mutex> lock(mutex_);
    isConnected = connected;
}

bool BondedDeviceManager::IsConnectedLocked()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return isConnected;
}

void BondedDeviceManager::SetIsSaisDeviceLocked(bool isSais)
{
    std::lock_guard<std::mutex> lock(mutex_);
    isSaisDevice = isSais;
}

bool BondedDeviceManager::IsSaisDeviceLocked()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return isSaisDevice;
}

bool BondedDeviceManager::IsJustConnectAclLocked()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return isJustConnectAcl_;
}

void BondedDeviceManager::SetJustConnectAclLocked(bool isJustConnect)
{
    std::lock_guard<std::mutex> lock(mutex_);
    isJustConnectAcl_ = isJustConnect;
}

bool BondedDeviceManager::IsRemovePairLocked()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return isRemovePair_;
}

void BondedDeviceManager::SetRemovePairLocked(bool isRemovePair)
{
    std::lock_guard<std::mutex> lock(mutex_);
    isRemovePair_ = isRemovePair;
}

bool BondedDeviceManager::IsRouteScoRunningLocked()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return isRouteScoRunning_;
}

void BondedDeviceManager::StopAllTimers()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (btDisconnectTimer_) {
        btDisconnectTimer_->Stop();
        btDisconnectTimer_ = nullptr;
    }
    if (connectHfpOnDemandTimer_) {
        connectHfpOnDemandTimer_->Stop();
        connectHfpOnDemandTimer_ = nullptr;
    }
    if (connectHfpTimer_) {
        connectHfpTimer_->Stop();
        connectHfpTimer_ = nullptr;
    }
    if (controlScoConnectTimer_) {
        controlScoConnectTimer_->Stop();
        controlScoConnectTimer_ = nullptr;
    }
}

bool BondedDeviceManager::IsAcceptVoipByWatchLocked() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return isAcceptVoipByWatch;
}

void BondedDeviceManager::SetNeedDialAfterHFConnectedLocked(bool isNeed)
{
    std::lock_guard<std::mutex> lock(mutex_);
    isNeedDialAfterHFConnected_ = isNeed;
}

bool BondedDeviceManager::IsNeedDialAfterHFConnectedLocked() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return isNeedDialAfterHFConnected_;
}

void BondedDeviceManager::SetWatchHealthConnectedLocked(bool connected)
{
    std::lock_guard<std::mutex> lock(mutex_);
    isWatchHealthConnected_ = connected;
}

void BondedDeviceManager::SetHfConnectionStateDirectLocked(int state)
{
    std::lock_guard<std::mutex> lock(mutex_);
    hfConnectionState_ = state;
}

void BondedDeviceManager::StartBtDisconnectTimerLocked(std::unique_ptr<utility::Timer> timer)
{
    std::lock_guard<std::mutex> lock(mutex_);
    btDisconnectTimer_ = std::move(timer);
}

void BondedDeviceManager::StopBtDisconnectTimerLocked()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (btDisconnectTimer_) {
        btDisconnectTimer_->Stop();
        btDisconnectTimer_ = nullptr;
    }
}

bool BondedDeviceManager::IsBtDisconnectTimerNullLocked() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return btDisconnectTimer_ == nullptr;
}

void BondedDeviceManager::StartConnectHfpOnDemandTimerLocked(std::unique_ptr<utility::Timer> timer)
{
    std::lock_guard<std::mutex> lock(mutex_);
    connectHfpOnDemandTimer_ = std::move(timer);
}

void BondedDeviceManager::StopConnectHfpOnDemandTimerLocked()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (connectHfpOnDemandTimer_) {
        connectHfpOnDemandTimer_->Stop();
        connectHfpOnDemandTimer_ = nullptr;
    }
}

bool BondedDeviceManager::IsConnectHfpOnDemandTimerNullLocked() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return connectHfpOnDemandTimer_ == nullptr;
}

void BondedDeviceManager::StartConnectHfpTimerLocked(std::unique_ptr<utility::Timer> timer)
{
    std::lock_guard<std::mutex> lock(mutex_);
    connectHfpTimer_ = std::move(timer);
}

void BondedDeviceManager::StopConnectHfpTimerLocked()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (connectHfpTimer_) {
        connectHfpTimer_->Stop();
        connectHfpTimer_ = nullptr;
    }
}

bool BondedDeviceManager::IsConnectHfpTimerNullLocked() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return connectHfpTimer_ == nullptr;
}

void BondedDeviceManager::StartControlScoConnectTimerLocked(std::unique_ptr<utility::Timer> timer)
{
    std::lock_guard<std::mutex> lock(mutex_);
    controlScoConnectTimer_ = std::move(timer);
}

void BondedDeviceManager::StopControlScoConnectTimerLocked()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (controlScoConnectTimer_) {
        controlScoConnectTimer_->Stop();
        controlScoConnectTimer_ = nullptr;
    }
}

bool BondedDeviceManager::IsControlScoConnectTimerNullLocked() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return controlScoConnectTimer_ == nullptr;
}

bool GlobalDeviceManager::IsAllowConnectHfpAgSco()
{
    bool allow = true;
    deviceManagers_.Iterate([&allow](const std::string&, std::shared_ptr<BondedDeviceManager>& device) {
        if (!allow) {
            return;
        }
        if (device) {
            if (device->IsHfOnCallLocked() || device->IsDialingByWatchLocked()) {
                allow = false;
            }
        }
    });
    return allow;
}

bool GlobalDeviceManager::ResetDeviceStatus(const std::string& address)
{
    auto device = GetDevice(address);
    if (device) {
        device->resetAllStatus();
        return true;
    }
    return false;
}

size_t GlobalDeviceManager::GetDeviceCount()
{
    return deviceManagers_.Size();
}

bool GlobalDeviceManager::RemoveDevice(const std::string &address)
{
    std::shared_ptr<BondedDeviceManager> existing;
    if (!deviceManagers_.Find(address, existing)) {
        return false;
    }

    if (existing) {
        existing->StopAllTimers();
    }
    deviceManagers_.Erase(address);
    HILOGI("RemoveDevice device = %{public}s", GetEncryptAddr(address).c_str());
    return true;
}

std::shared_ptr<BondedDeviceManager> GetDeviceManager(const std::string& address)
{
    return GlobalDeviceManager::getInstance().GetDevice(address);
}

bool RemoveDeviceManager(const std::string& address)
{
    return GlobalDeviceManager::getInstance().RemoveDevice(address);
}

bool AddDeviceManager(const std::string& address)
{
    return GlobalDeviceManager::getInstance().AddDevice(address);
}
}
}