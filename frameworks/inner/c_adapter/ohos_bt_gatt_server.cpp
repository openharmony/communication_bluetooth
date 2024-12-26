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
#define LOG_TAG "bt_c_adapter_gatt_server"
#endif

#include "ohos_bt_gatt_server.h"

#include <iostream>
#include <string.h>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
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

#define MAXIMUM_NUMBER_APPLICATION 10
#define MAXIMUM_NUMBER_GATTSERVICE 10

static const char *GetAttributeTypeString(int attributeType);

struct GattAttribute {
    enum AttributeType {
        GATT_SERVICE = 0,
        GATT_CHARACTERISTIC,
        GATT_DESCRIPTOR,
    };

    GattAttribute(uint16_t handle, UUID uuid, int attrType)
        : handle(handle), actualHandle(0), uuid(uuid), attrType(attrType) {}

    std::string ToLogString(void)
    {
        std::stringstream ss;
        ss << "uuid: " << uuid.ToString();
        ss << ", attrType: " << GetAttributeTypeString(attrType);
        ss << ", handle: " << handle;
        ss << ", actualHandle: " << actualHandle;
        return ss.str();
    }

    uint16_t handle;  // Attribute handle showed in application (fake attribute handle, used as index)
    uint16_t actualHandle;  // Attribute handle in bluetooth stack (actual attribute handle)
    UUID uuid;
    int attrType;
};

static const char *GetAttributeTypeString(int attributeType)
{
    switch (attributeType) {
        case GattAttribute::GATT_SERVICE:
            return "GATT_SERVICE";
        case GattAttribute::GATT_CHARACTERISTIC:
            return "GATT_CHARACTERISTIC";
        case GattAttribute::GATT_DESCRIPTOR:
            return "GATT_DESCRIPTOR";
        default:
            break;
    }
    return "Unknown";
}

struct GattServiceWrapper {
    GattService *gattService;
    bool isAdding;
    std::vector<std::shared_ptr<GattAttribute>> attributes;
};

struct GattServerWrapper {
    std::shared_ptr<GattServer> gattServer = nullptr;
    struct GattServiceWrapper gattServices[MAXIMUM_NUMBER_GATTSERVICE];
};

static uint16_t GetNextAttributeHandle(void)
{
    static std::atomic_uint16_t nextAttributeHandle = 1;  // The next attribute handle showed in application.
    if (nextAttributeHandle.load() == UINT16_MAX) {
        nextAttributeHandle = 1;
    }
    return nextAttributeHandle++;
}

GattServerWrapper g_gattServers[MAXIMUM_NUMBER_APPLICATION];
static mutex g_gattServersMutex;

#define GATTSERVER(x) g_gattServers[x].gattServer
#define GATTSERVICES(x, y) g_gattServers[x].gattServices[y]
#define GATTSERVICE(x, y) GATTSERVICES(x, y).gattService

static int AddDeviceRecord(struct ConnectedDevice &device);
static void RemoveDeviceRecord(int connId);
static std::optional<ConnectedDevice> GetDeviceInfoByConnId(int connId);
static std::optional<int> GetConnIdByDeviceInfo(struct ConnectedDevice &device);

static void AddAttribute(int serverId, int serviceIndex, int attrType, UUID uuid, uint16_t handle)
{
    CHECK_AND_RETURN_LOG(
        0 <= serverId && serverId < MAXIMUM_NUMBER_APPLICATION, "serverId(%{public}d) is invalid", serverId);
    CHECK_AND_RETURN_LOG(0 <= serviceIndex && serviceIndex < MAXIMUM_NUMBER_GATTSERVICE,
        "serviceIndex(%{public}d) is invalid", serviceIndex);

    std::lock_guard<std::mutex> lock(g_gattServersMutex);
    GattServiceWrapper &gattServiceWrapper = GATTSERVICES(serverId, serviceIndex);
    auto attribute = std::make_shared<GattAttribute>(handle, uuid, attrType);
    gattServiceWrapper.attributes.push_back(attribute);
    HILOGD("serverId(%{public}d), serviceIndex(%{public}d), attribute(%{public}s)",
        serverId, serviceIndex, attribute->ToLogString().c_str());
}

static std::shared_ptr<GattAttribute> GetAttribute(int serverId, int serviceIndex,
    std::function<bool(GattAttribute &)> predicate)
{
    CHECK_AND_RETURN_LOG_RET(
        0 <= serverId && serverId < MAXIMUM_NUMBER_APPLICATION, nullptr, "serverId(%{public}d) is invalid", serverId);
    CHECK_AND_RETURN_LOG_RET(0 <= serviceIndex && serviceIndex < MAXIMUM_NUMBER_GATTSERVICE, nullptr,
        "serviceIndex(%{public}d) is invalid", serviceIndex);

    std::lock_guard<std::mutex> lock(g_gattServersMutex);
    GattServiceWrapper &gattServiceWrapper = GATTSERVICES(serverId, serviceIndex);
    for (auto &attribute : gattServiceWrapper.attributes) {
        if (attribute != nullptr && predicate(*attribute)) {
            HILOGD("serverId(%{public}d), serviceIndex(%{public}d), attribute(%{public}s)",
                serverId, serviceIndex, attribute->ToLogString().c_str());
            return attribute;
        }
    }
    return nullptr;
}

static std::shared_ptr<GattAttribute> GetAttributeWithHandle(int serverId, int handle)
{
    for (int i = 0; i < MAXIMUM_NUMBER_GATTSERVICE; i++) {
        auto attribute =
            GetAttribute(serverId, i, [handle](const GattAttribute &attr) { return attr.handle == handle; });
        if (attribute) {
            return attribute;
        }
    }

    HILOGE("Not found attribute handle(%{public}d) in serverId(%{public}d)", handle, serverId);
    return nullptr;
}

static std::shared_ptr<GattAttribute> GetAttributeWithUuid(int serverId, int serviceIndex, int attributeType, UUID uuid)
{
    auto attribute = GetAttribute(serverId, serviceIndex, [attributeType, uuid](GattAttribute &attr) {
        return attr.attrType == attributeType && attr.uuid.Equals(uuid);
    });
    if (!attribute) {
        HILOGE("Not found attribute(attributeType: %{public}s, uuid: %{public}s)"
            "in serverId(%{public}d) serviceIndex(%{public}d)",
            GetAttributeTypeString(attributeType), uuid.ToString().c_str(), serverId, serviceIndex);
    }
    return attribute;
}

static std::shared_ptr<GattAttribute> GetDescriptorAttribute(
    int serverId, int serviceIndex, UUID characterUuid, UUID descriptorUuid)
{
    auto characterAttribute =
        GetAttributeWithUuid(serverId, serviceIndex, GattAttribute::GATT_CHARACTERISTIC, characterUuid);
    CHECK_AND_RETURN_LOG_RET(
        characterAttribute, nullptr, "not found character uuid:%{public}s", characterUuid.ToString().c_str());

    // Descriptor attribute handle is after the characteristic attribute handle.
    auto descriptorAttribute = GetAttribute(serverId, serviceIndex,
        [characterHandle = characterAttribute->handle, descriptorUuid](GattAttribute &attr) {
            return attr.handle > characterHandle && attr.uuid.Equals(descriptorUuid);
    });
    if (!descriptorAttribute) {
        HILOGE("Not found descriptorAttribute(uuid: %{public}s) in serverId(%{public}d) serviceIndex(%{public}d)",
            descriptorUuid.ToString().c_str(), serverId, serviceIndex);
    }
    return descriptorAttribute;
}

static std::shared_ptr<GattAttribute> GetAttributeWithActualHandle(int serverId, uint16_t actualHandle)
{
    for (int i = 0; i < MAXIMUM_NUMBER_GATTSERVICE; i++) {
        auto attribute = GetAttribute(serverId, i, [actualHandle](const GattAttribute &attr) {
            return attr.actualHandle == actualHandle;
        });
        if (attribute) {
            return attribute;
        }
    }

    HILOGE("Not found attribute actualHandle(%{public}u) in serverId(%{public}d)", actualHandle, serverId);
    return nullptr;
}

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

    void OnServiceAdded(GattService service, int ret) override
    {
        int i;
        CHECK_AND_RETURN_LOG(
            0 <= serverId_ && serverId_ < MAXIMUM_NUMBER_APPLICATION, "serverId(%{public}d) is invalid", serverId_);
        CHECK_AND_RETURN_LOG(g_GattsCallback && g_GattsCallback->serviceStartCb, "callback is nullptr");
        if (ret != GattStatus::GATT_SUCCESS) {
            g_GattsCallback->serviceStartCb(OHOS_BT_STATUS_FAIL, serverId_, MAXIMUM_NUMBER_GATTSERVICE);
            return;
        }

        {
            std::lock_guard<std::mutex> lock(g_gattServersMutex);
            for (i = 0; i < MAXIMUM_NUMBER_GATTSERVICE; i++) {
                if (GATTSERVICE(serverId_, i) == nullptr) {
                    continue;
                }
                auto &gattServiceWrapper = GATTSERVICES(serverId_, i);
                if (gattServiceWrapper.isAdding && GATTSERVICE(serverId_, i)->GetUuid().Equals(service.GetUuid())) {
                    gattServiceWrapper.isAdding = false;
                    HILOGI("find service, serverId: %{public}d, serviceIndex: %{public}d", serverId_, i);
                    break;
                }
            }
        }
        if (i == MAXIMUM_NUMBER_GATTSERVICE) {
            HILOGE("add service failed, invalid srvcHandle: %{public}u", service.GetHandle());
            g_GattsCallback->serviceStartCb(OHOS_BT_STATUS_FAIL, serverId_, i);
            return;
        }

        vector<GattCharacteristic> &characteristics = service.GetCharacteristics();
        for (auto item = characteristics.begin(); item != characteristics.end(); item++) {
            auto characterAttribute =
                GetAttributeWithUuid(serverId_, i, GattAttribute::GATT_CHARACTERISTIC, item->GetUuid());
            if (characterAttribute == nullptr) {
                HILOGE("not found characterAttribute");
                continue;
            }
            characterAttribute->actualHandle = item->GetHandle();
            HILOGI("characterAttribute(%{public}s)", characterAttribute->ToLogString().c_str());
            vector<GattDescriptor> &descriptors = item->GetDescriptors();
            for (auto des = descriptors.begin(); des != descriptors.end(); des++) {
                auto descAttribute = GetDescriptorAttribute(serverId_, i, item->GetUuid(), des->GetUuid());
                if (descAttribute != nullptr) {
                    descAttribute->actualHandle = des->GetHandle();
                    HILOGI("descAttribute(%{public}s)", descAttribute->ToLogString().c_str());
                }
            }
        }

        g_GattsCallback->serviceStartCb(OHOS_BT_STATUS_SUCCESS, serverId_, i);
    }

    void OnCharacteristicReadRequest(
        const BluetoothRemoteDevice &device, GattCharacteristic &characteristic, int requestId) override
    {
        CHECK_AND_RETURN_LOG(
            0 <= serverId_ && serverId_ < MAXIMUM_NUMBER_APPLICATION, "serverId(%{public}d) is invalid", serverId_);

        struct ConnectedDevice dev;
        dev.serverId = serverId_;
        GetAddrFromString(device.GetDeviceAddr(), dev.remoteAddr.addr);

        std::optional<int> connId = GetConnIdByDeviceInfo(dev);
        if (!connId.has_value()) {
            HILOGW("device is not exist.");
            return;
        }

        std::shared_ptr<GattAttribute> attribute = GetAttributeWithActualHandle(serverId_, characteristic.GetHandle());
        CHECK_AND_RETURN_LOG(attribute, "not found attribute in serverId(%{public}d) with actualHandle(%{public}u)",
            serverId_, characteristic.GetHandle());

        BtReqReadCbPara readInfo;
        readInfo.connId = connId.value();
        readInfo.transId = requestId;
        readInfo.bdAddr = &dev.remoteAddr;
        readInfo.attrHandle = attribute->handle;
        readInfo.offset = 0;
        readInfo.isLong = false;
        HILOGD("connId: %{public}d, requestId: %{public}d, %{public}s",
            connId.value(), requestId, attribute->ToLogString().c_str());
        if (g_GattsCallback != nullptr && g_GattsCallback->requestReadCb != nullptr) {
            g_GattsCallback->requestReadCb(readInfo);
        } else {
            HILOGW("call back is null.");
        }
    }

    void OnCharacteristicWriteRequest(
        const BluetoothRemoteDevice &device, GattCharacteristic &characteristic, int requestId) override
    {
        CHECK_AND_RETURN_LOG(
            0 <= serverId_ && serverId_ < MAXIMUM_NUMBER_APPLICATION, "serverId(%{public}d) is invalid", serverId_);

        struct ConnectedDevice dev;
        dev.serverId = serverId_;
        GetAddrFromString(device.GetDeviceAddr(), dev.remoteAddr.addr);

        std::optional<int> connId = GetConnIdByDeviceInfo(dev);
        if (!connId.has_value()) {
            HILOGW("device is not exist.");
            return;
        }

        std::shared_ptr<GattAttribute> attribute = GetAttributeWithActualHandle(serverId_, characteristic.GetHandle());
        CHECK_AND_RETURN_LOG(attribute, "not found attribute in serverId(%{public}d) with actualHandle(%{public}u)",
            serverId_, characteristic.GetHandle());

        BtReqWriteCbPara writeInfo;
        size_t length = 0;
        writeInfo.connId = connId.value();
        writeInfo.transId = requestId;
        writeInfo.bdAddr = &dev.remoteAddr;
        writeInfo.attrHandle = attribute->handle;
        writeInfo.offset = 0;
        writeInfo.value = characteristic.GetValue(&length).get();
        writeInfo.length = static_cast<int>(length);
        writeInfo.needRsp = (characteristic.GetWriteType() == GattCharacteristic::WriteType::DEFAULT);
        writeInfo.isPrep = false;
        HILOGD("connId: %{public}d, requestId: %{public}d, valueLen: %{public}d, %{public}s",
            connId.value(), requestId, writeInfo.length, attribute->ToLogString().c_str());
        if (g_GattsCallback != nullptr && g_GattsCallback->requestWriteCb != nullptr) {
            g_GattsCallback->requestWriteCb(writeInfo);
        } else {
            HILOGW("call back is null.");
        }
    }

    void OnDescriptorReadRequest(
        const BluetoothRemoteDevice &device, GattDescriptor &descriptor, int requestId) override
    {
        CHECK_AND_RETURN_LOG(
            0 <= serverId_ && serverId_ < MAXIMUM_NUMBER_APPLICATION, "serverId(%{public}d) is invalid", serverId_);

        struct ConnectedDevice dev;
        dev.serverId = serverId_;
        GetAddrFromString(device.GetDeviceAddr(), dev.remoteAddr.addr);

        std::optional<int> connId = GetConnIdByDeviceInfo(dev);
        if (!connId.has_value()) {
            HILOGW("device is not exist.");
            return;
        }

        std::shared_ptr<GattAttribute> attribute = GetAttributeWithActualHandle(serverId_, descriptor.GetHandle());
        CHECK_AND_RETURN_LOG(attribute, "not found attribute in serverId(%{public}d) with actualHandle(%{public}u)",
            serverId_, descriptor.GetHandle());

        BtReqReadCbPara readInfo;
        readInfo.connId = connId.value();
        readInfo.transId = requestId;
        readInfo.bdAddr = &dev.remoteAddr;
        readInfo.attrHandle = attribute->handle;
        readInfo.offset = 0;
        readInfo.isLong = false;
        HILOGD("connId: %{public}d, requestId: %{public}d, %{public}s",
            connId.value(), requestId, attribute->ToLogString().c_str());
        if (g_GattsCallback != nullptr && g_GattsCallback->requestReadCb != nullptr) {
            g_GattsCallback->requestReadCb(readInfo);
        } else {
            HILOGW("call back is null.");
        }
    }

    void OnDescriptorWriteRequest(
        const BluetoothRemoteDevice &device, GattDescriptor &descriptor, int requestId) override
    {
        CHECK_AND_RETURN_LOG(
            0 <= serverId_ && serverId_ < MAXIMUM_NUMBER_APPLICATION, "serverId(%{public}d) is invalid", serverId_);

        struct ConnectedDevice dev;
        dev.serverId = serverId_;
        GetAddrFromString(device.GetDeviceAddr(), dev.remoteAddr.addr);

        std::optional<int> connId = GetConnIdByDeviceInfo(dev);
        if (!connId.has_value()) {
            HILOGW("device is not exist.");
            return;
        }

        std::shared_ptr<GattAttribute> attribute = GetAttributeWithActualHandle(serverId_, descriptor.GetHandle());
        CHECK_AND_RETURN_LOG(attribute, "not found attribute in serverId(%{public}d) with actualHandle(%{public}u)",
            serverId_, descriptor.GetHandle());

        BtReqWriteCbPara writeInfo;
        size_t length = 0;
        writeInfo.connId = connId.value();
        writeInfo.transId = requestId;
        writeInfo.bdAddr = &dev.remoteAddr;
        writeInfo.attrHandle = attribute->handle;
        writeInfo.offset = 0;
        writeInfo.value = descriptor.GetValue(&length).get();
        writeInfo.length = static_cast<int>(length);
        writeInfo.needRsp = true;
        writeInfo.isPrep = false;

        HILOGD("connId: %{public}d, requestId: %{public}d, valueLen: %{public}d, %{public}s",
            connId.value(), requestId, writeInfo.length, attribute->ToLogString().c_str());
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
        HILOGD("result: %{public}d", result);
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
        HILOGD("enter");
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
    int i = 0;
    {
        std::lock_guard<std::mutex> lock(g_gattServersMutex);
        for (; i < MAXIMUM_NUMBER_APPLICATION; i++) {
            if (GATTSERVER(i) == nullptr) {
                std::shared_ptr<GattServerCallback> callbackWapper =
                    std::make_shared<GattServerCallbackWapper>(g_GattsCallback, i);
                GATTSERVER(i) = GattServer::CreateInstance(callbackWapper);
                HILOGI("register gattServer: %{public}d", i);
                break;
            }
        }
    }
    if (i != MAXIMUM_NUMBER_APPLICATION) {
        if (g_GattsCallback->registerServerCb != nullptr) {
            g_GattsCallback->registerServerCb(0, i, &appUuid);
        }
        return OHOS_BT_STATUS_SUCCESS;
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
    std::lock_guard<std::mutex> lock(g_gattServersMutex);
    if (serverId >= 0 && serverId < MAXIMUM_NUMBER_APPLICATION) {
        if (GATTSERVER(serverId) != nullptr) {
            GATTSERVER(serverId) = nullptr;
            return OHOS_BT_STATUS_SUCCESS;
        }
    }

    return OHOS_BT_STATUS_FAIL;
}

/**
 * @brief GATT server connect the client.
 *
 * @param serverId Indicates the ID of the GATT server.
 * @param bdAddr Indicates the address of the client.
 * @return Returns {@link OHOS_BT_STATUS_SUCCESS} if the GATT server is disconnected from the client;
 * returns an error code defined in {@link BtStatus} otherwise.
 * @since 6
 */
int BleGattsConnect(int serverId, BdAddr bdAddr)
{
    HILOGI("serverId: %{public}d", serverId);
    CHECK_AND_RETURN_LOG_RET(
        0 <= serverId && serverId < MAXIMUM_NUMBER_APPLICATION, OHOS_BT_STATUS_PARM_INVALID, "serverId is invalid!");
    std::lock_guard<std::mutex> lock(g_gattServersMutex);
    CHECK_AND_RETURN_LOG_RET(GATTSERVER(serverId), OHOS_BT_STATUS_UNHANDLED, "GATTSERVER(serverId) is null!");

    string strAddress;
    GetAddrFromByte(bdAddr.addr, strAddress);
    BluetoothRemoteDevice device(strAddress, BT_TRANSPORT_BLE);
    return GATTSERVER(serverId)->Connect(device, true);
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
    CHECK_AND_RETURN_LOG_RET(
        0 <= serverId && serverId < MAXIMUM_NUMBER_APPLICATION, OHOS_BT_STATUS_PARM_INVALID, "serverId is invalid!");
    std::lock_guard<std::mutex> lock(g_gattServersMutex);
    CHECK_AND_RETURN_LOG_RET(GATTSERVER(serverId), OHOS_BT_STATUS_UNHANDLED, "GATTSERVER(serverId) is null!");

    string strAddress;
    GetAddrFromByte(bdAddr.addr, strAddress);
    BluetoothRemoteDevice device(strAddress, BT_TRANSPORT_BLE);
    return GATTSERVER(serverId)->CancelConnection(device);
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
    if (!IsValidUuid(strUuid)) {
        HILOGE("match the UUID faild.");
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    UUID uuid(UUID::FromString(strUuid));

    if (serverId >= MAXIMUM_NUMBER_APPLICATION || serverId < 0) {
        HILOGE("serverId is invalid!");
        return OHOS_BT_STATUS_PARM_INVALID;
    }

    int i = 0;
    {
        std::lock_guard<std::mutex> lock(g_gattServersMutex);
        for (; i < MAXIMUM_NUMBER_GATTSERVICE; i++) {
            if (GATTSERVICE(serverId, i) == nullptr) {
                HILOGD("add srvcHandle: %{public}d", i);
                GATTSERVICE(serverId, i) = new GattService(
                    uuid, i, number, isPrimary ? GattServiceType::PRIMARY : GattServiceType::SECONDARY);
                GATTSERVICES(serverId, i).isAdding = false;
                break;
            }
        }
    }
    if (i != MAXIMUM_NUMBER_GATTSERVICE) {
        if (g_GattsCallback != nullptr && g_GattsCallback->serviceAddCb != nullptr) {
            g_GattsCallback->serviceAddCb(0, serverId, &srvcUuid, i);
        } else {
            HILOGW("call back is null");
        }
        return OHOS_BT_STATUS_SUCCESS;
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
    if (!IsValidUuid(strUuid)) {
        HILOGE("match the UUID faild.");
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    UUID uuid(UUID::FromString(strUuid));

    uint16_t characterHandle = 0;
    {
        std::lock_guard<std::mutex> lock(g_gattServersMutex);
        if (GATTSERVICE(serverId, srvcHandle) == nullptr) {
            HILOGE("GATTSERVICE(serverId:%{public}d, srvcHandle:%{public}u) is null!", serverId, srvcHandle);
            return OHOS_BT_STATUS_UNHANDLED;
        }

        unsigned char stubValue[1] = {0x31};
        GattCharacteristic characteristic(uuid, permissions, properties);
        characteristic.SetValue(stubValue, sizeof(stubValue));

        characterHandle = GetNextAttributeHandle();
        GATTSERVICE(serverId, srvcHandle)->AddCharacteristic(characteristic);
    }
    AddAttribute(serverId, srvcHandle, GattAttribute::GATT_CHARACTERISTIC, uuid, characterHandle);

    HILOGI("serverId: %{public}d, srvcHandle: %{public}d, charHandle: %{public}d",
        serverId, srvcHandle, characterHandle);
    if (g_GattsCallback != nullptr && g_GattsCallback->characteristicAddCb != nullptr) {
        g_GattsCallback->characteristicAddCb(0, serverId, &characUuid, srvcHandle, characterHandle);
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
    if (!IsValidUuid(strUuid)) {
        HILOGE("match the UUID faild.");
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    UUID uuid(UUID::FromString(strUuid));

    uint16_t desHandle = 0;
    {
        std::lock_guard<std::mutex> lock(g_gattServersMutex);
        if (GATTSERVICE(serverId, srvcHandle) == nullptr) {
            HILOGE("GATTSERVICE(serverId, srvcHandle) is null!");
            return OHOS_BT_STATUS_UNHANDLED;
        }
        GattCharacteristic &characteristic = GATTSERVICE(serverId, srvcHandle)->GetCharacteristics().back();
        desHandle = GetNextAttributeHandle();
        GattDescriptor descriptor(uuid, desHandle, permissions);

        unsigned char stubValue[2] = {0x01, 0x00};
        descriptor.SetValue(stubValue, sizeof(stubValue));

        characteristic.AddDescriptor(descriptor);
    }
    AddAttribute(serverId, srvcHandle, GattAttribute::GATT_DESCRIPTOR, uuid, desHandle);
    HILOGI("serverId: %{public}d, srvcHandle: %{public}d, desHandle: %{public}d", serverId, srvcHandle, desHandle);
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
    std::lock_guard<std::mutex> lock(g_gattServersMutex);
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
    {
        std::lock_guard<std::mutex> lock(g_gattServersMutex);
        if (GATTSERVER(serverId) == nullptr || GATTSERVICE(serverId, srvcHandle) == nullptr) {
            HILOGE("param is null!");
            return OHOS_BT_STATUS_UNHANDLED;
        }

        GATTSERVICES(serverId, srvcHandle).isAdding = false;
        GATTSERVER(serverId)->RemoveGattService(*GATTSERVICE(serverId, srvcHandle));
    }
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
    {
        std::lock_guard<std::mutex> lock(g_gattServersMutex);
        if (GATTSERVER(serverId) == nullptr || GATTSERVICE(serverId, srvcHandle) == nullptr) {
            HILOGE("param is null!");
            return OHOS_BT_STATUS_UNHANDLED;
        }
        GATTSERVER(serverId)->RemoveGattService(*GATTSERVICE(serverId, srvcHandle));
        delete GATTSERVICE(serverId, srvcHandle);
        GATTSERVICE(serverId, srvcHandle) = nullptr;
        GATTSERVICES(serverId, srvcHandle).attributes.clear();
    }
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

    int ret = 0;
    {
        std::lock_guard<std::mutex> lock(g_gattServersMutex);
        if (GATTSERVER(serverId) == nullptr) {
            HILOGE("param is null!");
            return OHOS_BT_STATUS_UNHANDLED;
        }

        HILOGD("serverId:%{public}d, requestId:%{public}d, valueLen:%{public}d",
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
        ret = GATTSERVER(serverId)->SendResponse(device, param->attrHandle,
            param->status, 0, reinterpret_cast<unsigned char *>(param->value), param->valueLen);
    }

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
    if (serverId >= MAXIMUM_NUMBER_APPLICATION || serverId < 0) {
        HILOGE("param is null!");
        return OHOS_BT_STATUS_UNHANDLED;
    }
    std::optional<ConnectedDevice> deviceInfo = GetDeviceInfoByConnId(param->connectId);
    if (!deviceInfo.has_value()) {
        HILOGE("connectId is invalid!");
        return OHOS_BT_STATUS_FAIL;
    }
    struct ConnectedDevice value = deviceInfo.value();

    string strAddress;
    GetAddrFromByte(value.remoteAddr.addr, strAddress);
    BluetoothRemoteDevice device(strAddress, 1);

    std::shared_ptr<GattAttribute> attribute = GetAttributeWithHandle(serverId, param->attrHandle);
    CHECK_AND_RETURN_LOG_RET(attribute, OHOS_BT_STATUS_FAIL, "not found attribute");
    HILOGD("serverId: %{public}d, handle:%{public}u, actualHandle:%{public}u confirm:%{public}d, valueLen:%{public}d",
        serverId, attribute->handle, attribute->actualHandle, param->confirm, param->valueLen);
    // permission, properties is not used.
    GattCharacteristic characteristic(attribute->uuid, attribute->actualHandle, 0, 0);

    characteristic.SetValue(reinterpret_cast<unsigned char*>(param->value), param->valueLen);
    {
        std::lock_guard<std::mutex> lock(g_gattServersMutex);
        if (GATTSERVER(serverId) == nullptr) {
            HILOGE("param is null!");
            return OHOS_BT_STATUS_UNHANDLED;
        }
        GATTSERVER(serverId)->NotifyCharacteristicChanged(device, characteristic,
            (param->confirm == 1) ? true : false);
    }
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
 * explain: This function does not support dynamic registration;
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
