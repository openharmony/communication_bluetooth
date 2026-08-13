/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_hfp_hf"
#endif

#include "hfp_hf_call_manager.h"
#include "hfp_hf_defines.h"
#include "hfp_hf_service.h"
#include "bluetooth_log.h"
#include "util/safe_vector.h"
#ifdef BLUETOOTH_WATCH_ENABLE
#include "watch_service.h"
#endif

namespace OHOS {
namespace bluetooth {
HfpHfCallManager::HfpHfCallManager(const std::string &address) : address_(address)
{}

HfpHfCallManager::~HfpHfCallManager()
{
    calls_.Clear();
    updateCalls_.Clear();
}

bool HfpHfCallManager::HasCallState(int state)
{
    bool isFind = false;
    auto callback = [state, &isFind] (int id, HandsFreeUnitCalls call) {
        if (call.GetState() == state) {
            isFind = true;
            HILOGI("[HFP HF] find call success. state=[%{public}d]", state);
        }
    };
    calls_.Iterate(callback);
    return isFind;
}

std::vector<HandsFreeUnitCalls> HfpHfCallManager::GetCurrentCalls()
{
    std::vector<HandsFreeUnitCalls> calls;
    auto callback = [&calls] (int id, HandsFreeUnitCalls call) {
        calls.push_back(call);
    };
    calls_.Iterate(callback);
    return calls;
}

void HfpHfCallManager::UpdateCall(HandsFreeUnitCalls &call)
{
    if (call.GetId() == HFP_HF_OUTGOING_CALL_ID) {
        calls_.EnsureInsert(call.GetId(), call);
        dialingCallTime_ = GetTimeStamp();
    } else {
        calls_.Erase(HFP_HF_OUTGOING_CALL_ID);
        updateCalls_.Erase(HFP_HF_OUTGOING_CALL_ID);
        dialingCallTime_ = 0;
    }
    updateCalls_.EnsureInsert(call.GetId(), call);
}

void HfpHfCallManager::UpdateCallDone()
{
    utility::SafeVector<int> finishCallIds;
    utility::SafeVector<HandsFreeUnitCalls> updateCalls;
    auto updateCallback = [this, &finishCallIds, &updateCalls] (int id, HandsFreeUnitCalls call) {
        HandsFreeUnitCalls callFind;
        if (!updateCalls_.Find(id, callFind)) {
            call.SetState(HFP_CALL_STATE_FINISHED);
            NotifyCallChanged(call);
            finishCallIds.Push(id);
        } else {
            if ((call.GetState() != callFind.GetState()) ||
                (call.GetNumber() != callFind.GetNumber()) ||
                (call.IsMultiParty() != callFind.IsMultiParty())) {
                call.SetState(callFind.GetState());
                call.SetNumber(callFind.GetNumber());
                call.SetMultiParty(callFind.IsMultiParty());
                NotifyCallChanged(call);
                updateCalls.Push(call);
            }
        }
    };
    calls_.Iterate(updateCallback);

    std::vector<HandsFreeUnitCalls> updateCallList = updateCalls.GetVector();
    for (const auto& call : updateCallList) {
        HILOGI("[HFP HF] update call state=[%{public}d]", call.GetState());
        calls_.EnsureInsert(call.GetId(), call);
    }

    std::vector<int> finishCallIdList = finishCallIds.GetVector();
    for (const auto& id : finishCallIdList) {
        HILOGI("[HFP HF] erase call id=[%{public}d]", id);
        calls_.Erase(id);
    }

    AddNewCall();
    ProcessUpdateCallTimeout();
#ifdef BLUETOOTH_WATCH_ENABLE
    auto watchService = WatchService::GetInstance();
    if (calls_.Size() == 0 && watchService != nullptr) {
        watchService->SetIsHfOnCall(address_, false);
        watchService->AllHfCallFinished(address_, false);
        watchService->SetAllowHfScoConnect(address_, false);
    }
#endif
    updateCalls_.Clear();
}

void HfpHfCallManager::ProcessUpdateCallTimeout()
{
    HandsFreeUnitCalls callFind;
    if (calls_.Size() == 1 && calls_.Find(HFP_HF_OUTGOING_CALL_ID, callFind)) {
        int64_t currentTime = GetTimeStamp();
        if ((currentTime > dialingCallTime_) && ((currentTime - dialingCallTime_) >= OUTGOING_TIMEOUT_MS)) {
            callFind.SetState(HFP_CALL_STATE_FINISHED);
            NotifyCallChanged(callFind);
            dialingCallTime_ = 0;
            calls_.Erase(HFP_HF_OUTGOING_CALL_ID);
        }
    }
}

void HfpHfCallManager::AddNewCall()
{
    auto addCallback = [this] (int id, HandsFreeUnitCalls call) {
        HandsFreeUnitCalls callFind;
        if (!calls_.Find(id, callFind)) {
            HILOGI("[HFP HF] new call id=[%{public}d]", call.GetId());
#ifdef BLUETOOTH_WATCH_ENABLE
            auto watchService = WatchService::GetInstance();
            if (watchService != nullptr) {
                watchService->AddNewCallToWatch(address_, call.GetId(), isScoConnected_);
            }
#endif
            calls_.Insert(call.GetId(), call);
            NotifyCallChanged(call);
        }
    };
    updateCalls_.Iterate(addCallback);
}

void HfpHfCallManager::NotifyCallChanged(const HandsFreeUnitCalls &call)
{
    RawAddress rawAddress(address_);
    HfpHfService *service = HfpHfService::GetService();
    if (service != nullptr) {
        service->NotifyCallChanged(rawAddress, call);
#ifdef BLUETOOTH_WATCH_ENABLE
        auto watchService = WatchService::GetInstance();
        if (watchService != nullptr) {
            watchService->ProcessCallStateChangeEvt(address_, call);
        }
#endif
    } else {
        HILOGE("[HFP HF]service is null.");
    }
    return;
}

void HfpHfCallManager::SetScoConnected(bool isScoConnected)
{
    isScoConnected_ = isScoConnected;
}
}  // namespace bluetooth
}  // namespace OHOS
