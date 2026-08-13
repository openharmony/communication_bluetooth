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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_audio_manager"
#endif

#include "bluetooth_audio_manager_interface.h"
#include "bluetooth_audio_manager.h"

namespace OHOS {
namespace bluetooth {

int32_t BluetoothAudioManagerInterface::BtAudioManagerSetSupport(std::string addr, bool enable)
{
    return BluetoothAudioManager::GetInstance().SetAudioManagerSupport(addr, enable);
}

int32_t BluetoothAudioManagerInterface::BtAudioManagerGetWearState(std::string addr, int32_t &ability)
{
    return BluetoothAudioManager::GetInstance().GetWearState(addr, ability);
}

int32_t BluetoothAudioManagerInterface::IsDeviceWearing(const RawAddress &address)
{
    return BluetoothAudioManager::GetInstance().IsDeviceWearing(address);
}

int32_t BluetoothAudioManagerInterface::SendDeviceSelection(const RawAddress &address,
    int useA2dp, int useHfp, int userSelect)
{
    return BluetoothAudioManager::GetInstance().SendDeviceSelection(address, useA2dp, useHfp, userSelect);
}

int32_t BluetoothAudioManagerInterface::IsWearDetectionSupported(const RawAddress &address, bool &isSupported)
{
    return BluetoothAudioManager::GetInstance().IsWearDetectionSupported(address, isSupported);
}

int32_t BluetoothAudioManagerInterface::GetProfileStatus(const RawAddress &address,
    uint8_t &a2dpState, uint8_t &hfpState)
{
    return BluetoothAudioManager::GetInstance().GetProfileStatus(address.GetAddress(), a2dpState, hfpState);
}
}
}

