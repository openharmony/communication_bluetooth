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
#ifndef LOG_TAG
#define LOG_TAG "pan_network_manager"
#endif

#include "pan_network_manager.h"
#include "pan_defines.h"
#include "thread_util.h"
#include "log.h"

namespace OHOS {
namespace Bluetooth {

PanNetworkManager::DhcpResultCallback PanNetworkManager::dhcpResultCallback_;

PanNetworkManager::PanNetworkManager()
{
}

PanNetworkManager::~PanNetworkManager()
{
}

void PanNetworkManager::RegisterNetSupplier()
{
    using NetManagerStandard::NetBearType;
    using NetManagerStandard::NetCap;
    std::set<NetCap> netCaps {NetCap::NET_CAPABILITY_INTERNET};
    std::string ident = ifname_;
    int32_t result = NetManagerStandard::NetConnClient::GetInstance().RegisterNetSupplier(
        NetBearType::BEARER_BLUETOOTH, ident, netCaps, supplierId_);
    HILOGI("RegisterNetSupplier result: %{public}d, supplierId: %{public}u",
        result, supplierId_);
}

void PanNetworkManager::UnregisterNetSupplier()
{
    HILOGI("UnregisterNetSupplier supplierId: %{public}u", supplierId_);
    if (supplierId_ == 0) {
        HILOGW("supplierId is 0, skip");
        return;
    }
    int32_t result = NetManagerStandard::NetConnClient::GetInstance().UnregisterNetSupplier(supplierId_);
    HILOGI("UnregisterNetSupplier result: %{public}d", result);
    supplierId_ = 0;
}

void PanNetworkManager::UpdateNetSupplierInfo(bool isAvailable, bool isRoaming)
{
    HILOGI("UpdateNetSupplierInfo enter, isAvailable: %{public}d, supplierId: %{public}u",
    isAvailable, supplierId_);
    if (supplierId_ == 0) {
        HILOGE("UpdateNetSupplierInfo supplierId is 0");
        return;
    }
    sptr<NetManagerStandard::NetSupplierInfo> netSupplierInfo = new NetManagerStandard::NetSupplierInfo();
    if (netSupplierInfo == nullptr) {
        HILOGE("UpdateNe SupplierInfo new NetSupplierInfo failed");
        return;
    }
    netSupplierInfo->isAvailable_ = isAvailable;
    netSupplierInfo->isRoaming_ = isRoaming;
    netSupplierInfo->ident_ = ifname_;
    netSupplierInfo->linkUpBandwidthKbps_ = LINK_UP_BAND_WIDTH_KBPS;
    netSupplierInfo->linkDownBandwidthKbps_ = LINK_DOWN_BAND_WIDTH_KBPS;
    netSupplierInfo->score_ = NET_SCORE;
    int32_t result =
        NetManagerStandard::NetConnClient::GetInstance().UpdateNetSupplierInfo(supplierId_, netSupplierInfo);
    HILOGI("UpdateNetSupplierInfo result: %{public}d", result);
}

void PanNetworkManager::UpdateNetLinkInfo(const PanIpInfo &ipInfo)
{
    if (supplierId_ == 0) {
        HILOGE("UpdateNetLinkInfo supplierId is 0");
        return;
    }
    sptr<NetManagerStandard::NetLinkInfo> netLinkInfo = sptr<NetManagerStandard::NetLinkInfo>::MakeSptr();
    if (netLinkInfo == nullptr) {
        HILOGE("netLinkInfo is null");
        return;
    }
    CreateNetLinkInfo(netLinkInfo, ipInfo);
    int32_t result = NetManagerStandard::NetConnClient::GetInstance().UpdateNetLinkInfo(supplierId_, netLinkInfo);
    HILOGI("UpdateNetLinkInfo result: %{public}d", result);
}

int PanNetworkManager::RegisterDhcpClientCallback(const std::string &ifname)
{
    dhcpClientCallBack_.OnIpSuccessChanged = PanNetworkManager::OnDhcpIpSuccessChanged;
    dhcpClientCallBack_.OnIpFailChanged = PanNetworkManager::OnDhcpIpFailChanged;
    DhcpErrorCode result = RegisterDhcpClientCallBack(ifname.c_str(), &dhcpClientCallBack_);
    HILOGI("RegisterDhcpClientCallback result: %{public}d", result);
    return static_cast<int>(result);
}

int PanNetworkManager::StartDhcpClient(const std::string &ifname)
{
    RouterConfig config = {};
    if (strncpy_s(config.ifname, sizeof(config.ifname), ifname.c_str(), ifname.length()) != EOK) {
        return DHCP_FAILED;
    }
    std::string bssid = "default";
    if (strncpy_s(config.bssid, sizeof(config.bssid), bssid.c_str(), bssid.length()) != EOK) {
        return DHCP_FAILED;
    }
    config.bIpv6 = false;
    DhcpErrorCode result = ::StartDhcpClient(config);
    HILOGI("StartDhcpClient result: %{public}d", result);
    return static_cast<int>(result);
}

int PanNetworkManager::StopDhcpClient(const std::string &ifname, bool bIpv4, bool bIpv6)
{
    DhcpErrorCode result = ::StopDhcpClient(ifname.c_str(), bIpv6, bIpv4);
    HILOGI("StopDhcpClient result: %{public}d", result);
    return static_cast<int>(result);
}

void PanNetworkManager::SetIfname(const std::string &ifname)
{
    ifname_ = ifname;
}

std::string PanNetworkManager::GetIfname() const
{
    return ifname_;
}

uint32_t PanNetworkManager::GetSupplierId() const
{
    return supplierId_;
}

void PanNetworkManager::OnDhcpIpSuccessChanged(int status, const char *ifname, DhcpResult *result)
{
    if (result == nullptr) {
        HILOGE("result is nullptr");
        return;
    }
    bluetooth::DoInPanThread([status, ifname, result]() {
        if (PanNetworkManager::dhcpResultCallback_ != nullptr) {
            PanNetworkManager::dhcpResultCallback_(PAN_GET_IP_SUCCESS_EVT, result->iptype, *result);
        }
    });
}

void PanNetworkManager::OnDhcpIpFailChanged(int status, const char *ifname, const char *reason)
{
    if (reason == nullptr) {
        HILOGE("reason is nullptr");
        return;
    }
    HILOGI("reason:%{public}s", reason);
    bluetooth::DoInPanThread([]() {
        if (PanNetworkManager::dhcpResultCallback_ != nullptr) {
            DhcpResult emptyResult = {};
            emptyResult.iptype = 0;
            PanNetworkManager::dhcpResultCallback_(PAN_GET_IP_FAIL_EVT, IPV4, emptyResult);
        }
    });
}

void PanNetworkManager::SetDhcpResultCallback(const DhcpResultCallback &callback)
{
    dhcpResultCallback_ = callback;
}

void PanNetworkManager::CreateNetLinkInfo(
    sptr<NetManagerStandard::NetLinkInfo> &netLinkInfo, const PanIpInfo &ipInfo)
{
    netLinkInfo->ifaceName_ = ifname_;
    netLinkInfo->domain_ = ipInfo.domain.empty() ? "bt-pan" : ipInfo.domain;

    if (!ipInfo.ipAddress.empty()) {
        NetManagerStandard::INetAddr netAddr;
        netAddr.type_ = NetManagerStandard::INetAddr::IPV4;
        netAddr.family_ = NetManagerStandard::INetAddr::IPV4;
        netAddr.address_ = ipInfo.ipAddress;
        netAddr.prefixlen_ = ipInfo.prefixLength;
        netLinkInfo->netAddrList_.push_back(netAddr);

        NetManagerStandard::Route defaultRoute;
        defaultRoute.iface_ = ifname_;
        defaultRoute.destination_.type_ = NetManagerStandard::INetAddr::IPV4;
        defaultRoute.destination_.address_ = "0.0.0.0";
        defaultRoute.destination_.family_ = NetManagerStandard::INetAddr::IPV4;
        if (!ipInfo.gateway.empty()) {
            defaultRoute.gateway_.address_ = ipInfo.gateway;
            defaultRoute.gateway_.family_ = NetManagerStandard::INetAddr::IPV4;
        }
        netLinkInfo->routeList_.push_back(defaultRoute);

        NetManagerStandard::Route hostRoute;
        hostRoute.iface_ = ifname_;
        hostRoute.destination_.type_ = NetManagerStandard::INetAddr::IPV4;
        hostRoute.destination_.address_ = ipInfo.gateway;
        hostRoute.destination_.family_ = NetManagerStandard::INetAddr::IPV4;
        hostRoute.destination_.prefixlen_ = MAX_PREFIX_LENGTH;
        netLinkInfo->routeList_.push_back(hostRoute);

        NetManagerStandard::Route localRoute;
        localRoute.iface_ = ifname_;
        localRoute.destination_.type_ = NetManagerStandard::INetAddr::IPV4;
        localRoute.destination_.address_ = ipInfo.ipAddress;
        localRoute.destination_.family_ = NetManagerStandard::INetAddr::IPV4;
        localRoute.destination_.prefixlen_ = ipInfo.prefixLength;
        localRoute.gateway_.address_ = "0.0.0.0";
        netLinkInfo->routeList_.push_back(localRoute);
    }

    if (!ipInfo.dns1.empty()) {
        NetManagerStandard::INetAddr dns;
        dns.type_ = NetManagerStandard::INetAddr::IPV4;
        dns.family_ = NetManagerStandard::INetAddr::IPV4;
        dns.address_ = ipInfo.dns1;
        netLinkInfo->dnsList_.push_back(dns);
    }
}

void PanNetworkManager::SaveDhcpResult(int ipType, const DhcpResult &result)
{
    savedIpType_ = ipType;
    savedDhcpResult_ = result;
}

bool PanNetworkManager::DealDhcpResult()
{
    HILOGI("ipType: %{public}d", savedIpType_);
    if (savedIpType_ == IPV6) {
        HILOGE("bluetooth pan ipv6 is not supported");
        return false;
    }
    PanIpInfo ipInfo {};
    if (SaveIpv4Address(savedDhcpResult_, ipInfo)) {
        UpdateNetLinkInfo(ipInfo);
        return true;
    } else {
        HILOGE(" IPv4 save failed");
        return false;
    }
}

void PanNetworkManager::DealDhcpFailed()
{
    HILOGE("Dhcp failed");
    Clear();
}

void PanNetworkManager::Clear()
{
    savedIpType_ = 0;
    savedDhcpResult_ = {};
}

bool PanNetworkManager::IsValidDhcpResult(const DhcpResult &result)
{
    auto isValidCString = [](const char *str, size_t maxLen) -> bool {
        return str != nullptr && strnlen(str, maxLen) > 0 && strnlen(str, maxLen) < maxLen;
    };

    if (!isValidCString(result.strOptClientId, sizeof(result.strOptClientId))) {
        HILOGE("strOptClientId is invalid");
        return false;
    }
    if (!isValidCString(result.strOptRouter1, sizeof(result.strOptRouter1))) {
        HILOGE("strOptRouter1 is invalid");
        return false;
    }
    if (!isValidCString(result.strOptSubnet, sizeof(result.strOptSubnet))) {
        HILOGE("strOptSubnet is invalid");
        return false;
    }
    if (!isValidCString(result.strOptDns1, sizeof(result.strOptDns1))) {
        HILOGE("strOptDns1 is invalid");
        return false;
    }
    if (!isValidCString(result.strOptDns2, sizeof(result.strOptDns2))) {
        HILOGE("strOptDns2 is invalid");
        return false;
    }

    if (result.dnsList.dnsNumber > DHCP_DNS_MAX_NUMBER) {
        HILOGE("dnsNumber %{public}u exceeds max %{public}d",
            result.dnsList.dnsNumber, DHCP_DNS_MAX_NUMBER);
        return false;
    }
    for (uint32_t i = 0; i < result.dnsList.dnsNumber; i++) {
        if (!isValidCString(result.dnsList.dnsAddr[i], sizeof(result.dnsList.dnsAddr[i]))) {
            HILOGE("dnsAddr[%{public}u] is invalid", i);
            return false;
        }
    }
    return true;
}

bool PanNetworkManager::SaveIpv4Address(const DhcpResult &result, PanIpInfo &ipInfo)
{
    HILOGI("PanNetworkManager::SaveIpv4Address enter");
    if (!IsValidDhcpResult(result)) {
        HILOGE("Invalid Dhcp Result");
        return false;
    }
    if (!result.isOptSuc) {
        HILOGE("DHCP result not success");
        return false;
    }

    ipInfo.ipAddress = result.strOptClientId;
    ipInfo.gateway = result.strOptRouter1;
    ipInfo.netmask = result.strOptSubnet;
    ipInfo.dns1 = result.strOptDns1;
    ipInfo.dns2 = result.strOptDns2;
    if (result.dnsList.dnsNumber > 0 && strlen(result.dnsList.dnsAddr[0]) > 0) {
        ipInfo.dns1 = result.dnsList.dnsAddr[0];
    }
    if (result.dnsList.dnsNumber > 1 && strlen(result.dnsList.dnsAddr[1]) > 0) {
        ipInfo.dns2 = result.dnsList.dnsAddr[1];
    }
    ipInfo.prefixLength = PAN_NETWORK_IPV4_PREFIX_LENGTH;
    ipInfo.leaseTime = result.uOptLeasetime;
    ipInfo.domain = "bt-pan";
    return true;
}
}  // namespace bluetooth
}  // namespace OHOS
