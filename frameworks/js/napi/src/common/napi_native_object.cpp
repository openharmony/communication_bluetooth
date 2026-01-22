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
#ifndef LOG_TAG
#define LOG_TAG "bt_napi_native_object"
#endif

#include "napi_native_object.h"
#include "securec.h"

namespace OHOS {
namespace Bluetooth {

constexpr int ADDRESS_BYTE_LEN = 6;
constexpr int ADDRESS_WITH_TYPE_BYTE_LEN = 7; // 6字节地址+1字节地址类型

napi_value NapiNativeInt::ToNapiValue(napi_env env) const
{
    napi_value value = nullptr;
    napi_status status = napi_create_int32(env, value_, &value);
    if (status != napi_ok) {
        HILOGE("napi_create_int32 failed");
    }
    return value;
}

napi_value NapiNativeBool::ToNapiValue(napi_env env) const
{
    napi_value value = nullptr;
    napi_status status = napi_get_boolean(env, value_, &value);
    if (status != napi_ok) {
        HILOGE("napi_create_int32 failed");
    }
    return value;
}

napi_value NapiNativeString::ToNapiValue(napi_env env) const
{
    napi_value value = nullptr;
    napi_status status = napi_create_string_utf8(env, value_.c_str(), NAPI_AUTO_LENGTH, &value);
    if (status != napi_ok) {
        HILOGE("napi_create_string_utf8 failed");
    }
    return value;
}

napi_value NapiNativeUuidsArray::ToNapiValue(napi_env env) const
{
    napi_value array;
    napi_create_array(env, &array);
    ConvertUuidsVectorToJS(env, array, uuids_);
    return array;
}

napi_value NapiNativeDiscoveryResultArray::ToNapiValue(napi_env env) const
{
    CHECK_AND_RETURN_LOG_RET(remoteDevice_, nullptr, "remoteDevice is nullptr");

    napi_value array;
    napi_value value;
    std::string addr = remoteDevice_->GetDeviceAddr();
    napi_create_array(env, &array);
    napi_create_string_utf8(env, addr.c_str(), addr.size(), &value);
    napi_set_element(env, array, 0, value);
    return array;
}

void ConvertDeviceClassToJS(napi_env env, napi_value result, int deviceClass)
{
    BluetoothDeviceClass classOfDevice = BluetoothDeviceClass(deviceClass);
    int tmpMajorClass = classOfDevice.GetMajorClass();
    int tmpMajorMinorClass = classOfDevice.GetMajorMinorClass();

    napi_value majorClass = 0;
    napi_create_int32(env, tmpMajorClass, &majorClass);
    napi_set_named_property(env, result, "majorClass", majorClass);
    napi_value majorMinorClass = 0;
    napi_create_int32(env, tmpMajorMinorClass, &majorMinorClass);
    napi_set_named_property(env, result, "majorMinorClass", majorMinorClass);
    napi_value cod = 0;
    napi_create_int32(env, deviceClass, &cod);
    napi_set_named_property(env, result, "classOfDevice", cod);
}

void ConvertAddressInfoToJs(napi_env env, napi_value result, const AddressInfo &addressInfo)
{
    napi_value address = nullptr;
    std::string addr = addressInfo.GetAddress();
    napi_create_string_utf8(env, addr.c_str(), addr.size(), &address);
    napi_set_named_property(env, result, "address", address);

    napi_value addressType = nullptr;
    if (addressInfo.HasAddressType()) {
        napi_create_int32(env, static_cast<int32_t>(addressInfo.GetAddressType()), &addressType);
    } else {
        napi_create_int32(env, static_cast<int32_t>(AddressType::VIRTUAL_ADDRESS), &addressType);
    }
    napi_set_named_property(env, result, "addressType", addressType);

    napi_value rawAddressType = nullptr;
    if (addressInfo.HasRawAddressType()) {
        napi_create_int32(env, static_cast<int32_t>(addressInfo.GetRawAddressType()), &rawAddressType);
    } else {
        napi_create_int32(env, static_cast<int32_t>(RawAddressType::PUBLIC_ADDRESS), &rawAddressType);
    }
    napi_set_named_property(env, result, "rawAddressType", rawAddressType);
}

void ConvertUint8VectorToJsUint8Array(napi_env env, napi_value &result, const std::vector<uint8_t> &data)
{
    size_t valueSize = data.size();
    const uint8_t *valueData = data.data();
    napi_value buffer = nullptr;
    uint8_t *bufferData = nullptr;
    napi_create_arraybuffer(env, valueSize, (void**)&bufferData, &buffer);
    if (valueSize > 0 && memcpy_s(bufferData, valueSize, valueData, valueSize) != EOK) {
        HILOGE("memcpy_s error");
    }
    napi_create_typedarray(env, napi_uint8_array, valueSize, buffer, 0, &result);
}

napi_value NapiNativeOobData::ToNapiValue(napi_env env) const
{
    napi_value result = nullptr;
    napi_create_object(env, &result);

    napi_value deviceId = nullptr;
    napi_create_object(env, &deviceId);

    std::string address = "";
    uint8_t addressType = AddressType::REAL_ADDRESS; // OOB数据中的地址一定是真实地址
    uint8_t rawAddressType = RawAddressType::RANDOM_ADDRESS;
    if (data_.HasAddressWithType() && (data_.GetAddressWithType().size() == ADDRESS_WITH_TYPE_BYTE_LEN)) {
        std::vector<uint8_t> addressWithType = data_.GetAddressWithType();
        // addressWithType中前6位为地址，第7位为地址类型
        std::vector<uint8_t> addressVec(addressWithType.begin(), addressWithType.begin() + ADDRESS_BYTE_LEN);
        address = BuildAddressString(addressVec);
        rawAddressType = addressWithType[ADDRESS_BYTE_LEN];
    }
    AddressInfo addressInfo;
    addressInfo.SetAddress(address);
    addressInfo.SetAddressType(addressType);
    addressInfo.SetRawAddressType(rawAddressType);
    ConvertAddressInfoToJs(env, deviceId, addressInfo);
    napi_set_named_property(env, result, "deviceId", deviceId);

    napi_value confirmationHash = nullptr;
    std::vector<uint8_t> confirmHash = data_.HasConfirmHash() ? data_.GetConfirmationHash() : std::vector<uint8_t> {};
    ConvertUint8VectorToJsUint8Array(env, confirmationHash, confirmHash);
    napi_set_named_property(env, result, "confirmationHash", confirmationHash);

    napi_value randomizerHash = nullptr;
    std::vector<uint8_t> randomHash = data_.HasRandomHash() ? data_.GetRandomizerHash() : std::vector<uint8_t> {};
    ConvertUint8VectorToJsUint8Array(env, randomizerHash, randomHash);
    napi_set_named_property(env, result, "randomizerHash", randomizerHash);

    napi_value deviceName = nullptr;
    std::string deviceNameStr = "";
    if (data_.HasDeviceName()) {
        deviceNameStr = data_.GetDeviceName();
    }
    napi_create_string_utf8(env, deviceNameStr.c_str(), deviceNameStr.size(), &deviceName);
    napi_set_named_property(env, result, "deviceName", deviceName);

    napi_value deviceRole = nullptr;
    napi_create_int32(env, static_cast<int32_t>(data_.GetDeviceRole()), &deviceRole);
    napi_set_named_property(env, result, "deviceRole", deviceRole);
    return result;
}

napi_value NapiNativeDiscoveryInfoResultArray::ToNapiValue(napi_env env) const
{
    CHECK_AND_RETURN_LOG_RET(remoteDevice_, nullptr, "remoteDevice is nullptr");

    napi_value array;
    napi_value result = nullptr;
    napi_create_array(env, &array);
    napi_create_object(env, &result);

    napi_value device = nullptr;
    std::string addr = remoteDevice_->GetDeviceAddr();
    napi_create_string_utf8(env, addr.c_str(), addr.size(), &device);
    napi_set_named_property(env, result, "deviceId", device);

    napi_value rssi = nullptr;
    napi_create_int32(env, rssi_, &rssi);
    napi_set_named_property(env, result, "rssi", rssi);

    napi_value deviceName = nullptr;
    napi_create_string_utf8(env, deviceName_.c_str(), deviceName_.size(), &deviceName);
    napi_set_named_property(env, result, "deviceName", deviceName);

    napi_value deviceClass = nullptr;
    napi_create_object(env, &deviceClass);
    ConvertDeviceClassToJS(env, deviceClass, deviceClass_);
    napi_set_named_property(env, result, "deviceClass", deviceClass);
    napi_set_element(env, array, 0, result);
    return array;
}

static std::string GetFormatPinCode(uint32_t pinType, uint32_t pinCode)
{
    std::string pinCodeStr = std::to_string(pinCode);
    if (pinType != PIN_TYPE_CONFIRM_PASSKEY && pinType != PIN_TYPE_NOTIFY_PASSKEY) {
        return pinCodeStr;
    }

    const uint32_t FORMAT_PINCODE_LENGTH = 6;
    while (pinCodeStr.length() < FORMAT_PINCODE_LENGTH) {
        pinCodeStr = "0" + pinCodeStr;
    }
    return pinCodeStr;
}

napi_value NapiNativePinRequiredParam::ToNapiValue(napi_env env) const
{
    CHECK_AND_RETURN_LOG_RET(pairConfirmInfo_, nullptr, "pairConfirmInfo is nullptr");

    napi_value result = nullptr;
    napi_create_object(env, &result);

    napi_value device = nullptr;
    std::string addr = pairConfirmInfo_->deviceAddr;
    napi_create_string_utf8(env, addr.c_str(), addr.size(), &device);
    napi_set_named_property(env, result, "deviceId", device);

    napi_value pinCode = nullptr;
    std::string pinCodeStr = GetFormatPinCode(pairConfirmInfo_->pinType, pairConfirmInfo_->number);
    napi_create_string_utf8(env, pinCodeStr.c_str(), pinCodeStr.size(), &pinCode);
    napi_set_named_property(env, result, "pinCode", pinCode);

    napi_value pinType = nullptr;
    napi_create_int32(env, pairConfirmInfo_->pinType, &pinType);
    napi_set_named_property(env, result, "pinType", pinType);
    return result;
}

napi_value NapiNativeBondStateParam::ToNapiValue(napi_env env) const
{
    napi_value result = nullptr;
    napi_create_object(env, &result);

    napi_value device = nullptr;
    napi_create_string_utf8(env, deviceAddr_.c_str(), deviceAddr_.size(), &device);
    napi_set_named_property(env, result, "deviceId", device);

    napi_value bondState = nullptr;
    napi_create_int32(env, bondStatus_, &bondState);
    napi_set_named_property(env, result, "state", bondState);

    napi_value unbondCause = nullptr;
    napi_create_int32(env, unbondCause_, &unbondCause);
    napi_set_named_property(env, result, "cause", unbondCause);
    return result;
}

napi_value NapiNativeStateChangeParam::ToNapiValue(napi_env env) const
{
    napi_value result = nullptr;
    napi_create_object(env, &result);
    ConnStateChangeParam stateChangeParam {
        deviceAddr_, connectState_, isDisconnected_, disconnectReason_, stateChangeCause_
    };
    ConvertStateChangeParamToJS(env, result, stateChangeParam);
    return result;
}

napi_value NapiNativeOppTransferInformation::ToNapiValue(napi_env env) const
{
    napi_value result = nullptr;
    napi_create_object(env, &result);

    ConvertOppTransferInformationToJS(env, result, information_);
    return result;
}

napi_value NapiNativeInt64::ToNapiValue(napi_env env) const
{
    napi_value value = nullptr;
    napi_status status = napi_create_int64(env, value_, &value);
    if (status != napi_ok) {
        HILOGE("napi_create_int64 failed");
    }
    return value;
}

napi_value NapiNativeBatteryInfo::ToNapiValue(napi_env env) const
{
    napi_value result = nullptr;
    napi_create_object(env, &result);

    napi_value value = nullptr;
    napi_create_string_utf8(env, batteryInfo_.deviceId_.c_str(), batteryInfo_.deviceId_.size(), &value);
    napi_set_named_property(env, result, "deviceId", value);
    napi_create_int32(env, batteryInfo_.batteryLevel_, &value);
    napi_set_named_property(env, result, "batteryLevel", value);
    napi_create_int32(env, batteryInfo_.leftEarBatteryLevel_, &value);
    napi_set_named_property(env, result, "leftEarBatteryLevel", value);
    napi_create_int32(env, static_cast<int32_t>(batteryInfo_.leftEarChargeState_), &value);
    napi_set_named_property(env, result, "leftEarChargeState", value);
    napi_create_int32(env, batteryInfo_.rightEarBatteryLevel_, &value);
    napi_set_named_property(env, result, "rightEarBatteryLevel", value);
    napi_create_int32(env, static_cast<int32_t>(batteryInfo_.rightEarChargeState_), &value);
    napi_set_named_property(env, result, "rightEarChargeState", value);
    napi_create_int32(env, batteryInfo_.boxBatteryLevel_, &value);
    napi_set_named_property(env, result, "boxBatteryLevel", value);
    napi_create_int32(env, static_cast<int32_t>(batteryInfo_.boxChargeState_), &value);
    napi_set_named_property(env, result, "boxChargeState", value);
    return result;
}

napi_value NapiNativeArrayBuffer::ToNapiValue(napi_env env) const
{
    napi_value object = nullptr;
    uint8_t* bufferData = nullptr;
    napi_create_arraybuffer(env, sppBuffer_.len_, (void**)&bufferData, &object);
    if (sppBuffer_.len_ <= 0) {
        HILOGE("bufferSize_ < 0, bufferSize_: %{public}zu", sppBuffer_.len_);
        return object;
    }
    if (memset_s(bufferData, sppBuffer_.len_, 0, sppBuffer_.len_) != EOK) {
        HILOGE("memset_s error");
    }
    if (memcpy_s(bufferData, sppBuffer_.len_, sppBuffer_.data_.data(), sppBuffer_.len_) != EOK) {
        HILOGE("memcpy_s error");
    }
    return object;
}

napi_value NapiNativeStringArray::ToNapiValue(napi_env env) const
{
    napi_value object = nullptr;
    napi_create_array(env, &object);
    ConvertStringVectorToJS(env, object, value_);
    return object;
}

napi_value NapiNativeHIDGetReportData::ToNapiValue(napi_env env) const
{
    napi_value result = nullptr;
    napi_create_object(env, &result);

    napi_value type = nullptr;
    napi_create_int32(env, type_, &type);
    napi_set_named_property(env, result, "type", type);
    napi_value id = nullptr;
    napi_create_int32(env, id_, &id);
    napi_set_named_property(env, result, "id", id);
    napi_value bufferSize = nullptr;
    napi_create_int32(env, bufferSize_, &bufferSize);
    napi_set_named_property(env, result, "bufferSize", bufferSize);
    return result;
}

napi_value NapiNativeHIDProtocolData::ToNapiValue(napi_env env) const
{
    napi_value result = nullptr;
    napi_create_object(env, &result);

    napi_value protocol = nullptr;
    napi_create_int32(env, protocol_, &protocol);
    napi_set_named_property(env, result, "protocol", protocol);
    return result;
}

napi_value NapiNativeHIDInterruptData::ToNapiValue(napi_env env) const
{
    napi_value result = nullptr;
    napi_create_object(env, &result);

    napi_value id = nullptr;
    napi_create_int32(env, id_, &id);
    napi_set_named_property(env, result, "id", id);
    size_t valueSize = data_.size();
    const uint8_t *valueData = data_.data();
    napi_value buffer = nullptr;
    uint8_t *bufferData = nullptr;
    napi_create_arraybuffer(env, valueSize, (void**)&bufferData, &buffer);
    if (valueSize > 0 && memcpy_s(bufferData, valueSize, valueData, valueSize) != EOK) {
        HILOGE("memcpy_s error");
    }
    napi_value data = nullptr;
    napi_create_typedarray(env, napi_uint8_array, valueSize, buffer, 0, &data);
    napi_set_named_property(env, result, "data", data);
    return result;
}

napi_value NapiNativeHIDSetReportData::ToNapiValue(napi_env env) const
{
    napi_value result = nullptr;
    napi_create_object(env, &result);
    napi_value type = nullptr;
    napi_create_int32(env, type_, &type);
    napi_set_named_property(env, result, "type", type);
    napi_value id = nullptr;
    napi_create_int32(env, id_, &id);
    napi_set_named_property(env, result, "id", id);

    size_t valueSize = data_.size();
    const uint8_t *valueData = data_.data();
    napi_value buffer = nullptr;
    uint8_t *bufferData = nullptr;
    napi_create_arraybuffer(env, valueSize, (void**)&bufferData, &buffer);
    if (valueSize > 0 && memcpy_s(bufferData, valueSize, valueData, valueSize) != EOK) {
        HILOGE("memcpy_s error");
    }
    napi_value data = nullptr;
    napi_create_typedarray(env, napi_uint8_array, valueSize, buffer, 0, &data);
    napi_set_named_property(env, result, "data", data);
    return result;
}

napi_value NapiNativeBlePhyInfo::ToNapiValue(napi_env env) const
{
    //协议栈的回调不返phyMode，因此不构造phyMode返回
    napi_value object = nullptr;
    napi_create_object(env, &object);

    napi_value value;
    napi_create_int32(env, txPhy_, &value);
    napi_set_named_property(env, object, "txPhy", value);

    napi_create_int32(env, rxPhy_, &value);
    napi_set_named_property(env, object, "rxPhy", value);
    return object;
}
}  // namespace Bluetooth
}  // namespace OHOS