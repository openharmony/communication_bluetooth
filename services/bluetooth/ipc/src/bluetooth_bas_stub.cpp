/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_ipc_bas_stub"
#endif

#include "bluetooth_bas_stub.h"

#include <memory>
#include <regex>
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "permission_manager.h"
#include "bt_hicollie_adapter.h"

#ifdef BLUETOOTH_BAS_FEATURE_ENABLE
#include "bluetooth_device_battery_observer_proxy.h"
#include "i_bluetooth_device_battery_observer.h"
#endif

#ifdef STUB_FUNC
#undef STUB_FUNC
#endif
#define STUB_FUNC(code, func, perm) \
    static_cast<uint32_t>(BluetoothBasInterfaceCode::code), {&BluetoothBasStub::func, perm}

namespace OHOS {
namespace Bluetooth {

const std::map<uint32_t, BluetoothBasStub::BasStubFuncPerm> BluetoothBasStub::memberFuncMap_ = {
    {STUB_FUNC(BT_REGISTER_DEVICE_BATTERY_OBSERVER, RegisterDeviceBatteryObserverInner,
        CHECK_PERM(true, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_DEREGISTER_DEVICE_BATTERY_OBSERVER, DeregisterDeviceBatteryObserverInner,
        CHECK_PERM(true, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_GET_BATTERY_LEVEL, GetBatteryLevelInner,
        CHECK_PERM(true, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(GET_CONNECTED_DEVICE_BATTERY_INFOS, GetConnectedDeviceBatteryInfosInner,
        CHECK_PERM(true, {}, {ACCESS_BLUETOOTH}))},
};

BluetoothBasStub::BluetoothBasStub()
{}

BluetoothBasStub::~BluetoothBasStub()
{}

int BluetoothBasStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::string name = "BluetoothBas ipcCode: " + std::to_string(code);
    BtHicollieAdapter hicollie(name);
    CHECK_PERMISSION_AND_EXECUTE_FUNC_RETURN(BluetoothBasStub);
}

int32_t BluetoothBasStub::RegisterDeviceBatteryObserverInner(MessageParcel &data, MessageParcel &reply)
{
#ifdef BLUETOOTH_BAS_FEATURE_ENABLE
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        HILOGE("RegisterDeviceBatteryObserverInner remote is nullptr");
        return BT_ERR_INVALID_PARAM;
    }
    sptr<IBluetoothDeviceBatteryObserver> observer = OHOS::iface_cast<IBluetoothDeviceBatteryObserver>(remote);
    if (observer == nullptr) {
        HILOGE("RegisterDeviceBatteryObserverInner observer is nullptr");
        return BT_ERR_INVALID_PARAM;
    }
    int32_t result = RegisterDeviceBatteryObserver(observer);
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(result), BT_ERR_IPC_TRANS_FAILED, "reply writing failed.");
#else
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(BT_ERR_API_NOT_SUPPORT),
        BT_ERR_IPC_TRANS_FAILED, "reply writing failed.");
#endif
    return NO_ERROR;
}

int32_t BluetoothBasStub::DeregisterDeviceBatteryObserverInner(MessageParcel &data, MessageParcel &reply)
{
#ifdef BLUETOOTH_BAS_FEATURE_ENABLE
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        HILOGE("DeregisterDeviceBatteryObserverInner remote is nullptr");
        return BT_ERR_INVALID_PARAM;
    }
    sptr<IBluetoothDeviceBatteryObserver> observer = OHOS::iface_cast<IBluetoothDeviceBatteryObserver>(remote);
    if (observer == nullptr) {
        HILOGE("DeregisterDeviceBatteryObserverInner observer is nullptr");
        return BT_ERR_INVALID_PARAM;
    }
    int32_t result = DeregisterDeviceBatteryObserver(observer);
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(result), BT_ERR_IPC_TRANS_FAILED, "reply writing failed.");
#else
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(BT_ERR_API_NOT_SUPPORT),
        BT_ERR_IPC_TRANS_FAILED, "reply writing failed.");
#endif
    return NO_ERROR;
}

int32_t BluetoothBasStub::GetBatteryLevelInner(MessageParcel &data, MessageParcel &reply)
{
#ifdef BLUETOOTH_BAS_FEATURE_ENABLE
    std::string address;
    CHECK_AND_RETURN_LOG_RET(data.ReadString(address), BT_ERR_IPC_TRANS_FAILED, "Read address failed");
    CHECK_AND_RETURN_LOG_RET(IsValidAddress(address), BT_ERR_INVALID_PARAM, "Invalid address");
    int32_t result = GetBatteryLevel(address);
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(result), BT_ERR_IPC_TRANS_FAILED, "reply writing failed.");
#else
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(BT_ERR_API_NOT_SUPPORT),
        BT_ERR_IPC_TRANS_FAILED, "reply writing failed.");
#endif
    return NO_ERROR;
}

int32_t BluetoothBasStub::GetConnectedDeviceBatteryInfosInner(MessageParcel &data, MessageParcel &reply)
{
#ifdef BLUETOOTH_BAS_FEATURE_ENABLE
    std::map<std::string, int32_t> batteryInfos;
    int32_t ret = GetConnectedDeviceBatteryInfos(batteryInfos);
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(ret), BT_ERR_IPC_TRANS_FAILED, "reply writing failed.");
    CHECK_AND_RETURN_LOG_RET(ret == BT_NO_ERROR, ret, "get ret failed.");
    int32_t size = static_cast<int32_t>(batteryInfos.size());
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(size), BT_ERR_IPC_TRANS_FAILED, "write size failed.");
    for (const auto &info : batteryInfos) {
        CHECK_AND_RETURN_LOG_RET(reply.WriteString(info.first), BT_ERR_IPC_TRANS_FAILED, "write address failed.");
        CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(info.second), BT_ERR_IPC_TRANS_FAILED, "write batteryLevel failed.");
    }
#else
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(BT_ERR_API_NOT_SUPPORT),
        BT_ERR_IPC_TRANS_FAILED, "reply writing failed.");
#endif
    return NO_ERROR;
}

bool BluetoothBasStub::IsValidAddress(const std::string &addr)
{
    if (addr.empty()) {
        HILOGE("addr is empty.");
        return false;
    }
    const std::regex deviceIdRegex("^[0-9a-fA-F]{2}(:[0-9a-fA-F]{2}){5}$");
    return std::regex_match(addr, deviceIdRegex);
}
}  // namespace Bluetooth
}  // namespace OHOS