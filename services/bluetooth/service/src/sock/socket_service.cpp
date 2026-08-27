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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_socket"
#endif

#include "socket_def.h"
#include "socket_service.h"
#include "bluetooth_connection_manager.h"
#include "class_creator.h"
#include "datetime_ex.h"
#include "ipc_skeleton.h"
#include "log.h"
#include "btif_sock.h"
#include "service_util.h"
#include "hal_util.h"
#include "bluetooth_hw_interface.h"
#include "bt_chr_dft_exception.h"
#include "bt_chr_ue_manager.h"
#include "permission_manager.h"
#include "thread_util.h"
#include "bluetooth_errorcode.h"
#include "log_utils.h"

#include "hw_bt_chr.h"
#include "control_intercept_plugin.h"
#include "bluetooth_resource_manager.h"

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;
const std::string SPP_VERSION = "0.4.1";

static const btsock_interface_t* sBluetoothSocketInterface = nullptr;

static btsock_type_t ConvertBtSockType(int type)
{
    btsock_type_t ret;
    switch (type) {
        case SOCK_RFCOMM:
            ret = BTSOCK_RFCOMM;
            break;
        case SOCK_L2CAP:
            ret = BTSOCK_L2CAP;
            break;
        case SOCK_L2CAP_LE:
            ret = BTSOCK_L2CAP_LE;
            break;
        default:
            ret = BTSOCK_RFCOMM;
            break;
    }
    return ret;
}


class SocketServiceObserver : public BluetoothHwInterface::SocketConnectionObserver {
public:
    explicit SocketServiceObserver(const std::string &addr, const Uuid &uuid,
        std::weak_ptr<IBtClientSocketCallback> callback);
    ~SocketServiceObserver() override;

    void ConnectCallback(const StackCallbackParam &param) override;

    inline std::string GetAddress(void) const
    {
        return addr_;
    }

    inline Uuid GetUuid(void) const
    {
        return uuid_;
    }

    inline std::weak_ptr<IBtClientSocketCallback> GetCallback(void) const
    {
        return callback_;
    }

private:
    std::string addr_;
    Uuid uuid_;
    std::weak_ptr<IBtClientSocketCallback> callback_;
};


SocketServiceObserver::SocketServiceObserver(const std::string &addr, const Uuid &uuid,
    std::weak_ptr<IBtClientSocketCallback> callback)
    : addr_(addr),
      uuid_(uuid),
      callback_(callback)
{}

SocketServiceObserver::~SocketServiceObserver()
{
    HILOGI("~SocketServiceObserver");
}

void SocketServiceObserver::ConnectCallback(const StackCallbackParam &param)
{
    HILOGI_TIME_LIMIT(param.addr.ToStringForLogging().c_str(),
        "addr: %{public}s, status =%{public}d, result=%{public}d.",
        param.addr.ToStringForLogging().c_str(), param.status, param.result);
    const RawAddress address = ServiceUtil::AddrFromStack(param.addr);
    Uuid uuid = Uuid::ConvertFromMostAndLeastBit(param.msb, param.lsb);
    if (address == RawAddress(addr_) && (uuid == uuid_)) {
        HILOGI("Match addr & uuid, report socket ConnectCallback, addr: %{public}s",
            GetEncryptAddr(addr_).c_str());
        IBtClientSocketCallback::CallbackParam callbackParam = {
            .addr = address,
            .uuid = uuid,
            .status = param.status,
            .result = param.result,
            .type = param.type,
            .psm = param.psm,
        };
        WPTR_SOCKET_CBACK(callback_, OnConnectionStateCallback, callbackParam);
    }
}

// !!! The following variables are only used in inner thread.
static std::list<std::shared_ptr<SocketServiceObserver>> serviceObservers;

SocketService::SocketService()
    : utility::Context(PROFILE_NAME_SPP, SPP_VERSION)
{
    HILOGI("ProfileService: SocketService Create");
}

SocketService::~SocketService()
{
    HILOGI("ProfileService:SocketService Destroy");
}

utility::Context *SocketService::GetContext()
{
    return this;
}

void SocketService::Enable()
{
    DoInSocketThread([this] {this->EnableNative(); });
}

void SocketService::EnableNative()
{
    HILOGI("[SocketService] get_profile_interface");
    const bt_interface_t* bt_interface = nullptr;
    int status = hal_util_load_bt_library(&bt_interface);
    if (status || bt_interface == nullptr) {
        HILOGE("[SocketService] Failed to open the Bluetooth module");
        return;
    }
    sBluetoothSocketInterface = static_cast<btsock_interface_t*>(
        const_cast<void *>(bt_interface->get_profile_interface(BT_PROFILE_SOCKETS_ID)));
    if (!sBluetoothSocketInterface) {
        HILOGE("Error getting socket BT_PROFILE_SOCKETS_ID interface");
        return;
    }
    GetContext()->OnEnable(PROFILE_NAME_SPP, true);
}

void SocketService::Disable()
{
    DoInSocketThread([this] {this->DisableNative();});
}

void SocketService::DisableNative()
{
    ShutDownInternal();
}

int SocketService::Connect(const RawAddress &device)
{
    (void)device;
    return 0;
}

int SocketService::Disconnect(const RawAddress &device)
{
    (void)device;
    return 0;
}

std::list<RawAddress> SocketService::GetConnectDevices()
{
    std::list<RawAddress> devList;

    return devList;
}

int SocketService::GetConnectState()
{
    return 0;
}

int SocketService::GetMaxConnectNum()
{
    return 0;
}

// convert the flag
// mExcludeSdp, mAuthMitm, mMin16DigitPin is set to false.
static int GetSecurityFlags(int securityFlag, int type)
{
    int flags = 0;
    if (securityFlag & SOCK_FLAG_AUTHENTICATION) {
        flags |= BTSOCK_FLAG_AUTH;
    }
    if (securityFlag & SOCK_FLAG_ENCRYPTION) {
        flags |= BTSOCK_FLAG_ENCRYPT;
    }
    if (type == SOCK_L2CAP_LE) {
        flags |= BTSOCK_FLAG_NO_SDP;
    }
    return flags;
}

void SocketService::ReportSocketConnectChr(
    const std::string &addr, const Uuid &uuid, int type, int psm, std::string callingName)
{
    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_SOCKET_CLENT_CONN, RawAddress(addr), psm,
        uuid.ToString(), callingName);
    if (type == SOCK_RFCOMM || type == SOCK_L2CAP_LE) {
        BtChrAddSocketCallingRecord(addr, uuid.ToString(), callingName);
    }
    if (type == SOCK_L2CAP_LE) {
        BtChrBleConnData(addr, CHR_BLE_COC, callingName);
    }
}

int SocketService::IsLocalDeviceConnectAllowed(const std::string &addr, const Uuid &uuid, int securityFlag,
    int type, int psm)
{
    ControlInterceptMessage msg {
        .addr = addr,
        .isSystemHap = PermissionManager::IsSystemHap(),
        .isNativeCaller = PermissionManager::IsNativeCaller(),
        .pid = IPCSkeleton::GetCallingPid(),
        .uid = IPCSkeleton::GetCallingUid(),
    };
    CHECK_AND_RETURN_LOG_RET(ControlInterceptIsAllowedSocketConn(msg), SOCK_INVALID_FD,
        "Restricted by control intercept");
    int socketFd = SOCK_INVALID_FD;
    std::string callingName = PermissionManager::GetCallingName();
    auto connectionManager = BluetoothConnectionManager::GetInstance();
    if (connectionManager->ShouldRefuseSocketConnectEX(addr, uuid, callingName)) {
        SocketConnectError(addr, uuid, CONNECT_CONNECTING_LINK_LIMIT, type, psm);
        return socketFd;
    }
    std::uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    if (connectionManager->ShouldRefuseSocketConnectTooFrequently(tokenId, GetSecondsSince1970ToNow(), addr)) {
        HILOGE("[SocketService::Connect] Failed connect acl link because of too frequent.");
        BtChrSocketExcpEvent(addr, uuid.ToString(),
            CLIENT_CONNECT_REFUSE, SOCKET_SUBERROR_TOO_FREQUENTLY, callingName);
        SocketConnectError(addr, uuid, CONNECT_TOO_FREQUENTLY, type, psm);
        return socketFd;
    }
    return RET_NO_ERROR;
}

int SocketService::Connect(const std::string &addr, const Uuid &uuid, int securityFlag, int type, int psm)
{
    HILOGI("[SocketService] securityFlag: %{public}d, type: %{public}d", securityFlag, type);
    int ret = IsLocalDeviceConnectAllowed(addr, uuid, securityFlag, type, psm);
    if (ret != RET_NO_ERROR) {
        HILOGE("[SocketService::Connect] LocalDevice Connect not Allowed.");
        return ret;
    }
    int socketFd = SOCK_INVALID_FD;
    std::string callingName = PermissionManager::GetCallingName();
    int32_t uid = IPCSkeleton::GetCallingUid();
    if (!sBluetoothSocketInterface) {
        HILOGE("[SocketService] sBluetoothSocketInterface invalid");
        SocketConnectError(addr, uuid, SOCKET_INTERFACE_INVALID, type, psm);
        return socketFd;
    }
    STACK::RawAddress rawAddr;
    if (!STACK::RawAddress::FromString(addr, rawAddr)) {
        HILOGE("[SocketService] addr error");
        SocketConnectError(addr, uuid, ADDR_ERROR, type, psm);
        return socketFd;
    }
    ReportSocketConnectChr(addr, uuid, type, psm, callingName);
    BtChrAddConnSceneInfo(addr, PAIR_TYPE_USER_CONNECT, callingName, 1);
    auto resourceMgr = BluetoothResourceManager::GetInstance();
    if (resourceMgr) {
        resourceMgr->SendSensingStateChanged(SOCKET_REGISTER_APPLICATION_PKGNAME,
            SensingInfo(uuid.ToString(), callingName));
    }
    BluetoothHwInterface::GetInstance()->KeepBleScanInConn(callingName, uid,
        (type == SOCK_L2CAP_LE) ? BT_TRANSPORT_LE : BT_TRANSPORT_BR_EDR, rawAddr);
    // uuid is of the same type used by the stack interface.
    int result = sBluetoothSocketInterface->connect(&rawAddr, ConvertBtSockType(type), &uuid, psm, &socketFd,
        GetSecurityFlags(securityFlag, type), uid);
    if (result != RET_NO_ERROR) {
        HILOGE("[SocketService] connect failed");
        if (type == SOCK_L2CAP_LE) {
            SocketConnectError(addr, uuid, -(CONNECT_ERR_BASE + result), type, psm);
        }
        socketFd = SOCK_INVALID_FD;
    }
    return socketFd;
}

int SocketService::Listen(const std::string &name, const Uuid &uuid, int securityFlag, int type, int channel)
{
    ControlInterceptMessage msg {
        .isSystemHap = PermissionManager::IsSystemHap(),
        .isNativeCaller = PermissionManager::IsNativeCaller(),
        .pid = IPCSkeleton::GetCallingPid(),
        .uid = IPCSkeleton::GetCallingUid(),
    };
    CHECK_AND_RETURN_LOG_RET(ControlInterceptIsAllowedSocketListen(msg), SOCK_INVALID_FD,
        "Restricted by control intercept");
    int socketFd = SOCK_INVALID_FD;
    if (!sBluetoothSocketInterface) {
        HILOGE("[SocketService] sBluetoothSocketInterface invalid");
        return socketFd;
    }
    int32_t uid = IPCSkeleton::GetCallingUid();
    std::string callingName = PermissionManager::GetCallingName();
    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_SOCKET_SERVER_CONN, RawAddress(""), -1,
        uuid.ToString(), callingName);
    int ret = sBluetoothSocketInterface->listen(ConvertBtSockType(type), name.c_str(),
        &uuid, channel, &socketFd, GetSecurityFlags(securityFlag, type), uid);
    if (ret != RET_NO_ERROR) {
        HILOGD("[SocketService] listen failed, ret=%{public}d", ret);
        socketFd = SOCK_INVALID_FD;
    }
    HILOGD("[SocketService] socketFd:%{public}d", socketFd);

    auto resourceMgr = BluetoothResourceManager::GetInstance();
    if (resourceMgr) {
        resourceMgr->SendSensingStateChanged(SOCKET_REGISTER_APPLICATION_PKGNAME,
            SensingInfo(uuid.ToString(), callingName));
    }

    return socketFd;
}

void SocketService::ShutDownInternal()
{
    btif_sock_cleanup();
    GetContext()->OnDisable(PROFILE_NAME_SPP, true);
    // Clear all resources
    if (serviceObservers.empty()) {
        return;
    }
    for (std::shared_ptr<SocketServiceObserver> it : serviceObservers) {
        BluetoothHwInterface::GetInstance()->RemoveSocketObserver(it);
    }
}

void SocketService::UpdateCocConnectionParams(const Bluetooth::BluetoothSocketCocInfo &info)
{
    const bt_interface_t *btInterface = nullptr;
    uint16_t params[COC_PARAMS_LEN] = {};
    int ret = hal_util_load_bt_library(&btInterface);
    if (ret != BT_STATUS_SUCCESS || btInterface == nullptr) {
        HILOGE("Load bluetooth library failed");
        return;
    }

    const bthwif_interface_t *bthwif =
        reinterpret_cast<const bthwif_interface_t*>(btInterface->get_profile_interface(BT_VENDER_INTERFACE_ID));
    if (bthwif == nullptr) {
        HILOGE("bthwif is null");
        return;
    }

    RawAddress device = RawAddress(info.addr);
    params[0] = info.minInterval; // 0 is param index 0 for mini interval
    params[1] = info.maxInterval; // 1 is param index 1 for max interval
    params[2] = info.peripheralLatency; // 2 is param index 2 for max interval
    params[3] = info.supervisionTimeout; // 3 is param index 3 for supervision timeout
    params[4] = info.minConnEventLen; // 4 is param index 4 for min connect event length
    params[5] = info.maxConnEventLen;  // 5 is param index 5 for max connect event length

    bthwif->updateCocConnectionParams(ServiceUtil::AddrToStack(device), params, COC_PARAMS_LEN);
}

int SocketService::RegisterConnectionObserver(const std::string &addr, const Uuid &uuid,
    std::shared_ptr<IBtClientSocketCallback> callback)
{
    STACK::RawAddress rawAddr;
    if (!STACK::RawAddress::FromString(addr, rawAddr)) {
        HILOGE("[SocketService] addr error");
        return BT_ERR_INTERNAL_ERROR;
    }

    DoInSocketThread(std::bind(
        [](const std::string addr, const Uuid uuid, std::shared_ptr<IBtClientSocketCallback> callback) {
            // Remove register that are not cleaned up when Bluetooth is turned off.
            for (auto it = serviceObservers.begin(); it != serviceObservers.end();) {
                if (((*it)->GetAddress() == addr) && ((*it)->GetUuid() == uuid)) {
                     HILOGE("register is not cleared");
                     it = serviceObservers.erase(it);
                } else {
                    ++it;
                }
            }
            std::shared_ptr<SocketServiceObserver> serviceObserver =
                std::make_shared<SocketServiceObserver>(addr, uuid, callback);
            BluetoothHwInterface::GetInstance()->AddSocketObserver(serviceObserver);
            serviceObservers.emplace(serviceObservers.end(), serviceObserver);
        },
        addr, uuid, callback));

    const bthwif_interface_t *bthwif = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    if (!bthwif) {
        HILOGE("Get bthwif_interface_t fail");
        return BT_ERR_INTERNAL_ERROR;
    }
    bthwif->registerConnection(rawAddr, uuid);
    return BT_NO_ERROR;
}

int SocketService::UnregisterConnectionObserver(const std::string &addr, const Uuid &uuid,
    std::shared_ptr<IBtClientSocketCallback> callback)
{
    STACK::RawAddress address;
    if (!STACK::RawAddress::FromString(addr, address)) {
        HILOGE("[SocketService] addr error");
        return BT_ERR_INTERNAL_ERROR;
    }

    DoInSocketThread(std::bind(
        [](const std::string addr, const Uuid uuid) {
            // Remove register that are not cleaned up when Bluetooth is turned off.
            auto it = std::find_if(serviceObservers.begin(), serviceObservers.end(),
                [&addr, &uuid](std::shared_ptr<SocketServiceObserver> observer) {
                    return (observer->GetAddress() == addr) && (observer->GetUuid() == uuid);
                });
            if (it != serviceObservers.end()) {
                serviceObservers.erase(it);
            }
        },
        addr, uuid));

    const bthwif_interface_t *bthwif = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    if (!bthwif) {
        HILOGE("Get bthwif_interface_t fail");
        return BT_ERR_INTERNAL_ERROR;
    }
    bthwif->unRegisterConnection(address, uuid);
    return BT_NO_ERROR;
}

bool SocketService::IsAllowSocketConnect(int32_t socketType, const std::string &addr)
{
    HILOGI("socketType: %{public}d, addr: %{public}s", socketType, GET_ENCRYPT_STR_ADDR(addr));
    if (socketType == SOCK_L2CAP_LE) {
        return true;
    }
    auto connectionManager = BluetoothConnectionManager::GetInstance();
    return !connectionManager->ReachingMaxBrLinks(addr);
}

__attribute__((no_sanitize("cfi")))
void SocketService::SocketConnectError(const std::string &addr, const Uuid &uuid, int errorCode, int type, int psm)
{
    DoInSocketThread(std::bind(
        [](const std::string &addr, const Uuid &uuid, int psm, int errorCode, int type) {
            if (serviceObservers.empty()) {
                return;
            }
            for (auto it = serviceObservers.begin(); it != serviceObservers.end(); ++it) {
                if (((*it)->GetAddress() == addr) && ((*it)->GetUuid() == uuid)) {
                    IBtClientSocketCallback::CallbackParam callbackParam = {
                        .addr = RawAddress(addr),
                        .uuid = uuid,
                        .status = -1,
                        .result = errorCode,
                        .type = type,
                        .psm = psm,
                    };
                    WPTR_SOCKET_CBACK((*it)->GetCallback(), OnConnectionStateCallback, callbackParam);
                    return;
                }
            }
        },
    addr, uuid, psm, errorCode, type));
}
REGISTER_CLASS_CREATOR(SocketService);
}  // namespace bluetooth
}  // namespace OHOS