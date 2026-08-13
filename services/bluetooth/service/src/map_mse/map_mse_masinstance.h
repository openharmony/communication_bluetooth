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

#ifndef MAP_MSE_MASINSTANCE_H
#define MAP_MSE_MASINSTANCE_H

#include <string>
#include <cstdint>

#include "map_mse_obex_server.h"
#include "../obex/obex_server_session.h"

namespace OHOS {
namespace bluetooth {
class MapMseServiceImpl;
class MapMseMasInstance : public std::enable_shared_from_this<MapMseMasInstance> {
public:
    explicit MapMseMasInstance(int32_t masId, MapMseServiceImpl *mapMseServiceImpl);
    virtual ~MapMseMasInstance() {}

    bool StartObexServerSession(
        std::shared_ptr<ObexSocketDevice> &socketDevice, std::shared_ptr<MapMseMnsClient> mnsClient);
    void StopObexServerSession();

    void OnClose();
    void UpdateMnsMseClient(std::shared_ptr<MapMseMnsClient> mnsClient);

    int32_t GetMasId();
    int64_t GetFolderVersionCounter();
    int64_t GetCombinedConversationVersionCounter();

private:
    void UpdateFolderVersionCounter();

private:
    int32_t masInstanceId_ = -1;
    int64_t folderVersionCounter_ = 0;
    int64_t combinedVersionCounter_ = 0;

    std::mutex obexServerSessionMutex_ {};
    std::shared_ptr<ObexServerSession> obexServerSession_ = nullptr;
    MapMseServiceImpl *mapMseServiceImpl_ = nullptr;
    std::shared_ptr<MapMseObexServer> mapMseObexServer_ = nullptr;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // MAP_MSE_MASINSTANCE_H