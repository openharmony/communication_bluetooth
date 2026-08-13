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

#ifndef MAP_MSE_MNS_CLIENT_H
#define MAP_MSE_MNS_CLIENT_H

#include "../obex/obex_client_session.h"
#include "../sdp_adapter/sdp_adapter.h"
#include "../obex/obex_socket_device.h"

namespace OHOS {
namespace bluetooth {
class MapMseServiceImpl;
class MapMseMnsClient {
public:
    explicit MapMseMnsClient(const MapMnsSdpFoundRecord &record, MapMseServiceImpl *mapMseServiceImpl);
    ~MapMseMnsClient() = default;

    void HandleRegistration(int32_t masId, uint8_t notificationStatus);
    void Connect();
    void DisConnect();
    void SendEvent(int32_t masId, const std::string &eventData);
    bool IsConnect();

private:
    std::shared_ptr<ObexSocketDevice> RecvSocketDevice(SocketType socketType);

private:
    MapMnsSdpFoundRecord mnsRecord_;
    std::vector<int32_t> registerMasIds_{};
    // lock map client connect and disconnect process.
    std::mutex connectLock_;
    std::atomic<bool> connected_ = false;
    int32_t socketFd_ = -1;
    std::shared_ptr<ObexClientSession> clientSession_ = nullptr;
    ObexHeader connectHeader_;
    MapMseServiceImpl *mapMseServiceImpl_ = nullptr;
};

}  // namespace bluetooth
}  // namespace OHOS
#endif