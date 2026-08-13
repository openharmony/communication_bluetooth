/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

/**
 * @file power_on_reconnect.h
 *
 * @brief Service module of the tv product
 *
 * @since 24
 */

#ifndef POWER_ON_RECONNECT_H
#define POWER_ON_RECONNECT_H

#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "raw_address.h"
#include "interface_profile_hid_host.h"

namespace OHOS {
namespace bluetooth {
class PowerOnReconnectSubscriber : public OHOS::EventFwk::CommonEventSubscriber {
public:
    explicit PowerOnReconnectSubscriber(const OHOS::EventFwk::CommonEventSubscribeInfo &subscriberInfo)
        : CommonEventSubscriber(subscriberInfo) {}
    virtual ~PowerOnReconnectSubscriber() = default;

    void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data) override;
};

class PowerOnReconnect {
public:
    /**
     * @brief Get power on reconnect singleton instance pointer.
     *
     * @return Returns the singleton instance pointer.
     * @since 24
     */
    static PowerOnReconnect *GetInstance();

    /**
     * @brief Initialize
     *
     * @since 24
     */
    void Initialize();
     /**
     * @brief Uninitialize tv service.
     *
     * @since 24
     */
    void Uninitialize();
    /**
     * @brief Disconnect peripherals when in str mode.
     *
     * @since 24
     */
    void DisconnectPeripheralsInStr();
    /**
     * @brief Auto connect device when awake.
     *
     * @since 24
     */
    void StartAutoConnect();

    /**
     * @brief process hid host connetion state change.
     *
     * @param device device address.
     * @param status connection state.
     * @param cause reason.
     * @since 24
     */
    void ProcessHidConnectionStateChanged(const RawAddress &device, int state, int cause);

    class HidHostCallback : public bluetooth::IHidHostObserver {
    public:
        explicit HidHostCallback(PowerOnReconnect *powerOnReconnect)
            : powerOnReconnect_(powerOnReconnect){};
        ~HidHostCallback() = default;
        void OnConnectionStateChanged(const RawAddress &device, int state, int cause) override
        {
            powerOnReconnect_->ProcessHidConnectionStateChanged(device, state, cause);
        }
    private:
        PowerOnReconnect *powerOnReconnect_{nullptr};
    };

private:
    void SubscribeTvCommonEvent();

    std::shared_ptr<PowerOnReconnectSubscriber> subscriber_ {nullptr};
    std::unique_ptr<HidHostCallback> hidHostCallback_ {nullptr};
    std::atomic_bool isForceSleep_{false};
};
}
}
#endif