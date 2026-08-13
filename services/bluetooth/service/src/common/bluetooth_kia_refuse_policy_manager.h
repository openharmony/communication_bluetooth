/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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
#ifndef BLUETOOTH_REFUSE_POLICY_MANAGER_H
#define BLUETOOTH_REFUSE_POLICY_MANAGER_H

#include <mutex>
#include <map>
#include <string>

namespace OHOS {
namespace bluetooth {
const int REFUSE_PROTOCOL_TYPE_GATT = 1;
const int REFUSE_PROTOCOL_TYPE_SPP = 2;
const int REFUSE_PROTOCOL_TYPE_OPP = 3;
/**
 * @brief  kia bluetooth refuse policy manager.
 */
class BluetoothRefusePolicyManager {
public:
    /**
     * @brief Get bluetooth refuse policy manager singleton instance pointer.
     *
     * @return Returns the singleton instance pointer.
     * @since 6
     */
    static BluetoothRefusePolicyManager& GetInstance();
    /**
     * @brief update kia refusePolicy.
     *
     * @param protocolType protocol type
     * @param pid process PID
     * @param prohibitedSecondsTime end time of the application to be controlled.
     * @return
     * @since 16
     */
    int32_t UpdateRefusePolicy(const int32_t protocolType, const int32_t pid, const int64_t prohibitedSecondsTime);

    /**
     * @brief shoule or not refuse socket connect because of kia refusePolicy.
     *
     * @param protocolType protocol type
     * @param pid caller pid
     * @return when allowed return false when not allowed return true.
     * @since 16
     */
    bool ShouldRefuseConnect(const int32_t protocolType, const int32_t pid);
    /**
     * @brief report the refuse information to KIA
     *
     * @param pid caller pid
     * @param refuseTime refuse time
     * @param filePath file path
     * @return
     * @since 16
     */
    void ReportRefuseInfo(int32_t pid, const std::string &filePath = "");
private:
    BluetoothRefusePolicyManager();
    ~BluetoothRefusePolicyManager();
    std::mutex refusePolicyMapMutex_{};
    std::map<std::pair<int32_t, int32_t>, int64_t> refusePolicyMap_;
};
} // namespace bluetooth
} // namespace OHOS

#endif // BLUETOOTH_REFUSE_POLICY_MANAGER_H