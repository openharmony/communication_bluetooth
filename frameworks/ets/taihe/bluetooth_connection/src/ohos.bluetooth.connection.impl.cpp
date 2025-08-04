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
#define LOG_TAG "bt_connection_impl_ohbluetooth"
#endif

#include "ohos.bluetooth.connection.proj.hpp"
#include "ohos.bluetooth.connection.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "bluetooth_remote_device.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "taihe_bluetooth_connection_callback.h"

namespace OHOS {
namespace Bluetooth {

using namespace taihe;
using namespace ohos::bluetooth::connection;
using namespace ohos::bluetooth::constant;

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

void controlDeviceAction(ControlDeviceActionParams controlDeviceActionParams)
{
    HILOGD("enter");
    std::string deviceId = std::string(controlDeviceActionParams.deviceId);
    uint32_t controlType = (controlDeviceActionParams.type).get_value();
    uint32_t controlTypeVal = (controlDeviceActionParams.typeValue).get_value();
    uint32_t controlObject = controlDeviceActionParams.controlObject;
    OHOS::Bluetooth::BluetoothRemoteDevice remoteDevice = OHOS::Bluetooth::BluetoothRemoteDevice(deviceId);
    int32_t err = remoteDevice.ControlDeviceAction(controlType, controlTypeVal, controlObject);
    if (err != OHOS::Bluetooth::BT_NO_ERROR) {
        set_business_error(err, "controlDeviceAction return error");
    }
}

int getLastConnectionTime(string_view deviceId)
{
    HILOGD("enter");
    std::string remoteAddr = std::string(deviceId);
    int64_t connectionTime = 0;
    OHOS::Bluetooth::BluetoothRemoteDevice remoteDevice = OHOS::Bluetooth::BluetoothRemoteDevice(remoteAddr);
    int32_t err = remoteDevice.GetLastConnectionTime(connectionTime);
    if (err != OHOS::Bluetooth::BT_NO_ERROR) {
        set_business_error(err, "getLastConnectionTime return error");
    }
    return static_cast<int>(connectionTime);
}

ohos::bluetooth::connection::DeviceType getRemoteDeviceType(string_view deviceId)
{
    HILOGD("enter");
    std::string remoteAddr = std::string(deviceId);
    int32_t deviceType = 0;
    OHOS::Bluetooth::BluetoothRemoteDevice remoteDevice = OHOS::Bluetooth::BluetoothRemoteDevice(remoteAddr);
    int32_t err = remoteDevice.GetDeviceCustomType(deviceType);
    HILOGI("err: %{public}d", err);
    if (err != OHOS::Bluetooth::BT_NO_ERROR) {
        set_business_error(err, "getRemoteDeviceType return error");
    }

    return ohos::bluetooth::connection::DeviceType::from_value(deviceType);
}

void updateCloudBluetoothDevice(TrustedPairedDevices trustedPairedDevices)
{
    HILOGI("enter");
    std::vector<OHOS::Bluetooth::TrustPairDeviceParam> trustPairs {};
    for (const auto &device : trustedPairedDevices.trustedPairedDevices) {
        OHOS::Bluetooth::TrustPairDeviceParam trustPair;
        trustPair.sn_ = std::string(device.sn);
        trustPair.deviceType_ = std::string(device.deviceType);
        trustPair.modelId_ = std::string(device.modelId);
        trustPair.manufactory_ = std::string(device.manufactory);
        trustPair.productId_ = std::string(device.productId);
        trustPair.hiLinkVersion_ = std::string(device.hiLinkVersion);
        trustPair.macAddress_ = std::string(device.macAddress);
        trustPair.serviceType_ = std::string(device.serviceType);
        trustPair.serviceId_ = std::string(device.serviceId);
        trustPair.deviceName_ = std::string(device.deviceName);
        trustPair.uuids_.clear();
        trustPair.bluetoothClass_ = device.bluetoothClass;
        trustPair.token_.clear();
        for (const auto &token : device.token) {
            trustPair.token_.push_back(static_cast<uint8_t>(token));
        }
        trustPair.deviceNameTime_ = device.deviceNameTime;
        trustPair.secureAdvertisingInfo_.clear();
        for (const auto &info : device.secureAdvertisingInfo) {
            trustPair.secureAdvertisingInfo_.push_back(static_cast<uint8_t>(info));
        }
        trustPair.pairState_ = device.pairState;
        trustPairs.push_back(trustPair);
    }
    int32_t err = OHOS::Bluetooth::BluetoothHost::GetDefaultHost().UpdateCloudBluetoothDevice(trustPairs);
    HILOGI("err: %{public}d", err);
    if (err != OHOS::Bluetooth::BT_NO_ERROR) {
        set_business_error(err, "updateCloudBluetoothDevice return error");
    }
}

BatteryInfo getRemoteDeviceBatteryInfo(string_view deviceId)
{
    HILOGD("enter getRemoteDeviceBatteryInfo");
    std::string remoteAddr = std::string(deviceId);
    OHOS::Bluetooth::DeviceBatteryInfo batteryInfo;
    OHOS::Bluetooth::BluetoothRemoteDevice remoteDevice = OHOS::Bluetooth::BluetoothRemoteDevice(remoteAddr);
    int32_t err = remoteDevice.GetRemoteDeviceBatteryInfo(batteryInfo);
    HILOGI("err: %{public}d", err);
    if (err != OHOS::Bluetooth::BT_NO_ERROR) {
        set_business_error(err, "getRemoteDeviceBatteryInfo return error");
    }

    int tmpCod = MajorClass(MajorClass::key_t::MAJOR_UNCATEGORIZED).get_value();
    int tmpMajorClass = MajorClass(
        MajorClass::key_t::MAJOR_UNCATEGORIZED).get_value();
    int tmpMajorMinorClass = MajorMinorClass(MajorMinorClass::key_t::COMPUTER_UNCATEGORIZED).get_value();
    DeviceClass deviceClass = {
        MajorClass::from_value(tmpMajorClass),
        MajorMinorClass::from_value(tmpMajorMinorClass),
        tmpCod
    };

    BatteryInfo batterys{"1", 1, 2, 
            DeviceChargeState::key_t::DEVICE_NORMAL_CHARGE_NOT_CHARGED,
            3, deviceClass,
            DeviceChargeState::key_t::DEVICE_SUPER_CHARGE_NOT_CHARGED,
            4, DeviceChargeState::key_t::DEVICE_SUPER_CHARGE_IN_CHARGING};
    batterys.deviceId = batteryInfo.deviceId_;
    batterys.batteryLevel = batteryInfo.batteryLevel_;
    batterys.leftEarBatteryLevel = batteryInfo.leftEarBatteryLevel_;
    batterys.leftEarChargeState = (DeviceChargeState::key_t)batteryInfo.leftEarChargeState_;
    batterys.rightEarBatteryLevel = batteryInfo.rightEarBatteryLevel_;
    batterys.rightEarChargeState = (DeviceChargeState::key_t)batteryInfo.rightEarChargeState_;
    batterys.boxBatteryLevel = batteryInfo.boxBatteryLevel_;
    batterys.boxChargeState = (DeviceChargeState::key_t)batteryInfo.boxChargeState_;
    return batterys;
}

void setRemoteDeviceName(string_view deviceId, string_view name)
{
    HILOGD("GetAutoPlayDisabledDuration");
    std::string remoteAddr = static_cast<std::string>(deviceId);
    std::string deviceName = static_cast<std::string>(name);
    OHOS::Bluetooth::BluetoothRemoteDevice remoteDevice = OHOS::Bluetooth::BluetoothRemoteDevice(remoteAddr);
    int32_t err = remoteDevice.SetDeviceAlias(deviceName);
    if (err != OHOS::Bluetooth::BT_NO_ERROR) {
        set_business_error(err, "setRemoteDeviceName return error");
    }
}

void setRemoteDeviceType(string_view deviceId, DeviceType type)
{
    HILOGD("enter");
    std::string remoteAddr = std::string(deviceId);
    int32_t deviceType = type.get_value();
    OHOS::Bluetooth::BluetoothRemoteDevice remoteDevice = OHOS::Bluetooth::BluetoothRemoteDevice(remoteAddr);
    int32_t err = remoteDevice.SetDeviceCustomType(deviceType);
    HILOGD("err: %{public}d", err);
    if (err != OHOS::Bluetooth::BT_NO_ERROR) {
        set_business_error(err, "getPairState return error");
    }
}

ScanMode getBluetoothScanMode()
{
    HILOGD("enter");
    OHOS::Bluetooth::BluetoothHost *host = &OHOS::Bluetooth::BluetoothHost::GetDefaultHost();
    int32_t scanMode = 0;
    int32_t err = host->GetBtScanMode(scanMode);
    if (err != OHOS::Bluetooth::BT_NO_ERROR) {
        set_business_error(err, "getPairState return error");
    }
    return ScanMode::from_value(scanMode);
}

ProfileConnectionState getProfileConnectionState(optional<ProfileId> profileId)
{
    uint32_t conProfileId = 0;
    int state = 0;
    OHOS::Bluetooth::BluetoothHost *host = &OHOS::Bluetooth::BluetoothHost::GetDefaultHost();
    host->GetBtProfileConnState(conProfileId, state);
    return ProfileConnectionState::from_value(state);
}

string getLocalName()
{
    HILOGD("enter");
    OHOS::Bluetooth::BluetoothHost *host = &OHOS::Bluetooth::BluetoothHost::GetDefaultHost();
    std::string localName = OHOS::Bluetooth::INVALID_NAME;
    int32_t err = host->GetLocalName(localName);
    if (err != OHOS::Bluetooth::BT_NO_ERROR) {
        set_business_error(err, "getLocalName return error");
    }
    return localName;
}

void DealPairStatus(const int &status, int &bondStatus)
{
    HILOGD("status is %{public}d", status);
    switch (status) {
        case OHOS::Bluetooth::PAIR_NONE:
            bondStatus = static_cast<int>(BondState::key_t::BOND_STATE_INVALID);
            break;
        case OHOS::Bluetooth::PAIR_PAIRING:
            bondStatus = static_cast<int>(BondState::key_t::BOND_STATE_BONDING);
            break;
        case OHOS::Bluetooth::PAIR_PAIRED:
            bondStatus = static_cast<int>(BondState::key_t::BOND_STATE_BONDED);
            break;
        default:
            break;
    }
}

BondState getPairState(string_view deviceId)
{
    HILOGD("enter");
    std::string remoteAddr = std::string(deviceId);
    OHOS::Bluetooth::BluetoothRemoteDevice remoteDevice = OHOS::Bluetooth::BluetoothRemoteDevice(remoteAddr);
    int state = OHOS::Bluetooth::PAIR_NONE;
    int32_t err = remoteDevice.GetPairState(state);
    if (err != OHOS::Bluetooth::BT_NO_ERROR) {
        set_business_error(err, "getPairState return error");
    }
    int pairState = static_cast<int>(BondState::key_t::BOND_STATE_INVALID);
    DealPairStatus(state, pairState);
    return BondState::from_value(pairState);
}

void startBluetoothDiscovery()
{
    HILOGD("enter");
    OHOS::Bluetooth::BluetoothHost *host = &OHOS::Bluetooth::BluetoothHost::GetDefaultHost();
    int ret = host->StartBtDiscovery();
    if (ret != OHOS::Bluetooth::BT_NO_ERROR) {
        set_business_error(ret, "startBluetoothDiscovery return error");
    }
}

bool isBluetoothDiscovering()
{
    HILOGD("enter");
    OHOS::Bluetooth::BluetoothHost *host = &OHOS::Bluetooth::BluetoothHost::GetDefaultHost();
    bool isDiscovering = false;
    int32_t err = host->IsBtDiscovering(isDiscovering);
    if (err != OHOS::Bluetooth::BT_NO_ERROR) {
        set_business_error(err, "startBluetoothDiscovery return error");
    }
    return isDiscovering;
}

void stopBluetoothDiscovery()
{
    HILOGD("enter");
    OHOS::Bluetooth::BluetoothHost *host = &OHOS::Bluetooth::BluetoothHost::GetDefaultHost();
    int ret = host->CancelBtDiscovery();
    if (ret != OHOS::Bluetooth::BT_NO_ERROR) {
        set_business_error(ret, "stopBluetoothDiscovery return error");
    }
}

void setDevicePairingConfirmation(string_view deviceId, bool accept)
{
    HILOGD("enter");
    std::string remoteAddr = std::string(deviceId);
    OHOS::Bluetooth::BluetoothRemoteDevice remoteDevice = OHOS::Bluetooth::BluetoothRemoteDevice(remoteAddr);
    int32_t ret = OHOS::Bluetooth::BT_NO_ERROR;
    if (accept) {
        ret = remoteDevice.SetDevicePairingConfirmation(accept);
    } else {
        ret = remoteDevice.CancelPairing();
    }
    HILOGD("ret: %{public}d", ret);
    if (ret != OHOS::Bluetooth::BT_NO_ERROR) {
        set_business_error(ret, "setDevicePairingConfirmation return error");
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
TH_EXPORT_CPP_API_controlDeviceAction(OHOS::Bluetooth::controlDeviceAction);
TH_EXPORT_CPP_API_getLastConnectionTime(OHOS::Bluetooth::getLastConnectionTime);
TH_EXPORT_CPP_API_getRemoteDeviceType(OHOS::Bluetooth::getRemoteDeviceType);
TH_EXPORT_CPP_API_updateCloudBluetoothDevice(OHOS::Bluetooth::updateCloudBluetoothDevice);
TH_EXPORT_CPP_API_getRemoteDeviceBatteryInfo(OHOS::Bluetooth::getRemoteDeviceBatteryInfo);
TH_EXPORT_CPP_API_setRemoteDeviceName(OHOS::Bluetooth::setRemoteDeviceName);
TH_EXPORT_CPP_API_setRemoteDeviceType(OHOS::Bluetooth::setRemoteDeviceType);
TH_EXPORT_CPP_API_getBluetoothScanMode(OHOS::Bluetooth::getBluetoothScanMode);
TH_EXPORT_CPP_API_getProfileConnectionState(OHOS::Bluetooth::getProfileConnectionState);
TH_EXPORT_CPP_API_getLocalName(OHOS::Bluetooth::getLocalName);
TH_EXPORT_CPP_API_getPairState(OHOS::Bluetooth::getPairState);
TH_EXPORT_CPP_API_startBluetoothDiscovery(OHOS::Bluetooth::startBluetoothDiscovery);
TH_EXPORT_CPP_API_isBluetoothDiscovering(OHOS::Bluetooth::isBluetoothDiscovering);
TH_EXPORT_CPP_API_stopBluetoothDiscovery(OHOS::Bluetooth::stopBluetoothDiscovery);
TH_EXPORT_CPP_API_setDevicePairingConfirmation(OHOS::Bluetooth::setDevicePairingConfirmation);
// NOLINTEND