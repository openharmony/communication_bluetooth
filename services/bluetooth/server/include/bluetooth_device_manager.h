/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_DEVICE_MANAGER_H
#define BLUETOOTH_DEVICE_MANAGER_H

#include <unordered_map>
#include <mutex>
#include <set>
#include <string>
#include "btcommon/timer_manager.h"
#include "bt_def.h"
#include "bluetooth_raw_address.h"
#include "raw_address.h"
#include "string_ex.h"
#include "bluetooth_no_destructor.h"

namespace OHOS {
namespace bluetooth {

constexpr int32_t RET_SUCCESS = 0;
constexpr int32_t RET_NO_EXIST = -1;
constexpr int64_t TIME_MINUTES_10 = 600000;

class BluetoothDeviceManager {
public:
    struct BluetoothDeviceInfo {
        BluetoothDeviceInfo(const std::string &realMacAddr, const std::string &randomMacAddr, uint64_t updateTime)
            : realMacAddr(realMacAddr), randomMacAddr(randomMacAddr), updateTime(updateTime) {}
        BluetoothDeviceInfo(const std::string &realMacAddr, const std::string &randomMacAddr, uint64_t updateTime,
            bool isRetention)
            : realMacAddr(realMacAddr), randomMacAddr(randomMacAddr), updateTime(updateTime),
              isRetention(isRetention) {}

        std::string realMacAddr {INVALID_MAC_ADDRESS};
        std::string randomMacAddr {INVALID_MAC_ADDRESS};
        int64_t updateTime {0};
        bool isRetention {false};
        bool isPersistRandomAddr {false};
        std::set<uint64_t> appTokenIdSet {};
    };

    BluetoothDeviceManager();
    ~BluetoothDeviceManager();
    static BluetoothDeviceManager *GetInstance(void);

    RawAddress AddDeviceInfo(const RawAddress &realAddr);
    void AddDeviceInfo(const RawAddress &realAddr, const RawAddress &randomAddr);
    void AddRetentionDeviceInfo(const RawAddress &realAddr, const RawAddress &randomAddr);
    int32_t GetDeviceRandomAddr(const RawAddress &realAddr, RawAddress &randomAddr);
    int32_t GetDeviceRealAddr(const RawAddress &randomAddr, RawAddress &realAddr);
    int32_t SetDeviceRetentionFlag(const RawAddress &realAddr, bool isRetention);
    void ClearDevicesInfo();
    /**
    * @brief 获取真正使用的蓝牙设备地址
    *
    * @return 1.如果调用方拥有"ohos.permission.GET_BLUETOOTH_PEERS_MAC"权限，randomAddr当成真实地址并返回。
    *         2.如果调用方无"ohos.permission.GET_BLUETOOTH_PEERS_MAC"权限，
    *           1）在地址随机化列表里找到randomAddr对应的真实地址，返回该真实地址；
    *           2）在地址随机化列表里没找到randomAddr对应的真实地址，randomAddr当成真实地址并返回。
    */
    RawAddress GetRealUsedAddress(const RawAddress &randomAddr);
    bool GetRealUsedAddress(const RawAddress &inputAddr, RawAddress &realAddr);
    bool IsRealAddrInBtDeviceMap(const RawAddress &inputAddr);
    bool GetRealUsedAddrByAddrType(const RawAddress &addr, int32_t addressType, RawAddress &realAddr);
    bool ConvertToRealAddress(const RawAddress &rawAddr, RawAddress &realAddr);
    bool ConvertToRandomAddress(const uint64_t tokenId, const RawAddress &rawAddr, RawAddress &realAddr);
    void ConvertToRandomAddress(std::vector<RawAddress> &rawAddrVec,
        std::vector<OHOS::Bluetooth::BluetoothRawAddress> &randomAddrVec);
    void ConvertToRandomAddress(std::vector<RawAddress> &rawAddrVec,
        std::vector<RawAddress> &randomAddrVec);
    void AddPairedDevInfo();

    int32_t AddPersistRandomDeviceId(const RawAddress &randomAddr, uint64_t appTokenId);
    int32_t DeletePersistRandomDeviceId(const RawAddress &randomAddr, uint64_t appTokenId);
    int32_t GetPersistDeviceIds(uint64_t appTokenId, std::vector<std::string> &randomAddrVec) const;
    bool IsValidRandomDeviceId(const RawAddress &randomAddr);
    std::string GetHashValue(int hashAlgorithmType, const std::string &virtualAddress);

private:
    void ScheduleCleanDeviceInfo();
    void SyncPairedDevInfo();
    int32_t SetPersistRandomAddrFlag(const RawAddress &realAddr, uint64_t appTokenId, bool isPersistRandomAddr);
    bool IsPersistentRandomAddr(const RawAddress &randomAddr, uint64_t appTokenId);
    bool CheckPersistentRandomAddrLimitReached(uint64_t appTokenId);

    // 从配置文件里获取已固化的虚拟地址
    void AddPersistRandomDeviceIdFromFile(std::vector<BluetoothDeviceInfo> &deviceInfoVec);
    void GetPersistRandomDeviceIdFromFile(
        std::vector<BluetoothDeviceInfo> &vec, const std::string &key, const std::string &value);
    void GetPersistRandomDeviceIdAppInfoFromFile(
        std::vector<BluetoothDeviceInfo> &vec, const std::string &key, const std::vector<std::string> &value);

    // 更新配置文件里的固化虚拟地址信息
    int32_t AddPersistRandomDeviceIdToFile(
    const RawAddress &realAddr, const RawAddress &randomAddr, uint64_t appTokenId);
    int32_t DeletePersistRandomDeviceIdToFile(const RawAddress &realAddr, uint64_t appTokenId);

private:
    int timerMs_ = 60000; // 1 mintue
    std::shared_ptr<utility::Timer> bluetoothDeviceManagerTimer = nullptr;
    mutable std::mutex deviceMapLock_;
    // key: realMacAddr  value:BluetoothDeviceInfo
    std::unordered_map<std::string, BluetoothDeviceInfo> btDevicesMap {};
    const std::string PERSISTENT_RANDOM_DEVICE_ID = "/data/service/el1/public/bluetooth/bt_persistent_deviceid.xml";
    const uint32_t PERSISTENT_RANDOM_DEVICE_ID_MAX_COUNT = 100;

    friend class Bluetooth::BluetoothNoDestructor<BluetoothDeviceManager>;
};
}  // namespace bluetooth
}  // namespace OHOS

#endif
