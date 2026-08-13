/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#ifndef OHOS_BLUETOOTH_STANDARD_HFP_HF_STUB_H
#define OHOS_BLUETOOTH_STANDARD_HFP_HF_STUB_H

#include <map>
#include "iremote_stub.h"
#include "i_bluetooth_hfp_hf.h"
#include "permission_item.h"

namespace OHOS {
namespace Bluetooth {

class BluetoothHfpHfStub : public IRemoteStub<IBluetoothHfpHf> {
public:
    BluetoothHfpHfStub();
    ~BluetoothHfpHfStub();

    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    using HfpHfStubFunc = std::function<int32_t(BluetoothHfpHfStub *, MessageParcel &, MessageParcel &)>;
    using HfpHfStubFuncPerm = std::pair<HfpHfStubFunc, std::shared_ptr<PermissionItem>>;

private:
    int32_t ConnectScoInner(MessageParcel &data, MessageParcel &reply);
    int32_t DisconnectScoInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetDevicesByStatesInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetDeviceStateInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetScoStateInner(MessageParcel &data, MessageParcel &reply);
    int32_t SendDTMFToneInner(MessageParcel &data, MessageParcel &reply);
    int32_t ConnectInner(MessageParcel &data, MessageParcel &reply);
    int32_t DisconnectInner(MessageParcel &data, MessageParcel &reply);
    int32_t OpenVoiceRecognitionInner(MessageParcel &data, MessageParcel &reply);
    int32_t CloseVoiceRecognitionInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetCurrentCallListInner(MessageParcel &data, MessageParcel &reply);
    int32_t AcceptIncomingCallInner(MessageParcel &data, MessageParcel &reply);
    int32_t HoldActiveCallInner(MessageParcel &data, MessageParcel &reply);
    int32_t RejectIncomingCallInner(MessageParcel &data, MessageParcel &reply);
    int32_t SendKeyPressedInner(MessageParcel &data, MessageParcel &reply);
    int32_t HandleIncomingCallInner(MessageParcel &data, MessageParcel &reply);
    int32_t HandleMultiCallInner(MessageParcel &data, MessageParcel &reply);
    int32_t DialLastNumberInner(MessageParcel &data, MessageParcel &reply);
    int32_t DialMemoryInner(MessageParcel &data, MessageParcel &reply);
    int32_t SendVoiceTagInner(MessageParcel &data, MessageParcel &reply);
    int32_t FinishActiveCallInner(MessageParcel &data, MessageParcel &reply);
    int32_t StartDialInner(MessageParcel &data, MessageParcel &reply);
    int32_t RegisterObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t DeregisterObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetConnectStrategyInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetConnectStrategyInner(MessageParcel &data, MessageParcel &reply);

private:
    static const std::map<uint32_t, HfpHfStubFuncPerm> memberFuncMap_;
    DISALLOW_COPY_AND_MOVE(BluetoothHfpHfStub);
};

}  // namespace Bluetooth
}  // namespace OHOS

#endif  // OHOS_BLUETOOTH_STANDARD_HFP_HF_STUB_H