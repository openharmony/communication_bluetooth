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

#include "reconnect_predicates.h"

#include "adapter_manager.h"
#include "bluetooth_def.h"
#include "car_connection_guard.h"
#include "car_policy_manager.h"
#include "car_profile_service.h"
#include "log.h"
#include "raw_address.h"
#include "smart_call_transfer_controller.h"

#ifndef LOG_TAG
#define LOG_TAG "bt_service_car_policy_predicates"
#endif

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;

bool IncomingConnPredicate::ShouldTerminate(const std::string &addr, int retryTimes)
{
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter == nullptr) {
        return false;
    }
    // 遍历所有已配对设备，任一存在 incoming 连接或 profile 未断开（CONNECTING/CONNECTED）则终止，
    // 避免与对端发起的连接形成双向冲突。
    for (const auto &device : classicAdapter->GetPairedDevices()) {
        if (classicAdapter->IsIncomingConnection(device)) {
            HILOGI("IncomingConnPredicate: %{public}s has incoming connection, terminate",
                   GET_ENCRYPT_ADDR(device));
            return true;
        }
        std::string devAddr = device.GetAddress();
        int hfpState = CarProfileService::GetHfpDeviceState(devAddr);
        int a2dpState = CarProfileService::GetA2dpDeviceState(devAddr);
        if (hfpState == static_cast<int>(BTConnectState::CONNECTING) ||
            hfpState == static_cast<int>(BTConnectState::CONNECTED) ||
            a2dpState == static_cast<int>(BTConnectState::CONNECTING) ||
            a2dpState == static_cast<int>(BTConnectState::CONNECTED)) {
            HILOGI("IncomingConnPredicate: %{public}s profile already connected/connecting, terminate",
                   GET_ENCRYPT_ADDR(device));
            return true;
        }
    }
    return false;
}

// ── 3. BT 设置 UI 活跃且 retry>=1 ──
// 首次重试放行（给首次连接一次机会），retryTimes>=1 后才检查 UI 是否活跃：
// UI 活跃意味着用户正在扫描/配对，回连与扫描竞争射频资源会互相拖慢，故终止。
BtSettingsUiPredicate::BtSettingsUiPredicate(CarPolicyManager &mgr) : mgr_(mgr) {}
bool BtSettingsUiPredicate::ShouldTerminate(const std::string &addr, int retryTimes)
{
    // 首次重试前不检查（给首次连接机会），第二次及以后重试前检查 UI
    if (retryTimes < 1) {
        return false;
    }
    return mgr_.GetBtSettingsUiActive() != 0;
}
void BtSettingsUiPredicate::OnTerminate()
{
    // 回连重试到达且 BT 设置 UI 活跃时取消扫描，避免射频资源抢占
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter != nullptr) {
        classicAdapter->CancelBtDiscovery();
        HILOGI("CancelBtDiscovery triggered by BtSettingsUiPredicate");
    }
    mgr_.ReportScanTerminate("BtSettingsUiActive");
}

// ── 4. 华为手机+智能来电转移 OUT ──
SmartCallOutPredicate::SmartCallOutPredicate(SmartCallTransferController &sct) : sct_(sct) {}
bool SmartCallOutPredicate::ShouldTerminate(const std::string &addr, int retryTimes)
{
    return sct_.IsHwPhoneAndTransferOut(addr);
}

// ── 5. CarPlay 为最后连接 ──
CarplayMutexPredicate::CarplayMutexPredicate(CarConnectionGuard &guard) : guard_(guard) {}
bool CarplayMutexPredicate::ShouldTerminate(const std::string &addr, int retryTimes)
{
    return guard_.IsCarPlayMutex();
}

}  // namespace bluetooth
}  // namespace OHOS
