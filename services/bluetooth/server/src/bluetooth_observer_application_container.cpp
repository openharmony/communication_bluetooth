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
#define LOG_TAG "bt_server_observer_container"
#endif

#include "bluetooth_observer_application_container.h"

using namespace std;

namespace OHOS {
namespace Bluetooth {
void BluetoothObserverApplicationContainer::AddObject(const sptr<IRemoteObject> &remote)
{
    AddRemoteObject(IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid(), remote);
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = GetApplicationIter(remote);
    if (it != container_.end() && it->tokenId == 0) {
        it->tokenId = IPCSkeleton::GetCallingFullTokenID();
    }
}

uint64_t BluetoothObserverApplicationContainer::GetTokenId(const wptr<IRemoteObject> &remote)
{
    std::lock_guard<std::mutex> lock(containerMutex_);
    auto it = GetApplicationIter(remote);
    if (it == container_.end()) {
        HILOGE("Unknown remote");
        return 0;
    }
    return it->tokenId;
}

void BluetoothObserverApplicationContainer::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    if (!Contain(remote)) {
        HILOGE("Invalid remote object");
        return;
    }
    RemoveRemoteObject(remote);
}
}  // namespace Bluetooth
}  // namespace OHOS