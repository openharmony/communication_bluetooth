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
#ifndef LOG_TAG
#define LOG_TAG "bt_ipc_host_stub"
#endif

#include <regex>
#include "bt_def.h"
#include "bluetooth_host_stub.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "permission_manager.h"
#include "raw_address.h"
#include "bluetooth_remote_device_info.h"
#include "bt_hicollie_adapter.h"
#include "datetime_ex.h"
#include "bluetooth_oob_observer_proxy.h"

#ifdef STUB_FUNC
#undef STUB_FUNC
#endif
#define STUB_FUNC(code, func, perm) BluetoothHostInterfaceCode::code, {&BluetoothHostStub::func, perm}

using namespace OHOS::bluetooth;
namespace OHOS {
namespace Bluetooth {

// Note: Permissions need to be configured when the itf to be used. "nullptr" means no permission needed.
const std::map<uint32_t, BluetoothHostStub::BluetoothHostStubFuncPerm> BluetoothHostStub::memberFuncMap_ = {
    {STUB_FUNC(BT_REGISTER_OBSERVER, RegisterObserverInner, nullptr)},
    {STUB_FUNC(BT_DEREGISTER_OBSERVER, DeregisterObserverInner, nullptr)},
    {STUB_FUNC(BT_ENABLE, EnableBtInner, CHECK_PERM(false, {DISCOVER_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_DISABLE, DisableBtInner, CHECK_PERM(false, {DISCOVER_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_GETPROFILE, GetProfileInner, nullptr)},
    {STUB_FUNC(BT_GET_BLE, GetBleRemoteInner, nullptr)},
    {STUB_FUNC(BT_FACTORY_RESET, BluetoothFactoryResetInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_GETSTATE, GetBtStateInner, nullptr)},
    {STUB_FUNC(BT_GET_LOCAL_ADDRESS, GetLocalAddressInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, GET_BLUETOOTH_LOCAL_MAC)))},
    {STUB_FUNC(BT_GENERATE_LOCAL_OOB_DATA, GenerateLocalOobDataInner, CHECK_PERM(true, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_DISABLE_BLE, DisableBleInner, CHECK_PERM(false, {DISCOVER_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_ENABLE_BLE, EnableBleInner, CHECK_PERM(false, {DISCOVER_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_GET_PROFILE_LIST, GetProfileListInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_GET_MAXNUM_CONNECTED_AUDIODEVICES, GetMaxNumConnectedAudioDevicesInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_GET_BT_STATE, GetBtConnectionStateInner, CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_GET_BT_PROFILE_CONNSTATE, GetBtProfileConnStateInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_GET_LOCAL_DEVICE_CLASS, GetLocalDeviceClassInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_SET_LOCAL_DEVICE_CLASS, SetLocalDeviceClassInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_GET_LOCAL_NAME, GetLocalNameInner, CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_SET_LOCAL_NAME, SetLocalNameInner, CHECK_PERM(false, {DISCOVER_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_GET_BT_SCAN_MODE, GetBtScanModeInner, CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_SET_BT_SCAN_MODE, SetBtScanModeInner, CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_GET_BONDABLE_MODE, GetBondableModeInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_SET_BONDABLE_MODE, SetBondableModeInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_START_BT_DISCOVERY, StartBtDiscoveryInner, nullptr)},
    {STUB_FUNC(BT_CANCEL_BT_DISCOVERY, CancelBtDiscoveryInner,
        CHECK_PERM(false, {DISCOVER_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_IS_BT_DISCOVERING, IsBtDiscoveringInner, CHECK_PERM(false, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_GET_BT_DISCOVERY_END_MILLIS, GetBtDiscoveryEndMillisInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_GET_PAIRED_DEVICES, GetPairedDevicesInner, CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_REMOVE_PAIR, RemovePairInner, CHECK_PERM(true, {DISCOVER_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_REMOVE_ALL_PAIRS, RemoveAllPairsInner,
        CHECK_PERM(true, {DISCOVER_BLUETOOTH}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(CANCEL_PAIRING, CancelPairingInner, CHECK_PERM(true, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_REGISTER_REMOTE_DEVICE_OBSERVER, RegisterRemoteDeviceObserverInner, nullptr)},
    {STUB_FUNC(BT_DEREGISTER_REMOTE_DEVICE_OBSERVER, DeregisterRemoteDeviceObserverInner, nullptr)},
    {STUB_FUNC(BT_GET_BLE_MAX_ADVERTISING_DATALENGTH, GetBleMaxAdvertisingDataLengthInner, nullptr)},
    {STUB_FUNC(BT_GET_CONNECTED_BLE_DEVICES, GetConnectedBLEDevicesInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(GET_DEVICE_TYPE, GetDeviceTypeInner, nullptr)},
    {STUB_FUNC(GET_PHONEBOOK_PERMISSION, GetPhonebookPermissionInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(SET_PHONEBOOK_PERMISSION, SetPhonebookPermissionInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(GET_MESSAGE_PERMISSION, GetMessagePermissionInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(SET_MESSAGE_PERMISSION, SetMessagePermissionInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(GET_POWER_MODE, GetPowerModeInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(GET_DEVICE_NAME, GetDeviceNameInner, CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(GET_DEVICE_ALIAS, GetDeviceAliasInner,
        CHECK_PERM(true, {USE_BLUETOOTH}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(SET_DEVICE_ALIAS, SetDeviceAliasInner,
        CHECK_PERM(false, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(GET_DEVICE_BATTERY_INFO, GetRemoteDeviceBatteryInfoInner, CHECK_PERM(false, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(SET_DEVICE_BATTERY_INFO, SetRemoteDeviceBatteryInfoInner, CHECK_PERM(true, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(GET_PAIR_STATE, GetPairStateInner, CHECK_PERM(false, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(START_PAIR, StartPairInner, CHECK_PERM(false, {DISCOVER_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(START_CREDIBLE_PAIR, StartCrediblePairInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(IS_BONDED_FROM_LOCAL, IsBondedFromLocalInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(IS_ACL_CONNECTED, IsAclConnectedInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(IS_ACL_ENCRYPTED, IsAclEncryptedInner, nullptr)},
    {STUB_FUNC(GET_DEVICE_CLASS, GetDeviceClassInner, nullptr)},
    {STUB_FUNC(SET_DEVICE_PIN, SetDevicePinInner, CHECK_PERM(false, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(SET_DEVICE_PAIRING_CONFIRMATION, SetDevicePairingConfirmationInner,
        CHECK_PERM(false, {MANAGE_BLUETOOTH}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(SET_DEVICE_PASSKEY, SetDevicePasskeyInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(PAIR_REQUEST_PEPLY, PairRequestReplyInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(READ_REMOTE_RSSI_VALUE, ReadRemoteRssiValueInner, nullptr)},
    {STUB_FUNC(GET_LOCAL_SUPPORTED_UUIDS, GetLocalSupportedUuidsInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(GET_LOCAL_PROFILE_UUIDS, GetLocalProfileUuidsInner, CHECK_PERM(true, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(GET_DEVICE_UUIDS, GetDeviceUuidsInner, CHECK_PERM(false, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_REGISTER_BLE_ADAPTER_OBSERVER, RegisterBleAdapterObserverInner, nullptr)},
    {STUB_FUNC(BT_DEREGISTER_BLE_ADAPTER_OBSERVER, DeregisterBleAdapterObserverInner, nullptr)},
    {STUB_FUNC(BT_REGISTER_BLE_PERIPHERAL_OBSERVER, RegisterBlePeripheralCallbackInner, nullptr)},
    {STUB_FUNC(BT_DEREGISTER_BLE_PERIPHERAL_OBSERVER, DeregisterBlePeripheralCallbackInner, nullptr)},
    {STUB_FUNC(BT_SET_FAST_SCAN, SetFastScanInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(GET_RANDOM_ADDRESS, GetRandomAddressInner,
        CHECK_PERM(false, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(GET_REAL_ADDRESS, GetRealAddressInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(SYNC_RANDOM_ADDRESS, SyncRandomAddressInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(CONNECT_ALLOWED_PROFILES, ConnectAllowedProfilesInner,
        CHECK_PERM(false, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(DISCONNECT_ALLOWED_PROFILES, DisconnectAllowedProfilesInner,
        CHECK_PERM(false, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(SET_CUSTOM_TYPE, SetDeviceCustomTypeInner, CHECK_PERM(true, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(GET_DEVICE_INFO_ID, GetRemoteDeviceInfoInner, nullptr)},
    {STUB_FUNC(SATELLITE_CONTROL, SatelliteControlInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_REGISTER_RESOURCE_MANAGER_OBSERVER, RegisterBtResourceManagerObserverInner,
        CHECK_PERM(true, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_DEREGISTER_RESOURCE_MANAGER_OBSERVER, DeregisterBtResourceManagerObserverInner,
        CHECK_PERM(true, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(GET_VIRTUAL_AUTO_CONN_SWITCH, IsSupportVirtualAutoConnectInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(SET_VIRTUAL_AUTO_CONN_TYPE, SetVirtualAutoConnectTypeInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(SET_FAST_SCAN_LEVEL, SetFastScanLevelInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(UPDATE_VIRTUAL_DEVICE, UpdateVirtualDeviceInner,
        CHECK_PERM(true, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_ENABLE_BLUETOOTH_TO_RESTRICT_MODE, EnableBluetoothToRestrictModeInner,
        CHECK_PERM(false, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(CTRL_DEVICE_ACTION, ControlDeviceActionInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(GET_CONNECTION_TIME, GetLastConnectionTimeInner, nullptr)},
    {STUB_FUNC(BT_UPDATE_CLOUD_DEVICE, UpdateCloudBluetoothDeviceInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(GET_CLOUD_BOND_STATE, GetCloudBondStateInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_UPDATE_REFUSE_POLICY, UpdateRefusePolicyInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(PROCESS_RANDOM_DEVICE_ID_COMMAND, ProcessRandomDeviceIdCommandInner,
        CHECK_PERM(false, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(GET_DEVICE_TRANSPORT, GetDeviceTransportInner, nullptr)},
    {STUB_FUNC(BT_GET_CAR_KEY_DFX_DATA, GetCarKeyDfxDataInner, nullptr)},
    {STUB_FUNC(BT_SET_CAR_KEY_CARD_DATA, SetCarKeyCardDataInner, nullptr)},
    // 判断Profile是否存在，不涉及对外接口，无权限要求
    {STUB_FUNC(IS_PROFILE_EXIST, IsProfileExistInner, nullptr)},
    {STUB_FUNC(BT_NOTIFY_DIALOG_RESULT, NotifyDialogResultInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_SET_CALLING_PACKAGE_NAME, SetCallingPackageNameInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(START_REMOTE_SDP_SEARCH, StartRemoteSdpSearchInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(GET_REMOTE_SERVICES, GetRemoteServicesInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(GET_VIRTUAL_ADDRESS_BY_HASH, GetVirtualAddressByHashInner,
        CHECK_PERM(false, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(SET_CONNECTION_PRIORITY, SetConnectionPriorityInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_VERIFY_MULTI_PERMISSIONS, VerifyMultiPermissionsInner, nullptr)},
};

BluetoothHostStub::BluetoothHostStub()
{}

BluetoothHostStub::~BluetoothHostStub()
{}

int32_t BluetoothHostStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::string name = "BluetoothHost ipcCode: " + std::to_string(code);
    BtHicollieAdapter hicollie(name);
    CHECK_PERMISSION_AND_EXECUTE_FUNC_RETURN(BluetoothHostStub);
}

int32_t BluetoothHostStub::RegisterObserverInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    const sptr<IBluetoothHostObserver> observer = OHOS::iface_cast<IBluetoothHostObserver>(remote);
    CHECK_AND_RETURN_LOG_RET(observer != nullptr, ERR_INVALID_VALUE, "observer is nullptr");
    RegisterObserver(observer);
    return NO_ERROR;
}

int32_t BluetoothHostStub::DeregisterObserverInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    const sptr<IBluetoothHostObserver> observer = OHOS::iface_cast<IBluetoothHostObserver>(remote);
    CHECK_AND_RETURN_LOG_RET(observer != nullptr, ERR_INVALID_VALUE, "observer is nullptr");
    DeregisterObserver(observer);
    return NO_ERROR;
}

int32_t BluetoothHostStub::EnableBtInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = EnableBt();
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::DisableBtInner(MessageParcel &data, MessageParcel &reply)
{
    bool isAsync = false;
    if (!data.ReadBool(isAsync)) {
        HILOGE("isAsync failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    std::string callingName = data.ReadString();
    int32_t result = DisableBt(isAsync, callingName);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::GetProfileInner(MessageParcel &data, MessageParcel &reply)
{
    std::string name = data.ReadString();
    sptr<IRemoteObject> result = GetProfile(name);
    bool ret = reply.WriteRemoteObject(result);
    if (!ret) {
        HILOGD("reply writing failed.");
        return TRANSACTION_ERR;
    }
    return NO_ERROR;
}

int32_t BluetoothHostStub::GetBleRemoteInner(MessageParcel &data, MessageParcel &reply)
{
    std::string name = data.ReadString();
    sptr<IRemoteObject> result = GetBleRemote(name);
    bool ret = reply.WriteRemoteObject(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    return NO_ERROR;
}

int32_t BluetoothHostStub::BluetoothFactoryResetInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = BluetoothFactoryReset();
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(result), ERR_INVALID_VALUE, "writingInt32 failed.");
    return NO_ERROR;
}

int32_t BluetoothHostStub::GetBtStateInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t state = 0;
    int32_t result = GetBtState(state);
    (void)reply.WriteInt32(result);
    (void)reply.WriteInt32(state);
    return NO_ERROR;
}

int32_t BluetoothHostStub::GetLocalAddressInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = INVALID_MAC_ADDRESS;
    int32_t result = GetLocalAddress(addr);

    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(result), BT_ERR_IPC_TRANS_FAILED, "writing res failed");
    if (result != BT_NO_ERROR) {
        HILOGE("get local addr failed");
        return NO_ERROR;
    }
    CHECK_AND_RETURN_LOG_RET(reply.WriteString(addr), BT_ERR_IPC_TRANS_FAILED, "writing addr failed");
    return NO_ERROR;
}

int32_t BluetoothHostStub::GenerateLocalOobDataInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t transport;
    CHECK_AND_RETURN_LOG_RET(data.ReadInt32(transport), BT_ERR_IPC_TRANS_FAILED, "read transport failed");

    auto tempObject = data.ReadRemoteObject();
    sptr<IBluetoothOobObserver> observer = new BluetoothOobObserverProxy(tempObject);
    CHECK_AND_RETURN_LOG_RET(observer != nullptr, ERR_INVALID_VALUE, "observer is nullptr");

    int32_t result = GenerateLocalOobData(transport, observer);
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(result), BT_ERR_IPC_TRANS_FAILED, "writing result failed");
    return NO_ERROR;
}

int32_t BluetoothHostStub::DisableBleInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = DisableBle();
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::EnableBleInner(MessageParcel &data, MessageParcel &reply)
{
    bool noAutoConnect = false;
    if (!data.ReadBool(noAutoConnect)) {
        HILOGE("noAutoConnect failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    bool isAsync = false;
    if (!data.ReadBool(isAsync)) {
        HILOGE("isAsync failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    std::string callingName = data.ReadString();
    int32_t result = EnableBle(noAutoConnect, isAsync, callingName);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::GetProfileListInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<uint32_t> result = GetProfileList();
    bool ret = reply.WriteUInt32Vector(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    return NO_ERROR;
}

int32_t BluetoothHostStub::GetMaxNumConnectedAudioDevicesInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = GetMaxNumConnectedAudioDevices();
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    return NO_ERROR;
}

int32_t BluetoothHostStub::GetBtConnectionStateInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t state = 0;
    int32_t result = GetBtConnectionState(state);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    ret = reply.WriteInt32(state);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::GetBtProfileConnStateInner(MessageParcel &data, MessageParcel &reply)
{
    uint32_t profileId;
    data.ReadUint32(profileId);
    int32_t state = 0;
    int32_t result = GetBtProfileConnState(profileId, state);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    ret = reply.WriteInt32(state);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::GetLocalDeviceClassInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = GetLocalDeviceClass();
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    return NO_ERROR;
}

int32_t BluetoothHostStub::SetLocalDeviceClassInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t deviceClass;
    data.ReadInt32(deviceClass);
    bool result = SetLocalDeviceClass(deviceClass);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    return NO_ERROR;
}

int32_t BluetoothHostStub::GetLocalNameInner(MessageParcel &data, MessageParcel &reply)
{
    std::string name = "";
    int32_t result = GetLocalName(name);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    ret = reply.WriteString(name);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::SetLocalNameInner(MessageParcel &data, MessageParcel &reply)
{
    std::string name;
    data.ReadString(name);
    int32_t result = SetLocalName(name);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::GetDeviceTypeInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t transport;
    if (!data.ReadInt32(transport)) {
        HILOGE("transport failed");
        return TRANSACTION_ERR;
    }
    std::string address;
    if (!data.ReadString(address)) {
        HILOGE("address failed");
        return TRANSACTION_ERR;
    }
    int result = GetDeviceType(transport, address);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t BluetoothHostStub::GetBtScanModeInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t scanMode = 0;
    int32_t result = GetBtScanMode(scanMode);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    ret = reply.WriteInt32(scanMode);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::GetPhonebookPermissionInner(MessageParcel &data, MessageParcel &reply)
{
    std::string address;
    if (!data.ReadString(address)) {
        HILOGE("address failed");
        return TRANSACTION_ERR;
    }
    int result = GetPhonebookPermission(address);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    return NO_ERROR;
}

int32_t BluetoothHostStub::SetBtScanModeInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t mode;
    data.ReadInt32(mode);
    int32_t duration;
    data.ReadInt32(duration);
    int32_t result = SetBtScanMode(mode, duration);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::SetPhonebookPermissionInner(MessageParcel &data, MessageParcel &reply)
{
    std::string address;
    if (!data.ReadString(address)) {
        HILOGE("address failed");
        return TRANSACTION_ERR;
    }
    int32_t permission;
    if (!data.ReadInt32(permission)) {
        HILOGE("permission failed");
        return TRANSACTION_ERR;
    }
    bool result = SetPhonebookPermission(address, permission);
    bool ret = reply.WriteBool(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t BluetoothHostStub::GetBondableModeInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t transport;
    data.ReadInt32(transport);
    int32_t result = GetBondableMode(transport);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    return NO_ERROR;
}
int32_t BluetoothHostStub::GetMessagePermissionInner(MessageParcel &data, MessageParcel &reply)
{
    std::string address;
    if (!data.ReadString(address)) {
        HILOGE("address failed");
        return TRANSACTION_ERR;
    }
    int result = GetMessagePermission(address);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    return NO_ERROR;
}

int32_t BluetoothHostStub::SetBondableModeInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t transport;
    data.ReadInt32(transport);
    int32_t mode;
    data.ReadInt32(mode);

    bool result = SetBondableMode(transport, mode);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t BluetoothHostStub::SetMessagePermissionInner(MessageParcel &data, MessageParcel &reply)
{
    std::string address;
    if (!data.ReadString(address)) {
        HILOGE("address failed");
        return TRANSACTION_ERR;
    }
    int32_t permission;
    if (!data.ReadInt32(permission)) {
        HILOGE("permission failed");
        return TRANSACTION_ERR;
    }
    bool result = SetMessagePermission(address, permission);
    bool ret = reply.WriteBool(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    return NO_ERROR;
}

int32_t BluetoothHostStub::GetPowerModeInner(MessageParcel &data, MessageParcel &reply)
{
    std::string address;
    if (!data.ReadString(address)) {
        HILOGE("address failed");
        return TRANSACTION_ERR;
    }
    int result = GetPowerMode(address);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    return NO_ERROR;
}

int32_t BluetoothHostStub::StartBtDiscoveryInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = StartBtDiscovery();
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::CancelBtDiscoveryInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = CancelBtDiscovery();
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::GetDeviceNameInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t transport;
    if (!data.ReadInt32(transport)) {
        HILOGE("transport failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    std::string address;
    if (!data.ReadString(address)) {
        HILOGE("address failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    bool alias = true;
    if (!data.ReadBool(alias)) {
        HILOGE("alias failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    std::string name = "";
    int32_t result = GetDeviceName(transport, address, name, alias);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    ret = reply.WriteString(name);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::GetDeviceAliasInner(MessageParcel &data, MessageParcel &reply)
{
    std::string address;
    if (!data.ReadString(address)) {
        HILOGE("address failed");
        return TRANSACTION_ERR;
    }
    std::string result = GetDeviceAlias(address);
    bool ret = reply.WriteString(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    return NO_ERROR;
}

int32_t BluetoothHostStub::IsBtDiscoveringInner(MessageParcel &data, MessageParcel &reply)
{
    bool isDiscovering = false;
    int32_t transport;
    data.ReadInt32(transport);
    int32_t result = IsBtDiscovering(isDiscovering, transport);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    ret = reply.WriteBool(isDiscovering);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::SetDeviceAliasInner(MessageParcel &data, MessageParcel &reply)
{
    std::string address;
    if (!data.ReadString(address)) {
        HILOGE("address failed");
        return TRANSACTION_ERR;
    }
    std::string aliasName;
    if (!data.ReadString(aliasName)) {
        HILOGE("aliasName failed");
        return TRANSACTION_ERR;
    }
    int32_t result = SetDeviceAlias(address, aliasName);
    int32_t ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    return NO_ERROR;
}

int32_t BluetoothHostStub::GetRemoteDeviceBatteryInfoInner(MessageParcel &data, MessageParcel &reply)
{
    std::string address = "";
    CHECK_AND_RETURN_LOG_RET(data.ReadString(address), BT_ERR_INTERNAL_ERROR, "read address failed");

    BluetoothBatteryInfo info;
    int32_t ret = GetRemoteDeviceBatteryInfo(address, info);
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(ret), BT_ERR_INTERNAL_ERROR, "write ret failed");
    CHECK_AND_RETURN_LOG_RET(reply.WriteParcelable(&info), BT_ERR_INTERNAL_ERROR, "write batteryInfo failed");
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::SetRemoteDeviceBatteryInfoInner(MessageParcel &data, MessageParcel &reply)
{
    std::string address = "";
    CHECK_AND_RETURN_LOG_RET(data.ReadString(address), BT_ERR_INTERNAL_ERROR, "read address failed");
    if (!IsValidAddress(address)) {
        HILOGE("addr is invalid.");
        return TRANSACTION_ERR;
    }
    std::shared_ptr<BluetoothBatteryInfo> info(data.ReadParcelable<BluetoothBatteryInfo>());
    CHECK_AND_RETURN_LOG_RET(info != nullptr, BT_ERR_INTERNAL_ERROR, "read battery info fail");
    int32_t ret = SetRemoteDeviceBatteryInfo(address, *info);
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(ret), BT_ERR_INTERNAL_ERROR, "write ret failed");
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::GetBtDiscoveryEndMillisInner(MessageParcel &data, MessageParcel &reply)
{
    int64_t result = GetBtDiscoveryEndMillis();
    bool ret = reply.WriteInt64(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    return NO_ERROR;
}

int32_t BluetoothHostStub::GetPairStateInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t transport;
    if (!data.ReadInt32(transport)) {
        HILOGE("transport failed");
        return TRANSACTION_ERR;
    }
    std::string address;
    if (!data.ReadString(address)) {
        HILOGE("address failed");
        return TRANSACTION_ERR;
    }
    int32_t pairState;
    int result = GetPairState(transport, address, pairState);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("result writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    ret = reply.WriteInt32(pairState);
    if (!ret) {
        HILOGE("pairState writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::GetPairedDevicesInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<BluetoothRawAddress> pairDevice;
    int32_t result = GetPairedDevices(pairDevice);
    /* Write result (exception) first, then size and devices. Proxy reads the
     * first int32 as the exception code (BT_ERR_INVALID_STATE etc.), so writing
     * size first caused the proxy to misread the device count as an error
     * (GetPairedDevices exception:1) and return an empty list. */
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("result writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!reply.WriteInt32(pairDevice.size())) {
        HILOGE("pairDevice size writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    for (auto device : pairDevice) {
        reply.WriteParcelable(&device);
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::StartPairInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t transport;
    if (!data.ReadInt32(transport)) {
        HILOGE("transport failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    sptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        HILOGE("device failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    sptr<BluetoothOobData> oobData(data.ReadParcelable<BluetoothOobData>());
    if (!oobData) {
        HILOGE("oobData failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    int32_t result = StartPair(transport, *device, *oobData);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("result writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::StartCrediblePairInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t transport;
    if (!data.ReadInt32(transport)) {
        HILOGE("transport failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    std::string address;
    if (!data.ReadString(address)) {
        HILOGE("address failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    int32_t result = StartCrediblePair(transport, address);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("result writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::RemovePairInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t transport = data.ReadInt32();
    sptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (!device) {
        return BT_ERR_IPC_TRANS_FAILED;
    }
    int32_t result = RemovePair(transport, device);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("result writing failed.");
        return TRANSACTION_ERR;
    }
    return NO_ERROR;
}

int32_t BluetoothHostStub::CancelPairingInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t transport;
    if (!data.ReadInt32(transport)) {
        HILOGE("transport failed");
        return TRANSACTION_ERR;
    }
    std::string address;
    if (!data.ReadString(address)) {
        HILOGE("address failed");
        return TRANSACTION_ERR;
    }
    bool result = CancelPairing(transport, address);
    bool ret = reply.WriteBool(result);
    if (!ret) {
        HILOGE("result writing failed.");
        return TRANSACTION_ERR;
    }
    return NO_ERROR;
}

int32_t BluetoothHostStub::RemoveAllPairsInner(MessageParcel &data, MessageParcel &reply)
{
    bool result = RemoveAllPairs();
    bool ret = reply.WriteBool(result);
    if (!ret) {
        HILOGE("result writing failed.");
        return TRANSACTION_ERR;
    }

    return NO_ERROR;
}

int32_t BluetoothHostStub::IsBondedFromLocalInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t transport;
    if (!data.ReadInt32(transport)) {
        HILOGE("transport failed");
        return TRANSACTION_ERR;
    }
    std::string address;
    if (!data.ReadString(address)) {
        HILOGE("address failed");
        return TRANSACTION_ERR;
    }
    bool result = IsBondedFromLocal(transport, address);
    bool ret = reply.WriteBool(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    return NO_ERROR;
}

int32_t BluetoothHostStub::SetDevicePinInner(MessageParcel &data, MessageParcel &reply)
{
    std::string address;
    if (!data.ReadString(address)) {
        HILOGE("address failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    std::string pin;
    if (!data.ReadString(pin)) {
        HILOGE("pin failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    int32_t result = SetDevicePin(address, pin);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("result writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::IsAclConnectedInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t transport;
    if (!data.ReadInt32(transport)) {
        HILOGE("transport failed");
        return TRANSACTION_ERR;
    }
    std::string address;
    if (!data.ReadString(address)) {
        HILOGE("address failed");
        return TRANSACTION_ERR;
    }
    bool result = IsAclConnected(transport, address);
    bool ret = reply.WriteBool(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    return NO_ERROR;
}

int32_t BluetoothHostStub::RegisterRemoteDeviceObserverInner(MessageParcel &data, MessageParcel &reply)
{
    auto tempObject = data.ReadRemoteObject();
    sptr<IBluetoothRemoteDeviceObserver> observer;
    observer = iface_cast<IBluetoothRemoteDeviceObserver>(tempObject);
    CHECK_AND_RETURN_LOG_RET(observer != nullptr, ERR_INVALID_VALUE, "observer is nullptr");
    RegisterRemoteDeviceObserver(observer);
    return NO_ERROR;
}

int32_t BluetoothHostStub::DeregisterRemoteDeviceObserverInner(MessageParcel &data, MessageParcel &reply)
{
    auto tempObject = data.ReadRemoteObject();
    sptr<IBluetoothRemoteDeviceObserver> observer;
    observer = iface_cast<IBluetoothRemoteDeviceObserver>(tempObject);
    CHECK_AND_RETURN_LOG_RET(observer != nullptr, ERR_INVALID_VALUE, "observer is nullptr");
    DeregisterRemoteDeviceObserver(observer);
    return NO_ERROR;
}

int32_t BluetoothHostStub::GetBleMaxAdvertisingDataLengthInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = GetBleMaxAdvertisingDataLength();
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    return NO_ERROR;
}

int32_t BluetoothHostStub::GetConnectedBLEDevicesInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t bleProfile;
    if (!data.ReadInt32(bleProfile)) {
        HILOGE("bleProfile failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    std::vector<std::string> connectedDevices;
    int res = GetConnectedBLEDevices(bleProfile, connectedDevices);
    if (!reply.WriteInt32(res)) {
        HILOGE("write res failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (res != BT_NO_ERROR) {
        return res;
    }
    uint32_t size = static_cast<uint32_t>(connectedDevices.size());
    if (!reply.WriteUint32(size)) {
        HILOGE("write size failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    for (const auto &device : connectedDevices) {
        if (!reply.WriteString(device)) {
            HILOGE("write devices failed");
            return BT_ERR_IPC_TRANS_FAILED;
        }
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::IsAclEncryptedInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t transport;
    if (!data.ReadInt32(transport)) {
        HILOGE("transport failed");
        return TRANSACTION_ERR;
    }
    std::string address;
    if (!data.ReadString(address)) {
        HILOGE("address failed");
        return TRANSACTION_ERR;
    }
    bool result = IsAclEncrypted(transport, address);
    bool ret = reply.WriteBool(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    return NO_ERROR;
}

int32_t BluetoothHostStub::GetDeviceClassInner(MessageParcel &data, MessageParcel &reply)
{
    std::string address;
    if (!data.ReadString(address)) {
        HILOGE("address failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    int32_t cod = 0;
    int result = GetDeviceClass(address, cod);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    ret = reply.WriteInt32(cod);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::SetDevicePairingConfirmationInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t transport;
    if (!data.ReadInt32(transport)) {
        HILOGE("transport failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    std::string address;
    if (!data.ReadString(address)) {
        HILOGE("address failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    bool accept = false;
    if (!data.ReadBool(accept)) {
        HILOGE("accept failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    int32_t result = SetDevicePairingConfirmation(transport, address, accept);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::SetDevicePasskeyInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t transport;
    if (!data.ReadInt32(transport)) {
        HILOGE("transport failed");
        return TRANSACTION_ERR;
    }
    std::string address;
    if (!data.ReadString(address)) {
        HILOGE("address failed");
        return TRANSACTION_ERR;
    }
    int32_t passkey;
    if (!data.ReadInt32(passkey)) {
        HILOGE("passkey failed");
        return TRANSACTION_ERR;
    }
    bool accept = false;
    if (!data.ReadBool(accept)) {
        HILOGE("accept failed");
        return TRANSACTION_ERR;
    }
    bool result = SetDevicePasskey(transport, address, passkey, accept);
    bool ret = reply.WriteBool(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    return NO_ERROR;
}

int32_t BluetoothHostStub::PairRequestReplyInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t transport;
    if (!data.ReadInt32(transport)) {
        HILOGE("transport failed");
        return TRANSACTION_ERR;
    }
    std::string address;
    if (!data.ReadString(address)) {
        HILOGE("address failed");
        return TRANSACTION_ERR;
    }
    bool accept = false;
    if (!data.ReadBool(accept)) {
        HILOGE("accept failed");
        return TRANSACTION_ERR;
    }
    bool result = PairRequestReply(transport, address, accept);
    bool ret = reply.WriteBool(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    return NO_ERROR;
}

int32_t BluetoothHostStub::ReadRemoteRssiValueInner(MessageParcel &data, MessageParcel &reply)
{
    std::string address;
    if (!data.ReadString(address)) {
        HILOGE("address failed");
        return TRANSACTION_ERR;
    }
    bool result = ReadRemoteRssiValue(address);
    bool ret = reply.WriteBool(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    return NO_ERROR;
}

int32_t BluetoothHostStub::GetLocalSupportedUuidsInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::string> uuids;
    GetLocalSupportedUuids(uuids);
    int32_t size = static_cast<int32_t>(uuids.size());
    bool ret = reply.WriteInt32(size);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    for (auto uuid : uuids) {
        if (!reply.WriteString(uuid)) {
            HILOGE("write uuid error");
            return TRANSACTION_ERR;
        }
    }
    return NO_ERROR;
}

int32_t BluetoothHostStub::GetDeviceUuidsInner(MessageParcel &data, MessageParcel &reply)
{
    std::string address;
    std::vector<std::string> uuids;
    if (!data.ReadString(address)) {
        HILOGE("Read address error");
        return TRANSACTION_ERR;
    }

    int res = GetDeviceUuids(address, uuids);
    int32_t size = static_cast<int32_t>(uuids.size());
    bool ret = reply.WriteInt32(size);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    for (auto uuid : uuids) {
        if (!reply.WriteString(uuid)) {
            HILOGE("write uuid error");
            return TRANSACTION_ERR;
        }
    }
    if (!reply.WriteInt32(res)) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    return NO_ERROR;
}

int32_t BluetoothHostStub::GetLocalProfileUuidsInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::string> uuids;
    int res = GetLocalProfileUuids(uuids);
    int32_t size = static_cast<int32_t>(uuids.size());
    bool ret = reply.WriteInt32(size);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    for (auto uuid : uuids) {
        if (!reply.WriteString(uuid)) {
            HILOGE("write uuid error");
            return TRANSACTION_ERR;
        }
    }
    if (!reply.WriteInt32(res)) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    return NO_ERROR;
}

int32_t BluetoothHostStub::RegisterBleAdapterObserverInner(MessageParcel &data, MessageParcel &reply)
{
    auto tempObject = data.ReadRemoteObject();
    sptr<IBluetoothHostObserver> observer;
    observer = iface_cast<IBluetoothHostObserver>(tempObject);
    CHECK_AND_RETURN_LOG_RET(observer != nullptr, ERR_INVALID_VALUE, "observer is nullptr");
    RegisterBleAdapterObserver(observer);
    return NO_ERROR;
}

int32_t BluetoothHostStub::DeregisterBleAdapterObserverInner(MessageParcel &data, MessageParcel &reply)
{
    auto tempObject = data.ReadRemoteObject();
    sptr<IBluetoothHostObserver> observer;
    observer = iface_cast<IBluetoothHostObserver>(tempObject);
    CHECK_AND_RETURN_LOG_RET(observer != nullptr, ERR_INVALID_VALUE, "observer is nullptr");
    DeregisterBleAdapterObserver(observer);
    return NO_ERROR;
}

int32_t BluetoothHostStub::RegisterBlePeripheralCallbackInner(MessageParcel &data, MessageParcel &reply)
{
    auto tempObject = data.ReadRemoteObject();
    sptr<IBluetoothBlePeripheralObserver> observer;
    observer = iface_cast<IBluetoothBlePeripheralObserver>(tempObject);
    CHECK_AND_RETURN_LOG_RET(observer != nullptr, ERR_INVALID_VALUE, "observer is nullptr");
    RegisterBlePeripheralCallback(observer);
    return NO_ERROR;
}

int32_t BluetoothHostStub::DeregisterBlePeripheralCallbackInner(MessageParcel &data, MessageParcel &reply)
{
    auto tempObject = data.ReadRemoteObject();
    sptr<IBluetoothBlePeripheralObserver> observer;
    observer = iface_cast<IBluetoothBlePeripheralObserver>(tempObject);
    CHECK_AND_RETURN_LOG_RET(observer != nullptr, ERR_INVALID_VALUE, "observer is nullptr");
    DeregisterBlePeripheralCallback(observer);
    return NO_ERROR;
}

int32_t BluetoothHostStub::SetFastScanInner(MessageParcel &data, MessageParcel &reply)
{
    bool isEnable;
    if (!data.ReadBool(isEnable)) {
        HILOGE("read isEnable failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    int32_t result = SetFastScan(isEnable);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::GetRandomAddressInner(MessageParcel &data, MessageParcel &reply)
{
    std::string realAddr;
    if (!data.ReadString(realAddr)) {
        HILOGE("read realAddress failed");
        return TRANSACTION_ERR;
    }
    CHECK_AND_RETURN_LOG_RET(IsValidAddress(realAddr), BT_ERR_INVALID_PARAM, "Invalid address");
    uint64_t tokenId = 0;
    if (!data.ReadUint64(tokenId)) {
        HILOGE("read tokenId failed");
        return TRANSACTION_ERR;
    }
    std::string randomAddr;
    int32_t ret = GetRandomAddress(realAddr, randomAddr, tokenId);
    if (!reply.WriteInt32(ret)) {
        HILOGE("Write ret failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (ret == BT_NO_ERROR) {
        if (!reply.WriteString(randomAddr)) {
            HILOGE("Write randomAddr failed");
            return BT_ERR_IPC_TRANS_FAILED;
        }
    }
    return ret;
}

int32_t BluetoothHostStub::GetRealAddressInner(MessageParcel &data, MessageParcel &reply)
{
    std::string randomAddr;
    if (!data.ReadString(randomAddr)) {
        HILOGE("read randomAddr failed");
        return BT_ERR_INTERNAL_ERROR;
    }
    CHECK_AND_RETURN_LOG_RET(IsValidAddress(randomAddr), BT_ERR_INVALID_PARAM, "Invalid address");
    std::string realAddr;
    int32_t ret = GetRealAddress(randomAddr, realAddr);
    reply.WriteInt32(ret);
    if (ret == BT_NO_ERROR) {
        reply.WriteString(realAddr);
    }
    return ret;
}

int32_t BluetoothHostStub::SyncRandomAddressInner(MessageParcel &data, MessageParcel &reply)
{
    std::string realAddr;
    if (!data.ReadString(realAddr)) {
        HILOGE("read realAddress failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    std::string randomAddr;
    if (!data.ReadString(randomAddr)) {
        HILOGE("read randomAddr failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    int32_t ret = SyncRandomAddress(realAddr, randomAddr);
    if (!reply.WriteInt32(ret)) {
        HILOGE("Write ret failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return NO_ERROR;
}

int32_t BluetoothHostStub::ConnectAllowedProfilesInner(MessageParcel &data, MessageParcel &reply)
{
    std::string address;
    CHECK_AND_RETURN_LOG_RET(data.ReadString(address), BT_ERR_IPC_TRANS_FAILED, "Read address failed");

    int32_t result = ConnectAllowedProfiles(address);
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(result), BT_ERR_IPC_TRANS_FAILED, "Writing failed");

    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::DisconnectAllowedProfilesInner(MessageParcel &data, MessageParcel &reply)
{
    std::string address;
    CHECK_AND_RETURN_LOG_RET(data.ReadString(address), BT_ERR_IPC_TRANS_FAILED, "Read address failed");

    int32_t result = DisconnectAllowedProfiles(address);
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(result), BT_ERR_IPC_TRANS_FAILED, "Writing failed");

    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::SetDeviceCustomTypeInner(MessageParcel &data, MessageParcel &reply)
{
    std::string address;
    int32_t deviceCustomType;
    CHECK_AND_RETURN_LOG_RET(data.ReadString(address), BT_ERR_IPC_TRANS_FAILED, "Read address failed");
    CHECK_AND_RETURN_LOG_RET(data.ReadInt32(deviceCustomType), BT_ERR_IPC_TRANS_FAILED, "Read deviceCustomType failed");

    int result = SetDeviceCustomType(address, deviceCustomType);
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(result), BT_ERR_IPC_TRANS_FAILED, "reply writing failed.");
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::GetRemoteDeviceInfoInner(MessageParcel &data, MessageParcel &reply)
{
    std::string address;
    CHECK_AND_RETURN_LOG_RET(data.ReadString(address), BT_ERR_IPC_TRANS_FAILED, "Read address failed");
    CHECK_AND_RETURN_LOG_RET(IsValidAddress(address), BT_ERR_INVALID_PARAM, "Invalid address");
    int type = bluetooth::DeviceInfoType::DEVICE_INFO_UNKNOWN;
    CHECK_AND_RETURN_LOG_RET(data.ReadInt32(type), BT_ERR_IPC_TRANS_FAILED, "Read type failed");

    auto deviceInfo = std::make_shared<BluetoothRemoteDeviceInfo>();
    int32_t result = GetRemoteDeviceInfo(address, deviceInfo, type);
    CHECK_AND_RETURN_LOG_RET(deviceInfo != nullptr, BT_ERR_IPC_TRANS_FAILED, "Get device info failed");
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(result), BT_ERR_IPC_TRANS_FAILED, "reply writing failed.");
    CHECK_AND_RETURN_LOG_RET(reply.WriteParcelable(deviceInfo.get()), BT_ERR_IPC_TRANS_FAILED, "reply writing failed.");
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::UpdateVirtualDeviceInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t action;
    std::string address;
    CHECK_AND_RETURN_LOG_RET(data.ReadInt32(action), BT_ERR_IPC_TRANS_FAILED, "Read action failed");
    CHECK_AND_RETURN_LOG_RET(data.ReadString(address), BT_ERR_IPC_TRANS_FAILED, "Read address failed");

    UpdateVirtualDevice(action, address);

    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::SatelliteControlInner(MessageParcel &data, MessageParcel &reply)
{
    int type;
    int state;
    CHECK_AND_RETURN_LOG_RET(data.ReadInt32(type), BT_ERR_IPC_TRANS_FAILED, "Read state failed");
    CHECK_AND_RETURN_LOG_RET(data.ReadInt32(state), BT_ERR_IPC_TRANS_FAILED, "Read state failed");

    int32_t result = SatelliteControl(type, state);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::RegisterBtResourceManagerObserverInner(MessageParcel &data, MessageParcel &reply)
{
    auto tempObject = data.ReadRemoteObject();
    sptr<IBluetoothResourceManagerObserver> observer;
    observer = iface_cast<IBluetoothResourceManagerObserver>(tempObject);
    CHECK_AND_RETURN_LOG_RET(observer != nullptr, ERR_INVALID_VALUE, "observer is nullptr");
    RegisterBtResourceManagerObserver(observer);
    return NO_ERROR;
}

int32_t BluetoothHostStub::DeregisterBtResourceManagerObserverInner(MessageParcel &data, MessageParcel &reply)
{
    auto tempObject = data.ReadRemoteObject();
    sptr<IBluetoothResourceManagerObserver> observer;
    observer = iface_cast<IBluetoothResourceManagerObserver>(tempObject);
    CHECK_AND_RETURN_LOG_RET(observer != nullptr, ERR_INVALID_VALUE, "observer is nullptr");
    DeregisterBtResourceManagerObserver(observer);
    return NO_ERROR;
}

int32_t BluetoothHostStub::IsSupportVirtualAutoConnectInner(MessageParcel &data, MessageParcel &reply)
{
    std::string address;
    CHECK_AND_RETURN_LOG_RET(data.ReadString(address), BT_ERR_IPC_TRANS_FAILED, "Read address failed");
    CHECK_AND_RETURN_LOG_RET(IsValidAddress(address), BT_ERR_INVALID_PARAM, "Invalid address");
    bool support = false;
    IsSupportVirtualAutoConnect(address, support);
    int32_t result = reply.WriteBool(support);
    if (result != BT_NO_ERROR) {
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::SetVirtualAutoConnectTypeInner(MessageParcel &data, MessageParcel &reply)
{
    std::string address;
    int32_t connType = 0;
    int32_t businessType = 0;
    CHECK_AND_RETURN_LOG_RET(data.ReadString(address), BT_ERR_IPC_TRANS_FAILED, "Read address failed");
    CHECK_AND_RETURN_LOG_RET(data.ReadInt32(connType), BT_ERR_IPC_TRANS_FAILED, "Read connType failed");
    CHECK_AND_RETURN_LOG_RET(data.ReadInt32(businessType), BT_ERR_IPC_TRANS_FAILED, "Read businessType failed");
    CHECK_AND_RETURN_LOG_RET(IsValidAddress(address), BT_ERR_INVALID_PARAM, "Invalid address");
    return SetVirtualAutoConnectType(address, connType, businessType);
}

int32_t BluetoothHostStub::SetFastScanLevelInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t level = 0;
    CHECK_AND_RETURN_LOG_RET(data.ReadInt32(level), BT_ERR_IPC_TRANS_FAILED, "Read level failed");
    int32_t result = SetFastScanLevel(level);
    result = reply.WriteInt32(result);
    if (result != BT_NO_ERROR) {
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

bool BluetoothHostStub::IsValidAddress(std::string addr)
{
    if (addr.empty()) {
        HILOGE("addr is empty.");
        return false;
    }
    const std::regex deviceIdRegex("^[0-9a-fA-F]{2}(:[0-9a-fA-F]{2}){5}$");
    return regex_match(addr, deviceIdRegex);
}

bool BluetoothHostStub::IsValidUuid(const std::string& uuid)
{
    if (uuid.empty()) {
        HILOGE("uuid is empty.");
        return false;
    }
    const std::regex uuidRegex("^[0-9a-fA-F]{8}-([0-9a-fA-F]{4}-){3}[0-9a-fA-F]{12}$");
    return regex_match(uuid, uuidRegex);
}

bool BluetoothHostStub::IsValidHashValue(const std::string &hashValue)
{
    std::regex hexPattern("^[0-9a-fA-F]{32}$");
    return std::regex_match(hashValue, hexPattern);
}

bool BluetoothHostStub::IsValidHashAlgorithmType(int hashAlgorithmType)
{
    return hashAlgorithmType == HASH_ALGORITHM_SHA256;
}

int32_t BluetoothHostStub::EnableBluetoothToRestrictModeInner(MessageParcel &data, MessageParcel &reply)
{
    std::string callingName = data.ReadString();
    int32_t result = EnableBluetoothToRestrictMode(callingName);
    result = reply.WriteInt32(result);
    if (result != BT_NO_ERROR) {
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::ControlDeviceActionInner(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId = INVALID_MAC_ADDRESS;
    uint32_t controlType = INVALID_CONTROL_TYPE;
    uint32_t controlTypeVal = INVALID_CONTROL_TYPE_VAL;
    uint32_t controlObject = INVALID_CONTROL_OBJECT;
    CHECK_AND_RETURN_LOG_RET(data.ReadString(deviceId), BT_ERR_IPC_TRANS_FAILED, "Read address failed");
    CHECK_AND_RETURN_LOG_RET(data.ReadUint32(controlType), BT_ERR_IPC_TRANS_FAILED, "Read controlType failed");
    CHECK_AND_RETURN_LOG_RET(data.ReadUint32(controlTypeVal), BT_ERR_IPC_TRANS_FAILED, "Read controlTypeVal failed");
    CHECK_AND_RETURN_LOG_RET(data.ReadUint32(controlObject), BT_ERR_IPC_TRANS_FAILED, "Read controlObject failed");
    int32_t result = ControlDeviceAction(deviceId, controlType, controlTypeVal, controlObject);
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(result), BT_ERR_IPC_TRANS_FAILED, "reply writing failed");
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::GetLastConnectionTimeInner(MessageParcel &data, MessageParcel &reply)
{
    std::string address;
    int64_t connectionTime = 0;
    CHECK_AND_RETURN_LOG_RET(data.ReadString(address), BT_ERR_IPC_TRANS_FAILED, "Read address failed");
    CHECK_AND_RETURN_LOG_RET(IsValidAddress(address), BT_ERR_INVALID_PARAM, "Invalid address");
    int32_t result = GetLastConnectionTime(address, connectionTime);
    bool ret = reply.WriteInt32(result);
    if (!ret || result != BT_NO_ERROR) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    ret = reply.WriteInt64(connectionTime);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::GetCloudBondStateInner(MessageParcel &data, MessageParcel &reply)
{
    std::string address;
    int32_t cloudBondState = 0;
    CHECK_AND_RETURN_LOG_RET(data.ReadString(address), BT_ERR_IPC_TRANS_FAILED, "Read address failed");
    CHECK_AND_RETURN_LOG_RET(IsValidAddress(address), BT_ERR_INVALID_PARAM, "Invalid address");
    int32_t result = GetCloudBondState(address, cloudBondState);
    bool ret = reply.WriteInt32(result);
    if (!ret || result != BT_NO_ERROR) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    ret = reply.WriteInt32(cloudBondState);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::GetDeviceTransportInner(MessageParcel &data, MessageParcel &reply)
{
    std::string address;
    if (!data.ReadString(address)) {
        HILOGE("address failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    int32_t transport = TRANSPORT_UNKNOWN;
    int result = GetDeviceTransport(address, transport);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    ret = reply.WriteInt32(transport);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::UpdateCloudBluetoothDeviceInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t size;
    CHECK_AND_RETURN_LOG_RET(data.ReadInt32(size), BT_ERR_IPC_TRANS_FAILED, "Read size failed");
    CHECK_AND_RETURN_LOG_RET(size < TRUST_PAIR_DEVICE_SIZE_MAX, BT_ERR_INVALID_PARAM,
        "error size:%{public}d", size);
    std::vector<BluetoothTrustPairDevice> trustDevices;
    HILOGI("size: %{public}d", size);
    for (auto i = 0; i < size; i++) {
        std::shared_ptr<BluetoothTrustPairDevice> device(data.ReadParcelable<BluetoothTrustPairDevice>());
        if (device == nullptr) {
            HILOGE("create device failed");
            continue;
        }
        trustDevices.push_back(*device);
    }
    int32_t result = UpdateCloudBluetoothDevice(trustDevices);
    result = reply.WriteInt32(result);
    if (result != BT_NO_ERROR) {
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::UpdateRefusePolicyInner(MessageParcel &data, MessageParcel &reply)
{
#ifdef BLUETOOTH_KIA_ENABLE
    int32_t protocolType = 0;
    CHECK_AND_RETURN_LOG_RET(data.ReadInt32(protocolType), BT_ERR_IPC_TRANS_FAILED,
                             "read type error");
    int32_t pid = 0;
    CHECK_AND_RETURN_LOG_RET(data.ReadInt32(pid), BT_ERR_IPC_TRANS_FAILED,
                             "read pid error");
    int64_t prohibitedSecondsTime = 0;
    CHECK_AND_RETURN_LOG_RET(data.ReadInt64(prohibitedSecondsTime), BT_ERR_IPC_TRANS_FAILED,
                             "read time error");
    int32_t result = UpdateRefusePolicy(protocolType, pid, prohibitedSecondsTime);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
#endif
    return BT_NO_ERROR;
}

static bool CheckProcessRandomDeviceIdCommandPermission(int32_t cmd)
{
    if (cmd != static_cast<int>(RandomDeviceIdCommand::IS_VALID)) {
        return PermissionManager::VerifyPermission(PERSISTENT_BLUETOOTH_PEERS_MAC);
    }
    // ohos.permission.ACCESS_BLUETOOTH has already check.
    return true;
}

int32_t BluetoothHostStub::ProcessRandomDeviceIdCommandInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t cmd;
    data.ReadInt32(cmd);
    if (!CheckProcessRandomDeviceIdCommandPermission(cmd)) {
        reply.WriteInt32(BT_ERR_PERMISSION_FAILED);
        return BT_ERR_IPC_TRANS_FAILED;
    }

    uint32_t size = 0;
    const uint32_t maxSize = 10000;
    data.ReadUint32(size);
    if (size > maxSize) {
        HILOGE("error size: %{public}d", size);
        reply.WriteInt32(BT_ERR_INTERNAL_ERROR);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    std::vector<std::string> deviceIdVec {};
    for (uint32_t i = 0; i < size; i++) {
        std::string deviceId = "";
        data.ReadString(deviceId);
        if (IsValidAddress(deviceId)) {
            deviceIdVec.push_back(deviceId);
        }
    }

    bool isValid = false;
    int32_t ret =  ProcessRandomDeviceIdCommand(cmd, deviceIdVec, isValid);
    reply.WriteInt32(ret);
    if (ret != BT_NO_ERROR) {
        return BT_ERR_IPC_TRANS_FAILED;
    }

    size = static_cast<uint32_t>(deviceIdVec.size());
    reply.WriteUint32(size);
    for (uint32_t i = 0; i < size; i++) {
        reply.WriteString(deviceIdVec[i]);
    }
    reply.WriteBool(isValid);
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::GetCarKeyDfxDataInner(MessageParcel &data, MessageParcel &reply)
{
    std::string dfxData;
    int32_t result = GetCarKeyDfxData(dfxData);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    ret = reply.WriteString(dfxData);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::SetCarKeyCardDataInner(MessageParcel &data, MessageParcel &reply)
{
    std::string address;
    CHECK_AND_RETURN_LOG_RET(data.ReadString(address), BT_ERR_IPC_TRANS_FAILED, "Read address failed");
    CHECK_AND_RETURN_LOG_RET(IsValidAddress(address), BT_ERR_INVALID_PARAM, "Invalid address");
    int32_t action;
    CHECK_AND_RETURN_LOG_RET(data.ReadInt32(action), BT_ERR_IPC_TRANS_FAILED, "Read action failed");

    int32_t result = SetCarKeyCardData(address, action);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::NotifyDialogResultInner(MessageParcel &data, MessageParcel &reply)
{
    uint32_t dialogType = INVALID_DIALOG_TYPE;
    if (!data.ReadUint32(dialogType)) {
        HILOGE("dialogType failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    bool dialogResult = false;
    if (!data.ReadBool(dialogResult)) {
        HILOGE("dialogResult failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    int32_t result = NotifyDialogResult(dialogType, dialogResult);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::IsProfileExistInner(MessageParcel &data, MessageParcel &reply)
{
    std::string profileName;
    CHECK_AND_RETURN_LOG_RET(data.ReadString(profileName), BT_ERR_INTERNAL_ERROR, "Read profileName failed");
    bool isProfileExist = false;
    int res = IsProfileExist(profileName, isProfileExist);
    if (!reply.WriteInt32(res)) {
        HILOGE("reply writing failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (res != BT_NO_ERROR) {
        return res;
    }
    if (!reply.WriteBool(isProfileExist)) {
        HILOGE("reply writing failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::SetCallingPackageNameInner(MessageParcel &data, MessageParcel &reply)
{
    std::string address;
    CHECK_AND_RETURN_LOG_RET(data.ReadString(address), BT_ERR_IPC_TRANS_FAILED, "Read address failed");
    CHECK_AND_RETURN_LOG_RET(IsValidAddress(address), BT_ERR_INVALID_PARAM, "Invalid address");
    std::string packageName;
    CHECK_AND_RETURN_LOG_RET(data.ReadString(packageName), BT_ERR_IPC_TRANS_FAILED, "Read action failed");
    static const int32_t pkgNameMaxLength = 1024;
    if (packageName.size() > pkgNameMaxLength) {
        packageName = packageName.substr(0, pkgNameMaxLength);
    }

    SetCallingPackageName(address, packageName);
    return BT_NO_ERROR;
}

int32_t BluetoothHostStub::StartRemoteSdpSearchInner(MessageParcel &data, MessageParcel &reply)
{
    std::string address;
    CHECK_AND_RETURN_LOG_RET(data.ReadString(address), BT_ERR_IPC_TRANS_FAILED, "Read address failed");
    CHECK_AND_RETURN_LOG_RET(IsValidAddress(address), BT_ERR_INVALID_PARAM, "Invalid address");
    std::string uuid;
    CHECK_AND_RETURN_LOG_RET(data.ReadString(uuid), BT_ERR_IPC_TRANS_FAILED, "Read uuid failed");
    CHECK_AND_RETURN_LOG_RET(IsValidUuid(uuid), BT_ERR_INVALID_PARAM, "Invalid uuid");
    int32_t result = StartRemoteSdpSearch(address, uuid);
    bool ret = reply.WriteInt32(result);
    CHECK_AND_RETURN_LOG_RET(ret, BT_ERR_IPC_TRANS_FAILED, "reply writing failed.");
    return NO_ERROR;
}

int32_t BluetoothHostStub::GetRemoteServicesInner(MessageParcel &data, MessageParcel &reply)
{
    std::string address;
    CHECK_AND_RETURN_LOG_RET(data.ReadString(address), BT_ERR_IPC_TRANS_FAILED, "Read address failed");
    CHECK_AND_RETURN_LOG_RET(IsValidAddress(address), BT_ERR_INVALID_PARAM, "Invalid address");
    int32_t result = GetRemoteServices(address);
    bool ret = reply.WriteInt32(result);
    CHECK_AND_RETURN_LOG_RET(ret, BT_ERR_IPC_TRANS_FAILED, "reply writing failed.");
    return NO_ERROR;
}

int32_t BluetoothHostStub::GetVirtualAddressByHashInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t hashAlgorithmType = INVALID_TYPE;
    std::string hashValue;
    if (!data.ReadInt32(hashAlgorithmType)) {
        HILOGE("read hashAlgorithmType failed");
        return BT_ERR_INTERNAL_ERROR;
    }
    CHECK_AND_RETURN_LOG_RET(IsValidHashAlgorithmType(hashAlgorithmType),
        BT_ERR_PARAM_NOT_COMPLIANT, "Invalid hash type");
    if (!data.ReadString(hashValue)) {
        HILOGE("read hashValue failed");
        return BT_ERR_INTERNAL_ERROR;
    }
    CHECK_AND_RETURN_LOG_RET(IsValidHashValue(hashValue), BT_ERR_PARAM_NOT_COMPLIANT, "Invalid hash value");
    std::transform(hashValue.begin(), hashValue.end(), hashValue.begin(),
        [](unsigned char c) {return std::tolower(c);});
    std::string virtualAddress;
    int32_t ret = GetVirtualAddressByHash(hashAlgorithmType, hashValue, virtualAddress);
    if (!reply.WriteInt32(ret)) {
        HILOGE("Write ret failed");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (ret == BT_NO_ERROR) {
        if (!reply.WriteString(virtualAddress)) {
            HILOGE("Write addr failed");
            return BT_ERR_INTERNAL_ERROR;
        }
    }
    return ret;
}

int32_t BluetoothHostStub::SetConnectionPriorityInner(MessageParcel &data, MessageParcel &reply)
{
    std::string address;
    CHECK_AND_RETURN_LOG_RET(data.ReadString(address), BT_ERR_IPC_TRANS_FAILED, "Read address failed");
    CHECK_AND_RETURN_LOG_RET(IsValidAddress(address), BT_ERR_INVALID_PARAM, "Invalid address");
    int32_t priority;
    CHECK_AND_RETURN_LOG_RET(data.ReadInt32(priority), BT_ERR_IPC_TRANS_FAILED, "Read priority failed");
    int32_t result = SetConnectionPriority(address, priority);
    bool ret = reply.WriteInt32(result);
    CHECK_AND_RETURN_LOG_RET(ret, BT_ERR_IPC_TRANS_FAILED, "reply writing failed.");
    return NO_ERROR;
}

int32_t BluetoothHostStub::VerifyMultiPermissionsInner(MessageParcel &data, MessageParcel &reply)
{
    bool systemHapNeeded = false;
    CHECK_AND_RETURN_LOG_RET(data.ReadBool(systemHapNeeded), BT_ERR_IPC_TRANS_FAILED, "Read systemHapNeeded failed.");
    int32_t size = 0;
    CHECK_AND_RETURN_LOG_RET(data.ReadInt32(size), BT_ERR_IPC_TRANS_FAILED, "Read size failed.");
    if (size > MAX_PERMISSION_SIZE) {
        HILOGE("size(%{public}d) exceeds max size", size);
        return BT_ERR_INVALID_PARAM;
    }
    std::set<std::string> permissionSet;
    for (int32_t i = 0; i < size; i++) {
        std::string permission;
        CHECK_AND_RETURN_LOG_RET(data.ReadString(permission), BT_ERR_IPC_TRANS_FAILED, "Read permission failed.");
        permissionSet.insert(permission);
    }
    int32_t ret = VerifyMultiPermissions(systemHapNeeded, permissionSet);
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(ret), BT_ERR_IPC_TRANS_FAILED, "reply writing failed.");
    return ret;
}
}  // namespace Bluetooth
}  // namespace OHOS
