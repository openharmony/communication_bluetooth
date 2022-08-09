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

#ifndef BLUETOOTH_HCI_CALLBACKS_H
#define BLUETOOTH_HCI_CALLBACKS_H

#include <v1_0/ihci_callback.h>
#include "bluetooth_hdi.h"

using OHOS::HDI::Bluetooth::Hci::V1_0::IHciCallback;
using OHOS::HDI::Bluetooth::Hci::V1_0::BtStatus;
using OHOS::HDI::Bluetooth::Hci::V1_0::BtType;

class BluetoothHciCallbacks : public IHciCallback {
public:
    BluetoothHciCallbacks(BtHciCallbacks *callbacks) : callbacks_(callbacks)
    {}
    virtual ~BluetoothHciCallbacks()
    {}

    int32_t OnInited(BtStatus status) override;

    int32_t OnReceivedHciPacket(BtType type, const std::vector<uint8_t> &data) override;

private:
    BtHciCallbacks *callbacks_;
};
#endif