/*
 * Copyright (C) 2024-2024 Huawei Device Co., Ltd.
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
#include "bluetooth_host_server.h"
#include "string_ex.h"
#include "../../common/log.h"
#include "raw_address.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_a2dp_source_server.h"
#include "bluetooth_hfp_ag_server.h"

namespace OHOS {
namespace Bluetooth {

void BluetoothHostServer::UpdateVirtualDevice(int32_t action, const std::string &address)
{
    HILOGI("Intro");
    std::vector<std::string> uuids;
    int result = GetDeviceUuids(address, uuids);
    if (result != BT_NO_ERROR) {
        HILOGE("address cannot found");
        return;
    }

    for (const auto &uuid : uuids) {
        if (IsSameTextStr(uuid, BLUETOOTH_UUID_A2DP_SINK)) {
            auto a2dpRemote = GetProfile(PROFILE_A2DP_SRC);
            if (a2dpRemote == nullptr) {
                HILOGE("get a2dp remote fail");
                break;
            }
            sptr<IBluetoothA2dpSrc> a2dpSource = iface_cast<IBluetoothA2dpSrc>(a2dpRemote);
            if (a2dpSource == nullptr) {
                break;
            }
            int strategy = 0;
            a2dpSource->GetConnectStrategy(RawAddress(address), strategy);
            if (strategy != static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)) {
                a2dpSource->UpdateVirtualDevice(action, address);
            }
            break;
        }
    }

    for (const auto &uuid : uuids) {
        if (IsSameTextStr(uuid, BLUETOOTH_UUID_HFP_HF)) {
            auto hfpRemote = GetProfile(PROFILE_HFP_AG);
            if (hfpRemote == nullptr) {
                HILOGE("get hfp remote fail");
                break;
            }
            sptr<IBluetoothHfpAg> hfpAg = iface_cast<IBluetoothHfpAg>(hfpRemote);
            if (hfpAg == nullptr) {
                break;
            }
            int strategy = 0;
            hfpAg->GetConnectStrategy(RawAddress(address), strategy);
            if (strategy != static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)) {
                hfpAg->UpdateVirtualDevice(action, address);
            }
            break;
        }
    }
}
}  // namespace Bluetooth
}  // namespace OHOS