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
#define LOG_TAG "bt_c_adapter_gatt_client"
#endif

#include "ohos_bt_gatt_client.h"
#include <cstring>
#include <map>
#include "__config"
#include <functional>
#include <atomic>

#include "bluetooth_gatt_characteristic.h"
#include "bluetooth_gatt_client.h"
#include "bluetooth_gatt_descriptor.h"
#include "bluetooth_gatt_service.h"
#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "bluetooth_remote_device.h"

#include "iosfwd"
#include "memory"
#include "new"
#include "ohos_bt_adapter_utils.h"
#include "ohos_bt_def.h"
#include "optional"
#include "string"
#include "type_traits"
#include "utility"
#include "uuid.h"

#ifdef __cplusplus
extern "C" {
#endif

using namespace std;

namespace OHOS {
namespace Bluetooth {
class GattClientCallbackWrapper;

struct GattClientWrapper {
    std::shared_ptr<GattClient> gattClient;
    std::shared_ptr<GattClientCallbackWrapper> gattClientCallback;
    string remoteAddr;
    bool fastestConnFlag;
};

using ClientIterator = std::map<int, struct GattClientWrapper>::iterator;

static std::atomic<int> g_clientIncrease(0);
static std::map<int, struct GattClientWrapper> g_MapGattClient;
static std::mutex g_MapGattClientMutex;

#define GATTCLIENT g_MapGattClient

int ConverWriteType(BtGattWriteType writeType)
{
    int outWriteType = GattCharacteristic::WriteType::DEFAULT;
    if (writeType == OHOS_GATT_WRITE_DEFAULT) {
        outWriteType = GattCharacteristic::WriteType::DEFAULT;
    } else if (writeType == OHOS_GATT_WRITE_NO_RSP) {
        outWriteType = GattCharacteristic::WriteType::NO_RESPONSE;
    } else if (writeType == OHOS_GATT_WRITE_SIGNED) {
        outWriteType = GattCharacteristic::WriteType::SIGNED;
    } else {
        HILOGE("write type: %{public}d is not supported and the default type is used.", writeType);
        outWriteType = GattCharacteristic::WriteType::DEFAULT;
    }
    return outWriteType;
}

static GattCharacteristic *GattcFindCharacteristic(int clientId, std::shared_ptr<GattClient> &client,
    BtGattCharacteristic characteristic)
{
    std::lock_guard<std::mutex> lock(g_MapGattClientMutex);
    ClientIterator iter = GATTCLIENT.find(clientId);
    if (iter == GATTCLIENT.end()) {
        HILOGE("clientId: %{public}d, has not been registered.", clientId);
        return nullptr;
    }

    client = iter->second.gattClient;
    if (client == nullptr) {
        HILOGE("client is null.");
        return nullptr;
    }

    string strUuidSvc(characteristic.serviceUuid.uuid);
    string strUuidChar(characteristic.characteristicUuid.uuid);
    HILOGD("execute, strUuidSvc: %{public}s, strUuidChar: %{public}s",
        strUuidSvc.c_str(), strUuidChar.c_str());
    if (!IsValidUuid(strUuidSvc) || !IsValidUuid(strUuidChar)) {
        HILOGE("match the UUID faild.");
        return nullptr;
    }
    std::optional<std::reference_wrapper<GattService>> service = client->GetService(UUID::FromString(strUuidSvc));
    if (service == std::nullopt) {
        HILOGE("find service fail.");
        return nullptr;
    }
    GattCharacteristic *charac = service->get().GetCharacteristic(UUID::FromString(strUuidChar));
    if (charac == nullptr) {
        HILOGE("find characteristic fail.");
    }
    return charac;
}


class GattClientCallbackWrapper : public GattClientCallback {
public:
    GattClientCallbackWrapper(BtGattClientCallbacks *callback, int clientId)
    {
        appCallback_ = callback;
        clientId_ = clientId;
    }

    void OnConnectionStateChanged(int connectionState, int ret) override
    {
        if (appCallback_ == nullptr || appCallback_->ConnectionStateCb == nullptr) {
            HILOGI("callback is null.");
            return;
        }

        HILOGD("clientId: %{public}d, connectionState: %{public}d, ret: %{public}d",
            clientId_, connectionState, ret);
        if (connectionState == static_cast<int>(BTConnectState::CONNECTED)) {
            HILOGI("GattcOnConnectionStateChanged Connected, clientId: %{public}d", clientId_);
        }
        appCallback_->ConnectionStateCb(clientId_, connectionState, GetGattcResult(ret));
    }

    void OnConnectionParameterChanged(int interval, int latency, int timeout, int status) override
    {
        if (appCallback_ == nullptr || appCallback_->connectParaUpdateCb == nullptr) {
            HILOGI("callback is null.");
            return;
        }

        HILOGI("clientId: %{public}d, status: %{public}d, interval: %{public}d, latency: %{public}d, "
            "timeout: %{public}d", clientId_, status, interval, latency, timeout);
        appCallback_->connectParaUpdateCb(clientId_, interval, latency, timeout, GetGattcResult(status));
    }

    void OnServicesDiscovered(int status) override
    {
        if (appCallback_ == nullptr || appCallback_->searchServiceCompleteCb == nullptr) {
            HILOGI("callback is null.");
            return;
        }

        HILOGI("GattcOnServicesDiscovered complete, clientId: %{public}d, status: %{public}d", clientId_, status);
        appCallback_->searchServiceCompleteCb(clientId_, GetGattcResult(status));
    }

    void OnCharacteristicReadResult(const GattCharacteristic &characteristic, int ret) override
    {
        if (appCallback_ == nullptr || appCallback_->readCharacteristicCb == nullptr) {
            HILOGI("callback is null.");
            return;
        }
        if (characteristic.GetService() == nullptr) {
            HILOGE("get service null.");
            return;
        }

        BtGattReadData characData;
        string srvUuid = characteristic.GetService()->GetUuid().ToString();
        string charcUuid = characteristic.GetUuid().ToString();
        GattcBuildUuid(&characData.attribute.characteristic.serviceUuid, srvUuid);
        GattcBuildUuid(&characData.attribute.characteristic.characteristicUuid, charcUuid);
        size_t tmpLen = 0;
        characData.data = characteristic.GetValue(&tmpLen).get();
        characData.dataLen = (unsigned short)tmpLen;

        HILOGI("clientId: %{public}d, ret: %{public}d, dataLen: %{public}d",
            clientId_, ret, characData.dataLen);
        HILOGI("srvUuid: %{public}s, charcUuid: %{public}s",
            srvUuid.c_str(), charcUuid.c_str());
        appCallback_->readCharacteristicCb(clientId_, &characData, GetGattcResult(ret));
    }

    void OnCharacteristicWriteResult(const GattCharacteristic &characteristic, int ret) override
    {
        if (appCallback_ == nullptr || appCallback_->writeCharacteristicCb == nullptr) {
            HILOGI("callback is null.");
            return;
        }
        if (characteristic.GetService() == nullptr) {
            HILOGE("get service null.");
            return;
        }

        BtGattCharacteristic tmpCharac;
        string srvUuid = characteristic.GetService()->GetUuid().ToString();
        string charcUuid = characteristic.GetUuid().ToString();
        GattcBuildUuid(&tmpCharac.serviceUuid, srvUuid);
        GattcBuildUuid(&tmpCharac.characteristicUuid, charcUuid);

        HILOGI("clientId: %{public}d, ret: %{public}d, ", clientId_, ret);
        HILOGI("srvUuid: %{public}s, charcUuid: %{public}s", srvUuid.c_str(), charcUuid.c_str());
        appCallback_->writeCharacteristicCb(clientId_, &tmpCharac, GetGattcResult(ret));
    }

    void OnDescriptorReadResult(const GattDescriptor &descriptor, int ret) override
    {
        if (appCallback_ == nullptr || appCallback_->readDescriptorCb == nullptr) {
            HILOGI("callback is null.");
            return;
        }

        if (descriptor.GetCharacteristic() == nullptr ||
            descriptor.GetCharacteristic()->GetService() == nullptr) {
            HILOGE("get characteristic or service null.");
            return;
        }

        BtGattReadData descData;
        string srvUuid = descriptor.GetCharacteristic()->GetService()->GetUuid().ToString();
        string charcUuid = descriptor.GetCharacteristic()->GetUuid().ToString();
        string descUuid = descriptor.GetUuid().ToString();
        GattcBuildUuid(&descData.attribute.descriptor.characteristic.serviceUuid, srvUuid);
        GattcBuildUuid(&descData.attribute.descriptor.characteristic.characteristicUuid, charcUuid);
        GattcBuildUuid(&descData.attribute.descriptor.descriptorUuid, descUuid);
        size_t tmpLen = 0;
        descData.data = descriptor.GetValue(&tmpLen).get();
        descData.dataLen = (unsigned short)tmpLen;

        HILOGI("clientId: %{public}d, ret: %{public}d, dataLen: %{public}d", clientId_, ret, descData.dataLen);
        HILOGI("srvUuid: %{public}s, charcUuid: %{public}s, descUuid: %{public}s",
            srvUuid.c_str(), charcUuid.c_str(), descUuid.c_str());
        appCallback_->readDescriptorCb(clientId_, &descData, GetGattcResult(ret));
    }

    void OnDescriptorWriteResult(const GattDescriptor &descriptor, int ret) override
    {
        if (appCallback_ == nullptr || appCallback_->writeDescriptorCb == NULL) {
            HILOGI("callback is null.");
            return;
        }

        if (descriptor.GetCharacteristic() == nullptr ||
            descriptor.GetCharacteristic()->GetService() == nullptr) {
            HILOGE("get characteristic or service null.");
            return;
        }

        BtGattDescriptor tmpDesc;
        string srvUuid = descriptor.GetCharacteristic()->GetService()->GetUuid().ToString();
        string charcUuid = descriptor.GetCharacteristic()->GetUuid().ToString();
        string descUuid = descriptor.GetUuid().ToString();
        GattcBuildUuid(&tmpDesc.characteristic.serviceUuid, srvUuid);
        GattcBuildUuid(&tmpDesc.characteristic.characteristicUuid, charcUuid);
        GattcBuildUuid(&tmpDesc.descriptorUuid, descUuid);

        HILOGI("clientId: %{public}d, ret: %{public}d", clientId_, ret);
        HILOGI("srvUuid: %{public}s, charcUuid: %{public}s, descUuid: %{public}s",
            srvUuid.c_str(), charcUuid.c_str(), descUuid.c_str());
        appCallback_->writeDescriptorCb(clientId_, &tmpDesc, GetGattcResult(ret));
    }

    void OnMtuUpdate(int mtu, int ret) override
    {
        if (appCallback_ == nullptr || appCallback_->configureMtuSizeCb == nullptr) {
            HILOGI("callback is null.");
            return;
        }

        HILOGI("clientId: %{public}d, mtu: %{public}d, ret: %{public}d", clientId_, mtu, ret);
        appCallback_->configureMtuSizeCb(clientId_, mtu, GetGattcResult(ret));
    }

    void OnSetNotifyCharacteristic(const GattCharacteristic &characteristic, int status) override
    {
        if (appCallback_ == nullptr || appCallback_->registerNotificationCb == nullptr) {
            HILOGI("callback is null.");
            return;
        }

        HILOGI("clientId: %{public}d, status: %{public}d", clientId_, status);
        appCallback_->registerNotificationCb(clientId_, GetGattcResult(status));
    }

    void OnCharacteristicChanged(const GattCharacteristic &characteristic) override
    {
        if (appCallback_ == nullptr || appCallback_->notificationCb == nullptr) {
            HILOGI("callback is null.");
            return;
        }
        if (characteristic.GetService() == nullptr) {
            HILOGE("get service null.");
            return;
        }

        BtGattReadData notificationData;
        string srvUuid = characteristic.GetService()->GetUuid().ToString();
        string charcUuid = characteristic.GetUuid().ToString();
        GattcBuildUuid(&notificationData.attribute.characteristic.serviceUuid, srvUuid);
        GattcBuildUuid(&notificationData.attribute.characteristic.characteristicUuid, charcUuid);
        size_t tmpLen = 0;
        notificationData.data = characteristic.GetValue(&tmpLen).get();
        notificationData.dataLen = (unsigned short)tmpLen;

        HILOGD("clientId: %{public}d, dataLen: %{public}d, ", clientId_, notificationData.dataLen);
        HILOGD("srvUuid: %{public}s, charcUuid: %{public}s", srvUuid.c_str(), charcUuid.c_str());
        appCallback_->notificationCb(clientId_, &notificationData, OHOS_BT_STATUS_SUCCESS);
    }
private:
    void GattcBuildUuid(BtUuid *desUuid, const std::string &srcUuid)
    {
        desUuid->uuid = (char *)srcUuid.c_str();
        desUuid->uuidLen = srcUuid.size();
    }

    BtGattClientCallbacks *appCallback_;
    int clientId_;
};

/**
 * @brief Registers a GATT client with a specified application UUID.
 *
 * @param appUuid Indicates the UUID of the application for which the GATT client is to be registered.
 * The UUID is defined by the application.
 * @return Returns the client ID.
 */
int BleGattcRegister(BtUuid appUuid)
{
    std::lock_guard<std::mutex> lock(g_MapGattClientMutex);
    g_clientIncrease++;
    struct GattClientWrapper clientWrapper;
    clientWrapper.gattClient = nullptr;
    clientWrapper.gattClientCallback = nullptr;
    clientWrapper.remoteAddr = "";
    clientWrapper.fastestConnFlag = false;
    int clientId = g_clientIncrease.load();
    GATTCLIENT.insert(std::pair<int, struct GattClientWrapper>(clientId, clientWrapper));
    HILOGI("clientId: %{public}d", clientId);
    return clientId;
}

/**
 * @brief Unregisters a GATT client with a specified ID.
 *
 * @param clientId Indicates the ID of the GATT client.
 * @return Returns the operation result status {@link BtStatus}.
 */
int BleGattcUnRegister(int clientId)
{
    std::lock_guard<std::mutex> lock(g_MapGattClientMutex);
    HILOGI("clientId: %{public}d", clientId);
    ClientIterator it = GATTCLIENT.find(clientId);
    if (it != GATTCLIENT.end()) {
        auto &clientWrapper = it->second;
        if (clientWrapper.gattClient != nullptr) {
            clientWrapper.gattClient = nullptr;
        }
        if (clientWrapper.gattClientCallback != nullptr) {
            clientWrapper.gattClientCallback = nullptr;
        }
        GATTCLIENT.erase(it);
    }
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Set fastest connection of the Gatt connection, add address to the accelerate connection map
 *  before create a new connection.
 *
 * @param clientId Indicates the ID of the GATT client.
 * @param fastestConnFlag Indicates whether to enable the fastest connection.
 * @return Returns the operation result status {@link BtStatus}.
 */
int BleGattcSetFastestConn(int clientId, bool fastestConnFlag)
{
    std::lock_guard<std::mutex> lock(g_MapGattClientMutex);
    HILOGI("clientId: %{public}d, fastestConnFlag: %{public}d", clientId, fastestConnFlag);
    ClientIterator iter = GATTCLIENT.find(clientId);
    if (iter == GATTCLIENT.end()) {
        HILOGE("clientId: %{public}d, has not been registered.", clientId);
        return OHOS_BT_STATUS_FAIL;
    }
    iter->second.fastestConnFlag = fastestConnFlag;
    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Create a Gatt connection to a remote device.
 *
 * @param clientId Indicates the ID of the GATT client.
 * @param bdAddr Indicates the remote device's address.
 * @param isAutoConnect Indicates whether it is a direct connection(false) or a background connection(true).
 * @param transport Indicates the transport of Gatt client {@link BtTransportType}.
 * @return Returns the operation result status {@link BtStatus}.
 */
int BleGattcConnect(int clientId, BtGattClientCallbacks *func, const BdAddr *bdAddr,
    bool isAutoConnect, BtTransportType transport)
{
    std::lock_guard<std::mutex> lock(g_MapGattClientMutex);
    if (func == nullptr || bdAddr == nullptr) {
        HILOGE("func or bdAddr is null.");
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    ClientIterator iter = GATTCLIENT.find(clientId);
    if (iter == GATTCLIENT.end()) {
        HILOGE("clientId: %{public}d, has not been registered.", clientId);
        return OHOS_BT_STATUS_FAIL;
    }

    string strAddress;
    ConvertAddr(bdAddr->addr, strAddress);
    HILOGI("BleGattcConnect start, clientId: %{public}d, addr: %{public}s, isAutoConnect: %{public}d",
        clientId, GetEncryptAddr(strAddress).c_str(), isAutoConnect);
    std::shared_ptr<GattClient> client = nullptr;
    if (iter->second.gattClient != nullptr && iter->second.remoteAddr == strAddress) {
        HILOGI("connect to the same remote device again.");
        client = iter->second.gattClient;
        iter->second.gattClientCallback = nullptr;
    } else {
        BluetoothRemoteDevice device(strAddress, transport);
        client = std::make_shared<GattClient>(device);
        client->Init();
    }

    if (iter->second.fastestConnFlag) {
        int result = client->RequestFastestConn();
        if (result != OHOS_BT_STATUS_SUCCESS) {
            HILOGE("request fastest connect fail.");
        }
        iter->second.fastestConnFlag = false;
    }

    std::shared_ptr<GattClientCallbackWrapper> clientWrapper = std::make_shared<GattClientCallbackWrapper>(
        func, clientId);
    iter->second.gattClient = client;
    iter->second.gattClientCallback = clientWrapper;
    iter->second.remoteAddr = strAddress;
    int result = client->Connect(clientWrapper, isAutoConnect, transport);
    HILOGI("clientId: %{public}d, result: %{public}d", clientId, result);
    if (result != OHOS_BT_STATUS_SUCCESS) {
        client = nullptr;
        clientWrapper = nullptr;
        iter->second.gattClient = nullptr;
        iter->second.gattClientCallback = nullptr;
        iter->second.remoteAddr = "";
        return OHOS_BT_STATUS_FAIL;
    }

    return OHOS_BT_STATUS_SUCCESS;
}

/**
 * @brief Set priority of the Gatt connection.
 *
 * @param clientId Indicates the ID of the GATT client.
 * @param bdAddr Indicates the remote device's address.
 * @param priority Indicates the priority of Gatt client {@link BtGattPriority}.
 * @return Returns the operation result status {@link BtStatus}.
 */
int BleGattcSetPriority(int clientId, const BdAddr *bdAddr, BtGattPriority priority)
{
    std::lock_guard<std::mutex> lock(g_MapGattClientMutex);
    if (bdAddr == nullptr) {
        HILOGE("bdAddr is null.");
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    ClientIterator iter = GATTCLIENT.find(clientId);
    if (iter == GATTCLIENT.end()) {
        HILOGE("clientId: %{public}d, has not been registered.", clientId);
        return OHOS_BT_STATUS_FAIL;
    }

    string strAddress;
    ConvertAddr(bdAddr->addr, strAddress);
    HILOGI("clientId: %{public}d, addr: %{public}s, priority: %{public}d",
        clientId, GetEncryptAddr(strAddress).c_str(), priority);
    if (iter->second.gattClient == nullptr || iter->second.remoteAddr != strAddress) {
        HILOGE("fail.");
        return OHOS_BT_STATUS_FAIL;
    }

    std::shared_ptr<GattClient> client = iter->second.gattClient;
    int result = client->RequestConnectionPriority(priority);
    HILOGI("clientId: %{public}d, result: %{public}d", clientId, result);
    return GetGattcResult(result);
}

/**
 * @brief Disconnect a Gatt connection with a remote device.
 *
 * @param clientId Indicates the ID of the GATT client.
 * @Returns the operation result status {@link BtStatus}.
 */
int BleGattcDisconnect(int clientId)
{
    std::lock_guard<std::mutex> lock(g_MapGattClientMutex);
    ClientIterator iter = GATTCLIENT.find(clientId);
    if (iter == GATTCLIENT.end()) {
        HILOGE("clientId: %{public}d, has not been registered.", clientId);
        return OHOS_BT_STATUS_FAIL;
    }

    std::shared_ptr<GattClient> client = iter->second.gattClient;
    if (client == nullptr) {
        HILOGE("clientId: %{public}d, has not been connected.", clientId);
        return OHOS_BT_STATUS_FAIL;
    }

    int result = client->Disconnect();
    HILOGI("clientId: %{public}d, result: %{public}d", clientId, result);
    return GetGattcResult(result);
}

/**
 * @brief Request a GATT service discovery on a remote device.
 *
 * @param clientId Indicates the ID of the GATT client.
 * @return Returns the operation result status {@link BtStatus}.
 */
int BleGattcSearchServices(int clientId)
{
    std::lock_guard<std::mutex> lock(g_MapGattClientMutex);
    HILOGI("BleGattcSearchServices start, clientId: %{public}d", clientId);
    ClientIterator iter = GATTCLIENT.find(clientId);
    if (iter == GATTCLIENT.end()) {
        HILOGE("clientId: %{public}d, has not been registered.", clientId);
        return OHOS_BT_STATUS_FAIL;
    }

    std::shared_ptr<GattClient> client = iter->second.gattClient;
    if (client == nullptr) {
        HILOGE("clientId: %{public}d, has not been connected.", clientId);
        return OHOS_BT_STATUS_FAIL;
    }

    HILOGI("DiscoverServices() called");
    int result = client->DiscoverServices();
    HILOGI("clientId: %{public}d, result: %{public}d", clientId, result);
    return GetGattcResult(result);
}

/**
 * @brief Check whether the expected service exists.
 *
 * @param clientId Indicates the ID of the GATT client.
 * @param serviceUuid Indicates the UUID of the service.
 * @return Returns true or false.
 */
bool BleGattcGetService(int clientId, BtUuid serviceUuid)
{
    std::lock_guard<std::mutex> lock(g_MapGattClientMutex);
    HILOGI("clientId: %{public}d", clientId);
    ClientIterator iter = GATTCLIENT.find(clientId);
    if (iter == GATTCLIENT.end()) {
        HILOGE("clientId has not been registered.");
        return false;
    }

    std::shared_ptr<GattClient> client = iter->second.gattClient;
    if (client == nullptr) {
        HILOGE("gatt is not connected.");
        return false;
    }

    string strUuid(serviceUuid.uuid);
    if (!IsValidUuid(strUuid)) {
        HILOGE("match the UUID faild.");
        return false;
    }
    UUID uuid(UUID::FromString(strUuid));
    HILOGI("service uuid: %{public}s", strUuid.c_str());
    std::optional<std::reference_wrapper<GattService>> gattService = client->GetService(uuid);
    if (gattService == std::nullopt) {
        HILOGE("get service failed, gattService is null.");
        return false;
    }
    GattService service = gattService->get();
    if (service.GetUuid().Equals(uuid)) {
        HILOGI("get service success.");
        return true;
    } else {
        HILOGE("get service failed, the service uuid is not exist.");
        return false;
    }
}

/**
 * @brief Read characteristic value from the remote device.
 *
 * @param clientId Indicates the ID of the GATT client.
 * @param characteristic The specified characteristic {@link BtGattCharacteristic} to be read.
 * @return Returns the operation result status {@link BtStatus}.
 */
int BleGattcReadCharacteristic(int clientId, BtGattCharacteristic characteristic)
{
    HILOGI("clientId: %{public}d", clientId);
    std::shared_ptr<GattClient> client = nullptr;
    GattCharacteristic *tmpCharac = GattcFindCharacteristic(clientId, client, characteristic);
    if (tmpCharac == nullptr || client == nullptr) {
        HILOGE("find characteristic fail.");
        return OHOS_BT_STATUS_FAIL;
    }

    int result = client->ReadCharacteristic(*tmpCharac);
    HILOGI("clientId: %{public}d, result: %{public}d", clientId, result);
    return GetGattcResult(result);
}

/**
 * @brief Write characteristic value to the remote device.
 *
 * @param clientId Indicates the ID of the GATT client.
 * @param characteristic The specified characteristic {@link BtGattCharacteristic} to be read.
 * @param writeType Indicates the characteristic write type.
 * @param value The value to be write.
 * @param len The length of the value.
 * @return Returns the operation result status {@link BtStatus}.
 */
int BleGattcWriteCharacteristic(int clientId, BtGattCharacteristic characteristic,
    BtGattWriteType writeType, int len, const char *value)
{
    HILOGD("clientId:%{public}d, writeType:%{public}d, len:%{public}d", clientId, writeType, len);
    std::shared_ptr<GattClient> client = nullptr;
    GattCharacteristic *tmpCharac = GattcFindCharacteristic(clientId, client, characteristic);
    if (tmpCharac == nullptr || client == nullptr) {
        HILOGE("find characteristic fail.");
        return OHOS_BT_STATUS_FAIL;
    }

    if (writeType != OHOS_GATT_WRITE_TYPE_UNKNOWN) {
        int newWriteType = ConverWriteType(writeType);
        tmpCharac->SetWriteType(newWriteType);
    }

    std::vector<uint8_t> characterValue(value, value + len);
    int result = client->WriteCharacteristic(*tmpCharac, std::move(characterValue));
    HILOGD("clientId: %{public}d, result: %{public}d", clientId, result);
    return GetGattcResult(result);
}

/**
 * @brief Read descriptor value from the remote device.
 *
 * @param clientId Indicates the ID of the GATT client.
 * @param descriptor The specified characteristic {@link BtGattDescriptor} to be read.
 * @return Returns the operation result status {@link BtStatus}.
 */
int BleGattcReadDescriptor(int clientId, BtGattDescriptor descriptor)
{
    HILOGI("clientId: %{public}d", clientId);
    std::shared_ptr<GattClient> client = nullptr;
    GattCharacteristic *tmpCharac = GattcFindCharacteristic(clientId, client, descriptor.characteristic);
    if (tmpCharac == nullptr || client == nullptr) {
        HILOGE("find characteristic fail.");
        return OHOS_BT_STATUS_FAIL;
    }

    string strUuidDesc(descriptor.descriptorUuid.uuid);
    if (!IsValidUuid(strUuidDesc)) {
        HILOGE("match the UUID faild.");
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    GattDescriptor *tmpDescriptor = tmpCharac->GetDescriptor(UUID::FromString(strUuidDesc));
    if (tmpDescriptor == nullptr) {
        HILOGE("find descriptor fail.");
        return OHOS_BT_STATUS_FAIL;
    }

    int result = client->ReadDescriptor(*tmpDescriptor);
    HILOGI("clientId: %{public}d, result: %{public}d", clientId, result);
    return GetGattcResult(result);
}

/**
 * @brief Write descriptor value to the remote device.
 *
 * @param clientId Indicates the ID of the GATT client.
 * @param descriptor The specified descriptor {@link BtGattDescriptor} to be read.
 * @param value The value to be write.
 * @param len The length of the value.
 * @return Returns the operation result status {@link BtStatus}.
 */
int BleGattcWriteDescriptor(int clientId, BtGattDescriptor descriptor, int len, const char *value)
{
    HILOGI("clientId:%{public}d, len:%{public}d", clientId, len);
    std::shared_ptr<GattClient> client = nullptr;
    GattCharacteristic *tmpCharac = GattcFindCharacteristic(clientId, client, descriptor.characteristic);
    if (tmpCharac == nullptr || client == nullptr) {
        HILOGE("find characteristic fail.");
        return OHOS_BT_STATUS_FAIL;
    }

    string strUuidDesc(descriptor.descriptorUuid.uuid);
    if (!IsValidUuid(strUuidDesc)) {
        HILOGE("match the UUID faild.");
        return OHOS_BT_STATUS_PARM_INVALID;
    }
    GattDescriptor *tmpDescriptor = tmpCharac->GetDescriptor(UUID::FromString(strUuidDesc));
    if (tmpDescriptor == nullptr) {
        HILOGE("find descriptor fail.");
        return OHOS_BT_STATUS_FAIL;
    }

    tmpDescriptor->SetValue(reinterpret_cast<unsigned char *>(const_cast<char *>(value)), len);
    int result = client->WriteDescriptor(*tmpDescriptor);
    HILOGI("clientId: %{public}d, result: %{public}d", clientId, result);
    return GetGattcResult(result);
}

/**
 * @brief Configure the ATT MTU size.
 *
 * @param clientId Indicates the ID of the GATT client.
 * @param mtuSize The size of MTU.
 * @return Returns the operation result status {@link BtStatus}.
 */
int BleGattcConfigureMtuSize(int clientId, int mtuSize)
{
    std::lock_guard<std::mutex> lock(g_MapGattClientMutex);
    HILOGI("clientId:%{public}d, mtuSize:%{public}d", clientId, mtuSize);
    ClientIterator iter = GATTCLIENT.find(clientId);
    if (iter == GATTCLIENT.end()) {
        HILOGE("GattcFindCharacteristic, clientId: %{public}d, has not been registered.", clientId);
        return OHOS_BT_STATUS_FAIL;
    }

    std::shared_ptr<GattClient> client = iter->second.gattClient;
    if (client == nullptr) {
        HILOGE("client is null.");
        return OHOS_BT_STATUS_FAIL;
    }

    int result = client->RequestBleMtuSize(mtuSize);
    HILOGD("clientId: %{public}d, result: %{public}d", clientId, result);
    return GetGattcResult(result);
}

/**
 * @brief Enable or disable notifications for a given characteristic.
 *
 * @param clientId Indicates the ID of the GATT client.
 * @param characteristic The specified characteristic {@link BtGattCharacteristic}.
 * @param enable True or false.
 * @return Returns the operation result status {@link BtStatus}.
 */
int BleGattcRegisterNotification(int clientId, BtGattCharacteristic characteristic, bool enable)
{
    HILOGI("clientId:%{public}d, enable:%{public}d", clientId, enable);
    std::shared_ptr<GattClient> client = nullptr;
    GattCharacteristic *tmpCharac = GattcFindCharacteristic(clientId, client, characteristic);
    if (tmpCharac == nullptr || client == nullptr) {
        HILOGE("find characteristic fail.");
        return OHOS_BT_STATUS_FAIL;
    }

    int result = client->SetNotifyCharacteristic(*tmpCharac, enable);
    HILOGI("clientId: %{public}d, result: %{public}d", clientId, result);
    return GetGattcResult(result);
}
}  // namespace Bluetooth
}  // namespace OHOS
#ifdef __cplusplus
}
#endif
/** @} */