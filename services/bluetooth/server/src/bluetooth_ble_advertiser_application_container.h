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

#ifndef BLUETOOTH_BLE_ADVERTISER_APPLICATION_CONTAINER_H
#define BLUETOOTH_BLE_ADVERTISER_APPLICATION_CONTAINER_H

#include <set>
#include <condition_variable>
#include "bluetooth_application_container.h"
#include "bluetooth_def.h"

namespace OHOS {
namespace Bluetooth {
using TimePoint = std::chrono::system_clock::time_point;
constexpr int ADV_PREEMPTION_TIMEOUT = 200;
constexpr int64_t ADV_HOLDING_TIME_LIMIT = 300000; // 300000ms = 5min
constexpr int ADV_NUM_LIMIT = 3;
constexpr int SOFTBUS_UID = 1024;
constexpr int SOFTBUS_ADV_NUM_LIMIT = 6;
const std::map<std::pair<std::string, int>, int> g_advNumWhiteList = {
    {{"softbus_server", SOFTBUS_UID}, SOFTBUS_ADV_NUM_LIMIT},
};
// application involved in g_advAuditingWhiteList should have higher priority in adv preemption
const std::vector<std::pair<std::string, int>> g_advAuditingWhiteList = {
    {"softbus_server", SOFTBUS_UID},
};

struct AdvHandleRecord {
    explicit AdvHandleRecord(int handle) : advHandle(handle), timestamp(std::chrono::system_clock::now()) {}
    AdvHandleRecord(int handle, TimePoint time) : advHandle(handle), timestamp(time) {}

    bool operator==(const AdvHandleRecord &handle) const
    {
        return this->advHandle == handle.advHandle;
    }

    bool operator<(const AdvHandleRecord &handle) const
    {
        return this->advHandle < handle.advHandle;
    }

    int advHandle;
    TimePoint timestamp;
};

struct AdvHandleComparator {
    bool operator()(const AdvHandleRecord &lhs, const AdvHandleRecord &rhs)
    {
        return lhs.timestamp < rhs.timestamp;
    }
};

struct BluetoothBleAdvertiserApplication : public BluetoothApplication {
    BluetoothBleAdvertiserApplication(int pid, int uid, const sptr<IRemoteObject> &remote)
        : BluetoothApplication(pid, uid, remote) {}
    ~BluetoothBleAdvertiserApplication() override = default;

    uint64_t tokenId = 0;
    std::string callingName;
    std::set<AdvHandleRecord> advHandles {};
};

class BluetoothBleAdvertiserApplicationContainer
    : public BluetoothApplicationContainer<BluetoothBleAdvertiserApplication> {
public:
    BluetoothBleAdvertiserApplicationContainer()
        : BluetoothApplicationContainer("BluetoothBleAdvertiserApplicationContainer") {}
    ~BluetoothBleAdvertiserApplicationContainer() override = default;

    void AddTokenId(const wptr<IRemoteObject> &remote, uint64_t tokenId);
    void SetCallingName(const wptr<IRemoteObject> &remote, const std::string &name);
    int AddAdvHandle(const wptr<IRemoteObject> &remote, int handle);
    int AddAdvHandle(const wptr<IRemoteObject> &remote, const AdvHandleRecord &handleRecord);
    void RemoveAdvHandle(int handle);
    int PreemptAdvHandle();
    std::set<AdvHandleRecord> GetAdvHandles(const wptr<IRemoteObject> &remote) const;
    std::set<AdvHandleRecord> GetAdvHandles(int pid, int uid);
    std::set<AdvHandleRecord> GetPreemptibleAdvHandles(const wptr<IRemoteObject> &remote);
    bool IsApplicationAdv(int pid, int uid, int handle);
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

    int preemptingAdvHandle = BLE_INVALID_ADVERTISING_HANDLE;
    std::mutex advPreemptionLock = {};
    std::condition_variable advPreemptionCV = {};

private:
    auto GetApplicationIterByHandle(int handle)
    {
        return std::find_if(container_.begin(), container_.end(),
            [handle](const auto &app) { return app.advHandles.find(AdvHandleRecord(handle)) != app.advHandles.end(); });
    }
};
} // namespace Bluetooth
} // namespace OHOS
#endif  // BLUETOOTH_BLE_ADVERTISER_APPLICATION_CONTAINER_H