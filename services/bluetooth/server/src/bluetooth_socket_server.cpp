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
#ifndef LOG_TAG
#define LOG_TAG "bt_server_socket"
#endif

#include "bluetooth_socket_server.h"
#include "bluetooth_device_manager.h"

#include <sys/socket.h>
#include <unistd.h>

#include "bluetooth_log.h"
#include "bluetooth_utils_server.h"
#include "bt_def.h"
#include "interface_profile_manager.h"
#include "interface_profile_socket.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_application_container.h"
#include "ipc_skeleton.h"

using namespace OHOS::bluetooth;

namespace OHOS {
namespace Bluetooth {
struct BluetoothSocketApplication : public BluetoothApplication {
    BluetoothSocketApplication(int pid, int uid, const sptr<IRemoteObject> &remote)
        : BluetoothApplication(pid, uid, remote) {}
    ~BluetoothSocketApplication() override = default;

    int socketFd = -1;
};

class BluetoothSocketApplicationContainer : public BluetoothApplicationContainer<BluetoothSocketApplication> {
public:
    BluetoothSocketApplicationContainer() : BluetoothApplicationContainer("BluetoothSocketContainer") {};
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override
    {
        if (!Contain(remote)) {
            HILOGE("Invalid remote object");
            return;
        }
        int pid = GetRemotePid(remote);
        int uid = GetRemoteUid(remote);
        HILOGI("OnRemoteDied pid(%{public}d), uid(%{public}d)", pid, uid);

        RemoveRemoteObject(remote);
    }

    void SetSocketFd(const wptr<IRemoteObject> &remote, int fd)
    {
        std::lock_guard<std::mutex> lock(containerMutex_);
        auto it = GetApplicationIter(remote);
        if (it == container_.end()) {
            HILOGE("Unknown remote");
            return;
        }
        it->socketFd = fd;
    }

    int GetSocketFd(const wptr<IRemoteObject> &remote)
    {
        std::lock_guard<std::mutex> lock(containerMutex_);
        auto it = GetApplicationIter(remote);
        if (it == container_.end()) {
            HILOGE("Unknown remote");
            return -1;
        }
        return it->socketFd;
    }
};

static std::unique_ptr<BluetoothSocketApplicationContainer> g_socketContainer = nullptr;

struct BluetoothSocketServer::impl {
    class BtClientSocketCallbackImpl;
    impl();
    ~impl();
    std::list<std::shared_ptr<BtClientSocketCallbackImpl>> observers_;
    IProfileSocket *socketService_ = nullptr;
    std::mutex registerMutex_;

    IProfileSocket *GetServicePtr()
    {
        if (IProfileManager::GetInstance() == nullptr) {
            return nullptr;
        }
        return static_cast<IProfileSocket *>(
            IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_SPP));
    }
};

BluetoothSocketServer::impl::impl()
{
}

BluetoothSocketServer::impl::~impl()
{
}
class BluetoothSocketServer::impl::BtClientSocketCallbackImpl : public IBtClientSocketCallback {
public:
    __attribute__((no_sanitize("cfi")))
    void OnConnectionStateCallback(const CallbackParam &callbackParam) override
    {
        IBluetoothClientSocketObserver::CallbackParam param = {
            .dev = BluetoothRawAddress(callbackParam.addr.GetAddress()),
            .uuid = callbackParam.uuid,
            .status = callbackParam.status,
            .result = callbackParam.result,
            .type = callbackParam.type,
            .psm = callbackParam.psm,
        };
        callback_->OnConnectionStateChanged(param);
    }

    sptr<IBluetoothClientSocketObserver> GetCallback()
    {
        return callback_;
    }

    BluetoothRawAddress GetDev()
    {
        return dev_;
    }

    bluetooth::Uuid GetDevUuid()
    {
        return uuid_;
    }

    void SetSocketFd(int fd)
    {
        socketFd = fd;
    }

    int GetSocketFd()
    {
        return socketFd;
    }

    BtClientSocketCallbackImpl(const sptr<IBluetoothClientSocketObserver> &callback, BluetoothSocketServer &owner,
        const BluetoothRawAddress &dev, const bluetooth::Uuid uuid);
    ~BtClientSocketCallbackImpl() override
    {
        if (!callback_->AsObject()->RemoveDeathRecipient(deathRecipient_)) {
            HILOGE("Failed to unlink death recipient to callback");
        }
        callback_ = nullptr;
        deathRecipient_ = nullptr;
    }
private:
    class CallbackDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        CallbackDeathRecipient(const sptr<IBluetoothClientSocketObserver> &callback, BluetoothSocketServer &owner);

        sptr<IBluetoothClientSocketObserver> GetCallback() const
        {
            return callback_;
        };

        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

    private:
        sptr<IBluetoothClientSocketObserver> callback_;
        BluetoothSocketServer &owner_;
    };
    sptr<IBluetoothClientSocketObserver> callback_;
    sptr<CallbackDeathRecipient> deathRecipient_;
    BluetoothRawAddress dev_;
    bluetooth::Uuid uuid_;
    int socketFd = -1;
};

BluetoothSocketServer::impl::BtClientSocketCallbackImpl::BtClientSocketCallbackImpl(
    const sptr<IBluetoothClientSocketObserver> &callback, BluetoothSocketServer &owner,
    const BluetoothRawAddress &dev, const bluetooth::Uuid uuid)
    : callback_(callback), deathRecipient_(new CallbackDeathRecipient(callback, owner)), dev_(dev), uuid_(uuid)
{
    if (!callback_->AsObject()->AddDeathRecipient(deathRecipient_)) {
        HILOGE("Failed to link death recipient to callback");
    }
}

BluetoothSocketServer::impl::BtClientSocketCallbackImpl::CallbackDeathRecipient::CallbackDeathRecipient(
    const sptr<IBluetoothClientSocketObserver> &callback, BluetoothSocketServer &owner)
    : callback_(callback), owner_(owner)
{}

void BluetoothSocketServer::impl::BtClientSocketCallbackImpl::CallbackDeathRecipient::OnRemoteDied(
    const wptr<IRemoteObject> &remote)
{
    if (owner_.pimpl == nullptr || owner_.pimpl->socketService_ == nullptr) {
        HILOGE(" socketService_ is not support.");
        return;
    }

    std::shared_ptr<BtClientSocketCallbackImpl>socketCallback = nullptr;
    {
        std::lock_guard<std::mutex> lck(owner_.pimpl->registerMutex_);
        auto it = std::find_if(owner_.pimpl->observers_.begin(), owner_.pimpl->observers_.end(),
            [&remote](const auto &p) {
            return p->GetCallback()->AsObject() == remote;
        });
        if (it == owner_.pimpl->observers_.end()) {
            HILOGW("No callback found from callbacks");
            return;
        }
        HILOGI("callback is found from callbacks");
        socketCallback = *it;
    }
    // clear socket resources
    owner_.DeregisterClientObserver(socketCallback->GetDev(), socketCallback->GetDevUuid(),
        socketCallback->GetCallback());
    return;
}

BluetoothSocketServer::BluetoothSocketServer()
{
    g_socketContainer = std::make_unique<BluetoothSocketApplicationContainer>();
    g_socketContainer->Init();
    pimpl = std::make_unique<BluetoothSocketServer::impl>();
}

BluetoothSocketServer::~BluetoothSocketServer()
{
    g_socketContainer = nullptr;
}

int BluetoothSocketServer::Connect(ConnectSocketParam &param, int &fd)
{
    HILOGI("addr: %{public}s, securityFlag: %{public}d, type: %{public}d, psm: %{public}d",
        GetEncryptAddr(param.addr).c_str(),
        param.securityFlag,
        param.type,
        param.psm);
    RawAddress device(param.addr);
    bluetooth::Uuid uuidtemp = param.uuid;

    if (pimpl->socketService_ != nullptr) {
        RawAddress realAddr = BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(device);
        fd = pimpl->socketService_->Connect(realAddr.GetAddress(), param.uuid, (int)param.securityFlag,
            (int)param.type, param.psm);
    }
    if (fd != -1) {
        std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
        auto it = std::find_if(pimpl->observers_.begin(), pimpl->observers_.end(),
            [device, uuidtemp](const auto &p) {
                return p->GetDev().GetAddress() == device.GetAddress() && p->GetDevUuid() == uuidtemp;
            });
        if (it != pimpl->observers_.end()) {
            (*it)->SetSocketFd(fd);
            return NO_ERROR;
        }
        HILOGW("no match dev");
    }
    return BT_ERR_INTERNAL_ERROR;
}

int BluetoothSocketServer::Listen(ListenSocketParam &param, int &fd)
{
    HILOGD("name: %{public}s, securityFlag: %{public}d, type: %{public}d", param.name.c_str(),
        param.securityFlag, param.type);
    IProfileSocket *socket = (IProfileSocket *)IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_SPP);
    if (socket != nullptr) {
        fd = socket->Listen(param.name, param.uuid, (int)param.securityFlag, (int)param.type, (int)param.psm);
    }
    if (fd != -1) {
        int pid = IPCSkeleton::GetCallingPid();
        int uid = IPCSkeleton::GetCallingUid();
        if (!param.observer) {
            HILOGE("observer is nullptr.");
            return BT_ERR_INTERNAL_ERROR;
        }
        g_socketContainer->AddRemoteObject(pid, uid, param.observer->AsObject());
        g_socketContainer->SetSocketFd(param.observer->AsObject(), fd);
    }

    return NO_ERROR;
}

int BluetoothSocketServer::DeregisterServerObserver(const sptr<IBluetoothServerSocketObserver> &observer)
{
    if (observer == nullptr || g_socketContainer == nullptr) {
        HILOGE("Input param is nullptr.");
        return BT_ERR_INTERNAL_ERROR;
    }
    g_socketContainer->RemoveRemoteObject(observer->AsObject());
    return BT_NO_ERROR;
}

int BluetoothSocketServer::UpdateCocConnectionParams(const BluetoothSocketCocInfo &info)
{
    HILOGI("addr: %{public}s", GetEncryptAddr(info.addr).c_str());
    IProfileSocket *socket = static_cast<IProfileSocket *>(
        IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_SPP));
    if (socket != nullptr) {
        socket->UpdateCocConnectionParams(info);
        return BT_NO_ERROR;
    }
    return BT_ERR_INTERNAL_ERROR;
}

int BluetoothSocketServer::RegisterClientObserver(const BluetoothRawAddress &addr, const bluetooth::Uuid uuid,
    const sptr<IBluetoothClientSocketObserver> &observer)
{
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    pimpl->socketService_ = pimpl->GetServicePtr();
    if (!pimpl->socketService_) {
        HILOGE("socketService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    auto it = pimpl->observers_.emplace(pimpl->observers_.begin(),
        std::make_shared<impl::BtClientSocketCallbackImpl>(observer, *this, addr, uuid));

    RawAddress device(addr.GetAddress());
    RawAddress realAddr = BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(device);

    return pimpl->socketService_->RegisterConnectionObserver(realAddr.GetAddress(), uuid, *it);
}

int BluetoothSocketServer::DeregisterClientObserver(const BluetoothRawAddress &addr, const bluetooth::Uuid uuid,
    const sptr<IBluetoothClientSocketObserver> &observer)
{
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    auto it = std::find_if(pimpl->observers_.begin(), pimpl->observers_.end(),
        [addr, uuid](const auto &p) { return p->GetDev() == addr && p->GetDevUuid() == uuid; });
    if (it == pimpl->observers_.end()) {
        HILOGE("addr or uuid wrong");
        return BT_ERR_INTERNAL_ERROR;
    }

    RawAddress device(addr.GetAddress());
    RawAddress realAddr = BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(device);
    int ret = pimpl->socketService_->UnregisterConnectionObserver(realAddr.GetAddress(), uuid, *it);
    pimpl->observers_.erase(it);
    if (ret != BT_NO_ERROR) {
        ret = BT_ERR_INTERNAL_ERROR;
    }
    return ret;
}

int BluetoothSocketServer::IsAllowSocketConnect(int32_t socketType, const std::string &addr, bool &isAllowed)
{
    HILOGI("socketType: %{public}d, addr: %{public}s", socketType, GetEncryptAddr(addr).c_str());
    IProfileSocket *socket = static_cast<IProfileSocket *>(
        IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_SPP));
    CHECK_AND_RETURN_LOG_RET(socket != nullptr, BT_ERR_INTERNAL_ERROR, "socketService_ is null");
    isAllowed = socket->IsAllowSocketConnect(socketType, addr);
    return BT_NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS