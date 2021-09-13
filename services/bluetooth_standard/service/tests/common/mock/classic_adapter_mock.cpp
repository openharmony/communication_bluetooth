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

#include "classic_adapter_mock.h"
#include "class_creator.h"
#include "log.h"
#include "btm.h"
#include "adapter_manager.h"
#include "unistd.h"
#include "common_mock.h"

namespace bluetooth {

ClassicAdapter::ClassicAdapter() : utility::Context(ADAPTER_NAME_CLASSIC, "5.0")
{
    LOG_INFO("Adapter:%s Constructor", Name().c_str());
}

ClassicAdapter::~ClassicAdapter()
{
    LOG_INFO("Adapter:%s Destructor", Name().c_str());
}
utility::Context *ClassicAdapter::GetContext()
{
    return this;
}

void ClassicAdapter::Enable()
{
    LOG_INFO("Adapter:%s Enable", Name().c_str());

    GetDispatcher()->PostTask(std::bind(&ClassicAdapter::EnableNative, this));
}

void ClassicAdapter::EnableNative()
{
    bool ret = false;

    int result = BTM_Enable(BREDR_CONTROLLER);
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

    // send response to adapt manager right now
    utility::Context *context = GetContext();
    if (context != nullptr) {
        context->OnEnable(ADAPTER_NAME_CLASSIC, ret);
    }
}

void ClassicAdapter::Disable()
{
    LOG_INFO("Adapter:%s Disable", Name().c_str());

    GetDispatcher()->PostTask(std::bind(&ClassicAdapter::DisableNative, this));
}

void ClassicAdapter::DisableNative()
{
    bool ret = false;

    /// Disable BTM
    int result = BTM_Disable(BREDR_CONTROLLER);
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
        context->OnDisable(ADAPTER_NAME_CLASSIC, ret);
    }
}

void ClassicAdapter::PostEnable()
{
    LOG_INFO("Adapter:%s PostEnable", Name().c_str());
}

// gap
int ClassicAdapter::GetDeviceType(const RawAddress &device) const
{
    return 0;
}
bool ClassicAdapter::RemoveAllPairs()
{
    return true;
}

std::string ClassicAdapter::GetLocalAddress() const
{
    std::string value = "00:00:00:00:00:01";
    return value;
}
std::string ClassicAdapter::GetLocalName() const
{
    std::string value = "classicLocalName";
    return value;
}
bool ClassicAdapter::SetLocalName(const std::string &name) const
{
    return true;
}
bool ClassicAdapter::SetBondableMode(int mode) const
{
    return true;
}
int ClassicAdapter::GetBondableMode() const
{
    return 1;
}
// remote device information
std::string ClassicAdapter::GetDeviceName(const RawAddress &device) const
{
    return "classicRemoteName";
}
std::vector<Uuid> ClassicAdapter::GetDeviceUuids(const RawAddress &device) const
{
    std::vector<Uuid> uuids;
    uuids.push_back(Uuid::ConvertFrom16Bits(0x1122));
    return uuids;
}
// pair
std::vector<RawAddress> ClassicAdapter::GetPairedDevices() const
{
    std::vector<RawAddress> pairedList;
    pairedList.push_back(RawAddress("00:00:00:00:00:11"));
    return pairedList;
}
bool ClassicAdapter::StartPair(const RawAddress &device)
{
    return true;
}
bool ClassicAdapter::IsBondedFromLocal(const RawAddress &device) const
{
    return true;
}
bool ClassicAdapter::CancelPairing(const RawAddress &device)
{
    return true;
}
bool ClassicAdapter::RemovePair(const RawAddress &device)
{
    return true;
}
int ClassicAdapter::GetPairState(const RawAddress &device) const
{
    return 1;
}
bool ClassicAdapter::SetDevicePairingConfirmation(const RawAddress &device, bool accept) const
{
    return true;
}
bool ClassicAdapter::SetDevicePasskey(const RawAddress &device, int passkey, bool accept) const
{
    return true;
}
bool ClassicAdapter::PairRequestReply(const RawAddress &device, bool accept) const
{
    return true;
}
// other
bool ClassicAdapter::IsAclConnected(const RawAddress &device) const
{
    return true;
}
bool ClassicAdapter::IsAclEncrypted(const RawAddress &device) const
{
    return true;
}

REGISTER_CLASS_CREATOR(ClassicAdapter);

}  // namespace bluetooth