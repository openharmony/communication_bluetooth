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

#include "smart_call_transfer_controller.h"

#include "log.h"

#include "service/car_profile_service.h"

namespace OHOS {
namespace bluetooth {
namespace {
#ifndef LOG_TAG
#define LOG_TAG "bt_service_car_policy_sct"
#endif
}

SmartCallTransferController::SmartCallTransferController(CarDeviceTypeResolver &resolver, DevicePriorityManager &dpm,
                                                        ReconnectEngine &reconnectEngine)
    : resolver_(resolver), dpm_(dpm), reconnectEngine_(reconnectEngine) {}

void SmartCallTransferController::OnStatusChanged(int newStatus)
{
    // 状态未变化时跳过，避免 DataShare 重复通知触发重复回连/断开。
    if (newStatus == status_) {
        HILOGI("OnStatusChanged: status not changed (%{public}d), skip", newStatus);
        return;
    }
    HILOGI("OnStatusChanged: %{public}d -> %{public}d", status_, newStatus);
    status_ = newStatus;
    if (newStatus == TRANSFER_STATUS_IN) {
        HandleTransferIn();
    } else if (newStatus == TRANSFER_STATUS_OUT) {
        HandleTransferOut();
    }
}

void SmartCallTransferController::HandleTransferIn()
{
    // 已有 HFP 连接/连接中则跳过（不重复连）。
    if (CarProfileService::HasHfpConnectingOrConnected()) {
        HILOGI("TransferIn skipped: HFP already connecting/connected");
        return;
    }
    // 取 lastDevice 作为回连目标。无回连进行中且 preferred 设备非空时用 preferred 设备
    // （preferred 设备优先级高于 lastDevice）。
    std::string device = dpm_.GetLastDevice();
    if (!reconnectEngine_.IsActive() && !dpm_.GetPreferredDevice().empty()) {
        device = dpm_.GetPreferredDevice();
    }
    if (device.empty()) {
        HILOGI("TransferIn skipped: no last/preferred device");
        return;
    }
    if (resolver_.IsHwPhone(device)) {
        HILOGI("TransferIn: trigger reconnect for %{public}s", device.c_str());
        reconnectEngine_.StartReconnect(device, ReconnectReason::SMART_CALL_TRANSFER_IN);
    } else {
        HILOGI("TransferIn skipped: %{public}s not HwPhone", device.c_str());
    }
}

void SmartCallTransferController::HandleTransferOut()
{
    // 取 lastDevice，仅对华为手机主动断开 HFP+A2DP。非华为手机不主动断开（由用户手动切换）。
    std::string lastDevice = dpm_.GetLastDevice();
    if (lastDevice.empty()) {
        HILOGI("TransferOut skipped: no last device");
        return;
    }
    if (resolver_.IsHwPhone(lastDevice)) {
        HILOGI("TransferOut: disconnect HFP+A2DP for %{public}s", lastDevice.c_str());
        CarProfileService::Disconnect(ProfileType::HFP_HF, lastDevice);
        CarProfileService::Disconnect(ProfileType::A2DP_SINK, lastDevice);
    } else {
        HILOGI("TransferOut skipped: %{public}s not HwPhone", lastDevice.c_str());
    }
}

bool SmartCallTransferController::IsHwPhoneAndTransferOut(const std::string &addr) const
{
    if (status_ != TRANSFER_STATUS_OUT) {
        return false;
    }
    return resolver_.IsHwPhone(addr);
}

}  // namespace bluetooth
}  // namespace OHOS
