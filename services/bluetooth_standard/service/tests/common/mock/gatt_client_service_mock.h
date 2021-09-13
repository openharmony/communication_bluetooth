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

#ifndef GATT_CLIENT_SERVICE_MOCK_H
#define GATT_CLIENT_SERVICE_MOCK_H

#include "context.h"
#include "interface_profile_gatt_client.h"

namespace bluetooth {

class GattClientService : public IProfile, public utility::Context {
public:
    GattClientService();
    ~GattClientService();
    utility::Context *GetContext() override;

    void Enable(void) override;
    void Disable(void) override;

    int Connect(const RawAddress &device) override;
    int Disconnect(const RawAddress &device) override;

    std::list<RawAddress> GetConnectDevices() override;
    int GetConnectState() override;
    int GetMaxConnectNum() override;

private:
    void EnableNative();
    void DisableNative();
};

}  // namespace bluetooth

#endif  // GATT_CLIENT_SERVICE_MOCK_H