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
#define LOG_TAG "bt_connection_impl_bluetooth"
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
#include "taihe_bluetooth_utils.h"
#include "taihe_event_module.h"

namespace OHOS {
namespace Bluetooth {
std::shared_ptr<TaiheBluetoothRemoteDeviceObserver> g_remoteDeviceObserver =
    TaiheBluetoothRemoteDeviceObserver::GetInstance();
std::shared_ptr<TaiheBluetoothConnectionObserver> g_connectionObserver =
    TaiheBluetoothConnectionObserver::GetInstance();

taihe::string GetRemoteProductId(taihe::string_view deviceId)
{
    HILOGD("start");
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM, deviceId);

    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    std::string productId = "";
    int32_t err = remoteDevice.GetDeviceProductId(productId);
    TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, productId);
    HILOGI("GetRemoteProductId :%{public}s", productId.c_str());
    return productId;
}

taihe::string GetRemoteDeviceName(taihe::string_view deviceId, taihe::optional_view<bool> alias)
{
    HILOGD("start");
    std::string remoteAddr = std::string(deviceId);
    std::string name = INVALID_NAME;
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM, deviceId);

    bool deviceAlias = true;
    if (alias.has_value()) {
        deviceAlias = alias.value();
    }

    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int32_t err = remoteDevice.GetDeviceName(name, deviceAlias);
    TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, name);

    return name;
}

taihe::array<taihe::string> GetPairedDevices()
{
    HILOGD("enter");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    std::vector<BluetoothRemoteDevice> remoteDeviceLists;

    int32_t err = host->GetPairedDevices(BT_TRANSPORT_BREDR, remoteDeviceLists);
    std::vector<std::string> dstDevicesvec;

    for (auto vec : remoteDeviceLists) {
        dstDevicesvec.push_back(vec.GetDeviceAddr().c_str());
    }
    taihe::array<taihe::string> result(taihe::copy_data_t{}, dstDevicesvec.data(), dstDevicesvec.size());
    TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, result);
    HILOGI("end");
    return result;
}

void SetBluetoothScanMode(ohos::bluetooth::connection::ScanMode mode, int duration)
{
    HILOGD("enter");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int32_t ret = host->SetBtScanMode(mode, duration);
    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);

    int32_t bondableMode = 1;
    host->SetBondableMode(BT_TRANSPORT_BREDR, bondableMode);
}

ohos::bluetooth::connection::DeviceClass GetRemoteDeviceClass(taihe::string_view deviceId)
{
    HILOGD("start");
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    ohos::bluetooth::connection::DeviceClass deviceClass = {
        ohos::bluetooth::constant::MajorClass::key_t::MAJOR_UNCATEGORIZED,
        ohos::bluetooth::constant::MajorMinorClass::key_t::COMPUTER_UNCATEGORIZED,
        0
    };
    TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM, deviceClass);

    int tmpCod = ohos::bluetooth::constant::MajorClass(
        ohos::bluetooth::constant::MajorClass::key_t::MAJOR_UNCATEGORIZED).get_value();
    int tmpMajorClass = ohos::bluetooth::constant::MajorClass(
        ohos::bluetooth::constant::MajorClass::key_t::MAJOR_UNCATEGORIZED).get_value();
    int tmpMajorMinorClass = ohos::bluetooth::constant::MajorMinorClass(
        ohos::bluetooth::constant::MajorMinorClass::key_t::COMPUTER_UNCATEGORIZED).get_value();
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int32_t err = remoteDevice.GetDeviceProductType(tmpCod, tmpMajorClass, tmpMajorMinorClass);
    deviceClass = {
        ohos::bluetooth::constant::MajorClass::from_value(tmpMajorClass),
        ohos::bluetooth::constant::MajorMinorClass::from_value(tmpMajorMinorClass),
        tmpCod
    };
    TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, deviceClass);

    return deviceClass;
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

void ControlDeviceAction(ohos::bluetooth::connection::ControlDeviceActionParams controlDeviceActionParams)
{
    HILOGD("ControlDeviceAction enter");
    std::string deviceId = std::string(controlDeviceActionParams.deviceId);
    bool checkRet = CheckDeviceIdParam(deviceId);
    TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

    uint32_t controlType = (controlDeviceActionParams.type).get_value();
    uint32_t controlTypeVal = (controlDeviceActionParams.typeValue).get_value();
    uint32_t controlObject = (controlDeviceActionParams.controlObject).get_value();
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(deviceId);
    int32_t err = remoteDevice.ControlDeviceAction(controlType, controlTypeVal, controlObject);
    HILOGI("ControlDeviceAction err: %{public}d", err);
    TAIHE_BT_ASSERT_RETURN_VOID(err == BT_NO_ERROR, err);
}

int64_t GetLastConnectionTime(taihe::string_view deviceId)
{
    HILOGD("enter");
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM, 0);

    int64_t connectionTime = 0;
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int32_t err = remoteDevice.GetLastConnectionTime(connectionTime);
    HILOGI("GetRemoteDeviceConnectionTime GetLastConnectionTime err: %{public}d", err);
    TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, connectionTime);

    return connectionTime;
}

ohos::bluetooth::connection::DeviceType GetRemoteDeviceType(taihe::string_view deviceId)
{
    HILOGD("enter");
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM, ohos::bluetooth::connection::DeviceType::from_value(0));

    int32_t deviceType = 0;
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int32_t err = remoteDevice.GetDeviceCustomType(deviceType);
    HILOGI("GetRemoteDeviceType err: %{public}d", err);
    TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, ohos::bluetooth::connection::DeviceType::from_value(deviceType));

    return ohos::bluetooth::connection::DeviceType::from_value(deviceType);
}

void UpdateCloudBluetoothDevice(ohos::bluetooth::connection::TrustedPairedDevices trustedPairedDevices)
{
    HILOGI("[CLOUD_DEV] UpdateCloudBluetoothDevice enter");
    std::vector<TrustPairDeviceParam> trustPairs {};
    for (const auto &device : trustedPairedDevices.trustedPairedDevices) {
        TrustPairDeviceParam trustPair;
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
        std::string uuidStr = device.uuids.c_str();
        auto first = uuidStr.begin();
        while (first != uuidStr.end()) {
            auto last = std::find(first, uuidStr.end(), ',');
            std::string uuid(first, last);
            if (!uuid.empty()) {
                trustPair.uuids_.push_back(uuid);
            }
            if (last == uuidStr.end()) {
                break;
            }
            first = last + 1;
        }
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
    int32_t err = BluetoothHost::GetDefaultHost().UpdateCloudBluetoothDevice(trustPairs);
    HILOGI("[CLOUD_DEV] UpdateCloudBluetoothDevice err: %{public}d", err);
    TAIHE_BT_ASSERT_RETURN_VOID(err == BT_NO_ERROR, err);
}

ohos::bluetooth::connection::BatteryInfo GetRemoteDeviceBatteryInfo(taihe::string_view deviceId)
{
    HILOGD("enter");
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    ohos::bluetooth::connection::BatteryInfo batterys {"1", 1, 2,
        ohos::bluetooth::connection::DeviceChargeState(
            ohos::bluetooth::connection::DeviceChargeState::key_t::DEVICE_NORMAL_CHARGE_NOT_CHARGED),
        3,
        ohos::bluetooth::connection::DeviceChargeState(
            ohos::bluetooth::connection::DeviceChargeState::key_t::DEVICE_SUPER_CHARGE_NOT_CHARGED),
        4,
        ohos::bluetooth::connection::DeviceChargeState(
            ohos::bluetooth::connection::DeviceChargeState::key_t::DEVICE_SUPER_CHARGE_IN_CHARGING)
    };
    TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM, batterys);

    DeviceBatteryInfo batteryInfo;
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int32_t err = remoteDevice.GetRemoteDeviceBatteryInfo(batteryInfo);
    HILOGI("err: %{public}d", err);
    TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, batterys);

    int tmpCod = ohos::bluetooth::constant::MajorClass(
        ohos::bluetooth::constant::MajorClass::key_t::MAJOR_UNCATEGORIZED).get_value();
    int tmpMajorClass = ohos::bluetooth::constant::MajorClass(
        ohos::bluetooth::constant::MajorClass::key_t::MAJOR_UNCATEGORIZED).get_value();
    int tmpMajorMinorClass = ohos::bluetooth::constant::MajorMinorClass(
        ohos::bluetooth::constant::MajorMinorClass::key_t::COMPUTER_UNCATEGORIZED).get_value();
    remoteDevice.GetDeviceProductType(tmpCod, tmpMajorClass, tmpMajorMinorClass);

    batterys.deviceId = batteryInfo.deviceId_;
    batterys.batteryLevel = batteryInfo.batteryLevel_;
    batterys.leftEarBatteryLevel = batteryInfo.leftEarBatteryLevel_;
    batterys.leftEarChargeState =
        ohos::bluetooth::connection::DeviceChargeState::from_value(static_cast<int>(batteryInfo.leftEarChargeState_));
    batterys.rightEarBatteryLevel = batteryInfo.rightEarBatteryLevel_;
    batterys.rightEarChargeState =
        ohos::bluetooth::connection::DeviceChargeState::from_value(static_cast<int>(batteryInfo.rightEarChargeState_));
    batterys.boxBatteryLevel = batteryInfo.boxBatteryLevel_;
    batterys.boxChargeState =
        ohos::bluetooth::connection::DeviceChargeState::from_value(static_cast<int>(batteryInfo.boxChargeState_));

    return batterys;
}

void SetRemoteDeviceName(taihe::string_view deviceId, taihe::string_view name)
{
    HILOGD("enter");
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

    std::string deviceName = static_cast<std::string>(name);
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int32_t err = remoteDevice.SetDeviceAlias(deviceName);
    HILOGI("SetDeviceName err: %{public}d", err);
    TAIHE_BT_ASSERT_RETURN_VOID(err == BT_NO_ERROR, err);
}

void SetRemoteDeviceType(taihe::string_view deviceId, ohos::bluetooth::connection::DeviceType type)
{
    HILOGD("enter");
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

    int32_t deviceType = type.get_value();
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int32_t err = remoteDevice.SetDeviceCustomType(deviceType);
    HILOGI("SetRemoteDeviceType err: %{public}d", err);
    TAIHE_BT_ASSERT_RETURN_VOID(err == BT_NO_ERROR, err);
}

ohos::bluetooth::connection::ScanMode GetBluetoothScanMode()
{
    HILOGD("enter");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int32_t scanMode = 0;
    int32_t err = host->GetBtScanMode(scanMode);
    TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, ohos::bluetooth::connection::ScanMode::from_value(scanMode));
    HILOGI("end");
    return ohos::bluetooth::connection::ScanMode::from_value(scanMode);
}

ohos::bluetooth::constant::ProfileConnectionState GetProfileConnectionState(
    taihe::optional<ohos::bluetooth::constant::ProfileId> profileId)
{
    HILOGD("enter");
    int state = static_cast<int>(BTConnectState::DISCONNECTED);

    if (profileId.has_value()) {
        uint32_t conProfileId = profileId.value().get_value();
        BluetoothHost *host = &BluetoothHost::GetDefaultHost();
        int32_t err = host->GetBtProfileConnState(conProfileId, state);
        TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err,
            ohos::bluetooth::constant::ProfileConnectionState::from_value(state));

        state = TaiheUtils::GetProfileConnectionState(state);
    } else {
        BluetoothHost *host = &BluetoothHost::GetDefaultHost();
        int32_t err = host->GetBtConnectionState(state);
        TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err,
            ohos::bluetooth::constant::ProfileConnectionState::from_value(state));
    }

    return ohos::bluetooth::constant::ProfileConnectionState::from_value(state);
}

taihe::string GetLocalName()
{
    HILOGD("enter");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    std::string localName = INVALID_NAME;
    int32_t err = host->GetLocalName(localName);
    TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, localName);
    HILOGI("end");
    return localName;
}

void DealPairStatus(const int &status, int &bondStatus)
{
    HILOGD("status is %{public}d", status);
    switch (status) {
        case PAIR_NONE:
            bondStatus = ohos::bluetooth::connection::BondState(
                ohos::bluetooth::connection::BondState::key_t::BOND_STATE_INVALID).get_value();
            break;
        case PAIR_PAIRING:
            bondStatus = ohos::bluetooth::connection::BondState(
                ohos::bluetooth::connection::BondState::key_t::BOND_STATE_BONDING).get_value();
            break;
        case PAIR_PAIRED:
            bondStatus = ohos::bluetooth::connection::BondState(
                ohos::bluetooth::connection::BondState::key_t::BOND_STATE_BONDED).get_value();
            break;
        default:
            break;
    }
}

ohos::bluetooth::connection::BondState GetPairState(taihe::string_view deviceId)
{
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM, ohos::bluetooth::connection::BondState::from_value(0));

    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int state = PAIR_NONE;
    int32_t err = remoteDevice.GetPairState(state);
    int pairState = ohos::bluetooth::connection::BondState(
        ohos::bluetooth::connection::BondState::key_t::BOND_STATE_INVALID).get_value();
    DealPairStatus(state, pairState);
    TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, ohos::bluetooth::connection::BondState::from_value(pairState));
    HILOGI("getPairState :%{public}d", pairState);

    return ohos::bluetooth::connection::BondState::from_value(pairState);
}

void StartBluetoothDiscovery()
{
    HILOGD("enter");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int ret = host->StartBtDiscovery();
    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
}

bool IsBluetoothDiscovering()
{
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    bool isDiscovering = false;
    int32_t err = host->IsBtDiscovering(isDiscovering);
    TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, isDiscovering);
    HILOGE("isBluetoothDiscovering :%{public}d", isDiscovering);

    return isDiscovering;
}

void StopBluetoothDiscovery()
{
    HILOGD("enter");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int ret = host->CancelBtDiscovery();
    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
}

void SetDevicePairingConfirmation(taihe::string_view deviceId, bool accept)
{
    HILOGD("enter");
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_ASSERT_RETURN_VOID(checkRet, BT_ERR_INVALID_PARAM);

    HILOGI("SetDevicePairingConfirmation::accept = %{public}d", accept);
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr);
    int32_t ret = BT_NO_ERROR;
    if (accept) {
        ret = remoteDevice.SetDevicePairingConfirmation(accept);
    } else {
        ret = remoteDevice.CancelPairing();
    }
    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
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
TH_EXPORT_CPP_API_GetPairedDevices(OHOS::Bluetooth::GetPairedDevices);
TH_EXPORT_CPP_API_SetBluetoothScanMode(OHOS::Bluetooth::SetBluetoothScanMode);
TH_EXPORT_CPP_API_GetRemoteDeviceClass(OHOS::Bluetooth::GetRemoteDeviceClass);
TH_EXPORT_CPP_API_ControlDeviceAction(OHOS::Bluetooth::ControlDeviceAction);
TH_EXPORT_CPP_API_GetLastConnectionTime(OHOS::Bluetooth::GetLastConnectionTime);
TH_EXPORT_CPP_API_GetRemoteDeviceType(OHOS::Bluetooth::GetRemoteDeviceType);
TH_EXPORT_CPP_API_UpdateCloudBluetoothDevice(OHOS::Bluetooth::UpdateCloudBluetoothDevice);
TH_EXPORT_CPP_API_GetRemoteDeviceBatteryInfo(OHOS::Bluetooth::GetRemoteDeviceBatteryInfo);
TH_EXPORT_CPP_API_SetRemoteDeviceName(OHOS::Bluetooth::SetRemoteDeviceName);
TH_EXPORT_CPP_API_SetRemoteDeviceType(OHOS::Bluetooth::SetRemoteDeviceType);
TH_EXPORT_CPP_API_GetBluetoothScanMode(OHOS::Bluetooth::GetBluetoothScanMode);
TH_EXPORT_CPP_API_GetProfileConnectionState(OHOS::Bluetooth::GetProfileConnectionState);
TH_EXPORT_CPP_API_GetLocalName(OHOS::Bluetooth::GetLocalName);
TH_EXPORT_CPP_API_GetPairState(OHOS::Bluetooth::GetPairState);
TH_EXPORT_CPP_API_StartBluetoothDiscovery(OHOS::Bluetooth::StartBluetoothDiscovery);
TH_EXPORT_CPP_API_IsBluetoothDiscovering(OHOS::Bluetooth::IsBluetoothDiscovering);
TH_EXPORT_CPP_API_StopBluetoothDiscovery(OHOS::Bluetooth::StopBluetoothDiscovery);
TH_EXPORT_CPP_API_SetDevicePairingConfirmation(OHOS::Bluetooth::SetDevicePairingConfirmation);
// NOLINTEND