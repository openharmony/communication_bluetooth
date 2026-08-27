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
 * The service layer references it through the Telephony namespace.
 */

#ifndef VCARD_CONSTRUCTOR_H
#define VCARD_CONSTRUCTOR_H

#include <iomanip>
#include <string>
#include <vector>

namespace Telephony {
/* Phone number type used by AddTelLine. */
enum class PhoneVcType : int32_t {
    NUM_HOME = 1,
    NUM_MOBILE = 2,
    NUM_WORK = 3,
    NUM_FAX_HOME = 4,
    NUM_FAX_WORK = 5,
    NUM_OTHER = 6,
};

inline constexpr const char *VCARD_TYPE_FN = "FN";
inline constexpr const char *VCARD_TYPE_N = "N";
inline constexpr const char *VCARD_PROPERTY_X_TIMESTAMP = "X-IRMC-CALL-DATETIME";

class VCardConstructor {
public:
    explicit VCardConstructor(int charset) : charset_(charset) {}
    virtual ~VCardConstructor() = default;

    void ContactBegin() {}
    void ContactEnd() {}

    void AddLine(const std::string &lineType, const std::string &lineValue)
    {
        (void)lineType;
        (void)lineValue;
    }

    void AddLine(const std::string &lineType, const std::vector<std::string> &params,
        const std::string &lineValue)
    {
        (void)lineType;
        (void)params;
        (void)lineValue;
    }

    void AddTelLine(const std::string &labelId, const std::string &labelName,
        const std::string &phoneNumber)
    {
        (void)labelId;
        (void)labelName;
        (void)phoneNumber;
    }

    std::string ToString() const
    {
        return "";
    }

private:
    int charset_;
};
}  // namespace Telephony

/* Keep the bare constant usable (pbap_pse_datashare_utils.cpp spells it
 * without the Telephony:: qualifier). */
using Telephony::VCARD_PROPERTY_X_TIMESTAMP;

#endif  // VCARD_CONSTRUCTOR_H
