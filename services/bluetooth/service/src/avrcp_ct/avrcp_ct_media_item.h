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

#ifndef AVRCP_CT_MEDIA_ITEM_H
#define AVRCP_CT_MEDIA_ITEM_H

#include "raw_address.h"
#include "log.h"
#include "common_util.h"
#include "avmeta_data.h"
#include "avplayback_state.h"
#include "bluetooth_def.h"

namespace OHOS {
namespace bluetooth {

class AvrcpCtMediaItem {
public:
    static constexpr uint32_t MEDIA_ATTRIBUTE_TITLE = 0x01;
    static constexpr uint32_t MEDIA_ATTRIBUTE_ARTIST_NAME = 0x02;
    static constexpr uint32_t MEDIA_ATTRIBUTE_ALBUM_NAME = 0x03;
    static constexpr uint32_t MEDIA_ATTRIBUTE_TRACK_NUMBER = 0x04;
    static constexpr uint32_t MEDIA_ATTRIBUTE_TOTAL_TRACK_NUMBER = 0x05;
    static constexpr uint32_t MEDIA_ATTRIBUTE_GENRE = 0x06;
    static constexpr uint32_t MEDIA_ATTRIBUTE_PLAYING_TIME = 0x07;
    static constexpr uint32_t MEDIA_ATTRIBUTE_COVER_ART_HANDLE = 0x08;

    // 封面 handle 合法性校验（对齐双框架 AvrcpCoverArtManager.isValidImageHandle：7 位纯数字）
    static constexpr size_t COVER_ART_HANDLE_LENGTH = 7;
    static bool IsValidImageHandle(const std::string &handle)
    {
        if (handle.empty() || handle.length() != COVER_ART_HANDLE_LENGTH) {
            return false;
        }
        for (char c : handle) {
            if (!std::isdigit(static_cast<unsigned char>(c))) {
                return false;
            }
        }
        return true;
    }

    static constexpr uint8_t TYPE_PLAYER = 0x1;
    static constexpr uint8_t TYPE_FOLDER = 0x2;
    static constexpr uint8_t TYPE_MEDIA = 0x3;

    // 封面下载中标记，写入 AVMetaData.Description 字段，告知上层 AVSessionController 暂缓互联网封面匹配
    static constexpr const char *COVER_ART_DOWNLOADING_FLAG = "bt_cover_downloading";

    void CreateAvrcpAttribute(std::vector<uint32_t> &attrIds, std::vector<std::string> &attrVals)
    {
        int attributeCount = std::min(attrIds.size(), attrVals.size());
        for (int i = 0; i < attributeCount; i++) {
            HILOGD("attrId=%{public}d, attrVal=%{public}s", i, attrVals[i].c_str());
            switch (attrIds[i]) {
                case MEDIA_ATTRIBUTE_TITLE:
                    title_ = attrVals[i];
                    break;
                case MEDIA_ATTRIBUTE_ARTIST_NAME:
                    artistName_ = attrVals[i];
                    break;
                case MEDIA_ATTRIBUTE_ALBUM_NAME:
                    albumName_ = attrVals[i];
                    break;
                case MEDIA_ATTRIBUTE_TRACK_NUMBER:
                    ConvertStrToDigit(attrVals[i], trackNumber_);
                    break;
                case MEDIA_ATTRIBUTE_TOTAL_TRACK_NUMBER:
                    ConvertStrToDigit(attrVals[i], totalNumberOfTracks_);
                    break;
                case MEDIA_ATTRIBUTE_GENRE:
                    genre_ = attrVals[i];
                    break;
                case MEDIA_ATTRIBUTE_PLAYING_TIME:
                    ConvertStrToDigit(attrVals[i], playingTime_);
                    break;
                case MEDIA_ATTRIBUTE_COVER_ART_HANDLE:
                    // 对齐双框架 parseImageHandle：仅当 handle 合法（7 位纯数字）时才保留，否则留空
                    if (IsValidImageHandle(attrVals[i])) {
                        coverArtHandle_ = attrVals[i];
                    }
                    break;
                default:
                    HILOGE("invalid attrIds");
            }
        }
    }

    // common field
    RawAddress device_{""};
    uint64_t uid_{0};
    uint8_t itemType_{0};

    // media item
    uint8_t mediaType_{0};
    std::string mediaDisplayName_{""};

    // folder item
    uint8_t folderType_{0};
    bool playable_{false};
    std::string folderDisplayName_{""};

    // media attributes
    std::string uuid_{""};
    std::string displayName_{""};
    std::string title_{""};
    std::string artistName_{""};
    std::string albumName_{""};
    uint64_t trackNumber_{0};
    uint64_t totalNumberOfTracks_{0};
    std::string genre_{""};
    uint64_t playingTime_{0};
    std::string coverArtHandle_{""};

    std::string imageUuid_{""};

    std::vector<uint8_t> coverArtData_{};

    bool HasCoverArt() const
    {
        return !coverArtData_.empty() || !coverArtHandle_.empty();
    }

    std::string GetCoverArtUuid() const
    {
        return imageUuid_;
    }

    void SetCoverArtUuid(const std::string& uuid)
    {
        imageUuid_ = uuid;
    }

    std::string GetCoverArtHandle() const
    {
        return coverArtHandle_;
    }

    void SetCoverArtHandle(const std::string& handle)
    {
        coverArtHandle_ = handle;
    }

    const std::vector<uint8_t>& GetCoverArtData() const
    {
        return coverArtData_;
    }

    void SetCoverArtData(const std::vector<uint8_t>& data)
    {
        coverArtData_ = data;
    }

    AVSession::AVMetaData ToAVMetaData() const
    {
        AVSession::AVMetaData meta;
        meta.SetAssetId(title_); //todo assetId暂时标注为title_,需确定这个数据应该是什么
        meta.SetTitle(title_);
        meta.SetArtist(artistName_);
        meta.SetAlbum(albumName_);
        meta.SetDuration(playingTime_);

        if (!coverArtData_.empty()) {
            // 封面已就绪：写入真实封面，Description 留空（清除下载中标记）
            auto pixelMap = std::make_shared<AVSession::AVSessionPixelMap>();
            pixelMap->SetInnerImgBuffer(coverArtData_);
            meta.SetMediaImage(pixelMap);
        } else if (!coverArtHandle_.empty()) {
            // 封面待下载：复用 Description 字段写入下载中标记，告知上层暂缓互联网封面匹配
            meta.SetDescription(COVER_ART_DOWNLOADING_FLAG);
        }

        return meta;
    }

    bool IsPlayable() const
    {
        return playable_;
    }

    void SetPlayable(bool playable)
    {
        playable_ = playable;
    }

    int GetItemType() const
    {
        return itemType_;
    }

    void SetItemType(int type)
    {
        itemType_ = type;
    }

    int GetType() const
    {
        return mediaType_;
    }

    void SetType(int type)
    {
        mediaType_ = type;
    }

    std::string GetDisplayName() const
    {
        if (!displayName_.empty()) {
            return displayName_;
        }
        return title_;
    }

    void SetDisplayName(const std::string& name)
    {
        displayName_ = name;
    }

    uint64_t GetUid() const
    {
        return uid_;
    }

    void SetUid(uint64_t uid)
    {
        uid_ = uid;
    }

    void SetTitle(const std::string& title)
    {
        title_ = title;
    }

    const std::string& GetTitle() const
    {
        return title_;
    }

    void SetArtist(const std::string& artist)
    {
        artistName_ = artist;
    }

    const std::string& GetArtist() const
    {
        return artistName_;
    }

    void SetAlbum(const std::string& album)
    {
        albumName_ = album;
    }

    const std::string& GetAlbum() const
    {
        return albumName_;
    }

    void SetTrackNumber(uint64_t trackNum)
    {
        trackNumber_ = trackNum;
    }

    uint64_t GetTrackNumber() const
    {
        return trackNumber_;
    }

    void SetTotalTracks(uint64_t totalTracks)
    {
        totalNumberOfTracks_ = totalTracks;
    }

    uint64_t GetTotalTracks() const
    {
        return totalNumberOfTracks_;
    }

    void SetDuration(uint64_t duration)
    {
        playingTime_ = duration;
    }

    uint64_t GetDuration() const
    {
        return playingTime_;
    }

    void SetGenre(const std::string& genre)
    {
        genre_ = genre;
    }

    const std::string& GetGenre() const
    {
        return genre_;
    }
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // AVRCP_CT_MEDIA_ITEM_H
