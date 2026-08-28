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

#ifndef AVRCP_CT_SERVICE_H
#define AVRCP_CT_SERVICE_H

#include <atomic>
#include <map>

#include "service_util.h"
#include "bt_rc.h"
#include "context.h"

#include "interface_profile_avrcp_ct.h"
#include "avrcp_ct_message.h"
#include "avrcp_ct_state_machine.h"
#include "bt_recursive_mutex.h"
#include "avrcp_ct_cover_art_manager.h"

namespace OHOS {
namespace bluetooth {
/**
 * @brief This enumeration declares the states of the AVRCP TG service.
 */
enum AvrcCtServiceState : uint8_t {
    AVRC_CT_SERVICE_STATE_INVALID,
    AVRC_CT_SERVICE_STATE_ENABLING,
    AVRC_CT_SERVICE_STATE_ENABLED,
    AVRC_CT_SERVICE_STATE_DISABLING,
    AVRC_CT_SERVICE_STATE_DISABLED,
};

/**
 * @brief This class provides a set of methods related to the role of the controller described in the Audio/Video Remote
 * Control profile.
 */
class AvrcpCtService : public IProfileAvrcpCt, public utility::Context {
public:
    static AvrcpCtService *GetService();

    /**
     * @brief A constructor used to create an <b>AvrcpCtService</b> instance.
     */
    AvrcpCtService();

    /**
     * @brief A destructor used to delete the <b>AvrcpCtService</b> instance.
     */
    ~AvrcpCtService() override;

    utility::Context *GetContext() override;

    /******************************************************************
     * REGISTER / UNREGISTER OBSERVER                                 *
     ******************************************************************/

    /**
     * @brief Registers the observer.
     *
     * @param observer The instance of the struct <b>IProfileAvrcpCt::Observer</b>.
     */
    void RegisterObserver(IObserver *observer) override;

    /**
     * @brief Unregisters the observer.
     */
    void UnregisterObserver(void) override;

    /******************************************************************
     * ENABLE / DISABLE                                               *
     ******************************************************************/

    /**
     * @brief Enables the AVRCP CT service.
     *
     * @details Switch to the thread of the AVRCP CT service in this method.
     * @return The result of the method execution.
     * @retval true  Execute success.
     * @retval false Execute failure.
     */
    void Enable(void) override;

    /**
     * @brief Disables the AVRCP CT service.
     *
     * @detail Switch to the thread of the AVRCP CT service in this method.
     * @return The result of the method execution.
     * @retval true  Execute success.
     * @retval false Execute failure.
     */
    void Disable(void) override;

    /**
     * @brief Checks whether the AVRCP CT service is enabled.
     *
     * @return The result of the method execution.
     * @retval true  The service is enabled.
     * @retval false The service is disabled.
     */
    bool IsEnabled(void) override;

    /******************************************************************
     * CONNECTION                                                     *
     ******************************************************************/

    /**
     * @brief Useless.
     */
    std::list<RawAddress> GetConnectDevices(void) override
    {
        std::list<RawAddress> rawAddrs;
        return rawAddrs;
    }

    /**
     * @brief Gets the connected devices.
     *
     * @return The list of the instance of the <b>RawAddress</b> class.
     */
    std::vector<RawAddress> GetConnectedDevices(void) override;

    /**
     * @brief Gets the devices of the specified states.
     *
     * @param[in] states The connection states. Refer to <b>BTConnectState</b>.
     * @return The list of the instance of the <b>RawAddress</b> class.
     */
    std::vector<RawAddress> GetDevicesByStates(const std::vector<int> &states) override;

    /**
     * @brief Gets the connection state of the specified bluetooth device.
     *
     * @param[in] rawAddr The address of the bluetooth device.
     * @return The connection state. Refer to <b>BTConnectState</b>.
     */
    int GetDeviceState(const RawAddress &rawAddr) override;

    /**
     * @brief Gets the max number of the connection.
     *
     * @return The max number of the connection.
     */
    int GetMaxConnectNum(void) override;

    /**
     * @brief Connects to the AVRCP TG service.
     *
     * @details Switch to the thread of the AVRCP CT service in this method.
     * @param[in] rawAddr The address of the bluetooth device.
     * @return The result of the method execution.
     * @retval RET_NO_ERROR   Execute success.
     * @retval RET_NO_SUPPORT Not support.
     * @retval RET_BAD_STATUS Execute failure.
     */
    int Connect(const RawAddress &rawAddr) override;

    /**
     * @brief Disconnects from the AVRCP TG service.
     *
     * @details Switch to the thread of the AVRCP CT service in this method.
     * @param[in] rawAddr The address of the bluetooth device.
     * @return The result of the method execution.
     * @retval RET_NO_ERROR   Execute success.
     * @retval RET_NO_SUPPORT Not support.
     * @retval RET_BAD_STATUS Execute failure.
     */
    int Disconnect(const RawAddress &rawAddr) override;

    /**
     * @brief Gets the connect states.
     *
     * @return The connect states.
     */
    int GetConnectState(void) override;

    /******************************************************************
     * BUTTON OPERATION                                               *
     ******************************************************************/

    /**
     * @brief Presses the button.
     *
     * @details Switch to the thread of the AVRCP CT service in this method.
     * @param[in] rawAddr The address of the bluetooth device.
     * @param[in] button  The value of the key operation. Refer to <b>AvrcKeyOperation</b>
     * @return The result of the method execution.
     * @retval RET_NO_ERROR   Execute success.
     * @retval RET_NO_SUPPORT Not support.
     * @retval RET_BAD_STATUS Execute failure.
     */
    int PressButton(const RawAddress &rawAddr, uint8_t button) override;

    /**
     * @brief Releases the button.
     *
     * @details Switch to the thread of the AVRCP CT service in this method.
     * @param[in] rawAddr The address of the bluetooth device.
     * @param[in] button  The value of the key operation. Refer to <b>AvrcKeyOperation</b>
     * @return The result of the method execution.
     * @retval RET_NO_ERROR   Execute success.
     * @retval RET_NO_SUPPORT Not support.
     * @retval RET_BAD_STATUS Execute failure.
     */
    int ReleaseButton(const RawAddress &rawAddr, uint8_t button) override;

    /******************************************************************
     * UNIT INFO / SUB UNIT INFO                                      *
     ******************************************************************/

    /**
     * @brief Gets the unit information.
     *
     * @details Switch to the thread of the AVRCP CT service in this method.
     * @param[in] rawAddr The address of the bluetooth device.
     * @return The result of the method execution.
     * @retval RET_NO_ERROR   Execute success.
     * @retval RET_NO_SUPPORT Not support.
     * @retval RET_BAD_STATUS Execute failure.
     */
    int GetUnitInfo(const RawAddress &rawAddr) override;

    /**
     * @brief Gets the sub unit information.
     *
     * @details Switch to the thread of the AVRCP CT service in this method.
     * @param[in] rawAddr The address of the bluetooth device.
     * @return The result of the method execution.
     * @retval RET_NO_ERROR   Execute success.
     * @retval RET_NO_SUPPORT Not support.
     * @retval RET_BAD_STATUS Execute failure.
     */
    int GetSubUnitInfo(const RawAddress &rawAddr) override;

    /******************************************************************
     * Media Player Selection                                         *
     ******************************************************************/

    /**
     * @brief Informs to which media player wishes to control.
     *
     * @details Switch to the thread of the AVRCP CT service in this method.
     * @param[in] rawAddr  The address of the bluetooth device.
     * @param[in] playerId The unique media player id.
     * @return The result of the method execution.
     * @retval RET_NO_ERROR   Execute success.
     * @retval RET_NO_SUPPORT Not Support.
     * @retval RET_BAD_STATUS Execute failure.
     */
    int SetAddressedPlayer(const RawAddress &rawAddr, uint16_t playerId) override;

    /**
     * @brief Informs to which player browsing commands should be routed.
     *
     * @details Switch to the thread of the AVRCP CT service in this method.
     * @param[in] rawAddr  The address of the bluetooth device.
     * @param[in] playerId The unique media player id.
     * @return The result of the method execution.
     * @retval RET_NO_ERROR   Execute success.
     * @retval RET_NO_SUPPORT Not Support.
     * @retval RET_BAD_STATUS Execute failure.
     */
    int SetBrowsedPlayer(const RawAddress &rawAddr, uint16_t playerId) override;

    /******************************************************************
     * Capabilities                                                   *
     ******************************************************************/

    /**
     * @brief Get the supported companies by remote device.
     *
     * @details This is sent by CT to get the capabilities of the peer device.
     * @param[in] rawAddr      The address of the bluetooth device.
     * @param[in] capabilityId Specific capability requested.
     * @return The result of the method execution.
     * @retval RET_NO_ERROR   Execute success.
     * @retval RET_NO_SUPPORT Not Support.
     * @retval RET_BAD_STATUS Execute failure.
     */
    int GetSupportedCompanies(const RawAddress &rawAddr) override;

    /**
     * @brief Get the supported events by remote device.
     *
     * @details This is sent by CT to get the capabilities of the peer device.
     * @param[in] rawAddr      The address of the bluetooth device.
     * @param[in] capabilityId Specific capability requested.
     * @return The result of the method execution.
     * @retval RET_NO_ERROR   Execute success.
     * @retval RET_NO_SUPPORT Not Support.
     * @retval RET_BAD_STATUS Execute failure.
     */
    int GetSupportedEvents(const RawAddress &rawAddr) override;

    /******************************************************************
     * PLAYER APPLICATION SETTINGS                                    *
     ******************************************************************/

    /**
     * @brief Gets the attribute of the player application.
     *
     * @details Switch to the thread of the AVRCP CT service in this method.
     * @param[in] rawAddr The address of the bluetooth device.
     * @return The result of the method execution.
     * @retval RET_NO_ERROR   Execute success.
     * @retval RET_NO_SUPPORT Not Support.
     * @retval RET_BAD_STATUS Execute failure.
     */
    int GetPlayerAppSettingAttributes(const RawAddress &rawAddr) override;

    /**
     * @brief Gets the values of the specified attribute of the player application.
     *
     * @details Switch to the thread of the AVRCP CT service in this method.
     * @param[in] rawAddr   The address of the bluetooth device.
     * @param[in] attribute The attribute of the player application setting. Refer to <b>AvrcPlayerAttribute</b>.
     * @return The result of the method execution.
     * @retval RET_NO_ERROR   Execute success.
     * @retval RET_NO_SUPPORT Not Support.
     * @retval RET_BAD_STATUS Execute failure.
     */
    int GetPlayerAppSettingValues(const RawAddress &rawAddr, uint8_t attribute) override;

    /**
     * @brief Gets the current set values on the target for the provided player application setting attributes list.
     *
     * @details Switch to the thread of the AVRCP CT service in this method.
     * @param[in] rawAddr    The address of the bluetooth device.
     * @param[in] attributes The attribute of the player application settings. Refer to <b>AvrcPlayerAttribute</b>.
     * @return The result of the method execution.
     * @retval RET_NO_ERROR   Execute success.
     * @retval RET_NO_SUPPORT Not Support.
     * @retval RET_BAD_STATUS Execute failure.
     */
    int GetPlayerAppSettingCurrentValue(const RawAddress &rawAddr, const std::vector<uint8_t> &attributes) override;

    /**
     * @brief Sets the player application setting list of player application setting values on the target device for the
     * corresponding defined list of AvrcPlayerAttribute.
     *
     * @details Switch to the thread of the AVRCP CT service in this method.
     * @param[in] rawAddr    The address of the bluetooth device.
     * @param[in] attributes The attribute of the player application settings. Refer to <b>AvrcPlayerAttribute</b>.
     * @param[in] values     The value of the player application setting attribute.
     * @return The result of the method execution.
     * @retval RET_NO_ERROR   Execute success.
     * @retval RET_NO_SUPPORT Not Support.
     * @retval RET_BAD_STATUS Execute failure.
     */
    int SetPlayerAppSettingCurrentValue(
        const RawAddress &rawAddr, const std::vector<uint8_t> &attributes, const std::vector<uint8_t> &values) override;

    /**
     * @brief  provide supported player application setting attribute displayable text.
     *
     * @details Switch to the thread of the AVRCP CT service in this method.
     * @param[in] rawAddr    The address of the bluetooth device.
     * @param[in] attributes The attribute of the player application settings.
     * @return The result of the method execution.
     * @retval RET_NO_ERROR   Execute success.
     * @retval RET_NO_SUPPORT Not Support.
     * @retval RET_BAD_STATUS Execute failure.
     */
    int GetPlayerAppSettingAttributeText(const RawAddress &rawAddr, const std::vector<uint8_t> &attributes) override;

    /**
     * @brief  request the target device to provide target supported player application setting value displayable text.
     *
     * @details Switch to the thread of the AVRCP CT service in this method.
     * @param[in] rawAddr     The address of the bluetooth device.
     * @param[in] attributeId Player application setting attribute ID.
     * @param[in] values      Player application setting value ID.
     * @return The result of the method execution.
     * @retval RET_NO_ERROR   Execute success.
     * @retval RET_NO_SUPPORT Not Support.
     * @retval RET_BAD_STATUS Execute failure.
     */
    int GetPlayerAppSettingValueText(
        const RawAddress &rawAddr, uint8_t attributeId, const std::vector<uint8_t> &values) override;

    /******************************************************************
     * MEDIA INFORMATION PDUS                                         *
     ******************************************************************/

    /**
     * @brief Requests the TG to provide the attributes of the element specified in the parameter.
     *
     * @details Switch to the thread of the AVRCP CT service in this function.
     * @param[in] rawAddr    The address of the bluetooth device.
     * @param[in] identifier Unique identifier to identify an element on TG
     * @param[in] attributes Specifies the attribute ID for the attributes to be retrieved
     *            @c RET_NO_ERROR   : The action is successful.
     *            @c RET_NO_SUPPORT : The action is not supported.
     *            @c RET_BAD_STATUS : The action is failed.
     */
    int GetElementAttributes(
        const RawAddress &rawAddr, uint64_t identifier, const std::vector<uint32_t> &attributes) override;

    /******************************************************************
     * PLAY                                                           *
     ******************************************************************/

    /**
     * @brief Gets the play status.
     *
     * @details Switch to the thread of the AVRCP CT service in this method.
     * @param[in] rawAddr The address of the bluetooth device.
     * @return The result of the method execution.
     * @retval RET_NO_ERROR   Execute success.
     * @retval RET_NO_SUPPORT Not support.
     * @retval RET_BAD_STATUS Execute failure.
     */
    int GetPlayStatus(const RawAddress &rawAddr) override;

    /**
     * @brief Starts playing an item indicated by the UID.
     *
     * @details Switch to the thread of the AVRCP CT service in this method.
     * @param[in] rawAddr    The address of the bluetooth device.
     * @param[in] scope      The scope in which media content navigation may take place. Refer to <b>AvrcMediaScope</b>.
     * @param[in] uid        The unique ID of media item.
     * @param[in] uidCounter The UID counter shall be incremented every time the TG makes an update.
     * @return The result of the method execution.
     * @retval RET_NO_ERROR   Execute success.
     * @retval RET_NO_SUPPORT Not support.
     * @retval RET_BAD_STATUS Execute failure.
     */
    int PlayItem(const RawAddress &rawAddr, uint8_t scope, uint64_t uid, uint16_t uidCounter) override;

    /**
     * @brief Adds an item indicated by the UID to the Now Playing queue.
     *
     * @details Switch to the thread of the AVRCP CT service in this method.
     * @param[in] rawAddr    The address of the bluetooth device.
     * @param[in] scope      The scope in which media content navigation may take place. Refer to <b>AvrcMediaScope</b>.
     * @param[in] uid        The UID of the media element item or folder item.
     * @param[in] uidCounter The UID Counter.
     * @return The result of the method execution.
     * @retval RET_NO_ERROR   Execute success.
     * @retval RET_NO_SUPPORT Not support.
     * @retval RET_BAD_STATUS Execute failure.
     */
    int AddToNowPlaying(const RawAddress &rawAddr, uint8_t scope, uint64_t uid, uint16_t uidCounter) override;

    /******************************************************************
     * CONTINUING RESPONSE / ABORT CONTINUING RESPONSE                *
     ******************************************************************/

    /**
     * @brief Requests continuing response.
     *
     * @details Switch to the thread of the AVRCP CT service in this method.
     * @param[in] rawAddr The address of the bluetooth device.
     * @param[in] pduId   The PDU ID which wants to request.
     * @return The result of the method execution.
     * @retval RET_NO_ERROR   Execute success.
     * @retval RET_NO_SUPPORT Not support.
     * @retval RET_BAD_STATUS Execute failure.
     */
    int RequestContinuingResponse(const RawAddress &rawAddr, uint8_t pduId) override;

    /**
     * @brief Aborts continuing response.
     *
     * @details Switch to the thread of the AVRCP CT service in this method.
     * @param[in] rawAddr The address of the bluetooth device.
     * @param[in] pduId   The PDU ID which wants to abort.
     * @return The result of the method execution.
     * @retval RET_NO_ERROR   Execute success.
     * @retval RET_NO_SUPPORT Not support.
     * @retval RET_BAD_STATUS Execute failure.
     */
    int AbortContinuingResponse(const RawAddress &rawAddr, uint8_t pduId) override;

    /******************************************************************
     * OPERATE THE VIRTUAL FILE SYSTEM                                *
     ******************************************************************/

    /**
     * @brief Navigates one level up or down in the virtual file system.
     *
     * @details Switch to the thread of the AVRCP CT service in this method.
     * @param[in] rawAddr    The address of the bluetooth device.
     * @param[in] uidCounter The value of the uid counter.
     * @param[in] direction  The flag of the navigation. Refer to <b>AvrcFolderDirection</b>.
     * @param[in] folderUid  The UID of the folder to navigate to. This may be retrieved via a GetFolderItems command.
     * If the navigation command is Folder Up this field is reserved.
     * @return The result of the method execution.
     * @retval RET_NO_ERROR   Execute success.
     * @retval RET_NO_SUPPORT Not support.
     * @retval RET_BAD_STATUS Execute failure.
     */
    int ChangePath(const RawAddress &rawAddr, uint16_t uidCounter, uint8_t direction, uint64_t folderUid) override;

    /**
     * @brief Retrieves a listing of the contents of a folder.
     *
     * @details Switch to the thread of the AVRCP CT service in this method.
     * @param[in] rawAddr    The address of the bluetooth device.
     * @param[in] scope      The scope in which media content navigation may take place. Refer to <b>AvrcMediaScope</b>.
     * @param[in] startItem  The offset within the listing of the item, which should be the first returned item. The
     * first element in the listing is at offset 0.
     * @param[in] endItem    The offset within the listing of the item which should be the final returned item. If this
     * is set to a value beyond what is available, the TG shall return items from the provided Start Item index to the
     * index of the final item. If the End Item index is smaller than the Start Item index, the TG shall return an
     * error. If CT requests too many items, TG can respond with a sub-set of the requested items.
     * @param[in] attributes The list of media attributes.
     * @return The result of the method execution.
     * @retval RET_NO_ERROR   Execute success.
     * @retval RET_NO_SUPPORT Not support.
     * @retval RET_BAD_STATUS Execute failure.
     */
    int GetFolderItems(const RawAddress &rawAddr, uint8_t scope, uint32_t startItem, uint32_t endItem,
        const std::vector<uint32_t> &attributes) override;

    /**
     * @brief Retrieves the metadata attributes for a particular media element item or folder item.
     *
     * @details Switch to the thread of the AVRCP CT service in this method.
     * @param[in] rawAddr    The address of the bluetooth device.
     * @param[in] scope      The scope in which media content navigation may take place. Refer to <b>AvrcMediaScope</b>.
     * @param[in] uid        The UID of the media element item or folder item.
     * @param[in] uidCounter The UID Counter.
     * @param[in] attributes The list of media attributes.
     * @return The result of the method execution.
     * @retval RET_NO_ERROR   Execute success.
     * @retval RET_NO_SUPPORT Not support.
     * @retval RET_BAD_STATUS Execute failure.
     */
    int GetItemAttributes(const RawAddress &rawAddr, uint8_t scope, uint64_t uid, uint16_t uidCounter,
        const std::vector<uint32_t> &attributes) override;

    /**
     * @brief Gets the number of items in the selected folder at the selected scope.
     *
     * @details Switch to the thread of the AVRCP CT service in this method.
     * @param[in] rawAddr The address of the bluetooth device.
     * @param[in] scope   The scope in which media content navigation may take place. Refer to <b>AvrcMediaScope</b>.
     * @return The result of the method execution.
     * @retval RET_NO_ERROR   Execute success.
     * @retval RET_NO_SUPPORT Not support.
     * @retval RET_BAD_STATUS Execute failure.
     */
    int GetTotalNumberOfItems(const RawAddress &rawAddr, uint8_t scope) override;

    /******************************************************************
     * ABSOLUTE VOLUME                                                *
     ******************************************************************/

    /**
     * @brief Sets an absolute volume to be used by the rendering device.
     *
     * @details Switch to the thread of the AVRCP CT service in this method.
     * @param[in] rawAddr The address of the bluetooth device.
     * @param[in] volume  The percentage of the absolute volume. Refer to <b>AvrcAbsoluteVolume</b>.
     * @return The result of the method execution.
     * @retval RET_NO_ERROR   Execute success.
     * @retval RET_NO_SUPPORT Not support.
     * @retval RET_BAD_STATUS Execute failure.
     */
    int SetAbsoluteVolume(const RawAddress &rawAddr, uint8_t volume) override;

    /******************************************************************
     * NOTIFICATION                                                   *
     ******************************************************************/

    /**
     * @brief Enables for receiving notifications asynchronously based on specific events occurring.
     *
     * @details Switch to the thread of the AVRCP CT service in this method.
     * @param[in] rawAddr  The address of the bluetooth device.
     * @param[in] events   The event for which the requires notification. Refer to <b>AvrcEventId</b>.
     * @param[in] interval The specifies the time interval (in seconds) at which the change in playback position will be
     * notified.
     * @return The result of the method execution.
     * @retval RET_NO_ERROR   Execute success.
     * @retval RET_NO_SUPPORT Not support.
     * @retval RET_BAD_STATUS Execute failure.
     */
    int EnableNotification(const RawAddress &rawAddr, const std::vector<uint8_t> &events,
        uint8_t interval = AVRC_PLAYBACK_INTERVAL_1_SEC) override;

    /**
     * @brief Disables for receiving notifications asynchronously based on specific events occurring.
     *
     * @details Switch to the thread of the AVRCP CT service in this method.
     * @param[in] rawAddr The address of the bluetooth device.
     * @param[in] events  The event for which the requires notification. Refer to <b>AvrcEventId</b>.
     * @return The result of the method execution.
     * @retval RET_NO_ERROR   Execute success.
     * @retval RET_NO_SUPPORT Not support.
     * @retval RET_BAD_STATUS Execute failure.
     */
    int DisableNotification(const RawAddress &rawAddr, const std::vector<uint8_t> &events) override;

    void PostEvent(const AvrcpCtMessage &event);
    void ProcessEvent(const AvrcpCtMessage &event);
    void RemoveStateMachine(const std::string &dev);
    bool SendPassThroughCommand(const RawAddress &rawAddr, uint8_t keyCode, uint8_t keyState);
    void SendAbsVolumeResponse(const RawAddress &rawAddr, int absVol, int label);
    void SendRegisterAbsVolResponse(const RawAddress &rawAddr, int rspType, int absVol, int label);
    void RequestCurrentMetadata(const RawAddress &rawAddr);
    void RequestPlaybackState(const RawAddress &rawAddr);
    void RequestRemotePlay(const RawAddress &rawAddr);
    void RequestRemotePause(const RawAddress &rawAddr);
    void GetCurrentMetadataIfNoCoverArt(const std::string &device);
    int GetConnectionState(const std::string &device);
    void GetRcPsm(const std::string &dev, int psm);
    void HandleAddressedPlayerChanged(const std::string &dev, int id);
    std::vector<RawAddress> GetDevicesMatchingConnectionStates(void);
    std::string GetUuidForHandle(const std::string &deviceAddr, const std::string &handle);
    std::shared_ptr<AvrcpCtCoverArtManager> GetCoverArtManager() const { return coverArtManager_; }
    std::string GetActiveDevice() const;

    // 本地音量变化通知（由 BluetoothAudioFrameworkAdapter::OnVolumeKeyEvent 调用）。
    // 对齐双框架 AvrcpControllerService.processAbsoluteVolumeChangeNotification：
    // 向所有已连接设备的 StateMachine 投递 VOLUME_CHANGED_NOTIFICATION，
    // 使 TG 注册的 abs vol notification 收到 CHANGED 响应，保持 TG 端绝对音量与车机本地同步。
    void NotifyAudioVolumeEvent(int32_t streamType, int32_t volume) override;

private:
    void StartUp();
    void ShutDown();
    void ShutDownDone(bool isAllDisconnected);
    void ProcessConnectEvent(const AvrcpCtMessage &event);
    bool SetActiveDevice(const std::string &dev);
    bool FindSmAndSendMsg(const std::string &dev, int msg);

    // 封面下载完成回调（对齐双框架 ImageDownloadCallback.onImageDownloadComplete）
    void OnCoverArtDownloaded(const std::string &deviceAddr, const std::string &imageUuid,
                              const std::vector<uint8_t> &imageData);

    bool isStarted_{false};
    bool isShuttingDown_{false};
    BtRecursiveMutex mutex_{};  // protect statemachine
    mutable BtRecursiveMutex activeDevMutex_;
    std::string activeDevice_{""};
    std::map<const std::string, std::shared_ptr<AvrcpCtStateMachine>> stateMachines_{};
    std::shared_ptr<AvrcpCtCoverArtManager> coverArtManager_;
    const BtrcCtrlInterface *btAvrcpInterface_{nullptr};

    BT_DISALLOW_COPY_AND_ASSIGN(AvrcpCtService);
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // !AVRCP_TG_SERVICE_H
