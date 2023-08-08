/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

import type baseProfile from './@ohos.bluetooth.baseProfile';

/**
 * Provides methods to accessing bluetooth audio related capabilities.
 *
 * @namespace a2dp
 * @syscap SystemCapability.Communication.Bluetooth.Core
 * @since 10
 */
declare namespace a2dp {
  /**
   * Base interface of profile.
   *
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  type BaseProfile = baseProfile.BaseProfile;

  /**
   * create the instance of a2dp profile.
   *
   * @returns { A2dpSourceProfile } Returns the instance of profile.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 801 - Capability not supported.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  function createA2dpSrcProfile(): A2dpSourceProfile;

  /**
   * Manager a2dp source profile.
   *
   * @typedef A2dpSourceProfile
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  interface A2dpSourceProfile extends BaseProfile {
    /**
     * Initiate an A2DP connection to a remote device.
     *
     * @permission ohos.permission.ACCESS_BLUETOOTH
     * @param { string } deviceId - Indicates device ID. For example, "11:22:33:AA:BB:FF".
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
    connect(deviceId: string): void;

    /**
     * Disconnect the A2DP connection with the remote device.
     *
     * @permission ohos.permission.ACCESS_BLUETOOTH
     * @param { string } deviceId - Indicates device ID. For example, "11:22:33:AA:BB:FF".
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
    disconnect(deviceId: string): void;

    /**
     * Obtains the playing state of device.
     *
     * @permission ohos.permission.ACCESS_BLUETOOTH
     * @param { string } deviceId - Indicates device ID. For example, "11:22:33:AA:BB:FF".
     * @returns { PlayingState } Returns the playing state.
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
    getPlayingState(deviceId: string): PlayingState;
  }

  /**
   * The enum of a2dp playing state.
   *
   * @enum { number }
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  enum PlayingState {
    /**
     * Not playing.
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @since 10
     */
    STATE_NOT_PLAYING,
    /**
     * Playing.
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @since 10
     */
    STATE_PLAYING
  }
}

export default a2dp;