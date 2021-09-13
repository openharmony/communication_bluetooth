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

#include "log.h"
#include <map>
#include <list>
#include <string>
#include <string.h>

#include "power_manager.h"

namespace bluetooth {
/// static function
IPowerManager &IPowerManager::GetInstance()
{
    return PowerManager::GetInstance();
}

void IPowerManager::Initialize(utility::Dispatcher &dispatcher)
{
}

void IPowerManager::Uninitialize()
{
}

PowerManager &PowerManager::GetInstance()
{
    static utility::Dispatcher disp;
    static PowerManager powerManager(disp);
    return powerManager;
}

void PowerManager::Initialize(utility::Dispatcher &dispatcher)
{

}

void PowerManager::Uninitialize()
{

}

/// PowerManager class
struct PowerManager::impl {
public:
};

PowerManager::PowerManager(utility::Dispatcher &dispatcher)
{}
PowerManager::~PowerManager()
{}

void PowerManager::Enable()
{
}

void PowerManager::Disable()
{
}

void PowerManager::StatusUpdate(const RequestStatus status, const std::string &profileName,
    const RawAddress &addr) const
{
}

BTPowerMode PowerManager::GetPowerMode(const RawAddress &rawAddr) const
{
    return BTPowerMode::MODE_INVALID;
}
}  // namespace bluetooth