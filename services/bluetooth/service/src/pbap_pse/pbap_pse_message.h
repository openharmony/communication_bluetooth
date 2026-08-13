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

#ifndef PBAP_PSE_MESSAGE_H
#define PBAP_PSE_MESSAGE_H

#include <string>
#include "btcommon/message.h"
#include "btcommon/timer_manager.h"
#include "../obex/obex_socket_device.h"

namespace OHOS {
namespace bluetooth {
class PbapPseMessage : public utility::Message {
public:
    explicit PbapPseMessage(int what = 0, int arg1 = 0, void *arg2 = nullptr) : utility::Message(what, arg1, arg2)
    {}
    ~PbapPseMessage() override = default;
    std::string dev_ = "";
    int state_ = 0;
    bool isAccess_ = false;
    std::shared_ptr<ObexSocketDevice> socketDevice_ = nullptr;
    std::shared_ptr<utility::Timer> timer_ = nullptr;
};

enum {
    PBAP_PSE_INVALID_EVT = 0,
    PBAP_PSE_REQUEST_PERMISSION_EVT = 1,
    PBAP_PSE_VERIFY_RESULT_EVT = 2,
    PBAP_PSE_REMOVE_STATE_MACHINE_EVT = 3,
    PBAP_PSE_REQ_PREMISSION_TIMEOUT_EVT = 4,
    PBAP_PSE_CONNECT_EVT = 5,
    PBAP_PSE_DISCONNECT_EVT = 6,
    PBAP_PSE_STATE_CHANGED = 7,
};

enum PbapPseConnectState {
    PBAP_PSE_STATE_WAITFORAUTH = 0,
    PBAP_PSE_STATE_DISCONNECTED,
    PBAP_PSE_STATE_CONNECTED,
};
}  // namespace bluetooth
}
#endif // PBAP_PSE_MESSAGE_H
