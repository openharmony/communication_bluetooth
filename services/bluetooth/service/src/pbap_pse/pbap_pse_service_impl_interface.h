/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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

#ifndef PBAP_PSE_SERVICE_IMPL_INTERFACE_H
#define PBAP_PSE_SERVICE_IMPL_INTERFACE_H

#include "raw_address.h"
#include "interface_profile_pbap_pse.h"
#include "bluetooth_errorcode.h"
#include "obex_socket_device.h"
#include "safe_map.h"

namespace OHOS {
namespace bluetooth {
class PbapPseServiceImplInterface {
public:
    PbapPseServiceImplInterface() = default;
    virtual ~PbapPseServiceImplInterface() = default;

    /**
     * @brief register observer
     *
     * @param observer the pointer that point to a PbapPseObserver
     * @return void
     */
    virtual void RegisterObserver(std::shared_ptr<IPbapPseObserver> &observer) = 0;

    /**
     * @brief deregister observer
     *
     * @param observer the pointer that point to a PbapPseObserver
     * @return void
     */
    virtual void DeregisterObserver(std::shared_ptr<IPbapPseObserver> &observer) = 0;

    /**
     * @brief disconnect remote device
     *
     * @return result
     */
    virtual int32_t Disconnect(const RawAddress &device) = 0;

    /**
     * @brief get the remote devices with the specified states
     *
     * @param states states
     * @return std::vector remote devices
     */
    virtual std::vector<RawAddress> GetDevicesByStates(const std::vector<int32_t> &states) = 0;

    /**
     * @brief get the state of remote device
     *
     * @param device  remote device
     * @return int state
     */
    virtual int32_t GetDeviceState(const RawAddress &device) = 0;

    /**
     * @brief Get Connect State for base service
     *
     */
    virtual int32_t GetConnectState() = 0;

    virtual void SetPhoneBookAccessAuthorization(const RawAddress& device, int32_t accessAuthorization) = 0;

    virtual void HandlerConnectEvt(std::shared_ptr<ObexSocketDevice> &socketDevice) = 0;

    virtual void DllRegisterFunc(const std::function<bool(const std::string&, std::shared_ptr<utility::Timer>,
        bool)> &dialogFunc, std::shared_ptr<SafeMap<const std::string, int64_t>> permissionRequestTimeMap) = 0;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // PBAP_PSE_SERVICE_IMPL_INTERFACE_H