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
#ifndef LOG_TAG
#define LOG_TAG "bt_c_adapter_adapter_utils"
#endif

#include "ohos_bt_adapter_utils.h"
#include "__config"
#include "bluetooth_def.h"
#include "bluetooth_log.h"
#include "ohos_bt_def.h"
#include "securec.h"
#include <cstdlib>
#include "string"

#ifdef __cplusplus
extern "C" {
#endif

using namespace std;

namespace OHOS {
namespace Bluetooth {
constexpr int HEADSET_SUPPORT_VLINK_PRODUCT_ID_MIN = 0x3500;
constexpr int HEADSET_SUPPORT_VLINK_PRODUCT_ID_MAX = 0x48ff;
constexpr int GLASSES_SUPPORT_VLINK_PRODUCT_ID_MIN = 0x1700;
constexpr int GLASSES_SUPPORT_VLINK_PRODUCT_ID_MAX = 0x18ff;

void ConvertAddr(const unsigned char in[6], std::string &out)
{
    char temp[18] = {0}; // convert addr len.
    int ret = sprintf_s(temp, sizeof(temp), "%02X:%02X:%02X:%02X:%02X:%02X",
        in[0], in[1], in[2], in[3], in[4], in[5]);
    if (ret == -1) {
        HILOGE("ConvertAddr sprintf_s return error, ret -1");
    }
    out = string(temp);
}

void GetAddrFromString(std::string in, unsigned char out[6])
{
    int j = 0;
    for (unsigned int i = 0; i < in.length(); i++) {
        if (in.at(i) != ':') {
            out[j] = strtoul(in.substr(i, 2).c_str(), 0, 16); // 2，16 作为截取字符的开始位置或截取长度
            i += 2; // for循环中每轮i值增加2
            j++;
        }
    }
}

int GetGattcResult(int ret)
{
    return (ret == OHOS_BT_STATUS_SUCCESS) ? OHOS_BT_STATUS_SUCCESS : OHOS_BT_STATUS_FAIL;
}

void GetAddrFromByte(unsigned char in[6], std::string &out)
{
    char temp[18] = {0};
    (void)sprintf_s(temp, sizeof(temp), "%02X:%02X:%02X:%02X:%02X:%02X",
        in[0], in[1], in[2], in[3], in[4], in[5]); // 0, 1, 2, 3, 4, 5, 指MAC地址的6个数据段
    out = string(temp);
}

void GetAclStateName(int transport, int state, std::string &out)
{
    switch (transport) {
        case ADAPTER_BREDR:
            if (state == ACL_CONNECTION_STATE_CONNECTED) {
                out = "OHOS_GAP_ACL_STATE_CONNECTED(0)";
            } else if (state == ACL_CONNECTION_STATE_DISCONNECTED) {
                out = "OHOS_GAP_ACL_STATE_DISCONNECTED(1)";
            } else {
                out = "Unknown state";
            }
            break;
        case ADAPTER_BLE:
            if (state == ACL_CONNECTION_STATE_CONNECTED) {
                out = "OHOS_GAP_ACL_STATE_LE_CONNECTED(2)";
            } else if (state == ACL_CONNECTION_STATE_DISCONNECTED) {
                out = "OHOS_GAP_ACL_STATE_LE_DISCONNECTED(3)";
            } else {
                out = "Unknown state";
            }
            break;
        default:
            out = "Unknown transport";
            break;
    }
}

GapAclState ConvertAclState(int transport, int state)
{
    if (transport == ADAPTER_BREDR && state == ACL_CONNECTION_STATE_CONNECTED) {
        return OHOS_GAP_ACL_STATE_CONNECTED;
    } else if (transport == ADAPTER_BREDR && state == ACL_CONNECTION_STATE_DISCONNECTED) {
        return OHOS_GAP_ACL_STATE_DISCONNECTED;
    } else if (transport == ADAPTER_BLE && state == ACL_CONNECTION_STATE_CONNECTED) {
        return OHOS_GAP_ACL_STATE_LE_CONNECTED;
    } else if (transport == ADAPTER_BLE && state == ACL_CONNECTION_STATE_DISCONNECTED) {
        return OHOS_GAP_ACL_STATE_LE_DISCONNECTED;
    }

    HILOGE("invalid transport: %{public}d, state: %{public}d", transport, state);
    return OHOS_GAP_ACL_STATE_INVALID;
}

void ConvertDataToHex(const unsigned char *data, unsigned int dataLen, std::string &outStr)
{
    const std::string hex = "0123456789ABCDEF";
    const uint8_t sizeFour = 4;
    for (size_t i = 0; i < dataLen; ++i) {
        uint8_t n = data[i];
        outStr.push_back(hex[n >> sizeFour]);
        outStr.push_back(hex[n & 0xF]);
    }
}

void ConvertBtTransport(const int ohosTransport, int &innerTransport)
{
    if (ohosTransport == OHOS_BT_TRANSPORT_BR_EDR) {
        innerTransport = BT_TRANSPORT_BREDR;
    } else if (ohosTransport == OHOS_BT_TRANSPORT_LE) {
        innerTransport = BT_TRANSPORT_BLE;
    } else {
        innerTransport = BT_TRANSPORT_NONE;
    }
}

bool IsSupportVlinkFeature(int productId)
{
    return
        ((productId >= HEADSET_SUPPORT_VLINK_PRODUCT_ID_MIN) && (productId <= HEADSET_SUPPORT_VLINK_PRODUCT_ID_MAX)) ||
        ((productId >= GLASSES_SUPPORT_VLINK_PRODUCT_ID_MIN) && (productId <= GLASSES_SUPPORT_VLINK_PRODUCT_ID_MAX));
}
}  // namespace Bluetooth
}  // namespace OHOS
#ifdef __cplusplus
}
#endif
