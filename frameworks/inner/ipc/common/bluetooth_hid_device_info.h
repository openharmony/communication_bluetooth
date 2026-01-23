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

#ifndef BLUETOOTH_HID_DEVICE_INFO_H
#define BLUETOOTH_HID_DEVICE_INFO_H

namespace OHOS {
namespace Bluetooth {

const int HID_DEVICE_UNREGISTERED = 0;
const int HID_DEVICE_REGISTERED = 1;

enum Subclass {
    SUBCLASS_UNCATEGORIZED = 0,
    SUBCLASS_JOYSTICK = 1,
    SUBCLASS_GAMEPAD = 2,
    SUBCLASS_REMOTE_CONTROL = 3,
    SUBCLASS_SENSING_DEVICE = 4,
    SUBCLASS_DIGITIZER_TABLET = 5,
    SUBCLASS_CARD_READER = 6,
    SUBCLASS_KEYBOARD = 64,
    SUBCLASS_MOUSE = 128,
    SUBCLASS_COMBO = 192,
};

enum ReportType {
    REPORT_TYPE_INPUT = 1,
    REPORT_TYPE_OUTPUT,
    REPORT_TYPE_FEATURE,
};

enum ServiceType {
    SERVICE_NO_TRAFFIC = 0,
    SERVICE_BEST_EFFORT,
    SERVICE_GUARANTEED,
};

enum ErrorReason {
    RSP_SUCCESS = 0,
    RSP_NOT_READY = 1,
    RSP_INVALID_REP_ID = 2,
    RSP_UNSUPPORTED_REQ = 3,
    INVALID_PARAM = 4,
    RSP_UNKNOWN = 14,
};

enum ProtocolType {
    PROTOCOL_BOOT_MODE = 0,
    PROTOCOL_REPORT_MODE = 1,
};

struct HidDeviceAppInfo {
    uint32_t tokenId = {};
    std::string appId = {};
    std::string name = {};
    bool operator!=(const HidDeviceAppInfo& other) const
    {
        return (tokenId != other.tokenId) || (appId != other.appId) || (name != other.name);
    }
    bool operator==(const HidDeviceAppInfo& other) const
    {
        return (tokenId == other.tokenId) && (appId == other.appId) && (name == other.name);
    }
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // BLUETOOTH_HID_DEVICE_INFO_H