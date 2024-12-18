/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "bluetooth_ble_clientDevice.h"

#include "bluetooth_ble_common.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace CJSystemapi {
namespace CJBluetoothBle {

FfiGattClientCallback::FfiGattClientCallback()
{
}

void FfiGattClientCallback::OnReadRemoteRssiValueResult(int rssi, int status)
{
    if (getRssiValueFunc != nullptr) {
        RetDataI32 res{};
        res.code = status;
        res.data = rssi;
        getRssiValueFunc(res);
    }
}

void FfiGattClientCallback::OnCharacteristicChanged(const GattCharacteristic &characteristic)
{
    NativeBLECharacteristic outCharacteristic{};

    GattCharacteristic character_ = const_cast<GattCharacteristic &>(characteristic);

    outCharacteristic.characteristicUuid = MallocCString(character_.GetUuid().ToString().c_str());
    if (character_.GetService() != nullptr) {
        outCharacteristic.serviceUuid = MallocCString(character_.GetService()->GetUuid().ToString().c_str());
    }
    size_t valueSize = 0;
    uint8_t *valueData = character_.GetValue(&valueSize).get();

    CArrUI8 arr{};
    arr.head = valueData;
    arr.size = static_cast<int64_t>(valueSize);
    outCharacteristic.characteristicValue = arr;

    outCharacteristic.properties = ConvertGattPropertiesToCJ(character_.GetProperties());
    outCharacteristic.descriptors = Convert2CArrBLEDescriptor(character_.GetDescriptors());

    if (bleCharacteristicChangeFunc != nullptr) {
        bleCharacteristicChangeFunc(outCharacteristic);
    }
    FreeNativeBLECharacteristic(outCharacteristic);
}

void FfiGattClientCallback::OnCharacteristicReadResult(const GattCharacteristic &characteristic, int ret)
{
    HILOGI("UUID: %{public}s, ret: %{public}d", characteristic.GetUuid().ToString().c_str(), ret);
    RetNativeBLECharacteristic res{};
    res.data = ConvertBLECharacteristicToCJ(const_cast<GattCharacteristic &>(characteristic));
    res.code = ret;
    if (readCharacteristicFunc != nullptr) {
        readCharacteristicFunc(res);
    }
    FreeNativeBLECharacteristic(res.data);
}

void FfiGattClientCallback::OnCharacteristicWriteResult(const GattCharacteristic &characteristic, int ret)
{
    HILOGI("UUID: %{public}s, ret: %{public}d", characteristic.GetUuid().ToString().c_str(), ret);
    if (writeCharacteristicFunc != nullptr) {
        writeCharacteristicFunc(ret);
    }
}

void FfiGattClientCallback::OnDescriptorReadResult(const GattDescriptor &descriptor, int ret)
{
    HILOGI("UUID: %{public}s, ret: %{public}d", descriptor.GetUuid().ToString().c_str(), ret);
    RetNativeBLEDescriptor res{};
    res.data = ConvertBLEDescriptorToCJ(const_cast<GattDescriptor &>(descriptor));
    res.code = ret;
    if (readDescriptorFunc != nullptr) {
        readDescriptorFunc(res);
    }
    FreeNativeBLEDescriptor(res.data);
}

void FfiGattClientCallback::OnDescriptorWriteResult(const GattDescriptor &descriptor, int ret)
{
    HILOGI("UUID: %{public}s, ret: %{public}d", descriptor.GetUuid().ToString().c_str(), ret);
    if (writeDescriptorValueFunc != nullptr) {
        writeDescriptorValueFunc(ret);
    }
}

void FfiGattClientCallback::OnConnectionStateChanged(int connectionState, int ret)
{
    int connectState_ = connectionState;

    NativeBLEConnectionChangeState outState{};

    outState.deviceId = MallocCString(deviceAddr_.c_str());
    outState.state = GetProfileConnectionState(connectState_);

    if (bleConnectionStateChangeFunc != nullptr) {
        bleConnectionStateChangeFunc(outState);
    }
    free(outState.deviceId);
    outState.deviceId = nullptr;
}

void FfiGattClientCallback::OnMtuUpdate(int mtu, int ret)
{
    if (bleMtuChangeFunc != nullptr) {
        bleMtuChangeFunc(mtu);
    }
}

void FfiGattClientCallback::RegisterBLECharacteristicChangeFunc(std::function<void(NativeBLECharacteristic)> cjCallback)
{
    bleCharacteristicChangeFunc = cjCallback;
}

void FfiGattClientCallback::RegisterBLEConnectionStateChangeFunc(
    std::function<void(NativeBLEConnectionChangeState)> cjCallback)
{
    bleConnectionStateChangeFunc = cjCallback;
}

void FfiGattClientCallback::RegisterBLEMtuChangeFunc(std::function<void(int32_t)> cjCallback)
{
    bleMtuChangeFunc = cjCallback;
}

void FfiGattClientCallback::RegisterGetRemoteRssicallback(std::function<void(RetDataI32)> cjCallback)
{
    getRssiValueFunc = cjCallback;
}

void FfiGattClientCallback::RegisterReadCharacteristicCallback(
    std::function<void(RetNativeBLECharacteristic)> cjCallback)
{
    readCharacteristicFunc = cjCallback;
}

void FfiGattClientCallback::RegisterReadDescriptorCallback(std::function<void(RetNativeBLEDescriptor)> cjCallback)
{
    readDescriptorFunc = cjCallback;
}
void FfiGattClientCallback::RegisterWriteCharacteristicCallback(std::function<void(int32_t)> cjCallback)
{
    writeCharacteristicFunc = cjCallback;
}

void FfiGattClientCallback::RegisterWriteDescriptorCallback(std::function<void(int32_t)> cjCallback)
{
    writeDescriptorValueFunc = cjCallback;
}

} // namespace CJBluetoothBle
} // namespace CJSystemapi
} // namespace OHOS