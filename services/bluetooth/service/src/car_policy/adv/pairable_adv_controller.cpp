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

#include "pairable_adv_controller.h"

#ifndef LOG_TAG
#define LOG_TAG "bt_service_car_policy_adv"
#endif
#include "log.h"

#include "adapter_manager.h"
#include "ble_adapter.h"
#include "ble_service_data.h"
#include "bluetooth_def.h"
#include "classic_adapter.h"

namespace OHOS {
namespace bluetooth {

namespace {
// 可配对广播的 duration：0 表示持续广播直到主动 Disable（BLE spec 语义）。
constexpr uint16_t ADV_DURATION_UNLIMITED = 0;

// 可配对广播 interval（单位 0.625ms）：
// 0x20 = 32*0.625 = 20ms，0x80 = 128*0.625 = 80ms。
// 可配对窗口使用快速间隔让手机尽快扫到车机，配对完成后会被 StopAdv 暂停。
constexpr int PAIRABLE_ADV_INTERVAL = 0x20;

// BLE ADV flags 字段：LE General Discoverable Mode + BR/EDR Not Supported。
// 用于让旧版本扫描器正确识别广播类型。
constexpr uint8_t ADV_FLAGS_LE_GENERAL_DISCOVERABLE = 0x06;
}  // namespace

PairableAdvController::~PairableAdvController()
{
    // 析构时彻底释放：先停广播再释放 handle，避免 controller 资源泄漏。
    if (advHandle_ == BLE_INVALID_ADVERTISING_HANDLE) {
        return;
    }
    auto bleAdapter = AdapterManager::GetInstance()->GetBleAdapter();
    if (bleAdapter == nullptr) {
        HILOGW("~PairableAdvController: BleAdapter null, cannot release handle %{public}u", advHandle_);
        advHandle_ = BLE_INVALID_ADVERTISING_HANDLE;
        advStarted_ = false;
        advEnabled_ = false;
        return;
    }
    if (advStarted_) {
        bleAdapter->StopAdvertising(advHandle_);
        advStarted_ = false;
        advEnabled_ = false;
    }
    bleAdapter->UnregisterAdvertisingHandle(advHandle_);
    HILOGI("~PairableAdvController: released handle %{public}u", advHandle_);
    advHandle_ = BLE_INVALID_ADVERTISING_HANDLE;
}

bool PairableAdvController::StartAdvInternal()
{
    auto bleAdapter = AdapterManager::GetInstance()->GetBleAdapter();
    if (bleAdapter == nullptr) {
        HILOGE("StartAdvInternal: BleAdapter null");
        return false;
    }

    // ① 向 controller 申请一个 adv handle（内部 RegisterAdvertiser，加入 advInstances_）。
    int status = 0;
    uint8_t handle = bleAdapter->GetAdvertiserHandle(status);
    if (handle == BLE_INVALID_ADVERTISING_HANDLE) {
        HILOGE("StartAdvInternal: GetAdvertiserHandle failed, status=%{public}d", status);
        return false;
    }

    // ② 配置可配对广播参数：
    //   - legacy mode：兼容旧版扫描器
    //   - connectable：允许手机发起 GATT 连接（用于配对流程）
    //   - 设置快速 interval 让手机尽快扫到车机
    //   - scan response 带本机蓝牙设备名，让手机能显示车机名称
    BleAdvertiserSettingsImpl settings;
    settings.SetLegacyMode(true);
    settings.SetConnectable(true);
    settings.SetInterval(PAIRABLE_ADV_INTERVAL);

    BleAdvertiserDataImpl advData;
    advData.SetFlags(ADV_FLAGS_LE_GENERAL_DISCOVERABLE);

    BleAdvertiserDataImpl scanResp;
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter != nullptr) {
        scanResp.SetDeviceName(classicAdapter->GetLocalName());
    }

    // ③ StartAdvertising：将参数下发给 controller 并启动广播。
    bleAdapter->StartAdvertising(settings, advData, scanResp, handle, ADV_DURATION_UNLIMITED);

    advHandle_ = handle;
    advStarted_ = true;
    advEnabled_ = true;
    HILOGI("StartAdvInternal: started, handle=%{public}u", advHandle_);
    return true;
}

void PairableAdvController::StartAdv()
{
    // 已 Enable：无需重复操作。
    if (advEnabled_) {
        HILOGD("StartAdv: already enabled, handle=%{public}u", advHandle_);
        return;
    }

    auto bleAdapter = AdapterManager::GetInstance()->GetBleAdapter();
    if (bleAdapter == nullptr) {
        HILOGW("StartAdv: BleAdapter null, no-op");
        return;
    }

    // 尚未启动（首次或之前已析构重建）：走完整的申请 handle + StartAdvertising 流程。
    if (!advStarted_) {
        if (!StartAdvInternal()) {
            return;
        }
        return;
    }

    // 已 Start 但被 Disable 暂停过：直接 EnableAdvertising 恢复，无需重新申请 handle 与配置参数。
    // duration=0 表示持续广播直到下次 Disable。
    bleAdapter->EnableAdvertising(advHandle_, ADV_DURATION_UNLIMITED);
    advEnabled_ = true;
    HILOGI("StartAdv: re-enabled, handle=%{public}u", advHandle_);
}

void PairableAdvController::StopAdv()
{
    // 尚未分配 handle 或已 Disable：无需重复操作。
    if (advHandle_ == BLE_INVALID_ADVERTISING_HANDLE || !advEnabled_) {
        HILOGD("StopAdv: not enabled (handle=%{public}u, enabled=%{public}d), skip",
               advHandle_, advEnabled_);
        return;
    }

    auto bleAdapter = AdapterManager::GetInstance()->GetBleAdapter();
    if (bleAdapter == nullptr) {
        HILOGW("StopAdv: BleAdapter null, no-op");
        return;
    }

    // 仅暂停广播，保留 handle 与已配置的参数，下次 StartAdv 可直接 EnableAdvertising 恢复。
    bleAdapter->DisableAdvertising(advHandle_);
    advEnabled_ = false;
    HILOGI("StopAdv: disabled, handle=%{public}u", advHandle_);
}

}  // namespace bluetooth
}  // namespace OHOS
