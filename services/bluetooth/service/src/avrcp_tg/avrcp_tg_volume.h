/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef AVRCP_TG_VOLUME_H
#define AVRCP_TG_VOLUME_H

#ifndef LOG_TAG
#define LOG_TAG "bt_service_avrcp_tg_volume"
#endif

#include <unordered_map>
#include "bluetooth.h"
#include "service_util.h"
#include "log.h"
#include "interface_adapter_classic.h"

namespace std {
template <>
struct hash<OHOS::bluetooth::RawAddress> {
    size_t operator()(const OHOS::bluetooth::RawAddress &address) const noexcept
    {
        return std::hash<std::string>()(address.GetAddress());
    }
};
}  // namespace std

namespace OHOS {
namespace bluetooth {
constexpr int32_t AVRCP_MAX_VOL = 127;
constexpr uint8_t AVRCP_SUCCESS = 0;
constexpr uint8_t AVRCP_ERROR = -1;
constexpr int32_t AVRCP_DEFAULT_VOLUME_LEVEL = 7;
constexpr int32_t AVRCP_DEFAULT_VOLUME_DIVISOR = 2;
using namespace ::bluetooth::avrcp;
/**
 * @brief This class implements the volume interface. When the state of the remote device changes, these callbacks
 * method will be called.
 */
class AvrcpVolumeInterfaceImpl : public VolumeInterface {
public:
    AvrcpVolumeInterfaceImpl() = default;
    ~AvrcpVolumeInterfaceImpl() = default;

    void Init();
    void Cleanup();

    /**
     * @brief callback method.
     * Indicate that a device has been connected that does not support absolute volume.
     */
    void DeviceConnected(const OHOS::bluetooth::RawAddress &bdaddr) override;

    /**
     * @brief callback method.
     * Indicate that a device has been connected that does support absolute volume.
     * the callback called will send current valume to the remote devices
     */
    void DeviceConnected(const OHOS::bluetooth::RawAddress &bdaddr, VolumeChangedCb cb) override;

    /**
     * @brief callback method.
     * Register the SetVolume method to jni.
     */
    void DeviceEmplaceSetVolume(const OHOS::bluetooth::RawAddress &bdaddr, VolumeChangedCb cb) override;

    /**
     * @brief callback method.
     * Indicate that a device has been disconnected from AVRCP. Will unregister any callbacks
     * if absolute volume is supported.
     */
    void DeviceDisconnected(const OHOS::bluetooth::RawAddress &bdaddr) override;

    /**
     * @brief callback method.
     * Remote device change the volume
     */
    void SetVolume(int8_t volume) override;

    /**
     * @brief callback method.
     * Remote device change the volume
     */
    void setVolumeOfDevice(int8_t volume, const OHOS::bluetooth::RawAddress &bdaddr) override;

    /**
     * @brief Current device changed volume will send to remote by this function.
     * @param[in] addr The current device will changed.
     * @param[in] volume The current device volume to be changed.
     */
    void SendVolumeChanged(const OHOS::bluetooth::RawAddress &addr, int32_t volume);

    /**
     * @brief a2dp device actived, restore abs volume if support.
     * @param[in] rawAddr The device.
     */
    void SwitchAbsVolumeDevice(const OHOS::bluetooth::RawAddress &rawAddr, bool isNeedSetVolume = true);

    /**
     * @brief Set device abs volume ability.
     * @param[in] rawAddr The device.
     * @param[in] ability The device abs volume ability.
     */
    void SetDeviceAbsVolumeAbility(const OHOS::bluetooth::RawAddress &rawAddr, int32_t ability);

    /**
     * @brief Get device abs volume ability.
     * @param[in] addr The device.
     * @return Device abs volume ability.
     */
    int32_t GetDeviceAbsVolumeAbility(const OHOS::bluetooth::RawAddress &rawAddr);

    /**
     * @brief system volume changed, audio notify volume changed.
     * @param[in] addr The device.
     * @param[in] volumeLevel The device volume.
     */
    void SetDeviceAbsoluteVolume(const OHOS::bluetooth::RawAddress &rawAddr, int32_t volumeLevel);

    void NotifyAudioVolumeEvent(int32_t streamType, int32_t volume);

    void SetDeviceAbsVolumeProperty(const OHOS::bluetooth::RawAddress &rawAddr, int32_t ability);

private:
    inline int32_t SystemToAvrcpVolume(int32_t volume)
    {
        if (musicMaxVolumeLevel_ == 0) {
            return AVRCP_MAX_VOL;
        }
        return std::floor(static_cast<double>(volume) * AVRCP_MAX_VOL / musicMaxVolumeLevel_);
    }
    inline int32_t AvrcpToSystemVolume(int8_t volume)
    {
        return std::round(static_cast<double>(volume) * musicMaxVolumeLevel_ / AVRCP_MAX_VOL);
    }

    inline int32_t GetDefaultVolume()
    {
        if (musicMaxVolumeLevel_ < AVRCP_DEFAULT_VOLUME_LEVEL) {
            return AVRCP_DEFAULT_VOLUME_LEVEL;
        }
        return musicMaxVolumeLevel_ / AVRCP_DEFAULT_VOLUME_DIVISOR;
    }

private:
    std::mutex volumeCallbackMapLock_;
    std::unordered_map<OHOS::bluetooth::RawAddress, VolumeChangedCb> volumeCallbackMap_;
    int32_t musicMaxVolumeLevel_;
};
}  // namespace bluetooth
}  // namespace OHOS

#endif
