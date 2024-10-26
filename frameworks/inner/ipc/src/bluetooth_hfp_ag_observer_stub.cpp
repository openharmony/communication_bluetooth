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
#define LOG_TAG "bt_ipc_hfp_ag_observer_stub"
#endif

#include "bluetooth_errorcode.h"
#include "bluetooth_hfp_ag_observer_stub.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
BluetoothHfpAgObserverStub::BluetoothHfpAgObserverStub()
{
    HILOGD("start.");
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothHfpAgObserverInterfaceCode::BT_HFP_AG_OBSERVER_CONNECTION_STATE_CHANGED)] =
        &BluetoothHfpAgObserverStub::OnConnectionStateChangedInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothHfpAgObserverInterfaceCode::BT_HFP_AG_OBSERVER_SCO_STATE_CHANGED)] =
        &BluetoothHfpAgObserverStub::OnScoStateChangedInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothHfpAgObserverInterfaceCode::BT_HFP_AG_OBSERVER_ACTIVE_DEVICE_CHANGED)] =
        &BluetoothHfpAgObserverStub::OnActiveDeviceChangedInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothHfpAgObserverInterfaceCode::BT_HFP_AG_OBSERVER_HF_ENHANCED_DRIVER_SAFETY_CHANGED)] =
        &BluetoothHfpAgObserverStub::OnHfEnhancedDriverSafetyChangedInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothHfpAgObserverInterfaceCode::BT_HFP_AG_OBSERVER_HFP_STACK_CHANGED)] =
        &BluetoothHfpAgObserverStub::OnHfpStackChangedInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothHfpAgObserverInterfaceCode::BT_HFP_AG_OBSERVER_VIRTUALDEVICE_CHANGED)] =
        &BluetoothHfpAgObserverStub::OnVirtualDeviceChangedInner;

    HILOGD("ends.");
}

BluetoothHfpAgObserverStub::~BluetoothHfpAgObserverStub()
{
    HILOGD("start.");
    memberFuncMap_.clear();
}

int BluetoothHfpAgObserverStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    HILOGD("BluetoothHfpAgObserverStub::OnRemoteRequest, cmd = %{public}d, flags= %{public}d", code, option.GetFlags());
    if (BluetoothHfpAgObserverStub::GetDescriptor() != data.ReadInterfaceToken()) {
        HILOGI("local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    HILOGW("BluetoothHfpAgObserverStub::OnRemoteRequest, default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

ErrCode BluetoothHfpAgObserverStub::OnConnectionStateChangedInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    int state = data.ReadInt32();
    int cause = data.ReadInt32();
    OnConnectionStateChanged(*device, state, cause);
    return BT_NO_ERROR;
}

ErrCode BluetoothHfpAgObserverStub::OnScoStateChangedInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    int state = data.ReadInt32();
    int reason = data.ReadInt32();
    OnScoStateChanged(*device, state, reason);
    return BT_NO_ERROR;
}

ErrCode BluetoothHfpAgObserverStub::OnActiveDeviceChangedInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    OnActiveDeviceChanged(*device);
    return BT_NO_ERROR;
}

ErrCode BluetoothHfpAgObserverStub::OnHfEnhancedDriverSafetyChangedInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    int indValue = data.ReadInt32();
    OnScoStateChanged(*device, indValue);
    return BT_NO_ERROR;
}

int32_t BluetoothHfpAgObserverStub::OnHfpStackChangedInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    CHECK_AND_RETURN_LOG_RET(device, BT_ERR_INTERNAL_ERROR,
        "BluetoothHfpAgObserverStub::OnHfpStackChangedInnerInner error, service is null");
    int action = data.ReadInt32();
    OnHfpStackChanged(*device, action);
    return BT_NO_ERROR;
}

ErrCode BluetoothHfpAgObserverStub::OnVirtualDeviceChangedInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t action = data.ReadInt32();
    std::string addr = data.ReadString();

    OnVirtualDeviceChanged(action, addr);
    
    return BT_NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS
