/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

/*
 * Stub of the removed stack layer A2DP audio render interface
 * (btif/a2dp_audio_render_interface.h).
 */

#ifndef BTIF_A2DP_AUDIO_RENDER_INTERFACE_H
#define BTIF_A2DP_AUDIO_RENDER_INTERFACE_H

#include <cstddef>
#include <cstdint>
#include <memory>

#include "bt_types.h"

/* Abstract audio render sink consumed by the A2DP sink profile; the service
 * layer registers an AudioRenderer backed implementation (A2dpSnkAudioRender)
 * through SetInstance, the removed stack layer drives it through the virtual
 * methods while streaming. */
class IA2dpAudioRender {
public:
    IA2dpAudioRender() = default;
    virtual ~IA2dpAudioRender() = default;

    virtual bool CreateRender(int freq, int bits, int channels) = 0;
    virtual bool StartRender() = 0;
    virtual bool StopRender() = 0;
    virtual bool PauseRender() = 0;
    virtual bool ReleaseRender() = 0;
    virtual bool WriteStream(uint8_t *buffer, size_t size) = 0;

    static void SetInstance(std::shared_ptr<IA2dpAudioRender> instance);
    static std::shared_ptr<IA2dpAudioRender> GetInstance();
};

struct BtavA2dpAudioRenderInterface {
    size_t size;
    int (*init)(int freq, int bits, int channels);
    void (*cleanup)(void);
    int (*start)(void);
    int (*stop)(void);
    int (*writeData)(uint8_t *buffer, size_t size);
};

#endif  // BTIF_A2DP_AUDIO_RENDER_INTERFACE_H
