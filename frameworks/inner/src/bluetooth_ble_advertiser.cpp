/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_fwk_ble_advertiser"
#endif

#include "bluetooth_ble_advertiser.h"

#include "bluetooth_ble_advertise_callback_stub.h"
#include "bluetooth_def.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_observer_map.h"
#include "bluetooth_utils.h"
#include "i_bluetooth_ble_advertiser.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "securec.h"

#include <memory>
#include "bluetooth_profile_manager.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;
struct BleAdvertiser::impl {
    impl();
    ~impl();
    void Init(std::weak_ptr<BleAdvertiser> advertiser);
    void ConvertBleAdvertiserData(const BleAdvertiserData &data, BluetoothBleAdvertiserData &outData);
    uint32_t GetAdvertiserTotalBytes(const BluetoothBleAdvertiserData &data, bool isFlagsIncluded);
    int32_t CheckAdvertiserSettings(const BleAdvertiserSettings &settings);
    int32_t CheckAdvertiserData(const BluetoothBleAdvertiserSettings &setting,
        const BluetoothBleAdvertiserData &advData, const BluetoothBleAdvertiserData &scanResponse);

    class BluetoothBleAdvertiserCallbackImp : public BluetoothBleAdvertiseCallbackStub {
    public:
        explicit BluetoothBleAdvertiserCallbackImp(std::weak_ptr<BleAdvertiser> advertiser)
            : advertiser_(advertiser){};
        ~BluetoothBleAdvertiserCallbackImp()
        {}

        __attribute__((no_sanitize("cfi")))
        void OnStartResultEvent(int32_t result, int32_t advHandle, int32_t opcode) override
        {
            std::shared_ptr<BleAdvertiser> advertiserSptr = advertiser_.lock();
            CHECK_AND_RETURN_LOG(advertiserSptr, "BleAdvertiser is destructed");

            HILOGD("result: %{public}d, advHandle: %{public}d, opcode: %{public}d", result, advHandle, opcode);
            std::shared_ptr<BleAdvertiseCallback> observer = nullptr;
            if (opcode == bluetooth::BLE_ADV_START_FAILED_OP_CODE) {
                observer = advertiserSptr->pimpl->callbacks_.PopAdvertiserObserver(advHandle);
            } else {
                observer = advertiserSptr->pimpl->callbacks_.GetAdvertiserObserver(advHandle);
            }

            if (observer != nullptr) {
                observer->OnStartResultEvent(result, advHandle);
            }
        }

        void OnEnableResultEvent(int32_t result, int32_t advHandle) override
        {
            std::shared_ptr<BleAdvertiser> advertiserSptr = advertiser_.lock();
            CHECK_AND_RETURN_LOG(advertiserSptr, "BleAdvertiser is destructed");

            HILOGI("result: %{public}d, advHandle: %{public}d", result, advHandle);
            auto observer = advertiserSptr->pimpl->callbacks_.GetAdvertiserObserver(advHandle);
            if (observer != nullptr) {
                observer->OnEnableResultEvent(result, advHandle);
            }
        }

        void OnDisableResultEvent(int32_t result, int32_t advHandle) override
        {
            std::shared_ptr<BleAdvertiser> advertiserSptr = advertiser_.lock();
            CHECK_AND_RETURN_LOG(advertiserSptr, "BleAdvertiser is destructed");

            HILOGI("result: %{public}d, advHandle: %{public}d", result, advHandle);
            auto observer = advertiserSptr->pimpl->callbacks_.GetAdvertiserObserver(advHandle);
            if (observer != nullptr) {
                observer->OnDisableResultEvent(result, advHandle);
            }
        }

        __attribute__((no_sanitize("cfi")))
        void OnStopResultEvent(int32_t result, int32_t advHandle) override
        {
            std::shared_ptr<BleAdvertiser> advertiserSptr = advertiser_.lock();
            CHECK_AND_RETURN_LOG(advertiserSptr, "BleAdvertiser is destructed");

            HILOGD("result: %{public}d, advHandle: %{public}d", result, advHandle);
            auto observer = advertiserSptr->pimpl->callbacks_.PopAdvertiserObserver(advHandle);
            if (observer != nullptr) {
                observer->OnStopResultEvent(result, advHandle);
            }
        }

        void OnAutoStopAdvEvent(int32_t advHandle) override
        {
            std::shared_ptr<BleAdvertiser> advertiserSptr = advertiser_.lock();
            CHECK_AND_RETURN_LOG(advertiserSptr, "BleAdvertiser is destructed");

            HILOGI("advHandle: %{public}d", advHandle);
            auto observer = advertiserSptr->pimpl->callbacks_.GetAdvertiserObserver(advHandle);
            if (observer != nullptr) {
                advertiserSptr->pimpl->callbacks_.Deregister(observer);
            }
        }

        void OnSetAdvDataEvent(int32_t result, int32_t advHandle) override
        {
            std::shared_ptr<BleAdvertiser> advertiserSptr = advertiser_.lock();
            CHECK_AND_RETURN_LOG(advertiserSptr, "BleAdvertiser is destructed");

            HILOGD("result: %{public}d, advHandle: %{public}d", result, advHandle);
            auto observer = advertiserSptr->pimpl->callbacks_.GetAdvertiserObserver(advHandle);
            if (observer) {
                observer->OnSetAdvDataEvent(result);
            }
        }

        void OnChangeAdvResultEvent(int32_t result, int32_t advHandle) override
        {
            std::shared_ptr<BleAdvertiser> advertiserSptr = advertiser_.lock();
            CHECK_AND_RETURN_LOG(advertiserSptr, "BleAdvertiser is destructed");

            HILOGI("result: %{public}d, advHandle: %{public}d", result, advHandle);
            auto observer = advertiserSptr->pimpl->callbacks_.GetAdvertiserObserver(advHandle);
            if (observer != nullptr) {
                observer->OnChangeAdvResultEvent(result, advHandle);
            }
        }

    private:
        std::weak_ptr<BleAdvertiser> advertiser_;
        BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothBleAdvertiserCallbackImp);
    };
    sptr<BluetoothBleAdvertiserCallbackImp> callbackImp_ = nullptr;

    BluetoothObserverMap<std::shared_ptr<BleAdvertiseCallback>> callbacks_;
    class BleAdvertiserDeathRecipient;
    sptr<BleAdvertiserDeathRecipient> deathRecipient_ = nullptr;
    int32_t profileRegisterId = 0;
};

class BleAdvertiser::impl::BleAdvertiserDeathRecipient final : public IRemoteObject::DeathRecipient {
public:
    explicit BleAdvertiserDeathRecipient(std::weak_ptr<BleAdvertiser> advertiser) : advertiser_(advertiser) {}
    ~BleAdvertiserDeathRecipient() final = default;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BleAdvertiserDeathRecipient);

    void OnRemoteDied(const wptr<IRemoteObject> &remote) final
    {
        HILOGI("enter");

        std::shared_ptr<BleAdvertiser> advertiserSptr = advertiser_.lock();
        CHECK_AND_RETURN_LOG(advertiserSptr, "BleAdvertiser is destructed");
        advertiserSptr->pimpl->callbacks_.Clear();
    }

private:
    std::weak_ptr<BleAdvertiser> advertiser_;
};

BleAdvertiser::impl::impl()
{}

BleAdvertiser::impl::~impl()
{
    HILOGD("start");
    BluetoothProfileManager::GetInstance().DeregisterFunc(profileRegisterId);
    callbacks_.Clear();
    sptr<IBluetoothBleAdvertiser> proxy = GetRemoteProxy<IBluetoothBleAdvertiser>(BLE_ADVERTISER_SERVER);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
    proxy->DeregisterBleAdvertiserCallback(callbackImp_);
    proxy->AsObject()->RemoveDeathRecipient(deathRecipient_);
}

BleAdvertiser::BleAdvertiser() : pimpl(nullptr)
{
    if (pimpl == nullptr) {
        pimpl = std::make_unique<impl>();
        if (!pimpl) {
            HILOGE("failed, no pimpl");
        }
    }

    HILOGI("successful");
}

BleAdvertiser::~BleAdvertiser()
{}

void BleAdvertiser::impl::Init(std::weak_ptr<BleAdvertiser> advertiser)
{
    callbackImp_ = new BluetoothBleAdvertiserCallbackImp(advertiser);
    profileRegisterId = BluetoothProfileManager::GetInstance().RegisterFunc(BLE_ADVERTISER_SERVER,
        [this, advertiser](sptr<IRemoteObject> remote) {
        sptr<IBluetoothBleAdvertiser> proxy = iface_cast<IBluetoothBleAdvertiser>(remote);
        CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
        proxy->RegisterBleAdvertiserCallback(callbackImp_);
        deathRecipient_ = new BleAdvertiserDeathRecipient(advertiser);
        if (deathRecipient_ != nullptr) {
            proxy->AsObject()->AddDeathRecipient(deathRecipient_);
        }
    });
}

std::shared_ptr<BleAdvertiser> BleAdvertiser::CreateInstance(void)
{
    auto instance = std::make_shared<BleAdvertiser>(PassKey());
    CHECK_AND_RETURN_LOG_RET(instance, nullptr, "Create BleAdvertiser failed");

    instance->pimpl->Init(instance);
    return instance;
}

void BleAdvertiser::impl::ConvertBleAdvertiserData(const BleAdvertiserData &data, BluetoothBleAdvertiserData &outData)
{
    std::map<uint16_t, std::string> manufacturerData = data.GetManufacturerData();
    for (auto iter = manufacturerData.begin(); iter != manufacturerData.end(); iter++) {
        outData.AddManufacturerData(iter->first, iter->second);
    }
    std::map<ParcelUuid, std::string> serviceData = data.GetServiceData();
    for (auto it = serviceData.begin(); it != serviceData.end(); it++) {
        outData.AddServiceData(Uuid::ConvertFromString(it->first.ToString()), it->second);
    }
    std::vector<ParcelUuid> serviceUuids = data.GetServiceUuids();
    for (auto it = serviceUuids.begin(); it != serviceUuids.end(); it++) {
        outData.AddServiceUuid(Uuid::ConvertFromString(it->ToString()));
    }
    outData.SetIncludeDeviceName(data.GetIncludeDeviceName());
    outData.SetIncludeTxPower(data.GetIncludeTxPower());
}

uint32_t BleAdvertiser::impl::GetAdvertiserTotalBytes(const BluetoothBleAdvertiserData &data, bool isFlagsIncluded)
{
    // If the flag field is contained, the protocol stack adds the flag field, which consists of three bytes.
    uint32_t size = (isFlagsIncluded) ? BLE_ADV_FLAGS_FIELD_LENGTH : 0;
    std::map<uint16_t, std::string> manufacturerData = data.GetManufacturerData();
    for (auto iter = manufacturerData.begin(); iter != manufacturerData.end(); ++iter) {
        size += BLE_ADV_PER_FIELD_OVERHEAD_LENGTH + BLE_ADV_MANUFACTURER_ID_LENGTH + iter->second.length();
    }

    std::map<Uuid, std::string> serviceData = data.GetServiceData();
    for (auto iter = serviceData.begin(); iter != serviceData.end(); ++iter) {
        size += BLE_ADV_PER_FIELD_OVERHEAD_LENGTH + static_cast<uint32_t>(iter->first.GetUuidType())
        + iter->second.length();
    }
    int num16BitUuids = 0;
    int num32BitUuids = 0;
    int num128BitUuids = 0;
    std::vector<Uuid> serviceUuids = data.GetServiceUuids();
    for (auto iter = serviceUuids.begin(); iter != serviceUuids.end(); ++iter) {
        if (iter->GetUuidType() == Uuid::UUID16_BYTES_TYPE) {
            ++num16BitUuids;
        } else if (iter->GetUuidType() == Uuid::UUID32_BYTES_TYPE) {
            ++num32BitUuids;
        } else {
            ++num128BitUuids;
        }
    }
    if (num16BitUuids != 0) {
        size += BLE_ADV_PER_FIELD_OVERHEAD_LENGTH + static_cast<uint32_t>(num16BitUuids * Uuid::UUID16_BYTES_TYPE);
    }
    if (num32BitUuids != 0) {
        size += BLE_ADV_PER_FIELD_OVERHEAD_LENGTH + static_cast<uint32_t>(num32BitUuids * Uuid::UUID32_BYTES_TYPE);
    }
    if (num128BitUuids != 0) {
        size += BLE_ADV_PER_FIELD_OVERHEAD_LENGTH + static_cast<uint32_t>(num128BitUuids * Uuid::UUID128_BYTES_TYPE);
    }
    if (data.GetIncludeDeviceName()) {
        uint32_t deviceNameLen = BluetoothHost::GetDefaultHost().GetLocalName().length();
        deviceNameLen = (deviceNameLen > DEVICE_NAME_MAX_LEN) ?  DEVICE_NAME_MAX_LEN : deviceNameLen;
        size += BLE_ADV_PER_FIELD_OVERHEAD_LENGTH + deviceNameLen;
    }
    if (data.GetIncludeTxPower()) {
        uint32_t txPowerLen = 3;
        size += BLE_ADV_PER_FIELD_OVERHEAD_LENGTH + txPowerLen;
    }
    return size;
}

int32_t BleAdvertiser::impl::CheckAdvertiserSettings(const BleAdvertiserSettings &settings)
{
    int8_t txPower = settings.GetTxPower();
    if (txPower < BLE_ADV_TX_POWER_MIN_VALUE || txPower > BLE_ADV_TX_POWER_MAX_VALUE) {
        HILOGE("Illegal txPower argument %{public}d", txPower);
        return BT_ERR_INVALID_PARAM;
    }

    return BT_NO_ERROR;
}

int32_t BleAdvertiser::impl::CheckAdvertiserData(const BluetoothBleAdvertiserSettings &setting,
    const BluetoothBleAdvertiserData &advData, const BluetoothBleAdvertiserData &scanResponse)
{
    uint32_t maxSize = BLE_ADV_MAX_LEGACY_ADVERTISING_DATA_BYTES;
    if (!setting.IsLegacyMode()) {
        maxSize = static_cast<uint32_t>(BluetoothHost::GetDefaultHost().GetBleMaxAdvertisingDataLength());
    }
    // If is Connect, flag field is contained in AdvertiserData.
    uint32_t size = GetAdvertiserTotalBytes(advData, setting.IsConnectable());
    if (size > maxSize) {
        HILOGE("bleAdvertiserData size = %{public}d, maxSize = %{public}d", size, maxSize);
        return BT_ERR_INTERNAL_ERROR;
    }
    size = GetAdvertiserTotalBytes(scanResponse, false);
    if (size > maxSize) {
        HILOGE("bleScanResponse size = %{public}d, maxSize = %{public}d,", size, maxSize);
        return BT_ERR_INTERNAL_ERROR;
    }
    return BT_NO_ERROR;
}

int BleAdvertiser::StartAdvertising(const BleAdvertiserSettings &settings, const BleAdvertiserData &advData,
    const BleAdvertiserData &scanResponse, uint16_t duration, std::shared_ptr<BleAdvertiseCallback> callback)
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }
    sptr<IBluetoothBleAdvertiser> proxy = GetRemoteProxy<IBluetoothBleAdvertiser>(BLE_ADVERTISER_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    CHECK_AND_RETURN_LOG_RET(callback != nullptr, BT_ERR_INTERNAL_ERROR, "callback is nullptr");

    int ret = pimpl->CheckAdvertiserSettings(settings);
    if (ret != BT_NO_ERROR) {
        return ret;
    }
    BluetoothBleAdvertiserSettings setting;
    setting.SetConnectable(settings.IsConnectable());
    setting.SetInterval(settings.GetInterval());
    setting.SetLegacyMode(settings.IsLegacyMode());
    setting.SetTxPower(settings.GetTxPower());
    setting.SetOwnAddr(settings.GetOwnAddr());
    setting.SetOwnAddrType(settings.GetOwnAddrType());

    BluetoothBleAdvertiserData bleAdvertiserData;
    BluetoothBleAdvertiserData bleScanResponse;
    bleAdvertiserData.SetAdvFlag(advData.GetAdvFlag());
    pimpl->ConvertBleAdvertiserData(advData, bleAdvertiserData);
    pimpl->ConvertBleAdvertiserData(scanResponse, bleScanResponse);

    ret = pimpl->CheckAdvertiserData(setting, bleAdvertiserData, bleScanResponse);
    if (ret != BT_NO_ERROR) {
        return ret;
    }

    HILOGD("duration=%{public}d", duration);
    int32_t advHandle = BLE_INVALID_ADVERTISING_HANDLE;
    if (pimpl->callbacks_.IsExistAdvertiserCallback(callback, advHandle)) {
        ret = proxy->StartAdvertising(setting, bleAdvertiserData, bleScanResponse, advHandle, duration, false);
    } else {
        ret = proxy->GetAdvertiserHandle(advHandle);
        if (ret != BT_NO_ERROR || advHandle == BLE_INVALID_ADVERTISING_HANDLE) {
            HILOGE("Invalid advertising handle");
            callback->OnStartResultEvent(BT_ERR_INTERNAL_ERROR, static_cast<int>(BLE_INVALID_ADVERTISING_HANDLE));
            return ret;
        }
        callback->OnGetAdvHandleEvent(0, advHandle);
        pimpl->callbacks_.Register(advHandle, callback);
        ret = proxy->StartAdvertising(setting, bleAdvertiserData, bleScanResponse, advHandle, duration, false);
    }
    return ret;
}

int BleAdvertiser::StartAdvertising(const BleAdvertiserSettings &settings, const std::vector<uint8_t> &advData,
    const std::vector<uint8_t> &scanResponse, uint16_t duration, std::shared_ptr<BleAdvertiseCallback> callback)
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    sptr<IBluetoothBleAdvertiser> proxy = GetRemoteProxy<IBluetoothBleAdvertiser>(BLE_ADVERTISER_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    CHECK_AND_RETURN_LOG_RET(callback != nullptr, BT_ERR_INTERNAL_ERROR, "callback is nullptr");

    int ret = pimpl->CheckAdvertiserSettings(settings);
    if (ret != BT_NO_ERROR) {
        return ret;
    }
    BluetoothBleAdvertiserSettings setting;
    setting.SetConnectable(settings.IsConnectable());
    setting.SetInterval(settings.GetInterval());
    setting.SetLegacyMode(settings.IsLegacyMode());
    setting.SetTxPower(settings.GetTxPower());
    setting.SetOwnAddr(settings.GetOwnAddr());
    setting.SetOwnAddrType(settings.GetOwnAddrType());

    BluetoothBleAdvertiserData bleAdvertiserData;
    bleAdvertiserData.SetPayload(std::string(advData.begin(), advData.end()));
    bleAdvertiserData.SetAdvFlag(0);
    BluetoothBleAdvertiserData bleScanResponse;
    bleScanResponse.SetPayload(std::string(scanResponse.begin(), scanResponse.end()));

    HILOGI("duration=%{public}d", duration);
    int32_t advHandle = BLE_INVALID_ADVERTISING_HANDLE;
    if (pimpl->callbacks_.IsExistAdvertiserCallback(callback, advHandle)) {
        ret = proxy->StartAdvertising(setting, bleAdvertiserData, bleScanResponse, advHandle, duration, true);
    } else {
        ret = proxy->GetAdvertiserHandle(advHandle);
        if (ret != BT_NO_ERROR || advHandle == BLE_INVALID_ADVERTISING_HANDLE) {
            HILOGE("Invalid advertising handle");
            callback->OnStartResultEvent(BT_ERR_INTERNAL_ERROR, BLE_INVALID_ADVERTISING_HANDLE);
            return ret;
        }
        pimpl->callbacks_.Register(advHandle, callback);
        ret = proxy->StartAdvertising(setting, bleAdvertiserData, bleScanResponse, advHandle, duration, true);
    }
    return ret;
}

void BleAdvertiser::SetAdvertisingData(const std::vector<uint8_t> &advData, const std::vector<uint8_t> &scanResponse,
    std::shared_ptr<BleAdvertiseCallback> callback)
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }
    sptr<IBluetoothBleAdvertiser> proxy = GetRemoteProxy<IBluetoothBleAdvertiser>(BLE_ADVERTISER_SERVER);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");

    CHECK_AND_RETURN_LOG(callback != nullptr, "callback is nullptr");

    int advHandle = BLE_INVALID_ADVERTISING_HANDLE;
    if (!pimpl->callbacks_.IsExistAdvertiserCallback(callback, advHandle)) {
        HILOGE("Advertising is not started");
        return;
    }

    BluetoothBleAdvertiserData bleAdvertiserData;
    bleAdvertiserData.SetPayload(std::string(advData.begin(), advData.end()));
    BluetoothBleAdvertiserData bleScanResponse;
    bleScanResponse.SetPayload(std::string(scanResponse.begin(), scanResponse.end()));
    proxy->SetAdvertisingData(bleAdvertiserData, bleScanResponse, advHandle);
}

int BleAdvertiser::ChangeAdvertisingParams(uint8_t advHandle, const BleAdvertiserSettings &settings)
{
    CHECK_AND_RETURN_LOG_RET(IS_BLE_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    sptr<IBluetoothBleAdvertiser> proxy = GetRemoteProxy<IBluetoothBleAdvertiser>(BLE_ADVERTISER_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");

    int ret = pimpl->CheckAdvertiserSettings(settings);
    if (ret != BT_NO_ERROR) {
        HILOGE("check settings err:%{public}d", ret);
        return ret;
    }
    BluetoothBleAdvertiserSettings setting;
    setting.SetConnectable(settings.IsConnectable());
    setting.SetInterval(settings.GetInterval());
    setting.SetLegacyMode(settings.IsLegacyMode());
    setting.SetTxPower(settings.GetTxPower());
    setting.SetOwnAddr(settings.GetOwnAddr());
    setting.SetOwnAddrType(settings.GetOwnAddrType());
    return proxy->ChangeAdvertisingParams(advHandle, setting);
}

int BleAdvertiser::EnableAdvertising(uint8_t advHandle, uint16_t duration,
    std::shared_ptr<BleAdvertiseCallback> callback)
{
    HILOGI("enter");
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }
    sptr<IBluetoothBleAdvertiser> proxy = GetRemoteProxy<IBluetoothBleAdvertiser>(BLE_ADVERTISER_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");

    CHECK_AND_RETURN_LOG_RET(callback != nullptr, BT_ERR_INTERNAL_ERROR, "callback is nullptr");
    uint8_t tmpAdvHandle = pimpl->callbacks_.GetAdvertiserHandle(callback);
    if (tmpAdvHandle == BLE_INVALID_ADVERTISING_HANDLE) {
        HILOGE("Invalid advertising callback");
        return BT_ERR_INTERNAL_ERROR;
    }

    std::shared_ptr<BleAdvertiseCallback> observer = pimpl->callbacks_.GetAdvertiserObserver(advHandle);
    if (observer == nullptr) {
        HILOGE("Invalid advertising handle");
        return BT_ERR_INTERNAL_ERROR;
    }

    int ret = proxy->EnableAdvertising(advHandle, duration);
    return ret;
}

int BleAdvertiser::DisableAdvertising(uint8_t advHandle, std::shared_ptr<BleAdvertiseCallback> callback)
{
    HILOGI("enter");
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    sptr<IBluetoothBleAdvertiser> proxy = GetRemoteProxy<IBluetoothBleAdvertiser>(BLE_ADVERTISER_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    CHECK_AND_RETURN_LOG_RET(callback != nullptr, BT_ERR_INTERNAL_ERROR, "callback is nullptr");
    uint8_t tmpAdvHandle = pimpl->callbacks_.GetAdvertiserHandle(callback);
    if (tmpAdvHandle == BLE_INVALID_ADVERTISING_HANDLE) {
        HILOGE("Invalid advertising callback");
        return BT_ERR_INTERNAL_ERROR;
    }

    std::shared_ptr<BleAdvertiseCallback> observer = pimpl->callbacks_.GetAdvertiserObserver(advHandle);
    if (observer == nullptr) {
        HILOGE("Invalid advertising handle");
        return BT_ERR_INTERNAL_ERROR;
    }

    int ret = proxy->DisableAdvertising(advHandle);
    return ret;
}

int BleAdvertiser::StopAdvertising(std::shared_ptr<BleAdvertiseCallback> callback)
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    sptr<IBluetoothBleAdvertiser> proxy = GetRemoteProxy<IBluetoothBleAdvertiser>(BLE_ADVERTISER_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    CHECK_AND_RETURN_LOG_RET(callback != nullptr, BT_ERR_INTERNAL_ERROR, "callback is nullptr");

    HILOGD("enter");
    uint8_t advHandle = pimpl->callbacks_.GetAdvertiserHandle(callback);
    if (advHandle == BLE_INVALID_ADVERTISING_HANDLE) {
        HILOGE("Invalid advertising handle");
        return BT_ERR_INTERNAL_ERROR;
    }

    int ret = proxy->StopAdvertising(advHandle);
    return ret;
}

void BleAdvertiser::Close(std::shared_ptr<BleAdvertiseCallback> callback)
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("BLE is not enabled");
        return;
    }

    sptr<IBluetoothBleAdvertiser> proxy = GetRemoteProxy<IBluetoothBleAdvertiser>(BLE_ADVERTISER_SERVER);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "proxy is nullptr");
    CHECK_AND_RETURN_LOG(callback != nullptr, "callback is nullptr");

    HILOGI("enter");
    if (proxy != nullptr) {
        uint8_t advHandle = pimpl->callbacks_.GetAdvertiserHandle(callback);
        if (advHandle != BLE_INVALID_ADVERTISING_HANDLE) {
            proxy->Close(advHandle);
        }

        std::shared_ptr<BleAdvertiseCallback> observer = pimpl->callbacks_.GetAdvertiserObserver(advHandle);
        if (observer != nullptr) {
            pimpl->callbacks_.Deregister(observer);
        }
    }
}

uint8_t BleAdvertiser::GetAdvHandle(std::shared_ptr<BleAdvertiseCallback> callback)
{
    if (!BluetoothHost::GetDefaultHost().IsBleEnabled()) {
        HILOGE("BLE is not enabled");
        return BLE_INVALID_ADVERTISING_HANDLE;
    }
    CHECK_AND_RETURN_LOG_RET(callback != nullptr, BLE_INVALID_ADVERTISING_HANDLE, "callback is nullptr");
    return pimpl->callbacks_.GetAdvertiserHandle(callback);
}

std::shared_ptr<BleAdvertiseCallback> BleAdvertiser::GetAdvObserver(uint32_t advHandle)
{
    if (!BluetoothHost::GetDefaultHost().IsBleEnabled()) {
        HILOGE("BLE is not enabled");
        return nullptr;
    }
    return pimpl->callbacks_.GetAdvertiserObserver(advHandle);
}

std::vector<std::shared_ptr<BleAdvertiseCallback>> BleAdvertiser::GetAdvObservers()
{
    if (!BluetoothHost::GetDefaultHost().IsBleEnabled()) {
        HILOGE("BLE is not enabled");
        return {};
    }
    return pimpl->callbacks_.GetAdvertiserObservers();
}

BleAdvertiserData::BleAdvertiserData()
{}

BleAdvertiserData::~BleAdvertiserData()
{}

void BleAdvertiserData::AddServiceData(const ParcelUuid &uuid, const std::string &serviceData)
{
    if (serviceData.empty()) {
        HILOGE("serviceData is empty");
        return;
    }

    serviceData_.insert(std::make_pair(uuid, serviceData));
}

void BleAdvertiserData::AddManufacturerData(uint16_t manufacturerId, const std::string &data)
{
    if (data.empty()) {
        HILOGE("serviceData is empty");
        return;
    }

    manufacturerSpecificData_.insert(std::make_pair(manufacturerId, data));
}

std::map<uint16_t, std::string> BleAdvertiserData::GetManufacturerData() const
{
    return manufacturerSpecificData_;
}

void BleAdvertiserData::AddServiceUuid(const ParcelUuid &serviceUuid)
{
    serviceUuids_.push_back(serviceUuid);
}

std::vector<ParcelUuid> BleAdvertiserData::GetServiceUuids() const
{
    return serviceUuids_;
}

void BleAdvertiserData::SetAdvFlag(uint8_t flag)
{
    advFlag_ = flag;
}

uint8_t BleAdvertiserData::GetAdvFlag() const
{
    return advFlag_;
}

std::map<ParcelUuid, std::string> BleAdvertiserData::GetServiceData() const
{
    return serviceData_;
}

bool BleAdvertiserData::GetIncludeDeviceName() const
{
    return includeDeviceName_;
}

void BleAdvertiserData::SetIncludeDeviceName(bool flag)
{
    includeDeviceName_ = flag;
}

bool BleAdvertiserData::GetIncludeTxPower() const
{
    return includeTxPower_;
}

void BleAdvertiserData::SetIncludeTxPower(bool flag)
{
    includeTxPower_ = flag;
}

BleAdvertiserSettings::BleAdvertiserSettings()
{}

BleAdvertiserSettings::~BleAdvertiserSettings()
{}

void BleAdvertiserSettings::SetConnectable(bool connectable)
{
    connectable_ = connectable;
}

bool BleAdvertiserSettings::IsConnectable() const
{
    return connectable_;
}

void BleAdvertiserSettings::SetLegacyMode(bool legacyMode)
{
    legacyMode_ = legacyMode;
}

bool BleAdvertiserSettings::IsLegacyMode() const
{
    return legacyMode_;
}

void BleAdvertiserSettings::SetInterval(uint16_t interval)
{
    interval_ = interval;
}

uint16_t BleAdvertiserSettings::GetInterval() const
{
    return interval_;
}

void BleAdvertiserSettings::SetTxPower(int8_t txPower)
{
    txPower_ = txPower;
}

int8_t BleAdvertiserSettings::GetTxPower() const
{
    return txPower_;
}

int BleAdvertiserSettings::GetPrimaryPhy() const
{
    return primaryPhy_;
}

void BleAdvertiserSettings::SetPrimaryPhy(int primaryPhy)
{
    primaryPhy_ = primaryPhy;
}

int BleAdvertiserSettings::GetSecondaryPhy() const
{
    return secondaryPhy_;
}

void BleAdvertiserSettings::SetSecondaryPhy(int secondaryPhy)
{
    secondaryPhy_ = secondaryPhy;
}

std::array<uint8_t, OHOS_BD_ADDR_LEN> BleAdvertiserSettings::GetOwnAddr() const
{
    return ownAddr_;
}

void BleAdvertiserSettings::SetOwnAddr(const std::array<uint8_t, OHOS_BD_ADDR_LEN>& addr)
{
    ownAddr_ = addr;
}

int8_t BleAdvertiserSettings::GetOwnAddrType() const
{
    return ownAddrType_;
}

void BleAdvertiserSettings::SetOwnAddrType(int8_t addrType)
{
    ownAddrType_ = addrType;
}
}  // namespace Bluetooth
}  // namespace OHOS
