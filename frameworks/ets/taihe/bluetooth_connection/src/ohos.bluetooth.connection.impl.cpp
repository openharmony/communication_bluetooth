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


#include "stdexcept"

#include "ohos.bluetooth.connection.proj.hpp"
#include "ohos.bluetooth.connection.impl.hpp"
#include "taihe/runtime.hpp"

#include "bluetooth_errorcode.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_remote_device.h"
#include "taihe_bluetooth_connection_callback.h"

namespace OHOS {
namespace Bluetooth {

using namespace taihe;
using namespace ohos::bluetooth::connection;

std::shared_ptr<TaiheBluetoothRemoteDeviceObserver> g_remoteDeviceObserver =
    TaiheBluetoothRemoteDeviceObserver::GetInstance();
std::shared_ptr<TaiheBluetoothConnectionObserver> g_connectionObserver =
    TaiheBluetoothConnectionObserver::GetInstance();

string GetRemoteProductId(string_view deviceId)
{
    std::string remoteAddr = std::string(deviceId);
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    std::string productId = "";
    int32_t err = remoteDevice.GetDeviceProductId(productId);
    if (err != BT_NO_ERROR) {
        set_business_error(err, "GetRemoteProductId return error");
    }
    return productId;
}

string GetRemoteDeviceName(string_view deviceId)
{
    std::string remoteAddr = std::string(deviceId);
    std::string name = INVALID_NAME;
    bool deviceAlias = false;

    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int32_t err = remoteDevice.GetDeviceName(name, deviceAlias);
    if (err != BT_NO_ERROR) {
        set_business_error(err, "GetRemoteDeviceName return error");
    }
    return name;
}

string GetRemoteDeviceNameWithAlias(string_view deviceId, optional_view<bool> alias)
{
    std::string remoteAddr = std::string(deviceId);
    std::string name = INVALID_NAME;
    bool deviceAlias = static_cast<bool>(alias);
 
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int32_t err = remoteDevice.GetDeviceName(name, deviceAlias);
    if (err != BT_NO_ERROR) {
        set_business_error(err, "GetRemoteDeviceNameWithAlias return error");
    }
    return name;
}

array<string> GetPairedDevices()
{
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    std::vector<BluetoothRemoteDevice> remoteDeviceLists;
    std::vector<std::string> dstDevicesvec;
    int32_t err = host->GetPairedDevices(BT_TRANSPORT_BREDR, remoteDeviceLists);
    if (err != BT_NO_ERROR) {
        set_business_error(err, "GetPairedDevices return error");
        return {};
    }
    for (auto vec : remoteDeviceLists) {
        dstDevicesvec.push_back(vec.GetDeviceAddr().c_str());
    }
    array<string> result(taihe::copy_data_t{}, dstDevicesvec.data(), dstDevicesvec.size());
    return result;
}

void SetBluetoothScanMode(ScanMode mode, int duration)
{
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int32_t bondableMode = 1;
    int32_t err = host->SetBtScanMode(mode, duration);
    if (err != BT_NO_ERROR) {
        set_business_error(err, "SetBluetoothScanMode return error");
        return;
    }
    host->SetBondableMode(BT_TRANSPORT_BREDR, bondableMode);
}

DeviceClass GetRemoteDeviceClass(string_view deviceId)
{
    std::string remoteAddr = std::string(deviceId);
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int tmpCod = ohos::bluetooth::constant::MajorClass(
        ohos::bluetooth::constant::MajorClass::key_t::MAJOR_UNCATEGORIZED).get_value();
    int tmpMajorClass = ohos::bluetooth::constant::MajorClass(
        ohos::bluetooth::constant::MajorClass::key_t::MAJOR_UNCATEGORIZED).get_value();
    int tmpMajorMinorClass = ohos::bluetooth::constant::MajorMinorClass(
        ohos::bluetooth::constant::MajorMinorClass::key_t::COMPUTER_UNCATEGORIZED).get_value();
    int32_t err = remoteDevice.GetDeviceProductType(tmpCod, tmpMajorClass, tmpMajorMinorClass);
    if (err != BT_NO_ERROR) {
        set_business_error(err, "GetRemoteDeviceClass return error");
    }
    return {ohos::bluetooth::constant::MajorClass::from_value(tmpMajorClass),
        ohos::bluetooth::constant::MajorMinorClass::from_value(tmpMajorMinorClass), tmpCod};
}

void OnDiscoveryResult(::taihe::callback_view<void(
    ::taihe::array_view<::ohos::bluetooth::connection::DiscoveryResult> data)> callback)
{
    std::unique_lock<std::shared_mutex> guard(g_connectionObserver->discoveryResultLock);
    auto discoveryResultCb = ::taihe::optional<::taihe::callback<void(
        ::taihe::array_view<::ohos::bluetooth::connection::DiscoveryResult> data)>>{std::in_place_t{}, callback};
    auto& callbackVec = g_connectionObserver->discoveryResultVec;
    // callback重复注册不处理
    if (std::find(callbackVec.begin(), callbackVec.end(), discoveryResultCb) != callbackVec.end()) {
        return;
    }
    callbackVec.emplace_back(discoveryResultCb);
}

void OffDiscoveryResult(::taihe::optional_view<::taihe::callback<void(
    ::taihe::array_view<::ohos::bluetooth::connection::DiscoveryResult> data)>> callback)
{
    std::unique_lock<std::shared_mutex> guard(g_connectionObserver->discoveryResultLock);
    auto& callbackVec = g_connectionObserver->discoveryResultVec;
    // callback有值，则删除该callback，否则删除vec中注册的所有callback
    if (callback.has_value()) {
        callbackVec.erase(
            std::remove_if(callbackVec.begin(), callbackVec.end(),
                [callback](const ::taihe::optional<::taihe::callback<void(
                    ::taihe::array_view<::ohos::bluetooth::connection::DiscoveryResult> data)>>& it) {
                    return it == callback;
                }),
            callbackVec.end());
    } else {
        callbackVec.clear();
    }
}

void OnBatteryChange(::taihe::callback_view<void(::ohos::bluetooth::connection::BatteryInfo const& data)> callback)
{
    std::unique_lock<std::shared_mutex> guard(g_remoteDeviceObserver->remoteBatteryChangeLock);
    auto batteryChangeCb = ::taihe::optional<::taihe::callback<void(
        ::ohos::bluetooth::connection::BatteryInfo const& data)>>{std::in_place_t{}, callback};
    auto& callbackVec = g_remoteDeviceObserver->remoteBatteryChangeVec;
    if (std::find(callbackVec.begin(), callbackVec.end(), batteryChangeCb) != callbackVec.end()) {
        return;
    }
    callbackVec.emplace_back(batteryChangeCb);
}

void OffBatteryChange(::taihe::optional_view<::taihe::callback<void(
    ::ohos::bluetooth::connection::BatteryInfo const& data)>> callback)
{
    std::unique_lock<std::shared_mutex> guard(g_remoteDeviceObserver->remoteBatteryChangeLock);
    auto& callbackVec = g_remoteDeviceObserver->remoteBatteryChangeVec;
    if (callback.has_value()) {
        callbackVec.erase(
            std::remove_if(callbackVec.begin(), callbackVec.end(),
                [callback](const ::taihe::optional<
                    ::taihe::callback<void(::ohos::bluetooth::connection::BatteryInfo const& data)>>& it) {
                    return it == callback;
                }),
            callbackVec.end());
    } else {
        callbackVec.clear();
    }
}

void OnBluetoothDeviceFind(::taihe::callback_view<void(::taihe::array_view<::taihe::string> data)> callback)
{
    std::unique_lock<std::shared_mutex> guard(g_connectionObserver->bluetoothDeviceFindLock);
    auto bluetoothDeviceFindCb = ::taihe::optional<::taihe::callback<void(
        ::taihe::array_view<::taihe::string> data)>>{std::in_place_t{}, callback};
    auto& callbackVec = g_connectionObserver->bluetoothDeviceFindVec;
    if (std::find(callbackVec.begin(), callbackVec.end(), bluetoothDeviceFindCb) != callbackVec.end()) {
        return;
    }
    callbackVec.emplace_back(bluetoothDeviceFindCb);
}

void OffBluetoothDeviceFind(::taihe::optional_view<::taihe::callback<void(
    ::taihe::array_view<::taihe::string> data)>> callback)
{
    std::unique_lock<std::shared_mutex> guard(g_connectionObserver->bluetoothDeviceFindLock);
    auto& callbackVec = g_connectionObserver->bluetoothDeviceFindVec;
    if (callback.has_value()) {
        callbackVec.erase(
            std::remove_if(callbackVec.begin(), callbackVec.end(),
                [callback](const ::taihe::optional<
                    ::taihe::callback<void(::taihe::array_view<::taihe::string> data)>>& it) {
                    return it == callback;
                }),
            callbackVec.end());
    } else {
        callbackVec.clear();
    }
}

void OnBondStateChange(::taihe::callback_view<void(::ohos::bluetooth::connection::BondStateParam const& data)> callback)
{
    std::unique_lock<std::shared_mutex> guard(g_remoteDeviceObserver->bondStateChangeLock);
    auto bondStateChangeCb = ::taihe::optional<::taihe::callback<void(
        ::ohos::bluetooth::connection::BondStateParam const& data)>>{std::in_place_t{}, callback};
    auto& callbackVec = g_remoteDeviceObserver->bondStateChangeVec;
    if (std::find(callbackVec.begin(), callbackVec.end(), bondStateChangeCb) != callbackVec.end()) {
        return;
    }
    callbackVec.emplace_back(bondStateChangeCb);
}

void OffBondStateChange(::taihe::optional_view<::taihe::callback<void(
    ::ohos::bluetooth::connection::BondStateParam const& data)>> callback)
{
    std::unique_lock<std::shared_mutex> guard(g_remoteDeviceObserver->bondStateChangeLock);
    auto& callbackVec = g_remoteDeviceObserver->bondStateChangeVec;
    if (callback.has_value()) {
        callbackVec.erase(
            std::remove_if(callbackVec.begin(), callbackVec.end(),
                [callback](const ::taihe::optional<
                    ::taihe::callback<void(::ohos::bluetooth::connection::BondStateParam const& data)>>& it) {
                    return it == callback;
                }),
            callbackVec.end());
    } else {
        callbackVec.clear();
    }
}

void OnPinRequired(::taihe::callback_view<void(::ohos::bluetooth::connection::PinRequiredParam const& data)> callback)
{
    std::unique_lock<std::shared_mutex> guard(g_connectionObserver->pinRequiredLock);
    auto pinRequiredCb = ::taihe::optional<::taihe::callback<void(
        ::ohos::bluetooth::connection::PinRequiredParam const& data)>>{std::in_place_t{}, callback};
    auto& callbackVec = g_connectionObserver->pinRequiredVec;
    if (std::find(callbackVec.begin(), callbackVec.end(), pinRequiredCb) != callbackVec.end()) {
        return;
    }
    callbackVec.emplace_back(pinRequiredCb);
}

void OffPinRequired(::taihe::optional_view<::taihe::callback<
    void(::ohos::bluetooth::connection::PinRequiredParam const& data)>> callback)
{
    std::unique_lock<std::shared_mutex> guard(g_connectionObserver->pinRequiredLock);
    auto& callbackVec = g_connectionObserver->pinRequiredVec;
    if (callback.has_value()) {
        callbackVec.erase(
            std::remove_if(callbackVec.begin(), callbackVec.end(),
                [callback](const ::taihe::optional<
                    ::taihe::callback<void(::ohos::bluetooth::connection::PinRequiredParam const& data)>>& it) {
                    return it == callback;
                }),
            callbackVec.end());
    } else {
        callbackVec.clear();
    }
}
}  // Bluetooth
}  // OHOS

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_OnDiscoveryResult(OHOS::Bluetooth::OnDiscoveryResult);
TH_EXPORT_CPP_API_OffDiscoveryResult(OHOS::Bluetooth::OffDiscoveryResult);
TH_EXPORT_CPP_API_OnBatteryChange(OHOS::Bluetooth::OnBatteryChange);
TH_EXPORT_CPP_API_OffBatteryChange(OHOS::Bluetooth::OffBatteryChange);
TH_EXPORT_CPP_API_OnBluetoothDeviceFind(OHOS::Bluetooth::OnBluetoothDeviceFind);
TH_EXPORT_CPP_API_OffBluetoothDeviceFind(OHOS::Bluetooth::OffBluetoothDeviceFind);
TH_EXPORT_CPP_API_OnBondStateChange(OHOS::Bluetooth::OnBondStateChange);
TH_EXPORT_CPP_API_OffBondStateChange(OHOS::Bluetooth::OffBondStateChange);
TH_EXPORT_CPP_API_OnPinRequired(OHOS::Bluetooth::OnPinRequired);
TH_EXPORT_CPP_API_OffPinRequired(OHOS::Bluetooth::OffPinRequired);
TH_EXPORT_CPP_API_GetRemoteProductId(OHOS::Bluetooth::GetRemoteProductId);
TH_EXPORT_CPP_API_GetRemoteDeviceName(OHOS::Bluetooth::GetRemoteDeviceName);
TH_EXPORT_CPP_API_GetRemoteDeviceNameWithAlias(OHOS::Bluetooth::GetRemoteDeviceNameWithAlias);
TH_EXPORT_CPP_API_GetPairedDevices(OHOS::Bluetooth::GetPairedDevices);
TH_EXPORT_CPP_API_SetBluetoothScanMode(OHOS::Bluetooth::SetBluetoothScanMode);
TH_EXPORT_CPP_API_GetRemoteDeviceClass(OHOS::Bluetooth::GetRemoteDeviceClass);
// NOLINTEND