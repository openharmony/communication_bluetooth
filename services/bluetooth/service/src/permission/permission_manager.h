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

#ifndef PERMISSION_MANAGER_H
#define PERMISSION_MANAGER_H

#include <map>
#include <string>
#include "accesstoken_kit.h"
#include "permission_item.h"
#include "tokenid_kit.h"

#ifdef CHECK_PERM
#undef CHECK_PERM
#endif
#define CHECK_PERM(isSystemApi, api9Perm, api10Perm) PermissionManager::CreateItem(isSystemApi, api9Perm, api10Perm)

#ifdef MULTI_PERM
#undef MULTI_PERM
#endif
#define MULTI_PERM(...) {__VA_ARGS__}

#define CHECK_PERMISSION_AND_EXECUTE_FUNC_RETURN(STUB_CLASS)                               \
do {                                                                                \
    HILOGD("cmd(%{public}u), flags(%{public}d)", code, option.GetFlags());          \
    if (STUB_CLASS::GetDescriptor() != data.ReadInterfaceToken()) {                 \
        HILOGE("interface token check failed.");                                    \
        return BT_ERR_IPC_TRANS_FAILED;                                             \
    }                                                                               \
    auto itFunc = memberFuncMap_.find(code);                                        \
    if (itFunc == memberFuncMap_.end()) {                                           \
        HILOGE("code(%{public}d) is not exist.", code);                             \
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);           \
    }                                                                               \
    auto memberFunc = itFunc->second.first;                                         \
    if (memberFunc == nullptr) {                                                    \
        HILOGE("memberFunc is nullptr. code(%{public}d)", code);                    \
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);           \
    }                                                                               \
    if (itFunc->second.second != nullptr) {                                         \
        int errCode = PermissionManager::VerifyMultiPermissions(itFunc->second.second); \
        if (errCode != BT_NO_ERROR) {                                               \
            HILOGE("[PERMISSION] failed. code(%{public}d), callingName(%{public}s)", \
                code, PermissionManager::GetCallingName().c_str());                 \
            reply.WriteInt32(errCode);                                              \
            return BT_NO_ERROR;                                                     \
        }                                                                           \
    }                                                                               \
    return memberFunc(this, data, reply);                                           \
} while (0)

namespace OHOS {
namespace Bluetooth {

// Bluetooth permission.
const std::string GET_BLUETOOTH_PEERS_MAC =
    "ohos.permission.GET_BLUETOOTH_PEERS_MAC"; // since API 12, system_basic, system_grant
const std::string GET_BLUETOOTH_LOCAL_MAC =
    "ohos.permission.GET_BLUETOOTH_LOCAL_MAC"; // since API 11, system_basic, system_grant
const std::string ACCESS_BLUETOOTH = "ohos.permission.ACCESS_BLUETOOTH"; // since API 10, normal, user_grant
const std::string USE_BLUETOOTH = "ohos.permission.USE_BLUETOOTH"; // since API 8, normal, system_grant
const std::string DISCOVER_BLUETOOTH = "ohos.permission.DISCOVER_BLUETOOTH"; // since API 8, normal, system_grant
const std::string MANAGE_BLUETOOTH = "ohos.permission.MANAGE_BLUETOOTH"; // since API 7, system_basic, system_grant
constexpr const char *PERSISTENT_BLUETOOTH_PEERS_MAC = "ohos.permission.PERSISTENT_BLUETOOTH_PEERS_MAC"; // since API 16
constexpr const char *MANAGE_BLUETOOTH_ADVERTISER_NAME = "ohos.permission.MANAGE_BLUETOOTH_ADVERTISER_NAME";
// since API 23, system_basic, system_grant


// Location permission.
const std::string APPROXIMATELY_LOCATION = "ohos.permission.APPROXIMATELY_LOCATION"; // unused from API 10
const std::string LOCATION = "ohos.permission.LOCATION"; // unused from API 10

enum DiscardHapType {
    ONLY_THIRDLY_HAP = 0,
    SYSTEM_AND_THIRDLY_HAP = 1,
};

/**
* @Description get the appIdentifier of hap.
*
* @param userId userId of hap
* @param bundleName bundleName of hap
* @return string appIdentifier of hap
*/
std::string GetHapAppIdentifier(int32_t userId, const std::string &bundleName);

class PermissionManager {
public:
    /**
     * @Description Verify permission.
     *
     * @param permissionName Permission name.
     * @return true - permission granted, false - permission denied.
     */
    static bool VerifyPermission(const std::string &permissionName);

    /**
     * @Description Verify permission with token id.
     *
     * @param permissionName Permission name.
     * @param tokenId The app's token id.
     * @param isNeedRecord verify is need to record.
     * @return true - permission granted, false - permission denied.
     */
    static bool VerifyPermission(
        const std::string &permissionName, const uint32_t &tokenId, bool isNeedRecord = false);

    /**
     * @Description Set the skip permission check flag.
     *
     */
    static void SetAllGranted();

    /**
     * @Description Verify permissions.
     *
     * @param item Permissions.
     * @return BT_NO_ERROR - permission granted, otherwise - permission denied
     */
    static int32_t VerifyMultiPermissions(const std::shared_ptr<PermissionItem> &item);

    /**
     * @Description Get api version.
     *
     * @return which version of the SDK is used to develop this hap.
     */
    static int32_t GetApiVersion();

    /**
     * @Description Get api version.
     *
     * @param tokenId The app's token id.
     * @return int32_t Api version.
     */
    static int32_t GetApiVersion(uint32_t tokenId);

    /**
     * @Description Get pid_t id
     *
     * @return int32_t pid_t.
     */
    static int32_t GetCallingUid();

    /**
     * @Description Get BundleName.
     *
     * @return string BundleName.
     */
    static std::string GetCallingName();

    /**
     * @Description Get BundleName.
     *
     * @param tokenId The app's token id.
     * @return string BundleName.
     */
    static std::string GetCallingName(const uint32_t &tokenId);

    /**
     * @Description Check whether the caller is hap.
     *
     * @param tokenId The app's token id.
     * @return true or false.
     */
    static bool IsHapCaller(void);

    /**
     * @DescriptionCheck whether the caller is hap with token id.
     *
     * @param tokenId The app's token id.
     * @return true or false.
     */
    static bool IsHapCaller(uint32_t &tokenId);

    /**
     * @DescriptionCheck whether the caller is native process.
     *
     * @return true or false.
     */
    static bool IsNativeCaller(void);

    /**
     * @DescriptionCheck whether the caller is native with token id.
     *
     * @param fullTokenId The app's fulltoken id.
     * @return true or false.
     */
    static bool IsNativeCaller(const uint32_t &tokenId);

    /**
     * @Description Check is system hap application .
     *
     * @return true or false.
     */
    static bool IsSystemHap();

    /**
     * @Description Check is system hap application with token id.
     *
     * @return true or false.
     */
    static bool IsSystemHap(const uint64_t &fullTokenId);

    /**
     * @Description Check whether to use real address.
     *
     * @return true or false.
     */
    static bool IsUseRealAddr();

    /**
     * @Description Check whether to use real address with full token id.
     *
     * @return true or false.
     */
    static bool IsUseRealAddr(const uint64_t &fullTokenId);

    /**
     * @Description Create permission items.
     *
     * @return true or false.
     */
    static std::shared_ptr<PermissionItem> CreateItem(
        bool isSystemApi, const std::set<std::string> api9PermSet, const std::set<std::string> api10PermSet);

    /**
     * @Description check whether the interface that needs to be discarded
     *
     * @param discardHapType discard for thirdlyhap or all
     * @param startApiversion begin Api Version to discard
     * @return true or false.
     */
    static bool IsNeededDiscarded(DiscardHapType discardHapType, int startApiVersion);

    static bool IsInAdvWhiteList(uint32_t tokenId);

#ifdef MMI_NOT_DISCOVER_BLE
    static bool IsInMmiTmpWhiteList(uint64_t tokenId);
#endif

private:
    static bool IsPermissionsGranted(const std::set<std::string> &permissions);
};

}  // namespace bluetooth
}  // namespace OHOS
#endif