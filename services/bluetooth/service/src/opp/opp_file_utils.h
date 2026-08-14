/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OPP_FILE_UTILS_H
#define OPP_FILE_UTILS_H

#include <map>
#include <string>
#include "ability_connect_callback_stub.h"
#include "ability_manager_client.h"
#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "common_event_support.h"
#include "bt_chr_transaction_manager.h"
#include "opp_def.h"
#include "want.h"
#include "thread_util.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace bluetooth {
extern const std::map<std::string, std::string> suffixToMimeTypeMap_;
constexpr const char* EVENT_BLUETOOTH_OPP_TAP_ACCEPT = "usual.event.bluetooth.OPP.TAP.ACCEPT";
constexpr const char* EVENT_BLUETOOTH_OPP_TAP_REJECT = "usual.event.bluetooth.OPP.TAP.REJECT";
constexpr const char* EVENT_BLUETOOTH_OPP_TAP_REMOVE = "usual.event.bluetooth.OPP.TAP.REMOVE";
constexpr const char* EVENT_BLUETOOTH_OPP_TAP_FINISH_NOTIFICATION = "usual.event.bluetooth.OPP.FINISH.NOTIFICATION";
constexpr const char* EVENT_BLUETOOTH_OPP_TAP_FINISH_REMOVE = "usual.event.bluetooth.OPP.FINISH.REMOVE";
constexpr const char* EVENT_BLUETOOTH_TRANSACTION_CHR_REPORT = "usual.event.bluetooth.REPORT.TRANSACTION";

constexpr const char* EVENT_NOTIFICATION_BT_TAP_ACCEPT = "ohos.event.notification.BT.TAP_ACCEPT";
constexpr const char* EVENT_NOTIFICATION_BT_TAP_REJECT = "ohos.event.notification.BT.TAP_REJECT";
constexpr const char* EVENT_NOTIFICATION_BT_TAP_REMOVE = "ohos.event.notification.BT.TAP_REMOVE";
constexpr const char* EVENT_NOTIFICATION_BT_TAP_FINISH_NOTIFICATION = "ohos.event.notification.BT.FINISH_NOTIFICATION";
constexpr const char* EVENT_NOTIFICATION_BT_TAP_FINISH_REMOVE = "ohos.event.notification.BT.FINISH_REMOVE";

const std::string EVENT_BLUETOOTH_REMOTEDEVICE_ACL_DISCONNECTED_REALMAC =
    "usual.event.bluetooth.remotedevice.ACL_DISCONNECTED_REALMAC";

const std::vector<std::string> MAMAGE_PERMISSIONS {"ohos.permission.MANAGE_BLUETOOTH"};
constexpr const char* MANAGE_BLUETOOTH = "ohos.permission.MANAGE_BLUETOOTH";
const uint64_t DELAY_TIME_MS = 100;
const int DEFAULT_VAL = 0;
constexpr int32_t SIGNAL_NUM = 3;
constexpr size_t MAX_FILE_NAME_LENGTH = 255;


constexpr const char* PHOTO_BUNDLENAME = "com.huawei.hmos.photos";
constexpr const char* PHOTO_ABILITYNAME = "com.huawei.hmos.photos.MainAbility";
constexpr const char* FILES_BUNDLENAME = "com.huawei.hmos.filemanager";
#ifdef BLUETOOTH_PC_ENABLE
//PC filemanager bundlename is different from other product
constexpr const char* FILES_ABILITYNAME = "MainAbility";
#else
constexpr const char* FILES_ABILITYNAME = "EntryAbility";
#endif

#define TIME_STMP_STR_LEN 32
const std::string MEDIA_PREFIX = "file://media/";
const std::string DOCS_PREFIX = "file://docs/";
const std::string REMOVEFILE_PREFIX = "removeFile: ";
const std::string FILE_ENCRYPTED_SUFFFIX = "dlp";

std::string GetFileNameFromFilePath(std::string filePath, bool isNeedTimeStamp);
std::string GetMimeTypeFromFilePath(std::string filePath);
std::string GetEncryptedFileName(const std::string filePath);
std::string GetLimitedLengthFileName(std::string fileName, size_t maxNameLen);
std::string GetFileExtension(std::string filePath);
std::string GetCurrentTime(bool isNeedTimeStamp);
BtTransactionStatisticsType GetBtTransactionStatisticType(const int transactionType);
BtTransactionStatisticsResult GetBtTransactionStatisticResult(const int result);
BtTransactionStatisticsSceneCode GetBtTransactionStatisticSceneCode(const int sceneCode);

class OppEventSubscriber : public OHOS::EventFwk::CommonEventSubscriber {
public:
    explicit OppEventSubscriber(const OHOS::EventFwk::CommonEventSubscribeInfo &subscriberInfo)
        : CommonEventSubscriber(subscriberInfo) {}
    ~OppEventSubscriber() {}

    void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data) override;
};

class OppAbilityManagerUtils {
public:
    static void StartUIAbilityByCall(AAFwk::Want &abilityWant, AAFwk::Want &eventWant);
    static void StartUIExtentionAbility(std::map<std::string, std::string> &params);
    static void SubscribOppReceiveEvent();
    static void WakeUpOppSendUIAbility(const std::string &fileName);
    static void SubscribTransactionChrEvent();
    static void SetOppContinuousTaskFunc(std::function<void(const std::string &action)> oppTaskFunc);
    static void SetAclDisConnEventFunc(std::function<void(const std::string &addr)> aclDisConnEventFunc);
};

class UIAbilityConnection : public AAFwk::AbilityConnectionStub {
public:
    explicit UIAbilityConnection(AAFwk::Want eventWant) : eventWant_(eventWant) {}
    virtual ~UIAbilityConnection() = default;

    /**
     * @brief will be execute when abilitity function onCreate() finished.
     *
     * when need publish event, delay 100ms to give time for abilitity acieve the event subscription.
     *
     * @return void
     */
    void OnAbilityConnectDone(const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int32_t resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode) override;
private:
    AAFwk::Want eventWant_;
};

class UIExtensionAbilityConnection : public AAFwk::AbilityConnectionStub {
public:
    UIExtensionAbilityConnection(
        const std::string commandStr, const std::string bundleName, const std::string abilityName)
        : commandStr_(commandStr), bundleName_(bundleName), abilityName_(abilityName)
    {}

    virtual ~UIExtensionAbilityConnection() = default;

    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int32_t resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode) override;

private:
    std::string commandStr_;
    std::string bundleName_;
    std::string abilityName_;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif // OPP_FILE_UTILS_H