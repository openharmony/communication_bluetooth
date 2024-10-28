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
#define LOG_TAG "bt_ipc_hfp_hf_observer_stub"
#endif

#include "bluetooth_hfp_hf_observer_stub.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
BluetoothHfpHfObserverStub::BluetoothHfpHfObserverStub()
{
    HILOGD("start.");
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothHfpHfObserverInterfaceCode::BT_HFP_HF_OBSERVER_CONNECTION_STATE_CHANGED)] =
        BluetoothHfpHfObserverStub::OnConnectionStateChangedInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothHfpHfObserverInterfaceCode::BT_HFP_HF_OBSERVER_SCO_STATE_CHANGED)] =
        BluetoothHfpHfObserverStub::OnScoStateChangedInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothHfpHfObserverInterfaceCode::BT_HFP_HF_OBSERVER_CALL_CHANGED)] =
        BluetoothHfpHfObserverStub::OnCallChangedInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothHfpHfObserverInterfaceCode::BT_HFP_HF_OBSERVER_SIGNAL_STRENGTH_CHANGED)] =
        BluetoothHfpHfObserverStub::OnSignalStrengthChangedInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothHfpHfObserverInterfaceCode::BT_HFP_HF_OBSERVER_REGISTRATION_STATUS_CHANGED)] =
        BluetoothHfpHfObserverStub::OnRegistrationStatusChangedInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothHfpHfObserverInterfaceCode::BT_HFP_HF_OBSERVER_ROAMING_STATUS_CHANGED)] =
        BluetoothHfpHfObserverStub::OnRoamingStatusChangedInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothHfpHfObserverInterfaceCode::BT_HFP_HF_OBSERVER_OPERATOR_SELECTION_CHANGED)] =
        BluetoothHfpHfObserverStub::OnOperatorSelectionChangedInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothHfpHfObserverInterfaceCode::BT_HFP_HF_OBSERVER_SUBSCRIBER_NUMBER_CHANGED)] =
        BluetoothHfpHfObserverStub::OnSubscriberNumberChangedInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothHfpHfObserverInterfaceCode::BT_HFP_HF_OBSERVER_VOICE_RECOGNITION_STATUS_CHANGED)] =
        BluetoothHfpHfObserverStub::OnVoiceRecognitionStatusChangedInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothHfpHfObserverInterfaceCode::BT_HFP_HF_OBSERVER_IN_BAND_RING_TONE_CHANGED)] =
        BluetoothHfpHfObserverStub::OnInBandRingToneChangedInner;

    HILOGI("ends.");
}

BluetoothHfpHfObserverStub::~BluetoothHfpHfObserverStub()
{
    HILOGD("start.");
    memberFuncMap_.clear();
}

int BluetoothHfpHfObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOGD("BluetoothHfpHfObserverStub::OnRemoteRequest, cmd = %{public}d, flags= %{public}d", code, option.GetFlags());
    if (BluetoothHfpHfObserverStub::GetDescriptor() != data.ReadInterfaceToken()) {
        HILOGI("local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return memberFunc(this, data, reply);
        }
    }
    HILOGW("BluetoothHfpHfObserverStub::OnRemoteRequest, default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

ErrCode BluetoothHfpHfObserverStub::OnConnectionStateChangedInner(
    BluetoothHfpHfObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    int state = data.ReadInt32();
    int cause = data.ReadInt32();
    stub->OnConnectionStateChanged(*device, state, cause);
    return NO_ERROR;
}

ErrCode BluetoothHfpHfObserverStub::OnScoStateChangedInner(
    BluetoothHfpHfObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    int state = data.ReadInt32();
    stub->OnScoStateChanged(*device, state);
    return NO_ERROR;
}

ErrCode BluetoothHfpHfObserverStub::OnCallChangedInner(
    BluetoothHfpHfObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    sptr<BluetoothHfpHfCall> call = data.ReadParcelable<BluetoothHfpHfCall>();
    if (!call) {
        return TRANSACTION_ERR;
    }
    stub->OnCallChanged(*device, *call);
    return NO_ERROR;
}

ErrCode BluetoothHfpHfObserverStub::OnSignalStrengthChangedInner(
    BluetoothHfpHfObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    int signal = data.ReadInt32();
    stub->OnSignalStrengthChanged(*device, signal);
    return NO_ERROR;
}

ErrCode BluetoothHfpHfObserverStub::OnRegistrationStatusChangedInner(
    BluetoothHfpHfObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    int status = data.ReadInt32();
    stub->OnSignalStrengthChanged(*device, status);
    return NO_ERROR;
}

ErrCode BluetoothHfpHfObserverStub::OnRoamingStatusChangedInner(
    BluetoothHfpHfObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    int status = data.ReadInt32();
    stub->OnRoamingStatusChanged(*device, status);
    return NO_ERROR;
}

ErrCode BluetoothHfpHfObserverStub::OnOperatorSelectionChangedInner(
    BluetoothHfpHfObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    std::string name = data.ReadString();
    stub->OnOperatorSelectionChanged(*device, name);
    return NO_ERROR;
}

ErrCode BluetoothHfpHfObserverStub::OnSubscriberNumberChangedInner(
    BluetoothHfpHfObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    std::string number = data.ReadString();
    stub->OnSubscriberNumberChanged(*device, number);
    return NO_ERROR;
}

ErrCode BluetoothHfpHfObserverStub::OnVoiceRecognitionStatusChangedInner(
    BluetoothHfpHfObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    int status = data.ReadInt32();
    stub->OnVoiceRecognitionStatusChanged(*device, status);
    return NO_ERROR;
}

ErrCode BluetoothHfpHfObserverStub::OnInBandRingToneChangedInner(
    BluetoothHfpHfObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return TRANSACTION_ERR;
    }
    int status = data.ReadInt32();
    stub->OnInBandRingToneChanged(*device, status);
    return NO_ERROR;
}

}  // namespace Bluetooth
}  // namespace OHOS
