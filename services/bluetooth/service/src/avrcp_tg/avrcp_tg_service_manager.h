/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#ifndef AVRCP_TG_SERVICE_MANAGER_H
#define AVRCP_TG_SERVICE_MANAGER_H

#include "avrcp_tg_volume.h"
#ifdef AVRCP_AVSESSION
#include "avrcp_tg_avsession_media_loader.h"
#endif

namespace OHOS {
namespace bluetooth {
class AvrcpServiceManager {
public:
    AvrcpServiceManager() : mediaLoader_(std::make_shared<AvrcpTgAvsessionMediaLoader>()) {};
    ~AvrcpServiceManager() = default;
    static AvrcpServiceManager &GetInstance();
    void Init();
    void DeInit();
    void Connect(const RawAddress &rawAddr);
    void DisConnect(const RawAddress &rawAddr);
    void SwitchAbsVolumeDevice(const RawAddress &rawAddr);
    void SetDeviceAbsVolumeAbility(const RawAddress &rawAddr, int32_t ability);
    void SetDeviceAbsoluteVolume(const RawAddress &rawAddr, int32_t volumeLevel);
    int32_t GetDeviceAbsVolumeAbility(const RawAddress &rawAddr);
    void NotifyAudioVolumeEvent(int32_t streamType, int32_t volume);
    void SetDeviceAbsVolumeProperty(const OHOS::bluetooth::RawAddress &rawAddr, int32_t ability);
    void SetActiveDevice(const RawAddress &rawAddr);
    void DisableBipService();

    std::shared_ptr<AvrcpTgAvsessionMediaLoader> GetAvrcpMediaLoader(void) const
    {
        return mediaLoader_;
    }
private:
    ServiceInterface *avrcpServiceInterface_{nullptr};
    std::shared_ptr<AvrcpTgAvsessionMediaLoader> mediaLoader_ {nullptr};
};
}  // namespace bluetooth
}  // namespace OHOS

#endif