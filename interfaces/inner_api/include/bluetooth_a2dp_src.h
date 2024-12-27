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

/**
 * @addtogroup Bluetooth
 * @{
 *
 * @brief The framework interface and callback function of a2dp source are defined.
 *
 * @since 6
 */

#ifndef BLUETOOTH_A2DP_SRC_H
#define BLUETOOTH_A2DP_SRC_H

#include <vector>

#include "bluetooth_def.h"
#include "bluetooth_types.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_a2dp_codec.h"
#include "bluetooth_no_destructor.h"

namespace OHOS {
namespace Bluetooth {
/**
 * @brief A2dp source API callback function.
 *
 * @since 6.0
 */
class A2dpSourceObserver {
public:
    /**
     * @brief A destructor used to delete the a2dp source Observer instance.
     *
     * @since 6.0
     */
    virtual ~A2dpSourceObserver() = default;

    /**
     * @brief The callback function after device's playing state changed.
     *
     * @param device  the remote bluetooth device.
     * @param playingState  the playing state after changing.
     * @param error  the error information.
     * @since 6.0
     */
    virtual void OnPlayingStatusChanged(const BluetoothRemoteDevice &device, int playingState, int error)
    {}

    /**
     * @brief The callback function after device's codec information changed.
     *
     * @param device  the remote bluetooth device.
     * @param info  the device's codec information.
     * @param error  the error information.
     * @since 6.0
     */
    virtual void OnConfigurationChanged(const BluetoothRemoteDevice &device, const A2dpCodecInfo &info, int error)
    {}

    /**
     * @brief ConnectionState Changed observer.
     * @param device bluetooth device address.
     * @param state Connection state.
     * @param cause Connecton cause.
     * @since 12
     */
    virtual void OnConnectionStateChanged(const BluetoothRemoteDevice &device, int state, int cause)
    {}

    /**
     * @brief Media Stack Changed observer.
     * @param device bluetooth device address.
     * @param Action on the device.
     * @since 11.0
     */
    virtual void OnMediaStackChanged(const BluetoothRemoteDevice &device, int action)
    {}

    /**
     * @brief The observer function to notify virtual device changed.
     *
     * @param Action on the device.
     * @param device bluetooth device address.
     * @since 12
     */
    virtual void OnVirtualDeviceChanged(int32_t action, std::string address)
    {}
    
    /**
     * @brief Hdap ConnectionState Changed observer.
     * @param device bluetooth device address.
     * @param state Connection state.
     * @param info the device's codec information.
     * @since 12
     */
    virtual void OnCaptureConnectionStateChanged(const BluetoothRemoteDevice &device, int state,
        const A2dpCodecInfo &info)
    {}
};

/**
 * @brief audio stream details.
 *
 * @since 11.0
 */
struct A2dpStreamInfo {
    int32_t sessionId;
    int32_t streamType;
    int32_t sampleRate;
    bool isSpatialAudio;
};

/**
 * @brief a2dp audio stream encode type.
 *
 * @since 11.0
 */
enum A2dpStreamEncodeType : uint8_t {
    A2DP_STREAM_ENCODE_UNKNOWN = 0,
    A2DP_STREAM_ENCODE_SOFTWARE,
    A2DP_STREAM_ENCODE_HARDWARE,
};

/**
 * @brief A2dp source API.
 *
 * @since 6.0
 */
class BLUETOOTH_API A2dpSource {
public:
    /**
     * @brief Get a2dp source instance.
     *
     * @return Returns an instance of a2dp source.
     * @since 6.0
     */
    static A2dpSource *GetProfile();

    /**
     * @brief Get devices by connection states.
     *
     * @param states The connection states of the bluetooth device.
     * @return Returns devices that match the connection states.
     * @since 6.0
     */
    int GetDevicesByStates(const std::vector<int> &states, std::vector<BluetoothRemoteDevice> &devices) const;

    /**
     * @brief Get device connection state by address.
     *
     * @param device The address of the peer bluetooth device.
     * @return Returns <b>A2DP_DISCONNECTED</b> if device connect state is disconnected;
     *         Returns <b>A2DP_DISCONNECTING</b> if device connect state is disconnecting;
     *         Returns <b>A2DP_CONNECTED</b> if device connect state is connected;
     *         Returns <b>A2DP_CONNECTING</b> if device connect state is connecting;
     *         Returns <b>A2DP_INVALID_STATUS</b> if can not find peer device.
     * @since 6.0
     */
    int GetDeviceState(const BluetoothRemoteDevice &device, int &state) const;

    /**
     * @brief Get device playing state by address when peer device is on connected.
     *
     * @param device The address of the peer bluetooth device.
     * @return Returns <b>1</b> if device is on playing;
     *         Returns <b>0</b> if device is not on playing.
     * @since 6.0
     */
    int GetPlayingState(const BluetoothRemoteDevice &device) const;

    /**
     * @brief Get device playing state by address when peer device is on connected.
     *
     * @param device The address of the peer bluetooth device.
     * @param state The playing state of the peer bluetooth device.
     * @return Returns operation result;
     * @since 6.0
     */
    int GetPlayingState(const BluetoothRemoteDevice &device, int &state) const;

    /**
     * @brief Connect to the peer bluetooth device.
     *
     * @param device The address of the peer bluetooth device.
     * @return Returns <b>true</b> Perform normal connection processing.
     *         Returns <b>false</b> Target device is on connected,or connecting,
                                    or device is not allowed to connect,or the connection fails.
     * @since 6.0
     */
    int32_t Connect(const BluetoothRemoteDevice &device);

    /**
     * @brief Disconnect with the peer bluetooth service.
     *
     * @param device The address of the peer bluetooth device.
     * @return Returns <b>true</b> if perform normal disconnection processing.
     *         Returns <b>false</b> if target device is on disconnected,or disconnecting,or disconnection fails.
     * @since 6.0
     */
    int32_t Disconnect(const BluetoothRemoteDevice &device);

    /**
     * @brief Set target device as active device.
     *
     * @param device The address of the peer bluetooth device.
     * @return Returns <b>RET_NO_ERROR</b> Target device has already been active, or perform normal setting processing.
     *         Returns <b>RET_BAD_PARAM</b> Input error.
     *         Returns <b>RET_BAD_STATUS</b> Target device is not on connected, or set fails.
     * @since 6.0
     */
    int SetActiveSinkDevice(const BluetoothRemoteDevice &device);

    /**
     * @brief Get active device.
     * @return Returns active device.
     * @since 6.0
     */
    const BluetoothRemoteDevice &GetActiveSinkDevice() const;

    /**
     * @brief Set connection strategy for peer bluetooth device.
     *        If peer device is connected and the policy is set not allowed,then perform disconnect operation.
     *        If peer device is disconnected and the policy is set allowed,then perform connect operation.
     *
     * @param device The address of the peer bluetooth device.
     * @param strategy The device connect strategy.
     * @return Returns <b>RET_NO_ERROR</b> if the operation is successful.
     *         Returns <b>BT_ERR_PERMISSION_FAILED</b> Permission denied.
     *         Returns <b>BT_ERR_INVALID_PARAM</b> Input error.
     *         Returns <b>BT_ERR_INVALID_STATE</b> BT_ERR_INVALID_STATE.
     *         Returns <b>BT_ERR_INTERNAL_ERROR</b> Operation failed.
     * @since 6.0
     */
    int SetConnectStrategy(const BluetoothRemoteDevice &device, int strategy);

    /**
     * @brief Get connection strategy of peer bluetooth device.
     *
     * @param device The address of the peer bluetooth device.
     * @param strategy The device connect strategy.
     * @return Returns <b>RET_NO_ERROR</b> if the operation is successful.
     *         Returns <b>BT_ERR_PERMISSION_FAILED</b> Permission denied.
     *         Returns <b>BT_ERR_INVALID_PARAM</b> Input error.
     *         Returns <b>BT_ERR_INVALID_STATE</b> BT_ERR_INVALID_STATE.
     *         Returns <b>BT_ERR_INTERNAL_ERROR</b> Operation failed.
     * @since 6.0
     */
    int GetConnectStrategy(const BluetoothRemoteDevice &device, int &strategy) const;

    /**
     * @brief Get codec status information of connected device.
     *
     * @param device The address of the bluetooth device.
     * @return Returns codec status information of connected device.
     * @since 6.0
     */
    A2dpCodecStatus GetCodecStatus(const BluetoothRemoteDevice &device) const;

    /**
     * @brief Get the codec encoding preferences of the specified device.
     *
     * @param device The address of the bluetooth device.
     * @param info The codec encoding information.
     * @return Return the result getted.
     * @since 6.0
     */
    int GetCodecPreference(const BluetoothRemoteDevice &device, A2dpCodecInfo &info);

    /**
     * @brief Set the codec encoding preferences of the specified device.
     *
     * @param device The address of the bluetooth device.
     * @param info The codec encoding information.
     * @return Return the result setted.
     * @since 6.0
     */
    int SetCodecPreference(const BluetoothRemoteDevice &device, const A2dpCodecInfo &info);

    /**
     * @brief Set whether enables the optional codec.
     *
     * @param device The address of the bluetooth device.
     * @param isEnable Set true if enables the optional codec and set optional codec's priority high.
     *                 Set false if disables the optional codec and set optional codec's priority low.
     * @since 6.0
     */
    void SwitchOptionalCodecs(const BluetoothRemoteDevice &device, bool isEnable);

    /**
     * @brief Get whether the peer bluetooth device supports optional codec.
     *
     * @param device The address of the bluetooth device.
     * @return Returns <b>A2DP_OPTIONAL_SUPPORT</b> The device supports optional codec.
     *         Returns <b>A2DP_OPTIONAL_NOT_SUPPORT</b> The device dosn't support optional codec.
     *         Returns <b>A2DP_OPTIONAL_SUPPORT_UNKNOWN</b> Don't know if the device support optional codec.
     * @since 6.0
     */
    int GetOptionalCodecsSupportState(const BluetoothRemoteDevice &device) const;

    /**
     * @brief Audio start streaming.
     *
     * @param device The address of the bluetooth device.
     * @return Returns <b>RET_NO_ERROR</b> if the operation is successful.
     *         Returns <b>RET_BAD_PARAM</b> Input error.
     *         Returns <b>RET_BAD_STATUS</b> if the operation fails.
     * @since 6.0
     */
    int StartPlaying(const BluetoothRemoteDevice &device);

    /**
     * @brief Audio suspend streaming.
     *
     * @param device The address of the bluetooth device.
     * @return Returns <b>RET_NO_ERROR</b> if the operation is successful.
     *         Returns <b>RET_BAD_PARAM</b> Input error.
     *         Returns <b>RET_BAD_STATUS</b> if the operation fails.
     * @since 6.0
     */
    int SuspendPlaying(const BluetoothRemoteDevice &device);

    /**
     * @brief Audio stop streaming.
     *
     * @param device The address of the bluetooth device.
     * @return Returns <b>RET_NO_ERROR</b> if the operation is successful.
     *         Returns <b>RET_BAD_PARAM</b> Input error.
     *         Returns <b>RET_BAD_STATUS</b> if the operation fails.
     * @since 6.0
     */
    int StopPlaying(const BluetoothRemoteDevice &device);

    /**
     * @brief Register callback function of framework.
     *
     * @param observer Reference to the a2dp source observer.
     * @since 6.0
     */
    void RegisterObserver(std::shared_ptr<A2dpSourceObserver> observer);

    /**
     * @brief Deregister callback function of framework.
     *
     * @param observer Reference to the a2dp source observer.
     * @since 6.0
     */
    void DeregisterObserver(std::shared_ptr<A2dpSourceObserver> observer);

    /**
     * @brief Write the pcm data to a2dp source profile.
     *
     * @param data Pointer of the data.
     * @param size Size of the data
     * @return Returns <b>RET_NO_ERROR</b> if the operation is successful.
     *         Returns <b>RET_BAD_PARAM</b> Input error.
     *         Returns <b>RET_BAD_STATUS</b> if the operation fails.
     *         Returns <b>RET_NO_SPACE</b> if the buffer of a2dp source profile is full.
     * @since 6.0
     */
    int WriteFrame(const uint8_t *data, uint32_t size);

    /**
     * @brief Get the information of the current rendered position.
     * @param device The address of the peer bluetooth device.
     * @param[out] dalayValue is the delayed time
     * @param[out] sendDataSize is the data size that has been sent
     * @param[out] timeStamp is the current time stamp
     * @since 6.0
     */
    int GetRenderPosition(const BluetoothRemoteDevice &device, uint32_t &delayValue, uint64_t &sendDataSize,
                          uint32_t &timeStamp);

    /**
     * @brief Audio start offload streaming for hardware encoding datapath.
     *
     * @param device remote bluetooth sink device.
     * @return Returns general <b>enum BtErrCode</b> for the operation.
     * @since 6.0
     */
    int OffloadStartPlaying(const BluetoothRemoteDevice &device, const std::vector<int32_t> &sessionsId);

    /**
     * @brief Audio stop offload streaming for hardware encoding datapath.
     *
     * @param device remote bluetooth sink device.
     * @return Returns general <b>enum BtErrCode</b> for the operation.
     * @since 6.0
     */
    int OffloadStopPlaying(const BluetoothRemoteDevice &device, const std::vector<int32_t> &sessionsId);

    /**
     * brief Get a2dp encoding data path information of connected sink device.
     *
     * @param device remote bluetooth sink device.
     * @info streams detail information
     * @return Returns <b>UNKNOWN_ENCODING_PATH: 0</b>, bt unable to judge encoding data path
     *         Returns <b>SOFTWARE_ENCODING_PATH: 1</b>, a2dp audio encoding path should select a2dp hdi.
     *         Returns <b>HARDWARE_ENCODING_PATH: 2</b>, a2dp audio encoding path should select a2dp offload hdi.
     *         Returns general <b>enum BtErrCode</b> for the operation.
     */
    int A2dpOffloadSessionRequest(const BluetoothRemoteDevice &device, const std::vector<A2dpStreamInfo> &info);

    /**
     * @brief Get A2dp Offload codec status information of connected device.
     *
     * @param device remote bluetooth sink device.
     * @return a2dp offload configration information of connected device.
     * @since 6.0
     */
    A2dpOffloadCodecStatus GetOffloadCodecStatus(const BluetoothRemoteDevice &device) const;

    /**
     * Allow devices to automatically play music when connected.
     *
     * @param device Remote bluetooth sink device.
     * @return Returns general <b>enum BtErrCode</b> for the operation.
     * @since 12
     */
    int EnableAutoPlay(const BluetoothRemoteDevice &device);

    /**
     * Restriction devices to play music within {@code duration} milliseconds of connection.
     *
     * @param device Remote bluetooth sink device.
     * @param duration Restricted duration <milliseconds>.
     * @return Returns general <b>enum BtErrCode</b> for the operation.
     * @since 12
     */
    int DisableAutoPlay(const BluetoothRemoteDevice &device, const int duration);

    /**
     * Obtains the duration for which automatic playback is disabled.
     *
     * @param device Remote bluetooth sink device.
     * @param duration Restricted duration <milliseconds>.
     * @return Returns general <b>enum BtErrCode</b> for the operation.
     * @since 12
     */
    int GetAutoPlayDisabledDuration(const BluetoothRemoteDevice &device, int &duration);

    /**
     * get virtual device list.
     *
     * @param devices virtual device list.
     * @since 12
     */
    void GetVirtualDeviceList(std::vector<std::string> &devices);
private:
    /**
     * @brief A constructor used to create a a2dp source instance.
     *
     * @since 6.0
     */
    A2dpSource(void);

    /**
     * @brief A destructor used to delete the a2dp source instance.
     *
     * @since 6.0
     */
    ~A2dpSource(void);
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(A2dpSource);
    BLUETOOTH_DECLARE_IMPL();

#ifdef DTFUZZ_TEST
    friend class BluetoothNoDestructor<A2dpSource>;
#endif
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // BLUETOOTH_A2DP_SRC_H
