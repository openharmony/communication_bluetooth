/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_airplane"
#endif

#include "bluetooth_airplane_manager.h"
#include "bluetooth_datashare_utils.h"
#include "bluetooth_errorcode.h"
#include "log.h"
#include "common_util.h"
#include "thread_util.h"
#include "parameter.h"
#include "parameters.h"
#include "hitrace_meter.h"

#undef LOG_TAG
#define LOG_TAG "BtSwitch"

using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {

BluetoothAirplaneManager::BluetoothAirplaneManager() : funcs_(), isAirplaneOn_(false)
{}

BluetoothAirplaneManager::~BluetoothAirplaneManager()
{}

BluetoothAirplaneManager *BluetoothAirplaneManager::GetInstance()
{
    static BluetoothAirplaneManager instance;
    return &instance;
}

bool BluetoothAirplaneManager::IsAirplaneModeOn()
{
    std::string value = "";
    BluetoothDataShareQuery(SETTINGS_DATASHARE_URL_AIRPLANE_MODE, SETTINGS_DATASHARE_KEY_AIRPLANE_MODE, value);
    return value == "1";
}
} // namespace bluetooth
} // namespace OHOS
