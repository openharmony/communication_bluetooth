/*
 * Copyright (C) 2024-2024 Huawei Device Co., Ltd.
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

#ifndef PARAM_UPDATE_EVENT_MANAGER_H
#define PARAM_UPDATE_EVENT_MANAGER_H

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include "common_event_subscriber.h"
#include "want.h"
#include "base_def.h"

namespace OHOS {
namespace bluetooth {
class ParamUpdateEventManager : public std::enable_shared_from_this<ParamUpdateEventManager> {
public:
    ParamUpdateEventManager();
    ~ParamUpdateEventManager();
    void SubscriberEvent();
    void UnSubscriberEvent();
private:
    void OnReceiveEvent(const AAFwk::Want &want);
    void HandleParamUpdate();
    void DoBtParamUpdate();
    void CheckIdleStateToTryParamUpdate();

    class ParamCommonEventSubscriber : public EventFwk::CommonEventSubscriber {
    public:
        explicit ParamCommonEventSubscriber(const EventFwk::CommonEventSubscribeInfo &subscriberInfo,
            ParamUpdateEventManager &registry)
            : CommonEventSubscriber(subscriberInfo), registry_(registry)
        {}
        ~ParamCommonEventSubscriber() = default;
 
        void OnReceiveEvent(const EventFwk::CommonEventData &data) override
        {
            registry_.OnReceiveEvent(data.GetWant());
        }
        
    private:
        ParamUpdateEventManager &registry_;
    };
    std::mutex paramUpdateLock_ {};
    bool isParamUpdateHandling_ = false;
    int32_t paramUpdateRetryCount_ = 0;
    std::shared_ptr<ParamCommonEventSubscriber> subscriber_ = nullptr;
    BT_DISALLOW_COPY_AND_ASSIGN(ParamUpdateEventManager);
};
}  // namespace bluetooth
}  // namespace OHOS

#endif
