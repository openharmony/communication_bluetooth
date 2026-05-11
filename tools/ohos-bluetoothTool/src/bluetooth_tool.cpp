/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include "bluetooth_tool.h"

namespace OHOS {
namespace Bluetooth {
namespace Tool {

BluetoothTool::BluetoothTool() : host_(BluetoothHost::GetDefaultHost())
{
}

BluetoothTool::~BluetoothTool()
{
}

int BluetoothTool::EnableBluetooth()
{
    return host_.EnableBle("", false);
}

int BluetoothTool::DisableBluetooth()
{
    return host_.DisableBt("", false);
}

int BluetoothTool::GetBtState()
{
    return host_.GetBtState();
}

bool BluetoothTool::IsBrEnabled()
{
    return host_.IsBrEnabled();
}

bool BluetoothTool::IsBleEnabled()
{
    return host_.IsBleEnabled();
}

}
}
}