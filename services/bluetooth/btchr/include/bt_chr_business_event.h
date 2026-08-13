/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef BT_CHR_AUDIO_EVENT_H
#define BT_CHR_AUDIO_EVENT_H
#include "bt_chr_base.h"

namespace OHOS {
namespace bluetooth {

class BtChrBusinessEvent {
public:
    static BtChrBusinessEvent &GetInstance();
    void SetOffloadFlag(int offloadFlag);
    void SetVolumeParams(int volumeType, int volumeValue);
    void SetAbsolutVolumeParams(int isAbsolutVolume, int absolutVolumeStatus);
    void SetAudioDeviceParams(std::string audioDevicePkg, int activeDeviceResult);
    void SetAudioStartTime(std::string audioStartTime);
    int IsAbsolutVolume();
    int GetAbsolutVolumeStatus();
    std::string GetAudioDevicePkg();
    int GetActiveDeviceResult();
    std::string GetAudioStartTime();
    int GetOffloadFlag();
    int GetVolumeType();
    int GetVolumeValue();
    void SavePkgName(const std::string &pkgNames);
    std::string GetSavedPkgName();
public:
    BtChrBusinessEvent();
    std::string audioStartTime_ = "";
    std::string audioDevicePkg_ = "";
    int offloadFlag_ = 0; // 0: offload, 1: no offload
    int activeDeviceResult_ = 0;
    int isAbsolutVolume_ = 1; // 1 means support, 2 means not support
    int absolutVolumeStatus_ = 1; // 0 means close, 1 means open
    int volumeType_ = 0; // 0 means recive, 1 means send
    int volumeValue_ = 0;

    int32_t btConnectScene = -1;
    int32_t isPassive = -1;
    std::string pkgName = "";
    std::string aclConnectTime = "";
    std::string aclConnectedTime = "";
    std::string aclDisconnectedTime = "";
    int32_t btMtuInitiator = -1;
    int32_t btMtuResult = -1;
    int32_t encryptionStatus = -1;
    std::string encryptionEndTime = "";
    int32_t sdpInitiator = -1;
    int32_t sdpErrorReason = -1;
    int32_t gattServiceInitiator = -1;
    int32_t gattErrorReason = -1;
    int32_t needA2dp = 0;
    int32_t a2dpInitiator = -1;
    int32_t a2dpStatus = -1;
    int32_t a2dpErrorReason = -1;
    std::string a2dpConnectTime = "";
    std::string a2dpConnectedTime = "";
    std::string a2dpDisconnectedTime = "";
    int32_t needHfp = 0;
    int32_t hfpInitiator = -1;
    int32_t hfpStatus = -1;
    int32_t hfpErrorReason = -1;
    std::string hfpConnectTime = "";
    std::string hfpConnectedTime = "";
    std::string hfpDisconnectedTime = "";
    int32_t hidStatus = -1;
    int32_t hidErrorReason = -1;
    std::string hidConnectTime = "";
    std::string hidConnectedTime = "";
    std::string hidDisconnectedTime = "";
    int32_t gattClientStatus = -1;
    int32_t gattServiceStatus = -1;
    int32_t gattConnectErrorReason = -1;
    std::string gattConnectTime = "";
    std::string gattConnectedTime = "";
    std::string gattDisconnectedTime = "";
    int32_t smpType = -1;
    int32_t smpStatus = -1;
    std::string smpConnectTime = "";
    std::string smpConnectedTime = "";
    std::string smpDisconnectedTime = "";
    int32_t btPairStatus = -1;
    int32_t disconnectReason = -1;
    int32_t disconnectRssi = 0;
 
    int32_t connectStatus = -1;
    int32_t removePairResult = -1;
    int32_t removePairInitiator = -1;
    int32_t removePairScence = -1;
    std::string removePairTime = "";
    int32_t deleteLinkkeyResult = -1;
    std::string deleteLinkkeyTime = "";
    int32_t disconnectResult = -1;
    int32_t disconnectErrorCode = -1;
    int32_t failErrorCode = 0;
private:
    std::string savedPkgNames_ = "";
};
 
bool BtChrBusinessEventHandler(const std::string& eventName, const std::string& peerAddr,
    const std::string& paramName, int32_t paramValue);
bool BtChrBusinessEventHandler(const std::string& eventName, const std::string& peerAddr,
    const std::string& paramName, const std::string& paramValue);
void BtChrAssignIntParam(const std::string& paramName, const int32_t paramValue,
    BtChrBusinessEvent *btChrBusinessEvent);
void BtChrAssignStringParam(const std::string& paramName, const std::string& paramValue,
    BtChrBusinessEvent *btChrBusinessEvent);
bool BtChrBusinessFillAllParam(const std::string& peerAddr, const std::string& eventName,
    const std::string& paramName);
bool BtChrFillWatchConnectParam(const std::string& peerAddr, const std::string& eventName,
    const std::string& paramName);
void BtChrFillWatchConnectParams(const std::string& peerAddr, const std::string& eventName,
    const BtChrBusinessEvent btChrBusinessEvent);
bool BtChrFillWatchRemovePairParam(const std::string& peerAddr, const std::string& eventName,
    const std::string& paramName);
}  // namespace bluetooth
}  // namespace OHOS
#endif /* BT_CHR_AUDIO_EVENT_H */
