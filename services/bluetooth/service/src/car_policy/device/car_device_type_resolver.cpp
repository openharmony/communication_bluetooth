/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "car_device_type_resolver.h"

#ifndef LOG_TAG
#define LOG_TAG "bt_service_car_policy_device"
#endif
#include "log.h"

#include "adapter_manager.h"
#include "bluetooth_def.h"
#include "bluetooth_hw_interface.h"
#include "classic_adapter.h"
#include "raw_address.h"

namespace OHOS {
namespace bluetooth {

// 通过 GetRemoteDeviceInfo 获取协议栈上报的厂商 ID 与设备类型，
//   vendorId_   == VENDOR_ID_OF_HW_DEVICE(0x12D1)  → 华为品牌
//   deviceOfType_ == HW_ECHO_PRODUCT::PHONE(0x01)  → 手机类型
// 这两个字段由 echo 9,1 DEVICE_DESCRIP 上报后存入 BluetoothDevice，
// 配对完成时即写入 config 持久化，回连谓词调用时已就绪。
bool CarDeviceTypeResolver::IsHwPhone(const std::string &addr) const
{
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter == nullptr) {
        HILOGW("IsHwPhone: ClassicAdapter null, addr=%{public}s -> false", addr.c_str());
        return false;
    }

    Bluetooth::BluetoothRemoteDeviceInfo info;
    int32_t ret = classicAdapter->GetRemoteDeviceInfo(RawAddress(addr), info);
    if (ret != Bluetooth::BT_NO_ERROR) {
        HILOGI("IsHwPhone: GetRemoteDeviceInfo fail, addr=%{public}s, ret=%{public}d -> false",
               addr.c_str(), ret);
        return false;
    }

    bool isHwVendor = (info.vendorId_ == VENDOR_ID_OF_HW_DEVICE);
    bool isPhone = (info.deviceOfType_ == static_cast<uint8_t>(HW_ECHO_PRODUCT::PHONE));
    HILOGI("IsHwPhone: addr=%{public}s, vendorId=0x%{public}04x, deviceType=0x%{public}02x, "
           "isHwVendor=%{public}d, isPhone=%{public}d",
           addr.c_str(), info.vendorId_, info.deviceOfType_, isHwVendor, isPhone);
    return isHwVendor && isPhone;
}

bool CarDeviceTypeResolver::IsHwCarDevice(const std::string &addr) const
{
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter == nullptr) {
        HILOGW("IsHwCarDevice: ClassicAdapter null, addr=%{public}s -> false", addr.c_str());
        return false;
    }

    // 与 IsHwPhone 同样的判定方式：
    //   vendorId_     == VENDOR_ID_OF_HW_DEVICE(0x12D1) → 华为品牌
    //   deviceOfType_ == HW_ECHO_PRODUCT::CAR(0x69)     → 车机类型
    // HW_CAR_DEVICE_TYPE 宏(remote_device_properties.h:45)定义为 105 即 0x69，与本判定等价。
    Bluetooth::BluetoothRemoteDeviceInfo info;
    int32_t ret = classicAdapter->GetRemoteDeviceInfo(RawAddress(addr), info);
    if (ret != Bluetooth::BT_NO_ERROR) {
        HILOGI("IsHwCarDevice: GetRemoteDeviceInfo fail, addr=%{public}s, ret=%{public}d -> false",
               addr.c_str(), ret);
        return false;
    }

    bool isHwVendor = (info.vendorId_ == VENDOR_ID_OF_HW_DEVICE);
    bool isCar = (info.deviceOfType_ == static_cast<uint8_t>(HW_ECHO_PRODUCT::CAR));
    HILOGI("IsHwCarDevice: addr=%{public}s, vendorId=0x%{public}04x, deviceType=0x%{public}02x, "
           "isHwVendor=%{public}d, isCar=%{public}d",
           addr.c_str(), info.vendorId_, info.deviceOfType_, isHwVendor, isCar);
    return isHwVendor && isCar;
}

}  // namespace bluetooth
}  // namespace OHOS
