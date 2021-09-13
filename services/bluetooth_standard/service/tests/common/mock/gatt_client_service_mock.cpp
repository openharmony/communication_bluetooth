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

#include "gatt_client_service_mock.h"
#include "class_creator.h"
#include "log.h"
#include "profile_service_manager.h"
#include "common_mock.h"

namespace bluetooth {

GattClientService::GattClientService() : utility::Context(PROFILE_NAME_GATT_CLIENT, "0.4.1")
{
    LOG_INFO("ProfileService:%s Create", Name().c_str());
}
GattClientService::~GattClientService()
{
    LOG_INFO("ProfileService:%s delete", Name().c_str());
}
utility::Context *GattClientService::GetContext()
{
    return this;
}

void GattClientService::Enable()
{
    LOG_INFO("ProfileService:%s StartUp", Name().c_str());
    GetDispatcher()->PostTask(std::bind(&GattClientService::EnableNative, this));
}

void GattClientService::EnableNative()
{
    bool ret = false;

    int result = MockProfileEnable(PROFILE_ID_GATT_CLIENT);
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
        context->OnEnable(PROFILE_NAME_GATT_CLIENT, ret);
    }
}

void GattClientService::Disable()
{
    LOG_INFO("ProfileService:%s ShutDown", Name().c_str());

    GetDispatcher()->PostTask(std::bind(&GattClientService::DisableNative, this));
}

void GattClientService::DisableNative()
{
    bool ret = false;

    /// Disable BTM
    int result = MockProfileDisable(PROFILE_ID_GATT_CLIENT);
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
        context->OnDisable(PROFILE_NAME_GATT_CLIENT, ret);
    }
}

int GattClientService::Connect(const RawAddress &device)
{
    return 1;
}

int GattClientService::Disconnect(const RawAddress &device)
{
    return 1;
}
std::list<RawAddress> GattClientService::GetConnectDevices()
{
    std::list<RawAddress> result;
    result.push_back(RawAddress("00:00:00:00:00:00"));
    return result;
}

int GattClientService::GetConnectState()
{
    return MockGetConnectState(PROFILE_ID_GATT_CLIENT);
}

int GattClientService::GetMaxConnectNum()
{
    return 1;
}

REGISTER_CLASS_CREATOR(GattClientService);

}  // namespace bluetooth
