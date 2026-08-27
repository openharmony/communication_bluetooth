/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include <iomanip>

#include "bluetooth_audio_manager.h"
#include "bluetooth_device_battery_manager.h"
#include "hfp_ag_system_event_processer.h"

#include "hfp_ag_defines.h"
#include "hfp_ag_message.h"
#include "hfp_ag_service.h"
#include "common_util.h"
#include "service_util.h"
#include "bt_chr_dft_exception.h"
#include "bt_chr_ue_manager.h"
#include "permission_manager.h"
#include "preferences_manager.h"
#include "remote_device_properties.h"
#include "adapter_device_config.h"
#include "thread_util.h"
#include "a2dp_service.h"
#include "audio_device_info.h"
#include "audio_info.h"
#include "refuse_play_helper.h"

using namespace OHOS::Bluetooth;
namespace OHOS {
namespace bluetooth {
const int HW_WEAR_DETECTION_AT_COMMAND_HEAD_LEN = 2;
const int DISCONNECTED_SEND_BCC_RETRY_COUNT = 5;
const int WAITE_SCO_STATE_CHANGE_TIME = 50;
int g_scoSateTimeOutCount = 0;
static const size_t MAX_COPS_LENGTH = 15; //运营商名字最长15个字符

HfpAgSystemEventProcesser::HfpAgSystemEventProcesser(const std::string &address)
    :address_(address), atPhonebook_(address)
{
    hfpMaxVolume_ = BluetoothAudioFrameworkAdapter::BtGetMaxVolume(BtVolumeType::VOLUME_VOICE_CALL,
        BtDeviceType::DEVICE_TYPE_BLUETOOTH_SCO);
}

void HfpAgSystemEventProcesser::Start()
{
    HfpAgSystemInterface::GetInstance().Start();
    return;
}

void HfpAgSystemEventProcesser::Stop()
{
    HfpAgSystemInterface::GetInstance().Stop();
    return;
}

void HfpAgSystemEventProcesser::ExecuteOthersEvent(const HfpAgMessage &event)
{
    switch (event.type_) {
        case HFP_AG_MSG_TYPE_AT_CIND:
            GetAgIndicator();
            break;
        case HFP_AG_MSG_TYPE_AT_COPS:
            GetNetworkOperator();
            break;
        case HFP_AG_MSG_TYPE_AT_CLCC:
            ProcessClccEvent();
            break;
        case HFP_AG_MSG_TYPE_AT_UNKNOWN:
            ProcessAtUnknownEvent(event.str_, event.dev_);
            break;
        case HFP_AG_MSG_TYPE_KEY_PRESSED:
            ProcessKeyPressedEvent();
            break;
        case HFP_AG_MSG_TYPE_AT_BIND:
            ProcessATBindEvent(event.str_);
            break;
        case HFP_AG_MSG_TYPE_AT_BIEV:
            SendHfIndicator(event.arg1_, event.arg3_);
            break;
        case HFP_AG_MSG_TYPE_AT_BIA:
            ProcessAtBiaEvent(event);
            break;
        case HFP_AG_MSG_TYPE_QUERY_AG_INDICATOR:
            QueryAgIndicator();
            break;
        case HFP_AG_MSG_TYPE_VR_CHANGED:
            VoiceRecognitionStateChanged(event.arg1_);
            break;
        case HFP_AG_MSG_TYPE_AT_BCC:
            ProcessBccEvent();
            break;
        case HFP_AG_MSG_TYPE_AT_BRSF:
            ProcessBrsfEvent(event.arg1_);
            break;
        case HFP_AG_MSG_TYPE_AT_VENDOR_SPECIFIC:
            ProcessVendorSpecificAt(event.str_, event.dev_);
            break;
        default:
            break;
    }
}

void HfpAgSystemEventProcesser::ExecuteEventProcess(const HfpAgMessage &event)
{
    HILOGD("[HFP_EVENT_PROCESSER]EventType=%{public}s", GetEventType(event.type_).c_str());
    atPhonebook_.NotifyNewAtCommandRecv();
    switch (event.type_) {
        case HFP_AG_MSG_TYPE_ANSWER_CALL:
            ProcessAnswerCallEvent();
            break;
        case HFP_AG_MSG_TYPE_HANGUP_CALL:
            ProcessHangupCallEvent();
            break;
        case HFP_AG_MSG_TYPE_VOLUME_CHANGED:
            ProcessHfVolumeChangedEvent(event.arg1_, event.arg3_);
            break;
        case HFP_AG_MSG_TYPE_DIAL_CALL:
            ProcessDialOutCallEvent(event.str_);
            break;
        case HFP_AG_MSG_TYPE_SEND_DTMF:
            ProcessSendDtmfEvent(event.arg1_);
            break;
        case HFP_AG_MSG_TYPE_NOISE_REDUCTION:
            ProcessNoiseReductionEvent(event.arg1_);
            break;
        case HFP_AG_MSG_TYPE_AT_WBS:
            ProcessWideBandSpeechEvent(event.arg1_);
            break;
        case HFP_AG_MSG_TYPE_AT_CHLD:
            ProcessHoldCallEvent(event.arg1_);
            break;
        case HFP_AG_MSG_TYPE_SUBSCRIBER_NUMBER_REQUEST:
            GetSubscriberNumber();
            break;
        default:
            ExecuteOthersEvent(event);
            break;
    }
}

std::string HfpAgSystemEventProcesser::GetCallState(int state)
{
    switch (state) {
        case HFP_AG_CALL_STATE_ACTIVE:
            return "HFP_AG_CALL_STATE_ACTIVE";
        case HFP_AG_CALL_STATE_HELD:
            return "HFP_AG_CALL_STATE_HELD";
        case HFP_AG_CALL_STATE_DIALING:
            return "HFP_AG_CALL_STATE_DIALING";
        case HFP_AG_CALL_STATE_ALERTING:
            return "HFP_AG_CALL_STATE_ALERTING";
        case HFP_AG_CALL_STATE_INCOMING:
            return "HFP_AG_CALL_STATE_INCOMING";
        case HFP_AG_CALL_STATE_WAITING:
            return "HFP_AG_CALL_STATE_WAITING";
        case HFP_AG_CALL_STATE_IDLE:
            return "HFP_AG_CALL_STATE_IDLE";
        case HFP_AG_CALL_STATE_DISCONNECTED:
            return "HFP_AG_CALL_STATE_DISCONNECTED";
        case HFP_AG_CALL_STATE_DISCONNECTING:
            return "HFP_AG_CALL_STATE_DISCONNECTING";
        default:
            return "Unknown";
    }
}

int HfpAgSystemEventProcesser::CovertCallStateToBluetdroid(int callState)
{
    if (callState == HFP_AG_CALL_STATE_DISCONNECTED || callState == HFP_AG_CALL_STATE_DISCONNECTING) {
        return ::bluetooth::headset::BTHF_CALL_STATE_DISCONNECTED;
    }
    if (callState == HFP_AG_CALL_STATE_IDLE) {
        return ::bluetooth::headset::BTHF_CALL_STATE_IDLE;
    }
    return callState;
}

void HfpAgSystemEventProcesser::ProcessPhoneStateChange(const HfpAgPhoneState &phoneState)
{
    int numActive = phoneState.activeNum;
    int numHeld = phoneState.heldNum;
    int callState = phoneState.callState;
    std::string number = phoneState.number;
    int type = phoneState.type;
    std::string name = phoneState.name;

    HILOGI("[HFP_EVENT_PROCESSER]NumActive:%{public}d, numHeld:%{public}d, callState:%{public}d",
        numActive, numHeld, callState);

    HILOGI("[HFP_EVENT_PROCESSER] type:%{public}d, name:%{public}s", type, name.c_str());

    if (callState == HFP_AG_CALL_STATE_DISCONNECTED) {
        RefusePlayHelper::GetInstance()->SetLastHangUpTime(address_);
    }
    RawAddress device(address_);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    HfpAgService *service = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "get service failed!");
    ::bluetooth::headset::Interface* bluetoothHfpInterface = service->getBluetoothHfpInterface();
    if (bluetoothHfpInterface == nullptr) {
        HILOGE("[HFP_EVENT_PROCESSER]BluetoothHfpInterface is null");
        return;
    }
    HILOGI("[HFP_EVENT_PROCESSER]Hfp device address[%{public}s]", GetEncryptAddr(address_).c_str());
    bt_status_t result = bluetoothHfpInterface->PhoneStateChange(numActive, numHeld,
        (::bluetooth::headset::bthf_call_state_t)CovertCallStateToBluetdroid(callState), number.c_str(),
        (::bluetooth::headset::bthf_call_addrtype_t)type, name.c_str(), &rawAddr);
    if (result != BT_STATUS_SUCCESS) {
        HILOGE("[HFP_EVENT_PROCESSER]Failed ProcessPhoneStateChange, status: %{public}d", result);
        return;
    }
}

void HfpAgSystemEventProcesser::ProcessAnswerCallEvent() const
{
    HILOGI("[HFP_EVENT_PROCESSER]Answer call! address:%{public}s", GET_ENCRYPT_STR_ADDR(address_));
    HfpAgService *service = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "get service failed!");

    RawAddress device(address_);
    service->NotifyHfpStackChanged(UpdateOutputStackAction::ACTION_USER_OPERATION, device);
    systemInterface_.AnswerCall(address_);
}

void HfpAgSystemEventProcesser::ProcessHangupCallEvent() const
{
    HILOGI("[HFP_EVENT_PROCESSER]Hang up call! address:%{public}s, callState:%{public}d",
        GetEncryptAddr(address_).c_str(), systemInterface_.GetCallState());

    HfpAgService *service = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "get service failed!");
    if (service->IsVirtualCallStarted()) {
        NotifyStopVirtualCall();
        return;
    }

    RawAddress device(address_);
    if (service->GetScoState(device) == HFP_AG_AUDIO_STATE_CONNECTING) {
        HILOGI("ProcessHangupCallEvent do nothing when setting up sco");
        return;
    }

    if (systemInterface_.IsForegroundCallExist()) {
        systemInterface_.HangupCall(address_);
    } else if (systemInterface_.IsRinging()) {
        systemInterface_.RejectCall(address_);
    } else {
        systemInterface_.HangupCall(address_);
    }
}

void HfpAgSystemEventProcesser::NotifyStopVirtualCall() const
{
    HfpAgService *service = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "get service failed!");
    RawAddress device(address_);
    service->NotifyHfpStackChanged(UpdateOutputStackAction::ACTION_STOP_VIRTUAL_CALL, device);
}

void HfpAgSystemEventProcesser::ProcessHfVolumeChangedEvent(int type, int volume)
{
    HfpAgService *service = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "get service failed!");
    if (service->GetActiveDevice() != address_) {
        HILOGE("[HFP_EVENT_PROCESSER]This device is not active!");
        return;
    }
    HILOGD("[HFP_EVENT_PROCESSER]volume changed, address:%{public}s, , type:%{public}d, volume:%{public}d",
        GET_ENCRYPT_STR_ADDR(address_), type, volume);
    if (type == HFP_AG_VOLUME_TYPE_SPK) {
        speakerVolume_ = volume;
        if (service->GetScoState(RawAddress(address_)) == HFP_AG_AUDIO_STATE_CONNECTED) {
            PreferencesManager::Save(address_, volume, PreferencesManagerType::HFP_VOLUME);
        }
        systemInterface_.SetStreamVolume(STREAM_VOICE_CALL, volume, 1);
    } else {
        HILOGE("[HFP_EVENT_PROCESSER]The error volume type:%{public}d", type);
    }
}

void HfpAgSystemEventProcesser::ProcessDialOutCallEvent(const std::string &number)
{
    std::string dialNumber;
    if (number.length() == 0) {
        dialNumber = systemInterface_.GetLastDialNumber();
    } else if (number.at(0) == '>') {
        // Comparison with ">9999" is for PTS and BQB test
        if (number.compare(0, AT_COMMAND_LENGTH_FIVE, ">9999") == 0) {
            HILOGW("Number is too big.");
            ProcessAtResponseCodeEvent(HFP_AG_RESULT_ERROR, 0);
            return;
        }
        dialNumber = systemInterface_.GetLastDialNumber();
    } else {
        dialNumber = number;
    }

    HILOGI("[HFP_EVENT_PROCESSER] dialNumber length[%{public}d]", dialNumber.length());

    if (dialNumber.length() == 0) {
        ProcessAtResponseCodeEvent(HFP_AG_RESULT_ERROR, 0);
        return;
    }

    // Remove trailing ';'
    char type = dialNumber.at(dialNumber.length() - 1);
    if (type == ';') {
        dialNumber = dialNumber.substr(0, dialNumber.length() - 1);
    }

    HfpAgService *service = HfpAgService::GetService();

    if (service == nullptr) {
        ProcessAtResponseCodeEvent(HFP_AG_RESULT_ERROR, 0);
        return;
    }
    if (!service->DialOutCallByHf(address_)) {
        ProcessAtResponseCodeEvent(HFP_AG_RESULT_ERROR, 0);
        return;
    }
    if (systemInterface_.DialOutCall(address_, dialNumber) != DIAL_OUT_CALL_SUCCESS) {
        service->StopDialingTimer();
        HILOGE("[HFP_EVENT_PROCESSER] Failed DialOutCall!");
        ProcessAtResponseCodeEvent(HFP_AG_RESULT_ERROR, 0);
    } else {
        service->NotifyHfpStackChanged(UpdateOutputStackAction::ACTION_USER_OPERATION, RawAddress(address_));
    }
}

void HfpAgSystemEventProcesser::ProcessSendDtmfEvent(int dtmf) const
{
    if ((dtmf >= '0' && dtmf <= '9') || dtmf == '*' || dtmf == '#' ||
        dtmf == 'A' || dtmf == 'B' || dtmf == 'C' || dtmf == 'D') {
        systemInterface_.SendDtmf(dtmf, address_);
    } else {
        HILOGE("[HFP_EVENT_PROCESSER]The error dtmf value:%{public}d", dtmf);
    }
}

void HfpAgSystemEventProcesser::SetAudioExtraParametersForSwbOn()
{
    const std::string key = "bt_extra";
    std::vector<std::pair<std::string, std::string>> audioParametersKVPairs;
    audioParametersKVPairs.push_back(std::make_pair(HFP_AG_SERVER_SWB_NAME, HFP_AG_SERVER_AUDIO_FEATURE_ON));

    std::string nrecValue = HFP_AG_SERVER_AUDIO_FEATURE_OFF;
    auto it = audioParameterMap.find(HFP_AG_SERVER_NREC_NAME);
    if (it != audioParameterMap.end()) {
        nrecValue = it->second;
    }
    audioParametersKVPairs.push_back(std::make_pair(HFP_AG_SERVER_NREC_NAME, nrecValue));

    std::string wbsValueList(HFP_AG_SERVER_SWB_NAME);
    wbsValueList.append("=").append(HFP_AG_SERVER_AUDIO_FEATURE_ON);
    std::string nrecValueList(HFP_AG_SERVER_NREC_NAME);
    nrecValueList.append("=").append(nrecValue);
    std::string valueList = wbsValueList.append(";").append(nrecValueList);
    HILOGI("HFP_EVENT_PROCESSER]SetAudioParameters extra for key:%{public}s, %{public}s : %{public}s",
        key.c_str(), GetEncryptAddr(address_).c_str(), valueList.c_str());
    systemInterface_.SetExtraAudioParameters(key, audioParametersKVPairs);
}

void HfpAgSystemEventProcesser::SetAudioParameters(const std::string &key)
{
    std::string swbValue = HFP_AG_SERVER_AUDIO_FEATURE_OFF;
    auto it = audioParameterMap.find(HFP_AG_SERVER_SWB_NAME);
    if (it != audioParameterMap.end()) {
        swbValue = it->second;
    }
    if (swbValue == HFP_AG_SERVER_AUDIO_FEATURE_ON) {
        SetAudioExtraParametersForSwbOn();
        return;
    }

    std::string nrecValue = HFP_AG_SERVER_AUDIO_FEATURE_OFF;
    it = audioParameterMap.find(HFP_AG_SERVER_NREC_NAME);
    if (it != audioParameterMap.end()) {
        nrecValue = it->second;
    }
    std::string wbsValue = HFP_AG_SERVER_AUDIO_FEATURE_OFF;
    it = audioParameterMap.find(HFP_AG_SERVER_WBS_NAME);
    if (it != audioParameterMap.end()) {
        wbsValue = it->second;
    }

    std::string wbsValueList(HFP_AG_SERVER_WBS_NAME);
    wbsValueList.append("=").append(wbsValue);
    std::string nrecValueList(HFP_AG_SERVER_NREC_NAME);
    nrecValueList.append("=").append(nrecValue);
    std::string valueList = wbsValueList.append(";").append(nrecValueList);
    HILOGI("HFP_EVENT_PROCESSER]SetAudioParameters for key:%{public}s, %{public}s : %{public}s",
        key.c_str(), GetEncryptAddr(address_).c_str(), valueList.c_str());
    systemInterface_.SetAudioParameters(key, valueList);
}

void HfpAgSystemEventProcesser::ProcessNoiseReductionEvent(int flag)
{
    std::string prevNrecValue = HFP_AG_SERVER_AUDIO_FEATURE_OFF;
    auto it = audioParameterMap.find(HFP_AG_SERVER_NREC_NAME);
    if (it != audioParameterMap.end()) {
        prevNrecValue = it->second;
    }
    std::string newNrecValue = (flag == 0) ? HFP_AG_SERVER_AUDIO_FEATURE_OFF : HFP_AG_SERVER_AUDIO_FEATURE_ON;
    audioParameterMap[HFP_AG_SERVER_NREC_NAME] = newNrecValue;
    HILOGI("HFP_EVENT_PROCESSER]Nrec value is changed %{public}s -> %{public}s",
        prevNrecValue.c_str(), newNrecValue.c_str());

    HfpAgService *service = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "get service failed!");
    if (service->GetScoState(RawAddress(address_)) == HFP_AG_AUDIO_STATE_CONNECTED) {
        SetAudioParameters(HFP_AG_SERVER_NREC_NAME);
    }
}

void HfpAgSystemEventProcesser::ProcessWideBandSpeechEvent(int codec)
{
    std::string prevWbsValue = HFP_AG_SERVER_AUDIO_FEATURE_OFF;
    auto it = audioParameterMap.find(HFP_AG_SERVER_WBS_NAME);
    if (it != audioParameterMap.end()) {
        prevWbsValue = it->second;
    }
    std::string prevSwbValue = HFP_AG_SERVER_AUDIO_FEATURE_OFF;
    it = audioParameterMap.find(HFP_AG_SERVER_SWB_NAME);
    if (it != audioParameterMap.end()) {
        prevSwbValue = it->second;
    }
    std::string newWbsValue = HFP_AG_SERVER_AUDIO_FEATURE_OFF;
    if (codec == HFP_AG_SWB_YES) {
        audioParameterMap[HFP_AG_SERVER_SWB_NAME] = HFP_AG_SERVER_AUDIO_FEATURE_ON;
        audioParameterMap[HFP_AG_SERVER_WBS_NAME] = HFP_AG_SERVER_AUDIO_FEATURE_OFF;
        HILOGI("HFP_EVENT_PROCESSER]Wbs codec is %{public}d value is changed %{public}s -> %{public}s",
            codec, prevSwbValue.c_str(), HFP_AG_SERVER_AUDIO_FEATURE_ON);
    } else {
        newWbsValue = (codec == HFP_AG_WBS_YES) ? HFP_AG_SERVER_AUDIO_FEATURE_ON :
            HFP_AG_SERVER_AUDIO_FEATURE_OFF;
        audioParameterMap[HFP_AG_SERVER_WBS_NAME] = newWbsValue;
        audioParameterMap[HFP_AG_SERVER_SWB_NAME] = HFP_AG_SERVER_AUDIO_FEATURE_OFF;
        HILOGI("HFP_EVENT_PROCESSER]Wbs codec is %{public}d value is changed %{public}s -> %{public}s",
            codec, prevWbsValue.c_str(), newWbsValue.c_str());
    }

    HfpAgService *service = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "get service failed!");
    if (service->GetScoState(RawAddress(address_)) == HFP_AG_AUDIO_STATE_CONNECTED &&
        prevSwbValue == HFP_AG_SERVER_AUDIO_FEATURE_ON) {
        SetAudioParameters(HFP_AG_SERVER_WBS_NAME);
    }

    if (newWbsValue == "on" && handSetDelayTimer_ != nullptr) {
        HILOGI("Report that the talkband hfp is available.");
        UpdateTalkbandHandSetMode();
        handSetDelayTimer_->Stop();
        handSetDelayTimer_ = nullptr;
    }
}

void HfpAgSystemEventProcesser::ProcessA2dpSuspendStateEvent(bool a2dpSuspendState) const
{
    std::string key("A2dpSuspended");
    std::string value = "0";
    if (a2dpSuspendState) {
        value = "1";
    }
    systemInterface_.SetAudioParameters(key, value);
}

void HfpAgSystemEventProcesser::ProcessHoldCallEvent(int chld) const
{
    if (systemInterface_.HoldCall(chld, address_)) {
        ProcessAtResponseCodeEvent(HFP_AG_RESULT_OK, 0);
    } else {
        ProcessAtResponseCodeEvent(HFP_AG_RESULT_ERROR, 0);
    }
}

void HfpAgSystemEventProcesser::GetSubscriberNumber() const
{
    std::string number = systemInterface_.GetSubscriberNumber();
    if (!number.empty()) {
        int type;
        if (number[0] == '+') {
            type = HFP_AG_CALL_NUMBER_INTERNATIONAL;
        } else {
            type = HFP_AG_CALL_NUMBER_UNKNOW;
        }
        std::string cmd("+CNUM: ");
        cmd.append(",\"" + number + "\"," + std::to_string(type) + ",,4");
        ProcessAtResponseStringEvent(cmd);
        ProcessAtResponseCodeEvent(HFP_AG_RESULT_OK, 0);
    } else {
        ProcessAtResponseCodeEvent(HFP_AG_RESULT_ERROR, 0);
    }
}

void HfpAgSystemEventProcesser::ProcessAtCindCmdEvent(int call, int callsetup, int callheld)
{
    RawAddress device(address_);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    HfpAgService *service = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "get service failed!");
    ::bluetooth::headset::Interface* bluetoothHfpInterface = service->getBluetoothHfpInterface();
    if (bluetoothHfpInterface == nullptr) {
        HILOGE("[HFP_EVENT_PROCESSER]BluetoothHfpInterface is null");
        return;
    }
    HILOGI("[HFP_EVENT_PROCESSER]Hfp device address[%{public}s]", GetEncryptAddr(address_).c_str());
    int status = systemInterface_.GetServiceState();
    int signal = systemInterface_.GetSignalStrength();
    int roam = systemInterface_.GetRoamState();
    int battery = systemInterface_.GetBatteryLevel();
    bt_status_t result = bluetoothHfpInterface->CindResponse(status, call, callheld,
        static_cast<::bluetooth::headset::bthf_call_state_t>(callsetup), signal, roam, battery,
        &rawAddr);
    if (result != BT_STATUS_SUCCESS) {
        HILOGE("[HFP_EVENT_PROCESSER]Failed CindResponse, status: %{public}d", result);
        return;
    }
}

void HfpAgSystemEventProcesser::GetAgIndicator()
{
    int activeNum = systemInterface_.GetActiveCallNumber();
    int heldNum = systemInterface_.GetHeldCallNumber();
    int callsetup = systemInterface_.GetCallState();

    HfpAgService *service = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "get service failed!");
    if (service->IsVirtualCallStarted()) {
        activeNum = 1;
        heldNum = 0;
    }

    HILOGI("[HFP_EVENT_PROCESSER]ActiveNum[%{public}d], heldNum[%{public}d], callsetup[%{public}d]",
        activeNum, heldNum, callsetup);

    ProcessAtCindCmdEvent(activeNum, CovertCallStateToBluetdroid(callsetup), heldNum);
}

void HfpAgSystemEventProcesser::GetNetworkOperator() const
{
    std::string networkOperator = systemInterface_.GetNetworkOperator();
    RawAddress device(address_);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    HfpAgService *service = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "get service failed!");
    ::bluetooth::headset::Interface* bluetoothHfpInterface = service->getBluetoothHfpInterface();
    if (bluetoothHfpInterface == nullptr) {
        HILOGE("[HFP_EVENT_PROCESSER]BluetoothHfpInterface is null");
        return;
    }
    HILOGI("[HFP_EVENT_PROCESSER]Hfp device address[%{public}s]", GetEncryptAddr(address_).c_str());
    networkOperator = TruncateString(networkOperator, MAX_COPS_LENGTH);
    bt_status_t result = bluetoothHfpInterface->CopsResponse(networkOperator.c_str(), &rawAddr);
    if (result != BT_STATUS_SUCCESS) {
        HILOGE("[HFP_EVENT_PROCESSER]Failed CopsResponse, status: %{public}d", result);
        return;
    }
}

/**
 * Response for CLCC command. Can be iteratively called for each call index. Call index of 0
 * will be treated as NULL termination (Completes response)
 *
 * @param index index of the call given by the sequence of setting up or receiving the calls
 * as seen by the served subscriber. Calls hold their number until they are released. New
 * calls take the lowest available number.
 * @param dir direction of the call, 0 (outgoing), 1 (incoming)
 * @param status 0 = Active, 1 = Held, 2 = Dialing (outgoing calls only), 3 = Alerting
 * (outgoing calls only), 4 = Incoming (incoming calls only), 5 = Waiting (incoming calls
 * only), 6 = Call held by Response and Hold
 * @param mode 0 (Voice), 1 (Data), 2 (FAX)
 * @param mpty 0 - this call is NOT a member of a multi-party (conference) call, 1 - this
 * call IS a member of a multi-party (conference) call
 * @param number optional
 * @param type optional
 * @return True on success, False on failure
 */
void HfpAgSystemEventProcesser::ProcessClccResponseCmd(HfpAgCallList callList) const
{
    RawAddress device(address_);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    HfpAgService *service = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "get service failed!");
    ::bluetooth::headset::Interface* bluetoothHfpInterface = service->getBluetoothHfpInterface();
    if (bluetoothHfpInterface == nullptr) {
        HILOGE("[HFP_EVENT_PROCESSER]BluetoothHfpInterface is null");
        return;
    }
    HILOGI("[HFP_EVENT_PROCESSER]Hfp device address[%{public}s]", GetEncryptAddr(address_).c_str());
    int index = callList.index;
    int dir = callList.dir;
    int state = callList.state;
    int mode = callList.mode;
    int mpty = callList.mpty;
    std::string number = callList.number;
    int type = callList.type;
    /* index of the call, index == 2 means the second call */
    if ((index == SECOND_CALL_INDEX) && (state == HFP_AG_CALL_STATE_INCOMING)) {
        state = HFP_AG_CALL_STATE_WAITING;
    }
    if (!number.empty()) {
        type = GetPhoneNumberType(number);
    }
    bt_status_t result = bluetoothHfpInterface->ClccResponse(index,
        (::bluetooth::headset::bthf_call_direction_t)dir,
        (::bluetooth::headset::bthf_call_state_t)state,
        (::bluetooth::headset::bthf_call_mode_t)mode,
        mpty ? (::bluetooth::headset::BTHF_CALL_MPTY_TYPE_MULTI) : (::bluetooth::headset::BTHF_CALL_MPTY_TYPE_SINGLE),
        number.c_str(), (::bluetooth::headset::bthf_call_addrtype_t)type, &rawAddr);
    if (result != BT_STATUS_SUCCESS) {
        HILOGE("[HFP_EVENT_PROCESSER]Failed ClccResponse, status: %{public}d", result);
        return;
    }
}

int HfpAgSystemEventProcesser::GetPhoneNumberType(std::string phoneNumber) const
{
    if (phoneNumber.length() > 0 && phoneNumber[0] == '+') {
        return HFP_AG_CALL_NUMBER_INTERNATIONAL;
    }
    return HFP_AG_CALL_NUMBER_UNKNOW;
}

HfpAgCallList HfpAgSystemEventProcesser::CreateCallList(int index, int state, std::string phoneNumber, int type) const
{
    HfpAgCallList callList;
    callList.index = index;
    callList.dir = 0;
    callList.state = state;
    callList.mode = 0;
    callList.mpty = 0;
    callList.number = phoneNumber;
    callList.type = type;
    return callList;
}

void HfpAgSystemEventProcesser::ProcessClccEvent()
{
    HfpAgService *service = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "Get HfpAgService failed, service is null.");
    HfpAgCallList callList;
    if (service->IsVirtualCallStarted()) {
        // In order to fix LingKe and Chevrolet car virtual call no sound issue, send 00000000000 when response AT CLCC
        std::string phoneNumber = "00000000000";
        int phoneType = GetPhoneNumberType(phoneNumber);
        int activeNum = service->GetVirtualCallActiveNum();
        if (activeNum == 0) {
            callList = CreateCallList(1, service->GetVirtualCallState(), phoneNumber, phoneType);
        } else {
            callList = CreateCallList(1, 0, phoneNumber, phoneType);
        }
        ProcessClccResponseCmd(callList);
        callList = CreateCallList(0, 0, "", 0);
        ProcessClccResponseCmd(callList);
    } else {
        if (!systemInterface_.QueryCurrentCallsList()) {
            HILOGE("[HFP_EVENT_PROCESSER]processAtClcc: failed to list current calls for %{public}s",
                GetEncryptAddr(address_).c_str());
            callList = CreateCallList(0, 0, "", 0);
            ProcessClccResponseCmd(callList);
        } else {
            StartResponseClccTimer();
        }
    }
}

void HfpAgSystemEventProcesser::StartResponseClccTimer()
{
    ResponseClccTimeout_ = std::make_unique<utility::Timer>([this]() { this->ResponseClccTimeOut(); });
    ResponseClccTimeout_->Start(RESPONSE_CLCC_TIMEOUT_TIME);
    HILOGI("Start clcc timer!");
}

void HfpAgSystemEventProcesser::ResponseClccTimeOut()
{
    HfpAgService *service = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "Get HfpAgService failed, service is null.");
    service->ResponseClccTimeOut();
}

bool HfpAgSystemEventProcesser::ProcessAtPhonebookCommand(const std::string &atCommand)
{
    if (atCommand.compare(0, AT_COMMAND_LENGTH_FIVE, "+CSCS") == 0) {
        atPhonebook_.HandleCscsCommand(atCommand);
    } else if (atCommand.compare(0, AT_COMMAND_LENGTH_FIVE, "+CPBS") == 0) {
        atPhonebook_.HandleCpbsCommand(atCommand);
    } else if (atCommand.compare(0, AT_COMMAND_LENGTH_FIVE, "+CPBR") == 0) {
        atPhonebook_.HandleCpbrCommand(atCommand);
    } else {
        return false;
    }
    return true;
}

bool HfpAgSystemEventProcesser::ProcessBatteryAtCommand(const std::string &atCommand) const
{
    HILOGD("atCommand = %{public}s", atCommand.c_str());
    bool result = false;
    if (StartWith(atCommand, "+IPHONEACCEV")) {
        result = BluetoothDeviceBatteryManager::GetInstance()->ProcessBatteryCommond(address_, atCommand);
    } else if (StartWith(atCommand, "+XAPL")) {
        result = ProcessXaplAtCommand(atCommand);
    } else if (StartWith(atCommand, "+XEVENT")) {
        result = true;
    } else {
        return false;
    }

    if (result) {
        ProcessAtResponseCodeEvent(HFP_AG_RESULT_OK, 0);
    } else {
        ProcessAtResponseCodeEvent(HFP_AG_RESULT_ERROR, 0);
    }
    return true;
}

bool HfpAgSystemEventProcesser::ProcessXaplAtCommand(const std::string &atCommand) const
{
    std::string commandValue = GetCommandValue(atCommand);
    if (commandValue.empty()) {
        HILOGE("wrong atCommand = %{public}s", atCommand.c_str());
        return true;
    }
    std::vector<std::string> values = SplitValueByDelim(commandValue, ",");
    if (values.size() != AT_XAPL_VALUE_SIZE) {
        HILOGE("value length must be 2, size = %{public}d", values.size());
        return true;
    }

    std::vector<std::string> deviceInfos = SplitValueByDelim(values[0], "-");
    if (deviceInfos.size() != AT_XAPL_DEVICE_INFO_SIZE) {
        HILOGE("deviceInfos length must be 3, size = %{public}d", deviceInfos.size());
        return true;
    }

    // feature = 2 indicates that we support battery level reporting only
    std::string atCommandResponse = "+XAPL=iPhone,2";
    ProcessAtResponseStringEvent(atCommandResponse);
    return true;
}

bool HfpAgSystemEventProcesser::ProcessHwBatteryAtCommand(const std::string &atCommand) const
{
    HILOGD("atCommand = %{public}s", atCommand.c_str());
    return false;
}

bool HfpAgSystemEventProcesser::IsSupportedAtCommand(const std::string &atCommand) const
{
    std::vector<std::string> supportedAtCommand = {
        "+XEVENT",
        "+XAPL",
        "+IPHONEACCEV",
        "+TBSF",
        "+TBSR",
        "+HWCMD",
    };

    for (const auto &supportedAt : supportedAtCommand) {
        if (StartWith(atCommand, supportedAt)) {
            return true;
        }
    }

    HILOGI("unsupported at command: %{public}s", atCommand.c_str());
    if(StartWith(atCommand, "+BRSF")) {
        // The BRSF instruction is expected to be processed but not recognized.
        BtChrCallExcpEvent(address_, ERRCODE_HFP_ATCOMNAND_ERROR, CHR_SUB_ERRCODE_CASE1, -1);
    }
    ProcessAtResponseCodeEvent(HFP_AG_RESULT_ERROR, 0);
    return false;
}

void HfpAgSystemEventProcesser::ProcessPhonebookAuthResultEvent(const HfpAgMessage &event)
{
    atPhonebook_.StopRequestPermissionTimer();
    if (!atPhonebook_.IsCheckingAccessPermission()) {
        HILOGI("[HFP_EVENT_PROCESSER] not checking AccessPermission");
        return;
    }
    atPhonebook_.HandlePbAccessResult(event.dev_, event.arg1_, event.arg3_);
}

bool HfpAgSystemEventProcesser::ProcessVendorSpecificAt(const std::string &atCommand, const std::string &address)
{
    auto indexOfEqual = atCommand.find("=");
    if (indexOfEqual == std::string::npos) {
        return false;
    }
    std::string atCommandPara = atCommand.substr(0, indexOfEqual);
    std::string atCommandValue = atCommand.substr(indexOfEqual + 1);
    return ProcessHwPrivateCommand(atCommandPara, atCommandValue, address);
}

bool HfpAgSystemEventProcesser::ProcessHwPrivateCommand(const std::string &atCommandPara,
    const std::string &atCommandValue, const std::string &address)
{
    if (address.empty()) {
        HILOGI("address is invalid");
        return false;
    }
    HILOGI("[ProcessHwPrivateCommand] atCommandPara = %{public}s", atCommandPara.c_str());
    if (atCommandPara.empty() || atCommandValue.empty()) {
        HILOGI("AT command is invalid");
        return false;
    }
    if (atCommandPara == TALK_BAND_SET_FEATURE_FORMAT) { // +TBSF
        return ProcessTalkBandSetFeatureCommand(atCommandValue);
    } else if (atCommandPara == TALK_BAND_SET_REQUEST_FORMAT) { // +TBSR
        return ProcessTalkBandSetRequestCommand(atCommandValue, address);
    }
    return false;
}

bool HfpAgSystemEventProcesser::ProcessTalkBandSetFeatureCommand(const std::string &atArgument) const
{
    if (atArgument.empty()) {
        return false;
    }
    if (atArgument.compare(0, 1, "?") == 0) {
        ProcessAtResponseCodeEvent(HFP_AG_RESULT_OK, 0);
        std::string stringToSend = std::string{TALK_BAND_SET_FEATURE_FORMAT} + ": " + HW_PHONE_FEATURES;
        ProcessAtResponseStringEvent(stringToSend);
    } else {
        ProcessAtResponseCodeEvent(HFP_AG_RESULT_OK, 0);
    }
    return true;
}

bool HfpAgSystemEventProcesser::ProcessTalkBandSetRequestCommand(const std::string &atArgument,
    const std::string &address)
{
    if (atArgument.empty()) {
        return false;
    }
    if ((atArgument.compare(0, HW_WEAR_DETECTION_AT_COMMAND_HEAD_LEN, HW_WEAR_DETECTION_TWS_COMMAND_HEAD)) == 0 ||
        (atArgument.compare(0, HW_WEAR_DETECTION_AT_COMMAND_HEAD_LEN, HW_WEAR_DETECTION_NOT_TWS_COMMAND_HEAD)) == 0) {
        ProcessWearDetection(atArgument, address);
        return true;
    }
    const auto &itFunc = bandFeatures_.find(atArgument);
    if (itFunc != bandFeatures_.end()) {
        (this->*(itFunc->second))();
        return true;
    }
    return false;
}

static void SetHwDeviceInfo(RemoteDeviceProperties* remoteDeviceProp, std::string cmdType, std::string cmdValue,
    RawAddress device)
{
    static std::map<std::string, bt_property_type_t> devicePropertiesTable {
        {VALUE_OF_DEVICE_INFO_NAME, bt_property_type_t::BT_PROPERTY_BDNAME},
        {VALUE_OF_DEVICE_INFO_VENDOR_ID, bt_property_type_t::BT_PROPERTY_VENDOR_ID},
        {VALUE_OF_DEVICE_INFO_PRODUCT_ID, bt_property_type_t::BT_PROPERTY_PRODUCT_ID},
        {VALUE_OF_DEVICE_INFO_VERSION, bt_property_type_t::BT_PROPERTY_NAME_CHANGE_VERSION},
        {VALUE_OF_DEVICE_INFO_NEW_MODEL_ID, bt_property_type_t::BT_PROPERTY_REMOTE_NEW_MODEL_ID},
        {VALUE_OF_DEVICE_INFO_MODEL_ID, bt_property_type_t::BT_PROPERTY_REMOTE_MODEL_ID},
        {VALUE_OF_DEVICE_CLASS_COD, bt_property_type_t::BT_PROPERTY_CLASS_OF_DEVICE},
        {VALUE_OF_DEVICE_ICON, bt_property_type_t::BT_PROPERTY_DEVICE_INFO},
        {VALUE_OF_DEVICE_TYPE_ID, bt_property_type_t::BT_PROPERTY_DEVICE_TYPE_ID}
    };
    auto iter = devicePropertiesTable.find(cmdType);
    if (iter == devicePropertiesTable.end()) {
        HILOGI("invalid cmdType, cmdType: %{public}s", cmdType.c_str());
        return;
    }
    remoteDeviceProp->SetRemoteDevicePropertyInfo(device, iter->second, cmdValue);
}

void HfpAgSystemEventProcesser::UpdateHwDeviceInfo(const std::string &deviceName, const std::string &deviceTimeStamp)
{
    CHECK_AND_RETURN_LOG(deviceName != INVALID_NAME, "deviceName is invalid");
    RawAddress device(address_);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    int32_t result = RemoteDeviceProperties::GetInstance()->GetRemoteDeviceProperty(rawAddr,
        bt_property_type_t::BT_PROPERTY_TIMESTAMP);
    CHECK_AND_RETURN_LOG(result == BT_STATUS_SUCCESS, "get timestamp failed");
    int32_t timeStamp = INVALID_VALUE;
    if (!ConvertStrToDigit(deviceTimeStamp, timeStamp, HEX_STRING_TO_INT)) {
        HILOGE("convert timestamp failed, deviceTimeStamp: %{public}s", deviceTimeStamp.c_str());
        return;
    }
    std::shared_ptr<BluetoothDevice> remoteDevice =
        RemoteDeviceProperties::GetInstance()->GetBluetoothDeviceFromMap(device);
    CHECK_AND_RETURN_LOG(remoteDevice != nullptr, "remoteDevice is null");
    int32_t localTimeStamp = remoteDevice->GetTimeStamp();
    HILOGI("timeStamp: %{public}d, localTimeStamp: %{public}d", timeStamp, localTimeStamp);
    if (timeStamp > localTimeStamp) {
        RemoteDeviceProperties::GetInstance()->SetRemoteDevicePropertyInfo(device,
            bt_property_type_t::BT_PROPERTY_BDNAME, deviceName);
        RemoteDeviceProperties::GetInstance()->SetRemoteDevicePropertyInfo(device,
            bt_property_type_t::BT_PROPERTY_REMOTE_FRIENDLY_NAME, deviceName);
        RemoteDeviceProperties::GetInstance()->SetRemoteDevicePropertyInfo(device,
            bt_property_type_t::BT_PROPERTY_TIMESTAMP, deviceTimeStamp);
    } else if (timeStamp < localTimeStamp) {
        if (deviceTimestamp_.empty()) {
            deviceTimestamp_ = DexToHexString(localTimeStamp);
        }
    } else {
        HILOGE("timestamp is not up-to-date");
    }
}

bool HfpAgSystemEventProcesser::ProcessHwDeviceInfo(const std::string &atArgument)
{
    int atLen = static_cast<int>(atArgument.size());
    int index = LENGTH_OF_AT_CMD_HEADER - 1;
    RawAddress device(address_);
    RemoteDeviceProperties* remoteDeviceProp = RemoteDeviceProperties::GetInstance();
    CHECK_AND_RETURN_LOG_RET(remoteDeviceProp != nullptr, false, "device not exist.");
    std::string deviceName;
    std::string deviceTimeStamp;
    while (index + LENGTH_OF_DEVICE_INFO_TYPE < atLen) {
        std::string cmdType = atArgument.substr(index, LENGTH_OF_DEVICE_INFO_TYPE);
        std::string cmdLen = atArgument.substr(index + LENGTH_OF_DEVICE_INFO_TYPE, LENGTH_OF_DEVICE_INFO_FLAG);
        uint32_t len = 0;
        if (!ConvertStrToDigit(cmdLen, len)) {
            HILOGE("invalid cmdLen, cmdType: %{public}s, cmdLen: %{public}s", cmdType.c_str(), cmdLen.c_str());
            return false;
        }
        if (index + LENGTH_OF_DEVICE_INFO_TYPE + LENGTH_OF_DEVICE_INFO_FLAG + static_cast<int>(len) > atLen) {
            HILOGE("invalid length, index: %{public}d, len: %{public}u", index, len);
            return false;
        }
        std::string cmdValue = atArgument.substr(index + LENGTH_OF_DEVICE_INFO_TYPE + LENGTH_OF_DEVICE_INFO_FLAG, len);
        HILOGD("cmdType: %{public}s, len: %{public}u, cmdValue: %{public}s ", cmdType.c_str(), len, cmdValue.c_str());
        if (cmdType == VALUE_OF_DEVICE_INFO_NAME) {
            deviceName = cmdValue;
        } else if (cmdType == VALUE_OF_DEVICE_INFO_TIMESTAMP) {
            deviceTimeStamp = cmdValue;
        } else {
            SetHwDeviceInfo(remoteDeviceProp, cmdType, cmdValue, device);
        }
        index += LENGTH_OF_DEVICE_INFO_TYPE + LENGTH_OF_DEVICE_INFO_FLAG + static_cast<int>(len);
    }
    UpdateHwDeviceInfo(deviceName, deviceTimeStamp);
    return true;
}

void HfpAgSystemEventProcesser::DoHwTalkbandBandMode()
{
    HfpAgService *service = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "get service failed!");
    RawAddress device(address_);
    service->NotifyHfpStackChanged(UpdateOutputStackAction::ACTION_DISABLE_FROM_REMOTE, device);
    ProcessAtResponseCodeEvent(HFP_AG_RESULT_OK, 0);
    BtChrUeManager::GetInstance()->ReportBandWearStateUe(GHR_UE_BAND_WEAR_STATE_CHANGE, device, UE_INBAND);
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    CHECK_AND_RETURN_LOG(remoteDevice != nullptr, "remoteDevice is null");
    remoteDevice->SetHwTalkBandBandMode(TalkBandMode::VALUE_OF_TALKBAND_BAND_MODE);
}

void HfpAgSystemEventProcesser::DoHwTalkbandHandSetMode()
{
    ProcessAtResponseCodeEvent(HFP_AG_RESULT_OK, 0);
    int32_t delayMs = 500;
    handSetDelayTimer_ = std::make_shared<utility::Timer>([this]() { this->UpdateTalkbandHandSetMode(); });
    int ret = handSetDelayTimer_->Start(delayMs);
    HILOGI("DoHwTalkbandHandSetMode:ret:%{public}d", ret);
    RawAddress device(address_);
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    CHECK_AND_RETURN_LOG(remoteDevice != nullptr, "remoteDevice is null");
    remoteDevice->SetHwTalkBandBandMode(TalkBandMode::VALUE_OF_TALKBAND_HEADSET_MODE);
}

void HfpAgSystemEventProcesser::UpdateTalkbandHandSetMode()
{
    HILOGI("Update HfpStack ENABLE_FROM_REMOTE");
    HfpAgService *service = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "get service failed!");
    RawAddress device(address_);
    service->NotifyHfpStackChanged(UpdateOutputStackAction::ACTION_ENABLE_FROM_REMOTE, device);
    service->NotifyHfpStackChanged(UpdateOutputStackAction::ACTION_UPDATE_DEVICE_TYPE, device);
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService != nullptr) {
        HILOGI("headset mode, update a2dp stack change");
        a2dpService->NotifyMediaStackChanged(UpdateOutputStackAction::ACTION_UPDATE_DEVICE_TYPE, device);
    }
    BtChrUeManager::GetInstance()->ReportBandWearStateUe(GHR_UE_BAND_WEAR_STATE_CHANGE, device, UE_OUTBAND);
}

void HfpAgSystemEventProcesser::DoHwCameraTakePicture()
{
    ProcessAtResponseCodeEvent(HFP_AG_RESULT_OK, 0);
}

void HfpAgSystemEventProcesser::DoFindPhoneAT()
{
    ProcessAtResponseCodeEvent(HFP_AG_RESULT_OK, 0);
}

void HfpAgSystemEventProcesser::DoHwMutePhoneAT()
{
    systemInterface_.MuteRinger();
    ProcessAtResponseCodeEvent(HFP_AG_RESULT_OK, 0);
}

void HfpAgSystemEventProcesser::DoHwActiveAssistant()
{
    HfpAgService *service = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "get service failed!");
    std::string callingName = PermissionManager::GetCallingName();
    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_START_VOICE_REG, RawAddress(address_),
        UE_REMOTE_START_VR_RECV_TBSR_REQ, callingName);
    if (!service->OpenVoiceRecognitionByHf(address_)) {
        ProcessAtResponseCodeEvent(HFP_AG_RESULT_ERROR, 0);
    }
}

void HfpAgSystemEventProcesser::DoHwTalkbandDeiveMode()
{
    ProcessAtResponseCodeEvent(HFP_AG_RESULT_OK, 0);
}

void HfpAgSystemEventProcesser::DoHwVrDeviceOutMode()
{
    ProcessAtResponseCodeEvent(HFP_AG_RESULT_OK, 0);
}

void HfpAgSystemEventProcesser::DoHwVrDeviceInMode()
{
    ProcessAtResponseCodeEvent(HFP_AG_RESULT_OK, 0);
}

void HfpAgSystemEventProcesser::DoHwPhoneHolder()
{
    ProcessAtResponseCodeEvent(HFP_AG_RESULT_OK, 0);
}

void HfpAgSystemEventProcesser::ProcessWearDetection(const std::string &atArgument, const std::string &address) const
{
    RawAddress device(address);
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    remoteDevice->SetWearDetectionSupportValue(true);
    if (atArgument == HW_WEAR_DETECTION_TWS_BOTH_OUT || atArgument == HW_WEAR_DETECTION_NOT_TWS_OUT) {
        BluetoothAudioManager::GetInstance().UpdateDeviceWearState(remoteDevice, false, false);
        ProcessAtResponseCodeEvent(HFP_AG_RESULT_OK, 0);
    } else if (atArgument == HW_WEAR_DETECTION_TWS_RIGHT_IN) {
        BluetoothAudioManager::GetInstance().UpdateDeviceWearState(remoteDevice, false, true);
        ProcessAtResponseCodeEvent(HFP_AG_RESULT_OK, 0);
    } else if (atArgument == HW_WEAR_DETECTION_TWS_LEFT_IN) {
        BluetoothAudioManager::GetInstance().UpdateDeviceWearState(remoteDevice, true, false);
        ProcessAtResponseCodeEvent(HFP_AG_RESULT_OK, 0);
    } else if (atArgument == HW_WEAR_DETECTION_TWS_BOTH_IN || atArgument == HW_WEAR_DETECTION_NOT_TWS_IN) {
        BluetoothAudioManager::GetInstance().UpdateDeviceWearState(remoteDevice, true, true);
        ProcessAtResponseCodeEvent(HFP_AG_RESULT_OK, 0);
    } else {
        HILOGI("processWearDetection got wrong atArgument");
        ProcessAtResponseCodeEvent(HFP_AG_RESULT_ERROR, 0);
    }
}

bool HfpAgSystemEventProcesser::ProcessPhoneATCommand(const std::string atCommand) const
{
    std::string stringToSend = atCommand + ": ";
    HILOGI("[HFP_EVENT_PROCESSER] atCommand = %{public}s", atCommand.c_str());
    if (atCommand.compare(0, AT_COMMAND_LENGTH_FIVE, "+CGMI") == 0) {
        // The +CGMI command reports the GSM Data Module's manufacturer identification.
        // 系统属性可以读取后,需要优先从系统属性读取
    } else if (atCommand.compare(0, AT_COMMAND_LENGTH_FIVE, "+CGMM") == 0) {
        // The +CGMM command reports the specific model of Data Module
        // 系统属性可以读取后,需要优先从系统属性读取
        stringToSend += "UNKNOWN";
    } else if (atCommand.compare(0, AT_COMMAND_LENGTH_FIVE, "+CGMR") == 0) {
        // The +CGMR command reports version number
        // 系统属性可以读取后,需要优先从系统属性读取
        stringToSend += "UNKNOWN";
    } else if (atCommand.compare(0, AT_COMMAND_LENGTH_FIVE, "+CGSN") == 0) {
        // get CGSN, now set unknown for testing
        // 系统属性可以读取后,需要优先从系统属性读取
        stringToSend += "UNKNOWN";
    } else if (atCommand.compare(0, AT_COMMAND_LENGTH_FIVE, "+CIMI") == 0) {
        // get CIMI, now set unknown for testing
        // 系统属性可以读取后,需要优先从系统属性读取
        stringToSend += "UNKNOWN";
    } else if (atCommand.compare(0, AT_COMMAND_LENGTH_FOUR, "+GMI") == 0) {
        // get GMI, now set unknown for testing
        // 系统属性可以读取后,需要优先从系统属性读取
        stringToSend += "UNKNOWN";
    } else if (atCommand.compare(0, AT_COMMAND_LENGTH_FOUR, "+GMM") == 0) {
        // get GMM, now set unknown for testing
        // 系统属性可以读取后,需要优先从系统属性读取
        stringToSend += "UNKNOWN";
    } else if (atCommand.compare(0, AT_COMMAND_LENGTH_FOUR, "+GMR") == 0) {
        // get GMR, now set unknown for testing
        // 系统属性可以读取后,需要优先从系统属性读取
        stringToSend += "UNKNOWN";
    } else {
        return false;
    }
    ProcessAtResponseStringEvent(stringToSend);
    ProcessAtResponseCodeEvent(HFP_AG_RESULT_OK, 0);
    return true;
}

void HfpAgSystemEventProcesser::ProcessAtUnknownEvent(const std::string &atString, const std::string &address)
{
    if (atString.empty()) {
        HILOGE("[HFP_EVENT_PROCESSER] atString is empty");
        return;
    }
    if (ProcessAtPhonebookCommand(atString)) {
        HILOGI("[HFP_EVENT_PROCESSER] ProcessAtPhonebookCommand is true");
        return;
    }

    if (ProcessPhoneATCommand(atString)) {
        HILOGD("[HFP_EVENT_PROCESSER] ProcessPhoneATCommand is true");
        return;
    }

    if (!IsSupportedAtCommand(atString)) {
        HILOGD("[HFP_EVENT_PROCESSER] IsSupportedAtCommand is false");
        return;
    }

    if (ProcessVendorSpecificAt(atString, address)) {
        HILOGD("[HFP_EVENT_PROCESSER] ProcessVendorSpecificAt is true");
        return;
    }

    if (ProcessBatteryAtCommand(atString)) {
        HILOGD("[HFP_EVENT_PROCESSER] ProcessBatteryAtCommand is true");
        return;
    }
    if (ProcessHwBatteryAtCommand(atString)) {
        HILOGD("[HFP_EVENT_PROCESSER] ProcessHwBatteryAtCommand is true");
        return;
    }

    ProcessAtResponseCodeEvent(HFP_AG_RESULT_OK, 0);
}

void HfpAgSystemEventProcesser::ProcessKeyPressedEvent() const
{}

void HfpAgSystemEventProcesser::ProcessATBindEvent(const std::string &atString) const
{}

void HfpAgSystemEventProcesser::SendHfIndicator(int indId, int indValue) const
{
    systemInterface_.SendHfIndicator(address_, indId, indValue);
}

void HfpAgSystemEventProcesser::ProcessAtBiaEvent(const HfpAgMessage &event)
{
    HILOGI("[HFP_EVENT_PROCESSER]BIA command is battery[%{public}d], roam[%{public}d], "
        "service[%{public}d], signal[%{public}d]",
        event.data_.battery, event.data_.roam, event.data_.service, event.data_.signal);
}

void HfpAgSystemEventProcesser::QueryAgIndicator()
{
    systemInterface_.QueryAgIndicator();
}

void HfpAgSystemEventProcesser::VoiceRecognitionStateChanged(int status)
{
    HfpAgService *service = HfpAgService::GetService();
    std::string callingName = PermissionManager::GetCallingName();
    if (service == nullptr) {
        HILOGE("hfp ag service is null.");
        ProcessAtResponseCodeEvent(HFP_AG_RESULT_ERROR, 0);
        return;
    }

    if (status != HFP_AG_HF_VR_OPENED && status != HFP_AG_HF_VR_ClOSED) {
        HILOGE("voice recognition status[%{public}d] is invaild", status);
        ProcessAtResponseCodeEvent(HFP_AG_RESULT_ERROR, 0);
        return;
    }

    HILOGI("voice recognition status[%{public}d]", status);

    if (status == HFP_AG_HF_VR_OPENED) {
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_START_VOICE_REG, RawAddress(address_),
            UE_REMOTE_START_VR_RECV_REQ, callingName);
        if (!service->OpenVoiceRecognitionByHf(address_)) {
            ProcessAtResponseCodeEvent(HFP_AG_RESULT_ERROR, 0);
        }
    } else {
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_STOP_VOICE_REG, RawAddress(address_),
            UE_REMOTE_STOP_VR_RECV_REQ, callingName);
        bool result = service->CloseVoiceRecognitionByHf(address_);
        ProcessAtResponseCodeEvent(static_cast<int>(result), 0);
    }
}

void HfpAgSystemEventProcesser::ProcessAtResponseCodeEvent(int responseCode, int errorCode) const
{
    RawAddress device(address_);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    HfpAgService *service = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "get service failed!");
    ::bluetooth::headset::Interface* bluetoothHfpInterface = service->getBluetoothHfpInterface();
    if (bluetoothHfpInterface == nullptr) {
        HILOGE("[HFP_EVENT_PROCESSER]BluetoothHfpInterface is null");
        return;
    }
    HILOGI("[HFP_EVENT_PROCESSER]Hfp device address[%{public}s]", GetEncryptAddr(address_).c_str());
    bt_status_t result = bluetoothHfpInterface->AtResponse(
        static_cast<::bluetooth::headset::bthf_at_response_t>(responseCode), errorCode, &rawAddr);
    if (result != BT_STATUS_SUCCESS) {
        HILOGE("[HFP_EVENT_PROCESSER]Failed AtResponseCode, status: %{public}d", result);
        return;
    }
}

void HfpAgSystemEventProcesser::ProcessAtResponseStringEvent(const std::string &response) const
{
    RawAddress device(address_);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    HfpAgService *service = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "get service failed!");
    ::bluetooth::headset::Interface* bluetoothHfpInterface = service->getBluetoothHfpInterface();
    if (bluetoothHfpInterface == nullptr) {
        HILOGE("[HFP_EVENT_PROCESSER]BluetoothHfpInterface is null");
        return;
    }
    HILOGI("[HFP_EVENT_PROCESSER]Hfp device address[%{public}s]", GetEncryptAddr(address_).c_str());
    bt_status_t result = bluetoothHfpInterface->FormattedAtResponse(response.c_str(), &rawAddr);
    if (result != BT_STATUS_SUCCESS) {
        HILOGE("[HFP_EVENT_PROCESSER]Failed AtResponseString, status: %{public}d", result);
        return;
    }
}

void HfpAgSystemEventProcesser::NotifyDeviceStatusChangedEvent()
{
    RawAddress device(address_);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    HfpAgService *service = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "get service failed!");
    ::bluetooth::headset::Interface* bluetoothHfpInterface = service->getBluetoothHfpInterface();
    if (bluetoothHfpInterface == nullptr) {
        HILOGE("[HFP_EVENT_PROCESSER]BluetoothHfpInterface is null");
        return;
    }
    int serviceState = systemInterface_.GetServiceState();
    int signalStrength = systemInterface_.GetSignalStrength();
    int roamState = systemInterface_.GetRoamState();
    int batteryLevel = systemInterface_.GetBatteryLevel();
    HILOGI("Hfp device address: %{public}s, serviceState: %{public}d, signalStrength: %{public}d,"
        " roamState: %{public}d, batteryLevel: %{public}d",
        GET_ENCRYPT_STR_ADDR(address_), serviceState, signalStrength, roamState, batteryLevel);
    bt_status_t result = bluetoothHfpInterface->DeviceStatusNotification(
        static_cast<::bluetooth::headset::bthf_network_state_t>(serviceState),
        static_cast<::bluetooth::headset::bthf_service_type_t>(roamState), signalStrength, batteryLevel, &rawAddr);
    if (result != BT_STATUS_SUCCESS) {
        HILOGE("[HFP_EVENT_PROCESSER]Failed notify device state, status:%{public}d", result);
        return;
    }
}

void HfpAgSystemEventProcesser::ProfcessDialingOutResultEvent(int result)
{
    HfpAgResultType resultType = (result == HFP_AG_RESULT_OK) ? HFP_AG_RESULT_OK : HFP_AG_RESULT_ERROR;
    ProcessAtResponseCodeEvent(resultType, 0);
}

void HfpAgSystemEventProcesser::SendBSIRValueEvent(int action)
{
    bool value = (action == HFP_AG_INBAND_RING_DISABLE) ? false : true;
    RawAddress device(address_);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    HfpAgService *service = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "get service failed!");
    ::bluetooth::headset::Interface* bluetoothHfpInterface = service->getBluetoothHfpInterface();
    if (bluetoothHfpInterface == nullptr) {
        HILOGE("[HFP_EVENT_PROCESSER]BluetoothHfpInterface is null");
        return;
    }
    HILOGI("[HFP_EVENT_PROCESSER]Hfp device address[%{public}s]", GetEncryptAddr(address_).c_str());
    bt_status_t result = bluetoothHfpInterface->SendBsir(value, &rawAddr);
    if (result != BT_STATUS_SUCCESS) {
        HILOGE("[HFP_EVENT_PROCESSER]Failed send BSIR Value, status: %{public}d", result);
        return;
    }
}

void HfpAgSystemEventProcesser::ProcessScoVolumeChangedEvent(int volumeType, int volumeValue)
{
    HILOGI("[HFP_EVENT_PROCESSER]Hfp device address[%{public}s], volumeType:%{public}d, volumeValue:%{public}d,"
        "speakerVolume_:%{public}d", GET_ENCRYPT_STR_ADDR(address_), volumeType, volumeValue, speakerVolume_);
    if (speakerVolume_ == volumeValue) {
        return;
    }
    RawAddress device(address_);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    HfpAgService *service = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "get service failed!");
    ::bluetooth::headset::Interface* bluetoothHfpInterface = service->getBluetoothHfpInterface();
    if (bluetoothHfpInterface == nullptr) {
        HILOGE("[HFP_EVENT_PROCESSER]BluetoothHfpInterface is null");
        return;
    }
    
    std::string callingName = PermissionManager::GetCallingName();
    BtChrUeManager::GetInstance()->WriteVolChangeUe(::bluetooth::headset::BTHF_VOLUME_TYPE_SPK,
        volumeValue, callingName);
    PreferencesManager::Save(address_, volumeValue, PreferencesManagerType::HFP_VOLUME);
    bt_status_t result = bluetoothHfpInterface->VolumeControl(
        ::bluetooth::headset::BTHF_VOLUME_TYPE_SPK, volumeValue, &rawAddr);
    if (result != BT_STATUS_SUCCESS) {
        HILOGI("[HFP_EVENT_PROCESSER]Failed volume control, status: %{public}d", result);
        return;
    }
    speakerVolume_ = volumeValue;
}

void HfpAgSystemEventProcesser::ProcessScoVolumeChangedWithoutSendAt(int volumeType, int volumeValue)
{
    HfpAgService *service = HfpAgService::GetService();
    if (service != nullptr && service->GetActiveDevice() != address_) {
        HILOGE("This device %{public}s is not active!", GET_ENCRYPT_STR_ADDR(address_));
        return;
    }

    HILOGI("Hfp device address[%{public}s], volumeType:%{public}d, volumeValue:%{public}d, speakerVolume_:%{public}d",
        GET_ENCRYPT_STR_ADDR(address_), volumeType, volumeValue, speakerVolume_);
    if (speakerVolume_ == volumeValue) {
        return;
    }
    std::string callingName = PermissionManager::GetCallingName();
    BtChrUeManager::GetInstance()->WriteVolChangeUe(::bluetooth::headset::BTHF_VOLUME_TYPE_SPK,
        volumeValue, callingName);
}

void HfpAgSystemEventProcesser::RecoverScoVolume()
{
    ThreadUtil::GetInstance().PostTask(THREAD_ID_HFP,
            [this]() { this->SetScoVolume(); }, SET_SCO_VOLUME_DELAY_TIME, "SET_SCO_VOLUME");
}

void HfpAgSystemEventProcesser::SetScoVolume()
{
    int volumeValue = PreferencesManager::Get(address_, GetDefaultVolume(), PreferencesManagerType::HFP_VOLUME);
    systemInterface_.SetStreamVolume(STREAM_VOICE_CALL, volumeValue, 1);
    HILOGI("[HFP_EVENT_PROCESSER]Hfp device address[%{public}s], volumeValue:%{public}d, speakerVolume_:%{public}d",
        GET_ENCRYPT_STR_ADDR(address_), volumeValue, speakerVolume_);
    if (speakerVolume_ == volumeValue) {
        return;
    }

    HfpAgService *service = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "get service failed!");
    ::bluetooth::headset::Interface* bluetoothHfpInterface = service->getBluetoothHfpInterface();
    if (bluetoothHfpInterface == nullptr) {
        HILOGE("[HFP_EVENT_PROCESSER]BluetoothHfpInterface is null");
        return;
    }
    RawAddress device(address_);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    bt_status_t result = bluetoothHfpInterface->VolumeControl(::bluetooth::headset::BTHF_VOLUME_TYPE_SPK,
        volumeValue, &rawAddr);
    if (result != BT_STATUS_SUCCESS) {
        HILOGE("[HFP_EVENT_PROCESSER]Failed volume control, status: %{public}d", result);
        return;
    }
    speakerVolume_ = volumeValue;
}

void HfpAgSystemEventProcesser::WaitScoStateTimeOut()
{
    HfpAgService *service = HfpAgService::GetService();
    RawAddress device(address_);
    CHECK_AND_RETURN_LOG(service != nullptr, "[HFP_EVENT_PROCESSER]hfp service is null.");
    int scoState = service->GetScoState(device);
    if (scoState == HFP_AG_AUDIO_STATE_DISCONNECTED) {
        HILOGI("SCO State is disconnected, send BCC");
        service->NotifyHfpStackChanged(UpdateOutputStackAction::ACTION_USER_OPERATION, device);
        if (waitScoStateTimeOut_ != nullptr) {
            waitScoStateTimeOut_->Stop();
        }
        waitScoStateTimeOut_ = nullptr;
        return;
    }
    if (g_scoSateTimeOutCount > DISCONNECTED_SEND_BCC_RETRY_COUNT) {
        HILOGE("WaitScoState failed");
        service->NotifyHfpStackChanged(UpdateOutputStackAction::ACTION_USER_OPERATION, device);
        if (waitScoStateTimeOut_ != nullptr) {
            waitScoStateTimeOut_->Stop();
        }
        waitScoStateTimeOut_ = nullptr;
        return;
    }
    if (waitScoStateTimeOut_ != nullptr) {
        waitScoStateTimeOut_->Start(WAITE_SCO_STATE_CHANGE_TIME);
        g_scoSateTimeOutCount++;
    }
}

void HfpAgSystemEventProcesser::WaitScoState()
{
    waitScoStateTimeOut_ = std::make_shared<utility::Timer>([this]() { this->WaitScoStateTimeOut(); });
    waitScoStateTimeOut_->Start(WAITE_SCO_STATE_CHANGE_TIME);
}

void HfpAgSystemEventProcesser::ProcessBccEvent()
{
    HfpAgService *service = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "[HFP_EVENT_PROCESSER]hfp service is null.");

    if (service->IsBccBlockTimerActive() &&
        AudioStandard::AudioDevicesClientManager::GetInstance().GetActiveOutputDevice() ==
        AudioStandard::DeviceType::DEVICE_TYPE_NEARLINK) {
        HILOGI("Block BCC event within 3.5s after call connected");
        return;
    }
    RawAddress device(address_);
    if (service->GetActiveDevice() != address_) {
        // Don't create sco when the device support wear dection but not wearing
        bool isSupport = false;
        int32_t isEnabled = -1;
        BluetoothAudioManager::GetInstance().IsWearDetectionSupported(device, isSupport);
        BluetoothAudioManager::GetInstance().GetWearState(address_, isEnabled);
        bool isWearing = BluetoothAudioManager::GetInstance().IsDeviceWearing(device);
        HILOGI("DeviceAddr=%{public}s, isSupport=%{public}d, isEnabled=%{public}d, isWearing=%{public}d",
            GET_ENCRYPT_ADDR(device), isSupport, isEnabled, isWearing);
        if (isSupport && isEnabled == WEAR_DETECTION_ENABLED && !isWearing) {
            return;
        }
    }
    int scoState = service->GetScoState(device);
    if (scoState == HFP_AG_AUDIO_STATE_DISCONNECTED) {
        std::vector<std::shared_ptr<AudioStandard::AudioDeviceDescriptor>> currentAudioDevice = AudioStandard::
            AudioDevicesClientManager::GetInstance().GetDevices(AudioStandard::DeviceFlag::OUTPUT_DEVICES_FLAG);
        if (currentAudioDevice.empty()) {
            HILOGE("currentAudioDevice is empty!");
            return;
        }
        std::vector<std::shared_ptr<AudioStandard::AudioDeviceDescriptor>> audioDeviceDescriptor;
        for (const auto &desc : currentAudioDevice) {
            CHECK_AND_RETURN_LOG(desc != nullptr, "[HFP_EVENT_PROCESSER]desc is null.");
            if (desc->deviceType_ == AudioStandard::DeviceType::DEVICE_TYPE_BLUETOOTH_SCO) {
                desc->macAddress_ = address_;
                audioDeviceDescriptor.push_back(desc);
                break;
            }
        }
        sptr<AudioStandard::AudioRendererFilter> audioRendererFilter =
            sptr<AudioStandard::AudioRendererFilter>::MakeSptr();
        audioRendererFilter->rendererInfo.streamUsage = AudioStandard::StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION;
        AudioStandard::AudioDevicesClientManager::GetInstance().
            SelectOutputDevice(audioRendererFilter, audioDeviceDescriptor);
    } else {
        WaitScoState();
    }
}

void HfpAgSystemEventProcesser::ProcessBrsfEvent(int features)
{
    bool isSupported = (static_cast<uint32_t>(features) & HFP_AG_HF_FEATURES_REMOTE_VOLUME_CONTROL);
    bool result = SetHfpAgVgsSupport(address_, isSupported);
    HILOGI("addr: %{public}s, result:%{public}d, isSupported:%{public}d", GET_ENCRYPT_STR_ADDR(address_),
        result, isSupported);
}

void HfpAgSystemEventProcesser::UpdateDeviceNameAndTimestamp()
{
    if (deviceTimestamp_.empty()) {
        return;
    }

    RawAddress device(address_);
    std::string deviceName = RemoteDeviceProperties::GetInstance()->GetDeviceName(device);
    CHECK_AND_RETURN_LOG(!deviceName.empty(), "deviceName is empty.");

    std::stringstream ss;
    ss << SET_HW_DEVICE_FEATURE << ":" << MASK_OF_DEVICE_NAME_CHANGED;

    ss << VALUE_OF_DEVICE_INFO_NAME;
    ss << std::setw(LENGTH_OF_VALUE_LENGTH) << std::setfill('0') << deviceName.length();
    ss << deviceName;

    ss << VALUE_OF_DEVICE_INFO_TIMESTAMP;
    ss << std::setw(LENGTH_OF_VALUE_LENGTH) << std::setfill('0') << LENGTH_OF_DEVICE_INFO_TIMESTAMP;
    ss << deviceTimestamp_;

    ProcessAtResponseStringEvent(ss.str());
    HILOGI("addr: %{public}s update success", GET_ENCRYPT_STR_ADDR(address_));
    deviceTimestamp_ = "";
}

std::string HfpAgSystemEventProcesser::GetEventType(int type)
{
    switch (type) {
        case HFP_AG_MSG_TYPE_TYPE_NONE:
            return "HFP_AG_MSG_TYPE_TYPE_NONE";
        case HFP_AG_MSG_TYPE_ANSWER_CALL:
            return "HFP_AG_MSG_TYPE_ANSWER_CALL";
        case HFP_AG_MSG_TYPE_HANGUP_CALL:
            return "HFP_AG_MSG_TYPE_HANGUP_CALL";
        case HFP_AG_MSG_TYPE_VOLUME_CHANGED:
            return "HFP_AG_MSG_TYPE_VOLUME_CHANGED";
        case HFP_AG_MSG_TYPE_DIAL_CALL:
            return "HFP_AG_MSG_TYPE_DIAL_CALL";
        case HFP_AG_MSG_TYPE_SEND_DTMF:
            return "HFP_AG_MSG_TYPE_SEND_DTMF";
        case HFP_AG_MSG_TYPE_NOISE_REDUCTION:
            return "HFP_AG_MSG_TYPE_NOISE_REDUCTION";
        case HFP_AG_MSG_TYPE_AT_WBS:
            return "HFP_AG_MSG_TYPE_AT_WBS";
        case HFP_AG_MSG_TYPE_AT_CHLD:
            return "HFP_AG_MSG_TYPE_AT_CHLD";
        case HFP_AG_MSG_TYPE_SUBSCRIBER_NUMBER_REQUEST:
            return "HFP_AG_MSG_TYPE_SUBSCRIBER_NUMBER_REQUEST";
        default:
            return GetOtherEventType(type);
    }
}

std::string HfpAgSystemEventProcesser::GetOtherEventType(int type)
{
    switch (type) {
        case HFP_AG_MSG_TYPE_AT_CIND:
            return "HFP_AG_MSG_TYPE_AT_CIND";
        case HFP_AG_MSG_TYPE_AT_COPS:
            return "HFP_AG_MSG_TYPE_AT_COPS";
        case HFP_AG_MSG_TYPE_AT_CLCC:
            return "HFP_AG_MSG_TYPE_AT_CLCC";
        case HFP_AG_MSG_TYPE_AT_UNKNOWN:
            return "HFP_AG_MSG_TYPE_AT_UNKNOWN";
        case HFP_AG_MSG_TYPE_KEY_PRESSED:
            return "HFP_AG_MSG_TYPE_KEY_PRESSED";
        case HFP_AG_MSG_TYPE_AT_BIND:
            return "HFP_AG_MSG_TYPE_AT_BIND";
        case HFP_AG_MSG_TYPE_AT_BIEV:
            return "HFP_AG_MSG_TYPE_AT_BIEV";
        case HFP_AG_MSG_TYPE_AT_BIA:
            return "HFP_AG_MSG_TYPE_AT_BIA";
        case HFP_AG_MSG_TYPE_QUERY_AG_INDICATOR:
            return "HFP_AG_MSG_TYPE_QUERY_AG_INDICATOR";
        case HFP_AG_MSG_TYPE_AT_BCC:
            return "HFP_AG_MSG_TYPE_AT_BCC";
        case HFP_AG_MSG_TYPE_AT_BRSF:
            return "HFP_AG_MSG_TYPE_AT_BRSF";
        case HFP_AG_MSG_TYPE_VR_CHANGED:
            return "HFP_AG_MSG_TYPE_VR_CHANGED";
        case HFP_AG_MSG_TYPE_AT_VENDOR_SPECIFIC:
            return "HFP_AG_MSG_TYPE_AT_VENDOR_SPECIFIC";
        default:
            return "Unknown";
    }
}

void HfpAgSystemEventProcesser::AddEnableHwAtAddr(const std::string addr)
{
    enableHwAtAddrSet_.insert(addr);
}

bool HfpAgSystemEventProcesser::IsDeviceSupportHwAt(const std::string& addr)
{
    return enableHwAtAddrSet_.find(addr) != enableHwAtAddrSet_.end();
}

void HfpAgSystemEventProcesser::SendAtCmdOnApptypeChange(AppCategory topApptype)
{
    if (!IsDeviceSupportHwAt(address_)) {
        HILOGW("Not HW Device Block.");
        return;
    }
    HILOGI("try send At command topApptype=%{public}d to %{public}s", topApptype, GET_ENCRYPT_STR_ADDR(address_));

    switch (topApptype) {
        case APP_CATEGORY_MUSIC:
        default:
            HILOGD("Unsupported Audio Scene:%{public}d", topApptype);
            break;
    }
}

void HfpAgSystemEventProcesser::RemoveTask()
{
    ThreadUtil::GetInstance().RemoveTask(THREAD_ID_HFP, "SET_SCO_VOLUME");
}

}  // namespace bluetooth
}
