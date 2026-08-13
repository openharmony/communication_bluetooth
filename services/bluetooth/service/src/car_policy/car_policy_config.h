/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef CAR_POLICY_CONFIG_H
#define CAR_POLICY_CONFIG_H

#include <cstdint>

namespace OHOS {
namespace bluetooth {

/// 车机策略配置参数集中管理：回连最大重试次数、退避延时档位、ACL 超时重试延时、HFP 重试超时等配置。
class CarPolicyConfig {
public:
    CarPolicyConfig() = default;
    ~CarPolicyConfig() = default;

    int GetMaxRetry() const { return MAX_RETRY; }
    uint64_t GetRetryHfpTimeoutMs() const { return RETRY_HFP_TIMEOUT_MS; }
    uint64_t GetAclTimeoutRetryDelayMs() const { return ACL_TIMEOUT_RETRY_DELAY_MS; }
    uint64_t GetBackoffTier1Ms() const { return BACKOFF_TIER1_MS; }
    uint64_t GetBackoffTier2Ms() const { return BACKOFF_TIER2_MS; }
    uint64_t GetBackoffTier3Ms() const { return BACKOFF_TIER3_MS; }
    int GetBackoffTier2Threshold() const { return BACKOFF_TIER2_THRESHOLD; }
    int GetBackoffTier3Threshold() const { return BACKOFF_TIER3_THRESHOLD; }

private:
    // 单位：次（达到该次数后停止重试）
    static constexpr int MAX_RETRY = 15;
    // 单位：毫秒（BT_ON 场景 HFP 首次连接失败后的容错重试延时）
    static constexpr uint64_t RETRY_HFP_TIMEOUT_MS = 800;
    // 单位：毫秒（ACL 超时断开后延时多久发起回连）
    static constexpr uint64_t ACL_TIMEOUT_RETRY_DELAY_MS = 1000;
    // 单位：毫秒（重试退避延时三档）
    static constexpr uint64_t BACKOFF_TIER1_MS = 10000;
    static constexpr uint64_t BACKOFF_TIER2_MS = 20000;
    static constexpr uint64_t BACKOFF_TIER3_MS = 30000;
    // 单位：次（达到该重试次数后升级到 TIER2/TIER3 退避）
    static constexpr int BACKOFF_TIER2_THRESHOLD = 3;
    static constexpr int BACKOFF_TIER3_THRESHOLD = 7;
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // CAR_POLICY_CONFIG_H
