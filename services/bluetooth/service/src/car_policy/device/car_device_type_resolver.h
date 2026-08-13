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

#ifndef CAR_DEVICE_TYPE_RESOLVER_H
#define CAR_DEVICE_TYPE_RESOLVER_H

#include <string>

namespace OHOS {
namespace bluetooth {

class CarDeviceTypeResolver {
public:
    CarDeviceTypeResolver() = default;
    ~CarDeviceTypeResolver() = default;

    /// 判断是否华为手机。
    bool IsHwPhone(const std::string &addr) const;

    /// 判断是否华为车机设备。
    bool IsHwCarDevice(const std::string &addr) const;
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // CAR_DEVICE_TYPE_RESOLVER_H
