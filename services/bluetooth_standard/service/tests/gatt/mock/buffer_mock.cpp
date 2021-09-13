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

#include "buffer.h"
#include "platform/include/allocator.h"
#include "platform/include/platform_def.h"
#include "log.h"
#include <memory.h>

typedef struct Buffer {
    uint32_t size;
    uint32_t refcount;
    Buffer *rootbuf;
    uint8_t *data;
} BufferInternal;

Buffer *BufferMalloc(uint32_t size)
{
    if (size == 0) {
        return NULL;
    }

    Buffer *buf = (Buffer *)MEM_CALLOC.alloc(sizeof(Buffer) + size);
    buf->size = size;
    buf->refcount = 1;
    buf->rootbuf = buf;
    buf->data = (uint8_t *)buf + sizeof(Buffer);
    return buf;
}
void BufferFree(Buffer *buf)
{
    if (!buf) {
        return;
    }

    if (buf->rootbuf != buf) {
        if (--buf->rootbuf->refcount == 0) {
            MEM_CALLOC.free(buf->rootbuf);
        }
        MEM_CALLOC.free(buf);
    } else if (--buf->refcount == 0) {
        MEM_CALLOC.free(buf->rootbuf);
    }
}
void *BufferPtr(const Buffer *buf)
{
    if (!buf) {
        return NULL;
    }
    return buf->data;
}
uint32_t BufferGetSize(const Buffer *buf)
{
    if (!buf) {
        return 0;
    }
    return buf->size;
}
Buffer *BufferResize(Buffer *buf, uint32_t offset, uint32_t size)
{
    buf->data += offset;
    buf->size = size;
    return buf;
}

Buffer *BufferSliceMalloc(const Buffer *buf, uint32_t offset, uint32_t size)
{
    if (!buf) {
        return NULL;
    }

    if (size + offset > buf->size || size == 0) {
        LOG_ERROR("Size too small or too big");
        return NULL;
    }

    if (offset > buf->size) {
        LOG_ERROR("BufferSliceMalloc##Buffer offset out of bound");
        return NULL;
    }

    Buffer *slice = BufferRefMalloc(buf);
    slice = BufferResize(slice, offset, size);

    return slice;
}

Buffer *BufferRefMalloc(const Buffer *buf)
{
    if (!buf) {
        return NULL;
    }

    Buffer *ref = (Buffer *)MEM_CALLOC.alloc(sizeof(Buffer));
    ref->data = buf->data;
    ref->size = buf->size;
    ref->rootbuf = buf->rootbuf;
    ref->rootbuf->refcount++;

    return ref;
}