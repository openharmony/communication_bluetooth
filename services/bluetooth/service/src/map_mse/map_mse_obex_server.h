/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MAP_MSE_OBEX_SERVER_H
#define MAP_MSE_OBEX_SERVER_H

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include "../obex/obex_request_handler.h"
#include "../obex/obex_def.h"
#include "map_mse_folder.h"
#include "map_mse_appparams.h"
#include "map_mse_content.h"
#include "map_mse_content_observer.h"

namespace OHOS {
namespace bluetooth {
const int32_t UUID_LENGTH = 16;
const int32_t THREADED_MAIL_HEADER_ID = 0xFA;
const int32_t THREAD_MAIL_KEY = 0x534c5349;
const int32_t MAX_LIST_COUNT = 1024;

class MapMseMasInstance;
class MapMseServiceImpl;
class MapMseObexServer : public ObexRequestHandler {
public:
    explicit MapMseObexServer(
        std::weak_ptr<MapMseMasInstance> masInstance, std::weak_ptr<MapMseMnsClient> mnsClient,
        MapMseServiceImpl *mapMseServiceImpl);
    virtual ~MapMseObexServer(){};

    int OnGet(ObexHeader &request, ObexHeader &reply, std::vector<uint8_t> &output, bool &sendBodyHeader) override;
    int OnPut(ObexHeader &request, ObexHeader &reply, std::vector<uint8_t> &input, bool &requestFinished) override;
    int OnConnect(ObexHeader &request, ObexHeader &reply) override;
    int OnDisconnect(ObexHeader &request, ObexHeader &reply) override;
    int OnSetPath(ObexHeader &request, ObexHeader &reply, bool backup, bool create) override;
    int OnDelete(ObexHeader &request, ObexHeader &reply) override;
    int OnAbort(ObexHeader &request, ObexHeader &reply) override;
    void OnClose(bool isThreadStart) override;
    void SetConnectionId(int connectionId) override;
    int GetConnectionId() override;
    void UpdateMnsClient(std::weak_ptr<MapMseMnsClient> mnsClient);

private:
    void CreateFolder();
    void InitDefaultFolders(std::shared_ptr<MapMseFolder> &root);
    void InitSmsMmsFolders(std::shared_ptr<MapMseFolder> &root);

    int SendFolderListingRsp(ObexHeader &reply, std::vector<uint8_t> &output, bool &sendBodyHeader);
    int SendMessageListingRsp(
        ObexHeader &request, ObexHeader &reply, std::vector<uint8_t> &output, bool &sendBodyHeader);
    int SendConvoListingRsp(ObexHeader &reply, bool &sendBodyHeader);
    int SendMasInstanceInfoRsp(std::vector<uint8_t> &output);
    int SendMessageRsp(ObexHeader &request, std::vector<uint8_t> &output);
    int PushMessage(const std::string &folderName,
        ObexHeader &reply, std::vector<uint8_t> &input, bool requestFinished);

    std::shared_ptr<MapMseFolder> ParseMessageListingFolder(ObexHeader &request);
    std::shared_ptr<MapMseFolder> GetFolderElementFromName(std::string folderName);

private:
    std::weak_ptr<MapMseMasInstance> masInstance_;
    std::string messageVersion_ = MAP_V10_STR;
    bool threadIdSupport_ = false;
    int32_t remoteFeatureMask_ = MAP_FEATURE_DEFAULT_BITMASK;
    std::shared_ptr<MapMseFolder> currentFolder_;
    std::shared_ptr<MapMseAppParams> appParams_ = nullptr;
    std::shared_ptr<MapMseContent> content_ = nullptr;
    std::shared_ptr<MapMseContentObserver> contentObserver_ = nullptr;
    int32_t connectionId_ = -1;
    MapMseServiceImpl *mapMseServiceImpl_ = nullptr;
    std::vector<uint8_t> inputMessage_ {};
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // MAP_MSE_OBEX_SERVER_H