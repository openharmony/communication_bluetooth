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

#ifndef AVRCP_TG_SERVICE_H
#define AVRCP_TG_SERVICE_H

#ifndef LOG_TAG
#define LOG_TAG "bt_service_avrcp_tg"
#endif

#include <atomic>
#include <deque>

#include "bt_def.h"
#include "context.h"
#include "interface_profile_avrcp_tg.h"
#include "raw_address.h"
#include "stub/media_service.h"

namespace OHOS {
namespace bluetooth {
/**
 * @brief This enumeration declares the states of the AVRCP TG service.
 */
enum AvrcTgServiceState : uint8_t {
    AVRC_TG_SERVICE_STATE_INVALID = 0x00,
    AVRC_TG_SERVICE_STATE_ENABLING,
    AVRC_TG_SERVICE_STATE_ENABLED,
    AVRC_TG_SERVICE_STATE_DISABLING,
    AVRC_TG_SERVICE_STATE_DISABLED,
};

/**
 * @brief This class provides a set of methods related to the role of the target described in the Audio/Video Remote
 * Control profile.
 */
class AvrcpTgService : public IProfileAvrcpTg, public utility::Context {
public:
    /**
     * @brief This class implement the <b>stub::MediaService::IObserver</b> interface for observing the state change.
     */
    class ObserverImpl : public stub::MediaService::IObserver {
    public:
        /**
         * @brief A constructor used to create an <b>ObserverImpl</b> instance.
         */
        ObserverImpl() = default;

        /**
         * @brief A destructor used to delete the <b>ObserverImpl</b> instance.
         */
        ~ObserverImpl() override = default;

        void OnConnectionStateChanged(const std::string &addr, int state) override;
        void OnPressButton(const std::string &addr, uint8_t button) override;
        void OnReleaseButton(const std::string &addr, uint8_t button) override;

        /**
         * @brief Responds the data of the <b>SetAddressedPlayer</b>.
         *
         * @param[in] addr    The address of the bluetooth device.
         * @param[in] label   The label which is used to distinguish different call.
         * @param[in] status  The detail result of the execution. Refer to <b>AvrcEsCode</b>.<br>
         *            @a AVRC_ES_CODE_NO_ERROR<br>
         *            @a AVRC_ES_CODE_INTERNAL_ERROR<br>
         *            @a AVRC_ES_CODE_UID_CHANGED<br>
         *            @a AVRC_ES_CODE_INVALID_PLAYER_ID<br>
         *            @a AVRC_ES_CODE_NO_AVAILABLE_PLAYERS
         */
        void OnSetAddressedPlayer(const std::string &addr, uint8_t label, int status) override;

        /**
         * @brief Responds the data of the <b>SetBrowsedPlayer</b>.
         *
         * @param[in] addr        The address of the bluetooth device.
         * @param[in] uidCounter  The value of the uid counter.
         * @param[in] numOfItems  The number of items in the directory.
         * @param[in] folderNames The current browsed path of the player. The first element in folderNames is the parent
         * folder. The root folder has no name. The names are in UTF-8.
         * @param[in] label       The label which is used to distinguish different call.
         * @param[in] status      The detail result of the execution. Refer to <b>AvrcEsCode</b>.<br>
         *            @a AVRC_ES_CODE_NO_ERROR<br>
         *            @a AVRC_ES_CODE_INTERNAL_ERROR<br>
         *            @a AVRC_ES_CODE_UID_CHANGED<br>
         *            @a AVRC_ES_CODE_INVALID_PLAYER_ID<br>
         *            @a AVRC_ES_CODE_PLAYER_NOT_BROWSABLE<br>
         *            @a AVRC_ES_CODE_PLAYER_NOT_ADDRESSED<br>
         *            @a AVRC_ES_CODE_NO_AVAILABLE_PLAYERS
         */
        void OnSetBrowsedPlayer(const std::string &addr, uint16_t uidCounter, uint32_t numOfItems,
            const std::vector<std::string> &folderNames, uint8_t label, int status) override;

        /**
         * @brief Responds the data of the <b>OnGetPlayerAppSettingAttributes</b>.
         *
         * @param[in] addr   The address of the bluetooth device.
         * @param[in] The attribute of the player application settings.
         * @param[in] label  The label which is used to distinguish different call.
         */
        void OnGetPlayerAppSettingAttributes(
            const std::string &addr, const std::deque<uint8_t> &attributes, uint8_t label) override;

        /**
         * @brief Responds the data of the <b>GetPlayerAppSettingValues</b>.
         *
         * @param[in] addr   The address of the bluetooth device.
         * @param[in] values The values associated witch the attribute.
         * @param[in] label  The label which is used to distinguish different call.
         */
        void OnGetPlayerAppSettingValues(
            const std::string &addr, const std::deque<uint8_t> &values, uint8_t label) override;

        /**
         * @brief Responds the data of the <b>GetCurrentPlayerAppSettingValue</b>.
         *
         * @param[in] addr       The address of the bluetooth device.
         * @param[in] attributes The attribute of the player application settings. Refer to <b>AvrcPlayerAttribute</b>.
         * @param[in] values     The values associated witch the attribute.
         * @param[in] label      The label which is used to distinguish different call.
         * @param[in] context    The context which is used to distinguish different purpose.
         */
        void OnGetPlayerAppSettingCurrentValue(const std::string &addr, const std::deque<uint8_t> &attributes,
            const std::deque<uint8_t> &values, uint8_t label, uint8_t context) override;

        /**
         * @brief Responds the data of the <b>SetSetPlayerAppSettingCurrentValue</b>.
         *
         * @param[in] addr  The address of the bluetooth device.
         * @param[in] label The label which is used to distinguish different call.
         */
        void OnSetPlayerAppSettingCurrentValue(const std::string &addr, uint8_t label) override;

        /**
         * @brief Responds the data of the <b>GetPlayerAppSettingAttributeTest</b>.
         *
         * @param[in] addr       The address of the bluetooth device.
         * @param[in] attributes The attribute of the player application settings.
         * @param[in] attrStr    The values associated witch the attribute.
         * @param[in] label      The label which is used to distinguish different call.
         */
        void OnGetPlayerAppSettingAttributeText(const std::string &addr, const std::vector<uint8_t> &attributes,
            const std::vector<std::string> &attrStr, uint8_t label) override;

        /**
         * @brief Responds the data of the <b>GetPlayerAppSettingValueText</b>.
         *
         * @param[in] addr       The address of the bluetooth device.
         * @param[in] values     Player application setting value ID.
         * @param[in] valueStr   Specifies the value string of player application setting value.
         * @param[in] label      The label which is used to distinguish different call.
         */
        void OnGetPlayerAppSettingValueText(const std::string &addr, const std::vector<uint8_t> &values,
            const std::vector<std::string> &valueStr, uint8_t label) override;

        /**
         * @brief Responds the data of the <b>GetElementAttributes</b>.
         *
         * @details Switch to the thread of the AVRCP CT service in this function.
         * @param[in] rawAddr    The address of the bluetooth device.
         * @param[in] attributes  Specifies the attribute ID for the attributes to be retrieved.
         * @param[in] values     The list of the value of this attribute.
         * @param[in] label      The label which is used to distinguish different call.
         */
        void OnGetElementAttributes(const std::string &addr, const std::vector<uint32_t> &attribtues,
            const std::vector<std::string> &values, uint8_t label) override;

        /**
         * @brief Responds the data of the <b>GetPlayStatus</b>.
         *
         * @param[in] addr         The address of the bluetooth device.
         * @param[in] songLength   The total length of the playing song in milliseconds.
         * @param[in] songPosition The current position of the playing in milliseconds elapsed.
         * @param[in] playStatus   The current status of playing. Refer to <b>bluetooth::AvrcPlayStatus</b>.
         * @param[in] label        The label which is used to distinguish different call.
         * @param[in] context      The context which is used to distinguish different purpose.
         */
        void OnGetPlayStatus(const std::string &addr, uint32_t songLength, uint32_t songPosition, uint8_t playStatus,
            uint8_t label, uint8_t context) override;

        /**
         * @brief Responds the data of the <b>PlayItem</b>.
         *
         * @param[in] addr    The address of the bluetooth device.
         * @param[in] label   The label which is used to distinguish different call.
         * @param[in] status  The detail result of the execution. Refer to <b>AvrcEsCode</b>.<br>
         *            @a AVRC_ES_CODE_INVALID_COMMAND<br>
         *            @a AVRC_ES_CODE_INTERNAL_ERROR<br>
         *            @a AVRC_ES_CODE_INVALID_PARAMETER<br>
         *            @a AVRC_ES_CODE_PARAMETER_CONTENT_ERROR<br>
         *            @a AVRC_ES_CODE_NO_ERROR<br>
         *            @a AVRC_ES_CODE_UID_CHANGED<br>
         *            @a AVRC_ES_CODE_RESERVED<br>
         *            @a AVRC_ES_CODE_DOES_NOT_EXIST<br>
         *            @a AVRC_ES_CODE_INVALID_SCOPE
         *            @a AVRC_ES_CODE_FOLDER_ITEM_IS_NOT_PLAYABLE
         *            @a AVRC_ES_CODE_MEDIA_IN_USE
         *            @a AVRC_ES_CODE_NO_AVAILABLE_PLAYERS
         *            @a AVRC_ES_CODE_FOLDER_ITEM_IS_NOT_PLAYABLE
         */
        void OnPlayItem(const std::string &addr, uint8_t label, int status) override;

        /**
         * @brief Responds the data of the <b>AddToNowPlaying</b>.
         *
         * @param[in] addr    The address of the bluetooth device.
         * @param[in] label   The label which is used to distinguish different call.
         * @param[in] status  The detail result of the execution. Refer to <b>AvrcEsCode</b>.<br>
         *            @a AVRC_ES_CODE_NO_ERROR<br>
         *            @a AVRC_ES_CODE_INTERNAL_ERROR<br>
         *            @a AVRC_ES_CODE_UID_CHANGED<br>
         *            @a AVRC_ES_CODE_DOES_NOT_EXIST<br>
         *            @a AVRC_ES_CODE_INVALID_SCOPE<br>
         *            @a AVRC_ES_CODE_FOLDER_ITEM_IS_NOT_PLAYABLE<br>
         *            @a AVRC_ES_CODE_MEDIA_IN_USE<br>
         *            @a AVRC_ES_CODE_NOW_PLAYING_LIST_FULL<br>
         *            @a AVRC_ES_CODE_NO_AVAILABLE_PLAYERS
         */
        void OnAddToNowPlaying(const std::string &addr, uint8_t label, int status) override;

        /**
         * @brief Responds the data of the <b>ChangePath</b>.
         *
         * @param[in] addr       The address of the bluetooth device.
         * @param[in] numOfItems The number of items in the directory.
         * @param[in] label      The label which is used to distinguish different call.
         * @param[in] status     The detail result of the execution. Refer to <b>AvrcEsCode</b>.<br>
         *            @a AVRC_ES_CODE_NO_ERROR<br>
         *            @a AVRC_ES_CODE_INTERNAL_ERROR<br>
         *            @a AVRC_ES_CODE_UID_CHANGED<br>
         *            @a AVRC_ES_CODE_INVALID_DIRECTION<br>
         *            @a AVRC_ES_CODE_NOT_A_DIRECTORY<br>
         *            @a AVRC_ES_CODE_DOES_NOT_EXIST<br>
         *            @a AVRC_ES_CODE_NO_AVAILABLE_PLAYERS
         */
        void OnChangePath(const std::string &addr, uint32_t numOfItems, uint8_t label, int status) override;

        /**
         * @brief Responds the data of the <b>GetFolderItems</b>.
         *
         * @param[in] addr       The address of the bluetooth device.
         * @param[in] uidCounter The value of the uid counter.
         * @param[in] items      The list of media player.
         * @param[in] label      The label which is used to distinguish different call.
         * @param[in] status     The detail result of the execution. Refer to <b>AvrcEsCode</b>.<br>
         *            @a AVRC_ES_CODE_NO_ERROR<br>
         *            @a AVRC_ES_CODE_INTERNAL_ERROR<br>
         *            @a AVRC_ES_CODE_UID_CHANGED<br>
         *            @a AVRC_ES_CODE_DOES_NOT_EXIST<br>
         *            @a AVRC_ES_CODE_INVALID_SCOPE<br>
         *            @a AVRC_ES_CODE_NO_AVAILABLE_PLAYERS
         */
        void OnGetMediaPlayers(const std::string &addr, uint16_t uidCounter,
            const std::vector<stub::MediaService::MediaPlayer> &items, uint8_t label, int status) override;

        /**
         * @brief Responds the data of the <b>GetFolderItems</b>.
         *
         * @param[in] addr       The address of the bluetooth device.
         * @param[in] uidCounter The value of the uid counter.
         * @param[in] items      The list of media items.
         * @param[in] label      The label which is used to distinguish different call.
         * @param[in] status     The detail result of the execution. Refer to <b>AvrcEsCode</b>.<br>
         *            @a AVRC_ES_CODE_NO_ERROR<br>
         *            @a AVRC_ES_CODE_INTERNAL_ERROR<br>
         *            @a AVRC_ES_CODE_UID_CHANGED<br>
         *            @a AVRC_ES_CODE_DOES_NOT_EXIST<br>
         *            @a AVRC_ES_CODE_INVALID_SCOPE<br>
         *            @a AVRC_ES_CODE_NO_AVAILABLE_PLAYERS
         */
        void OnGetFolderItems(const std::string &addr, uint16_t uidCounter,
            const std::vector<stub::MediaService::MediaItem> &items, uint8_t label, int status) override;

        /**
         * @brief Responds the data of the <b>GetItemAttributes</b>.
         *
         * @param[in] addr       The address of the bluetooth device.
         * @param[in] attributes The list of media attributes.
         * @param[in] values     The list of the value of this attribute.
         * @param[in] label      The label which is used to distinguish different call.
         * @param[in] status     The detail result of the execution. Refer to <b>AvrcEsCode</b>.<br>
         *            @a AVRC_ES_CODE_NO_ERROR<br>
         *            @a AVRC_ES_CODE_INTERNAL_ERROR<br>
         *            @a AVRC_ES_CODE_UID_CHANGED<br>
         *            @a AVRC_ES_CODE_DOES_NOT_EXIST<br>
         *            @a AVRC_ES_CODE_INVALID_SCOPE<br>
         *            @a AVRC_ES_CODE_NO_AVAILABLE_PLAYERS
         */
        void OnGetItemAttributes(const std::string &addr, const std::vector<uint32_t> &attributes,
            const std::vector<std::string> &values, uint8_t label, int status) override;

        /**
         * @brief Responds the data of the <b>GetTotalNumberOfItems</b>.
         *
         * @param[in] addr       The address of the bluetooth device.
         * @param[in] uidCounter The value of the uid counter.
         * @param[in] numOfItems The number of items in the directory.
         * @param[in] label      The label which is used to distinguish different call.
         * @param[in] status     The detail result of the execution. Refer to <b>AvrcEsCode</b>.<br>
         *            @a AVRC_ES_CODE_NO_ERROR<br>
         *            @a AVRC_ES_CODE_INTERNAL_ERROR<br>
         *            @a AVRC_ES_CODE_UID_CHANGED<br>
         *            @a AVRC_ES_CODE_NO_AVAILABLE_PLAYERS
         */
        void OnGetTotalNumberOfItems(
            const std::string &addr, uint16_t uidCounter, uint32_t numOfItems, uint8_t label, int status) override;

        /**
         * @brief Responds the data of the <b>SetAbsoluteVolume</b>.
         *
         * @param[in] addr   The address of the bluetooth device.
         * @param[in] volume The percentage of the absolute volume. Refer to <b>AvrcAbsoluteVolume</b>.
         * @param[in] label  The label which is used to distinguish different call.
         */
        void OnSetAbsoluteVolume(const std::string &addr, uint8_t volume, uint8_t label) override;

        void OnGetSelectedTrack(const std::string &addr, uint64_t uid, uint8_t label) override;

        void OnGetAddressedPlayer(
            const std::string &addr, uint16_t playerId, uint16_t uidCounter, uint8_t label) override;

        void OnGetUidCounter(const std::string &addr, uint16_t uidCounter, uint8_t label) override;

        void OnGetCurrentAbsoluteVolume(const std::string &addr, uint8_t volume, uint8_t label) override;

    private:
        IProfileAvrcpTg *GetService(void);
    };

    /**
     * @brief A constructor used to create an <b>AvrcpTgService</b> instance.
     */
    AvrcpTgService();

    /**
     * @brief A destructor used to delete the <b>AvrcpTgService</b> instance.
     */
    ~AvrcpTgService() override;

    utility::Context *GetContext() override;

    /******************************************************************
     * REGISTER / UNREGISTER OBSERVER                                 *
     ******************************************************************/

    /**
     * @brief Registers the observer.
     *
     * @param[in] observer The pointer to the instance of the <b>AvrcTgProfile::IObserver</b>.
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
     * @brief Enables the AVRCP TG service.
     *
     * @details Switch to the thread of the AVRCP TG service in this method.
     * @return The result of the method execution.
     * @retval true  Execute success.
     * @retval false Execute failure.
     */
    void Enable(void) override;

    /**
     * @brief Disables the AVRCP TG service.
     *
     * @detail Switch to the thread of the AVRCP TG service in this method.
     * @return The result of the method execution.
     * @retval true  Execute success.
     * @retval false Execute failure.
     */
    void Disable(void) override;

    /**
     * @brief Checks whether the AVRCP TG service is enabled.
     *
     * @return The result of the method execution.
     * @retval true  The service is enabled.
     * @retval false The service is not enabled.
     */
    bool IsEnabled(void) override;

    /******************************************************************
     * CONNECTION                                                     *
     ******************************************************************/

    /**
     * @brief Sets the active device.
     *
     * @detail Only one CT can interact witch TG.
     */
    void SetActiveDevice(const RawAddress &rawAddr) override;

    /**
     * @brief Gets the devices of the specified states.
     *
     * @param[in] states The connection states. Refer to <b>BTConnectState</b>.
     * @return The list of the instance of the <b>RawAddress</b> class.
     */
    std::list<RawAddress> GetConnectDevices(void) override
    {
        std::list<RawAddress> rawAddrs;
        return rawAddrs;
    };

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
     * @brief Gets the maximum number of connections.
     *
     * @return The maximum number of connections.
     */
    int GetMaxConnectNum(void) override;

    /**
     * @brief Connects to the AVRCP TG service.
     *
     * @details Switch to the thread of the AVRCP TG service in this method.
     * @param[in] rawAddr The address of the bluetooth device.
     * @return The result of the method execution.
     * @retval RET_NO_ERROR   Execute success.
     * @retval RET_NO_SUPPORT The method is not supported.
     * @retval RET_BAD_STATUS Execute failure.
     */
    int Connect(const RawAddress &rawAddr) override;

    /**
     * @brief Disconnects from the AVRCP TG service.
     *
     * @details Switch to the thread of the AVRCP TG service in this method.
     * @param[in] rawAddr The address of the bluetooth device.
     * @return The result of the method execution.
     * @retval RET_NO_ERROR   Execute success.
     * @retval RET_NO_SUPPORT The method is not supported.
     * @retval RET_BAD_STATUS Execute failure.
     */
    int Disconnect(const RawAddress &rawAddr) override;

    /**
     * @brief Gets the connection states.
     *
     * @return The connection states. The values are in bits.
     */
    int GetConnectState(void) override;

    /******************************************************************
     * Media Player Selection                                         *
     ******************************************************************/

    /**
     * @brief Responds the data of the <b>SetAddressedPlayer</b>.
     *
     * @details Switch to the thread of the AVRCP TG service in this function.
     * @param[in] rawAddr The address of the bluetooth device.
     * @param[in] label   The label which is used to distinguish different call.
     * @param[in] status  The detail result of the execution. Refer to <b>AvrcEsCode</b>.<br>
     *            @a AVRC_ES_CODE_NO_ERROR<br>
     *            @a AVRC_ES_CODE_INTERNAL_ERROR<br>
     *            @a AVRC_ES_CODE_UID_CHANGED<br>
     *            @a AVRC_ES_CODE_INVALID_PLAYER_ID<br>
     *            @a AVRC_ES_CODE_NO_AVAILABLE_PLAYERS
     */
    void OnSetAddressedPlayer(const RawAddress &rawAddr, uint8_t label, int status) override;

    /**
     * @brief Responds the data of the <b>SetBrowsedPlayer</b>.
     *
     * @details Switch to the thread of the AVRCP TG service in this function.
     * @param[in] rawAddr     The address of the bluetooth device.
     * @param[in] uidCounter  The value of the uid counter.
     * @param[in] numOfItems  The number of items in the directory.
     * @param[in] folderNames The current browsed path of the player. The first element in folderNames is the parent
     * folder. The root folder has no name. The names are in UTF-8.
     * @param[in] label       The label which is used to distinguish different call.
     * @param[in] status      The detail result of the execution. Refer to <b>AvrcEsCode</b>.<br>
     *            @a AVRC_ES_CODE_NO_ERROR<br>
     *            @a AVRC_ES_CODE_INTERNAL_ERROR<br>
     *            @a AVRC_ES_CODE_UID_CHANGED<br>
     *            @a AVRC_ES_CODE_INVALID_PLAYER_ID<br>
     *            @a AVRC_ES_CODE_PLAYER_NOT_BROWSABLE<br>
     *            @a AVRC_ES_CODE_PLAYER_NOT_ADDRESSED<br>
     *            @a AVRC_ES_CODE_NO_AVAILABLE_PLAYERS
     */
    void OnSetBrowsedPlayer(const RawAddress &rawAddr, uint16_t uidCounter, uint32_t numOfItems,
        const std::vector<std::string> &folderNames, uint8_t label, int status) override;

    /******************************************************************
     * PLAYER APPLICATION SETTINGS                                    *
     ******************************************************************/

    /**
     * @brief Responds the data of the <b>GetPlayerAppSettingAttributes</b>.
     *
     * @details Switch to the thread of the AVRCP TG service in this function.
     * @param[in] rawAddr    The address of the bluetooth device.
     * @param[in] attributes The attribute of player application settings.
     * @param[in] label      The label which is used to distinguish different call.
     */
    void OnGetPlayerAppSettingAttributes(
        const RawAddress &rawAddr, const std::deque<uint8_t> &attributes, uint8_t label) override;

    /**
     * @brief Responds the data of the <b>GetPlayerAppSettingValues</b>.
     *
     * @details Switch to the thread of the AVRCP TG service in this function.
     * @param[in] rawAddr   The address of the bluetooth device.
     * @param[in] values    The values associated witch the attribute.
     * @param[in] label     The label which is used to distinguish different call.
     */
    void OnGetPlayerAppSettingValues(
        const RawAddress &rawAddr, const std::deque<uint8_t> &values, uint8_t label) override;

    /**
     * @brief Responds the data of the <b>GetCurrentPlayerAppSettingValue</b>.
     *
     * @details Switch to the thread of the AVRCP TG service in this function.
     * @param[in] rawAddr    The address of the bluetooth device.
     * @param[in] attributes The attribute of the player application settings. Refer to <b>AvrcPlayerAttribute</b>.
     * @param[in] values     The values associated witch the attribute.
     * @param[in] label      The label which is used to distinguish different call.
     * @param[in] context    The context which is used to distinguish different purpose.
     */
    void OnGetPlayerAppSettingCurrentValue(const RawAddress &rawAddr, const std::deque<uint8_t> &attributes,
        const std::deque<uint8_t> &values, uint8_t label, uint8_t context) override;

    /**
     * @brief Responds the data of the <b>SetPlayerAppSettingCurrentValue</b>.
     *
     * @details Switch to the thread of the AVRCP TG service in this function.
     * @param[in] rawAddr The address of the bluetooth device.
     * @param[in] label   The label which is used to distinguish different call.
     */
    void OnSetPlayerAppSettingCurrentValue(const RawAddress &rawAddr, uint8_t label) override;

    /**
     * @brief Responds the data of the <b>GetPlayerAppSettingAttributeTest</b>.
     *
     * @param[in] rawAddr    The address of the bluetooth device.
     * @param[in] attributes The attribute of the player application settings.
     * @param[in] attrStr    The values associated witch the attribute.
     * @param[in] label      The label which is used to distinguish different call.
     */
    void OnGetPlayerAppSettingAttributeText(const RawAddress &rawAddr, const std::vector<uint8_t> &attributes,
        const std::vector<std::string> &attrStr, uint8_t label) override;

    /**
     * @brief Responds the data of the <b>GetPlayerAppSettingValueText</b>.
     *
     * @param[in] addr       The address of the bluetooth device.
     * @param[in] values     Player application setting value ID.
     * @param[in] valueStr   Specifies the value string of player application setting value.
     * @param[in] label      The label which is used to distinguish different call.
     */
    void OnGetPlayerAppSettingValueText(const RawAddress &rawAddr, const std::vector<uint8_t> &values,
        const std::vector<std::string> &valueStr, uint8_t label) override;

    /******************************************************************
     * MEDIA INFORMATION PDUS                                         *
     ******************************************************************/

    /**
     * @brief Responds the data of the <b>GetElementAttributes</b>.
     *
     * @details Switch to the thread of the AVRCP CT service in this function.
     * @param[in] rawAddr    The address of the bluetooth device.
     * @param[in] attributes  Specifies the attribute ID forthe attributes to be retrieved.
     * @param[in] values     The list of the value of this attribute.
     * @param[in] label      The label which is used to distinguish different call.
     */
    void OnGetElementAttributes(const RawAddress &rawAddr, const std::vector<uint32_t> &attribtues,
        const std::vector<std::string> &values, uint8_t label) override;

    /******************************************************************
     * PLAY                                                           *
     ******************************************************************/

    /**
     * @brief Responds the data of the <b>GetPlayStatus</b>.
     *
     * @details Switch to the thread of the AVRCP TG service in this function.
     * @param[in] rawAddr      The address of the bluetooth device.
     * @param[in] songLength   The total length of the playing song in milliseconds.
     * @param[in] songPosition The current position of the playing in milliseconds elapsed.
     * @param[in] playStatus   The current status of playing. Refer to <b>AvrcPlayStatus</b>.
     * @param[in] label        The label which is used to distinguish different call.
     * @param[in] context      The context which is used to distinguish different purpose.     * @param[in] context The
     * context which is used to distinguish different purpose.     * @param[in] context      The context which is used
     * to distinguish different purpose.
     */
    void OnGetPlayStatus(const RawAddress &rawAddr, uint32_t songLength, uint32_t songPosition, uint8_t playStatus,
        uint8_t label, uint8_t context) override;

    /**
     * @brief Sends the response of the <b>PlayItem</b>.
     *
     * @details Switch to the thread of the AVRCP TG service in this function.
     * @param[in] rawAddr The address of the bluetooth device.
     * @param[in] label   The label which is used to distinguish different call.
     * @param[in] status  The current status of media.
     *            @a AVRC_ES_CODE_INVALID_COMMAND<br>
     *            @a AVRC_ES_CODE_INTERNAL_ERROR<br>
     *            @a AVRC_ES_CODE_INVALID_PARAMETER<br>
     *            @a AVRC_ES_CODE_PARAMETER_CONTENT_ERROR<br>
     *            @a AVRC_ES_CODE_NO_ERROR<br>
     *            @a AVRC_ES_CODE_UID_CHANGED<br>
     *            @a AVRC_ES_CODE_RESERVED<br>
     *            @a AVRC_ES_CODE_DOES_NOT_EXIST<br>
     *            @a AVRC_ES_CODE_INVALID_SCOPE
     *            @a AVRC_ES_CODE_FOLDER_ITEM_IS_NOT_PLAYABLE
     *            @a AVRC_ES_CODE_MEDIA_IN_USE
     *            @a AVRC_ES_CODE_NO_AVAILABLE_PLAYERS
     *            @a AVRC_ES_CODE_FOLDER_ITEM_IS_NOT_PLAYABLE
     */
    void OnPlayItem(const RawAddress &rawAddr, uint8_t label, int status) override;

    /**
     * @brief Responds the data of the <b>AddToNowPlaying</b>.
     *
     * @details Switch to the thread of the AVRCP TG service in this function.
     * @param[in] rawAddr The address of the bluetooth device.
     * @param[in] label   The label which is used to distinguish different call.
     * @param[in] status  The status of the method execution. Refer to <b>AvrcEsCode</b>.<br>
     *            @a AVRC_ES_CODE_NO_ERROR<br>
     *            @a AVRC_ES_CODE_INTERNAL_ERROR<br>
     *            @a AVRC_ES_CODE_UID_CHANGED<br>
     *            @a AVRC_ES_CODE_DOES_NOT_EXIST<br>
     *            @a AVRC_ES_CODE_INVALID_SCOPE<br>
     *            @a AVRC_ES_CODE_FOLDER_ITEM_IS_NOT_PLAYABLE<br>
     *            @a AVRC_ES_CODE_MEDIA_IN_USE<br>
     *            @a AVRC_ES_CODE_NOW_PLAYING_LIST_FULL<br>
     *            @a AVRC_ES_CODE_NO_AVAILABLE_PLAYERS
     */
    void OnAddToNowPlaying(const RawAddress &rawAddr, uint8_t label, int status) override;

    /******************************************************************
     * OPERATE THE VIRTUAL FILE SYSTEM                                *
     ******************************************************************/

    /**
     * @brief Responds the data of the <b>ChangePath</b>.
     *
     * @details Switch to the thread of the AVRCP TG service in this function.
     * @param[in] rawAddr    The address of the bluetooth device.
     * @param[in] numOfItems The number of items in the directory.
     * @param[in] label      The label which is used to distinguish different call.
     * @param[in] status     The status of the method execution. Refer to <b>AvrcEsCode</b>.<br>
     *            @a AVRC_ES_CODE_NO_ERROR<br>
     *            @a AVRC_ES_CODE_INTERNAL_ERROR<br>
     *            @a AVRC_ES_CODE_UID_CHANGED<br>
     *            @a AVRC_ES_CODE_INVALID_DIRECTION<br>
     *            @a AVRC_ES_CODE_NOT_A_DIRECTORY<br>
     *            @a AVRC_ES_CODE_DOES_NOT_EXIST<br>
     *            @a AVRC_ES_CODE_NO_AVAILABLE_PLAYERS
     */
    void OnChangePath(const RawAddress &rawAddr, uint32_t numOfItems, uint8_t label, int status) override;

    /**
     * @brief Responds the data of the <b>GetFolderItems</b>.
     *
     * @details Switch to the thread of the AVRCP TG service in this function.
     * @param[in] rawAddr    The address of the bluetooth device.
     * @param[in] uidCounter The value of the uid counter.
     * @param[in] items      The list of media player.
     * @param[in] label      The label which is used to distinguish different call.
     * @param[in] status     The detail result of the execution. Refer to <b>AvrcEsCode</b>.<br>
     *            @a AVRC_ES_CODE_NO_ERROR<br>
     *            @a AVRC_ES_CODE_INTERNAL_ERROR<br>
     *            @a AVRC_ES_CODE_UID_CHANGED<br>
     *            @a AVRC_ES_CODE_DOES_NOT_EXIST<br>
     *            @a AVRC_ES_CODE_INVALID_SCOPE<br>
     *            @a AVRC_ES_CODE_NO_AVAILABLE_PLAYERS
     */
    void OnGetMediaPlayers(const RawAddress &rawAddr, uint16_t uidCounter, const std::vector<AvrcMpItem> &items,
        uint8_t label, int status) override;

    /**
     * @brief Responds the data of the <b>GetFolderItems</b>.
     *
     * @details Switch to the thread of the AVRCP TG service in this function.
     * @param[in] rawAddr    The address of the bluetooth device.
     * @param[in] uidCounter The value of the uid counter.
     * @param[in] items      The list of media items.
     * @param[in] label      The label which is used to distinguish different call.
     * @param[in] status     The detail result of the execution. Refer to <b>AvrcEsCode</b>.<br>
     *            @a AVRC_ES_CODE_NO_ERROR<br>
     *            @a AVRC_ES_CODE_INTERNAL_ERROR<br>
     *            @a AVRC_ES_CODE_UID_CHANGED<br>
     *            @a AVRC_ES_CODE_DOES_NOT_EXIST<br>
     *            @a AVRC_ES_CODE_INVALID_SCOPE<br>
     *            @a AVRC_ES_CODE_NO_AVAILABLE_PLAYERS
     */
    void OnGetFolderItems(const RawAddress &rawAddr, uint16_t uidCounter, const std::vector<AvrcMeItem> &items,
        uint8_t label, int status) override;

    /**
     * @brief Responds the data of the <b>GetItemAttributes</b>.
     *
     * @details Switch to the thread of the AVRCP TG service in this function.
     * @param[in] rawAddr    The address of the bluetooth device.
     * @param[in] attributes The list of media attributes.
     * @param[in] values     The list of the value of this attribute.
     * @param[in] label      The label which is used to distinguish different call.
     * @param[in] status     The detail result of the execution. Refer to <b>AvrcEsCode</b>.<br>
     *            @a AVRC_ES_CODE_NO_ERROR<br>
     *            @a AVRC_ES_CODE_INTERNAL_ERROR<br>
     *            @a AVRC_ES_CODE_UID_CHANGED<br>
     *            @a AVRC_ES_CODE_DOES_NOT_EXIST<br>
     *            @a AVRC_ES_CODE_INVALID_SCOPE<br>
     *            @a AVRC_ES_CODE_NO_AVAILABLE_PLAYERS
     */
    void OnGetItemAttributes(const RawAddress &rawAddr, const std::vector<uint32_t> &attributes,
        const std::vector<std::string> &values, uint8_t label, int status) override;

    /**
     * @brief Responds the data of the <b>GetTotalNumberOfItems</b>.
     *
     * @details Switch to the thread of the AVRCP TG service in this function.
     * @param[in] rawAddr    The address of the bluetooth device.
     * @param[in] uidCounter The value of the uid counter.
     * @param[in] numOfItems The number of items in the directory.
     * @param[in] label      The label which is used to distinguish different call.
     * @param[in] status     The status of the method execution. Refer to <b>AvrcEsCode</b>.<br>
     *            @a AVRC_ES_CODE_NO_ERROR<br>
     *            @a AVRC_ES_CODE_INTERNAL_ERROR<br>
     *            @a AVRC_ES_CODE_UID_CHANGED<br>
     *            @a AVRC_ES_CODE_NO_AVAILABLE_PLAYERS
     */
    void OnGetTotalNumberOfItems(
        const RawAddress &rawAddr, uint16_t uidCounter, uint32_t numOfItems, uint8_t label, int status) override;

    /******************************************************************
     * ABSOLUTE VOLUME                                                *
     ******************************************************************/

    /**
     * @brief Responds the data of the <b>SetAbsoluteVolume</b>.
     *
     * @details Switch to the thread of the AVRCP TG service in this function.
     * @param[in] rawAddr The address of the bluetooth device.
     * @param[in] volume  The percentage of the absolute volume. Refer to <b>AvrcAbsoluteVolume</b>.
     * @param[in] label   The label which is used to distinguish different call.
     */
    void OnSetAbsoluteVolume(const RawAddress &rawAddr, uint8_t volume, uint8_t label) override;

    /******************************************************************
     * NOTIFICATION                                                   *
     ******************************************************************/

    void OnGetSelectedTrack(const RawAddress &rawAddr, uint64_t uid, uint8_t label) override;

    void OnGetAddressedPlayer(
        const RawAddress &rawAddr, uint16_t playerId, uint16_t uidCounter, uint8_t label) override;

    void OnGetUidCounter(const RawAddress &rawAddr, uint16_t uidCounter, uint8_t label) override;

    void OnGetCurrentAbsoluteVolume(const RawAddress &rawAddr, uint8_t volume, uint8_t label) override;

    /**
     * @brief Notifies the playback status is changed.
     *
     * @param[in] playStatus  The current status of playing. Refer to <b>AvrcPlayStatus</b>.
     * @param[in] playbackPos Current playback position in millisecond.
     * @param[in] label       The label which is used to distinguish different call.
     */
    void NotifyPlaybackStatusChanged(
        uint8_t playStatus, uint32_t playbackPos, uint8_t label = AVRC_DEFAULT_LABEL) override;

    /**
     * @brief Notifies the track reached end is changed.
     *
     * @param[in] uid         The unique ID of media item.
     * @param[in] playbackPos Current playback position in millisecond.
     * @param[in] label       The label which is used to distinguish different call.
     */
    void NotifyTrackChanged(uint64_t uid, uint32_t playbackPos, uint8_t label = AVRC_DEFAULT_LABEL) override;

    /**
     * @brief Notifies the track reached end is changed.
     *
     * @param[in] playbackPos Current playback position in millisecond.
     * @param[in] label       The label which is used to distinguish different call.
     */
    void NotifyTrackReachedEnd(uint32_t playbackPos, uint8_t label = AVRC_DEFAULT_LABEL) override;

    /**
     * @brief Notifies the track reached start is changed.
     *
     * @param[in] playbackPos Current playback position in millisecond.
     * @param[in] label       The label which is used to distinguish different call.
     */
    void NotifyTrackReachedStart(uint32_t playbackPos, uint8_t label = AVRC_DEFAULT_LABEL) override;

    /**
     * @brief Notifies the player application setting is changed.
     *
     * @param[in] playbackPos Current playback position in millisecond.
     * @param[in] label       The label which is used to distinguish different call.
     */
    void NotifyPlaybackPosChanged(uint32_t playbackPos, uint8_t label = AVRC_DEFAULT_LABEL) override;

    /**
     * @brief Notifies the player application setting is changed.
     *
     * @param[in] attribute The attribute of the player application setting. Refer to <b>AvrcPlayerAttribute</b>.
     * @param[in] values    The values associated witch the attribute.
     * @param[in] label     The label which is used to distinguish different call.
     */
    void NotifyPlayerAppSettingChanged(const std::deque<uint8_t> &attributes, const std::deque<uint8_t> &values,
        uint8_t label = AVRC_DEFAULT_LABEL) override;

    /**
     * @brief Notifies the now playing content is changed.
     *
     * @param[in] label The label which is used to distinguish different call.
     */
    void NotifyNowPlayingContentChanged(uint8_t label = AVRC_DEFAULT_LABEL) override;

    /**
     * @brief Notifies the addressed player is changed.
     *
     * @param[in] label The label which is used to distinguish different call.
     */
    void NotifyAvailablePlayersChanged(uint8_t label = AVRC_DEFAULT_LABEL) override;

    /**
     * @brief Notifies the addressed player is changed.
     *
     * @param[in] playerId   The unique media player id.
     * @param[in] uidCounter The UID counter shall be incremented every time the TG makes an update.
     * @param[in] label      The label which is used to distinguish different call.
     */
    void NotifyAddressedPlayerChanged(
        uint16_t playerId, uint16_t uidCounter, uint8_t label = AVRC_DEFAULT_LABEL) override;

    /**
     * @brief Notifies the uids is changed.
     *
     * @param[in] uidCounter The UID counter shall be incremented every time the TG makes an update.
     * @param[in] label      The label which is used to distinguish different call.
     */
    void NotifyUidChanged(uint16_t uidCounter, uint8_t label = AVRC_DEFAULT_LABEL) override;

    /**
     * @brief Notifies the absolute volume is changed.
     *
     * @param[in] volume The percentage of the absolute volume. Refer to <b>AvrcAbsoluteVolume</b>.
     * @param[in] label  The label which is used to distinguish different call.
     */
    void NotifyVolumeChanged(uint8_t volume, uint8_t label = AVRC_DEFAULT_LABEL) override;

    /**
     * @brief Notifies the AVSession is created.
     */
    void OnSessionCreate(std::string sessionId) override;

    /**
     * @brief Notifies the AVSession is released.
     */
    void OnSessionRelease(std::string sessionId) override;

    /**
     * @brief Notifies the TopAVSession is changed.
     */
    void OnTopSessionChange(std::string sessionId) override;

    /**
     * @brief Notifies the playback state is changed.
     */
    void OnPlaybackStateChange(const int32_t state) override;

   /**
     * @brief Enables the AVRCP TG service.
     *
     * @return The result of the method execution.
     * @retval true  Execute success.
     * @retval false Execute failure.
     */
    void EnableNative(void);

   /**
     * @brief Disables the AVRCP TG service.
     *
     * @return The result of the method execution.
     * @retval true  Execute success.
     * @retval false Execute failure.
     */
    void DisableNative(void);

    /**
     * @brief Set device abs volume ability.
     * @param[in] addr The device.
     * @param[in] ability The device abs volume ability.
     */
    void SetDeviceAbsVolumeAbility(const RawAddress &addr, int32_t ability) override;

    /**
     * @brief a2dp device actived, restore abs volume if support.
     * @param[in] addr The device.
     * @return Device abs volume ability.
     */
    int32_t GetDeviceAbsVolumeAbility(const RawAddress &addr) override;

    /**
     * @brief system volume changed, audio notify volume changed.
     * @param[in] addr The device.
     * @param[in] volumeLevel The device volume.
     */
    void SetDeviceAbsoluteVolume(const RawAddress &addr, int32_t volumeLevel) override;

    /**
     * @brief Notify audio volume info.
     * @param[in] streamType The stream type.
     * @param[in] volume The device volume.
     */
    void NotifyAudioVolumeEvent(int32_t streamType, int32_t volume) override;
private:

    BT_DISALLOW_COPY_AND_ASSIGN(AvrcpTgService);
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // !AVRCP_TG_SERVICE_H
