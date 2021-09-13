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
#include "log.h"
#include "btm.h"
#include "btstack.h"
#include "adapter_manager.h"
#include "ble_feature.h"
#include "btm.h"
#include "ble_utils.h"
#include "base_observer_list.h"
#include <unistd.h>

namespace bluetooth {
struct BleAdapter::impl {
    impl();

    std::recursive_mutex syncMutex_;
    std::unique_ptr<BaseObserverList<IAdapterBleObserver>> observer_ =
        std::make_unique<BaseObserverList<IAdapterBleObserver>>();
    std::unique_ptr<BaseObserverList<IBlePeripheralCallback>> blePeripheralCallback_ =
        std::make_unique<BaseObserverList<IBlePeripheralCallback>>();
};

BleAdapter::impl::impl() : syncMutex_()
{}
BleAdapter::BleAdapter() : utility::Context(ADAPTER_NAME_BLE, "5.0"), pimpl(std::make_unique<BleAdapter::impl>())
{}
BleAdapter::~BleAdapter()
{}
void BleAdapter::Enable()
{}

bool BleAdapter::EnableTask()
{
    return true;
}

int BleAdapter::InitBtmAndGap()
{
    return 0;
}

void BleAdapter::Disable()
{}

bool BleAdapter::DisableTask()
{
    return true;
}

void BleAdapter::BtmDisable()
{}
void BleAdapter::PostEnable()
{}

bool BleAdapter::PostEnableTask()
{
    return true;
}

bool BleAdapter::StopAdvAndScan()
{
    return false;
}

bool BleAdapter::LoadConfig()
{
    return true;
}

int BleAdapter::DeregisterAllCallback()
{
    return 0;
}

int BleAdapter::SetRpaAddrAndTypeToBtm()
{
    return 0;
}

std::string BleAdapter::GetLocalAddress() const
{
    return "";
}

std::string BleAdapter::GetLocalName() const
{
    return "";
}

bool BleAdapter::SetLocalName(const std::string &name) const
{
    return true;
}
int BleAdapter::GetBleRoles() const
{
    return 0;
}

bool BleAdapter::SetBleRoles(uint8_t roles)
{
    return true;
}

std::string BleAdapter::GetDeviceName(const RawAddress &device) const
{
    return "";
}

std::vector<Uuid> BleAdapter::GetDeviceUuids(const RawAddress &device) const
{
    std::vector<Uuid> uuids;
    uuids.clear();

    return uuids;
}

std::vector<RawAddress> BleAdapter::GetPairedDevices() const
{
    std::vector<RawAddress> pairedList;
    pairedList.clear();

    return pairedList;
}

bool BleAdapter::StartPair(const RawAddress &device)
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

bool BleAdapter::RemoveAllPairs()
{
    return true;
}

bool BleAdapter::IsBondedFromLocal(const RawAddress &device) const
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

bool BleAdapter::IsAclConnected(const RawAddress &device) const
{
    return true;
}

bool BleAdapter::IsAclEncrypted(const RawAddress &device) const
{
    return true;
}

utility::Context *BleAdapter::GetContext()
{
    return this;
}

int BleAdapter::GetPairState(const RawAddress &device) const
{
    return 0;
}

int BleAdapter::GetBondableMode() const
{
    return 0;
}

bool BleAdapter::SetBondableMode(int mode) const
{
    return true;
}

bool BleAdapter::SetDevicePairingConfirmation(const RawAddress &device, bool accept) const
{
    return true;
}

int BleAdapter::GetBleMaxAdvertisingDataLength() const
{
    return 23;
}

int BleAdapter::GetIoCapability() const
{
    return 0;
}

bool BleAdapter::SetIoCapability(int ioCapability) const
{
    return true;
}

bool BleAdapter::IsBtDiscovering() const
{
    return true;
}

void BleAdapter::RegisterBleAdvertiserCallback(IBleAdvertiserCallback &callback)
{}
void BleAdapter::DeregisterBleAdvertiserCallback() const
{}
void BleAdapter::RegisterBleCentralManagerCallback(IBleCentralManagerCallback &callback)
{}
void BleAdapter::DeregisterBleCentralManagerCallback() const
{}
void BleAdapter::RegisterBlePeripheralCallback(IBlePeripheralCallback &callback) const
{}
void BleAdapter::DeregisterBlePeripheralCallback(IBlePeripheralCallback &callback) const
{}

bool BleAdapter::RegisterBleAdapterObserver(IAdapterBleObserver &observer) const
{
    RawAddress addres;
    observer.OnDeviceNameChanged("abc");
    observer.OnDiscoveryStateChanged(0);
    observer.OnDiscoveryResult(addres);
    observer.OnPairRequested(ADAPTER_BLE, addres);
    observer.OnPairConfirmed(ADAPTER_BLE, addres, 0, 0);
    observer.OnScanModeChanged(0);
    observer.OnDeviceAddrChanged("abc");
    observer.OnAdvertisingStateChanged(1);
    return true;
}

bool BleAdapter::DeregisterBleAdapterObserver(IAdapterBleObserver &observer) const
{
    return true;
}

int BleAdapter::GetPeerDeviceAddrType(const RawAddress &device) const
{
    return 0;
}

bool BleAdapter::ReadRemoteRssiValue(const RawAddress &device) const
{
    return true;
}

int BleAdapter::GetDeviceType(const RawAddress &device) const
{
    return 0;
}

void BleAdapter::StartAdvertising(const BleAdvertiserSettingsImpl &settings, const BleAdvertiserDataImpl &advData,
    const BleAdvertiserDataImpl &scanResponse, uint8_t advHandle) const
{}
void BleAdapter::StopAdvertising(uint8_t advHandle) const
{}
void BleAdapter::Close(uint8_t advHandle) const
{}
void BleAdapter::StartScan() const
{}
void BleAdapter::StartScan(const BleScanSettingsImpl &setting) const
{}
void BleAdapter::StopScan() const
{}

int BleAdapter::GetAdvertisingStatus() const
{
    return 0;
}

bool BleAdapter::IsLlPrivacySupported() const
{
    return true;
}

void BleAdapter::AddCharacteristicValue(uint8_t adtype, const std::string &data) const
{}

uint8_t BleAdapter::GetAdvertiserHandle() const
{
    return 0;
}

REGISTER_CLASS_CREATOR(BleAdapter);
}  // namespace bluetooth
