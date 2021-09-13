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

#include "ble_adapter_mock.h"
#include "class_creator.h"
#include "adapter_manager.h"
#include "log.h"
#include "btm.h"
#include "unistd.h"
#include "common_mock.h"

namespace bluetooth {

BleAdapter::BleAdapter() : utility::Context(ADAPTER_NAME_BLE, "5.0")
{
    LOG_INFO("Adapter:%s Create", Name().c_str());
}

BleAdapter::~BleAdapter()
{
    LOG_INFO("Adapter:%s Destroy", Name().c_str());
}
utility::Context *BleAdapter::GetContext()
{
    return this;
}

void BleAdapter::Enable()
{
    LOG_INFO("Adapter:%s Enable", Name().c_str());

    GetDispatcher()->PostTask(std::bind(&BleAdapter::EnableNative, this));
}

void BleAdapter::EnableNative()
{
    bool ret = false;

    int result = BTM_Enable(LE_CONTROLLER);
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
        context->OnEnable(ADAPTER_NAME_BLE, ret);
    }
}

void BleAdapter::Disable()
{
    LOG_INFO("Adapter:%s Disable", Name().c_str());

    GetDispatcher()->PostTask(std::bind(&BleAdapter::DisableNative, this));
}

void BleAdapter::DisableNative()
{
    bool ret = false;

    int result = BTM_Disable(LE_CONTROLLER);
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
        context->OnDisable(ADAPTER_NAME_BLE, ret);
    }
}

void BleAdapter::PostEnable()
{
    LOG_INFO("Adapter:%s %s", Name().c_str(), __FUNCTION__);
}

// gap
std::string BleAdapter::GetLocalAddress() const
{
    std::string value = "00:00:00:00:00:02";
    return value;
}
std::string BleAdapter::GetLocalName() const
{
    std::string value = "bleLocalName";
    return value;
}
bool BleAdapter::SetLocalName(const std::string &name) const
{
    return true;
}
bool BleAdapter::SetBondableMode(int mode) const
{
    return true;
}
int BleAdapter::GetBondableMode() const
{
    return 1;
}
// remote device information
int BleAdapter::GetDeviceType(const RawAddress &device) const
{
    return 0;
}
bool BleAdapter::RemoveAllPairs()
{
    return true;
}

std::string BleAdapter::GetDeviceName(const RawAddress &device) const
{
    return "bleRemoteName";
}
std::vector<Uuid> BleAdapter::GetDeviceUuids(const RawAddress &device) const
{
    std::vector<Uuid> uuids;
    uuids.push_back(Uuid::ConvertFrom16Bits(0x2233));
    return uuids;
}
// pair
std::vector<RawAddress> BleAdapter::GetPairedDevices() const
{
    std::vector<RawAddress> pairedList;
    pairedList.push_back(RawAddress("00:00:00:00:00:22"));
    return pairedList;
}
bool BleAdapter::StartPair(const RawAddress &device)
{
    return true;
}
bool BleAdapter::IsBondedFromLocal(const RawAddress &device) const
{
    return true;
}
bool BleAdapter::CancelPairing(const RawAddress &device)
{
    return true;
}
bool BleAdapter::RemovePair(const RawAddress &device)
{
    return true;
}
int BleAdapter::GetPairState(const RawAddress &device) const
{
    return 1;
}
bool BleAdapter::SetDevicePairingConfirmation(const RawAddress &device, bool accept) const
{
    return true;
}
bool BleAdapter::SetDevicePasskey(const RawAddress &device, int passkey, bool accept) const
{
    return true;
}
bool BleAdapter::PairRequestReply(const RawAddress &device, bool accept) const
{
    return true;
}
// other
bool BleAdapter::IsAclConnected(const RawAddress &device) const
{
    return true;
}
bool BleAdapter::IsAclEncrypted(const RawAddress &device) const
{
    return true;
}

REGISTER_CLASS_CREATOR(BleAdapter);

}  // namespace bluetooth