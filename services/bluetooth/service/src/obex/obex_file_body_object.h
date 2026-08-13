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

#ifndef OBEX_FILE_BODY_OBJECT_H
#define OBEX_FILE_BODY_OBJECT_H

#include <cstdint>
#include <string>
#include <vector>
#include "obex_body.h"

namespace OHOS {
namespace bluetooth {

class ObexFileBodyObject : public ObexBodyObject {
public:
    explicit ObexFileBodyObject(const std::string &filePath);
    ~ObexFileBodyObject() override;

    void OpenFile(const int fileFd) override;
    int Close() override;
    size_t Read(uint8_t *buf, size_t bufLen) override;
    size_t Write(const uint8_t *buf, size_t bufLen) override;
    bool IsReadFinished() override;

private:
    std::string filePath_ {};
    int fileFd_ = -1;
    bool readFinished_ = false;
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // OBEX_FILE_BODY_OBJECT_H
