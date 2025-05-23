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
#ifndef LOG_TAG
#define LOG_TAG "bt_napi_opp_observer"
#endif

#include <uv.h>
#include "bluetooth_utils.h"
#include "napi_bluetooth_event.h"
#include "napi_bluetooth_opp_observer.h"
#include "napi_event_subscribe_module.h"

namespace OHOS {
namespace Bluetooth {
NapiBluetoothOppObserver::NapiBluetoothOppObserver()
    : eventSubscribe_({STR_BT_OPP_OBSERVER_RECEIVE_INCOMING_FILE,
        STR_BT_OPP_OBSERVER_TRANSFER_STATE_CHANGE},
        BT_MODULE_NAME)
{}

void NapiBluetoothOppObserver::OnReceiveIncomingFileChanged(const BluetoothOppTransferInformation &transferInformation)
{
    HILOGI("enter, OnReceiveIncomingFileChanged");
    auto nativeObject = std::make_shared<NapiNativeOppTransferInformation>(transferInformation);
    eventSubscribe_.PublishEvent(STR_BT_OPP_OBSERVER_RECEIVE_INCOMING_FILE, nativeObject);
}

void NapiBluetoothOppObserver::OnTransferStateChanged(const BluetoothOppTransferInformation &transferInformation)
{
    HILOGI("enter, OnTransferStateChanged");
    auto nativeObject = std::make_shared<NapiNativeOppTransferInformation>(transferInformation);
    eventSubscribe_.PublishEvent(STR_BT_OPP_OBSERVER_TRANSFER_STATE_CHANGE, nativeObject);
}
} // namespace Bluetooth
} // namespace OHOS