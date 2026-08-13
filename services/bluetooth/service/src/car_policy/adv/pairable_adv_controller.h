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

#ifndef PAIRABLE_ADV_CONTROLLER_H
#define PAIRABLE_ADV_CONTROLLER_H

#include <cstdint>

namespace OHOS {
namespace bluetooth {

/// 可配对广播控制器：管理车机可配对 BLE 广播的开启/关闭。
/// BT 设置 UI 活跃且无设备连接时开启广播，设备连接/配对成功/UI 退出时关闭广播。
///
/// 生命周期策略（保留 handle 只 Enable/Disable）：
///   首次 StartAdv 时通过 GetAdvertiserHandle 动态分配 adv handle，并调
///   StartAdvertising 配置可配对广播参数（connectable + scan response 带设备名）。
///   后续 StartAdv 只调 EnableAdvertising(handle, duration) 恢复广播，
///   StopAdv 只调 DisableAdvertising(handle) 暂停广播，不释放 handle。
///   析构时才调 StopAdvertising + UnregisterAdvertisingHandle 彻底释放资源。
/// 这样可配对窗口的频繁开关不会反复向 controller 申请/释放 handle。
class PairableAdvController {
public:
    PairableAdvController() = default;
    ~PairableAdvController();

    /// 停止可配对广播（HFP CONNECTED / 配对成功 / UI 退出后调用，避免持续广播耗电）。
    /// 仅暂停广播（DisableAdvertising），保留 handle 供下次 StartAdv 直接 Enable。
    void StopAdv();

    /// 启动可配对广播（蓝牙开启 / 全部设备断开后调用）。
    /// 首次启动会申请 handle 并 StartAdvertising 配置参数；后续启动只 EnableAdvertising 恢复广播。
    void StartAdv();

private:
    /// 申请 adv handle 并用可配对参数 StartAdvertising。
    /// 成功后 advHandle_ 保存分配到的 handle，advStarted_ 置 true。
    bool StartAdvInternal();

    /// 当前持有的 adv handle，BLE_INVALID_ADVERTISING_HANDLE 表示尚未分配。
    uint8_t advHandle_ = 0xFF;
    /// 是否已完成 StartAdvertising（区分"首次启动"与"恢复广播"）。
    bool advStarted_ = false;
    /// 广播是否处于 Enable 状态（避免重复 Enable/Disable）。
    bool advEnabled_ = false;
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // PAIRABLE_ADV_CONTROLLER_H
