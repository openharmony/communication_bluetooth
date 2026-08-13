/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef I_NATIVE_A2DP_ADAPTER_H
#define I_NATIVE_A2DP_ADAPTER_H

#include "bt_av.h"
#include "raw_address.h"

namespace OHOS {
namespace bluetooth {

/**
 * @brief Bluedroid A2DP Sink 回调接口。
 *
 * NativeA2dpAdapter 收到 Bluedroid 栈回调后通过此接口转发给上层，
 * 解耦 static 回调与 A2dpSnkService 单例之间的强耦合。
 */
class INativeA2dpSinkCallback {
public:
    virtual ~INativeA2dpSinkCallback() = default;

    // 连接状态变化回调（来自 Bluedroid 协议栈）
    virtual void OnNativeConnectionStateChanged(const RawAddress &device, btav_connection_state_t state) = 0;

    // 音频流状态变化回调（来自 Bluedroid 协议栈）
    virtual void OnNativeAudioStateChanged(const RawAddress &device, btav_audio_state_t state) = 0;

    // 音频配置变化回调（来自 Bluedroid 协议栈）
    virtual void OnNativeAudioConfigChanged(const RawAddress &device, uint32_t sampleRate, uint8_t channelCount) = 0;
};

class INativeA2dpAdapter {
public:
    virtual ~INativeA2dpAdapter() = default;
    virtual bt_status_t Init(INativeA2dpSinkCallback *callback, int maxDevices) = 0;
    virtual bt_status_t Connect(const RawAddress& bdAddr) = 0;
    virtual bt_status_t Disconnect(const RawAddress& bdAddr) = 0;
    virtual void Cleanup() = 0;
    virtual void SetAudioFocusState(int focusState) = 0;
    virtual void SetAudioTrackGain(float gain) = 0;
    virtual bt_status_t SetActiveDevice(const RawAddress& bdAddr) = 0;
};

} // namespace bluetooth
} // namespace OHOS
#endif
