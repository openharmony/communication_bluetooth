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

#ifndef OBEX_ARRAY_BODY_OBJECT_H
#define OBEX_ARRAY_BODY_OBJECT_H

#include <cstdint>
#include <vector>
#include "obex_body.h"

namespace OHOS {
namespace bluetooth {

class ObexArrayBodyObject : public ObexBodyObject {
public:
    ObexArrayBodyObject() = default;
    ~ObexArrayBodyObject() override = default;

    void OpenFile(const int fileFd) override {}
    int Close() override { return 0; }
    size_t Read(uint8_t *buf, size_t bufLen) override;
    size_t Write(const uint8_t *buf, size_t bufLen) override;
    bool IsReadFinished() override;

    const std::vector<uint8_t> &GetData() const;
    void ResetReadPosition();

private:
    std::vector<uint8_t> data_ {};
    size_t readPos_ = 0;
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // OBEX_ARRAY_BODY_OBJECT_H
