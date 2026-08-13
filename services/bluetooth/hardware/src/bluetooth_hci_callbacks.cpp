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

#include "bluetooth_hci_callbacks.h"
#include "cstdint"
#include "hitrace_meter.h"
#include <sys/types.h>
#include <syscall.h>
#include <unistd.h>
const int32_t RT_PRIORITY = 1;
thread_local bool BluetoothHciCallbacks::isThreadPromoted = false;

int32_t BluetoothHciCallbacks::OnInited(BtStatus status)
{
    HITRACE_METER(BT_TRACE_TAG);
    if ((callbacks_ != nullptr) && (callbacks_->OnInited)) {
        BtInitStatus initStatus = BtInitStatus::SUCCESS;
        if (status != BtStatus::SUCCESS) {
            initStatus = BtInitStatus::INITIALIZATION_ERROR;
        }
        callbacks_->OnInited(initStatus);
    }
    return 0;
}

void BluetoothHciCallbacks::SetRTSchedule()
{
    if (isThreadPromoted) {
        return;
    }
    pid_t tid = gettid();
    struct sched_param rtParams = {.sched_priority = RT_PRIORITY};
    sched_setscheduler(tid, SCHED_FIFO, &rtParams);
    isThreadPromoted = true;
}

__attribute__((no_sanitize("cfi")))
int32_t BluetoothHciCallbacks::OnReceivedHciPacket(BtType type, const std::vector<uint8_t> &data)
{
    HITRACE_METER(BT_TRACE_TAG);
    SetRTSchedule();
    if ((callbacks_ != nullptr) && (callbacks_->OnReceivedHciPacket)) {
        BtPacketType packetType = BtPacketType::PACKET_TYPE_UNKNOWN;
        switch (type) {
            case BtType::HCI_EVENT:
                packetType = BtPacketType::PACKET_TYPE_EVENT;
                break;
            case BtType::ACL_DATA:
                packetType = BtPacketType::PACKET_TYPE_ACL;
                break;
            case BtType::SCO_DATA:
                packetType = BtPacketType::PACKET_TYPE_SCO;
                break;
            case BtType::ISO_DATA:
                packetType = BtPacketType::PACKET_TYPE_ISO;
                break;
            default:
                break;
        }
        BtPacket packet = {
            .data = (data.size() ? (uint8_t *)&data[0] : nullptr),
            .size = data.size(),
        };
        callbacks_->OnReceivedHciPacket(packetType, &packet);
    }
    return 0;
}