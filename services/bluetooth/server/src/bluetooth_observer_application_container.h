/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_OBSERVER_APPLICATION_CONTAINER_H
#define BLUETOOTH_OBSERVER_APPLICATION_CONTAINER_H

#include "bluetooth_application_container.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace Bluetooth {
struct BluetoothObserverApplication : public BluetoothApplication {
    BluetoothObserverApplication(int pid, int uid, const sptr<IRemoteObject> &remote)
        : BluetoothApplication(pid, uid, remote) {}
    ~BluetoothObserverApplication() override = default;

    uint64_t tokenId {};
};

class BluetoothObserverApplicationContainer
    : public BluetoothApplicationContainer<BluetoothObserverApplication> {
public:
    BluetoothObserverApplicationContainer()
        : BluetoothApplicationContainer("BluetoothObserverApplicationContainer") {}
    ~BluetoothObserverApplicationContainer() override = default;

    void AddObject(const sptr<IRemoteObject> &remote);
    uint64_t GetTokenId(const wptr<IRemoteObject> &remote);

    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
};
} // namespace Bluetooth
} // namespace OHOS
#endif  // BLUETOOTH_OBSERVER_APPLICATION_CONTAINER_H