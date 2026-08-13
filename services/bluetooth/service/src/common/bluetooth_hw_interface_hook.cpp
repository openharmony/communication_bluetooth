/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#undef LOG_TAG
#define LOG_TAG "bt_hw_interface_hook"
#include "log.h"
#include "bluetooth_hw_interface_hook.h"
#include "hfp_ag_service.h"
#include "a2dp_service.h"
#include <optional>

namespace OHOS {
namespace bluetooth {

static std::optional<AppCategory> Int2AppCategory(int type) {
    static const std::unordered_map<int, AppCategory> intToCategoryMap = {
        {7, APP_CATEGORY_MUSIC},
        {8, APP_CATEGORY_VIDEO},
        {9, APP_CATEGORY_GAME}
    };

    auto it = intToCategoryMap.find(type);
    if (it != intToCategoryMap.end()) {
        return it->second;
    }
    return std::nullopt;
}

static void sendAtCmd(int type) {
    auto hfpAgService = HfpAgService::GetService();
    if (!hfpAgService) {
        HILOGE("hfpAgService is null");
        return;
    }

    auto appCategoryOpt = Int2AppCategory(type);
    if (appCategoryOpt.has_value()) {
        AppCategory appType = appCategoryOpt.value();
        hfpAgService->SendAtCmdByTopApptype(appType);
    } else {
        HILOGE("No AppCategory found for type: %d", type);
    }
}

static bool IsA2dpPlaying() {
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService == nullptr) {
        return false;
    }
    const RawAddress device = a2dpService->GetActiveSinkDevice();
    if (a2dpService->IsA2dpPlaying(device)) {
        return true;
    }
    return false;
}

static void SetA2dpPlayingCb(std::function<void(int)> callbackFunction)
{
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService == nullptr) {
        return;
    }
    a2dpService->SetA2dpPlayingStateCallback(callbackFunction);
}

static bool HasA2dpConnectedDevicesForCurrentMobile(){
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    CHECK_AND_RETURN_LOG_RET(a2dpService != nullptr, false, "a2dpService is nullptr.");
    const RawAddress activeA2dpDevice = a2dpService->GetActiveSinkDevice();

    int connectionA2dpState = a2dpService->GetDeviceState(activeA2dpDevice);
    if (connectionA2dpState == static_cast<int>(BTConnectState::CONNECTED)) {
        return true;
    }
    return false;
}

static bool HasHfpConnectedDevicesForCurrentMobile(){
    HfpAgService *hfpAgService = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG_RET(hfpAgService != nullptr, false, "hfpAgService is nullptr.");
    std::string activeHfpDeviceAddr = hfpAgService->GetActiveDevice();
    const RawAddress activeHfpDevice = RawAddress(activeHfpDeviceAddr);

    int connectionHfpState = hfpAgService->GetDeviceState(activeHfpDevice);
    if (connectionHfpState == static_cast<int>(BTConnectState::CONNECTED)) {
        return true;
    }
    return false;
}

void GetHfpAgServiceFuncs(HfpAgServiceFuncs& funcs)
{
    funcs.sendAtCmdFunc = sendAtCmd;
    funcs.hasHfpConnectedDevicesForCurrentMobilefunc = HasHfpConnectedDevicesForCurrentMobile;
}

void GetA2dpServiceFuncs(A2dpServiceFuncs& funcs)
{
    funcs.isA2dpPlayingCb = IsA2dpPlaying;
    funcs.setA2dpPlayingCb = SetA2dpPlayingCb;
    funcs.hasA2dpConnectedDevicesForCurrentMobilefunc = HasA2dpConnectedDevicesForCurrentMobile;
}

}  // namespace bluetooth
}  // namespace OHOS