/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_power_manager"
#endif

#include "power_manager.h"
#include <cstring>
#include <list>
#include <map>
#include <mutex>
#include <string>
#include "adapter_manager.h"
#include "log.h"
#include "common_util.h"
#include "power_device.h"
#include "btcommon/timer_manager.h"
#include "thread_util.h"

namespace OHOS {
namespace bluetooth {
/// PowerManager class
struct PowerManager::impl {
public:
    impl() = default;
    ~impl(){};

    std::mutex mutex_ {};
    std::atomic_bool isEnabled_ = false;
    std::map<RawAddress, std::shared_ptr<PowerDevice>> powerDevices_ {};
    std::map<uint16_t, RawAddress> connectionHandles_ {};

    void PowerProcess(const RequestStatus status, const std::string &profileName, const RawAddress rawAddr);
    void UpdatePowerDevicesInfo(const RawAddress rawAddr, const std::string &profileName, const RequestStatus status);

    void ModeChangeCallBackProcess(uint8_t status, const RawAddress rawAddr, uint8_t currentMode, uint16_t interval);

    void SsrCompleteCallBackProcess(uint8_t status, const RawAddress rawAddr);

    void ConnectionCompleteCallBackProcess(const RawAddress rawAddr, uint16_t connectionHandle);

    void DisconnectionCompleteCallBackProcess(uint8_t status, uint16_t connectionHandle, uint8_t reason);
    static void DisconnectionCompleteCallBack(uint8_t status, uint16_t connectionHandle, uint8_t reason, void *context);

    BT_DISALLOW_COPY_AND_ASSIGN(impl);
};

IPowerManager *IPowerManager::GetInstance()
{
    return PowerManager::GetInstance();
}

void IPowerManager::Uninitialize()
{
    PowerManager::GetInstance()->Uninitialize();
}

PowerManager *PowerManager::GetInstance()
{
    static PowerManager instance;
    return &instance;
}

void PowerManager::Initialize()
{}

void PowerManager::Uninitialize()
{
    return;
}

PowerManager::PowerManager() : pimpl(std::make_unique<PowerManager::impl>())
{}
PowerManager::~PowerManager()
{}

void PowerManager::Enable()
{
}

void PowerManager::Disable()
{
}

void PowerManager::StatusUpdate(
    const RequestStatus status, const std::string &profileName, const RawAddress &addr) const
{
    HILOGI("profileName: %{public}s, status: %{public}u,  addr: %{public}s", profileName.c_str(), status,
        GetEncryptAddr(addr.GetAddress()).c_str());

    if (pimpl->isEnabled_) {
        DoInAdapterManagerThread([this, status, profileName,
            addr] {this->pimpl.get()->PowerProcess(status, profileName, addr);});
    }
}

BTPowerMode PowerManager::GetPowerMode(const RawAddress &addr) const
{
    std::unique_lock<std::mutex> lock(pimpl->mutex_);
    auto iter = pimpl->powerDevices_.find(addr);
    if (iter != pimpl->powerDevices_.end()) {
        return iter->second->GetPowerMode();
    }
    return BTPowerMode::MODE_INVALID;
}

void PowerManager::impl::PowerProcess(
    const RequestStatus status, const std::string &profileName, const RawAddress rawAddr)
{
    HILOGI("status: %{public}u, profileName: %{public}s", status, profileName.c_str());
    std::unique_lock<std::mutex> lock(mutex_);
    UpdatePowerDevicesInfo(rawAddr, profileName, status);
    auto iter = powerDevices_.find(rawAddr);
    if (iter != powerDevices_.end()) {
        iter->second->SetPowerMode();
    }
}

void PowerManager::impl::UpdatePowerDevicesInfo(
    const RawAddress rawAddr, const std::string &profileName, const RequestStatus status)
{
    auto iter = powerDevices_.find(rawAddr);
    if (iter == powerDevices_.end()) {
        HILOGI("UpdatePowerDevicesInfo(), create powerDevices");
        powerDevices_[rawAddr] = std::make_shared<PowerDevice>(rawAddr);
    }

    if (status == RequestStatus::CONNECT_OFF) {
        auto its = powerDevices_.find(rawAddr);
        if (its != powerDevices_.end()) {
            its->second->DeleteRequestPower(profileName);
        }
    } else {
        HILOGI("UpdatePowerDevicesInfo(), execute SetRequesetPower()");
        powerDevices_[rawAddr]->SetRequestPower(profileName, status);
    }
}

void PowerManager::impl::ModeChangeCallBackProcess(
    uint8_t status, const RawAddress rawAddr, uint8_t currentMode, uint16_t interval)
{
    HILOGI("status: %{public}u, currentMode: %{public}u, interval: %{public}u", status, currentMode, interval);
    std::unique_lock<std::mutex> lock(mutex_);
    auto iter = powerDevices_.find(rawAddr);
    if (iter == powerDevices_.end()) {
        if (status != 0) {
            HILOGI("PM_: ModeChangeCallBackProcess(), no need to create powerDevices for error status");
            return;
        }
        HILOGI("PM_: ModeChangeCallBackProcess(), create powerDevices");
        powerDevices_[rawAddr] = std::make_shared<PowerDevice>(rawAddr);
    }
}

void PowerManager::impl::SsrCompleteCallBackProcess(uint8_t status, const RawAddress rawAddr)
{
    HILOGI("status: %{public}d", status);
    std::unique_lock<std::mutex> lock(mutex_);
    auto iter = powerDevices_.find(rawAddr);
    if (iter != powerDevices_.end()) {
        iter->second->SniffSubratingCompleteCallback(status);
    }
}

void PowerManager::impl::ConnectionCompleteCallBackProcess(const RawAddress rawAddr, uint16_t connectionHandle)
{
    HILOGI("addr: %{public}s, Handle: %{public}d", GetEncryptAddr(rawAddr.GetAddress()).c_str(), connectionHandle);
    // construct
    std::unique_lock<std::mutex> lock(mutex_);
    auto iter = powerDevices_.find(rawAddr);
    if (iter == powerDevices_.end()) {
        HILOGI("PM_: ConnectionCompleteCallBackProcess(), create powerDevices");
        powerDevices_[rawAddr] = std::make_shared<PowerDevice>(rawAddr);
    }
    connectionHandles_[connectionHandle] = rawAddr;
}

void PowerManager::impl::DisconnectionCompleteCallBackProcess(uint8_t status, uint16_t connectionHandle, uint8_t reason)
{
    HILOGI("status=%{public}u, reason=%{public}u", status, reason);
    // destruct
    if (status == 0) {
        std::unique_lock<std::mutex> lock(mutex_);
        auto iter = connectionHandles_.find(connectionHandle);
        if (iter != connectionHandles_.end()) {
            auto its = powerDevices_.find(iter->second);
            if (its != powerDevices_.end()) {
                powerDevices_.erase(its);
            }
            HILOGI("delete powerDevices, addr=%{public}s", GetEncryptAddr(iter->second.GetAddress()).c_str());
            connectionHandles_.erase(iter);
        }
    }
}

void PowerManager::impl::DisconnectionCompleteCallBack(
    uint8_t status, uint16_t connectionHandle, uint8_t reason, void *context)
{
    DoInAdapterManagerThread(std::bind(&PowerManager::impl::DisconnectionCompleteCallBackProcess,
        static_cast<PowerManager*>(context)->pimpl.get(),
        status,
        connectionHandle,
        reason));
}
}  // namespace bluetooth
}  // namespace OHOS