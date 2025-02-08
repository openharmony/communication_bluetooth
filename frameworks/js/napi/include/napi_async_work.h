/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef NAPI_ASYNC_WORK_H
#define NAPI_ASYNC_WORK_H

#include <memory>
#include <mutex>
#include "napi_bluetooth_utils.h"
#include "napi_native_object.h"

namespace OHOS {
namespace Bluetooth {
struct NapiAsyncCallback;

enum NapiAsyncType : int {
    GATT_CLIENT_READ_CHARACTER,
    GATT_CLIENT_READ_REMOTE_RSSI_VALUE,
    GATT_CLIENT_READ_DESCRIPTOR,
    GATT_CLIENT_WRITE_CHARACTER,
    GATT_CLIENT_WRITE_DESCRIPTOR,
    GATT_CLIENT_ENABLE_CHARACTER_CHANGED,
    GATT_SERVER_NOTIFY_CHARACTERISTIC,
    GET_ADVERTISING_HANDLE,
    BLE_STOP_ADVERTISING,
    BLE_START_SCAN,
    BLE_STOP_SCAN,
    BLE_ENABLE_ADVERTISING,
    BLE_DISABLE_ADVERTISING
};

static constexpr bool ASYNC_WORK_NEED_CALLBACK = true;
static constexpr bool ASYNC_WORK_NO_NEED_CALLBACK = false;

struct NapiAsyncWorkRet {
    NapiAsyncWorkRet(int errCode) : errCode(errCode) {}
    NapiAsyncWorkRet(int errCode, std::shared_ptr<NapiNativeObject> object)
        : errCode(errCode), object(std::move(object)) {}

    int errCode = -1;
    std::shared_ptr<NapiNativeObject> object = nullptr;
};

class NapiAsyncWork : public std::enable_shared_from_this<NapiAsyncWork> {
public:
    NapiAsyncWork(napi_env env, std::function<NapiAsyncWorkRet(void)> func,
        std::shared_ptr<NapiAsyncCallback> asyncCallback, bool needCallback = false)
        : env_(env), func_(func), napiAsyncCallback_(asyncCallback), needCallback_(needCallback) {}
    ~NapiAsyncWork() = default;

    void Run(void);
    void CallFunction(int errorCode, std::shared_ptr<NapiNativeObject> object);
    napi_value GetRet(void);

    struct Info {
        void Execute(void);
        void Complete(void);

        int errCode = -1;
        bool needCallback = false;
        napi_async_work asyncWork;
        std::shared_ptr<NapiNativeObject> object;
        std::shared_ptr<NapiAsyncWork> napiAsyncWork = nullptr;
    };

private:
    friend class NapiAsyncWorkMap;

    void TimeoutCallback(void);

    napi_env env_;
    uint32_t timerId_ = 0;  // Is used to reference a timer.
    std::function<NapiAsyncWorkRet(void)> func_;
    std::shared_ptr<NapiAsyncCallback> napiAsyncCallback_ = nullptr;
    std::atomic_bool needCallback_ = false; // Indicates whether an asynchronous work needs to wait for callback.
    std::atomic_bool triggered_ = false; // Indicates whether the asynchronous callback is called.
};

class NapiAsyncWorkFactory {
public:
    static std::shared_ptr<NapiAsyncWork> CreateAsyncWork(napi_env env, napi_callback_info info,
        std::function<NapiAsyncWorkRet(void)> asyncWork, bool needCallback = ASYNC_WORK_NO_NEED_CALLBACK);
};

class NapiAsyncWorkMap {
public:
    bool TryPush(NapiAsyncType type, std::shared_ptr<NapiAsyncWork> asyncWork);
    void Erase(NapiAsyncType type);
    std::shared_ptr<NapiAsyncWork> Get(NapiAsyncType type);

private:
    mutable std::mutex mutex_ {};
    std::map<int, std::shared_ptr<NapiAsyncWork>> map_ {};
};

void AsyncWorkCallFunction(NapiAsyncWorkMap &map, NapiAsyncType type, std::shared_ptr<NapiNativeObject> nativeObject,
    int status);

}  // namespace Bluetooth
}  // namespace OHOS
#endif // NAPI_ASYNC_WORK_H