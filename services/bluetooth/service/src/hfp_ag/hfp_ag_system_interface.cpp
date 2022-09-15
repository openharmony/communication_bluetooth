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

#include "hfp_ag_system_interface.h"
#include "hfp_ag_service.h"
#include "log.h"
#include "stub/telephone_service.h"
#include "telephony_observer_client.h"
#include "bluetooth_call_client.h"
#include "call_manager_client.h"
#include "core_service_client.h"

using namespace OHOS;
using namespace OHOS::Telephony;
namespace OHOS {
namespace bluetooth {
HfpAgSystemInterface::HfpAgSystemInterface()
{}

HfpAgSystemInterface &HfpAgSystemInterface::GetInstance()
{
    static HfpAgSystemInterface hfpAgSystemInterface;
    return hfpAgSystemInterface;
}

void HfpAgSystemInterface::Start()
{
    LOG_INFO("[HFP AG]%{public}s():enter",  __FUNCTION__);
    QueryAgIndicator();
    RegisterObserver();
    DelayedSingleton<BluetoothCallClient>::GetInstance()->Init();
    return;
}

void HfpAgSystemInterface::Stop()
{
    LOG_INFO("[HFP AG]%{public}s():enter",  __FUNCTION__);
    UnregisterObserver();
    DelayedSingleton<BluetoothCallClient>::GetInstance()->UnInit();
    slotId_ = 0;
    serviceState_ = 0;
    signalStrength_ = 0;
    roamState_ = 0;
    batteryLevel_ = 0;
    activeNum_ = 0;
    heldNum_ = 0;
    subscriberNumber_ = "";
    operatorName_ = "";
    callState_ = HFP_AG_CALL_STATE_DISCONNECTED;
    return;
}

void HfpAgSystemInterface::RegisterObserver()
{
    LOG_INFO("[HFP AG]%{public}s():enter",  __FUNCTION__);
    if (observer_ == nullptr) {
        observer_ = new (std::nothrow) AgTelephonyObserver(*this);
    }
    slotId_ = CoreServiceClient::GetInstance().GetPrimarySlotId();
    if (slotId_ < 0) {
        LOG_INFO("[HFP AG]%{public}s(): slotId_ is invalid",  __FUNCTION__);
        return;
    }
    TelephonyObserverClient::GetInstance().AddStateObserver(observer_, slotId_,
        TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE | TelephonyObserverBroker::OBSERVER_MASK_SIGNAL_STRENGTHS,
        true);
}

void HfpAgSystemInterface::UnregisterObserver()
{
    LOG_INFO("[HFP AG]%{public}s():enter",  __FUNCTION__);
    if (observer_ == nullptr) {
        return;
    }
    slotId_ = CoreServiceClient::GetInstance().GetPrimarySlotId();
    if (slotId_ < 0) {
        LOG_INFO("[HFP AG]%{public}s(): slotId_ is invalid",  __FUNCTION__);
        return;
    }
    TelephonyObserverClient::GetInstance().RemoveStateObserver(slotId_,
        TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE | TelephonyObserverBroker::OBSERVER_MASK_SIGNAL_STRENGTHS);
}

void HfpAgSystemInterface::RejectCall(const std::string &address) const
{
    LOG_INFO("[HFP AG]%{public}s():enter",  __FUNCTION__);
    DelayedSingleton<BluetoothCallClient>::GetInstance()->RejectCall();
}

void HfpAgSystemInterface::DialOutCall(const std::string &address, const std::string &number) const
{
    LOG_INFO("[HFP AG]%{public}s():enter",  __FUNCTION__);
    AppExecFwk::PacMap extras;
    std::u16string u16number = Str8ToStr16(number);
    DelayedSingleton<CallManagerClient>::GetInstance()->DialCall(u16number, extras);
}

void HfpAgSystemInterface::HangupCall(const std::string &address) const
{
    LOG_INFO("[HFP AG]%{public}s():enter",  __FUNCTION__);
    DelayedSingleton<BluetoothCallClient>::GetInstance()->HangUpCall();
}

void HfpAgSystemInterface::AnswerCall(const std::string &address) const
{
    LOG_INFO("[HFP AG]%{public}s():enter",  __FUNCTION__);
    DelayedSingleton<BluetoothCallClient>::GetInstance()->AnswerCall();
}

bool HfpAgSystemInterface::SendDtmf(int dtmf, const std::string &address) const
{
    LOG_INFO("[HFP AG]%{public}s():enter",  __FUNCTION__);
    char str = dtmf + '0';
    if (!DelayedSingleton<BluetoothCallClient>::GetInstance()->StartDtmf(str)) {
        return false;
    }
    DelayedSingleton<BluetoothCallClient>::GetInstance()->StopDtmf();
    return true;
}

bool HfpAgSystemInterface::HoldCall(int chld) const
{
    LOG_INFO("[HFP AG]%{public}s():enter",  __FUNCTION__);
    if (!DelayedSingleton<BluetoothCallClient>::GetInstance()->HoldCall()) {
        return false;
    }
    return true;
}

std::string HfpAgSystemInterface::GetNetworkOperator()
{
    slotId_ = CoreServiceClient::GetInstance().GetPrimarySlotId();
    if (slotId_ < 0) {
        LOG_INFO("[HFP AG]%{public}s(): slotId_ is invalid",  __FUNCTION__);
    } else {
        sptr<NetworkState> netWorkState = CoreServiceClient::GetInstance().GetNetworkState(slotId_);
        if (netWorkState != nullptr) {
            operatorName_ = CoreServiceClient::GetInstance().GetNetworkState(slotId_)->GetLongOperatorName();
            LOG_INFO("[HFP AG]%{public}s(): operatorName_ is %{public}s",  __FUNCTION__, operatorName_.c_str());
        } else {
            LOG_INFO("[HFP AG]%{public}s(): netWorkState is nullptr",  __FUNCTION__);
        }
    }
    return operatorName_;
}

std::string HfpAgSystemInterface::GetSubscriberNumber()
{
    slotId_ = CoreServiceClient::GetInstance().GetPrimarySlotId();
    if (slotId_ < 0) {
        LOG_INFO("[HFP AG]%{public}s(): slotId_ is invalid",  __FUNCTION__);
    } else {
        subscriberNumber_ = Str16ToStr8(CoreServiceClient::GetInstance().GetSimTelephoneNumber(slotId_));
        LOG_INFO("[HFP AG]%{public}s(): subscriberNumber_ is %{public}s",  __FUNCTION__, subscriberNumber_.c_str());
    }
    return subscriberNumber_;
}

bool HfpAgSystemInterface::QueryCurrentCallsList()
{
    LOG_INFO("[HFP AG]%{public}s():enter",  __FUNCTION__);
    std::vector<CallAttributeInfo> callList;
    HfpAgService *service = HfpAgService::GetService();
    if (service == nullptr) {
        LOG_ERROR("[HFP AG]%{public}s():no service",  __FUNCTION__);
        return false;
    }
    slotId_ = CoreServiceClient::GetInstance().GetPrimarySlotId();
    if (slotId_ < 0) {
        LOG_ERROR("[HFP AG]%{public}s():slotId_ is invalid",  __FUNCTION__);
        return false;
    }
    callList = DelayedSingleton<BluetoothCallClient>::GetInstance()->GetCurrentCallList(slotId_);
    for (auto call : callList) {
        std::string number = call.accountNumber;
        bool conferenceState = false;
        if (call.conferenceState != TelConferenceState::TEL_CONFERENCE_IDLE) {
            conferenceState = true;
        }
        service->ClccResponse(call.callId, (int)call.callDirection, (int)call.callState, 0, conferenceState, number,
            (int)call.callType);
    }
    return true;
}

void HfpAgSystemInterface::QueryPhoneState() const
{
    LOG_INFO("[HFP AG]%{public}s():enter",  __FUNCTION__);
    DelayedSingleton<BluetoothCallClient>::GetInstance()->GetCallState();
    return;
}

bool HfpAgSystemInterface::StartVoiceRecognition(const std::string &address) const
{
    LOG_INFO("[HFP AG]%{public}s():enter",  __FUNCTION__);
    return stub::TelephoneService::GetInstance()->StartVoiceRecognition(address);
}

bool HfpAgSystemInterface::StopVoiceRecogition(const std::string &address) const
{
    LOG_INFO("[HFP AG]%{public}s():enter",  __FUNCTION__);
    return stub::TelephoneService::GetInstance()->StopVoiceRecogition(address);
}

bool HfpAgSystemInterface::SetAudioParameters(const std::string &parameters) const
{
    LOG_INFO("[HFP AG]%{public}s():enter",  __FUNCTION__);
    return stub::TelephoneService::GetInstance()->SetAudioParameters(parameters);
}

void HfpAgSystemInterface::SetStreamVolume(int streamType, int volume, int flag) const
{
    LOG_INFO("[HFP AG]%{public}s():enter",  __FUNCTION__);
    stub::TelephoneService::GetInstance()->SetStreamVolume(streamType, volume, flag);
}

std::string HfpAgSystemInterface::GetLastDialNumber()
{
    LOG_DEBUG("[HFP AG]%{public}s():enter",  __FUNCTION__);
    std::string number = stub::TelephoneService::GetInstance()->GetLastDialNumber();
    return number;
}

int HfpAgSystemInterface::GetServiceState()
{
    slotId_ = CoreServiceClient::GetInstance().GetPrimarySlotId();
    LOG_INFO("[HFP AG]%{public}s(): slotId_ is %{public}d",  __FUNCTION__, slotId_);
    if (slotId_ < 0) {
        LOG_INFO("[HFP AG]%{public}s(): slotId_ is invalid",  __FUNCTION__);
    } else {
        sptr<NetworkState> netWorkState = CoreServiceClient::GetInstance().GetNetworkState(slotId_);
        if (netWorkState != nullptr) {
            serviceState_ = static_cast<std::underlying_type<RegServiceState>::type>(netWorkState->GetRegStatus());
            LOG_DEBUG("[HFP AG]%{public}s(): serviceState_ is %{public}d",  __FUNCTION__, serviceState_);
        }
    }
    return serviceState_;
}

int HfpAgSystemInterface::GetSignalStrength()
{
    LOG_INFO("[HFP AG]%{public}s(): slotId_ is %{public}d",  __FUNCTION__, slotId_);
    if (slotId_ < 0) {
        LOG_INFO("[HFP AG]%{public}s(): slotId_ is invalid",  __FUNCTION__);
    } else {
        if (!CoreServiceClient::GetInstance().GetSignalInfoList(slotId_).empty()) {
            signalStrength_ = CoreServiceClient::GetInstance().GetSignalInfoList(slotId_)[0]->GetSignalLevel();
        } else {
            signalStrength_ = 0;
            LOG_INFO("[HFP AG]%{public}s(): GetSignalInfoList is empty",  __FUNCTION__);
        }
        LOG_INFO("[HFP AG]%{public}s(): signalStrength_ is %{public}d",  __FUNCTION__, signalStrength_);
    }
    return signalStrength_;
}

int HfpAgSystemInterface::GetRoamState()
{
    slotId_ = CoreServiceClient::GetInstance().GetPrimarySlotId();
    LOG_INFO("[HFP AG]%{public}s(): slotId_ is %{public}d",  __FUNCTION__, slotId_);
    if (slotId_ < 0) {
        LOG_INFO("[HFP AG]%{public}s(): slotId_ is invalid",  __FUNCTION__);
    } else {
        sptr<NetworkState> netWorkState = CoreServiceClient::GetInstance().GetNetworkState(slotId_);
        if (netWorkState != nullptr) {
            roamState_ = CoreServiceClient::GetInstance().GetNetworkState(slotId_)->IsRoaming();
            LOG_INFO("[HFP AG]%{public}s(): roamState_ is %{public}d",  __FUNCTION__, roamState_);
        }
    }
    return roamState_;
}

int HfpAgSystemInterface::GetBatteryLevel()
{
    return batteryLevel_;
}

void HfpAgSystemInterface::QueryAgIndicator()
{
    LOG_INFO("[HFP AG]%{public}s():enter",  __FUNCTION__);
    // No interface for querying battery level
    batteryLevel_ = 0;
    // query ag indicator for the other module.
    serviceState_ = 0;
    roamState_ = 0;
    operatorName_ = "";
    signalStrength_ = 0;
    subscriberNumber_ = "";
    slotId_ = CoreServiceClient::GetInstance().GetPrimarySlotId();
    LOG_INFO("[HFP AG]%{public}s(): slotId_ is %{public}d",  __FUNCTION__, slotId_);
    if (slotId_ < 0) {
        LOG_INFO("[HFP AG]%{public}s(): slotId_ is invalid",  __FUNCTION__);
    } else {
        sptr<NetworkState> netWorkState = CoreServiceClient::GetInstance().GetNetworkState(slotId_);
        if (netWorkState != nullptr) {
            serviceState_ = static_cast<std::underlying_type<RegServiceState>::type>(netWorkState->GetRegStatus());
            LOG_INFO("[HFP AG]%{public}s(): serviceState_ is %{public}d",  __FUNCTION__, serviceState_);
            roamState_ = CoreServiceClient::GetInstance().GetNetworkState(slotId_)->IsRoaming();
            LOG_INFO("[HFP AG]%{public}s(): roamState_ is %{public}d",  __FUNCTION__, roamState_);
            operatorName_ = CoreServiceClient::GetInstance().GetNetworkState(slotId_)->GetLongOperatorName();
            LOG_INFO("[HFP AG]%{public}s(): operatorName_ is %{public}s",  __FUNCTION__, operatorName_.c_str());
        } else {
            LOG_INFO("[HFP AG]%{public}s(): netWorkState is nullptr",  __FUNCTION__);
        }
        LOG_DEBUG("[HFP AG]%{public}s(): serviceState_ is %{public}d",  __FUNCTION__, serviceState_);
        if (!CoreServiceClient::GetInstance().GetSignalInfoList(slotId_).empty()) {
            signalStrength_ = CoreServiceClient::GetInstance().GetSignalInfoList(slotId_)[0]->GetSignalLevel();
        } else {
            signalStrength_ = 0;
            LOG_INFO("[HFP AG]%{public}s(): GetSignalInfoList is empty",  __FUNCTION__);
        }
        LOG_INFO("[HFP AG]%{public}s(): signalStrength_ is %{public}d",  __FUNCTION__, signalStrength_);

        subscriberNumber_ = Str16ToStr8(CoreServiceClient::GetInstance().GetSimTelephoneNumber(slotId_));
        LOG_INFO("[HFP AG]%{public}s(): subscriberNumber_ is %{public}s",  __FUNCTION__, subscriberNumber_.c_str());
    }
    SendServiceStateToService();
    SendRoamStateToService();
    SendSignalStrengthToService();
    SendBatteryLevelToService();

    return;
}

void HfpAgSystemInterface::SendHfIndicator(const std::string &address, int indId, int indValue) const
{
    LOG_INFO("[HFP AG]%{public}s():enter",  __FUNCTION__);
    RawAddress device(address);
    HfpAgService *service = HfpAgService::GetService();
    if (service == nullptr) {
        LOG_ERROR("[HFP AG]%{public}s():service is nullptr", __FUNCTION__);
    }
    if (indId == HFP_AG_HF_INDICATOR_ENHANCED_DRIVER_SAFETY_ID) {
        service->NotifyHfEnhancedDriverSafety(device, indValue);
        stub::TelephoneService::GetInstance()->NotifyHfEnhancedDriverSafety(indValue);
    } else if (indId == HFP_AG_HF_INDICATOR_BATTERY_LEVEL_ID) {
        service->NotifyHfBatteryLevel(device, indValue);
        stub::TelephoneService::GetInstance()->NotifyHfBatteryLevel(indValue);
    } else {
        LOG_INFO("[HFP AG]%{public}s():HF Indicator id:%{public}d", __FUNCTION__, indId);
    }
    return;
}

void HfpAgSystemInterface::SetActiveCallNumber(int number)
{
    activeNum_ = number;
}

int HfpAgSystemInterface::GetActiveCallNumber() const
{
    return activeNum_;
}

void HfpAgSystemInterface::SetHeldCallNumber(int number)
{
    heldNum_ = number;
}

int HfpAgSystemInterface::GetHeldCallNumber() const
{
    return heldNum_;
}

void HfpAgSystemInterface::SetCallState(int state)
{
    callState_ = state;
}

int HfpAgSystemInterface::GetCallState() const
{
    return callState_;
}

void HfpAgSystemInterface::SendServiceStateToService() const
{
    LOG_DEBUG("[HFP AG]%{public}s():enter",  __FUNCTION__);
    HfpAgService *service = HfpAgService::GetService();
    if (service != nullptr) {
        service->NotifyAgIndicatorStateChanged(HFP_AG_NOTIFY_SERVICE_STATE, serviceState_);
    } else {
        LOG_ERROR("[HFP AG]%{public}s():service is null!", __FUNCTION__);
    }
}

void HfpAgSystemInterface::SendRoamStateToService() const
{
    LOG_DEBUG("[HFP AG]%{public}s():enter",  __FUNCTION__);
    HfpAgService *service = HfpAgService::GetService();
    if (service != nullptr) {
        service->NotifyAgIndicatorStateChanged(HFP_AG_NOTIFY_ROAM_STATE, roamState_);
    } else {
        LOG_ERROR("[HFP AG]%{public}s():service is null!", __FUNCTION__);
    }
}

void HfpAgSystemInterface::SendSignalStrengthToService() const
{
    LOG_DEBUG("[HFP AG]%{public}s():enter",  __FUNCTION__);
    HfpAgService *service = HfpAgService::GetService();
    if (service != nullptr) {
        service->NotifyAgIndicatorStateChanged(HFP_AG_NOTIFY_SIGNAL_STRENGTH, signalStrength_);
    } else {
        LOG_ERROR("[HFP AG]%{public}s():service is null!", __FUNCTION__);
    }
}

void HfpAgSystemInterface::SendBatteryLevelToService() const
{
    LOG_DEBUG("[HFP AG]%{public}s():enter",  __FUNCTION__);
    HfpAgService *service = HfpAgService::GetService();
    if (service != nullptr) {
        service->NotifyAgIndicatorStateChanged(HFP_AG_NOTIFY_BATTERY_LEVEL, batteryLevel_);
    } else {
        LOG_ERROR("[HFP AG]%{public}s():service is null!", __FUNCTION__);
    }
}

void HfpAgSystemInterface::OnSubscriptionStateChanged(int state)
{
    LOG_DEBUG("[HFP AG]%{public}s():enter",  __FUNCTION__);
    int preService = serviceState_;
    serviceState_ = state;
    if (preService != serviceState_) {
        SendServiceStateToService();
    }
}

void HfpAgSystemInterface::OnSignalStrengthChanged(int signal)
{
    LOG_DEBUG("[HFP AG]%{public}s():enter",  __FUNCTION__);
    int preSignal = signalStrength_;
    signalStrength_ = signal;
    if (preSignal != signalStrength_) {
        SendSignalStrengthToService();
    }
}

void HfpAgSystemInterface::OnRoamStateChanged(int state)
{
    LOG_DEBUG("[HFP AG]%{public}s():enter",  __FUNCTION__);
    int preRoam = roamState_;
    roamState_ = state;
    if (preRoam != roamState_) {
        SendRoamStateToService();
    }
}

void HfpAgSystemInterface::OnBatteryLevel(int level)
{
    LOG_DEBUG("[HFP AG]%{public}s():enter",  __FUNCTION__);
    int preBattery = batteryLevel_;
    batteryLevel_ = level;
    if (preBattery != batteryLevel_) {
        SendBatteryLevelToService();
    }
}

bool HfpAgSystemInterface::IsInCall() const
{
    LOG_DEBUG("[HFP AG]%{public}s():enter",  __FUNCTION__);
    return ((activeNum_ > 0) || (heldNum_ > 0) ||
            ((callState_ != HFP_AG_CALL_STATE_IDLE) && (callState_ != HFP_AG_CALL_STATE_INCOMING) &&
                (callState_ != HFP_AG_CALL_STATE_DISCONNECTED) && (callState_ != HFP_AG_CALL_STATE_DISCONNECTING)));
}

bool HfpAgSystemInterface::IsRinging() const
{
    LOG_DEBUG("[HFP AG]%{public}s():enter",  __FUNCTION__);
    return (callState_ == HFP_AG_CALL_STATE_INCOMING);
}

void HfpAgSystemInterface::AgTelephonyObserver::OnNetworkStateUpdated(int32_t slotId,
    const sptr<NetworkState> &networkState)
{
    HILOGI("[HFP AG] slotId: %{public}d", slotId);
    if (networkState == nullptr) {
        HILOGE("[HFP AG] networkState is null");
        return;
    }
    interface_.slotId_ = slotId;
    interface_.operatorName_ = networkState->GetLongOperatorName();
    interface_.OnSubscriptionStateChanged(static_cast<std::underlying_type<RegServiceState>::type>(
        networkState->GetRegStatus()));
    interface_.OnRoamStateChanged(networkState->IsRoaming());
}

void HfpAgSystemInterface::AgTelephonyObserver::OnSignalInfoUpdated(int32_t slotId,
    const std::vector<sptr<SignalInformation>> &vec)
{
    LOG_DEBUG("[HFP AG]%{public}s():enter",  __FUNCTION__);
    interface_.slotId_ = slotId;
    if (!vec.empty()) {
        interface_.OnSignalStrengthChanged(vec[0]->GetSignalLevel());
    }
}

inline std::u16string HfpAgSystemInterface::Str8ToStr16(const std::string& str) const
{
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
    std::u16string result = convert.from_bytes(str);
    return result;
}

inline std::string HfpAgSystemInterface::Str16ToStr8(const std::u16string& str) const
{
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
    std::string result = convert.to_bytes(str);
    return result;
}
}  // namespace bluetooth
}  // namespace OHOS
