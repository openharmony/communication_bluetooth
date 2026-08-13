/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MAP_MSE_CONTENT_OBSERVER_H
#define MAP_MSE_CONTENT_OBSERVER_H

#include <cstdint>
#include <memory>
#include <map>
#include <string>
#include "map_mse_mns_client.h"
#include "map_mse_message_element.h"
#include "map_mse_appparams.h"
#include "map_mse_bmessage.h"
#include "map_mse_folder.h"
#include "thread_util.h"
#include "log.h"
#include "datashare_helper.h"
#include "i_send_short_message_callback.h"
#include "iremote_stub.h"
#include "send_short_message_callback_ipc_interface_code.h"

namespace OHOS {
namespace bluetooth {
const int32_t MAP_EVENT_REPORT_V10 = 10;
const int32_t MAP_EVENT_REPORT_V11 = 11;
const int32_t MAP_EVENT_REPORT_V12 = 12;
using namespace Telephony;
class MapMseContentObserver {
public:
    struct Message {
        int64_t handle;
        int32_t msgState;
        bool sent;
        bool readFlag;
    };

    class MapDataShareObserver : public DataShare::DataShareObserver {
    public:
        explicit MapDataShareObserver(MapMseContentObserver &instance) : mapMseContentObserver_(instance) {}
        ~MapDataShareObserver() = default;
        void OnChange(const ChangeInfo &changeInfo) override
        {
            DoInMapThread([observer = &mapMseContentObserver_]() { observer->HandleSmsMessageListChanged(); });
        }

    private:
        MapMseContentObserver &mapMseContentObserver_;
    };

    class MapSendMessageCallback : public IRemoteStub<ISendShortMessageCallback> {
    public:
        explicit MapSendMessageCallback(MapMseContentObserver &instance) : mapMseContentObserver_(instance) {}
        ~MapSendMessageCallback() = default;
        int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
        {
            if (data.ReadInterfaceToken() != GetDescriptor()) {
                HILOGE("descriptor checked fail");
                return -1;
            }
            switch (code) {
                case static_cast<int>(SendShortMessageCallbackInterfaceCode::ON_SMS_SEND_RESULT): {
                    int32_t result = data.ReadInt32();
                    OnSmsSendResult(static_cast<ISendShortMessageCallback::SmsSendResult>(result));
                    return 0;
                }
                default: {
                    return -1;
                }
            }
        }
        void OnSmsSendResult(const ISendShortMessageCallback::SmsSendResult result) override
        {
            DoInMapThread([observer = &mapMseContentObserver_, result]() { observer->HandleSmsSendResult(result); });
        }

    private:
        MapMseContentObserver &mapMseContentObserver_;
    };

    class MapEvent {
    public:
        MapEvent() {}
        MapEvent(int64_t handle, const std::string &type, const std::string &folder)
            : handle_(handle), type_(type), folder_(folder){};
        MapEvent(int64_t handle, const std::string &type, const std::string &folder, const std::string &oldFolder)
            : handle_(handle), type_(type), folder_(folder), oldFolder_(oldFolder){};
        void SetReportVersion(int32_t version) { reportVersion_ = version; }
        std::string Encode() const;

    private:
        int64_t handle_;
        std::string type_;
        std::string folder_;
        std::string oldFolder_;
        std::string dateTime_;
        std::string senderName_;
        std::string priority_;
        int32_t reportVersion_ = 0;
    };

    explicit MapMseContentObserver(std::weak_ptr<MapMseMnsClient> mnsClient) : mnsClient_(mnsClient)
    {
        SubscribeSmsEvent();
        InitMessageList();
    }
    ~MapMseContentObserver() { UnSubscribeSmsEvent(); };

    int32_t SetNotificationRegistration(int32_t masId, std::shared_ptr<MapMseAppParams> &params);
    int32_t SetMessageStatus(const std::string &handle, std::shared_ptr<MapMseAppParams> &params);
    int32_t SendNewMessage(const std::string &messageVersion, std::shared_ptr<MapMseFolder> &folder,
        std::shared_ptr<MapMseAppParams> &params, ObexHeader &reply, std::vector<uint8_t> &input);
    void HandleSmsMessageListChanged();
    void HandleSmsSendResult(const ISendShortMessageCallback::SmsSendResult result);
    void UpdateMnsClient(std::weak_ptr<MapMseMnsClient> mnsClient);

private:
    void SubscribeSmsEvent();
    void UnSubscribeSmsEvent();
    void InitMessageList();
    std::shared_ptr<DataShare::DataShareResultSet> QueryMessages();
    void SendEvent(const MapEvent &event);
    bool SetMessageStatusRead(int64_t handle, int32_t statusValue);
    bool SetMessageStatusDeleted(int64_t handle);
    bool PublishRefreshSmsListEvent();
    void HandleListChangeForNewMessage(std::map<int64_t, Message> &newMsgList, const Message &msg);
    void HandleListChangeForExistMessage(std::map<int64_t, Message> &newMsgList, const Message &msg);
    void HandleListChangeForDeleteMessage(std::map<int64_t, Message> &newMsgList);
    int64_t GetSendMessageHandle();
    std::string GetMessageEventFolder(const Message &msg);

private:
    int32_t masInstanceId_ = 0;
    uint8_t retryQueryTimes_ = 0;
    std::weak_ptr<MapMseMnsClient> mnsClient_;
    std::shared_ptr<DataShare::DataShareHelper> smsDataShareHelper_ = nullptr;
    std::shared_ptr<MapDataShareObserver> mapDataShareObserver_ = nullptr;
    sptr<MapSendMessageCallback> mapSendMessageCallback_ = nullptr;
    std::mutex smsMessageCollectionLock_;
    std::map<int64_t, Message> smsMessageCollection_;
    // 蓝牙发送短信后，拿到的handle
    int64_t smsMessageSentHandle_ = -1;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif