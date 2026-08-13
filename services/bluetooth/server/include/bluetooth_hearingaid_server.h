/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#ifndef OHOS_BLUETOOTH_HEARINGAID_SERVER_H
#define OHOS_BLUETOOTH_HEARINGAID_SERVER_H

#include "bluetooth_hearing_aid_stub.h"
#include "permission_manager.h"
#include "bluetooth_types.h"
#include "bluetooth_override_errorcode.h"

namespace OHOS {
namespace Bluetooth {

constexpr const char* PROFILE_HEARINGAID_SERVER = "BluetoothHearingAidServer";

class BluetoothHearingAidServer : public BluetoothHearingAidStub {
public:
    BluetoothHearingAidServer();
    ~BluetoothHearingAidServer();
    ErrCode IsLocalDeviceSupportHearingAid(bool& support) override;
    ErrCode Connect(const std::string& address) override;
    ErrCode Disconnect(const std::string& address) override;
    ErrCode SetVolume(const std::string& address, int64_t volume) override;
    ErrCode GetHearingAidDeviceInfo(const std::string& address,
        DeviceSide& side, DeviceMode& mode, int64_t& id) override;
    ErrCode GetConnectedDevices(std::vector<std::string>& connectedDevices) override;
    ErrCode GetActiveDevice(std::string& address, std::string& name) override;
    ErrCode GetConnectionState(const std::string& address, ConnectionState& state) override;
    ErrCode SetConnectionStrategy(const std::string& address, ConnectionStrategy strategy) override;
    ErrCode GetConnectionStrategy(const std::string& address, ConnectionStrategy& strategy) override;
    ErrCode RegisterObserver(const sptr<IBluetoothHearingAidObserver>& observer) override;
    ErrCode DeRegisterObserver(const sptr<IBluetoothHearingAidObserver>& observer) override;
    int32_t CallbackEnter(uint32_t code) override;
    int32_t CallbackExit(uint32_t code, int32_t result) override;
private:
    static const std::map<IBluetoothHearingAidIpcCode, std::shared_ptr<PermissionItem>> permissionCheckMap_;
    const int64_t VOLUME_MIN = -128;
    const int64_t VOLUME_MAX = 0;
    const int32_t STRATEGY_MIN = 0;
    const int32_t STRATEGY_MAX = 2;
    BLUETOOTH_DECLARE_IMPL();
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothHearingAidServer);
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_HEARINGAID_SERVER_H