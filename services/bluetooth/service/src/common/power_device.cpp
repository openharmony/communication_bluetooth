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
#define LOG_TAG "bt_service_power_device"
#endif

#include "power_device.h"
#include <cstring>
#include <memory>
#include <mutex>
#include "log.h"
#include "btcommon/message.h"
#include "power_state_machine.h"
#include "securec.h"
#include "thread_util.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace bluetooth {
struct PowerDevice::impl {
public:
    impl(const RawAddress rawAddr):rawAddr_(rawAddr) {};
    ~impl(){};
    const RawAddress rawAddr_;
    std::map<std::string, RequestStatus> requestPower_ {};
    std::unique_ptr<PowerStateMachine> psm_ = nullptr;
    std::pair<PowerSsrLevel, PowerModeLevel> requestPowerLevel_ =
        std::pair<PowerSsrLevel, PowerModeLevel>(PowerSsrLevel::NO_ACTION, PowerModeLevel::NO_ACTION);
    std::pair<PowerSsrLevel, PowerModeLevel> controlPowerLevel_ =
        std::pair<PowerSsrLevel, PowerModeLevel>(PowerSsrLevel::NO_ACTION, PowerModeLevel::NO_ACTION);
    int controlPowerMode_ {};
    int controlInterval_ {};
    std::unique_ptr<PowerTimer> sniffDelayTimer_ = nullptr;
    std::mutex mutex_ {};

    BT_DISALLOW_COPY_AND_ASSIGN(impl);
};

PowerDevice::PowerDevice(const RawAddress rawAddr)
    : pimpl(std::make_unique<PowerDevice::impl>(rawAddr))
{
    pimpl->psm_ = std::make_unique<PowerStateMachine>();
    pimpl->psm_->Init(*this);

    pimpl->sniffDelayTimer_ = std::make_unique<PowerTimer>(
        [&]()->void {
            HITRACE_METER_NAME(BT_TRACE_TAG, "PowerDevice::PowerDevice");
            std::weak_ptr<PowerDevice> weakDevice = shared_from_this();
            DoInAdapterManagerThread([this, weakDevice] {this->DelayTimeoutCallback(weakDevice);});
        });
}

PowerDevice::~PowerDevice()
{
    pimpl->sniffDelayTimer_->Stop();
}

/// update request power function
void PowerDevice::SetRequestPower(const std::string &profileName, const RequestStatus status) const
{
    pimpl->requestPower_[profileName] = status;
}

void PowerDevice::DeleteRequestPower(const std::string &profileName) const
{
    pimpl->requestPower_.erase(profileName);
}

void PowerDevice::SetPowerMode()
{
    PowerInfo maxPower = CalcMaxPower();
    if (maxPower.powerMode_ == PowerModeLevel::NO_ACTION) {
        return;
    }
    if (maxPower.powerMode_ == PowerModeLevel::LEVEL_ACTIVE) {
        SetActiveMode();
    } else {
        SetSniffMode(maxPower);
    }
}

/// set power mode interface, use by SetPowerMode() / DelayTimeoutCallback()
void PowerDevice::SetActiveMode()
{
    StopDelayTimer();  // Stop Sniff Delay Timer.
    utility::Message msg(PowerStateMachine::MSG_PM_SET_ACTIVE);
    pimpl->psm_->ProcessMessage(msg);
}

void PowerDevice::SetSniffMode(PowerInfo requestPower)
{
    // Because the hardware does not support resetting the sniff parameter in the sniff state,
    // it exits the sniff first, and then delays to enter the sniff.
    PowerModeLevel controlPowerMode = GetControlPowerLevel().second;
    if (requestPower.powerMode_ != controlPowerMode &&
        (controlPowerMode == PowerModeLevel::LEVEL_LOW ||
        controlPowerMode == PowerModeLevel::LEVEL_MID ||
        controlPowerMode == PowerModeLevel::LEVEL_HIG)) {
        HILOGI("contorl is in sniff state, to reset the sniff parameter, you need to exit sniff state first");
        SetActiveMode();
    }

    if (GetDelayTimerRemainMs() <= static_cast<uint64_t>(requestPower.timeout_)) {
        HILOGI("Reset Timer: timeoutMs: %{public}d", requestPower.timeout_);
        StopDelayTimer();
        StartDelayTimer(static_cast<uint64_t>(requestPower.timeout_));
    }
}

BTPowerMode PowerDevice::GetPowerMode() const
{
    PowerModeLevel powerLevel = GetControlPowerLevel().second;
    BTPowerMode powerMode = BTPowerMode::MODE_ACTIVE;
    if (powerLevel == PowerModeLevel::LEVEL_ACTIVE) {
        powerMode = BTPowerMode::MODE_ACTIVE;
    } else if (powerLevel == PowerModeLevel::LEVEL_HIG) {
        powerMode = BTPowerMode::MODE_SNIFF_LEVEL_HIG;
    } else if (powerLevel == PowerModeLevel::LEVEL_MID) {
        powerMode = BTPowerMode::MODE_SNIFF_LEVEL_MID;
    } else if (powerLevel == PowerModeLevel::LEVEL_LOW) {
        powerMode = BTPowerMode::MODE_SNIFF_LEVEL_LOW;
    }
    return powerMode;
}

void PowerDevice::ModeChangeCallBack(uint8_t status, uint8_t currentMode, uint16_t interval)
{
}

void PowerDevice::SniffSubratingCompleteCallback(uint8_t status) const
{
    HILOGI("status: %{public}d", status);
    if (status == 0) {
        UpdateControlSniffSubrating(GetRequestPowerLevel().first);
        utility::Message msg(PowerStateMachine::MSG_PM_SET_SUBRATING_COMPLETE);
        pimpl->psm_->ProcessMessage(msg);
    }
}

/// calc power level and ssr level
PowerInfo PowerDevice::CalcMaxPower() const
{
    PowerInfo maxPower = PowerInfo(PowerModeLevel::NO_ACTION, 0);
    for (auto &its : pimpl->requestPower_) {
        PowerInfo itSpec = PowerSpec::GetPowerSpec(its.first, its.second);
        if (itSpec.powerMode_ > maxPower.powerMode_) {
            maxPower = itSpec;
        }
    }
    return maxPower;
}

PowerSsrLevel PowerDevice::CalcLowestSsrLevel() const
{
    PowerSsrLevel lowestLevel = PowerSsrLevel::NO_ACTION;
    for (auto &its : pimpl->requestPower_) {
        PowerSsrLevel level = PowerSpec::GetPowerSsrLevel(its.first, its.second);
        if ((level != PowerSsrLevel::NO_ACTION) && (level < lowestLevel)) {
            lowestLevel = level;
        }
    }
    return lowestLevel;
}

/// btm power interface
int PowerDevice::BtmSetSniffSubrating(const PowerSsrParam &ssrParam) const
{
    return 0;
}

int PowerDevice::BtmExitSniffMode() const
{
    return 0;
}

int PowerDevice::BtmEnterSniffMode(const PowerParam &param) const
{
    return 0;
}

const std::pair<PowerSsrLevel, PowerModeLevel> &PowerDevice::GetRequestPowerLevel() const
{
    return pimpl->requestPowerLevel_;
}

void PowerDevice::SetRequestPowerLevel(const PowerSsrLevel ssr, const PowerModeLevel power) const
{
    pimpl->requestPowerLevel_ = std::pair<PowerSsrLevel, PowerModeLevel>(ssr, power);
}

const std::pair<PowerSsrLevel, PowerModeLevel> &PowerDevice::GetControlPowerLevel() const
{
    std::unique_lock<std::mutex> lock(pimpl->mutex_);
    return pimpl->controlPowerLevel_;
}

void PowerDevice::UpdateControlSniffSubrating(const PowerSsrLevel ssr) const
{
    std::unique_lock<std::mutex> lock(pimpl->mutex_);
    pimpl->controlPowerLevel_.first = ssr;
}

void PowerDevice::UpdatecontrolPowerLevel(const PowerModeLevel powerLevel) const
{
    std::unique_lock<std::mutex> lock(pimpl->mutex_);
    pimpl->controlPowerLevel_.second = powerLevel;
}

void PowerDevice::DelayTimeoutCallback(const std::weak_ptr<PowerDevice>& weakDevice)
{
    std::shared_ptr<PowerDevice> sharedDevice = weakDevice.lock();
    if (sharedDevice != nullptr) {
        PowerInfo maxPowerInfo = sharedDevice->CalcMaxPower();
        HILOGI("maxPowerInfo: %{public}d", static_cast<int>(maxPowerInfo.powerMode_));
        if (maxPowerInfo.powerMode_ == PowerModeLevel::LEVEL_ACTIVE) {
            sharedDevice->SetActiveMode();
        } else if (maxPowerInfo.powerMode_ == PowerModeLevel::NO_ACTION) {
            return;
        } else {
            PowerSsrLevel lowestSsrLevel = sharedDevice->CalcLowestSsrLevel();
            sharedDevice->SetRequestPowerLevel(lowestSsrLevel, maxPowerInfo.powerMode_);
            utility::Message msg(PowerStateMachine::MSG_PM_SET_SNIFF);
            sharedDevice->pimpl->psm_->ProcessMessage(msg);
        }
    } else {
        HILOGI("sharedDevice == nullptr");
    }
}

/// delay timer function
void PowerDevice::StartDelayTimer(int ms) const
{
    pimpl->sniffDelayTimer_->Start(ms);
}

void PowerDevice::StopDelayTimer() const
{
    pimpl->sniffDelayTimer_->Stop();
}

uint64_t PowerDevice::GetDelayTimerRemainMs() const
{
    return pimpl->sniffDelayTimer_->GetRemainMs();
}

bool PowerDevice::PowerTimer::Start(int ms, bool isPeriodic)
{
    bool ret = utility::Timer::Start(ms, isPeriodic);
    struct timespec ts = {};
    clock_gettime(CLOCK_BOOTTIME, &ts);
    deadLineMs_ = ts.tv_sec * MS_PER_SECOND + ts.tv_nsec / NS_PER_MS + ms;
    return ret;
}

void PowerDevice::PowerTimer::Stop()
{
    deadLineMs_ = 0;
    utility::Timer::Stop();
}

uint64_t PowerDevice::PowerTimer::GetRemainMs()
{
    struct timespec ts = {};
    clock_gettime(CLOCK_BOOTTIME, &ts);
    int64_t presentMs = ts.tv_sec * MS_PER_SECOND + ts.tv_nsec / NS_PER_MS;
    if (deadLineMs_ > presentMs) {
        return deadLineMs_ - presentMs;
    } else {
        deadLineMs_ = 0;
        return 0;
    }
}
}  // namespace bluetooth
}  // namespace OHOS