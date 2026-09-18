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

#ifndef OHOS_BLUETOOTH_STANDARD_HOST_STUB_H
#define OHOS_BLUETOOTH_STANDARD_HOST_STUB_H

#include <map>

#include "i_bluetooth_host.h"
#include "iremote_stub.h"
#include "permission_item.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothHostStub : public IRemoteStub<IBluetoothHost> {
public:
    BluetoothHostStub();
    virtual ~BluetoothHostStub();

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    using BluetoothHostStubFunc = std::function<int32_t(BluetoothHostStub *, MessageParcel &, MessageParcel &)>;
    using BluetoothHostStubFuncPerm = std::pair<BluetoothHostStubFunc, std::shared_ptr<PermissionItem>>;

private:
    int32_t RegisterObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t DeregisterObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t EnableBtInner(MessageParcel &data, MessageParcel &reply);
    int32_t DisableBtInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetProfileInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetBleRemoteInner(MessageParcel &data, MessageParcel &reply);
    int32_t BluetoothFactoryResetInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetBtStateInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetLocalAddressInner(MessageParcel &data, MessageParcel &reply);
    int32_t GenerateLocalOobDataInner(MessageParcel &data, MessageParcel &reply);
    int32_t DisableBleInner(MessageParcel &data, MessageParcel &reply);
    int32_t EnableBleInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetProfileListInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetMaxNumConnectedAudioDevicesInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetBtConnectionStateInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetBtProfileConnStateInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetLocalDeviceClassInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetLocalDeviceClassInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetLocalNameInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetLocalNameInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetBtScanModeInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetBtScanModeInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetBondableModeInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetBondableModeInner(MessageParcel &data, MessageParcel &reply);
    int32_t StartBtDiscoveryInner(MessageParcel &data, MessageParcel &reply);
    int32_t CancelBtDiscoveryInner(MessageParcel &data, MessageParcel &reply);
    int32_t IsBtDiscoveringInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetBtDiscoveryEndMillisInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetPairedDevicesInner(MessageParcel &data, MessageParcel &reply);
    int32_t RemovePairInner(MessageParcel &data, MessageParcel &reply);
    int32_t RemoveAllPairsInner(MessageParcel &data, MessageParcel &reply);
    int32_t RegisterRemoteDeviceObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t DeregisterRemoteDeviceObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetBleMaxAdvertisingDataLengthInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetConnectedBLEDevicesInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetDeviceTypeInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetPhonebookPermissionInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetPhonebookPermissionInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetMessagePermissionInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetMessagePermissionInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetPowerModeInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetDeviceNameInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetDeviceAliasInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetDeviceAliasInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetRemoteDeviceBatteryInfoInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetRemoteDeviceBatteryInfoInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetPairStateInner(MessageParcel &data, MessageParcel &reply);
    int32_t StartPairInner(MessageParcel &data, MessageParcel &reply);
    int32_t StartCrediblePairInner(MessageParcel &data, MessageParcel &reply);
    int32_t CancelPairingInner(MessageParcel &data, MessageParcel &reply);
    int32_t IsBondedFromLocalInner(MessageParcel &data, MessageParcel &reply);
    int32_t IsAclConnectedInner(MessageParcel &data, MessageParcel &reply);
    int32_t IsAclEncryptedInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetDeviceClassInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetDevicePinInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetDevicePairingConfirmationInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetDevicePasskeyInner(MessageParcel &data, MessageParcel &reply);
    int32_t PairRequestReplyInner(MessageParcel &data, MessageParcel &reply);
    int32_t ReadRemoteRssiValueInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetLocalSupportedUuidsInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetDeviceUuidsInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetLocalProfileUuidsInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetFastScanInner(MessageParcel &data, MessageParcel &reply);
    int32_t RegisterBleAdapterObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t DeregisterBleAdapterObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t RegisterBlePeripheralCallbackInner(MessageParcel &data, MessageParcel &reply);
    int32_t DeregisterBlePeripheralCallbackInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetRandomAddressInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetRealAddressInner(MessageParcel &data, MessageParcel &reply);
    int32_t SyncRandomAddressInner(MessageParcel &data, MessageParcel &reply);
    int32_t ConnectAllowedProfilesInner(MessageParcel &data, MessageParcel &reply);
    int32_t DisconnectAllowedProfilesInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetDeviceCustomTypeInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetRemoteDeviceInfoInner(MessageParcel &data, MessageParcel &reply);
    int32_t RegisterBtResourceManagerObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t DeregisterBtResourceManagerObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t SatelliteControlInner(MessageParcel &data, MessageParcel &reply);
    int32_t IsSupportVirtualAutoConnectInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetVirtualAutoConnectTypeInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetFastScanLevelInner(MessageParcel &data, MessageParcel &reply);
    int32_t UpdateVirtualDeviceInner(MessageParcel &data, MessageParcel &reply);
    int32_t EnableBluetoothToRestrictModeInner(MessageParcel &data, MessageParcel &reply);
    int32_t EnableBluetoothToBleOwnerOnlyModeInner(MessageParcel &data, MessageParcel &reply);
    int32_t ControlDeviceActionInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetLastConnectionTimeInner(MessageParcel &data, MessageParcel &reply);
    int32_t UpdateCloudBluetoothDeviceInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetCloudBondStateInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetDeviceTransportInner(MessageParcel &data, MessageParcel &reply);
    int32_t UpdateRefusePolicyInner(MessageParcel &data, MessageParcel &reply);
    int32_t ProcessRandomDeviceIdCommandInner(MessageParcel &data, MessageParcel &reply);
    bool IsValidAddress(std::string addr);
    bool IsValidUuid(const std::string& uuid);
    bool IsValidHashValue(const std::string &hashValue);
    bool IsValidHashAlgorithmType(int hashAlgorithmType);
    int32_t GetCarKeyDfxDataInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetCarKeyCardDataInner(MessageParcel &data, MessageParcel &reply);
    int32_t NotifyDialogResultInner(MessageParcel &data, MessageParcel &reply);
    int32_t IsProfileExistInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetCallingPackageNameInner(MessageParcel &data, MessageParcel &reply);
    int32_t StartRemoteSdpSearchInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetRemoteServicesInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetVirtualAddressByHashInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetConnectionPriorityInner(MessageParcel &data, MessageParcel &reply);
    int32_t VerifyMultiPermissionsInner(MessageParcel &data, MessageParcel &reply);
private:
    static const std::map<uint32_t, BluetoothHostStubFuncPerm> memberFuncMap_;
    static constexpr int HASH_ALGORITHM_SHA256 = 0;
    static constexpr int MAX_PERMISSION_SIZE = 100;
    DISALLOW_COPY_AND_MOVE(BluetoothHostStub);
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_HOST_STUB_H
