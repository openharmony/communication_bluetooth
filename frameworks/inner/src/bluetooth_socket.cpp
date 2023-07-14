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

#include <sys/socket.h>
#include <string>
#include <unistd.h>
#include "bluetooth_log.h"
#include "bluetooth_host.h"
#include "bluetooth_host_proxy.h"
#include "bluetooth_utils.h"
#include "bluetooth_socket_proxy.h"
#include "hisysevent.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "securec.h"
#include "system_ability_definition.h"
#include "raw_address.h"
#include "bluetooth_socket.h"

namespace OHOS {
namespace Bluetooth {
const int LENGTH = 18;
const int MIN_BUFFER_SIZE_TO_SET = 4 * 1024; // 4KB
const int MAX_BUFFER_SIZE_TO_SET = 50 * 1024; // 50KB
const int PSM_BUFFER_SIZE = 4;
struct ClientSocket::impl {
    impl(const BluetoothRemoteDevice &addr, UUID uuid, BtSocketType type, bool auth);
    impl(int fd, std::string address, BtSocketType type);
    ~impl()
    {
        if (proxy_ != nullptr && proxy_->AsObject() != nullptr) {
            proxy_->AsObject()->RemoveDeathRecipient(deathRecipient_);
        } else {
            HILOGI("failed: no proxy_");
        }

        if (fd_ > 0) {
            shutdown(fd_, SHUT_RD);
            shutdown(fd_, SHUT_WR);
            HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::BLUETOOTH, "SPP_CONNECT_STATE",
                HiviewDFX::HiSysEvent::EventType::STATISTIC, "ACTION", "close", "ID", fd_, "ADDRESS", "empty",
                "PID", IPCSkeleton::GetCallingPid(), "UID", IPCSkeleton::GetCallingUid());
            HILOGI("fd closed, fd_: %{pubilc}d", fd_);
            close(fd_);
            fd_ = -1;
        }
    }

    void Close()
    {
        HILOGI("starts");
        if (socketStatus_ == SOCKET_CLOSED) {
            HILOGW("The socketStatus_ is already SOCKET_CLOSED");
            return;
        } else {
            socketStatus_ = SOCKET_CLOSED;
            if (fd_ > 0) {
                shutdown(fd_, SHUT_RD);
                shutdown(fd_, SHUT_WR);
                HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::BLUETOOTH, "SPP_CONNECT_STATE",
                    HiviewDFX::HiSysEvent::EventType::STATISTIC, "ACTION", "close", "ID", fd_, "ADDRESS", "empty",
                    "PID", IPCSkeleton::GetCallingPid(), "UID", IPCSkeleton::GetCallingUid());
                HILOGI("fd closed, fd_: %{pubilc}d", fd_);
                close(fd_);
                fd_ = -1;
            } else {
                HILOGE("socket not created");
                return;
            }
        }
    }

    bool RecvSocketSignal()
    {
        uint8_t recvStateBuf[1];
#ifdef DARWIN_PLATFORM
        int recvBufSize = recv(fd_, recvStateBuf, sizeof(recvStateBuf), 0);
#else
        int recvBufSize = recv(fd_, recvStateBuf, sizeof(recvStateBuf), MSG_WAITALL);
#endif
        if (recvBufSize <= 0) {
            HILOGE("service closed");
            return false;
        }
        bool state = recvStateBuf[0];

        uint8_t buf[6] = {0}; // addr buffer len
#ifdef DARWIN_PLATFORM
        int recvAddrSize = recv(fd_, buf, sizeof(buf), 0);
#else
        int recvAddrSize = recv(fd_, buf, sizeof(buf), MSG_WAITALL);
#endif
        if (recvAddrSize <= 0) {
            HILOGE("service closed");
            return false;
        }
        char token[LENGTH] = {0};
        (void)sprintf_s(token, sizeof(token), "%02X:%02X:%02X:%02X:%02X:%02X",
            buf[0x05], buf[0x04], buf[0x03], buf[0x02], buf[0x01], buf[0x00]);
        BluetoothRawAddress rawAddr {
            token
        };
        std::string address = rawAddr.GetAddress().c_str();
        return state;
    }

    int getSecurityFlags()
    {
        int flags = 0;
        if (auth_) {
            flags |= FLAG_AUTH;
            flags |= FLAG_ENCRYPT;
        }
        return flags;
    }

    InputStream &GetInputStream()
    {
        HILOGI("starts");
        if (inputStream_ == nullptr) {
            HILOGE("inputStream is NULL, failed. please Connect");
        }
        return *inputStream_;
    }

    OutputStream &GetOutputStream()
    {
        HILOGI("starts");
        if (outputStream_ == nullptr) {
            HILOGE("outputStream is NULL, failed. please Connect");
        }
        return *outputStream_;
    }

    BluetoothRemoteDevice &GetRemoteDevice()
    {
        HILOGI("starts");
        return remoteDevice_;
    }

    bool IsConnected()
    {
        HILOGI("starts");
        return socketStatus_ == SOCKET_CONNECTED;
    }

    int SetBufferSize(int bufferSize)
    {
        HILOGI("SetBufferSize bufferSize is %{public}d.", bufferSize);
        if (bufferSize < MIN_BUFFER_SIZE_TO_SET || bufferSize > MAX_BUFFER_SIZE_TO_SET) {
            HILOGE("SetBufferSize param is invalid.");
            return RET_BAD_PARAM;
        }

        if (fd_ <= 0) {
            HILOGE("SetBufferSize socket fd invalid.");
            return RET_BAD_STATUS;
        }

        const std::pair<const char*, int> sockOpts[] = {
            {"recvBuffer", SO_RCVBUF},
            {"sendBuffer", SO_SNDBUF},
        };
        for (auto opt : sockOpts) {
            int curSize = 0;
            socklen_t optlen = sizeof(curSize);
            if (getsockopt(fd_, SOL_SOCKET, opt.second, &curSize, &optlen) != 0) {
                HILOGE("SetBufferSize getsockopt %{public}s failed.", opt.first);
                return RET_BAD_STATUS;
            }
            HILOGI("SetBufferSize %{public}s before set size is %{public}d.", opt.first, curSize);

            if (curSize != bufferSize) {
                int setSize = bufferSize / 2;
                if (setsockopt(fd_, SOL_SOCKET, opt.second, &setSize, sizeof(setSize)) != 0) {
                    HILOGE("SetBufferSize setsockopt  %{public}s failed.", opt.first);
                    return RET_BAD_STATUS;
                }

                curSize = 0;
                if (getsockopt(fd_, SOL_SOCKET, opt.second, &curSize, &optlen) != 0) {
                    HILOGE("SetBufferSize after getsockopt %{public}s failed.", opt.first);
                    return RET_BAD_STATUS;
                }
                HILOGI("SetBufferSize %{public}s after set size is %{public}d.", opt.first, curSize);
            }
        }

        return RET_NO_ERROR;
    }

    class BluetoothClientSocketDeathRecipient;
    sptr<BluetoothClientSocketDeathRecipient> deathRecipient_;

    sptr<IBluetoothSocket> proxy_;
    std::unique_ptr<InputStream> inputStream_ {
        nullptr
    };
    std::unique_ptr<OutputStream> outputStream_ {
        nullptr
    };
    BluetoothRemoteDevice remoteDevice_;
    UUID uuid_;
    BtSocketType type_;
    std::string address_;
    int fd_;
    bool auth_;
    int socketStatus_;
};

class ClientSocket::impl::BluetoothClientSocketDeathRecipient final : public IRemoteObject::DeathRecipient {
public:
    explicit BluetoothClientSocketDeathRecipient(ClientSocket::impl &host) : host_(host) {};
    ~BluetoothClientSocketDeathRecipient() final = default;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothClientSocketDeathRecipient);

    void OnRemoteDied(const wptr<IRemoteObject> &remote) final
    {
        HILOGI("starts");
        host_.proxy_->AsObject()->RemoveDeathRecipient(host_.deathRecipient_);
        host_.proxy_ = nullptr;
    }

private:
    ClientSocket::impl &host_;
};

ClientSocket::impl::impl(const BluetoothRemoteDevice &addr, UUID uuid, BtSocketType type, bool auth)
    : inputStream_(nullptr),
      outputStream_(nullptr),
      remoteDevice_(addr),
      uuid_(uuid),
      type_(type),
      fd_(-1),
      auth_(auth),
      socketStatus_(SOCKET_INIT)
{
    HILOGI("starts");
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> hostRemote = samgr->GetSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);

    if (!hostRemote) {
        HILOGI("failed: no hostRemote");
        return;
    }

    sptr<IBluetoothHost> hostProxy = iface_cast<IBluetoothHost>(hostRemote);
    sptr<IRemoteObject> remote = hostProxy->GetProfile(PROFILE_SOCKET);

    if (!remote) {
        HILOGI("failed: no remote");
        return;
    }
    HILOGI("remote obtained");

    proxy_ = iface_cast<IBluetoothSocket>(remote);

    deathRecipient_ = new BluetoothClientSocketDeathRecipient(*this);

    if (!proxy_) {
        HILOGE("proxy_ is nullptr");
        return;
    }

    proxy_->AsObject()->AddDeathRecipient(deathRecipient_);
}

ClientSocket::impl::impl(int fd, std::string address, BtSocketType type)
    : inputStream_(std::make_unique<InputStream>(fd)),
      outputStream_(std::make_unique<OutputStream>(fd)),
      remoteDevice_(BluetoothRemoteDevice(address, 0)),
      type_(type),
      address_(address),
      fd_(fd),
      auth_(false),
      socketStatus_(SOCKET_CONNECTED)
{
    HILOGI("starts");
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> hostRemote = samgr->GetSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);

    if (!hostRemote) {
        HILOGI("failed: no hostRemote");
        return;
    }

    sptr<IBluetoothHost> hostProxy = iface_cast<IBluetoothHost>(hostRemote);
    sptr<IRemoteObject> remote = hostProxy->GetProfile(PROFILE_SOCKET);

    if (!remote) {
        HILOGI("failed: no remote");
        return;
    }
    HILOGI("remote obtained");

    proxy_ = iface_cast<IBluetoothSocket>(remote);

    deathRecipient_ = new BluetoothClientSocketDeathRecipient(*this);

    if (!proxy_) {
        HILOGE("proxy_ is nullptr");
        return;
    }

    proxy_->AsObject()->AddDeathRecipient(deathRecipient_);
}

ClientSocket::ClientSocket(const BluetoothRemoteDevice &bda, UUID uuid, BtSocketType type, bool auth)
    : pimpl(new ClientSocket::impl(bda, uuid, type, auth))
{}

ClientSocket::ClientSocket(int fd, std::string address, BtSocketType type)
    : pimpl(new ClientSocket::impl(fd, address, type))
{}

ClientSocket::~ClientSocket()
{}

int ClientSocket::Connect(int psm)
{
    HILOGI("starts");
    if (!IS_BT_ENABLED()) {
        HILOGI("BR is not TURN_ON");
        return BT_ERR_INVALID_STATE;
    }

    pimpl->address_ = pimpl->remoteDevice_.GetDeviceAddr();

    std::string tempAddress = pimpl->address_;
    if (!tempAddress.size()) {
        return BtStatus::BT_FAILURE;
    }
    if (pimpl->socketStatus_ == SOCKET_CLOSED) {
        HILOGE("socket closed");
        return BT_ERR_INVALID_STATE;
    }

    if (!pimpl->proxy_) {
        HILOGE("proxy_ is nullptr");
        return BT_ERR_SERVICE_DISCONNECTED;
    }

    ConnectSocketParam param {
        .addr = tempAddress,
        .uuid = bluetooth::Uuid::ConvertFrom128Bits(pimpl->uuid_.ConvertTo128Bits()),
        .securityFlag = (int32_t)pimpl->getSecurityFlags(),
        .type = (int32_t)pimpl->type_,
        .psm = psm
    };
    int ret = pimpl->proxy_->Connect(param, pimpl->fd_);
    if (ret != BT_NO_ERROR) {
        HILOGE("Connect error %{public}d.", ret);
        return ret;
    }

    HILOGI("fd_: %{public}d", pimpl->fd_);
    if (pimpl->fd_ == -1) {
        HILOGE("connect failed!");
        return BtStatus::BT_FAILURE;
    }

    bool recvret = pimpl->RecvSocketSignal();
    HILOGI("recvret: %{public}d", recvret);
    pimpl->inputStream_ = std::make_unique<InputStream>(pimpl->fd_);
    pimpl->outputStream_ = std::make_unique<OutputStream>(pimpl->fd_);

    if (!recvret) {
        HILOGE("connect failed!");
        return BtStatus::BT_FAILURE;
    }
    pimpl->socketStatus_ = SOCKET_CONNECTED;
    HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::BLUETOOTH, "SPP_CONNECT_STATE",
        HiviewDFX::HiSysEvent::EventType::STATISTIC, "ACTION", "connect", "ID", pimpl->fd_, "ADDRESS",
        GetEncryptAddr(tempAddress), "PID", IPCSkeleton::GetCallingPid(), "UID", IPCSkeleton::GetCallingUid());
    return BtStatus::BT_SUCCESS;
}

void ClientSocket::Close()
{
    HILOGI("starts");
    return pimpl->Close();
}

InputStream &ClientSocket::GetInputStream()
{
    HILOGI("starts");
    return pimpl->GetInputStream();
}

OutputStream &ClientSocket::GetOutputStream()
{
    HILOGI("starts");
    return pimpl->GetOutputStream();
}

BluetoothRemoteDevice &ClientSocket::GetRemoteDevice()
{
    HILOGI("starts");
    return pimpl->GetRemoteDevice();
}

bool ClientSocket::IsConnected() const
{
    HILOGI("starts");
    return pimpl->IsConnected();
}

int ClientSocket::SetBufferSize(int bufferSize)
{
    HILOGI("starts");
    return pimpl->SetBufferSize(bufferSize);
}

struct ServerSocket::impl {
    impl(const std::string &name, UUID uuid, BtSocketType type, bool encrypt);
    ~impl()
    {
        HILOGI("enter");
        if (proxy_ != nullptr && proxy_->AsObject() != nullptr) {
            proxy_->AsObject()->RemoveDeathRecipient(deathRecipient_);
        } else {
            HILOGE("proxy_ is nullptr");
        }

        if (fd_ > 0) {
            shutdown(fd_, SHUT_RD);
            shutdown(fd_, SHUT_WR);
            close(fd_);
            HILOGI("fd closed, fd_: %{public}d", fd_);
            fd_ = -1;
        }
    }

    int Listen()
    {
        HILOGI("starts");
        if (!IS_BT_ENABLED()) {
            HILOGE("BR is not TURN_ON");
            return BT_ERR_INVALID_STATE;
        }
        if (!proxy_) {
            HILOGE("failed, proxy_ is nullptr");
            socketStatus_ = SOCKET_CLOSED;
            return BT_ERR_SERVICE_DISCONNECTED;
        }
        if (socketStatus_ == SOCKET_CLOSED) {
            HILOGE("failed, socketStatus_ is SOCKET_CLOSED");
            return BT_ERR_INVALID_STATE;
        }

        ListenSocketParam param {
            name_,
            bluetooth::Uuid::ConvertFrom128Bits(uuid_.ConvertTo128Bits()),
            (int32_t)getSecurityFlags(),
            (int32_t)type_
        };
        int ret = proxy_->Listen(param, fd_);
        if (ret != BT_NO_ERROR) {
            HILOGE("Listen error %{public}d.", ret);
            return ret;
        }
        if (fd_ == BT_INVALID_SOCKET_FD) {
            HILOGE("listen socket failed, fd_ is -1");
            socketStatus_ = SOCKET_CLOSED;
            return BT_ERR_INVALID_STATE;
        }

        if (type_ == TYPE_L2CAP_LE) {
            psm_ = RecvSocketPsm();
            if (psm_ <= 0) {
                return BT_ERR_INVALID_STATE;
            }
        }

        if (socketStatus_ == SOCKET_INIT) {
            socketStatus_ = SOCKET_LISTENING;
        } else {
            HILOGE("failed, socketStatus_: %{public}d is not SOCKET_INIT", socketStatus_);
            close(fd_);
            socketStatus_ = SOCKET_CLOSED;
            return BT_ERR_INVALID_STATE;
        }

        return BT_NO_ERROR;
    }

    int getSecurityFlags()
    {
        int flags = 0;
        if (encrypt_) {
            flags |= FLAG_AUTH;
            flags |= FLAG_ENCRYPT;
        }
        return flags;
    }

    std::shared_ptr<ClientSocket> Accept(int timeout)
    {
        HILOGI("starts");
        if (socketStatus_ != SOCKET_LISTENING) {
            HILOGE("socket is not in listen state");
            return nullptr;
        }
        struct timeval time = {timeout, 0};
        setsockopt(fd_, SOL_SOCKET, SO_SNDTIMEO, (const char *)&time, sizeof(time));
        setsockopt(fd_, SOL_SOCKET, SO_RCVTIMEO, (const char *)&time, sizeof(time));

        acceptFd_ = RecvSocketFd();
        HILOGE("RecvSocketFd acceptFd: %{public}d", acceptFd_);
        if (acceptFd_ <= 0) {
            return nullptr;
        }
        if (timeout > 0) {
            time = {0, 0};
            setsockopt(fd_, SOL_SOCKET, SO_SNDTIMEO, (const char *)&time, sizeof(time));
            setsockopt(fd_, SOL_SOCKET, SO_RCVTIMEO, (const char *)&time, sizeof(time));
        }

        std::shared_ptr<ClientSocket> clientSocket = std::make_shared<ClientSocket>(acceptFd_, acceptAddress_, type_);

        HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::BLUETOOTH, "SPP_CONNECT_STATE",
            HiviewDFX::HiSysEvent::EventType::STATISTIC, "ACTION", "connect", "ID", acceptFd_, "ADDRESS",
            GetEncryptAddr(acceptAddress_), "PID", IPCSkeleton::GetCallingPid(), "UID", IPCSkeleton::GetCallingUid());

        return clientSocket;
    }

    int RecvSocketFd()
    {
        HILOGI("starts");
        int rv = 0;
        int cfd = -1;
        int clientFd = -1;
        char ccmsg[CMSG_SPACE(sizeof(cfd))];
        char buffer[10];
        struct iovec io = {.iov_base = buffer, .iov_len = sizeof(buffer)};
        struct msghdr msg;
        (void)memset_s(&msg, sizeof(msg), 0, sizeof(msg));
        msg.msg_control = ccmsg;
        msg.msg_controllen = sizeof(ccmsg);
        msg.msg_iov = &io;
        msg.msg_iovlen = 1;

#ifdef DARWIN_PLATFORM
        rv = recvmsg(fd_, &msg, 0);
#else
        rv = recvmsg(fd_, &msg, MSG_NOSIGNAL);
#endif
        if (rv == -1) {
            HILOGE("[sock] recvmsg error  %{public}d, fd: %{public}d", errno, fd_);
            return BtStatus::BT_FAILURE;
        }
        struct cmsghdr *cmptr = CMSG_FIRSTHDR(&msg);
        if ((cmptr != nullptr) && (cmptr->cmsg_len == CMSG_LEN(sizeof(int)))) {
            if (cmptr->cmsg_level != SOL_SOCKET || cmptr->cmsg_type != SCM_RIGHTS) {
                HILOGE("[sock] control level: %{public}d", cmptr->cmsg_level);
                HILOGE("[sock] control type: %{public}d", cmptr->cmsg_type);
                return BtStatus::BT_FAILURE;
            }
            clientFd = *(reinterpret_cast<int *>(CMSG_DATA(cmptr)));
        } else {
            return BtStatus::BT_FAILURE;
        }
        uint8_t recvBuf[rv];
        (void)memset_s(&recvBuf, sizeof(recvBuf), 0, sizeof(recvBuf));
        if (memcpy_s(recvBuf, sizeof(recvBuf), (uint8_t *)msg.msg_iov[0].iov_base, rv) != EOK) {
            HILOGE("[sock] RecvSocketFd, recvBuf memcpy_s fail");
            return BtStatus::BT_FAILURE;
        }

        uint8_t buf[6] = {0};
        if (memcpy_s(buf, sizeof(buf), &recvBuf[1], sizeof(buf)) != EOK) {
            HILOGE("[sock] RecvSocketFd, buf memcpy_s fail");
            return BtStatus::BT_FAILURE;
        }

        char token[LENGTH] = {0};
        (void)sprintf_s(token, sizeof(token), "%02X:%02X:%02X:%02X:%02X:%02X",
            buf[0x05], buf[0x04], buf[0x03], buf[0x02], buf[0x01], buf[0x00]);
        BluetoothRawAddress rawAddr {token};
        acceptAddress_ = rawAddr.GetAddress().c_str();
        return clientFd;
    }

    int RecvSocketPsm()
    {
        uint8_t recvBuf[PSM_BUFFER_SIZE];
        int recvBufSize = recv(fd_, recvBuf, sizeof(recvBuf), MSG_WAITALL);
        if (recvBufSize <= 0) {
            HILOGE("[sock] recv error  %{public}d, fd: %{public}d", errno, fd_);
            return BtStatus::BT_FAILURE;
        }
        int psm = recvBuf[0];
        HILOGI("[sock] RecvSocketPsm psm: %{public}d", psm);
        return psm;
    }

    void Close()
    {
        HILOGI("starts");
        if (socketStatus_ == SOCKET_CLOSED) {
            HILOGW("The socketStatus_ is already SOCKET_CLOSED");
            return;
        } else {
            socketStatus_ = SOCKET_CLOSED;
            if (fd_ > 0) {
                shutdown(fd_, SHUT_RD);
                shutdown(fd_, SHUT_WR);
                close(fd_);
                HILOGI("fd closed, fd_: %{public}d", fd_);
                fd_ = -1;
                return;
            } else {
                HILOGE("socket not created");
                return;
            }
        }
    }

    const std::string &GetStringTag()
    {
        HILOGI("starts");
        if (socketStatus_ == SOCKET_CLOSED) {
            HILOGE("socketStatus_ is SOCKET_CLOSED");
            socketServiceType_ = "";
        } else {
            socketServiceType_ = "ServerSocket:";
            socketServiceType_.append(" Type: ").append(ConvertTypeToString(type_))
                .append(" ServerName: ").append(name_);
        }
        return socketServiceType_;
    }

    static std::string ConvertTypeToString(BtSocketType type)
    {
        std::string retStr;
        if (type == TYPE_RFCOMM) {
            retStr = "TYPE_RFCOMM";
        } else if (type == TYPE_L2CAP) {
            retStr = "TYPE_L2CAP";
        } else if (type == TYPE_L2CAP_LE) {
            retStr = "TYPE_L2CAP_LE";
        } else {
            retStr = "TYPE_UNKNOW";
        }
        return retStr;
    }

    class BluetoothServerSocketDeathRecipient;
    sptr<BluetoothServerSocketDeathRecipient> deathRecipient_;

    sptr<IBluetoothSocket> proxy_;
    UUID uuid_;
    BtSocketType type_;
    bool encrypt_;
    int fd_;
    int socketStatus_;
    std::string name_ {
        ""
        };
    int acceptFd_;
    std::string acceptAddress_;
    std::string socketServiceType_ {
        ""
    };
    int psm_;
};

class ServerSocket::impl::BluetoothServerSocketDeathRecipient final : public IRemoteObject::DeathRecipient {
public:
    explicit BluetoothServerSocketDeathRecipient(ServerSocket::impl &host) : host_(host) {};
    ~BluetoothServerSocketDeathRecipient() final = default;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothServerSocketDeathRecipient);

    void OnRemoteDied(const wptr<IRemoteObject> &remote) final
    {
        HILOGI("starts");
        if (!host_.proxy_) {
            return;
        }
        host_.proxy_->AsObject()->RemoveDeathRecipient(host_.deathRecipient_);
        host_.proxy_ = nullptr;
    }

private:
    ServerSocket::impl &host_;
};

ServerSocket::impl::impl(const std::string &name, UUID uuid, BtSocketType type, bool encrypt)
    : uuid_(uuid), type_(type), encrypt_(encrypt), fd_(-1), socketStatus_(SOCKET_INIT), name_(name), psm_(-1)
{
    HILOGI("(4 parameters) starts");
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> hostRemote = samgr->GetSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);

    if (!hostRemote) {
        HILOGI("failed: no hostRemote");
        return;
    }
    sptr<IBluetoothHost> hostProxy = iface_cast<IBluetoothHost>(hostRemote);
    sptr<IRemoteObject> remote = hostProxy->GetProfile(PROFILE_SOCKET);

    if (!remote) {
        HILOGE("failed: no remote");
        return;
    }
    HILOGI("remote obtained");

    proxy_ = iface_cast<IBluetoothSocket>(remote);
    if (proxy_ == nullptr) {
        return;
    }
    deathRecipient_ = new BluetoothServerSocketDeathRecipient(*this);
    proxy_->AsObject()->AddDeathRecipient(deathRecipient_);
}

ServerSocket::ServerSocket(const std::string &name, UUID uuid, BtSocketType type, bool encrypt)
    : pimpl(new ServerSocket::impl(name, uuid, type, encrypt))
{
    HILOGI("(4 parameters) starts");
}

ServerSocket::~ServerSocket()
{}

int ServerSocket::Listen()
{
    HILOGI("starts");
    return pimpl->Listen();
}

std::shared_ptr<ClientSocket> ServerSocket::Accept(int timeout)
{
    HILOGI("starts");
    return pimpl->Accept(timeout);
}

void ServerSocket::Close()
{
    HILOGI("starts");
    return pimpl->Close();
}

const std::string &ServerSocket::GetStringTag()
{
    HILOGI("starts");
    return pimpl->GetStringTag();
}

int ServerSocket::GetPsm()
{
    return pimpl->psm_;
}

std::shared_ptr<ClientSocket> SocketFactory::BuildInsecureRfcommDataSocketByServiceRecord(
    const BluetoothRemoteDevice &device, const UUID &uuid)
{
    HILOGI("starts");
    if (device.IsValidBluetoothRemoteDevice()) {
        return std::make_shared<ClientSocket>(device, uuid, TYPE_RFCOMM, false);
    } else {
        HILOGE("[sock] Device is not valid.");
        return nullptr;
    }
}

std::shared_ptr<ClientSocket> SocketFactory::BuildRfcommDataSocketByServiceRecord(
    const BluetoothRemoteDevice &device, const UUID &uuid)
{
    HILOGI("starts");
    if (device.IsValidBluetoothRemoteDevice()) {
        return std::make_shared<ClientSocket>(device, uuid, TYPE_RFCOMM, true);
    } else {
        HILOGE("[sock] Device is not valid.");
        return nullptr;
    }
}

std::shared_ptr<ServerSocket> SocketFactory::DataListenInsecureRfcommByServiceRecord(
    const std::string &name, const UUID &uuid)
{
    HILOGI("starts");
    return std::make_shared<ServerSocket>(name, uuid, TYPE_RFCOMM, false);
}

std::shared_ptr<ServerSocket> SocketFactory::DataListenRfcommByServiceRecord(const std::string &name, const UUID &uuid)
{
    HILOGI("starts");
    return std::make_shared<ServerSocket>(name, uuid, TYPE_RFCOMM, true);
}
}  // namespace Bluetooth
}  // namespace OHOS