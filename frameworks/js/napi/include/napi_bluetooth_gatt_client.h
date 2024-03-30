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
#ifndef NAPI_BLUETOOTH_GATT_CLIENT_H_
#define NAPI_BLUETOOTH_GATT_CLIENT_H_

#include "bluetooth_gatt_client.h"
#include "bluetooth_log.h"
#include "bluetooth_remote_device.h"
#include "napi_bluetooth_gatt_client_callback.h"

namespace OHOS {
namespace Bluetooth {
const int SLEEP_TIME = 50000;
const int VALUE_1 = 1;
const int VALUE_2 = 2;
const int VALUE_3 = 3;

class NapiGattClient {
public:

    static napi_value CreateGattClientDevice(napi_env env, napi_callback_info info);
    static void DefineGattClientJSClass(napi_env env);
    static napi_value GattClientConstructor(napi_env env, napi_callback_info info);

    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);

    static napi_value Connect(napi_env env, napi_callback_info info);
    static napi_value Disconnect(napi_env env, napi_callback_info info);
    static napi_value Close(napi_env env, napi_callback_info info);
    static napi_value GetServices(napi_env env, napi_callback_info info);
    static napi_value ReadCharacteristicValue(napi_env env, napi_callback_info info);
    static napi_value ReadDescriptorValue(napi_env env, napi_callback_info info);

#ifdef BLUETOOTH_API_SINCE_10
    static napi_value WriteCharacteristicValueEx(napi_env env, napi_callback_info info);
    static napi_value WriteDescriptorValueEx(napi_env env, napi_callback_info info);
    static napi_value setCharacteristicChangeNotification(napi_env env, napi_callback_info info);
    static napi_value setCharacteristicChangeIndication(napi_env env, napi_callback_info info);
#else
    static napi_value WriteCharacteristicValue(napi_env env, napi_callback_info info);
    static napi_value WriteDescriptorValue(napi_env env, napi_callback_info info);
    static napi_value SetNotifyCharacteristicChanged(napi_env env, napi_callback_info info);
#endif

    static napi_value SetBLEMtuSize(napi_env env, napi_callback_info info);
    static napi_value GetRssiValue(napi_env env, napi_callback_info info);
    static napi_value GetDeviceName(napi_env env, napi_callback_info info);

    std::shared_ptr<GattClient> &GetClient()
    {
        return client_;
    }

    std::shared_ptr<NapiGattClientCallback> GetCallback()
    {
        return callback_;
    }

    std::shared_ptr<BluetoothRemoteDevice> GetDevice()
    {
        return device_;
    }

    explicit NapiGattClient(std::string &deviceId)
    {
        HILOGI("enter");
        device_ = std::make_shared<BluetoothRemoteDevice>(deviceId, 1);
        client_ = std::make_shared<GattClient>(*device_);
        client_->Init();
        callback_ = std::make_shared<NapiGattClientCallback>();
        callback_->deviceAddr_ = deviceId;
    }
    ~NapiGattClient() = default;

    static thread_local napi_ref consRef_;

private:
    std::shared_ptr<GattClient> client_ = nullptr;
    std::shared_ptr<NapiGattClientCallback> callback_;
    std::shared_ptr<BluetoothRemoteDevice> device_ = nullptr;
};
} // namespace Bluetooth
} // namespace OHOS
#endif /* NAPI_BLUETOOTH_GATT_CLIENT_H_ */