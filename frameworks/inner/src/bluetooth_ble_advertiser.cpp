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
#include "bluetooth_ble_advertiser.h"

#include "bluetooth_ble_advertise_callback_stub.h"
#include "bluetooth_def.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_observer_map.h"
#include "bluetooth_utils.h"
#include "i_bluetooth_ble_advertiser.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include <memory>
#include "securec.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;
std::mutex g_bleProxyMutex;
struct BleAdvertiser::impl {
    impl();
    ~impl();
    void ConvertBleAdvertiserData(const BleAdvertiserData &data, BluetoothBleAdvertiserData &outData);
    uint32_t GetAdvertiserTotalBytes(const BluetoothBleAdvertiserData &data, bool isFlagsIncluded);
    int32_t CheckAdvertiserData(const BluetoothBleAdvertiserSettings &setting,
        const BluetoothBleAdvertiserData &advData, const BluetoothBleAdvertiserData &scanResponse);
    bool InitBleAdvertiserProxy(void);

    class BluetoothBleAdvertiserCallbackImp : public BluetoothBleAdvertiseCallbackStub {
    public:
        explicit BluetoothBleAdvertiserCallbackImp(BleAdvertiser::impl &bleAdvertiser)
            : bleAdvertiser_(bleAdvertiser){};
        ~BluetoothBleAdvertiserCallbackImp()
        {}

        void OnStartResultEvent(int32_t result, int32_t advHandle, int32_t opcode) override
        {
            HILOGI("result: %{public}d, advHandle: %{public}d, opcode: %{public}d", result, advHandle, opcode);
            std::shared_ptr<BleAdvertiseCallback> observe = nullptr;
            if (opcode == bluetooth::BLE_ADV_START_FAILED_OP_CODE) {
                observer = bleAdvertiser_.callbacks_.PopAdvertiserObserver(advHandle);
            } else {
                observer = bleAdvertiser_.callbacks_.GetAdvertiserObserver(advHandle);
            }

            if (observer != nullptr) {
                observer->OnStartResultEvent(result, advHandle);
            }
        }

        void OnEnableResultEvent(int32_t result, int32_t advHandle) override
        {
            HILOGI("result: %{public}d, advHandle: %{public}d", result, advHandle);
            std::shared_ptr<BleAdvertiseCallback> observe = bleAdvertiser_.callbacks_.GetAdvertiserObserver(advHandle);
            if (observer != nullptr) {
                observer->OnEnableResultEvent(result, advHandle);
            }
        }

        void OnDisableResultEvent(int32_t result, int32_t advHandle) override
        {
            HILOGI("result: %{public}d, advHandle: %{public}d", result, advHandle);
            std::shared_ptr<BleAdvertiseCallback> observe = bleAdvertiser_.callbacks_.GetAdvertiserObserver(advHandle);
            if (observer != nullptr) {
                observer->OnDisableResultEvent(result, advHandle);
            }
        }

        void OnStopResultEvent(int32_t result, int32_t advHandle) override
        {
            HILOGI("result: %{public}d, advHandle: %{public}d", result, advHandle);
            std::shared_ptr<BleAdvertiseCallback> observe = bleAdvertiser_.callbacks_.PopAdvertiserObserver(advHandle);
            if (observer != nullptr) {
                observer->OnStopResultEvent(result, advHandle);
            }
        }

        void OnAutoStopAdvEvent(int32_t advHandle) override
        {
            HILOGI("advHandle: %{public}d", advHandle);
            std::shared_ptr<BleAdvertiseCallback> observe = bleAdvertiser_.callbacks_.GetAdvertiserObserver(advHandle);
            if (observer != nullptr) {
                bleAdvertiser_.callbacks_.Deregister(observer);
            }
        }

        void OnSetAdvDataEvent(int32_t result, int32_t advHandle) override
        {
            HILOGI("result: %{public}d, advHandle: %{public}d", result, advHandle);
            std::shared_ptr<BleAdvertiseCallback> observe = bleAdvertiser_.callbacks_.GetAdvertiserObserver(advHandle);
            if (observer) {
                observer->OnSetAdvDataEvent(result);
            }
        }

    private:
        BleAdvertiser::impl &bleAdvertiser_;
        BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothBleAdvertiserCallbackImp);
    };
    sptr<BluetoothBleAdvertiserCallbackImp> callbackImp_ = nullptr;

    BluetoothObserverMap<std::shared_ptr<BleAdvertiseCallback>> callbacks_;
    sptr<IBluetoothBleAdvertiser> proxy_ = nullptr;

    class BleAdvertiserDeathRecipient;
    sptr<BleAdvertiserDeathRecipient> deathRecipient_ = nullptr;
};

class BleAdvertiser::impl::BleAdvertiserDeathRecipient final : public IRemoteObject::DeathRecipient {
public:
    explicit BleAdvertiserDeathRecipient(BleAdvertiser::impl &impl) : owner_(impl) {};
    ~BleAdvertiserDeathRecipient() final = default;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BleAdvertiserDeathRecipient);

    void OnRemoteDied(const wptr<IRemoteObject> &remote) final
    {
        HILOGI("enter");
        std::lock_guard<std::mutex> lock(g_bleProxyMutex);
        if (!owner_.proxy_) {
            return;
        }
        owner_.proxy_ = nullptr;
        owner_.callbacks_.Clear();
    }

private:
    BleAdvertiser::impl &owner_;
};

bool BleAdvertiser::impl::InitBleAdvertiserProxy(void)
{
    std::lock_guard<std::mutex> lock(g_bleProxyMutex);
    if (proxy_) {
        return true;
    }
    HILOGI("enter");
    proxy_ = GetRemoteProxy<IBluetoothBleAdvertiser>(BLE_ADVERTISER_SERVER);
    if (!proxy_) {
        HILOGE("get bleAdvertiser proxy_ failed");
        return false;
    }
    callbackImp_ = new BluetoothBleAdvertiserCallbackImp(*this);
    if (callbackImp_ != nullptr) {
        proxy_->RegisterBleAdvertiserCallback(callbackImp_);
    }

    deathRecipient_ = new BleAdvertiserDeathRecipient(*this);
    if (deathRecipient_ != nullptr) {
        proxy_->AsObject()->AddDeathRecipient(deathRecipient_);
    }
    return true;
}

BleAdvertiser::impl::impl()
{}

BleAdvertiser::impl::~impl()
{
    if (proxy_ != nullptr) {
        proxy_->DeregisterBleAdvertiserCallback(callbackImp_);
        proxy_->AsObject()->RemoveDeathRecipient(deathRecipient_);
    }
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

    std::vector<Uuid> serviceUuids = data.GetServiceUuids();
    for (auto iter = serviceUuids.begin(); iter != serviceUuids.end(); ++iter) {
        size += BLE_ADV_PER_FIELD_OVERHEAD_LENGTH + static_cast<uint32_t>(iter->GetUuidType());
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
    const BleAdvertiserData &scanResponse, uint16_t duration, shared_ptr<BleAdvertiseCallback> callback)
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->InitBleAdvertiserProxy()) {
        HILOGE("pimpl or bleAdvertiser proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }

    BluetoothBleAdvertiserSettings setting;
    setting.SetConnectable(settings.IsConnectable());
    setting.SetInterval(settings.GetInterval());
    setting.SetLegacyMode(settings.IsLegacyMode());
    setting.SetTxPower(settings.GetTxPower());
    uint8_t addr[OHOS_BD_ADDR_LEN] = {};
    settings.GetOwnAddr(addr);
    setting.SetOwnAddr(addr);
    setting.SetOwnAddrType(settings.GetOwnAddrType());

    BluetoothBleAdvertiserData bleAdvertiserData;
    BluetoothBleAdvertiserData bleScanResponse;
    bleAdvertiserData.SetAdvFlag(advData.GetAdvFlag());
    pimpl->ConvertBleAdvertiserData(advData, bleAdvertiserData);
    pimpl->ConvertBleAdvertiserData(scanResponse, bleScanResponse);

    int ret = pimpl->CheckAdvertiserData(setting, bleAdvertiserData, bleScanResponse);
    if (ret != BT_NO_ERROR) {
        return ret;
    }

    int32_t advHandle = BLE_INVALID_ADVERTISING_HANDLE;
    if (pimpl->callbacks_.IsExistAdvertiserCallback(callback, advHandle)) {
        ret = pimpl->proxy_->StartAdvertising(setting, bleAdvertiserData, bleScanResponse, advHandle, duration, false);
    } else {
        ret = pimpl->proxy_->GetAdvertiserHandle(advHandle);
        if (ret != BT_NO_ERROR || advHandle == BLE_INVALID_ADVERTISING_HANDLE) {
            HILOGE("Invalid advertising handle");
            callback.OnStartResultEvent(BT_ERR_INTERNAL_ERROR, BLE_INVALID_ADVERTISING_HANDLE);
            return ret;
        }
        callback->OnGetAdvHandleEvent(0, advHandle);
        pimpl->callbacks_.Register(advHandle, callback);
        ret = pimpl->proxy_->StartAdvertising(setting, bleAdvertiserData, bleScanResponse, advHandle, duration, false);
    }
    return ret;
}

int BleAdvertiser::StartAdvertising(const BleAdvertiserSettings &settings, const std::vector<uint8_t> &advData,
    const std::vector<uint8_t> &scanResponse, uint16_t duration, shared_ptr<BleAdvertiseCallback> callback)
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->InitBleAdvertiserProxy()) {
        HILOGE("pimpl or bleAdvertiser proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }

    BluetoothBleAdvertiserSettings setting;
    setting.SetConnectable(settings.IsConnectable());
    setting.SetInterval(settings.GetInterval());
    setting.SetLegacyMode(settings.IsLegacyMode());
    setting.SetTxPower(settings.GetTxPower());
    uint8_t addr[OHOS_BD_ADDR_LEN] = {};
    settings.GetOwnAddr(addr);
    setting.SetOwnAddr(addr);
    setting.SetOwnAddrType(settings.GetOwnAddrType());

    BluetoothBleAdvertiserData bleAdvertiserData;
    bleAdvertiserData.SetPayload(std::string(advData.begin(), advData.end()));
    bleAdvertiserData.SetAdvFlag(0);
    BluetoothBleAdvertiserData bleScanResponse;
    bleScanResponse.SetPayload(std::string(scanResponse.begin(), scanResponse.end()));

    int32_t advHandle = BLE_INVALID_ADVERTISING_HANDLE;
    int ret = BT_ERR_INTERNAL_ERROR;
    if (pimpl->callbacks_.IsExistAdvertiserCallback(callback, advHandle)) {
        ret = pimpl->proxy_->StartAdvertising(setting, bleAdvertiserData, bleScanResponse, advHandle, duration, true);
    } else {
        ret = pimpl->proxy_->GetAdvertiserHandle(advHandle);
        if (ret != BT_NO_ERROR || advHandle == BLE_INVALID_ADVERTISING_HANDLE) {
            HILOGE("Invalid advertising handle");
            callback->OnStartResultEvent(BT_ERR_INTERNAL_ERROR, BLE_INVALID_ADVERTISING_HANDLE);
            return ret;
        }
        pimpl->callbacks_.Register(advHandle, callback);
        ret = pimpl->proxy_->StartAdvertising(setting, bleAdvertiserData, bleScanResponse, advHandle, duration, true);
    }
    return ret;
}

void BleAdvertiser::SetAdvertisingData(const std::vector<uint8_t> &advData, const std::vector<uint8_t> &scanResponse,
    shared_ptr<BleAdvertiseCallback> callback)
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    if (pimpl == nullptr || !pimpl->InitBleAdvertiserProxy()) {
        HILOGE("pimpl or bleAdvertiser proxy is nullptr");
        return;
    }

    int advHandle = BLE_INVALID_ADVERTISING_HANDLE;
    if (!pimpl->callbacks_.IsExistAdvertiserCallback(callback, advHandle)) {
        HILOGE("Advertising is not started");
        return;
    }

    BluetoothBleAdvertiserData bleAdvertiserData;
    bleAdvertiserData.SetPayload(std::string(advData.begin(), advData.end()));
    BluetoothBleAdvertiserData bleScanResponse;
    bleScanResponse.SetPayload(std::string(scanResponse.begin(), scanResponse.end()));
    pimpl->proxy_->SetAdvertisingData(bleAdvertiserData, bleScanResponse, advHandle);
}

int BleAdvertiser::EnableAdvertising(uint8_t advHandle, uint16_t duration, shared_ptr<BleAdvertiseCallback> callback)
{
    HILOGI("enter");
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->InitBleAdvertiserProxy()) {
        HILOGE("pimpl or bleAdvertiser proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }

    uint8_t tmpAdvHandle = pimpl->callbacks_.GetAdvertiserHandle(callback);
    if (tmpAdvHandle == BLE_INVALID_ADVERTISING_HANDLE) {
        HILOGE("Invalid advertising callback");
        return BT_ERR_INTERNAL_ERROR;
    }

    std::shared_ptr<BleAdvertiseCallback> observe = pimpl->callbacks_.GetAdvertiserObserver(advHandle);
    if (observer == nullptr) {
        HILOGE("Invalid advertising handle");
        return BT_ERR_INTERNAL_ERROR;
    }

    int ret = pimpl->proxy_->EnableAdvertising(advHandle, duration);
    return ret;
}

int BleAdvertiser::DisableAdvertising(uint8_t advHandle, shared_ptr<BleAdvertiseCallback> callback)
{
    HILOGI("enter");
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->InitBleAdvertiserProxy()) {
        HILOGE("pimpl or bleAdvertiser proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }

    uint8_t tmpAdvHandle = pimpl->callbacks_.GetAdvertiserHandle(callback);
    if (tmpAdvHandle == BLE_INVALID_ADVERTISING_HANDLE) {
        HILOGE("Invalid advertising callback");
        return BT_ERR_INTERNAL_ERROR;
    }

    std::shared_ptr<BleAdvertiseCallback> observe = pimpl->callbacks_.GetAdvertiserObserver(advHandle);
    if (observer == nullptr) {
        HILOGE("Invalid advertising handle");
        return BT_ERR_INTERNAL_ERROR;
    }

    int ret = pimpl->proxy_->DisableAdvertising(advHandle);
    return ret;
}

int BleAdvertiser::StopAdvertising(shared_ptr<BleAdvertiseCallback> callback)
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->InitBleAdvertiserProxy()) {
        HILOGE("pimpl or bleAdvertiser proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }

    HILOGI("enter");
    uint8_t advHandle = pimpl->callbacks_.GetAdvertiserHandle(callback);
    if (advHandle == BLE_INVALID_ADVERTISING_HANDLE) {
        HILOGE("Invalid advertising handle");
        return BT_ERR_INTERNAL_ERROR;
    }

    int ret = pimpl->proxy_->StopAdvertising(advHandle);
    return ret;
}

void BleAdvertiser::Close(shared_ptr<BleAdvertiseCallback> callback)
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("BLE is not enabled");
        return;
    }

    if (pimpl == nullptr || !pimpl->InitBleAdvertiserProxy()) {
        HILOGE("pimpl or bleAdvertiser proxy is nullptr");
        return;
    }

    HILOGI("enter");
    if (pimpl->proxy_ != nullptr) {
        uint8_t advHandle = pimpl->callbacks_.GetAdvertiserHandle(callback);
        if (advHandle != BLE_INVALID_ADVERTISING_HANDLE) {
            pimpl->proxy_->Close(advHandle);
        }

        std::shared_ptr<BleAdvertiseCallback> observe = pimpl->callbacks_.GetAdvertiserObserver(advHandle);
        if (observer != nullptr) {
            pimpl->callbacks_.Deregister(observer);
        }
    }
}

uint8_t BleAdvertiser::GetAdvHandle(shared_ptr<BleAdvertiseCallback> callback)
{
    if (!BluetoothHost::GetDefaultHost().IsBleEnabled()) {
        HILOGE("BLE is not enabled");
        return BLE_INVALID_ADVERTISING_HANDLE;
    }
    return pimpl->callbacks_.GetAdvertiserHandle(callback);
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

void BleAdvertiserSettings::SetTxPower(uint8_t txPower)
{
    txPower_ = txPower;
}

uint8_t BleAdvertiserSettings::GetTxPower() const
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

void BleAdvertiserSettings::GetOwnAddr(uint8_t addr[OHOS_BD_ADDR_LEN]) const
{
    memcpy_s(addr, OHOS_BD_ADDR_LEN, ownAddr_, OHOS_BD_ADDR_LEN);
}

void BleAdvertiserSettings::SetOwnAddr(const uint8_t addr[OHOS_BD_ADDR_LEN])
{
    memcpy_s(ownAddr_, OHOS_BD_ADDR_LEN, addr, OHOS_BD_ADDR_LEN);
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
