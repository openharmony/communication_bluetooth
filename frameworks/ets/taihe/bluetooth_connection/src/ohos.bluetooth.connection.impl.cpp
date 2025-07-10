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

#include "ohos.bluetooth.connection.proj.hpp"
#include "ohos.bluetooth.connection.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "bluetooth_remote_device.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"

using namespace taihe;
using namespace ohos::bluetooth::connection;

namespace {

string GetRemoteProductId(string_view deviceId)
{
    HILOGI("enter");
    std::string remoteAddr = std::string(deviceId);
    OHOS::Bluetooth::BluetoothRemoteDevice remoteDevice = OHOS::Bluetooth::BluetoothRemoteDevice(remoteAddr);
    std::string productId = "";
    int32_t err = remoteDevice.GetDeviceProductId(productId);
    if (err != OHOS::Bluetooth::BT_NO_ERROR) {
        set_business_error(err, "GetRemoteProductId return error");
    }
    return productId;
}

string GetRemoteDeviceName(string_view deviceId)
{
    HILOGI("enter");
    std::string remoteAddr = std::string(deviceId);
    std::string name = OHOS::Bluetooth::INVALID_NAME;
    bool deviceAlias = false;

    OHOS::Bluetooth::BluetoothRemoteDevice remoteDevice = OHOS::Bluetooth::BluetoothRemoteDevice(remoteAddr);
    int32_t err = remoteDevice.GetDeviceName(name, deviceAlias);
    if (err != OHOS::Bluetooth::BT_NO_ERROR) {
        set_business_error(err, "GetRemoteDeviceName return error");
    }
    return name;
}

string GetRemoteDeviceNameWithAlias(string_view deviceId, optional_view<bool> alias)
{
    HILOGI("enter");
    std::string remoteAddr = std::string(deviceId);
    std::string name = OHOS::Bluetooth::INVALID_NAME;
    bool deviceAlias = static_cast<bool>(alias);
 
    OHOS::Bluetooth::BluetoothRemoteDevice remoteDevice = OHOS::Bluetooth::BluetoothRemoteDevice(remoteAddr);
    int32_t err = remoteDevice.GetDeviceName(name, deviceAlias);
    if (err != OHOS::Bluetooth::BT_NO_ERROR) {
        set_business_error(err, "GetRemoteDeviceNameWithAlias return error");
    }
    return name;
}

array<string> GetPairedDevices()
{
    HILOGI("enter");
    OHOS::Bluetooth::BluetoothHost *host = &OHOS::Bluetooth::BluetoothHost::GetDefaultHost();
    std::vector<OHOS::Bluetooth::BluetoothRemoteDevice> remoteDeviceLists;
    std::vector<std::string> dstDevicesvec;
    int32_t err = host->GetPairedDevices(OHOS::Bluetooth::BT_TRANSPORT_BREDR, remoteDeviceLists);
    if (err != OHOS::Bluetooth::BT_NO_ERROR) {
        set_business_error(err, "GetPairedDevices return error");
        return {};
    }
    for (auto vec : remoteDeviceLists) {
        dstDevicesvec.push_back(vec.GetDeviceAddr().c_str());
    }
    array<string> result(taihe::copy_data_t{}, dstDevicesvec.data(), dstDevicesvec.size());
    return result;
}

void SetBluetoothScanMode(ScanMode mode, double duration)
{
    HILOGI("enter");
    OHOS::Bluetooth::BluetoothHost *host = &OHOS::Bluetooth::BluetoothHost::GetDefaultHost();
    int32_t bondableMode = 1;
    int32_t err = host->SetBtScanMode(mode, duration);
    if (err != OHOS::Bluetooth::BT_NO_ERROR) {
        set_business_error(err, "SetBluetoothScanMode return error");
        return;
    }
    host->SetBondableMode(OHOS::Bluetooth::BT_TRANSPORT_BREDR, bondableMode);
}

DeviceClass GetRemoteDeviceClass(string_view deviceId)
{
    HILOGI("enter");
    std::string remoteAddr = std::string(deviceId);
    OHOS::Bluetooth::BluetoothRemoteDevice remoteDevice = OHOS::Bluetooth::BluetoothRemoteDevice(remoteAddr);
    int tmpCod = ohos::bluetooth::constant::MajorClass(
        ohos::bluetooth::constant::MajorClass::key_t::MAJOR_UNCATEGORIZED).get_value();
    int tmpMajorClass = ohos::bluetooth::constant::MajorClass(
        ohos::bluetooth::constant::MajorClass::key_t::MAJOR_UNCATEGORIZED).get_value();
    int tmpMajorMinorClass = ohos::bluetooth::constant::MajorMinorClass(
        ohos::bluetooth::constant::MajorMinorClass::key_t::COMPUTER_UNCATEGORIZED).get_value();
    int32_t err = remoteDevice.GetDeviceProductType(tmpCod, tmpMajorClass, tmpMajorMinorClass);
    if (err != OHOS::Bluetooth::BT_NO_ERROR) {
        set_business_error(err, "GetRemoteDeviceClass return error");
    }
    return {ohos::bluetooth::constant::MajorClass::from_value(tmpMajorClass),
        ohos::bluetooth::constant::MajorMinorClass::from_value(tmpMajorMinorClass), tmpCod};
}
}  // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_GetRemoteProductId(GetRemoteProductId);
TH_EXPORT_CPP_API_GetRemoteDeviceName(GetRemoteDeviceName);
TH_EXPORT_CPP_API_GetRemoteDeviceNameWithAlias(GetRemoteDeviceNameWithAlias);
TH_EXPORT_CPP_API_GetPairedDevices(GetPairedDevices);
TH_EXPORT_CPP_API_SetBluetoothScanMode(SetBluetoothScanMode);
TH_EXPORT_CPP_API_GetRemoteDeviceClass(GetRemoteDeviceClass);
// NOLINTEND