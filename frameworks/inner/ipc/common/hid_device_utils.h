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

#ifndef HID_DEVICE_UTILS_H
#define HID_DEVICE_UTILS_H

#include <cstring>
#include <memory>
#include <vector>
#include <string>
#include "bluetooth_hid_device_info.h"

namespace OHOS {
namespace Bluetooth {
const size_t MAX_DESCRIPTOR_SIZE = 2048;
const size_t MAX_NAME_SIZE = 50;
const size_t MAX_DESCRIPTION_SIZE = 50;
const size_t MAX_PROVIDER_SIZE = 50;
const int DEFAULT_TOKEN_RATE = 0;
const int DEFAULT_TOKEN_BUCKET_SIZE = 0;
const int DEFAULT_PEAK_BANDWIDTH = 0;
const int DEFAULT_LATENCY = -1;
const int DEFAULT_DELAY_VARIATION = -1;
const int PARAM_SIZE_ZERO = 0;

bool IsValidServiceType(int type);
bool IsValidSubclass(int type);
bool IsValidSDPSettingParam(const std::string &param, size_t maxSdpParamSize);
bool IsValidReportType(int type);
bool IsValidErrorReason(int errorReason);

}  // namespace Bluetooth
}  // namespace OHOS
#endif // HID_DEVICE_UTILS_H