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
#ifndef BLUETOOTH_CONNECTION_MANAGER_H
#define BLUETOOTH_CONNECTION_MANAGER_H

#include "bt_def.h"
#include "bluetooth_device.h"
#include <string>
#include <list>
#include <mutex>
#include "raw_address.h"
#include "datetime_ex.h"
#include "util/safe_vector.h"
#include "safe_map.h"  // SafeMap
#include "btcommon/timer_manager.h"

namespace OHOS {
namespace bluetooth {

const int BLUETOOTH_MAX_BR_LINKS = 6;
const int64_t EXCESSIVE_CONNECTING_NO_CONTROL_TIMER = 300; // 300s
const int64_t EXCESSIVE_CONNECTING_ALLOW_CONNECT_TIMER = 60; // 60s
const int64_t EXCESSIVE_CONNECTING_EXIST_CONTROL_TIMER = 120; // 120s
constexpr const uint8_t CONN_REASON_AUTO_CONNECT_FLAG = 0x01; // triggered by reboot or open bluetooth.
constexpr const uint8_t CONN_REASON_VIRTUAL_CONNECT_FLAG = 0x02;
constexpr const uint8_t CONN_REASON_MANUAL_VIRTUAL_CONNECT_PREEMPT_FLAG = 0x03;

/**
 * connect record for framework calling
 */
struct ConnectRecord {
    int64_t timestamp;
    std::uint32_t tokenId;
    int64_t firstConnectTime;
    int64_t lastConnectTime;
    int64_t lastConnectedTime;

    ConnectRecord(uint32_t tokenId, int64_t currentTime): timestamp(currentTime), tokenId(tokenId),
        firstConnectTime(currentTime), lastConnectTime(currentTime), lastConnectedTime(currentTime) {}
    
    bool operator == (const ConnectRecord &rhs) const
    {
        return this->tokenId == rhs.tokenId;
    }
};

class BluetoothConnectionManager {
public:
    BluetoothConnectionManager();
    ~BluetoothConnectionManager();

    static BluetoothConnectionManager *GetInstance();
    
    /**
     * @brief is allow the address br link.
     *
     * @param device the device we want to check whether it's ACL exists
     * @return when allowed return true when not allowed return false.
     * @since 11
     */
    bool IsBrLinkAllowed(std::string address);

    /**
     * @brief check is allow the acl connect cb
     *
     * @param address the remote device address
     * @param unPairedAddr allow when unPairedAddr acl link
     * @return when allowed return true when not allowed return false.
     */
    bool IsBrLinkAllowed(std::string address, std::string &unPairedAddr);

    /**
     * @brief Disconnecting the earliest connected device when the number of profile connections reaches the maximum.
     *
     * @return when disconnect return true, when not disconnect return false.
     * @since 12
     */
    bool DisconnectForDeviceLimit();

    /**
     * @brief record the acl connect cb
     *
     * @param address the remote device address
     * @param linkType the acl link type
     * @param newConnectState the remote device new connect state
     */
    void RecordAclConnect(std::string address, int linkType, int newConnectState, std::string unPairedAddr);

    /**
     * @brief Add manufacture data.
     *
     * @param address the device we want to check whether it's ACL exists
     * @return true: reach BLUETOOTH_MAX_BR_LINKS.
     * @since 11
     */
    bool ReachingMaxBrLinks(std::string address);
 
    /**
     * @brief Get unPaired background br link address.
     *
     * @return unPaired background br link address.
     * @since 11
     */
    std::string GetFirstUnPairedBrLink();

    /**
     * @brief shoule or not refuse socket connect.
     *
     * @param address connect address
     * @param uuid uuid
     * @param bundle chr bundle
     * @return when allowed return false when not allowed return true.
     * @since 11
     */
    bool ShouldRefuseSocketConnectEX(const std::string address, const Uuid &uuid, std::string bundle);

    /**
     * @brief shoule or not refuse socket connect because of too frequently.
     *
     * @param tokenId IPC connect token
     * @param currentTime connect time
     * @param connect address
     * @return when allowed return false when not allowed return true.
     * @since 11
     */
    bool ShouldRefuseSocketConnectTooFrequently(std::uint32_t tokenId, int64_t currentTime, std::string address);

    /**
     * @brief IsConnectTooFrequently
     *
     * @param connectRecord connectRecord
     * @return when allowed return fasle when not allowed return true.
     * @since 11
     */
    bool IsConnectTooFrequently(ConnectRecord connectRecord);

    /**
     * @brief RemoveConnectRecords
     *
     * @param tokenId removeRecordToken
     * @since 11
     */
    void RemoveConnectRecords(std::uint32_t tokenId);

    void RecordBleAclConnect(RawAddress address, bool isAllowed);
    bool RemoveAllBrAclLinks();
    void ClearBleConnect();
    bool IsNativeAppOrSystemHap();
    void CheckNeedReportConnectedDevice();
    void RemoveAllAclLinks();
    void OnGetConnReasonEchoRsp(const std::string &address, int type, int typeval);
    void SetVirtualAutoConnectType(const RawAddress &address, int connType, int businessType);
    bool IsInCloudBondingState(const std::string &address);
    bool IsBrConnected(std::string address);
    bool IsBleConnected(std::string address);
    int32_t SetConnectionPriority(const std::string &address, int priority);

    friend class BluetoothChannelRssiManager;
    friend class BluetoothResourceManager;
private:
    void AddAclLinks(std::string address, int linkType, std::string unPairedAddr);
    void RemoveAclLinks(std::string address, int linkType);
    void UpdateConnectRecord(bool &isRecordExist,
        int64_t &lastConnectTime, int64_t &firstConnectTime, std::uint32_t tokenId, int64_t currentTime);
    bool isBleAclAllowed(RawAddress address);
    void AddBleConnectionMaps(std::string address);
    void UpdateVirtualAutoConnCap(const std::string &address, int autoConnRejectReason);
    bool ShouldPermitWhitelistFrequentConnect(const std::string &address);
    void SendAntennaStatusMsg(bool isFixed);

    utility::SafeVector<std::string> brLinks_ {};
    utility::SafeVector<std::string> bleLinks_ {};
    utility::SafeVector<std::uint32_t> connectTokenList_ {};
    std::mutex connectRecordListMutex_ {};
    std::list<ConnectRecord> connectRecordList_ {};
    SafeMap<std::string, bool> bleConnection_ {};
    SafeMap<std::string, bool> bleTempRecordLinks_ {};
    utility::SafeVector<std::string> highPriorityLinks_ {};
    std::shared_ptr<utility::Timer> noRefuseLinkTimer_ = nullptr;
    std::mutex noRefuseLinkMutex_ {};
    std::string noRefuseLinkAddr_ {};
};
} // namespace bluetooth
} // namespace OHOS

#endif //BLUETOOTH_CONNECTION_MANAGER_H