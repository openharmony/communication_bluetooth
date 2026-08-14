/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_HIECHO_H
#define BLUETOOTH_HIECHO_H

#include <cstdint>

/* Vendor private echo (hiecho) protocol constants. */

/* AT command prefix used to set HW device feature via the HFP AG. */
constexpr const char *SET_HW_DEVICE_FEATURE = "AT+SETDEVICEFEATURE";

/* SHA-256 account hash length in bytes. */
constexpr int HW_ACCOUNT_HASH_LEN = 32;

/* hiecho message type for the account hash report. */
constexpr uint8_t HW_ACCOUNT_HASH = 0x27;

/* Length of the device control request payload. */
constexpr int DEVICE_CTRL_REQ_LEN = 8;

/* Length of the device description request payload. */
constexpr int DEVICE_DESCRIPTION_REQ_LEN = 8;

#endif  // BLUETOOTH_HIECHO_H
