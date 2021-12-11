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

#ifndef NAPI_BLUETOOTH_GATT_SERVER_H_
#define NAPI_BLUETOOTH_GATT_SERVER_H_

#include "bluetooth_gatt_server.h"
#include "bluetooth_log.h"
#include "napi_bluetooth_gatt_server_callback.h"
#include <vector>

namespace OHOS {
namespace Bluetooth {


// std::enable_shared_from_this<NGattServer>
class NGattServer {

public:
    static napi_value CreateGattServer(napi_env env, napi_callback_info info);
    static void DefineGattServerJSClass(napi_env env);
    static napi_value GattServerConstructor(napi_env env, napi_callback_info info);

    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info); 

    static napi_value AddService(napi_env env, napi_callback_info info);
    static napi_value Close(napi_env env, napi_callback_info info);
    static napi_value RemoveGattService(napi_env env, napi_callback_info info);
    static napi_value SendResponse(napi_env env, napi_callback_info info);
    static napi_value NotifyCharacteristicChanged(napi_env env, napi_callback_info info);

    static napi_value OnCharacteristicReadRequest(napi_env env, napi_callback_info info); //Test callback only
    static napi_value OnCharacteristicWriteRequest(napi_env env, napi_callback_info info); //Test callback only
    static napi_value OnDescriptorReadRequest(napi_env env, napi_callback_info info);//Test callback only
    static napi_value OnDescriptorWriteRequest(napi_env env, napi_callback_info info);//Test callback only

    static napi_value OnConnectionStateUpdate(napi_env env, napi_callback_info info); //Test callback only
    

    std::shared_ptr<GattServer> &GetServer() {return server_;}
    NGattServerCallback &GetCallback(){return callback_;} 
    static std::vector<std::string> deviceList;

    void OnCharacteristicReadRequest() { //Test callback only
        HILOGI("OnCharacteristicReadRequest called");       
        GattCharacteristic characteristic(UUID::FromString("21"),1,2,3);
        BluetoothRemoteDevice device("31", 0);
        callback_.OnCharacteristicReadRequest(device, characteristic, 3);
    }
    
    void OnCharacteristicWriteRequest() { //Test callback only
        HILOGI("OnCharacteristicWriteRequest called");    
        uint8_t data[1];
        data[0] = 11;   
        GattCharacteristic characteristic(UUID::FromString("21"),1,2,3);
        characteristic.SetValue(data, 1);
        BluetoothRemoteDevice device("31", 0);
        callback_.OnCharacteristicWriteRequest(device, characteristic, 3);
    }

    void OnDescriptorReadRequest() { //Test callback only
        HILOGI("OnDescriptorReadRequest called");       
        GattDescriptor descriptor(UUID::FromString("33"),1,2);
        BluetoothRemoteDevice device("31", 0);
        callback_.OnDescriptorReadRequest(device, descriptor, 2);
    }
    
    void OnDescriptorWriteRequest() { //Test callback only
        HILOGI("OnDescriptorWriteRequest called");       
        // GattDescriptor descriptor(UUID::FromString("34"),1,2);
        // BluetoothRemoteDevice device("31", 0);
        // callback_.OnDescriptorWriteRequest(device, descriptor, 2);
        uint8_t data[1];
        UUID uuidDes;
        data[0] = 10;
        uuidDes = UUID::FromString("00001830-0000-1000-8000-00805F9B34FB");

        GattDescriptor descriptorTest(uuidDes, 3);
        descriptorTest.SetValue(data, 1);
        BluetoothRemoteDevice device("8F:8F:8E:8E:6D:6D", 0);
        callback_.OnDescriptorWriteRequest(device, descriptorTest, 2);
    }

    void OnConnectionStateUpdate() { //Test callback only
        HILOGI("Server OnConnectionStateUpdate called");       
        BluetoothRemoteDevice device("8F:8F:8E:8E:6D:6D", 0);
        int state = 0;
        callback_.OnConnectionStateUpdate(device, state);
    }
    NGattServer() {
        HILOGI("NGattServer called");
        server_ = std::make_shared<GattServer>(callback_);
    }
    ~NGattServer() = default;

    static napi_value constructor_;

private:
    std::shared_ptr<GattServer> server_ = nullptr;
    NGattServerCallback callback_;
};

}  // namespace Bluetooth
}  // namespace OHOS
#endif /* NAPI_BLUETOOTH_GATT_SERVER_H_ */