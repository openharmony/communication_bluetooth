/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "fixed_queue.h"
#include <string>
#include <thread>
#include <atomic>
#include <future>
#include <functional>

namespace utility {

class Dispatcher {
public:
    explicit Dispatcher(const std::string &name = "bt-dispatcher");
    virtual ~Dispatcher();
    void Initialize();
    void Uninitialize();
    void PostTask(const std::function<void()> &task);
    const std::string &Name() const;

private:
    static void Run(Dispatcher *dispatcher, std::promise<void> promise);
    std::string name_;
    std::thread *thread_ = nullptr;
    std::atomic_bool start_ = ATOMIC_FLAG_INIT;
    utility::FixedQueue<std::function<void()> > taskQueue_;

    DISALLOW_COPY_AND_ASSIGN(Dispatcher);
};

}  // namespace utility

#endif  // DISPATCHER_H