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
#include "taihe_bluetooth_a2dp_src_observer.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"

namespace OHOS {
namespace Bluetooth {

using namespace taihe;
using namespace ohos::bluetooth::a2dp;

class A2dpSourceProfileImpl {
public:
    A2dpSourceProfileImpl()
    {
        observer_ = std::make_shared<TaiheA2dpSourceObserver>();
    }

    void On(::taihe::string_view type, ::taihe::callback_view<void(
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
    
    void Off(::taihe::string_view type, ::taihe::optional_view<::taihe::callback<void(
        ::ohos::bluetooth::baseProfile::StateChangeParam const& data)>> callback)
    {
        if (observer_) {
            observer_->eventSubscribe_.DeregisterEvent(callback);
        }
    }

    void SetCurrentCodecInfo(taihe::string_view deviceId, CodecInfo codecInfo)
    {
        std::string remoteAddr = static_cast<std::string>(deviceId);
        A2dpCodecInfo a2dpCodecInfo;
        a2dpCodecInfo.codecType = static_cast<uint8_t>(codecInfo.codecType);
        a2dpCodecInfo.bitsPerSample = static_cast<uint8_t>(codecInfo.codecBitsPerSample);
        a2dpCodecInfo.channelMode = static_cast<uint8_t>(codecInfo.codecChannelMode);
        a2dpCodecInfo.sampleRate = static_cast<uint8_t>(codecInfo.codecSampleRate);
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        A2dpSource *profile = A2dpSource::GetProfile();
        int32_t err = profile->SetCodecPreference(remoteDevice, a2dpCodecInfo);
        if (err != BT_NO_ERROR) {
            taihe::set_business_error(err, "SetCurrentCodecInfo return error");
        }
    }

    CodecInfo GetCurrentCodecInfo(taihe::string_view deviceId)
    {
        std::string remoteAddr = static_cast<std::string>(deviceId);
        A2dpCodecInfo a2dpCodecInfo;
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        A2dpSource *profile = A2dpSource::GetProfile();
        int err = profile->GetCodecPreference(remoteDevice, a2dpCodecInfo);
        if (err != BT_NO_ERROR) {
            taihe::set_business_error(err, "GetCurrentCodecInfo return error");
        }

        CodecInfo codecInfo = {
            CodecType::key_t::CODEC_TYPE_INVALID,
            CodecBitsPerSample::key_t::CODEC_BITS_PER_SAMPLE_NONE,
            CodecChannelMode::key_t::CODEC_CHANNEL_MODE_NONE,
            CodecSampleRate::key_t::CODEC_SAMPLE_RATE_NONE
        };
        codecInfo.codecType = CodecType::from_value(a2dpCodecInfo.codecType);
        codecInfo.codecBitsPerSample = CodecBitsPerSample::from_value(a2dpCodecInfo.bitsPerSample);
        codecInfo.codecChannelMode = CodecChannelMode::from_value(a2dpCodecInfo.channelMode);
        codecInfo.codecSampleRate = CodecSampleRate::from_value(a2dpCodecInfo.sampleRate);
        return codecInfo;
    }

    PlayingState GetPlayingState(taihe::string_view deviceId)
    {
        std::string remoteAddr = static_cast<std::string>(deviceId);
        int state = 0;
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        A2dpSource *profile = A2dpSource::GetProfile();
        int32_t err = profile->GetPlayingState(remoteDevice, state);
        if (err != BT_NO_ERROR) {
            taihe::set_business_error(err, "GetPlayingState return error");
        }
        return static_cast<PlayingState::key_t>(state);
    }

    void Connect(taihe::string_view deviceId)
    {
        std::string remoteAddr = static_cast<std::string>(deviceId);
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        A2dpSource *profile = A2dpSource::GetProfile();
        int32_t err = profile->Connect(remoteDevice);
        if (err != BT_NO_ERROR) {
            taihe::set_business_error(err, "Connect return error");
        }
    }

    void Disconnect(taihe::string_view deviceId)
    {
        std::string remoteAddr = static_cast<std::string>(deviceId);
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        A2dpSource *profile = A2dpSource::GetProfile();
        int32_t err = profile->Disconnect(remoteDevice);
        if (err != BT_NO_ERROR) {
            taihe::set_business_error(err, "Disconnect return error");
        }
    }

    ohos::bluetooth::constant::ProfileConnectionState GetConnectionState(taihe::string_view deviceId)
    {
        std::string remoteAddr = std::string(deviceId);
        PbapPse *profile = PbapPse::GetProfile();
        BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
        int32_t state = static_cast<int32_t>(BTConnectState::DISCONNECTED);
        int32_t errorCode = profile->GetDeviceState(device, state);
        if (errorCode != BT_NO_ERROR) {
            taihe::set_business_error(errorCode, "GetConnectionState return error");
        }

        int32_t profileState = GetProfileConnectionState(state);
        return (ohos::bluetooth::constant::ProfileConnectionState::key_t)profileState;
    }

    array<string> GetConnectedDevices()
    {
        A2dpSource *profile = A2dpSource::GetProfile();
        std::vector<int> states;
        states.push_back(1);
        std::vector<BluetoothRemoteDevice> devices;
        int errorCode = profile->GetDevicesByStates(states, devices);
        if (errorCode != BT_NO_ERROR) {
            taihe::set_business_error(errorCode, "GetConnectedDevices return error");
        }

        std::vector<string> deviceVector;
        for (auto &device : devices) {
            deviceVector.push_back(device.GetDeviceAddr());
        }

        return deviceVector.empty() ? array<string>{}
                                    : array<string>(taihe::copy_data_t{}, deviceVector.data(), deviceVector.size());
    }

    void SetConnectionStrategySync(string_view deviceId, ohos::bluetooth::baseProfile::ConnectionStrategy strategy) {}
private:
    std::shared_ptr<TaiheA2dpSourceObserver> observer_ = nullptr;
    bool isRegistered_ = false;
};

A2dpSourceProfile CreateA2dpSrcProfile()
{
    // The parameters in the make_holder function should be of the same type
    // as the parameters in the constructor of the actual implementation class.
    return make_holder<A2dpSourceProfileImpl, ohos::bluetooth::a2dp::A2dpSourceProfile>();
}
}  // namespace Bluetooth
}  // namespace OHOS

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_CreateA2dpSrcProfile(OHOS::Bluetooth::CreateA2dpSrcProfile);
// NOLINTEND
