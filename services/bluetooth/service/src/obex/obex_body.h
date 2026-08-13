/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OBEX_BODY_H
#define OBEX_BODY_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

namespace OHOS {
namespace bluetooth {
class ObexBodyObject {
public:
    virtual ~ObexBodyObject() = default;
    virtual void OpenFile(const int fileFd) = 0;
    virtual int Close() = 0;
    virtual size_t Read(uint8_t *buf, size_t bufLen) = 0;
    virtual size_t Write(const uint8_t *buf, size_t bufLen) = 0;
    virtual bool IsReadFinished() = 0;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // OBEX_BODY_H