/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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

#ifndef INTERFACE_PROFILE_RESOURCE_MANAGER_H
#define INTERFACE_PROFILE_RESOURCE_MANAGER_H

#include "bt_sensing_info.h"
#include "ble_service_data.h"

namespace OHOS {
namespace bluetooth {
/**
 * @brief obsever for resource manager
 * when bluetooth service occur resource manager event, call these
 */
class IResourceManagerObserver {
public:
    /**
     * @brief A destructor used to delete the <b>IResourceManagerObserver</b> instance.
     *
     * @since 12
     */
    virtual ~IResourceManagerObserver() = default;

    /**
     * @brief sensing state changed observer.
     *
     * @param eventId bluetooth resource manager event id.
     * @param info bluetooth sensing information.
     * @since 12
     */
    virtual void OnSensingStateChanged(uint8_t eventId, const SensingInfo &info) = 0;

    /**
     * @brief bluetooth resource decision observer.
     *
     * @param eventId bluetooth resource manager event id.
     * @param info bluetooth sensing information.
     * @param result bluetooth resource decision result.
     * @since 12
     */
    virtual void OnBluetoothResourceDecision(uint8_t eventId, const SensingInfo &info, uint32_t &result) = 0;
};

class IBluetoothResourceManager {
public:
    /**
     * @brief A destructor used to delete the <b>IBluetoothResourceManager</b> instance.
     *
     * @since 12
     */
    virtual ~IBluetoothResourceManager() = default;

    /**
     * @brief Get bluetooth resource manager service instance.
     *
     * @return Returns an instance of bluetooth resource manager service.
     */
    static IBluetoothResourceManager *GetInstance();

    /**
     * @brief register observer
     *
     * @param observer sub class observer of IResourceManagerObserver
     * @return void
     */
    virtual void RegisterObserver(std::shared_ptr<IResourceManagerObserver> observer) = 0;

    /**
     * @brief deregister observer
     *
     * @param observer sub class observer of IResourceManagerObserver
     * @return void
     */
    virtual void DeregisterObserver(std::shared_ptr<IResourceManagerObserver> observer) = 0;

    /**
     * @brief set ble scan enhance mode
     *
     * @param enhanceMode ble scan enhance mode param
     * @return result of ble scan enhance mode.
     */
    virtual int SetBleScanEnhanceMode(const BleScanEnhanceModeImpl &enhanceMode) = 0;
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // INTERFACE_PROFILE_RESOURCE_MANAGER_H