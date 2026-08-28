/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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
#ifndef BLUETOOTH_CLOUD_DEVICE_MANAGER_H
#define BLUETOOTH_CLOUD_DEVICE_MANAGER_H

#include <memory>
#include <string>
#include <vector>
#include "adapter_properties.h"
#include "classic_adapter.h"
#include "interface_adapter.h"
#include "interface_adapter_manager.h"
#include "safe_map.h"

namespace OHOS {
namespace bluetooth {

/* Huawei cloud-pairing advertisement info byte-array lengths. */
constexpr int ADV_INFO_BYTE_ARRY_LEN_16 = 16;
constexpr int ADV_INFO_BYTE_ARRY_LEN_49 = 49;

class CloudDeviceManager {
public:
    CloudDeviceManager();
    ~CloudDeviceManager();
    static CloudDeviceManager *GetInstance();

    void AddCloudBluetoothDev(std::vector<Bluetooth::BluetoothTrustPairDevice> &cloudList);
    void DelCloudBluetoothDev(std::vector<Bluetooth::BluetoothTrustPairDevice> &cloudList);
    void AddCloudDeviceProperties(std::shared_ptr<BluetoothDevice> device);
    void DelCloudDeviceProperties(std::shared_ptr<BluetoothDevice> device);
    bool IsCloudDevice(const RawAddress &device) const;
    void SetCloudDevice(const RawAddress &device, bool isCloudDevice);
    int32_t GetCloudBondState(const RawAddress &device) const;
    void SetCloudBondState(const RawAddress &device, int32_t cloudBondState);
    void SendCloudDevState(const RawAddress &device);
    bool IsStartCloudPair(const RawAddress &device);
    void StartBtAclTimer(const RawAddress &device);
    void StopBtAclTimer(const RawAddress &device);
    void HandleCloudPairAclConnectTimeOut(const RawAddress &device);
    bool IsAnyDeviceInCloudPairing();
private:
    void SetDeviceClassProp(std::shared_ptr<BluetoothDevice> device, int32_t deviceClass);
    void SetDeviceNameProp(std::shared_ptr<BluetoothDevice> device, const std::string &deviceName);
    void SetDeviceUuidsProp(std::shared_ptr<BluetoothDevice> device, const std::vector<Uuid> &uuids);
    void SetCloudDeviceProp(std::shared_ptr<BluetoothDevice> device, bool isCloudDev);
    void SetDeviceTypeProp(std::shared_ptr<BluetoothDevice> device, int32_t devType);
    void AddDeviceCloudProp(std::shared_ptr<BluetoothDevice> device, Bluetooth::BluetoothTrustPairDevice &element);
    void DelCloudRemoteDevice(const std::string &addr);
    void UpdateCloudDeviceProp(std::shared_ptr<BluetoothDevice> device, Bluetooth::BluetoothTrustPairDevice &element);
    void DeleteCloudBtDevice(std::shared_ptr<BluetoothDevice> cloudDevice, const std::string &address);
    void SetAdvKey(std::vector<uint8_t> advInfo, std::shared_ptr<BluetoothDevice> cloudDevice);
    void RemoveAdvKey(std::shared_ptr<BluetoothDevice> device);
    void SendCloudUuidIntent(std::shared_ptr<BluetoothDevice> device);
    bool IsDeviceHasLinkkey(std::shared_ptr<BluetoothDevice> device);
    void SetRemoteDevicePropertyInt(const RawAddress &device, BtPropertyType type, int32_t value);
    void SetDeviceProductIdProp(std::shared_ptr<BluetoothDevice> device, const std::string &productId);
    bool IsValidAddress(const std::string &addr);
    std::shared_ptr<BluetoothDevice> GetCloudBluetoothDeviceFromMap(const RawAddress &device);
    std::shared_ptr<BluetoothDevice> AddCloudRemoteDevice(const RawAddress &device);
    SafeMap<std::string, std::shared_ptr<BluetoothDevice>> cloudDevicesMap_ {};
    mutable std::mutex btAclTimeoutMutex_;
    inline static constexpr int ACL_TIMEOUT_TIME {10000};
    std::map<std::string, std::shared_ptr<utility::Timer>> btAclTimeoutMap_{};
};

std::string CloudTokenToLogString(const std::vector<uint8_t> &tokens);
bool IsAdvIrkChanged(const std::vector<uint8_t>& advInfo, const std::vector<uint8_t>& currentAdvIrk);
bool IsUuidChanged(const std::vector<Uuid>& newUuids, const std::vector<Uuid>& currentUuids);
bool IsAdvIrkInvalid(const std::vector<uint8_t>& advInfo);
} // namespace bluetooth
} // namespace OHOS

#endif //BLUETOOTH_CLOUD_DEVICE_MANAGER_H