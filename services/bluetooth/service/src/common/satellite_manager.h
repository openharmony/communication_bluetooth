/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef SATELLITE_MANAGER_H
#define SATELLITE_MANAGER_H

#include <atomic>
#include <memory>
#include "btcommon/timer_manager.h"

namespace OHOS {
namespace bluetooth {

class SatelliteManager : public std::enable_shared_from_this<SatelliteManager> {
public:
    SatelliteManager();
    ~SatelliteManager();

    /**
     * @brief 初始化函数
     * @note 初始化卫星管理类的相关参数
     */
    void Init(void);
    /**
     * @brief 卫星控制函数
     * @param[in] type 卫星控制类型
     * @param[in] state 卫星控制状态
     * @return 返回卫星控制结果
     */
    int32_t SatelliteControl(int type, int state);
    /**
     * @brief 判断卫星控制开关功能是否打开
     * @return 返回卫星控制开关是否打开的结果
     */
    bool IsSatelliteControlBtSwitch(void) const
    {
        return isSatelliteControlBtSwitch_.load();
    }

private:
    /**
     * @brief 删除卫星控制配置函数
     * @note 删除卫星控制相关的配置信息
     */
    void DeleteSatelliteControlConfig(void);
    /**
     * @brief 处理控制天线事件函数
     * @param[in] state 控制天线的状态
     * @return 返回处理结果
     */
    int32_t ProcessControlAntennaEvent(int state);
    /**
     * @brief 处理控制蓝牙开关事件函数
     * @param[in] state 控制蓝牙开关的状态
     * @return 返回处理结果
     */
    int32_t ProcessControlBtSwitchEvent(int state);

    int32_t ProcessSetMacIdEvent(int state);

    static constexpr const char *BT_SWITCH_TIME_KEY = "BtSwitchTime";
    static constexpr const char *IS_BT_SWITCH_CONTROL_ENABLED_KEY = "IsBtSwitchControlEnabled";

    std::atomic_bool isSatelliteControlBtSwitch_ = false;
    const int TWO_MIN_TIMEOUT = 120;  // 2min
    int timeoutMs_ = 120000;  // 2min
    std::shared_ptr<utility::Timer> timer_ { nullptr };  // Used for 2 min timeout
};

}  // namespace bluetooth
}  // namespace OHOS
#endif // SATELLITE_MANAGER_H
