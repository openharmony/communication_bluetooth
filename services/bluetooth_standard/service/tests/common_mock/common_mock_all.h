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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "adapter_config_mock.h"
#include "adapter_device_config_mock.h"
#include "adapter_device_info_mock.h"
#include "adapter_manager.h"
#include "profile_config_mock.h"
#include "profile_service_manager_mock.h"
#include "adapter_manager.h"
#include "power_manager_mock.h"
#include "compat.h"

using namespace bluetooth;

class AdapterConfigInstanceMock : public AdapterConfigInstance {
public:
    MOCK_METHOD0(GetInstance, IAdapterConfig *());
};

class AdapterConfigMock : public IAdapterConfig {
public:
    AdapterConfigMock(){};
    ~AdapterConfigMock(){};
    MOCK_METHOD0(Load, bool());
    MOCK_METHOD0(Reload, bool());
    MOCK_METHOD3(GetValue, bool(const std::string &section, const std::string &property, int &value));
    MOCK_METHOD3(GetValue, bool(const std::string &section, const std::string &property, std::string &value));
    MOCK_METHOD3(GetValue, bool(const std::string &section, const std::string &property, bool &value));
    MOCK_METHOD1(HasSection, bool(const std::string &section));
};

class AdapterDeviceConfigInstanceMock : public AdapterDeviceConfigInstance {
public:
    MOCK_METHOD0(GetInstance, IAdapterDeviceConfig *());
};

class AdapterDeviceConfigMock : public IAdapterDeviceConfig {
public:
    AdapterDeviceConfigMock(){};
    ~AdapterDeviceConfigMock(){};
    MOCK_METHOD0(Load, bool());
    MOCK_METHOD0(Reload, bool());
    MOCK_METHOD0(Save, bool());
    MOCK_METHOD4(GetValue,
        bool(const std::string &section, const std::string &subSection, const std::string &property, int &value));
    MOCK_METHOD4(GetValue, bool(const std::string &section, const std::string &subSection, const std::string &property,
                               std::string &value));
    MOCK_METHOD4(GetValue,
        bool(const std::string &section, const std::string &subSection, const std::string &property, bool &value));
    MOCK_METHOD4(SetValue,
        bool(const std::string &section, const std::string &subSection, const std::string &property, const int &value));
    MOCK_METHOD4(SetValue, bool(const std::string &section, const std::string &subSection, const std::string &property,
                               const std::string &value));
    MOCK_METHOD4(SetValue, bool(const std::string &section, const std::string &subSection, const std::string &property,
                               const bool &value));
    MOCK_METHOD2(GetSubSections, bool(const std::string &section, std::vector<std::string> &subSections));
    MOCK_METHOD2(RemoveSection, bool(const std::string &section, const std::string &subSection));
    MOCK_METHOD3(GetValue, bool(const std::string &section, const std::string &property, int &value));
    MOCK_METHOD3(GetValue, bool(const std::string &section, const std::string &property, std::string &value));
    MOCK_METHOD3(GetValue, bool(const std::string &section, const std::string &property, bool &value));
    MOCK_METHOD3(SetValue, bool(const std::string &section, const std::string &property, const int &value));
    MOCK_METHOD3(SetValue, bool(const std::string &section, const std::string &property, const std::string &value));
};

class AdapterDeviceInfoInstanceMock : public AdapterDeviceInfoInstance {
public:
    MOCK_METHOD0(GetInstance, IAdapterDeviceInfo *());
};

class AdapterDeviceInfoMock : public IAdapterDeviceInfo {
public:
    AdapterDeviceInfoMock(){};
    ~AdapterDeviceInfoMock(){};
    MOCK_METHOD0(Load, bool());
    MOCK_METHOD0(Reload, bool());
    MOCK_METHOD3(GetValue, bool(const std::string &section, const std::string &property, std::string &value));
    MOCK_METHOD3(GetValue, bool(const std::string &section, const std::string &property, int &value));
    MOCK_METHOD3(GetValue, bool(const std::string &section, const std::string &property, bool &value));
};

class ProfileConfigInstanceMock : public ProfileConfigInstance {
public:
    MOCK_METHOD0(GetInstance, IProfileConfig *());
};

class ProfileConfigMock : public IProfileConfig {
public:
    ProfileConfigMock(){};
    ~ProfileConfigMock(){};
    MOCK_METHOD0(Load, bool());
    MOCK_METHOD0(Reload, bool());
    MOCK_METHOD4(
        GetValue, bool(const std::string &addr, const std::string &section, const std::string &property, int &value));
    MOCK_METHOD4(
        GetValue, bool(const std::string &addr, const std::string &section, const std::string &property, bool &value));
    MOCK_METHOD4(
        SetValue, bool(const std::string &addr, const std::string &section, const std::string &property, int &value));
    MOCK_METHOD4(
        SetValue, bool(const std::string &addr, const std::string &section, const std::string &property, bool &value));
    MOCK_METHOD3(
        RemoveProperty, bool(const std::string &addr, const std::string &section, const std::string &property));
    MOCK_METHOD1(HasAddr, bool(const std::string &addr));
    MOCK_METHOD2(HasSection, bool(const std::string &addr, const std::string &section));
    MOCK_METHOD2(GetSections, bool(const std::string &addr, std::vector<std::string> &sections));
    MOCK_METHOD1(RemoveAddr, bool(const std::string &addr));
};

class AdapterManagerMock {
public:
    MOCK_METHOD0(AdapterManager_GetState, bluetooth::BTStateID());
    MOCK_METHOD0(GetAdapter, bluetooth::IAdapter *());
};

class ProfileServiceManagerInstanceMock : public ProfileServiceManagerInstance {
public:
    MOCK_METHOD0(GetInstance, IProfileManager *());
};

class ProfileServiceManagerMock : public IProfileManager {
public:
    ProfileServiceManagerMock(){};
    ~ProfileServiceManagerMock(){};
    MOCK_CONST_METHOD1(GetProfileService, IProfile *(const std::string &name));
    MOCK_CONST_METHOD0(GetProfileServicesList, std::vector<uint32_t>());
    MOCK_CONST_METHOD1(GetProfileServiceConnectState, BTConnectState(const uint32_t profileID));
    MOCK_CONST_METHOD1(GetProfileServicesSupportedUuids, void(std::vector<std::string> &uuids));
};

class PowerManagerInstanceMock : public PowerManagerInstance {
public:
    MOCK_METHOD0(GetInstance, IPowerManager &());
};

class PowerManagerMock : public IPowerManager {
public:
    PowerManagerMock(){};
    ~PowerManagerMock(){};
    MOCK_METHOD3(StatusUpdate, void(const RequestStatus status, const std::string &profileName, const RawAddress &addr));
	MOCK_CONST_METHOD1(GetPowerMode, BTPowerMode(const RawAddress &addr));
};

void registerNewMockAdapterManager(AdapterManagerMock *newMock);
AdapterManagerMock& GetAdapterManagerMock();