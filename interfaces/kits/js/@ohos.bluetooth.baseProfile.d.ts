/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

import type { AsyncCallback, Callback } from './@ohos.base';
import type constant from './@ohos.bluetooth.constant';

/**
 * Provides basic profile methods.
 *
 * @namespace baseProfile
 * @syscap SystemCapability.Communication.Bluetooth.Core
 * @since 10
 */
declare namespace baseProfile {
  /**
   * Indicate the profile connection state.
   *
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  type ProfileConnectionState = constant.ProfileConnectionState;

  /**
   * Enum for connection strategy of the profile
   *
   * @enum { number }
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @systemapi
   * @since 10
   */
  export enum ConnectionStrategy {
    /**
     * The value of connection strategy unsupported.
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @systemapi
     * @since 10
     */
    CONNECTION_STRATEGY_UNSUPPORTED = 0,
    /**
     * The value of connection strategy allowed.
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @systemapi
     * @since 10
     */
    CONNECTION_STRATEGY_ALLOWED = 1,
    /**
     * The value of connection strategy forbidden.
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @systemapi
     * @since 10
     */
    CONNECTION_STRATEGY_FORBIDDEN = 2
  }

  /**
   * Profile state change parameters.
   *
   * @typedef StateChangeParam
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  export interface StateChangeParam {
    /**
     * The address of device
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @since 10
     */
    deviceId: string;

    /**
     * Profile state value
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @since 10
     */
    state: ProfileConnectionState;
  }

  /**
   * Base interface of profile.
   *
   * @typedef BaseProfile
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  export interface BaseProfile {
    /**
     * Set connection strategy of this profile.
     *
     * @permission ohos.permission.ACCESS_BLUETOOTH and ohos.permission.MANAGE_BLUETOOTH
     * @param { string } deviceId - Indicates device ID. For example, "11:22:33:AA:BB:FF".
     * @param { ConnectionStrategy } strategy - the connection strategy of this profile.
     * @returns { Promise<void> } Returns the promise object.
     * @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 202 - Non-system applications are not allowed to use system APIs.
     * @throws { BusinessError } 401 - Invalid parameter.
     * @throws { BusinessError } 801 - Capability not supported.
     * @throws { BusinessError } 2900001 - Service stopped.
     * @throws { BusinessError } 2900003 - Bluetooth switch is off.
     * @throws { BusinessError } 2900004 - Profile is not supported.
     * @throws { BusinessError } 2900099 - Operation failed.
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @systemapi
     * @since 10
     */
    setConnectionStrategy(deviceId: string, strategy: ConnectionStrategy): Promise<void>;

    /**
     * Set connection strategy of this profile.
     *
     * @permission ohos.permission.ACCESS_BLUETOOTH and ohos.permission.MANAGE_BLUETOOTH
     * @param { string } deviceId - Indicates device ID. For example, "11:22:33:AA:BB:FF".
     * @param { ConnectionStrategy } strategy - the connection strategy of this profile.
     * @param { AsyncCallback<void> } callback - the callback of setConnectionStrategy.
     * @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 202 - Non-system applications are not allowed to use system APIs.
     * @throws { BusinessError } 401 - Invalid parameter.
     * @throws { BusinessError } 801 - Capability not supported.
     * @throws { BusinessError } 2900001 - Service stopped.
     * @throws { BusinessError } 2900003 - Bluetooth switch is off.
     * @throws { BusinessError } 2900004 - Profile is not supported.
     * @throws { BusinessError } 2900099 - Operation failed.
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @systemapi
     * @since 10
     */
    setConnectionStrategy(deviceId: string, strategy: ConnectionStrategy, callback: AsyncCallback<void>): void;

    /**
     * Get connection strategy of this profile.
     *
     * @permission ohos.permission.ACCESS_BLUETOOTH and ohos.permission.MANAGE_BLUETOOTH
     * @param { string } deviceId - Indicates device ID. For example, "11:22:33:AA:BB:FF".
     * @param { AsyncCallback<ConnectionStrategy> } callback - the callback of getConnectionStrategy.
     * @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 202 - Non-system applications are not allowed to use system APIs.
     * @throws { BusinessError } 401 - Invalid parameter.
     * @throws { BusinessError } 801 - Capability not supported.
     * @throws { BusinessError } 2900001 - Service stopped.
     * @throws { BusinessError } 2900003 - Bluetooth switch is off.
     * @throws { BusinessError } 2900004 - Profile is not supported.
     * @throws { BusinessError } 2900099 - Operation failed.
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @systemapi
     * @since 10
     */
    getConnectionStrategy(deviceId: string, callback: AsyncCallback<ConnectionStrategy>): void;

    /**
     * Get connection strategy of this profile.
     *
     * @permission ohos.permission.ACCESS_BLUETOOTH and ohos.permission.MANAGE_BLUETOOTH
     * @param { string } deviceId - Indicates device ID. For example, "11:22:33:AA:BB:FF".
     * @returns { Promise<ConnectionStrategy> } Returns the promise object.
     * @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 202 - Non-system applications are not allowed to use system APIs.
     * @throws { BusinessError } 401 - Invalid parameter.
     * @throws { BusinessError } 801 - Capability not supported.
     * @throws { BusinessError } 2900001 - Service stopped.
     * @throws { BusinessError } 2900003 - Bluetooth switch is off.
     * @throws { BusinessError } 2900004 - Profile is not supported.
     * @throws { BusinessError } 2900099 - Operation failed.
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @systemapi
     * @since 10
     */
    getConnectionStrategy(deviceId: string): Promise<ConnectionStrategy>;

    /**
     * Obtains the connected devices list of profile.
     *
     * @permission ohos.permission.ACCESS_BLUETOOTH
     * @returns { Array<string> } Returns the address of connected devices list.
     * @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 801 - Capability not supported.
     * @throws { BusinessError } 2900001 - Service stopped.
     * @throws { BusinessError } 2900003 - Bluetooth switch is off.
     * @throws { BusinessError } 2900004 - Profile is not supported.
     * @throws { BusinessError } 2900099 - Operation failed.
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @since 10
     */
    getConnectedDevices(): Array<string>;

    /**
     * Obtains the profile connection state.
     *
     * @permission ohos.permission.ACCESS_BLUETOOTH
     * @param { string } deviceId - Indicates device ID. For example, "11:22:33:AA:BB:FF".
     * @returns { ProfileConnectionState } Returns the connection state.
     * @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 401 - Invalid parameter.
     * @throws { BusinessError } 801 - Capability not supported.
     * @throws { BusinessError } 2900001 - Service stopped.
     * @throws { BusinessError } 2900003 - Bluetooth switch is off.
     * @throws { BusinessError } 2900004 - Profile is not supported.
     * @throws { BusinessError } 2900099 - Operation failed.
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @since 10
     */
    getConnectionState(deviceId: string): ProfileConnectionState;

    /**
     * Subscribe the event reported when the profile connection state changes .
     *
     * @permission ohos.permission.ACCESS_BLUETOOTH
     * @param { 'connectionStateChange' } type - Type of the profile connection state changes event to listen for.
     * @param { Callback<StateChangeParam> } callback - Callback used to listen for event.
     * @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 401 - Invalid parameter.
     * @throws { BusinessError } 801 - Capability not supported.
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @since 10
     */
    on(type: 'connectionStateChange', callback: Callback<StateChangeParam>): void;

    /**
     * Unsubscribe the event reported when the profile connection state changes .
     *
     * @permission ohos.permission.ACCESS_BLUETOOTH
     * @param { 'connectionStateChange' } type - Type of the profile connection state changes event to listen for.
     * @param { Callback<StateChangeParam> } callback - Callback used to listen for event.
     * @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 401 - Invalid parameter.
     * @throws { BusinessError } 801 - Capability not supported.
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @since 10
     */
    off(type: 'connectionStateChange', callback?: Callback<StateChangeParam>): void;
  }
}

export default baseProfile;