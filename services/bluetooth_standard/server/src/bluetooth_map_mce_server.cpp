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
#include "bluetooth_map_mce_server.h"
#include "interface_adapter_manager.h"
#include "interface_profile_manager.h"
#include "interface_profile_map_mce.h"
#include "bluetooth_log.h"
#include "remote_observer_list.h"

using namespace bluetooth;

namespace OHOS {
namespace Bluetooth {
/**
 * @brief make oberser for framework
 */
class BluetoothMapMceObserverImpl : public IProfileMapMceObserver {
public:
    explicit BluetoothMapMceObserverImpl(RemoteObserverList<IBluetoothMapMceObserver> *observerList);
    ~BluetoothMapMceObserverImpl();
    void OnMapActionCompleted(
        const RawAddress &deviceAddress, const IProfileMapAction &action, MapExecuteStatus status) override;
    void OnMapEventReported(const RawAddress &deviceAddress, const IProfileMapEventReport &report) override;
    void OnConnectionStateChanged(const RawAddress &deviceAddress, int state) override;
    void OnBmessageCompleted(
        const RawAddress &deviceAddress, const IProfileBMessage &bmsg, MapExecuteStatus status) override;
    void OnMessagesListingCompleted(
        const RawAddress &deviceAddress, const IProfileMessagesListing &listing, MapExecuteStatus status) override;
    void OnConversationListingCompleted(
        const RawAddress &deviceAddress, const IProfileConversationListing &listing, MapExecuteStatus status) override;

private:
    RemoteObserverList<IBluetoothMapMceObserver> *frameworkObserverList_;
};

BluetoothMapMceObserverImpl::BluetoothMapMceObserverImpl(RemoteObserverList<IBluetoothMapMceObserver> *observerList)
{
    frameworkObserverList_ = observerList;
}
BluetoothMapMceObserverImpl::~BluetoothMapMceObserverImpl()
{}
void BluetoothMapMceObserverImpl::OnMapActionCompleted(
    const RawAddress &deviceAddress, const IProfileMapAction &action, MapExecuteStatus status)
{
    BluetoothIProfileMapAction bAction;

    bAction.action_ = action.action_;
    bAction.ownerStatus_ = action.ownerStatus_;
    bAction.supportedFeatures_ = action.supportedFeatures_;

    frameworkObserverList_->ForEach([deviceAddress, bAction, status](IBluetoothMapMceObserver *observer) {
        observer->OnMapActionCompleted(deviceAddress, bAction, (int)status);
    });
}
void BluetoothMapMceObserverImpl::OnMapEventReported(
    const RawAddress &deviceAddress, const IProfileMapEventReport &report)
{
    BluetoothIProfileMapEventReport fEventReport;

    fEventReport.type = report.type;
    fEventReport.handle = report.handle;
    fEventReport.folder = report.folder;
    fEventReport.old_folder = report.old_folder;
    fEventReport.msg_type = report.msg_type;
    fEventReport.datetime = report.datetime;
    fEventReport.subject = report.subject;
    fEventReport.sender_name = report.sender_name;
    fEventReport.priority = report.priority;
    fEventReport.conversation_name = report.conversation_name;
    fEventReport.conversation_id = report.conversation_id;
    fEventReport.presence_availability = report.presence_availability;
    fEventReport.presence_text = report.presence_text;
    fEventReport.last_activity = report.last_activity;
    fEventReport.chat_state = report.chat_state;
    fEventReport.read_status = report.read_status;
    fEventReport.extended_data = report.extended_data;
    fEventReport.participant_uci = report.participant_uci;
    fEventReport.contact_uid = report.contact_uid;
    fEventReport.version = report.version;
    fEventReport.masInstanceId_ = report.masInstanceId_;
    fEventReport.eventReportStringObject_ = report.eventReportStringObject_;

    frameworkObserverList_->ForEach([deviceAddress, fEventReport](IBluetoothMapMceObserver *observer) {
        observer->OnMapEventReported(deviceAddress, fEventReport);
    });
}
void BluetoothMapMceObserverImpl::OnConnectionStateChanged(const RawAddress &deviceAddress, int state)
{
    frameworkObserverList_->ForEach([deviceAddress, state](IBluetoothMapMceObserver *observer) {
        observer->OnConnectionStateChanged(deviceAddress, state);
    });
}
void BluetoothMapMceObserverImpl::OnBmessageCompleted(
    const RawAddress &deviceAddress, const IProfileBMessage &bmsg, MapExecuteStatus status)
{
    int fStatus = (int)status;
    BluetoothIProfileBMessage fmsg;

    fmsg.FractionDeliver = bmsg.FractionDeliver;
    fmsg.bMessageParam_ = bmsg.bMessageParam_;
    fmsg.bMessageStringObject_ = bmsg.bMessageStringObject_;

    frameworkObserverList_->ForEach([deviceAddress, fmsg, fStatus](IBluetoothMapMceObserver *observer) {
        observer->OnBmessageCompleted(deviceAddress, fmsg, fStatus);
    });
}
void BluetoothMapMceObserverImpl::OnMessagesListingCompleted(
    const RawAddress &deviceAddress, const IProfileMessagesListing &listing, MapExecuteStatus status)
{
    int fStatus = (int)status;
    BluetoothIProfileMessagesListing fListing;

    fListing.messageOutlineList_ = listing.messageOutlineList_;
    fListing.messagesListingParam_ = listing.messagesListingParam_;
    fListing.messagesListingStringObject_ = listing.messagesListingStringObject_;

    frameworkObserverList_->ForEach([deviceAddress, fListing, fStatus](IBluetoothMapMceObserver *observer) {
        observer->OnMessagesListingCompleted(deviceAddress, fListing, fStatus);
    });
}
void BluetoothMapMceObserverImpl::OnConversationListingCompleted(
    const RawAddress &deviceAddress, const IProfileConversationListing &listing, MapExecuteStatus status)
{
    int fStatus = (int)status;
    BluetoothIProfileConversationListing fListing;

    fListing.conversationOutlineList_ = listing.conversationOutlineList_;
    fListing.conversationListingParam_ = listing.conversationListingParam_;
    fListing.conversationListingStringObject_ = listing.conversationListingStringObject_;

    frameworkObserverList_->ForEach([deviceAddress, fListing, fStatus](IBluetoothMapMceObserver *observer) {
        observer->OnConversationListingCompleted(deviceAddress, fListing, fStatus);
    });
}

struct BluetoothMapMceServer::impl {
    RemoteObserverList<IBluetoothMapMceObserver> observersList_;
    BluetoothMapMceObserverImpl *observerImp_ = nullptr;
    bluetooth::IProfileMapMce *mceService_ = nullptr;
    class MapMceSystemStateObserver;
    MapMceSystemStateObserver *systemStateObserver_ = nullptr;
};

class BluetoothMapMceServer::impl::MapMceSystemStateObserver : public ISystemStateObserver {
public:
    MapMceSystemStateObserver(BluetoothMapMceServer::impl *impl) : implPtr_(impl) {};
    ~MapMceSystemStateObserver() override = default;
    void OnSystemStateChange(const BTSystemState state) override
    {
        if (state == BTSystemState::ON) {
            IProfileManager *serviceMgr = IProfileManager::GetInstance();
            if (serviceMgr != nullptr) {
                implPtr_->mceService_ = (IProfileMapMce *)serviceMgr->GetProfileService(PROFILE_NAME_MAP_MCE);
                if (implPtr_->mceService_ != nullptr) {
                    implPtr_->mceService_->RegisterObserver(*(implPtr_->observerImp_));  // re regist observer
                }
            }
        } else if (state == BTSystemState::OFF) {
            implPtr_->mceService_ = nullptr;
        } else {
            // erro
        }
    }

private:
    BluetoothMapMceServer::impl *implPtr_;
};

BluetoothMapMceServer::BluetoothMapMceServer()
{
    pimpl = std::make_unique<impl>();

    IProfileManager *serviceMgr = IProfileManager::GetInstance();
    if (serviceMgr != nullptr) {
        pimpl->mceService_ = (IProfileMapMce *)serviceMgr->GetProfileService(PROFILE_NAME_MAP_MCE);
        if (pimpl->mceService_ != nullptr) {
            // register service callback
            pimpl->observerImp_ = new BluetoothMapMceObserverImpl(&(pimpl->observersList_));
            pimpl->mceService_->RegisterObserver(*(pimpl->observerImp_));
        } else {
            // erro
            HILOGI("%{public}s pimpl->mceService_ null", __func__);
        }
    } else {
        // erro
        HILOGI("%{public}s serviceMgr null", __func__);
    }

    pimpl->systemStateObserver_ = new BluetoothMapMceServer::impl::MapMceSystemStateObserver(pimpl.get());
    IAdapterManager::GetInstance()->RegisterSystemStateObserver(*(pimpl->systemStateObserver_));
}

BluetoothMapMceServer::~BluetoothMapMceServer()
{
    IProfileManager *serviceMgr = IProfileManager::GetInstance();
    if (serviceMgr != nullptr) {
        pimpl->mceService_ = (IProfileMapMce *)serviceMgr->GetProfileService(PROFILE_NAME_MAP_MCE);
        if (pimpl->mceService_ != nullptr) {
            pimpl->mceService_->DeregisterObserver(*(pimpl->observerImp_));
        }
        pimpl->mceService_ = nullptr;
    }
    if (pimpl->observerImp_ != nullptr) {
        delete pimpl->observerImp_;
        pimpl->observerImp_ = nullptr;
    }

    IAdapterManager::GetInstance()->DeregisterSystemStateObserver(*(pimpl->systemStateObserver_));
    if (pimpl->systemStateObserver_ != nullptr) {
        delete pimpl->systemStateObserver_;
        pimpl->systemStateObserver_ = nullptr;
    }
}

void BluetoothMapMceServer::RegisterObserver(
    const sptr<IBluetoothMapMceObserver> &observer)
{
    HILOGI("%{public}s start", __func__);
    if (!observer) {
        HILOGE("%{public}s RegisterObserver called with NULL . Ignoring.", __func__);
        return;
    }
    pimpl->observersList_.Register(observer);
    HILOGI("%{public}s end", __func__);
}

void BluetoothMapMceServer::DeregisterObserver(
    const sptr<IBluetoothMapMceObserver> &observer)
{
    HILOGI("%{public}s start", __func__);
    if (!observer) {
        HILOGE("%{public}s UnregisterObserver called with NULL . Ignoring.", __func__);
        return;
    }
    pimpl->observersList_.Deregister(observer);
}

int BluetoothMapMceServer::Connect(
    const BluetoothRawAddress &device)
{
    HILOGI("%{public}s start", __func__);
    int ret = -1;
    if (pimpl->mceService_ != nullptr) {
        ret = pimpl->mceService_->Connect((RawAddress)device);
    } else {
        HILOGE("pimpl->mceService_ null %{public}s", __func__);
    }
    HILOGI("%{public}s end", __func__);
    return ret;
}

int BluetoothMapMceServer::Disconnect(
    const BluetoothRawAddress &device)
{
    HILOGI("%{public}s start", __func__);
    int ret = -1;
    if (pimpl->mceService_ != nullptr) {
        ret =  pimpl->mceService_->Disconnect((RawAddress)device);
    } else {
        HILOGE("pimpl->mceService_ null %{public}s", __func__);
    }
    HILOGI("%{public}s end", __func__);
    return ret;
}

int BluetoothMapMceServer::IsConnected(
    const BluetoothRawAddress &device)
{
    HILOGI("%{public}s start", __func__);
    int ret = -1;
    if (pimpl->mceService_ != nullptr) {
        ret =  (int)pimpl->mceService_->IsConnected((RawAddress)device);
    } else {
        HILOGE("pimpl->mceService_ null %{public}s", __func__);
    }
    HILOGI("%{public}s end", __func__);
    return ret;
}

void BluetoothMapMceServer::GetConnectDevices(
    std::vector<BluetoothRawAddress> &devices)
{
    HILOGI("%{public}s start", __func__);
    if (pimpl->mceService_ != nullptr) {
        std::list<RawAddress> btDevice = pimpl->mceService_->GetConnectDevices();
        for (auto it : btDevice) {
            devices.push_back(BluetoothRawAddress(it));
        }
        pimpl->mceService_->GetConnectDevices();
    } else {
        HILOGE("pimpl->mceService_ null %{public}s", __func__);
    }
    HILOGI("%{public}s end", __func__);
}

void BluetoothMapMceServer::GetDevicesByStates(
    const std::vector<int32_t> &statusList, std::vector<BluetoothRawAddress> &devices)
{
    HILOGI("%{public}s start", __func__);
    std::vector<RawAddress> serviceDeviceList;

    if (pimpl->mceService_ != nullptr) {
        serviceDeviceList = pimpl->mceService_->GetDevicesByStates(statusList);
        RawAddress serviceDevice;
        BluetoothRawAddress *bluetoothDevice;
        for (auto it = serviceDeviceList.begin(); it != serviceDeviceList.end(); it++) {
            serviceDevice = *it;
            bluetoothDevice = new BluetoothRawAddress(serviceDevice.GetAddress());
            devices.push_back(*bluetoothDevice);
            delete bluetoothDevice;
        }
    } else {
        HILOGE("pimpl->mceService_ null %{public}s", __func__);
    }
    HILOGI("%{public}s end", __func__);
}

int BluetoothMapMceServer::GetConnectionState(
    const BluetoothRawAddress &device)
{
    HILOGI("%{public}s start", __func__);
    int ret = -1;
    if (pimpl->mceService_ != nullptr) {
        ret =  pimpl->mceService_->GetDeviceConnectState((RawAddress)device);
    } else {
        HILOGE("pimpl->mceService_ null %{public}s", __func__);
    }
    HILOGI("%{public}s end", __func__);
    return ret;
}

int BluetoothMapMceServer::SetConnectionStrategy(
    const BluetoothRawAddress &device, int32_t strategy)
{
    HILOGI("%{public}s start", __func__);
    HILOGI("%{public}s: strategy = %{public}d ", __func__, strategy);
    int ret = -1;
    if (pimpl->mceService_ != nullptr) {
        ret =  pimpl->mceService_->SetConnectionStrategy((RawAddress)device, strategy);
    } else {
        HILOGE("pimpl->mceService_ null %{public}s", __func__);
    }
    HILOGI("%{public}s end", __func__);
    return ret;
}

int BluetoothMapMceServer::GetConnectionStrategy(
    const BluetoothRawAddress &device)
{
    HILOGI("%{public}s start", __func__);
    int ret = -1;
    if (pimpl->mceService_ != nullptr) {
        ret =  pimpl->mceService_->GetConnectionStrategy((RawAddress)device);
    } else {
        HILOGE("pimpl->mceService_ null %{public}s", __func__);
    }
    HILOGI("%{public}s end", __func__);
    return ret;
}

int BluetoothMapMceServer::GetSupportedFeatures(
    const BluetoothRawAddress &device)
{
    HILOGI("%{public}s start", __func__);
    int ret = -1;
    if (pimpl->mceService_ != nullptr) {
        ret =  pimpl->mceService_->GetSupportedFeatures((RawAddress)device);
    } else {
        HILOGE("pimpl->mceService_ null %{public}s", __func__);
    }
    HILOGI("%{public}s end", __func__);
    return ret;
}

int BluetoothMapMceServer::SetNotificationFilter(
    const BluetoothRawAddress &device, int32_t mask)
{
    HILOGI("%{public}s start", __func__);
    HILOGI("%{public}s: mask = %{public}d ", __func__, mask);
    int ret = -1;
    if (pimpl->mceService_ != nullptr) {
        ret =  pimpl->mceService_->SetNotificationFilter((RawAddress)device, mask);
    } else {
        HILOGE("pimpl->mceService_ null %{public}s", __func__);
    }
    HILOGI("%{public}s end", __func__);
    return ret;
}

int BluetoothMapMceServer::UpdateInbox(
    const BluetoothRawAddress &device, int32_t msgType)
{
    HILOGI("%{public}s start", __func__);
    HILOGI("%{public}s: msgType = %{public}d ", __func__, msgType);
    int ret = -1;
    if (pimpl->mceService_ != nullptr) {
        ret =  pimpl->mceService_->UpdateInbox((RawAddress)device, MapMessageType(msgType));
    } else {
        HILOGE("pimpl->mceService_ null %{public}s", __func__);
    }
    HILOGI("%{public}s end", __func__);
    return ret;
}

int BluetoothMapMceServer::SetMessageStatus(const BluetoothRawAddress &device,
    int32_t msgType, const std::u16string &msgHandle, int32_t statusIndicator, int32_t statusValue,
    const std::string &extendedData)
{
    HILOGI("%{public}s start", __func__);
    HILOGI("%{public}s: msgType = %{public}d, statusIndicator = %{public}d, statusValue = %{public}d ",
             __func__, msgType, statusIndicator, statusValue);
    int ret = -1;
    IProfileSetMessageStatus serviceStatus;
    serviceStatus.msgHandle = msgHandle;
    serviceStatus.statusIndicator = MapStatusIndicatorType(statusIndicator);
    serviceStatus.statusValue = MapStatusValueType(statusValue);
    serviceStatus.extendedData = extendedData;

    if (pimpl->mceService_ != nullptr) {
        ret =
            pimpl->mceService_->SetMessageStatus((RawAddress)device, MapMessageType(msgType), serviceStatus);
    } else {
        HILOGE("pimpl->mceService_ null %{public}s", __func__);
    }
    HILOGI("%{public}s end", __func__);
    return ret;
}

int BluetoothMapMceServer::GetOwnerStatus(const BluetoothRawAddress &device,
    const std::string &conversationId)
{
    HILOGI("%{public}s start", __func__);
    HILOGI("%{public}s: msgType = %{public}s ", __func__, conversationId.c_str());
    int ret = -1;
    if (pimpl->mceService_ != nullptr) {
        ret =  pimpl->mceService_->GetOwnerStatus((RawAddress)device, conversationId);
    } else {
        HILOGE("pimpl->mceService_ null %{public}s", __func__);
        ret =  -1;
    }
    HILOGI("%{public}s end", __func__);
    return ret;
}

int BluetoothMapMceServer::GetUnreadMessages(
    const BluetoothRawAddress &device, int32_t msgType, int32_t max)
{
    HILOGI("%{public}s start", __func__);
    HILOGI("%{public}s: msgType = %{public}d, max = %{public}d ", __func__, msgType, max);
    int ret = -1;
    if (pimpl->mceService_ != nullptr) {
        ret =
            pimpl->mceService_->GetUnreadMessages((RawAddress)device, MapMessageType(msgType), (uint8_t)max);
    } else {
        HILOGE("pimpl->mceService_ null %{public}s", __func__);
    }
    HILOGI("%{public}s end", __func__);
    return ret;
}

int BluetoothMapMceServer::SendMessage(const BluetoothRawAddress &device,
    const BluetoothIProfileSendMessageParameters &msg)
{
    HILOGI("%{public}s start", __func__);
    int ret = -1;
    if (pimpl->mceService_ != nullptr) {
        ret =  pimpl->mceService_->SendMessage((RawAddress)device, (IProfileSendMessageParameters)msg);
    } else {
        HILOGE("pimpl->mceService_ null %{public}s", __func__);
    }
    HILOGI("%{public}s end", __func__);
    return ret;
}

int BluetoothMapMceServer::GetMessagesListing(const BluetoothRawAddress &device,
    const BluetoothIProfileGetMessagesListingParameters &para)
{
    HILOGI("%{public}s start", __func__);
    int ret = -1;
    if (pimpl->mceService_ != nullptr) {
        ret =
            pimpl->mceService_->GetMessagesListing((RawAddress)device, (IProfileGetMessagesListingParameters)para);
    } else {
        HILOGE("pimpl->mceService_ null %{public}s", __func__);
    }
    HILOGI("%{public}s end", __func__);
    return ret;
}

int BluetoothMapMceServer::GetMessage(const BluetoothRawAddress &device, int32_t msgType,
    const std::u16string &msgHandle, const BluetoothIProfileGetMessageParameters &para)
{
    HILOGI("%{public}s start", __func__);
    HILOGI("%{public}s: msgType = %{public}d", __func__, msgType);
    int ret = -1;
    if (pimpl->mceService_ != nullptr) {
        ret =  pimpl->mceService_->GetMessage(
            (RawAddress)device, MapMessageType(msgType), msgHandle, (IProfileGetMessageParameters)para);
    } else {
        HILOGE("pimpl->mceService_ null %{public}s", __func__);
    }
    HILOGI("%{public}s end", __func__);
    return ret;
}

int BluetoothMapMceServer::GetConversationListing(const BluetoothRawAddress &device,
    const BluetoothIProfileGetConversationListingParameters &para)
{
    HILOGI("%{public}s start", __func__);
    int ret = -1;
    if (pimpl->mceService_ != nullptr) {
        ret =  pimpl->mceService_->GetConversationListing(
            (RawAddress)device, (IProfileGetConversationListingParameters)para);
    } else {
        HILOGE("pimpl->mceService_ null %{public}s", __func__);
    }
    HILOGI("%{public}s end", __func__);
    return ret;
}

int BluetoothMapMceServer::SetOwnerStatus(const BluetoothRawAddress &device,
    const BluetoothIProfileSetOwnerStatusParameters &para)
{
    HILOGI("%{public}s start", __func__);
    int ret = -1;
    if (pimpl->mceService_ != nullptr) {
        ret =  pimpl->mceService_->SetOwnerStatus((RawAddress)device, (IProfileSetOwnerStatusParameters)para);
    } else {
        HILOGE("pimpl->mceService_ null %{public}s", __func__);
    }
    HILOGI("%{public}s end", __func__);
    return ret;
}

void BluetoothMapMceServer::GetMasInstanceInfo(const BluetoothRawAddress &device,
    BluetoothIProfileMasInstanceInfoList &bluetoothlist)
{
    HILOGI("%{public}s start", __func__);
    IProfileMasInstanceInfoList serviceInstanceList;
    if (pimpl->mceService_ != nullptr) {
        serviceInstanceList = pimpl->mceService_->GetMasInstanceInfo((RawAddress)device);
        bluetoothlist.isValid = serviceInstanceList.isValid;
        bluetoothlist.masInfoList = serviceInstanceList.masInfoList;
    } else {
        HILOGE("pimpl->mceService_ null %{public}s", __func__);
    }
    HILOGI("%{public}s end", __func__);
}
}  // namespace Bluetooth
}  // namespace OHOS
