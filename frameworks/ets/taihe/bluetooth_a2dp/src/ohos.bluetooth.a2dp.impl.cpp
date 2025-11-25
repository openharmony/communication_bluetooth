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
#define LOG_TAG "bt_a2dp_impl_ohbluetooth"
#endif

#include "ohos.bluetooth.a2dp.proj.hpp"
#include "ohos.bluetooth.a2dp.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "bluetooth_a2dp_src.h"
#include "bluetooth_avrcp_tg.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "taihe_bluetooth_a2dp_src_observer.h"
#include "taihe_bluetooth_utils.h"

namespace OHOS {
namespace Bluetooth {
class A2dpSourceProfileImpl {
public:
    A2dpSourceProfileImpl()
    {
        observer_ = std::make_shared<TaiheA2dpSourceObserver>();
    }

    void OnConnectionStateChange(::taihe::callback_view<void(
        ::ohos::bluetooth::baseProfile::StateChangeParam const& data)> callback)
    {
        if (observer_) {
            observer_->eventSubscribe_.RegisterEvent(callback);
        }
        if (!isRegistered_) {
            A2dpSource *profile = A2dpSource::GetProfile();
            profile->RegisterObserver(observer_);
            isRegistered_ = true;
        }
    }

    void OffConnectionStateChange(::taihe::optional_view<::taihe::callback<void(
        ::ohos::bluetooth::baseProfile::StateChangeParam const& data)>> callback)
    {
        if (observer_) {
            observer_->eventSubscribe_.DeregisterEvent(callback);
        }
    }

    void SetCurrentCodecInfo(taihe::string_view deviceId, ohos::bluetooth::a2dp::CodecInfo codecInfo)
    {
        std::string remoteAddr = static_cast<std::string>(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

        A2dpCodecInfo a2dpCodecInfo;
        a2dpCodecInfo.codecType = static_cast<uint8_t>(codecInfo.codecType);
        a2dpCodecInfo.bitsPerSample = static_cast<uint8_t>(codecInfo.codecBitsPerSample);
        a2dpCodecInfo.channelMode = static_cast<uint8_t>(codecInfo.codecChannelMode);
        a2dpCodecInfo.sampleRate = static_cast<uint8_t>(codecInfo.codecSampleRate);
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        A2dpSource *profile = A2dpSource::GetProfile();
        int32_t errorCode = profile->SetCodecPreference(remoteDevice, a2dpCodecInfo);
        HILOGI("err: %{public}d", errorCode);
        TAIHE_BT_ASSERT_RETURN_VOID(errorCode == BT_NO_ERROR, errorCode);
    }

    ohos::bluetooth::a2dp::CodecInfo GetCurrentCodecInfo(taihe::string_view deviceId)
    {
        HILOGI("start");
        std::string remoteAddr = static_cast<std::string>(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        ohos::bluetooth::a2dp::CodecInfo codecInfo = {
            ohos::bluetooth::a2dp::CodecType::key_t::CODEC_TYPE_INVALID,
            ohos::bluetooth::a2dp::CodecBitsPerSample::key_t::CODEC_BITS_PER_SAMPLE_NONE,
            ohos::bluetooth::a2dp::CodecChannelMode::key_t::CODEC_CHANNEL_MODE_NONE,
            ohos::bluetooth::a2dp::CodecSampleRate::key_t::CODEC_SAMPLE_RATE_NONE
        };
        TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM, codecInfo);

        A2dpCodecInfo a2dpCodecInfo;
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        A2dpSource *profile = A2dpSource::GetProfile();
        int errorCode = profile->GetCodecPreference(remoteDevice, a2dpCodecInfo);
        TAIHE_BT_ASSERT_RETURN(errorCode == BT_NO_ERROR, errorCode, codecInfo);

        codecInfo.codecType = ohos::bluetooth::a2dp::CodecType::from_value(a2dpCodecInfo.codecType);
        codecInfo.codecBitsPerSample =
            ohos::bluetooth::a2dp::CodecBitsPerSample::from_value(a2dpCodecInfo.bitsPerSample);
        codecInfo.codecChannelMode = ohos::bluetooth::a2dp::CodecChannelMode::from_value(a2dpCodecInfo.channelMode);
        codecInfo.codecSampleRate = ohos::bluetooth::a2dp::CodecSampleRate::from_value(a2dpCodecInfo.sampleRate);

        return codecInfo;
    }

    ohos::bluetooth::a2dp::PlayingState GetPlayingState(taihe::string_view deviceId)
    {
        HILOGD("start");
        std::string remoteAddr = static_cast<std::string>(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM, ohos::bluetooth::a2dp::PlayingState::from_value(0));

        int state = 0;
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        A2dpSource *profile = A2dpSource::GetProfile();
        int32_t errorCode = profile->GetPlayingState(remoteDevice, state);
        HILOGI("errorCode: %{public}d", errorCode);
        TAIHE_BT_ASSERT_RETURN(errorCode == BT_NO_ERROR, errorCode,
            ohos::bluetooth::a2dp::PlayingState::from_value(state));

        return ohos::bluetooth::a2dp::PlayingState::from_value(state);
    }

    void Connect(taihe::string_view deviceId)
    {
        HILOGD("start");
        std::string remoteAddr = static_cast<std::string>(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        A2dpSource *profile = A2dpSource::GetProfile();
        int32_t ret = profile->Connect(remoteDevice);
        TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
    }

    void Disconnect(taihe::string_view deviceId)
    {
        HILOGD("start");
        std::string remoteAddr = static_cast<std::string>(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        A2dpSource *profile = A2dpSource::GetProfile();
        int32_t ret = profile->Disconnect(remoteDevice);
        TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
    }

    int32_t GetAutoPlayDisabledDuration(taihe::string_view deviceId)
    {
        std::string remoteAddr = static_cast<std::string>(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM, 0);

        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        A2dpSource *profile = A2dpSource::GetProfile();
        int32_t duration = 0;
        int err = profile->GetAutoPlayDisabledDuration(device, duration);
        HILOGI("err: %{public}d, duration: %{public}d", err, duration);
        TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, duration);

        return duration;
    }

    void DisableAutoPlay(taihe::string_view deviceId, int32_t duration)
    {
        std::string remoteAddr = static_cast<std::string>(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        A2dpSource *profile = A2dpSource::GetProfile();
        int err = profile->DisableAutoPlay(device, duration);
        HILOGI("err: %{public}d", err);
        TAIHE_BT_ASSERT_RETURN_VOID(err == BT_NO_ERROR, err);
    }

    void EnableAutoPlay(taihe::string_view deviceId)
    {
        std::string remoteAddr = static_cast<std::string>(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        A2dpSource *profile = A2dpSource::GetProfile();
        int err = profile->EnableAutoPlay(device);
        HILOGI("err: %{public}d", err);
        TAIHE_BT_ASSERT_RETURN_VOID(err == BT_NO_ERROR, err);
    }

    ohos::bluetooth::constant::ProfileConnectionState GetConnectionState(taihe::string_view deviceId)
    {
        HILOGD("enter");
        std::string remoteAddr = std::string(deviceId);
        A2dpSource *profile = A2dpSource::GetProfile();
        BluetoothRemoteDevice device(remoteAddr, 1);
        int btConnectState = static_cast<int32_t>(BTConnectState::DISCONNECTED);
        int errorCode = profile->GetDeviceState(device, btConnectState);
        int status = TaiheUtils::GetProfileConnectionState(btConnectState);
        TAIHE_BT_ASSERT_RETURN(errorCode == BT_NO_ERROR, errorCode,
            ohos::bluetooth::constant::ProfileConnectionState::from_value(status));
        HILOGD("status: %{public}d", status);

        return ohos::bluetooth::constant::ProfileConnectionState::from_value(status);
    }

    taihe::array<taihe::string> GetConnectedDevices()
    {
        HILOGI("enter");
        A2dpSource *profile = A2dpSource::GetProfile();
        std::vector<int> states;
        states.push_back(1);
        std::vector<BluetoothRemoteDevice> devices;
        int errorCode = profile->GetDevicesByStates(states, devices);
        TAIHE_BT_ASSERT_RETURN(errorCode == BT_NO_ERROR, errorCode, taihe::array<taihe::string>{});

        std::vector<std::string> deviceVector;
        for (auto &device : devices) {
            deviceVector.push_back(device.GetDeviceAddr());
        }

        return deviceVector.empty() ? taihe::array<taihe::string>{}
            : taihe::array<taihe::string>(taihe::copy_data_t{}, deviceVector.data(), deviceVector.size());
    }

    void DisableAbsoluteVolumeSync(taihe::string_view deviceId)
    {
        HILOGD("start");
        std::string remoteAddr = static_cast<std::string>(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

        int32_t ability = DeviceAbsVolumeAbility::DEVICE_ABSVOL_CLOSE;
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t err = AvrcpTarget::GetProfile()->SetDeviceAbsVolumeAbility(remoteDevice, ability);
        TAIHE_BT_ASSERT_RETURN_VOID(err == BT_NO_ERROR, err);
    }

    void EnableAbsoluteVolumeSync(taihe::string_view deviceId)
    {
        HILOGD("start");
        std::string remoteAddr = static_cast<std::string>(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

        int32_t ability = DeviceAbsVolumeAbility::DEVICE_ABSVOL_OPEN;
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t err = AvrcpTarget::GetProfile()->SetDeviceAbsVolumeAbility(remoteDevice, ability);
        TAIHE_BT_ASSERT_RETURN_VOID(err == BT_NO_ERROR, err);
    }

    bool IsAbsoluteVolumeEnabledSync(taihe::string_view deviceId)
    {
        HILOGD("start");
        std::string remoteAddr = static_cast<std::string>(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM, false);

        int32_t ability = DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT;
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t err = AvrcpTarget::GetProfile()->GetDeviceAbsVolumeAbility(remoteDevice, ability);
        TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, false);
        if (ability == DeviceAbsVolumeAbility::DEVICE_ABSVOL_OPEN) {
            return true;
        }
        return false;
    }

    bool IsAbsoluteVolumeSupportedSync(taihe::string_view deviceId)
    {
        HILOGD("start");
        std::string remoteAddr = static_cast<std::string>(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM, false);

        int32_t ability = DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT;
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t err = AvrcpTarget::GetProfile()->GetDeviceAbsVolumeAbility(remoteDevice, ability);
        TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, false);
        if (ability == DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT) {
            return false;
        }
        return true;
    }

    ohos::bluetooth::baseProfile::ConnectionStrategy GetConnectionStrategySync(taihe::string_view deviceId)
    {
        HILOGD("start");
        std::string remoteAddr = std::string(deviceId);
        bool checkRet = CheckDeviceIdParam(remoteAddr);
        TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM,
            ohos::bluetooth::baseProfile::ConnectionStrategy::from_value(0));

        int transport = BT_TRANSPORT_BREDR;
        int strategy = 0;
        BluetoothRemoteDevice remoteDevice(remoteAddr, transport);
        A2dpSource *profile = A2dpSource::GetProfile();
        int32_t err = profile->GetConnectStrategy(remoteDevice, strategy);
        HILOGD("err: %{public}d, strategy: %{public}d", err, strategy);
        TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err,
            ohos::bluetooth::baseProfile::ConnectionStrategy::from_value(strategy));

        return ohos::bluetooth::baseProfile::ConnectionStrategy::from_value(strategy);
    }

    void SetConnectionStrategySync(taihe::string_view deviceId,
                                   ohos::bluetooth::baseProfile::ConnectionStrategy strategy)
    {
        HILOGD("start");
        std::string remoteAddr = std::string(deviceId);
        int32_t connStrategy = strategy.get_value();
        bool checkRet = CheckSetConnectStrategyParam(remoteAddr, connStrategy);
        TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

        int transport = BT_TRANSPORT_BREDR;
        BluetoothRemoteDevice remoteDevice(remoteAddr, transport);
        A2dpSource *profile = A2dpSource::GetProfile();
        int32_t err = profile->SetConnectStrategy(remoteDevice, connStrategy);
        HILOGI("err: %{public}d", err);
        TAIHE_BT_ASSERT_RETURN_VOID(err == BT_NO_ERROR, err);
    }
private:
    std::shared_ptr<TaiheA2dpSourceObserver> observer_ = nullptr;
    bool isRegistered_ = false;
};

::ohos::bluetooth::a2dp::A2dpSourceProfile CreateA2dpSrcProfile()
{
    // The parameters in the make_holder function should be of the same type
    // as the parameters in the constructor of the actual implementation class.
    return taihe::make_holder<A2dpSourceProfileImpl, ohos::bluetooth::a2dp::A2dpSourceProfile>();
}
}  // namespace Bluetooth
}  // namespace OHOS

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_CreateA2dpSrcProfile(OHOS::Bluetooth::CreateA2dpSrcProfile);
// NOLINTEND
