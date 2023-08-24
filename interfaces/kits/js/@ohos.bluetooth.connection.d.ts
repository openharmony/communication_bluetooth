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

import type { AsyncCallback, Callback } from './@ohos.base';
import type constant from './@ohos.bluetooth.constant';

/**
 * Provides methods to operate or manage Bluetooth.
 *
 * @namespace connection
 * @syscap SystemCapability.Communication.Bluetooth.Core
 * @since 10
 */
declare namespace connection {
  /**
   * Indicate the profile connection state.
   *
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  type ProfileConnectionState = constant.ProfileConnectionState;

  /**
   * Indicate the profile id.
   *
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  type ProfileId = constant.ProfileId;

  /**
   * Indicate the profile uuid.
   *
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @systemapi
   * @since 10
   */
  type ProfileUuids = constant.ProfileUuids;

  /**
   * Indicate the major class of a bluetooth device.
   *
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  type MajorClass = constant.MajorClass;

  /**
   * Indicate the major minor class of a bluetooth device.
   *
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  type MajorMinorClass = constant.MajorMinorClass;

  /**
   * Get the profile connection state of the current device.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH
   * @param { ProfileId } [profileId] - Indicate the profile id. This is an optional parameter.
   *                                  With profileId, returns the current connection state of this profile, {@link ProfileConnectionState}.
   *                                  Without profileId, if any profile is connected, {@link ProfileConnectionState#STATE_CONNECTED} is returned.
   *                                  Otherwise, {@link ProfileConnectionState#STATE_DISCONNECTED} is returned.
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
  function getProfileConnectionState(profileId?: ProfileId): ProfileConnectionState;

  /**
   * Starts pairing with a remote Bluetooth device.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH
   * @param { string } deviceId - Indicates device ID. For example, "11:22:33:AA:BB:FF".
   * @param { AsyncCallback<void> } callback - the callback of pairDevice.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900001 - Service stopped.
   * @throws { BusinessError } 2900003 - Bluetooth switch is off.
   * @throws { BusinessError } 2900099 - Operation failed.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  function pairDevice(deviceId: string, callback: AsyncCallback<void>): void;

  /**
   * Starts pairing with a remote Bluetooth device.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH
   * @param { string } deviceId - Indicates device ID. For example, "11:22:33:AA:BB:FF".
   * @returns { Promise<void> } Returns the promise object.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900001 - Service stopped.
   * @throws { BusinessError } 2900003 - Bluetooth switch is off.
   * @throws { BusinessError } 2900099 - Operation failed.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  function pairDevice(deviceId: string): Promise<void>;

  /**
   * Starts pairing with a credible remote Bluetooth device with transport.
   * This interface does not trigger a dialog box and does not require user authorization.
   * Only specific system application can use this function.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH and ohos.permission.MANAGE_BLUETOOTH
   * @param { string } deviceId - Indicates device ID. For example, "11:22:33:AA:BB:FF".
   * @param { BluetoothTransport } transport - the transport of the remote device to pair.
   * @param { AsyncCallback<void> } callback - the callback of pairCredibleDevice.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Non-system applications are not allowed to use system APIs.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900001 - Service stopped.
   * @throws { BusinessError } 2900003 - Bluetooth switch is off.
   * @throws { BusinessError } 2900099 - Operation failed.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @systemapi
   * @since 10
   */
  function pairCredibleDevice(deviceId: string, transport: BluetoothTransport, callback: AsyncCallback<void>): void;

  /**
   * Starts pairing with a credible remote Bluetooth device with transport.
   * This interface does not trigger a dialog box and does not require user authorization.
   * Only specific system application can use this function.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH and ohos.permission.MANAGE_BLUETOOTH
   * @param { string } deviceId - Indicates device ID. For example, "11:22:33:AA:BB:FF".
   * @param { BluetoothTransport } transport - the transport of the remote device to pair.
   * @returns { Promise<void> } Returns the promise object.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Non-system applications are not allowed to use system APIs.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900001 - Service stopped.
   * @throws { BusinessError } 2900003 - Bluetooth switch is off.
   * @throws { BusinessError } 2900099 - Operation failed.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @systemapi
   * @since 10
   */
  function pairCredibleDevice(deviceId: string, transport: BluetoothTransport): Promise<void>;

  /**
   * Remove a paired remote device.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH
   * @param { string } deviceId - Indicates device ID. For example, "11:22:33:AA:BB:FF".
   * @param { AsyncCallback<void> } callback - the callback of cancelPairedDevice.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Non-system applications are not allowed to use system APIs.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900001 - Service stopped.
   * @throws { BusinessError } 2900003 - Bluetooth switch is off.
   * @throws { BusinessError } 2900099 - Operation failed.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @systemapi
   * @since 10
   */
  function cancelPairedDevice(deviceId: string, callback: AsyncCallback<void>): void;

  /**
   * Remove a paired remote device.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH
   * @param { string } deviceId - Indicates device ID. For example, "11:22:33:AA:BB:FF".
   * @returns { Promise<void> } Returns the promise object.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Non-system applications are not allowed to use system APIs.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900001 - Service stopped.
   * @throws { BusinessError } 2900003 - Bluetooth switch is off.
   * @throws { BusinessError } 2900099 - Operation failed.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @systemapi
   * @since 10
   */
  function cancelPairedDevice(deviceId: string): Promise<void>;

  /**
   * Remove a pairing remote device.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH
   * @param { string } deviceId - Indicates device ID. For example, "11:22:33:AA:BB:FF".
   * @param { AsyncCallback<void> } callback - the callback of cancelPairingDevice.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Non-system applications are not allowed to use system APIs.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900001 - Service stopped.
   * @throws { BusinessError } 2900003 - Bluetooth switch is off.
   * @throws { BusinessError } 2900099 - Operation failed.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @systemapi
   * @since 10
   */
  function cancelPairingDevice(deviceId: string, callback: AsyncCallback<void>): void;

  /**
   * Remove a pairing remote device.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH
   * @param { string } deviceId - Indicates device ID. For example, "11:22:33:AA:BB:FF".
   * @returns { Promise<void> } Returns the promise object.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Non-system applications are not allowed to use system APIs.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900001 - Service stopped.
   * @throws { BusinessError } 2900003 - Bluetooth switch is off.
   * @throws { BusinessError } 2900099 - Operation failed.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @systemapi
   * @since 10
   */
  function cancelPairingDevice(deviceId: string): Promise<void>;

  /**
   * Obtains the name of a peer Bluetooth device.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH
   * @param { string } deviceId - Indicates device ID. For example, "11:22:33:AA:BB:FF".
   * @returns { string } Returns the device name in character string format.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900001 - Service stopped.
   * @throws { BusinessError } 2900003 - Bluetooth switch is off.
   * @throws { BusinessError } 2900099 - Operation failed.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  function getRemoteDeviceName(deviceId: string): string;

  /**
   * Obtains the class of a peer Bluetooth device.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH
   * @param { string } deviceId - Indicates device ID. For example, "11:22:33:AA:BB:FF".
   * @returns { DeviceClass } The class of the remote device.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900001 - Service stopped.
   * @throws { BusinessError } 2900003 - Bluetooth switch is off.
   * @throws { BusinessError } 2900099 - Operation failed.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  function getRemoteDeviceClass(deviceId: string): DeviceClass;

  /**
   * Obtains the Bluetooth local name of a device.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH
   * @returns { string } Returns the name the device.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900001 - Service stopped.
   * @throws { BusinessError } 2900099 - Operation failed.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  function getLocalName(): string;

  /**
   * Obtains the list of Bluetooth devices that have been paired with the current device.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH
   * @returns { Array<string> } Returns a list of paired Bluetooth devices's address.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900001 - Service stopped.
   * @throws { BusinessError } 2900003 - Bluetooth switch is off.
   * @throws { BusinessError } 2900099 - Operation failed.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  function getPairedDevices(): Array<string>;

  /**
   * Sets the confirmation of pairing with a certain device.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH and ohos.permission.MANAGE_BLUETOOTH
   * @param { string } deviceId - Indicates device ID. For example, "11:22:33:AA:BB:FF".
   * @param { boolean } accept - Indicates whether to accept the pairing request, {@code true} indicates accept or {@code false} otherwise.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900001 - Service stopped.
   * @throws { BusinessError } 2900003 - Bluetooth switch is off.
   * @throws { BusinessError } 2900099 - Operation failed.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  function setDevicePairingConfirmation(deviceId: string, accept: boolean): void;

  /**
   * Set the pin during pairing when the pin type is PIN_TYPE_ENTER_PIN_CODE.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH
   * @param { string } deviceId - Indicates device ID. For example, "11:22:33:AA:BB:FF".
   * @param { string } code - The pin code entered by the user.
   * @param { AsyncCallback<void> } callback - the callback of setDevicePinCode.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900001 - Service stopped.
   * @throws { BusinessError } 2900003 - Bluetooth switch is off.
   * @throws { BusinessError } 2900099 - Operation failed.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  function setDevicePinCode(deviceId: string, code: string, callback: AsyncCallback<void>): void;

  /**
   * Set the pin during pairing when the pin type is PIN_TYPE_ENTER_PIN_CODE.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH
   * @param { string } deviceId - Indicates device ID. For example, "11:22:33:AA:BB:FF".
   * @param { string } code - The pin code entered by the user.
   * @returns { Promise<void> } Returns the promise object.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900001 - Service stopped.
   * @throws { BusinessError } 2900003 - Bluetooth switch is off.
   * @throws { BusinessError } 2900099 - Operation failed.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  function setDevicePinCode(deviceId: string, code: string): Promise<void>;

  /**
   * Sets the Bluetooth friendly name of a device.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH
   * @param { string } name - Indicates a valid Bluetooth name.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900001 - Service stopped.
   * @throws { BusinessError } 2900003 - Bluetooth switch is off.
   * @throws { BusinessError } 2900099 - Operation failed.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  function setLocalName(name: string): void;

  /**
   * Sets the Bluetooth scan mode for a device.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH
   * @param { ScanMode } mode - Indicates the Bluetooth scan mode to set.
   * @param { number } duration - Indicates the duration in seconds, in which the host is discoverable.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900001 - Service stopped.
   * @throws { BusinessError } 2900003 - Bluetooth switch is off.
   * @throws { BusinessError } 2900099 - Operation failed.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  function setBluetoothScanMode(mode: ScanMode, duration: number): void;

  /**
   * Obtains the Bluetooth scanning mode of a device.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH
   * @returns { ScanMode } Returns the Bluetooth scanning mode.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900001 - Service stopped.
   * @throws { BusinessError } 2900003 - Bluetooth switch is off.
   * @throws { BusinessError } 2900099 - Operation failed.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  function getBluetoothScanMode(): ScanMode;

  /**
   * Starts scanning Bluetooth devices.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900001 - Service stopped.
   * @throws { BusinessError } 2900003 - Bluetooth switch is off.
   * @throws { BusinessError } 2900099 - Operation failed.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  function startBluetoothDiscovery(): void;

  /**
   * Stops Bluetooth device scanning.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900001 - Service stopped.
   * @throws { BusinessError } 2900003 - Bluetooth switch is off.
   * @throws { BusinessError } 2900099 - Operation failed.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  function stopBluetoothDiscovery(): void;

  /**
   * Obtains the profile UUIDs supported by the local device.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH
   * @param { AsyncCallback<Array<ProfileUuids>> } callback - the callback of getLocalProfileUuids.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Non-system applications are not allowed to use system APIs.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900001 - Service stopped.
   * @throws { BusinessError } 2900003 - Bluetooth switch is off.
   * @throws { BusinessError } 2900099 - Operation failed.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @systemapi
   * @since 10
   */
  function getLocalProfileUuids(callback: AsyncCallback<Array<ProfileUuids>>): void;

  /**
   * Obtains the profile UUIDs supported by the local device.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH
   * @returns { Promise<Array<ProfileUuids>> } Returns the promise object.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Non-system applications are not allowed to use system APIs.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900001 - Service stopped.
   * @throws { BusinessError } 2900003 - Bluetooth switch is off.
   * @throws { BusinessError } 2900099 - Operation failed.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @systemapi
   * @since 10
   */
  function getLocalProfileUuids(): Promise<Array<ProfileUuids>>;

  /**
   * Obtains the profile UUIDs supported by the remote device.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH
   * @param { string } deviceId - Indicates device ID. For example, "11:22:33:AA:BB:FF".
   * @param { AsyncCallback<Array<ProfileUuids>> } callback - the callback of getRemoteProfileUuids.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Non-system applications are not allowed to use system APIs.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900001 - Service stopped.
   * @throws { BusinessError } 2900003 - Bluetooth switch is off.
   * @throws { BusinessError } 2900099 - Operation failed.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @systemapi
   * @since 10
   */
  function getRemoteProfileUuids(deviceId: string, callback: AsyncCallback<Array<ProfileUuids>>): void;

  /**
   * Obtains the profile UUIDs supported by the remote device.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH
   * @param { string } deviceId - Indicates device ID. For example, "11:22:33:AA:BB:FF".
   * @returns { Promise<Array<ProfileUuids>> } Returns the promise object.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Non-system applications are not allowed to use system APIs.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900001 - Service stopped.
   * @throws { BusinessError } 2900003 - Bluetooth switch is off.
   * @throws { BusinessError } 2900099 - Operation failed.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @systemapi
   * @since 10
   */
  function getRemoteProfileUuids(deviceId: string): Promise<Array<ProfileUuids>>;

  /**
   * Subscribe the event reported when a remote Bluetooth device is discovered.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH
   * @param { 'bluetoothDeviceFind' } type - Type of the discovering event to listen for.
   * @param { Callback<Array<string>> } callback - Callback used to listen for the discovering event.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900099 - Operation failed.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  function on(type: 'bluetoothDeviceFind', callback: Callback<Array<string>>): void;

  /**
   * Unsubscribe the event reported when a remote Bluetooth device is discovered.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH
   * @param { 'bluetoothDeviceFind' } type - Type of the discovering event to listen for.
   * @param { Callback<Array<string>> } callback - Callback used to listen for the discovering event.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900099 - Operation failed.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  function off(type: 'bluetoothDeviceFind', callback?: Callback<Array<string>>): void;

  /**
   * Subscribe the event reported when a remote Bluetooth device is bonded.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH
   * @param { 'bondStateChange' } type - Type of the bond state event to listen for.
   * @param { Callback<BondStateParam> } callback - Callback used to listen for the bond state event.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900099 - Operation failed.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  function on(type: 'bondStateChange', callback: Callback<BondStateParam>): void;

  /**
   * Unsubscribe the event reported when a remote Bluetooth device is bonded.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH
   * @param { 'bondStateChange' } type - Type of the bond state event to listen for.
   * @param { Callback<BondStateParam> } callback - Callback used to listen for the bond state event.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900099 - Operation failed.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  function off(type: 'bondStateChange', callback?: Callback<BondStateParam>): void;

  /**
   * Subscribe the event of a pairing request from a remote Bluetooth device.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH
   * @param { 'pinRequired' } type - Type of the pairing request event to listen for.
   * @param { Callback<PinRequiredParam> } callback - Callback used to listen for the pairing request event.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900099 - Operation failed.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  function on(type: 'pinRequired', callback: Callback<PinRequiredParam>): void;

  /**
   * Unsubscribe the event of a pairing request from a remote Bluetooth device.
   *
   * @permission ohos.permission.ACCESS_BLUETOOTH
   * @param { 'pinRequired' } type - Type of the pairing request event to listen for.
   * @param { Callback<PinRequiredParam> } callback - Callback used to listen for the pairing request event.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 401 - Invalid parameter.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900099 - Operation failed.
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  function off(type: 'pinRequired', callback?: Callback<PinRequiredParam>): void;

  /**
   * Describes the class of a bluetooth device.
   *
   * @typedef BondStateParam
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  interface BondStateParam {
    /**
     * Address of a Bluetooth device.
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @since 10
     */
    deviceId: string;
    /**
     * Profile connection state of the device.
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @since 10
     */
    state: BondState;
  }

  /**
   * Describes the bond key param.
   *
   * @typedef PinRequiredParam
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  interface PinRequiredParam {
    /**
     * ID of the device to pair.
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @since 10
     */
    deviceId: string;
    /**
     * Key for the device pairing.
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @since 10
     */
    pinCode: string;
    /**
     * Indicates the pairing type to a peer device.
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @systemapi
     * @since 10
     */
    pinType: PinType;
  }

  /**
   * Describes the class of a bluetooth device.
   *
   * @typedef DeviceClass
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  interface DeviceClass {
    /**
     * Major classes of Bluetooth devices.
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @since 10
     */
    majorClass: MajorClass;
    /**
     * Major and minor classes of Bluetooth devices.
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @since 10
     */
    majorMinorClass: MajorMinorClass;
    /**
     * Class of the device.
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @since 10
     */
    classOfDevice: number;
  }

  /**
   * Enum for the transport of a remote device
   *
   * @enum { number }
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  enum BluetoothTransport {
    /**
     * The value of bluetooth transport BR/EDR.
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @since 10
     */
    TRANSPORT_BR_EDR = 0,
    /**
     * The value of bluetooth transport LE.
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @since 10
     */
    TRANSPORT_LE = 1
  }

  /**
   * The enum of BR scan mode.
   *
   * @enum { number }
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  enum ScanMode {
    /**
     * Indicates the scan mode is none
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @since 10
     */
    SCAN_MODE_NONE = 0,
    /**
     * Indicates the scan mode is connectable
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @since 10
     */
    SCAN_MODE_CONNECTABLE = 1,
    /**
     * Indicates the scan mode is general discoverable
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @since 10
     */
    SCAN_MODE_GENERAL_DISCOVERABLE = 2,
    /**
     * Indicates the scan mode is limited discoverable
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @since 10
     */
    SCAN_MODE_LIMITED_DISCOVERABLE = 3,
    /**
     * Indicates the scan mode is connectable and general discoverable
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @since 10
     */
    SCAN_MODE_CONNECTABLE_GENERAL_DISCOVERABLE = 4,
    /**
     * Indicates the scan mode is connectable and limited discoverable
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @since 10
     */
    SCAN_MODE_CONNECTABLE_LIMITED_DISCOVERABLE = 5
  }

  /**
   * The enum of bond state.
   *
   * @enum { number }
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @since 10
   */
  enum BondState {
    /**
     * Indicate the bond state is invalid
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @since 10
     */
    BOND_STATE_INVALID = 0,
    /**
     * Indicate the bond state is bonding
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @since 10
     */
    BOND_STATE_BONDING = 1,
    /**
     * Indicate the bond state is bonded
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @since 10
     */
    BOND_STATE_BONDED = 2
  }

  /**
   * Enum for the type of pairing to a remote device
   *
   * @enum { number }
   * @syscap SystemCapability.Communication.Bluetooth.Core
   * @systemapi
   * @since 10
   */
  enum PinType {
    /**
     * The user needs to enter the pin code displayed on the peer device.
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @systemapi
     * @since 10
     */
    PIN_TYPE_ENTER_PIN_CODE = 0,
    /**
     * The user needs to enter the passkey displayed on the peer device.
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @systemapi
     * @since 10
     */
    PIN_TYPE_ENTER_PASSKEY = 1,
    /**
     * The user needs to confirm the passkey displayed on the local device.
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @systemapi
     * @since 10
     */
    PIN_TYPE_CONFIRM_PASSKEY = 2,
    /**
     * The user needs to accept or deny the pairing request.
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @systemapi
     * @since 10
     */
    PIN_TYPE_NO_PASSKEY_CONSENT = 3,
    /**
     * The user needs to enter the passkey displayed on the local device on the peer device.
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @systemapi
     * @since 10
     */
    PIN_TYPE_NOTIFY_PASSKEY = 4,
    /**
     * The user needs to enter the pin code displayed on the peer device, used for bluetooth 2.0.
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @systemapi
     * @since 10
     */
    PIN_TYPE_DISPLAY_PIN_CODE = 5,
    /**
     * The user needs to accept or deny the OOB pairing request.
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @systemapi
     * @since 10
     */
    PIN_TYPE_OOB_CONSENT = 6,
    /**
     * The user needs to enter the 16-digit pin code displayed on the peer device.
     *
     * @syscap SystemCapability.Communication.Bluetooth.Core
     * @systemapi
     * @since 10
     */
    PIN_TYPE_PIN_16_DIGITS = 7
  }
}

export default connection;