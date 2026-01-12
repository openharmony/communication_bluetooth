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
#ifndef LOG_TAG
#define LOG_TAG "bt_napi_hid_device"
#endif

#include "bluetooth_hid_device.h"
#include "bluetooth_log.h"
#include "napi_async_work.h"
#include "napi_bluetooth_hid_device_observer.h"
#include "napi_bluetooth_error.h"
#include "napi_bluetooth_utils.h"
#include "napi_bluetooth_profile.h"
#include "hitrace_meter.h"
#include "napi_parser_utils.h"
#include "bluetooth_def.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_utils.h"
#include "napi_bluetooth_hid_device.h"
#include "hid_device_utils.h"

namespace OHOS {
namespace Bluetooth {
std::shared_ptr<NapiBluetoothHidDeviceObserver> NapiBluetoothHidDevice::observer_ =
    std::make_shared<NapiBluetoothHidDeviceObserver>();
thread_local napi_ref NapiBluetoothHidDevice::consRef_ = nullptr;

napi_value SubclassInit(napi_env env)
{
    napi_value subclass = nullptr;
    napi_create_object(env, &subclass);
    SetNamedPropertyByInteger(env, subclass, Subclass::SUBCLASS_UNCATEGORIZED, "SUBCLASS_UNCATEGORIZED");
    SetNamedPropertyByInteger(env, subclass, Subclass::SUBCLASS_JOYSTICK, "SUBCLASS_JOYSTICK");
    SetNamedPropertyByInteger(env, subclass, Subclass::SUBCLASS_GAMEPAD, "SUBCLASS_GAMEPAD");
    SetNamedPropertyByInteger(env, subclass, Subclass::SUBCLASS_REMOTE_CONTROL, "SUBCLASS_REMOTE_CONTROL");
    SetNamedPropertyByInteger(env, subclass, Subclass::SUBCLASS_SENSING_DEVICE, "SUBCLASS_SENSING_DEVICE");
    SetNamedPropertyByInteger(env, subclass, Subclass::SUBCLASS_DIGITIZER_TABLET, "SUBCLASS_DIGITIZER_TABLET");
    SetNamedPropertyByInteger(env, subclass, Subclass::SUBCLASS_CARD_READER, "SUBCLASS_CARD_READER");
    SetNamedPropertyByInteger(env, subclass, Subclass::SUBCLASS_KEYBOARD, "SUBCLASS_KEYBOARD");
    SetNamedPropertyByInteger(env, subclass, Subclass::SUBCLASS_MOUSE, "SUBCLASS_MOUSE");
    SetNamedPropertyByInteger(env, subclass, Subclass::SUBCLASS_COMBO, "SUBCLASS_COMBO");
    return subclass;
}

napi_value ReportTypeInit(napi_env env)
{
    napi_value reportType = nullptr;
    napi_create_object(env, &reportType);
    SetNamedPropertyByInteger(env, reportType, ReportType::REPORT_TYPE_INPUT, "REPORT_TYPE_INPUT");
    SetNamedPropertyByInteger(env, reportType, ReportType::REPORT_TYPE_OUTPUT, "REPORT_TYPE_OUTPUT");
    SetNamedPropertyByInteger(env, reportType, ReportType::REPORT_TYPE_FEATURE, "REPORT_TYPE_FEATURE");
    return reportType;
}

napi_value ServiceTypeInit(napi_env env)
{
    napi_value serviceType = nullptr;
    napi_create_object(env, &serviceType);
    SetNamedPropertyByInteger(env, serviceType, ServiceType::SERVICE_NO_TRAFFIC, "SERVICE_NO_TRAFFIC");
    SetNamedPropertyByInteger(env, serviceType, ServiceType::SERVICE_BEST_EFFORT, "SERVICE_BEST_EFFORT");
    SetNamedPropertyByInteger(env, serviceType, ServiceType::SERVICE_GUARANTEED, "SERVICE_GUARANTEED");
    return serviceType;
}

napi_value ErrorReasonInit(napi_env env)
{
    napi_value errorReason = nullptr;
    napi_create_object(env, &errorReason);
    SetNamedPropertyByInteger(env, errorReason, ErrorReason::RSP_SUCCESS, "RSP_SUCCESS");
    SetNamedPropertyByInteger(env, errorReason, ErrorReason::RSP_NOT_READY, "RSP_NOT_READY");
    SetNamedPropertyByInteger(env, errorReason, ErrorReason::RSP_INVALID_REP_ID, "RSP_INVALID_REP_ID");
    SetNamedPropertyByInteger(env, errorReason, ErrorReason::RSP_UNSUPPORTED_REQ, "RSP_UNSUPPORTED_REQ");
    SetNamedPropertyByInteger(env, errorReason, ErrorReason::INVALID_PARAM, "INVALID_PARAM");
    SetNamedPropertyByInteger(env, errorReason, ErrorReason::RSP_UNKNOWN, "RSP_UNKNOWN");
    return errorReason;
}

napi_value ProtocolTypeInit(napi_env env)
{
    napi_value protocolType = nullptr;
    napi_create_object(env, &protocolType);
    SetNamedPropertyByInteger(env, protocolType, ProtocolType::PROTOCOL_BOOT_MODE, "PROTOCOL_BOOT_MODE");
    SetNamedPropertyByInteger(env, protocolType, ProtocolType::PROTOCOL_REPORT_MODE, "PROTOCOL_REPORT_MODE");
    return protocolType;
}

void HidDevicePropertyInit(napi_env env, napi_value exports)
{
    napi_value subclassObj = SubclassInit(env);
    napi_value reportTypeObj = ReportTypeInit(env);
    napi_value errorReasonObj = ErrorReasonInit(env);
    napi_value serviceTypeObj = ServiceTypeInit(env);
    napi_value protocolTypeObj = ProtocolTypeInit(env);
    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("Subclass", subclassObj),
        DECLARE_NAPI_PROPERTY("ReportType", reportTypeObj),
        DECLARE_NAPI_PROPERTY("ErrorReason", errorReasonObj),
        DECLARE_NAPI_PROPERTY("ServiceType", serviceTypeObj),
        DECLARE_NAPI_PROPERTY("ProtocolType", protocolTypeObj),
    };
    HITRACE_METER_NAME(HITRACE_TAG_OHOS, "hid_device:napi_define_properties");
    napi_define_properties(env, exports, sizeof(exportFuncs)/sizeof(*exportFuncs), exportFuncs);
}

void NapiBluetoothHidDevice::DefineHidDeviceJSClass(napi_env env, napi_value exports)
{
    HidDevicePropertyInit(env, exports);
    napi_value constructor;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("registerHidDevice", RegisterHidDevice),
        DECLARE_NAPI_FUNCTION("unregisterHidDevice", UnregisterHidDevice),
        DECLARE_NAPI_FUNCTION("connect", Connect),
        DECLARE_NAPI_FUNCTION("disconnect", Disconnect),
        DECLARE_NAPI_FUNCTION("sendReport", SendReport),
        DECLARE_NAPI_FUNCTION("replyReport", ReplyReport),
        DECLARE_NAPI_FUNCTION("reportError", ReportError),

        DECLARE_NAPI_FUNCTION("getConnectedDevices", GetConnectedDevices),
        DECLARE_NAPI_FUNCTION("getConnectionState", GetConnectionState),
        DECLARE_NAPI_FUNCTION("setConnectionStrategy", SetConnectionStrategy),
        DECLARE_NAPI_FUNCTION("getConnectionStrategy", GetConnectionStrategy),

        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
        DECLARE_NAPI_FUNCTION("onGetReport", OnGetReport),
        DECLARE_NAPI_FUNCTION("onSetReport", OnSetReport),
        DECLARE_NAPI_FUNCTION("onInterruptDataReceived", OnInterruptDataReceived),
        DECLARE_NAPI_FUNCTION("onSetProtocol", OnSetProtocol),
        DECLARE_NAPI_FUNCTION("onVirtualCableUnplug", OnVirtualCableUnplug),
        DECLARE_NAPI_FUNCTION("onConnectionStateChange", OnConnectionStateChange),
        DECLARE_NAPI_FUNCTION("offGetReport", OffGetReport),
        DECLARE_NAPI_FUNCTION("offSetReport", OffSetReport),
        DECLARE_NAPI_FUNCTION("offInterruptDataReceived", OffInterruptDataReceived),
        DECLARE_NAPI_FUNCTION("offSetProtocol", OffSetProtocol),
        DECLARE_NAPI_FUNCTION("offVirtualCableUnplug", OffVirtualCableUnplug),
        DECLARE_NAPI_FUNCTION("offConnectionStateChange", OffConnectionStateChange),
    };

    napi_define_class(env, "NapiBluetoothHidDevice", NAPI_AUTO_LENGTH, HidDeviceConstructor, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &constructor);

    DefineCreateProfile(env, exports);
    napi_create_reference(env, constructor, 1, &consRef_);
}

napi_value NapiBluetoothHidDevice::DefineCreateProfile(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("createHidDeviceProfile", CreateHidDeviceProfile),
    };
    HITRACE_METER_NAME(HITRACE_TAG_OHOS, "hid_device:napi_define_properties");
    napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties);
    return exports;
}

napi_value NapiBluetoothHidDevice::CreateHidDeviceProfile(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    napi_value napiProfile;
    napi_value constructor = nullptr;
    napi_get_reference_value(env, consRef_, &constructor);
    napi_new_instance(env, constructor, 0, nullptr, &napiProfile);
    NapiProfile::SetProfile(env, ProfileId::PROFILE_HID_DEVICE, napiProfile);

    HidDevice *profile = HidDevice::GetProfile();
    profile->RegisterObserver(observer_);
    return napiProfile;
}

napi_value NapiBluetoothHidDevice::HidDeviceConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    return thisVar;
}

napi_value NapiBluetoothHidDevice::On(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.Register(env, info);
        NAPI_BT_ASSERT_ERR_NUM_RETURN(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiBluetoothHidDevice::Off(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.Deregister(env, info);
        NAPI_BT_ASSERT_ERR_NUM_RETURN(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiBluetoothHidDevice::OnGetReport(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.RegisterWithName(env, info,
            STR_BT_HID_DEVICE_OBSERVER_APP_GET_REPORT);
        NAPI_BT_ASSERT_ERR_NUM_RETURN(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiBluetoothHidDevice::OnSetReport(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.RegisterWithName(env, info,
            STR_BT_HID_DEVICE_OBSERVER_APP_SET_REPORT);
        NAPI_BT_ASSERT_ERR_NUM_RETURN(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiBluetoothHidDevice::OnInterruptDataReceived(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.RegisterWithName(env, info,
            STR_BT_HID_DEVICE_OBSERVER_INTERRUPT_DATA_RECEIVED);
        NAPI_BT_ASSERT_ERR_NUM_RETURN(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiBluetoothHidDevice::OnSetProtocol(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.RegisterWithName(env, info,
            STR_BT_HID_DEVICE_OBSERVER_SET_PROTOCOL);
        NAPI_BT_ASSERT_ERR_NUM_RETURN(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiBluetoothHidDevice::OnVirtualCableUnplug(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.RegisterWithName(env, info,
            STR_BT_HID_DEVICE_OBSERVER_VIRTUAL_CABLE_UNPLUG);
        NAPI_BT_ASSERT_ERR_NUM_RETURN(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiBluetoothHidDevice::OnConnectionStateChange(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.RegisterWithName(env, info,
            STR_BT_HID_DEVICE_CONNECT_STATE_CHANGE);
        NAPI_BT_ASSERT_ERR_NUM_RETURN(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiBluetoothHidDevice::OffGetReport(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.DeregisterWithName(env, info,
            STR_BT_HID_DEVICE_OBSERVER_APP_GET_REPORT);
        NAPI_BT_ASSERT_ERR_NUM_RETURN(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiBluetoothHidDevice::OffSetReport(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.DeregisterWithName(env, info,
            STR_BT_HID_DEVICE_OBSERVER_APP_SET_REPORT);
        NAPI_BT_ASSERT_ERR_NUM_RETURN(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiBluetoothHidDevice::OffInterruptDataReceived(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.DeregisterWithName(env, info,
            STR_BT_HID_DEVICE_OBSERVER_INTERRUPT_DATA_RECEIVED);
        NAPI_BT_ASSERT_ERR_NUM_RETURN(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiBluetoothHidDevice::OffSetProtocol(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.DeregisterWithName(env, info,
            STR_BT_HID_DEVICE_OBSERVER_SET_PROTOCOL);
        NAPI_BT_ASSERT_ERR_NUM_RETURN(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiBluetoothHidDevice::OffVirtualCableUnplug(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.DeregisterWithName(env, info,
            STR_BT_HID_DEVICE_OBSERVER_VIRTUAL_CABLE_UNPLUG);
        NAPI_BT_ASSERT_ERR_NUM_RETURN(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiBluetoothHidDevice::OffConnectionStateChange(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.DeregisterWithName(env, info,
            STR_BT_HID_DEVICE_CONNECT_STATE_CHANGE);
        NAPI_BT_ASSERT_ERR_NUM_RETURN(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiBluetoothHidDevice::GetConnectedDevices(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    napi_value ret = nullptr;
    if (napi_create_array(env, &ret) != napi_ok) {
        HILOGE("napi_create_array failed.");
    }
    napi_status status = CheckEmptyParam(env, info);
    NAPI_BT_ASSERT_ERR_NUM_RETURN(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    HidDevice *profile = HidDevice::GetProfile();
    std::vector<BluetoothRemoteDevice> devices;
    int errorCode = profile->GetConnectedDevices(devices);
    NAPI_BT_ASSERT_NUM_RETURN(env, errorCode == BT_NO_ERROR, errorCode, ret);

    std::vector<std::string> deviceVector;
    for (auto &device: devices) {
        deviceVector.push_back(device.GetDeviceAddr());
    }
    ConvertStringVectorToJS(env, ret, deviceVector);
    return ret;
}

napi_value NapiBluetoothHidDevice::GetConnectionState(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    int32_t profileState = ProfileConnectionState::STATE_DISCONNECTED;
    if (napi_create_int32(env, profileState, &result) != napi_ok) {
        HILOGE("napi_create_int32 failed.");
    }

    std::string remoteAddr {};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_NUM_RETURN(env, checkRet, BT_ERR_INVALID_PARAM, result);

    HidDevice *profile = HidDevice::GetProfile();
    BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
    int32_t state = static_cast<int32_t>(BTConnectState::DISCONNECTED);
    int32_t errorCode = profile->GetConnectionState(device, state);
    NAPI_BT_ASSERT_NUM_RETURN(env, errorCode == BT_NO_ERROR, errorCode, result);

    profileState = GetProfileConnectionState(state);
    if (napi_create_int32(env, profileState, &result) != napi_ok) {
        HILOGE("napi_create_int32 failed.");
    }
    return result;
}

napi_status NapiParseHidDeviceSDPSetting(napi_env env, napi_value object, BluetoothHidDeviceSdp &sdpSetting)
{
    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(env, object, {"name", "description", "provider", "subclass",
        "descriptors"}));
    std::string name {};
    std::string description {};
    std::string provider {};
    int subclass {};
    std::vector<uint8_t> descriptors {};
    NAPI_BT_CALL_RETURN(NapiParseObjectString(env, object, "name", name));
    if (!IsValidSDPSettingParam(name, MAX_NAME_SIZE)) {
        HILOGE("invalid SDPSetting name size");
        return napi_invalid_arg;
    }
    NAPI_BT_CALL_RETURN(NapiParseObjectString(env, object, "description", description));
    if (!IsValidSDPSettingParam(description, MAX_DESCRIPTION_SIZE)) {
        HILOGE("invalid SDPSetting description size");
        return napi_invalid_arg;
    }
    NAPI_BT_CALL_RETURN(NapiParseObjectString(env, object, "provider", provider));
    if (!IsValidSDPSettingParam(provider, MAX_PROVIDER_SIZE)) {
        HILOGE("invalid SDPSetting provider size");
        return napi_invalid_arg;
    }
    NAPI_BT_CALL_RETURN(NapiParseObjectInt32(env, object, "subclass", subclass));
    if (!IsValidSubclass(subclass)) {
        HILOGE("subclass is not support");
        return napi_invalid_arg;
    }
    bool exist = false;
    NAPI_BT_CALL_RETURN(NapiParseObjectUint8Array(env, object, "descriptors", exist, descriptors));
    if (!exist) {
        HILOGE("descriptors is not exist");
        return napi_invalid_arg;
    }
    size_t descriptorSize = descriptors.size();
    if (descriptorSize > MAX_DESCRIPTOR_SIZE) {
        HILOGE("invalid descriptors size");
        return napi_invalid_arg;
    }
    sdpSetting.name_ = name;
    sdpSetting.description_ = description;
    sdpSetting.provider_ = provider;
    sdpSetting.subclass_ = subclass;
    sdpSetting.sdpSettings_ = descriptors;
    return napi_ok;
}

napi_status NapiParseHidDeviceQosSetting(napi_env env, napi_value object, BluetoothHidDeviceQos &qosSetting)
{
    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(env, object, {"serviceType", "tokenRate",
        "tokenBucketSize", "peakBandwidth", "latency", "delayVariation"}));
    int serviceType = static_cast<int32_t>(ServiceType::SERVICE_BEST_EFFORT);
    int tokenRate = DEFAULT_TOKEN_RATE;
    int tokenBucketSize = DEFAULT_TOKEN_BUCKET_SIZE;
    int peakBandwidth = DEFAULT_PEAK_BANDWIDTH;
    int latency = DEFAULT_LATENCY;
    int delayVariation = DEFAULT_DELAY_VARIATION;

    bool exist = false;
    NAPI_BT_CALL_RETURN(NapiParseObjectInt32Optional(env, object, "serviceType", serviceType, exist));
    if (exist) {
        if (!IsValidServiceType(serviceType)) {
            HILOGE("Invalid serviceType");
            return napi_invalid_arg;
        }
    }
    NAPI_BT_CALL_RETURN(NapiParseObjectInt32Optional(env, object, "tokenRate", tokenRate, exist));
    NAPI_BT_CALL_RETURN(NapiParseObjectInt32Optional(env, object, "tokenBucketSize", tokenBucketSize, exist));
    NAPI_BT_CALL_RETURN(NapiParseObjectInt32Optional(env, object, "peakBandwidth", peakBandwidth, exist));
    NAPI_BT_CALL_RETURN(NapiParseObjectInt32Optional(env, object, "latency", latency, exist));
    NAPI_BT_CALL_RETURN(NapiParseObjectInt32Optional(env, object, "delayVariation", delayVariation, exist));

    qosSetting.serviceType_ = serviceType;
    qosSetting.tokenRate_ = tokenRate;
    qosSetting.tokenBucketSize_ = tokenBucketSize;
    qosSetting.peakBandwidth_ = peakBandwidth;
    qosSetting.latency_ = latency;
    qosSetting.delayVariation_ = delayVariation;
    return napi_ok;
}

static napi_status CheckRegisterParam(napi_env env, napi_callback_info info,
    BluetoothHidDeviceSdp &outsdpSetting, BluetoothHidDeviceQos &outinQos,
    BluetoothHidDeviceQos &outoutQos)
{
    size_t argc = ARGS_SIZE_FOUR;
    napi_value argv[ARGS_SIZE_FOUR] = {nullptr};
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_FOUR, "Requires 4 arguments.", napi_invalid_arg);
    NAPI_BT_CALL_RETURN(NapiParseHidDeviceSDPSetting(env, argv[PARAM0], outsdpSetting));
    NAPI_BT_CALL_RETURN(NapiParseHidDeviceQosSetting(env, argv[PARAM1], outinQos));
    NAPI_BT_CALL_RETURN(NapiParseHidDeviceQosSetting(env, argv[PARAM2], outoutQos));
    return napi_ok;
}

static napi_status CheckDeviceParam(napi_env env, napi_callback_info info, AddressInfo &addressInfo)
{
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[ARGS_SIZE_ONE] = {nullptr};
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_ONE, "Requires 1 argument", napi_invalid_arg);
    NAPI_BT_CALL_RETURN(ParseAddressInfoParam(env, argv[PARAM0], addressInfo));
    return napi_ok;
}

static napi_status GetHidDeviceCallback(napi_env env, napi_callback_info info, napi_value &callback)
{
    size_t argc = ARGS_SIZE_FOUR;
    napi_value argv[ARGS_SIZE_FOUR] = {nullptr};
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    callback = argv[PARAM3];
    NAPI_BT_CALL_RETURN(NapiIsFunction(env, callback));
    return napi_ok;
}

napi_value NapiBluetoothHidDevice::RegisterHidDevice(napi_env env, napi_callback_info info)
{
    BluetoothHidDeviceSdp sdpSetting;
    BluetoothHidDeviceQos inQos;
    BluetoothHidDeviceQos outQos;
    napi_value callback;
    auto status = CheckRegisterParam(env, info, sdpSetting, inQos, outQos);
    NAPI_BT_ASSERT_ERR_NUM_RETURN(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    status = GetHidDeviceCallback(env, info, callback);
    NAPI_BT_ASSERT_ERR_NUM_RETURN(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    HidDevice *profile = HidDevice::GetProfile();
    int32_t errorCode = profile->RegisterHidDevice(sdpSetting, inQos, outQos);
    HILOGI("errorCode:%{public}s", GetErrorCode(errorCode).c_str());
    NAPI_BT_ASSERT_ERR_NUM_RETURN(env, errorCode == BT_NO_ERROR, errorCode);
    if (observer_) {
        observer_->eventSubscribe_.RegisterCallback(
            env, STR_BT_HID_DEVICE_OBSERVER_APP_STATUS_CHANGE, callback);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiBluetoothHidDevice::UnregisterHidDevice(napi_env env, napi_callback_info info)
{
    napi_status status = CheckEmptyParam(env, info);
    NAPI_BT_ASSERT_ERR_NUM_RETURN(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    HidDevice *profile = HidDevice::GetProfile();
    if (observer_) {
        observer_->eventSubscribe_.DeregisterAllCallback(
            STR_BT_HID_DEVICE_OBSERVER_APP_STATUS_CHANGE);
    }
    int32_t errorCode = profile->UnregisterHidDevice();
    NAPI_BT_ASSERT_ERR_NUM_RETURN(env, errorCode == BT_NO_ERROR, errorCode);
    return NapiGetUndefinedRet(env);
}

napi_value NapiBluetoothHidDevice::Connect(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    AddressInfo addressInfo;
    auto checkRet = CheckDeviceParam(env, info, addressInfo);
    NAPI_BT_ASSERT_ERR_NUM_RETURN(env, checkRet == napi_ok, BT_ERR_INVALID_PARAM);
    HidDevice *profile = HidDevice::GetProfile();
    int32_t errorCode = profile->Connect(addressInfo);
    NAPI_BT_ASSERT_ERR_NUM_RETURN(env, errorCode == BT_NO_ERROR, errorCode);
    return NapiGetUndefinedRet(env);
}

napi_value NapiBluetoothHidDevice::Disconnect(napi_env env, napi_callback_info info)
{
    HILOGI("Disconnect called");
    napi_status status = CheckEmptyParam(env, info);
    NAPI_BT_ASSERT_ERR_NUM_RETURN(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    HidDevice *profile = HidDevice::GetProfile();
    int32_t errorCode = profile->Disconnect();
    HILOGI("errorCode:%{public}s", GetErrorCode(errorCode).c_str());
    NAPI_BT_ASSERT_ERR_NUM_RETURN(env, errorCode == BT_NO_ERROR, errorCode);
    return NapiGetUndefinedRet(env);
}

static napi_status CheckSendReport(napi_env env, napi_callback_info info,
    int &outid, std::vector<uint8_t> &outdata)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_TWO, "Requires 2 arguments.", napi_invalid_arg);
    NAPI_BT_RETURN_IF(!ParseInt32(env, outid, argv[PARAM0]), "ParseInt32 failed", napi_invalid_arg);
    NAPI_BT_RETURN_IF(ParseUint8ArrayParam(env, argv[PARAM1], outdata) != napi_ok,
        "ParseUint8Array failed", napi_invalid_arg);
    return napi_ok;
}

napi_value NapiBluetoothHidDevice::SendReport(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    int id {};
    std::vector<uint8_t> data {};
    auto status = CheckSendReport(env, info, id, data);
    NAPI_BT_ASSERT_ERR_NUM_RETURN(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    HidDevice *profile = HidDevice::GetProfile();
    int32_t errorCode = profile->SendReport(id, data);
    HILOGI("errorCode:%{public}s", GetErrorCode(errorCode).c_str());
    NAPI_BT_ASSERT_ERR_NUM_RETURN(env, errorCode == BT_NO_ERROR, errorCode);
    return NapiGetUndefinedRet(env);
}

static napi_status CheckReplyReport(napi_env env, napi_callback_info info,
    ReportType &outtype, int &outid, std::vector<uint8_t> &outdata)
{
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {nullptr};
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_THREE, "Requires 3 arguments.", napi_invalid_arg);
    int type {};
    NAPI_BT_RETURN_IF(!ParseInt32(env, type, argv[PARAM0]), "ParseInt32 failed", napi_invalid_arg);
    if (!IsValidReportType(type)) {
        HILOGE("type is not support");
        return napi_invalid_arg;
    }
    NAPI_BT_RETURN_IF(!ParseInt32(env, outid, argv[PARAM1]), "ParseInt32 failed", napi_invalid_arg);
    NAPI_BT_RETURN_IF(ParseUint8ArrayParam(env, argv[PARAM2], outdata) != napi_ok,
        "ParseUint8Array failed", napi_invalid_arg);
    outtype = static_cast<ReportType>(type);
    return napi_ok;
}

napi_value NapiBluetoothHidDevice::ReplyReport(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    ReportType type = ReportType::REPORT_TYPE_INPUT;
    int id {};
    std::vector<uint8_t> data {};
    auto status = CheckReplyReport(env, info, type, id, data);
    NAPI_BT_ASSERT_ERR_NUM_RETURN(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    HidDevice *profile = HidDevice::GetProfile();
    int32_t errorCode = profile->ReplyReport(type, id, data);
    HILOGI("errorCode:%{public}s", GetErrorCode(errorCode).c_str());
    NAPI_BT_ASSERT_ERR_NUM_RETURN(env, errorCode == BT_NO_ERROR, errorCode);
    return NapiGetUndefinedRet(env);
}

static napi_status CheckReportError(napi_env env, napi_callback_info info, ErrorReason &outtype)
{
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[ARGS_SIZE_ONE] = {nullptr};
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_ONE, "Requires 1 argument.", napi_invalid_arg);

    int errorReason = static_cast<int>(ErrorReason::RSP_UNKNOWN);
    NAPI_BT_RETURN_IF(!ParseInt32(env, errorReason, argv[PARAM0]), "ParseInt32 failed", napi_invalid_arg);

    if (!IsValidErrorReason(errorReason)) {
        HILOGE("errorReason is not support");
        return napi_invalid_arg;
    }
    outtype = static_cast<ErrorReason>(errorReason);
    return napi_ok;
}

napi_value NapiBluetoothHidDevice::ReportError(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    ErrorReason errorReason = ErrorReason::RSP_UNKNOWN;
    auto status = CheckReportError(env, info, errorReason);
    NAPI_BT_ASSERT_ERR_NUM_RETURN(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    HidDevice *profile = HidDevice::GetProfile();
    int32_t errorCode = profile->ReportError(errorReason);
    NAPI_BT_ASSERT_ERR_NUM_RETURN(env, errorCode == BT_NO_ERROR, errorCode);
    return NapiGetUndefinedRet(env);
}

napi_value NapiBluetoothHidDevice::SetConnectionStrategy(napi_env env, napi_callback_info info)
{
    HILOGD("start");
    std::string remoteAddr {};
    int32_t strategy = static_cast<int>(BTStrategyType::CONNECTION_UNKNOWN);
    auto status = CheckSetConnectStrategyParam(env, info, remoteAddr, strategy);
    NAPI_BT_ASSERT_ERR_NUM_RETURN(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr, strategy]() {
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        HidDevice *profile = HidDevice::GetProfile();
        int32_t err = profile->SetConnectStrategy(remoteDevice, strategy);
        HILOGI("err: %{public}d", err);
        return NapiAsyncWorkRet(err);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_ERR_NUM_RETURN(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiBluetoothHidDevice::GetConnectionStrategy(napi_env env, napi_callback_info info)
{
    HILOGD("start");
    std::string remoteAddr {};
    auto status = CheckDeviceAddressParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_ERR_NUM_RETURN(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr]() {
        int strategy = 0;
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        HidDevice *profile = HidDevice::GetProfile();
        int32_t err = profile->GetConnectStrategy(remoteDevice, strategy);
        HILOGI("err: %{public}d, deviceName: %{public}d", err, strategy);
        auto object = std::make_shared<NapiNativeInt>(strategy);
        return NapiAsyncWorkRet(err, object);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_ERR_NUM_RETURN(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}
} // namespace Bluetooth
} // namespace OHOS