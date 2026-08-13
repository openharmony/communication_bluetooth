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

#ifndef OPP_TRANSFER_H
#define OPP_TRANSFER_H

#include <condition_variable>
#include "opp_def.h"
#include "opp_transfer_information.h"
#include "opp_obex_client.h"
#include "../obex/obex_server_session.h"
#include <queue>
#include "bt_chr_dft_statictics.h"
#include "btcommon/timer_manager.h"
#include "util/safe_vector.h"

namespace OHOS {
namespace bluetooth {
class OppServiceImpl;
class OppTransfer {
public:
    OppTransfer(const std::string &address, const std::vector<IOppTransferFileHolder> fileHolders, int direction,
       std::shared_ptr<ObexServerSession> serverSession, OppServiceImpl *oppServiceImpl);
    ~OppTransfer();

    /**
     * @brief get transfer file count
     *
     * @return transfer file count
     */
    int GetFileNumber();

    /**
     * @brief ConnectObex
     *
     * @return error code
     */
    int ConnectObex(std::shared_ptr<ObexClientConfig> obexConfig);

    /**
     * @brief get Device Address
     *
     * @return device address
     */
    std::string GetDeviceAddress();

    /**
     * @brief OnReceiveIncomingConnect
     *
     * @parma connectId
     */
    void OnReceiveIncomingConnect(uint32_t connectId, int32_t totalFileCount);

    /**
     * @brief AcceptConnect
     *
     * @return 0 fail 1 success
     */
    bool AcceptConnect();

    /**
     * @brief GetDirection
     *
     * @return GetDirection
     */
    int GetDirection() const;

    /**
     * @brief GetFirstTransferFileName
     *
     * @return Get first transfer fileName
     */
    std::string GetFirstTransferFileName();

    /**
     * @brief OnObexDisconnected
     */
    void OnObexDisconnected();

    /**
     * @brief OnObexDisconnected
     */
    int StartTransfer();

    /**
     * @brief CancelTransfer
     */
    int CancelTransfer();

    /**
     * @brief OnTransferStateChange
     * @parma state
     * @parma reason
     */
    void OnTransferStateChange(int state, int reason);

    void OnTransferPositionChange(size_t position);

    int OnReceiveIncomingFile(IOppTransferInformation info);

    int SetIncomingFileConfirmation(bool accept, int fd);

    void ContinueSendFileList();

    void DisconnectObex() const;

    void SetConfirm(int confirm);

    IOppTransferInformation GetCurrentTransferInformation();

    utility::SafeVector<std::shared_ptr<IOppTransferInformation>> fileList_;

    int transferId_ = 0;
private:
    void IncomingConnectTimeout();
    void IncomingFileTimeout();
    void OnTransferStateChangeRunning();
    void OnTransferStateChangeRunning(int fd);
    void OnTransferStateChangeSuccess();
    void OnTransferStateChangeFaild(int reason);
    std::shared_ptr<IOppTransferInformation> GetOppTransferInformation(IOppTransferInformation info);

    std::shared_ptr<IOppTransferInformation> curretTransferInfo_ = nullptr;
    std::shared_ptr<OppObexClient> obexClient_ = nullptr;
    std::shared_ptr<ObexServerSession> serverSession_ = nullptr;

    std::unique_ptr<utility::Timer> incomingConnectTimer_ = nullptr;

    static int currentTransferId_;
    uint32_t connectId_ = 0;
    int32_t totalReceiveFileCount_ = 0;
    int32_t curReceiveFileCount_ = 0;
    bool isConnected_ = false;
    time_t timeStamp_ = 0;
    std::string address_;
    std::string deviceName_;
    OppServiceImpl *oppServiceImpl_ = nullptr;
    int direction_ = OPP_TRANSFER_DIRECTION_OUTBOUND;
    std::atomic<int> confirm_  = OPP_TRANSFER_CONFIRM_PENDING;
    std::mutex syncMutex = {};
    std::condition_variable cv = {};
    std::atomic<bool> receiveFileReady_ = false;
    std::mutex receiveFileReadySyncMutex_ = {};
    std::condition_variable receiveFileReadycv_ = {};
};

}  // namespace bluetooth
}  // namespace OHOS
#endif // OPP_TRANSFER_H