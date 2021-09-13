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

#include <stdint.h>
#include "btstack.h"
#include "buffer.h"

typedef struct {
    uint8_t status;
    uint8_t role;
    BtAddr addr;
} AttLeConnectCallback;

typedef struct {
    uint8_t status;
    uint8_t reason;
} AttLeDisconnectCallback;

typedef struct {
    BtAddr addr;
    uint8_t status;
    uint16_t mtu;
} AttBredrConnectCallback;

typedef struct {
    uint8_t reason;
} AttBredrDisconnectCallback;
typedef void (*attCallback)(uint16_t connectHandle, uint16_t event, void *eventData, Buffer *buffer, void *context);

typedef struct {
    void (*attLEConnectCompleted)(uint16_t connectHandle, AttLeConnectCallback *data, void *context);
    void (*attLEDisconnectCompleted)(uint16_t connectHandle, AttLeDisconnectCallback *data, void *context);
    void (*attBREDRConnectCompleted)(uint16_t connectHandle, AttBredrConnectCallback *data, void *context);
    void (*attBREDRDisconnectCompleted)(uint16_t connectHandle, AttBredrDisconnectCallback *data, void *context);
    void (*attBREDRConnectInd)(uint16_t connectHandle, void *context);
} AttConnectCallback;