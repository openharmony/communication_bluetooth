/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef REMOTE_DEVICE_PROPERTIES_H
#define REMOTE_DEVICE_PROPERTIES_H

#include <functional>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

#include "btcommon/timer_manager.h"
#include "adapter_properties.h"
#include "base_def.h"
#include "bluetooth.h"
#include "bluetooth_device.h"
#include "bt_def.h"
#include "bt_types.h"
#include "bt_uuid.h"
#include "util/context.h"
#include "service_util.h"
#include "bt_recursive_mutex.h"
#include "bluetooth_trust_pair_device.h"

namespace OHOS {
namespace bluetooth {

#define MAX_DEVICE_QUEUE_SIZE 200
// HW Device type
#define HW_INVALID_DEVICE_TYPE (-1)
#define HW_CAR_DEVICE_TYPE 105
// Thread safety object
class RemoteDeviceProperties {
public:

    static RemoteDeviceProperties *GetInstance(void);

    BtProperty* PropertyDeepCopy(int numProperties, BtProperty* properties);
    void GetRemoteDevicePropsCallBack(BtStackStatus status, STACK::RawAddress* bdAddr, int numProperties,
        BtProperty* properties);
    void DeviceFoundCallBack(int numProperties, BtProperty* properties);

    std::shared_ptr<BluetoothDevice> FindRemoteDevice(const RawAddress &device);
    void FreeMemory();
    bool IsRemoteDeviceMapEmpty();
    std::string GetDeviceName(const RawAddress &device, bool alias = true) const;
    std::vector<Uuid> GetDeviceUuids(const RawAddress &device) const;
    bool IsBondedFromLocal(const RawAddress &device) const;
    std::vector<RawAddress> GetBtPairedDevices() const;
    bool CancelPairing(const RawAddress &device) const;
    std::shared_ptr<BluetoothDevice> GetBluetoothDeviceFromMap(const RawAddress &device) const;
    bool RemovePair(const RawAddress &device);
    std::vector<RawAddress> removeAllDevicesFromMap();
    int GetPairState(const RawAddress &device) const;
    int GetDeviceType(const RawAddress &device) const;
    int GetDeviceClass(const RawAddress &device) const;
    std::string GetAliasName(const RawAddress &device) const;
    bool SetAliasName(const RawAddress &device, const std::string &name);
    int GetHwRemoteDeviceType(const RawAddress &device) const;
    void DeleteLinkKey(std::shared_ptr<BluetoothDevice> remoteDevice) const;
    bool SetDevicePairingConfirmation(const RawAddress &device, bool accept) const;
    bool IsBrDeviceType(std::string address) const;
    bool SetDevicePin(const RawAddress &device, const std::string &pinCode) const;
    void AddPairWhiteList(const RawAddress &device);
    bool CheckPairWhiteList(const RawAddress &device);
    void RemovePairWhiteList(const RawAddress &device);
    int32_t GetDeviceAbsVolumeAbility(const RawAddress &device);
    // get device property from stack, result will return in GetRemoteDevicePropsCallbackInner
    int32_t GetRemoteDeviceProperty(const STACK::RawAddress &addr, BtPropertyType type);
    void SetRemoteDeviceProperty(const STACK::RawAddress &addr, const BtProperty &prop);
    bool IsAclConnected(const RawAddress &device);
    bool IsAclEncrypted(const RawAddress &device);
    std::vector<std::string> GetNotPairNoneDevices() const;
    int32_t GetCustomType(const RawAddress &device) const;
    int32_t SetCustomType(const RawAddress &device, int32_t customType);
    bool GetDeviceBatteryInfo(const std::string &addr, BatteryDetailInfo &batteryInfo) const;
    void SetDeviceBatteryInfo(const std::string &addr, const BatteryDetailInfo &batteryInfo);
    void ClearDeviceBatteryInfo(const std::string &addr);
    void SetDevProactiveDisconnectFlag(const std::string &addr, uint32_t profileId, bool isProactive);
    bool IsDevProactiveDisconnect(const std::string &addr, uint32_t profileId) const;
    int GetDevConnStateChangeCause(const RawAddress &device, uint32_t profileId, int state);
    bool SetRemoteDevicePropertyInfo(const RawAddress &device, BtPropertyType type,
        const std::string &propertyValue);
    void UpdateRemoteHwDeviceType(const RawAddress &device, int newDeviceType);
    void StackErrnoCallback(const RawAddress &device, int status, int errNum);
    bool IsNeedDelayConnect(const RawAddress &device);

    void HandlePropertyRemoteDeviceConnectionTime(BtProperty* property,
    std::shared_ptr<BluetoothDevice> remoteDevice);
 
    int64_t GetLastConnectionTime(const RawAddress &device) const;
    int32_t SetConnectionTime(const RawAddress &device, int64_t connectionTime);

    bool IsCloudDevice(const RawAddress &device) const;
    void SetCloudDevice(const RawAddress &device, bool isCloudDevice);
    int32_t GetCloudBondState(const RawAddress &device) const;
    void SetCloudBondState(const RawAddress &device, int32_t cloudBondState);
    bool IsStartCloudPair(const RawAddress &device);
private:
    RemoteDeviceProperties();
    ~RemoteDeviceProperties();

    void GetRemoteDevicePropsCallbackInner(STACK::RawAddress bdAddr, int numProperties,
        BtProperty* properties);
    void DeviceFoundInner(int numProperties, BtProperty* properties);
    void WhiltListCallTimeOut(const RawAddress &device);
    void UpdateRemoteDeviceCod(const RawAddress &device, int newCod);
    void UpdateRemoteDeviceName(const RawAddress &device, std::string newRemoteName);
    void HandlePropertyRemoteDeviceName(BtProperty* property, RawAddress device);
    void HandlePropertyRemoteDeviceUuid(BtProperty* property, std::shared_ptr<BluetoothDevice> remoteDevice);
    void HandlePropertyRemoteDeviceCod(BtProperty* property, RawAddress device);
    void HandlePropertyRemoteDeviceType(BtProperty* property, std::shared_ptr<BluetoothDevice> remoteDevice);
    void HandlePropertyRemoteDeviceAbsVolumKey(BtProperty* property, std::shared_ptr<BluetoothDevice> remoteDevice);
    void HandlePropertyRemoteDeviceRssi(BtProperty* property, std::shared_ptr<BluetoothDevice> remoteDevice);
    void HandlePropertyRemoteDeviceAlias(BtProperty* property, std::shared_ptr<BluetoothDevice> remoteDevice);
    void HandlePropertyRemoteDeviceCustomType(BtProperty* property, std::shared_ptr<BluetoothDevice> remoteDevice);
    void HandlePropertyRemoteDeviceIoCapability(BtProperty* property, std::shared_ptr<BluetoothDevice> remoteDevice);
    void HandlePropertyRemoteDeviceVendorId(BtProperty* property, std::shared_ptr<BluetoothDevice> remoteDevice);
    void HandlePropertyRemoteDeviceProductId(BtProperty* property, std::shared_ptr<BluetoothDevice> remoteDevice);
    void HandlePropertyRemoteDeviceManuSpecData(BtProperty* property, std::shared_ptr<BluetoothDevice> remoteDevice);
    void HandlePropertyRemoteDeviceChipInfo(BtProperty* property, std::shared_ptr<BluetoothDevice> remoteDevice);
    void HandlePropertyRemoteDeviceAutoConnSwitch(BtProperty* property,
        std::shared_ptr<BluetoothDevice> remoteDevice);
    void HandlePropertyRemoteDeviceMessage(BtProperty* property, std::shared_ptr<BluetoothDevice> remoteDevice);
    bool SetDeviceProperty(const RawAddress &device, BtPropertyType type, const std::string &propertyValue,
        bool isIntProperty, int32_t intPropertyValue);
    bool SetOtherPropertyInfo(const RawAddress &device, BtPropertyType type, const std::string &propertyValue);
    void HandlePropertyRemoteDeviceCloudCap(BtProperty* property, std::shared_ptr<BluetoothDevice> remoteDevice);
    void HandlePropertyByType(BtProperty* property, const RawAddress &device,
        std::shared_ptr<BluetoothDevice> remoteDevice);
    // properties mutex
    std::mutex mutex_ {};

    std::map<std::string, std::shared_ptr<BluetoothDevice>> remoteDevicesMap_ {};
    std::queue<std::string> remoteDeviceQueue {};
    mutable BtRecursiveMutex remoteDevicesMapMutex_ {};
    mutable BtRecursiveMutex remoteDevicesQueueMutex_ {};

    std::map<std::string, std::shared_ptr<utility::Timer>> whiteListMap_{};
    mutable BtRecursiveMutex whiteListMapMutex_ {};

    // The time of whitelist timeout
    inline static constexpr int WHITELIST_TIMEOUT_TIME {10000};

    AdapterProperties *adapterProperties_;

    BT_DISALLOW_COPY_AND_ASSIGN(RemoteDeviceProperties);
    BT_DISALLOW_MOVE_AND_ASSIGN(RemoteDeviceProperties);
};
}  // namespace bluetooth
}  // namespace OHOS

#endif
