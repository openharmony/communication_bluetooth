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
#ifndef LOG_TAG
#define LOG_TAG "bt_server_device_manager"
#endif

#include <algorithm>
#include <chrono>
#include <random>
#include <memory>
#include "common_util.h"
#include "hash_utils.h"
#include "log.h"
#include "log_utils.h"
#include "securec.h"
#include "bluetooth_device_manager.h"
#include "classic_sync_paired_device.h"
#include "classic_config.h"
#include "permission_manager.h"
#include "hitrace_meter.h"
#include "bluetooth_errorcode.h"
#include "preferences_helper.h"
#include "access_token.h"
#include "access_token_error.h"

using namespace OHOS::Bluetooth;
using OHOS::NativePreferences::PreferencesHelper;

namespace OHOS {
namespace bluetooth {
constexpr int ARRAY_SIZE = 4;
constexpr int MAC_BIT_SIZE = 12;
constexpr int FIRST_BIT = 1;
constexpr int LAST_BIT = 11;
constexpr int NUMBER_TWO = 2;
constexpr int HEX_BASE = 16;
constexpr int OCT_BASE = 8;

void ToUpper(char* arr)
{
    for (size_t i = 0; i < strlen(arr); ++i) {
        if (arr[i] >= 'a' && arr[i] <= 'z') {
            arr[i] = toupper(arr[i]);
        }
    }
}

RawAddress GenerateRandomMacAddress()
{
    int ret = 0;
    std::string randomMac = "";
    char strMacTmp[ARRAY_SIZE] = {0};
    std::mt19937_64 gen(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    for (int i = 0; i < MAC_BIT_SIZE; i++) {
        if (i != FIRST_BIT) {
            std::uniform_int_distribution<> distribution(0, HEX_BASE - 1);
            ret = sprintf_s(strMacTmp, ARRAY_SIZE, "%x", distribution(gen));
        } else {
            std::uniform_int_distribution<> distribution(0, OCT_BASE - 1);
            ret = sprintf_s(strMacTmp, ARRAY_SIZE, "%x", NUMBER_TWO * distribution(gen));
        }
        if (ret == -1) {
            HILOGE("failed, sprintf_s return -1!");
        }
        ToUpper(strMacTmp);
        randomMac += strMacTmp;
        if ((i % NUMBER_TWO) != 0 && (i != LAST_BIT)) {
            randomMac.append(":");
        }
    }
    return RawAddress(randomMac);
}

BluetoothDeviceManager::BluetoothDeviceManager()
{
    auto timeoutFunc = []() {
        HITRACE_METER_NAME(BT_TRACE_TAG, "BluetoothDeviceManager::BluetoothDeviceManager");
        HILOGD("BluetoothDeviceManager: Timeout");
        BluetoothDeviceManager::GetInstance()->ScheduleCleanDeviceInfo();
    };
    bluetoothDeviceManagerTimer = std::make_shared<utility::Timer>(timeoutFunc);
}

BluetoothDeviceManager::~BluetoothDeviceManager()
{
    ClearDevicesInfo();
}

BluetoothDeviceManager* BluetoothDeviceManager::GetInstance(void)
{
    static Bluetooth::BluetoothNoDestructor<BluetoothDeviceManager> singleton;
    return singleton.get();
}

RawAddress BluetoothDeviceManager::AddDeviceInfo(const RawAddress &realAddr)
{
    RawAddress randomMacAddr = GenerateRandomMacAddress();
    AddDeviceInfo(realAddr, randomMacAddr);
    return randomMacAddr;
}

void BluetoothDeviceManager::AddDeviceInfo(const RawAddress &realAddr, const RawAddress &randomAddr)
{
    BluetoothDeviceInfo bluetoothdeviceInfo(realAddr.GetAddress(), randomAddr.GetAddress(), GetTimeStamp());
    std::lock_guard<std::mutex> lock(deviceMapLock_);
    btDevicesMap.insert(std::make_pair(realAddr.GetAddress(), bluetoothdeviceInfo));
    HILOGI("realMacAddr: %{public}s, randomMacAddr: %{public}s",
        GetEncryptAddr(realAddr.GetAddress()).c_str(), GetEncryptAddr(randomAddr.GetAddress()).c_str());

    if (!bluetoothDeviceManagerTimer->IsStarted()) {
        bluetoothDeviceManagerTimer->Start(timerMs_, true); // 周期定时器
    }
}

void BluetoothDeviceManager::AddRetentionDeviceInfo(const RawAddress &realAddr, const RawAddress &randomAddr)
{
    std::string realUpperAddr = UpperStr(realAddr.GetAddress());
    BluetoothDeviceInfo bluetoothdeviceInfo(realUpperAddr, randomAddr.GetAddress(), GetTimeStamp(), true);
    std::lock_guard<std::mutex> lock(deviceMapLock_);
    auto iter = btDevicesMap.find(realUpperAddr);
    if (iter != btDevicesMap.end()) {
        CHECK_AND_RETURN_LOG(realUpperAddr == realAddr.GetAddress(), "already has upper random address");
        HILOGE("already exist, realMacAddr: %{public}s, randomMacAddr: %{public}s",
            GetEncryptAddr(realUpperAddr).c_str(), GetEncryptAddr(iter->second.randomMacAddr).c_str());
        btDevicesMap.erase(iter);
    }
    btDevicesMap.insert(std::make_pair(realUpperAddr, bluetoothdeviceInfo));
    HILOGI("realMacAddr: %{public}s, randomMacAddr: %{public}s",
        GetEncryptAddr(realUpperAddr).c_str(), GetEncryptAddr(randomAddr.GetAddress()).c_str());
}

int32_t BluetoothDeviceManager::GetDeviceRandomAddr(const RawAddress &realAddr, RawAddress &randomAddr)
{
    std::lock_guard<std::mutex> lock(deviceMapLock_);
    std::string randomMac = "";
    auto iter = btDevicesMap.find(realAddr.GetAddress());
    if (iter != btDevicesMap.end()) {
        randomMac = iter->second.randomMacAddr;
        iter->second.updateTime = GetTimeStamp();
        HILOGD("realMacAddr: %{public}s, randomMacAddr: %{public}s",
            GetEncryptAddr(realAddr.GetAddress()).c_str(), GetEncryptAddr(randomMac).c_str());
        randomAddr.SetAddress(randomMac);
        return RET_SUCCESS;
    }
    HILOGD("not exist, realMacAddr: %{public}s", GetEncryptAddr(realAddr.GetAddress()).c_str());
    return RET_NO_EXIST;
}

int32_t BluetoothDeviceManager::GetDeviceRealAddr(const RawAddress &randomAddr, RawAddress &realAddr)
{
    std::lock_guard<std::mutex> lock(deviceMapLock_);
    std::string realMac = "";
    auto iter = std::find_if(btDevicesMap.begin(), btDevicesMap.end(),
        [&randomAddr](const auto &item) {
        return item.second.randomMacAddr == randomAddr.GetAddress();
    });
    if (iter != btDevicesMap.end()) {
        realMac = iter->first;
        iter->second.updateTime = GetTimeStamp();
        realAddr.SetAddress(realMac);
        HILOGI("realMacAddr: %{public}s, randomMacAddr: %{public}s", GetEncryptAddr(realMac).c_str(),
            GetEncryptAddr(randomAddr.GetAddress()).c_str());
        return RET_SUCCESS;
    }
    HILOGD("not exist, randomAddr mac: %{public}s", GetEncryptAddr(randomAddr.GetAddress()).c_str());
    return RET_NO_EXIST;
}

int32_t BluetoothDeviceManager::SetDeviceRetentionFlag(const RawAddress &realAddr, bool isRetention)
{
    std::lock_guard<std::mutex> lock(deviceMapLock_);
    auto iter = btDevicesMap.find(realAddr.GetAddress());
    if (iter == btDevicesMap.end()) {
        HILOGI("not exist, realMacAddr: %{public}s", GetEncryptAddr(realAddr.GetAddress()).c_str());
        return RET_NO_EXIST;
    }

    if (iter->second.isPersistRandomAddr && !isRetention) {
        HILOGW("Can not clear retention flag, the address is persistent");
        return RET_NO_EXIST;
    }

    iter->second.updateTime = GetTimeStamp();
    iter->second.isRetention = isRetention;
    HILOGI("realMacAddr: %{public}s, randomMacAddr: %{public}s, isRetention: %{public}u",
        GetEncryptAddr(realAddr.GetAddress()).c_str(), GetEncryptAddr(iter->second.randomMacAddr).c_str(),
        iter->second.isRetention);

    // If a device does not need to be reserved, the timer needs to be started to delete the device.
    if (!isRetention && !bluetoothDeviceManagerTimer->IsStarted()) {
        bluetoothDeviceManagerTimer->Start(timerMs_, true); // 周期定时器
    }
    return RET_SUCCESS;
}

void BluetoothDeviceManager::ClearDevicesInfo()
{
    std::lock_guard<std::mutex> lock(deviceMapLock_);
    btDevicesMap.clear();
}

void BluetoothDeviceManager::ScheduleCleanDeviceInfo()
{
    std::lock_guard<std::mutex> lock(deviceMapLock_);
    HILOGI("btDevicesMap size: %{public}lu", btDevicesMap.size());
    uint32_t retentionDevCount = 0;
    int64_t currentTime = GetTimeStamp();
    for (auto iter = btDevicesMap.begin(); iter != btDevicesMap.end();) {
        // 异常情况，如果设备记录时间大于当前时间，记录日志
        if (iter->second.updateTime > currentTime) {
            HILOGI("updateTime large than currentTime, realMacAddr: %{public}s, randomMacAddr: %{public}s",
                GetEncryptAddr(iter->second.realMacAddr).c_str(), GetEncryptAddr(iter->second.randomMacAddr).c_str());
            iter++;
            continue;
        }
        if (iter->second.isRetention) {
            iter++;
            retentionDevCount++;
            continue;
        }
        if (currentTime - iter->second.updateTime > TIME_MINUTES_10) {
            HILOGD("remove addr, realMacAddr: %{public}s, randomMacAddr: %{public}s",
                GetEncryptAddr(iter->second.realMacAddr).c_str(), GetEncryptAddr(iter->second.randomMacAddr).c_str());
            btDevicesMap.erase(iter++);
            continue;
        }
        iter++;
    }

    // No device need to delete, stop the timer.
    if (btDevicesMap.size() == retentionDevCount) {
        bluetoothDeviceManagerTimer->Stop();
    }
    HILOGI("btDevicesMap size: %{public}lu, retentionDevCount: %{public}u",
        btDevicesMap.size(), retentionDevCount);
}

RawAddress BluetoothDeviceManager::GetRealUsedAddress(const RawAddress &randomAddr)
{
    RawAddress realUsedAddr;
    if (ConvertToRealAddress(randomAddr, realUsedAddr)) {
        return realUsedAddr;
    }
    // Convert random addr to real addr failed, the random addr is treated as the real used address.
    return randomAddr;
}

bool BluetoothDeviceManager::GetRealUsedAddress(const RawAddress &inputAddr, RawAddress &realUsedAddr)
{
    // search in btDeviceMap as a random address
    if (ConvertToRealAddress(inputAddr, realUsedAddr)) {
        return true;
    }
    // search in btDeviceMap as a real address
    if (IsRealAddrInBtDeviceMap(inputAddr)) {
        realUsedAddr.SetAddress(inputAddr.GetAddress());
        return true;
    }
    return false;
}

bool BluetoothDeviceManager::IsRealAddrInBtDeviceMap(const RawAddress &inputAddr)
{
    std::lock_guard<std::mutex> lock(deviceMapLock_);
    std::string randomMac = "";
    auto iter = btDevicesMap.find(inputAddr.GetAddress());
    if (iter != btDevicesMap.end()) {
        randomMac = iter->second.randomMacAddr;
        iter->second.updateTime = GetTimeStamp();
        HILOGI("real address exists, inputAddr: %{public}s, randomAddr: %{public}s",
            GetEncryptAddr(inputAddr.GetAddress()).c_str(), GetEncryptAddr(randomMac).c_str());
        return true;
    }
    HILOGI("real address not exist, inputAddr: %{public}s", GetEncryptAddr(inputAddr.GetAddress()).c_str());
    return false;
}

bool BluetoothDeviceManager::GetRealUsedAddrByAddrType(const RawAddress &addr, int32_t addressType,
    RawAddress &realAddr)
{
    // adapt to pairDevice(deviceId: string): Promise<void> and pairDevice(deviceId: string, cb: AsyncCb<void>): <void>
    if (addressType == AddressType::UNSET_ADDRESS) {
        // search in btDeviceMap as a random address and check for IsUseRealAddr permission
        if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
            return false;
        }
        return true;
    }
    // adapt to pairDevice(deviceId: BluetoothAddress): Promise<void>
    else if (addressType == AddressType::VIRTUAL_ADDRESS) {
        // only search in btDeviceMap as a random address
        int ret = BluetoothDeviceManager::GetInstance()->GetDeviceRealAddr(addr, realAddr);
        if (ret == RET_NO_EXIST) {
            return false;
        }
        return true;
    }
    // adapt to pairDevice(deviceId: BluetoothAddress): Promise<void>
    else if (addressType == AddressType::REAL_ADDRESS) {
        // use as real address directly
        realAddr.SetAddress(addr.GetAddress());
        return true;
    }
    else {
        // handle unknown address type
        return false;
    }
}

bool BluetoothDeviceManager::ConvertToRealAddress(const RawAddress &randomAddr, RawAddress &realAddr)
{
    int ret = BluetoothDeviceManager::GetInstance()->GetDeviceRealAddr(randomAddr, realAddr);
    if (ret == RET_NO_EXIST) {
        if (PermissionManager::IsUseRealAddr()) {
            realAddr.SetAddress(randomAddr.GetAddress());
            return true;
        }
        return false;
    }
    return true;
}

bool BluetoothDeviceManager::ConvertToRandomAddress(const uint64_t tokenId, const RawAddress &realAddr,
    RawAddress &randomAddr)
{
    if (PermissionManager::IsUseRealAddr(tokenId)) {
        randomAddr.SetAddress(realAddr.GetAddress());
        return true;
    }
    int ret = BluetoothDeviceManager::GetInstance()->GetDeviceRandomAddr(realAddr, randomAddr);
    if (ret == RET_NO_EXIST) {
        HILOGI_TIME_LIMIT(__func__,
            "ConvertToRandomAddress failed, device not exist, tokenId: %{public}s, realMacAddr: %{public}s",
            std::to_string(tokenId).c_str(),
            GetEncryptAddr(realAddr.GetAddress()).c_str());
        return false;
    }
    return true;
}

void BluetoothDeviceManager::ConvertToRandomAddress(std::vector<RawAddress> &rawAddrVec,
    std::vector<RawAddress> &randomAddrVec)
{
    if (rawAddrVec.empty()) {
        return;
    }

    if (PermissionManager::IsUseRealAddr()) {
        for (auto it = rawAddrVec.begin(); it != rawAddrVec.end(); ++it) {
            randomAddrVec.emplace_back(RawAddress(*it));
        }
    } else {
        RawAddress randomAddr;
        int ret;
        for (auto it = rawAddrVec.begin(); it != rawAddrVec.end(); ++it) {
            ret = BluetoothDeviceManager::GetInstance()->GetDeviceRandomAddr(*it, randomAddr);
            if (ret == OHOS::bluetooth::RET_SUCCESS) {
                randomAddrVec.emplace_back(RawAddress(randomAddr));
            }
        }
    }
}

void BluetoothDeviceManager::ConvertToRandomAddress(std::vector<RawAddress> &rawAddrVec,
    std::vector<BluetoothRawAddress> &randomAddrVec)
{
    if (rawAddrVec.empty()) {
        return;
    }
    if (PermissionManager::IsUseRealAddr()) {
        for (auto it = rawAddrVec.begin(); it != rawAddrVec.end(); ++it) {
            randomAddrVec.emplace_back(BluetoothRawAddress(*it));
        }
    } else {
        RawAddress randomAddr;
        int ret;
        for (auto it = rawAddrVec.begin(); it != rawAddrVec.end(); ++it) {
            ret = BluetoothDeviceManager::GetInstance()->GetDeviceRandomAddr(*it, randomAddr);
            if (ret == OHOS::bluetooth::RET_SUCCESS) {
                randomAddrVec.emplace_back(BluetoothRawAddress(randomAddr));
            }
        }
    }
}

void BluetoothDeviceManager::SyncPairedDevInfo()
{
    std::vector<std::string> pairedAddr;
    bool needSync = OHOS::bluetooth::BluetoothSyncPairedDev::GetInstance().SyncPairedDev(pairedAddr);
    if (!needSync) {
        return;
    }
    HILOGI("start sync paired device info when the first time open bt");
    bool needSave = false;
    for (const auto &realAddr : pairedAddr) {
        if (OHOS::bluetooth::ClassicConfig::GetInstance().GetRemoteRandomAddr(realAddr) != "") {
            continue;
        }
        std::string realUpperAddr = UpperStr(realAddr);
        RawAddress randomAddr = GenerateRandomMacAddress();
        HILOGD("SyncPairedDevInfo, realMacAddr: %{public}s, randomMacAddr: %{public}s",
            GetEncryptAddr(realUpperAddr).c_str(), GetEncryptAddr(randomAddr.GetAddress()).c_str());
        OHOS::bluetooth::ClassicConfig::GetInstance().SetRemoteRandomAddrNoSave(
            realUpperAddr, randomAddr.GetAddress());
        needSave = true;
    }
    if (needSave) {
        OHOS::bluetooth::ClassicConfig::GetInstance().Save();
    }
}

void BluetoothDeviceManager::GetPersistRandomDeviceIdFromFile(
    std::vector<BluetoothDeviceInfo> &vec, const std::string &key, const std::string &value)
{
    std::string realAddr = UpperStr(key);
    std::string randomAddr = UpperStr(value);
    if (!IsValidAddr(realAddr) || !IsValidAddr(randomAddr)) {
        HILOGE("Invalid addr format");
        return;
    }

    auto iter = std::find_if(vec.begin(), vec.end(),
        [&realAddr](BluetoothDeviceInfo &deviceInfo) { return deviceInfo.realMacAddr == realAddr; });
    if (iter == vec.end()) {
        BluetoothDeviceInfo deviceInfo(realAddr, randomAddr, GetTimeStamp(), true);
        deviceInfo.isPersistRandomAddr = true;
        vec.push_back(deviceInfo);
    } else { // iter != vec.end()
        iter->realMacAddr = realAddr;
        iter->randomMacAddr = randomAddr;
        iter->updateTime = GetTimeStamp();
        iter->isRetention = true;
        iter->isPersistRandomAddr = true;
    }
}

void BluetoothDeviceManager::GetPersistRandomDeviceIdAppInfoFromFile(
    std::vector<BluetoothDeviceInfo> &vec, const std::string &key, const std::vector<std::string> &value)
{
    // realAddr, App = split key
    std::string realAddr = "";
    std::vector<std::string> keySplit {};
    SplitStr(key, "_", keySplit);
    if (keySplit.empty() || !IsValidAddr(keySplit[0])) {
        HILOGE("Invalid addr format");
        return;
    }

    realAddr = UpperStr(keySplit[0]);
    std::set<uint64_t> appTokenIds {};
    std::vector<std::string> tokenIds = value;
    for (auto &str : tokenIds) {
        int tokenId = 0;
        bool ret = StrToInt(str, tokenId);
        if (!ret) {
            HILOGE("Invalid tokenId number: %{public}s", str.c_str());
            continue;
        }
        appTokenIds.insert(tokenId);
    }

    auto iter = std::find_if(vec.begin(), vec.end(),
        [&realAddr](BluetoothDeviceInfo &deviceInfo) { return deviceInfo.realMacAddr == realAddr; });
    if (iter == vec.end()) {
        BluetoothDeviceInfo deviceInfo(realAddr, "", GetTimeStamp());
        deviceInfo.appTokenIdSet = appTokenIds;
        vec.push_back(deviceInfo);
    } else {  // iter != vec.end()
        iter->realMacAddr = realAddr;
        iter->appTokenIdSet = appTokenIds;
    }
}

static bool IsInvalidAppTokenId(uint64_t tokenId)
{
    Security::AccessToken::HapTokenInfo hapTokenInfo;
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(static_cast<uint32_t>(tokenId));
    if (tokenType != Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        return true;
    }
    int ret = Security::AccessToken::AccessTokenKit::GetHapTokenInfo(static_cast<uint32_t>(tokenId), hapTokenInfo);
    return ret == Security::AccessToken::ERR_TOKENID_NOT_EXIST;
}

void BluetoothDeviceManager::AddPersistRandomDeviceIdFromFile(std::vector<BluetoothDeviceInfo> &deviceInfoVec)
{
    // 删除无效的数据
    deviceInfoVec.erase(
        std::remove_if(deviceInfoVec.begin(), deviceInfoVec.end(),
            [](BluetoothDeviceInfo &deviceInfo) {
                return !IsValidAddr(deviceInfo.realMacAddr) || !IsValidAddr(deviceInfo.randomMacAddr);
            }),
        deviceInfoVec.end());

    {
        std::lock_guard<std::mutex> lock(deviceMapLock_);
        for (auto &deviceInfo: deviceInfoVec) {
            // 强制更新虚拟地址Map
            {
                auto iter = btDevicesMap.find(deviceInfo.realMacAddr);
                if (iter != btDevicesMap.end()) {
                    btDevicesMap.erase(iter);
                }
            }
            HILOGI("add device, realMacAddr: %{public}s, randomMacAddr: %{public}s",
                GetEncryptAddr(deviceInfo.realMacAddr).c_str(), GetEncryptAddr(deviceInfo.randomMacAddr).c_str());
            btDevicesMap.insert(std::make_pair(deviceInfo.realMacAddr, deviceInfo));
        }
    }

    // 判断应用是否已卸载，应用若已卸载，且该固化虚拟地址无其他应用持有，需删除该虚拟地址。
    std::set<uint64_t> invalidApp {};
    for (auto &deviceInfo: deviceInfoVec) {
        for (uint64_t tokenId: deviceInfo.appTokenIdSet) {
            if (invalidApp.find(tokenId) != invalidApp.end()) {
                HILOGW("Invalid app, clear it");
                SetPersistRandomAddrFlag(RawAddress(deviceInfo.realMacAddr), tokenId, false);
                DeletePersistRandomDeviceIdToFile(RawAddress(deviceInfo.realMacAddr), tokenId);
                continue;
            }
            if (IsInvalidAppTokenId(tokenId)) {
                invalidApp.insert(tokenId);
                HILOGW("Invalid app, clear it");
                SetPersistRandomAddrFlag(RawAddress(deviceInfo.realMacAddr), tokenId, false);
                DeletePersistRandomDeviceIdToFile(RawAddress(deviceInfo.realMacAddr), tokenId);
            }
        }
    }
}

void BluetoothDeviceManager::AddPairedDevInfo()
{
    SyncPairedDevInfo();
    std::vector<std::string> pairedDevices = OHOS::bluetooth::ClassicConfig::GetInstance().GetPairedAddrList();
    for (auto device : pairedDevices) {
        std::string randomAddr = OHOS::bluetooth::ClassicConfig::GetInstance().GetRemoteRandomAddr(device);
        if (randomAddr == "") {
            HILOGE("randomAddr is empty, generate a new one.");
            randomAddr = GenerateRandomMacAddress().GetAddress();
            OHOS::bluetooth::ClassicConfig::GetInstance().SetRemoteRandomAddr(device, randomAddr);
        }
        AddRetentionDeviceInfo(RawAddress(device), RawAddress(randomAddr));
    }

    // Load persistent random deviceid from file
    int32_t errCode = NativePreferences::E_ERROR;
    auto preferences = PreferencesHelper::GetPreferences(PERSISTENT_RANDOM_DEVICE_ID, errCode);
    if (preferences == nullptr || errCode != NativePreferences::E_OK) {
        HILOGE("get preferences failed: %{public}d", errCode);
        return;
    }
    std::vector<BluetoothDeviceInfo> deviceInfoVec {};
    std::map<std::string, NativePreferences::PreferencesValue> values = preferences->GetAll();
    for (auto &[key, value] : values) {
        if (value.IsString()) {
            GetPersistRandomDeviceIdFromFile(deviceInfoVec, key, value);
        }
        if (value.IsStringArray()) {
            GetPersistRandomDeviceIdAppInfoFromFile(deviceInfoVec, key, value);
        }
    }
    AddPersistRandomDeviceIdFromFile(deviceInfoVec);
}

bool BluetoothDeviceManager::IsPersistentRandomAddr(const RawAddress &randomAddr, uint64_t appTokenId)
{
    std::lock_guard<std::mutex> lock(deviceMapLock_);
    auto it = std::find_if(btDevicesMap.begin(), btDevicesMap.end(),
        [&randomAddr, appTokenId](const std::pair<std::string, BluetoothDeviceInfo> &item) {
        return item.second.isPersistRandomAddr &&
               item.second.randomMacAddr == randomAddr.GetAddress() &&
               item.second.appTokenIdSet.find(appTokenId) != item.second.appTokenIdSet.end();
    });
    return it != btDevicesMap.end();
}

int32_t BluetoothDeviceManager::SetPersistRandomAddrFlag(
    const RawAddress &realAddr, uint64_t appTokenId, bool isPersistRandomAddr)
{
    std::lock_guard<std::mutex> lock(deviceMapLock_);
    auto iter = btDevicesMap.find(realAddr.GetAddress());
    if (iter == btDevicesMap.end()) {
        HILOGI("not exist, realMacAddr: %{public}s", GetEncryptAddr(realAddr.GetAddress()).c_str());
        return RET_NO_EXIST;
    }

    auto &deviceInfo = iter->second;
    HILOGI("realAddr: %{public}s, randomAddr: %{public}s, isPersistRandomAddr: %{public}d",
        GET_ENCRYPT_ADDR(realAddr), GetEncryptAddr(deviceInfo.randomMacAddr).c_str(), isPersistRandomAddr);
    if (isPersistRandomAddr) {
        deviceInfo.appTokenIdSet.insert(appTokenId);
        deviceInfo.isPersistRandomAddr = true;
    } else {
        deviceInfo.appTokenIdSet.erase(appTokenId);
        // 如果本次操作是取消固化虚拟地址，且该虚拟地址无其他应用固化操作过，取消固化标记位，允许该虚拟地址后续发生变化
        if (deviceInfo.appTokenIdSet.empty()) {
            deviceInfo.isRetention = false;
            deviceInfo.updateTime = GetTimeStamp();
            deviceInfo.isPersistRandomAddr = false;
        }
    }
    return RET_SUCCESS;
}

bool BluetoothDeviceManager::CheckPersistentRandomAddrLimitReached(uint64_t appTokenId)
{
    std::lock_guard<std::mutex> lock(deviceMapLock_);
    uint32_t count = 0;
    for (const auto &[realAddr, deviceInfo] : btDevicesMap) {
        if (deviceInfo.isPersistRandomAddr &&
            deviceInfo.appTokenIdSet.find(appTokenId) != deviceInfo.appTokenIdSet.end()) {
            count++;
        }
    }
    return count >= PERSISTENT_RANDOM_DEVICE_ID_MAX_COUNT;
}

int32_t BluetoothDeviceManager::AddPersistRandomDeviceIdToFile(
    const RawAddress &realAddr, const RawAddress &randomAddr, uint64_t appTokenId)
{
    // Save persistent random deviceid to file.
    int32_t errCode = NativePreferences::E_ERROR;
    auto preferences = PreferencesHelper::GetPreferences(PERSISTENT_RANDOM_DEVICE_ID, errCode);
    if (preferences == nullptr || errCode != NativePreferences::E_OK) {
        HILOGE("get preferences failed: %{public}d", errCode);
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }
    // 重复添加默认刷新数据
    errCode = preferences->PutString(realAddr.GetAddress(), UpperStr(randomAddr.GetAddress()));
    if (errCode != NativePreferences::E_OK) {
        HILOGE("put string failed: %{public}d", errCode);
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }

    std::string appKey = realAddr.GetAddress() + "_AppIdentifiers";
    NativePreferences::PreferencesValue emptyValue (std::vector<std::string> {});
    NativePreferences::PreferencesValue value = preferences->Get(appKey, emptyValue);
    if (!value.IsStringArray()) {
        HILOGE("Invalid preferences value: not string array");
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }
    std::vector<std::string> appVec = value;
    // update app tokenid
    {
        auto iter = std::find(appVec.begin(), appVec.end(), std::to_string(appTokenId));
        if (iter != appVec.end()) {
            appVec.erase(iter);
        }
    }
    appVec.push_back(std::to_string(appTokenId));
    value = NativePreferences::PreferencesValue(appVec);
    errCode = preferences->Put(appKey, value);
    if (errCode != NativePreferences::E_OK) {
        HILOGE("put string failed: %{public}d", errCode);
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }

    preferences->Flush();
    return Bluetooth::BT_NO_ERROR;
}

int32_t BluetoothDeviceManager::AddPersistRandomDeviceId(const RawAddress &randomAddr, uint64_t appTokenId)
{
    std::string callingName = PermissionManager::GetCallingName();
    if (IsPersistentRandomAddr(randomAddr, appTokenId)) {
        HILOGW("%{public}s add duplicate persistent random address", callingName.c_str());
        return Bluetooth::BT_NO_ERROR;
    }
    if (CheckPersistentRandomAddrLimitReached(appTokenId)) {
        HILOGE("persistent random deviceid is reached the upper limit, callingname: %{public}s", callingName.c_str());
        return Bluetooth::BT_ERR_MAX_RESOURCES;
    }

    RawAddress realAddr;
    int32_t ret = GetDeviceRealAddr(randomAddr, realAddr);
    if (ret != RET_SUCCESS) {
        HILOGE("address: %{public}s is not unknown", GET_ENCRYPT_ADDR(randomAddr));
        return Bluetooth::BT_ERR_INVALID_PARAM;
    }
    HILOGI("%{public}s add persistent random address: %{public}s",
        callingName.c_str(), GET_ENCRYPT_ADDR(randomAddr));

    ret = AddPersistRandomDeviceIdToFile(realAddr, randomAddr, appTokenId);
    if (ret != BT_NO_ERROR) {
        return ret;
    }
    SetDeviceRetentionFlag(realAddr, true);
    SetPersistRandomAddrFlag(realAddr, appTokenId, true);
    return Bluetooth::BT_NO_ERROR;
}

int32_t BluetoothDeviceManager::DeletePersistRandomDeviceIdToFile(const RawAddress &realAddr, uint64_t appTokenId)
{
    // delete persistent random deviceid from file
    int32_t errCode = NativePreferences::E_ERROR;
    auto preferences = PreferencesHelper::GetPreferences(PERSISTENT_RANDOM_DEVICE_ID, errCode);
    if (preferences == nullptr || errCode != NativePreferences::E_OK) {
        HILOGE("get preferences failed: %{public}d", errCode);
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }
    // Get app tokenid array from addr_AppIdentifiers
    std::string appKey = realAddr.GetAddress() + "_AppIdentifiers";
    NativePreferences::PreferencesValue emptyValue (std::vector<std::string> {});
    NativePreferences::PreferencesValue value = preferences->Get(appKey, emptyValue);
    if (!value.IsStringArray()) {
        HILOGE("Invalid preferences value: not string array");
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }
    std::vector<std::string> appVec = value;
    // update app tokenid
    {
        auto iter = std::find(appVec.begin(), appVec.end(), std::to_string(appTokenId));
        if (iter != appVec.end()) {
            appVec.erase(iter);
        }
    }
    // 该固化虚拟地址仍有应用使用，仅在文件里删除该虚拟地址对应的应用
    if (!appVec.empty()) {
        HILOGI("real addr: %{public}s delete app token: %{public}zu", GET_ENCRYPT_ADDR(realAddr), appTokenId);
        value = NativePreferences::PreferencesValue(appVec);
        errCode = preferences->Put(appKey, value);
        if (errCode != NativePreferences::E_OK) {
            HILOGW("put preferences value failed, errCode: %{public}d", errCode);
        }
        preferences->Flush();
        return Bluetooth::BT_NO_ERROR;
    }

    // 该固化虚拟地址被所有应用给删除，需要在文件里删除该虚拟地址的固化记录
    HILOGI("delete persistent real addr: %{public}s", GET_ENCRYPT_ADDR(realAddr));
    preferences->Delete(realAddr.GetAddress());
    preferences->Delete(appKey);
    preferences->Flush();
    return Bluetooth::BT_NO_ERROR;
}

int32_t BluetoothDeviceManager::DeletePersistRandomDeviceId(const RawAddress &randomAddr, uint64_t appTokenId)
{
    std::string callingName = PermissionManager::GetCallingName();
    RawAddress realAddr;
    int32_t ret = GetDeviceRealAddr(randomAddr, realAddr);
    if (ret != RET_SUCCESS) {
        HILOGE("address: %{public}s is not unknown", GET_ENCRYPT_ADDR(randomAddr));
        return Bluetooth::BT_ERR_INVALID_PARAM;
    }
    HILOGI("%{public}s remove persistent random address: %{public}s",
        callingName.c_str(), GET_ENCRYPT_ADDR(randomAddr));

    SetPersistRandomAddrFlag(realAddr, appTokenId, false);
    ret = DeletePersistRandomDeviceIdToFile(realAddr, appTokenId);
    return ret;
}

int32_t BluetoothDeviceManager::GetPersistDeviceIds(uint64_t appTokenId, std::vector<std::string> &randomAddrVec) const
{
    randomAddrVec.clear();
    std::lock_guard<std::mutex> lock(deviceMapLock_);
    for (const auto &[realAddr, deviceInfo]: btDevicesMap) {
        if (!deviceInfo.isPersistRandomAddr) {
            continue;
        }
        // OTA升级上来的应用，appTokenIdSet为空，从兼容性角度考虑，该固化虚拟地址属于所有应用
        bool sameApp = deviceInfo.appTokenIdSet.empty() ||
                       (deviceInfo.appTokenIdSet.find(appTokenId) != deviceInfo.appTokenIdSet.end());
        if (sameApp) {
            randomAddrVec.push_back(deviceInfo.randomMacAddr);
        }
    }
    return Bluetooth::BT_NO_ERROR;
}
bool BluetoothDeviceManager::IsValidRandomDeviceId(const RawAddress &randomAddr)
{
    RawAddress realAddr;
    return GetDeviceRealAddr(randomAddr, realAddr) == RET_SUCCESS;
}

std::string BluetoothDeviceManager::GetHashValue(int hashAlgorithmType, const std::string &virtualAddress)
{
    return HashUtil::EncodeString(hashAlgorithmType, virtualAddress);
}
}  // namespace bluetooth
}  // namespace OHOS