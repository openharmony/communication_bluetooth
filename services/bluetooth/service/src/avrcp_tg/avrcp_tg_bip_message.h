/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef AVRCP_TG_BIP_MESSAGE_H
#define AVRCP_TG_BIP_MESSAGE_H

#include <string>
#include "btcommon/message.h"
#include "../obex/obex_socket_device.h"

namespace OHOS {
namespace bluetooth {

enum BipStateMachineEvent {
    BIP_INVALID_EVT = 0,
    BIP_CONNECT_EVT = 1,
    BIP_DISCONNECT_EVT = 2,
    BIP_REMOVE_STATE_MACHINE_EVT = 3,
};

enum BipStateMachineState {
    BIP_STATE_DISCONNECTED = 0,
    BIP_STATE_CONNECTED = 1
};

class BipMessage : public utility::Message {
public:
    explicit BipMessage(int what = 0, int arg1 = 0, void *arg2 = nullptr) : utility::Message(what, arg1, arg2)
    {}
    ~BipMessage() override = default;
    std::string dev_ = "";
    std::shared_ptr<ObexSocketDevice> socketDevice_ = nullptr;
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // AVRCP_TG_BIP_MESSAGE_H