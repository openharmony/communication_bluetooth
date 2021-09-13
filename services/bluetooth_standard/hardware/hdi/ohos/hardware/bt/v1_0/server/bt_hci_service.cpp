/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "bluetooth_hal.h"
#include "bt_hci_service.h"
#include "vendor_interface.h"

namespace ohos {
namespace hardware {
namespace bt {
namespace v1_0 {

using android::hardware::hidl_vec;
using VendorInterface = android::hardware::bluetooth::V1_0::implementation::VendorInterface;

int32_t BtHciService::Init(const sptr<IBtHciCallbacks>& callbacks)
{
    if (callbacks == nullptr) {
        return HDF_FAILURE;
    }

    bool result = VendorInterface::Initialize(
        [&](bool status) {
            callbacks->OnInited(status ? BtStatus::SUCCESS : BtStatus::INITIAL_ERROR);
        },
        [&](const hidl_vec<uint8_t> &packet) {
            std::vector<int8_t> vec {packet.begin(), packet.end()};
            callbacks->OnReceivedHciPacket(BtType::HCI_EVENT, vec);
        },
        [&](const hidl_vec<uint8_t> &packet) {
            std::vector<int8_t> vec {packet.begin(), packet.end()};
            callbacks->OnReceivedHciPacket(BtType::ACL_DATA, vec);
        },
        [&](const hidl_vec<uint8_t> &packet) {
            std::vector<int8_t> vec {packet.begin(), packet.end()};
            callbacks->OnReceivedHciPacket(BtType::SCO_DATA, vec);
        });

    return result ? HDF_SUCCESS : HDF_FAILURE;
}

int32_t BtHciService::SendHciPacket(BtType type, const std::vector<int8_t>& data)
{
    if (data.empty()) {
        return HDF_FAILURE;
    }

    size_t result = VendorInterface::get()->Send(type, (const uint8_t *)&data[0], data.size());
    return result ? HDF_SUCCESS : HDF_FAILURE;
}

int32_t BtHciService::Close()
{
    VendorInterface::Shutdown();
    return HDF_SUCCESS;
}

} // v1_0
} // bt
} // hardware
} // ohos
