/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include <uv.h>
#include "bluetooth_utils.h"
#include "napi_bluetooth_event.h"
#include "napi_bluetooth_opp_observer.h"

namespace OHOS {
namespace Bluetooth {
std::shared_mutex NapiBluetoothOppObserver::g_oppCallbackInfosMutex;
void NapiBluetoothOppObserver::OnReceiveIncomingFileChanged(const BluetoothOppTransferInformation &transferInformation)
{
    HILOGI("enter, OnReceiveIncomingFileChanged");
    std::unique_lock<std::shared_mutex> guard(g_oppCallbackInfosMutex);

    std::map<std::string, std::shared_ptr<BluetoothCallbackInfo>>::iterator it =
        callbackInfos_.find(STR_BT_OPP_OBSERVER_RECEIVE_INCOMING_FILE);
    if (it == callbackInfos_.end() || it->second == nullptr) {
        HILOGW("This callback is not registered by ability.");
        return;
    }
    std::shared_ptr<TransforInformationCallbackInfo> callbackInfo =
        std::static_pointer_cast<TransforInformationCallbackInfo>(it->second);

    callbackInfo->deviceId_ = transferInformation.GetDeviceAddress();
    NapiEvent::CheckAndNotify(callbackInfo, transferInformation);
}

void NapiBluetoothOppObserver::OnTransferStateChanged(const BluetoothOppTransferInformation &transferInformation)
{
    HILOGI("enter, OnTransferStateChanged");
    std::unique_lock<std::shared_mutex> guard(g_oppCallbackInfosMutex);

    std::map<std::string, std::shared_ptr<BluetoothCallbackInfo>>::iterator it =
        callbackInfos_.find(STR_BT_OPP_OBSERVER_TRANSFER_STATE_CHANGE);
    if (it == callbackInfos_.end() || it->second == nullptr) {
        HILOGW("This callback is not registered by ability.");
        return;
    }
    std::shared_ptr<TransforInformationCallbackInfo> callbackInfo =
        std::static_pointer_cast<TransforInformationCallbackInfo>(it->second);

    callbackInfo->deviceId_ = transferInformation.GetDeviceAddress();
    NapiEvent::CheckAndNotify(callbackInfo, transferInformation);
}
} // namespace Bluetooth
} // namespace OHOS