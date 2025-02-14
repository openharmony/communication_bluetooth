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
#ifndef LOG_TAG
#define LOG_TAG "bt_fwk_opp"
#endif

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

BluetoothOppTransferInformation TransferInformation(const BluetoothIOppTransferInformation &other)
{
    BluetoothOppTransferInformation oppTransferinformation;
    oppTransferinformation.SetId(other.GetId());
    oppTransferinformation.SetFileName(other.GetFileName());
    oppTransferinformation.SetFilePath(other.GetFilePath());
    oppTransferinformation.SetMimeType(other.GetFileType());
    oppTransferinformation.SetDeviceName(other.GetDeviceName());
    oppTransferinformation.SetDeviceAddress(other.GetDeviceAddress());
    oppTransferinformation.SetFailedReason(other.GetFailedReason());
    oppTransferinformation.SetStatus(other.GetStatus());
    oppTransferinformation.SetDirection(other.GetDirection());
    oppTransferinformation.SetTimeStamp(other.GetTimeStamp());
    oppTransferinformation.SetCurrentBytes(other.GetCurrentBytes());
    oppTransferinformation.SetTotalBytes(other.GetTotalBytes());
    return oppTransferinformation;
}

class BluetoothOppObserverImpl : public BluetoothOppObserverStub {
public:
    explicit BluetoothOppObserverImpl(BluetoothObserverList<OppObserver> &observers)
        : observers_(observers)
    {}
    ~BluetoothOppObserverImpl() override
    {}

    void OnReceiveIncomingFileChanged(const BluetoothIOppTransferInformation &transferInformation) override
    {
        BluetoothOppTransferInformation oppTransferinformation = TransferInformation(transferInformation);
        observers_.ForEach([oppTransferinformation](std::shared_ptr<OppObserver> observer) {
            observer->OnReceiveIncomingFileChanged(oppTransferinformation);
        });
        return;
    }

    void OnTransferStateChanged(const BluetoothIOppTransferInformation &transferInformation) override
    {
        BluetoothOppTransferInformation oppTransferinformation = TransferInformation(transferInformation);
        observers_.ForEach([oppTransferinformation](std::shared_ptr<OppObserver> observer) {
            observer->OnTransferStateChanged(oppTransferinformation);
        });
        return;
    }

private:
    BluetoothObserverList<OppObserver> &observers_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothOppObserverImpl);
};

struct Opp::impl {
    impl()
    {
        serviceObserverImp_ = new BluetoothOppObserverImpl(observers_);
        profileRegisterId = BluetoothProfileManager::GetInstance().RegisterFunc(PROFILE_OPP_SERVER,
        [this](sptr<IRemoteObject> remote) {
            sptr<IBluetoothOpp> proxy = iface_cast<IBluetoothOpp>(remote);
            CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
            proxy->RegisterObserver(serviceObserverImp_);
        });
    }
    ~impl()
    {
        BluetoothProfileManager::GetInstance().DeregisterFunc(profileRegisterId);
        sptr<IBluetoothOpp> proxy = GetRemoteProxy<IBluetoothOpp>(PROFILE_OPP_SERVER);
        CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
        proxy->DeregisterObserver(serviceObserverImp_);
    }
    void RegisterObserver(std::shared_ptr<OppObserver> &observer)
    {
        HILOGI("enter");
        if (observer) {
            observers_.Register(observer);
        }
    }
    void DeregisterObserver(std::shared_ptr<OppObserver> &observer)
    {
        HILOGI("enter");
        if (observer) {
            observers_.Deregister(observer);
        }
    }
    int32_t profileRegisterId = 0;
private:
    BluetoothObserverList<OppObserver> observers_;
    sptr<BluetoothOppObserverImpl> serviceObserverImp_ = nullptr;
};

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

BluetoothOppFileHolder::BluetoothOppFileHolder()
{}

BluetoothOppFileHolder::BluetoothOppFileHolder(const std::string &filePath,
    const int64_t &fileSize, const int32_t &fileFd)
{
    filePath_ = filePath;
    fileSize_ = fileSize;
    fileFd_ = fileFd;
}

BluetoothOppFileHolder::~BluetoothOppFileHolder()
{}

std::string BluetoothOppFileHolder::GetFilePath() const
{
    return filePath_;
}

int64_t BluetoothOppFileHolder::GetFileSize() const
{
    return fileSize_;
}

int32_t BluetoothOppFileHolder::GetFileFd() const
{
    return fileFd_;
}

void BluetoothOppFileHolder::SetFilePath(const std::string &filePath)
{
    filePath_ = filePath;
}

void BluetoothOppFileHolder::SetFileFd(const int32_t &fileFd)
{
    fileFd_ = fileFd;
}

void BluetoothOppFileHolder::SetFileSize(const int64_t &fileSize)
{
    fileSize_ = fileSize;
}

Opp::Opp()
{
    pimpl = std::make_unique<impl>();
}

Opp::~Opp()
{}

Opp *Opp::GetProfile()
{
#ifdef DTFUZZ_TEST
    static BluetoothNoDestructor<Opp> instance;
    return instance.get();
#else
    static Opp instance;
    return &instance;
#endif
}

int32_t Opp::GetDevicesByStates(const std::vector<int32_t> &states,
    std::vector<BluetoothRemoteDevice> &result) const
{
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothOpp> proxy = GetRemoteProxy<IBluetoothOpp>(PROFILE_OPP_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");

    std::vector<BluetoothRawAddress> rawAddress {};
    int32_t ret = proxy->GetDevicesByStates(states, rawAddress);
    CHECK_AND_RETURN_LOG_RET((ret == BT_NO_ERROR), ret, "inner error");

    for (BluetoothRawAddress rawAddr : rawAddress) {
        BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);
        result.push_back(device);
    }
    return BT_NO_ERROR;
}

int32_t Opp::GetDeviceState(const BluetoothRemoteDevice &device, int32_t &state) const
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothOpp> proxy = GetRemoteProxy<IBluetoothOpp>(PROFILE_OPP_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "device param error");

    return proxy->GetDeviceState(BluetoothRawAddress(device.GetDeviceAddr()), state);
}

int32_t Opp::SendFile(std::string device, std::vector<BluetoothOppFileHolder> fileHolders, bool& result)
{
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothOpp> proxy = GetRemoteProxy<IBluetoothOpp>(PROFILE_OPP_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");
    std::vector<BluetoothIOppTransferFileHolder> fileHoldersInterface;
    for (BluetoothOppFileHolder fileHolder : fileHolders) {
        fileHoldersInterface.push_back(BluetoothIOppTransferFileHolder(fileHolder.GetFilePath(),
            fileHolder.GetFileSize(), fileHolder.GetFileFd()));
    }
    int ret = proxy->SendFile(device, fileHoldersInterface, result);
    HILOGI("send file result is : %{public}d", result);
    return ret;
}

int32_t Opp::SetLastReceivedFileUri(const std::string &uri)
{
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothOpp> proxy = GetRemoteProxy<IBluetoothOpp>(PROFILE_OPP_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");
    int ret = proxy->SetLastReceivedFileUri(uri);
    HILOGI("setLastReceivedFileUri is : %{public}d", ret);
    return ret;
}

int32_t Opp::SetIncomingFileConfirmation(bool accept, int32_t fileFd)
{
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");

    sptr<IBluetoothOpp> proxy = GetRemoteProxy<IBluetoothOpp>(PROFILE_OPP_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");
    int ret = proxy->SetIncomingFileConfirmation(accept, fileFd);
    HILOGI("setIncomingFileConfirmation result is : %{public}d", ret);
    return ret;
}

int32_t Opp::GetCurrentTransferInformation(BluetoothOppTransferInformation &transferInformation)
{
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothOpp> proxy = GetRemoteProxy<IBluetoothOpp>(PROFILE_OPP_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");

    BluetoothIOppTransferInformation oppInformation;
    int ret = proxy->GetCurrentTransferInformation(oppInformation);
    HILOGI("getCurrentTransferInformation result is : %{public}d", ret);
    if (ret == BT_NO_ERROR) {
        transferInformation = TransferInformation(oppInformation);
    }
    return ret;
}

int32_t Opp::CancelTransfer(bool &result)
{
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothOpp> proxy = GetRemoteProxy<IBluetoothOpp>(PROFILE_OPP_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");

    int ret = proxy->CancelTransfer(result);
    HILOGI("cancelTransfer result is : %{public}d", ret);
    return ret;
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