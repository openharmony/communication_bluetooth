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
#ifndef LOG_TAG
#define LOG_TAG "bt_ipc_host_observer_stub"
#endif

#include "bluetooth_host_observer_stub.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
BluetoothHostObserverStub::BluetoothHostObserverStub()
{
    HILOGD("enter");
    memberFuncMap_[static_cast<uint32_t>(BluetoothHostObserverInterfaceCode::BT_HOST_OBSERVER_STATE_CHANGE)] =
        BluetoothHostObserverStub::OnStateChangedInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothHostObserverInterfaceCode::BT_HOST_OBSERVER_DISCOVERY_STATE_CHANGE)] =
        BluetoothHostObserverStub::OnDiscoveryStateChangedInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothHostObserverInterfaceCode::BT_HOST_OBSERVER_DISCOVERY_RESULT)] =
        BluetoothHostObserverStub::OnDiscoveryResultInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothHostObserverInterfaceCode::BT_HOST_OBSERVER_PAIR_REQUESTED)] =
        BluetoothHostObserverStub::OnPairRequestedInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothHostObserverInterfaceCode::BT_HOST_OBSERVER_PAIR_CONFIRMED)] =
        BluetoothHostObserverStub::OnPairConfirmedInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothHostObserverInterfaceCode::BT_HOST_OBSERVER_SCAN_MODE_CHANGED)] =
        BluetoothHostObserverStub::OnScanModeChangedInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothHostObserverInterfaceCode::BT_HOST_OBSERVER_DEVICE_NAME_CHANGED)] =
        BluetoothHostObserverStub::OnDeviceNameChangedInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothHostObserverInterfaceCode::BT_HOST_OBSERVER_DEVICE_ADDR_CHANGED)] =
        BluetoothHostObserverStub::OnDeviceAddrChangedInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothHostObserverInterfaceCode::BT_HOST_OBSERVER_STATE_CHANGE_V2)] =
        BluetoothHostObserverStub::OnBluetoothStateChangedInner;
}

BluetoothHostObserverStub::~BluetoothHostObserverStub()
{
    HILOGD("enter");
    memberFuncMap_.clear();
}

int32_t BluetoothHostObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOGD("transaction of code: %{public}d is received", code);
    if (BluetoothHostObserverStub::GetDescriptor() != data.ReadInterfaceToken()) {
        HILOGE("BluetoothHostObserverStub::OnRemoteRequest, local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return memberFunc(this, data, reply);
        }
    }

    HILOGW("OnRemoteRequest, default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t BluetoothHostObserverStub::OnStateChangedInner(BluetoothHostObserverStub *stub,
    MessageParcel &data, MessageParcel &reply)
{
    int32_t transport = data.ReadInt32();
    int32_t status = data.ReadInt32();

    HILOGD("OnStateChangedInner starts");
    stub->OnStateChanged(transport, status);

    return NO_ERROR;
}

// ON_DIS_STA_CHANGE_CODE
int32_t BluetoothHostObserverStub::OnDiscoveryStateChangedInner(BluetoothHostObserverStub *stub,
    MessageParcel &data, MessageParcel &reply)
{
    int32_t status = data.ReadInt32();

    HILOGD("OnDiscoveryStateChangedInner starts");
    stub->OnDiscoveryStateChanged(status);

    return NO_ERROR;
}

int32_t BluetoothHostObserverStub::OnDiscoveryResultInner(BluetoothHostObserverStub *stub,
    MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return TRANSACTION_ERR;
    }

    int32_t rssi = data.ReadInt32();
    std::string deviceName = data.ReadString();
    int32_t deviceClass = data.ReadInt32();
    HILOGD("OnDiscoveryResultInner starts");
    stub->OnDiscoveryResult(*device, rssi, deviceName, deviceClass);

    return NO_ERROR;
}

int32_t BluetoothHostObserverStub::OnPairRequestedInner(BluetoothHostObserverStub *stub,
    MessageParcel &data, MessageParcel &reply)
{
    int32_t transport = data.ReadInt32();
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return TRANSACTION_ERR;
    }

    HILOGI("OnPairRequestedInner starts");
    stub->OnPairRequested(transport, *device);

    return NO_ERROR;
}

int32_t BluetoothHostObserverStub::OnPairConfirmedInner(BluetoothHostObserverStub *stub,
    MessageParcel &data, MessageParcel &reply)
{
    int32_t transport = data.ReadInt32();
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    int32_t reqType = data.ReadInt32();
    int32_t number = data.ReadInt32();

    HILOGD("OnPairConfirmedInner starts");
    stub->OnPairConfirmed(transport, *device, reqType, number);

    return NO_ERROR;
}

int32_t BluetoothHostObserverStub::OnScanModeChangedInner(BluetoothHostObserverStub *stub,
    MessageParcel &data, MessageParcel &reply)
{
    int32_t mode = data.ReadInt32();

    HILOGI("OnScanModeChangedInner starts");
    stub->OnScanModeChanged(mode);

    return NO_ERROR;
}

int32_t BluetoothHostObserverStub::OnDeviceNameChangedInner(BluetoothHostObserverStub *stub,
    MessageParcel &data, MessageParcel &reply)
{
    std::string deviceName = data.ReadString();

    HILOGI("OnDeviceNameChangedInner starts");
    stub->OnDeviceNameChanged(deviceName);

    return NO_ERROR;
}

int32_t BluetoothHostObserverStub::OnDeviceAddrChangedInner(BluetoothHostObserverStub *stub,
    MessageParcel &data, MessageParcel &reply)
{
    std::string address = data.ReadString();

    HILOGI("OnDeviceAddrChangedInner starts");
    stub->OnDeviceAddrChanged(address);

    return NO_ERROR;
}

int32_t BluetoothHostObserverStub::OnBluetoothStateChangedInner(BluetoothHostObserverStub *stub,
    MessageParcel &data, MessageParcel &reply)
{
    int32_t state = data.ReadInt32();
    stub->OnBluetoothStateChanged(state);
    return NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS
