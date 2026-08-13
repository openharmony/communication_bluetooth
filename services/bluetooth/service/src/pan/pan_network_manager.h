/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#ifndef PAN_NETWORK_MANAGER_H
#define PAN_NETWORK_MANAGER_H

#include <mutex>
#include <string>
#include "net_conn_client.h"
#include "base_def.h"
#include "dhcp_c_api.h"
#include "refbase.h"

namespace OHOS {
namespace Bluetooth {

struct PanIpInfo {
    std::string ipAddress {};
    std::string gateway {};
    std::string netmask {};
    std::string dns1 {};
    std::string dns2 {};
    std::string dns3 {};
    std::string domain {};
    int prefixLength {0};
    uint32_t leaseTime {0};
};

class PanNetworkManager {
public:
    PanNetworkManager();
    ~PanNetworkManager();

    void RegisterNetSupplier();
    void UnregisterNetSupplier();
    void UpdateNetSupplierInfo(bool isAvailable, bool isRoaming = false);
    void UpdateNetLinkInfo(const PanIpInfo &ipInfo);

    int RegisterDhcpClientCallback(const std::string &ifname);
    int StartDhcpClient(const std::string &ifname);
    int StopDhcpClient(const std::string &ifname, bool bIpv4 = true, bool bIpv6 = true);

    void SetIfname(const std::string &ifname);
    std::string GetIfname() const;
    uint32_t GetSupplierId() const;

    static void OnDhcpIpSuccessChanged(int status, const char *ifname, DhcpResult *result);
    static void OnDhcpIpFailChanged(int status, const char *ifname, const char *reason);

    using DhcpResultCallback = std::function<void(int evt, int ipType, const DhcpResult &result)>;
    static void SetDhcpResultCallback(const DhcpResultCallback &callback);

    void SaveDhcpResult(int ipType, const DhcpResult &result);
    bool DealDhcpResult();
    void DealDhcpFailed();
    void Clear();

    static DhcpResultCallback dhcpResultCallback_;
private:
    void CreateNetLinkInfo(sptr<NetManagerStandard::NetLinkInfo> &netLinkInfo, const PanIpInfo &ipInfo);
    bool SaveIpv4Address(const DhcpResult &result, PanIpInfo &ipInfo);
    bool IsValidDhcpResult(const DhcpResult &result);

    uint32_t supplierId_ {0};
    std::string ifname_ {"bt-pan"};
    ClientCallBack dhcpClientCallBack_ {};
    
    int savedIpType_ {0};
    DhcpResult savedDhcpResult_ {};

    BT_DISALLOW_COPY_AND_ASSIGN(PanNetworkManager);
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // PAN_NETWORK_MANAGER_H
