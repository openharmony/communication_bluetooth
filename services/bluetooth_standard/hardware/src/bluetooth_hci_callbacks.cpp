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

#include "bluetooth_hci_callbacks.h"

int32_t BluetoothHciCallbacks::OnInited(ohos::hardware::bt::v1_0::BtStatus status)
{
    if ((callbacks_ != nullptr) && (callbacks_->OnInited)) {
        callbacks_->OnInited((status == ohos::hardware::bt::v1_0::BtStatus::SUCCESS)
                                 ? ::BtStatus::SUCCESS
                                 : ::BtStatus::INITIALIZATION_ERROR);
    }
    return 0;
}

int32_t BluetoothHciCallbacks::OnReceivedHciPacket(
    ohos::hardware::bt::v1_0::BtType type, const std::vector<int8_t> &data)
{
    if ((callbacks_ != nullptr) && (callbacks_->OnReceivedHciPacket)) {
        BtPacketType packetType = BtPacketType::PACKET_TYPE_UNKNOWN;
        switch (type) {
            case ohos::hardware::bt::v1_0::BtType::HCI_EVENT:
                packetType = BtPacketType::PACKET_TYPE_EVENT;
                break;
            case ohos::hardware::bt::v1_0::BtType::ACL_DATA:
                packetType = BtPacketType::PACKET_TYPE_ACL;
                break;
            case ohos::hardware::bt::v1_0::BtType::SCO_DATA:
                packetType = BtPacketType::PACKET_TYPE_SCO;
                break;
            default:
                break;
        }
        BtPacket packet = {
            .data = (data.size() ? (uint8_t *)&data[0] : nullptr),
            .size = data.size(),
        };
        callbacks_->OnReceivedHciPacket(packetType, &packet);
    }
    return 0;
}