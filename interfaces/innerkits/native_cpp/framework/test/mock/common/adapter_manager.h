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

#ifndef ADAPTER_MANAGER_H
#define ADAPTER_MANAGER_H

#include <memory>
#include <string>
#include <vector>
#include "bluetooth_def.h"
#include "interface_adapter.h"
#include "interface_adapter_manager.h"
#include "context.h"

namespace bluetooth {
/**
 * @brief Represents adapter manager.
 *
 * @since 6
 */
const static int CLASSIC_ENABLE_STATE_BIT = 0x04;
class AdapterManager : public IAdapterManager {
public:
   
    static AdapterManager *GetInstance();
    IAdapter *GetAdapter(const BTTransport transport) const override;
    bool Start() override{return true;}
    void Stop() const override{}
    void Reset() const override{}
    bool FactoryReset() const override{return true;}
    bool Enable(const BTTransport transport) const override {return true;}
    bool Disable(const BTTransport transport) const override {return true;}
    BTStateID GetState(const BTTransport transport) const override {return BTStateID::STATE_TURN_ON;}
    BTConnectState GetAdapterConnectState() const override {return BTConnectState::CONNECTED;}
    bool RegisterStateObserver(IAdapterStateObserver &observer) const override {return true;}
    bool DeregisterStateObserver(IAdapterStateObserver &observer) const override{return true;}
    bool RegisterSystemStateObserver(ISystemStateObserver &observer) const override{return true;}
    bool DeregisterSystemStateObserver(ISystemStateObserver &observer) const override{return true;}
    int GetMaxNumConnectedAudioDevices() const override{return 0;}
    bool SetPhonebookPermission(const std::string &address, BTPermissionType permission) const override{return true;}
    BTPermissionType GetPhonebookPermission(const std::string &address) const override{return BTPermissionType::ACCESS_ALLOWED;}
    bool SetMessagePermission(const std::string &address, BTPermissionType permission) const override{return true;}
    BTPermissionType GetMessagePermission(const std::string &address) const override{return BTPermissionType::ACCESS_ALLOWED;}
    int GetPowerMode(const std::string &address) const override{return 0;}
    bool AdapterStop() const{return true;}
    bool ClearAllStorage() const{return true;}
    void OnSysStateChange(const std::string &state) const{}
    void OnSysStateExit(const std::string &state) const{}
    void OnAdapterStateChange(const BTTransport transport, const BTStateID state) const{}
    void OnProfileServicesEnableComplete(const BTTransport transport, const bool ret) const{}
    void OnProfileServicesDisableComplete(const BTTransport transport, const bool ret) const{}
    void OnPairDevicesRemoved(const BTTransport transport, const std::vector<RawAddress> &devices) const{}

private:
    AdapterManager();
    ~AdapterManager();

    void CreateAdapters() const;
    std::string GetSysState() const;
    bool OutputSetting() const;
    void RegisterHciResetCallback();
    void DeregisterHciResetCallback() const;
    void RemoveDeviceProfileConfig(const BTTransport transport, const std::vector<RawAddress> &devices) const;

    static void HciFailedReset(void *context);
};
};  // namespace bluetooth

#endif  // ADAPTER_MANAGER_H