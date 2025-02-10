/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef NAPI_NATIVE_OBJECT_H
#define NAPI_NATIVE_OBJECT_H

#include "napi_bluetooth_utils.h"
#include "napi_bluetooth_spp_client.h"

namespace OHOS {
namespace Bluetooth {
class NapiNativeObject {
public:
    virtual ~NapiNativeObject() = default;
    virtual napi_value ToNapiValue(napi_env env) const = 0;
};

class NapiNativeEmpty : public NapiNativeObject {
public:
    ~NapiNativeEmpty() override = default;

    napi_value ToNapiValue(napi_env env) const override
    {
        return NapiGetNull(env);
    }
};

class NapiNativeInt : public NapiNativeObject {
public:
    explicit NapiNativeInt(int value) : value_(value) {}
    ~NapiNativeInt() override = default;

    napi_value ToNapiValue(napi_env env) const override;
private:
    int value_;
};

class NapiNativeBool : public NapiNativeObject {
public:
    explicit NapiNativeBool(bool value) : value_(value) {}
    ~NapiNativeBool() override = default;

    napi_value ToNapiValue(napi_env env) const override;
private:
    bool value_;
};

class NapiNativeString : public NapiNativeObject {
public:
    explicit NapiNativeString(std::string value) : value_(std::move(value)) {}
    ~NapiNativeString() override = default;

    napi_value ToNapiValue(napi_env env) const override;
private:
    std::string value_;
};

class NapiNativeUuidsArray : public NapiNativeObject {
public:
    explicit NapiNativeUuidsArray(const std::vector<std::string> uuids) : uuids_(uuids) {}
    ~NapiNativeUuidsArray() override = default;

    napi_value ToNapiValue(napi_env env) const override;
private:
    std::vector<std::string> uuids_;
};

class NapiNativeDiscoveryResultArray : public NapiNativeObject {
public:
    explicit NapiNativeDiscoveryResultArray(const std::shared_ptr<BluetoothRemoteDevice> &device)
        : remoteDevice_(device) {}
    ~NapiNativeDiscoveryResultArray() override = default;

    napi_value ToNapiValue(napi_env env) const override;
private:
    std::shared_ptr<BluetoothRemoteDevice> remoteDevice_ {nullptr};
};

class NapiNativeDiscoveryInfoResultArray : public NapiNativeObject {
public:
    explicit NapiNativeDiscoveryInfoResultArray(
        const std::shared_ptr<BluetoothRemoteDevice> &device, int rssi, const std::string deviceName, int deviceClass)
        : remoteDevice_(device), rssi_(rssi), deviceName_(deviceName), deviceClass_(deviceClass) {}
    ~NapiNativeDiscoveryInfoResultArray() override = default;

    napi_value ToNapiValue(napi_env env) const override;
private:
    std::shared_ptr<BluetoothRemoteDevice> remoteDevice_ {nullptr};
    int rssi_ = 0;
    std::string deviceName_ = "";
    int deviceClass_ = MajorClass::MAJOR_UNCATEGORIZED;
};

class NapiNativePinRequiredParam : public NapiNativeObject {
public:
    explicit NapiNativePinRequiredParam(const std::shared_ptr<PairConfirmedCallBackInfo> &pairConfirmInfo)
        : pairConfirmInfo_(pairConfirmInfo) {}
    ~NapiNativePinRequiredParam() override = default;

    napi_value ToNapiValue(napi_env env) const override;
private:
    std::shared_ptr<PairConfirmedCallBackInfo> pairConfirmInfo_ {nullptr};
};

class NapiNativeBondStateParam : public NapiNativeObject {
public:
    NapiNativeBondStateParam(std::string deviceAddr, int bondStatus, int unbondCause)
        : deviceAddr_(deviceAddr), bondStatus_(bondStatus), unbondCause_(unbondCause) {}
    ~NapiNativeBondStateParam() override = default;

    napi_value ToNapiValue(napi_env env) const override;
private:
    std::string deviceAddr_ = "";
    int bondStatus_ = -1;
    int unbondCause_ = -1;
};

class NapiNativeStateChangeParam : public NapiNativeObject {
public:
    NapiNativeStateChangeParam(std::string deviceAddr, int connectState, int cause = 0)
        : deviceAddr_(deviceAddr), connectState_(connectState), stateChangeCause_(cause) {}
    virtual ~NapiNativeStateChangeParam() override = default;

    napi_value ToNapiValue(napi_env env) const override;
private:
    std::string deviceAddr_ = "";
    int connectState_ = -1;
    int stateChangeCause_ = -1;
};

class NapiNativeBleConnectionStateChangeParam : public NapiNativeStateChangeParam {
public:
    NapiNativeBleConnectionStateChangeParam(std::string deviceAddr, int connectState)
        : NapiNativeStateChangeParam(deviceAddr, connectState) {}
    ~NapiNativeBleConnectionStateChangeParam() override = default;
};

class NapiNativeGattsCharacterReadRequest : public NapiNativeObject {
public:
    NapiNativeGattsCharacterReadRequest(int transId, std::string deviceAddr, GattCharacteristic &character)
        : transId_(transId), deviceAddr_(deviceAddr), character_(character) {}
    ~NapiNativeGattsCharacterReadRequest() override = default;

    napi_value ToNapiValue(napi_env env) const override;
private:
    int transId_ = 0;
    std::string deviceAddr_ = "";
    GattCharacteristic character_;
};

class NapiNativeGattsCharacterWriteRequest : public NapiNativeObject {
public:
    NapiNativeGattsCharacterWriteRequest(int transId, std::string deviceAddr, GattCharacteristic &character)
        : transId_(transId), deviceAddr_(deviceAddr), character_(character) {}
    ~NapiNativeGattsCharacterWriteRequest() override = default;

    napi_value ToNapiValue(napi_env env) const override;
private:
    int transId_ = 0;
    std::string deviceAddr_ = "";
    GattCharacteristic character_;
};

class NapiNativeGattsDescriptorWriteRequest : public NapiNativeObject {
public:
    NapiNativeGattsDescriptorWriteRequest(int transId, std::string deviceAddr, GattDescriptor &descriptor)
        : transId_(transId), deviceAddr_(deviceAddr), descriptor_(descriptor) {}
    ~NapiNativeGattsDescriptorWriteRequest() override = default;

    napi_value ToNapiValue(napi_env env) const override;
private:
    int transId_ = 0;
    std::string deviceAddr_ = "";
    GattDescriptor descriptor_;
};

class NapiNativeGattsDescriptorReadRequest : public NapiNativeObject {
public:
    NapiNativeGattsDescriptorReadRequest(int transId, std::string deviceAddr, GattDescriptor &descriptor)
        : transId_(transId), deviceAddr_(deviceAddr), descriptor_(descriptor) {}
    ~NapiNativeGattsDescriptorReadRequest() override = default;

    napi_value ToNapiValue(napi_env env) const override;
private:
    int transId_ = 0;
    std::string deviceAddr_ = "";
    GattDescriptor descriptor_;
};

class NapiNativeOppTransferInformation : public NapiNativeObject {
public:
    explicit NapiNativeOppTransferInformation(const BluetoothOppTransferInformation &information)
        : information_(information) {}
    ~NapiNativeOppTransferInformation() override = default;

    napi_value ToNapiValue(napi_env env) const override;
private:
    BluetoothOppTransferInformation information_;
};

class NapiNativeBatteryInfo : public NapiNativeObject {
public:
    explicit NapiNativeBatteryInfo(DeviceBatteryInfo batteryInfo) : batteryInfo_(batteryInfo) {}
    ~NapiNativeBatteryInfo() override = default;

    napi_value ToNapiValue(napi_env env) const override;
private:
    DeviceBatteryInfo batteryInfo_;
};

class NapiNativeInt64 : public NapiNativeObject {
public:
    explicit NapiNativeInt64(int64_t value) : value_(value) {}
    ~NapiNativeInt64() override = default;

    napi_value ToNapiValue(napi_env env) const override;
private:
    int64_t value_;
};

class NapiNativeArrayBuffer : public NapiNativeObject {
public:
    explicit NapiNativeArrayBuffer(SppCallbackBuffer sppBuffer)
        : sppBuffer_(sppBuffer) {}
    ~NapiNativeArrayBuffer() override = default;

    napi_value ToNapiValue(napi_env env) const override;
private:
    SppCallbackBuffer sppBuffer_;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // NAPI_NATIVE_OBJECT_H