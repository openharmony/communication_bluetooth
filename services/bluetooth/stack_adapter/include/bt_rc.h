/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

/*
 * Stub of the removed stack layer AVRCP profile interface (bt_rc.h).
 * Types and the controller interface mirror bluedroid
 * system/include/hardware/bt_rc.h.
 */

#ifndef BT_RC_H
#define BT_RC_H

#include <cstddef>
#include <cstdint>

#include "bt_types.h"

/* Macros of the removed stack layer (bluedroid system/include/hardware/bt_rc.h). */
#define BTRC_MAX_ATTR_STR_LEN (1 << 16)
#define BTRC_UID_SIZE 8
#define BTRC_MAX_APP_SETTINGS 8
#define BTRC_MAX_FOLDER_DEPTH 4
#define BTRC_MAX_APP_ATTR_SIZE 16
#define BTRC_MAX_ELEM_ATTR_SIZE 8
#define BTRC_FEATURE_BIT_MASK_SIZE 16

/* Macros for valid scopes in get_folder_items. */
#define BTRC_SCOPE_PLAYER_LIST 0x00
#define BTRC_SCOPE_FILE_SYSTEM 0x01
#define BTRC_SCOPE_SEARCH 0x02
#define BTRC_SCOPE_NOW_PLAYING 0x03

/* Macros for supported character encoding. */
#define BTRC_CHARSET_ID_UTF8 0x006A

/* Macros for item types. */
#define BTRC_ITEM_PLAYER 0x01
#define BTRC_ITEM_FOLDER 0x02
#define BTRC_ITEM_MEDIA 0x03

/* Macros for folder types. */
#define BTRC_FOLDER_TYPE_MIXED 0x00
#define BTRC_FOLDER_TYPE_TITLES 0x01
#define BTRC_FOLDER_TYPE_ALBUMS 0x02
#define BTRC_FOLDER_TYPE_ARTISTS 0x03
#define BTRC_FOLDER_TYPE_GENRES 0x04
#define BTRC_FOLDER_TYPE_PLAYLISTS 0x05
#define BTRC_FOLDER_TYPE_YEARS 0x06

/* Macros for media types. */
#define BTRC_MEDIA_TYPE_AUDIO 0x00
#define BTRC_MEDIA_TYPE_VIDEO 0x01

/* Macros for num attributes. */
#define BTRC_NUM_ATTR_NONE 0xFF
#define BTRC_NUM_ATTR_ALL 0x00

#define BTRC_HANDLE_NONE 0xFF

typedef uint8_t BtrcUid[BTRC_UID_SIZE];

enum BtrcConnectionState {
    BTRC_CONNECTION_STATE_DISCONNECTED = 0,
    BTRC_CONNECTION_STATE_CONNECTED
};

enum BtrcRemoteFeatures {
    BTRC_FEAT_NONE = 0x00,
    BTRC_FEAT_METADATA = 0x01,
    BTRC_FEAT_ABSOLUTE_VOLUME = 0x02,
    BTRC_FEAT_BROWSE = 0x04,
    BTRC_FEAT_COVER_ARTWORK = 0x8,
};

enum BtrcPlayStatus {
    BTRC_PLAYSTATE_STOPPED = 0x00,
    BTRC_PLAYSTATE_PLAYING = 0x01,
    BTRC_PLAYSTATE_PAUSED = 0x02,
    BTRC_PLAYSTATE_FWD_SEEK = 0x03,
    BTRC_PLAYSTATE_REV_SEEK = 0x04,
    BTRC_PLAYSTATE_ERROR = 0xFF,
};

enum BtrcEventId {
    BTRC_EVT_PLAY_STATUS_CHANGED = 0x01,
    BTRC_EVT_TRACK_CHANGE = 0x02,
    BTRC_EVT_TRACK_REACHED_END = 0x03,
    BTRC_EVT_TRACK_REACHED_START = 0x04,
    BTRC_EVT_PLAY_POS_CHANGED = 0x05,
    BTRC_EVT_APP_SETTINGS_CHANGED = 0x08,
    BTRC_EVT_NOW_PLAYING_CONTENT_CHANGED = 0x09,
    BTRC_EVT_AVAL_PLAYER_CHANGE = 0x0a,
    BTRC_EVT_ADDR_PLAYER_CHANGE = 0x0b,
    BTRC_EVT_UIDS_CHANGED = 0x0c,
    BTRC_EVT_VOL_CHANGED = 0x0d,
};

enum BtrcNotificationType {
    BTRC_NOTIFICATION_TYPE_INTERIM = 0,
    BTRC_NOTIFICATION_TYPE_CHANGED = 1,
};

enum BtrcPlayerAttr {
    BTRC_PLAYER_ATTR_EQUALIZER = 0x01,
    BTRC_PLAYER_ATTR_REPEAT = 0x02,
    BTRC_PLAYER_ATTR_SHUFFLE = 0x03,
    BTRC_PLAYER_ATTR_SCAN = 0x04,
};

enum BtrcMediaAttr {
    BTRC_MEDIA_ATTR_TITLE = 0x01,
    BTRC_MEDIA_ATTR_ARTIST = 0x02,
    BTRC_MEDIA_ATTR_ALBUM = 0x03,
    BTRC_MEDIA_ATTR_TRACK_NUM = 0x04,
    BTRC_MEDIA_ATTR_NUM_TRACKS = 0x05,
    BTRC_MEDIA_ATTR_GENRE = 0x06,
    BTRC_MEDIA_ATTR_PLAYING_TIME = 0x07,
};

enum BtrcPlayerRepeatVal {
    BTRC_PLAYER_VAL_OFF_REPEAT = 0x01,
    BTRC_PLAYER_VAL_SINGLE_REPEAT = 0x02,
    BTRC_PLAYER_VAL_ALL_REPEAT = 0x03,
    BTRC_PLAYER_VAL_GROUP_REPEAT = 0x04
};

enum BtrcPlayerShuffleVal {
    BTRC_PLAYER_VAL_OFF_SHUFFLE = 0x01,
    BTRC_PLAYER_VAL_ALL_SHUFFLE = 0x02,
    BTRC_PLAYER_VAL_GROUP_SHUFFLE = 0x03
};

enum BtrcStatus {
    BTRC_STS_BAD_CMD = 0x00,
    BTRC_STS_BAD_PARAM = 0x01,
    BTRC_STS_NOT_FOUND = 0x02,
    BTRC_STS_INTERNAL_ERR = 0x03,
    BTRC_STS_NO_ERROR = 0x04,
    BTRC_STS_UID_CHANGED = 0x05,
    BTRC_STS_RESERVED = 0x06,
    BTRC_STS_INV_DIRN = 0x07,
    BTRC_STS_INV_DIRECTORY = 0x08,
    BTRC_STS_INV_ITEM = 0x09,
    BTRC_STS_INV_SCOPE = 0x0a,
    BTRC_STS_INV_RANGE = 0x0b,
    BTRC_STS_DIRECTORY = 0x0c,
    BTRC_STS_MEDIA_IN_USE = 0x0d,
    BTRC_STS_PLAY_LIST_FULL = 0x0e,
    BTRC_STS_SRCH_NOT_SPRTD = 0x0f,
    BTRC_STS_SRCH_IN_PROG = 0x10,
    BTRC_STS_INV_PLAYER = 0x11,
    BTRC_STS_PLAY_NOT_BROW = 0x12,
    BTRC_STS_PLAY_NOT_ADDR = 0x13,
    BTRC_STS_INV_RESULTS = 0x14,
    BTRC_STS_NO_AVBL_PLAY = 0x15,
    BTRC_STS_ADDR_PLAY_CHGD = 0x16,
};

struct BtrcPlayerSettings {
    uint8_t numAttr;
    uint8_t attrIds[BTRC_MAX_APP_SETTINGS];
    uint8_t attrValues[BTRC_MAX_APP_SETTINGS];
};

struct BtrcPlayerAppExtAttrVal {
    uint8_t val;
    uint16_t charsetId;
    uint16_t strLen;
    uint8_t *pStr;
};

struct BtrcPlayerAppExtAttr {
    uint8_t attrId;
    uint16_t charsetId;
    uint16_t strLen;
    uint8_t *pStr;
    uint8_t numVal;
    BtrcPlayerAppExtAttrVal extAttrVal[BTRC_MAX_APP_ATTR_SIZE];
};

struct BtrcPlayerAppAttr {
    uint8_t attrId;
    uint8_t numVal;
    uint8_t attrVal[BTRC_MAX_APP_ATTR_SIZE];
};

struct BtrcElementAttrVal {
    uint32_t attrId;
    uint8_t text[BTRC_MAX_ATTR_STR_LEN];
};

struct BtrcItemPlayer {
    uint16_t playerId;
    uint8_t majorType;
    uint32_t subType;
    uint8_t playStatus;
    uint8_t features[BTRC_FEATURE_BIT_MASK_SIZE];
    uint16_t charsetId;
    uint8_t name[BTRC_MAX_ATTR_STR_LEN];
};

struct BtrcItemFolder {
    uint8_t uid[BTRC_UID_SIZE];
    uint8_t type;
    uint8_t playable;
    uint16_t charsetId;
    uint8_t name[BTRC_MAX_ATTR_STR_LEN];
};

struct BtrcItemMedia {
    uint8_t uid[BTRC_UID_SIZE];
    uint8_t type;
    uint16_t charsetId;
    uint8_t name[BTRC_MAX_ATTR_STR_LEN];
    int numAttrs;
    BtrcElementAttrVal *pAttrs;
};

struct BtrcFolderItems {
    uint8_t itemType;
    union {
        BtrcItemPlayer player;
        BtrcItemFolder folder;
        BtrcItemMedia media;
    };
};

/* AVRCP controller callbacks dispatched by the removed stack layer; the
 * signatures mirror bluedroid system/include/hardware/bt_rc.h. */
typedef void (*BtrcPassthroughRspCallback)(const RawAddress &bdAddr, int id, int keyState);
typedef void (*BtrcGroupnavigationRspCallback)(int id, int keyState);
typedef void (*BtrcConnectionStateCallback)(bool rcConnect, bool btConnect,
                                               const RawAddress &bdAddr);
typedef void (*BtrcCtrlGetrcfeaturesCallback)(const RawAddress &bdAddr, int features);
typedef void (*BtrcCtrlSetabsvolCmdCallback)(const RawAddress &bdAddr, uint8_t absVol,
                                                 uint8_t label);
typedef void (*BtrcCtrlRegisternotificationAbsVolCallback)(const RawAddress &bdAddr,
                                                                uint8_t label);
typedef void (*BtrcCtrlSetplayerapplicationsettingRspCallback)(const RawAddress &bdAddr,
                                                                   uint8_t accepted);
typedef void (*BtrcCtrlPlayerapplicationsettingCallback)(const RawAddress &bdAddr,
                                                            uint8_t numAttr,
                                                            BtrcPlayerAppAttr *appAttrs,
                                                            uint8_t numExtAttr,
                                                            BtrcPlayerAppExtAttr *extAttrs);
typedef void (*BtrcCtrlPlayerapplicationsettingChangedCallback)(
    const RawAddress &bdAddr, const BtrcPlayerSettings &vals);
typedef void (*BtrcCtrlTrackChangedCallback)(const RawAddress &bdAddr, uint8_t numAttr,
                                                 BtrcElementAttrVal *pAttrs);
typedef void (*BtrcCtrlPlayPositionChangedCallback)(const RawAddress &bdAddr,
                                                         uint32_t songLen, uint32_t songPos);
typedef void (*BtrcCtrlPlayStatusChangedCallback)(const RawAddress &bdAddr,
                                                       BtrcPlayStatus playStatus);
typedef void (*BtrcCtrlGetFolderItemsCallback)(const RawAddress &bdAddr,
                                                    BtrcStatus status,
                                                    const BtrcFolderItems *folderItems,
                                                    uint8_t count);
typedef void (*BtrcCtrlChangePathCallback)(const RawAddress &bdAddr, uint32_t count);
typedef void (*BtrcCtrlSetBrowsedPlayerCallback)(const RawAddress &bdAddr,
                                                      uint8_t numItems, uint8_t depth);
typedef void (*BtrcCtrlSetAddressedPlayerCallback)(const RawAddress &bdAddr,
                                                        uint8_t status);
typedef void (*BtrcCtrlAddressedPlayerChangedCallback)(const RawAddress &bdAddr,
                                                            uint16_t id);
typedef void (*BtrcCtrlNowPlayingContentsChangedCallback)(const RawAddress &bdAddr);
typedef void (*BtrcCtrlAvailablePlayerChangedCallback)(const RawAddress &bdAddr);
typedef void (*BtrcCtrlGetCoverArtPsmCallback)(const RawAddress &bdAddr,
                                                     const uint16_t psm);

/* AVRCP controller callback structure registered via BtrcCtrlInterface::init. */
struct BtrcCtrlCallbacks {
    size_t size;
    BtrcPassthroughRspCallback passthroughRspCb;
    BtrcGroupnavigationRspCallback groupnavigationRspCb;
    BtrcConnectionStateCallback connectionStateCb;
    BtrcCtrlGetrcfeaturesCallback getrcfeaturesCb;
    BtrcCtrlSetplayerapplicationsettingRspCallback setplayerappsettingRspCb;
    BtrcCtrlPlayerapplicationsettingCallback playerApplicationSettingCb;
    BtrcCtrlPlayerapplicationsettingChangedCallback playerApplicationSettingChangedCb;
    BtrcCtrlSetabsvolCmdCallback setabsvolCmdCb;
    BtrcCtrlRegisternotificationAbsVolCallback registernotificationAbsvolCb;
    BtrcCtrlTrackChangedCallback trackChangedCb;
    BtrcCtrlPlayPositionChangedCallback playPositionChangedCb;
    BtrcCtrlPlayStatusChangedCallback playStatusChangedCb;
    BtrcCtrlGetFolderItemsCallback getFolderItemsCb;
    BtrcCtrlChangePathCallback changeFolderPathCb;
    BtrcCtrlSetBrowsedPlayerCallback setBrowsedPlayerCb;
    BtrcCtrlSetAddressedPlayerCallback setAddressedPlayerCb;
    BtrcCtrlAddressedPlayerChangedCallback addressedPlayerChangedCb;
    BtrcCtrlNowPlayingContentsChangedCallback nowPlayingContentsChangedCb;
    BtrcCtrlAvailablePlayerChangedCallback availablePlayerChangedCb;
    BtrcCtrlGetCoverArtPsmCallback getCoverArtPsmCb;
};

/* AVRCP controller interface consumed by avrcp_ct_service.cpp; layout mirrors
 * bluedroid system/include/hardware/bt_rc.h. */
struct BtrcCtrlInterface {
    size_t size;
    BtStackStatus (*init)(BtrcCtrlCallbacks *callbacks);
    BtStackStatus (*sendPassThroughCmd)(const RawAddress &bdAddr, uint8_t keyCode,
                                         uint8_t keyState);
    BtStackStatus (*sendGroupNavigationCmd)(const RawAddress &bdAddr, uint8_t keyCode,
                                             uint8_t keyState);
    BtStackStatus (*setPlayerAppSettingCmd)(const RawAddress &bdAddr, uint8_t numAttrib,
                                              uint8_t *attribIds, uint8_t *attribVals);
    BtStackStatus (*playItemCmd)(const RawAddress &bdAddr, uint8_t scope, uint8_t *uid,
                                 uint16_t uidCounter);
    BtStackStatus (*getCurrentMetadataCmd)(const RawAddress &bdAddr);
    BtStackStatus (*getPlaybackStateCmd)(const RawAddress &bdAddr);
    BtStackStatus (*getNowPlayingListCmd)(const RawAddress &bdAddr, uint32_t start,
                                            uint32_t end);
    BtStackStatus (*getFolderListCmd)(const RawAddress &bdAddr, uint32_t start, uint32_t end);
    BtStackStatus (*getPlayerListCmd)(const RawAddress &bdAddr, uint32_t start, uint32_t end);
    BtStackStatus (*changeFolderPathCmd)(const RawAddress &bdAddr, uint8_t direction,
                                          uint8_t *uid);
    BtStackStatus (*setBrowsedPlayerCmd)(const RawAddress &bdAddr, uint16_t playerId);
    BtStackStatus (*setAddressedPlayerCmd)(const RawAddress &bdAddr, uint16_t playerId);
    BtStackStatus (*setVolumeRsp)(const RawAddress &bdAddr, uint8_t absVol, uint8_t label);
    BtStackStatus (*registerAbsVolRsp)(const RawAddress &bdAddr,
                                        BtrcNotificationType rspType, uint8_t absVol,
                                        uint8_t label);
    void (*cleanup)(void);
};

#endif  // BT_RC_H
