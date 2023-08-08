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
 * Provides methods to accessing bluetooth PAN(Personal Area Networking Profile)-related capabilities.
 *
 * @namespace pan
 * @syscap SystemCapability.Communication.Bluetooth.Core
 * @since 10
 */
declare namespace pan {
  /**
   * Base interface of profile.
   *
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  type BaseProfile = baseProfile.BaseProfile;

  /**
   * create the instance of pan profile.
   *
   * @returns { PanProfile } Returns the instance of pan profile.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 801 - Capability not supported.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  function createPanProfile(): PanProfile;

  /**
   * Manager pan host profile.
   *
   * @typedef PanProfile
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  interface PanProfile extends BaseProfile {
    /**
     * Disconnect the PAN connection with the remote device.
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
     * Enable bluetooth tethering.
     *
     * @permission ohos.permission.ACCESS_BLUETOOTH and ohos.permission.MANAGE_BLUETOOTH
     * @param { boolean } enable - Specifies whether to enable tethering.
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
    setTethering(enable: boolean): void;

    /**
     * Obtains the tethering enable or disable.
     *
     * @permission ohos.permission.ACCESS_BLUETOOTH
     * @returns { boolean } Returns the value {@code true} is tethering is on, returns {@code false} otherwise.
     * @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 202 - Non-system applications are not allowed to use system APIs.
     * @throws { BusinessError } 801 - Capability not supported.
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @systemapi
     * @since 10
     */
    isTetheringOn(): boolean;
  }
}

export default pan;