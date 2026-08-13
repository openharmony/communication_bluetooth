/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef AVRCP_CT_METADATA_H
#define AVRCP_CT_METADATA_H

#include "av_session.h"

namespace OHOS {
namespace bluetooth {
class AvrcpCtMetaData {
public:
    AvrcpCtMetaData() = default;
    ~AvrcpCtMetaData() = default;
    void CreateMetaData()
    {
        if (avMetaData_ == nullptr) {
            avMetaData_ = std::make_shared<AVSession::AVMetaData>();
        }
    }

    std::shared_ptr<AVSession::AVMetaData> GetMetaData()
    {
        return avMetaData_;
    }

private:
    std::shared_ptr<AVSession::AVMetaData> avMetaData_{nullptr};
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // AVRCP_CT_METADATA_H