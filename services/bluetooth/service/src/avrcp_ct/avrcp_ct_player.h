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

#ifndef AVRCP_CT_PLAYER_H
#define AVRCP_CT_PLAYER_H

#include <string>
#include <memory>
#include <cstdint>
#include "avplayback_state.h"
#include "bt_def.h"

namespace OHOS {
namespace bluetooth {

class AvrcpCtMediaItem;

class AvrcpCtPlayer {
public:
    static constexpr int DEFAULT_ID = -1;

    static constexpr int TYPE_UNKNOWN = -1;
    static constexpr int TYPE_AUDIO = 0;
    static constexpr int TYPE_VIDEO = 1;
    static constexpr int TYPE_BROADCASTING_AUDIO = 2;
    static constexpr int TYPE_BROADCASTING_VIDEO = 3;

    static constexpr int FEATURE_PLAY = 40;
    static constexpr int FEATURE_STOP = 41;
    static constexpr int FEATURE_PAUSE = 42;
    static constexpr int FEATURE_REWIND = 44;
    static constexpr int FEATURE_FAST_FORWARD = 45;
    static constexpr int FEATURE_FORWARD = 47;
    static constexpr int FEATURE_PREVIOUS = 48;

    AvrcpCtPlayer();
    ~AvrcpCtPlayer() = default;

    void SetPlayerId(int id);
    int GetPlayerId() const;

    void SetPlayerName(const std::string& name);
    std::string GetPlayerName() const;

    void SetPlayerType(int type);
    int GetPlayerType() const;

    void SetPlayStatus(int status);
    int GetPlayStatus() const;

    void SetPlayTime(int64_t playTime);
    int64_t GetPlayTime() const;

    void SetPlaySpeed(float speed);
    float GetPlaySpeed() const;

    void SetDuration(int64_t duration);
    int64_t GetDuration() const;

    void UpdateCurrentTrack(std::shared_ptr<AvrcpCtMediaItem> track);
    std::shared_ptr<AvrcpCtMediaItem> GetCurrentTrack() const;

    AVSession::AVPlaybackState ToAVPlaybackState() const;

    bool HasCoverArt() const;
    std::string GetCoverArtHandle() const;

    bool NotifyImageDownload(const std::string& uuid, const std::string& uri);

    void SetSupportedFeatures(const std::vector<uint8_t>& features);
    bool SupportsFeature(int featureId) const;
    void SetSupportedFeature(int featureId);

    void SetDeviceAddress(const std::string& address);
    std::string GetDeviceAddress() const;

    void SetPlayPosition(int64_t position);
    int64_t GetPlayPosition() const;

private:
    std::string deviceAddr_;
    int playerId_ = DEFAULT_ID;
    std::string playerName_;
    int playerType_ = TYPE_UNKNOWN;
    int playStatus_ = 0;
    int64_t playPosition_ = 0;
    int64_t playTime_ = 0;
    int64_t playTimeUpdate_ = 0;
    float playSpeed_ = 1.0f;
    int64_t duration_ = 0;
    std::vector<uint8_t> playerFeatures_;
    std::shared_ptr<AvrcpCtMediaItem> currentTrack_;
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // AVRCP_CT_PLAYER_H