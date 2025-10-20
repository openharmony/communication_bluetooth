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

#ifndef TAIHE_BLUETOOTH_GATT_SERVER_H_
#define TAIHE_BLUETOOTH_GATT_SERVER_H_

#include <vector>

#include "bluetooth_gatt_server.h"
#include "bluetooth_log.h"
#include "ohos.bluetooth.ble.impl.hpp"
#include "ohos.bluetooth.ble.proj.hpp"
#include "taihe_bluetooth_gatt_server_callback.h"
#include "taihe/runtime.hpp"

namespace OHOS {
namespace Bluetooth {
class GattServerImpl {
public:
    void AddService(ohos::bluetooth::ble::GattService service);
    void Close();
    void RemoveService(taihe::string_view serviceUuid);
    void SendResponse(ohos::bluetooth::ble::ServerResponse serverResponse);
    uintptr_t NotifyCharacteristicChangedPromise(::taihe::string_view deviceId,
        ::ohos::bluetooth::ble::NotifyCharacteristic const& notifyCharacteristic);
    void NotifyCharacteristicChangedAsync(::taihe::string_view deviceId,
        ::ohos::bluetooth::ble::NotifyCharacteristic const& notifyCharacteristic, uintptr_t callback);

    std::shared_ptr<GattServer> &GetServer()
    {
        return server_;
    }
    std::shared_ptr<TaiheGattServerCallback> GetCallback()
    {
        return callback_;
    }
    static std::vector<std::string> deviceList_;
    static std::mutex deviceListMutex_;

    GattServerImpl()
    {
        HILOGI("enter");
        callback_ = std::make_shared<TaiheGattServerCallback>();
        std::shared_ptr<GattServerCallback> tmp = std::static_pointer_cast<GattServerCallback>(callback_);
        server_  = GattServer::CreateInstance(tmp);
    }
    ~GattServerImpl() = default;

private:
    std::shared_ptr<GattServer> server_ = nullptr;
    std::shared_ptr<TaiheGattServerCallback> callback_;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif /* TAIHE_BLUETOOTH_GATT_SERVER_H_ */
