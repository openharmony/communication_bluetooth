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

#ifndef LOG_TAG
#define LOG_TAG "bt_service_native_a2dp_adapter"
#endif

#include "native_a2dp_adapter.h"
#include "log.h"
#include "service_util.h"

namespace OHOS {
namespace bluetooth {

// Bluedroid 回调表 — 指向 static 函数
BtavSinkCallbacks NativeA2dpAdapter::s_callbacks_ = {
    sizeof(BtavSinkCallbacks),
    BtavConnectionState,
    BtavAudioState,
    BtavAudioConfig
};

NativeA2dpAdapter *NativeA2dpAdapter::s_instance_ = nullptr;

NativeA2dpAdapter::NativeA2dpAdapter(BtavSinkInterface* iface) : iface_(iface)
{
    s_instance_ = this;
}

NativeA2dpAdapter::~NativeA2dpAdapter()
{
    if (s_instance_ == this) {
        s_instance_ = nullptr;
    }
}

BtStackStatus NativeA2dpAdapter::Init(INativeA2dpSinkCallback *callback, int maxDevices)
{
    if (!iface_ || !iface_->init) {
        HILOGE("interface or init is null");
        return BT_STATUS_FAIL;
    }
    callback_ = callback;
    return iface_->init(&s_callbacks_, maxDevices);
}

BtStackStatus NativeA2dpAdapter::Connect(const RawAddress& bdAddr)
{
    if (!iface_ || !iface_->connect) {
        HILOGE("interface or connect is null");
        return BT_STATUS_FAIL;
    }
    return iface_->connect(bdAddr);
}

BtStackStatus NativeA2dpAdapter::Disconnect(const RawAddress& bdAddr)
{
    if (!iface_ || !iface_->disconnect) {
        HILOGE("interface or disconnect is null");
        return BT_STATUS_FAIL;
    }
    return iface_->disconnect(bdAddr);
}

void NativeA2dpAdapter::Cleanup()
{
    if (iface_ && iface_->cleanup) {
        iface_->cleanup();
    }
    iface_ = nullptr;
    callback_ = nullptr;
}

void NativeA2dpAdapter::SetAudioFocusState(int focusState)
{
    if (iface_ && iface_->setAudioFocusState) {
        iface_->setAudioFocusState(focusState);
    }
}

void NativeA2dpAdapter::SetAudioTrackGain(float gain)
{
    if (iface_ && iface_->setAudioTrackGain) {
        iface_->setAudioTrackGain(gain);
    }
}

BtStackStatus NativeA2dpAdapter::SetActiveDevice(const RawAddress& bdAddr)
{
    if (!iface_ || !iface_->setActiveDevice) {
        HILOGE("interface or set_active_device is null");
        return BT_STATUS_FAIL;
    }
    return iface_->setActiveDevice(bdAddr);
}

// --- trampoline 实现 ---

void NativeA2dpAdapter::BtavConnectionState(const OHOS::bluetooth::RawAddress &bdAddr, BtavConnectionState state)
{
    if (s_instance_ && s_instance_->callback_) {
        s_instance_->callback_->OnNativeConnectionStateChanged(bdAddr, state);
    }
}

void NativeA2dpAdapter::BtavAudioState(const OHOS::bluetooth::RawAddress &bdAddr, BtavAudioState state)
{
    if (s_instance_ && s_instance_->callback_) {
        s_instance_->callback_->OnNativeAudioStateChanged(bdAddr, state);
    }
}

void NativeA2dpAdapter::BtavAudioConfig(const OHOS::bluetooth::RawAddress &bdAddr, uint32_t sampleRate, uint8_t channelCount)
{
    if (s_instance_ && s_instance_->callback_) {
        s_instance_->callback_->OnNativeAudioConfigChanged(bdAddr, sampleRate, channelCount);
    }
}

} // namespace bluetooth
} // namespace OHOS
