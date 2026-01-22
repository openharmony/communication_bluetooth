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

#ifndef BLUETOOTH_OOB_DATA_H
#define BLUETOOTH_OOB_DATA_H

#include "bluetooth_def.h"

namespace OHOS {
namespace Bluetooth {

enum OobDataType : uint8_t {
    P192 = 0x01,
    P256 = 0x02,
};

enum LeDeviceRole : uint8_t {
    DEVICE_ROLE_PERIPHERAL_ONLY = 0x00,
    DEVICE_ROLE_CENTRAL_ONLY = 0x01,
    DEVICE_ROLE_BOTH_PREFER_PERIPHERAL = 0x02,
    DEVICE_ROLE_BOTH_PREFER_CENTRAL = 0x03,
};

class BLUETOOTH_API OobData {
public:
    OobData() = default;

    ~OobData() = default;

    std::vector<uint8_t> GetAddressWithType() const;

    void SetAddressWithType(std::vector<uint8_t> addressWithType);

    std::vector<uint8_t> GetConfirmationHash() const;

    void SetConfirmationHash(std::vector<uint8_t> confirmationHash);

    std::vector<uint8_t> GetRandomizerHash() const;

    void SetRandomizerHash(std::vector<uint8_t> randomizerHash);

    std::string GetDeviceName() const;

    void SetDeviceName(const std::string &deviceName);

    uint8_t GetOobDataType() const;

    void SetOobDataType(const uint8_t OobDataType);

    uint8_t GetDeviceRole() const;

    void SetDeviceRole(const uint8_t deviceRole);

    bool HasOobData() const;

    bool HasConfirmHash() const;

    bool HasRandomHash() const;

    bool HasDeviceName() const;

    bool HasAddressWithType() const;

private:
    bool hasAddressWithType_ = false;
    std::vector<uint8_t> addressWithType_ = {};
    bool hasConfirmHash_ = false;
    std::vector<uint8_t> confirmationHash_ = {};
    bool hasRandomHash_ = false;
    std::vector<uint8_t> randomizerHash_ = {};
    bool hasDeviceName_ = false;
    std::string deviceName_ = {};
    uint8_t oobDataType_ = P256;
    uint8_t deviceRole_ = DEVICE_ROLE_PERIPHERAL_ONLY;
};
}  // namespace Bluetooth
}  // namespace OHOS

#endif  // BLUETOOTH_OOB_DATA_H