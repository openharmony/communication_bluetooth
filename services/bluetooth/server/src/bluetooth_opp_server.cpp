/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_server_opp"
#endif

#include "bluetooth_def.h"
#include "bluetooth_device_manager.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "bluetooth_observer_application_container.h"
#include "bluetooth_utils_server.h"
#include "common_util.h"
#include "interface_adapter_manager.h"
#include "i_bluetooth_opp.h"
#include "interface_profile_manager.h"
#include "interface_profile_opp.h"
#include "ipc_skeleton.h"
#include "permission_manager.h"
#include "remote_observer_list.h"
#include "bluetooth_opp_server.h"
#include "safe_map.h"
#ifdef RES_SCHED_SUPPORT
#include "res_type.h"
#include "res_sched_client.h"
#endif

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;
struct BluetoothOppServer::impl {
    impl();
    RemoteObserverList<IBluetoothOppObserver> observers_;

    class SystemStateObserver;
    std::unique_ptr<SystemStateObserver> systemStateObserver_{nullptr};

    class OppObserver;
    std::unique_ptr<OppObserver> observerImp_{nullptr};

    std::shared_ptr<BluetoothObserverApplicationContainer> appContainer_ = nullptr;

    bluetooth::IProfileOpp* oppService_{nullptr};
};

class BluetoothOppServer::impl::OppObserver : public bluetooth::IOppObserver {
public:
    explicit OppObserver(BluetoothOppServer::impl *pimpl,
        RemoteObserverList<IBluetoothOppObserver> *observers) : observers_(observers), pimpl_(pimpl) {};
    ~OppObserver() override = default;
    void OnReceiveIncomingFile(const IOppTransferInformation &transferInformation) override
    {
        observers_->ForEach([this, transferInformation](IBluetoothOppObserver* observer) {
            uint64_t tokenId = this->pimpl_->appContainer_->GetTokenId(observer->AsObject());
            CHECK_AND_RETURN_LOG(PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId),
                "[PERMISSION] check ACCESS_BLUETOOTH permission failed, callingName(%{public}s)",
                PermissionManager::GetCallingName(tokenId).c_str());
            observer->OnReceiveIncomingFileChanged(BluetoothIOppTransferInformation(transferInformation));
        });
        OppContinuousTask("connect");
    }
    void OnTransferStateChange(const IOppTransferInformation &transferInformation) override
    {
        observers_->ForEach([this, transferInformation](IBluetoothOppObserver* observer) {
            uint64_t tokenId = this->pimpl_->appContainer_->GetTokenId(observer->AsObject());
            CHECK_AND_RETURN_LOG(PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId),
                "[PERMISSION] check ACCESS_BLUETOOTH permission failed, callingName(%{public}s)",
                PermissionManager::GetCallingName(tokenId).c_str());
            observer->OnTransferStateChanged(BluetoothIOppTransferInformation(transferInformation));

        });
        constexpr int OPP_TRANSFER_STATUS_FINISH = 2;
        if (transferInformation.GetStatus() == OPP_TRANSFER_STATUS_FINISH &&
            transferInformation.GetCurrentCount() == transferInformation.GetTotalCount()) {
            OppContinuousTask("close");
        } else {
            OppContinuousTask("connect");
        }
    }
 
    void SetObserver(RemoteObserverList<IBluetoothOppObserver> *observers)
    {
        observers_ = observers;
    }

    void OppContinuousTask(const std::string &action) override
    {
#ifdef RES_SCHED_SUPPORT
        HILOGI("OppContinuousTask action is %{public}s", action.c_str());
        observers_->ForEach([this, &action](IBluetoothOppObserver* observer) {
            int pid = this->pimpl_->appContainer_->GetRemotePid(observer->AsObject());
            int uid = this->pimpl_->appContainer_->GetRemoteUid(observer->AsObject());
            std::unordered_map<std::string, std::string> payload;
            payload["ACTION"] = action;
            payload["ID"] = "-1";
            payload["ADDRESS"] = "empty";
            payload["PID"] = std::to_string(pid);
            payload["UID"] = std::to_string(uid);
            ResourceSchedule::ResSchedClient::GetInstance().ReportData(
                OHOS::ResourceSchedule::ResType::RES_TYPE_BT_SERVICE_EVENT,
                OHOS::ResourceSchedule::ResType::BtServiceEvent::SPP_CONNECT_STATE,
                payload);
        });
    }
#endif

private:
    RemoteObserverList<IBluetoothOppObserver>* observers_ = nullptr;
    BluetoothOppServer::impl *pimpl_ = nullptr;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(OppObserver);
};

class BluetoothOppServer::impl::SystemStateObserver : public bluetooth::ISystemStateObserver {
public:
    explicit SystemStateObserver(BluetoothOppServer::impl* impl) : impl_(impl) {};
    ~SystemStateObserver() override = default;

    void OnSystemStateChange(const bluetooth::BTSystemState state) override
    {
        HILOGI("OnSystemStateChange state=%{public}d", static_cast<int>(state));
        switch (state) {
            case bluetooth::BTSystemState::ON: {
                bluetooth::IProfileManager* serviceManager = bluetooth::IProfileManager::GetInstance();
                CHECK_AND_RETURN_LOG((serviceManager != nullptr), "can't find ProfileServiceManager");
                bluetooth::IProfile* profileService =
                    serviceManager->GetProfileService(bluetooth::PROFILE_NAME_OPP);
                CHECK_AND_RETURN_LOG((profileService != nullptr), "can't find OppService");
                impl_->oppService_ = static_cast<bluetooth::IProfileOpp*>(profileService);
                impl_->oppService_->RegisterObserver(*impl_->observerImp_);
                break;
            }
            case bluetooth::BTSystemState::OFF:
                impl_->oppService_ = nullptr;
                break;
            default:
                break;
            }
    }

private:
    BluetoothOppServer::impl* impl_;
};

BluetoothOppServer::impl::impl()
{
    systemStateObserver_ = std::make_unique<SystemStateObserver>(this);
    bluetooth::IAdapterManager::GetInstance()->RegisterSystemStateObserver(*systemStateObserver_);
    appContainer_ = std::make_shared<BluetoothObserverApplicationContainer>();
    appContainer_->Init();
}

BluetoothOppServer::BluetoothOppServer()
{
    HILOGI("BluetoothOppServer!");
    pimpl = std::make_unique<impl>();
    pimpl->observerImp_ = std::make_unique<impl::OppObserver>(pimpl.get(), &(pimpl->observers_));
    pimpl->observerImp_->SetObserver(&(pimpl->observers_));

    bluetooth::IProfileManager* serviceManager = bluetooth::IProfileManager::GetInstance();
    CHECK_AND_RETURN_LOG((serviceManager != nullptr), "can't find ProfileServiceManager");

    bluetooth::IProfile* profileService = serviceManager->GetProfileService(bluetooth::PROFILE_NAME_OPP);
    CHECK_AND_RETURN_LOG((profileService != nullptr), "can't find OppService");

    CHECK_AND_RETURN_LOG((pimpl != nullptr), "pimpl is null");
    pimpl->oppService_ = (bluetooth::IProfileOpp*)profileService;
    pimpl->oppService_->RegisterObserver(*pimpl->observerImp_);
}

BluetoothOppServer::~BluetoothOppServer() {}

int BluetoothOppServer::GetDeviceState(const BluetoothRawAddress &device, int32_t &state)
{
    HILOGI("device: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->oppService_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or oppService_ is null");

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(RawAddress(device), realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    state = pimpl->oppService_->GetDeviceState(realAddr);
    HILOGD("state : %{public}d", state);
    return BT_NO_ERROR;
}

int BluetoothOppServer::GetDevicesByStates(const std::vector<int32_t> &states,
    std::vector<BluetoothRawAddress> &rawDevices)
{
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->oppService_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or oppService_ is null");

    auto devices = pimpl->oppService_->GetDevicesByStates(states);
    BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(devices, rawDevices);
    return BT_NO_ERROR;
}

int BluetoothOppServer::SendFile(const std::string &device,
    const std::vector<BluetoothIOppTransferFileHolder> &fileHolders, bool& result)
{
    HILOGI("addr:%{public}s", GET_ENCRYPT_STR_ADDR(device));
    RawAddress realAddr = RawAddress(device);
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->oppService_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or oppService_ is null");
    std::vector<IOppTransferFileHolder> fileHolderList;
    for (BluetoothIOppTransferFileHolder fileHolder : fileHolders) {
        fileHolderList.push_back(IOppTransferFileHolder(fileHolder.GetFilePath(),
            fileHolder.GetFileSize(), fileHolder.GetFileFd()));
    }
    HILOGI("fileHolderList size is :%{public}lu", fileHolderList.size());
    result = pimpl->oppService_->SendFile(realAddr, fileHolderList);
    return result;
}

int32_t BluetoothOppServer::SetIncomingFileConfirmation(bool accept, int fd)
{
    HILOGI("accept:%{public}d fd: %{public}d", accept, fd);
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->oppService_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or oppService_ is null");

    return pimpl->oppService_->SetIncomingFileConfirmation(accept, fd);
}

int BluetoothOppServer::GetCurrentTransferInformation(BluetoothIOppTransferInformation &transferInformation)
{
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->oppService_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or oppService_ is null");
    transferInformation = BluetoothIOppTransferInformation(pimpl->oppService_->GetCurrentTransferInformation());
    return BT_NO_ERROR;
}

int BluetoothOppServer::CancelTransfer()
{
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->oppService_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or oppService_ is null");

    return pimpl->oppService_->CancelTransfer();
}

int32_t BluetoothOppServer::SetLastReceivedFileUri(const std::string &uri)
{
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->oppService_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or oppService_ is null");

    return pimpl->oppService_->SetLastReceivedFileUri(uri);
}

void BluetoothOppServer::RegisterObserver(const sptr<IBluetoothOppObserver>& observer)
{
    CHECK_AND_RETURN_LOG((observer != nullptr), "observer is null");
    CHECK_AND_RETURN_LOG((pimpl != nullptr), "pimpl is null");
    if (pimpl->observers_.Size() > MAX_MAP_SIZE || pimpl->appContainer_->Size() > MAX_MAP_SIZE) {
        HILOGE("observers_ or appContainer_ too much");
        return;
    }
    pimpl->observers_.Register(observer);
    pimpl->appContainer_->AddObject(observer->AsObject());
}

void BluetoothOppServer::DeregisterObserver(const sptr<IBluetoothOppObserver>& observer)
{
    CHECK_AND_RETURN_LOG((observer != nullptr), "observer is null");
    CHECK_AND_RETURN_LOG((pimpl != nullptr), "pimpl is null");

    pimpl->observers_.Deregister(observer);
    pimpl->appContainer_->RemoveRemoteObject(observer->AsObject());
}

}  // namespace Bluetooth
}  // namespace OHOS