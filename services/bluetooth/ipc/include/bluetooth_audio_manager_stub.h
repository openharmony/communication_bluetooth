/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#ifndef OHOS_BLUETOOTH_AUDIO_MANAGER_STUB_H
#define OHOS_BLUETOOTH_AUDIO_MANAGER_STUB_H

#include <map>
#include <iremote_stub.h>
#include "i_bluetooth_audio_manager.h"
#include "permission_item.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothAudioManagerStub : public IRemoteStub<IBluetoothAudioManager> {
public:
    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    BluetoothAudioManagerStub();
    virtual ~BluetoothAudioManagerStub();

    using AudioManagerStubFunc = std::function<int32_t(BluetoothAudioManagerStub *, MessageParcel &, MessageParcel &)>;
    using AudioManagerStubFuncPerm = std::pair<AudioManagerStubFunc, std::shared_ptr<PermissionItem>>;

private:
    int32_t EnableWearDetectionInner(MessageParcel &data, MessageParcel &reply);
    int32_t DisableWearDetectionInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetWearDetectionStateInner(MessageParcel &data, MessageParcel &reply);
    int32_t IsDeviceWearingInner(MessageParcel &data, MessageParcel &reply);
    int32_t SendDeviceSelectionInner(MessageParcel &data, MessageParcel &reply);
    int32_t IsWearDetectionSupportedInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetProfileStatusInner(MessageParcel &data, MessageParcel &reply);

private:
    static const std::map<uint32_t, AudioManagerStubFuncPerm> memberFuncMap_;
    DISALLOW_COPY_AND_MOVE(BluetoothAudioManagerStub);
};
}  // namespace Bluetooth
}  // namespace OHOS

#endif  // OHOS_BLUETOOTH_AUDIO_MANAGER_STUB_H
