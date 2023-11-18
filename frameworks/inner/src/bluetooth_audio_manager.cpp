/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
 
#include "bluetooth_audio_manager.h"
#include "i_bluetooth_audio_manager.h"
#include "i_bluetooth_host.h"
#include "bluetooth_utils.h"
 
 
namespace OHOS {
namespace Bluetooth {
struct BtAudioManager::impl {
    impl();
    ~impl()
    {}
    int EnableBtAudioManager(const std::string &deviceId, int32_t supportVal);
    int DisableBtAudioManager(const std::string &deviceId, int32_t supportVal);
    int IsBtAudioManagerEnabled(const std::string &deviceId);
    sptr<IBluetoothAudioManager> proxy_;
};
 
BtAudioManager::impl::impl()
{
    proxy_ = GetRemoteProxy<IBluetoothAudioManager>(PROFILE_AUDIO_MANAGER);
}
 
BtAudioManager::BtAudioManager():pimpl(std::make_unique<impl>())
{}
 
int BtAudioManager::impl::EnableBtAudioManager(const std :: string & deviceId, int32_t supportVal)
{
    if (proxy_ == nullptr) {
    } else {
        proxy_->EnableBtAudioManager(deviceId, supportVal);
    }
    return 0;
}
 
int BtAudioManager::impl::DisableBtAudioManager(const std :: string & deviceId, int32_t supportVal)
{
    if (proxy_ == nullptr) {
    } else {
        proxy_->DisableBtAudioManager(deviceId, supportVal);
    }
    return 0;
}
 
int BtAudioManager::impl::IsBtAudioManagerEnabled(const std :: string & deviceId)
{
    if (proxy_ == nullptr) {
        return -1;
    } else {
        return proxy_->IsBtAudioManagerEnabled(deviceId);
    }
}
 
int BtAudioManager::EnableBtAudioManager(const std::string &deviceId, int32_t supportVal)
{
    pimpl->EnableBtAudioManager(deviceId, supportVal);
    return 0;
}
 
int BtAudioManager::DisableBtAudioManager(const std::string &deviceId, int32_t supportVal)
{
    pimpl->DisableBtAudioManager(deviceId, supportVal);
    return 0;
}
 
int BtAudioManager::IsBtAudioManagerEnabled(const std::string &deviceId)
{
    return pimpl->IsBtAudioManagerEnabled(deviceId);
}
 
BtAudioManager &BtAudioManager::GetInstance()
{
    static BtAudioManager instance;
    return instance;
}
 
}  // namespace Bluetooth
}  // namespace OHOS