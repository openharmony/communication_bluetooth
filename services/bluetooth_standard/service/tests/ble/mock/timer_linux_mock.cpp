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

#include "mock/timer.h"
#include "log.h"
#include <unistd.h>
#include <thread>
#include <future>
#include <memory>
#include <list>
#include <algorithm>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>

namespace utility {

Timer::Timer(const std::function<void()> &callback) : callback_(callback)
{
    callback();
}

Timer::~Timer()
{}

bool Timer::Start(int ms, bool isPeriodic)
{
    return true;
}

bool Timer::Stop()
{
    return true;
}

}  // namespace utility