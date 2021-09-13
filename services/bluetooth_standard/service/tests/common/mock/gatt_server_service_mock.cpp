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

#include "gatt_server_service_mock.h"
#include "class_creator.h"
#include "gatt_database.h"
#include "gatt_server_profile.h"
#include "gatt_service_base.h"
#include "log.h"
#include "profile_service_manager.h"
#include "common_mock.h"

namespace bluetooth {

GattServerService::GattServerService() : utility::Context(PROFILE_NAME_GATT_SERVER, "0.4.1")
{
    LOG_INFO("ProfileService:%s Create", Name().c_str());
}
GattServerService::~GattServerService()
{
    LOG_INFO("ProfileService:%s Destroy", Name().c_str());
}
utility::Context *GattServerService::GetContext()
{
    return this;
}

void GattServerService::Enable()
{
    LOG_INFO("ProfileService:%s StartUp", Name().c_str());
    GetDispatcher()->PostTask(std::bind(&GattServerService::EnableNative, this));
}

void GattServerService::EnableNative()
{
    bool ret = false;

    int result = MockProfileEnable(PROFILE_ID_GATT_SERVER);
    if (BT_NO_ERROR == result) {
        usleep(TEST_TIMER_NORMAL);
        LOG_INFO("Enable::BTM enable success!");
        ret = true;
    } else if (BT_TIMEOUT == result) {
        sleep(TEST_TIMER_TIMEOUT);
        ret = true;
    } else {
        LOG_ERROR("Enable::BTM enable failed, error code is %d", result);
        ret = false;
    }

    // send response to adapt manager
    utility::Context *context = GetContext();
    if (context != nullptr) {
        context->OnEnable(PROFILE_NAME_GATT_SERVER, ret);
    }
}

void GattServerService::Disable()
{
    LOG_INFO("ProfileService:%s ShutDown", Name().c_str());

    GetDispatcher()->PostTask(std::bind(&GattServerService::DisableNative, this));
}

void GattServerService::DisableNative()
{
    bool ret = false;

    /// Disable BTM
    int result = MockProfileDisable(PROFILE_ID_GATT_SERVER);
    if (BT_NO_ERROR == result) {
        usleep(TEST_TIMER_NORMAL);
        LOG_INFO("Disable::BTM disable success!");
        ret = true;
    } else if (BT_TIMEOUT == result) {
        sleep(TEST_TIMER_TIMEOUT);
        ret = true;
    } else {
        LOG_ERROR("Disable::BTM disable failed, error code is %d", result);
        ret = false;
    }

    // send response to adapt manager right now
    utility::Context *context = GetContext();
    if (context != nullptr) {
        context->OnDisable(PROFILE_NAME_GATT_SERVER, ret);
    }
}

int GattServerService::Connect(const RawAddress &device)
{
    return 1;
}

int GattServerService::Disconnect(const RawAddress &device)
{
    return 1;
}
std::list<RawAddress> GattServerService::GetConnectDevices()
{
    std::list<RawAddress> result;
    result.push_back(RawAddress("00:00:00:00:00:00"));
    return result;
}

int GattServerService::GetConnectState()
{
    return MockGetConnectState(PROFILE_ID_GATT_SERVER);
}

int GattServerService::GetMaxConnectNum()
{
    return 1;
}

REGISTER_CLASS_CREATOR(GattServerService);
}  // namespace bluetooth
