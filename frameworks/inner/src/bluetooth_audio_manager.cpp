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

#include "bluetooth_audio_manager.h"
#include "i_bluetooth_audio_manager.h"
#include "i_bluetooth_host.h"
#include "bluetooth_utils.h"

namespace OHOS {
namespace Bluetooth {
struct BluetoothAudioManager::impl {
    impl();
    ~impl()
    {}
    int EnableWearDetection(const std::string &deviceId, int32_t supportVal);
    int DisableWearDetection(const std::string &deviceId, int32_t supportVal);
    int IsWearDetectionEnabled(const std::string &deviceId, int32_t &ability);
    sptr<IBluetoothAudioManager> proxy_;
};

BluetoothAudioManager::impl::impl()
{
    proxy_ = GetRemoteProxy<IBluetoothAudioManager>(PROFILE_AUDIO_MANAGER);
}

BluetoothAudioManager::BluetoothAudioManager():pimpl(std::make_unique<impl>())
{}

int BluetoothAudioManager::impl::EnableWearDetection(const std::string & deviceId, int32_t supportVal)
{
    if (proxy_ == nullptr) {
        HILOGE("proxy_ is null");
        return BT_ERR_INVALID_STATE;
    }
    return proxy_->EnableWearDetection(deviceId, supportVal);
}

int BluetoothAudioManager::impl::DisableWearDetection(const std::string & deviceId, int32_t supportVal)
{
    if (proxy_ == nullptr) {
        HILOGE("proxy_ is null");
        return BT_ERR_INVALID_STATE;
    }
    return proxy_->DisableWearDetection(deviceId, supportVal);
}

int BluetoothAudioManager::impl::IsWearDetectionEnabled(const std::string & deviceId, int32_t &ability)
{
    if (proxy_ == nullptr) {
        HILOGE("proxy_ is null");
        return BT_ERR_INVALID_STATE;
    }
    return proxy_->IsWearDetectionEnabled(deviceId, ability);
}

int BluetoothAudioManager::EnableWearDetection(const std::string &deviceId, int32_t supportVal)
{
    if (pimpl == nullptr) {
        HILOGE("pimpl is null");
        return BT_ERR_INVALID_STATE;
    }
    return pimpl->EnableWearDetection(deviceId, supportVal);
}

int BluetoothAudioManager::DisableWearDetection(const std::string &deviceId, int32_t supportVal)
{
    if (pimpl == nullptr) {
        HILOGE("pimpl is null");
        return BT_ERR_INVALID_STATE;
    }
    return pimpl->DisableWearDetection(deviceId, supportVal);
}

int BluetoothAudioManager::IsWearDetectionEnabled(const std::string &deviceId, int32_t &ability)
{
    if (pimpl == nullptr) {
        HILOGE("pimpl is null");
        return BT_ERR_INVALID_STATE;
    }
    return pimpl->IsWearDetectionEnabled(deviceId, ability);
}

BluetoothAudioManager &BluetoothAudioManager::GetInstance()
{
    static BluetoothAudioManager instance;
    return instance;
}

}  // namespace Bluetooth
}  // namespace OHOS
