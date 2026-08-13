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
#ifndef OHOS_BLUETOOTH_STANDARD_HFP_AG_STUB_H
#define OHOS_BLUETOOTH_STANDARD_HFP_AG_STUB_H

#include <map>
#include "iremote_stub.h"
#include "i_bluetooth_hfp_ag.h"
#include "permission_item.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothHfpAgStub : public IRemoteStub<IBluetoothHfpAg> {
public:
    BluetoothHfpAgStub();
    ~BluetoothHfpAgStub();

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    using HfpAgStubFunc = std::function<int32_t(BluetoothHfpAgStub *, MessageParcel &, MessageParcel &)>;
    using HfpAgStubFuncPerm = std::pair<HfpAgStubFunc, std::shared_ptr<PermissionItem>>;

private:
    int32_t GetConnectDevicesInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetDevicesByStatesInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetDeviceStateInner(MessageParcel &data, MessageParcel &reply);
    int32_t ConnectInner(MessageParcel &data, MessageParcel &reply);
    int32_t DisconnectInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetScoStateInner(MessageParcel &data, MessageParcel &reply);
    int32_t ConnectScoInnerEx(MessageParcel &data, MessageParcel &reply);
    int32_t DisconnectScoInnerEx(MessageParcel &data, MessageParcel &reply);
    int32_t ConnectScoInner(MessageParcel &data, MessageParcel &reply);
    int32_t DisconnectScoInner(MessageParcel &data, MessageParcel &reply);
    int32_t PhoneStateChangedInner(MessageParcel &data, MessageParcel &reply);
    int32_t ClccResponseInner(MessageParcel &data, MessageParcel &reply);
    int32_t OpenVoiceRecognitionInner(MessageParcel &data, MessageParcel &reply);
    int32_t CloseVoiceRecognitionInner(MessageParcel &data, MessageParcel &reply);
    int32_t IsAudioConnectedInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetActiveDeviceInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetActiveDeviceInner(MessageParcel &data, MessageParcel &reply);
    int32_t IntoMockInner(MessageParcel &data, MessageParcel &reply);
    int32_t SendNoCarrierInner(MessageParcel &data, MessageParcel &reply);
    int32_t RegisterObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t DeregisterObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetConnectStrategyInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetConnectStrategyInner(MessageParcel &data, MessageParcel &reply);
    int32_t IsInbandRingingEnabledInner(MessageParcel &data, MessageParcel &reply);
    int32_t CallDetailsChangedInner(MessageParcel &data, MessageParcel &reply);
    int32_t IsHfpFeatureSupportedInner(MessageParcel &data, MessageParcel &reply);
    int32_t EnableBtCallLogInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetVirtualDeviceListInner(MessageParcel &data, MessageParcel &reply);
    int32_t UpdateVirtualDeviceInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetCurrentCallTypeInner(MessageParcel &data, MessageParcel &reply);

private:
    static const std::map<uint32_t, HfpAgStubFuncPerm> memberFuncMap_;
    DISALLOW_COPY_AND_MOVE(BluetoothHfpAgStub);
};

}  // namespace Bluetooth
}  // namespace OHOS

#endif  // OHOS_BLUETOOTH_STANDARD_HFP_AG_PROXY_H