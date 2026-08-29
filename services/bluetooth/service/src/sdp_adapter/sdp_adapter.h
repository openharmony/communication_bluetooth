/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef SDP_ADAPTER_H
#define SDP_ADAPTER_H

#include <cstdint>
#include "bt_recursive_mutex.h"
#include "bt_sdp.h"

namespace OHOS {
namespace bluetooth {
struct PbapPseSdpCreateParam {
    const char *name;
    int32_t channel;
    int32_t l2capPsm;
    int32_t version;
    int32_t supportedRepositories;
    int32_t features;
};
struct MapMasSdpCreateParam {
    const char *name;
    int32_t masId;
    int32_t channel;
    int32_t l2capPsm;
    int32_t version;
    int32_t msgTypes;
    int32_t features;
};

struct OppSdpCreateParam {
    char *name;
    int32_t channel;
    int32_t l2capPsm;
    int32_t version;
    int32_t supportedFormatsListLen;
    uint8_t supportedFormatsList[];
};

struct MapMnsSdpFoundRecord {
    int32_t status;
    int32_t channel;
    int32_t l2capPsm;
    int32_t version;
    int32_t features;
    std::string deviceAddr;
    std::string uuid;
    std::string serviceName;
    bool moreResults;
};

using MapMnsSdpFoundCallback = std::function<void(const MapMnsSdpFoundRecord &record)>;

struct OppSdpFoundRecord {
    int32_t status;
    int32_t channel;
    int32_t l2capPsm;
    int32_t version;
    std::string deviceAddr;
    std::string uuid;
    std::string serviceName;
    bool moreResults;
};
using OppSdpFoundCallback = std::function<void(const OppSdpFoundRecord &record)>;
class SdpAdapter {
public:
    static SdpAdapter &GetInstance();
    int32_t CreatePbapPseSdpRecord(const PbapPseSdpCreateParam &param);
    int32_t CreateMapMasSdpRecord(const MapMasSdpCreateParam &param);
    int32_t CreateOppSdpRecord(const OppSdpCreateParam &param);
    bool RemoveSdpRecord(int32_t handle);
    bool StartRemoteSdpSearch(const std::string &address, const std::string &uuid);
    void SetMapMnsSdpFoundCallback(const MapMnsSdpFoundCallback &callback);
    MapMnsSdpFoundCallback GetMapMnsSdpFoundCallback();
    void SetOppSdpFoundCallback(const OppSdpFoundCallback &callback);
    OppSdpFoundCallback GetOppSdpFoundCallback();

private:
    SdpAdapter();
    BtsdpInterface *bluetoothSdpInterface_ = nullptr;
    MapMnsSdpFoundCallback mapMnsSdpFoundCallback_ = nullptr;
    OppSdpFoundCallback oppSdpFoundCallback_ = nullptr;
};
}  // namespace bluetooth
}  // namespace OHOS

#endif
