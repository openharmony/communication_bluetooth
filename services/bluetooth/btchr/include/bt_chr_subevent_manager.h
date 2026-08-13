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

#ifndef BT_CHR_SUBEVENT_MANAGER_H
#define BT_CHR_SUBEVENT_MANAGER_H
#include <cstddef>
#include <iostream>

namespace OHOS {
namespace bluetooth {

constexpr int32_t CHR_FUSION_SIGHUB_ANT_MAX_NUM = 3;

struct BtChrDeviceInfo {
    std::string btDeviceName;
    int btDeviceRssi;
    int btDeviceClass;
    int32_t linkType;
    int32_t needA2dp;
    int32_t a2dpStatus;
    int32_t needHfp;
    int32_t hfpStatus;
    int32_t needHid;
    int32_t hidStatus;
    int32_t btVersion;
    int32_t btSubVersion;
    int32_t chipCompanyId;
};

enum BtChrPairType {
    PAIR_TYPE_USER_CONNECT = 0,
    PAIR_TYPE_USER_PAIR,
    PAIR_TYPE_BLUETOOTH_SERVICE,
    PAIR_TYPE_REMOTE_DEVICE,

    PAIR_TYPE_MAX_VALUE = 0xF,
};

/*******************************************************************************
 *  Functions
 ******************************************************************************/
void BtChrSetLocalDeviceName(char *deviceName);
bool GetDeviceInfoFromMap(const std::string& peerAddr, BtChrDeviceInfo& deviceInfo);
void BtChrUpdateLinkType(const std::string& peerAddr, int32_t linkType);
void BtChrUpdateA2dpStatus(const std::string& peerAddr, int32_t status);
void BtChrUpdateHfpStatus(const std::string& peerAddr, int32_t status);
void BtChrUpdateHidStatus(const std::string& peerAddr, int32_t status);
void BtChrUpdateSupportA2dpStatus(const std::string& peerAddr, int32_t status);
void BtChrUpdateSupportHfpStatus(const std::string& peerAddr, int32_t status);
void BtChrUpdateSupportHidStatus(const std::string& peerAddr, int32_t status);
void BtChrUpdateDeviceInfo(const std::string& peerAddr, const std::string& deviceName, int rssi, int deviceClass);
void BtChrUpdateChipInfo(const std::string& peerAddr, int btVersion, int btSubVersion, int chipCompanyId);
void BtChrAddConnectedDevice(const std::string& peerAddr);
void BtChrDelConnectedDevice(const std::string& peerAddr, int32_t status = 0);
std::string BtChrGetBtCommonJsonStr(const std::string& peerAddr);
std::string BtChrGetConnectedDeviceJsonStr();
void BtChrAddAuthResultInfo(const std::string& peerAddr, int32_t authStatus);
std::string BtChrGetAuthResultJsonStr(const std::string& peerAddr);
void BtChrSubEventReleaseAll(const std::vector<std::string> &notClearAddrs);
std::string BtChrGetConnSceneJsonStr(const std::string& peerAddr);
void BtChrAddConnSceneInfo(const std::string& peerAddr, BtChrPairType pairType, const std::string& pairApp,
    int32_t secureConnectionFlag);
void BtChrAddCallingPackageName(const std::string& peerAddr, const std::string& packageName);
std::string BtChrGetChipInfoJsonStr(const std::string& peerAddr, const std::string& eventKey);
void BtChrUpdateGoodChannelNum(const std::string& peerAddr, int goodChannelsNum);
void BtChrUpdateAntennaStatus(const std::string& peerAddr, const std::string &antennaStatus);
void BtChrUpdatePowerLevel(const std::string &peerAddr, std::string powerLevel);
void BtChrUpdateTransPerform(const std::string &peerAddr, std::string transPerform);
void BtChrDelAudioChipInfo(const std::string& peerAddr);
void AddPowerLevelDuration(uint8_t powerLevel, std::chrono::milliseconds duration);
void AddPowerLevelTimes(uint8_t powerLevel);
std::string GetPowerLevelStatistics();
void BtChrClearAudioAntSwitchState();
void BtChrUpdateAntennaSwitchStatus(uint8_t *antGroupIdx, uint8_t *configStatus, uint32_t length);
void BtChrUpdateAudioAntennaSwitchStatus(uint16_t *antGroupIdx, uint16_t *configStatus, uint32_t length);

}  // namespace bluetooth
}  // namespace OHOS
#endif /* BT_CHR_SUBEVENT_MANAGER_H */
