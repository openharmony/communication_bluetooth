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

#include "adapter_device_config_mock.h"

namespace bluetooth {

AdapterDeviceConfigInstance *adcinstanceMock_ = nullptr;

AdapterDeviceConfigInstance *registerNewMockAdapterDeviceConfig(AdapterDeviceConfigInstance *newInstance)
{
    adcinstanceMock_ = newInstance;

    return adcinstanceMock_;
}

IAdapterDeviceConfig *AdapterDeviceConfig::GetInstance()
{
    return adcinstanceMock_->GetInstance();
}

}  // namespace bluetooth
