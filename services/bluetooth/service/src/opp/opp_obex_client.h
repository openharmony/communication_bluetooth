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

#ifndef OPP_OBEX_CLIENT_H
#define OPP_OBEX_CLIENT_H

#include "bt_chr_dft_statictics.h"
#include "context.h"
#include "opp_def.h"
#include "opp_file_utils.h"
#include "opp_transfer_information.h"
#include <fstream>
#include "../sdp_adapter/sdp_adapter.h"
#include "../obex/obex_body.h"
#include "../obex/obex_client_session.h"
#include "../obex/obex_socket_device.h"
#include "../obex/obex_server_socket.h"
#include "../obex/obex_response_handler.h"

namespace OHOS {
namespace bluetooth {
class OppSendFileBodyObject : public ObexBodyObject {
public:
    explicit OppSendFileBodyObject(const int fd, const std::string &fileName);
    OppSendFileBodyObject() = default;
    ~OppSendFileBodyObject() override;
    void OpenFile(const int fd) override;
    size_t Read(uint8_t *buf, size_t bufLen) override;
    size_t Write(const uint8_t *buf, size_t bufLen) override;
    int Close() override;
    size_t GetFileSize() const;
    size_t GetFileSendSize() const;
    bool IsReadFinished() override;

private:
    FILE* fdFile_ = nullptr;
    int fd_ = 0;
    int64_t fileSize_ = 0;
    size_t fileSendSize_ = 0;
};
struct ObexClientConfig {
    std::string addr;                        // Remote bluetooth address
    uint16_t l2capLocalPsm = 0;              // l2cap's local psm while use l2cap
    uint16_t l2capPsm = 0;                   // l2cap's psm
    uint16_t rfCommChannel = 0;              // Rfcomm's channel num/ l2cap's psm
    uint16_t mtu = 0;                       // The Maximum OBEX Packet Length.Default 1024byte
    int32_t fileCount = 0;                  // The transfer file Count
    bool isL2capPSM = false;                 // L2cap:true, rfcomm:false
    bool isSupportSrm = false;               // Using Single Response Mode
    bool isSupportReliableSession = false;   // Using reliable session
    std::string serviceUUID;                 // Service's UUID128
};
class OppServiceImpl;
class OppObexClient : public ObexResponseHandler {
public:
    explicit OppObexClient(std::shared_ptr<ObexClientConfig> obexConfig, OppServiceImpl *oppServiceImpl);
    virtual ~OppObexClient() {}

    void OnConnectResp(ObexHeader &reply, uint8_t responseCode) override;
    void OnDisConnectResp(ObexHeader &reply, uint8_t responseCode) override;
    void OnPutDateAvailableResp(ObexHeader &reply, uint8_t responseCode) override;
    void OnAbortResp() override;

    void Connect(std::shared_ptr<OppObexClient> obexClient);
    void Disconnect(bool withObexReq);
    void DisconnectLocal(bool withObexReq);
    int SendFile(std::shared_ptr<IOppTransferInformation> fileInfo);
    int CancelSendFile(bool isImmediately);
    void OnActionCompleted(ObexHeader &reply, uint8_t responseCode);
    void SetBusy(bool isBusy);
    void OnBusy(bool isBusy);

private:
    std::shared_ptr<ObexSocketDevice> RecvSocketDevice(SocketType socketType);
    void SendFileBody();
    void SendFileOneBody();
    static void SendFileHeader(std::shared_ptr<ObexClientSendObject> sendObject,
        std::shared_ptr<IOppTransferInformation> fileInfo, std::shared_ptr<ObexClientSession> clientSession);
    void SendAbortRequest();
    bool CheckBeforeRequest(uint8_t opeId);
    void Abort();
    void OnTransferStateChangeFaild(int reason);
    void OnOperationActionSync(uint8_t responseCode);

    std::mutex socketFdLock_;
    int32_t socketFd_ = -1;
    bool connected_ = false;
    int32_t connectionId_ = 0;
    std::string address_;
    bool sendAbort_ = false;
    bool isSupportSrm_ = true;
    int status_ = OPP_OBEX_STATUS_IDLE;
    std::mutex mutexBusyChanged_;
    std::condition_variable cvWaitBusyChanged_;
    OppServiceImpl *oppServiceImpl_ = nullptr;
    std::shared_ptr<ObexClientConfig> obexConfig_ = nullptr;
    std::shared_ptr<ObexClientSession> clientSession_ = nullptr;
    std::shared_ptr<OppSendFileBodyObject> fileObject_ = nullptr;
    bool isWaitingSendAbort_ = false;
    bool isObexConnected_ = false;
    bool isAbortSended_ = false;
    bool isProcessing_ = false;
    bool isBusy_ = false;
    std::atomic_bool isFirstFileBody_ = true;
    ObexHeader connectHeader_;
    ObexHeader replyHeader_;
    ObexHeader requestHeader_;
};

}  // namespace bluetooth
}  // namespace OHOS
#endif