/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_permission_manag"
#endif

#include "permission_manager.h"

#include "ipc_skeleton.h"
#include "bluetooth_errorcode.h"
#include "log.h"
#include "hitrace_meter.h"
#include "privacy_kit.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "bundle_mgr_proxy.h"

namespace OHOS {
namespace Bluetooth {

using namespace OHOS;
using namespace Security::AccessToken;

bool g_isAllGranted = false;

bool PermissionManager::VerifyPermission(const std::string &permissionName)
{
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    return VerifyPermission(permissionName, tokenId, true);
}

bool PermissionManager::VerifyPermission(const std::string &permissionName, const uint32_t &tokenId, bool isNeedRecord)
{
    HITRACE_METER(BT_TRACE_TAG);
    ATokenTypeEnum tokenType = AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenType == ATokenTypeEnum::TOKEN_NATIVE || tokenType == ATokenTypeEnum::TOKEN_SHELL) {
        bool ret = AccessTokenKit::VerifyAccessToken(tokenId, permissionName);
        if (ret != PermissionState::PERMISSION_GRANTED) {
            HILOGE("[PERMISSION] tokenType(%{public}d), permission(%{public}s)", tokenType, permissionName.c_str());
            return false;
        }
        return true;
    } else if (tokenType == ATokenTypeEnum::TOKEN_HAP) {
        bool ret = (AccessTokenKit::VerifyAccessToken(tokenId, permissionName) == PermissionState::PERMISSION_GRANTED);
        if (permissionName == ACCESS_BLUETOOTH && isNeedRecord) {
            int successCount = ret ? 1 : 0; // 1,0分別是成功次数
            int failCount = ret ? 0 : 1; // 0,1分別是失败次数
            int permissionRet =
                PrivacyKit::AddPermissionUsedRecord(tokenId, permissionName, successCount, failCount, true);
            if (permissionRet != NO_ERROR) {
                HILOGE("AddPermissionUsedRecord falied, ret is %{public}d", permissionRet);
            }
        }
        return ret;
    } else {
        HILOGI("[PERMISSION] unknown token type(%{public}d)", tokenType);
        return false;
    }
}

bool PermissionManager::IsPermissionsGranted(const std::set<std::string> &permissions)
{
    for (auto &it : permissions) {
        CHECK_AND_RETURN_LOG_RET(VerifyPermission(it), false,
            "[PERMISSION] check permission failed, permission(%{public}s), callingName(%{public}s)",
            it.c_str(), GetCallingName().c_str());
    }
    return true;
}

void PermissionManager::SetAllGranted()
{
    g_isAllGranted = true;
}

int32_t PermissionManager::VerifyMultiPermissions(const std::shared_ptr<PermissionItem> &item)
{
    if (g_isAllGranted) {
        // Applicable only for skipping permission checks in fuzz test cases.
        return BT_NO_ERROR;
    }
    if (item == nullptr) {
        return BT_NO_ERROR;
    }

    int32_t apiVerIdx = API_VERSION_10;
    int32_t apiVersion = API_VERSION_10;
    if (IsHapCaller()) {
        apiVersion = GetApiVersion();
        apiVerIdx =
            (apiVersion >= API_VERSION_10 || apiVersion == API_VERSION_INVALID) ? API_VERSION_10 : API_VERSION_9;
    }

    std::set<std::string> perm = item->GetPermissions(apiVerIdx);
    // Empty means the caller's version is earlier than the start version of the interface.
    CHECK_AND_RETURN_LOG_RET(!perm.empty(), BT_ERR_PERMISSION_FAILED,
        "[PERMISSION] the caller's api version is unsupported, apiVersion(%{public}d)", apiVersion);

    // Check whether the caller is a system HAP.
    if (item->SystemHapNeeded() && !IsSystemHap()) {
        HILOGE("[PERMISSION] system hap needed.");
        return BT_ERR_SYSTEM_PERMISSION_FAILED;
    }

    // Check permissions.
    CHECK_AND_RETURN_LOG_RET(IsPermissionsGranted(perm), BT_ERR_PERMISSION_FAILED,
        "[PERMISSION] check permissions failed.");

    return BT_NO_ERROR;
}

int32_t PermissionManager::GetApiVersion()
{
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    return GetApiVersion(tokenId);
}

int32_t PermissionManager::GetApiVersion(uint32_t tokenId)
{
    HITRACE_METER(BT_TRACE_TAG);
    if (!IsHapCaller(tokenId)) {
        return API_VERSION_INVALID;
    }
    HapTokenInfo hapTokenInfo;
    if (AccessTokenKit::GetHapTokenInfo(tokenId, hapTokenInfo) != AccessTokenKitRet::RET_SUCCESS) {
        HILOGE("[PERMISSION] GetHapTokenInfo failed.");
        return API_VERSION_INVALID;
    }
    return hapTokenInfo.apiVersion;
}

int32_t PermissionManager::GetCallingUid()
{
    return IPCSkeleton::GetCallingUid();
}

std::string PermissionManager::GetCallingName()
{
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    return GetCallingName(tokenId);
}

std::string PermissionManager::GetCallingName(const uint32_t &tokenId)
{
    HITRACE_METER(BT_TRACE_TAG);
    ATokenTypeEnum callingType = AccessTokenKit::GetTokenTypeFlag(tokenId);
    switch (callingType) {
        case ATokenTypeEnum::TOKEN_HAP : {
            HapTokenInfo hapTokenInfo;
            if (AccessTokenKit::GetHapTokenInfo(tokenId, hapTokenInfo) == AccessTokenKitRet::RET_SUCCESS) {
                return hapTokenInfo.bundleName;
            }
            HILOGE("[PERMISSION] callingType(%{public}d), GetHapTokenInfo failed.", callingType);
            return "";
        }
        case ATokenTypeEnum::TOKEN_SHELL:
        case ATokenTypeEnum::TOKEN_NATIVE: {
            NativeTokenInfo nativeTokenInfo;
            if (AccessTokenKit::GetNativeTokenInfo(tokenId, nativeTokenInfo) == AccessTokenKitRet::RET_SUCCESS) {
                return nativeTokenInfo.processName;
            }
            HILOGE("[PERMISSION] callingType(%{public}d), GetNativeTokenInfo failed.", callingType);
            return "";
        }
        default:
            HILOGE("[PERMISSION] callingType(%{public}d) is invalid.", callingType);
            return "";
    }
}

bool PermissionManager::IsHapCaller(void)
{
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    return IsHapCaller(tokenId);
}

bool PermissionManager::IsHapCaller(uint32_t &tokenId)
{
    HITRACE_METER(BT_TRACE_TAG);
    ATokenTypeEnum callingType = AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (callingType == ATokenTypeEnum::TOKEN_HAP) {
        return true;
    }
    return false;
}

bool PermissionManager::IsNativeCaller(void)
{
    HITRACE_METER(BT_TRACE_TAG);
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    return IsNativeCaller(tokenId);
}

bool PermissionManager::IsNativeCaller(const uint32_t &tokenId)
{
    ATokenTypeEnum callingType = AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (callingType == ATokenTypeEnum::TOKEN_NATIVE || callingType == ATokenTypeEnum::TOKEN_SHELL) {
        return true;
    }
    return false;
}

bool PermissionManager::IsSystemHap()
{
    HITRACE_METER(BT_TRACE_TAG);
    uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    return IsSystemHap(fullTokenId);
}

bool PermissionManager::IsSystemHap(const uint64_t &fullTokenId)
{
    bool isSystemApp = TokenIdKit::IsSystemAppByFullTokenID(fullTokenId);
    ATokenTypeEnum callingType = AccessTokenKit::GetTokenTypeFlag(static_cast<uint32_t>(fullTokenId));
    if (callingType == ATokenTypeEnum::TOKEN_HAP && !isSystemApp) {
        return false;
    }
    return true;
}

bool PermissionManager::IsUseRealAddr()
{
    uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    return IsUseRealAddr(fullTokenId);
}

static const std::map<std::string, std::string> REAL_ADDRESS_WHITE_MAP = {
};

#ifdef MMI_NOT_DISCOVER_BLE
static const std::map<std::string, std::string> MMI_NOT_DISCOVERY_BLE_WHITE_MAP = {
};
#endif

__attribute__((weak)) std::string GetHapAppIdentifier(int32_t userId, const std::string &bundleName)
{
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        HILOGE("fail to get system ability mgr.");
        return "";
    }
    auto remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!remoteObject) {
        HILOGE("fail to get bundle manager proxy.");
        return "";
    }
    sptr<AppExecFwk::IBundleMgr> iBundleMgr = iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    if (iBundleMgr == nullptr) {
        HILOGE("Failed to get bundle manager proxy.");
        return "";
    }
    AppExecFwk::BundleInfo bundleInfo;
    ErrCode ret = iBundleMgr->GetBundleInfoV9(bundleName,
        static_cast<int>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_SIGNATURE_INFO), bundleInfo, userId);
    if (ret != ERR_OK) {
        HILOGE("failed to get bundle info for %{public}s due to errCode %{public}d", bundleName.c_str(), ret);
        return "";
    }
    return bundleInfo.signatureInfo.appIdentifier;
}

static bool IsInRealAddrTmpWhiteList(uint32_t tokenId)
{
    Security::AccessToken::HapTokenInfo hapTokenInfo;
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenType != Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        return false;
    }
    int accessRet = Security::AccessToken::AccessTokenKit::GetHapTokenInfo(tokenId, hapTokenInfo);
    if (accessRet != 0) {
        // not hap application
        return false;
    }

    int32_t userId = hapTokenInfo.userID;
    std::string bundleName = hapTokenInfo.bundleName;
    if (REAL_ADDRESS_WHITE_MAP.find(bundleName) == REAL_ADDRESS_WHITE_MAP.end()) {
        // hap is not in white list
        return false;
    }

    std::string appId = GetHapAppIdentifier(userId, bundleName);
    if (appId.empty()) {
        HILOGE("can not get appIdentifier.");
        return false;
    }
    if (appId != REAL_ADDRESS_WHITE_MAP.at(bundleName)) {
        HILOGE("appId error %{public}s", appId.c_str());
        return false;
    }
    return true;
}

bool PermissionManager::IsUseRealAddr(const uint64_t &fullTokenId)
{
    HITRACE_METER(BT_TRACE_TAG);
    uint32_t tokenId = static_cast<uint32_t>(fullTokenId);
    ATokenTypeEnum callingType = AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (callingType != ATokenTypeEnum::TOKEN_HAP) {
        return true;
    }
    bool isSystemApp = TokenIdKit::IsSystemAppByFullTokenID(fullTokenId);
    int32_t apiVersion = GetApiVersion(tokenId);
    if (apiVersion < API_VERSION_10) {
        return true;
    }
    if (apiVersion >= API_VERSION_12 && VerifyPermission(GET_BLUETOOTH_PEERS_MAC, tokenId)) {
        return true;
    }
    if (isSystemApp && IsInRealAddrTmpWhiteList(tokenId)) {
        return true;
    }
    return false;
}

std::shared_ptr<PermissionItem> PermissionManager::CreateItem(bool isSystemApi, const std::set<std::string> api9PermSet,
    const std::set<std::string> api10PermSet)
{
    return std::make_shared<PermissionItem>(isSystemApi, api9PermSet, api10PermSet);
}

bool PermissionManager::IsNeededDiscarded(DiscardHapType discardHapType, int startApiVersion)
{
    if (GetApiVersion() < startApiVersion) {
        return false;
    }
    if (discardHapType == ONLY_THIRDLY_HAP) {
        if (!IsSystemHap()) {
            return true;
        }
    } else {
        return true;
    }
    return false;
}

static const std::map<std::string, std::string> ADV_WHITE_MAP = { // {bundleName, appId}
    {"com.gemdale.ghome", "5765880207854665327"}, // 想家社区
    {"com.copm.owner", "6917573033807628697"}, // 优你家Plus
    {"com.hrmos.esenxt", "5765880207855224985"}, // 易视云APP
};

bool PermissionManager::IsInAdvWhiteList(uint32_t tokenId)
{
    Security::AccessToken::HapTokenInfo hapTokenInfo;
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenType != Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        return false;
    }
    int accessRet = Security::AccessToken::AccessTokenKit::GetHapTokenInfo(tokenId, hapTokenInfo);
    if (accessRet != 0) {
        // not hap application
        return false;
    }

    int32_t userId = hapTokenInfo.userID;
    std::string bundleName = hapTokenInfo.bundleName;
    if (ADV_WHITE_MAP.find(bundleName) == ADV_WHITE_MAP.end()) {
        // hap is not in white list
        return false;
    }

    std::string appId = GetHapAppIdentifier(userId, bundleName);
    if (appId.empty()) {
        HILOGE("can not get appIdentifier.");
        return false;
    }
    if (appId != ADV_WHITE_MAP.at(bundleName)) {
        HILOGE("appId error %{public}s", appId.c_str());
        return false;
    }
    return true;
}

#ifdef MMI_NOT_DISCOVER_BLE
bool PermissionManager::IsInMmiTmpWhiteList(uint64_t tokenId)
{
    uint32_t checkTokenId = static_cast<uint64_t>(tokenId);
    Security::AccessToken::HapTokenInfo hapTokenInfo;
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(checkTokenId);
    if (tokenType != Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        return false;
    }
    int accessRet = Security::AccessToken::AccessTokenKit::GetHapTokenInfo(checkTokenId, hapTokenInfo);
    if (accessRet != 0) {
        // not hap application
        return false;
    }

    int32_t userId = hapTokenInfo.userID;
    std::string bundleName = hapTokenInfo.bundleName;
    if (MMI_NOT_DISCOVERY_BLE_WHITE_MAP.find(bundleName) == MMI_NOT_DISCOVERY_BLE_WHITE_MAP.end()) {
        // hap is not in white list
        return false;
    }

    std::string appId = GetHapAppIdentifier(userId, bundleName);
    if (appId.empty()) {
        HILOGE("can not get appIdentifier.");
        return false;
    }
    if (appId != MMI_NOT_DISCOVERY_BLE_WHITE_MAP.at(bundleName)) {
        HILOGE("appId error %{public}s", appId.c_str());
        return false;
    }
    return true;
}
#endif

}  // namespace bluetooth
}  // namespace OHOS
