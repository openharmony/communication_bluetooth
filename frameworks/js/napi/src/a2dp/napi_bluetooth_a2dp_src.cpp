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
#define LOG_TAG "bt_napi_a2dp_src"
#endif

#include "bluetooth_a2dp_src.h"
#include "bluetooth_avrcp_tg.h"
#include "bluetooth_errorcode.h"
#include "napi_async_work.h"
#include "napi_bluetooth_profile.h"
#include "napi_bluetooth_a2dp_src.h"
#include "napi_bluetooth_event.h"
#include "napi_bluetooth_error.h"
#include "napi_bluetooth_host.h"
#include "napi_bluetooth_utils.h"
#include "../parser/napi_parser_utils.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace Bluetooth {
using namespace std;

std::shared_ptr<NapiA2dpSourceObserver> NapiA2dpSource::observer_ = std::make_shared<NapiA2dpSourceObserver>();
bool NapiA2dpSource::isRegistered_ = false;
thread_local napi_ref g_napiProfile = nullptr;

const static std::map<int32_t, int32_t> g_codecTypeMap = {
    {CODEC_TYPE_SBC, A2DP_CODEC_TYPE_SBC_USER},
    {CODEC_TYPE_AAC, A2DP_CODEC_TYPE_AAC_USER},
    {CODEC_TYPE_L2HC, A2DP_CODEC_TYPE_L2HCV2_USER},
    {CODEC_TYPE_L2HCST, A2DP_CODEC_TYPE_L2HCST_USER},
    {CODEC_TYPE_LDAC, A2DP_CODEC_TYPE_LDAC_USER},
    {CODEC_TYPE_INVALID, A2DP_CODEC_TYPE_NONA2DP_USER},
};

const static std::map<int32_t, int32_t> g_codecBitsPerSampleMap = {
    {CODEC_BITS_PER_SAMPLE_NONE, A2DP_SAMPLE_BITS_NONE_USER},
    {CODEC_BITS_PER_SAMPLE_16, A2DP_SAMPLE_BITS_16_USER},
    {CODEC_BITS_PER_SAMPLE_24, A2DP_SAMPLE_BITS_24_USER},
    {CODEC_BITS_PER_SAMPLE_32, A2DP_SAMPLE_BITS_32_USER},
};

const static std::map<int32_t, int32_t> g_codecChannelModeMap = {
    {CODEC_CHANNEL_MODE_NONE, A2DP_CHANNEL_MODE_NONE_USER},
    {CODEC_CHANNEL_MODE_MONO, A2DP_SBC_CHANNEL_MODE_MONO_USER},
    {CODEC_CHANNEL_MODE_STEREO, A2DP_SBC_CHANNEL_MODE_STEREO_USER},
};

const static std::map<int32_t, int32_t> g_codecSampleRateMap = {
    {CODEC_SAMPLE_RATE_NONE, A2DP_SAMPLE_RATE_NONE_USER},
    {CODEC_SAMPLE_RATE_44100, A2DP_SBC_SAMPLE_RATE_44100_USER},
    {CODEC_SAMPLE_RATE_48000, A2DP_L2HCV2_SAMPLE_RATE_48000_USER},
    {CODEC_SAMPLE_RATE_88200, A2DP_SAMPLE_RATE_NONE_USER},
    {CODEC_SAMPLE_RATE_96000, A2DP_L2HCV2_SAMPLE_RATE_96000_USER},
    {CODEC_SAMPLE_RATE_176400, A2DP_SAMPLE_RATE_NONE_USER},
    {CODEC_SAMPLE_RATE_192000, A2DP_SAMPLE_RATE_NONE_USER},
};

const static std::map<int32_t, CodecType> g_a2dpCodecTypeMap = {
    {A2DP_CODEC_TYPE_SBC_USER, CODEC_TYPE_SBC},
    {A2DP_CODEC_TYPE_AAC_USER, CODEC_TYPE_AAC},
    {A2DP_CODEC_TYPE_L2HCV2_USER, CODEC_TYPE_L2HC},
    {A2DP_CODEC_TYPE_L2HCST_USER, CODEC_TYPE_L2HCST},
    {A2DP_CODEC_TYPE_LDAC_USER, CODEC_TYPE_LDAC},
    {A2DP_CODEC_TYPE_NONA2DP_USER, CODEC_TYPE_INVALID},
};

const static std::map<int32_t, CodecBitsPerSample> g_a2dpCodecBitsPerSampleMap = {
    {A2DP_SAMPLE_BITS_NONE_USER, CODEC_BITS_PER_SAMPLE_NONE},
    {A2DP_SAMPLE_BITS_16_USER, CODEC_BITS_PER_SAMPLE_16},
    {A2DP_SAMPLE_BITS_24_USER, CODEC_BITS_PER_SAMPLE_24},
    {A2DP_SAMPLE_BITS_32_USER, CODEC_BITS_PER_SAMPLE_32},
};

const static std::map<int32_t, CodecChannelMode> g_a2dpCodecChannelModeMap = {
    {A2DP_CHANNEL_MODE_NONE_USER, CODEC_CHANNEL_MODE_NONE},
    {A2DP_SBC_CHANNEL_MODE_MONO_USER, CODEC_CHANNEL_MODE_MONO},
    {A2DP_SBC_CHANNEL_MODE_STEREO_USER, CODEC_CHANNEL_MODE_STEREO},
};

const static std::map<int32_t, CodecSampleRate> g_a2dpCodecSampleRateMap = {
    {A2DP_SAMPLE_RATE_NONE_USER, CODEC_SAMPLE_RATE_NONE},
    {A2DP_SBC_SAMPLE_RATE_44100_USER, CODEC_SAMPLE_RATE_44100},
    {A2DP_SBC_SAMPLE_RATE_48000_USER, CODEC_SAMPLE_RATE_48000},
    {A2DP_L2HCV2_SAMPLE_RATE_48000_USER, CODEC_SAMPLE_RATE_48000},
    {A2DP_L2HCV2_SAMPLE_RATE_96000_USER, CODEC_SAMPLE_RATE_96000},
};

napi_value NapiA2dpSource::DefineA2dpSourceJSClass(napi_env env, napi_value exports)
{
    A2dpPropertyValueInit(env, exports);
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
        DECLARE_NAPI_FUNCTION("connect", Connect),
        DECLARE_NAPI_FUNCTION("disconnect", Disconnect),
        DECLARE_NAPI_FUNCTION("getPlayingState", GetPlayingState),
#ifndef BLUETOOTH_API_SINCE_10
        DECLARE_NAPI_FUNCTION("getConnectionDevices", GetConnectionDevices),
        DECLARE_NAPI_FUNCTION("getDeviceState", GetDeviceState),
#endif
#ifdef BLUETOOTH_API_SINCE_10
        DECLARE_NAPI_FUNCTION("setConnectionStrategy", SetConnectionStrategy),
        DECLARE_NAPI_FUNCTION("getConnectionStrategy", GetConnectionStrategy),
        DECLARE_NAPI_FUNCTION("getConnectionState", GetConnectionState),
        DECLARE_NAPI_FUNCTION("getConnectedDevices", GetConnectedDevices),
        DECLARE_NAPI_FUNCTION("isAbsoluteVolumeSupported", IsAbsoluteVolumeSupported),
        DECLARE_NAPI_FUNCTION("isAbsoluteVolumeEnabled", IsAbsoluteVolumeEnabled),
        DECLARE_NAPI_FUNCTION("enableAbsoluteVolume", EnableAbsoluteVolume),
        DECLARE_NAPI_FUNCTION("disableAbsoluteVolume", DisableAbsoluteVolume),
        DECLARE_NAPI_FUNCTION("setCurrentCodecInfo", SetCurrentCodecInfo),
        DECLARE_NAPI_FUNCTION("getCurrentCodecInfo", GetCurrentCodecInfo),
        DECLARE_NAPI_FUNCTION("enableAutoPlay", EnableAutoPlay),
        DECLARE_NAPI_FUNCTION("disableAutoPlay", DisableAutoPlay),
        DECLARE_NAPI_FUNCTION("getAutoPlayDisabledDuration", GetAutoPlayDisabledDuration),
#endif
    };

    napi_value constructor;
    napi_define_class(env,
        "A2dpSource",
        NAPI_AUTO_LENGTH,
        A2dpSourceConstructor,
        nullptr,
        sizeof(properties) / sizeof(properties[0]),
        properties,
        &constructor);
#ifdef BLUETOOTH_API_SINCE_10
    DefineCreateProfile(env, exports);
    napi_create_reference(env, constructor, 1, &g_napiProfile);
#else
    napi_value napiProfile;
    napi_new_instance(env, constructor, 0, nullptr, &napiProfile);
    NapiProfile::SetProfile(env, ProfileId::PROFILE_A2DP_SOURCE, napiProfile);
#endif
    return exports;
}

napi_value NapiA2dpSource::A2dpSourceConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    return thisVar;
}

napi_value NapiA2dpSource::On(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.Register(env, info);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }

    if (!isRegistered_) {
        A2dpSource *profile = A2dpSource::GetProfile();
        profile->RegisterObserver(observer_);
        isRegistered_ = true;
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiA2dpSource::Off(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.Deregister(env, info);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiA2dpSource::GetPlayingState(napi_env env, napi_callback_info info)
{
    HILOGD("start");
    int state = PlayingState::STATE_NOT_PLAYING;
    napi_value ret = nullptr;
    napi_create_int32(env, state, &ret);

    std::string remoteAddr{};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN(env, checkRet, BT_ERR_INVALID_PARAM, ret);

    int transport = BT_TRANSPORT_BREDR;
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr, transport);
    A2dpSource *profile = A2dpSource::GetProfile();
    int32_t errorCode = profile->GetPlayingState(remoteDevice, state);
    HILOGI("errorCode: %{public}d", errorCode);
    NAPI_BT_ASSERT_RETURN(env, (errorCode == BT_NO_ERROR), errorCode, ret);

    return NapiGetInt32Ret(env, state);
}

napi_value NapiA2dpSource::Connect(napi_env env, napi_callback_info info)
{
    HILOGD("start");
    std::string remoteAddr{};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_FALSE(env, checkRet, BT_ERR_INVALID_PARAM);

    int transport = BT_TRANSPORT_BREDR;
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr, transport);
    A2dpSource *profile = A2dpSource::GetProfile();
    int32_t ret = profile->Connect(remoteDevice);
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);

    return NapiGetBooleanTrue(env);
}

napi_value NapiA2dpSource::Disconnect(napi_env env, napi_callback_info info)
{
    HILOGD("start");
    std::string remoteAddr{};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_FALSE(env, checkRet, BT_ERR_INVALID_PARAM);

    int transport = BT_TRANSPORT_BREDR;
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr, transport);
    A2dpSource *profile = A2dpSource::GetProfile();
    int32_t ret = profile->Disconnect(remoteDevice);
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);

    return NapiGetBooleanTrue(env);
}

napi_value PlayingStateInit(napi_env env)
{
    napi_value playingState = nullptr;
    napi_create_object(env, &playingState);
    SetNamedPropertyByInteger(env, playingState, PlayingState::STATE_NOT_PLAYING, "STATE_NOT_PLAYING");
    SetNamedPropertyByInteger(env, playingState, PlayingState::STATE_PLAYING, "STATE_PLAYING");
    return playingState;
}

napi_value CodecTypeInit(napi_env env)
{
    napi_value codecType = nullptr;
    napi_create_object(env, &codecType);
    SetNamedPropertyByInteger(env, codecType, CodecType::CODEC_TYPE_SBC, "CODEC_TYPE_SBC");
    SetNamedPropertyByInteger(env, codecType, CodecType::CODEC_TYPE_AAC, "CODEC_TYPE_AAC");
    SetNamedPropertyByInteger(env, codecType, CodecType::CODEC_TYPE_L2HC, "CODEC_TYPE_L2HC");
    SetNamedPropertyByInteger(env, codecType, CodecType::CODEC_TYPE_L2HCST, "CODEC_TYPE_L2HCST");
    SetNamedPropertyByInteger(env, codecType, CodecType::CODEC_TYPE_LDAC, "CODEC_TYPE_LDAC");
    SetNamedPropertyByInteger(env, codecType, CodecType::CODEC_TYPE_INVALID, "CODEC_TYPE_INVALID");
    return codecType;
}

napi_value CodecBitsPerSampleInit(napi_env env)
{
    napi_value codecBitsPerSample = nullptr;
    napi_create_object(env, &codecBitsPerSample);
    SetNamedPropertyByInteger(env, codecBitsPerSample,
        CodecBitsPerSample::CODEC_BITS_PER_SAMPLE_NONE, "CODEC_BITS_PER_SAMPLE_NONE");
    SetNamedPropertyByInteger(env, codecBitsPerSample,
        CodecBitsPerSample::CODEC_BITS_PER_SAMPLE_16, "CODEC_BITS_PER_SAMPLE_16");
    SetNamedPropertyByInteger(env, codecBitsPerSample,
        CodecBitsPerSample::CODEC_BITS_PER_SAMPLE_24, "CODEC_BITS_PER_SAMPLE_24");
    SetNamedPropertyByInteger(env, codecBitsPerSample,
        CodecBitsPerSample::CODEC_BITS_PER_SAMPLE_32, "CODEC_BITS_PER_SAMPLE_32");
    return codecBitsPerSample;
}

napi_value CodecChannelModeInit(napi_env env)
{
    napi_value codecChannelMode = nullptr;
    napi_create_object(env, &codecChannelMode);
    SetNamedPropertyByInteger(env, codecChannelMode,
        CodecChannelMode::CODEC_CHANNEL_MODE_NONE, "CODEC_CHANNEL_MODE_NONE");
    SetNamedPropertyByInteger(env, codecChannelMode,
        CodecChannelMode::CODEC_CHANNEL_MODE_MONO, "CODEC_CHANNEL_MODE_MONO");
    SetNamedPropertyByInteger(env, codecChannelMode,
        CodecChannelMode::CODEC_CHANNEL_MODE_STEREO, "CODEC_CHANNEL_MODE_STEREO");
    return codecChannelMode;
}

napi_value CodecSampleRateInit(napi_env env)
{
    napi_value codecSampleRate = nullptr;
    napi_create_object(env, &codecSampleRate);
    SetNamedPropertyByInteger(env, codecSampleRate,
        CodecSampleRate::CODEC_SAMPLE_RATE_NONE, "CODEC_SAMPLE_RATE_NONE");
    SetNamedPropertyByInteger(env, codecSampleRate,
        CodecSampleRate::CODEC_SAMPLE_RATE_44100, "CODEC_SAMPLE_RATE_44100");
    SetNamedPropertyByInteger(env, codecSampleRate,
        CodecSampleRate::CODEC_SAMPLE_RATE_48000, "CODEC_SAMPLE_RATE_48000");
    SetNamedPropertyByInteger(env, codecSampleRate,
        CodecSampleRate::CODEC_SAMPLE_RATE_88200, "CODEC_SAMPLE_RATE_88200");
    SetNamedPropertyByInteger(env, codecSampleRate,
        CodecSampleRate::CODEC_SAMPLE_RATE_96000, "CODEC_SAMPLE_RATE_96000");
    SetNamedPropertyByInteger(env, codecSampleRate,
        CodecSampleRate::CODEC_SAMPLE_RATE_176400, "CODEC_SAMPLE_RATE_176400");
    SetNamedPropertyByInteger(env, codecSampleRate,
        CodecSampleRate::CODEC_SAMPLE_RATE_192000, "CODEC_SAMPLE_RATE_192000");
    return codecSampleRate;
}

napi_value NapiA2dpSource::A2dpPropertyValueInit(napi_env env, napi_value exports)
{
    napi_value playingStateObj = PlayingStateInit(env);
    napi_value codecTypeObj = CodecTypeInit(env);
    napi_value codecBitsPerSampleObj = CodecBitsPerSampleInit(env);
    napi_value codecChannelModeObj = CodecChannelModeInit(env);
    napi_value codecSampleRateObj = CodecSampleRateInit(env);
    napi_property_descriptor exportProps[] = {
        DECLARE_NAPI_PROPERTY("PlayingState", playingStateObj),
        DECLARE_NAPI_PROPERTY("CodecType", codecTypeObj),
        DECLARE_NAPI_PROPERTY("CodecBitsPerSample", codecBitsPerSampleObj),
        DECLARE_NAPI_PROPERTY("CodecChannelMode", codecChannelModeObj),
        DECLARE_NAPI_PROPERTY("CodecSampleRate", codecSampleRateObj),
    };
    HITRACE_METER_NAME(HITRACE_TAG_OHOS, "a2dp_src:napi_define_properties");
    napi_define_properties(env, exports, sizeof(exportProps) / sizeof(*exportProps), exportProps);
    return exports;
}

napi_value NapiA2dpSource::GetConnectionDevices(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    napi_value ret = nullptr;
    napi_create_array(env, &ret);
    A2dpSource *profile = A2dpSource::GetProfile();
    vector<int> states;
    states.push_back(1);
    vector<BluetoothRemoteDevice> devices;
    int errorCode = profile->GetDevicesByStates(states, devices);
    NAPI_BT_ASSERT_RETURN(env, (errorCode == BT_NO_ERROR), errorCode, ret);

    vector<string> deviceVector;
    for (auto &device : devices) {
        deviceVector.push_back(device.GetDeviceAddr());
    }
    ConvertStringVectorToJS(env, ret, deviceVector);
    return ret;
}

napi_value NapiA2dpSource::GetDeviceState(napi_env env, napi_callback_info info)
{
    HILOGD("enter");

    size_t expectedArgsCount = ARGS_SIZE_ONE;
    size_t argc = expectedArgsCount;
    napi_value argv[ARGS_SIZE_ONE] = {0};
    napi_value thisVar = nullptr;

    napi_value ret = nullptr;
    napi_get_undefined(env, &ret);

    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != expectedArgsCount) {
        HILOGE("Requires 1 argument.");
        return ret;
    }
    string deviceId;
    if (!ParseString(env, deviceId, argv[PARAM0])) {
        HILOGE("string expected.");
        return ret;
    }

    A2dpSource *profile = A2dpSource::GetProfile();
    BluetoothRemoteDevice device(deviceId, 1);

    int32_t profileState = ProfileConnectionState::STATE_DISCONNECTED;
    if (napi_create_int32(env, profileState, &ret) != napi_ok) {
        HILOGE("napi_create_int32 failed.");
    }

    int btConnectState = static_cast<int32_t>(BTConnectState::DISCONNECTED);
    int errorCode = profile->GetDeviceState(device, btConnectState);
    NAPI_BT_ASSERT_RETURN(env, (errorCode == BT_NO_ERROR), errorCode, ret);

    napi_value result = nullptr;
    int status = GetProfileConnectionState(btConnectState);
    napi_create_int32(env, status, &result);
    HILOGD("status: %{public}d", status);
    return result;
}

#ifdef BLUETOOTH_API_SINCE_10
napi_value NapiA2dpSource::DefineCreateProfile(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("createA2dpSrcProfile", CreateA2dpSrcProfile),
    };
    HITRACE_METER_NAME(HITRACE_TAG_OHOS, "a2dp_src:napi_define_properties");
    napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties);
    return exports;
}

napi_value NapiA2dpSource::CreateA2dpSrcProfile(napi_env env, napi_callback_info info)
{
    napi_value profile;
    napi_value constructor = nullptr;
    napi_get_reference_value(env, g_napiProfile, &constructor);
    napi_new_instance(env, constructor, 0, nullptr, &profile);
    return profile;
}

napi_value NapiA2dpSource::SetConnectionStrategy(napi_env env, napi_callback_info info)
{
    HILOGD("start");
    std::string remoteAddr{};
    int32_t strategy = 0;
    auto status = CheckSetConnectStrategyParam(env, info, remoteAddr, strategy);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    int transport = BT_TRANSPORT_BREDR;
    auto func = [remoteAddr, transport, strategy]() {
        BluetoothRemoteDevice remoteDevice(remoteAddr, transport);
        A2dpSource *profile = A2dpSource::GetProfile();
        int32_t err = profile->SetConnectStrategy(remoteDevice, strategy);
        HILOGI("err: %{public}d", err);
        return NapiAsyncWorkRet(err);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiA2dpSource::GetConnectionStrategy(napi_env env, napi_callback_info info)
{
    HILOGD("start");
    std::string remoteAddr{};
    auto status = CheckDeviceAddressParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    int transport = BT_TRANSPORT_BREDR;
    auto func = [remoteAddr, transport]() {
        int strategy = 0;
        BluetoothRemoteDevice remoteDevice(remoteAddr, transport);
        A2dpSource *profile = A2dpSource::GetProfile();
        int32_t err = profile->GetConnectStrategy(remoteDevice, strategy);
        HILOGD("err: %{public}d, deviceName: %{public}d", err, strategy);
        auto object = std::make_shared<NapiNativeInt>(strategy);
        return NapiAsyncWorkRet(err, object);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiA2dpSource::GetConnectionState(napi_env env, napi_callback_info info)
{
    return GetDeviceState(env, info);
}
napi_value NapiA2dpSource::GetConnectedDevices(napi_env env, napi_callback_info info)
{
    return GetConnectionDevices(env, info);
}

napi_value NapiA2dpSource::IsAbsoluteVolumeSupported(napi_env env, napi_callback_info info)
{
    HILOGD("start");
    std::string remoteAddr{};
    auto status = CheckDeviceAddressParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr]() {
        int32_t ability = DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT;
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t err = AvrcpTarget::GetProfile()->GetDeviceAbsVolumeAbility(remoteDevice, ability);
        if (ability == DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT) {
            return NapiAsyncWorkRet(err, std::make_shared<NapiNativeBool>(false));
        }
        return NapiAsyncWorkRet(err, std::make_shared<NapiNativeBool>(true));
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiA2dpSource::IsAbsoluteVolumeEnabled(napi_env env, napi_callback_info info)
{
    HILOGD("start");
    std::string remoteAddr{};
    auto status = CheckDeviceAddressParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr]() {
        int32_t ability = DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT;
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t err = AvrcpTarget::GetProfile()->GetDeviceAbsVolumeAbility(remoteDevice, ability);
        if (ability == DeviceAbsVolumeAbility::DEVICE_ABSVOL_OPEN) {
            return NapiAsyncWorkRet(err, std::make_shared<NapiNativeBool>(true));
        }
        return NapiAsyncWorkRet(err, std::make_shared<NapiNativeBool>(false));
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiA2dpSource::EnableAbsoluteVolume(napi_env env, napi_callback_info info)
{
    HILOGD("start");
    std::string remoteAddr{};
    auto status = CheckDeviceAddressParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr]() {
        int32_t ability = DeviceAbsVolumeAbility::DEVICE_ABSVOL_OPEN;
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t err = AvrcpTarget::GetProfile()->SetDeviceAbsVolumeAbility(remoteDevice, ability);
        return NapiAsyncWorkRet(err);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiA2dpSource::DisableAbsoluteVolume(napi_env env, napi_callback_info info)
{
    HILOGD("start");
    std::string remoteAddr{};
    auto status = CheckDeviceAddressParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr]() {
        int32_t ability = DeviceAbsVolumeAbility::DEVICE_ABSVOL_CLOSE;
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t err = AvrcpTarget::GetProfile()->SetDeviceAbsVolumeAbility(remoteDevice, ability);
        return NapiAsyncWorkRet(err);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

static void ConvertCodecType(A2dpCodecInfo &a2dpCodecInfo, int32_t codecType)
{
    auto iter = g_codecTypeMap.find(codecType);
    if (iter != g_codecTypeMap.end()) {
        a2dpCodecInfo.codecType = iter->second;
    }
}

static void ConvertCodecBitsPerSample(A2dpCodecInfo &a2dpCodecInfo, int32_t codecBitsPerSample)
{
    auto iter = g_codecBitsPerSampleMap.find(codecBitsPerSample);
    if (iter != g_codecBitsPerSampleMap.end()) {
        a2dpCodecInfo.bitsPerSample = iter->second;
    }
}

static void ConvertCodecChannelMode(A2dpCodecInfo &a2dpCodecInfo, int32_t codecChannelMode)
{
    auto iter = g_codecChannelModeMap.find(codecChannelMode);
    if (iter != g_codecChannelModeMap.end()) {
        a2dpCodecInfo.channelMode = iter->second;
    }
}

static void ConvertCodecSampleRate(A2dpCodecInfo &a2dpCodecInfo, int32_t codecSampleRate)
{
    auto iter = g_codecSampleRateMap.find(codecSampleRate);
    if (iter != g_codecSampleRateMap.end()) {
        a2dpCodecInfo.sampleRate = iter->second;
    }
}

static void ConvertCodecTypeToCodecInfo(CodecInfo &codecInfo, int32_t codecType)
{
    auto iter = g_a2dpCodecTypeMap.find(codecType);
    if (iter != g_a2dpCodecTypeMap.end()) {
        codecInfo.codecType = iter->second;
    }
}

static void ConvertCodecBitsPerSampleToCodecInfo(CodecInfo &codecInfo, int32_t codecBitsPerSample)
{
    auto iter = g_a2dpCodecBitsPerSampleMap.find(codecBitsPerSample);
    if (iter != g_a2dpCodecBitsPerSampleMap.end()) {
        codecInfo.codecBitsPerSample = iter->second;
    }
}

static void ConvertCodecChannelModeToCodecInfo(CodecInfo &codecInfo, int32_t codecChannelMode)
{
    auto iter = g_a2dpCodecChannelModeMap.find(codecChannelMode);
    if (iter != g_a2dpCodecChannelModeMap.end()) {
        codecInfo.codecChannelMode = iter->second;
    }
}

static void ConvertCodecSampleRateToCodecInfo(CodecInfo &codecInfo, int32_t codecSampleRate)
{
    auto iter = g_a2dpCodecSampleRateMap.find(codecSampleRate);
    if (iter != g_a2dpCodecSampleRateMap.end()) {
        codecInfo.codecSampleRate = iter->second;
    }
}

static void ConvertCodecInfoToJs(napi_env env, napi_value &object, const A2dpCodecInfo &a2dpCodecInfo)
{
    // convert A2dpCodecInfo to CodecInfo
    CodecInfo codecInfo;
    ConvertCodecSampleRateToCodecInfo(codecInfo, a2dpCodecInfo.sampleRate);
    ConvertCodecChannelModeToCodecInfo(codecInfo, a2dpCodecInfo.channelMode);
    ConvertCodecBitsPerSampleToCodecInfo(codecInfo, a2dpCodecInfo.bitsPerSample);
    ConvertCodecTypeToCodecInfo(codecInfo, a2dpCodecInfo.codecType);
    // convert CodecInfo to JS
    napi_value value = nullptr;
    napi_create_int32(env, codecInfo.codecType, &value);
    napi_set_named_property(env, object, "codecType", value);
    napi_create_int32(env, codecInfo.codecBitsPerSample, &value);
    napi_set_named_property(env, object, "codecBitsPerSample", value);
    napi_create_int32(env, codecInfo.codecChannelMode, &value);
    napi_set_named_property(env, object, "codecChannelMode", value);
    napi_create_int32(env, codecInfo.codecSampleRate, &value);
    napi_set_named_property(env, object, "codecSampleRate", value);
}

static napi_status CheckSetCodecPreferenceParam(napi_env env, napi_callback_info info,
    std::string &addr, A2dpCodecInfo &a2dpCodecInfo)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_TWO, "Requires 2 arguments.", napi_invalid_arg);
    NAPI_BT_CALL_RETURN(NapiParseBdAddr(env, argv[PARAM0], addr));
    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(
        env, argv[PARAM1], {"codecType", "codecBitsPerSample", "codecChannelMode", "codecSampleRate"}));
    // parse codecInfo
    int32_t codecType = 0;
    NAPI_BT_CALL_RETURN(NapiParseObjectInt32(env, argv[PARAM1], "codecType", codecType));
    ConvertCodecType(a2dpCodecInfo, codecType);
    int32_t codecBitsPerSample = 0;
    NAPI_BT_CALL_RETURN(NapiParseObjectInt32(env, argv[PARAM1], "codecBitsPerSample", codecBitsPerSample));
    ConvertCodecBitsPerSample(a2dpCodecInfo, codecBitsPerSample);
    int32_t codecChannelMode = 0;
    NAPI_BT_CALL_RETURN(NapiParseObjectInt32(env, argv[PARAM1], "codecChannelMode", codecChannelMode));
    ConvertCodecChannelMode(a2dpCodecInfo, codecChannelMode);
    int32_t codecSampleRate = 0;
    NAPI_BT_CALL_RETURN(NapiParseObjectInt32(env, argv[PARAM1], "codecSampleRate", codecSampleRate));
    ConvertCodecSampleRate(a2dpCodecInfo, codecSampleRate);
    return napi_ok;
}

napi_value NapiA2dpSource::SetCurrentCodecInfo(napi_env env, napi_callback_info info)
{
    std::string remoteAddr{};
    A2dpCodecInfo a2dpCodecInfo;
    auto status = CheckSetCodecPreferenceParam(env, info, remoteAddr, a2dpCodecInfo);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
    A2dpSource *profile = A2dpSource::GetProfile();
    int32_t errorCode = profile->SetCodecPreference(remoteDevice, a2dpCodecInfo);
    HILOGI("err: %{public}d", errorCode);
    NAPI_BT_ASSERT_RETURN_FALSE(env, errorCode == BT_NO_ERROR, errorCode);
    return NapiGetBooleanTrue(env);
}

napi_value NapiA2dpSource::GetCurrentCodecInfo(napi_env env, napi_callback_info info)
{
    HILOGI("start");
    napi_value ret = nullptr;
    napi_create_object(env, &ret);

    std::string remoteAddr{};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN(env, checkRet, BT_ERR_INVALID_PARAM, ret);
    A2dpCodecInfo a2dpCodecInfo;
    int transport = BT_TRANSPORT_BREDR;
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr, transport);
    A2dpSource *profile = A2dpSource::GetProfile();
    int errorCode = profile->GetCodecPreference(remoteDevice, a2dpCodecInfo);
    NAPI_BT_ASSERT_RETURN(env, (errorCode == BT_NO_ERROR), errorCode, ret);
    ConvertCodecInfoToJs(env, ret, a2dpCodecInfo);
    return ret;
}

napi_value NapiA2dpSource::EnableAutoPlay(napi_env env, napi_callback_info info)
{
    std::string deviceId = INVALID_MAC_ADDRESS;
    auto status = CheckDeivceIdParam(env, info, deviceId);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status, BT_ERR_INVALID_PARAM);

    auto func = [deviceId]() {
        BluetoothRemoteDevice device(deviceId, BT_TRANSPORT_BREDR);
        A2dpSource *profile = A2dpSource::GetProfile();
        int err = profile->EnableAutoPlay(device);
        HILOGI("err: %{public}d", err);
        return NapiAsyncWorkRet(err);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

static bool IsInvalidAutoPlayDuration(int32_t duration)
{
    return duration < MIN_AUTO_PLAY_DURATION_SEC
        || duration> MAX_AUTO_PLAY_DURATION_SEC;
}

static napi_status CheckDisableAutoPlayParam(napi_env env, napi_callback_info info, std::string &addr,
    int32_t &duration)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_TWO, "Requires 2 arguments", napi_invalid_arg);
    NAPI_BT_CALL_RETURN(NapiParseBdAddr(env, argv[PARAM0], addr));
    NAPI_BT_RETURN_IF(!ParseInt32(env, duration, argv[PARAM1]), "ParseInt failed", napi_invalid_arg);
    NAPI_BT_RETURN_IF(IsInvalidAutoPlayDuration(duration), "Invalid duration", napi_invalid_arg);
    return napi_ok;
}

napi_value NapiA2dpSource::DisableAutoPlay(napi_env env, napi_callback_info info)
{
    std::string deviceId = INVALID_MAC_ADDRESS;
    int duration = 0;
    auto status = CheckDisableAutoPlayParam(env, info, deviceId, duration);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [deviceId, duration]() {
        BluetoothRemoteDevice device(deviceId, BT_TRANSPORT_BREDR);
        A2dpSource *profile = A2dpSource::GetProfile();
        int err = profile->DisableAutoPlay(device, duration);
        HILOGI("err: %{public}d", err);
        return NapiAsyncWorkRet(err);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiA2dpSource::GetAutoPlayDisabledDuration(napi_env env, napi_callback_info info)
{
    std::string deviceId = INVALID_MAC_ADDRESS;
    auto status = CheckDeivceIdParam(env, info, deviceId);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status, BT_ERR_INVALID_PARAM);

    auto func = [deviceId]() {
        int duration = 0;
        BluetoothRemoteDevice device(deviceId, BT_TRANSPORT_BREDR);
        A2dpSource *profile = A2dpSource::GetProfile();
        int err = profile->GetAutoPlayDisabledDuration(device, duration);
        HILOGI("err: %{public}d, duration: %{public}d", err, duration);
        auto object = std::make_shared<NapiNativeInt>(duration);
        return NapiAsyncWorkRet(err, object);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}
#endif
}  // namespace Bluetooth
}  // namespace OHOS