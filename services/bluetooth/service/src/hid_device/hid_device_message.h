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

#ifndef HID_DEVICE_MESSAGE_H
#define HID_DEVICE_MESSAGE_H

#include <string>

#include "hid_device_defines.h"
#include "btcommon/message.h"
#include "raw_address.h"
#include "hid_device_service_impl_interface.h"
#include "bluetooth_hid_device_sdp_parcel.h"
#include "securec.h"

namespace OHOS {
namespace Bluetooth {
class HidDeviceMessage : public utility::Message {
public:
    explicit HidDeviceMessage(int what = 0, int arg1 = 0, void *arg2 = nullptr) : utility::Message(what, arg1, arg2)
    {}
    HidDeviceMessage(const HidDeviceMessage &src) : utility::Message(src.what_, src.arg1_, src.arg2_),
        dev_(src.dev_),
        id_(src.id_),
        type_(src.type_),
        data_(src.data_),
        sdp_(src.sdp_),
        inQos_(src.inQos_),
        outQos_(src.outQos_),
        bundleName_(src.bundleName_)
    {}
    ~HidDeviceMessage() override = default;
    bluetooth::RawAddress dev_ {};
    int32_t id_ {};
    uint8_t type_ {};
    std::vector<uint8_t> data_ {};
    BluetoothHidDeviceSdp sdp_;
    BluetoothHidDeviceQos inQos_;
    BluetoothHidDeviceQos outQos_;
    std::string bundleName_ {};

    HidDeviceMessage operator=(const HidDeviceMessage &src) = delete;
};
}  // namespace bluetooth
}
#endif // HID_DEVICE_MESSAGE_H
