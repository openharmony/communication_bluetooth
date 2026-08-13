/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_state_manager"
#endif

#include "bluetooth_state_manager.h"

#include "log.h"
#include "common_util.h"
#include "hfp_ag_defines.h"
#include "hfp_ag_statemachine.h"
#include "parameters.h"
#include "string_ex.h"

namespace OHOS {
namespace bluetooth {

BluetoothStateManager::BluetoothStateManager()
{}

BluetoothStateManager::~BluetoothStateManager()
{}

BluetoothStateManager *BluetoothStateManager::GetInstance()
{
    static BluetoothStateManager instance;
    return &instance;
}

void BluetoothStateManager::SetRemoteDeviceHfpAgState(const std::string &address, int state)
{
    int rawState = HFP_AG_STATE_INVAILD;
    hfpAgStatemachineState_.Find(address, rawState);
    HILOGI("SetRemoteDeviceHfpAgState Device[%{public}s] transition [%{public}d]->[%{public}d]",
        GET_ENCRYPT_STR_ADDR(address), rawState, state);
    hfpAgStatemachineState_.EnsureInsert(address, state);
}

void BluetoothStateManager::EraseRemoteDeviceHfpAgState(const std::string &address)
{
    HILOGI("EraseRemoteDeviceHfpAgState Device[%{public}s]",
        GET_ENCRYPT_STR_ADDR(address));
    hfpAgStatemachineState_.Erase(address);
}

bool BluetoothStateManager::FindRemoteDeviceHfpAgStateByAddr(const std::string &address, int &state)
{
    bool result = hfpAgStatemachineState_.Find(address, state);
    HILOGD("FindRemoteDeviceHfpAgStateByAddr Device[%{public}s] state [%{public}d] result %{public}d",
        GET_ENCRYPT_STR_ADDR(address), state, result);
    return result;
}

int BluetoothStateManager::ConvertHfpAgState(const std::string &state)
{
    if (state == HfpAgStateMachine::DISCONNECTED) {
        return HFP_AG_STATE_DISCONNECTED;
    } else if (state == HfpAgStateMachine::DISCONNECTING) {
        return HFP_AG_STATE_DISCONNECTING;
    } else if (state == HfpAgStateMachine::CONNECTING) {
        return HFP_AG_STATE_CONNECTING;
    } else if (state == HfpAgStateMachine::CONNECTED) {
        return HFP_AG_STATE_CONNECTED;
    } else if (state == HfpAgStateMachine::AUDIO_CONNECTING) {
        return HFP_AG_AUDIO_STATE_CONNECTING;
    } else if (state == HfpAgStateMachine::AUDIO_DISCONNECTING) {
        return HFP_AG_AUDIO_STATE_DISCONNECTING;
    } else if (state == HfpAgStateMachine::AUDIO_CONNECTED) {
        return HFP_AG_AUDIO_STATE_CONNECTED;
    }
    return HFP_AG_STATE_DISCONNECTED;
}

void BluetoothStateManager::SetRemoteDevicePanState(const std::string &address, int state)
{
    int rawState = -1;
    panStatemachineState_.Find(address, rawState);
    HILOGI("SetRemoteDevicePanState Device[%{public}s] transition [%{public}d]->[%{public}d]",
        GET_ENCRYPT_STR_ADDR(address), rawState, state);
    panStatemachineState_.EnsureInsert(address, state);
}

void BluetoothStateManager::EraseRemoteDevicePanState(const std::string &address)
{
    HILOGI("EraseRemoteDevicePanState Device[%{public}s]",
        GET_ENCRYPT_STR_ADDR(address));
    panStatemachineState_.Erase(address);
}

bool BluetoothStateManager::FindRemoteDevicePanStateByAddr(const std::string &address, int &state)
{
    bool result = panStatemachineState_.Find(address, state);
    HILOGD("FindRemoteDevicePanStateByAddr Device[%{public}s] state [%{public}d] result %{public}d",
        GET_ENCRYPT_STR_ADDR(address), state, result);
    return result;
}

void BluetoothStateManager::AddDeviceProfileConnectState(
    const std::string &profileId, const std::string &address, BTConnectState state)
{
    deviceConnectStateMap_.ChangeValueByLambda(UpperStr(address),
        [&profileId, &state](DeviceConnectState &deviceConnectState) {
        deviceConnectState.profileConnectStateMap.EnsureInsert(profileId, state);
    });
}

void BluetoothStateManager::RemoveDeviceConnectState(const std::string &address)
{
    deviceConnectStateMap_.Erase(UpperStr(address));
}

void BluetoothStateManager::RemoveDeviceProfileConnectState(const std::string &profileId, const std::string &address)
{
    deviceConnectStateMap_.ChangeValueByLambda(UpperStr(address), [&profileId](DeviceConnectState &deviceConnectState) {
        deviceConnectState.profileConnectStateMap.Erase(profileId);
    });
}

BTConnectState BluetoothStateManager::GetDeviceProfileConnectState(
    const std::string &profileId, const std::string &address)
{
    BTConnectState state = BTConnectState::DISCONNECTED;
    std::string addressUpper = UpperStr(address);
    deviceConnectStateMap_.Iterate([&profileId, &addressUpper, &state]
        (const std::string &addressStored, DeviceConnectState &deviceConnectStateStored) {
        if (addressStored == addressUpper) {
            deviceConnectStateStored.profileConnectStateMap.Find(profileId, state);
        }
    });
    return state;
}

BTConnectState BluetoothStateManager::GetProfileConnectState(const std::string &profileId)
{
    BTConnectState ret = BTConnectState::DISCONNECTED;
    deviceConnectStateMap_.Iterate([&profileId, &ret]
        (const std::string &addressStored, DeviceConnectState &deviceConnectStateStored) {
        BTConnectState state = BTConnectState::DISCONNECTED;
        deviceConnectStateStored.profileConnectStateMap.Find(profileId, state);
        if (state == BTConnectState::CONNECTED) {
            ret = BTConnectState::CONNECTED;
        }
    });
    return ret;
}

std::vector<std::string> BluetoothStateManager::GetClientDeviceList()
{
    return clientDeviceList_;
}

std::vector<std::string> BluetoothStateManager::GetServerDeviceList()
{
    return serverDeviceList_;
}

void BluetoothStateManager::AddClientDeviceList(std::string addr)
{
    std::lock_guard<std::mutex> clientLock(clientDeviceListMutex_);
    if (clientDeviceCountMap_.count(addr) == 0) {
        clientDeviceCountMap_.insert(std::make_pair(addr, 1));
        auto iter = std::find_if(clientDeviceList_.begin(), clientDeviceList_.end(),
            [&addr](auto &deviceAddr) { return deviceAddr == addr; });
        if (iter == clientDeviceList_.end()) {
            HILOGI("add devices, addr: %{public}s", GetEncryptAddr(addr).c_str());
            clientDeviceList_.emplace_back(addr);
        }
    } else {
        clientDeviceCountMap_[addr]++;
    }
}

void BluetoothStateManager::AddServerDeviceList(std::string addr)
{
    std::lock_guard<std::mutex> serverLock(serverDeviceListMutex_);
    if (serverDeviceCountMap_.count(addr) == 0) {
        serverDeviceCountMap_.insert(std::make_pair(addr, 1));
        auto iter = std::find_if(serverDeviceList_.begin(), serverDeviceList_.end(),
            [&addr](auto &deviceAddr) { return deviceAddr == addr; });
        if (iter == serverDeviceList_.end()) {
            HILOGI("add devices, addr: %{public}s", GetEncryptAddr(addr).c_str());
            serverDeviceList_.emplace_back(addr);
        }
    } else {
        serverDeviceCountMap_[addr]++;
    }
}

void BluetoothStateManager::RemoveClientDeviceList(std::string addr)
{
    std::lock_guard<std::mutex> clientLock(clientDeviceListMutex_);
    if (clientDeviceCountMap_.count(addr) != 0) {
        if (clientDeviceCountMap_[addr] <= 1) {
            HILOGI("remove devices, addr: %{public}s", GetEncryptAddr(addr).c_str());
            clientDeviceCountMap_.erase(addr);
            clientDeviceList_.erase(std::remove_if(clientDeviceList_.begin(), clientDeviceList_.end(),
                [&addr](auto &deviceAddr) { return deviceAddr == addr; }), clientDeviceList_.end());
        } else {
            clientDeviceCountMap_[addr]--;
        }
    } else {
        HILOGE("addr: %{public}s, not exist.", GetEncryptAddr(addr).c_str());
    }
}

void BluetoothStateManager::RemoveServerDeviceList(std::string addr)
{
    std::lock_guard<std::mutex> serverLock(serverDeviceListMutex_);
    if (serverDeviceCountMap_.count(addr) != 0) {
        if (serverDeviceCountMap_[addr] <= 1) {
            HILOGI("remove devices, addr: %{public}s", GetEncryptAddr(addr).c_str());
            serverDeviceCountMap_.erase(addr);
            serverDeviceList_.erase(std::remove_if(serverDeviceList_.begin(), serverDeviceList_.end(),
                [&addr](auto &deviceAddr) { return deviceAddr == addr; }), serverDeviceList_.end());
        } else {
            serverDeviceCountMap_[addr]--;
        }
    } else {
        HILOGE("addr: %{public}s, not exist.", GetEncryptAddr(addr).c_str());
    }
}
} //namespace bluetooth
} //namespace OHOS
