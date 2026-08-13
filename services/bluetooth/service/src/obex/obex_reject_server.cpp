/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_obex_reject_server"
#endif

#include "obex_reject_server.h"
#include "obex_def.h"
#include "log.h"

namespace OHOS {
namespace bluetooth {
int ObexRejectServer::OnGet(ObexHeader &request, ObexHeader &reply, std::vector<uint8_t> &output, bool &sendBodyHeader)
{
    HILOGW("return unavailable");
    return OBEX_HTTP_UNAVAILABLE;
}

int ObexRejectServer::OnPut(ObexHeader &request, ObexHeader &reply, std::vector<uint8_t> &input, bool &requestFinished)
{
    HILOGW("return unavailable");
    return OBEX_HTTP_UNAVAILABLE;
}

int ObexRejectServer::OnConnect(ObexHeader &request, ObexHeader &reply)
{
    HILOGW("return unavailable");
    return OBEX_HTTP_UNAVAILABLE;
}

int ObexRejectServer::OnDisconnect(ObexHeader &request, ObexHeader &reply)
{
    HILOGW("return unavailable");
    return OBEX_HTTP_UNAVAILABLE;
}

int ObexRejectServer::OnSetPath(ObexHeader &request, ObexHeader &reply, bool backup, bool create)
{
    HILOGW("return unavailable");
    return OBEX_HTTP_UNAVAILABLE;
}

int ObexRejectServer::OnDelete(ObexHeader &request, ObexHeader &reply)
{
    HILOGW("return unavailable");
    return OBEX_HTTP_UNAVAILABLE;
}

int ObexRejectServer::OnAbort(ObexHeader &request, ObexHeader &reply)
{
    HILOGW("return unavailable");
    return OBEX_HTTP_UNAVAILABLE;
}

void ObexRejectServer::OnClose(bool isThreadStart)
{
    HILOGW("unavailable");
}

void ObexRejectServer::SetConnectionId(int connectionId)
{
    HILOGW("unavailable");
}

int ObexRejectServer::GetConnectionId()
{
    HILOGW("return unavailable");
    return OBEX_HTTP_UNAVAILABLE;
}
}  // namespace bluetooth
}  // namespace OHOS