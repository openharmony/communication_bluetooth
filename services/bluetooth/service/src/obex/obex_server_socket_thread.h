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

#ifndef OBEX_SERVER_SOCKET_THREAD_H
#define OBEX_SERVER_SOCKET_THREAD_H

#include <poll.h>
#include <pthread.h>
#include <thread>
#include <future>

#include "obex_socket_observer.h"
#include "safe_map.h"
#include "bt_recursive_mutex.h"

namespace OHOS {
namespace bluetooth {

constexpr int OBEX_SOCK_THREAD_FD_RD = 1;               /* read signal */
constexpr int OBEX_SOCK_THREAD_FD_WR =  1 << 1;         /* write signal */
constexpr int OBEX_SOCK_THREAD_FD_EXCEPTION  = 1 << 2;  /* exception singal */
constexpr int OBEX_SOCK_THREAD_ADD_FD_SYNC = 1 << 3;    /* Add fd in immediately */
constexpr int MAX_POLL = 10;

typedef void (*CmdCb)(int cmdFd, int type, int flags);

struct PollSlot {
    struct pollfd pfd;
    int type;
    int flags;
};


struct SockCmd {
    int id;
    int fd;
    int type;
    int flags;
} ;

class ObexServerSocketThread {
public:
    ObexServerSocketThread() noexcept;
    ~ObexServerSocketThread();
    void InitThread();
    void DeInitThread();
    bool ThreadAddFd(int fd, int type, unsigned int flags, std::shared_ptr<IObexSocketObserver> callBack);
    bool ThreadRemoveFdAndClose(int fd);

private:
    BtRecursiveMutex lock_;
    int usedIndex_ = 0;
    std::unique_ptr<std::thread> thread_{nullptr};
    int cmdFdr_ = -1;
    int cmdFdw_ = -1;
    int pollCount_ = 0;
    // PollSlot数组，保存了pollfd信息，socket类型以及poll类型
    PollSlot pollSlotArray_[MAX_POLL] = {};
    // 保存临时数组pfds的PollSlot在ps_的索引
    int pollSlotIndex_[MAX_POLL] = {};
    SafeMap<int, int> fdErrCountMap_;
    SafeMap<int, std::shared_ptr<IObexSocketObserver>> onConnectMap_;

    void InitCmdFd();
    void AddPoll(int fd, int type, int flags);
    void SetPoll(PollSlot* pPollSlot, int fd, int type, int flags);
    unsigned int Flags2Pevents(unsigned int flags);
    void SockPollThread();
    bool ProcessCmdSock();

    void RemovePoll(PollSlot* pPollSlot);
    void ProcessDataSock(pollfd* pfds, int count);
    void CloseCmdFd();
    void PreparePollFds(pollfd* pfds, size_t size);
    std::shared_ptr<ObexSocketDevice> RecvSocketDevice(int fd, SocketType socketType);
    bool HandleMsg(int fd, int type);
};
}  // namespace bluetooth
}  // namespace OHOS

#endif // OBEX_SERVER_SOCKET_THREAD_H