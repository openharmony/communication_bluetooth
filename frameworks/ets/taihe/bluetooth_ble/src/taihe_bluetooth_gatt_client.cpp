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
#define LOG_TAG "bt_taihe_gatt_client"
#endif

#include "taihe_bluetooth_gatt_client.h"

#include "bluetooth_host.h"
#include "bluetooth_utils.h"
#include "taihe_bluetooth_ble_utils.h"
#include "taihe_bluetooth_error.h"
#include "taihe_bluetooth_utils.h"
#include "taihe_parser_utils.h"

namespace OHOS {
namespace Bluetooth {
const std::vector<std::pair<int, int>> GattClientDeviceImpl::g_gattStatusSrvToNapi = {
    { Bluetooth::BT_NO_ERROR,                                 GATT_SUCCESS },
    { Bluetooth::BT_ERR_GATT_WRITE_NOT_PERMITTED,             WRITE_NOT_PERMITTED },
    { Bluetooth::BT_ERR_GATT_READ_NOT_PERMITTED,              READ_NOT_PERMITTED },
    { Bluetooth::BT_ERR_GATT_CONNECTION_CONGESTED,            GATT_CONGESTION },
    { Bluetooth::BT_ERR_GATT_CONNECTION_NOT_ENCRYPTED,        INSUFFICIENT_ENCRYPTION },
    { Bluetooth::BT_ERR_GATT_CONNECTION_NOT_AUTHENTICATED,    AUTHENTICATION_FAILED },
    { Bluetooth::BT_ERR_GATT_CONNECTION_NOT_AUTHORIZED,       INSUFFICIENT_AUTHORIZATION },
};
void GattClientDeviceImpl::SetBLEMtuSize(int mtu)
{
    HILOGI("enter");
    TAIHE_BT_ASSERT_RETURN_VOID(client_ != nullptr, BT_ERR_INTERNAL_ERROR);

    int ret = client_->RequestBleMtuSize(mtu);
    HILOGI("ret: %{public}d", ret);
    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
}
void GattClientDeviceImpl::Connect()
{
    HILOGI("enter");
    TAIHE_BT_ASSERT_RETURN_VOID(client_ != nullptr, BT_ERR_INTERNAL_ERROR);
    TAIHE_BT_ASSERT_RETURN_VOID(callback_ != nullptr, BT_ERR_INTERNAL_ERROR);

    int ret = client_->Connect(callback_, false, GATT_TRANSPORT_TYPE_LE);
    HILOGI("ret: %{public}d", ret);
    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
}

void GattClientDeviceImpl::Disconnect()
{
    HILOGI("enter");
    TAIHE_BT_ASSERT_RETURN_VOID(client_ != nullptr, BT_ERR_INTERNAL_ERROR);

    int ret = client_->Disconnect();
    HILOGI("ret: %{public}d", ret);
    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
}

void GattClientDeviceImpl::Close()
{
    HILOGI("enter");
    TAIHE_BT_ASSERT_RETURN_VOID(client_ != nullptr, BT_ERR_INTERNAL_ERROR);
    int ret = client_->Close();
    HILOGI("ret: %{public}d", ret);

    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
}

taihe::string GattClientDeviceImpl::GetDeviceNameSync()
{
    HILOGD("start");
    std::string deviceAddr = "";
    if (GetDevice()) {
        deviceAddr = GetDevice()->GetDeviceAddr();
    }

    std::string deviceName = "";
    int32_t err = BluetoothHost::GetDefaultHost().GetRemoteDevice(
        deviceAddr, BT_TRANSPORT_BLE).GetDeviceName(deviceName);
    TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, deviceName);

    HILOGI("err: %{public}d, deviceName: %{public}s", err, deviceName.c_str());

    return deviceName;
}

static GattCharacteristic *GetCharacteristic(const std::shared_ptr<GattClient> &client,
    const UUID &serviceUuid, const UUID &characterUuid)
{
    GattCharacteristic *character = nullptr;
    if (client) {
        auto service = client->GetService(serviceUuid);
        if (service.has_value()) {
            character = service->get().GetCharacteristic(characterUuid);
        }
    }
    return character;
}

static GattCharacteristic *FindCharacteristic(std::vector<GattService> &service,
    const TaiheBleCharacteristic &napiCharacter)
{
    GattCharacteristic *character = nullptr;
    for (auto &svc : service) {
        if (svc.GetUuid().Equals(napiCharacter.serviceUuid)) {
            character = svc.GetCharacteristic(napiCharacter.characteristicValueHandle);
            if (character && character->GetUuid().Equals(napiCharacter.characteristicUuid)) {
                return character;
            }
        }
    }
    return nullptr;
}

static GattCharacteristic *GetCharacteristic(const std::shared_ptr<GattClient> &client,
    const TaiheBleCharacteristic &napiCharacter)
{
    if (client) {
        if (napiCharacter.characteristicValueHandle > 0) {
            std::vector<GattService> &services = client->GetService();
            return FindCharacteristic(services, napiCharacter);
        } else {
            GattCharacteristic *character = GetCharacteristic(client, napiCharacter.serviceUuid,
                napiCharacter.characteristicUuid);
            return character;
        }
    }
    return nullptr;
}

static GattCharacteristic *GetGattcCharacteristic(const std::shared_ptr<GattClient> &client,
    const TaiheBleCharacteristic &napiCharacter)
{
    GattCharacteristic *character = GetCharacteristic(client, napiCharacter);
    if (character) {
        character->SetValue(napiCharacter.characteristicValue.data(), napiCharacter.characteristicValue.size());
    }
    return character;
}

static GattDescriptor *GetGattcDescriptor(const std::shared_ptr<GattClient> &client,
    const TaiheBleDescriptor &napiDescriptor)
{
    GattDescriptor *descriptor = nullptr;
    if (client) {
        auto *character = GetCharacteristic(client, napiDescriptor.serviceUuid, napiDescriptor.characteristicUuid);
        if (character == nullptr) {
            HILOGE("character is nullptr");
            return nullptr;
        }
        descriptor = character->GetDescriptor(napiDescriptor.descriptorUuid);
        if (descriptor) {
            descriptor->SetValue(napiDescriptor.descriptorValue.data(), napiDescriptor.descriptorValue.size());
        }
    }
    return descriptor;
}

static taihe_status ParseGattClientReadCharacteristicValue(::ohos::bluetooth::ble::BLECharacteristic const &characteristic,
    GattClientDeviceImpl **outGattClient, GattCharacteristic **outCharacter)
{
    GattClientDeviceImpl *gattClient = *outGattClient;
    TAIHE_BT_RETURN_IF(gattClient == nullptr || outGattClient == nullptr, "gattClient is nullptr.", taihe_invalid_arg);

    TaiheBleCharacteristic taiheCharacter;
    TAIHE_BT_CALL_RETURN(TaiheParseGattBLECharacteristic(characteristic, taiheCharacter));

    GattCharacteristic *character = GetGattcCharacteristic(gattClient->GetClient(), taiheCharacter);
    TAIHE_BT_RETURN_IF(character == nullptr || outCharacter == nullptr, "Not found character", taihe_invalid_arg);

    *outGattClient = gattClient;
    *outCharacter = character;
    return taihe_ok;
}

static TaihePromiseAndCallback TaiheReadCharacteristicValue(
    ::ohos::bluetooth::ble::BLECharacteristic const &characteristic, uintptr_t cb,
    GattClientDeviceImpl* client, bool asPromise = true)
{
    HILOGI("enter");
    GattCharacteristic *character = nullptr;
    auto status = ParseGattClientReadCharacteristicValue(characteristic, &client, &character);
    if ((status != taihe_ok) && (client == nullptr) && (character == nullptr)) {
        return TaihePromiseAndCallback::Failure(BT_ERR_INVALID_PARAM);
    }
    if (client->GetCallback() == nullptr) {
        return TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR);
    }

    auto func = [gattClient = client->GetClient(), character]() {
        if (character == nullptr) {
            HILOGE("character is nullptr");
            return TaiheAsyncWorkRet(BT_ERR_INTERNAL_ERROR);
        }
        int ret = BT_ERR_INTERNAL_ERROR;
        if (gattClient) {
            ret = gattClient->ReadCharacteristic(*character);
            ret = GetSDKAdaptedStatusCode(GattClientDeviceImpl::GattStatusFromService(ret)); // Adaptation for old sdk
        }
        return TaiheAsyncWorkRet(ret);
    };
    auto asyncWork = TaiheAsyncWorkFactory::CreateAsyncWork(taihe::get_env(), reinterpret_cast<ani_object>(cb), func);
    if (asyncWork == nullptr) {
        return TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR);
    }
    bool success = client->GetCallback()->asyncWorkMap_.TryPush(TaiheAsyncType::GATT_CLIENT_READ_CHARACTER, asyncWork);
    if (!success) {
        return TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR);
    }

    asyncWork->Run();

    if (asPromise) {
        return TaihePromiseAndCallback::Success(reinterpret_cast<uintptr_t>(asyncWork->GetRet()));
    } else {
        return TaihePromiseAndCallback::Success(reinterpret_cast<uintptr_t>(nullptr));
    }
}

uintptr_t GattClientDeviceImpl::ReadCharacteristicValuePromise(
    ::ohos::bluetooth::ble::BLECharacteristic const &characteristic)
{
    TaihePromiseAndCallback result = TaiheReadCharacteristicValue(characteristic, reinterpret_cast<uintptr_t>(nullptr),
        this);
    if (!result.success || !result.handle.has_value()) {
        TAIHE_BT_ASSERT_RETURN(false, result.errorCode, reinterpret_cast<uintptr_t>(nullptr));
    }
    return result.handle.value();
}

void GattClientDeviceImpl::ReadCharacteristicValueAsync(::ohos::bluetooth::ble::BLECharacteristic const &characteristic,
    uintptr_t callback)
{
    TaihePromiseAndCallback result = TaiheReadCharacteristicValue(characteristic, callback, this, false);
    if (!result.success) {
        TAIHE_BT_ASSERT_RETURN_VOID(false, result.errorCode);
    }
}

static taihe_status ParseGattClientReadDescriptorValue(::ohos::bluetooth::ble::BLEDescriptor const &bleDescriptor,
    GattClientDeviceImpl **outGattClient, GattDescriptor **outDescriptor)
{
    GattClientDeviceImpl *gattClient = *outGattClient; // TaiheGetGattClient();
    TAIHE_BT_RETURN_IF(outGattClient == nullptr || gattClient == nullptr, "gattClient is nullptr.", taihe_invalid_arg);

    TaiheBleDescriptor taiheDescriptor;
    TAIHE_BT_CALL_RETURN(TaiheParseGattBLEDescriptor(bleDescriptor, taiheDescriptor));
    GattDescriptor *descriptor = GetGattcDescriptor(gattClient->GetClient(), taiheDescriptor);
    TAIHE_BT_RETURN_IF(outDescriptor == nullptr || descriptor == nullptr, "Not found Descriptor", taihe_invalid_arg);

    *outGattClient = gattClient;
    *outDescriptor = descriptor;
    return taihe_ok;
}

static TaihePromiseAndCallback TaiheReadDescriptorValue(::ohos::bluetooth::ble::BLEDescriptor const &bleDescriptor,
    uintptr_t cb, GattClientDeviceImpl* client, bool asPromise = true)
{
    HILOGI("enter");
    GattDescriptor *descriptor = nullptr;
    auto status = ParseGattClientReadDescriptorValue(bleDescriptor, &client, &descriptor);
    if ((status == taihe_ok) && (client == nullptr) && (descriptor == nullptr)) {
        return TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR);
    }
    if (client->GetCallback() == nullptr) {
        return TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR);
    }

    auto func = [gattClient = client->GetClient(), descriptor]() {
        if (descriptor == nullptr) {
            HILOGE("descriptor is nullptr");
            return TaiheAsyncWorkRet(BT_ERR_INTERNAL_ERROR);
        }
        int ret = BT_ERR_INTERNAL_ERROR;
        if (gattClient) {
            ret = gattClient->ReadDescriptor(*descriptor);
            ret = GetSDKAdaptedStatusCode(GattClientDeviceImpl::GattStatusFromService(ret)); // Adaptation for old sdk
        }
        return TaiheAsyncWorkRet(ret);
    };
    auto asyncWork = TaiheAsyncWorkFactory::CreateAsyncWork(taihe::get_env(), reinterpret_cast<ani_object>(cb), func);
    if (asyncWork == nullptr) {
        return TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR);
    }
    bool success = client->GetCallback()->asyncWorkMap_.TryPush(TaiheAsyncType::GATT_CLIENT_READ_DESCRIPTOR, asyncWork);
    if (!success) {
        return TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR);
    }

    asyncWork->Run();

    if (asPromise) {
        return TaihePromiseAndCallback::Success(reinterpret_cast<uintptr_t>(asyncWork->GetRet()));
    } else {
        return TaihePromiseAndCallback::Success(reinterpret_cast<uintptr_t>(nullptr));
    }
}

uintptr_t GattClientDeviceImpl::ReadDescriptorValuePromise(::ohos::bluetooth::ble::BLEDescriptor const &bleDescriptor)
{
    TaihePromiseAndCallback result = TaiheReadDescriptorValue(bleDescriptor, reinterpret_cast<uintptr_t>(nullptr), this);
    if (!result.success || !result.handle.has_value()) {
        TAIHE_BT_ASSERT_RETURN(false, result.errorCode, reinterpret_cast<uintptr_t>(nullptr));
    }
    return result.handle.value();
}

void GattClientDeviceImpl::ReadDescriptorValueAsync(::ohos::bluetooth::ble::BLEDescriptor const &bleDescriptor, uintptr_t callback)
{
    TaihePromiseAndCallback result = TaiheReadDescriptorValue(bleDescriptor, callback, this, false);
    if (!result.success) {
        TAIHE_BT_ASSERT_RETURN_VOID(false, result.errorCode);
    }
}

static taihe_status CheckSetCharacteristicChange(::ohos::bluetooth::ble::BLECharacteristic const& characteristic,
    GattCharacteristic **outCharacteristic, GattClientDeviceImpl **outGattClient)
{
    GattClientDeviceImpl *gattClient = *outGattClient; // NapiGetGattClient(env, thisVar);
    TAIHE_BT_RETURN_IF(gattClient == nullptr || outGattClient == nullptr, "gattClient is nullptr.", taihe_invalid_arg);

    TaiheBleCharacteristic taiheCharacter;
    TAIHE_BT_CALL_RETURN(TaiheParseGattBLECharacteristic(characteristic, taiheCharacter));
    GattCharacteristic *character = GetGattcCharacteristic(gattClient->GetClient(), taiheCharacter);
    TAIHE_BT_RETURN_IF(character == nullptr || outCharacteristic == nullptr, "Not found character", taihe_invalid_arg);

    *outGattClient = gattClient;
    *outCharacteristic = character;
    return taihe_ok;
}

static TaihePromiseAndCallback TaiheSetCharacteristicChangeIndication(
    ::ohos::bluetooth::ble::BLECharacteristic const &characteristic, bool enable, uintptr_t cb,
    GattClientDeviceImpl *client, bool asPromise = true)
{
    HILOGI("enter");
    GattCharacteristic *character = nullptr;

    auto status = CheckSetCharacteristicChange(characteristic, &character, &client);
    if ((status != taihe_ok) && (client == nullptr) && (character == nullptr)) {
        return TaihePromiseAndCallback::Failure(BT_ERR_INVALID_PARAM);
    }
    if (client->GetCallback() == nullptr) {
        return TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR);
    }

    bool isNotify = false;
    auto func = [gattClient = client->GetClient(), character, enable, isNotify]() {
        if (character == nullptr) {
            HILOGE("character is nullptr");
            return TaiheAsyncWorkRet(BT_ERR_INTERNAL_ERROR);
        }
        int ret = BT_ERR_INTERNAL_ERROR;
        if (gattClient) {
            if (isNotify) {
                ret = gattClient->SetNotifyCharacteristic(*character, enable);
            } else {
                ret = gattClient->SetIndicateCharacteristic(*character, enable);
            }
            ret = GetSDKAdaptedStatusCode(GattClientDeviceImpl::GattStatusFromService(ret)); // Adaptation for old sdk
        }
        return TaiheAsyncWorkRet(ret);
    };
    auto asyncWork = TaiheAsyncWorkFactory::CreateAsyncWork(taihe::get_env(), reinterpret_cast<ani_object>(cb), func);
    if (asyncWork == nullptr) {
        return TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR);
    }
    bool success = client->GetCallback()->asyncWorkMap_.TryPush(GATT_CLIENT_ENABLE_CHARACTER_CHANGED, asyncWork);
    if (!success) {
        return TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR);
    }

    asyncWork->Run();

    if (asPromise) {
        return TaihePromiseAndCallback::Success(reinterpret_cast<uintptr_t>(asyncWork->GetRet()));
    } else {
        return TaihePromiseAndCallback::Success(reinterpret_cast<uintptr_t>(nullptr));
    }
}

uintptr_t GattClientDeviceImpl::SetCharacteristicChangeIndicationPromise(
    ::ohos::bluetooth::ble::BLECharacteristic const &characteristic, bool enable)
{
    TaihePromiseAndCallback result = TaiheSetCharacteristicChangeIndication(characteristic, enable,
        reinterpret_cast<uintptr_t>(nullptr), this);
    if (!result.success || !result.handle.has_value()) {
        TAIHE_BT_ASSERT_RETURN(false, result.errorCode, reinterpret_cast<uintptr_t>(nullptr));
    }
    return result.handle.value();
}

void GattClientDeviceImpl::SetCharacteristicChangeIndicationAsync(
    ::ohos::bluetooth::ble::BLECharacteristic const &characteristic, bool enable, uintptr_t callback)
{
    TaihePromiseAndCallback result = TaiheSetCharacteristicChangeIndication(characteristic, enable, callback,
        this, false);
    if (!result.success) {
        TAIHE_BT_ASSERT_RETURN_VOID(false, result.errorCode);
    }
}

int GattClientDeviceImpl::GattStatusFromService(int status)
{
    // if status is from napi, do not deal with.
    if (status > 0) {
        return status;
    }
    int ret = BT_ERR_INTERNAL_ERROR;
    // statusCode srv -> napi
    auto iter = g_gattStatusSrvToNapi.begin();
    for (; iter != g_gattStatusSrvToNapi.end(); iter++) {
        if (iter->second == status) {
            ret = iter->first; // transfer to napi errorCode.
            break;
        }
    }
    if (iter == g_gattStatusSrvToNapi.end()) {
        HILOGW("Unsupported error code conversion, status: %{public}d", status);
    }
    return ret;
}
} // namespace Bluetooth
} // namespace OHOS
