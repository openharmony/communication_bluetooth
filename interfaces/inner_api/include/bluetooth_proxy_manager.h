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

#ifndef BLUETOOTH_PROXY_MANAGER_H
#define BLUETOOTH_PROXY_MANAGER_H

#include <mutex>
#include <set>

#include "iremote_broker.h"
#include "refbase.h"
#include "bluetooth_no_destructor.h"

namespace OHOS {
namespace Bluetooth {
class IBluetoothBleCentralManager;
class BluetoothProxyManager {
public:
    bool FreezeByRss(std::set<int> pidSet, bool isProxy);
    bool ResetAllProxy();

    static BluetoothProxyManager& GetInstance();
private:
    BluetoothProxyManager() = default;
    ~BluetoothProxyManager() = default;

#ifdef DTFUZZ_TEST
    friend class BluetoothNoDestructor<BluetoothProxyManager>;
#endif
};
} // namespace Bluetooth
} // namespace OHOS
#endif