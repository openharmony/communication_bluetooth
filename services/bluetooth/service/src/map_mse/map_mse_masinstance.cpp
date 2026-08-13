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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_map_mse_masinstance"
#endif

#include "map_mse_masinstance.h"

#include "map_mse_service_impl.h"
#include "log.h"

namespace OHOS {
namespace bluetooth {
MapMseMasInstance::MapMseMasInstance(int32_t masId, MapMseServiceImpl *mapMseServiceImpl) : masInstanceId_(masId),
    mapMseServiceImpl_(mapMseServiceImpl)
{
    HILOGI("MapMseMasInstance Create, masInstanceId %{public}d.", masInstanceId_);
}

bool MapMseMasInstance::StartObexServerSession(
    std::shared_ptr<ObexSocketDevice> &socketDevice, std::shared_ptr<MapMseMnsClient> mnsClient)
{
    HILOGI("StartObexServerSession.");
    std::lock_guard<std::mutex> lock(obexServerSessionMutex_);
    if (obexServerSession_ != nullptr) {
        HILOGI("obexServerSession_ is alread create.");
        return true;
    }

    auto obexTransport = std::make_shared<ObexServerTransport>(socketDevice);
    mapMseObexServer_ = std::make_shared<MapMseObexServer>(shared_from_this(), mnsClient, mapMseServiceImpl_);
    obexServerSession_ = std::make_shared<ObexServerSession>(obexTransport, mapMseObexServer_);
    obexServerSession_->Start();
    return true;
}

void MapMseMasInstance::UpdateMnsMseClient(std::shared_ptr<MapMseMnsClient> mnsClient)
{
    HILOGI("UpdateMnsMseClient.");
    if (mnsClient == nullptr) {
        HILOGE("UpdateMnsMseClient mnsClient is nullptr.");
        return;
    }
    std::lock_guard<std::mutex> lock(obexServerSessionMutex_);
    if (mapMseObexServer_ == nullptr || obexServerSession_ == nullptr) {
        HILOGE("obexServerSession not start.");
        return;
    }
    mapMseObexServer_->UpdateMnsClient(mnsClient);
    obexServerSession_->UpdateHandler(mapMseObexServer_);
}

void MapMseMasInstance::StopObexServerSession()
{
    std::lock_guard<std::mutex> lock(obexServerSessionMutex_);
    if (!obexServerSession_) {
        return;
    }
    obexServerSession_->Stop();
    obexServerSession_ = nullptr;
}

void MapMseMasInstance::OnClose()
{
    CHECK_AND_RETURN_LOG(mapMseServiceImpl_, "mapMseServiceImpl_ is null");
    mapMseServiceImpl_->OnClose(masInstanceId_);
}

int32_t MapMseMasInstance::GetMasId()
{
    return masInstanceId_;
}

void MapMseMasInstance::UpdateFolderVersionCounter()
{
    folderVersionCounter_++;
}

int64_t MapMseMasInstance::GetFolderVersionCounter()
{
    return folderVersionCounter_;
}

int64_t MapMseMasInstance::GetCombinedConversationVersionCounter()
{
    return combinedVersionCounter_;
}

}  // namespace bluetooth
}  // namespace OHOS