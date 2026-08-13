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

#ifndef OHOS_BLUETOOTH_STANDARD_BLE_CENTRAL_MANAGER_STUB_H
#define OHOS_BLUETOOTH_STANDARD_BLE_CENTRAL_MANAGER_STUB_H

#include <map>

#include "i_bluetooth_ble_central_manager.h"
#include "iremote_stub.h"
#include "permission_item.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothBleCentralManagerStub : public IRemoteStub<IBluetoothBleCentralManager> {
public:
    BluetoothBleCentralManagerStub();
    ~BluetoothBleCentralManagerStub() override;

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    using BleCentralManagerStubFunc =
        std::function<int32_t(BluetoothBleCentralManagerStub *, MessageParcel &, MessageParcel &)>;
    using BleCentralManagerStubFuncPerm = std::pair<BleCentralManagerStubFunc, std::shared_ptr<PermissionItem>>;

    using BleCentralManagerStubFuncMap = std::map<uint32_t, BleCentralManagerStubFuncPerm>;

private:
    int32_t RegisterBleCentralManagerCallbackInner(MessageParcel &data, MessageParcel &reply);
    int32_t DeregisterBleCentralManagerCallbackInner(MessageParcel &data, MessageParcel &reply);
    int32_t StartScanInner(MessageParcel &data, MessageParcel &reply);
    int32_t StopScanInner(MessageParcel &data, MessageParcel &reply);
    int32_t RemoveScanFilterInner(MessageParcel &data, MessageParcel &reply);
    int32_t FreezeByRssInner(MessageParcel &data, MessageParcel &reply);
    int32_t ResetAllProxyInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetLpDeviceAdvParamInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetScanReportChannelToLpDeviceInner(MessageParcel &data, MessageParcel &reply);
    int32_t EnableSyncDataToLpDeviceInner(MessageParcel &data, MessageParcel &reply);
    int32_t DisableSyncDataToLpDeviceInner(MessageParcel &data, MessageParcel &reply);
    int32_t SendParamsToLpDeviceInner(MessageParcel &data, MessageParcel &reply);
    int32_t IsLpDeviceAvailableInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetLpDeviceParamInner(MessageParcel &data, MessageParcel &reply);
    int32_t RemoveLpDeviceParamInner(MessageParcel &data, MessageParcel &reply);
    int32_t ChangeScanParamsInner(MessageParcel &data, MessageParcel &reply);
    int32_t IsValidScannerIdInner(MessageParcel &data, MessageParcel &reply);
    int32_t FlushBatchScanResultsInner(MessageParcel &data, MessageParcel &reply);

private:
    static const BleCentralManagerStubFuncMap memberFuncMap_;
    DISALLOW_COPY_AND_MOVE(BluetoothBleCentralManagerStub);
};
}  // namespace Bluetooth
}  // namespace OHOS

#endif