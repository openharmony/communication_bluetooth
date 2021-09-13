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

#include "adapter_manager.h"
#include "common_mock_all.h"
#include "log.h"
#include "power_manager.h"

using namespace bluetooth;
AdapterManagerMock *g_amMock_;

void registerNewMockAdapterManager(AdapterManagerMock *newMock)
{
    g_amMock_ = newMock;
}

AdapterManagerMock& GetAdapterManagerMock()
{
    return *g_amMock_;
}

IAdapterManager *IAdapterManager::GetInstance()
{
    return AdapterManager::GetInstance();
}

struct AdapterManager::impl {
    std::map<std::string, BTPermissionType> phoneBookPermissionMap_;
    std::map<std::string, BTPermissionType> mapPermissionMap_;
};

AdapterManager::AdapterManager()
{
    pimpl = std::make_unique<AdapterManager::impl>();
}

AdapterManager::~AdapterManager()
{}

AdapterManager *AdapterManager::GetInstance()
{
    static AdapterManager instance;

    return &instance;
}

IAdapter *AdapterManager::GetAdapter(const BTTransport transport) const
{
    return g_amMock_->GetAdapter();
}

bool AdapterManager::Start()
{
    LOG_DEBUG("%s start", __PRETTY_FUNCTION__);
    return true;
}

void AdapterManager::Stop() const
{
    LOG_DEBUG("%s start", __PRETTY_FUNCTION__);
}

void AdapterManager::Reset() const
{
    LOG_DEBUG("%s start", __PRETTY_FUNCTION__);
}

bool AdapterManager::FactoryReset() const
{
    LOG_DEBUG("%s start", __PRETTY_FUNCTION__);
    return true;
}

bool AdapterManager::Enable(const BTTransport transport) const
{
    LOG_DEBUG("%s start transport is %d", __PRETTY_FUNCTION__, transport);
    return true;
}

bool AdapterManager::Disable(const BTTransport transport) const
{
    LOG_DEBUG("%s start transport is %d", __PRETTY_FUNCTION__, transport);
    return true;
}

BTStateID AdapterManager::GetState(const BTTransport transport) const
{
    return g_amMock_->AdapterManager_GetState();
}

BTConnectState AdapterManager::GetAdapterConnectState() const
{
    LOG_DEBUG("%s start", __PRETTY_FUNCTION__);
    return BTConnectState::CONNECTED;
}

bool AdapterManager::RegisterStateObserver(IAdapterStateObserver &observer) const
{
    return true;
}

bool AdapterManager::DeregisterStateObserver(IAdapterStateObserver &observer) const
{
    return true;
}

bool AdapterManager::RegisterSystemStateObserver(ISystemStateObserver &observer) const
{
    return true;
}

bool AdapterManager::DeregisterSystemStateObserver(ISystemStateObserver &observer) const
{
    return true;
}

int AdapterManager::GetMaxNumConnectedAudioDevices() const
{
    LOG_DEBUG("%s start", __PRETTY_FUNCTION__);
    return 1;
}

bool AdapterManager::SetPhonebookPermission(const std::string &address, BTPermissionType permission) const
{
    LOG_DEBUG("%s start", __PRETTY_FUNCTION__);
    pimpl->phoneBookPermissionMap_[address] = permission;
    return true;
}

BTPermissionType AdapterManager::GetPhonebookPermission(const std::string &address) const
{
    LOG_DEBUG("%s start", __PRETTY_FUNCTION__);
    auto target = pimpl->phoneBookPermissionMap_.find(address);
    if (target != pimpl->phoneBookPermissionMap_.end()) {
        return target->second;
    }
    return BTPermissionType::ACCESS_ALLOWED;  // default allowed
}

bool AdapterManager::SetMessagePermission(const std::string &address, BTPermissionType permission) const
{
    LOG_DEBUG("%s start", __PRETTY_FUNCTION__);
    pimpl->mapPermissionMap_[address] = permission;
    return true;
}

BTPermissionType AdapterManager::GetMessagePermission(const std::string &address) const
{
    LOG_DEBUG("%s start", __PRETTY_FUNCTION__);
    auto iter = pimpl->mapPermissionMap_.find(address);
    if (iter != pimpl->mapPermissionMap_.end()) {
        return iter->second;
    }
    return BTPermissionType::ACCESS_UNKNOWN;
}
int AdapterManager::GetPowerMode(const std::string &address) const
{
    return static_cast<int>(BTPowerMode::MODE_INVALID);
}
void AdapterManager::OnPairDevicesRemoved(const BTTransport transport, const std::vector<RawAddress> &devices)
{}