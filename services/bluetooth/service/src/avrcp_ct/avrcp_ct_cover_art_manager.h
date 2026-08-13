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

#ifndef AVRCP_CT_COVER_ART_MANAGER_H
#define AVRCP_CT_COVER_ART_MANAGER_H

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "avrcp_ct_cover_art_cache.h"
#include "avrcp_ct_bip_client.h"
#include "avrcp_ct_bip_image_properties.h"

#define MAX_HANDLE_LENGTH 7

namespace OHOS {
namespace bluetooth {

class AvrcpCtCoverArtManager : public std::enable_shared_from_this<AvrcpCtCoverArtManager> {
public:
    static constexpr const char *AVRCP_CONTROLLER_COVER_ART_SCHEME =
        "persist.bluetooth.avrcpcontroller.BIP_DOWNLOAD_SCHEME";
    static constexpr const char *NATIVE = "native";
    static constexpr const char *THUMBNAIL = "thumbnail";

    using CoverArtCallback = std::function<void(const std::string &deviceAddr, const std::string &imageUuid,
                                                const std::vector<uint8_t> &imageData)>;

    // 对齐双框架：回调通过构造函数注入（final mCallback），不可后续替换，
    // 从根源杜绝多设备 StateMachine 重复 Set 导致的回调覆盖问题。
    explicit AvrcpCtCoverArtManager(CoverArtCallback callback);
    ~AvrcpCtCoverArtManager() = default;

    bool Connect(const std::string &deviceAddr, uint16_t psm);
    void Disconnect(const std::string &deviceAddr);
    void DisconnectAll();
    void Cleanup();

    std::string DownloadCoverArt(const std::string &deviceAddr, const std::string &imageUuid);

    bool IsCoverArtCached(const std::string &deviceAddr, const std::string &imageUuid);
    bool GetCoverArt(const std::string &deviceAddr, const std::string &imageUuid, std::vector<uint8_t> &imageData);
    void RemoveImage(const std::string &deviceAddr, const std::string &imageUuid);
    void ClearDeviceCache(const std::string &deviceAddr);

    static bool IsValidImageHandle(const std::string &handle);

    bool IsConnected(const std::string &deviceAddr);

    std::string GetUuidForHandle(const std::string &deviceAddr, const std::string &handle);

    BipImageDescriptor DetermineImageDescriptor(const BipImageProperties &properties);

    enum DownloadScheme { SCHEME_NATIVE = 0, SCHEME_THUMBNAIL = 1 };

    const std::unordered_map<std::string, int> SchemeNameMap = {{"native", SCHEME_NATIVE},
                                                                {"thumbnail", SCHEME_THUMBNAIL}};

    DownloadScheme downloadScheme_ = SCHEME_THUMBNAIL;

private:
    std::string GetHandleForUuid(const std::string &deviceAddr, const std::string &uuid);
    void ClearHandleUuids(const std::string &deviceAddr);
    std::shared_ptr<AvrcpCtBipClient> GetClient(const std::string &deviceAddr);

    struct BipSession {
        std::string deviceAddr;
        std::unordered_map<std::string, std::string> handleToUuid;
        std::unordered_map<std::string, std::string> uuidToHandle;
    };

    BipSession *GetOrCreateSession(const std::string &deviceAddr);

    class BipClientCallback : public AvrcpCtBipClient::Callback {
    public:
        BipClientCallback(std::weak_ptr<AvrcpCtCoverArtManager> manager, const std::string &deviceAddr)
            : manager_(std::move(manager)),
              deviceAddr_(deviceAddr)
        {
        }

        void OnConnectionStateChanged(int oldState, int newState) override;
        void OnGetImagePropertiesComplete(int status, const std::string &imageHandle,
                                          const BipImageProperties &properties) override;
        void OnGetImageComplete(int status, const std::string &imageHandle, const BipImage &image) override;

    private:
        // weak_ptr 避免 CoverArtManager 先于 BipClient 析构时悬垂引用
        std::weak_ptr<AvrcpCtCoverArtManager> manager_;
        std::string deviceAddr_;
    };

    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::shared_ptr<AvrcpCtBipClient>> clients_;
    std::unordered_map<std::string, std::shared_ptr<BipSession>> sessions_;
    AvrcpCtCoverArtCache cache_;
    const CoverArtCallback downloadCallback_;  // 构造函数注入，不可变（对齐双框架 final mCallback）
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // AVRCP_CT_COVER_ART_MANAGER_H