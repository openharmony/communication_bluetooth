/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef AVRCP_CT_AVSESSION_CONTROLLER_H
#define AVRCP_CT_AVSESSION_CONTROLLER_H

#include <string>
#include <memory>
#include <functional>
#include <thread>
#include <chrono>
#include "avrcp_ct_define.h"
#include "avrcp_ct_avsession.h"
#include "avsession_errors.h"
#include "thread_util.h"

namespace OHOS {
namespace bluetooth {

class AvrcpCtStateMachine;

class AVControllerObserverImpl : public AVSession::AVControllerCallback {
public:
    explicit AVControllerObserverImpl() = default;
    ~AVControllerObserverImpl() = default;

    void SetMetaDataCallback(std::function<void(const AVSession::AVMetaData &)> callback)
    {
        metaDataCallback_ = callback;
    }
    void SetPlaybackStateCallback(std::function<void(const AVSession::AVPlaybackState &)> callback)
    {
        playbackStateCallback_ = callback;
    }

    void OnSessionDestroy() override {}
    void OnMetaDataChange(const AVSession::AVMetaData &data) override
    {
        HILOGI("received metaDataChange event");
        if (metaDataCallback_) {
            DoInAvrcpCtThread([this, data]() {
                if (metaDataCallback_) {
                    metaDataCallback_(data);
                }
            });
        }
    }
    void OnActiveStateChange(bool isActive) override {}
    void OnValidCommandChange(const std::vector<int32_t> &cmds) override {}
    void OnOutputDeviceChange(const int32_t, const AVSession::OutputDeviceInfo &) override {}
    void OnSessionEventChange(const std::string &event, const AAFwk::WantParams &args) override {}
    void OnQueueItemsChange(const std::vector<AVSession::AVQueueItem> &items) override {}
    void OnQueueTitleChange(const std::string &title) override {}
    void OnExtrasChange(const AAFwk::WantParams &extras) override {}
    void OnAVCallStateChange(const AVSession::AVCallState &avCallState) override {}
    void OnAVCallMetaDataChange(const AVSession::AVCallMetaData &avCallMetaData) override {}

    void OnPlaybackStateChange(const AVSession::AVPlaybackState &state) override
    {
        HILOGI("state:%{public}d, position:%{public}u %{public}u loopMode: %{public}d", state.GetState(),
               state.GetPosition().elapsedTime_, state.GetPosition().updateTime_, state.GetLoopMode());
        if (playbackStateCallback_) {
            DoInAvrcpCtThread([this, state]() {
                if (playbackStateCallback_) {
                    playbackStateCallback_(state);
                }
            });
        }
    }

private:
    std::function<void(const AVSession::AVMetaData &)> metaDataCallback_;
    std::function<void(const AVSession::AVPlaybackState &)> playbackStateCallback_;
};

class AvrcpCtAvSessionController : public std::enable_shared_from_this<AvrcpCtAvSessionController> {
public:
    explicit AvrcpCtAvSessionController(std::weak_ptr<AvrcpCtStateMachine> stateMachine)
        : stateMachine_(stateMachine) {}
    ~AvrcpCtAvSessionController() = default;

    void Init();
    void RegisterSessionListener();
    void CreateAvSessionController(const std::string &sessionId);
    void SetAvsessionFilter();
    void OnMetaDataChange(const AVSession::AVMetaData &data);
    void OnPlaybackStateChange(const AVSession::AVPlaybackState &state);
    void CheckTrackStateChanged(
        const AVSession::AVMetaData &mateData, const AVSession::AVPlaybackState &playbackState);
    void CheckPlaybackStateChanged(const AVSession::AVPlaybackState &playbackState);
    void Destroy();
    void OnSessionCreate(const AVSession::AVSessionDescriptor &descriptor);
    void OnTopSessionChange(const AVSession::AVSessionDescriptor &descriptor);

    std::shared_ptr<AVSession::AVSessionController> GetController()
    {
        return avSessionController_;
    }

    std::shared_ptr<AVControllerObserverImpl> GetObserver()
    {
        return avControllerObserver_;
    }

private:
    std::weak_ptr<AvrcpCtStateMachine> stateMachine_;
    std::shared_ptr<AVSession::AVSessionController> avSessionController_ { nullptr };
    std::shared_ptr<AVControllerObserverImpl> avControllerObserver_ { nullptr };
    std::shared_ptr<AVSession::SessionListener> avSessionObserver_ { nullptr };
    bool isControllerCreated_ = false;
    int retryCount_ = 0;
    static constexpr int MAX_RETRY_COUNT = 5;
    static constexpr int RETRY_INTERVAL_MS = 100;
};

class AVSessionObserverImpl : public AVSession::SessionListener {
public:
    explicit AVSessionObserverImpl(std::weak_ptr<AvrcpCtAvSessionController> controller)
        : controller_(controller) {}
    ~AVSessionObserverImpl() = default;

    void OnSessionCreate(const AVSession::AVSessionDescriptor &descriptor) override
    {
        HILOGI("[AVSessionObserverImpl] OnSessionCreate, sessionId:%{public}s, isTopSession_:%{public}d", descriptor.sessionId_.c_str(), descriptor.isTopSession_);
        auto ctrl = controller_.lock();
        if (ctrl) {
            ctrl->OnSessionCreate(descriptor);
        }
    }

    void OnSessionRelease(const AVSession::AVSessionDescriptor &descriptor) override
    {
        HILOGI("[AVSessionObserverImpl] OnSessionRelease, sessionId:%{public}s", descriptor.sessionId_.c_str());
    }

    void OnTopSessionChange(const AVSession::AVSessionDescriptor &descriptor) override
    {
        HILOGI("[AVSessionObserverImpl] OnTopSessionChange, sessionId:%{public}s, isTopSession_:%{public}d",
            descriptor.sessionId_.c_str(), descriptor.isTopSession_);
        auto ctrl = controller_.lock();
        if (ctrl && descriptor.isTopSession_) {
            ctrl->OnTopSessionChange(descriptor);
        }
    }

private:
    std::weak_ptr<AvrcpCtAvSessionController> controller_;
};

} // namespace bluetooth
} // namespace OHOS

#endif // AVRCP_CT_AVSESSION_CONTROLLER_H