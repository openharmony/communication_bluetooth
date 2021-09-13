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

#ifndef BLE_MOCK_H
#define BLE_MOCK_H
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "interface_adapter.h"
#include "interface_profile_gatt_client.h"
#include "ble_adapter.h"
#include "common_mock_all.h"

class BleMock {
public:
    MOCK_METHOD0(BTM_Enable, int());
    MOCK_METHOD0(BTM_Disable, int());
    MOCK_METHOD0(BTM_RegisterAclCallbacks, int());
    MOCK_METHOD0(BTM_DeregisterAclCallbacks, int());
    MOCK_METHOD0(BTM_IsControllerSupportLeExtendedAdvertising, bool());
    MOCK_METHOD0(SDP_CreateServiceRecord, int());
    MOCK_METHOD0(BTM_IsControllerSupportLeDataPacketLengthExtension, bool());
    MOCK_METHOD0(SDP_AddServiceClassIdList, int());
    MOCK_METHOD0(SDP_AddAttribute, int());
    MOCK_METHOD0(SDP_RegisterServiceRecord, int());
    MOCK_METHOD0(SDP_DeregisterServiceRecord, int());
    MOCK_METHOD0(SDP_DestroyServiceRecord, int());
    MOCK_METHOD0(SDP_ServiceSearchAttribute, int());
    MOCK_METHOD0(GAPIF_RegisterDiscoveryCallback, int());
    MOCK_METHOD0(GAPIF_RegisterAuthenticationCallback, int());
    MOCK_METHOD0(GAPIF_RegisterSecurityCallback, int());
    MOCK_METHOD0(GAPIF_UserConfirmRsp, int());
    MOCK_METHOD0(GAPIF_UserPasskeyRsp, int());
    MOCK_METHOD0(GAPIF_PinCodeRsp, int());
    MOCK_METHOD0(GAPIF_SetScanMode, int());
    MOCK_METHOD0(GAPIF_SetClassOfDevice, int());
    MOCK_METHOD0(GAPIF_GetRemoteName, int());
    MOCK_METHOD0(GAPIF_LeSetRole, int());
    MOCK_METHOD0(GAPIF_RegisterAdvCallback, int());
    MOCK_METHOD0(GAPIF_LeAdvSetParam, int());
    MOCK_METHOD0(GAPIF_LeAdvReadTxPower, int());
    MOCK_METHOD0(GAPIF_LeAdvSetData, int());
    MOCK_METHOD0(GAPIF_LeAdvSetScanRspData, int());
    MOCK_METHOD0(GAPIF_LeAdvSetEnable, int());
    MOCK_METHOD0(GAPIF_LeScanSetParam, int());
    MOCK_METHOD0(GAPIF_LeScanSetEnable, int());
    MOCK_METHOD0(GAPIF_RegisterScanCallback, int());
    MOCK_METHOD0(GAPIF_LeSetBondMode, int());
    MOCK_METHOD0(GAPIF_LeExAdvGetMaxDataLen, int());
    MOCK_METHOD0(GAPIF_LeSetSecurityMode, int());
    MOCK_METHOD0(GAPIF_LeSetMinEncKeySize, int());
    MOCK_METHOD0(GAPIF_LeCancelPair, int());
    MOCK_METHOD0(GAPIF_RegisterExAdvCallback, int());
    MOCK_METHOD0(GAPIF_LeExAdvSetRandAddr, int());
    MOCK_METHOD0(GAPIF_LeExAdvSetEnable, int());
    MOCK_METHOD0(GAPIF_LeExAdvSetParam, int());
    MOCK_METHOD0(GAPIF_LeExAdvSetData, int());
    MOCK_METHOD0(GAPIF_LeExAdvSetScanRspData, int());
    MOCK_METHOD0(GAPIF_RegisterExScanCallback, int());
    MOCK_METHOD0(GAPIF_LeExScanSetParam, int());
    MOCK_METHOD0(GAPIF_LeExScanSetEnable, int());

    MOCK_METHOD0(AdapterManager_GetState, bluetooth::BTStateID());
    MOCK_METHOD0(AdapterManager_GetAdapter, bluetooth::IAdapter *());

    MOCK_METHOD0(GattServerService_RemoveService, int());
    MOCK_METHOD0(GattServerService_RegisterApplication, int());
};

class XmlMock {
public:
    MOCK_METHOD0(GetValue, int());
};

class BleAdvertiserCallback_MOCK : public bluetooth::IBleAdvertiserCallback {
public:
    MOCK_METHOD3(OnStartResultEvent, void(int result, uint8_t advHandle, int opcode));
    MOCK_METHOD1(OnAutoStopAdvEvent, void(uint8_t advHandle));
};

class BleCentralManagerCallback_MOCK : public bluetooth::IBleCentralManagerCallback {
public:
    MOCK_METHOD1(OnScanCallback, void(const bluetooth::BleScanResultImpl &result));
    MOCK_METHOD1(OnBleBatchScanResultsEvent, void(std::vector<bluetooth::BleScanResultImpl> &results));
    MOCK_METHOD1(OnStartScanFailed, void(int resultCode));
};

class AdapterBleObserver_MOCK : public bluetooth::IAdapterBleObserver {
public:
    ~AdapterBleObserver_MOCK()
    {}
    MOCK_METHOD1(OnDiscoveryStateChanged, void(int status));
    MOCK_METHOD1(OnDiscoveryResult, void(const bluetooth::RawAddress &device));
    MOCK_METHOD2(OnPairRequested, void(const bluetooth::BTTransport transport, const bluetooth::RawAddress &device));
    MOCK_METHOD4(OnPairConfirmed,
        void(const bluetooth::BTTransport transport, const bluetooth::RawAddress &device, int reqtype, int number));
    MOCK_METHOD1(OnScanModeChanged, void(int mode));
    MOCK_METHOD1(OnDeviceNameChanged, void(const std::string deviceName));
    MOCK_METHOD1(OnDeviceAddrChanged, void(const std::string address));
    MOCK_METHOD1(OnAdvertisingStateChanged, void(const int state));
};

class BlePeripheralCallback_MOCK : public bluetooth::IBlePeripheralCallback {
public:
    ~BlePeripheralCallback_MOCK()
    {}
    MOCK_METHOD3(OnReadRemoteRssiEvent, void(const bluetooth::RawAddress &device, int rssi, int status));
    MOCK_METHOD3(OnPairStatusChanged,
        void(const bluetooth::BTTransport transport, const bluetooth::RawAddress &device, int status));
};

class IContextCallbackMock : public utility::IContextCallback {
public:
    ~IContextCallbackMock() = default;
    MOCK_METHOD2(OnEnable, void(const std::string &name, bool ret));
    MOCK_METHOD2(OnDisable, void(const std::string &name, bool ret));
};

class GattClientMock : public bluetooth::IProfileGattClient {
public:
    int RegisterApplication(
        bluetooth::IGattClientCallback &callback, const bluetooth::RawAddress &addr, uint8_t transport)
    {
        return BT_NO_ERROR;
    }
    int RegisterSharedApplication(
        bluetooth::IGattClientCallback &callback, const bluetooth::RawAddress &addr, uint8_t transport)
    {
        return 1;
    }
    int DeregisterApplication(int appId)
    {
        return BT_NO_ERROR;
    }
    int Connect(int appId, bool autoConnect)
    {
        return BT_NO_ERROR;
    }
    int Disconnect(int appId)
    {
        return BT_NO_ERROR;
    }
    int DiscoveryServices(int appId)
    {
        return BT_NO_ERROR;
    }
    int ReadCharacteristic(int appId, const bluetooth::Characteristic &characteristic)
    {
        return BT_NO_ERROR;
    }
    int ReadCharacteristicByUuid(int appId, const bluetooth::Uuid &uuid)
    {
        return BT_NO_ERROR;
    }
    int WriteCharacteristic(int appId, bluetooth::Characteristic &characteristic, bool withoutRespond = false)
    {
        return BT_NO_ERROR;
    }
    int SignedWriteCharacteristic(int appId, bluetooth::Characteristic &characteristic)
    {
        return BT_NO_ERROR;
    }
    int ReadDescriptor(int appId, const bluetooth::Descriptor &descriptor)
    {
        return BT_NO_ERROR;
    }
    int WriteDescriptor(int appId, bluetooth::Descriptor &descriptor)
    {
        return BT_NO_ERROR;
    }
    int RequestExchangeMtu(int appId, int mtu)
    {
        return BT_NO_ERROR;
    }
    std::vector<bluetooth::GattDevice> GetAllDevice()
    {
        return {};
    }
    int RequestConnectionPriority(int appId, int connPriority)
    {
        return BT_NO_ERROR;
    }
    std::vector<bluetooth::Service> GetServices(int appId)
    {
        return {};
    }
    int Connect(const bluetooth::RawAddress &device)
    {
        return BT_NO_ERROR;
    }
    int Disconnect(const bluetooth::RawAddress &device)
    {
        return BT_NO_ERROR;
    }
    std::list<bluetooth::RawAddress> GetConnectDevices()
    {
        std::list<bluetooth::RawAddress> list{};
        return list;
    }
    int GetConnectState()
    {
        return BT_NO_ERROR;
    }
    int GetMaxConnectNum()
    {
        return BT_NO_ERROR;
    }
    utility::Context *GetContext()
    {
        return nullptr;
    }
};
BtmAclCallbacks *GetBtmCallbacks();
GapAdvCallback *GetAdvCallback();
GapScanCallback *GetScanCallback();
GapLeSecurityCallback *GetLeSecurityCallback();
GapLePairCallback *GetLePairCallback();
GapExScanCallback *GetExScanCallback();
GapExAdvCallback *GetExAdvCallback();
GenResPriAddrResult *GetGenRPACallback();
BleCentralManagerImpl *GetBleCentraManager();
BleAdvertiserImpl *GetBleAdvertiser();
BleSecurity *GetBleSecurity();
void registerBtmMock(BleMock *instance);
void registerGapMock(BleMock *instance);
void registerSdpMock(BleMock *instance);
void registerXmlMock(XmlMock *instance);
#endif