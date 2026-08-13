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
#define LOG_TAG "bt_service_hfp_ag"
#endif

#include "hfp_ag_system_interface.h"

#include <cmath>

#ifdef BLUETOOTH_HFP_AG_ENABLE
#include "battery_srv_client.h"
#include "call_manager_client.h"
#include "core_service_client.h"
#endif
#include "hfp_ag_service.h"
#include "iservice_registry.h"
#include "log.h"
#include "stub/telephone_service.h"
#include "system_ability_definition.h"
#include "system_ability_status_change_stub.h"
#include "thread_util.h"
#ifdef BLUETOOTH_HFP_AG_ENABLE
#include "telephony_observer_client.h"
#endif
#include "bluetooth_datashare_utils.h"
#include "bluetooth_device_battery_manager.h"
#include "bt_chr_dft_exception.h"
#include "uri.h"
#include "hitrace_meter.h"
#include "bluetooth_phone_state.h"

static constexpr const char *CALL_LOG_DATASHARE_URI_CONTENT = "datashare:///com.ohos.calllogability/calls/calllog";

namespace OHOS {
namespace bluetooth {
using namespace OHOS;
#ifdef BLUETOOTH_HFP_AG_ENABLE
using namespace OHOS::Telephony;
#endif

constexpr int32_t INVALID_SLOT_ID = -1;
constexpr int32_t NETWORK_STATE_NOT_AVAILABLE = 0;

constexpr int32_t INCOMING_CALL = 0;
constexpr int32_t OUTGOING_CALL = 1;
constexpr int32_t NETWORK_STATE_AVAILABLE = 1;

HfpAgSystemInterface::HfpAgSystemInterface()
{}

HfpAgSystemInterface &HfpAgSystemInterface::GetInstance()
{
    static HfpAgSystemInterface hfpAgSystemInterface;
    return hfpAgSystemInterface;
}

void HfpAgSystemInterface::Start()
{
    HITRACE_METER(BT_TRACE_TAG);
    QueryAgIndicator();
    return;
}

void HfpAgSystemInterface::Stop()
{
    std::lock_guard<BtRecursiveMutex> lk(agIndicatorMutex_);
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
    HITRACE_METER(BT_TRACE_TAG);
#ifdef BLUETOOTH_HFP_AG_ENABLE
    if (observer_ == nullptr) {
        observer_ = new (std::nothrow) AgTelephonyObserver(*this);
    }
    CHECK_AND_RETURN_LOG(observer_ != nullptr, "observer_ nullptr");
    CoreServiceClient::GetInstance().GetPrimarySlotId(slotId_);
    if (slotId_ < 0) {
        HILOGE("[HFP_SYSTEM_INTERFACE]SlotId_ is invalid");
        return;
    }
    TelephonyObserverClient::GetInstance().AddStateObserver(observer_, slotId_,
        TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE | TelephonyObserverBroker::OBSERVER_MASK_SIGNAL_STRENGTHS,
        true);
#endif
}

void HfpAgSystemInterface::UnregisterObserver()
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef BLUETOOTH_HFP_AG_ENABLE
    if (observer_ == nullptr) {
        return;
    }
    CoreServiceClient::GetInstance().GetPrimarySlotId(slotId_);
    if (slotId_ < 0) {
        HILOGE("[HFP_SYSTEM_INTERFACE]SlotId_ is invalid");
        return;
    }
    TelephonyObserverClient::GetInstance().RemoveStateObserver(slotId_,
        TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE | TelephonyObserverBroker::OBSERVER_MASK_SIGNAL_STRENGTHS);
#endif
}

void HfpAgSystemInterface::RejectCall(const std::string &address) const
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef BLUETOOTH_HFP_AG_ENABLE
    DelayedRefSingleton<BluetoothCallClient>::GetInstance().RejectCall();
#endif
}

int32_t HfpAgSystemInterface::DialOutCall(const std::string &address, const std::string &number) const
{
    HITRACE_METER(BT_TRACE_TAG);
    int32_t ret = -1;
#ifdef BLUETOOTH_HFP_AG_ENABLE
    AppExecFwk::PacMap extras;
    int slotId = GetSlotId(number);
    extras.PutIntValue("accountId", slotId);
    std::u16string u16number = Str8ToStr16(number);
    ret = DelayedRefSingleton<BluetoothCallClient>::GetInstance().DialCall(u16number, extras);
    HILOGI("The final slotid is %{public}d, ret is %{public}d", slotId, ret);
#endif
    return ret;
}

void HfpAgSystemInterface::HangupCall(const std::string &address) const
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef BLUETOOTH_HFP_AG_ENABLE
    DelayedRefSingleton<BluetoothCallClient>::GetInstance().HangUpCall();
#endif
}

void HfpAgSystemInterface::AnswerCall(const std::string &address) const
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef BLUETOOTH_HFP_AG_ENABLE
    DelayedRefSingleton<BluetoothCallClient>::GetInstance().AnswerCall();
#endif
}

bool HfpAgSystemInterface::SendDtmf(int dtmf, const std::string &address) const
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef BLUETOOTH_HFP_AG_ENABLE
    char str = static_cast<char>(dtmf);
    HILOGD("The dtmf is %{public}d, str is %{public}c", dtmf, str);
    if (!DelayedRefSingleton<BluetoothCallClient>::GetInstance().StartDtmf(str)) {
        return false;
    }
    DelayedRefSingleton<BluetoothCallClient>::GetInstance().StopDtmf();
#endif
    return true;
}

bool HfpAgSystemInterface::HoldCall(int chld, const std::string &address) const
{
    return HandleChld(chld, address);
}

bool HfpAgSystemInterface::HandleChld(int chld, const std::string &address) const
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef BLUETOOTH_HFP_AG_ENABLE
    // Use slotid -1 to query the current call list of all SIM cards
    std::vector<CallAttributeInfo> callList =
        DelayedRefSingleton<BluetoothCallClient>::GetInstance().GetCurrentCallList(INVALID_SLOT_ID);

    // just handle once when chld:ATCHLD_RELEASE_HOLD_ACCPET_OTHER
    if (chld == ATCHLD_RELEASE_HOLD_ACCPET_OTHER) {
        ProcessReleaseHoldAccpetOther(callList, address);
        return true;
    }
    for (auto call : callList) {
        HILOGI("[HFP_SYSTEM_INTERFACE]HandleChld for index = %{public}d state = %{public}d chld = %{public}d",
            call.index, static_cast<int>(call.callState), chld);

        switch (chld) {
            case ATCHLD_RELEASE_ALL_HELD_CALLS:
                ProcessReleaseAllHeldCalls(call);
                break;
            case ATCHLD_RELEASE_ACTIVE_ACCPET_OTHER:
                ProcessReleaseActiveAccpetOther(call);
                break;
            case ATCHLD_ADD_CALL_TO_CONVERSATION:
                ProcessAddCallToConversation(call);
                break;
            case ATCHLD_CONNECT_TWO_CALL:
                ProcessConnectTwoCall();
                break;
            default:
                break;
        }
    }
#endif
    return true;
}

void HfpAgSystemInterface::ProcessConnectTwoCall() const
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef BLUETOOTH_HFP_AG_ENABLE
    DelayedRefSingleton<BluetoothCallClient>::GetInstance().CombineConference();
    DelayedRefSingleton<BluetoothCallClient>::GetInstance().HangUpCall();
#endif
}

#ifdef BLUETOOTH_HFP_AG_ENABLE
void HfpAgSystemInterface::ProcessAddCallToConversation(const CallAttributeInfo &call) const
{
    HITRACE_METER(BT_TRACE_TAG);
    if (static_cast<int>(TelCallState::CALL_STATUS_WAITING) == static_cast<int>(call.callState)) {
        DelayedRefSingleton<BluetoothCallClient>::GetInstance().AnswerCall();
        DelayedRefSingleton<BluetoothCallClient>::GetInstance().CombineConference();
    }
}

void HfpAgSystemInterface::ProcessReleaseHoldAccpetOther(const std::vector<CallAttributeInfo> &callList,
    const std::string &address) const
{
    HITRACE_METER(BT_TRACE_TAG);
    CallAttributeInfo activeCall;
    CallAttributeInfo ringingCall;
    CallAttributeInfo heldCall;
    bool isSingleCall = callList.size() == 1;

    for (auto call : callList) {
        HILOGI("[HFP_SYSTEM_INTERFACE] index = %{public}d callState = %{public}d",
            call.index, static_cast<int>(call.callState));
        switch (static_cast<int>(call.callState)) {
            case static_cast<int>(TelCallState::CALL_STATUS_ACTIVE):
                activeCall = call;
                break;
            case static_cast<int>(TelCallState::CALL_STATUS_WAITING):
            case static_cast<int>(TelCallState::CALL_STATUS_INCOMING):
                ringingCall = call;
                break;
            case static_cast<int>(TelCallState::CALL_STATUS_HOLDING):
                heldCall = call;
                break;
            default:
                break;
        }
    }

    if (static_cast<int>(TelCallState::CALL_STATUS_WAITING) == static_cast<int>(ringingCall.callState) ||
        static_cast<int>(TelCallState::CALL_STATUS_INCOMING) == static_cast<int>(ringingCall.callState)) {
        DelayedRefSingleton<BluetoothCallClient>::GetInstance().AnswerCall();
        HfpAgService *service = HfpAgService::GetService();
        if (service != nullptr && !address.empty()) {
            HILOGI("hold accept answer call, resume device");
            service->SetResumeDevice(address);
        }
    } else if (static_cast<int>(TelCallState::CALL_STATUS_ACTIVE) == static_cast<int>(activeCall.callState)) {
        DelayedRefSingleton<BluetoothCallClient>::GetInstance().HoldCall();
    } else if (static_cast<int>(TelCallState::CALL_STATUS_HOLDING) == static_cast<int>(heldCall.callState) &&
        isSingleCall) {
        DelayedRefSingleton<BluetoothCallClient>::GetInstance().UnHoldCall();
    }
}

void HfpAgSystemInterface::ProcessReleaseActiveAccpetOther(const CallAttributeInfo &call) const
{
    HITRACE_METER(BT_TRACE_TAG);
    if (static_cast<int>(TelCallState::CALL_STATUS_ACTIVE) == static_cast<int>(call.callState)) {
        DelayedRefSingleton<BluetoothCallClient>::GetInstance().HangUpCall();
    }
    if (static_cast<int>(TelCallState::CALL_STATUS_WAITING) == static_cast<int>(call.callState) &&
        static_cast<int>(call.callDirection) == static_cast<int>(CallDirection::CALL_DIRECTION_IN)) {
        DelayedRefSingleton<BluetoothCallClient>::GetInstance().AnswerCall();
    } else if (static_cast<int>(TelCallState::CALL_STATUS_HOLDING) == static_cast<int>(call.callState)) {
        DelayedRefSingleton<BluetoothCallClient>::GetInstance().UnHoldCall();
    }
}

void HfpAgSystemInterface::ProcessReleaseAllHeldCalls(const CallAttributeInfo &call) const
{
    HITRACE_METER(BT_TRACE_TAG);
    if (static_cast<int>(TelCallState::CALL_STATUS_WAITING) == static_cast<int>(call.callState) ||
        static_cast<int>(TelCallState::CALL_STATUS_INCOMING) == static_cast<int>(call.callState)) {
        DelayedRefSingleton<BluetoothCallClient>::GetInstance().RejectCall();
    } else if (static_cast<int>(TelCallState::CALL_STATUS_HOLDING) == static_cast<int>(call.callState)) {
        DelayedRefSingleton<BluetoothCallClient>::GetInstance().HangUpCall();
    }
}

int HfpAgSystemInterface::GetSlotId(const std::string &number) const
{
    HITRACE_METER(BT_TRACE_TAG);
    int32_t slotId = INVALID_SLOT_ID;
    sptr<ISystemAbilityManager> saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_LOG_RET((saManager != nullptr), INVALID_SLOT_ID, "get saManager fail.");
    sptr<IRemoteObject> remoteObj = saManager->GetSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);
    CHECK_AND_RETURN_LOG_RET((remoteObj != nullptr), INVALID_SLOT_ID, "get remoteObj fail.");
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelperUtils =
        DataShare::DataShareHelper::Creator(remoteObj, CALL_LOG_DATASHARE_URI_CONTENT);
    CHECK_AND_RETURN_LOG_RET((dataShareHelperUtils != nullptr), INVALID_SLOT_ID, "get dataShareHelperUtils fail.");
    Uri uri(CALL_LOG_DATASHARE_URI_CONTENT);
    std::vector<std::string> columns = {"slot_id"};
    DataShare::DataSharePredicates callRecord;
    uint8_t callDirection = OUTGOING_CALL;
    do {
        callRecord.EqualTo("call_direction", callDirection)->And()
            ->EqualTo("phone_number", number)->And()->OrderByDesc("id");
        auto result = dataShareHelperUtils->Query(uri, callRecord, columns);
        if (result != nullptr && result->GoToFirstRow() == DataShare::E_OK) {
            int columnIndex;
            result->GetColumnIndex("slot_id", columnIndex);
            result->GetInt(columnIndex, slotId);
            result->Close();
            break;
        }
        callDirection--;
    } while (callDirection < INCOMING_CALL);
    dataShareHelperUtils->Release();
    return IsSimCardAvailable(slotId) ? slotId : INVALID_SLOT_ID;
}

bool HfpAgSystemInterface::IsSimCardAvailable(int32_t slotId) const
{
    if (slotId == INVALID_SLOT_ID) {
        return false;
    }
    SimState simState = SimState::SIM_STATE_UNKNOWN;
    CoreServiceClient::GetInstance().GetSimState(slotId, simState);
    return ((simState >= SimState::SIM_STATE_READY) && CoreServiceClient::GetInstance().IsSimActive(slotId));
}
#endif

std::string HfpAgSystemInterface::GetNetworkOperator()
{
    return operatorName_;
}

std::string HfpAgSystemInterface::GetSubscriberNumber()
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef BLUETOOTH_HFP_AG_ENABLE
    CoreServiceClient::GetInstance().GetPrimarySlotId(slotId_);
    if (slotId_ < 0) {
        HILOGE("[HFP_SYSTEM_INTERFACE]SlotId_ is invalid");
    } else {
        std::u16string telephoneNumber;
        CoreServiceClient::GetInstance().GetSimTelephoneNumber(slotId_, telephoneNumber);
        subscriberNumber_ = Str16ToStr8(telephoneNumber);
    }
#endif
    return subscriberNumber_;
}

bool HfpAgSystemInterface::QueryCurrentCallsList()
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef BLUETOOTH_HFP_AG_ENABLE
    HfpAgService *service = HfpAgService::GetService();
    if (service == nullptr) {
        HILOGE("[HFP_SYSTEM_INTERFACE]No service");
        return false;
    }
    int slotid = -1;
    std::vector<CallAttributeInfo> callList =
        DelayedRefSingleton<BluetoothCallClient>::GetInstance().GetCurrentCallList(slotid);
    for (auto call : callList) {
        std::string number = call.accountNumber;
        bool conferenceState = false;
        if (call.conferenceState != TelConferenceState::TEL_CONFERENCE_IDLE) {
            conferenceState = true;
        }
        int dire = -1;
        switch ((int)call.callDirection) {
            case (int)CallDirection::CALL_DIRECTION_IN:
                dire = BT_CALL_DIRECTION_IN;
                break;
            case (int)CallDirection::CALL_DIRECTION_OUT:
                dire = BT_CALL_DIRECTION_OUT;
                break;
            default:
                dire = BT_CALL_DIRECTION_UNKNOW;
                break;
        }
        int calltype = -1;
        if ((int)call.callType == (int)CallType::TYPE_CS) {
            calltype = CALL_TYPE_DEFAULT;
        } else {
            calltype = (int)call.callType;
        }

        /*
        * Note that when querying call information,
        * if the call status is disconnecting or disconnected, only need to reply OK
        */
        if (static_cast<int>(call.callState) == static_cast<int>(HFP_AG_CALL_STATE_DISCONNECTED) ||
            static_cast<int>(call.callState) == static_cast<int>(HFP_AG_CALL_STATE_DISCONNECTING)) {
            continue;
        }

        int callIndex = GetIndexForCall(call.callId);
        service->ClccResponse(callIndex, dire, static_cast<int>(call.callState), 0, conferenceState, number, calltype);
        HILOGI("[HFP_SYSTEM_INTERFACE]id: %{public}d state: %{public}d", callIndex, static_cast<int>(call.callState));
    }
    service->ClccResponse(0, 0, 0, 0, false, "", 0); // send ok
#endif
    return true;
}

void HfpAgSystemInterface::QueryPhoneState() const
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef BLUETOOTH_HFP_AG_ENABLE
    DelayedRefSingleton<BluetoothCallClient>::GetInstance().GetCallState();
#endif
    return;
}

void HfpAgSystemInterface::SetAudioParameters(const std::string &key, const std::string &value) const
{
    BluetoothAudioFrameworkAdapter::HfpSetAudioParameters(key, value);
}

void HfpAgSystemInterface::SetExtraAudioParameters(const std::string &key,
    const std::vector<std::pair<std::string, std::string>> &kvpairs) const
{
    BluetoothAudioFrameworkAdapter::HfpSetExtraAudioParameters(key, kvpairs);
}

void HfpAgSystemInterface::SetStreamVolume(int streamType, int volume, int flag) const
{
    BluetoothAudioFrameworkAdapter::SetStreamVolume(streamType, volume, flag);
}

std::string HfpAgSystemInterface::GetLastDialNumber()
{
    HITRACE_METER(BT_TRACE_TAG);
    std::string number = "";
    sptr<ISystemAbilityManager> saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        HILOGE("GetSystemAbilityManager Service Failed.");
        return number;
    }
    sptr<IRemoteObject> remoteObj = saManager->GetSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);
    if (remoteObj == nullptr) {
        HILOGE("GetSystemAbility Service Failed.");
        return number;
    }
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelperUtils =
        DataShare::DataShareHelper::Creator(remoteObj, CALL_LOG_DATASHARE_URI_CONTENT);
    if (dataShareHelperUtils == nullptr) {
        HILOGE("dataShareHelperUtils null");
        return number;
    }
    Uri uri(CALL_LOG_DATASHARE_URI_CONTENT);
    DataShare::DataSharePredicates predicates;
    std::vector<std::string> columns = {"id, phone_number, call_direction"};
    predicates.Contains("call_direction", std::to_string(OUTGOING_CALL))->
        EqualTo("features", CELLULAR_VOICE_CALL)->Or()->EqualTo("features", CELLULAR_VIDEO_CALL)->
        OrderByDesc("id");
    auto result = dataShareHelperUtils->Query(uri, predicates, columns);
    if (result == nullptr) {
        HILOGE("dataShareHelperUtils query error, result is null");
        return number;
    }
    if (result->GoToFirstRow() != DataShare::E_OK) {
        HILOGE("DatashareHelper query failed, go to first roe error");
        result->Close();
        return number;
    }
    int tempNumber;
    result->GetColumnIndex("phone_number", tempNumber);
    result->GetString(tempNumber, number);
    result->Close();
    dataShareHelperUtils->Release();
    return number;
}

int HfpAgSystemInterface::GetServiceState()
{
    return serviceState_;
}

int HfpAgSystemInterface::GetSignalStrength()
{
    return signalStrength_;
}

int HfpAgSystemInterface::GetRoamState()
{
    return roamState_;
}

int HfpAgSystemInterface::QuerySignalStrength() const
{
#ifdef BLUETOOTH_HFP_AG_ENABLE
    if (serviceState_ == NETWORK_STATE_NOT_AVAILABLE) {
        HILOGD("Service is not avaliable, signal strength is 0.");
        return 0;
    }

    std::vector<sptr<SignalInformation>> signals;
    if (CoreServiceClient::GetInstance().GetSignalInfoList(slotId_, signals) == 0 && !signals.empty()) {
        return signals[0]->GetSignalLevel();
    }

    HILOGE("[HFP_SYSTEM_INTERFACE]GetSignalInfoList is empty");
#endif
    return 0;
}

int HfpAgSystemInterface::GetBatteryLevel()
{
    HITRACE_METER(BT_TRACE_TAG);
    std::lock_guard<BtRecursiveMutex> lk(agIndicatorMutex_);
#ifdef BLUETOOTH_HFP_AG_ENABLE
    int32_t batteryValue = PowerMgr::BatterySrvClient::GetInstance().GetCapacity();
    if (batteryValue <= BATTERY_MIN_VALUE) {
        return BATTERY_MIN_VALUE;
    }
    HILOGI("[HFP_SYSTEM_INTERFACE]batteryLevel_ = %{public}d", batteryLevel_);
    batteryLevel_ = std::round(static_cast<float_t>(batteryValue) / BATTERY_TOTAL_LEVEL);
#endif
    return batteryLevel_;
}

void HfpAgSystemInterface::QueryAgIndicator()
{
    HITRACE_METER(BT_TRACE_TAG);
    std::lock_guard<BtRecursiveMutex> lk(agIndicatorMutex_);
    // No interface for querying battery level
    batteryLevel_ = 0;
    // query ag indicator for the other module.
    serviceState_ = 0;
    roamState_ = 0;
    operatorName_ = "";
    signalStrength_ = 0;
    subscriberNumber_ = "";
#ifdef BLUETOOTH_HFP_AG_ENABLE
    CoreServiceClient::GetInstance().GetPrimarySlotId(slotId_);
    HILOGI("[HFP_SYSTEM_INTERFACE]SlotId_ is %{public}d", slotId_);
    if (slotId_ < 0) {
        HILOGE("[HFP_SYSTEM_INTERFACE]SlotId_ is invalid");
    } else {
        sptr<NetworkState> netWorkState = nullptr;
        CoreServiceClient::GetInstance().GetNetworkState(slotId_, netWorkState);
        if (netWorkState != nullptr) {
            serviceState_ = (netWorkState->GetRegStatus() == RegServiceState::REG_STATE_IN_SERVICE) ?
                NETWORK_STATE_AVAILABLE : NETWORK_STATE_NOT_AVAILABLE;
            roamState_ = netWorkState->IsRoaming();
            operatorName_ = netWorkState->GetLongOperatorName();
        } else {
            HILOGE("[HFP_SYSTEM_INTERFACE]NetWorkState is nullptr");
        }

        signalStrength_ = QuerySignalStrength();
        HILOGI("[HFP_SYSTEM_INTERFACE]serviceState_ = %{public}d, roamState_ = %{public}d,"
            " operatorName_ = %{public}s, signalStrength_ is %{public}d",
            serviceState_, roamState_, operatorName_.c_str(), signalStrength_);

        std::u16string telephoneNumber;
        CoreServiceClient::GetInstance().GetSimTelephoneNumber(slotId_, telephoneNumber);
        subscriberNumber_ = Str16ToStr8(telephoneNumber);
    }
#endif
    SendDeviceStateChanged();
    return;
}

void HfpAgSystemInterface::SendHfIndicator(const std::string &address, int indId, int indValue) const
{
    RawAddress device(address);
    HfpAgService *service = HfpAgService::GetService();
    if (service == nullptr) {
        HILOGE("[HFP_SYSTEM_INTERFACE]Service is nullptr");
        return;
    }
    if (indId == HFP_AG_HF_INDICATOR_ENHANCED_DRIVER_SAFETY_ID) {
        service->NotifyHfEnhancedDriverSafety(device, indValue);
    } else if (indId == HFP_AG_HF_INDICATOR_BATTERY_LEVEL_ID) {
        HILOGI("[HFP_SYSTEM_INTERFACE]HF Indicator batteryLevel:%{public}d", indValue);
        BatteryInfo info;
        info.batteryLevel_ = indValue;
        BluetoothDeviceBatteryManager::GetInstance()->SetRemoteDeviceBatteryInfo(device, info);
    } else {
        HILOGE("[HFP_SYSTEM_INTERFACE]HF Indicator id:%{public}d", indId);
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

void HfpAgSystemInterface::SendDeviceStateChanged() const
{
    HfpAgService *service = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "Service is null!");
    HfpAgTransferData data = {serviceState_, roamState_, signalStrength_, batteryLevel_};
    service->NotifyAgIndicatorStateChanged(HFP_AG_NOTIFY_DEVICE_STATE_CHANGED, data);
}

void HfpAgSystemInterface::OnSubscriptionStateChanged(int state)
{
    std::lock_guard<BtRecursiveMutex> lk(agIndicatorMutex_);
    int preService = serviceState_;
    serviceState_ = state;
    if (preService != serviceState_) {
        /*
        * When the service status changes, the signal strength does not change accordingly.
        * So when the service is unavailable, the signal strength needs to be updated to 0
        */
        signalStrength_ = (serviceState_ == NETWORK_STATE_NOT_AVAILABLE) ? 0 : QuerySignalStrength();
        SendDeviceStateChanged();
    }
}

void HfpAgSystemInterface::OnSignalStrengthChanged(int signal)
{
    std::lock_guard<BtRecursiveMutex> lk(agIndicatorMutex_);
    int preSignal = signalStrength_;
    signalStrength_ = (serviceState_ == NETWORK_STATE_NOT_AVAILABLE) ? 0 : signal;
    if (preSignal != signalStrength_) {
        SendDeviceStateChanged();
    }
}

void HfpAgSystemInterface::OnRoamStateChanged(int state)
{
    std::lock_guard<BtRecursiveMutex> lk(agIndicatorMutex_);
    int preRoam = roamState_;
    roamState_ = state;
    if (preRoam != roamState_) {
        SendDeviceStateChanged();
    }
}

void HfpAgSystemInterface::OnBatteryLevel(int level)
{
    std::lock_guard<BtRecursiveMutex> lk(agIndicatorMutex_);
    int preBattery = batteryLevel_;
    batteryLevel_ = level;
    if (preBattery != batteryLevel_) {
        SendDeviceStateChanged();
    }
}

bool HfpAgSystemInterface::IsCallIdle() const
{
    return !IsInCall() && !IsRinging();
}

bool HfpAgSystemInterface::MuteRinger() const
{
#ifdef BLUETOOTH_HFP_AG_ENABLE
    if (IsRinging()) {
        DelayedRefSingleton<BluetoothCallClient>::GetInstance().MuteRinger();
        return true;
    }
#endif
    return false;
}

bool HfpAgSystemInterface::IsInCall() const
{
    HILOGD("activeNum_: %{public}d, heldNum_: %{public}d, callState_: %{public}d", activeNum_, heldNum_, callState_);
    return ((activeNum_ > 0) || (heldNum_ > 0) ||
            ((callState_ != HFP_AG_CALL_STATE_IDLE) && (callState_ != HFP_AG_CALL_STATE_INCOMING) &&
                (callState_ != HFP_AG_CALL_STATE_DISCONNECTED) && (callState_ != HFP_AG_CALL_STATE_DISCONNECTING)));
}

bool HfpAgSystemInterface::IsRinging() const
{
    return (callState_ == HFP_AG_CALL_STATE_INCOMING);
}

int HfpAgSystemInterface::GetIndexForCall(int callId)
{
    // Indexes for bluetooth clcc are 1-based.
    int i = 1;
    if (clccIndexMap_.Find(callId, i)) {
        return i;
    }

    clccIndexMap_.Iterate([&i](int callId, int callIndex) {
        if (callIndex == i) {
            i++;
        }
    });

    // Indexes are removed in RemoveClccIndex.
    clccIndexMap_.Insert(callId, i);

    return i;
}

void HfpAgSystemInterface::RemoveClccIndex(int callId)
{
    clccIndexMap_.Erase(callId);
}

bool HfpAgSystemInterface::IsForegroundCallExist()
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef BLUETOOTH_HFP_AG_ENABLE
    int slotId = INVALID_SLOT_ID;
    std::vector<CallAttributeInfo> callList =
        DelayedRefSingleton<BluetoothCallClient>::GetInstance().GetCurrentCallList(slotId);

    for (auto call : callList) {
        int callState = static_cast<int>(call.callState);
        if (callState == static_cast<int>(TelCallState::CALL_STATUS_ACTIVE) ||
            callState == static_cast<int>(TelCallState::CALL_STATUS_DIALING)) {
                return true;
        }
    }
#endif
    return false;
}

#ifdef BLUETOOTH_HFP_AG_ENABLE
void HfpAgSystemInterface::AgTelephonyObserver::OnNetworkStateUpdated(int32_t slotId,
    const sptr<NetworkState> &networkState)
{
    HITRACE_METER(BT_TRACE_TAG);
    if (networkState == nullptr) {
        HILOGE("[HFP AG] networkState is null");
        return;
    }
    interface_.slotId_ = slotId;
    interface_.operatorName_ = networkState->GetLongOperatorName();
    if (networkState->GetRegStatus() == RegServiceState::REG_STATE_IN_SERVICE) {
        interface_.OnSubscriptionStateChanged(NETWORK_STATE_AVAILABLE);
    } else {
        interface_.OnSubscriptionStateChanged(NETWORK_STATE_NOT_AVAILABLE);
    }

    interface_.OnRoamStateChanged(networkState->IsRoaming());
}

void HfpAgSystemInterface::AgTelephonyObserver::OnSignalInfoUpdated(int32_t slotId,
    const std::vector<sptr<SignalInformation>> &vec)
{
    HITRACE_METER(BT_TRACE_TAG);
    interface_.slotId_ = slotId;
    if (!vec.empty()) {
        interface_.OnSignalStrengthChanged(vec[0]->GetSignalLevel());
    }
}
#endif

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

void HfpAgSystemInterface::OnAddSystemAbility(int32_t systemAbilityId)
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef BLUETOOTH_HFP_AG_ENABLE
    HILOGI("[HFP_SYSTEM_INTERFACE]systemAbilityId:%{public}d", systemAbilityId);
    switch (systemAbilityId) {
        case TELEPHONY_CALL_MANAGER_SYS_ABILITY_ID:
            HILOGI("[HFP_SYSTEM_INTERFACE]OnAddSystemAbility input service start");
            DoInHfpThread([]() {
                DelayedRefSingleton<BluetoothCallClient>::GetInstance().Init();
            });
            break;
        case TELEPHONY_CORE_SERVICE_SYS_ABILITY_ID:
            HILOGI("[HFP_SYSTEM_INTERFACE]OnAddSystemAbility register core service observer");
            HfpAgSystemInterface::GetInstance().RegisterObserver();
            break;
        case AUDIO_DISTRIBUTED_SERVICE_ID:
            HILOGI("[HFP_SYSTEM_INTERFACE]OnAddSystemAbility register audio server observer");
            HfpAgSystemInterface::GetInstance().RegisterObserver();
            DoInHfpThread([]() {
                HfpAgService *service = HfpAgService::GetService();
                CHECK_AND_RETURN_LOG(service != nullptr, "get service failed!");
                service->SetAudioParameters();
            });
            break;
        default:
            HILOGE("[HFP_SYSTEM_INTERFACE]unhandled sysabilityId:%{public}d", systemAbilityId);
            break;
    }
#endif
    return;
}

void HfpAgSystemInterface::OnRemoveSystemAbility(int32_t systemAbilityId)
{
    HITRACE_METER(BT_TRACE_TAG);
#ifdef BLUETOOTH_HFP_AG_ENABLE
    HILOGI("[HFP_SYSTEM_INTERFACE]systemAbilityId:%{public}d", systemAbilityId);
    switch (systemAbilityId) {
        case TELEPHONY_CALL_MANAGER_SYS_ABILITY_ID:
            HILOGI("[HFP_SYSTEM_INTERFACE]OnRemoveSystemAbility UnInit");
            DoInHfpThread([]() {
                DelayedRefSingleton<BluetoothCallClient>::GetInstance().UnInit();
                HfpAgService *service = HfpAgService::GetService();
                CHECK_AND_RETURN_LOG(service != nullptr, "getService failed!");
                Bluetooth::BluetoothPhoneState phoneState;
                phoneState.SetActiveNum(0);
                phoneState.SetHeldNum(0);
                phoneState.SetCallState(HFP_AG_CALL_STATE_IDLE);
                service->PhoneStateChanged(phoneState);
            });
            break;
        case TELEPHONY_CORE_SERVICE_SYS_ABILITY_ID:
            HILOGI("[HFP_SYSTEM_INTERFACE]OnRemoveSystemAbility unregister core service observer");
            HfpAgSystemInterface::GetInstance().UnregisterObserver();
            break;
        case AUDIO_DISTRIBUTED_SERVICE_ID:
            HILOGI("[HFP_SYSTEM_INTERFACE]OnRemoveSystemAbility unregister audio server observer");
            HfpAgSystemInterface::GetInstance().UnregisterObserver();
            break;
        default:
            HILOGE("[HFP_SYSTEM_INTERFACE]unhandled sysabilityId:%{public}d", systemAbilityId);
            break;
    }
#endif
    return;
}
}  // namespace bluetooth
}
