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

#ifndef AVRCP_CT_BIP_CLIENT_H
#define AVRCP_CT_BIP_CLIENT_H

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <atomic>

#include "bt_def.h"
#include "raw_address.h"
#include "log.h"
#include "bt_uuid.h"
#include "obex_socket_device.h"
#include "obex_server_transport.h"
#include "obex_client_session.h"
#include "obex_header.h"
#include "obex_response_handler.h"
#include "thread_util.h"

#include "avrcp_ct_bip_request.h"
#include "avrcp_ct_bip_image_properties.h"

namespace OHOS {
namespace bluetooth {

class AvrcpCtBipClient : public std::enable_shared_from_this<AvrcpCtBipClient> {
public:
    static constexpr int STATE_DISCONNECTED = 0;
    static constexpr int STATE_CONNECTING = 1;
    static constexpr int STATE_CONNECTED = 2;
    static constexpr int STATE_DISCONNECTING = 3;

    class Callback {
    public:
        virtual ~Callback() = default;
        virtual void OnConnectionStateChanged(int oldState, int newState) = 0;
        virtual void OnGetImagePropertiesComplete(int status, const std::string &imageHandle,
                                                  const BipImageProperties &properties) = 0;
        virtual void OnGetImageComplete(int status, const std::string &imageHandle, const BipImage &image) = 0;
    };

    AvrcpCtBipClient(const std::string &deviceAddr, uint16_t psm, std::shared_ptr<Callback> callback);
    ~AvrcpCtBipClient();

    bool Connect();
    void Disconnect();
    void Shutdown();
    int GetState() const;
    bool IsConnected() const;
    uint16_t GetPsm() const;

    bool GetImageProperties(const std::string &imageHandle);
    bool GetImage(const std::string &imageHandle, const BipImageDescriptor &descriptor);

private:
    // 在全局 BIP 线程上执行（DoInBipThread），替代手搓 HandlerThread
    void PostToBipThread(const ThreadUtilFunc &func);

    void ProcessConnect();
    void ProcessDisconnect();
    void ProcessRequest(std::shared_ptr<BipRequest> request);
    void NotifyCaller(std::shared_ptr<BipRequest> request);

    bool ConnectL2cap();
    bool CreateObexSession();
    void DestroyObexSession();

    void SetConnectionState(int state);

    std::string deviceAddr_;
    uint16_t psm_;
    std::atomic<int> state_{STATE_DISCONNECTED};
    std::shared_ptr<Callback> callback_;
    ObexHeader connectHeader_;

    int socketFd_ = -1;
    std::shared_ptr<ObexSocketDevice> socketDevice_ = nullptr;
    std::shared_ptr<ObexServerTransport> transport_ = nullptr;
    std::shared_ptr<ObexClientSession> clientSession_ = nullptr;
    std::shared_ptr<ObexResponseHandler> responseHandler_ = nullptr;
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // AVRCP_CT_BIP_CLIENT_H
