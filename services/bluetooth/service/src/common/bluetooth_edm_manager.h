/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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
#ifndef BLUETOOTH_WHITELIST_MANAGER_H
#define BLUETOOTH_WHITELIST_MANAGER_H

#include <string>
#include <vector>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include "cJSON.h"
#include "bluetooth_common_event_subscriber.h"
#include "control_intercept_plugin.h"

namespace OHOS {
namespace bluetooth {
/**
 * @brief  Represents bluetooth whitelist manager.
 */
class BluetoothEdmManager {
public:
    /**
     * @brief Get bluetooth whitelist manager singleton instance pointer.
     *
     * @return Returns the singleton instance pointer.
     * @since 6
     */
    static BluetoothEdmManager& GetInstance();
    void Init();
    bool IsAllowedConnect(const std::string &mac);
    bool IsAccountConfigAllowedConnect(const ControlInterceptMessage &msg, const std::string &profile);
    bool IsAccountConfigAllowedReceive(const ControlInterceptMessage &msg, const std::string &profile);
    void OnEdmListChanged(const std::string &action);
    void ReportEdmAuditInfo(const std::string &connectedDeviceId, int appId, int userId,
        const std::string &filePath = "");
private:
    BluetoothEdmManager();
    ~BluetoothEdmManager();
    static std::mutex instanceMutexLock_;

    std::shared_ptr<BluetoothCommonEventSubscriber> deviceEventSubscriber_ = nullptr;
    std::shared_ptr<BluetoothCommonEventSubscriber> accountDisallowedEventSubscriber_ = nullptr;

    std::shared_mutex whitelistMutexLock_;
    std::vector<std::string> whitelist_;
    int GetWhitelist();

    std::shared_mutex blacklistMutexLock_;
    std::vector<std::string> denylist_;
    int GetBlacklist();

    std::shared_mutex accountBlacklistMutexLock_;
    std::map<std::string, std::vector<std::string>> accountBlacklist_;
    int GetAccountSendBlacklist();
    int GetAccountRecvBlacklist();

    std::shared_mutex accountRecvBlacklistMutexLock_;
    std::map<std::string, std::vector<std::string>> accountRecvBlacklist_;

    int GetAccountBlacklistInternal(std::shared_mutex& mutexLock,
        std::map<std::string, std::vector<std::string>>& denylist, const char* jsonKey);
    int ParseAccountBlacklist(cJSON *obj, std::map<std::string, std::vector<std::string>> &denylist);

    bool IsValidAddress(const std::string& address);
    bool IsAccountConfigAllowedInternal(const ControlInterceptMessage &msg, const std::string &profile,
        std::shared_mutex &mutexLock, std::map<std::string, std::vector<std::string>> &denylist);
    std::atomic_bool isInit_ = false;
};
} // namespace bluetooth
} // namespace OHOS

#endif // BLUETOOTH_WHITELIST_MANAGER_H