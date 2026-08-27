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

#ifndef BLUETOOTH_HEARING_AID_H
#define BLUETOOTH_HEARING_AID_H

#include <cstdint>

#include "types/raw_address.h"

#ifndef STACK
#define STACK
#endif

namespace bluetooth {
namespace hearing_aid {

/* Connection state of the hearing aid profile. */
enum class ConnectionState : uint8_t {
    DISCONNECTED = 0,
    CONNECTING,
    CONNECTED,
    DISCONNECTING
};

/* Callback interface dispatched from the hearing aid profile. */
class HearingAidCallbacks {
public:
    virtual ~HearingAidCallbacks() = default;

    virtual void OnConnectionState(ConnectionState state,
                                   const STACK::RawAddress& address) = 0;
    virtual void OnDeviceAvailable(uint8_t capabilities, uint64_t hiSyncId,
                                   const STACK::RawAddress& address) = 0;
};

/* Interface exposed by the hearing aid profile. */
class HearingAidInterface {
public:
    virtual ~HearingAidInterface() = default;

    virtual void Init(HearingAidCallbacks* callbacks) = 0;
    virtual void Cleanup() = 0;
    virtual void Connect(const STACK::RawAddress& address) = 0;
    virtual void Disconnect(const STACK::RawAddress& address) = 0;
    virtual void AddToAcceptlist(const STACK::RawAddress& address) = 0;
    virtual void SetVolumeByAddr(const STACK::RawAddress& address,
                                 int volume) = 0;
    virtual void StartSendingAudio(const STACK::RawAddress& address) = 0;
};

}  // namespace hearing_aid
}  // namespace bluetooth

#endif  // BLUETOOTH_HEARING_AID_H
