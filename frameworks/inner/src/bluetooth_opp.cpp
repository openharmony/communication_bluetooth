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
#include "bluetooth_profile_manager.h"
#include "bluetooth_log.h"
#include "bluetooth_observer_list.h"
#include "bluetooth_opp_observer_stub.h"
#include "bluetooth_utils.h"
#include "i_bluetooth_opp.h"
#include "i_bluetooth_host.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Bluetooth {
std::mutex g_oppProxyMutex;
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

class OppInnerObserver : public BluetoothOppObserverStub {
public:
    explicit OppInnerObserver(BluetoothObserverList<OppObserver> &observers) : observers_(observers)
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    }
    ~OppInnerObserver() override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    }

    ErrCode OnReceiveIncomingFileChanged(const BluetoothIOppTransferInformation &transferInformation) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        BluetoothOppTransferInformation oppTransferinformation;
        oppTransferinformation.SetId(transferInformation.GetId());
        oppTransferinformation.SetFileName(transferInformation.GetFileName());
        oppTransferinformation.SetFilePath(transferInformation.GetFilePath());
        oppTransferinformation.SetMimeType(transferInformation.GetFileType());
        oppTransferinformation.SetDeviceName(transferInformation.GetDeviceName());
        oppTransferinformation.SetDeviceAddress(transferInformation.GetDeviceAddress());
        oppTransferinformation.SetFailedReason(transferInformation.GetFailedReason());
        oppTransferinformation.SetStatus(transferInformation.GetStatus());
        oppTransferinformation.SetDirection(transferInformation.GetDirection());
        oppTransferinformation.SetTimeStamp(transferInformation.GetTimeStamp());
        oppTransferinformation.SetCurrentBytes(transferInformation.GetCurrentBytes());
        oppTransferinformation.SetTotalBytes(transferInformation.GetTotalBytes());
        observers_.ForEach([oppTransferinformation](std::shared_ptr<OppObserver> observer) {
            observer->OnReceiveIncomingFileChanged(oppTransferinformation);
        });
        return NO_ERROR;
    }
    ErrCode OnTransferStateChanged(const BluetoothIOppTransferInformation &transferInformation) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        BluetoothOppTransferInformation oppTransferinformation;
        oppTransferinformation.SetId(transferInformation.GetId());
        oppTransferinformation.SetFileName(transferInformation.GetFileName());
        oppTransferinformation.SetFilePath(transferInformation.GetFilePath());
        oppTransferinformation.SetMimeType(transferInformation.GetFileType());
        oppTransferinformation.SetDeviceName(transferInformation.GetDeviceName());
        oppTransferinformation.SetDeviceAddress(transferInformation.GetDeviceAddress());
        oppTransferinformation.SetFailedReason(transferInformation.GetFailedReason());
        oppTransferinformation.SetStatus(transferInformation.GetStatus());
        oppTransferinformation.SetDirection(transferInformation.GetDirection());
        oppTransferinformation.SetTimeStamp(transferInformation.GetTimeStamp());
        oppTransferinformation.SetCurrentBytes(transferInformation.GetCurrentBytes());
        oppTransferinformation.SetTotalBytes(transferInformation.GetTotalBytes());
        observers_.ForEach([oppTransferinformation](std::shared_ptr<OppObserver> observer) {
            observer->OnTransferStateChanged(oppTransferinformation);
        });
        return NO_ERROR;
    }

private:
    BluetoothObserverList<OppObserver> &observers_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(OppInnerObserver);
};

struct Opp::impl {
    impl();
    ~impl();

    std::vector<BluetoothRemoteDevice> GetDevicesByStates(std::vector<int> states)
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        std::vector<BluetoothRemoteDevice> remoteDevices;
        sptr<IBluetoothOpp> proxy = GetRemoteProxy<IBluetoothOpp>(PROFILE_OPP_SERVER);
        if (proxy != nullptr && IS_BT_ENABLED()) {
            std::vector<BluetoothRawAddress> rawDevices;
            std::vector<int32_t> tmpStates;
            for (int state : states) {
                tmpStates.push_back((int32_t)state);
            }

            proxy->GetDevicesByStates(tmpStates, rawDevices);
            for (BluetoothRawAddress rawDevice : rawDevices) {
                BluetoothRemoteDevice remoteDevice(rawDevice.GetAddress(), 0);
                remoteDevices.push_back(remoteDevice);
            }
        }
        return remoteDevices;
    }

    int GetDeviceState(const BluetoothRemoteDevice &device)
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        sptr<IBluetoothOpp> proxy = GetRemoteProxy<IBluetoothOpp>(PROFILE_OPP_SERVER);
        if (proxy != nullptr && IS_BT_ENABLED() && device.IsValidBluetoothRemoteDevice()) {
            int state;
            proxy->GetDeviceState(BluetoothRawAddress(device.GetDeviceAddr()), state);
            return state;
        }
        return static_cast<int>(BTConnectState::DISCONNECTED);
    }

    bool SendFile(std::string device, std::vector<std::string>filePaths, std::vector<std::string>mimeTypes)
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        sptr<IBluetoothOpp> proxy = GetRemoteProxy<IBluetoothOpp>(PROFILE_OPP_SERVER);
        if (proxy != nullptr && IS_BT_ENABLED()) {
            bool isOk;
            proxy->SendFile(device, filePaths, mimeTypes, isOk);
            return isOk;
        }
        HILOGE("[%{public}s]: %{public}s(): fw return false!", __FILE__, __FUNCTION__);
        return false;
    }

    bool SetIncomingFileConfirmation(bool accept)
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        sptr<IBluetoothOpp> proxy = GetRemoteProxy<IBluetoothOpp>(PROFILE_OPP_SERVER);
        if (proxy != nullptr && IS_BT_ENABLED()) {
            bool isOk;
            proxy->SetIncomingFileConfirmation(accept, isOk);
            return isOk;
        }
        HILOGE("[%{public}s]: %{public}s(): fw return false!", __FILE__, __FUNCTION__);
        return false;
    }

    BluetoothOppTransferInformation GetCurrentTransferInformation()
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        BluetoothIOppTransferInformation oppInformation;
        BluetoothOppTransferInformation transferInformation;
        sptr<IBluetoothOpp> proxy = GetRemoteProxy<IBluetoothOpp>(PROFILE_OPP_SERVER);
        if (proxy != nullptr && IS_BT_ENABLED()) {
            proxy->GetCurrentTransferInformation(oppInformation);
            transferInformation.SetId(oppInformation.GetId());
            transferInformation.SetFileName(oppInformation.GetFileName());
            transferInformation.SetFilePath(oppInformation.GetFilePath());
            transferInformation.SetMimeType(oppInformation.GetFileType());
            transferInformation.SetDeviceName(oppInformation.GetDeviceName());
            transferInformation.SetDeviceAddress(oppInformation.GetDeviceAddress());
            transferInformation.SetFailedReason(oppInformation.GetFailedReason());
            transferInformation.SetStatus(oppInformation.GetStatus());
            transferInformation.SetDirection(oppInformation.GetDirection());
            transferInformation.SetTimeStamp(oppInformation.GetTimeStamp());
            transferInformation.SetCurrentBytes(oppInformation.GetCurrentBytes());
            transferInformation.SetTotalBytes(oppInformation.GetTotalBytes());
        }
        return transferInformation;
    }

    bool CancelTransfer()
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        sptr<IBluetoothOpp> proxy = GetRemoteProxy<IBluetoothOpp>(PROFILE_OPP_SERVER);
        if (proxy != nullptr && IS_BT_ENABLED()) {
            bool isOk;
            proxy->CancelTransfer(isOk);
            return isOk;
        }
        HILOGE("[%{public}s]: %{public}s(): fw return false!", __FILE__, __FUNCTION__);
        return false;
    }

    void RegisterObserver(std::shared_ptr<OppObserver> observer)
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        observers_.Register(observer);
    }

    void DeregisterObserver(std::shared_ptr<OppObserver> observer)
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        observers_.Deregister(observer);
    }

    int32_t profileRegisterId;
private:
    BluetoothObserverList<OppObserver> observers_;
    sptr<OppInnerObserver> innerObserver_;
};

Opp::impl::impl()
{
    innerObserver_ = new OppInnerObserver(observers_);
    profileRegisterId = Singleton<BluetoothProfileManager>::GetInstance().RegisterFunc(PROFILE_OPP_SERVER,
        [this](sptr<IRemoteObject> remote) {
        sptr<IBluetoothOpp> proxy = iface_cast<IBluetoothOpp>(remote);
        CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
        proxy->RegisterObserver(innerObserver_);
    });
}

Opp::impl::~impl()
{
    HILOGD("start");
    Singleton<BluetoothProfileManager>::GetInstance().DeregisterFunc(profileRegisterId);
    sptr<IBluetoothOpp> proxy = GetRemoteProxy<IBluetoothOpp>(PROFILE_OPP_SERVER);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
    proxy->DeregisterObserver(innerObserver_);
}

Opp::Opp()
{
    pimpl = std::make_unique<impl>();
}

Opp::~Opp()
{}

Opp *Opp::GetProfile()
{
    static Opp instance;
    return &instance;
}
std::vector<BluetoothRemoteDevice> Opp::GetDevicesByStates(std::vector<int> states)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return std::vector<BluetoothRemoteDevice>();
    }
    sptr<IBluetoothOpp> proxy = GetRemoteProxy<IBluetoothOpp>(PROFILE_OPP_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr,
        std::vector<BluetoothRemoteDevice>(), "failed: no proxy");

    return pimpl->GetDevicesByStates(states);
}

int Opp::GetDeviceState(const BluetoothRemoteDevice &device)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return static_cast<int>(BTConnectState::DISCONNECTED);
    }
    sptr<IBluetoothOpp> proxy = GetRemoteProxy<IBluetoothOpp>(PROFILE_OPP_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr,
        static_cast<int>(BTConnectState::DISCONNECTED), "failed: no proxy");

    return pimpl->GetDeviceState(device);
}

bool Opp::SendFile(std::string device, std::vector<std::string>filePaths, std::vector<std::string>mimeTypes)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }
    sptr<IBluetoothOpp> proxy = GetRemoteProxy<IBluetoothOpp>(PROFILE_OPP_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");

    return pimpl->SendFile(device, filePaths, mimeTypes);
}

bool Opp::SetIncomingFileConfirmation(bool accept)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("pimpl or opp proxy is nullptr");
        return false;
    }

    sptr<IBluetoothOpp> proxy = GetRemoteProxy<IBluetoothOpp>(PROFILE_OPP_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");

    return pimpl->SetIncomingFileConfirmation(accept);
}

BluetoothOppTransferInformation Opp::GetCurrentTransferInformation()
{
    BluetoothOppTransferInformation transferInformation;
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return transferInformation;
    }

    sptr<IBluetoothOpp> proxy = GetRemoteProxy<IBluetoothOpp>(PROFILE_OPP_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, transferInformation, "failed: no proxy");

    transferInformation = pimpl->GetCurrentTransferInformation();
    return transferInformation;
}

bool Opp::CancelTransfer()
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    sptr<IBluetoothOpp> proxy = GetRemoteProxy<IBluetoothOpp>(PROFILE_OPP_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");

    return pimpl->CancelTransfer();
}

void Opp::RegisterObserver(std::shared_ptr<OppObserver> observer)
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG(pimpl != nullptr, "pimpl is null.");
    pimpl->RegisterObserver(observer);
}

void Opp::DeregisterObserver(std::shared_ptr<OppObserver> observer)
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG(pimpl != nullptr, "pimpl is null.");
    pimpl->DeregisterObserver(observer);
}
}  // namespace Bluetooth
}  // namespace OHOS