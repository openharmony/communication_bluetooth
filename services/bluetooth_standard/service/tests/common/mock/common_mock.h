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
#include "adapter_config.h"
#include "adapter_config_mock.h"
#include "profile_config.h"
#include "profile_config_mock.h"
#include "adapter_device_config.h"
#include "adapter_device_config_mock.h"
#include "adapter_device_info.h"
#include "adapter_device_info_mock.h"
#include "adapter_manager.h"
#include "profile_service_manager.h"
#include "power_manager.h"
#include "log.h"
#include "btm.h"

using namespace bluetooth;

#define TEST_TIMER_NORMAL (50000)  // us
#define TEST_TIMER_TIMEOUT (71)  // s
#define TEST_POWER_DELAY_4 (4)  // s
#define TEST_POWER_DELAY_8 (8)  // s

typedef struct {
    const BtmPmCallbacks *callbacks;
    const BtmAclCallbacks *aclcallbacks;
    void *context;
} BtmPmCallbackTest;

class TestObserver : public IAdapterStateObserver {
public:
    TestObserver(){};
    ~TestObserver() = default;
    void OnStateChange(const BTTransport transport, const BTStateID state) override
    {
        LOG_DEBUG("%s transport is %d state is %d", __PRETTY_FUNCTION__, transport, state);
        state_[transport] = (int)state;
    };

    void WaitStateChange(int transport, int state)
    {
        while (true) {
            if (state_[transport] == state) {
                break;
            }
        }
    };
private:
    int state_[2] = {-1, -1};
};

class TestSysObserver : public ISystemStateObserver {
public:
    TestSysObserver(){};
    ~TestSysObserver() = default;
    void OnSystemStateChange(const BTSystemState state) override
    {
        LOG_DEBUG("%s sysstate is %d", __PRETTY_FUNCTION__, state);
        sysState_ = (int)state;
    };

    void WaitSysStateChange(int state)
    {
        while (true) {
            if (sysState_ == state) {
                break;
            }
        }
    };
private:
    int sysState_ = -1;
};

class AdapterConfigInstanceMock : public AdapterConfigInstance {
public:
    MOCK_METHOD0(GetInstance, IAdapterConfig *());
};

class AdapterConfigMock : public IAdapterConfig {
public:
    AdapterConfigMock(){};
    ~AdapterConfigMock(){};
    MOCK_METHOD0(Load, bool());
    bool Reload() override
    {
        return true;
    }
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
    bool Reload() override
    {
        return true;
    }
    MOCK_METHOD0(Save, bool());
    MOCK_METHOD4(GetValue, bool(const std::string &section, const std::string &subSection, const std::string &property, int &value));
    MOCK_METHOD4(GetValue, bool(const std::string &section, const std::string &subSection, const std::string &property, std::string &value));
    MOCK_METHOD4(GetValue, bool(const std::string &section, const std::string &subSection, const std::string &property, bool &value));
    MOCK_METHOD4(GetValue, bool(const std::string &section, const std::string &subSection, const std::string &property, const int &value));
    MOCK_METHOD4(SetValue, bool(const std::string &section, const std::string &subSection, const std::string &property, const bool &value));
    MOCK_METHOD4(SetValue, bool(const std::string &section, const std::string &subSection, const std::string &property, const std::string &value));
    MOCK_METHOD4(SetValue, bool(const std::string &section, const std::string &subSection, const std::string &property, const int &value));
    MOCK_METHOD2(GetSubSections, bool(const std::string &section, std::vector<std::string> &subSections));
    MOCK_METHOD2(RemoveSection, bool(const std::string &section, const std::string &subSection));
    MOCK_METHOD3(GetValue, bool(const std::string &section, const std::string &property, int &value));
    MOCK_METHOD3(GetValue, bool(const std::string &section, const std::string &property, std::string &value));
    MOCK_METHOD3(GetValue, bool(const std::string &section, const std::string &property, bool &value));
    MOCK_METHOD3(SetValue, bool(const std::string &section, const std::string &property, const int &value));
    MOCK_METHOD3(SetValue, bool(const std::string &section, const std::string &property, const std::string &value));
    MOCK_METHOD1(HasSection, bool(const std::string &section));
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
    bool Reload() override
    {
        return true;
    }
    MOCK_METHOD3(GetValue, bool(const std::string &section, const std::string &property, int &value));
    MOCK_METHOD3(GetValue, bool(const std::string &section, const std::string &property, std::string &value));
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
    bool Reload() override
    {
        return true;
    }
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

class Mock_BTM {
public:
    MOCK_METHOD0(BTM_Initialize, int());
    MOCK_METHOD0(BTM_Close, int());
    MOCK_METHOD1(BTM_Enable, int(int controller));
    MOCK_METHOD1(BTM_Disable, int(int controller));
    MOCK_METHOD2(BTM_RegisterCallbacks, int(const BtmCallbacks *callbacks, void *context));
    MOCK_METHOD1(BTM_DeregisterCallbacks, int(const BtmCallbacks *callbacks));
    MOCK_METHOD0(BTM_SetSnoopFilePath, int());
    MOCK_METHOD0(BTM_EnableSnoopFileOutput, int());
    MOCK_METHOD1(MockProfileEnable, int(int controller));
    MOCK_METHOD1(MockProfileDisable, int(int controller));
    MOCK_METHOD1(MockGetConnectState, int(int controller));
};

void registerNewMockBTM(Mock_BTM *newInstance);

int MockProfileEnable(int controller);
int MockProfileDisable(int controller);
int MockGetConnectState(int controller);

void registerPmCallback(BtmPmCallbackTest *newBlock);
BtmPmCallbackTest* getPmCallback();