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

#ifndef OPP_MESSAGE_H
#define OPP_MESSAGE_H

#include <string>
#include "btcommon/message.h"
#include "../sdp_adapter/sdp_adapter.h"
#include "../obex/obex_socket_device.h"

namespace OHOS {
namespace bluetooth {
class OppMessage : public utility::Message {
public:
    explicit OppMessage(int what = 0, int arg1 = 0, void *arg2 = nullptr) : utility::Message(what, arg1, arg2)
    {}
    ~OppMessage() override = default;
    std::string dev_  = "";
    OppSdpFoundRecord oppRecord_ = {0, 0, 0, 0, "", "", "", false};
    std::shared_ptr<ObexSocketDevice> socketDevice_ = nullptr;
};

enum OppConnectState {
    OPP_STATE_DISCONNECTED = 0,
    OPP_STATE_CONNECTING,
    OPP_STATE_DISCONNECTING,
    OPP_STATE_CONNECTED,
};

enum {
    OPP_INVALID_EVT = 0,
    OPP_CONNECT_REQ_EVT = 1,
    OPP_DISCONNECT_REQ_EVT = 2,
    OPP_CONNECTED_EVT = 3,
    OPP_DISCONNECTED_EVT = 4,
    OPP_CONNECTION_TIMEOUT_EVT = 5,
    OPP_DISCONNECTION_TIMEOUT_EVT = 6,
    OPP_REMOVE_STATE_MACHINE_EVT = 7,
    OPP_SDP_CMPL_EVT = 8,
    OPP_SERVICE_STARTUP_EVT = 11,
    OPP_SERVICE_SHUTDOWN_EVT = 12,
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // OPP_MESSAGE_H