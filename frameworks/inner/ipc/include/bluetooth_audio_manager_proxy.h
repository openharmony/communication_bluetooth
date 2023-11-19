/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_BLUETOOTH_AUDIOMANAGER_PROXY_H
#define OHOS_BLUETOOTH_AUDIOMANAGER_PROXY_H
 
#include <iremote_proxy.h>
#include "i_bluetooth_audio_manager.h"
 
namespace OHOS {
namespace Bluetooth {
class BluetoothAudioManagerProxy : public IRemoteProxy<IBluetoothAudioManager> {
public:
    explicit BluetoothAudioManagerProxy(const sptr<IRemoteObject>& remote)
        : IRemoteProxy<IBluetoothAudioManager>(remote)
    {}
    virtual ~BluetoothAudioManagerProxy()
    {}
 
    int EnableBtAudioManager(const std::string &deviceId, int32_t supportVal) override;
    int DisableBtAudioManager(const std::string &deviceId, int32_t supportVal) override;
    int IsBtAudioManagerEnabled(const std::string &deviceId) override;

private:
    static inline BrokerDelegator<BluetoothAudioManagerProxy> delegator_;
};
}  // namespace Bluetooth
}  // namespace OHOS
 
#endif  // OHOS_BLUETOOTH_AUDIOMANAGER_PROXY_H