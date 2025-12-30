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
#define LOG_TAG "bt_ipc_parcel_hid_device_qos"
#endif

#include "bluetooth_hid_device_qos_parcel.h"
#include "bluetooth_log.h"
namespace OHOS {
namespace Bluetooth {
BluetoothHidDeviceQos& BluetoothHidDeviceQos::operator=(const bluetooth::QosSetting &other)
{
    if (this != &other) {
        bluetooth::QosSetting::operator=(other);
    }
    return *this;
}

bool BluetoothHidDeviceQos::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(serviceType_)) {
        return false;
    }
    if (!parcel.WriteInt32(tokenRate_)) {
        return false;
    }
    if (!parcel.WriteInt32(tokenBucketSize_)) {
        return false;
    }
    if (!parcel.WriteInt32(peakBandwidth_)) {
        return false;
    }
    if (!parcel.WriteInt32(latency_)) {
        return false;
    }
    if (!parcel.WriteInt32(delayVariation_)) {
        return false;
    }
    return true;
}

BluetoothHidDeviceQos *BluetoothHidDeviceQos::Unmarshalling(Parcel &parcel)
{
    BluetoothHidDeviceQos *service = new BluetoothHidDeviceQos();
    if (service != nullptr && !service->ReadFromParcel(parcel)) {
        delete service;
        service = nullptr;
    }
    return service;
}

bool BluetoothHidDeviceQos::WriteToParcel(Parcel &parcel)
{
    return Marshalling(parcel);
}

bool BluetoothHidDeviceQos::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadInt32(serviceType_)) {
        return false;
    }
    if (!parcel.ReadInt32(tokenRate_)) {
        return false;
    }
    if (!parcel.ReadInt32(tokenBucketSize_)) {
        return false;
    }
    if (!parcel.ReadInt32(peakBandwidth_)) {
        return false;
    }
    if (!parcel.ReadInt32(latency_)) {
        return false;
    }
    if (!parcel.ReadInt32(delayVariation_)) {
        return false;
    }
    return true;
}
}  // namespace Bluetooth
}  // namespace OHOS