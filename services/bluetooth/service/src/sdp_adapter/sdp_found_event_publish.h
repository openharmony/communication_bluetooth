/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef SDP_FOUND_EVENT_PUBLISH_H
#define SDP_FOUND_EVENT_PUBLISH_H
#include <cstdint>
#include "btif_sdp.h"

namespace OHOS {
namespace bluetooth {
class SdpFoundEventPublishHelper {
public:
	static bool PublishSdpFoundEvent(int32_t status, const std::string &address,
    	const std::string &uuid, bool more_results, BluetoothSdpRecord *record);
    static bool PublishMapMasSdpFoundEvent(int32_t status, const std::string &address,
    	const std::string &uuid, bool more_results, BluetoothSdpRecord* record);
    static bool PublishMapMnsSdpFoundEvent(int32_t status, const std::string &address,
    	const std::string &uuid, bool more_results, BluetoothSdpRecord* record);
    static bool PublishPbapPseSdpFoundEvent(int32_t status, const std::string &address,
    	const std::string &uuid, bool more_results, BluetoothSdpRecord* record);
    static bool PublishObexObjectPushSdpFoundEvent(int32_t status, const std::string &address,
    	const std::string &uuid, bool more_results, BluetoothSdpRecord* record);
    static bool PublishSapSdpFoundEvent(int32_t status, const std::string &address,
    	const std::string &uuid, bool more_results, BluetoothSdpRecord* record);
    static bool PublishDipSdpFoundEvent(int32_t status, const std::string &address,
    	const std::string &uuid, bool more_results, BluetoothSdpRecord* record);
    static bool PublishCommonSdpFoundEvent(int32_t status, const std::string &address,
    	const std::string &uuid, bool more_results, BluetoothSdpRecord* record);
};
}  // namespace bluetooth
}  // namespace OHOS
#endif