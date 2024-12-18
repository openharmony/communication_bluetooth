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

#include "bluetooth_ble_gattServer.h"

#include "bluetooth_ble_common.h"
#include "bluetooth_gatt_characteristic.h"
#include "bluetooth_gatt_descriptor.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace CJSystemapi {
namespace CJBluetoothBle {
using Bluetooth::BTConnectState;
using Bluetooth::GattCharacteristic;
using Bluetooth::GattDescriptor;

FfiGattServerCallback::FfiGattServerCallback()
{
}

void FfiGattServerCallback::OnCharacteristicReadRequest(const BluetoothRemoteDevice &device,
                                                        GattCharacteristic &characteristic, int requestId)
{
    int transId_ = requestId;
    std::string deviceAddr_ = device.GetDeviceAddr();

    NativeCharacteristicReadRequest request{};
    request.deviceId = MallocCString(deviceAddr_.c_str());
    request.transId = transId_;
    request.offset = 0;
    request.characteristicUuid = MallocCString(characteristic.GetUuid().ToString().c_str());
    if (characteristic.GetService() != nullptr) {
        request.serviceUuid = MallocCString(characteristic.GetService()->GetUuid().ToString().c_str());
    }

    if (characteristicReadFunc != nullptr) {
        characteristicReadFunc(request);
    }
    free(request.deviceId);
    free(request.characteristicUuid);
    free(request.serviceUuid);
    request.deviceId = nullptr;
    request.characteristicUuid = nullptr;
    request.serviceUuid = nullptr;
}

void FfiGattServerCallback::OnCharacteristicWriteRequest(const BluetoothRemoteDevice &device,
                                                         GattCharacteristic &characteristic, int requestId)
{
    int transId_ = requestId;
    std::string deviceAddr_ = device.GetDeviceAddr();

    NativeCharacteristicWriteRequest request{};
    request.deviceId = MallocCString(deviceAddr_.c_str());
    request.transId = transId_;
    request.offset = 0;
    request.isPrepared = false;
    request.needRsp = characteristic.GetWriteType() == GattCharacteristic::WriteType::DEFAULT;

    size_t valueSize;
    uint8_t *valueData = characteristic.GetValue(&valueSize).get();

    CArrUI8 arr{};
    arr.head = valueData;
    arr.size = static_cast<int64_t>(valueSize);
    request.value = arr;

    request.characteristicUuid = MallocCString(characteristic.GetUuid().ToString().c_str());
    if (characteristic.GetService() != nullptr) {
        request.serviceUuid = MallocCString(characteristic.GetService()->GetUuid().ToString().c_str());
    }
    if (characteristicWriteFunc != nullptr) {
        characteristicWriteFunc(request);
    }
    free(request.deviceId);
    free(request.characteristicUuid);
    free(request.serviceUuid);
    request.deviceId = nullptr;
    request.characteristicUuid = nullptr;
    request.serviceUuid = nullptr;
}

void FfiGattServerCallback::OnDescriptorReadRequest(const BluetoothRemoteDevice &device, GattDescriptor &descriptor,
                                                    int requestId)
{
    int transId_ = requestId;
    std::string deviceAddr_ = device.GetDeviceAddr();

    NativeDescriptorReadRequest request{};

    request.deviceId = MallocCString(deviceAddr_.c_str());
    request.transId = transId_;
    request.offset = 0;
    request.descriptorUuid = MallocCString(descriptor.GetUuid().ToString().c_str());
    request.characteristicUuid = MallocCString(descriptor.GetCharacteristic()->GetUuid().ToString().c_str());

    if (descriptor.GetCharacteristic()->GetService() != nullptr) {
        request.serviceUuid = MallocCString(descriptor.GetCharacteristic()->GetService()->GetUuid().ToString().c_str());
    }

    if (descriptorReadFunc != nullptr) {
        descriptorReadFunc(request);
    }
    free(request.deviceId);
    free(request.descriptorUuid);
    free(request.characteristicUuid);
    free(request.serviceUuid);
    request.deviceId = nullptr;
    request.descriptorUuid = nullptr;
    request.characteristicUuid = nullptr;
    request.serviceUuid = nullptr;
}

void FfiGattServerCallback::OnDescriptorWriteRequest(const BluetoothRemoteDevice &device, GattDescriptor &descriptor,
                                                     int requestId)
{
    int transId_ = requestId;
    std::string deviceAddr_ = device.GetDeviceAddr();

    NativeDescriptorWriteRequest request{};

    request.deviceId = MallocCString(deviceAddr_.c_str());
    request.transId = transId_;
    request.offset = 0;
    request.isPrepared = false;
    request.needRsp = true;

    size_t valueSize;
    uint8_t *valueData = descriptor.GetValue(&valueSize).get();

    CArrUI8 arr{};
    arr.head = valueData;
    arr.size = static_cast<int64_t>(valueSize);
    request.value = arr;

    request.descriptorUuid = MallocCString(descriptor.GetUuid().ToString().c_str());
    if (descriptor.GetCharacteristic() != nullptr) {
        request.characteristicUuid = MallocCString(descriptor.GetCharacteristic()->GetUuid().ToString().c_str());
        if (descriptor.GetCharacteristic()->GetService() != nullptr) {
            request.serviceUuid =
                MallocCString(descriptor.GetCharacteristic()->GetService()->GetUuid().ToString().c_str());
        }
    }

    if (descriptorWriteFunc != nullptr) {
        descriptorWriteFunc(request);
    }
    free(request.deviceId);
    free(request.descriptorUuid);
    free(request.characteristicUuid);
    free(request.serviceUuid);
    request.deviceId = nullptr;
    request.descriptorUuid = nullptr;
    request.characteristicUuid = nullptr;
    request.serviceUuid = nullptr;
}

void FfiGattServerCallback::OnConnectionStateUpdate(const BluetoothRemoteDevice &device, int state)
{
    std::lock_guard<std::mutex> lock(FfiGattServer::deviceListMutex_);
    if (state == static_cast<int>(BTConnectState::CONNECTED)) {
        HILOGI("connected");
        bool hasAddr = false;
        for (auto it = FfiGattServer::deviceList_.begin(); it != FfiGattServer::deviceList_.end(); ++it) {
            if (*it == device.GetDeviceAddr()) {
                hasAddr = true;
                break;
            }
        }
        if (!hasAddr) {
            HILOGI("add devices");
            FfiGattServer::deviceList_.push_back(device.GetDeviceAddr());
        }
    } else if (state == static_cast<int>(BTConnectState::DISCONNECTED)) {
        HILOGI("disconnected");
        for (auto it = FfiGattServer::deviceList_.begin(); it != FfiGattServer::deviceList_.end(); ++it) {
            if (*it == device.GetDeviceAddr()) {
                HILOGI("romove device");
                FfiGattServer::deviceList_.erase(it);
                break;
            }
        }
    }

    std::string deviceAddr_ = device.GetDeviceAddr();
    int connectState_ = GetProfileConnectionState(state);

    NativeBLEConnectionChangeState changeState{};

    changeState.deviceId = MallocCString(deviceAddr_.c_str());
    changeState.state = connectState_;

    if (connectionStateChangeFunc != nullptr) {
        connectionStateChangeFunc(changeState);
    }
    free(changeState.deviceId);
    changeState.deviceId = nullptr;
}

void FfiGattServerCallback::OnMtuUpdate(const BluetoothRemoteDevice &device, int mtu)
{
    if (bleMtuChangeFunc != nullptr) {
        bleMtuChangeFunc(mtu);
    }
}

void FfiGattServerCallback::RegisterCharacteristicReadFunc(
    std::function<void(NativeCharacteristicReadRequest)> cjCallback)
{
    characteristicReadFunc = cjCallback;
}

void FfiGattServerCallback::RegisterCharacteristicWriteFunc(
    std::function<void(NativeCharacteristicWriteRequest)> cjCallback)
{
    characteristicWriteFunc = cjCallback;
}

void FfiGattServerCallback::RegisterDescriptorReadFunc(std::function<void(NativeDescriptorReadRequest)> cjCallback)
{
    descriptorReadFunc = cjCallback;
}

void FfiGattServerCallback::RegisterDescriptorWriteFunc(std::function<void(NativeDescriptorWriteRequest)> cjCallback)
{
    descriptorWriteFunc = cjCallback;
}

void FfiGattServerCallback::RegisterConnectionStateChangeFunc(
    std::function<void(NativeBLEConnectionChangeState)> cjCallback)
{
    connectionStateChangeFunc = cjCallback;
}

void FfiGattServerCallback::RegisterBLEMtuChangeFunc(std::function<void(int32_t)> cjCallback)
{
    bleMtuChangeFunc = cjCallback;
}

} // namespace CJBluetoothBle
} // namespace CJSystemapi
} // namespace OHOS