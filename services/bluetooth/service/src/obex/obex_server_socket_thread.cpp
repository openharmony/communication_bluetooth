/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_obex_server_socket"
#endif

#include <chrono>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

#include "bt_sock.h"
#include "log.h"
#include "obex_socket_device.h"
#include "securec.h"
#include "service_util.h"
#include "obex_server_socket_thread.h"

namespace OHOS {
namespace bluetooth {
const uint8_t RECV_SOCK_DEVICE_MAX_ERR_TIMES = 10;
constexpr uint8_t MAC_ADDR_LEN = 6;
/* cmd executes in socket poll thread */
constexpr int  CMD_WAKEUP = 1;
constexpr int  CMD_EXIT = 2;
constexpr int  CMD_ADD_FD = 3;
constexpr int  CMD_REMOVE_FD = 4;
constexpr short POLL_EXCEPTION_EVENTS = POLLHUP | POLLRDHUP | POLLERR | POLLNVAL;
constexpr int FD_INDEX = 0;
constexpr int SOCKET_ERROR_DELAY_MS = 20;
#ifndef OSI_NO_EINTR
#define OSI_NO_EINTR(fn) \
do {                  \
} while ((fn) == -1 && errno == EINTR)
#endif

ObexServerSocketThread::ObexServerSocketThread() noexcept
{
}

ObexServerSocketThread::~ObexServerSocketThread()
{
    HILOGI("~ObexServerSocketThread");
}

void ObexServerSocketThread::InitThread()
{
    HILOGI("intro");

    std::lock_guard<BtRecursiveMutex> lock(lock_);

    if (usedIndex_++ != 0) {
        HILOGI("thread already initialized by %{public}d times", usedIndex_);
        return;
    }

    cmdFdr_ = -1;
    cmdFdw_ = -1;
    pollCount_ = 0;

    for (int i = 0; i < MAX_POLL; i++) {
        pollSlotArray_[i].pfd.fd = -1;
        pollSlotIndex_[i] = -1;
    }

    InitCmdFd();

    thread_ = std::make_unique<std::thread>([this] {this->SockPollThread();});
    thread_->detach();
}

void ObexServerSocketThread::DeInitThread()
{
    HILOGI("intro");

    std::lock_guard<BtRecursiveMutex> lock(lock_);

    if (usedIndex_ <= 0) {
        HILOGI("thread is not inited: %{public}d", usedIndex_);
        return;
    }
    if (--usedIndex_ != 0) {
        HILOGI("thread still be uesd as: %{public}d times", usedIndex_);
        return;
    }

    if (cmdFdw_ == -1) {
        HILOGE("cmd socket is not created");
        return;
    }
    SockCmd cmd = {CMD_EXIT, 0, 0, 0};

    ssize_t ret = -1;
    OSI_NO_EINTR(ret = send(cmdFdw_, &cmd, sizeof(cmd), 0));
}

void ObexServerSocketThread::InitCmdFd()
{
    HILOGI("intro");
    if (cmdFdr_ != -1 || cmdFdw_ != -1) {
        HILOGE("cmdFd already initialize, break");
        return;
    }
    int cmdFds[2] = {0, 0};
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, cmdFds) < 0) {
        HILOGE("socketpair failed: %{public}s", strerror(errno));
        return;
    }

    cmdFdr_ = cmdFds[0];
    cmdFdw_ = cmdFds[1];

    HILOGI("init pass, cmdFdr_:%{public}d, cmdFdw_:%{public}d", cmdFdr_, cmdFdw_);

    AddPoll(cmdFdr_, FD_INDEX, OBEX_SOCK_THREAD_FD_RD);
}

void ObexServerSocketThread::AddPoll(int fd, int type, int flags)
{
    HILOGI("intro");
    if (fd == -1) {
        return;
    }

    int emptySlot = -1;

    for (int i = 0; i < MAX_POLL; i++) {
        if (pollSlotArray_[i].pfd.fd == fd) {
            if (pollCount_ >= MAX_POLL) {
                HILOGE("no enough poll, break");
                break;
            }

            SetPoll(&pollSlotArray_[i], fd, type,
                static_cast<unsigned int>(flags) | static_cast<unsigned int>(pollSlotArray_[i].flags));
            return;
        } else if (emptySlot < 0 && pollSlotArray_[i].pfd.fd == -1) {
            emptySlot = i;
        }
    }
    if (emptySlot >= 0) {
        HILOGI("find emptySlot:%{public}d for fd:%{public}d", emptySlot, fd);
        SetPoll(&pollSlotArray_[emptySlot], fd, type, flags);
        ++pollCount_;
        return;
    }
    HILOGE("exceeded max poll slot:%{public}d!", MAX_POLL);
}

unsigned int ObexServerSocketThread::Flags2Pevents(unsigned int flags)
{
    unsigned int pevents = 0;
    if (flags & static_cast<unsigned int>(OBEX_SOCK_THREAD_FD_WR)) {
        pevents |= POLLOUT;
    }
    if (flags & static_cast<unsigned int>(OBEX_SOCK_THREAD_FD_RD)) {
        pevents |= POLLIN;
    }
    pevents |= static_cast<unsigned int>(POLL_EXCEPTION_EVENTS);
    return pevents;
}

void ObexServerSocketThread::SetPoll(PollSlot* pPollSlot, int fd, int type, int flags)
{
    HILOGI("setpoll, fd:%{public}d, type:%{public}d, flags:%{public}d", fd, type, flags);

    if (pPollSlot == nullptr) {
        HILOGE("SetPoll fail: pPollSlot is null");
        return;
    }

    pPollSlot->pfd.fd = fd;
    if (pPollSlot->type != 0 && pPollSlot->type != type) {
        HILOGE("poll socket type should not changed! type was:%{public}d, type now:%{public}d",
            pPollSlot->type, type);
    }

    pPollSlot->type = type;
    pPollSlot->flags = flags;
    pPollSlot->pfd.events = Flags2Pevents(flags);
    pPollSlot->pfd.revents = 0;
}

RawAddress ConvertRfcommAddr(OHOS::bluetooth::RawAddress &addr)
{
    uint8_t addrBytes[OHOS::bluetooth::RawAddress::BT_ADDRESS_BYTE_LEN];
    addr.ConvertToUint8(addrBytes);
    uint8_t reversedBytes[OHOS::bluetooth::RawAddress::BT_ADDRESS_BYTE_LEN];
    for (int i = 0; i < MAC_ADDR_LEN; i++) {
        reversedBytes[i] = addrBytes[MAC_ADDR_LEN - 1 - i];
    }
    OHOS::bluetooth::RawAddress newAddr =
        OHOS::bluetooth::RawAddress::ConvertToString(reversedBytes);

    return newAddr;
}

std::shared_ptr<ObexSocketDevice> ObexServerSocketThread::RecvSocketDevice(int fd, SocketType socketType)
{
    char buffer[sizeof(SockConnectSignal)] = {0};
    char msgCtrlBuff[CMSG_SPACE(1)] = {0};
    struct iovec iv = {.iov_base = buffer, .iov_len = sizeof(buffer)};
    struct msghdr msg;
    msg.msg_control = msgCtrlBuff;
    msg.msg_controllen = sizeof(msgCtrlBuff);
    msg.msg_iov = &iv;
    msg.msg_iovlen = 1;

    int ret = recvmsg(fd, &msg, MSG_NOSIGNAL);
    if (ret <= 0) {
        HILOGE("[sock] recvmsg error  %{public}s", strerror(errno));
        return nullptr;
    }

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    if (cmsg == nullptr) {
        HILOGE("[sock] recvmsg error cmsg nullptr");
        return nullptr;
    }
    int acceptFd = *(reinterpret_cast<int *>(CMSG_DATA(cmsg)));

    std::shared_ptr<ObexSocketDevice> socketDevice = nullptr;
    SockConnectSignal *info = static_cast<SockConnectSignal *>(msg.msg_iov->iov_base);
    if (info == nullptr) {
        HILOGE("[sock] recvmsg error cs nullptr");
        return nullptr;
    }
    RawAddress newAddr = ConvertRfcommAddr(info->bdAddr);

    if (socketType == SocketType::TYPE_L2CAP) {
        HILOGD("[sock] socketfd %{public}d. maxRxPacketSize %{public}d.", acceptFd, info->maxRxPacketSize);
        socketDevice = std::make_shared<ObexSocketDevice>(
            acceptFd, info->maxTxPacketSize, info->maxRxPacketSize, SocketType::TYPE_L2CAP, newAddr.GetAddress());
    } else if (socketType == SocketType::TYPE_RFCOMM) {
        socketDevice = std::make_shared<ObexSocketDevice>(
            acceptFd, MAX_PACKET_SIZE, MAX_PACKET_SIZE, SocketType::TYPE_RFCOMM, newAddr.GetAddress());
    } else {
        HILOGE("[sock] socketType_ error");
    }
    return socketDevice;
}

bool ObexServerSocketThread::HandleMsg(int fd, int type)
{
    HILOGD("intro");
    int errorTimes = fdErrCountMap_.ReadVal(fd);
    auto socketDevice = RecvSocketDevice(fd, static_cast<SocketType>(type));
    if (socketDevice == nullptr) {
            HILOGE("RecvSocketFd fail, fd =%{public}d", fd);
            errorTimes++;
            // delay when receive socket device error
            std::this_thread::sleep_for(std::chrono::milliseconds(SOCKET_ERROR_DELAY_MS));
            if (errorTimes >= RECV_SOCK_DEVICE_MAX_ERR_TIMES) {
                HILOGE("recv socket device error times more than 10 times, restart");
                fdErrCountMap_.EnsureInsert(fd, 0);
                onConnectMap_.ReadVal(fd)->OnConnect(socketDevice);
                return false;
            }
            fdErrCountMap_.EnsureInsert(fd, errorTimes);
            return true;
    }
    fdErrCountMap_.EnsureInsert(fd, 0);
    HILOGD("acceptFd: %{public}d, acceptAddress: %{public}s, maxRxPacketSize %{public}d, "
            "maxTxPacketSize %{public}d",
        socketDevice->mSocketFd,
        GET_ENCRYPT_STR_ADDR(socketDevice->mDeviceAddress),
        socketDevice->mMaxRxPacketSize,
        socketDevice->mMaxTxPacketSize);

    onConnectMap_.ReadVal(fd)->OnConnect(socketDevice);

    return true;
}

bool ObexServerSocketThread::ThreadAddFd(
    int fd, int type, unsigned int flags, std::shared_ptr<IObexSocketObserver> callBack)
{
    HILOGI("adding fd:%{public}d, flags:0x%{public}x", fd, flags);

    if (cmdFdw_ == -1) {
        HILOGE("cmd socket is not created. socket thread may not initialized");
        return false;
    }

    if (callBack == nullptr) {
        HILOGE("callBack cannot be null");
        return false;
    }

    fdErrCountMap_.EnsureInsert(fd, 0);
    onConnectMap_.EnsureInsert(fd, callBack);

    if (flags & static_cast<unsigned int>(OBEX_SOCK_THREAD_ADD_FD_SYNC)) {
        // cleanup one-time flags
        flags &= ~(static_cast<unsigned int>(OBEX_SOCK_THREAD_ADD_FD_SYNC));
        AddPoll(fd, type, flags);
        HILOGW("THREAD_ADD_FD_SYNC is not called in poll thread, fallback to async");
        return true;
    }

    SockCmd cmd = {CMD_ADD_FD, fd, type, flags};

    ssize_t ret = -1;
    OSI_NO_EINTR(ret = send(cmdFdw_, &cmd, sizeof(cmd), 0));

    return ret == static_cast<int>(sizeof(cmd));
}

bool ObexServerSocketThread::ThreadRemoveFdAndClose(int fd)
{
    HILOGI("remove fd:%{public}d", fd);

    if (fd == -1) {
        HILOGE("invalid file descriptor.");
        return false;
    }

    fdErrCountMap_.Erase(fd);
    onConnectMap_.Erase(fd);

    SockCmd cmd = {CMD_REMOVE_FD, fd, 0, 0};

    ssize_t ret = -1;
    OSI_NO_EINTR(ret = send(cmdFdw_, &cmd, sizeof(cmd), 0));
    return ret == static_cast<int>(sizeof(cmd));
}

void ObexServerSocketThread::RemovePoll(PollSlot* pPollSlot)
{
    if (pPollSlot == nullptr) {
        HILOGE("RemovePoll fail, pPollSlot is null");
        return;
    }

    //  clear the slot
    --pollCount_;
    if (memset_s(pPollSlot, sizeof(*pPollSlot), 0, sizeof(*pPollSlot)) != 0) {
        HILOGE("memset_s fail");
        return;
    }

    pPollSlot->pfd.fd = -1;
}

bool ObexServerSocketThread::ProcessCmdSock()
{
    HILOGI("intro");
    SockCmd cmd = {-1, 0, 0, 0};

    ssize_t ret = -1;
    OSI_NO_EINTR(ret = recv(cmdFdr_, &cmd, sizeof(cmd), MSG_WAITALL));

    if (ret != static_cast<int>(sizeof(cmd))) {
        HILOGE("recv cmd errno:%{public}d", errno);
        return false;
    }
    HILOGI("cmd id:%{public}d", cmd.id);
    switch (cmd.id) {
        case CMD_ADD_FD:
            AddPoll(cmd.fd, cmd.type, cmd.flags);
            break;
        case CMD_REMOVE_FD:
            for (int i = 1; i < MAX_POLL; ++i) {
                PollSlot* pPollSlot = &pollSlotArray_[i];
                if (pPollSlot->pfd.fd == cmd.fd) {
                    RemovePoll(pPollSlot);
                    break;
                }
            }
            close(cmd.fd);
            break;
        case CMD_WAKEUP:
            break;
        case CMD_EXIT:
            return false;
        default:
            HILOGW("unknown cmd: %{public}d", cmd.id);
            break;
    }
    return true;
}

void ObexServerSocketThread::ProcessDataSock(pollfd* pfds, int count)
{
    HILOGD("intro");
    if (count > pollCount_) {
        HILOGW("handling count:%{public}d is more than pollCount", count);
    }

    for (int i = 1; i < pollCount_; i++) {
        if (pfds[i].revents) {
            int psI = pollSlotIndex_[i];
            if (pollSlotArray_[psI].pfd.fd == -1) {
                HILOGE("fd = -1, continue");
                continue;
            }
            int type = pollSlotArray_[psI].type;
            unsigned int flags = 0;
            if (pfds[i].revents & POLLIN) {
                flags |= static_cast<unsigned int>(OBEX_SOCK_THREAD_FD_RD);
            }

            if (static_cast<unsigned int>(pfds[i].revents) & POLLOUT) {
                flags |= OBEX_SOCK_THREAD_FD_WR;
            }

            if (pfds[i].revents & POLL_EXCEPTION_EVENTS) {
                flags |= OBEX_SOCK_THREAD_FD_EXCEPTION;
                HILOGI("fd exception: %{public}d, RemovePoll", pfds[i].fd);
                RemovePoll(&pollSlotArray_[psI]);
            }

            if (flags && !HandleMsg(pfds[i].fd, type)) {
                // 出现异常，去掉fd监听
                ThreadRemoveFdAndClose(pfds[i].fd);
            }
        }
    }
}

void ObexServerSocketThread::PreparePollFds(pollfd* pfds, size_t size)
{
    int count = 0;
    int psI = 0;
    int pfdI = 0;
    if (memset_s(pfds, sizeof(pfds[0]) * pollCount_, 0, sizeof(pfds[0]) * pollCount_) != 0) {
        HILOGE("memset_s fail");
        return;
    }
    while (count < pollCount_) {
        if (psI >= static_cast<int>(size)) {
            HILOGE("exceed max poll range, ps_i:%{public}d, MAX_POLL:%{public}d, count:%{public}d, "
                "pollCount_:%{public}d", psI, size, count, pollCount_);
            return;
        }
        if (pollSlotArray_[psI].pfd.fd >= 0) {
            pfds[pfdI] = pollSlotArray_[psI].pfd;
            pollSlotIndex_[pfdI] = psI;
            count++;
            pfdI++;
        }
        psI++;
    }
}

void ObexServerSocketThread::SockPollThread()
{
    HILOGI("intro");
    prctl(PR_SET_NAME, "bt_obex_server_socket");

    struct pollfd pfds[MAX_POLL];
    if (memset_s(pfds, sizeof(pfds), 0, sizeof(pfds)) != 0) {
        HILOGE("memset_s fail");
        return;
    }
    for (;;) {
        PreparePollFds(pfds, MAX_POLL);
        int ret = -1;
        OSI_NO_EINTR(ret = poll(pfds, pollCount_, -1));
        if (ret == -1) {
            HILOGE("poll ret -1, exit the thread, errno:%{public}d, err:%{public}s", errno,
                strerror(errno));
            break;
        }

        // ret then means how many fds need to be handled
        if (ret == 0) {
            HILOGD("no data need to be handled, contionue");
            continue;
        }

        int needProcessFd = true;
        if (pfds[FD_INDEX].revents) { // cmd fd always is the first one
            if (!ProcessCmdSock()) {
                HILOGE("process_cmd_sock return false, exit...");
                break;
            }
            if (ret == 1) {
                needProcessFd = false;
            } else {
                ret--;  // exclude the cmd fd
            }
        }
        if (needProcessFd) {
            ProcessDataSock(pfds, ret);
        }
    }
    HILOGI("socket poll thread exiting");
    CloseCmdFd();
    return;
}

void ObexServerSocketThread::CloseCmdFd()
{
    HILOGI("intro cmdFdr_ is %{public}d cmdFdw_ is %{public}d", cmdFdr_, cmdFdw_);
    if (cmdFdr_ != -1) {
        close(cmdFdr_);
        cmdFdr_ = -1;
    }

    if (cmdFdw_ != -1) {
        close(cmdFdw_);
        cmdFdw_ = -1;
    }
}
}  // namespace bluetooth
}  // namespace OHOS