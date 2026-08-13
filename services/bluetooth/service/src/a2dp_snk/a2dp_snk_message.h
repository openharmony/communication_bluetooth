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

#ifndef A2DP_SNK_MESSAGE_H
#define A2DP_SNK_MESSAGE_H

#include <string>
#include "btcommon/message.h"

namespace OHOS {
namespace bluetooth {
class A2dpSnkMessage : public utility::Message {
public:
    explicit A2dpSnkMessage(int what, int arg1 = 0, void *arg2 = nullptr) : utility::Message(what, arg1, arg2)
    {}
    ~A2dpSnkMessage() = default;
    int arg3_{0};
    uint32_t sampleRate_{0};
    uint8_t channelCount_{0};
    std::string addr_{""};
    // 事件重试计数（IsRemoving 时延迟重投，超过上限丢弃）
    int retryCount_{0};
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // A2DP_SNK_MESSAGE_H