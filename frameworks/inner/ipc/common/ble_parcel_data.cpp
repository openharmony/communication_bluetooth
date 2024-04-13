/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "ble_parcel_data.h"

namespace OHOS {
namespace bluetooth {

ScanResult::ScanResult(const BleScanResultImpl &other)
{
    const auto &device = other.GetPeripheralDevice();
    if (device.IsRSSI()) {
        this->SetRssi(device.GetRSSI());
    }
    this->SetAdvertiseFlag(device.GetAdFlag());

    if (device.IsManufacturerData()) {
        auto manuData = device.GetManufacturerData();
        for (auto it = manuData.begin(); it != manuData.end(); it++) {
            this->AddManufacturerData(it->first, it->second);
        }
    }
    this->SetConnectable(device.IsConnectable());

    if (device.IsServiceUUID()) {
        auto uuids = device.GetServiceUUID();
        for (auto iter = uuids.begin(); iter != uuids.end(); iter++) {
            this->AddServiceUuid(*iter);
        }
    }

    if (device.IsServiceData()) {
        auto uuids = device.GetServiceDataUUID();
        int index = 0;
        for (auto iter = uuids.begin(); iter != uuids.end(); iter++) {
            this->AddServiceData(*iter, device.GetServiceData(index));
            ++index;
        }
    }

    if (device.IsName()) {
        this->SetName(device.GetName());
    }

    if (device.IsEventType()) {
        this->SetEventType(device.GetEventType());
    }

    this->SetPeripheralDevice(device.GetRawAddress());
    this->SetPayload(std::string(device.GetPayload(), device.GetPayload() + device.GetPayloadLen()));
}

}  // namespace bluetooth
}  // namespace OHOS