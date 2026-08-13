/*
 * Copyright (C) 2025-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_refuse_policy"
#endif

#include "bluetooth_kia_refuse_policy_manager.h"
#include "log.h"
#include "syspara/parameters.h"
#include "bluetooth_errorcode.h"
#include "cJSON.h"
#include "control_intercept_plugin.h"
#include "ipc_skeleton.h"
#include "sg_collect_client.h"
namespace OHOS {
namespace bluetooth {
constexpr const char SYS_PARAM_SERVICE_FORCE_ENABLE[] = "const.pc_security.fileguard_force_enable";

BluetoothRefusePolicyManager &BluetoothRefusePolicyManager::GetInstance()
{
    static BluetoothRefusePolicyManager ins;
    return ins;
}

static int64_t GetBootTime()
{
    constexpr int64_t msPerSecond = 1000;
    constexpr int64_t nsPerMs = 1000000;
    struct timespec times;
    if (clock_gettime(CLOCK_MONOTONIC, &times) < 0) {
        HILOGE("Failed clock_gettime:%{public}s, ShouldRefuseConnect:false", strerror(errno));
        return INT64_MAX;
    }
    int64_t bootTime = ((times.tv_sec * msPerSecond) + (times.tv_nsec / nsPerMs));
    HILOGI("bootTime:%{public}lu", bootTime);
    return bootTime;
}

int32_t BluetoothRefusePolicyManager::UpdateRefusePolicy(const int32_t protocolType,
    const int32_t pid, const int64_t prohibitedSecondsTime)
{
    std::string devicesCheck = system::GetParameter(SYS_PARAM_SERVICE_FORCE_ENABLE, "");
    if (devicesCheck != "true") {
        HILOGE("hasNotCheckedDevice, check hwit devices=%{public}s", devicesCheck.c_str());
        return Bluetooth::BT_ERR_SYSTEM_PERMISSION_FAILED;
    }
    HILOGI("add policy-protocolType:%{public}d, pid:%{public}d, prohibitedSecondsTime:%{public}lu",
        protocolType, pid, prohibitedSecondsTime);
    std::lock_guard<std::mutex> lock(refusePolicyMapMutex_);
    refusePolicyMap_[std::make_pair(protocolType, pid)] = prohibitedSecondsTime;
    HILOGI("start update,refusePolicyMap_ size:%{public}d", refusePolicyMap_.size());
    for (auto iter = refusePolicyMap_.begin(); iter != refusePolicyMap_.end();) {
        if (GetBootTime() > iter->second) {
            HILOGI("delete policy-protocolType:%{public}d, pid:%{public}d, prohibitedSecondsTime:%{public}lu",
                iter->first.first, iter->first.second, iter->second);
            iter = refusePolicyMap_.erase(iter);
        } else {
            ++iter;
        }
    }
    return Bluetooth::BT_NO_ERROR;
}

bool BluetoothRefusePolicyManager::ShouldRefuseConnect(const int32_t protocolType, const int32_t pid)
{
    HILOGI("check-Type:%{public}d, pid:%{public}d", protocolType, pid);
    std::lock_guard<std::mutex> lock(refusePolicyMapMutex_);
    if (refusePolicyMap_.count(std::make_pair(protocolType, pid)) &&
        GetBootTime() < refusePolicyMap_[std::make_pair(protocolType, pid)]) {
        HILOGI("ShouldRefuseConnect-Type:%{public}d, pid:%{public}d", protocolType, pid);
        return true;
    }
    return false;
}

void BluetoothRefusePolicyManager::ReportRefuseInfo(int32_t pid, const std::string &filePath)
{
    int64_t refuseTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    cJSON *outJson = cJSON_CreateObject();
    if (outJson == nullptr) {
        HILOGE("ReportRefuseInfo json object created error");
        return;
    }
    cJSON_AddNumberToObject(outJson, "timestamp", refuseTime);
    cJSON_AddStringToObject(outJson, "type", "bluetooth_send");
    cJSON_AddNumberToObject(outJson, "ftype", 1);
    cJSON_AddNumberToObject(outJson, "process_pid", pid);
    cJSON_AddStringToObject(outJson, "kia_path", filePath.c_str());
    char *jsonContent = cJSON_PrintUnformatted(outJson);
    if (jsonContent == nullptr) {
        HILOGE("ReportRefuseInfo print json unformatted error");
        cJSON_Delete(outJson);
        outJson = nullptr;
        return;
    }
    std::string content = std::string(jsonContent);
    cJSON_free(jsonContent);
    cJSON_Delete(outJson);
    outJson = nullptr;
    std::shared_ptr<Security::SecurityGuard::EventInfo> eventInfo =
        std::make_shared<Security::SecurityGuard::EventInfo>(0x01C00000A, "1.0", content);
    int ret = Security::SecurityGuard::NativeDataCollectKit::ReportSecurityInfo(eventInfo);
    HILOGI("report pid:%{public}d, refuseTime:%{public}lu, ret:%{public}d", pid, refuseTime, ret);
}

BluetoothRefusePolicyManager::BluetoothRefusePolicyManager()
{}

BluetoothRefusePolicyManager::~BluetoothRefusePolicyManager()
{}

/*************************      CONTROL_INTERCEPT_PLUGIN       *******************************/
static bool KiaIsAllowedSPP(const ControlInterceptMessage &msg)
{
    if (BluetoothRefusePolicyManager::GetInstance().ShouldRefuseConnect(REFUSE_PROTOCOL_TYPE_SPP, msg.pid)) {
        BluetoothRefusePolicyManager::GetInstance().ReportRefuseInfo(msg.pid);
        HILOGE("Failed SPP because of Refuse Policy.");
        return false;
    }
    return true;
}

static bool KiaIsAllowedGATT(const ControlInterceptMessage &msg)
{
    if (BluetoothRefusePolicyManager::GetInstance().ShouldRefuseConnect(REFUSE_PROTOCOL_TYPE_GATT, msg.pid)) {
        BluetoothRefusePolicyManager::GetInstance().ReportRefuseInfo(msg.pid);
        HILOGE("Failed GATT because of Refuse Policy.");
        return false;
    }
    return true;
}

static bool KiaIsAllowedOPP(const ControlInterceptMessage &msg)
{
    if (BluetoothRefusePolicyManager::GetInstance().ShouldRefuseConnect(REFUSE_PROTOCOL_TYPE_OPP, msg.pid)) {
        if (msg.filePaths.empty()) {
            BluetoothRefusePolicyManager::GetInstance().ReportRefuseInfo(msg.pid);
        }
        for (const auto& path : msg.filePaths) {
            BluetoothRefusePolicyManager::GetInstance().ReportRefuseInfo(msg.pid, path);
        }
        HILOGE("Failed OPP because of Refuse Policy.");
        return false;
    }
    return true;
}

static ControlInterceptPlugin g_kiaRefusePolicyMangerPlugin = {
    .fastestGattConn = KiaIsAllowedGATT,
    .gattConn = KiaIsAllowedGATT,
    .socketConn = KiaIsAllowedSPP,
    .gattWrite = KiaIsAllowedGATT,
    .oppWrite = KiaIsAllowedOPP,
};
REGISTER_CONTROL_INTERCEPT_PLUGIN(g_kiaRefusePolicyMangerPlugin);
/*************************      CONTROL_INTERCEPT_PLUGIN       *******************************/
}  // namespace bluetooth
}  // namespace OHOS