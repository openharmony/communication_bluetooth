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

#ifndef OBEX_RESPONSE_HANDLER_H
#define OBEX_RESPONSE_HANDLER_H

#include "obex_header.h"

namespace OHOS {
namespace bluetooth {
/**
 * @brief obex response handler
 *
 */
class ObexResponseHandler {
public:
    virtual void OnConnectResp(ObexHeader &reply, uint8_t responseCode) = 0;
    virtual void OnPutDateAvailableResp(ObexHeader &reply, uint8_t responseCode) = 0;
    virtual void OnDisConnectResp(ObexHeader &reply, uint8_t responseCode) = 0;
    virtual void OnAbortResp() = 0;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // OBEX_RESPONSE_HANDLER_H