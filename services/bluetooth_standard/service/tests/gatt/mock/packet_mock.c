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

#include "packet.h"
#include "buffer.h"
#include "platform/include/allocator.h"
#include "platform/include/platform_def.h"
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
typedef struct Payload {
    struct Payload *next;
    struct Payload *prev;
    Buffer *buf;
} Payload;

typedef struct Packet {
    Payload *head;
    Payload *tail;
    Payload *payload;
} PacketInternal;

void PacketExtractHead(Packet *pkt, uint8_t *data, uint32_t size)
{
    ASSERT(pkt);
    ASSERT(data);
    struct Payload *first = pkt->payload;
    uint32_t buffSize = BufferGetSize(first->buf);
    if (buffSize < size) {
        return;
    }
    memcpy(data, (uint8_t *)BufferPtr(first->buf) + 0, size);
    first->buf = BufferResize(first->buf, size, buffSize - size);
}

static inline struct Payload *PayloadNew(size_t size)
{
    struct Payload *payload = (struct Payload *)MEM_CALLOC.alloc(sizeof(struct Payload));
    payload->buf = BufferMalloc(size);

    return payload;
}

static inline struct Payload *PayloadNewRef(const Buffer *buf)
{
    struct Payload *payload = (struct Payload *)MEM_CALLOC.alloc(sizeof(struct Payload));
    payload->buf = BufferRefMalloc(buf);

    return payload;
}

Packet *PacketMalloc(uint16_t headSize, uint16_t tailSize, uint32_t payloadSize)
{
    Packet *packet = (Packet *)MEM_CALLOC.alloc(sizeof(Packet));

    packet->head = PayloadNew(headSize);
    packet->tail = PayloadNew(tailSize);
    packet->payload = PayloadNew(payloadSize);
    packet->head->next = packet->payload;
    packet->payload->next = packet->tail;

    return packet;
}

uint32_t PacketSize(const Packet *pkt)
{
    ASSERT(pkt);
    size_t size = 0;
    struct Payload *node = pkt->head;
    while (node != NULL) {
        size += BufferGetSize(node->buf);
        node = node->next;
    }
    return size;
}


uint32_t PacketRead(const Packet *pkt, uint8_t *dst, uint32_t offset, uint32_t size)
{
    ASSERT(pkt);
    ASSERT(dst);
    if (size <= 0) {
        return 0;
    }
    size_t pktSize = PacketSize(pkt);
    if (offset > pktSize) {
        return 0;
    }

    size_t currOffset = 0;
    struct Payload *payload = pkt->head;
    while (BufferGetSize(payload->buf) + currOffset <= offset) {
        if (payload == NULL) {
            return 0;
        } else {
            currOffset += BufferGetSize(payload->buf);
            payload = payload->next;
        }
    }

    size_t buffSize = BufferGetSize(payload->buf);
    size_t buffOffset = offset - currOffset;
    size_t buffRemain = buffSize - buffOffset;
    size_t readSize = 0;

    if (buffRemain >= size) {
        memcpy(dst, (uint8_t *)BufferPtr(payload->buf) + buffOffset, size);
        readSize = size;
    } else {
        memcpy(dst, (uint8_t *)BufferPtr(payload->buf) + buffOffset, buffRemain);
        readSize += buffRemain;
        size -= buffRemain;
        payload = payload->next;
        while (payload != NULL && size > 0) {
            buffSize = BufferGetSize(payload->buf);
            if (size <= buffSize) {
                memcpy(dst + readSize, (uint8_t *)BufferPtr(payload->buf), size);
                readSize += size;
                size -= size;
            } else {
                memcpy(dst + readSize, (uint8_t *)BufferPtr(payload->buf), buffSize);
                readSize += buffSize;
                size -= buffSize;
            }
            payload = payload->next;
        }
    }
    return readSize;
}

void PacketFree(Packet *pkt)
{
    return;
}

void PacketPayloadAddLast(const Packet *pkt, const Buffer *buf)
{
    struct Payload *last = pkt->payload;
    while (last != NULL && last->next != pkt->tail) {
        last = last->next;
    }

    if (last != NULL && last->buf == NULL) {
        last->buf = BufferRefMalloc(buf);
    } else {
        struct Payload *newLast = PayloadNewRef(buf);
        newLast->next = pkt->tail;
        last->next = newLast;
    }
}

uint32_t PacketPayloadSize(const Packet *pkt)
{
    ASSERT(pkt);
    size_t size = 0;
    struct Payload *node = pkt->payload;
    while (node != pkt->tail) {
        size += BufferGetSize(node->buf);
        node = node->next;
    }
    return size;
}

static inline void PayloadFree(Payload *payload)
{
    if (payload == NULL) {
        return;
    }
    BufferFree(payload->buf);
    free(payload);
}

Buffer *PacketContinuousPayload(Packet *pkt)
{
    ASSERT(pkt);
    if (pkt->payload == NULL) {
        return NULL;
    }
    if (pkt->payload->next == pkt->tail) {
        return pkt->payload->buf;
    }

    size_t payloadSize = PacketPayloadSize(pkt);
    if (payloadSize == 0) {
        return NULL;
    }

    struct Payload *newPayload = PayloadNew(payloadSize);
    struct Payload *payloadIter = pkt->payload;
    uint8_t *bufPtr = BufferPtr(newPayload->buf);

    while (payloadIter != pkt->tail) {
        size_t bufSize = BufferGetSize(payloadIter->buf);
        if (bufSize > 0) {
            memcpy(bufPtr, BufferPtr(payloadIter->buf), bufSize);
            bufPtr += bufSize;
        }
        struct Payload *payloadTemp = payloadIter;
        payloadIter = payloadIter->next;
        PayloadFree(payloadTemp);
    }

    newPayload->next = pkt->tail;
    pkt->head->next = newPayload;
    pkt->payload = newPayload;

    return newPayload->buf;
}