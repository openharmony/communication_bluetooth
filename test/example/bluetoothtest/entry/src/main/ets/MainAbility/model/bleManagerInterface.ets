/**
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

// @ts-nocheck

/**
 * BLE Test Page Of Bluetooth test
 */
import prompt from '@system.prompt';
import BluetoothModelManager from './BluetoothModelManager'
import bluetoothManager from '@ohos.bluetoothManager';

let gattServerInstance = null;

export function testCreateGattServerDeviceManager(): string {
  try {
    let message = "createGattServerDevice test"
    let btState = bluetoothManager.getState();
    if ( btState == bluetoothManager.BluetoothState.STATE_ON ) {
      message = "createGattServer starts";
      if ( !gattServerInstance ) {
        gattServerInstance = BluetoothModelManager.createGattServer();
        message = "createGattServer: succeeds.";
      } else {
        message = "GattServer existed!";
      }
    } else if ( btState == bluetoothManager.BluetoothState.STATE_OFF ) {
      message = "BT is not enabled!";
    } else {
      message = "switching,please wait!";
    }
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testCreateGattServerDeviceManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testCreateGattServerDeviceManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

let gattClientDeviceInstance = null

export function testCreateGattClientDeviceManager(): string {
  try {
    let message = "test createGattClientDevice"
    let btState = bluetoothManager.getState();
    let peripheralDeviceId = AppStorage.Get( 'peripheralDeviceId' );
    if ( btState == bluetoothManager.BluetoothState.STATE_ON ) {
      message = "createGattClient start"
      if ( !gattClientDeviceInstance ) {
        gattClientDeviceInstance = BluetoothModelManager.createGattClient( peripheralDeviceId );
        message = "createGattClient:succeeds.";
      } else {
        message = "gattClient existed!";
      }
    } else if ( btState == bluetoothManager.BluetoothState.STATE_OFF ) {
      message = "BT is not enabled!";
    } else {
      message = "switching,please wait!";
    }
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testCreateGattClientDeviceManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testCreateGattClientDeviceManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testGetConnectedBLEDevicesManager(): string {
  try {
    let connectedBleDevices = BluetoothModelManager.getConnectedBLEDevices();
    let message = "getConnectedBLEDevices" + JSON.stringify( connectedBleDevices );
    console.log( "getConnectedBLEDevices" + JSON.stringify( connectedBleDevices ) )
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testGetConnectedBLEDevicesManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testGetConnectedBLEDevicesManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testStartBLEScanManager(): string {
  try {
    console.log( "StartBLEScan -------------------------------" )

    let btState = bluetoothManager.getState();
    let message = "test startBLEScan"
    if ( btState == bluetoothManager.BluetoothState.STATE_OFF ) {
      message = "BT is not enabled!";
      return message;
    }
    let txtScanFilter_deviceId = AppStorage.Get( 'txtScanFilterDeviceId' );
    let txtScanFilter_name = AppStorage.Get( 'txtScanFilterName' );
    let txtScanFilter_serviceUuid = AppStorage.Get( 'txtScanFilterServiceUuid' );
    let txtScanOptions_interval = AppStorage.Get( 'txtScanOptions_interval' );
    let txtScanOptions_dutyMode = AppStorage.Get( 'txtScanOptionsDutyMode' );
    let txtScanOptions_matchMode = AppStorage.Get( 'txtScanOptionsMatchMode' );
    let cbxBleScanFilter = AppStorage.Get( 'cbxBleScanFilter' );
    let cbxBleScanOptions = AppStorage.Get( 'cbxBleScanOptions' );
    let curScanFilters = [];
    let bleScanFilter = {
      deviceId : txtScanFilter_deviceId ,
      name : txtScanFilter_name ,
      serviceUuid : txtScanFilter_serviceUuid
    }
    let curScanOptions = {
      interval : parseInt( txtScanOptions_interval ) ,
      dutyMode : parseInt( txtScanOptions_dutyMode ) ,
      matchMode : parseInt( txtScanOptions_matchMode )
    }
    if ( cbxBleScanFilter ) {
      curScanFilters.push( bleScanFilter );
    }

    if ( !cbxBleScanOptions ) {
      if ( cbxBleScanFilter ) {
        BluetoothModelManager.startBLEScan( curScanFilters );
        message = "startBLEScan Filters:" + JSON.stringify( curScanFilters );
      } else {
        BluetoothModelManager.startBLEScan( null );
        message = "startBLEScan(null)";
      }
    } else {
      BluetoothModelManager.startBLEScan( curScanFilters , curScanOptions );
      message = "startBLEScan Filters:" + JSON.stringify( curScanFilters ) + "\n" + "Options:" + JSON.stringify( curScanOptions );
    }
    console.log( "StartBLEScan end-------------------------------" )
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testStartBLEScanManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testStartBLEScanManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testStopBLEScanManager(): string {
  try {
    BluetoothModelManager.stopBLEScan();
    let message = "stopBLEScan OK!";
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testStopBLEScanManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testStopBLEScanManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

let bleDeviceFindManager = false

export function testOnBLEDeviceFindManager(): string {
  try {
    let message = "onBLEDeviceFind test"
    if ( bleDeviceFindManager ) {
      BluetoothModelManager.unsubscribeBLEDeviceFind();
      message = '关闭BLE查找状态变化'
      prompt.showToast( { message : message } )
    } else {
      BluetoothModelManager.subscribeBLEDeviceFind( ( deviceItems: Array<bluetoothManager.ScanResult> ) => {
        console.log( "BLE Test:" + 'available ble devices changed.' );
        let message = ""
        let availBleDeviceIds = []
        deviceItems?.forEach( deviceItem => {
          let newDevice = deviceItem.deviceId;
          message += "mac:" + newDevice;
          console.log( "BLE Test:" + 'available bluetooth new device : ' + newDevice );
          availBleDeviceIds.push( newDevice );
        } )
        AppStorage.SetOrCreate( 'bluetoothAvailableDevices' , availBleDeviceIds );
        prompt.showToast( { message : message } )
      } )
      message = "打开BLE查找状态变化"
      prompt.showToast( { message : message } )
    }
    bleDeviceFindManager = !bleDeviceFindManager
    return message
  } catch( e ) {
    console.log( `Failed to testOnBLEDeviceFindManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testOnBLEDeviceFindManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}
