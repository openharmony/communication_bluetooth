/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#ifndef BLUETOOTH_AIRPLANE_MANAGER_H
#define BLUETOOTH_AIRPLANE_MANAGER_H

#include <atomic>
#include <queue>
#include <string>
#include "bluetooth_common_event_subscriber.h"
#include "satellite_manager.h"

namespace OHOS {
namespace bluetooth {
struct AirplaneManagerFucs {
    std::function<int(void)> enableBluetoothFull = nullptr;
    std::function<int(void)> enableBluetoothHalf = nullptr;
    std::function<int(void)> disableBluetoothFull = nullptr;
    std::function<int(void)> getBluetoothState = nullptr;
    std::function<bool(void)> isCollaborationOn = nullptr;
};

enum AirplaneModeBtState {
    BT_SWITCH_OFF = 0,
    BT_SWITCH_ON = 1,
    BT_SWITCH_HALF = 2,
    BT_SWITCH_TURNING = 3,  // 非稳态，蓝牙开关状态正处于变化中
};

enum class AirplaneModeBtAction {
    ENABLE_BLUETOOTH = 0,
    DISABLE_BLUETOOTH = 1,
};

class BluetoothAirplaneManager {
public:
    BluetoothAirplaneManager();
    ~BluetoothAirplaneManager();
    static BluetoothAirplaneManager *GetInstance();

    void Init(AirplaneManagerFucs funcs, std::shared_ptr<SatelliteManager> &satelliteManager);
    bool CheckNeedAutoEnableBluetooth();
    void Destroy(void);
    void ResetSystemParams(void);

    void NotifyBluetoothStateAction(AirplaneModeBtAction action, bool isUserTriggered);
    void OnAirplaneModeChanged(bool isAirplaneOn);
    void SubscribeAirplaneModeStateChangeCommonEvent();

    /**
     * @brief 处理下一个飞行模式事件，需要在蓝牙状态稳定后调用
     */
    void ProcessNextAirplaneEvent(void);
    bool IsAirplaneModeOn();

private:
    void SubscribeCommonEventSA();
    void UnRegisterAirplaneEvent();

    void ProcessAirplaneModeOffEvent();
    void ProcessAirplaneModeOffEventInBindState();
    void ProcessAirplaneModeOffEventInNoBindState();
    void ProcessAirplaneModeOnEvent();
    void ProcessAirplaneModeOnEventInBindState();
    void ProcessAirplaneModeOnEventInNoBindState();

    void UpdateBluetoothStateBeforeEnterAirplaneMode();
    void SetBtStateBeforeEnterAirplane(int state);
    void CurrentAirplaneEventExecuteComplete(void);

    void DisableBluetoothWhenAirplaneModeChange(void);
    void EnableBluetoothHalfWhenAirplaneModeChange(void);
    void EnableBluetoothFullWhenAirplaneModeChange(void);

    AirplaneManagerFucs funcs_;
    bool isInBindState_ = true;  // Whether bluetooth switch state is bind with airplane mode change.
    bool isAirplaneAction_ = false;  // Whether the action of enable/disable is triggered by airplane mode manager
    int btStateBeforeEnterAirplane_ = BT_SWITCH_OFF;
    std::atomic_bool isAirplaneOn_ = false;
    std::shared_ptr<BluetoothCommonEventSubscriber> subscriber_ = nullptr;

    std::mutex cachedAirplaneEventVecMutex_ {};
    std::vector<bool> cachedAirplaneEventVec_ {};
    std::shared_ptr<SatelliteManager> satelliteManager_ = nullptr;
};
} // namespace bluetooth
} // namespace OHOS

#endif //BLUETOOTH_AIRPLANE_MANAGER_H