/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_server_audio_manager"
#endif

#include "bluetooth_audio_manager_server.h"
#include "bluetooth_utils_server.h"
#include "bluetooth_def.h"
#include "bluetooth_log.h"
#include "bluetooth_audio_manager_interface.h"
#include "bluetooth_device_manager.h"
#include "bluetooth_errorcode.h"
#include "common_util.h"

using namespace std;
using namespace OHOS::bluetooth;

namespace OHOS {
namespace Bluetooth {
struct BluetoothAudioManagerServer::impl {
    int32_t EnableWearDetection(const std::string &addr);
    int32_t DisableWearDetection(const std::string &addr);
    int32_t GetWearDetectionState(const std::string &addr, int32_t &ability);
    int32_t IsDeviceWearing(const BluetoothRawAddress &device);
    int32_t SendDeviceSelection(const BluetoothRawAddress &device, int useA2dp, int useHfp, int userSelect);
    int32_t IsWearDetectionSupported(const BluetoothRawAddress &device, bool &isSupported);
    int32_t GetProfileStatus(const BluetoothRawAddress &device, uint8_t &a2dpState, uint8_t &hfpState);
};

BluetoothAudioManagerServer::BluetoothAudioManagerServer() : pimpl(std::make_unique<impl>())
{}

BluetoothAudioManagerServer::~BluetoothAudioManagerServer() = default;

int32_t BluetoothAudioManagerServer::impl::EnableWearDetection(const std::string &addr)
{
    return bluetooth::BluetoothAudioManagerInterface::BtAudioManagerSetSupport(addr, true);
}

int32_t BluetoothAudioManagerServer::impl::DisableWearDetection(const std::string &addr)
{
    return bluetooth::BluetoothAudioManagerInterface::BtAudioManagerSetSupport(addr, false);
}

int32_t BluetoothAudioManagerServer::impl::GetWearDetectionState(const std::string &addr, int32_t &ability)
{
    return bluetooth::BluetoothAudioManagerInterface::BtAudioManagerGetWearState(addr, ability);
}

int32_t BluetoothAudioManagerServer::impl::IsDeviceWearing(const BluetoothRawAddress &device)
{
    HILOGI("address: %{public}s", GET_ENCRYPT_ADDR(device));
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }

    return bluetooth::BluetoothAudioManagerInterface::IsDeviceWearing(realAddr);
}

int32_t BluetoothAudioManagerServer::impl::SendDeviceSelection(const BluetoothRawAddress &device, int useA2dp,
    int useHfp, int userSelect)
{
    HILOG_COMM_INFO("SendDeviceSelection: %{public}s", GET_ENCRYPT_ADDR(device));

    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }

    return bluetooth::BluetoothAudioManagerInterface::SendDeviceSelection(realAddr, useA2dp, useHfp, userSelect);
}

int32_t BluetoothAudioManagerServer::impl::IsWearDetectionSupported(const BluetoothRawAddress &device,
    bool &isSupported)
{
    HILOGI("address: %{public}s", GET_ENCRYPT_ADDR(device));

    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }

    return bluetooth::BluetoothAudioManagerInterface::IsWearDetectionSupported(realAddr, isSupported);
}

int32_t BluetoothAudioManagerServer::impl::GetProfileStatus(const BluetoothRawAddress &device,
    uint8_t &a2dpState, uint8_t &hfpState)
{
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!bluetooth::BluetoothAudioManagerInterface::GetProfileStatus(realAddr, a2dpState, hfpState)) {
        return BT_ERR_INTERNAL_ERROR;
    }

    return BT_NO_ERROR;
}

int32_t BluetoothAudioManagerServer::EnableWearDetection(const std::string &addr)
{
    return pimpl->EnableWearDetection(addr);
}

int32_t BluetoothAudioManagerServer::DisableWearDetection(const std::string &addr)
{
    return pimpl->DisableWearDetection(addr);
}

int32_t BluetoothAudioManagerServer::GetWearDetectionState(const std::string &addr, int32_t &ability)
{
    return pimpl->GetWearDetectionState(addr, ability);
}

int32_t BluetoothAudioManagerServer::IsDeviceWearing(const BluetoothRawAddress &device)
{
    return pimpl->IsDeviceWearing(device);
}

int32_t BluetoothAudioManagerServer::SendDeviceSelection(const BluetoothRawAddress &device,
    int useA2dp, int useHfp, int userSelect)
{
    return pimpl->SendDeviceSelection(device, useA2dp, useHfp, userSelect);
}

int32_t BluetoothAudioManagerServer::IsWearDetectionSupported(const BluetoothRawAddress &device, bool &isSupported)
{
    return pimpl->IsWearDetectionSupported(device, isSupported);
}

int32_t BluetoothAudioManagerServer::GetProfileStatus(const BluetoothRawAddress &device,
    uint8_t &a2dpState, uint8_t &hfpState)
{
    return pimpl->GetProfileStatus(device, a2dpState, hfpState);
}
}  // namespace Bluetooth
}  // namespace OHOS