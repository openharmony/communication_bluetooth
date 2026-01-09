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

#ifndef BT_OOB_DATA_H
#define BT_OOB_DATA_H

#include <cstdint>
#include <vector>
#include <string>

namespace OHOS {
namespace bluetooth {

enum OobDataType : uint8_t {
    P192 = 0x01,
    P256 = 0x02,
};

class BtOobData {
public:
    BtOobData() = default;

    ~BtOobData() = default;

    std::vector<uint8_t> GetAddressWithType() const;

    void SetAddressWithType(std::vector<uint8_t> addressWithType);

    std::vector<uint8_t> GetConfirmationHash() const;

    void SetConfirmationHash(std::vector<uint8_t> confirmationHash);

    std::vector<uint8_t> GetRandomizerHash() const;

    void SetRandomizerHash(std::vector<uint8_t> randomizerHash);

    std::string GetDeviceName() const;

    void SetDeviceName(const std::string &deviceName);

    uint8_t GetOobDataType() const;

    void SetOobDataType(const uint8_t oobDataType);

    bool HasOobData() const;

    bool HasConfirmHash() const;

    bool HasRandomHash() const;

    bool HasDeviceName() const;

    bool HasAddressWithType() const;

    bool hasAddressWithType_ = false;
    std::vector<uint8_t> addressWithType_ = {};
    bool hasConfirmHash_ = false;
    std::vector<uint8_t> confirmationHash_ = {};
    bool hasRandomHash_ = false;
    std::vector<uint8_t> randomizerHash_ = {};
    bool hasDeviceName_ = false;
    std::string deviceName_ = {};
    uint8_t oobDataType_ = P256;
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // BT_OOB_DATA_H