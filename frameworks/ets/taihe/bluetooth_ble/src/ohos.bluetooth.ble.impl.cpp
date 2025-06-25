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

#include "ohos.bluetooth.ble.proj.hpp"
#include "ohos.bluetooth.ble.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"

#include "bluetooth_ble_advertiser.h"
#include "bluetooth_ble_central_manager.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_gatt_client.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_utils.h"
#include "bluetooth_log.h"

#include "taihe_bluetooth_ble_advertise_callback.h"
#include "taihe_bluetooth_gatt_client_callback.h"
#include "taihe_bluetooth_gatt_server_callback.h"
#include "taihe_bluetooth_ble_utils.h"
#include "taihe_bluetooth_ble_central_manager_callback.h"

using namespace taihe;
using namespace ohos::bluetooth::ble;

#ifndef ANI_BT_ASSERT_RETURN
#define ANI_BT_ASSERT_RETURN(cond, errCode, errMsg)     \
do {                                                     \
    if (!(cond)) {                                      \
        set_business_error(errCode, errMsg);            \
        HILOGE("bluetoothManager ani assert failed.");  \
        return;                                         \
    }                                                   \
} while (0)
#endif

namespace {

std::shared_ptr<OHOS::Bluetooth::BleAdvertiser> BleAdvertiserGetInstance(void)
{
    static auto instance = OHOS::Bluetooth::BleAdvertiser::CreateInstance();
    return instance;
}

OHOS::Bluetooth::BleCentralManager *BleCentralManagerGetInstance(void)
{
    static OHOS::Bluetooth::BleCentralManager
        instance(OHOS::Bluetooth::TaiheBluetoothBleCentralManagerCallback::GetInstance());
    return &instance;
}

class GattClientDeviceImpl {
public:
    explicit GattClientDeviceImpl(string_view deviceId)
    {
        HILOGI("enter");
        std::string remoteAddr = std::string(deviceId);
        device_ = std::make_shared<OHOS::Bluetooth::BluetoothRemoteDevice>(remoteAddr, 1);
        client_ = std::make_shared<OHOS::Bluetooth::GattClient>(*device_);
        client_->Init();
        callback_ = std::make_shared<OHOS::Bluetooth::TaiheGattClientCallback>();
        callback_->SetDeviceAddr(remoteAddr);
    }
    ~GattClientDeviceImpl() = default;
    
    void SetBLEMtuSize(double mtu)
    {
        ANI_BT_ASSERT_RETURN(client_ != nullptr, OHOS::Bluetooth::BT_ERR_INTERNAL_ERROR,
            "SetBLEMtuSize ani assert failed");

        int ret = client_->RequestBleMtuSize(mtu);
        HILOGI("ret: %{public}d", ret);
        ANI_BT_ASSERT_RETURN(ret == OHOS::Bluetooth::BT_NO_ERROR, ret, "SetBLEMtuSize return error");
    }

    void Connect()
    {
        ANI_BT_ASSERT_RETURN(client_ != nullptr, OHOS::Bluetooth::BT_ERR_INTERNAL_ERROR,
            "Connect ani assert failed");
        ANI_BT_ASSERT_RETURN(callback_ != nullptr, OHOS::Bluetooth::BT_ERR_INTERNAL_ERROR,
            "Connect ani assert failed");

        int ret = client_->Connect(callback_, false, OHOS::Bluetooth::GATT_TRANSPORT_TYPE_LE);
        HILOGI("ret: %{public}d", ret);
        ANI_BT_ASSERT_RETURN(ret == OHOS::Bluetooth::BT_NO_ERROR, ret, "Connect return error");
    }

    void Disconnect()
    {
        ANI_BT_ASSERT_RETURN(client_ != nullptr, OHOS::Bluetooth::BT_ERR_INTERNAL_ERROR,
            "Disconnect ani assert failed");

        int ret = client_->Disconnect();
        HILOGI("ret: %{public}d", ret);
        ANI_BT_ASSERT_RETURN(ret == OHOS::Bluetooth::BT_NO_ERROR, ret, "Disconnect return error");
    }

    void Close()
    {
        ANI_BT_ASSERT_RETURN(client_ != nullptr, OHOS::Bluetooth::BT_ERR_INTERNAL_ERROR, "Close ani assert failed");
        int ret = client_->Close();
        HILOGI("ret: %{public}d", ret);

        ANI_BT_ASSERT_RETURN(ret == OHOS::Bluetooth::BT_NO_ERROR, ret, "Close return error");
    }
private:
    std::shared_ptr<OHOS::Bluetooth::GattClient> client_ = nullptr;
    std::shared_ptr<OHOS::Bluetooth::TaiheGattClientCallback> callback_;
    std::shared_ptr<OHOS::Bluetooth::BluetoothRemoteDevice> device_ = nullptr;
};

class GattServerImpl {
public:
    GattServerImpl()
    {
        HILOGI("enter");
        callback_ = std::make_shared<OHOS::Bluetooth::TaiheGattServerCallback>();
        std::shared_ptr<OHOS::Bluetooth::GattServerCallback> tmp =
            std::static_pointer_cast<OHOS::Bluetooth::GattServerCallback>(callback_);
        server_  = OHOS::Bluetooth::GattServer::CreateInstance(tmp);
    }
    ~GattServerImpl() = default;

    void Close()
    {
        ANI_BT_ASSERT_RETURN(server_ != nullptr, OHOS::Bluetooth::BT_ERR_INTERNAL_ERROR, "Close ani assert failed");
        int ret = server_->Close();
        HILOGI("ret: %{public}d", ret);

        ANI_BT_ASSERT_RETURN(ret == OHOS::Bluetooth::BT_NO_ERROR, ret, "Close return error");
    }
private:
    std::shared_ptr<OHOS::Bluetooth::GattServer> server_ = nullptr;
    std::shared_ptr<OHOS::Bluetooth::TaiheGattServerCallback> callback_;
};

class BleScannerImpl {
public:
    BleScannerImpl()
    {
        callback_ = std::make_shared<OHOS::Bluetooth::TaiheBluetoothBleCentralManagerCallback>(true);
        bleCentralManager_ = std::make_shared<OHOS::Bluetooth::BleCentralManager>(callback_);
    }
    ~BleScannerImpl() = default;

private:
    std::shared_ptr<OHOS::Bluetooth::BleCentralManager> bleCentralManager_ = nullptr;
    std::shared_ptr<OHOS::Bluetooth::TaiheBluetoothBleCentralManagerCallback> callback_ = nullptr;
};

void StopAdvertising()
{
    std::shared_ptr<OHOS::Bluetooth::BleAdvertiser> bleAdvertiser = BleAdvertiserGetInstance();
    ANI_BT_ASSERT_RETURN(bleAdvertiser != nullptr, OHOS::Bluetooth::BT_ERR_INTERNAL_ERROR,
        "bleAdvertiser ani assert failed");

    std::vector<std::shared_ptr<OHOS::Bluetooth::BleAdvertiseCallback>> callbacks = bleAdvertiser->GetAdvObservers();
    if (callbacks.empty()) {
        int ret = bleAdvertiser->StopAdvertising(OHOS::Bluetooth::TaiheBluetoothBleAdvertiseCallback::GetInstance());
        ANI_BT_ASSERT_RETURN(ret == OHOS::Bluetooth::BT_NO_ERROR, ret, "StopAdvertising return error");
    }
}

void StopBLEScan()
{
    int ret = BleCentralManagerGetInstance()->StopScan();
    ANI_BT_ASSERT_RETURN(ret == OHOS::Bluetooth::BT_NO_ERROR, ret, "StopBLEScan return error");
}

GattClientDevice CreateGattClientDevice(string_view deviceId)
{
    std::string remoteAddr = std::string(deviceId);
    return make_holder<GattClientDeviceImpl, GattClientDevice>(remoteAddr);
}

GattServer CreateGattServer()
{
    return make_holder<GattServerImpl, GattServer>();
}

BleScanner CreateBleScanner()
{
    return make_holder<BleScannerImpl, BleScanner>();
}
}  // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_CreateGattClientDevice(CreateGattClientDevice);
TH_EXPORT_CPP_API_CreateGattServer(CreateGattServer);
TH_EXPORT_CPP_API_CreateBleScanner(CreateBleScanner);
TH_EXPORT_CPP_API_StopAdvertising(StopAdvertising);
TH_EXPORT_CPP_API_StopBLEScan(StopBLEScan);
// NOLINTEND