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

#ifndef CAR_POLICY_DISPATCHER_H
#define CAR_POLICY_DISPATCHER_H

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

#include "bluetooth_datashare_utils.h"

namespace OHOS {
namespace bluetooth {

class CarPolicyManager;

/// 事件分发器：将蓝牙协议栈回调、DataShare 状态变化、系统广播等事件投递到车机策略线程
/// （单线程串行执行），避免多线程竞争。
/// 统一汇聚所有异步事件（Profile 状态、FA/BtMusic 广播、Settings 数据变化），
/// 串行化到 car policy 线程处理。内部持有各 Service 的 Observer 实现类。
/// 线程切换复用 DoInCarPolicyThread（thread_util.h），不单独造 FFRT queue。
class CarPolicyDispatcher {
public:
    explicit CarPolicyDispatcher(CarPolicyManager &owner);
    ~CarPolicyDispatcher();

    // 注册/注销所有观察者（Profile Observer + FA/BtMusic 广播 + SCT DataShareObserver）
    void RegisterAll();
    void UnregisterAll();

    // 供外部模块切到 car policy 线程执行
    void Dispatch(std::function<void()> task);
    void DispatchDelayed(std::function<void()> task, uint64_t delayMs);

    /// BT ON 时从 DataShare 读取 FA/BtMusic 持久化状态（URI 常量仅在此定义，避免重复）。
    /// 读取失败时输出 STATUS_UNKNOWN(-1)，由调用方做 fail-open。
    void LoadFaBtMusicStatus(int &outFaStatus, int &outBtMusicStatus);

private:
    // Profile Observer 实现类（注册到各 Profile Service）
    class HfpHfObserverImpl;
    class A2dpSnkObserverImpl;
    // Classic 远端设备观察者（ACL 状态 + 配对状态）
    class ClassicRemoteDeviceObserverImpl;

    CarPolicyManager &owner_;
    std::unique_ptr<HfpHfObserverImpl> hfpObserver_;
    std::unique_ptr<A2dpSnkObserverImpl> a2dpObserver_;
    std::unique_ptr<ClassicRemoteDeviceObserverImpl> classicObserver_;

    // DataShare 状态变化观察者（复用 BluetoothObserver，回调式，无需 ChangeInfo）
    sptr<BluetoothObserver> faObserver_;
    sptr<BluetoothObserver> btMusicObserver_;
    sptr<BluetoothObserver> sctObserver_;
    sptr<BluetoothObserver> btSettingsUiObserver_;

    void RegisterProfileObservers();
    void DeregisterProfileObservers();
    void RegisterFaBtMusicObservers();
    void UnregisterFaBtMusicObservers();
    void RegisterSmartCallTransferObserver();
    void UnregisterSmartCallTransferObserver();
    void RegisterClassicRemoteDeviceObserver();
    void DeregisterClassicRemoteDeviceObserver();
    void RegisterBtSettingsUiObserver();
    void UnregisterBtSettingsUiObserver();

    /// 注册/注销单个 DataShare key 的观察者（内部构建用户空间 URI）。
    /// 复用 BluetoothDataShareHelperUtils::RegisterObserver，不需调用方持有 helper。
    void RegisterSettingsObserver(const std::string &key, const sptr<BluetoothObserver> &observer);
    void UnregisterSettingsObserver(const std::string &key, const sptr<BluetoothObserver> &observer);
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // CAR_POLICY_DISPATCHER_H
