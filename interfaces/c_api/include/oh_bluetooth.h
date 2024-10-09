/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

/**
 * @addtogroup Bluetooth
 * @{
 *
 * @brief Provide functions for querying the status of bluetooth switch.
 * @since 13
 */
/**
 * @file oh_bluetooth.h
 * @kit ConnectivityKit
 * @brief Define interfaces for querying bluetooth switch status.
 * @library libbluetooth.so
 * @syscap SystemCapability.Communication.Bluetooth.Core
 * @since 13
 */

#ifndef OH_BLUETOOTH_H
#define OH_BLUETOOTH_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumeration state of bluetooth switch.
 *
 * @since 13
 */
typedef enum Bluetooth_SwitchState {
    /** Indicates the local bluetooth is off. */
    BLUETOOTH_STATE_OFF = 0,
    /** Indicates the local bluetooth is turning on. */
    BLUETOOTH_STATE_TURNING_ON = 1,
    /** Indicates the local bluetooth is on, and ready for use. */
    BLUETOOTH_STATE_ON = 2,
    /** Indicates the local bluetooth is turning off. */
    BLUETOOTH_STATE_TURNING_OFF = 3,
    /** Indicates the local bluetooth is turning LE mode on. */
    BLUETOOTH_STATE_BLE_TURNING_ON = 4,
    /** Indicates the local bluetooth is in LE only mode. */
    BLUETOOTH_STATE_BLE_ON = 5,
    /** Indicates the local bluetooth is turning off LE only mode. */
    BLUETOOTH_STATE_BLE_TURNING_OFF = 6
} Bluetooth_SwitchState;

/**
 * @brief Enumeration the bluetooth result codes.
 *
 * @since 13
 */
typedef enum Bluetooth_ResultCode {
    /**
     * @error The operation is successful.
     */
    BLUETOOTH_SUCCESS = 0,
    /**
     * @error Parameter error. Possible reasons: 1. The input parameter is a null pointer;
     * 2. Parameter values exceed the defined range.
     */
    BLUETOOTH_INVALID_PARAM = 401,
} Bluetooth_ResultCode;

/**
 * @brief Get the bluetooth switch state.
 *
 * @param state - It is a pointer used to receive bluetooth switch status values.
 * The caller needs to pass in a non empty boolean pointer, otherwise an error will be returned.
 * For a detailed definition, please refer to {@link Bluetooth_SwitchState}.
 * @return Bluetooth functions result code.
 *     For a detailed definition, please refer to {@link Bluetooth_ResultCode}.
 *     {@link BLUETOOTH_SUCCESS} Successfully obtained the bluetooth switch status.
 *     {@link BLUETOOTH_INVALID_PARAM} The input parameter enabled is a null pointer.
 * @since 13
 */
Bluetooth_ResultCode OH_Bluetooth_GetBluetoothSwitchState(Bluetooth_SwitchState *state);
#ifdef __cplusplus
}
#endif
/** @} */
#endif // OH_BLUETOOTH_H