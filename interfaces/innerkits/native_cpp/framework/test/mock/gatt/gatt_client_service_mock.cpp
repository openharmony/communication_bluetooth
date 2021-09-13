/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "gatt_client_service.h"
// #include "gatt_connection_manager.h"

namespace bluetooth {

GattClientService::GattClientService()
{

}

GattClientService::~GattClientService()
{}

int GattClientService::RegisterApplication(IGattClientCallback &callback, const RawAddress &addr, uint8_t transport)
{
    callback_ = &callback;
    return 0;
}

void GattClientService::OnWriteCharacteristicValueEvent(int reqId, uint16_t connectHandle, uint16_t handle, int ret)
{
    callback_->OnCharacteristicWrite(ret, Characteristic());
}

void GattClientService::OnConnectionStateChanged(int state, int newState)
{
    callback_->OnConnectionStateChanged(state, newState);
}

// std::vector<GattDevice> GetAllDevice()
// {
//     std::vector<GattDevice> devices;
//     GattConnectionManager::GetInstance().GetDevices(devices);

//     return devices;
// }
}  // namespace bluetooth
