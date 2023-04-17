/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#include <fstream>
#include <list>
#include <mutex>
#include <string>
#include "bluetooth_pbap_client.h"
#include "bluetooth_host.h"
#include "i_bluetooth_pbap_pce.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_pbap_pce_observer_stub.h"
#include "bluetooth_observer_list.h"
#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "iservice_registry.h"
#include "raw_address.h"
#include "system_ability_definition.h"
namespace OHOS {
namespace Bluetooth {
std::u16string PbapPullPhoneBookParam::GetName() const
{
    return name_;
}

void PbapPullPhoneBookParam::SetName(const std::u16string &name)
{
    name_ = name;
    specifiedBitset_ |= NAME;
}

uint64_t PbapPullPhoneBookParam::GetPropertySelector() const
{
    return propertySelector_;
}

void PbapPullPhoneBookParam::SetPropertySelector(uint64_t propertySelector)
{
    propertySelector_ = propertySelector;
    specifiedBitset_ |= PROPERTY_SELECTOR;
}

uint8_t PbapPullPhoneBookParam::GetFormat() const
{
    return format_;
}

void PbapPullPhoneBookParam::SetFormat(uint8_t format)
{
    format_ = format;
    specifiedBitset_ |= FORMAT;
}

uint16_t PbapPullPhoneBookParam::GetMaxListCount() const
{
    return maxListCount_;
}

void PbapPullPhoneBookParam::SetMaxListCount(uint16_t maxListCount)
{
    maxListCount_ = maxListCount;
    specifiedBitset_ |= MAX_LIST_COUNT;
}

uint16_t PbapPullPhoneBookParam::GetListStartOffset() const
{
    return listStartOffset_;
}

void PbapPullPhoneBookParam::SetListStartOffset(uint16_t listStartOffset)
{
    listStartOffset_ = listStartOffset;
    specifiedBitset_ |= LIST_START_OFFSET;
}

uint8_t PbapPullPhoneBookParam::GetResetNewMissedCalls() const
{
    return resetNewMissedCalls_;
}

void PbapPullPhoneBookParam::SetResetNewMissedCalls(uint8_t resetNewMissedCalls)
{
    resetNewMissedCalls_ = resetNewMissedCalls;
    specifiedBitset_ |= RESET_NEW_MISSED_CALLS;
}

uint64_t PbapPullPhoneBookParam::GetvCardSelector() const
{
    return vCardSelector_;
}

void PbapPullPhoneBookParam::SetvCardSelector(uint64_t vCardSelector)
{
    vCardSelector_ = vCardSelector;
    specifiedBitset_ |= VCARD_SELECTOR;
}

uint8_t PbapPullPhoneBookParam::GetvCardSelectorOp() const
{
    return vCardSelectorOp_;
}

void PbapPullPhoneBookParam::SetvCardSelectorOp(uint8_t vCardSelectorOp)
{
    vCardSelectorOp_ = vCardSelectorOp;
    specifiedBitset_ |= VCARD_SELECTOROP;
}

uint64_t PbapPullPhoneBookParam::GetSpecifiedBitset() const
{
    return specifiedBitset_;
}

void PbapPullPhoneBookParam::SetSpecifiedBitset(uint64_t specifiedBitset)
{
    specifiedBitset_ = specifiedBitset;
}

bool PbapPullPhoneBookParam::IsSpecified(int bitSpecified) const
{
    if (bitSpecified & specifiedBitset_) {
        return true;
    } else {
        return false;
    }
}

// implement PbapPullvCardListingParam
std::u16string PbapPullvCardListingParam::GetName() const
{
    return name_;
}

void PbapPullvCardListingParam::SetName(const std::u16string &name)
{
    name_ = name;
    specifiedBitset_ |= NAME;
}

uint8_t PbapPullvCardListingParam::GetOrder() const
{
    return order_;
}

void PbapPullvCardListingParam::SetOrder(uint8_t order)
{
    order_ = order;
    specifiedBitset_ |= ORDER;
}

std::string PbapPullvCardListingParam::GetSearchValue() const
{
    return searchValue_;
}

void PbapPullvCardListingParam::SetSearchValue(const std::string &searchValue)
{
    searchValue_ = searchValue;
    specifiedBitset_ |= SEARCH_VALUE;
}

uint8_t PbapPullvCardListingParam::GetSearchProperty() const
{
    return searchProperty_;
}

void PbapPullvCardListingParam::SetSearchProperty(uint8_t searchProperty)
{
    searchProperty_ = searchProperty;
    specifiedBitset_ |= SEARCH_PROPERTY;
}

uint16_t PbapPullvCardListingParam::GetMaxListCount() const
{
    return maxListCount_;
}

void PbapPullvCardListingParam::SetMaxListCount(uint16_t maxListCount)
{
    maxListCount_ = maxListCount;
    specifiedBitset_ |= MAX_LIST_COUNT;
}

uint16_t PbapPullvCardListingParam::GetListStartOffset() const
{
    return listStartOffset_;
}

void PbapPullvCardListingParam::SetListStartOffset(uint16_t listStartOffset)
{
    listStartOffset_ = listStartOffset;
    specifiedBitset_ |= LIST_START_OFFSET;
}

uint8_t PbapPullvCardListingParam::GetResetNewMissedCalls() const
{
    return resetNewMissedCalls_;
}

void PbapPullvCardListingParam::SetResetNewMissedCalls(uint8_t resetNewMissedCalls)
{
    resetNewMissedCalls_ = resetNewMissedCalls;
    specifiedBitset_ |= RESET_NEW_MISSED_CALLS;
}

uint64_t PbapPullvCardListingParam::GetvCardSelector() const
{
    return vCardSelector_;
}

void PbapPullvCardListingParam::SetvCardSelector(uint64_t vCardSelector)
{
    vCardSelector_ = vCardSelector;
    specifiedBitset_ |= VCARD_SELECTOR;
}

uint8_t PbapPullvCardListingParam::GetvCardSelectorOp() const
{
    return vCardSelectorOp_;
}

void PbapPullvCardListingParam::SetvCardSelectorOp(uint8_t vCardSelectorOp)
{
    vCardSelectorOp_ = vCardSelectorOp;
    specifiedBitset_ |= VCARD_SELECTOROP;
}

uint64_t PbapPullvCardListingParam::GetSpecifiedBitset() const
{
    return specifiedBitset_;
}

void PbapPullvCardListingParam::SetSpecifiedBitset(uint64_t specifiedBitset)
{
    specifiedBitset_ = specifiedBitset;
}

bool PbapPullvCardListingParam::IsSpecified(int bitSpecified) const
{
    if (bitSpecified & specifiedBitset_) {
        return true;
    } else {
        return false;
    }
}

// implement PbapPullvCardEntryParam
std::u16string PbapPullvCardEntryParam::GetName() const
{
    return name_;
}

void PbapPullvCardEntryParam::SetName(const std::u16string &name)
{
    name_ = name;
    specifiedBitset_ |= NAME;
}

uint64_t PbapPullvCardEntryParam::GetPropertySelector() const
{
    return propertySelector_;
}

void PbapPullvCardEntryParam::SetPropertySelector(uint64_t propertySelector)
{
    propertySelector_ = propertySelector;
    specifiedBitset_ |= PROPERTY_SELECTOR;
}

uint8_t PbapPullvCardEntryParam::GetFormat() const
{
    return format_;
}

void PbapPullvCardEntryParam::SetFormat(uint8_t format)
{
    format_ = format;
    specifiedBitset_ |= FORMAT;
}

uint64_t PbapPullvCardEntryParam::GetSpecifiedBitset() const
{
    return specifiedBitset_;
}

void PbapPullvCardEntryParam::SetSpecifiedBitset(uint64_t specifiedBitset)
{
    specifiedBitset_ = specifiedBitset;
}

bool PbapPullvCardEntryParam::IsSpecified(int bitSpecified) const
{
    if (bitSpecified & specifiedBitset_) {
        return true;
    } else {
        return false;
    }
}

struct PbapClient::impl {
    impl();
    ~impl()
    {}
    void RegisterObserver(std::shared_ptr<PbapClientObserver> &observer)
    {
        return;
    }

    void DeregisterObserver(std::shared_ptr<PbapClientObserver> &observer)
    {
        return;
    }

    int GetDeviceState(const BluetoothRemoteDevice &device)
    {
        return 0;
    }

    std::vector<BluetoothRemoteDevice> GetDevicesByStates(const std::vector<int> &states)
    {
        HILOGI("enter");
        std::vector<BluetoothRemoteDevice> remoteDevices;
        return remoteDevices;
    }

    std::vector<BluetoothRemoteDevice> GetConnectedDevices()
    {
        HILOGI("enter");
        std::vector<BluetoothRemoteDevice> remoteDevices;
        return remoteDevices;
    }

    bool Connect(const BluetoothRemoteDevice &device)
    {
        return false;
    }

    int PullPhoneBook(const BluetoothRemoteDevice &device, const BluetoothIPbapPullPhoneBookParam &param)
    {
        return -1;
    }

    int SetPhoneBook(const BluetoothRemoteDevice &device, const std::u16string &name, int flag)
    {
        return -1;
    }

    int PullvCardListing(const BluetoothRemoteDevice &device, const BluetoothIPbapPullvCardListingParam &param)
    {
        return -1;
    }

    int PullvCardEntry(const BluetoothRemoteDevice &device, const BluetoothIPbapPullvCardEntryParam &param)
    {
        return -1;
    }

    bool IsDownloading(const BluetoothRemoteDevice &device)
    {
        return false;
    }

    int AbortDownloading(const BluetoothRemoteDevice &device)
    {
        return -1;
    }

    bool Disconnect(const BluetoothRemoteDevice &device)
    {
        return false;
    }

    bool SetConnectionStrategy(const BluetoothRemoteDevice &device, int strategy)
    {
        return false;
    }

    int GetConnectionStrategy(const BluetoothRemoteDevice &device)
    {
        return (int)BTStrategyType::CONNECTION_UNKNOWN;;
    }

    int SetDevicePassword(const BluetoothRemoteDevice &device, const std::string &password, const std::string &userId)
    {
        return -1;
    }

private:
    std::mutex mutex_;
    BluetoothObserverList<PbapClientObserver> frameworkObserverList_ {};
};

PbapClient::impl::impl()
{
    return;
}
PbapClient *PbapClient::GetProfile()
{
    return nullptr;
}

PbapClient::PbapClient()
{
    pimpl = std::make_unique<impl>();
}

PbapClient::~PbapClient()
{}

void PbapClient::RegisterObserver(PbapClientObserver *observer)
{
    HILOGI("enter");
    std::shared_ptr<PbapClientObserver> pointer(observer, [](PbapClientObserver *) {});
    return pimpl->RegisterObserver(pointer);
}

void PbapClient::DeregisterObserver(PbapClientObserver *observer)
{
    HILOGI("enter");
    std::shared_ptr<PbapClientObserver> pointer(observer, [](PbapClientObserver *) {});
    return pimpl->DeregisterObserver(pointer);
}

int PbapClient::GetDeviceState(const BluetoothRemoteDevice &device)
{
    return pimpl->GetDeviceState(device);
}

std::vector<BluetoothRemoteDevice> PbapClient::GetDevicesByStates(const std::vector<int> &states)
{
    return pimpl->GetDevicesByStates(states);
}

std::vector<BluetoothRemoteDevice> PbapClient::GetConnectedDevices()
{
    return pimpl->GetConnectedDevices();
}

bool PbapClient::Connect(const BluetoothRemoteDevice &device)
{
    return pimpl->Connect(device);
}

bool PbapClient::Disconnect(const BluetoothRemoteDevice &device)
{
    return pimpl->Disconnect(device);
}

bool PbapClient::SetConnectionStrategy(const BluetoothRemoteDevice &device, int strategy)
{
    return pimpl->SetConnectionStrategy(device, strategy);
}

int PbapClient::GetConnectionStrategy(const BluetoothRemoteDevice &device)
{
    return pimpl->GetConnectionStrategy(device);
}

int PbapClient::SetDevicePassword(
    const BluetoothRemoteDevice &device, const std::string &password, const std::string &userId)
{
    return pimpl->SetDevicePassword(device, password, userId);
}

int PbapClient::PullPhoneBook(const BluetoothRemoteDevice &device, const PbapPullPhoneBookParam &param)
{
    return -1;
}

int PbapClient::SetPhoneBook(const BluetoothRemoteDevice &device, const std::u16string &name, int flag)
{
    return pimpl->SetPhoneBook(device, name, flag);
}

int PbapClient::PullvCardListing(const BluetoothRemoteDevice &device, const PbapPullvCardListingParam &param)
{
    return -1;
}

int PbapClient::PullvCardEntry(const BluetoothRemoteDevice &device, const PbapPullvCardEntryParam &param)
{
    return -1;
}

bool PbapClient::IsDownloading(const BluetoothRemoteDevice &device)
{
    return pimpl->IsDownloading(device);
}

int PbapClient::AbortDownloading(const BluetoothRemoteDevice &device)
{
    return pimpl->AbortDownloading(device);
}
}  // namespace Bluetooth
}  // namespace OHOS