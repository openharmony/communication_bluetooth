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

#ifndef OHOS_BLUETOOTH_COMMON_EVENT_HELPER_H
#define OHOS_BLUETOOTH_COMMON_EVENT_HELPER_H

#include <string>

namespace OHOS {
namespace BluetoothHelper {
const std::string COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE = "usual.event.bluetooth.host.STATE_UPDATE";

class BluetoothCommonEventHelper {
public:
    static bool PublishEvent(const std::string &eventAction,int code,
        const std::vector<std::string> &permissions);
    static bool PublishBluetoothStateChangeEvent(int code, const std::vector<std::string> &permissions);
};
}  // namespace BluetoothHelper
}  // namespace OHOS
#endif