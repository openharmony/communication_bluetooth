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
 * Stub of the removed stack layer vCard manager (vcard_manager.h).
 * The service layer references it through the Telephony namespace.
 */

#ifndef VCARD_MANAGER_H
#define VCARD_MANAGER_H

#include <memory>
#include <string>

#include "datashare_helper.h"
#include "datashare_predicates.h"

namespace Telephony {
class VCardManager {
public:
    static VCardManager &GetInstance()
    {
        static VCardManager instance;
        return instance;
    }

    virtual ~VCardManager() = default;

    void SetDataHelper(std::shared_ptr<OHOS::DataShare::DataShareHelper> dataHelper)
    {
        (void)dataHelper;
    }

    void ExportToStr(std::string &vcard, OHOS::DataShare::DataSharePredicates &predicates, int32_t vcardType)
    {
        (void)vcard;
        (void)predicates;
        (void)vcardType;
    }

    virtual int GetVCardCount() const
    {
        return 0;
    }

    virtual std::string GetVCard(int index)
    {
        (void)index;
        return "";
    }
};
}  // namespace Telephony

#endif  // VCARD_MANAGER_H
