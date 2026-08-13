/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_device"
#endif

#include "bluetooth_device.h"

#include <cstring>
#include "log.h"
#include "common_util.h"
#include "log_utils.h"

namespace OHOS {
namespace bluetooth {
BluetoothDevice::BluetoothDevice()
{
    cod_ = INVALID_VALUE;
    rssi_ = INVALID_VALUE;
    connectionHandle_ = INVALID_VALUE;
    pairState_ = PAIR_NONE;
    ioCapability_ = INVALID_VALUE;
    batteryLevel_ = INVALID_VALUE;
    passkey_ = INVALID_VALUE;
    pairingVariant_ = INVALID_VALUE;
}

BluetoothDevice::BluetoothDevice(const std::string &addr) : macAddr_(addr)
{
    HILOGD("addr = %{public}s", GetEncryptAddr(addr).c_str());
    cod_ = INVALID_VALUE;
    rssi_ = INVALID_VALUE;
    connectionHandle_ = INVALID_VALUE;
    pairState_ = PAIR_NONE;
    ioCapability_ = INVALID_VALUE;
    batteryLevel_ = INVALID_VALUE;
    passkey_ = INVALID_VALUE;
    pairingVariant_ = INVALID_VALUE;
}

static std::mutex g_dialogControlMutex;

BluetoothDevice::~BluetoothDevice()
{
}

std::string BluetoothDevice::GetAddress() const
{
    HILOGD("addr = %{public}s", GetEncryptAddr(macAddr_).c_str());

    return macAddr_;
}

void BluetoothDevice::SetAddress(const std::string &addr)
{
    HILOGI("addr = %{public}s", GetEncryptAddr(addr).c_str());

    macAddr_ = addr;
}

std::string BluetoothDevice::GetRemoteName(bool alias) const
{
    HILOGD("name = %{public}s", GET_ENCRYPT_DEVICE_NAME(deviceName_));

    if (aliasName_ != INVALID_NAME && alias) {
        return aliasName_;
    }

    return deviceName_;
}

void BluetoothDevice::SetRemoteName(const std::string &name)
{
    HILOGD("[BluetoothDevice] name = %{public}s", GET_ENCRYPT_DEVICE_NAME(name));

    if (name.length() >= MAX_REMOTE_BT_NAME_LEN) {
        deviceName_ = name.substr(0, MAX_REMOTE_BT_NAME_LEN);
    } else {
        deviceName_ = name;
    }
}

std::string BluetoothDevice::GetAliasName() const
{
    HILOGD("[BluetoothDevice] name = %{public}s", GET_ENCRYPT_DEVICE_NAME(aliasName_));

    return aliasName_;
}

bool BluetoothDevice::SetAliasName(const std::string &name)
{
    HILOGD("[BluetoothDevice] name = %{public}s", GET_ENCRYPT_DEVICE_NAME(name));

    if (name.length() >= MAX_REMOTE_BT_NAME_LEN) {
        aliasName_ = name.substr(0, MAX_REMOTE_BT_NAME_LEN);
    } else {
        aliasName_ = name;
    }
    return true;
}

int BluetoothDevice::GetDeviceClass() const
{
    HILOGD("[BluetoothDevice] cod = %{public}d", cod_);

    return cod_;
}

void BluetoothDevice::SetDeviceClass(int deviceClass)
{
    HILOGD("[BluetoothDevice] cod = %{public}d", deviceClass);

    cod_ = deviceClass;
}

int BluetoothDevice::GetHwRemoteDeviceType() const
{
    HILOGD("[hwRemoteDeviceType] = %{public}d", hwRemoteDeviceType_);

    return hwRemoteDeviceType_;
}

void BluetoothDevice::SetHwRemoteDeviceType(int hwRemoteDeviceType)
{
    HILOGI("[hwRemoteDeviceType] = %{public}d", hwRemoteDeviceType);

    hwRemoteDeviceType_ = hwRemoteDeviceType;
}

bool BluetoothDevice::IsBondedFromLocal() const
{
    HILOGD("[BluetoothDevice] bondedFromLocal_ = %{public}d", bondedFromLocal_);

    return bondedFromLocal_;
}

void BluetoothDevice::SetBondedFromLocal(bool flag)
{
    HILOGD("[BluetoothDevice] bondedFromLocal_ = %{public}d", flag);

    bondedFromLocal_ = flag;
}

bool BluetoothDevice::DialogControlIsActivePair() const
{
    std::lock_guard<std::mutex> lock(g_dialogControlMutex);
    HILOGD("[DialogControl] dialogControlActivePair_ = %{public}d", dialogControlActivePair_);
    return dialogControlActivePair_;
}
 
void BluetoothDevice::DialogControlSetActivePair(bool dialogControlActivePair)
{
    std::lock_guard<std::mutex> lock(g_dialogControlMutex);
    HILOGD("[DialogControl] dialogControlActivePair_ = %{public}d", dialogControlActivePair);
    dialogControlActivePair_ = dialogControlActivePair;
}
 
void BluetoothDevice::DialogControlAddConnectingProfileCount()
{
    std::lock_guard<std::mutex> lock(g_dialogControlMutex);
    if (dialogControlActivePair_) {
        dialogControlConnectingProfileCount_++;
    }
}

void BluetoothDevice::DialogControlRemoveConnectingProfileCount(int state)
{
    std::lock_guard<std::mutex> lock(g_dialogControlMutex);
    if (dialogControlActivePair_) {
        bool isSteady = (state == static_cast<int>(BTConnectState::CONNECTED)
            || state == static_cast<int>(BTConnectState::DISCONNECTED));
        if (isSteady) {
            dialogControlConnectingProfileCount_--;
        }
        if (dialogControlConnectingProfileCount_ == 0) {
            dialogControlActivePair_ = false;
        }
    }
}

void BluetoothDevice::DialogControlClearConnectingProfileCount()
{
    std::lock_guard<std::mutex> lock(g_dialogControlMutex);
    dialogControlConnectingProfileCount_ = 0;
    dialogControlActivePair_ = false;
}
 
int BluetoothDevice::DialogControlConvertCause(int cause)
{
    std::lock_guard<std::mutex> lock(g_dialogControlMutex);
    if (dialogControlActivePair_ && dialogControlConnectingProfileCount_ > 0) {
        if (cause != 0) {
            HILOGI("[DialogControl] No dialog box is displayed when the connection fails during pairing.");
            cause = 0;
        }
    }
    return cause;
}

bool BluetoothDevice::IsAclConnected() const
{
    HILOGI("[BluetoothDevice] state = %{public}d", aclConnected_);

    return (aclConnected_ != CONNECTION_STATE_DISCONNECTED);
}

bool BluetoothDevice::IsAclEncrypted() const
{
    HILOGI("[BluetoothDevice] state = %{public}d", aclConnected_);

    return (aclConnected_ >= CONNECTION_STATE_CONNECTED);
}

void BluetoothDevice::SetAclConnectState(int connectState)
{
    HILOGI("[BluetoothDevice] state = %{public}d", connectState);

    aclConnected_ = connectState;
}

std::vector<Uuid> BluetoothDevice::GetDeviceUuids() const
{
    std::lock_guard<std::mutex> lock(uuidsMutex_);
    return uuids_;
}

void BluetoothDevice::SetDeviceUuids(const std::vector<Uuid> &uuids)
{
    std::lock_guard<std::mutex> lock(uuidsMutex_);
    uuids_.assign(uuids.begin(), uuids.end());
}

int BluetoothDevice::GetDeviceType() const
{
    HILOGD("[BluetoothDevice] type = %{public}d", deviceType_);

    return deviceType_;
}

void BluetoothDevice::SetDeviceType(int type)
{
    HILOGD("[BluetoothDevice] type = %{public}d", type);

    deviceType_ = type;
}

int BluetoothDevice::GetWearDetectionSupportValue() const
{
    HILOGI("[BluetoothDevice] wearDetectionSupportValue = %{public}d", wearDetectionSupportValue_);
    return wearDetectionSupportValue_;
}

void BluetoothDevice::SetWearDetectionSupportValue(int supportValue)
{
    HILOGD("[BluetoothDevice] WearDetectionSupportValue = %{public}d", supportValue);
    wearDetectionSupportValue_ = supportValue;
}

int BluetoothDevice::GetRssi() const
{
    HILOGD("[BluetoothDevice] rssi = %{public}d", rssi_);

    return rssi_;
}

void BluetoothDevice::SetRssi(int rssi)
{
    HILOGD("[BluetoothDevice] rssi = %{public}d", rssi);

    rssi_ = rssi;
}

void BluetoothDevice::SetConnectionHandle(int handle)
{
    HILOGI("[BluetoothDevice] connectionHandle = %{public}d", handle);

    connectionHandle_ = handle;
}

int BluetoothDevice::GetConnectionHandle() const
{
    HILOGI("[BluetoothDevice] connectionHandle = %{public}d", connectionHandle_);

    return connectionHandle_;
}

bool BluetoothDevice::SetPairedStatus(int status)
{
    HILOGI("[BluetoothDevice] pairedStatus = %{public}d", status);

    if (status < PAIR_NONE || status > PAIR_CANCELING) {
        HILOGE("[BluetoothDevice] Invalid Parameter");
        return false;
    }

    if (status == PAIR_NONE) {
        std::lock_guard<std::mutex> lock(uuidsMutex_);
        uuids_.clear();
    }

    pairState_ = status;
    return true;
}

int BluetoothDevice::GetPairedStatus() const
{
    HILOGD("[BluetoothDevice] pairedStatus = %{public}d", pairState_);

    return pairState_;
}

std::vector<uint8_t> BluetoothDevice::GetLinkKey() const
{
    return linkKey_;
}

void BluetoothDevice::SetLinkKey(const std::vector<uint8_t> &linkKey)
{
    linkKey_ = linkKey;
}

void BluetoothDevice::DeleteLinkKey()
{
    linkKey_.clear();
}

int BluetoothDevice::GetLinkKeyType() const
{
    HILOGI("[BluetoothDevice] linkKeyType = %{public}d", linkKeyType_);

    return linkKeyType_;
}

void BluetoothDevice::SetLinkKeyType(int linkKeyType)
{
    HILOGI("[BluetoothDevice] linkKeyType = %{public}d", linkKeyType);

    linkKeyType_ = linkKeyType;
}

int BluetoothDevice::GetIoCapability() const
{
    HILOGI("[BluetoothDevice] ioCapability = %{public}d", ioCapability_);

    return ioCapability_;
}

void BluetoothDevice::SetIoCapability(int io)
{
    HILOGI("[BluetoothDevice] ioCapability = %{public}d", io);

    ioCapability_ = io;
}

uint8_t BluetoothDevice::GetFlags() const
{
    HILOGI("[BluetoothDevice] flags = %{public}d", flags_);

    return flags_;
}

void BluetoothDevice::SetFlags(uint8_t flags)
{
    HILOGI("[BluetoothDevice] flags = %{public}d", flags);

    flags_ = flags;
}

void BluetoothDevice::SetManufacturerSpecificData(const std::vector<uint8_t> &data)
{
    manuSpecData_ = data;
}

const std::vector<uint8_t>& BluetoothDevice::GetManufacturerSpecificData() const
{
    return manuSpecData_;
}

bool BluetoothDevice::IsPaired() const
{
    HILOGD("[BluetoothDevice] pairState = %{public}d", pairState_);

    return (pairState_ == PAIR_PAIRED);
}

bool BluetoothDevice::IsPairNone() const
{
    HILOGD("[BluetoothDevice] pairState = %{public}d", pairState_);
    return (pairState_ == PAIR_NONE);
}

void BluetoothDevice::SetTxPower(uint8_t power)
{
    HILOGI("[BluetoothDevice] txPower = %{public}u", power);

    txPower_ = power;
}

void BluetoothDevice::SetURI(const std::string &uri)
{
    HILOGI("[BluetoothDevice] uri = %{public}s", uri.c_str());

    uri_ = uri;
}

int BluetoothDevice::GetPairConfirmState() const
{
    HILOGI("[BluetoothDevice] state = %{public}d", pairConfirmState_);

    return pairConfirmState_;
}

void BluetoothDevice::SetPairConfirmState(int state)
{
    HILOGD("[BluetoothDevice] state = %{public}d", state);

    pairConfirmState_ = state;
}

void BluetoothDevice::SetPairConfirmType(int type)
{
    HILOGD("[BluetoothDevice] Type = %{public}d", type);

    pairConfirmType_ = type;
}

int BluetoothDevice::GetPairConfirmType() const
{
    HILOGI("[BluetoothDevice] Type = %{public}d", pairConfirmType_);

    return pairConfirmType_;
}

void BluetoothDevice::SetNameNeedGet(bool nameUnknowned)
{
    HILOGI("[BluetoothDevice] nameNeedGet_ = %{public}d", nameUnknowned);
    nameNeedGet_ = nameUnknowned;
}

bool BluetoothDevice::GetNameNeedGet() const
{
    HILOGI("[BluetoothDevice] nameNeedGet_ = %{public}d", nameNeedGet_);

    return nameNeedGet_;
}

bool BluetoothDevice::CheckCod(uint32_t cod) const
{
    uint32_t tmpCod = cod_;
    return (tmpCod & CLASS_OF_DEVICE_MASK) == cod;
}

void BluetoothDevice::SetPasskey(uint32_t passKey)
{
    passkey_ =  static_cast<int>(passKey);
}

int BluetoothDevice::GetPasskey() const
{
    HILOGI("[BluetoothDevice] get passkey");
    return passkey_;
}

void BluetoothDevice::SetSspVariant(bt_ssp_variant_t pairingVariant)
{
    pairingVariant_ = static_cast<int>(pairingVariant);
}

int BluetoothDevice::GetSspVariant() const
{
    return pairingVariant_;
}

void BluetoothDevice::SetDeviceAbsVolumeAbility(int32_t ability)
{
    HILOGI("[BluetoothDevice] abs ability = %{public}d", ability);
    absVolumeAbility_ = ability;
}

int32_t BluetoothDevice::GetDeviceAbsVolumeAbility()
{
    return absVolumeAbility_;
}

int BluetoothDevice::GetMajorMinorClass() const
{
    int res = (cod_ & DEVICE_BITMASK);
    HILOGI("[BlueoothDevice] MajorMinorClass = 0x%{public}X", res);
    return res;
}

int BluetoothDevice::GetCustomType() const
{
    HILOGI_TIME_LIMIT(__func__, "[BluetoothDevice] customType:%{public}d", customType_);
    return customType_;
}

void BluetoothDevice::SetCustomType(int32_t customType)
{
    HILOGD("[BluetoothDevice] customType = %{public}d", customType);
    customType_ = customType;
}

BatteryDetailInfo BluetoothDevice::GetDeviceBatteryInfo() const
{
    return batteryInfo_;
}

void BluetoothDevice::SetDeviceBatteryInfo(const BatteryDetailInfo &batteryInfo)
{
    batteryInfo_ = batteryInfo;
}

void BluetoothDevice::ClearDeviceBatteryInfo()
{
    batteryInfo_.ResetInfo();
}

void BluetoothDevice::SetDevProactiveDisconnectFlag(uint32_t profileId, bool isProactive)
{
    proactiveDisconnectFlag_[profileId] = isProactive;
}

bool BluetoothDevice::IsDevProactiveDisconnect(uint32_t profileId) const
{
    auto it = proactiveDisconnectFlag_.find(profileId);
    if (it == proactiveDisconnectFlag_.end()) {
        HILOGE("Failed to find profileId: %{public}d", profileId);
        return false;
    }

    return proactiveDisconnectFlag_.at(profileId);
}

void BluetoothDevice::SetVendorId(int32_t vendorId)
{
    vendorId_ = vendorId;
}

int32_t BluetoothDevice::GetVendorId()
{
    return vendorId_;
}

void BluetoothDevice::SetProductId(int32_t productId)
{
    productId_ = productId;
}

int32_t BluetoothDevice::GetProductId()
{
    return productId_;
}

void BluetoothDevice::SetVersion(int32_t version)
{
    version_ = version;
}

int32_t BluetoothDevice::GetVersion()
{
    return version_;
}

void BluetoothDevice::SetModelId(int32_t modelId)
{
    modelId_ = modelId;
}

int32_t BluetoothDevice::GetModelId()
{
    return modelId_;
}

void BluetoothDevice::SetNewModelId(std::string newModelId)
{
    newModelId_ = newModelId;
}

std::string BluetoothDevice::GetNewModelId()
{
    return newModelId_;
}

void BluetoothDevice::SetIcon(int32_t icon)
{
    icon_ = icon;
}

int32_t BluetoothDevice::GetIcon()
{
    return icon_;
}

void BluetoothDevice::SetAutoConnSwitch(int32_t autoConnSwitch)
{
    autoConnSwitch_ = autoConnSwitch;
}

int32_t BluetoothDevice::GetAutoConnSwitch()
{
    return autoConnSwitch_;
}

void BluetoothDevice::SetTimeStamp(int32_t timeStamp)
{
    timeStamp_ = timeStamp;
}

int32_t BluetoothDevice::GetTimeStamp()
{
    return timeStamp_;
}

bool BluetoothDevice::IsNeedDelayConnect()
{
    HILOGD("[BluetoothDevice] isNeedDelayConnect_ = %{public}d", isNeedDelayConnect_);

    return isNeedDelayConnect_;
}

void BluetoothDevice::SetNeedDelayConnectFlag(bool isNeedDelayConnect)
{
    HILOGD("[BluetoothDevice] isNeedDelayConnect_ = %{public}d", isNeedDelayConnect);

    isNeedDelayConnect_ = isNeedDelayConnect;
}

void BluetoothDevice::SetHwTalkBandBandMode(int bandMode)
{
    headsetBandMode_ = bandMode;
}

bool BluetoothDevice::isBandMode() const
{
    return headsetBandMode_ == TalkBandMode::VALUE_OF_TALKBAND_BAND_MODE;
}

bool BluetoothDevice::isBandDevice() const
{
    return headsetBandMode_ != TalkBandMode::VALUE_OF_NOT_TALKBAND_DEVICE;
}

int64_t BluetoothDevice::GetLastConnectionTime() const
{
    HILOGD("[BluetoothDevice] GetLastConnectionTime = %{public}ld", connectionTime_);
    return connectionTime_;
}

void BluetoothDevice::SetConnectionTime(int64_t connectionTime)
{
    HILOGD("[BluetoothDevice] SetConnectionTime = %{public}ld", connectionTime);
    connectionTime_ = connectionTime;
}

bool BluetoothDevice::IsCloudDevice() const
{
    return isCloudDevice_;
}

void BluetoothDevice::SetCloudDevice(bool isCloudDevice)
{
    isCloudDevice_ = isCloudDevice;
    HILOGD("[CLOUD_PAIR] %{public}s set cloud device flag to %{public}d",
        GetEncryptAddr(macAddr_).c_str(), isCloudDevice_);
}

int32_t BluetoothDevice::GetCloudBondState() const
{
    HILOGD("[BluetoothDevice] GetCloudBondState %{public}s type = %{public}d",
        GetEncryptAddr(macAddr_).c_str(),
        cloudBondedState_);
    return cloudBondedState_;
}

void BluetoothDevice::SetCloudBondState(int32_t cloudBondState)
{
    HILOGI("[BluetoothDevice] SetCloudBondState %{public}s type = %{public}d",
        GetEncryptAddr(macAddr_).c_str(),
        cloudBondState);
    cloudBondedState_ = cloudBondState;
}

void BluetoothDevice::SetToken(const std::vector<uint8_t> &tokens)
{
    token_ = tokens;
}

std::vector<uint8_t> BluetoothDevice::GetToken()
{
    return token_;
}

void BluetoothDevice::SetIsConnectFromLocal(bool localConnect)
{
    HILOGI("[SetIsFirstBond] localConnect = %{public}d", localConnect);
    connectFromLocal_ = localConnect;
}

int BluetoothDevice::IsConnectFromLocalChangeCause(int cause) const
{
    if (connectFromLocal_) {
        return static_cast<int>(ConnChangeCause::CONNECT_FROM_LOCAL);
    }
    return cause;
}
}  // namespace bluetooth
}  // namespace OHOS
