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

#include "device_priority_manager.h"

#ifndef LOG_TAG
#define LOG_TAG "bt_service_car_policy_dpm"
#endif
#include "log.h"

#include "adapter_manager.h"
#include "bluetooth_datashare_utils.h"
#include "classic_adapter.h"
#include "raw_address.h"

namespace OHOS {
namespace bluetooth {

namespace {
constexpr size_t MAX_ORDER_LIST_SIZE = 3;
const char *const COMMON_BT_DEVICE_KEY = "common_bt_device";
const char *const LAST_DEVICE_KEY = "last_device";
const char *const PENULTIMATE_DEVICE_KEY = "penultimate";
}  // namespace

void DevicePriorityManager::LoadAll()
{
    if (!BluetoothDataShareQueryForUser(COMMON_BT_DEVICE_KEY, preferredDev_)) {
        HILOGW("LoadAll: query preferred device failed");
        preferredDev_.clear();
    }
    if (!BluetoothDataShareQueryForUser(LAST_DEVICE_KEY, lastDev_)) {
        HILOGW("LoadAll: query last device failed");
        lastDev_.clear();
    }
    if (!BluetoothDataShareQueryForUser(PENULTIMATE_DEVICE_KEY, penultimateDev_)) {
        HILOGW("LoadAll: query penultimate device failed");
        penultimateDev_.clear();
    }
}

void DevicePriorityManager::Init()
{
    LoadAll();
    CreateOrderList();
    HILOGI("DPM init: preferred=%{public}s, last=%{public}s, penultimate=%{public}s, listSize=%{public}zu",
           preferredDev_.c_str(), lastDev_.c_str(), penultimateDev_.c_str(), orderList_.size());
}

std::vector<std::string> DevicePriorityManager::GetPairedDevices() const
{
    std::vector<std::string> result;
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter == nullptr) {
        return result;
    }
    for (const auto &device : classicAdapter->GetPairedDevices()) {
        result.push_back(device.GetAddress());
    }
    return result;
}

void DevicePriorityManager::CreateOrderList()
{
    orderList_.clear();

    std::vector<std::string> pairedMacs = GetPairedDevices();
    auto isInPaired = [&pairedMacs](const std::string &mac) {
        for (const auto &paired : pairedMacs) {
            if (paired == mac) {
                return true;
            }
        }
        return false;
    };
    auto isAlreadyInList = [this](const std::string &mac) {
        for (const auto &dev : orderList_) {
            if (dev == mac) {
                return true;
            }
        }
        return false;
    };

    if (!preferredDev_.empty() && isInPaired(preferredDev_)) {
        orderList_.push_back(preferredDev_);
    }
    if (!lastDev_.empty() && isInPaired(lastDev_) && !isAlreadyInList(lastDev_)) {
        orderList_.push_back(lastDev_);
    }
    if (!penultimateDev_.empty() && isInPaired(penultimateDev_) && !isAlreadyInList(penultimateDev_)) {
        orderList_.push_back(penultimateDev_);
    }

    if (orderList_.size() > MAX_ORDER_LIST_SIZE) {
        orderList_.resize(MAX_ORDER_LIST_SIZE);
    }
}

std::string DevicePriorityManager::GetNextDevice(const std::string &currentDevice)
{
    if (orderList_.empty()) {
        return "";
    }
    if (currentDevice.empty()) {
        return orderList_[0];
    }
    size_t currentIndex = 0;
    bool found = false;
    for (size_t i = 0; i < orderList_.size(); ++i) {
        if (orderList_[i] == currentDevice) {
            currentIndex = i;
            found = true;
            break;
        }
    }
    if (!found) {
        return orderList_[0];
    }
    size_t nextIndex = (currentIndex + 1) % orderList_.size();
    return orderList_[nextIndex];
}

void DevicePriorityManager::RefreshPenultimateIfNeeded(const std::string &addr)
{
    if (!lastDev_.empty() && lastDev_ != addr) {
        penultimateDev_ = lastDev_;
        if (!BluetoothDataShareUpdateForUser(PENULTIMATE_DEVICE_KEY, penultimateDev_)) {
            HILOGW("RefreshPenultimateIfNeeded: update penultimate failed, addr=%{public}s", addr.c_str());
        }
        hasRefreshPenultimate_ = true;
    }
}

void DevicePriorityManager::SetLastDevice(const std::string &addr)
{
    if (addr.empty() || lastDev_ == addr) {
        return;
    }
    RefreshPenultimateIfNeeded(addr);
    lastDev_ = addr;
    if (!BluetoothDataShareUpdateForUser(LAST_DEVICE_KEY, addr)) {
        HILOGW("SetLastDevice: update last device failed, addr=%{public}s", addr.c_str());
    }
    HILOGI("SetLastDevice: %{public}s, penultimate now=%{public}s",
           addr.c_str(), penultimateDev_.c_str());
}

void DevicePriorityManager::SetPenultimateDevice(const std::string &addr)
{
    if (lastDev_ == addr && hasRefreshPenultimate_) {
        return;
    }
    penultimateDev_ = addr;
    if (!BluetoothDataShareUpdateForUser(PENULTIMATE_DEVICE_KEY, addr)) {
        HILOGW("SetPenultimateDevice: update penultimate failed, addr=%{public}s", addr.c_str());
    }
    hasRefreshPenultimate_ = true;
    HILOGI("SetPenultimateDevice: %{public}s", addr.c_str());
}

void DevicePriorityManager::ClearLastDevice()
{
    lastDev_.clear();
    hasRefreshPenultimate_ = false;
    if (!BluetoothDataShareUpdateForUser(LAST_DEVICE_KEY, "")) {
        HILOGW("ClearLastDevice: update failed");
    }
}

void DevicePriorityManager::ClearPenultimateDevice()
{
    penultimateDev_.clear();
    hasRefreshPenultimate_ = false;
    if (!BluetoothDataShareUpdateForUser(PENULTIMATE_DEVICE_KEY, "")) {
        HILOGW("ClearPenultimateDevice: update failed");
    }
}

}  // namespace bluetooth
}  // namespace OHOS
