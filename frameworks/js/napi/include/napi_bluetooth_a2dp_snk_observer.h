/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef NAPI_BLUETOOTH_A2DP_SNK_OBSERVER_H_
#define NAPI_BLUETOOTH_A2DP_SNK_OBSERVER_H_

#include <shared_mutex>
#include "bluetooth_a2dp_snk.h"
#include "napi_bluetooth_utils.h"
#include "napi_event_subscribe_module.h"

namespace OHOS {
namespace Bluetooth {
const char * const STR_BT_A2DP_SINK_CONNECTION_STATE_CHANGE = "connectionStateChange";

class NapiA2dpSinkObserver : public A2dpSinkObserver {
public:
    void OnConnectionStateChanged(const BluetoothRemoteDevice &device, int state, int cause) override;

    NapiA2dpSinkObserver();
    ~NapiA2dpSinkObserver() override = default;

    NapiEventSubscribeModule eventSubscribe_;
};

}  // namespace Bluetooth
}  // namespace OHOS
#endif /* NAPI_BLUETOOTH_A2DP_SNK_OBSERVER_H_ */