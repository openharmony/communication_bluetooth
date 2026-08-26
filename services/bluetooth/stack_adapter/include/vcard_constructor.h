/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

/*
 * Stub of the removed stack layer vCard constructor (vcard_constructor.h).
 */

#ifndef VCARD_CONSTRUCTOR_H
#define VCARD_CONSTRUCTOR_H

#include <string>

namespace OHOS {
namespace bluetooth {
class VCardConstructor {
public:
    explicit VCardConstructor(int charset) : charset_(charset) {}
    virtual ~VCardConstructor() = default;

    std::string GetResult() const
    {
        return "";
    }

private:
    int charset_;
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // VCARD_CONSTRUCTOR_H
