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

#ifndef CAR_CONNECTION_GUARD_H
#define CAR_CONNECTION_GUARD_H

#include <string>
#include <vector>

namespace OHOS {
namespace bluetooth {

/// Profile 类型枚举，用于 Guard 检查时区分 HFP_HF 和 A2DP_SINK 的不同门控策略。
enum class ProfileType {
    HFP_HF,
    A2DP_SINK,
    PBAP_PCE,
};

/// 连接被拒绝的原因枚举，用于标识 Guard.Check 返回 Deny 的具体原因。
enum class DenyReason {
    NONE,           // 允许连接
    SINK_LIMIT,     // Sink 连接数已达上限（可抢占断开其他设备后重试）
    A2DP_GATE,      // A2DP 被 FA/BtMusic 前台门控拦截
    CARPLAY_MUTEX,  // CarPlay 互斥
};

/// 连接检查结果结构体，包含是否允许连接及拒绝原因。
struct CheckResult {
    bool allowed{true};
    DenyReason reason{DenyReason::NONE};

    static CheckResult Allow() { return {true, DenyReason::NONE}; }
    static CheckResult Deny(DenyReason reason) { return {false, reason}; }
};

/// 仅供日志使用的 reason → string 转换。
inline const char *DenyReasonName(DenyReason r)
{
    switch (r) {
        case DenyReason::SINK_LIMIT:    return "SINK_LIMIT";
        case DenyReason::A2DP_GATE:     return "A2DP_GATE";
        case DenyReason::CARPLAY_MUTEX: return "CARPLAY_MUTEX";
        default:                        return "NONE";
    }
}

/// 连接守卫：在 profile 连接前进行门控检查。
/// 规则链前置拦截 + FA/BtMusic 门控 + Sink 限制 + CarPlay 互斥。
/// 简化版：直接调 service 静态方法查询状态，不使用回调注入；3 条规则内联为私有方法。
class CarConnectionGuard {
public:
    static constexpr int FA_STATUS_OFF = 0;
    static constexpr int FA_STATUS_BT_ON = 1;
    static constexpr int BT_MUSIC_STATUS_OFF = 0;
    static constexpr int BT_MUSIC_STATUS_ON = 1;
    /// 未初始化哨兵值：BT ON 后未读取到有效状态时 fail-open（允许连接），
    /// 避免 DataShare 读取失败导致无法连接。FA_STATUS_BT_ON=1, BtMusic=ON=1 为有效值，
    /// UNKNOWN=-1 表示尚未从 DataShare 读取到状态。
    static constexpr int STATUS_UNKNOWN = -1;

    CarConnectionGuard() = default;
    ~CarConnectionGuard() = default;

    /// 连接前检查（依次检查 SinkLimit / A2dpGate / CarPlayMutex，任一 Deny 即返回）。
    CheckResult Check(const std::string &addr, ProfileType profile) const;

    /// Sink 限制：A2dpSnk + HfpHf 合计 ≤ CAR_MAX_CONN。
    bool IsConnectionAllowedBySinkLimit(const std::string &addr) const;

    /// 断开除 addr 外所有已连接设备的 A2DP + HFP（Sink 抢占）。
    /// 车机只允许 1 个设备连接（CAR_MAX_CONN=1），新设备连接时需断开旧设备释放 Sink 名额。
    void DisconnectOthersForSinkLimit(const std::string &addr) const;

    /// 当前已连接的 A2DP 设备地址列表（CONNECTING|CONNECTED）。
    std::vector<std::string> GetA2dpConnectedAddrs() const;
    /// 当前已连接的 HFP 设备地址列表（CONNECTING|CONNECTED）。
    std::vector<std::string> GetHfpConnectedAddrs() const;

    /// A2DP 门控（FA 在蓝牙音乐 或 BtMusic 在前台）。
    bool IsAllowConnA2dp() const;
    /// 设置 FA 状态，并在 FA 切离蓝牙媒体源时重置 BtMusic 为 OFF（副作用独立可见，便于调用方理解）。
    void SetFaStatus(int status);
    void SetBtMusicStatus(int status);
    int GetFaStatus() const { return faStatus_; }
    int GetBtMusicStatus() const { return btMusicStatus_; }

    /// CarPlay 互斥（last_connect_device == 1 表示 CarPlay 为最后连接类型）。
    bool IsCarPlayMutex() const;

private:
    int faStatus_{STATUS_UNKNOWN};
    int btMusicStatus_{STATUS_UNKNOWN};

    static constexpr int CAR_MAX_CONN = 1;
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // CAR_CONNECTION_GUARD_H
