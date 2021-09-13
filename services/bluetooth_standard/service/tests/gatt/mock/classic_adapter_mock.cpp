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
#include <unistd.h>

#include <sys/time.h>
#include <algorithm>
#include "interface_profile_manager.h"
#include "base_observer_list.h"
#include "adapter_manager.h"
#include "btstack.h"

namespace bluetooth {
struct ClassicAdapter::impl {
    impl();
    std::recursive_mutex syncMutex_{};
    BaseObserverList<IAdapterClassicObserver> adapterObservers_{};
    BaseObserverList<IClassicRemoteDeviceObserver> remoteObservers_{};
};

ClassicAdapter::impl::impl()
{}
ClassicAdapter::ClassicAdapter() : utility::Context(ADAPTER_NAME_CLASSIC, "5.0"), pimpl(std::make_unique<ClassicAdapter::impl>())
{}
ClassicAdapter::~ClassicAdapter()
{}
void ClassicAdapter::Enable()
{}
void ClassicAdapter::Disable()
{}
void ClassicAdapter::PostEnable()
{}

std::string ClassicAdapter::GetLocalAddress() const
{
    return "abc";
}

std::string ClassicAdapter::GetLocalName() const
{
    return "abc";
}

bool ClassicAdapter::SetLocalName(const std::string &name) const
{
    return true;
}

int ClassicAdapter::GetLocalDeviceClass() const
{
    return 0;
}

bool ClassicAdapter::SetLocalDeviceClass(int deviceClass) const
{
    return true;
}

int ClassicAdapter::GetBtScanMode() const
{
    return 0;
}

bool ClassicAdapter::SetBtScanMode(int mode, int duration)
{
    return true;
}

int ClassicAdapter::GetBondableMode() const
{
    return 0;
}

bool ClassicAdapter::SetBondableMode(int mode) const
{
    return true;
}

bool ClassicAdapter::StartBtDiscovery()
{
    return true;
}

bool ClassicAdapter::CancelBtDiscovery()
{
    return true;
}

bool ClassicAdapter::IsBtDiscovering() const
{
    return true;
}

long ClassicAdapter::GetBtDiscoveryEndMillis() const
{
    return discoveryEndMs_;
}

std::string ClassicAdapter::GetDeviceName(const RawAddress &device) const
{
    return "";
}

std::vector<Uuid> ClassicAdapter::GetDeviceUuids(const RawAddress &device) const
{
    std::vector<Uuid> uuids;
    uuids.clear();
    return uuids;
}

bool ClassicAdapter::IsAclConnected(const RawAddress &device) const
{
    return true;
}

bool ClassicAdapter::IsAclEncrypted(const RawAddress &device) const
{
    return true;
}

utility::Context *ClassicAdapter::GetContext()
{
    return this;
}

bool ClassicAdapter::IsBondedFromLocal(const RawAddress &device) const
{
    return true;
}

std::vector<RawAddress> ClassicAdapter::GetPairedDevices() const
{
    std::vector<RawAddress> pairedList;
    pairedList.clear();

    return pairedList;
}

bool ClassicAdapter::StartPair(const RawAddress &device)
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

bool ClassicAdapter::RemoveAllPairs()
{
    return true;
}

int ClassicAdapter::GetPairState(const RawAddress &device) const
{
    return 0;
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

bool ClassicAdapter::RegisterClassicAdapterObserver(IAdapterClassicObserver &observer) const
{
    RawAddress addres;
    observer.OnDiscoveryStateChanged(0);
    observer.OnDiscoveryResult(addres);
    observer.OnPairRequested(ADAPTER_BREDR, addres);
    observer.OnPairConfirmed(ADAPTER_BREDR, addres, 0, 0);
    observer.OnScanModeChanged(0);
    observer.OnDeviceNameChanged("abc");
    observer.OnDeviceAddrChanged("abc");
    return true;
}

bool ClassicAdapter::DeregisterClassicAdapterObserver(IAdapterClassicObserver &observer) const
{
    return true;
}

bool ClassicAdapter::SetDevicePin(const RawAddress &device, const std::string &pinCode) const
{
    return true;
}

int ClassicAdapter::ClassicAdapter::GetDeviceType(const RawAddress &device) const
{
    return 0;
}

int ClassicAdapter::GetDeviceClass(const RawAddress &device) const
{
    return 0;
}

std::string ClassicAdapter::GetAliasName(const RawAddress &device) const
{
    return "alias";
}

uint8_t ClassicAdapter::GetFlags(const RawAddress &device) const
{
    return 0;
}

bool ClassicAdapter::SetAliasName(const RawAddress &device, const std::string &name) const
{
    return true;
}

bool ClassicAdapter::RegisterRemoteDeviceObserver(IClassicRemoteDeviceObserver &observer) const
{
    return true;
}

bool ClassicAdapter::DeregisterRemoteDeviceObserver(IClassicRemoteDeviceObserver &observer) const
{
    return true;
}

int ClassicAdapter::GetDeviceBatteryLevel(const RawAddress &device) const
{
    return 0;
}

void ClassicAdapter::SetDeviceBatteryLevel(const RawAddress &device, int batteryLevel) const
{}
}  // namespace bluetooth
