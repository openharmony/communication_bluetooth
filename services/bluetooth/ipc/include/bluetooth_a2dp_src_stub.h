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

#ifndef OHOS_BLUETOOTH_STANDARD_A2DP_SRC_STUB_H
#define OHOS_BLUETOOTH_STANDARD_A2DP_SRC_STUB_H

#include <map>
#include <regex>
#include "iremote_stub.h"
#include "i_bluetooth_a2dp_src.h"
#include "permission_item.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothA2dpSrcStub : public IRemoteStub<IBluetoothA2dpSrc> {
public:
    BluetoothA2dpSrcStub();
    virtual ~BluetoothA2dpSrcStub();

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    using A2dpSrcStubFunc = std::function<int32_t(BluetoothA2dpSrcStub *, MessageParcel &, MessageParcel &)>;
    using A2dpSrcStubFuncPerm = std::pair<A2dpSrcStubFunc, std::shared_ptr<PermissionItem>>;

private:
    int32_t ConnectInner(MessageParcel &data, MessageParcel &reply);
    int32_t DisconnectInner(MessageParcel &data, MessageParcel &reply);
    int32_t RegisterObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t DeregisterObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetDevicesByStatesInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetDeviceStateInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetPlayingStateInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetConnectStrategyInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetConnectStrategyInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetActiveSinkDeviceInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetActiveSinkDeviceInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetCodecStatusInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetCodecPreferenceInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetCodecPreferenceInner(MessageParcel &data, MessageParcel &reply);
    int32_t SwitchOptionalCodecsInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetOptionalCodecsSupportStateInner(MessageParcel &data, MessageParcel &reply);
    int32_t StartPlayingInner(MessageParcel &data, MessageParcel &reply);
    int32_t SuspendPlayingInner(MessageParcel &data, MessageParcel &reply);
    int32_t StopPlayingInner(MessageParcel &data, MessageParcel &reply);
    int32_t WriteFrameInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetRenderPositionInner(MessageParcel &data, MessageParcel &reply);
    int32_t OffloadStartPlayingInner(MessageParcel &data, MessageParcel &reply);
    int32_t OffloadStopPlayingInner(MessageParcel &data, MessageParcel &reply);
    int32_t A2dpOffloadSessionPathRequestInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetOffloadCodecStatusInner(MessageParcel &data, MessageParcel &reply);
    int32_t EnableAutoPlayInner(MessageParcel &data, MessageParcel &reply);
    int32_t DisableAutoPlayInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetAutoPlayDisabledDurationInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetVirtualDeviceListInner(MessageParcel &data, MessageParcel &reply);

    bool IsValidAddress(std::string addr);

private:
    static const std::map<uint32_t, A2dpSrcStubFuncPerm> memberFuncMap_;
    DISALLOW_COPY_AND_MOVE(BluetoothA2dpSrcStub);
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_A2DP_SRC_STUB_H