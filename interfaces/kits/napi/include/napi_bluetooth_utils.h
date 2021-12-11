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
#ifndef NAPI_BLUETOOTH_UTILS_H
#define NAPI_BLUETOOTH_UTILS_H

#include "bluetooth_gatt_characteristic.h"
#include "bluetooth_gatt_client.h"
#include "bluetooth_gatt_descriptor.h"
#include "bluetooth_gatt_server.h"
#include "bluetooth_gatt_service.h"
#include "bluetooth_log.h"
#include "bluetooth_remote_device.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include <condition_variable>
#include <mutex>
#include <stdint.h>
#include <string>
#include <vector>

#include "bluetooth_socket.h"

namespace OHOS {
namespace Bluetooth {

constexpr size_t CALLBACK_SIZE = 1;
constexpr size_t ARGS_SIZE_ONE = 1;
constexpr size_t ARGS_SIZE_TWO = 2;
constexpr size_t ARGS_SIZE_THREE = 3;
constexpr size_t ARGS_SIZE_FOUR = 4;
constexpr int32_t DEFAULT_INT32 = 0;
constexpr int32_t PARAM0 = 0;
constexpr int32_t PARAM1 = 1;
constexpr int32_t PARAM2 = 2;
constexpr int32_t CODE_SUCCESS = 0;
constexpr int32_t CODE_FAILED = -1;
constexpr int ASYNC_IDLE = 0;
constexpr int ASYNC_START = 1;
constexpr int ASYNC_DONE = 2;
constexpr int32_t THREAD_WAIT_TIMEOUT = 5;

struct ServerResponse {
    std::string deviceId = "";
    int transId = 0;
    int status = 0;
    int offset = 0;
    uint8_t *value = nullptr;
    int length = 0;
    void SetValue(uint8_t *values, size_t len)
    {
        HILOGI("GattCharacteristic::SetValue starts");
        if (value != nullptr) {
            value = nullptr;
        }

        length = len;
        value = values;
    }
};

struct SppOption {
    std::string uuid_ = "";
    bool secure_ = false;
    SppSocketType type_;
};

const std::string REGISTER_DEVICE_FIND_TYPE = "bluetoothDeviceFind";
const std::string REGISTER_STATE_CHANGE_TYPE = "stateChange";
const std::string REGISTER_PIN_REQUEST_TYPE = "pinRequired";
const std::string REGISTER_BONE_STATE_TYPE = "boneStateChange";
const std::string REGISTER_BLE_FIND_DEVICE_TYPE = "BLEDeviceFind";

const int STATUS_BONDED = 2;
const int STATUS_BONDING = 1;

bool ParseString(napi_env env, std::string &param, napi_value args);
bool ParseInt32(napi_env env, int32_t &param, napi_value args);
bool ParseBool(napi_env env, bool &param, napi_value args);
bool ParseArrayBuffer(napi_env env, uint8_t **data, size_t &size, napi_value args);
napi_value GetCallbackErrorValue(napi_env env, int errCode);

void ConvertStringVectorToJS(napi_env env, napi_value result, std::vector<std::string> &stringVector);

void ConvertGattServiceToJS(napi_env env, napi_value result, GattService &service);
void ConvertGattServiceVectorToJS(napi_env env, napi_value result, std::vector<GattService> &services);

void ConvertBLECharacteristicToJS(napi_env env, napi_value result, GattCharacteristic &characteristic);
void ConvertBLECharacteristicVectorToJS(
    napi_env env, napi_value result, std::vector<GattCharacteristic> &characteristics);

void ConvertBLEDescriptorToJS(napi_env env, napi_value result, GattDescriptor &descriptor);
void ConvertBLEDescriptorVectorToJS(napi_env env, napi_value result, std::vector<GattDescriptor> &descriptors);

void ConvertCharacteristicReadReqToJS(napi_env env, napi_value result, const BluetoothRemoteDevice &device,
    GattCharacteristic &characteristic, int requestId);

void ConvertCharacteristicWriteReqToJS(napi_env env, napi_value result, const BluetoothRemoteDevice &device,
    GattCharacteristic &characteristic, int requestId);

void ConvertDescriptorReadReqToJS(
    napi_env env, napi_value result, const BluetoothRemoteDevice &device, GattDescriptor &descriptor, int requestId);

void ConvertDescriptorWriteReqToJS(
    napi_env env, napi_value result, const BluetoothRemoteDevice &device, GattDescriptor &descriptor, int requestId);

void ConvertBLEConnectStateChangeToJS(napi_env env, napi_value result, int connectState, int ret);
void ConvertBLEConnectStateChangeToJS(napi_env env, napi_value result, const BluetoothRemoteDevice &device, int state);

void GetServiceVectorFromJS(napi_env env, napi_value object, std::vector<GattService> &services,
    std::shared_ptr<GattServer> server, std::shared_ptr<GattClient> client);
GattService *GetServiceFromJS(
    napi_env env, napi_value object, std::shared_ptr<GattServer> server, std::shared_ptr<GattClient> client);

void GetCharacteristicVectorFromJS(napi_env env, napi_value object, std::vector<GattCharacteristic> &characteristics,
    std::shared_ptr<GattServer> server, std::shared_ptr<GattClient> client);
GattCharacteristic *GetCharacteristicFromJS(
    napi_env env, napi_value object, std::shared_ptr<GattServer> server, std::shared_ptr<GattClient> client);

void GetDescriptorVectorFromJS(napi_env env, napi_value object, std::vector<GattDescriptor> &descriptors);
GattDescriptor *GetDescriptorFromJS(
    napi_env env, napi_value object, std::shared_ptr<GattServer> server, std::shared_ptr<GattClient> client);

ServerResponse GetServerResponseFromJS(napi_env env, napi_value object);
std::shared_ptr<SppOption> GetSppOptionFromJS(napi_env env, napi_value object);

void SetNamedPropertyByInteger(napi_env env, napi_value dstObj, int32_t objName, const char *propName);
napi_value NapiGetNull(napi_env env);
napi_value RegisterObserver(napi_env env, napi_callback_info info);
napi_value DeregisterObserver(napi_env env, napi_callback_info info);

struct AsyncCallbackInfo {
    napi_env env_;
    napi_async_work asyncWork_;
    napi_deferred deferred_;
    napi_ref callback_ = 0;
    int errorCode_ = 0;
};

struct ReadCharacteristicValueCallbackInfo : public AsyncCallbackInfo {
    GattCharacteristic *inputCharacteristic_ = nullptr;
    const GattCharacteristic *outputCharacteristic_ = nullptr;
    int ret = -1;
    std::shared_ptr<GattClient> client_ = nullptr;
    int asyncState_ = ASYNC_IDLE;
};

struct ReadDescriptorValueCallbackInfo : public AsyncCallbackInfo {
    // std::shared_ptr<GattDescriptor> inputDescriptor_ = nullptr;
    GattDescriptor *inputDescriptor_ = nullptr;
    const GattDescriptor *outputDescriptor_ = nullptr;
    int ret = -1;
    std::shared_ptr<GattClient> client_ = nullptr;
    int asyncState_ = ASYNC_IDLE;
};

struct GetServiceCallbackInfo : public AsyncCallbackInfo {
    std::vector<GattService> services_;
    std::shared_ptr<GattClient> client_ = nullptr;
};

struct SppListenCallbackInfo : public AsyncCallbackInfo {
    std::shared_ptr<SppServerSocket> server_ = nullptr;
    std::string name_ = "";
    std::shared_ptr<SppOption> sppOption_;
};

struct SppAcceptCallbackInfo : public AsyncCallbackInfo {
    std::shared_ptr<SppServerSocket> server_ = nullptr;
    std::shared_ptr<SppClientSocket> client_ = nullptr;
};

struct SppConnectCallbackInfo : public AsyncCallbackInfo {
    std::shared_ptr<SppClientSocket> client_ = nullptr;
    std::string deviceId_ = "";
    std::shared_ptr<BluetoothRemoteDevice> device_ = nullptr;
    std::shared_ptr<SppOption> sppOption_ = nullptr;
};

struct CallbackPromiseInfo {
    napi_ref callback = nullptr;
    napi_deferred deferred = nullptr;
    bool isCallback = false;
    int errorCode = 0;
};

struct GattGetDeviceNameCallbackInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_value result = nullptr;
    std::string deviceId = "";
    CallbackPromiseInfo promise;
};

struct GattGetRssiValueCallbackInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_value result = nullptr;
    int32_t rssi = 0;
    std::mutex mutexRssi {};
    std::condition_variable cvfull {};
    CallbackPromiseInfo promise;
};

struct DiscoverServicesCallbackInfo {
    int asyncState_ = ASYNC_IDLE;
    int status_;
};

struct BluetoothCallbackInfo {
    napi_env env_;
    napi_ref callback_ = 0;
};

namespace {
std::map<std::string, std::shared_ptr<BluetoothCallbackInfo>> g_Observer;
std::shared_ptr<GattGetRssiValueCallbackInfo> callbackInfo = nullptr;
std::string deviceAddr;
}  // namespace
std::map<std::string, std::shared_ptr<BluetoothCallbackInfo>> GetObserver();
void SetGattClinetDeviceId(const std::string &deviceId);
std::string GetGattClientDeviceId();

void SetRssiValueCallbackInfo(std::shared_ptr<GattGetRssiValueCallbackInfo> &callback);
std::shared_ptr<GattGetRssiValueCallbackInfo> GetRssiValueCallbackInfo();

struct ScanFilter {
    std::string deviceId;     // The name of a BLE peripheral device
    std::string name;         // The name of a BLE peripheral device
    std::string serviceUuid;  // The service UUID of a BLE peripheral device
};

enum MatchMode {
    MATCH_MODE_AGGRESSIVE = 1,  //  aggressive mode
    MATCH_MODE_STICKY = 2       // sticky mode
};

enum class ScanDuty {
    SCAN_MODE_LOW_POWER = 0,   // low power mode */
    SCAN_MODE_BALANCED = 1,    // balanced power mode
    SCAN_MODE_LOW_LATENCY = 2  // Scan using highest duty cycle
};

struct ScanOptions {
    int32_t interval;                                        // Time of delay for reporting the scan result
    ScanDuty dutyMode = ScanDuty::SCAN_MODE_LOW_POWER;       // Bluetooth LE scan mode
    MatchMode MatchMode = MatchMode::MATCH_MODE_AGGRESSIVE;  // Match mode for Bluetooth LE scan filters hardware match
};

struct ScanResult {
    std::string deviceId;       // Address of the scanned device
    int32_t rssi;               // RSSI of the remote device
    std::vector<uint8_t> data;  // The raw data of broadcast packet
};

enum ProfileConnectionState {
    STATE_DISCONNECTED = 0,  // the current profile is disconnected
    STATE_CONNECTING = 1,    // the current profile is being connected
    STATE_CONNECTED = 2,     // the current profile is connected
    STATE_DISCONNECTING = 3  // the current profile is being disconnected
};

enum class BluetoothState {
    /** Indicates the local Bluetooth is off */
    STATE_OFF = 0,
    /** Indicates the local Bluetooth is turning on */
    STATE_TURNING_ON = 1,
    /** Indicates the local Bluetooth is on, and ready for use */
    STATE_ON = 2,
    /** Indicates the local Bluetooth is turning off */
    STATE_TURNING_OFF = 3,
    /** Indicates the local Bluetooth is turning LE mode on */
    STATE_BLE_TURNING_ON = 4,
    /** Indicates the local Bluetooth is in LE only mode */
    STATE_BLE_ON = 5,
    /** Indicates the local Bluetooth is turning off LE only mode */
    STATE_BLE_TURNING_OFF = 6
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // NAPI_BLUETOOTH_UTILS_H
