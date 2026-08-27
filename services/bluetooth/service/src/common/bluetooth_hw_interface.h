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

#ifndef BLUETOOTH_VENDER_INTERFACE_H
#define BLUETOOTH_VENDER_INTERFACE_H

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "base_def.h"
#include "base_observer_list.h"
#include "service_util.h"
#include "hw_bt_hwif.h"

namespace OHOS {
namespace bluetooth {
/* Product definition */
enum class HW_ECHO_PRODUCT : int32_t {
    PHONE = 0x01,
    WATCH = 0x02,
    HEADSET_TWS = 0x03,
    HEADSET_HEADPHONE = 0x04,
    HEADSET_WERABLE = 0x05,
    SPEAKER = 0x06,
    TABLET = 0x07,
    GLASSES = 0x0A,
    LAPTOP = 0x29,
    TV = 0x30,
    CAR = 0x69,
};

struct CapsuleParam {
    RawAddress device;
    std::string targetBtDevice;
    bool isAudioOutputToA2dp;
    bool isAudioOutputToSco;
    bool isMusicActive;
    bool isA2dpPlaying;
    bool isInActualcall;
    bool isAudioConnected;
    bool isA2dpActiveDeviceEqual;
    bool isHfpActiveDeviceEqual;
    bool isStreamEmptyAduio;
};
class BluetoothHwInterface final {
public:
    class SensorhubObserver {
    public:
        virtual ~SensorhubObserver() = default;
        virtual void SensorhubDevInfoCallback(uint8_t *buffer, int length) {};
        virtual void SensorhubResetCallback(uint32_t state) {};
#ifdef CONTEXTHUB_BLE_V3
        virtual void SensorhubCollaborationCallback(const std::vector<uint8_t> &notifyValue) {};
#endif
    };

    /**
     * @brief This callback is observer socket connection from stack
     */
    class SocketConnectionObserver {
    public:
        virtual ~SocketConnectionObserver() = default;
        virtual void ConnectCallback(const StackCallbackParam &param) = 0;
    };

    static BluetoothHwInterface *GetInstance(void);
    bool InitBtHwInterface(const bthwif_interface_t *bthwif);
    const bthwif_interface_t* GetBtHwInterface(void);
    void SetBtHwInterface(const bthwif_interface_t* interface);
    void AddSensorhubObserver(SensorhubObserver &observer);
    void RemoveSensorhubObserver(SensorhubObserver &observer);
    void CleanHfpScoOccupied(const RawAddress &device);
    bool InteropMatch(const uint16_t feature, const RawAddress &device);
    void KeepBleScanInConn(const std::string &pkgName, int uid,
        uint8_t transport, const STACK::RawAddress &addr);
    void ReportProfileStatus(const RawAddress &addr, uint8_t a2dpState, uint8_t hfpState);

    void AddSocketObserver(std::shared_ptr<SocketConnectionObserver> observer);
    void RemoveSocketObserver(std::shared_ptr<SocketConnectionObserver> observer);
    void ClearSocketInValidObserver();
    bool GetSha256EncryptHwHashAccount(uint8_t *outHashArray, uint32_t arrayLen);
    void UpdateA2dpOffloadCodecStatus(const RawAddress &addr);
    void HwHiechoDeviceCtrl(uint8_t *remoteAddr, uint32_t controlType,
        uint32_t controlTypeVal, uint32_t controlObject);
    void HwHiechoCombRepData(uint8_t msgType, uint8_t *data, int dataLen, uint8_t *remoteAddr);

    void HwSetAdvKey(const RawAddress& addr, const char& irk, const char& hbk,
        const uint8_t version, const char& feature);
    void HwRemoveAdvKey(const RawAddress& addr);
    bool CheckDeviceBonded(const RawAddress& addr);
    void HwGetAdvIrk(const RawAddress &addr, std::vector<uint8_t>& irk);
private:
    BluetoothHwInterface();
    ~BluetoothHwInterface() = default;

    const bthwif_interface_t *bthwInterface_ = nullptr;

    BT_DISALLOW_COPY_AND_ASSIGN(BluetoothHwInterface);
};
    void StackErrnoCallback(bt_status_t status, STACK::RawAddress *addr, BtStackErrno state);
    void HwHdapConnectCallback(STACK::RawAddress *bdAddr, bool isConnected, uint8_t featureBit,
        HdapConfigCallback config);
    void HwProfileStateCallback(STACK::RawAddress *addr, uint8_t a2dpState,
        uint8_t hfpState, std::string targetBtDevice, uint8_t a2dpServiceType);
    void HwAclDisconnReasonCb(STACK::RawAddress *addr, int reason);
    void HwGetSensorhubDevInfoCb(uint8_t *buffer, int length);
    void HwSensorhubResetCb(uint32_t state);
    void HwHiechoIndCallCb(uint8_t echoType, uint8_t *payload, uint16_t len, uint8_t *args, uint16_t argsLen);
    void HwBluetoothConnCb(const StackCallbackParam &param);
    void HwDeviceCallback(STACK::RawAddress *addr, int deviceType);
    void HwA2dpOffloadCodecConfigCallback(STACK::RawAddress *bdAddr, A2dpOffloadConfigCallback config);
    void HwHiechoDeviceAclEncryptionChangedCb(const STACK::RawAddress &remoteAddr);
    void HwReportA2dpAbnormalStatusCb(const STACK::RawAddress &remoteAddr, int32_t errCode);

    void ProcessA2dpProfileState(RawAddress addr, uint8_t a2dpState);
    void ProcessHfpProfileState(RawAddress addr, uint8_t hfpState);
    bool IsOtherA2dpDeviceInPlayingMusic(const CapsuleParam &capsuleParam);
    bool IsOtherScoDeviceInCalling(const CapsuleParam &capsuleParam);
    void ReportCapsuleInfo(const RawAddress &addr, const uint8_t profileState,
        const std::string &targetBtDevice);
    void NotifyAudioManagerShowEnableCapsule(const CapsuleParam &capsuleParam);
    void NotifyAudioManagerShowDisableCapsule(const CapsuleParam &capsuleParam);
    void PrintCapsuleParam(const CapsuleParam &param);
    void NotifyAudioManagerShowCapsule(const RawAddress &addr, const uint8_t a2dpState,
        const uint8_t hfpState, std::string &targetBtDevice, bool isEmptyAduio);
    void HwHiechoSendPerServiceCapReqData(uint8_t cap, const STACK::RawAddress &remoteAddr);
    bool HwIsSupportMlinkDevice();
    void HwHiechoSendHwAccountHashToTwsCb(const STACK::RawAddress &remoteAddr);
    void HwHiechoDtsCap(uint8_t echoType, uint8_t *args);
    void HwHiechoAudioCapReq(const std::vector<uint8_t> &payload, uint16_t len, uint8_t *args);
    void HwHiechoWearState(const RawAddress &addr, const std::vector<uint8_t> &payload,
        uint16_t len, uint8_t *args);
    void HwHiechoDtsReport(uint8_t echoType, uint8_t *payload, uint16_t len, uint8_t *args, uint16_t argsLen);
    void HwHiechoQueryConn(uint8_t echoType, const std::vector<uint8_t> &payload, int length, uint8_t *args);
    void HwHiechoAutoConnSwitch(uint8_t echoType, const std::vector<uint8_t> &payload,
        uint16_t len, uint8_t *args);
    void HwHiechoParseDescriptionRsp(uint8_t echoType, const std::vector<uint8_t> &payload,
        uint16_t length, uint8_t *args);
    void HwHiechoDeviceFoundConn(uint8_t echoType, const std::vector<uint8_t> &payload,
        int length, uint8_t *args);
    void HwHiechoDeviceFoundState(uint8_t echoType, const std::vector<uint8_t> &payload,
        int length, uint8_t *args);
    void HwHiechoIndCallBack(uint8_t echoType, uint8_t *payload, uint16_t len, uint8_t *args, uint16_t argsLen);
}  // namespace bluetooth
}  // namespace OHOS

#endif  // BLUETOOTH_VENDER_INTERFACE_H
