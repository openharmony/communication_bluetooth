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

#ifndef OHOS_BT_ADAPTER_UTILS_H
#define OHOS_BT_ADAPTER_UTILS_H

#include "iosfwd"
#include "ohos_bt_gap.h"

#ifdef __cplusplus
extern "C" {
#endif

namespace OHOS {
namespace Bluetooth {
void ConvertAddr(const unsigned char in[6], std::string &out);
void GetAddrFromString(std::string in, unsigned char out[6]);
int GetGattcResult(int ret);
void GetAddrFromByte(unsigned char in[6], std::string &out);
void GetAclStateName(int transport, int state, std::string &out);
GapAclState ConvertAclState(int transport, int state);
void ConvertDataToHex(const unsigned char *data, unsigned int dataLen, std::string &outStr);
}  // namespace Bluetooth
}  // namespace OHOS
#ifdef __cplusplus
}
#endif
#endif