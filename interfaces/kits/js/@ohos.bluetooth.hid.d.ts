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
 * Provides methods to accessing bluetooth HID(Human Interface Device)-related capabilities.
 *
 * @namespace hid
 * @syscap SystemCapability.Communication.Bluetooth.Core
 * @since 10
 */
declare namespace hid {
  /**
   * Base interface of profile.
   *
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  type BaseProfile = baseProfile.BaseProfile;

  /**
   * create the instance of hid profile.
   *
   * @returns { HidHostProfile } Returns the instance of hid profile.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 801 - Capability not supported.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  function createHidHostProfile(): HidHostProfile;

  /**
   * Manager hid host profile.
   *
   * @typedef HidHostProfile
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  interface HidHostProfile extends BaseProfile {
    /**
     * Initiate an HID connection to a remote device.
     *
     * @permission ohos.permission.ACCESS_BLUETOOTH and ohos.permission.MANAGE_BLUETOOTH
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
     * Disconnect the HID connection with the remote device.
     *
     * @permission ohos.permission.ACCESS_BLUETOOTH and ohos.permission.MANAGE_BLUETOOTH
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
  }
}

export default hid;