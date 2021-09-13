/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include <memory.h>
#include "src/att/att_common.h"
#include "buffer.h"
#include "packet.h"
#include "list.h"
#include "log.h"
#include "gap_if.h"
#include <mutex>
#include <condition_variable>

void AttSendRespCallback(uint16_t lcid, int result);

void AttSendRespCallback_server(uint16_t lcid, int result);

void AssembleDataPackage(uint8_t *dest, uint8_t *offset, uint8_t *src, uint8_t size);

struct AsyncCallInfromation_2 {
    bool isCompleted_;
    int state_;

    std::mutex mutex_;
    std::condition_variable condition_;
};