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

#include "obex_file_body_object.h"
#include <unistd.h>
#include <fcntl.h>

namespace OHOS {
namespace bluetooth {

ObexFileBodyObject::ObexFileBodyObject(const std::string &filePath) : filePath_(filePath) {}

ObexFileBodyObject::~ObexFileBodyObject()
{
    Close();
}

void ObexFileBodyObject::OpenFile(const int fileFd)
{
    if (fileFd >= 0) {
        fileFd_ = fileFd;
        return;
    }
    if (!filePath_.empty()) {
        fileFd_ = open(filePath_.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
    }
}

int ObexFileBodyObject::Close()
{
    if (fileFd_ >= 0) {
        int ret = close(fileFd_);
        fileFd_ = -1;
        return ret;
    }
    return 0;
}

size_t ObexFileBodyObject::Read(uint8_t *buf, size_t bufLen)
{
    if (fileFd_ < 0 || buf == nullptr) {
        return 0;
    }
    ssize_t bytesRead = read(fileFd_, buf, bufLen);
    if (bytesRead <= 0) {
        readFinished_ = true;
        return 0;
    }
    return static_cast<size_t>(bytesRead);
}

size_t ObexFileBodyObject::Write(const uint8_t *buf, size_t bufLen)
{
    if (fileFd_ < 0 && !filePath_.empty()) {
        fileFd_ = open(filePath_.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
    }
    if (fileFd_ < 0 || buf == nullptr) {
        return 0;
    }
    ssize_t bytesWritten = write(fileFd_, buf, bufLen);
    return (bytesWritten >= 0) ? static_cast<size_t>(bytesWritten) : 0;
}

bool ObexFileBodyObject::IsReadFinished()
{
    return readFinished_;
}

}  // namespace bluetooth
}  // namespace OHOS
