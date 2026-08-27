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
#define LOG_TAG "bt_service_gatt_server"
#endif

#include "gatt_server_application.h"

#include <algorithm>
#include <vector>

#include "btif_config.h"
#include "bt_chr_ue_manager.h"
#include "common_util.h"
#include "gatt_api.h"
#include "gatt_data.h"
#include "log.h"
#include "permission_manager.h"
#include "securec.h"
#include "thread_util.h"
#include "bluetooth_resource_manager.h"
#include "bluetooth_state_manager.h"
#include "control_intercept_plugin.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;
GattServerApplication::GattServerApplication(int appId, const btgatt_server_interface_t *interface, uint32_t tokenId,
    std::weak_ptr<IGattServerCallback> callback, NotifyServiceChangedFunc func)
    : serverIf_(appId),
      connIdMap_(),
      serviceHandles_(),
      btIfGattServer_(interface),
      callback_(callback),
      notifyServiceChanged_(func),
      tokenId_(tokenId),
      isSystemHap_(PermissionManager::IsSystemHap(tokenId_)),
      isNativeCaller_(PermissionManager::IsNativeCaller(tokenId_))
{
}

GattServerApplication::~GattServerApplication()
{
    if (btIfGattServer_) {
        int ret = btIfGattServer_->unregister_server(serverIf_);
        if (ret != BT_STATUS_SUCCESS) {
            HILOGE("unregister server failed, ret: %{public}d", ret);
        }
    }
}

// valueHandle -> characteristic handle
static uint16_t FromValueHandle(uint16_t valueHandle)
{
    if (valueHandle < GATT_HANDLE_MIN + 1) {
        HILOGE("Invalid handle: %{public}d", valueHandle);
        return 0;
    }
    return valueHandle - 1;
}
// characteristic handle -> valueHandle
static uint16_t ToValueHandle(uint16_t characteristicHandle)
{
    if (characteristicHandle > GATT_HANDLE_MAX - 1) {
        HILOGE("Invalid handle: %{public}d", characteristicHandle);
        return 0;
    }
    return characteristicHandle + 1;
}

namespace {
std::string PermissionsLog(uint16_t permissions)
{
    static const std::map<GattPermission, std::string> table {
        {GattPermission::READABLE, "Read"},
        {GattPermission::READ_ENCRYPTED, "ReadEncrypted"},
        {GattPermission::READ_ENCRYPTED_MITM, "ReadableEncrypted_MITM"},
        {GattPermission::WRITEABLE, "Write"},
        {GattPermission::WRITE_ENCRYPTED, "WriteEncrypted"},
        {GattPermission::WRITE_ENCRYPTED_MITM, "WriteEncrypted_MITM"},
        {GattPermission::WRITE_SIGNED, "WriteSigned"},
        {GattPermission::WRITE_SIGNED_MITM, "WriteSigned_MITM"},
    };

    std::string ret = "permissions(" + std::to_string(permissions) + "):";
    for (const auto &[perm, desc] : table) {
        if (permissions & static_cast<uint16_t>(perm)) {
            ret += " " + desc;
        }
    }
    return ret;
}

std::string PropertiesLog(uint8_t properties)
{
    static const std::map<GattProperties, std::string> table {
        {GattProperties::BROADCAST, "Broadcast"},
        {GattProperties::READ, "Read"},
        {GattProperties::WRITE_WITHOUT_RESPONSE, "WriteNoRsp"},
        {GattProperties::WRITE, "Write"},
        {GattProperties::NOTIFY, "Notify"},
        {GattProperties::INDICATE, "Indicate"},
        {GattProperties::AUTHENTICATED_SIGNED_WRITES, "SignedWrite"},
        {GattProperties::EXTENDED_PROPERTIES, "ExtendedProperties"},
    };

    std::string ret = "properties(" + std::to_string(properties) + "):";
    for (const auto &[prop, desc] : table) {
        if (properties & static_cast<uint8_t>(prop)) {
            ret += " " + desc;
        }
    }
    return ret;
}
}

void GattServerApplication::BuildGattService(const std::vector<btgatt_db_element_t> &svc, Service &service)
{
    for (auto iter = svc.begin(); iter != svc.end(); iter++) {
        Uuid uuid = iter->uuid;
        switch (iter->type) {
            case BTGATT_DB_PRIMARY_SERVICE:
            case BTGATT_DB_SECONDARY_SERVICE: {
                service.handle_ = iter->attribute_handle;
                service.uuid_ = uuid;
                service.isPrimary_ = (iter->type == BTGATT_DB_PRIMARY_SERVICE);
                HILOGD("Service uuid: %{public}s, handle: %{public}#x, isPrimary: %{public}d",
                    iter->uuid.ToString().c_str(), iter->attribute_handle, iter->type);
                break;
            }
            case BTGATT_DB_CHARACTERISTIC: {
                uint16_t handle = FromValueHandle(iter->attribute_handle);  // Characteristic handle
                Characteristic ccc(uuid, handle, iter->properties, iter->permissions, nullptr, 0);
                service.characteristics_.push_back(std::move(ccc));
                // Store attribute handles
                serviceHandles_[service.handle_].insert(handle);
                serviceHandles_[service.handle_].insert(iter->attribute_handle);
                HILOGD("Character uuid: %{public}s, handle: %{public}#x, permissions: %{public}s, "
                    "properties: %{public}s", iter->uuid.ToString().c_str(), handle,
                    PermissionsLog(iter->permissions).c_str(), PropertiesLog(iter->properties).c_str());
                break;
            }
            case BTGATT_DB_DESCRIPTOR: {
                Descriptor descriptor(uuid, iter->attribute_handle, iter->permissions, nullptr, 0);
                service.characteristics_.back().descriptors_.push_back(std::move(descriptor));
                // Store attribute handles
                serviceHandles_[service.handle_].insert(iter->attribute_handle);
                HILOGD("Descriptor uuid: %{public}s, handle: %{public}#x, permissions: %{public}s",
                    iter->uuid.ToString().c_str(), iter->attribute_handle, PermissionsLog(iter->permissions).c_str());
                break;
            }
            case BTGATT_DB_INCLUDED_SERVICE:
            default:
                HILOGE("Unsupported, type: %{public}d", iter->type);
                break;
        }
    }
}

void GattServerApplication::ReportAddService(int ret, const Service &service)
{
    WPTR_CBACK(callback_, OnAddService, ret, service);
}

void GattServerApplication::ServiceAddedCallback(int status, int serverIf, std::vector<btgatt_db_element_t> svc)
{
    if (serverIf != serverIf_) {
        return;
    }
    HILOGD("status: %{public}d, serverIf: %{public}d", status, serverIf);

    auto resourceMgr = BluetoothResourceManager::GetInstance();
    if (resourceMgr) {
        resourceMgr->SendSensingStateChanged(GATT_SERVER_REGISTER_DONE, SensingInfo(serverIf));
    }
    // Build OH Gatt service
    Service service;
    BuildGattService(svc, service);

    if (status != GATT_SUCCESS) {
        HILOGE("Add service failed, status: %{public}d", status);
        ReportAddService(GattStatus::GATT_FAILURE, service);
        return;
    }
    ReportAddService(GattStatus::GATT_SUCCESS, service);
    notifyServiceChanged_(serverIf_, service);
}

void GattServerApplication::AddService(const Service &service)
{
    std::vector<btgatt_db_element_t> svc;
    svc.push_back({
        .uuid = service.uuid_,
        .type = (service.isPrimary_ ? BTGATT_DB_PRIMARY_SERVICE : BTGATT_DB_SECONDARY_SERVICE),
    });
    for (const auto &characteristic : service.characteristics_) {
        svc.push_back({
            .uuid = characteristic.uuid_,
            .type = BTGATT_DB_CHARACTERISTIC,
            .properties = static_cast<uint8_t>(characteristic.properties_),
            .permissions = static_cast<uint16_t>(characteristic.permissions_),
        });
        for (const auto &descriptor : characteristic.descriptors_) {
            svc.push_back({
                .uuid = descriptor.uuid_,
                .type = BTGATT_DB_DESCRIPTOR,
                .permissions = static_cast<uint16_t>(characteristic.permissions_),
            });
        }
    }
    for (const auto &include_svc : service.includeServices_) {
        svc.push_back({
            .type = BTGATT_DB_INCLUDED_SERVICE,
            .attribute_handle = include_svc.startHandle_,
        });
    }

    if (btIfGattServer_) {
        int ret = btIfGattServer_->add_service(serverIf_, std::move(svc));
        if (ret != BT_STATUS_SUCCESS) {
            HILOGE("Add service failed, ret: %{public}d", ret);
            return;
        }
    }
}

void GattServerApplication::ServiceDeletedCallback(int status, int serverIf, int serviceHandle)
{
    if (serverIf != serverIf_) {
        return;
    }
    HILOGI("status: %{public}d, serverIf: %{public}d, serviceHandle: %{public}#x", status, serverIf, serviceHandle);

    if (status != GATT_SUCCESS) {
        HILOGE("remove service failed, status: %{public}d", status);
        return;
    }
    // Delete stored attribute handles
    serviceHandles_.erase(serviceHandle);

    auto resourceMgr = BluetoothResourceManager::GetInstance();
    if (resourceMgr) {
        resourceMgr->SendSensingStateChanged(GATT_SERVER_UNREGISTER_DONE, SensingInfo(serverIf));
    }

    uint16_t handle = static_cast<uint16_t>(serviceHandle);
    notifyServiceChanged_(serverIf_, Service(handle));
}

void GattServerApplication::RemoveService(uint16_t handle)
{
    if (btIfGattServer_) {
        int ret = btIfGattServer_->delete_service(serverIf_, handle);
        if (ret != BT_STATUS_SUCCESS) {
            HILOGE("Remove service failed, ret: %{public}d", ret);
            return;
        }
    }
}

void GattServerApplication::ClearServices(void)
{
    std::vector<uint16_t> removedServices;
    for (auto iter = serviceHandles_.begin(); iter != serviceHandles_.end(); iter++) {
        removedServices.push_back(iter->first);
    }
    for (uint16_t handle : removedServices) {
        RemoveService(handle);
    }
}

void GattServerApplication::ConnectionCallback(int connId, int serverIf, int connected,
    const STACK::RawAddress &bda, int reason)
{
    if (serverIf != serverIf_) {
        return;
    }
    HILOGI("connId: %{public}d, serverIf: %{public}d, connected: %{public}d, address: %{public}s",
        connId, serverIf, connected, bda.ToStringForLogging().c_str());

    int convertReason = static_cast<int>(GattDisconnectReason::CONN_UNKNOWN);
    std::string reasonMessage = "";
    GattServiceBase::ConvertDisconnectReason(convertReason, reason, reasonMessage);
    uint8_t eventId = (connected) ? GATT_SERVER_CONNECT_DONE : GATT_SERVER_CONNECT_CLOSE;
    auto resourceMgr = BluetoothResourceManager::GetInstance();
    if (resourceMgr) {
        resourceMgr->SendSensingStateChanged(eventId, SensingInfo(bda.ToString(), static_cast<uint32_t>(serverIf)));
    }
    // to support other transport, or detect transport
    GattDevice device(ServiceUtil::AddrFromStack(bda), GATT_TRANSPORT_TYPE_LE, GATT_ROLE_SECONDARY);
    int state = 0;
    if (connected) {
        state = static_cast<int>(BTConnectState::CONNECTED);
        device.SetState(state);
        connIdMap_.emplace(connId, device);
        BluetoothStateManager::GetInstance()->AddDeviceProfileConnectState(
            PROFILE_NAME_GATT_SERVER, bda.ToString(), BTConnectState::CONNECTED);
    } else {
        state = static_cast<int>(BTConnectState::DISCONNECTED);
        connIdMap_.erase(connId);
    }

    WPTR_CBACK(callback_, OnConnectionStateChanged, device, GattStatus::GATT_SUCCESS, state, convertReason,
        reasonMessage);
}

void GattServerApplication::RequestReadCallback(const RequestReadCallbackContext &ctx, GattElement type)
{
    ControlInterceptMessage msg {
        .addr = ctx.addr.GetAddress(),
        .isSystemHap = isSystemHap_,
        .isNativeCaller = isNativeCaller_,
    };
    if (!ControlInterceptIsAllowedGattReceive(msg)) {
        return;
    }
    // ATT_READ_BLOB_REQ
    if (ctx.isLong) {
        ProcessLongValueRead(ctx);
        return;
    }

    // ATT_READ_REQ, Request
    auto connIter = connIdMap_.find(ctx.connId);
    if (connIter == connIdMap_.end()) {
        HILOGE("Invalid connId: %{public}d", ctx.connId);
        return;
    }

    uint16_t reqHandle = static_cast<uint16_t>(ctx.handle);
    connIter->second.transId = ctx.transId;
    if (type == GattElement::CHARACTERISTIC) {
        WPTR_CBACK(callback_, OnCharacteristicReadRequest,
            connIter->second.device, Characteristic(FromValueHandle(reqHandle)));
    } else if (type == GattElement::DESCRIPTOR) {
        WPTR_CBACK(callback_, OnDescriptorReadRequest, connIter->second.device, Descriptor(reqHandle));
    } else {
        HILOGE("Unknown type: %{public}d", type);
    }
}

void GattServerApplication::RequestReadCharacteristicCallback(int connId, int transId, const STACK::RawAddress &bda,
    int attrHandle, int offset, bool isLong)
{
    if (!IsValidConnId(connId)) {
        return;
    }
    HILOGI("connId: %{public}d, transId: %{public}d, address: %{public}s, attrHandle: %{public}#x, offset: %{public}d,"
        "isLong: %{public}d",
        connId, transId, bda.ToStringForLogging().c_str(), attrHandle, offset, isLong);

    RequestReadCallbackContext ctx = {connId, transId, ServiceUtil::AddrFromStack(bda), attrHandle, offset, isLong};
    RequestReadCallback(ctx, GattElement::CHARACTERISTIC);
}

void GattServerApplication::RespondCharacteristicRead(const RawAddress &addr, uint16_t handle,
    std::vector<uint8_t> value, int ret)
{
    auto iter = GetConnectionIter(addr);
    if (iter == connIdMap_.end()) {
        HILOGE("No connection to device: %{public}s", GET_ENCRYPT_ADDR(addr));
        return;
    }

    SendResponse(iter->first, iter->second.transId, ToValueHandle(handle), ret, value);

    // Cache value for ATT_READ_BLOB_REQ
    if (value.size() >= iter->second.mtu) {
        iter->second.longValue.value = std::move(value);
    }
}

void GattServerApplication::RequestReadDescriptorCallback(int connId, int transId, const STACK::RawAddress &bda,
    int attrHandle, int offset, bool isLong)
{
    if (!IsValidConnId(connId)) {
        return;
    }
    HILOGI("connId: %{public}d, transId: %{public}d, address: %{public}s, attrHandle: %{public}#x, offset: %{public}d,"
        "isLong: %{public}d",
        connId, transId, bda.ToStringForLogging().c_str(), attrHandle, offset, isLong);

    RequestReadCallbackContext ctx = {connId, transId, ServiceUtil::AddrFromStack(bda), attrHandle, offset, isLong};
    RequestReadCallback(ctx, GattElement::DESCRIPTOR);
}

void GattServerApplication::RespondDescriptorRead(const RawAddress &addr, uint16_t handle, std::vector<uint8_t> value,
    int ret)
{
    auto iter = GetConnectionIter(addr);
    if (iter == connIdMap_.end()) {
        HILOGE("No connection to device: %{public}s", GET_ENCRYPT_ADDR(addr));
        return;
    }

    SendResponse(iter->first, iter->second.transId, handle, ret, value);

    // Cache value for ATT_READ_BLOB_REQ
    if (value.size() >= iter->second.mtu) {
        iter->second.longValue.value = std::move(value);
    }
}

void GattServerApplication::ReportWriteRequest(const GattDevice &device, uint16_t handle,
    const std::vector<uint8_t> &value, bool needRsp, GattElement type)
{
    if (type == GattElement::CHARACTERISTIC) {
        Characteristic characteristic(FromValueHandle(handle), value.data(), value.size());
        WPTR_CBACK(callback_, OnCharacteristicWriteRequest, device, characteristic, needRsp);
    } else if (type == GattElement::DESCRIPTOR) {
        Descriptor descriptor(handle, value.data(), value.size());
        // Write descriptor always need response.
        WPTR_CBACK(callback_, OnDescriptorWriteRequest, device, descriptor);
    } else {
        HILOGE("Unknown type: %{public}d", type);
    }
}

void GattServerApplication::RequestWriteCallback(const RequestWriteCallbackContext &ctx, GattElement type)
{
    ControlInterceptMessage msg {
        .addr = ctx.addr.GetAddress(),
        .isSystemHap = isSystemHap_,
        .isNativeCaller = isNativeCaller_,
    };
    if (!ControlInterceptIsAllowedGattReceive(msg)) {
        return;
    }
    // ATT_PREPARE_WRITE_REQ
    if (ctx.isPrep) {
        ProcessLongValueWrite(ctx, type);
        return;
    }
    // ATT_WRITE_REQ or ATT_WRITE_CMD
    auto connIter = connIdMap_.find(ctx.connId);
    if (connIter == connIdMap_.end()) {
        HILOGE("Invalid connId: %{public}d", ctx.connId);
        return;
    }

    if (ctx.needRsp) {
        connIter->second.transId = ctx.transId;
    }
    ReportWriteRequest(connIter->second.device, ctx.handle, ctx.value, ctx.needRsp, type);
}

void GattServerApplication::RequestWriteCharacteristicCallback(int connId, int transId,
    const STACK::RawAddress &bda, int attrHandle, int offset, bool needRsp, bool isPrep, std::vector<uint8_t> value)
{
    if (!IsValidConnId(connId)) {
        return;
    }
    HILOGI("connId: %{public}d, transId: %{public}d, address: %{public}s, attrHandle: %{public}#x, offset: %{public}d,"
        "needRsp: %{public}d, isPrep: %{public}d",
        connId, transId, bda.ToStringForLogging().c_str(), attrHandle, offset, needRsp, isPrep);
    RequestWriteCallbackContext ctx = {connId, transId, ServiceUtil::AddrFromStack(bda), attrHandle, offset,
        needRsp, isPrep, std::move(value)};
    RequestWriteCallback(ctx, GattElement::CHARACTERISTIC);
}

void GattServerApplication::RespondCharacteristicWrite(const RawAddress &addr, uint16_t handle, int ret)
{
    auto iter = GetConnectionIter(addr);
    if (iter == connIdMap_.end()) {
        HILOGE("No connection to device: %{public}s", GET_ENCRYPT_ADDR(addr));
        return;
    }
    SendResponse(iter->first, iter->second.transId, ToValueHandle(handle), ret);
}

void GattServerApplication::RequestWriteDescriptorCallback(int connId, int transId, const STACK::RawAddress &bda,
    int attrHandle, int offset, bool needRsp, bool isPrep, std::vector<uint8_t> value)
{
    if (!IsValidConnId(connId)) {
        return;
    }
    HILOGI("connId: %{public}d, transId: %{public}d, address: %{public}s, attrHandle: %{public}#x, offset: %{public}d,"
        "needRsp: %{public}d, isPrep: %{public}d",
        connId, transId, bda.ToStringForLogging().c_str(), attrHandle, offset, needRsp, isPrep);

    RequestWriteCallbackContext ctx = {connId, transId, ServiceUtil::AddrFromStack(bda), attrHandle, offset,
        needRsp, isPrep, std::move(value)};
    RequestWriteCallback(ctx, GattElement::DESCRIPTOR);
}

void GattServerApplication::RespondDescriptorWrite(const RawAddress &addr, uint16_t handle, int ret)
{
    auto iter = GetConnectionIter(addr);
    if (iter == connIdMap_.end()) {
        HILOGE("No connection to device: %{public}s", GET_ENCRYPT_ADDR(addr));
        return;
    }

    SendResponse(iter->first, iter->second.transId, handle, ret);
}

void GattServerApplication::ProcessLongValueRead(const RequestReadCallbackContext &ctx)
{
    if (ctx.offset < 0) {
        HILOGE("Invalid offset: %{public}d", ctx.offset);
        return;
    }

    auto connIter = connIdMap_.find(ctx.connId);
    if (connIter == connIdMap_.end()) {
        HILOGE("Invalid connId: %{public}d", ctx.connId);
        return;
    }

    size_t offset = static_cast<size_t>(ctx.offset);
    size_t length = connIter->second.longValue.value.size();
    if (offset > length) {
        HILOGE("offset > length (offset: %{public}zu, length: %{public}zu)", offset, length);
        SendResponse(ctx.connId, ctx.transId, ctx.handle, GattStatus::INVALID_OFFSET);
        return;
    }

    auto &cacheValue = connIter->second.longValue.value;
    std::vector<uint8_t> value(cacheValue.begin() + offset, cacheValue.end());
    SendResponse(ctx.connId, ctx.transId, ctx.handle, GattStatus::GATT_SUCCESS, value);

    // Clear cache
    if (value.size() < connIter->second.mtu) {
        cacheValue.clear();
    }
}

void GattServerApplication::ProcessLongValueWrite(const RequestWriteCallbackContext &ctx, GattElement type)
{
    auto connIter = connIdMap_.find(ctx.connId);
    if (connIter == connIdMap_.end()) {
        HILOGE("Invalid connId: %{public}d", ctx.connId);
        return;
    }

    auto &prep = connIter->second.longValue.prep;
    prep.type = type;
    prep.handle = ctx.handle;

    if (ctx.offset < 0 || static_cast<size_t>(ctx.offset) != prep.value.size()) {
        HILOGE("Invalid offset for preparing write, offset: %{public}d", ctx.offset);
        prep.value.clear();
        SendResponse(ctx.connId, ctx.transId, ctx.handle, GattStatus::INVALID_OFFSET);
        return;
    }
    prep.value.insert(prep.value.end(), ctx.value.begin(), ctx.value.end());
    // ATT_PREPARE_WRITE_RSP include actual value and offset,
    // client need it to determine whether REQ is successful
    SendResponseContext rspContext = { ctx.connId, ctx.transId, ctx.handle, GattStatus::GATT_SUCCESS };
    SendResponse(rspContext, ctx.value, ctx.offset);
}

void GattServerApplication::RequestExecWriteCallback(int connId, int transId, const STACK::RawAddress &bda,
    int execWrite)
{
    if (!IsValidConnId(connId)) {
        return;
    }
    HILOGI("connId: %{public}d, transId: %{public}d, address: %{public}s, execWrite: %{public}d",
        connId, transId, bda.ToStringForLogging().c_str(), execWrite);
    auto connIter = connIdMap_.find(connId);
    if (connIter == connIdMap_.end()) {
        HILOGE("Invalid connId: %{public}d", connId);
        return;
    }

    auto &prep = connIter->second.longValue.prep;
    if (!execWrite) {
        HILOGI("Client cancel all prepared writes");
        prep.value.clear();
        // handle not used
        SendResponse(connId, transId, 0, GattStatus::GATT_SUCCESS);
        return;
    }
    if (prep.value.empty()) {
        HILOGE("No prepared writes");
        SendResponse(connId, transId, 0, GattStatus::REQUEST_NOT_SUPPORT);
        return;
    }
    // Expect the upper-layer application to respond to the request  ServiceUtil::AddrFromStack(bda)
    connIter->second.transId = transId;
    ReportWriteRequest(connIter->second.device, prep.handle, prep.value, true, prep.type);

    // prepare write procedure complete, clear cache
    prep.value.clear();
}

void GattServerApplication::SendResponse(int connId, int transId, uint16_t handle, int ret,
    const std::vector<uint8_t> &value)
{
    SendResponseContext ctx = { connId, transId, handle, ret };
    SendResponse(ctx, value, 0);
}

void GattServerApplication::SendResponse(const SendResponseContext &ctx, const std::vector<uint8_t> &value, int offset)
{
    // 600 bytes, could be too big for stacks
    btgatt_response_t rsp;
    (void)memset_s(&rsp, sizeof(rsp), 0x0, sizeof(rsp));
    rsp.handle = ctx.handle;
    rsp.attr_value.handle = ctx.handle;
    rsp.attr_value.offset = offset;

    int result;
    if (value.size() > 0) {
        result = memcpy_s(rsp.attr_value.value, BTGATT_MAX_ATTR_LEN, value.data(), value.size());
        if (result != EOK) {
            HILOGE("memcpy_s failed, result: %{public}d", result);
            return;
        }
        rsp.attr_value.len = value.size();
    }

    int status = GattServiceBase::GattStatusToStack(ctx.ret);
    if (btIfGattServer_) {
        result = btIfGattServer_->send_response(ctx.connId, ctx.transId, status, rsp);
        if (result != BT_STATUS_SUCCESS) {
            HILOGE("failed, result: %{public}d", result);
        }
    }
}

void GattServerApplication::IndicationSentCallback(int connId, int status)
{
    if (!IsValidConnId(connId)) {
        return;
    }
    HILOGD("connId: %{public}d, status: %{public}d", connId, status);

    auto connIter = connIdMap_.find(connId);
    if (connIter == connIdMap_.end()) {
        HILOGE("Invalid connId: %{public}d", connId);
        return;
    }
    // Notify server service if notify no confirm, don't callback to upper application.
    if (connIter->second.notifyNoCfmSem) {
        connIter->second.notifyNoCfmSem->Post();
        connIter->second.notifyNoCfmSem = nullptr;
    }

    int ret = GattServiceBase::GattStatusFromStack(status);
    uint16_t characteristicHandle = FromValueHandle(connIter->second.notifyHandle);
    WPTR_CBACK(callback_, OnNotifyConfirm, connIter->second.device, Characteristic(characteristicHandle), ret);
}

bool GattServerApplication::NotifyClientInner(const RawAddress &addr, uint16_t handle, std::vector<uint8_t> value,
    bool needConfirm)
{
    auto iter = GetConnectionIter(addr);
    if (iter == connIdMap_.end()) {
        HILOGE("No connection to device: %{public}s", GET_ENCRYPT_ADDR(addr));
        return false;
    }

    uint16_t valueHandle = ToValueHandle(handle);
    iter->second.notifyHandle = valueHandle;
    if (btIfGattServer_) {
        int ret = btIfGattServer_->send_indication(serverIf_, valueHandle, iter->first, needConfirm, std::move(value));
        if (ret != BT_STATUS_SUCCESS) {
            HILOGE("Send indication failed, ret: %{public}d", ret);
            return false;
        }
    }
    return true;
}

void GattServerApplication::NotifyClient(const RawAddress &addr, uint16_t handle, std::vector<uint8_t> value)
{
    NotifyClientInner(addr, handle, std::move(value), true);
}

void GattServerApplication::NotifyClientNoCfm(const RawAddress &addr, uint16_t handle, std::vector<uint8_t> value,
    const std::shared_ptr<utility::Semaphore> &semaphore)
{
    if (!semaphore) {
        HILOGE("semaphore is nullptr");
        return;
    }

    auto iter = GetConnectionIter(addr);
    if (iter == connIdMap_.end()) {
        HILOGE("No connection to device: %{public}s", GET_ENCRYPT_ADDR(addr));
        semaphore->Post();
        return;
    }

    iter->second.notifyNoCfmSem = semaphore;
    bool ok = NotifyClientInner(addr, handle, std::move(value), false);
    if (!ok) {
        semaphore->Post();
        iter->second.notifyNoCfmSem = nullptr;
        return;
    }
}

void GattServerApplication::MtuChangedCallback(int connId, int mtu)
{
    if (!IsValidConnId(connId)) {
        return;
    }
    HILOGI("connId: %{public}d, mtu: %{public}d", connId, mtu);

    auto connIter = connIdMap_.find(connId);
    if (connIter == connIdMap_.end()) {
        HILOGE("Invalid connId: %{public}d", connId);
        return;
    }
    // Update connection mtu
    connIter->second.mtu = static_cast<size_t>(mtu);
    WPTR_CBACK(callback_, OnMtuChanged, connIter->second.device, mtu);
    BtChrEventWriteInt(CHR_BLE_DISCONNECT, ServiceUtil::AddrToStack(connIter->second.device.addr_).ToString(),
        "BTCONMTUINITIATOR", 1);
    BtChrEventWriteInt(CHR_BLE_DISCONNECT, ServiceUtil::AddrToStack(connIter->second.device.addr_).ToString(),
        "BTCONMTURESULT", mtu);
}

void GattServerApplication::ConnUpdatedCallback(int connId, uint16_t interval, uint16_t latency, uint16_t timeout,
    uint8_t status)
{
    if (!IsValidConnId(connId)) {
        return;
    }
    HILOGI("connId: %{public}d, interval: %{public}u, latency: %{public}u, timeout: %{public}u, status: %{public}u",
        connId, interval, latency, timeout, status);

    auto connIter = connIdMap_.find(connId);
    if (connIter == connIdMap_.end()) {
        HILOGE("Invalid connId: %{public}d", connId);
        return;
    }
    WPTR_CBACK(callback_, OnConnectionParameterChanged, connIter->second.device, interval, latency, timeout, status);
    auto resourceMgr = BluetoothResourceManager::GetInstance();
    if (resourceMgr) {
        resourceMgr->SendSensingStateChanged(BLE_CONN_INTERVAL_UPDATE,
            SensingInfo(connIter->second.device.addr_.GetAddress(), static_cast<uint16_t>(interval)));
    }
}

void GattServerApplication::Connect(const RawAddress &addr, bool isDirect)
{
    HILOGI("serverIf: %{public}d, addr: %{public}s", serverIf_, GET_ENCRYPT_ADDR(addr));
    if (btIfGattServer_) {
        std::string callingName = PermissionManager::GetCallingName();
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_BLE_SERVER_CONN, addr.GetAddress(), serverIf_, callingName);
        int ret = btIfGattServer_->connect(serverIf_, ServiceUtil::AddrToStack(addr), isDirect, GATT_TRANSPORT_LE);
        CHECK_AND_RETURN_LOG(ret == BT_STATUS_SUCCESS, "failed, ret: %{public}d", ret);
    }
}

void GattServerApplication::CancelConnection(const RawAddress &addr)
{
    auto iter = GetConnectionIter(addr);
    if (iter == connIdMap_.end()) {
        HILOGE("No connection to device: %{public}s", GET_ENCRYPT_ADDR(addr));
        GattDevice device;
        device.addr_ = addr;
        WPTR_CBACK(callback_, OnConnectionStateChanged, device, GattStatus::GATT_FAILURE,
            static_cast<int>(BTConnectState::DISCONNECTED));
        return;
    }

    HILOGI("serverIf: %{public}d, addr: %{public}s, connId: %{public}d",
        serverIf_, GET_ENCRYPT_ADDR(addr), iter->first);
    if (btIfGattServer_) {
        std::string callingName = PermissionManager::GetCallingName();
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_BLE_SERVER_DISCONN, addr.GetAddress(),
            serverIf_, callingName);
        int ret = btIfGattServer_->disconnect(serverIf_, ServiceUtil::AddrToStack(addr), iter->first);
        if (ret != BT_STATUS_SUCCESS) {
            HILOGE("failed, ret: %{public}d", ret);
            WPTR_CBACK(callback_, OnConnectionStateChanged, iter->second.device, GattStatus::GATT_FAILURE,
                static_cast<int>(BTConnectState::CONNECTED));
            return;
        }
    }
}

void GattServerApplication::PhyUpdatedCallback(int connId, uint8_t txPhy, uint8_t rxPhy, uint8_t status)
{
    if (!IsValidConnId(connId)) {
        return;
    }
    HILOGI("connId: %{public}d, txPhy: %{public}u, rxPhy: %{public}u, status: %{public}u",
        connId, txPhy, rxPhy, status);
 
    auto connIter = connIdMap_.find(connId);
    if (connIter == connIdMap_.end()) {
        HILOGE("Invalid connId: %{public}d", connId);
        return;
    }
    int ret = GattServiceBase::GattStatusFromStack(status);
    WPTR_CBACK(callback_, OnBlePhyUpdate, connIter->second.device, txPhy, rxPhy, ret);
}
 
void GattServerApplication::SetPhy(const RawAddress &addr, int32_t txPhy, int32_t rxPhy, int32_t phyOptions)
{
    auto iter = GetConnectionIter(addr);
    if (iter == connIdMap_.end()) {
        HILOGE("No connection to device: %{public}s", GET_ENCRYPT_ADDR(addr));
        return;
    }
    int txPhyMask = ConvertToBlePhyMask(txPhy);
    int rxPhyMask = ConvertToBlePhyMask(rxPhy);

    if (btIfGattServer_) {
        int ret = btIfGattServer_->set_preferred_phy(
            ServiceUtil::AddrToStack(addr), txPhyMask, rxPhyMask, phyOptions);
        if (ret != BT_STATUS_SUCCESS) {
            HILOGE("failed, ret: %{public}d", ret);
            WPTR_CBACK(callback_, OnBlePhyUpdate, iter->second.device, 0, 0, GattStatus::GATT_FAILURE);
        }
    }
}
 
void GattServerApplication::ReadPhyCallback(uint8_t txPhy, uint8_t rxPhy, uint8_t status)
{
    HILOGI("txPhy: %{public}d, rxPhy: %{public}d, rxPhy: %{public}d", txPhy, rxPhy, status);
    int ret = GattServiceBase::GattStatusFromStack(status);
    WPTR_CBACK(callback_, OnBlePhyRead, txPhy, rxPhy, ret);
}
 
void GattServerApplication::ReadPhy(const RawAddress &addr)
{
    auto iter = GetConnectionIter(addr);
    if (iter == connIdMap_.end()) {
        HILOGE("No connection to device: %{public}s", GET_ENCRYPT_ADDR(addr));
        return;
    }
 
    if (btIfGattServer_) {
        int ret = btIfGattServer_->read_phy(ServiceUtil::AddrToStack(addr),
            [this](uint8_t txPhy, uint8_t rxPhy, uint8_t status) { ReadPhyCallback(txPhy, rxPhy, status); });
        if (ret != BT_STATUS_SUCCESS) {
            HILOGE("failed, ret: %{public}d", ret);
            WPTR_CBACK(callback_, OnBlePhyRead, 0, 0, GattStatus::GATT_FAILURE);
        }
    }
}
}  // namespace bluetooth
}  // namespace OHOS
