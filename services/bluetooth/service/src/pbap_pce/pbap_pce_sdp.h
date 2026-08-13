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

#ifndef PBAP_PCE_SDP_H
#define PBAP_PCE_SDP_H

#include <cstdint>
#include <functional>
#include <memory>
#include "btcommon/message.h"
#include "../sdp_adapter/sdp_adapter.h"

namespace OHOS {
namespace bluetooth {
static const uint16_t PBAP_PCE_VERSION_NUMBER = 0x0102;
static const uint32_t PBAP_PCE_SUPPORTED_FEATURES = 0x03;

class PbapPceSdp {
public:
    using SdpResponseCallback = std::function<void(const std::string &addr, const utility::Message &msg)>;

    explicit PbapPceSdp(SdpResponseCallback callback);
    virtual ~PbapPceSdp();

    int Register();
    int Deregister() const;
    int SdpSearch(const std::string &address);

private:
    void OnSdpFound(const PbapPceSdpFoundRecord &record);

    uint32_t sdpHandle_ = 0;
    SdpResponseCallback callback_;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // PBAP_PCE_SDP_H
