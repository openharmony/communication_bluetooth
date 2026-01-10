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

#include <string>
#include "hid_device_utils.h"

namespace OHOS {
namespace Bluetooth {

bool IsValidServiceType(int type)
{
    bool isValidServiceType = false;
    switch (type) {
        case static_cast<int>(ServiceType::SERVICE_NO_TRAFFIC):
        case static_cast<int>(ServiceType::SERVICE_BEST_EFFORT):
        case static_cast<int>(ServiceType::SERVICE_GUARANTEED):
            isValidServiceType = true;
            break;
        default:
            break;
    }
    return isValidServiceType;
}

bool IsValidSubclass(int type)
{
    bool isValidSubclass = false;
    switch (type) {
        case static_cast<int>(Subclass::SUBCLASS_UNCATEGORIZED):
        case static_cast<int>(Subclass::SUBCLASS_JOYSTICK):
        case static_cast<int>(Subclass::SUBCLASS_GAMEPAD):
        case static_cast<int>(Subclass::SUBCLASS_REMOTE_CONTROL):
        case static_cast<int>(Subclass::SUBCLASS_SENSING_DEVICE):
        case static_cast<int>(Subclass::SUBCLASS_DIGITIZER_TABLET):
        case static_cast<int>(Subclass::SUBCLASS_CARD_READER):
        case static_cast<int>(Subclass::SUBCLASS_KEYBOARD):
        case static_cast<int>(Subclass::SUBCLASS_MOUSE):
        case static_cast<int>(Subclass::SUBCLASS_COMBO):
            isValidSubclass = true;
            break;
        default:
            break;
    }
    return isValidSubclass;
}

bool IsValidSDPSettingParam(const std::string &param, size_t maxSdpParamSize)
{
    size_t nameLength = param.size();
    return nameLength > PARAM_SIZE_ZERO && nameLength <= maxSdpParamSize;
}

bool IsValidReportType(int type)
{
    bool isValidType = false;
    switch (type) {
        case static_cast<int>(ReportType::REPORT_TYPE_INPUT):
        case static_cast<int>(ReportType::REPORT_TYPE_OUTPUT):
        case static_cast<int>(ReportType::REPORT_TYPE_FEATURE):
            isValidType = true;
            break;
        default:
            break;
    }
    return isValidType;
}

bool IsValidErrorReason(int errorReason)
{
    bool isValidReason = false;
    switch (errorReason) {
        case static_cast<int>(ErrorReason::RSP_SUCCESS):
        case static_cast<int>(ErrorReason::RSP_NOT_READY):
        case static_cast<int>(ErrorReason::RSP_INVALID_REP_ID):
        case static_cast<int>(ErrorReason::RSP_UNSUPPORTED_REQ):
        case static_cast<int>(ErrorReason::INVALID_PARAM):
        case static_cast<int>(ErrorReason::RSP_UNKNOWN):
            isValidReason = true;
            break;
        default:
            break;
    }
    return isValidReason;
}

}  // namespace Bluetooth
}  // namespace OHOS