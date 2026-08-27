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

#ifndef NATIVE_A2DP_ADAPTER_H
#define NATIVE_A2DP_ADAPTER_H

#include "i_native_a2dp_adapter.h"
#include "service_util.h"

namespace OHOS {
namespace bluetooth {

class NativeA2dpAdapter : public INativeA2dpAdapter {
public:
    explicit NativeA2dpAdapter(btav_sink_interface_t* iface);
    ~NativeA2dpAdapter() override;
    bt_status_t Init(INativeA2dpSinkCallback *callback, int maxDevices) override;
    bt_status_t Connect(const RawAddress& bdAddr) override;
    bt_status_t Disconnect(const RawAddress& bdAddr) override;
    void Cleanup() override;
    void SetAudioFocusState(int focusState) override;
    void SetAudioTrackGain(float gain) override;
    bt_status_t SetActiveDevice(const RawAddress& bdAddr) override;

private:
    // Bluedroid C 回调 — 转发为 INativeA2dpSinkCallback 接口调用
    static void BtavConnectionState(const STACK::RawAddress &bdAddr, btav_connection_state_t state);
    static void BtavAudioState(const STACK::RawAddress &bdAddr, btav_audio_state_t state);
    static void BtavAudioConfig(const STACK::RawAddress &bdAddr, uint32_t sampleRate, uint8_t channelCount);

    btav_sink_interface_t* iface_;
    static btav_sink_callbacks_t s_callbacks_;

    INativeA2dpSinkCallback *callback_{nullptr};
    // Bluedroid 回调是 C 函数指针，无法携带实例上下文。
    // A2DP Sink 全局仅一个 NativeA2dpAdapter 实例，用 static 指针桥接。
    static NativeA2dpAdapter *s_instance_;
};

} // namespace bluetooth
} // namespace OHOS
#endif
