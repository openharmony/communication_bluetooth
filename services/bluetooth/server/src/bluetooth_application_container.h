/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_APPLICATION_CONTAINER_H
#define BLUETOOTH_APPLICATION_CONTAINER_H

#include <algorithm>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "log.h"
#include "iremote_object.h"
#include "refbase.h"

namespace OHOS {
namespace Bluetooth {
struct BluetoothApplication {
    BluetoothApplication(int pid, int uid, const sptr<IRemoteObject> &remote)
        : pid(pid), uid(uid), remote(remote) {}
    virtual ~BluetoothApplication() = default;

    int pid;
    int uid;
    wptr<IRemoteObject> remote;
};

template <typename T>
class BluetoothApplicationContainer {
public:
    class DeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit DeathRecipient(BluetoothApplicationContainer<T> &applications) : applications_(applications) {}
        ~DeathRecipient() = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) final;
    private:
        BluetoothApplicationContainer<T> &applications_;
    };

    explicit BluetoothApplicationContainer(const char *typeName) : deathRecipient_(nullptr), typeName_(typeName) {}
    virtual ~BluetoothApplicationContainer() = default;

    void Init(void);
    void Clear(void);
    void AddRemoteObject(int pid, int uid, const sptr<IRemoteObject> &remote);
    void RemoveRemoteObject(const wptr<IRemoteObject> &remote);
    bool Contain(const wptr<IRemoteObject> &remote) const;
    int GetRemoteUid(const wptr<IRemoteObject> &remote) const;
    int GetRemotePid(const wptr<IRemoteObject> &remote) const;
    int Size() const;

    virtual void OnRemoteDied(const wptr<IRemoteObject> &remote) {}

protected:
    auto GetApplicationIter(const wptr<IRemoteObject> &remote)
    {
        return std::find_if(container_.begin(), container_.end(),
            [remote](const auto &app) { return app.remote == remote; });
    }
    auto GetApplicationIter(const wptr<IRemoteObject> &remote) const
    {
        return std::find_if(container_.cbegin(), container_.cend(),
            [remote](const auto &app) { return app.remote == remote; });
    }

    auto GetApplicationIter(int pid, int uid)
    {
        return std::find_if(container_.begin(), container_.end(),
            [pid, uid](const auto &app) { return app.pid == pid && app.uid == uid; });
    }

    mutable std::mutex containerMutex_;
    std::vector<T> container_ {};

private:
    sptr<DeathRecipient> deathRecipient_;
    const char *typeName_ = nullptr;
};

template <typename T>
void BluetoothApplicationContainer<T>::Init(void)
{
    deathRecipient_ = new DeathRecipient(*this);
}

template <typename T>
void BluetoothApplicationContainer<T>::AddRemoteObject(int pid, int uid, const sptr<IRemoteObject> &remote)
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    HILOGD("%{public}s: before appInfos size: %{public}zu", typeName_, container_.size());
    if (GetApplicationIter(remote) != container_.end()) {
        HILOGE("%{public}s: remote exist pid(%{public}d), uid(%{public}d)", typeName_, pid, uid);
        return;
    }

    T app(pid, uid, remote);
    if (!app.remote->AddDeathRecipient(deathRecipient_)) {
        HILOGE("failed to add deathRecipient");
    }
    container_.push_back(app);
    HILOGD("pid: %{public}d, uid: %{public}d, current size(%{public}zu)", app.pid, app.uid, container_.size());

    HILOGD("%{public}s: after appInfos size: %{public}zu", typeName_, container_.size());
}

template <typename T>
void BluetoothApplicationContainer<T>::RemoveRemoteObject(const wptr<IRemoteObject> &remote)
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    HILOGD("%{public}s: before appInfos size: %{public}zu", typeName_, container_.size());
    auto it = GetApplicationIter(remote);
    if (it != container_.end()) {
        int pid = it->pid;
        int uid = it->uid;
        it->remote->RemoveDeathRecipient(deathRecipient_);
        container_.erase(it);
        HILOGD("pid: %{public}d, uid: %{public}d, current size(%{public}zu)", pid, uid, container_.size());
    }
    HILOGD("%{public}s: after appInfos size: %{public}zu", typeName_, container_.size());
}

template <typename T>
void BluetoothApplicationContainer<T>::Clear(void)
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    for (auto &it : container_) {
        it.remote->RemoveDeathRecipient(deathRecipient_);
    }
    container_.clear();
}

template <typename T>
bool BluetoothApplicationContainer<T>::Contain(const wptr<IRemoteObject> &remote) const
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = GetApplicationIter(remote);
    return it != container_.cend();
}

template <typename T>
int BluetoothApplicationContainer<T>::GetRemoteUid(const wptr<IRemoteObject> &remote) const
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = GetApplicationIter(remote);
    if (it == container_.end()) {
        HILOGE("Invalid remote");
        return 0;
    }
    return it->uid;
}

template <typename T>
int BluetoothApplicationContainer<T>::GetRemotePid(const wptr<IRemoteObject> &remote) const
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = GetApplicationIter(remote);
    if (it == container_.end()) {
        HILOGE("Invalid remote");
        return 0;
    }
    return it->pid;
}

template <typename T>
void BluetoothApplicationContainer<T>::DeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    HILOGD("typeName: %{public}s", applications_.typeName_);
    applications_.OnRemoteDied(remote);
}

template <typename T>
int BluetoothApplicationContainer<T>::Size() const
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    return static_cast<int>(container_.size());
}
} // namespace Bluetooth
} // namespace OHOS
#endif  // BLUETOOTH_APPLICATION_CONTAINER_H