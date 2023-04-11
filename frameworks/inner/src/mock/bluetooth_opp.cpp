/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "bluetooth_opp.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_observer_list.h"
#include "bluetooth_opp_observer_stub.h"
#include "i_bluetooth_opp.h"
#include "i_bluetooth_host.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Bluetooth {
BluetoothOppTransferInformation::BluetoothOppTransferInformation()
{}

BluetoothOppTransferInformation::~BluetoothOppTransferInformation()
{}

int BluetoothOppTransferInformation::GetId() const
{
    return id_;
}

std::string BluetoothOppTransferInformation::GetFileName() const
{
    return fileName_;
}

std::string BluetoothOppTransferInformation::GetFilePath() const
{
    return filePath_;
}

std::string BluetoothOppTransferInformation::GetMimeType() const
{
    return mimeType_;
}

std::string BluetoothOppTransferInformation::GetDeviceName() const
{
    return deviceName_;
}

std::string BluetoothOppTransferInformation::GetDeviceAddress() const
{
    return deviceAddress_;
}

int BluetoothOppTransferInformation::GetDirection() const
{
    return direction_;
}

int BluetoothOppTransferInformation::GetStatus() const
{
    return status_;
}

int BluetoothOppTransferInformation::GetFailedReason() const
{
    return failedReason_;
}

uint64_t BluetoothOppTransferInformation::GetTimeStamp() const
{
    return timeStamp_;
}

uint64_t BluetoothOppTransferInformation::GetCurrentBytes() const
{
    return currentBytes_;
}

uint64_t BluetoothOppTransferInformation::GetTotalBytes() const
{
    return totalBytes_;
}

void BluetoothOppTransferInformation::SetId(int id)
{
    id_ = id;
}

void BluetoothOppTransferInformation::SetFileName(std::string fileName)
{
    fileName_ = fileName;
}

void BluetoothOppTransferInformation::SetFilePath(std::string filePath)
{
    filePath_ = filePath;
}

void BluetoothOppTransferInformation::SetMimeType(std::string mimeType)
{
    mimeType_ = mimeType;
}

void BluetoothOppTransferInformation::SetDeviceName(std::string deviceName)
{
    deviceName_ = deviceName;
}

void BluetoothOppTransferInformation::SetDeviceAddress(std::string deviceAddress)
{
    deviceAddress_ = deviceAddress;
}

void BluetoothOppTransferInformation::SetDirection(int direction)
{
    direction_ = direction;
}

void BluetoothOppTransferInformation::SetStatus(int status)
{
    status_ = status;
}

void BluetoothOppTransferInformation::SetFailedReason(int failedReason)
{
    failedReason_ = failedReason;
}

void BluetoothOppTransferInformation::SetTimeStamp(uint64_t timeStamp)
{
    timeStamp_ = timeStamp;
}

void BluetoothOppTransferInformation::SetCurrentBytes(uint64_t currentBytes)
{
    currentBytes_ = currentBytes;
}

void BluetoothOppTransferInformation::SetTotalBytes(uint64_t totalBytes)
{
    totalBytes_ = totalBytes;
}

struct Opp::impl {
    impl();
    ~impl();
    std::vector<BluetoothRemoteDevice> GetDevicesByStates(std::vector<int> states)
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        std::vector<BluetoothRemoteDevice> remoteDevices;
        return remoteDevices;
    }

    int GetDeviceState(const BluetoothRemoteDevice &device)
    {
        return (int)BTConnectState::DISCONNECTED;
    }

    bool SendFile(std::string device, std::vector<std::string>filePaths, std::vector<std::string>mimeTypes)
    {
        return false;
    }

    bool SetIncomingFileConfirmation(bool accept)
    {
        return false;
    }

    BluetoothOppTransferInformation GetCurrentTransferInformation()
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        BluetoothOppTransferInformation transferInformation;
        return transferInformation;
    }

    bool CancelTransfer()
    {
        return false;
    }

    void RegisterObserver(std::shared_ptr<OppObserver> observer)
    {
        return;
    }

    void DeregisterObserver(std::shared_ptr<OppObserver> observer)
    {
        return;
    }

private:
    BluetoothObserverList<OppObserver> observers_;
    sptr<IBluetoothOpp> proxy_;
};

Opp::impl::impl()
{
    return;
}

Opp::impl::~impl()
{}

Opp::Opp()
{
    pimpl = std::make_unique<impl>();
}

Opp::~Opp()
{}

Opp *Opp::GetProfile()
{
    return nullptr;
}
std::vector<BluetoothRemoteDevice> Opp::GetDevicesByStates(std::vector<int> states)
{
    return pimpl->GetDevicesByStates(states);
}

int Opp::GetDeviceState(const BluetoothRemoteDevice &device)
{
    return pimpl->GetDeviceState(device);
}

bool Opp::SendFile(std::string device, std::vector<std::string>filePaths, std::vector<std::string>mimeTypes)
{
    return pimpl->SendFile(device, filePaths, mimeTypes);
}

bool Opp::SetIncomingFileConfirmation(bool accept)
{
    return pimpl->SetIncomingFileConfirmation(accept);
}

BluetoothOppTransferInformation Opp::GetCurrentTransferInformation()
{
    BluetoothOppTransferInformation transferInformation = pimpl->GetCurrentTransferInformation();
    return transferInformation;
}

bool Opp::CancelTransfer()
{
    return pimpl->CancelTransfer();
}

void Opp::RegisterObserver(OppObserver *observer)
{
    std::shared_ptr<OppObserver> observerPtr(observer, [](OppObserver *) {});
    return pimpl->RegisterObserver(observerPtr);
}

void Opp::DeregisterObserver(OppObserver *observer)
{
    std::shared_ptr<OppObserver> observerPtr(observer, [](OppObserver *) {});
    return pimpl->DeregisterObserver(observerPtr);
}
}  // namespace Bluetooth
}  // namespace OHOS