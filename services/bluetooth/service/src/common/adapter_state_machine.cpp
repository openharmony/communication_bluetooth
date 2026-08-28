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
#define LOG_TAG "bt_service_adapter_state"
#endif

#include "adapter_state_machine.h"

#include "adapter_manager.h"
#include "bluetooth_log.h"
#include "log.h"
#include "power_manager.h"
#include "profile_service_manager.h"
#include "bt_chr_dft_exception.h"
#include "bt_func_hook.h"
#include "thread_util.h"
#include "hitrace_meter.h"
#include "concurrent_task_client.h"
#include "bluetooth_hw_interface.h"
#include "bt_chr_switch.h"

namespace OHOS {
namespace bluetooth {

const int ENABLE_DISABLE_TIMEOUT_TIME = 70000;

void AdapterStateMachine::Init(utility::Context *context)
{
    std::unique_ptr<utility::StateMachine::State> turningOn = std::make_unique<AdapterTurningOnState>(*this, context);
    Move(turningOn);
    std::unique_ptr<utility::StateMachine::State> turnOn = std::make_unique<AdapterTurnOnState>(*this, context);
    Move(turnOn);
    std::unique_ptr<utility::StateMachine::State> turningOff = std::make_unique<AdapterTurningOffState>(*this, context);
    Move(turningOff);
    std::unique_ptr<utility::StateMachine::State> turnOff = std::make_unique<AdapterTurnOffState>(*this, context);
    Move(turnOff);
    InitState(TURN_OFF_STATE);
}

void AdapterTurnOffState::Entry()
{
    CHECK_AND_RETURN_LOG(adapterContext != nullptr, "Getcontext failed");
    BTTransport transport =
        (adapterContext->Name() == ADAPTER_NAME_CLASSIC) ? BTTransport::ADAPTER_BREDR : BTTransport::ADAPTER_BLE;
    AdapterManager::GetInstance()->OnAdapterStateChange(transport, BTStateID::STATE_TURN_OFF);
}

bool AdapterTurnOffState::Dispatch(const utility::Message &msg)
{
    switch (msg.what_) {
        case AdapterStateMachine::MSG_USER_ENABLE_REQ: {
            HILOGI("[ADAPTER_STATE_MACHINE]AdapterTurnOffState, MSG_USER_ENABLE_REQ");
            Transition(TURNING_ON_STATE);
            break;
        }
        default:
            return false;
    }
    return true;
}

AdapterTurningOnState::AdapterTurningOnState(AdapterStateMachine &stateMachine, utility::Context *context)
    : AdapterState(TURNING_ON_STATE, stateMachine, context)
{
    adapterTimer_ = std::make_unique<utility::Timer>([&]() -> void {
        HITRACE_METER_NAME(BT_TRACE_TAG, "AdapterTurningOnState::AdapterTurningOnState");
        DoInAdapterManagerThread(
            [&stateMachine] {stateMachine.ProcessMessage(AdapterStateMachine::MSG_ADAPTER_ENABLE_TIME_OUT);});
    });
    profileTimer_ = std::make_unique<utility::Timer>([&]() -> void {
        HITRACE_METER_NAME(BT_TRACE_TAG, "AdapterTurningOnState::AdapterTurningOnState");
        DoInAdapterManagerThread(
            [&stateMachine] {stateMachine.ProcessMessage(AdapterStateMachine::MSG_PROFILE_ENABLE_TIME_OUT);});
    });
}

void AdapterTurningOnState::Entry()
{
    CHECK_AND_RETURN_LOG(adapterContext != nullptr, "Getcontext failed");
    bool isClassicAdapter = (adapterContext->Name() == ADAPTER_NAME_CLASSIC);
    BTTransport transport = isClassicAdapter ? BTTransport::ADAPTER_BREDR : BTTransport::ADAPTER_BLE;
    AdapterManager::GetInstance()->OnAdapterStateChange(transport, BTStateID::STATE_TURNING_ON);
    adapterTimer_->Start(ENABLE_DISABLE_TIMEOUT_TIME, false);
    BTTransport otherTransport = isClassicAdapter ? BTTransport::ADAPTER_BLE : BTTransport::ADAPTER_BREDR;
    if (transport == BTTransport::ADAPTER_BLE) {
        BtChrWriteSwitchEvent(ChrSwitchEvent(adapterStateMachine_.callingName_, EVENT_TYPE_BT_ENABLE));
    }

    if (AdapterManager::GetInstance()->GetState(otherTransport) == BTStateID::STATE_TURN_OFF) {
        HILOGI("[ADAPTER_STATE_MACHINE]AdapterTurningOnState, enable bluetooth stack, transport = %{public}d",
            transport);
        BtInterface* bluetoothInterface = AdapterManager::GetInstance()->getBluetoothInterface();
        if (bluetoothInterface == nullptr) {
            HILOGE("bluetoothInterface is nullptr, abort enable");
            DoInAdapterManagerThread([this] { this->Transition(TURNING_OFF_STATE); });
            return;
        }
        const BthwifInterface *bthwif = reinterpret_cast<const BthwifInterface *>(
            bluetoothInterface->getProfileInterface(BT_VENDER_INTERFACE_ID));
        if (bthwif == nullptr) {
            HILOGW("Failed to get bthwif interface handle");
            DoInAdapterManagerThread([this] { this->Transition(TURNING_OFF_STATE); });
            return;
        }
        bool result = BluetoothHwInterface::GetInstance()->InitBtHwInterface(bthwif);
        if (!result) {
            HILOGE("Failed to init bthwif interface handle");
            DoInAdapterManagerThread([this] { this->Transition(TURNING_OFF_STATE); });
            return;
        }
#ifdef QOS_MANAGER_ENABLE
        std::unordered_map<std::string, std::string> payload;
        int32_t AUDIO_PROC_QOS_TABLE = 7;
        payload["groupId"] = std::to_string(AUDIO_PROC_QOS_TABLE);
	    payload["pid"] = std::to_string(getpid()); // 这里需要将pid的输入转化为string类型
	    OHOS::ConcurrentTask::ConcurrentTaskClient::GetInstance().RequestAuth(payload); // 向concurrent_task服务申请对自己进程鉴权
#endif
        int enableRet = bluetoothInterface->enable();
        if (enableRet != BT_STATUS_SUCCESS) {
            HILOGE("bluetoothInterface->enable failed: %{public}d", enableRet);
            DoInAdapterManagerThread([this] { this->Transition(TURNING_OFF_STATE); });
        }
    } else {
        HILOGI("[ADAPTER_STATE_MACHINE]AdapterTurningOnState, enable adapter, transport = %{public}d", transport);
        adapterContext->Enable();
    }
}

bool AdapterTurningOnState::DispatchOtherMsg(const utility::Message msg)
{
    CHECK_AND_RETURN_LOG_RET(adapterContext != nullptr, false, "Getcontext failed");
    BTTransport transport =
        (adapterContext->Name() == ADAPTER_NAME_CLASSIC) ? BTTransport::ADAPTER_BREDR : BTTransport::ADAPTER_BLE;
    int subCode = 0;
    switch (msg.what_) {
        case AdapterStateMachine::MSG_ADAPTER_ENABLE_TIME_OUT:
            HILOGI("[ADAPTER_STATE_MACHINE]AdapterTurningOnState, enable adapter timer out, transport is %{public}d",
                transport);
            AdapterManager::GetInstance()->Reset();
            DoInAdapterManagerThread([this] {this->Transition(TURNING_OFF_STATE);});
            subCode = (transport == BTTransport::ADAPTER_BLE) ? CHR_SUB_ERRCODE_CASE1 : CHR_SUB_ERRCODE_CASE2;
            BtChrBtExcpEvent("", BTOPT_SERVICE, subCode);
            return true;
        case AdapterStateMachine::MSG_PROFILE_ENABLE_TIME_OUT:
            HILOGI("[ADAPTER_STATE_MACHINE]AdapterTurningOnState, enable profile timer out, transport is %{public}d",
                transport);
            AdapterManager::GetInstance()->Reset();
            ProfileServiceManager::GetInstance()->OnAllEnabled(transport);
            DoInAdapterManagerThread([this] {this->Transition(TURNING_OFF_STATE);});
            return true;
        default:
            return false;
    }
    return true;
}

bool AdapterTurningOnState::Dispatch(const utility::Message &msg)
{
    CHECK_AND_RETURN_LOG_RET(adapterContext != nullptr, false, "Getcontext failed");
    BTTransport transport =
        (adapterContext->Name() == ADAPTER_NAME_CLASSIC) ? BTTransport::ADAPTER_BREDR : BTTransport::ADAPTER_BLE;
    switch (msg.what_) {
        case AdapterStateMachine::MSG_ADAPTER_ENABLE_CMP:
            HILOGI("[ADAPTER_STATE_MACHINE]AdapterTurningOnState, enable adapter cmp, transport is %{public}d",
                transport);
            adapterTimer_->Stop();
            if (msg.arg1_) {
                HILOGI("[ADAPTER_STATE_MACHINE]AdapterTurningOnState, start enable profile, transport is %{public}d",
                    transport);
                if (transport == BTTransport::ADAPTER_BREDR) {
                    IPowerManager::GetInstance()->Enable();
                }
                profileTimer_->Start(ENABLE_DISABLE_TIMEOUT_TIME, false);
                ProfileServiceManager::GetInstance()->Enable(transport);
            } else {
                Transition(TURNING_OFF_STATE);
            }
            break;
        case AdapterStateMachine::MSG_PROFILE_ENABLE_CMP:
            HILOGI("[ADAPTER_STATE_MACHINE]AdapterTurningOnState, enable profile cmp, transport is %{public}d",
                transport);
            profileTimer_->Stop();
            if (msg.arg1_) {
                Transition(TURN_ON_STATE);
            } else {
                ProfileServiceManager::GetInstance()->OnAllEnabled(transport);
                Transition(TURNING_OFF_STATE);
            }
            break;
        case AdapterStateMachine::MSG_STACK_ENABLE_CMP: {
            adapterContext->Enable();
            HILOGI("[ADAPTER_STATE_MACHINE]AdapterTurningOnState, stack enable cmp, transport is %{public}d",
                transport);
            break;
        }
        default:
            return DispatchOtherMsg(msg);
    }
    return true;
}

void AdapterTurnOnState::Entry()
{
    CHECK_AND_RETURN_LOG(adapterContext != nullptr, "Getcontext failed");
    BTTransport transport =
        (adapterContext->Name() == ADAPTER_NAME_CLASSIC) ? BTTransport::ADAPTER_BREDR : BTTransport::ADAPTER_BLE;
    AdapterManager::GetInstance()->OnAdapterStateChange(transport, BTStateID::STATE_TURN_ON);
    adapterContext->PostEnable();
}

bool AdapterTurnOnState::Dispatch(const utility::Message &msg)
{
    switch (msg.what_) {
        case AdapterStateMachine::MSG_USER_DISABLE_REQ:
            HILOGI("MSG_USER_DISABLE_REQ");
            Transition(TURNING_OFF_STATE);
            return true;
        default:
            return false;
    }
}

AdapterTurningOffState::AdapterTurningOffState(AdapterStateMachine &stateMachine, utility::Context *context)
    : AdapterState(TURNING_OFF_STATE, stateMachine, context)
{
    adapterTimer_ = std::make_unique<utility::Timer>([&]() -> void {
        HITRACE_METER_NAME(BT_TRACE_TAG, "AdapterTurningOffState::AdapterTurningOffState");
        DoInAdapterManagerThread(
            [&stateMachine] {stateMachine.ProcessMessage(AdapterStateMachine::MSG_ADAPTER_DISABLE_TIME_OUT);});
    });
    profileTimer_ = std::make_unique<utility::Timer>([&]() -> void {
        HITRACE_METER_NAME(BT_TRACE_TAG, "AdapterTurningOffState::AdapterTurningOffState");
        DoInAdapterManagerThread(
            [&stateMachine] {stateMachine.ProcessMessage(AdapterStateMachine::MSG_PROFILE_DISABLE_TIME_OUT);});
    });
}

void AdapterTurningOffState::Entry()
{
    CHECK_AND_RETURN_LOG(adapterContext != nullptr, "Getcontext failed");
    ThreadUtil::GetInstance().RemoveTask(
        THREAD_ID_ADAPTER_MANAGER, "BluetoothProfileAutoConnection");
    ThreadUtil::GetInstance().RemoveTask(THREAD_ID_CLASSIC, "BluetoothPassivePairedDelayConnect");
    BTTransport transport =
        (adapterContext->Name() == ADAPTER_NAME_CLASSIC) ? BTTransport::ADAPTER_BREDR : BTTransport::ADAPTER_BLE;
    AdapterManager::GetInstance()->OnAdapterStateChange(transport, BTStateID::STATE_TURNING_OFF);
    HILOGI("[ADAPTER_STATE_MACHINE]AdapterTurningOffState, start disable profile, transport is %{public}d",
        transport);
    profileTimer_->Start(ENABLE_DISABLE_TIMEOUT_TIME, false);
    ProfileServiceManager::GetInstance()->Disable(transport);
}

void AdapterTurningOffState::HandleDisableProfileCompleteEvent(bool isClassicAdapter)
{
    CHECK_AND_RETURN_LOG(adapterContext != nullptr, "Getcontext failed");
    HILOGI("[ADAPTER_STATE_MACHINE]AdapterTurningOffState, disable profile cmp, isClassicAdapter is %{public}d",
        isClassicAdapter);
    profileTimer_->Stop();
    adapterTimer_->Start(ENABLE_DISABLE_TIMEOUT_TIME, false);
    if (isClassicAdapter) {
        IPowerManager::GetInstance()->Disable();
    }
    adapterContext->Disable();
}

bool AdapterTurningOffState::HandleDisableAdapterCompleteEvent(bool isClassicAdapter)
{
    HILOGI("[ADAPTER_STATE_MACHINE]AdapterTurningOffState, disable adapter cmp, isClassicAdapter is %{public}d",
        isClassicAdapter);
    BTTransport otherTransport = isClassicAdapter ? BTTransport::ADAPTER_BLE : BTTransport::ADAPTER_BREDR;
    // 判断另一个是否成功关闭，是则关闭协议栈，否则将本adapter状态设置为关闭
    if (AdapterManager::GetInstance()->GetState(otherTransport) == BTStateID::STATE_TURN_OFF) {
        HILOGI("[ADAPTER_STATE_MACHINE]AdapterTurningOffState, disable stack, isClassicAdapter is %{public}d",
            isClassicAdapter);
        BtInterface* bluetoothInterface = AdapterManager::GetInstance()->getBluetoothInterface();
        if (bluetoothInterface == nullptr) {
            return false;
        }
        bluetoothInterface->disable();
    } else {
        adapterTimer_->Stop();
        Transition(TURN_OFF_STATE);
    }
    return true;
}

void AdapterTurningOffState::HandleDisableStackCompleteEvent(bool isClassicAdapter)
{
    HILOGI("[ADAPTER_STATE_MACHINE]AdapterTurningOffState, disable stack cmp, isClassicAdapter is %{public}d",
        isClassicAdapter);
    adapterTimer_->Stop();
    Transition(TURN_OFF_STATE);
}

void AdapterTurningOffState::HandleDisableProfileTimeOutEvent(bool isClassicAdapter)
{
    CHECK_AND_RETURN_LOG(adapterContext != nullptr, "Getcontext failed");
    BTTransport transport = isClassicAdapter ? BTTransport::ADAPTER_BREDR : BTTransport::ADAPTER_BLE;
    ProfileServiceManager::GetInstance()->OnAllDisabled(transport);
    adapterTimer_->Start(ENABLE_DISABLE_TIMEOUT_TIME, false);
    adapterContext->Disable();
    HILOGI("[ADAPTER_STATE_MACHINE]AdapterTurningOffState, disable profile timeout, isClassicAdapter is %{public}d",
        isClassicAdapter);
    AdapterManager::GetInstance()->Reset();
}

bool AdapterTurningOffState::HandleDisableAdapterTimeOutEvent(bool isClassicAdapter)
{
    HILOGI("[ADAPTER_STATE_MACHINE]AdapterTurningOffState, disable adapter timeout, isClassicAdapter is %{public}d",
        isClassicAdapter);
    int subCode = (!isClassicAdapter) ? CHR_SUB_ERRCODE_CASE3 : CHR_SUB_ERRCODE_CASE4;
    BtChrBtExcpEvent("", BTOPT_SERVICE, subCode);
    // 关闭adapter超时按成功关闭处理
    return HandleDisableAdapterCompleteEvent(isClassicAdapter);
}

bool AdapterTurningOffState::Dispatch(const utility::Message &msg)
{
    CHECK_AND_RETURN_LOG_RET(adapterContext != nullptr, false, "Getcontext failed");
    bool isClassicAdapter = (adapterContext->Name() == ADAPTER_NAME_CLASSIC);
    switch (msg.what_) {
        case AdapterStateMachine::MSG_PROFILE_DISABLE_CMP: {
            HandleDisableProfileCompleteEvent(isClassicAdapter);
            break;
        }
        case AdapterStateMachine::MSG_ADAPTER_DISABLE_CMP: {
            return HandleDisableAdapterCompleteEvent(isClassicAdapter);
        }
        case AdapterStateMachine::MSG_STACK_DISABLE_CMP: {
            HandleDisableStackCompleteEvent(isClassicAdapter);
            break;
        }
        case AdapterStateMachine::MSG_PROFILE_DISABLE_TIME_OUT: {
            HandleDisableProfileTimeOutEvent(isClassicAdapter);
            break;
        }
        case AdapterStateMachine::MSG_ADAPTER_DISABLE_TIME_OUT: {
            return HandleDisableAdapterTimeOutEvent(isClassicAdapter);
        }
        default:
            return false;
    }
    return true;
}
}  // namespace bluetooth
}  // namespace OHOS
