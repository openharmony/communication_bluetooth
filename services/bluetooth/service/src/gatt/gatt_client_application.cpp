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
#define LOG_TAG "bt_service_gatt_client"
#endif

#include "gatt_client_application.h"

#include <cstdint>
#include <vector>

#include "adapter_config.h"
#include "ble/ble_defs.h"
#include "btif_gatt.h"
#include "bt_chr_ue_manager.h"
#include "gatt_api.h"
#include "gatt_data.h"
#include "bluetooth_connection_manager.h"
#include "adapter_manager.h"
#include "log.h"
#include "permission_manager.h"
#include "thread_util.h"
#include "common/bluetooth_hw_interface.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_resource_manager.h"
#include "bt_func_hook.h"
#ifdef BLUETOOTH_WATCH_ENABLE
#include "watch_service.h"
#endif
#include "bluetooth_state_manager.h"

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;

GattClientApplication::GattClientApplication(
    int clientIf, const BtgattClientInterface *interface, const GattDevice &device,
    std::weak_ptr<IGattClientCallback> callback)
    : clientIf_(clientIf),
      connState_(static_cast<int>(BTConnectState::DISCONNECTED)),
      connId_(GATT_INVALID_CONN_ID),
      addr_(device.addr_),
      transport_(GATT_TRANSPORT_INVALID),
      gattDb_(),
      btIfGattClient_(interface),
      callback_(callback),
      authRetryState_(AUTH_RETRY_STATE_IDLE),
      cacheValue_()
{
    transport_ = GattServiceBase::GattTransportToStack(device.transport_);
}

GattClientApplication::~GattClientApplication()
{
    HILOGI("~GattClientApplication");
    if (btIfGattClient_) {
        int ret = btIfGattClient_->unregisterClient(clientIf_);
        if (ret != BT_STATUS_SUCCESS) {
            HILOGE("unregister client failed, ret: %{public}d", ret);
        }
    }
}

void GattClientApplication::ConnectCallback(
    int connId, int status, int clientIf, const STACK::RawAddress &bda)
{
    if (clientIf_ != clientIf) {
        return;
    }
    HILOGI("connId: %{public}d, status: %{public}d, clientIf: %{public}d, address: %{public}s",
        connId, status, clientIf, bda.ToStringForLogging().c_str());
    auto resourceMgr = BluetoothResourceManager::GetInstance();
    if (resourceMgr) {
        resourceMgr->SendSensingStateChanged(GATT_CLIENT_CONNECT_DONE,
            SensingInfo(bda.ToString(), static_cast<uint32_t>(clientIf)));
    }

    BtChrEventWriteInt(CHR_BT_WATCH_CONNECT, bda.ToString(), "GATTCSTATUS", status);

    if (status != GATT_SUCCESS) {
        BtChrEventWriteInt(CHR_BLE_DISCONNECT, bda.ToString(), "BLEPROFILEERROR", status);
        HILOGE("failed, status: %{public}d", status);
        connState_ = static_cast<int>(BTConnectState::DISCONNECTED);
        int disconnectReason = static_cast<int>(GattDisconnectReason::CONN_UNKNOWN);
        WPTR_CBACK(callback_, OnConnectionStateChanged, GattStatus::GATT_FAILURE, connState_, addr_, disconnectReason,
            GATT_DIS_MSG_CONNECT_FAIL);
        return;
    }
    BtChrEventWriteInt(CHR_BLE_DISCONNECT, bda.ToString(), "BLEPROFILEERROR", GATT_SUCCESS);
    BtChrEventWriteTime(CHR_BLE_DISCONNECT, bda.ToString(), "BLEPROFILECONNECTEDTIME");
    connId_ = connId;
    connState_ = static_cast<int>(BTConnectState::CONNECTED);
    BluetoothStateManager::GetInstance()->AddDeviceProfileConnectState(
        PROFILE_NAME_GATT_CLIENT, bda.ToString(), BTConnectState::CONNECTED);
    WPTR_CBACK(callback_, OnConnectionStateChanged, GattStatus::GATT_SUCCESS, connState_,
        ServiceUtil::AddrFromStack(bda));
}

void GattClientApplication::KeepBleScan(const std::string &pkgName, int uid)
{
    if (!IsConnected()) {
        STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(addr_);
        BluetoothHwInterface::GetInstance()->KeepBleScanInConn(pkgName, uid, BT_TRANSPORT_LE, rawAddr);
    }
}

void GattClientApplication::Connect(bool autoConnect)
{
    if (IsConnected()) {
        HILOGE("Already connected");
        WPTR_CBACK(callback_, OnConnectionStateChanged, GattStatus::GATT_SUCCESS, connState_, addr_);
        return;
    }
    auto resourceMgr = BluetoothResourceManager::GetInstance();
    if (resourceMgr) {
        resourceMgr->SendSensingStateChanged(GATT_REGISTER_APPLICATION_PKGNAME,
            SensingInfo(addr_.GetAddress(), PermissionManager::GetCallingName(), false));
    }
    if (btIfGattClient_) {
        // direct, not opportunistic
        int ret = btIfGattClient_->connect(
            clientIf_, ServiceUtil::AddrToStack(addr_), !autoConnect, transport_, false, BTM_PHY_LE_1M);
        if (ret != BT_STATUS_SUCCESS) {
            HILOGE("failed, ret: %{public}d", ret);
            int disconnectReason = static_cast<int>(GattDisconnectReason::CONN_UNKNOWN);
            WPTR_CBACK(callback_, OnConnectionStateChanged, GattStatus::GATT_FAILURE, connState_, addr_, 
                disconnectReason, GATT_DIS_MSG_CONNECT_FAIL);
            return;
        }
    }
    connState_ = static_cast<int>(BTConnectState::CONNECTING);
    WPTR_CBACK(callback_, OnConnectionStateChanged, GattStatus::GATT_SUCCESS, connState_, addr_);
}

void GattClientApplication::DisconnectCallback(
    int connId, int status, int clientIf, const STACK::RawAddress &bda, int reason)
{
    if (clientIf_ != clientIf) {
        return;
    }
    HILOGI("connId: %{public}d, status: %{public}d, clientIf: %{public}d, address: %{public}s",
        connId, status, clientIf, bda.ToStringForLogging().c_str());

    auto resourceMgr = BluetoothResourceManager::GetInstance();
    if (resourceMgr) {
        resourceMgr->SendSensingStateChanged(GATT_CLIENT_CONNECT_CLOSE,
            SensingInfo(bda.ToString(), static_cast<uint32_t>(clientIf)));
    }
    int convertReason = static_cast<int>(GattDisconnectReason::CONN_UNKNOWN);
    std::string reasonMessage = "";
    GattServiceBase::ConvertDisconnectReason(convertReason, reason, reasonMessage);
    if (status != GATT_SUCCESS) {
        HILOGE("failed, status: %{public}d", status);
        connState_ = static_cast<int>(BTConnectState::CONNECTED);
        WPTR_CBACK(callback_, OnConnectionStateChanged, GattStatus::GATT_FAILURE, connState_,
            ServiceUtil::AddrFromStack(bda), convertReason, reasonMessage);
        return;
    }
    BtChrEventWriteTime(CHR_BLE_DISCONNECT, bda.ToString(),
        "BLEPROFILEDISCONNECTTIME");
    connId_ = GATT_INVALID_CONN_ID;
    connState_ = static_cast<int>(BTConnectState::DISCONNECTED);
    WPTR_CBACK(callback_, OnConnectionStateChanged, GattStatus::GATT_SUCCESS, connState_,
        ServiceUtil::AddrFromStack(bda), convertReason, reasonMessage);
}

void GattClientApplication::CancelOpenCallback(int connId, int status, int clientIf, const STACK::RawAddress &bda)
{
    if (clientIf_ != clientIf) {
        return;
    }
    HILOGI("connId: %{public}d, status: %{public}d, clientIf: %{public}d, address: %{public}s",
        connId, status, clientIf, bda.ToStringForLogging().c_str());
    // When status is ERROR, do not handle.
    if (status != GATT_SUCCESS) {
        HILOGE("failed, status: %{public}d", status);
        return;
    }
    connId_ = GATT_INVALID_CONN_ID;
    connState_ = static_cast<int>(BTConnectState::DISCONNECTED);
    WPTR_CBACK(callback_, OnConnectionStateChanged, GattStatus::GATT_SUCCESS, connState_,
        ServiceUtil::AddrFromStack(bda));
}

void GattClientApplication::Disconnect(void)
{
    int convertReason = static_cast<int>(GattDisconnectReason::CONN_TERMINATE_LOCAL_HOST);
    if (connState_ == static_cast<int>(BTConnectState::DISCONNECTED) ||
        connState_ == static_cast<int>(BTConnectState::DISCONNECTING)) {
        HILOGE("Not connected");
        WPTR_CBACK(callback_, OnConnectionStateChanged, GattStatus::GATT_SUCCESS, connState_, addr_, convertReason,
            GATT_DIS_MSG_NO_CONNECTED);
        return;
    }
    if (connState_ == static_cast<int>(BTConnectState::CONNECTING)) {
        HILOGI("It is connecting, needs to cancelOpen.");
        connId_ = 0; // The connId_ must be 0, when cancel open is triggering.
    }

    if (btIfGattClient_) {
        std::string callingName = PermissionManager::GetCallingName();
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_BLE_CLIENT_DISCONN, addr_.GetAddress(), clientIf_,
            callingName);
        int ret = btIfGattClient_->disconnect(clientIf_, ServiceUtil::AddrToStack(addr_), connId_);
        if (ret != BT_STATUS_SUCCESS) {
            HILOGE("failed, ret: %{public}d", ret);
            WPTR_CBACK(callback_, OnConnectionStateChanged, GattStatus::GATT_FAILURE, connState_, addr_, convertReason,
                GATT_DIS_MSG_NO_CONN_TO_CANCEL);
            return;
        }
    }

    // Clear cache
    gattDb_.clear();
    connId_ = GATT_INVALID_CONN_ID;

    connState_ = static_cast<int>(BTConnectState::DISCONNECTING);
    WPTR_CBACK(callback_, OnConnectionStateChanged, GattStatus::GATT_SUCCESS, connState_, addr_);
}

void GattClientApplication::SearchCompleteCallback(int connId, int status)
{
    if (connId != connId_) {
        return;
    }
    HILOGI("connId: %{public}d, status: %{public}d", connId, status);

    if (status != GATT_SUCCESS) {
        HILOGE("failed, status: %{public}d", status);
        WPTR_CBACK(callback_, OnServicesDiscovered, GattStatus::GATT_FAILURE);
        return;
    }
    // Trigger geting gatt services
    GetGattDatabase(connId);
}

void GattClientApplication::DiscoveryServices(void)
{
    if (!IsConnected()) {
        HILOGE("Not connected");
        WPTR_CBACK(callback_, OnServicesDiscovered, GattStatus::GATT_FAILURE);
        return;
    }

    if (btIfGattClient_) {
        int ret = btIfGattClient_->searchService(connId_, nullptr);
        if (ret != BT_STATUS_SUCCESS) {
            HILOGE("failed, ret: %{public}d", ret);
            WPTR_CBACK(callback_, OnServicesDiscovered, GattStatus::GATT_FAILURE);
        }
    }
}

void GattClientApplication::GetGattDbCallback(int connId, const BtgattDbElement *db, int count)
{
    if (connId != connId_) {
        return;
    }
    HILOGI("connId: %{public}d, count: %{public}d", connId, count);
    if (count < 0 || count > UINT16_MAX) {
        HILOGE("Invalid count: %{public}d", count);
        DoInGattThread([callback = callback_]() {
            WPTR_CBACK(callback, OnServicesDiscovered, GattStatus::GATT_FAILURE);
        });
        return;
    }
    // Convert db to gatt services
    std::vector<Service> svc;
    const BtgattDbElement *p = db;
    const BtgattDbElement *end = db + count;
    while (p < end) {
        Uuid uuid = p->uuid;
        int perm = p->permissions;
#ifdef BLUETOOTH_WATCH_ENABLE
        if (uuid.ToString().compare(CONTROL_POINT_UUID) == 0) {
            std::string handle = std::to_string(p->attributeHandle);
            WatchService::GetInstance()->BluetoothDataShareUpdate(STATUS_SETTING_MODE_URI, CONTROL_POINT_KEY, handle);
        } else if (uuid.ToString().compare(NOTIFICATION_SOURCE_UUID) == 0) {
            std::string handle = std::to_string(p->attributeHandle);
            WatchService::GetInstance()->BluetoothDataShareUpdate(STATUS_SETTING_MODE_URI,
                NOTIFICATION_SOURCE_KEY, handle);
        } else if (uuid.ToString().compare(DATA_SOURCE_UUID) == 0) {
            std::string handle = std::to_string(p->attributeHandle);
            WatchService::GetInstance()->BluetoothDataShareUpdate(STATUS_SETTING_MODE_URI, DATA_SOURCE_KEY, handle);
        }
#endif
        if ((p->type == BTGATT_DB_PRIMARY_SERVICE) || (p->type == BTGATT_DB_SECONDARY_SERVICE)) {
            svc.push_back({uuid, p->attributeHandle, p->startHandle, p->endHandle});
        } else if (p->type == BTGATT_DB_CHARACTERISTIC) {
            svc.back().characteristics_.push_back({uuid, p->attributeHandle, p->properties, perm, nullptr, 0});
        } else if (p->type == BTGATT_DB_DESCRIPTOR) {
            svc.back().characteristics_.back().descriptors_.push_back({uuid, p->attributeHandle, perm, nullptr, 0});
        } else if (p->type == BTGATT_DB_INCLUDED_SERVICE) {
            HILOGE("Unsupported now");
        }
        p++;
    }
    std::swap(gattDb_, svc);
    DoInGattThread([callback = callback_]() {
        WPTR_CBACK(callback, OnServicesDiscovered, GattStatus::GATT_SUCCESS);
    });
}

void GattClientApplication::GetGattDatabase(int connId)
{
    if (btIfGattClient_) {
        int ret = btIfGattClient_->getGattDb(connId);
        if (ret != BT_STATUS_SUCCESS) {
            HILOGE("failed to get gatt db, ret: %{public}d", ret);
            WPTR_CBACK(callback_, OnServicesDiscovered, GattStatus::GATT_FAILURE);
            return;
        }
    }
}

std::vector<Service> GattClientApplication::GetServices(void)
{
    return gattDb_;
}

void GattClientApplication::ReadCharacteristicCallback(int connId, int status, BtgattReadParams *pData)
{
    if (connId != connId_) {
        return;
    }
    HILOGI("connId: %{public}d, status: %{public}d, handle: %{public}#x, valueType: %{public}u,"
        "value_status: %{public}u, value_len: %{public}u",
        connId, status, pData->handle, pData->valueType, pData->status, pData->value.len);

    auto value = GattServiceBase::BuildGattValue(pData->value.value, pData->value.len);
    std::shared_ptr<GattClientApplication> applicationPtr = shared_from_this();
    DoInGattThread([applicationPtr, status, value, handle = pData->handle, len = pData->value.len]() {
        if (applicationPtr->CheckAuthentication(
            applicationPtr->connId_, status, handle, GATT_TYPE_READ_CHARACTERISTIC)) {
            return;
        }
        Characteristic characteristic(handle);
        characteristic.value_ = std::move(*value);
        characteristic.length_ = len;
        int ret = GattServiceBase::GattStatusFromStack(status);
        WPTR_CBACK(applicationPtr->callback_, OnCharacteristicRead, ret, characteristic);
    });
}

void GattClientApplication::ReadCharacteristic(uint16_t handle)
{
    if (btIfGattClient_) {
        int ret = btIfGattClient_->readCharacteristic(connId_, handle, GATT_AUTH_REQ_NONE);
        if (ret != BT_STATUS_SUCCESS) {
            HILOGE("failed, ret: %{public}d", ret);
            WPTR_CBACK(callback_, OnCharacteristicRead, GattStatus::GATT_FAILURE, Characteristic(handle));
        }
    }
}

void GattClientApplication::ReadCharacteristicByUuid(const Uuid &uuid, int32_t startHandle, int32_t endHandle)
{
    if (btIfGattClient_) {
        int ret = btIfGattClient_->readUsingCharacteristicUuid(connId_, uuid,
            startHandle, endHandle, GATT_AUTH_REQ_NONE);
        if (ret != BT_STATUS_SUCCESS) {
            WPTR_CBACK(callback_, OnCharacteristicRead, GattStatus::GATT_FAILURE,
                Characteristic(startHandle, endHandle));
            HILOGE("failed, ret: %{public}d", ret);
        }
    }
}

void GattClientApplication::WriteCharacteristicCallback(int connId, int status, uint16_t handle,
    const BtgattRspParams &rspContext)
{
    if (connId != connId_) {
        return;
    }
    HILOGD("connId: %{public}d, status: %{public}d, handle: %{public}#x, timeStamp: %{public}llu", connId, status,
        handle, rspContext.timeStamp);

    // Notify client service if write no respond callback, no needed to callback.
    if (writeNoRespondSem_) {
        writeNoRespondSem_->Post();
        writeNoRespondSem_ = nullptr;
    }
    if (CheckAuthentication(connId_, status, handle, GATT_TYPE_WRITE_CHARACTERISTIC)) {
        return;
    }
    Characteristic characteristic(handle);
    int ret = GattServiceBase::GattStatusFromStack(status);
    // int64_t is enough for microseconds timeStamp till the year of 2262
    BluetoothGattRspContext context(static_cast<int64_t>(rspContext.timeStamp));
    WPTR_CBACK(callback_, OnCharacteristicWrite, ret, characteristic, context);
}

bool GattClientApplication::WriteCharacteristicInner(uint16_t handle, int writeType, int authReq,
    std::vector<uint8_t> value)
{
    bool ok = true;
    cacheValue_ = value;
    if (btIfGattClient_) {
        int ret = btIfGattClient_->writeCharacteristic(connId_, handle, writeType, authReq, std::move(value));
        if (ret != BT_STATUS_SUCCESS) {
            HILOGE("failed, ret: %{public}d", ret);
            WPTR_CBACK(callback_, OnCharacteristicWrite, GattStatus::GATT_FAILURE, Characteristic(handle),
                BluetoothGattRspContext());
            ok = false;
        }
    }

    return ok;
}

void GattClientApplication::WriteCharacteristic(uint16_t handle, std::vector<uint8_t> value)
{
    // bluedroid inner support "write long characteristic value procedure"
    WriteCharacteristicInner(handle, STACK::GATT_WRITE, GATT_AUTH_REQ_NONE, std::move(value));
}

void GattClientApplication::WriteCharacteristicNoRespond(uint16_t handle, std::vector<uint8_t> value,
    std::shared_ptr<utility::Semaphore> semaphore)
{
    writeNoRespondSem_ = semaphore;
    bool ok = WriteCharacteristicInner(handle, STACK::GATT_WRITE_NO_RSP, GATT_AUTH_REQ_NONE, std::move(value));
    if (!ok) {
        semaphore->Post();
        writeNoRespondSem_ = nullptr;
        return;
    }
}

void GattClientApplication::ReadDescriptorCallback(int connId, int status, const BtgattReadParams &data)
{
    if (connId != connId_) {
        return;
    }
    HILOGI("connId: %{public}d, status: %{public}d, handle: %{public}#x, valueType: %{public}u,"
        "value_status: %{public}u, value_len: %{public}u",
        connId, status, data.handle, data.valueType, data.status, data.value.len);

    auto value = GattServiceBase::BuildGattValue(data.value.value, data.value.len);
    std::shared_ptr<GattClientApplication> applicationPtr = shared_from_this();
    DoInGattThread([applicationPtr, status, value, handle = data.handle, len = data.value.len]() {
        if (applicationPtr->CheckAuthentication(
            applicationPtr->connId_, status, handle, GATT_TYPE_READ_DESCRIPTOR)) {
            return;
        }
        Descriptor descriptor(handle);
        descriptor.value_ = std::move(*value);
        descriptor.length_ = len;
        int ret = GattServiceBase::GattStatusFromStack(status);
        WPTR_CBACK(applicationPtr->callback_, OnDescriptorRead, ret, descriptor);
    });
}

void GattClientApplication::ReadDescriptor(uint16_t handle)
{
    if (btIfGattClient_) {
        int ret = btIfGattClient_->readDescriptor(connId_, handle, GATT_AUTH_REQ_NONE);
        if (ret != BT_STATUS_SUCCESS) {
            HILOGE("failed, ret: %{public}d", ret);
            WPTR_CBACK(callback_, OnDescriptorRead, GattStatus::GATT_FAILURE, Descriptor(handle));
        }
    }
}

void GattClientApplication::WriteDescriptorCallback(int connId, int status, uint16_t handle)
{
    if (connId != connId_) {
        return;
    }
    HILOGI("connId: %{public}d, status: %{public}d, handle: %{public}#x", connId, status, handle);

    if (CheckAuthentication(connId, status, handle, GATT_TYPE_WRITE_DESCRIPTOR)) {
        return;
    }
    Descriptor descriptor(handle);
    int ret = GattServiceBase::GattStatusFromStack(status);
    WPTR_CBACK(callback_, OnDescriptorWrite, ret, descriptor);
}

bool GattClientApplication::HandleWriteCharacteristic(int ret, int connId, uint16_t handle, int authReq)
{
    ret = btIfGattClient_->writeCharacteristic(connId, handle, STACK::GATT_WRITE, authReq, cacheValue_);
    if (ret != BT_STATUS_SUCCESS) {
        HILOGE("failed, ret: %{public}d", ret);
        WPTR_CBACK(callback_, OnCharacteristicWrite, GattStatus::GATT_FAILURE, Characteristic(handle),
            BluetoothGattRspContext());
        return true;
    }
    return false;
}

bool GattClientApplication::CheckAuthentication(int connId, int status, uint16_t handle, int gattType)
{
    if ((status == OHOS_GATT_INSUFFICIENT_AUTHENTICATION || status == OHOS_GATT_INSUFFICIENT_ENCRYPTION) &&
            (authRetryState_ != AUTH_RETRY_STATE_MITM)) {
        int authReq = (authRetryState_ == AUTH_RETRY_STATE_IDLE) ? AUTHENTICATION_NO_MITM : AUTHENTICATION_MITM;
        CHECK_AND_RETURN_LOG_RET(btIfGattClient_, false, "btIfGattClient_ is nullptr");
        int ret = BT_STATUS_NOT_READY;
        switch (gattType) {
            case GATT_TYPE_READ_CHARACTERISTIC:
                ret = btIfGattClient_->readCharacteristic(connId, handle, authReq);
                if (ret != BT_STATUS_SUCCESS) {
                    HILOGE("failed, ret: %{public}d", ret);
                    WPTR_CBACK(callback_, OnCharacteristicRead, GattStatus::GATT_FAILURE, Characteristic(handle));
                    return true;
                }
                break;
            case GATT_TYPE_READ_DESCRIPTOR:
                ret = btIfGattClient_->readDescriptor(connId, handle, authReq);
                if (ret != BT_STATUS_SUCCESS) {
                    HILOGE("failed, ret: %{public}d", ret);
                    WPTR_CBACK(callback_, OnDescriptorRead, GattStatus::GATT_FAILURE, Descriptor(handle));
                    return true;
                }
                break;
            case GATT_TYPE_WRITE_CHARACTERISTIC:
                if (HandleWriteCharacteristic(ret, connId, handle, authReq)) {
                    return true;
                }
                break;
            case GATT_TYPE_WRITE_DESCRIPTOR:
                ret = btIfGattClient_->writeDescriptor(connId, handle, authReq, cacheValue_);
                if (ret != BT_STATUS_SUCCESS) {
                    HILOGE("failed, ret: %{public}d", ret);
                    WPTR_CBACK(callback_, OnDescriptorWrite, GattStatus::GATT_FAILURE, Descriptor(handle));
                    return true;
                }
                break;
            default:
                HILOGE("Unsupported gattType.");
                return false;
        }
        authRetryState_++;
        return true;
    }
    authRetryState_ = AUTH_RETRY_STATE_IDLE;
    return false;
}

void GattClientApplication::WriteDescriptor(uint16_t handle, const std::vector<uint8_t> &value)
{
    int ret = 0;
    cacheValue_ = value;
    // Send write descriptor packet to the peer device.
    if (btIfGattClient_) {
        ret = btIfGattClient_->writeDescriptor(connId_, handle, GATT_AUTH_REQ_NONE, value);
        if (ret != BT_STATUS_SUCCESS) {
            HILOGE("failed, ret: %{public}d", ret);
            WPTR_CBACK(callback_, OnDescriptorWrite, GattStatus::GATT_FAILURE, Descriptor(handle));
        }
    }
}

void GattClientApplication::RegisterForNotificationCallback(int connId, int registered, int status, uint16_t handle)
{
    HILOGI("connId: %{public}d, registered: %{public}d, status: %{public}d, handle: %{public}#x",
        connId, registered, status, handle);
}

int GattClientApplication::RequestNotification(uint16_t characteristicHandle, bool enable)
{
    int ret = BT_STATUS_FAIL;
    if (btIfGattClient_) {
        if (enable) {
            ret = btIfGattClient_->registerForNotification(
                clientIf_, ServiceUtil::AddrToStack(addr_), characteristicHandle);
        } else {
            ret = btIfGattClient_->deregisterForNotification(
                clientIf_, ServiceUtil::AddrToStack(addr_), characteristicHandle);
        }
    }
    return ret;
}

void GattClientApplication::ConfigureMtuCallback(int connId, int status, int mtu)
{
    if (connId != connId_) {
        return;
    }
    HILOGI("connId: %{public}d, status: %{public}d, mtu: %{public}d", connId, status, mtu);
    BtChrEventWriteInt(CHR_BLE_DISCONNECT, ServiceUtil::AddrToStack(GetAddress()).ToString(),
        "BTCONMTUINITIATOR", 0);
    BtChrEventWriteInt(CHR_BLE_DISCONNECT, ServiceUtil::AddrToStack(GetAddress()).ToString(),
        "BTCONMTURESULT", mtu);

    int ret = GattServiceBase::GattStatusFromStack(status);
    WPTR_CBACK(callback_, OnMtuChanged, ret, mtu);
}

void GattClientApplication::RequestExchangeMtu(int mtu)
{
    if (btIfGattClient_) {
        int ret = btIfGattClient_->configureMtu(connId_, mtu);
        if (ret != BT_STATUS_SUCCESS) {
            HILOGE("failed, ret: %{public}d", ret);
        }
    }
}

void GattClientApplication::NotifyCallback(int connId, const BtgattNotifyParams &data)
{
    if (connId != connId_) {
        return;
    }
    HILOGD("connId: %{public}d, addr: %{public}s, handle: %{public}#x",
        connId, data.bda.ToStringForLogging().c_str(), data.handle);

    auto value = GattServiceBase::BuildGattValue(data.value, data.len);
    std::shared_ptr<GattClientApplication> applicationPtr = shared_from_this();
    DoInGattThread([applicationPtr, value, handle = data.handle, len = data.len]() {
        Characteristic characteristic(handle);
        characteristic.value_ = std::move(*value);
        characteristic.length_ = len;
        WPTR_CBACK(applicationPtr->callback_, OnCharacteristicChanged, characteristic);
    });
}

void GattClientApplication::ConnUpdatedCallback(int connId, uint16_t interval, uint16_t latency, uint16_t timeout,
    uint8_t status)
{
    if (connId != connId_) {
        return;
    }
    HILOGI("connId: %{public}d, interval: %{public}u, latency: %{public}u, timeout: %{public}u, status: %{public}u",
        connId, interval, latency, timeout, status);

    int ret = GattServiceBase::GattStatusFromStack(status);
    WPTR_CBACK(callback_, OnConnectionParameterChanged, interval, latency, timeout, ret);
}

void GattClientApplication::ServicesChangedCallback(int connId)
{
    HILOGI("connId: %{public}d", connId);
    if (connId != connId_) {
        return;
    }
    WPTR_CBACK(callback_, OnServicesChanged);
}

void GattClientApplication::RequestConnectionPriority(int connPriority)
{
    if (!IsConnected()) {
        HILOGE("Not connected");
        WPTR_CBACK(callback_, OnConnectionParameterChanged, 0, 0, 0, GattStatus::GATT_FAILURE);
        return;
    }

    if (btIfGattClient_) {
        int ret = btIfGattClient_->connParameterUpdate(
            ServiceUtil::AddrToStack(addr_),
            GetBleMinConnectionInterval(connPriority),
            GetBleMaxConnectionInterval(connPriority),
            GetBleConnectionLatency(connPriority),
            GetBleConnectionSupervisionTimeout(connPriority),
            0,
            0);
        if (ret != BT_STATUS_SUCCESS) {
            HILOGE("failed, ret: %{public}d", ret);
            WPTR_CBACK(callback_, OnConnectionParameterChanged, 0, 0, 0, GattStatus::GATT_FAILURE);
        }
    }
}

int GattClientApplication::GetBleMinConnectionInterval(int connPriority)
{
    int result = DEFAULT_BLE_MIN_CONNECTION_INTERVAL;
    std::string property = PROPERTY_BALANCED_PRIORITY_MIN_INTERVAL;
    if (connPriority == static_cast<int>(GattConnectionPriority::LOW_POWER)) {
        property = PROPERTY_LOW_POWER_MIN_INTERVAL;
    } else if (connPriority == static_cast<int>(GattConnectionPriority::HIGH)) {
        property = PROPERTY_HIGH_PRIORITY_MIN_INTERVAL;
    } else if (connPriority == static_cast<int>(GattConnectionPriority::LOW_POWER_WEAK)) {
        property = PROPERTY_LOW_POWER_WEAK_MIN_INTERVAL;
    } else if (connPriority == static_cast<int>(GattConnectionPriority::LOW_POWER_ENHANCE)) {
        property = PROPERTY_LOW_POWER_ENHANCE_MIN_INTERVAL;
    } else if (connPriority == static_cast<int>(GattConnectionPriority::LOW_POWER_ULTRA)) {
        property = PROPERTY_LOW_POWER_ULTRA_MIN_INTERVAL;
    }

    AdapterConfig::GetInstance()->GetValue(SECTION_GATT_SERVICE, property, result);
    return result;
}

int GattClientApplication::GetBleMaxConnectionInterval(int connPriority)
{
    int result = DEFAULT_BLE_MAX_CONNECTION_INTERVAL;
    const std::string *property = &PROPERTY_BALANCED_PRIORITY_MAX_INTERVAL;
    if (connPriority == static_cast<int>(GattConnectionPriority::LOW_POWER)) {
        property = &PROPERTY_LOW_POWER_MAX_INTERVAL;
    } else if (connPriority == static_cast<int>(GattConnectionPriority::HIGH)) {
        property = &PROPERTY_HIGH_PRIORITY_MAX_INTERVAL;
    } else if (connPriority == static_cast<int>(GattConnectionPriority::LOW_POWER_WEAK)) {
        property = &PROPERTY_LOW_POWER_WEAK_MAX_INTERVAL;
    } else if (connPriority == static_cast<int>(GattConnectionPriority::LOW_POWER_ENHANCE)) {
        property = &PROPERTY_LOW_POWER_ENHANCE_MAX_INTERVAL;
    } else if (connPriority == static_cast<int>(GattConnectionPriority::LOW_POWER_ULTRA)) {
        property = &PROPERTY_LOW_POWER_ULTRA_MAX_INTERVAL;
    }

    AdapterConfig::GetInstance()->GetValue(SECTION_GATT_SERVICE, *property, result);
    return result;
}

int GattClientApplication::GetBleConnectionLatency(int connPriority)
{
    int result = DEFAULT_BLE_CONNECTION_LATENCY;
    const std::string *property = &PROPERTY_BALANCED_PRIORITY_LATENCY;
    if (connPriority == static_cast<int>(GattConnectionPriority::LOW_POWER)) {
        property = &PROPERTY_LOW_POWER_LATENCY;
    } else if (connPriority == static_cast<int>(GattConnectionPriority::HIGH)) {
        property = &PROPERTY_HIGH_PRIORITY_LATENCY;
    } else if (connPriority == static_cast<int>(GattConnectionPriority::LOW_POWER_WEAK)) {
        property = &PROPERTY_LOW_POWER_WEAK_LATENCY;
    } else if (connPriority == static_cast<int>(GattConnectionPriority::LOW_POWER_ENHANCE)) {
        property = &PROPERTY_LOW_POWER_ENHANCE_LATENCY;
    } else if (connPriority == static_cast<int>(GattConnectionPriority::LOW_POWER_ULTRA)) {
        property = &PROPERTY_LOW_POWER_ULTRA_LATENCY;
    }

    AdapterConfig::GetInstance()->GetValue(SECTION_GATT_SERVICE, *property, result);
    return result;
}

int GattClientApplication::GetBleConnectionSupervisionTimeout(int connPriority)
{
    int result = DEFAULT_BLE_CONNECTION_SUPERVISION_TIMEOUT;
    const std::string *property = &PROPERTY_BALANCED_PRIORITY_TIMEOUT;
    if (connPriority == static_cast<int>(GattConnectionPriority::LOW_POWER)) {
        property = &PROPERTY_LOW_POWER_TIMEOUT;
    } else if (connPriority == static_cast<int>(GattConnectionPriority::HIGH)) {
        property = &PROPERTY_HIGH_PRIORITY_TIMEOUT;
    } else if (connPriority == static_cast<int>(GattConnectionPriority::LOW_POWER_WEAK)) {
        property = &PROPERTY_LOW_POWER_WEAK_TIMEOUT;
    } else if (connPriority == static_cast<int>(GattConnectionPriority::LOW_POWER_ENHANCE)) {
        property = &PROPERTY_LOW_POWER_ENHANCE_TIMEOUT;
    } else if (connPriority == static_cast<int>(GattConnectionPriority::LOW_POWER_ULTRA)) {
        property = &PROPERTY_LOW_POWER_ULTRA_TIMEOUT;
    }

    AdapterConfig::GetInstance()->GetValue(SECTION_GATT_SERVICE, *property, result);
    return result;
}

void GattClientApplication::ReadRemoteRssiValue(int appId)
{
    if (btIfGattClient_) {
        int ret = btIfGattClient_->readRemoteRssi(appId, ServiceUtil::AddrToStack(addr_));
        if (ret != BT_STATUS_SUCCESS) {
            HILOGE("failed, ret: %{public}d", ret);
            WPTR_CBACK(callback_, OnReadRemoteRssiValue, addr_, 0, GattStatus::GATT_FAILURE);
        }
    }
}

void GattClientApplication::ReadRemoteRssiValueCallback(int clientIf, const STACK::RawAddress &bda,
    int rssi, int status)
{
    if (clientIf_ != clientIf) {
        return;
    }
    HILOGI("rssi: %{public}d, status: %{public}d, clientIf: %{public}d, address: %{public}s",
        rssi, status, clientIf, bda.ToStringForLogging().c_str());
    int ret = GattServiceBase::GattStatusFromStack(status);
    WPTR_CBACK(callback_, OnReadRemoteRssiValue, ServiceUtil::AddrFromStack(bda), rssi, ret);
}

void GattClientApplication::PhyUpdatedCallback(int connId, uint8_t txPhy, uint8_t rxPhy, uint8_t status)
{
    if (connId != connId_) {
        return;
    }
    HILOGI("connId: %{public}d, txPhy: %{public}u, rxPhy: %{public}u, status: %{public}u",
        connId, txPhy, rxPhy, status);
 
    int ret = GattServiceBase::GattStatusFromStack(status);
    WPTR_CBACK(callback_, OnBlePhyUpdate, txPhy, rxPhy, ret);
}
 
void GattClientApplication::SetPhy(int32_t txPhy, int32_t rxPhy, int32_t phyOptions)
{
    int txPhyMask = ConvertToBlePhyMask(txPhy);
    int rxPhyMask = ConvertToBlePhyMask(rxPhy);

    if (btIfGattClient_) {
        int ret = btIfGattClient_->setPreferredPhy(
            ServiceUtil::AddrToStack(addr_), txPhyMask, rxPhyMask, phyOptions);
        if (ret != BT_STATUS_SUCCESS) {
            HILOGE("failed, ret: %{public}d", ret);
            WPTR_CBACK(callback_, OnBlePhyUpdate, 0, 0, GattStatus::GATT_FAILURE);
        }
    }
}
 
void GattClientApplication::ReadPhyCallback(uint8_t txPhy, uint8_t rxPhy, uint8_t status)
{
    HILOGI("txPhy: %{public}d, rxPhy: %{public}d, status: %{public}d", txPhy, rxPhy, status);
    int ret = GattServiceBase::GattStatusFromStack(status);
    WPTR_CBACK(callback_, OnBlePhyRead, txPhy, rxPhy, ret);
}
 
void GattClientApplication::ReadPhy(void)
{
    if (btIfGattClient_) {
        int ret = btIfGattClient_->readPhy(ServiceUtil::AddrToStack(addr_),
                [this](uint8_t txPhy, uint8_t rxPhy, uint8_t status) { ReadPhyCallback(txPhy, rxPhy, status); });
        if (ret != BT_STATUS_SUCCESS) {
            HILOGE("failed, ret: %{public}d", ret);
            WPTR_CBACK(callback_, OnBlePhyRead, 0, 0, GattStatus::GATT_FAILURE);
        }
    }
}

}  // namespace bluetooth
}  // namespace OHOS
