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

#include "ohos_bt_gatt_server.h"

#include <iostream>
#include <string.h>
#include <vector>
#include <map>
#include <mutex>
#include <algorithm>

#include "ohos_bt_adapter_utils.h"
#include "bluetooth_gatt_server.h"
#include "bluetooth_log.h"
#include "bluetooth_utils.h"

#include "securec.h"

#ifdef __cplusplus
extern "C" {
#endif

using namespace std;

namespace OHOS {
namespace Bluetooth {
static BtGattServerCallbacks *g_GattsCallback;

struct ConnectedDevice {
    int serverId;
    BdAddr remoteAddr;

    bool operator==(const ConnectedDevice& device) const
    {
        if (serverId == device.serverId &&
            memcmp(remoteAddr.addr, device.remoteAddr.addr, sizeof(remoteAddr.addr)) == 0) {
            return true;
        }
        return false;
    }
};
static int g_connId = 0;

static std::map<int, struct ConnectedDevice> g_MapConnectedDevice;
static std::mutex g_mapConnDeviceMutex;

#define MAXIMUM_NUMBER_APPLICATION 64
#define MAXIMUM_NUMBER_GATTSERVICE 64
struct GattServiceWapper {
    GattService *gattService;
    int index;
    int maxNum;
    int handleOffset;
    bool isAdding;
};

struct GattServerWapper {
    std::shared_ptr<GattServer> gattServer = nullptr;
    struct GattServiceWapper gattServices[MAXIMUM_NUMBER_GATTSERVICE];
};

GattServerWapper g_gattServers[MAXIMUM_NUMBER_APPLICATION];

#define GATTSERVER(x) g_gattServers[x].gattServer
#define GATTSERVICES(x, y) g_gattServers[x].gattServices[y]
#define GATTSERVICE(x, y) GATTSERVICES(x, y).gattService

static GattCharacteristic *FindCharacteristic(int serverId, int attrHandle, bool isOffset, int *srvcHandle);
static int AddDeviceRecord(struct ConnectedDevice &device);
static void RemoveDeviceRecord(int connId);
static std::optional<ConnectedDevice> GetDeviceInfoByConnId(int connId);
static std::optional<int> GetConnIdByDeviceInfo(struct ConnectedDevice &device);

class GattServerCallbackWapper : public GattServerCallback {
public:
    GattServerCallbackWapper(BtGattServerCallbacks *callback, int serverId)
    {
        appCallback_ = callback;
        serverId_ = serverId;
    }

    void OnConnectionStateUpdate(const BluetoothRemoteDevice &device, int state) override
    {
        struct ConnectedDevice dev;
        dev.serverId = serverId_;
        GetAddrFromString(device.GetDeviceAddr(), dev.remoteAddr.addr);
        HILOGI("device: %{public}s, connect state: %{public}d", GET_ENCRYPT_ADDR(device), state);

        if (state == static_cast<int>(BTConnectState::CONNECTED)) {
            if (g_GattsCallback == nullptr || g_GattsCallback->connectServerCb == nullptr) {
                HILOGW("call back is null.");
                return;
            }

            int connId = AddDeviceRecord(dev);
            g_GattsCallback->connectServerCb(connId, serverId_, &dev.remoteAddr);
        }

        if (state == static_cast<int>(BTConnectState::DISCONNECTED)) {
            if (g_GattsCallback == nullptr || g_GattsCallback->disconnectServerCb == nullptr) {
                HILOGW("call back is null.");
                return;
            }
            std::optional<int> connId = GetConnIdByDeviceInfo(dev);
            if (connId.has_value()) {
                HILOGI("device disconnected. connId: %{public}d", connId.value());
                g_GattsCallback->disconnectServerCb(connId.value(), serverId_, &dev.remoteAddr);
                RemoveDeviceRecord(connId.value());
            }
        }
    }

    void OnServiceAdded(GattService *Service, int ret) override
    {
        int i;
        int err = OHOS_BT_STATUS_SUCCESS;
        if (serverId_ >= MAXIMUM_NUMBER_APPLICATION || serverId_ < 0) {
            HILOGE("serverId_ is invalid!");
            return;
        }
        for (i = 0; i < MAXIMUM_NUMBER_GATTSERVICE; i++) {
            if (GATTSERVICE(serverId_, i) != nullptr) {
                HILOGI("isAdding: %{public}d, srvcUuid: %{public}s, ind: %{public}s",
                    GATTSERVICES(serverId_, i).isAdding,
                    GATTSERVICE(serverId_, i)->GetUuid().ToString().c_str(),
                    Service->GetUuid().ToString().c_str());
            } else {
                HILOGE("services is empty!");
            }
            if (GATTSERVICE(serverId_, i) != nullptr &&
                GATTSERVICES(serverId_, i).isAdding &&
                GATTSERVICE(serverId_, i)->GetUuid().CompareTo(Service->GetUuid()) == 0) {
                GATTSERVICES(serverId_, i).isAdding = false;
                GATTSERVICES(serverId_, i).handleOffset = Service->GetHandle() - i;
                HILOGI("serverId: %{public}d, service handle is: %{public}d, offset: %{public}d",
                    serverId_, GATTSERVICE(serverId_, i)->GetHandle(), GATTSERVICES(serverId_, i).handleOffset);
                break;
            }
        }

        if (i == MAXIMUM_NUMBER_GATTSERVICE) {
            HILOGE("add service failed, invalid srvcHandle: %{public}u", Service->GetHandle());
            err = OHOS_BT_STATUS_FAIL;
        }

        vector<GattCharacteristic> &characteristics = Service->GetCharacteristics();
        for (auto item = characteristics.begin(); item != characteristics.end(); item++) {
            HILOGI("charHandle: %{public}d, uuid: %{public}s",
                item->GetHandle(), item->GetUuid().ToString().c_str());
            vector<GattDescriptor> &descriptors = item->GetDescriptors();
            for (auto des = descriptors.begin(); des != descriptors.end(); des++) {
                HILOGI("desHandle: %{public}d, uuid: %{public}s", des->GetHandle(), des->GetUuid().ToString().c_str());
            }
        }

        if (g_GattsCallback != nullptr && g_GattsCallback->serviceStartCb != nullptr) {
            g_GattsCallback->serviceStartCb(err, serverId_, i);
        } else {
            HILOGW("call back is null.");
        }
    }

    void OnCharacteristicReadRequest(
        const BluetoothRemoteDevice &device, GattCharacteristic &characteristic, int requestId) override
    {
        if (serverId_ >= MAXIMUM_NUMBER_APPLICATION || serverId_ < 0) {
            HILOGE("serverId_ is invalid!");
            return;
        }

        struct ConnectedDevice dev;
        dev.serverId = serverId_;
        GetAddrFromString(device.GetDeviceAddr(), dev.remoteAddr.addr);

        std::optional<int> connId = GetConnIdByDeviceInfo(dev);
        if (!connId.has_value()) {
            HILOGW("device is not exist.");
            return;
        }

        int srvcHandle = 0;
        FindCharacteristic(serverId_, characteristic.GetHandle(), true, &srvcHandle);

        BtReqReadCbPara readInfo;
        readInfo.connId = connId.value();
        readInfo.transId = requestId;
        readInfo.bdAddr = &dev.remoteAddr;
        readInfo.attrHandle = characteristic.GetHandle() - GATTSERVICES(serverId_, srvcHandle).handleOffset;
        readInfo.offset = 0;
        readInfo.isLong = false;
        HILOGI("connId: %{public}d, requestId: %{public}d, attrHandle: %{public}d",
            connId.value(), requestId, readInfo.attrHandle);
        if (g_GattsCallback != nullptr && g_GattsCallback->requestReadCb != nullptr) {
            g_GattsCallback->requestReadCb(readInfo);
        } else {
            HILOGW("call back is null.");
        }
    }

    void OnCharacteristicWriteRequest(
        const BluetoothRemoteDevice &device, GattCharacteristic &characteristic, int requestId) override
    {
        if (serverId_ >= MAXIMUM_NUMBER_APPLICATION || serverId_ < 0) {
            HILOGE("serverId_ is invalid!");
            return;
        }
        struct ConnectedDevice dev;
        dev.serverId = serverId_;
        GetAddrFromString(device.GetDeviceAddr(), dev.remoteAddr.addr);

        std::optional<int> connId = GetConnIdByDeviceInfo(dev);
        if (!connId.has_value()) {
            HILOGW("device is not exist.");
            return;
        }

        int srvcHandle = 0;
        FindCharacteristic(serverId_, characteristic.GetHandle(), true, &srvcHandle);

        BtReqWriteCbPara writeInfo;
        size_t length = 0;
        writeInfo.connId = connId.value();
        writeInfo.transId = requestId;
        writeInfo.bdAddr = &dev.remoteAddr;
        writeInfo.attrHandle = characteristic.GetHandle() - GATTSERVICES(serverId_, srvcHandle).handleOffset;
        writeInfo.offset = 0;
        writeInfo.value = characteristic.GetValue(&length).get();
        writeInfo.length = length;
        writeInfo.needRsp = (characteristic.GetWriteType() == GattCharacteristic::WriteType::DEFAULT);
        writeInfo.isPrep = false;
        HILOGI("connId: %{public}d, requestId: %{public}d, attrHandle: %{public}d, valueLen: %{public}d",
            connId.value(), requestId, writeInfo.attrHandle, writeInfo.length);
        if (g_GattsCallback != nullptr && g_GattsCallback->requestWriteCb != nullptr) {
            g_GattsCallback->requestWriteCb(writeInfo);
        } else {
            HILOGW("call back is null.");
        }
    }

    void OnDescriptorReadRequest(
        const BluetoothRemoteDevice &device, GattDescriptor &descriptor, int requestId) override
    {
        if (serverId_ >= MAXIMUM_NUMBER_APPLICATION || serverId_ < 0) {
            HILOGE("serverId_ is invalid!");
            return;
        }
        struct ConnectedDevice dev;
        dev.serverId = serverId_;
        GetAddrFromString(device.GetDeviceAddr(), dev.remoteAddr.addr);

        std::optional<int> connId = GetConnIdByDeviceInfo(dev);
        if (!connId.has_value()) {
            HILOGW("device is not exist.");
            return;
        }

        const GattCharacteristic *characteristic = descriptor.GetCharacteristic();
        int srvcHandle = 0;
        FindCharacteristic(serverId_, characteristic->GetHandle(), true, &srvcHandle);

        BtReqReadCbPara readInfo;
        readInfo.connId = connId.value();
        readInfo.transId = requestId;
        readInfo.bdAddr = &dev.remoteAddr;
        readInfo.attrHandle = descriptor.GetHandle() - GATTSERVICES(serverId_, srvcHandle).handleOffset;
        readInfo.offset = 0;
        readInfo.isLong = false;
        if (g_GattsCallback != nullptr && g_GattsCallback->requestReadCb != nullptr) {
            g_GattsCallback->requestReadCb(readInfo);
        } else {
            HILOGW("call back is null.");
        }
    }

    void OnDescriptorWriteRequest(
        const BluetoothRemoteDevice &device, GattDescriptor &descriptor, int requestId) override
    {
        if (serverId_ >= MAXIMUM_NUMBER_APPLICATION || serverId_ < 0) {
            HILOGE("serverId_ is invalid!");
            return;
        }
        struct ConnectedDevice dev;
        dev.serverId = serverId_;
        GetAddrFromString(device.GetDeviceAddr(), dev.remoteAddr.addr);

        std::optional<int> connId = GetConnIdByDeviceInfo(dev);
        if (!connId.has_value()) {
            HILOGW("device is not exist.");
            return;
        }

        const GattCharacteristic *characteristic = descriptor.GetCharacteristic();
        int srvcHandle = 0;
        FindCharacteristic(serverId_, characteristic->GetHandle(), true, &srvcHandle);

        BtReqWriteCbPara writeInfo;
        size_t length = 0;
        writeInfo.connId = connId.value();
        writeInfo.transId = requestId;
        writeInfo.bdAddr = &dev.remoteAddr;
        writeInfo.attrHandle = descriptor.GetHandle() - GATTSERVICES(serverId_, srvcHandle).handleOffset;
        writeInfo.offset = 0;
        writeInfo.value = descriptor.GetValue(&length).get();
        writeInfo.length = length;
        writeInfo.needRsp = true;
        writeInfo.isPrep = false;
        if (g_GattsCallback != nullptr && g_GattsCallback->requestWriteCb != nullptr) {
            g_GattsCallback->requestWriteCb(writeInfo);
        } else {
            HILOGW("call back is null.");
        }
    }

    void OnMtuUpdate(const BluetoothRemoteDevice &device, int mtu) override
    {
        HILOGI("mtu: %{public}d", mtu);
        struct ConnectedDevice dev;
        dev.serverId = serverId_;
        GetAddrFromString(device.GetDeviceAddr(), dev.remoteAddr.addr);

        std::optional<int> connId = GetConnIdByDeviceInfo(dev);
        if (!connId.has_value()) {
            HILOGW("device is not exist.");
            return;
        }
        if (g_GattsCallback != nullptr && g_GattsCallback->mtuChangeCb != nullptr) {
            g_GattsCallback->mtuChangeCb(connId.value(), mtu);
        } else {
            HILOGW("call back is null.");
        }
    }

    void OnNotificationCharacteristicChanged(const BluetoothRemoteDevice &device, int result) override
    {
        HILOGI("result: %{public}d", result);
        struct ConnectedDevice dev;
        dev.serverId = serverId_;
        GetAddrFromString(device.GetDeviceAddr(), dev.remoteAddr.addr);

        std::optional<int> connId = GetConnIdByDeviceInfo(dev);
        if (!connId.has_value()) {
            HILOGW("device is not exist.");
            return;
        }
        if (g_GattsCallback != nullptr && g_GattsCallback->indicationSentCb != nullptr) {
            g_GattsCallback->indicationSentCb(connId.value(), result);
        } else {
            HILOGW("call back is null.");
        }
    }

    void OnConnectionParameterChanged(
        const BluetoothRemoteDevice &device, int interval, int latency, int timeout, int status) override
    {
        HILOGI("enter");
    }

private:
    BtGattServerCallbacks *appCallback_;
    int serverId_;
};

static int AddDeviceRecord(struct ConnectedDevice &device)
{
    std::lock_guard<std::mutex> lock(g_mapConnDeviceMutex);
    std::map<int, struct ConnectedDevice>::iterator iter =
        std::find_if(g_MapConnectedDevice.begin(), g_MapConnectedDevice.end(),
        [&device](const std::pair<int, ConnectedDevice> &it)->bool { return it.second == device; });

    int connId;
    if (iter != g_MapConnectedDevice.end()) {
        connId = iter->first;
        HILOGW("device already in maps! connId: %{public}d", connId);
    } else {
        g_MapConnectedDevice.insert(std::pair<int, struct ConnectedDevice>(g_connId, device));
        connId = g_connId++;
        HILOGI("device connected. connId: %{public}d", connId);
    }
    return connId;
}

static void RemoveDeviceRecord(int connId)
{
    std::lock_guard<std::mutex> lock(g_mapConnDeviceMutex);
    g_MapConnectedDevice.erase(connId);
}

static std::optional<ConnectedDevice> GetDeviceInfoByConnId(int connId)
{
    std::lock_guard<std::mutex> lock(g_mapConnDeviceMutex);
    std::map<int, struct ConnectedDevice>::iterator iter = g_MapConnectedDevice.find(connId);
    if (iter == g_MapConnectedDevice.end()) {
        return std::nullopt;
    }
    return iter->second;
}

static std::optional<int> GetConnIdByDeviceInfo(struct ConnectedDevice &device)
{
    std::lock_guard<std::mutex> lock(g_mapConnDeviceMutex);
    std::map<int, struct ConnectedDevice>::iterator iter =
        std::find_if(g_MapConnectedDevice.begin(), g_MapConnectedDevice.end(),
        [&device](const std::pair<int, ConnectedDevice> &it)->bool { return it.second == device; });
    if (iter == g_MapConnectedDevice.end()) {
        return std::nullopt;
    }
    return iter->first;
}

static GattCharacteristic *FindCharacteristic(int serverId, int attrHandle, bool isOffset, int *srvcHandle)
{
    if (serverId >= MAXIMUM_NUMBER_APPLICATION || serverId < 0) {
        HILOGE("serverId is invalid!");
        return nullptr;
    }

    if (srvcHandle == nullptr) {
        HILOGE("srvcHandle is null!");
        return nullptr;
    }
    for (int i = 0; i < MAXIMUM_NUMBER_GATTSERVICE; i++) {
        GattService *gattService = GATTSERVICE(serverId, i);
        if (gattService == nullptr) {
            continue;
        }

        std::vector<GattCharacteristic> &gattCharacteristics = gattService->GetCharacteristics();

        for (auto &character : gattCharacteristics) {
            if (character.GetHandle() == attrHandle) {
                *srvcHandle = i;
                return &character;
            }
        }
    }
    return nullptr;
}

/**
 * @brief Registers a GATT server with a specified application UUID.
 *
 * The <b>RegisterServerCallback</b> is invoked to return the GATT server ID.
 *
 * @param appUuid Indicates the UUID of the application for which the GATT server is to be registered.
 * The UUID is defined by the application.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the GATT server is registered;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsRegister(BtUuid appUuid)
{
    HILOGI("enter");
    if (g_GattsCallback == nullptr) {
        HILOGE("callback is null, call BleGattsRegisterCallbacks first");
        return OHOS_BT_STATUS_FAIL;
    }
    for (int i = 0; i < MAXIMUM_NUMBER_APPLICATION; i++) {
        if (GATTSERVER(i) == nullptr) {
            GattServerCallbackWapper *callbackWapper = new GattServerCallbackWapper(g_GattsCallback, i);
            GATTSERVER(i) = GattServer::CreateInstance(*callbackWapper);
            HILOGD("register gattServer: %{public}d", i);
            if (g_GattsCallback->registerServerCb != nullptr) {
                g_GattsCallback->registerServerCb(0, i, &appUuid);
            }
            return OHOS_BT_STATUS_SUCCESS;
        }
    }

    if (g_GattsCallback->registerServerCb != nullptr) {
        g_GattsCallback->registerServerCb(1, 0, &appUuid);
    }
    return OHOS_BT_STATUS_FAIL;
}

/**
 * @brief Unregisters a GATT server with a specified ID.
 *
 * @param serverId Indicates the ID of the GATT server.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the GATT server is unregistered;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsUnRegister(int serverId)
{
    HILOGI("serverId: %{public}d", serverId);
    if (serverId >= 0 && serverId < MAXIMUM_NUMBER_APPLICATION) {
        if (GATTSERVER(serverId) != nullptr) {
            GATTSERVER(serverId) = nullptr;
            return OHOS_BT_STATUS_SUCCESS;
        }
    }

    return OHOS_BT_STATUS_FAIL;
}

/**
 * @brief Disconnects the GATT server from the client.
 *
 * @param serverId Indicates the ID of the GATT server.
 * @param bdAddr Indicates the address of the client.
 * @param connId Indicates the connection ID, which is returned during the server registration.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the GATT server is disconnected from the client;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsDisconnect(int serverId, BdAddr bdAddr, int connId)
{
    HILOGI("serverId: %{public}d, connId: %{public}d", serverId, connId);
    if (serverId >= MAXIMUM_NUMBER_APPLICATION || serverId < 0) {
        HILOGE("serverId is invalid!");
        return OHOS_BT_STATUS_PARM_INVALID;
    }

    if (GATTSERVER(serverId) == nullptr) {
        HILOGE("GATTSERVER(serverId) is null!");
        return OHOS_BT_STATUS_UNHANDLED;
    }

    string strAddress;
    GetAddrFromByte(bdAddr.addr, strAddress);
    BluetoothRemoteDevice device(strAddress, BT_TRANSPORT_BLE);

    GATTSERVER(serverId)->CancelConnection(device);
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Adds a service.
 *
 * This function adds the service, its characteristics, and descriptors separately in sequence.\n
 * A service is a collection of data and related behavior that enable a specific capability or feature.\n
 * It consists of a service declaration and one or more included services and characteristics.
 *
 * @param serverId Indicates the ID of the GATT server.
 * @param srvcUuid Indicates the UUID of the service.
 * @param isPrimary Specifies whether the service is primary or secondary.
 * Value <b>true</b> indicates that the service is primary, and <b>false</b> indicates that the service is secondary.
 * @param number Indicates the number of attribute handles.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the service is added;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsAddService(int serverId, BtUuid srvcUuid, bool isPrimary, int number)
{
    HILOGD("enter");
    string strUuid(srvcUuid.uuid);
    if (!regex_match(strUuid, uuidRegex)) {
        HILOGE("match the UUID faild.");
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    UUID uuid(UUID::FromString(strUuid));

    if (serverId >= MAXIMUM_NUMBER_APPLICATION || serverId < 0) {
        HILOGE("serverId is invalid!");
        return OHOS_BT_STATUS_PARM_INVALID;
    }

    for (int i = 0; i < MAXIMUM_NUMBER_GATTSERVICE; i++) {
        if (GATTSERVICE(serverId, i) == nullptr) {
            HILOGD("add srvcHandle: %{public}d", i);
            GATTSERVICE(serverId, i) = new GattService(
                uuid, i, number, isPrimary ? GattServiceType::PRIMARY : GattServiceType::SECONDARY);
            GATTSERVICES(serverId, i).maxNum = number;
            GATTSERVICES(serverId, i).index = i + 1;
            GATTSERVICES(serverId, i).isAdding = false;
            if (g_GattsCallback != nullptr && g_GattsCallback->serviceAddCb != nullptr) {
                g_GattsCallback->serviceAddCb(0, serverId, &srvcUuid, i);
            } else {
                HILOGW("call back is null");
            }
            return OHOS_BT_STATUS_SUCCESS;
        }
    }
    return OHOS_BT_STATUS_FAIL;
}

/**
 * @brief Adds an included service to a specified service.
 *
 * An included service is referenced to define another service on the GATT server.
 *
 * @param serverId Indicates the ID of the GATT server.
 * @param srvcHandle Indicates the handle ID of the service.
 * @param includedHandle Indicates the attribute handle ID of the included service.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the included service is added to the service;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsAddIncludedService(int serverId, int srvcHandle, int includedHandle)
{
    return OHOS_BT_STATUS_UNSUPPORTED;
}

/**
 * @brief Adds a characteristic to a specified service.
 *
 * A characteristic consists of data, the data access method, data format, and how the data manifests itself.
 *
 * @param serverId Indicates the ID of the GATT server.
 * @param srvcHandle Indicates the handle ID of the service.
 * @param characUuid Indicates the UUID of the characteristic to add.
 * @param properties Indicates the access methods supported by the characteristic,
 * as enumerated in {@link GattCharacteristicProperty}.
 * @param permissions Indicates the access permissions supported by the characteristic,
 * as enumerated in {@link GattAttributePermission}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the characteristic is added to the service;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsAddCharacteristic(int serverId, int srvcHandle, BtUuid characUuid,
                              int properties, int permissions)
{
    HILOGD("properties: %{public}d, permissions:%{public}d", properties, permissions);

    if (serverId >= MAXIMUM_NUMBER_APPLICATION || serverId < 0 ||
        srvcHandle >= MAXIMUM_NUMBER_GATTSERVICE || srvcHandle < 0) {
        HILOGE("serverId srvcHandle is invalid!");
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    string strUuid(characUuid.uuid);
    if (!regex_match(strUuid, uuidRegex)) {
        HILOGE("match the UUID faild.");
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    UUID uuid(UUID::FromString(strUuid));

    int chHandle = GATTSERVICES(serverId, srvcHandle).index;
    GATTSERVICES(serverId, srvcHandle).index += 2;
    GattCharacteristic characteristic(uuid, chHandle, permissions, properties);

    unsigned char stubValue[1] = {0x31};
    characteristic.SetValue(stubValue, sizeof(stubValue));

    if (GATTSERVICE(serverId, srvcHandle) == nullptr) {
        HILOGE("GATTSERVICE(serverId, srvcHandle) is null!");
        return OHOS_BT_STATUS_UNHANDLED;
    }
    GATTSERVICE(serverId, srvcHandle)->AddCharacteristic(characteristic);

    HILOGD("serverId: %{public}d, srvcHandle: %{public}d, charHandle: %{public}d", serverId, srvcHandle, chHandle);
    if (g_GattsCallback != nullptr && g_GattsCallback->characteristicAddCb != nullptr) {
        g_GattsCallback->characteristicAddCb(0, serverId, &characUuid, srvcHandle, chHandle);
    } else {
        HILOGW("callback is null.");
    }
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Adds a descriptor to a specified characteristic.
 *
 * A descriptor contains the description, configuration, and format of a characteristic.
 *
 * @param serverId Indicates the ID of the GATT server.
 * @param srvcHandle Indicates the handle ID of the service to which the characteristic belongs.
 * @param descUuid Indicates the UUID of the descriptor to add.
 * @param permissions Indicates the access permissions supported by the descriptor,
 * as enumerated in {@link GattAttributePermission}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the descriptor is added to the characteristic;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsAddDescriptor(int serverId, int srvcHandle, BtUuid descUuid, int permissions)
{
    if (serverId >= MAXIMUM_NUMBER_APPLICATION || serverId < 0 ||
        srvcHandle >= MAXIMUM_NUMBER_GATTSERVICE || srvcHandle < 0) {
        HILOGE("serverId srvcHandle is invalid!");
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    string strUuid(descUuid.uuid);
    HILOGD("descUuid: %{public}s", strUuid.c_str());
    if (!regex_match(strUuid, uuidRegex)) {
        HILOGE("match the UUID faild.");
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    UUID uuid(UUID::FromString(strUuid));

    if (GATTSERVICE(serverId, srvcHandle) == nullptr) {
        HILOGE("GATTSERVICE(serverId, srvcHandle) is null!");
        return OHOS_BT_STATUS_UNHANDLED;
    }
    GattCharacteristic &characteristic = GATTSERVICE(serverId, srvcHandle)->GetCharacteristics().back();
    int desHandle = GATTSERVICES(serverId, srvcHandle).index++;
    GattDescriptor descriptor(uuid, desHandle, permissions);

    unsigned char stubValue[2] = {0x01, 0x00};
    descriptor.SetValue(stubValue, sizeof(stubValue));

    characteristic.AddDescriptor(descriptor);
    HILOGD("serverId: %{public}d, srvcHandle: %{public}d, desHandle: %{public}d", serverId, srvcHandle, desHandle);
    if (g_GattsCallback != nullptr && g_GattsCallback->descriptorAddCb != nullptr) {
        g_GattsCallback->descriptorAddCb(0, serverId, &descUuid, srvcHandle, desHandle);
    } else {
        HILOGW("callback is null.");
    }

    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Starts a service.
 *
 * @param serverId Indicates the ID of the GATT server.
 * @param srvcHandle Indicates the handle ID of the service.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the service is started;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsStartService(int serverId, int srvcHandle)
{
    HILOGD("serverId: %{public}d, srvcHandle: %{public}d", serverId, srvcHandle);

    if (serverId >= MAXIMUM_NUMBER_APPLICATION || serverId < 0 ||
        srvcHandle >= MAXIMUM_NUMBER_GATTSERVICE || srvcHandle < 0) {
        HILOGE("serverId srvcHandle is invalid!");
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    if (GATTSERVER(serverId) == nullptr) {
        HILOGE("GATTSERVER(serverId) is null!");
        return OHOS_BT_STATUS_UNHANDLED;
    }
    GATTSERVICES(serverId, srvcHandle).isAdding = true;
    GATTSERVER(serverId)->AddService(*GATTSERVICE(serverId, srvcHandle));
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Stops a service.
 *
 * @param serverId Indicates the ID of the GATT server.
 * @param srvcHandle Indicates the handle ID of the service.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the service is stopped;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsStopService(int serverId, int srvcHandle)
{
    HILOGI("serverId: %{public}d, srvcHandle: %{public}d", serverId, srvcHandle);

    if (serverId >= MAXIMUM_NUMBER_APPLICATION || serverId < 0 ||
        srvcHandle >= MAXIMUM_NUMBER_GATTSERVICE || srvcHandle < 0) {
        HILOGE("serverId srvcHandle is invalid!");
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    if (GATTSERVER(serverId) == nullptr || GATTSERVICE(serverId, srvcHandle) == nullptr) {
        HILOGE("param is null!");
        return OHOS_BT_STATUS_UNHANDLED;
    }

    GATTSERVICES(serverId, srvcHandle).isAdding = false;
    GATTSERVER(serverId)->RemoveGattService(*GATTSERVICE(serverId, srvcHandle));
    if (g_GattsCallback != nullptr && g_GattsCallback->serviceStopCb != nullptr) {
        g_GattsCallback->serviceStopCb(OHOS_BT_STATUS_SUCCESS, serverId, srvcHandle);
    } else {
        HILOGW("callback is null.");
    }
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Deletes a service.
 *
 * @param serverId Indicates the ID of the GATT server.
 * @param srvcHandle Indicates the handle ID of the service.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the service is deleted;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsDeleteService(int serverId, int srvcHandle)
{
    HILOGI("serverId: %{public}d, srvcHandle: %{public}d", serverId, srvcHandle);

    if (serverId >= MAXIMUM_NUMBER_APPLICATION || serverId < 0 ||
        srvcHandle >= MAXIMUM_NUMBER_GATTSERVICE || srvcHandle < 0) {
        HILOGE("serverId srvcHandle is invalid!");
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    if (GATTSERVER(serverId) == nullptr || GATTSERVICE(serverId, srvcHandle) == nullptr) {
        HILOGE("param is null!");
        return OHOS_BT_STATUS_UNHANDLED;
    }
    GATTSERVER(serverId)->RemoveGattService(*GATTSERVICE(serverId, srvcHandle));
    delete GATTSERVICE(serverId, srvcHandle);
    GATTSERVICE(serverId, srvcHandle) = nullptr;
    if (g_GattsCallback != nullptr && g_GattsCallback->serviceDeleteCb != nullptr) {
        g_GattsCallback->serviceDeleteCb(OHOS_BT_STATUS_SUCCESS, serverId, srvcHandle);
    } else {
        HILOGW("callback is null.");
    }
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Clears all services.
 *
 * @param serverId Indicates the ID of the GATT server.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the services are cleared;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsClearServices(int serverId)
{
    HILOGI("serverId: %{public}d", serverId);
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Sends a response to the client from which a read or write request has been received.
 *
 * @param serverId Indicates the ID of the GATT server.
 * @param param Indicates the pointer to the response parameters. For details, see {@link GattsSendRspParam}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the response is sent;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsSendResponse(int serverId, GattsSendRspParam *param)
{
    if (param == nullptr) {
        HILOGE("param is null, serverId: %{public}d", serverId);
        return OHOS_BT_STATUS_FAIL;
    }

    if (serverId >= MAXIMUM_NUMBER_APPLICATION || serverId < 0) {
        HILOGE("serverId is invalid!");
        return OHOS_BT_STATUS_PARM_INVALID;
    }

    if (GATTSERVER(serverId) == nullptr) {
        HILOGE("param is null!");
        return OHOS_BT_STATUS_UNHANDLED;
    }

    HILOGI("serverId:%{public}d, requestId:%{public}d, valueLen:%{public}d",
        serverId, param->attrHandle, param->valueLen);
    std::optional<ConnectedDevice> deviceInfo = GetDeviceInfoByConnId(param->connectId);
    if (!deviceInfo.has_value()) {
        HILOGE("connectId is invalid!");
        return OHOS_BT_STATUS_FAIL;
    }
    struct ConnectedDevice value = deviceInfo.value();

    string strAddress;
    GetAddrFromByte(value.remoteAddr.addr, strAddress);

    BluetoothRemoteDevice device(strAddress, 1);

    // param->attrHandle is used as requestId
    int ret = GATTSERVER(serverId)->SendResponse(device, param->attrHandle,
        param->status, 0, reinterpret_cast<unsigned char *>(param->value), param->valueLen);

    if (g_GattsCallback != nullptr && g_GattsCallback->responseConfirmationCb != nullptr) {
        g_GattsCallback->responseConfirmationCb(ret, param->attrHandle);
    } else {
        HILOGW("callback is null.");
    }
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Sends an indication or notification to the client.
 *
 * The <b>confirm</b> field in <b>param</b> determines whether to send an indication or a notification.
 *
 * @param serverId Indicates the ID of the GATT server.
 * @param param Indicates the pointer to the sending parameters. For details, see {@link GattsSendIndParam}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the indication or notification is sent;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsSendIndication(int serverId, GattsSendIndParam *param)
{
    if (param == nullptr) {
        HILOGE("param is null, serverId: %{public}d", serverId);
        return OHOS_BT_STATUS_FAIL;
    }
    if (serverId >= MAXIMUM_NUMBER_APPLICATION || serverId < 0 ||
        GATTSERVER(serverId) == nullptr) {
        HILOGE("param is null!");
        return OHOS_BT_STATUS_UNHANDLED;
    }
    HILOGI("serverId: %{public}d, attrHandle:%{public}d, confirm:%{public}d, valueLen:%{public}d",
        serverId, param->attrHandle, param->confirm, param->valueLen);
    std::optional<ConnectedDevice> deviceInfo = GetDeviceInfoByConnId(param->connectId);
    if (!deviceInfo.has_value()) {
        HILOGE("connectId is invalid!");
        return OHOS_BT_STATUS_FAIL;
    }
    struct ConnectedDevice value = deviceInfo.value();

    string strAddress;
    GetAddrFromByte(value.remoteAddr.addr, strAddress);
    HILOGI("device: %{public}s", GetEncryptAddr(strAddress).c_str());
    BluetoothRemoteDevice device(strAddress, 1);

    int srvcHandle = 0;
    GattCharacteristic *appCharacteristic = FindCharacteristic(serverId, param->attrHandle, false, &srvcHandle);
    if (appCharacteristic == nullptr) {
        HILOGE("not find characteristic");
        return OHOS_BT_STATUS_FAIL;
    }

    HILOGI("srvcHandle:%{public}d", srvcHandle);
    GattCharacteristic characteristic(appCharacteristic->GetUuid(),
        appCharacteristic->GetHandle() + GATTSERVICES(serverId, srvcHandle).handleOffset,
        appCharacteristic->GetPermissions(),
        appCharacteristic->GetProperties());

    characteristic.SetValue(reinterpret_cast<unsigned char*>(param->value), param->valueLen);

    GATTSERVER(serverId)->NotifyCharacteristicChanged(device, characteristic,
        (param->confirm == 1) ? true : false);
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Sets the encryption type for the GATT connection.
 *
 * @param bdAddr Indicates the address of the client.
 * @param secAct Indicates the encryption type, as enumerated in {@link BleSecAct}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the encryption type is set;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsSetEncryption(BdAddr bdAddr, BleSecAct secAct)
{
    return OHOS_BT_STATUS_UNSUPPORTED;
}

/**
 * @brief Registers GATT server callbacks.
 *
 * @param func Indicates the pointer to the callbacks to register, as enumerated in {@link BtGattServerCallbacks}.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the callbacks are registered;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsRegisterCallbacks(BtGattServerCallbacks *func)
{
    HILOGI("enter");
    if (func == nullptr) {
        HILOGE("func is null.");
        return OHOS_BT_STATUS_PARM_INVALID;
    }

    g_GattsCallback = func;
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Adds a service, its characteristics, and descriptors and starts the service.
 *
 * This function is available for system applications only.
 *
 * @param srvcHandle Indicates the pointer to the handle ID of the service,
 * which is returned by whoever implements this function.
 * @param srvcInfo Indicates the pointer to the service information.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the operation is successful;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsStartServiceEx(int *srvcHandle, BleGattService *srvcInfo)
{
    return OHOS_BT_STATUS_UNSUPPORTED;
}

/**
 * @brief Stops a service.
 *
 * This function is available for system applications only.
 *
 * @param srvcHandle Indicates the handle ID of the service.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the operation is successful;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsStopServiceEx(int srvcHandle)
{
    return OHOS_BT_STATUS_UNSUPPORTED;
}
}  // namespace Bluetooth
}  // namespace OHOS

#ifdef __cplusplus
}
#endif
/** @} */
