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

#include "obex_array_body_object.h"

namespace OHOS {
namespace bluetooth {

size_t ObexArrayBodyObject::Read(uint8_t *buf, size_t bufLen)
{
    if (readPos_ >= data_.size() || buf == nullptr) {
        return 0;
    }
    size_t available = data_.size() - readPos_;
    size_t readLen = (bufLen < available) ? bufLen : available;
    std::copy(data_.begin() + readPos_, data_.begin() + readPos_ + readLen, buf);
    readPos_ += readLen;
    return readLen;
}

size_t ObexArrayBodyObject::Write(const uint8_t *buf, size_t bufLen)
{
    if (buf == nullptr) {
        return 0;
    }
    data_.insert(data_.end(), buf, buf + bufLen);
    return bufLen;
}

bool ObexArrayBodyObject::IsReadFinished()
{
    return readPos_ >= data_.size();
}

const std::vector<uint8_t> &ObexArrayBodyObject::GetData() const
{
    return data_;
}

void ObexArrayBodyObject::ResetReadPosition()
{
    readPos_ = 0;
}

}  // namespace bluetooth
}  // namespace OHOS
