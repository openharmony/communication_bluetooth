/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "a2dp_hfp_recover"
#endif

#include "a2dp_hfp_recover.h"
#include "log.h"
#include "a2dp_service.h"
#include "raw_address.h"
#include "bluetooth_device.h"
#include "bt_chr_ue_manager.h"
#include "remote_device_properties.h"
#include "classic_defs.h"
#include "hfp_ag_service.h"

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;

static bool IsHeadphoneOrHeadsetDevice(std::shared_ptr<BluetoothDevice> dev)
{
    return (static_cast<uint32_t>(dev->GetDeviceClass()) & CLASS_OF_DEVICE_MASK) == CLASS_OF_DEVICE_AV_HEADPHONES ||
        (static_cast<uint32_t>(dev->GetDeviceClass()) & CLASS_OF_DEVICE_MASK) == CLASS_OF_DEVICE_AV_HEADSETS;
}

static bool IsA2dpRecoverConditionSatisfied(std::shared_ptr<A2dpDeviceInfo> deviceInfo,
    HfpAgService *hfpAgService, A2dpService* a2dpservice, RawAddress& rawAddr)
{
    return deviceInfo->GetPreConnectState() == static_cast<int>(BTConnectState::CONNECTED) &&
        deviceInfo->GetConnectState() == static_cast<int>(BTConnectState::DISCONNECTED) &&
        a2dpservice->GetConnectStrategy(rawAddr) != static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN) &&
        hfpAgService->GetDeviceState(rawAddr) == static_cast<int>(BTConnectState::CONNECTED);
};


static bool IsHfpRecoverConditionSatisfied(HfpAgService *hfpAgService, A2dpService* a2dpservice, RawAddress& rawAddr)
{
    return hfpAgService->GetDevicePreState(rawAddr) == static_cast<int>(BTConnectState::CONNECTED) &&
        hfpAgService->GetDeviceState(rawAddr) == static_cast<int>(BTConnectState::DISCONNECTED) &&
        hfpAgService->GetConnectStrategy(rawAddr) != static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN) &&
        a2dpservice->GetDeviceState(rawAddr) == static_cast<int>(BTConnectState::CONNECTED);
}

A2dpHfpRecover& A2dpHfpRecover::GetInstance()
{
    static A2dpHfpRecover instance;
    return instance;
}

void A2dpHfpRecover::SetTimerForRecoverA2dpService(const std::string& addr)
{
    RawAddress rawAddr(addr);
    A2dpService *a2dpservice = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG(a2dpservice, "Can't get the instance of a2dpservice");
    HfpAgService *hfpAgService = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(hfpAgService, "Can't get the instance of hfpAgService");
    std::shared_ptr<BluetoothDevice> dev = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(rawAddr);
    CHECK_AND_RETURN_LOG(dev != nullptr, "BluetoothDevice is nullptr");
    std::shared_ptr<A2dpDeviceInfo> deviceInfo = a2dpservice->GetDeviceFromList(rawAddr);
    CHECK_AND_RETURN_LOG(deviceInfo != nullptr, "A2dpDeviceInfo is nullptr");
    // check whether is hw headset or hw headphone
    if (!(dev->GetWearDetectionSupportValue() && IsHeadphoneOrHeadsetDevice(dev))) {
        return;
    }
    // 部分耳机与自愈方案冲突，通过ModelId屏蔽
    if (std::find(recoverBlackList_.begin(), recoverBlackList_.end(), dev->GetModelId()) != recoverBlackList_.end()) {
        return;
    }
    if (!IsA2dpRecoverConditionSatisfied(deviceInfo, hfpAgService, a2dpservice, rawAddr)) {
        return;
    }
    std::lock_guard<std::mutex> lock(recoverLock_);
    if (timerForRecoverA2dpService_ != nullptr && timerForRecoverA2dpService_->IsStarted()) {
        HILOGI("Another timer has started, stop it before set mute");
        timerForRecoverA2dpService_->Stop();
        timerForRecoverA2dpService_ = nullptr;
    }
    timerForRecoverA2dpService_ =
        std::make_shared<utility::Timer>([this, rawAddr]() {
            this->A2dpOrHfpTimerCallback(RecoverType::A2dpRecover, rawAddr.GetAddress());
        });
    timerForRecoverA2dpService_->Start(TIMER_FOR_RECOVER_A2DP_SERVICE_MS);
    if (a2dpservice->GetDeviceState(rawAddr) == static_cast<int>(BTConnectState::DISCONNECTED) &&
        timerForRecoverHfpService_ != nullptr && timerForRecoverHfpService_->IsStarted()) {
        timerForRecoverHfpService_->Stop();
        timerForRecoverHfpService_ = nullptr;
    }
}

void A2dpHfpRecover::SetTimerForRecoverHfpService(const std::string& addr)
{
    RawAddress rawAddr(addr);
    A2dpService *a2dpservice = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG(a2dpservice, "Can't get the instance of a2dpservice");
    HfpAgService *hfpAgService = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(hfpAgService, "Can't get the instance of hfpAgService");
    std::shared_ptr<BluetoothDevice> dev = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(rawAddr);
    CHECK_AND_RETURN_LOG(dev != nullptr, "BluetoothDevice is nullptr");
    // check whether is hw headset or hw headphone
    if (!(dev->GetWearDetectionSupportValue() && IsHeadphoneOrHeadsetDevice(dev))) {
        return;
    }
    if (!IsHfpRecoverConditionSatisfied(hfpAgService, a2dpservice, rawAddr)) {
        return;
    }
    std::lock_guard<std::mutex> lock(recoverLock_);
    if (timerForRecoverHfpService_ != nullptr && timerForRecoverHfpService_->IsStarted()) {
        HILOGI("Another timer has started, stop it before set mute");
        timerForRecoverHfpService_->Stop();
        timerForRecoverHfpService_ = nullptr;
    }
    timerForRecoverHfpService_ =
        std::make_shared<utility::Timer>([this, rawAddr]() {
            this->A2dpOrHfpTimerCallback(RecoverType::HfpRecover, rawAddr.GetAddress());
        });
    timerForRecoverHfpService_->Start(TIMER_FOR_RECOVER_HFP_SERVICE_MS);
    if (hfpAgService->GetDeviceState(rawAddr) == static_cast<int>(BTConnectState::DISCONNECTED) &&
        timerForRecoverA2dpService_ != nullptr && timerForRecoverA2dpService_->IsStarted()) {
        timerForRecoverA2dpService_->Stop();
        timerForRecoverA2dpService_ = nullptr;
    }
}

void A2dpHfpRecover::A2dpOrHfpTimerCallback(RecoverType type, const std::string& addr)
{
    A2dpService *a2dpservice = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG(a2dpservice, "Can't get the instance of a2dpservice");
    HfpAgService *hfpAgService = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(hfpAgService, "Can't get the instance of hfpAgService");
    RawAddress rawAddr(addr);
    if (type == RecoverType::A2dpRecover) {
        if (a2dpservice->GetConnectStrategy(rawAddr) !=
            static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN) &&
            a2dpservice->GetDeviceState(rawAddr) == static_cast<int>(BTConnectState::DISCONNECTED) &&
            hfpAgService->GetDeviceState(rawAddr) == static_cast<int>(BTConnectState::CONNECTED)) {
            HILOGI("reconnect a2dpservice");
            a2dpservice->Connect(rawAddr);
            BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_A2DP_CONN, rawAddr,
                UE_COMMON_SCENE_CASE9, "bluetooth_service");
        }
    } else if (type == RecoverType::HfpRecover) {
        if (hfpAgService->GetConnectStrategy(rawAddr) !=
                static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN) &&
                a2dpservice->GetDeviceState(rawAddr) == static_cast<int>(BTConnectState::CONNECTED) &&
                hfpAgService->GetDeviceState(rawAddr) == static_cast<int>(BTConnectState::DISCONNECTED)) {
                HILOGI("reconnect hfpservice");
                hfpAgService->Connect(rawAddr);
                BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_HFP_CONN, rawAddr,
                    UE_COMMON_SCENE_CASE10, "bluetooth_service");
        }
    }
}
}
}