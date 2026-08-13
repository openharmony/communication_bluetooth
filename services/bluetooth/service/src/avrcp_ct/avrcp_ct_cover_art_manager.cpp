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

#ifndef LOG_TAG
#define LOG_TAG "avrcp_ct_cover_art_manager"
#endif


#include "parameter.h"
#include "avrcp_ct_cover_art_manager.h"
#include "avrcp_ct_service.h"
#include "log.h"
#include "parameters.h"
#include "bt_uuid.h"

namespace OHOS {
namespace bluetooth {

AvrcpCtCoverArtManager::AvrcpCtCoverArtManager(CoverArtCallback callback)
    : downloadCallback_(std::move(callback))
{
    // 对齐双框架 AvrcpCoverArtManager(service, callback)：
    // 回调通过构造函数注入且为 const（final），不可后续替换。
    // TODO: 读取 persist.bluetooth.avrcpcontroller.BIP_DOWNLOAD_SCHEME system property
    downloadScheme_ = SCHEME_THUMBNAIL;
}

bool AvrcpCtCoverArtManager::IsConnected(const std::string& deviceAddr)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = clients_.find(deviceAddr);
    if (it == clients_.end()) {
        return false;
    }
    return it->second->IsConnected();
}

bool AvrcpCtCoverArtManager::Connect(const std::string& deviceAddr, uint16_t psm)
{
    HILOGI("Connect %{public}s, psm: %{public}d", deviceAddr.c_str(), psm);

    std::lock_guard<std::mutex> lock(mutex_);

    if (clients_.find(deviceAddr) != clients_.end()) {
        HILOGW("Client already exists for device: %{public}s", deviceAddr.c_str());
        return false;
    }

    auto callback = std::make_shared<BipClientCallback>(shared_from_this(), deviceAddr);
    auto client = std::make_shared<AvrcpCtBipClient>(deviceAddr, psm, callback);

    clients_[deviceAddr] = client;
    sessions_[deviceAddr] = std::make_shared<BipSession>(BipSession{deviceAddr, {}, {}});

    // make_shared 后对象已被 shared_ptr 管理，可安全触发异步 CONNECT
    client->Connect();

    HILOGI("BIP client connected successfully");
    return true;
}

void AvrcpCtCoverArtManager::Disconnect(const std::string& deviceAddr)
{
    HILOGI("Disconnect %{public}s", deviceAddr.c_str());

    std::lock_guard<std::mutex> lock(mutex_);

    auto it = clients_.find(deviceAddr);
    if (it != clients_.end()) {
        it->second->Shutdown();
        clients_.erase(it);
    }

    sessions_.erase(deviceAddr);
    cache_.ClearDevice(deviceAddr);
}

void AvrcpCtCoverArtManager::DisconnectAll()
{
    HILOGI("Disconnecting all BIP clients");

    std::lock_guard<std::mutex> lock(mutex_);

    for (auto& pair : clients_) {
        pair.second->Shutdown();
    }

    clients_.clear();
    sessions_.clear();
    cache_.ClearAll();
}

void AvrcpCtCoverArtManager::Cleanup()
{
    HILOGI("Cleaning up CoverArtManager");
    DisconnectAll();
}

std::string AvrcpCtCoverArtManager::DownloadCoverArt(const std::string& deviceAddr, const std::string& imageUuid)
{
    HILOGI("Download Image - device: %{public}s, uuid: %{public}s",
           deviceAddr.c_str(), imageUuid.c_str());

    if (cache_.HasImage(deviceAddr, imageUuid)) {
        HILOGI("Image is already downloaded");
        return "";
    }

    std::shared_ptr<AvrcpCtBipClient> client;
    std::string imageHandle;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = clients_.find(deviceAddr);
        if (it == clients_.end()) {
            HILOGE("Cannot download an image. No client is available.");
            return "";
        }
        client = it->second;
        // 锁内直接查 uuidToHandle，不调 GetHandleForUuid（它会再次加锁导致死锁）
        auto sessionIt = sessions_.find(deviceAddr);
        if (sessionIt == sessions_.end()) {
            HILOGW("No session for device: %{public}s", deviceAddr.c_str());
            return "";
        }
        auto handleIt = sessionIt->second->uuidToHandle.find(imageUuid);
        if (handleIt == sessionIt->second->uuidToHandle.end()) {
            HILOGW("No handle for UUID");
            return "";
        }
        imageHandle = handleIt->second;
    }

    if (!client->GetImageProperties(imageHandle)) {
        HILOGE("GetImageProperties failed");
        return "";
    }

    return imageUuid;
}

bool AvrcpCtCoverArtManager::IsCoverArtCached(const std::string& deviceAddr, const std::string& imageUuid)
{
    return cache_.HasImage(deviceAddr, imageUuid);
}

bool AvrcpCtCoverArtManager::GetCoverArt(const std::string& deviceAddr,
                                          const std::string& imageUuid, std::vector<uint8_t> &imageData)
{
    return cache_.GetImage(deviceAddr, imageUuid, imageData);
}

void AvrcpCtCoverArtManager::ClearDeviceCache(const std::string& deviceAddr)
{
    HILOGI("Clearing cache for device: %{public}s", deviceAddr.c_str());
    cache_.ClearDevice(deviceAddr);
}

void AvrcpCtCoverArtManager::RemoveImage(const std::string& deviceAddr, const std::string& imageUuid)
{
    HILOGI("Removing image: device=%{public}s, uuid=%{public}s", deviceAddr.c_str(), imageUuid.c_str());
    cache_.RemoveImage(deviceAddr, imageUuid);
}

bool AvrcpCtCoverArtManager::IsValidImageHandle(const std::string& handle)
{
    if (handle.empty() || handle.length() != MAX_HANDLE_LENGTH) {
        return false;
    }

    for (char c : handle) {
        if (!std::isdigit(c)) {
            return false;
        }
    }

    return true;
}

std::string AvrcpCtCoverArtManager::GetUuidForHandle(const std::string& deviceAddr, const std::string& handle)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto sessionIt = sessions_.find(deviceAddr);
    if (sessionIt == sessions_.end()) {
        HILOGW("No session for device: %{public}s", deviceAddr.c_str());
        return "";
    }

    auto& session = sessionIt->second;
    auto it = session->handleToUuid.find(handle);
    if (it != session->handleToUuid.end()) {
        return it->second;
    }

    // 不存在则生成新 UUID 并建立双向映射（对齐双框架 AvrcpBipSession.getHandleUuid 行为）
    std::string newUuid = Uuid::Random().ToString();
    session->handleToUuid[handle] = newUuid;
    session->uuidToHandle[newUuid] = handle;
    HILOGI("Create new uuid for handle: device=%{public}s, handle=%{public}s, uuid=%{public}s",
           deviceAddr.c_str(), handle.c_str(), newUuid.c_str());
    return newUuid;
}

std::string AvrcpCtCoverArtManager::GetHandleForUuid(const std::string& deviceAddr, const std::string& uuid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto sessionIt = sessions_.find(deviceAddr);
    if (sessionIt == sessions_.end()) {
        return "";
    }

    auto it = sessionIt->second->uuidToHandle.find(uuid);
    if (it == sessionIt->second->uuidToHandle.end()) {
        return "";
    }

    return it->second;
}

void AvrcpCtCoverArtManager::ClearHandleUuids(const std::string& deviceAddr)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto sessionIt = sessions_.find(deviceAddr);
    if (sessionIt == sessions_.end()) {
        return;
    }

    sessionIt->second->handleToUuid.clear();
    sessionIt->second->uuidToHandle.clear();
}

std::shared_ptr<AvrcpCtBipClient> AvrcpCtCoverArtManager::GetClient(const std::string& deviceAddr)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = clients_.find(deviceAddr);
    if (it == clients_.end()) {
        return nullptr;
    }
    return it->second;
}

AvrcpCtCoverArtManager::BipSession* AvrcpCtCoverArtManager::GetOrCreateSession(
    const std::string& deviceAddr)
{
    auto it = sessions_.find(deviceAddr);
    if (it != sessions_.end()) {
        return it->second.get();
    }

    auto session = std::make_shared<BipSession>(BipSession{deviceAddr, {}, {}});
    sessions_[deviceAddr] = session;
    return session.get();
}

BipImageDescriptor AvrcpCtCoverArtManager::DetermineImageDescriptor(
    const BipImageProperties& properties)
{
    if (properties.IsValid()) {
        HILOGI("Properties valid, using thumbnail format");
    } else {
        HILOGW("Provided properties don't meet the spec. Requesting thumbnail format anyway.");
    }

    switch (downloadScheme_) {
        case SCHEME_NATIVE:
            HILOGI("SCHEME_NATIVE: return native descriptor, RequestGetImage will not send IMG_DESCRIPTOR");
            return BipImageDescriptor{BIP_IMAGE_FORMAT_NATIVE, 0, 0};
        case SCHEME_THUMBNAIL:
        default: {
            BipImageDescriptor descriptor;
            descriptor.format = BIP_IMAGE_FORMAT_JPEG;
            descriptor.width = BIP_IMAGE_WIDTH;
            descriptor.height = BIP_IMAGE_HEIGHT;
            HILOGI("SCHEME_THUMBNAIL: JPEG 200x200");
            return descriptor;
        }
    }
}

void AvrcpCtCoverArtManager::BipClientCallback::OnConnectionStateChanged(int oldState, int newState)
{
    HILOGI("Connection state changed: %{public}d -> %{public}d", oldState, newState);

    auto manager = manager_.lock();
    if (!manager) {
        HILOGW("CoverArtManager already destroyed, skip OnConnectionStateChanged");
        return;
    }

    if (newState == AvrcpCtBipClient::STATE_CONNECTED) {
        manager->ClearHandleUuids(deviceAddr_);
        // BIP 连接成功后，若当前曲目还没有封面数据则重新请求元数据以拿到 image handle
        auto service = AvrcpCtService::GetService();
        if (service) {
            service->GetCurrentMetadataIfNoCoverArt(deviceAddr_);
        }
    } else if (newState == AvrcpCtBipClient::STATE_DISCONNECTED) {
        auto client = manager->GetClient(deviceAddr_);
        bool shouldReconnect = (client != nullptr);
        manager->Disconnect(deviceAddr_);
        if (shouldReconnect) {
            HILOGI("Disconnect was not expected, attempting to reconnect");
            manager->Connect(deviceAddr_, client->GetPsm());
        }
    }
}

void AvrcpCtCoverArtManager::BipClientCallback::OnGetImagePropertiesComplete(
    int status, const std::string& imageHandle, const BipImageProperties& properties)
{
    if (status != 0 || properties.GetImageHandle().empty()) {
        HILOGE("GetImageProperties() failed. Handle: %{public}s, Code: %{public}d",
               imageHandle.c_str(), status);
        return;
    }

    auto manager = manager_.lock();
    if (!manager) {
        HILOGW("CoverArtManager already destroyed, skip OnGetImagePropertiesComplete");
        return;
    }

    HILOGI("GetImageProperties complete for handle: %{public}s, %{public}s",
           imageHandle.c_str(), properties.ToString().c_str());

    auto client = manager->GetClient(deviceAddr_);
    if (client == nullptr) {
        HILOGW("Client disconnected, cannot getImage");
        return;
    }

    BipImageDescriptor descriptor = manager->DetermineImageDescriptor(properties);
    HILOGI("Download image - handle='%{public}s', descriptor: JPEG %ux%u",
           imageHandle.c_str(), descriptor.width, descriptor.height);

    client->GetImage(imageHandle, descriptor);
}

void AvrcpCtCoverArtManager::BipClientCallback::OnGetImageComplete(
    int status, const std::string& imageHandle, const BipImage& image)
{
    if (status != ResponseCodes::OBEX_HTTP_OK || image.data.empty()) {
        HILOGE("GetImage() failed - Handle: %{public}s, Code: %{public}d, size: %{public}zu",
               imageHandle.c_str(), status, image.data.size());
        return;
    }

    auto manager = manager_.lock();
    if (!manager) {
        HILOGW("CoverArtManager already destroyed, skip OnGetImageComplete");
        return;
    }

    std::string imageUuid = manager->GetUuidForHandle(deviceAddr_, imageHandle);
    HILOGI("Received image data for handle: %{public}s, uuid: %{public}s, size: %{public}zu",
           imageHandle.c_str(), imageUuid.c_str(), image.data.size());

    manager->cache_.AddImage(deviceAddr_, imageUuid, image.data);
    if (manager->downloadCallback_) {
        manager->downloadCallback_(deviceAddr_, imageUuid, image.data);
    }
}

}  // namespace bluetooth
}  // namespace OHOS