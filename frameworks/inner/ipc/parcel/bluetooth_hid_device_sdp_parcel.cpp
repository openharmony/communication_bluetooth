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
#ifndef LOG_TAG
#define LOG_TAG "bt_ipc_parcel_hid_device_sdp"
#endif

#include "bluetooth_hid_device_sdp_parcel.h"
#include "bluetooth_log.h"
namespace OHOS {
namespace Bluetooth {
const uint32_t HID_DEVICE_SDPSETTING_SIZE = 2048;
BluetoothHidDeviceSdp& BluetoothHidDeviceSdp::operator=(const bluetooth::SdpSetting &other)
{
    if (this != &other) {
        bluetooth::SdpSetting::operator=(other);
    }
    return *this;
}

bool BluetoothHidDeviceSdp::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(name_)) {
        return false;
    }
    if (!parcel.WriteString(description_)) {
        return false;
    }
    if (!parcel.WriteString(provider_)) {
        return false;
    }
    if (!parcel.WriteInt32(subclass_)) {
        return false;
    }

    uint32_t size = sdpSettings_.size();
    if (!parcel.WriteUint32(size)) {
        return false;
    }
    for (auto sdpSetting : sdpSettings_) {
        if (!parcel.WriteUint8(sdpSetting)) {
            return false;
        }
    }
    return true;
}

BluetoothHidDeviceSdp *BluetoothHidDeviceSdp::Unmarshalling(Parcel &parcel)
{
    BluetoothHidDeviceSdp *service = new BluetoothHidDeviceSdp();
    if (service != nullptr && !service->ReadFromParcel(parcel)) {
        delete service;
        service = nullptr;
    }
    return service;
}

bool BluetoothHidDeviceSdp::WriteToParcel(Parcel &parcel)
{
    return Marshalling(parcel);
}

bool BluetoothHidDeviceSdp::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadString(name_)) {
        return false;
    }
    if (!parcel.ReadString(description_)) {
        return false;
    }
    if (!parcel.ReadString(provider_)) {
        return false;
    }
    if (!parcel.ReadInt32(subclass_)) {
        return false;
    }

    uint32_t size = 0;
    if (!parcel.ReadUint32(size) || size > HID_DEVICE_SDPSETTING_SIZE) {
        return false;
    }
    for (size_t i = 0; i < size; i++) {
        uint8_t sdpSetting;
        if (!parcel.ReadUint8(sdpSetting)) {
            return false;
        }
        sdpSettings_.push_back(sdpSetting);
    }
    return true;
}
}  // namespace Bluetooth
}  // namespace OHOS