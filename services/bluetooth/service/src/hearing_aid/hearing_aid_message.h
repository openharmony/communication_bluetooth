/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef HEARING_AID_MESSAGE_H
#define HEARING_AID_MESSAGE_H

#include <string>

#include "hearing_aid_defines.h"
#include "btcommon/message.h"
#include "securec.h"

namespace OHOS {
namespace bluetooth {
class HearingAidMessage : public utility::Message {
public:
    explicit HearingAidMessage(const HearingAidMessage &src)
        : utility::Message(src.what_, src.arg1_, src.arg2_), msgAddr(src.msgAddr), state(src.state), volume(src.volume),
          isAllDisconnected(src.isAllDisconnected)
    {}
    HearingAidMessage& operator=(const HearingAidMessage&) = delete;
    ~HearingAidMessage() override = default;

    explicit HearingAidMessage(int what, std::string addr, int stateValue)
        : utility::Message(what, 0, nullptr), msgAddr(addr), state(stateValue){};

    explicit HearingAidMessage(int what, std::string addr) : utility::Message(what), msgAddr(addr){};

    explicit HearingAidMessage(int what) : utility::Message(what){};

    std::string msgAddr = "";
    int state = 0;
    int volume = 0;
    bool isAllDisconnected = false;
};
}  // namespace bluetooth
}
#endif // HEARING_AID_MESSAGE_H
