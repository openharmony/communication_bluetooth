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

#include "a2dp_src_service_mock.h"
#include "class_creator.h"
#include "log.h"
#include "profile_service_manager.h"
#include "common_mock.h"

namespace bluetooth {

A2dpSrcService::A2dpSrcService() : utility::Context(PROFILE_NAME_A2DP_SRC, "0.4.1")
{
    LOG_INFO("ProfileService:%s Create", Name().c_str());
}

A2dpSrcService::~A2dpSrcService()
{
    LOG_INFO("ProfileService:%s delete", Name().c_str());
}

utility::Context *A2dpSrcService::GetContext()
{
    return this;
}
void A2dpSrcService::Enable()
{
    LOG_INFO("ProfileService:%s StartUp", Name().c_str());
    GetDispatcher()->PostTask(std::bind(&A2dpSrcService::EnableNative, this));
}

void A2dpSrcService::EnableNative()
{
    bool ret = false;

    int result = MockProfileEnable(PROFILE_ID_A2DP_SRC);
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
        context->OnEnable(PROFILE_NAME_A2DP_SRC, ret);
    }
}

void A2dpSrcService::Disable()
{
    LOG_INFO("ProfileService:%s ShutDown", Name().c_str());

    GetDispatcher()->PostTask(std::bind(&A2dpSrcService::DisableNative, this));
}

void A2dpSrcService::DisableNative()
{
    bool ret = false;

    /// Disable BTM
    int result = MockProfileDisable(PROFILE_ID_A2DP_SRC);
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
        context->OnDisable(PROFILE_NAME_A2DP_SRC, ret);
    }
}

int A2dpSrcService::Connect(const RawAddress &device)

{
    return true;
}

int A2dpSrcService::Disconnect(const RawAddress &device)
{
    return -1;
}

std::list<RawAddress> A2dpSrcService::GetConnectDevices()
{
    std::list<RawAddress> devList;

    return devList;
}

int A2dpSrcService::GetConnectState()
{
    return MockGetConnectState(PROFILE_ID_A2DP_SRC);
}

int A2dpSrcService::GetMaxConnectNum()
{
    return -1;
}

REGISTER_CLASS_CREATOR(A2dpSrcService);

}  // namespace bluetooth