/*
 * Copyright (C) Huawei Device Co., Ltd. 2025-2025. All rights reserved.
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
#ifndef TRUST_PAIR_DEVICE_H
#define TRUST_PAIR_DEVICE_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include "iosfwd"
#include "bt_uuid.h"

namespace OHOS {
namespace bluetooth {
class TrustPairDevice {
public:
    TrustPairDevice() = default;
    ~TrustPairDevice() = default;

    void SetMacAddress(const std::string &mac);
    std::string GetMacAddress() const;

    void SetBluetoothClass(int32_t bluetoothClass);
    int32_t GetBluetoothClass() const;

    void SetDeviceName(const std::string &deviceName);
    std::string GetDeviceName() const;

    void SetToken(const std::vector<uint8_t> &token);
    std::vector<uint8_t> GetToken();

    void SetUuid(const std::vector<std::string> &uuids);
    std::vector<std::string> GetUuid() const;
    std::vector<bluetooth::Uuid> GetParseUuid();

    void SetSecureAdvertisingInfo(const std::vector<uint8_t> &randomDeviceInfo);
    std::vector<uint8_t> GetSecureAdvertisingInfo();
protected:
    std::string sn_{""};
    std::string deviceType_{""};
    std::string modelId_{""};
    std::string manufactory_{""};
    std::string productId_{""};
    std::string hiLinkVersion_{""};
    std::string macAddress_{""};
    std::string serviceType_{""};
    std::string serviceId_{""};
    std::string deviceName_{""};
    std::vector<std::string> uuids_{};
    int32_t bluetoothClass_{0};
    std::vector<uint8_t> token_;
    int64_t deviceNameTime_{0};
    std::vector<uint8_t> secureAdvertisingInfo_;
    int32_t pairState_{0};
};
}
}

#endif //BT_SENSING_INFO_H