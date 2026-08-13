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

#ifndef AVRCP_CT_MESSAGE_H
#define AVRCP_CT_MESSAGE_H

#include <string>
#include <memory>
#include "btcommon/message.h"
#include "avrcp_ct_define.h"
#include "avrcp_ct_media_item.h"

namespace OHOS {
namespace bluetooth {
class AvrcpCtMessage : public utility::Message {
public:
    explicit AvrcpCtMessage(int what, int arg1 = 0, void *arg2 = nullptr) : utility::Message(what, arg1, arg2)
    {}
    ~AvrcpCtMessage() = default;
    int eventThreeParameter_{0};
    // remoteCtlConnected flag
    bool rcConnect{false};
    std::string uuid_{""};
    // dev adress
    std::string dev_{""};
    AvrcpCtConnectFlag connectFlag_{};
    std::shared_ptr<AvrcpCtMediaItem> track_{nullptr};
    // PlayerApplicationSettings 变更属性 ID 和值（用于 CURRENT_APPLICATION_SETTINGS 消息）
    uint8_t appSettingAttr_{0};
    uint8_t appSettingVal_{0};
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // AVRCP_CT_MESSAGE_H
