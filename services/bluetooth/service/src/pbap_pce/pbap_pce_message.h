/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef PBAP_PCE_MESSAGE_H
#define PBAP_PCE_MESSAGE_H

#include <string>
#include "btcommon/message.h"
#include "btcommon/timer_manager.h"
#include "../obex/obex_socket_device.h"
#include "pbap_pce_header_msg.h"
#include "raw_address.h"

namespace OHOS {
namespace bluetooth {
class PbapPceMessage : public utility::Message {
public:
    explicit PbapPceMessage(int what = 0, int arg1 = 0, void *arg2 = nullptr) : utility::Message(what, arg1, arg2)
    {}
    ~PbapPceMessage() override = default;
    RawAddress dev_ {};
    PbapPcePullPhoneBookMsg phonebook_ {};
    std::string name_ = "";
    int flag_ = 0;
    PbapPcePullvCardListingMsg vcard_ {};
    PbapPcePullvCardEntryMsg vcardEntry_ {};
    utility::Message msg_ {};
};

enum {
    PBAP_PCE_CONNECT_EVT = 0,
    PBAP_PCE_DISCONNECT_EVT,
    PBAP_PCE_PULL_PHONE_BOOK_EVT,
    PBAP_PCE_SET_PHONE_BOOK_EVT ,
    PBAP_PCE_PULL_VCARD_LISTING_EVT,
    PBAP_PCE_PULL_VCARD_ENTRY_EVT,
    PBAP_PCE_ABORT_DOWNLOADING_EVT
};

}  // namespace bluetooth
}
#endif // PBAP_PCE_MESSAGE_H
