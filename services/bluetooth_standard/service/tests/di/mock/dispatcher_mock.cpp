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

#include "dispatcher.h"
#include "log.h"

namespace utility {

Dispatcher::Dispatcher(const std::string &name) : name_(name), taskQueue_(128)
{}

Dispatcher::~Dispatcher()
{}

void Dispatcher::Initialize()
{}

void Dispatcher::Uninitialize()
{}

void Dispatcher::PostTask(const std::function<void()> &task)
{
    task();
}

const std::string &Dispatcher::Name() const
{
    return name_;
}

void Dispatcher::Run(Dispatcher *dispatcher, std::promise<void> promise)
{}

}  // namespace utility