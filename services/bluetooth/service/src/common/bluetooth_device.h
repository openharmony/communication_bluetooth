/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

/**
 * @addtogroup Bluetooth
 * @{
 *
 * @brief Defines remote device , including common functions.
 *
 */

/**
 * @file classic_remote_device.h
 *
 * @brief Classic remote device common functions.
 *
 */

#ifndef BLUETOOTH_DEVICE_H
#define BLUETOOTH_DEVICE_H

#include <memory>
#include <string>
#include <vector>
#include <map>

#include "base_def.h"
#include "bt_def.h"
#include "bt_uuid.h"
#include "classic_defs.h"
#include "bluetooth.h"
#include "bt_battery_info.h"

namespace OHOS {
namespace bluetooth {

/* Cloud bond state constants */
constexpr int32_t CLOUD_BOND_NONE = 0;
constexpr int32_t CLOUD_BOND_BONDING = 1;
constexpr int32_t CLOUD_BOND_BONDED = 2;
constexpr int32_t CLOUD_BOND_REMOVING = 3;
constexpr int32_t CLOUD_BOND_CREATE_BOND = 4;

/**
 * @brief Represents remote device.
 *
 */
class BluetoothDevice {
public:
    static const int AUDIO_VIDEO_CAR_AUDIO = 0x0420;
    static const int PERIPHERAL_KEYBOARD = 0x0540;
    static const int PERIPHERAL_POINTING_DEVICE = 0x0580;
    static const int PERIPHERAL_KEYBOARD_POINTING = 0x05C0;
    static const int AUDIO_VIDEO_HEADPHONES = 0x240418;
    static const int WEARABLE_GLASSES = 0x0714;
    static const int WEARABLE_WRIST_WATCH = 0x280704;
    static const int AUDIO_VIDEO_LOUDSPEAKER = 0x0414;
    static const int AUDIO_VIDEO_HANDSFREE = 0x0408;
    static const int MAJOR_AUDIO_VIDEO_HEADPHONES = 0x0418;
    static const int MAJOR_WEARABLE_WRIST_WATCH = 0x0704;
    static const int AUDIO_VIDEO_WEARABLE_HEADSET = 0x0404;
    static const int DEVICE_BITMASK = 0x1FFC;

    /**
     * @brief A constructor used to create a <b>BluetoothDevice</b> instance.
     *
     */
    BluetoothDevice();

    /**
     * @brief A constructor used to create a <b>BluetoothDevice</b> instance.
     *
     * @param addr Device address create an <b>BluetoothDevice</b> instance.
     */
    explicit BluetoothDevice(const std::string &addr);

    /**
     * @brief A destructor used to delete the <b>BluetoothDevice</b> instance.
     *
     */
    ~BluetoothDevice();

    /**
     * @brief Get device type.
     *
     * @return Returns device type.
     */
    int GetDeviceType() const;

    /**
     * @brief Get device address.
     *
     * @return Returns device address.
     */
    std::string GetAddress() const;

    /**
     * @brief Get remote device name.
     *
     * @param alias Indicates whether to obtain the device alias.
     * @return Returns remote device name.
     */
    std::string GetRemoteName(bool alias = true) const;

    /**
     * @brief Get device alias name.
     *
     * @return Returns device alias name.
     */
    std::string GetAliasName() const;

    /**
     * @brief Get device class.
     *
     * @return Returns device class.
     */
    int GetDeviceClass() const;

    /**
     * @brief Get rssi.
     *
     * @return Returns rssi.
     */
    int GetRssi() const;

    int GetWearDetectionSupportValue() const;

    /**
     * @brief Get device uuids.
     *
     * @return Returns device uuids vector.
     */
    std::vector<Uuid> GetDeviceUuids() const;

    /**
     * @brief Get acl connection handle.
     *
     * @return Returns acl connection handle;
     */
    int GetConnectionHandle() const;

    /**
     * @brief Get paired status.
     *
     * @return Returns paired status.
     */
    int GetPairedStatus() const;

    /**
     * @brief Get link key.
     *
     * @return Returns link key.
     */
    std::vector<uint8_t> GetLinkKey() const;

    /**
     * @brief Get link key type.
     *
     * @return Returns link key type.
     */
    int GetLinkKeyType() const;

    /**
     * @brief Get IO capability.
     *
     * @return Returns IO capability.
     */
    int GetIoCapability() const;

    /**
     * @brief Get device flags.
     *
     * @return Returns flags.
     */
    uint8_t GetFlags() const;

    /**
     * @brief Check if remote device already paired.
     *
     * @return Returns <b>true</b> if remote device already paired;
     *         returns <b>false</b> if remote device not paired.
     */
    bool IsPaired() const;

    /**
     * @brief Get pair confirm state.
     *
     * @return Returns pair confirm state.
     */
    int GetPairConfirmState() const;

    /**
     * @brief Get pair confirm type.
     *
     * @return Returns pair confirm type.
     */
    int GetPairConfirmType() const;

    /**
     * @brief set rssi value.
     *
     * @param rssi Rssi value.
     */
    void SetRssi(int rssi);

    void SetWearDetectionSupportValue(int supportValue);

    /**
     * @brief Set device type.
     *
     * @param type Device type.
     */
    void SetDeviceType(int type);

    /**
     * @brief Set device address.
     *
     * @param addr Device address.
     */
    void SetAddress(const std::string &addr);

    /**
     * @brief Set remote device name.
     *
     * @param name Device name.
     */
    void SetRemoteName(const std::string &name);

    /**
     * @brief Set alias name.
     *
     * @param name Device alias name.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    bool SetAliasName(const std::string &name);

    /**
     * @brief Set device class.
     *
     * @param deviceClass Device class.
     */
    void SetDeviceClass(int deviceClass);

    /**
     * @brief Set bonded from local.
     *
     * @param flag Advertiser flag.
     */
    void SetBondedFromLocal(bool flag);

    /**
     * @brief Active pairing connection for dialog control.
     *
     * @param dialogControlActivePair Whether to Active pairing connection.
     */
    void DialogControlSetActivePair(bool dialogControlActivePair);

    /**
     * @brief Set acl connect state.
     *
     * @param connectState Acl connect state.
     */
    void SetAclConnectState(int connectState);

    /**
     * @brief Set device uuids.
     *
     * @param uuids Device uuids.
     */
    void SetDeviceUuids(const std::vector<Uuid> &uuids);

    /**
     * @brief Set acl connection handle.
     *
     * @param handle Acl connection handle.
     */
    void SetConnectionHandle(int handle);

    /**
     * @brief Set link key.
     *
     * @param linkKey Link key.
     */
    void SetLinkKey(const std::vector<uint8_t> &linkKey);

    /**
     * @brief Set link key type.
     *
     * @param linkKey Link key type.
     */
    void SetLinkKeyType(int linkKeyType);

    /**
     * @brief Set IO capability.
     *
     * @param io IO capability
     */
    void SetIoCapability(int io);

    /**
     * @brief Set device flags.
     *
     * @param flags Device flags.
     */
    void SetFlags(uint8_t flags);

    /**
     * @brief Set device manufacturer-specific data.
     *
     * @param data Manufacturer-specific data.
     */
    void SetManufacturerSpecificData(const std::vector<uint8_t> &data);

    /**
     * @brief Get device manufacturer-specific data.
     *
     * @return Returns manufacturer-specific data.
     */
    const std::vector<uint8_t>& GetManufacturerSpecificData() const;

    /**
     * @brief Set device Tx power.
     *
     * @param power Device Tx power.
     */
    void SetTxPower(uint8_t power);

    /**
     * @brief Set device URI.
     *
     * @param uri Device URI.
     */
    void SetURI(const std::string &uri);

    /**
     * @brief Set pair confirm state.
     *
     * @param state Pair confirm state.
     */
    void SetPairConfirmState(int state);

    /**
     * @brief Set pair confirm type.
     *
     * @param type Pair confirm type.
     */
    void SetPairConfirmType(int type);

    /**
     * @brief Check if device acl connected.
     *
     * @return Returns <b>true</b> if device acl connected;
     *         returns <b>false</b> if device does not acl connect.
     */
    bool IsAclConnected() const;

    /**
     * @brief Check if device acl encrypted.
     *
     * @return Returns <b>true</b> if device acl encrypted;
     *         returns <b>false</b> if device does not acl encrypted.
     */
    bool IsAclEncrypted() const;

    /**
     * @brief Check if device was bonded from local.
     *
     * @return Returns <b>true</b> if device was bonded from local;
     *         returns <b>false</b> if device was not bonded from local.
     */
    bool IsBondedFromLocal() const;

    /**
     * @brief Check if active pairing connection for dialog control.
     *
     * @return Returns <b>true</b> if device was active pair;
     *         returns <b>false</b> if device was not active pair.
     */
    bool DialogControlIsActivePair() const;

    /**
     * @brief Delete link key.
     *
     */
    void DeleteLinkKey();

    /**
     * @brief Set paired status.
     *
     * @param status Paired status.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    bool SetPairedStatus(int status);

    /**
     * @brief Save paired device info.
     *
     */
    void SavePairedDeviceInfo();

    /**
     * @brief Remote paired device info.
     *
     */
    void RemotePairedInfo();

    void SetNameNeedGet(bool nameUnknowned);
    bool GetNameNeedGet() const;
    bool CheckCod(uint32_t cod) const;
    void SetPasskey(uint32_t passKey);
    int GetPasskey() const;
    void SetSspVariant(BtSspVariant pairingVariant);
    int GetSspVariant() const;
    bool IsPairNone() const;
    void SetDeviceAbsVolumeAbility(int32_t ability);
    int32_t GetDeviceAbsVolumeAbility();
    int GetMajorMinorClass() const;
    int32_t GetCustomType() const;
    void SetCustomType(int32_t customType);
    BatteryDetailInfo GetDeviceBatteryInfo() const;
    void SetDeviceBatteryInfo(const BatteryDetailInfo &batteryInfo);
    void ClearDeviceBatteryInfo();
    void SetDevProactiveDisconnectFlag(uint32_t profileId, bool isProactive);
    bool IsDevProactiveDisconnect(uint32_t profileId) const;
    void SetVendorId(int32_t vendorId);
    int32_t GetVendorId();
    void SetProductId(int32_t productId);
    int32_t GetProductId();
    void SetVersion(int32_t version);
    int32_t GetVersion();
    void SetModelId(int32_t modelId);
    int32_t GetModelId();
    void SetNewModelId(std::string newModelId);
    std::string GetNewModelId();
    void SetIcon(int32_t icon);
    int32_t GetIcon();
    void SetAutoConnSwitch(int32_t autoConnSwitch);
    int32_t GetAutoConnSwitch();
    void SetTimeStamp(int32_t timeStamp);
    int32_t GetTimeStamp();
    bool IsNeedDelayConnect();
    void SetNeedDelayConnectFlag(bool isNeedDelayConnect);
    void SetHwTalkBandBandMode(int bandMode);
    int GetHwRemoteDeviceType() const;
    void SetHwRemoteDeviceType(int hwRemoteDeviceType);
    bool isBandMode() const;
    bool isBandDevice() const;
    void DialogControlAddConnectingProfileCount();
    void DialogControlRemoveConnectingProfileCount(int state);
    void DialogControlClearConnectingProfileCount();
    int DialogControlConvertCause(int cause);
    int64_t GetLastConnectionTime() const;
    void SetConnectionTime(int64_t connectionTime);

    bool IsCloudDevice() const;
    void SetCloudDevice(bool isCloudDevice);

    int32_t GetCloudBondState() const;
    void SetCloudBondState(int32_t cloudBondState);

    void SetToken(const std::vector<uint8_t> &tokens);
    void SetIsConnectFromLocal(bool localConnect);
    int IsConnectFromLocalChangeCause(int cause) const;
    std::vector<uint8_t> GetToken();
private:
    bool bondedFromLocal_ {false};
    bool dialogControlActivePair_ {false};
    bool nameNeedGet_ {};
    bool aclConnected_ {};
    bool isNeedDelayConnect_ {};
    bool connectFromLocal_ {false};
    uint8_t flags_ {};
    uint8_t txPower_ {};

    int hwRemoteDeviceType_ {};
    int cod_ {};
    int rssi_ {};
    int dialogControlConnectingProfileCount_ {0};
    int deviceType_ {DEVICE_TYPE_BREDR};
    int connectionHandle_ {};
    int pairState_ {};
    int pairConfirmType_ {PAIR_CONFIRM_TYPE_INVALID};
    int pairConfirmState_ {PAIR_CONFIRM_STATE_INVALID};
    int linkKeyType_ {PAIR_INVALID_LINK_KEY_TYPE};
    int ioCapability_ {};
    int batteryLevel_ {};
    int passkey_ {};
    int pairingVariant_ {};
    int wearDetectionSupportValue_ {};
    int32_t absVolumeAbility_ {DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT};
    int32_t customType_ {INVALID_VALUE};
    int vendorId_ {};
    int productId_ {};
    int32_t version_ {INVALID_VALUE};
    int32_t modelId_ {INVALID_VALUE};
    std::string newModelId_ {""};
    int32_t icon_ {INVALID_VALUE};
    int32_t autoConnSwitch_ {INVALID_VALUE};
    int32_t timeStamp_ {INVALID_VALUE};
    int64_t connectionTime_ {INVALID_VALUE};

    std::string macAddr_ {INVALID_MAC_ADDRESS};
    std::string deviceName_ {INVALID_NAME};
    std::string aliasName_ {INVALID_NAME};
    std::string uri_ {};
    std::vector<uint8_t> manuSpecData_ {};
    std::vector<uint8_t> linkKey_ {};
    std::vector<Uuid> uuids_ {};
    BatteryDetailInfo batteryInfo_;
    int headsetBandMode_ {TalkBandMode::VALUE_OF_NOT_TALKBAND_DEVICE};
    bool isCloudDevice_ {false};
    int32_t cloudBondedState_ {CLOUD_BOND_NONE};
    std::vector<uint8_t> token_ {};
    // Whether proactive disconnect device.
    std::map<uint32_t, bool> proactiveDisconnectFlag_ = {
        {PROFILE_ID_A2DP_SRC, false},
        {PROFILE_ID_HFP_AG, false},
        {PROFILE_ID_HID_HOST, false},
        {PROFILE_ID_MAP_MSE, false},
        {PROFILE_ID_PAN, false},
        {PROFILE_ID_PBAP_PSE, false}
    };
    mutable std::mutex uuidsMutex_{};

    BT_DISALLOW_COPY_AND_ASSIGN(BluetoothDevice);
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // BLUETOOTH_DEVICE_H
