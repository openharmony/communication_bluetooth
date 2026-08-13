/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_BLUETOOTH_STANDARD_BAS_SERVER_H
#define OHOS_BLUETOOTH_STANDARD_BAS_SERVER_H

#include <memory>
#include <string>
#include "bt_def.h"
#include "bluetooth_bas_stub.h"
#include "bluetooth_types.h"
#include "i_bluetooth_device_battery_observer.h"
#include "i_bluetooth_bas.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothBasServer : public BluetoothBasStub {
public:
    explicit BluetoothBasServer();
    ~BluetoothBasServer() override;

    int32_t RegisterDeviceBatteryObserver(const sptr<IBluetoothDeviceBatteryObserver> &observer) override;
    int32_t DeregisterDeviceBatteryObserver(const sptr<IBluetoothDeviceBatteryObserver> &observer) override;
    int32_t GetBatteryLevel(const std::string &address) override;
    int32_t GetConnectedDeviceBatteryInfos(std::map<std::string, int32_t> &batteryInfos) override;

private:
    sptr<IBluetoothDeviceBatteryObserver> FindCachedObserver(uint64_t fullTokenId, int32_t callingUid);
    bool CheckIfBasRequestExist(const std::vector<sptr<IBluetoothDeviceBatteryObserver>> &observerList,
        uint64_t fullTokenId, int32_t callingUid);

    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothBasServer);
    BLUETOOTH_DECLARE_IMPL();
};
}  // namespace Bluetooth
}  // namespace OHOS

#endif  // OHOS_BLUETOOTH_STANDARD_BAS_SERVER_H