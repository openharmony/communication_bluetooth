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

#ifndef DEVICE_PRIORITY_MANAGER_H
#define DEVICE_PRIORITY_MANAGER_H

#include <cstdint>
#include <string>
#include <vector>

namespace OHOS {
namespace bluetooth {

/// 设备优先级管理。
/// 管理 preferred/last/penultimate 三级设备记录与自动回连的设备轮询顺序，
/// 提供 GetNextDevice 接口供 ReconnectEngine 在 BT_ON 场景下逐个候选设备尝试连接。
class DevicePriorityManager {
public:
    DevicePriorityManager() = default;
    ~DevicePriorityManager() = default;

    // BT 开启时初始化：从持久化加载 MAC，过滤已配对设备，构建列表
    void Init();

    // 轮询获取下一个设备
    // currentDevice 为空时返回列表第一个（首选设备）
    // 列表为空时返回空字符串
    std::string GetNextDevice(const std::string &currentDevice);

    // 连接成功后更新记录
    void SetLastDevice(const std::string &addr);
    // 全部 Profile 断开后更新倒数第二
    void SetPenultimateDevice(const std::string &addr);

    void ClearLastDevice();
    void ClearPenultimateDevice();

    std::string GetPreferredDevice() const { return preferredDev_; }
    std::string GetLastDevice() const { return lastDev_; }
    std::string GetPenultimateDevice() const { return penultimateDev_; }
    const std::vector<std::string> &GetOrderList() const { return orderList_; }

private:
    void LoadAll();
    void CreateOrderList();
    void RefreshPenultimateIfNeeded(const std::string &addr);
    std::vector<std::string> GetPairedDevices() const;

    std::string preferredDev_;
    std::string lastDev_;
    std::string penultimateDev_;
    std::vector<std::string> orderList_;
    bool hasRefreshPenultimate_{false};
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // DEVICE_PRIORITY_MANAGER_H
