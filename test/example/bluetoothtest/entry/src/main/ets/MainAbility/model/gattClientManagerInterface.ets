/**
 * Copyright (c) 2022 Shenzhen Kaihong Digital Industry Development Co., Ltd.
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
 * GattClient Test Of Bluetooth test
 */

import prompt from '@system.prompt';
import bluetoothManager from '@ohos.bluetoothManager';
import BluetoothModelManager from './BluetoothModelManager'

let gattClientInstance = null

export function testCreateGattClientManager(): String {
  try {
    let message = "createGattClient test "
    let peripheralDeviceId = AppStorage.Get( 'peripheralDeviceId' );
    let btState = bluetoothManager.getState();
    if ( btState === bluetoothManager.BluetoothState.STATE_ON ) {
      if ( gattClientInstance ) {
        message = "GattClient exists!";
        prompt.showToast( { message : message } )
        return message;
      }
      //注册回调
      bluetoothManager.BLE.on( "BLEDeviceFind" , ( scanResult ) => {
        scanResult.forEach( device => {
          var devId = device.deviceId; //6c:96:d7:3d:87:6f
          var rssi = device.rssi;
          var data = device.data;
          prompt.showToast( { message : "findDevice:" + devId } )
          if ( devId == peripheralDeviceId ) {
            bluetoothManager.BLE.stopBLEScan();
            // create ble client instance
            if ( !gattClientInstance ) {
              gattClientInstance = BluetoothModelManager.createGattClient( devId );
              message = "createGattClient succeeds ";
              console.log( "createGattClient succeeds----------" )
            }
          }
        } )
      } )
      if ( peripheralDeviceId && peripheralDeviceId.length > 15 ) {
        gattClientInstance = BluetoothModelManager.createGattClient( peripheralDeviceId.toUpperCase() );
        if ( gattClientInstance ) {
          message = "createGattClient: " + peripheralDeviceId;
          console.log( "createGattClient peripheralDeviceId succeeds----------" )
        } else {
          message = "createGattClient:fail ";
        }
      } else {
        message = "peripheralDeviceId invalid! ";
      }
      prompt.showToast( { message : message } )
    } else if ( btState === bluetoothManager.BluetoothState.STATE_OFF ) {
      message = "BT is not enabled!";
      prompt.showToast( { message : message } )
    } else {
      message = "switching,please wait"
      prompt.showToast( { message : message } )
    }
    return message
  } catch( e ) {
    console.log( `Failed to testCreateGattClientManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testCreateGattClientManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

let bleConnectionState = 0;

export function testConnectManager(): String {
  try {
    let message = "connect test";
    let btState = bluetoothManager.getState();
    if ( btState === bluetoothManager.BluetoothState.STATE_ON ) {
      if ( gattClientInstance ) {
        if ( bleConnectionState == 0 ) {
          console.log( "GattClient.connect: start----------------" )
          let ret = gattClientInstance.connect();
          message = "GattClient.connect: " + ret;
          console.log( "GattClient.connect: end----------------" + ret )
          bleConnectionState = 2
        } else {
          message = "this device is not disconnected!";
        }
      } else {
        message = "NO_GATT_CLIENT_OBJECT";
      }
    } else if ( btState === bluetoothManager.BluetoothState.STATE_OFF ) {
      message = "BT is not enabled!";
    } else {
      message = "switching,please wait!";
    }
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testConnectManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testConnectManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testDisconnectManager(): String {
  try {
    let message = "test disconnect";
    let btState = bluetoothManager.getState();
    if ( btState == bluetoothManager.BluetoothState.STATE_ON ) {
      if ( !gattClientInstance ) {
        message = "NO_GATT_CLIENT_OBJECT";
        prompt.showToast( { message : message } )
        return message;
      }
      if ( bleConnectionState == 2 ) {
        let ret = gattClientInstance.disconnect();
        message = "GattClient.disconnect: " + ret;
      } else {
        message = " BLE is not connected and no need to disconnect it ";
      }
    } else if ( btState == bluetoothManager.BluetoothState.STATE_OFF ) {
      message = "BT is not enabled!";
    } else {
      message = "switching,please wait!";
    }
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testDisconnectManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testDisconnectManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

let BLEConnectionStateChange = false

export function testOnBLEConnectionStateChangeManager(): String {
  try {
    let message = "onBLEConnectionStateChange test";
    let btStatus = bluetoothManager.getState();
    if ( btStatus === bluetoothManager.BluetoothState.STATE_OFF ) {
      message = "BT is not enabled!";
      prompt.showToast( { message : message } )
      return message;
    }
    if ( !gattClientInstance ) {
      message = "NO_GATT_CLIENT_OBJECT";
      prompt.showToast( { message : message } )
      return message;
    }
    message = " HAVE_GATT_CLIENT_OBJECT "
    if ( BLEConnectionStateChange ) {
      gattClientInstance.off( 'BLEConnectionStateChange' , () => {
        prompt.showToast( { message : "off:BLEConnectionStateChange" } )
      } )
      message = "关闭BLE连接状态变化"
    } else {
      let bleConnectionStateInfo = "xxxx"
      gattClientInstance.on( 'BLEConnectionStateChange' , ( bleConnectionState ) => {
        switch ( bleConnectionState.state ) {
          case bluetoothManager.ProfileConnectionState.STATE_DISCONNECTED:
            bleConnectionStateInfo = "DISCONNECTED";
            break;
          case bluetoothManager.ProfileConnectionState.STATE_CONNECTING:
            bleConnectionStateInfo = "CONNECTING";
            break;
          case bluetoothManager.ProfileConnectionState.STATE_CONNECTED:
            bleConnectionStateInfo = "STATE_CONNECTED";
            break;
          case bluetoothManager.ProfileConnectionState.STATE_DISCONNECTING:
            bleConnectionStateInfo = "STATE_DISCONNECTING";
            break;
          default:
            bleConnectionStateInfo = "UNDEFINED!";
            break;
        }
        console.log( "gattClientInstance.onBLEConnectionStateChange is ------- " + JSON.stringify( bleConnectionState ) )
        AppStorage.SetOrCreate( 'bleConnectionStateInfo' , bleConnectionStateInfo );
        prompt.showToast( { message : "状态:" + bleConnectionStateInfo } )
      } )
      message = "打开BLE连接状态变化"
    }
    BLEConnectionStateChange = !BLEConnectionStateChange
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testOnBLEConnectionStateChangeManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testOnBLEConnectionStateChangeManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testGattClientCloseManager(): String {
  try {
    let message = "gattClientClose test"
    let btState = bluetoothManager.getState();
    if ( btState == bluetoothManager.BluetoothState.STATE_ON ) {
      if ( !gattClientInstance ) {
        message = "NO_GATT_CLIENT_OBJECT";
        prompt.showToast( { message : message } )
        return message;
      }
      let ret = gattClientInstance.close();
      if ( ret ) {
        gattClientInstance = null;
      }
      message = "GattClient.close:" + ret;
    } else if ( btState == bluetoothManager.BluetoothState.STATE_OFF ) {
      message = "BT is not enabled!";
    } else {
      message = "switching,please wait!";
    }
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testGattClientCloseManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testGattClientCloseManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testGetServicesCallbackManager(): String {
  try {
    let message = "getServicesCallback test"
    let btState = bluetoothManager.getState();
    if ( btState == bluetoothManager.BluetoothState.STATE_ON ) {
      if ( !gattClientInstance ) {
        message = "NO_GATT_CLIENT_OBJECT";
        prompt.showToast( { message : message } )
        return message;
      }
      console.log( "connectState " )
      console.log( "getServices(call back) start-----------------" )
      gattClientInstance.getServices( ( err , gattServices ) => {
        if ( err.code != 0 ) {
          message = "getServices(call back) error code:" + err.code + ",services :" + gattServices;
          console.log( "getServices(call back) error code:-----------------" + err.code + ",services :" + gattServices )
          prompt.showToast( { message : message } )
          //        return message;
        } else {
          message = "getServices(Callback):invoke succeeds! " + "\n"
          message += "bluetoothManager services size is " + gattServices.length + '\n';
          for ( let i = 0 ; i < gattServices.length ; i ++ ) {
            message += 'serviceUuid is ' + gattServices[i].serviceUuid + '\n';
          }
          prompt.showToast( { message : message } )
          AppStorage.SetOrCreate( "getServices" , message )
          console.log( "getServices(Callback)----------" + message )
        }
      } )
      console.log( "getServices(call back) end-----------------" )
    } else if ( btState == bluetoothManager.BluetoothState.STATE_OFF ) {
      message = "BT is not enabled!";
    } else {
      message = "switching,please wait!";
    }
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testGetServicesCallbackManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testGetServicesCallbackManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testGetServicesPromiseManager(): String {
  try {
    let message = "getServicesPromise test"
    if ( !gattClientInstance ) {
      message = "NO_GATT_CLIENT_OBJECT";
      prompt.showToast( { message : message } )
      return message;
    }
    message = "getServices promise:starts";
    let btState = bluetoothManager.getState();
    if ( btState == bluetoothManager.BluetoothState.STATE_ON ) {
      console.log( "gattClientInstance.getServices,promise start----------" )
      gattClientInstance.getServices().then( gattServices => {
        console.log( "getServices(): Promise<Array<GattService>> start --------------" )
        message = "getServicesPromise successfully:\n"
        message = "bluetoothManager services size is " + gattServices.length + "\n";
        console.log( "bluetoothManager services size is " + gattServices.length )
        for ( let i = 0 ; i < gattServices.length ; i ++ ) {
          message += 'serviceUuid is ' + gattServices[i].serviceUuid + '\n';
        }
        console.log( "getServices(): Promise<Array<GattService>> end --------------" )
        prompt.showToast( { message : message } )
      } ).catch( err => {
        console.log( "testGetServicesPromiseManager err:" + err )
      } )
      console.log( "gattClientInstance.getServices,promise end----------" )
      message = "getServices(promise):invoke succeeds!";
    } else if ( btState == bluetoothManager.BluetoothState.STATE_OFF ) {
      message = "BT is not enabled!";
    } else {
      message = "switching,please wait!";
    }
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testGetServicesPromiseManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testGetServicesPromiseManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testReadCharacteristicValueCallbackManager(): String {
  try {
    let serviceUUID = AppStorage.Get( 'serviceUUID' );
    let characteristicUUID = AppStorage.Get( 'characteristicUUID' );
    let descriptorUUID = AppStorage.Get( 'descriptorUUID' );
    let descriptorValue = AppStorage.Get( 'descriptorValue' );
    let message = "readCharacteristicValueCallback test"
    let btState = bluetoothManager.getState();
    if ( btState === bluetoothManager.BluetoothState.STATE_OFF ) {
      message = "BT is not enabled!";
      prompt.showToast( { message : message } )
      return message;
    }
    if ( !gattClientInstance ) {
      message = "NO_GATT_CLIENT_OBJECT";
      prompt.showToast( { message : message } )
      return message;
    }
    let descriptors = [];
    let bufferCCC = new ArrayBuffer(8);
    let cccV = new Uint8Array(bufferCCC);
    cccV[0] = 66;
    let descriptor = {
      serviceUuid : serviceUUID ,
      characteristicUuid : characteristicUUID ,
      descriptorUuid : descriptorUUID ,
      descriptorValue : string2ArrayBuffer( descriptorValue )
    }
    descriptors.push( descriptor );
    let bleCharacteristicDataIn = {
      serviceUuid : serviceUUID ,
      characteristicUuid : characteristicUUID ,
      characteristicValue : bufferCCC ,
      descriptors : descriptors
    };
    console.log( "gattClientInstance.readCharacteristicValue ,callback start--------------" )
    gattClientInstance.readCharacteristicValue( bleCharacteristicDataIn , ( err , bleCharacteristicDataOut ) => {
      if ( err.code != 0 ) {
        message = "readCharacteristicValue error code:" + err.code + ",id:" + serviceUUID;
        console.log( message )
        prompt.showToast( { message : message } )
        return message;
      }
      message = "callback success!:" + "\n";
      message += 'characteristic uuid:' + bleCharacteristicDataOut.characteristicUuid + "\n";
      console.log( message )
      let value = new Uint8Array(bleCharacteristicDataOut.characteristicValue);
      for ( let i = 0 ; i < bleCharacteristicDataOut.characteristicValue.byteLength ; i ++ ) {
        message += 'characteristic value: ' + value[i] + "\n";
      }
      AppStorage.SetOrCreate( "read_CharacteristicValue" , message )
      prompt.showToast( { message : message } )
    } );
    console.log( "gattClientInstance.readCharacteristicValue ,callback end--------------" )
    message = 'readCharacteristicValue(callback):invoke succeeds!';
    console.log( message )
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testReadCharacteristicValueCallbackManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testReadCharacteristicValueCallbackManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testReadCharacteristicValuePromiseManager(): String {
  try {
    let serviceUUID = AppStorage.Get( 'serviceUUID' );
    let characteristicUUID = AppStorage.Get( 'characteristicUUID' );
    let descriptorUUID = AppStorage.Get( 'descriptorUUID' );
    let descriptorValue = AppStorage.Get( 'descriptorValue' );
    let message = "readCharacteristicValuePromise test"
    let btState = bluetoothManager.getState();
    if ( btState === bluetoothManager.BluetoothState.STATE_OFF ) {
      message = "BT is not enabled!";
      prompt.showToast( { message : message } )
      return message;
    }
    if ( !gattClientInstance ) {
      message = "NO_GATT_CLIENT_OBJECT";
      prompt.showToast( { message : message } )
      return message;
    }
    let descriptors = [];
    let bufferCCC = new ArrayBuffer(8);
    let cccV = new Uint8Array(bufferCCC);
    cccV[0] = 66;
    let descriptor = {
      serviceUuid : serviceUUID ,
      characteristicUuid : characteristicUUID ,
      descriptorUuid : descriptorUUID ,
      descriptorValue : string2ArrayBuffer( descriptorValue )
    }
    descriptors.push( descriptor );
    let bleCharacteristicDataIn = {
      serviceUuid : serviceUUID ,
      characteristicUuid : characteristicUUID ,
      characteristicValue : bufferCCC ,
      descriptors : descriptors
    };
    console.log( "gattClientInstance.readCharacteristicValue ,promise start--------------" )
    gattClientInstance.readCharacteristicValue( bleCharacteristicDataIn ).then( ( bleCharacteristicDataOut ) => {
      message = "promise success!:\n"
      message += 'characteristic uuid:' + bleCharacteristicDataOut.characteristicUuid + "\n";
      console.log( message )
      let value = new Uint8Array(bleCharacteristicDataOut.characteristicValue);
      for ( let i = 0 ; i < bleCharacteristicDataOut.characteristicValue.byteLength ; i ++ ) {
        message += 'characteristic value: ' + value[i] + "\n";
      }
      prompt.showToast( { message : message } )
    } ).catch( err => {
      console.log( "testReadCharacteristicValuePromiseManager err:" + err )
    } );
    console.log( "gattClientInstance.readCharacteristicValue ,promise end--------------" )
    message = 'readCharacteristicValue(promise):invoke succeeds!';
    console.log( message )
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testReadCharacteristicValuePromiseManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testReadCharacteristicValuePromiseManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testReadDescriptorValueCallbackManager(): String {
  try {
    let serviceUUID = AppStorage.Get( 'serviceUUID' );
    let characteristicUUID = AppStorage.Get( 'characteristicUUID' );
    let descriptorUUID = AppStorage.Get( 'descriptorUUID' );
    let descriptorValue = AppStorage.Get( 'descriptorValue' );
    let message = "readDescriptorValueCallback test"
    let btState = bluetoothManager.getState();
    if ( btState === bluetoothManager.BluetoothState.STATE_OFF ) {
      message = "BT is not enabled!";
      prompt.showToast( { message : message } )
      return message;
    }
    if ( !gattClientInstance ) {
      message = "NO_GATT_CLIENT_OBJECT";
      prompt.showToast( { message : message } )
      return message;
    }
    let bufferDesc = new ArrayBuffer(8);
    let descV = new Uint8Array(bufferDesc);
    descV[0] = 88;
    let descriptorIn = {
      serviceUuid : serviceUUID ,
      characteristicUuid : characteristicUUID ,
      descriptorUuid : descriptorUUID ,
      //    descriptorValue: bufferDesc
      descriptorValue : string2ArrayBuffer( descriptorValue )
    };
    console.log( "gattClientInstance.readDescriptorValue,callback start--------------" )
    gattClientInstance.readDescriptorValue( descriptorIn , ( err , descriptorOut ) => {
      let message = ""
      if ( err.code != 0 ) {
        message = "readDescriptorValue error:" + err.code.toString()
        console.log( message )
        prompt.showToast( { message : message } )
        return message;
      }
      message = "\n" + "callback:descriptor uuid=" + descriptorOut.descriptorUuid + "\n";
      let value = new Uint8Array(descriptorOut.descriptorValue);
      for ( let i = 0 ; i < descriptorOut.descriptorValue.byteLength ; i ++ ) {
        message += 'descriptor value: ' + value[i] + "\n";
      }
      AppStorage.SetOrCreate( "read_DescriptorValue" , message )
      prompt.showToast( { message : message } )
    } );
    console.log( "gattClientInstance.readDescriptorValue ,callback end--------------" )
    message = 'readDescriptorValue(callback):invoke succeeds!';
    console.log( message )
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testReadDescriptorValueCallbackManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testReadDescriptorValueCallbackManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testReadDescriptorValuePromiseManager(): String {
  try {
    let serviceUUID = AppStorage.Get( 'serviceUUID' );
    let characteristicUUID = AppStorage.Get( 'characteristicUUID' );
    let descriptorUUID = AppStorage.Get( 'descriptorUUID' );
    let descriptorValue = AppStorage.Get( 'descriptorValue' );
    let message = "readDescriptorValuePromise test "
    let btState = bluetoothManager.getState();
    if ( btState === bluetoothManager.BluetoothState.STATE_OFF ) {
      message = "BT is not enabled!";
      prompt.showToast( { message : message } )
      return message;
    }
    if ( !gattClientInstance ) {
      message = "NO_GATT_CLIENT_OBJECT";
      prompt.showToast( { message : message } )
      return message;
    }
    let bufferDesc = new ArrayBuffer(8);
    let descV = new Uint8Array(bufferDesc);
    descV[0] = 99;
    let descriptorIn = {
      serviceUuid : serviceUUID ,
      characteristicUuid : characteristicUUID ,
      descriptorUuid : descriptorUUID ,
      //    descriptorValue: bufferDesc
      descriptorValue : string2ArrayBuffer( descriptorValue )
    };
    console.log( "gattClientInstance.readDescriptorValue,promise start--------------" )
    gattClientInstance.readDescriptorValue( descriptorIn ).then( ( descriptorOut ) => {
      message = "\n" + 'promise:descriptor uuid=' + descriptorOut.descriptorUuid + "\n";
      let value = new Uint8Array(descriptorOut.descriptorValue);
      for ( let i = 0 ; i < descriptorOut.descriptorValue.byteLength ; i ++ ) {
        message += 'descriptor value: ' + value[i] + "\n";
      }
      AppStorage.SetOrCreate( "read_DescriptorValue" , message )
      prompt.showToast( { message : message } )
    } ).catch( err => {
      console.log( "testReadDescriptorValuePromiseManager err:" + err )
    } );
    console.log( "gattClientInstance.readDescriptorValue,promise end--------------" )
    message = 'readDescriptorValue(promise):invoke succeeds!';
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testReadDescriptorValuePromiseManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testReadDescriptorValuePromiseManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testWriteCharacteristicValueManager(): String {
  try {
    let serviceUUID = AppStorage.Get( 'serviceUUID' );
    let characteristicUUID = AppStorage.Get( 'characteristicUUID' );
    let characteristicValue = AppStorage.Get( 'characteristicValue' );
    let message = "writeCharacteristicValue test"
    let btState = bluetoothManager.getState();
    if ( btState === bluetoothManager.BluetoothState.STATE_OFF ) {
      message = "BT is not enabled!";
      prompt.showToast( { message : message } )
      return message;
    }
    if ( !gattClientInstance ) {
      message = "NO_GATT_CLIENT_OBJECT";
      prompt.showToast( { message : message } )
      return message;
    }
    let bufferCCC = string2ArrayBuffer( characteristicValue );
    let characteristic = {
      serviceUuid : serviceUUID ,
      characteristicUuid : characteristicUUID ,
      characteristicValue : bufferCCC
    };
    if ( gattClientInstance.writeCharacteristicValue( characteristic ) ) {
      message = 'writeCharacteristicValue succeeds.';
    } else {
      message = 'writeCharacteristicValue failed!';
    }
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testWriteCharacteristicValueManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testWriteCharacteristicValueManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function string2ArrayBuffer( str ) {
  let array = new Uint8Array(str.length);
  for ( var i = 0 ; i < str.length ; i ++ ) {
    array[i] = str.charCodeAt( i );
  }
  return array.buffer
}

export function testWriteDescriptorValueManager(): String {
  try {
    let serviceUUID = AppStorage.Get( 'serviceUUID' );
    let characteristicUUID = AppStorage.Get( 'characteristicUUID' );
    let descriptorUUID = AppStorage.Get( 'descriptorUUID' );
    let descriptorValue = AppStorage.Get( 'descriptorValue' );
    let message = "writeDescriptorValue test"
    let btState = bluetoothManager.getState();
    if ( btState === bluetoothManager.BluetoothState.STATE_OFF ) {
      message = "BT is not enabled!";
      prompt.showToast( { message : message } )
      return message;
    }
    if ( !gattClientInstance ) {
      message = "NO_GATT_CLIENT_OBJECT";
      prompt.showToast( { message : message } )
      return message;
    }
    let bufferDesc = string2ArrayBuffer( descriptorValue );
    let descriptor = {
      serviceUuid : serviceUUID ,
      characteristicUuid : characteristicUUID ,
      descriptorUuid : descriptorUUID ,
      descriptorValue : bufferDesc
    };
    if ( gattClientInstance.writeDescriptorValue( descriptor ) ) {
      message = 'writeDescriptorValue succeeds.';
    }
    else {
      message = 'writeDescriptorValue,failed!';
    }
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testWriteDescriptorValueManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testWriteDescriptorValueManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

//回调函数
let BLECharacteristicChange = false

export function testOnBLECharacteristicChangeManager(): String {
  try {
    let message = "onBLECharacteristicChange test"
    let btStatus = bluetoothManager.getState();
    if ( btStatus === bluetoothManager.BluetoothState.STATE_OFF ) {
      message = "BT is not enabled!";
      prompt.showToast( { message : message } )
      return message;
    }
    if ( !gattClientInstance ) {
      message = "NO_GATT_CLIENT_OBJECT";
      prompt.showToast( { message : message } )
      return message;
    }

    if ( BLECharacteristicChange ) {
      gattClientInstance.off( 'BLECharacteristicChange' , () => {
        message = "off BLECharacteristicChange"
        prompt.showToast( { message : message } )
      } )
    } else {
      gattClientInstance.on( 'BLECharacteristicChange' , ( data ) => {
        let serviceUuid = data.serviceUuid;
        let characteristicUuid = data.characteristicUuid;
        let bleCharacteristicChangeInfo = "serviceUuid:" + serviceUuid + "\n";
        bleCharacteristicChangeInfo += "characteristicUuid:" + characteristicUuid + "\n";
        let value = new Uint8Array(data.characteristicValue);
        for ( let i = 0 ; i < data.characteristicValue.byteLength ; i ++ ) {
          bleCharacteristicChangeInfo += "\n" + 'value:' + value[i]
        }
        AppStorage.SetOrCreate( "bleCharacteristicChangeInfo" , bleCharacteristicChangeInfo )
        prompt.showToast( { message : "on:BLECharacteristicChange" } )
      } )
      message = "On BLECharacteristicChange"
    }
    BLECharacteristicChange = !BLECharacteristicChange
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testOnBLECharacteristicChangeManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testOnBLECharacteristicChangeManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testSetBLEMtuSizeManager(): String {
  try {
    let message = "setBLEMtuSize test"
    let btStatus = bluetoothManager.getState();
    if ( btStatus === bluetoothManager.BluetoothState.STATE_OFF ) {
      message = "BT is not enabled!";
      prompt.showToast( { message : message } )
      return message;
    }
    if ( !gattClientInstance ) {
      message = "NO_GATT_CLIENT_OBJECT";
      prompt.showToast( { message : message } )
      return message;
    }

    message = 'setBLEMtuSize: \n';
    let mtuSize = 256;
    let ret = gattClientInstance.setBLEMtuSize( mtuSize );
    message += "result : " + ret;
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testSetBLEMtuSizeManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testSetBLEMtuSizeManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testSetNotifyCharacteristicChangedManager(): String {
  try {
    let serviceUUID = AppStorage.Get( 'serviceUUID' );
    let characteristicUUID = AppStorage.Get( 'characteristicUUID' );
    let descriptorUUID = AppStorage.Get( 'descriptorUUID' );
    let descriptorValue = AppStorage.Get( 'descriptorValue' );
    let message = "setNotifyCharacteristicChanged test"
    let btStatus = bluetoothManager.getState();
    if ( btStatus === bluetoothManager.BluetoothState.STATE_OFF ) {
      message = "BT is not enabled!";
      prompt.showToast( { message : message } )
      return message;
    }
    if ( !gattClientInstance ) {
      message = "NO_GATT_CLIENT_OBJECT";
      prompt.showToast( { message : message } )
      return message;
    }

    let descriptors = [];
    let bufferCCC = new ArrayBuffer(8);
    let cccV = new Uint8Array(bufferCCC);
    cccV[0] = 66;
    let descriptor = {
      serviceUuid : serviceUUID ,
      characteristicUuid : characteristicUUID ,
      descriptorUuid : descriptorUUID ,
      descriptorValue : string2ArrayBuffer( descriptorValue )
    }
    descriptors.push( descriptor );
    let characteristicIn = {
      serviceUuid : serviceUUID ,
      characteristicUuid : characteristicUUID ,
      characteristicValue : bufferCCC ,
      descriptors : descriptors
    };
    if ( gattClientInstance.setNotifyCharacteristicChanged( characteristicIn , true ) ) {
      message = "setNotifyCharacteristicChanged(true):succeeds."
    } else {
      message = "setNotifyCharacteristicChanged(true):failed!"
      if ( gattClientInstance.setNotifyCharacteristicChanged( characteristicIn , false ) ) {
        message = "setNotifyCharacteristicChanged(false):true!"
      } else {
        message = "setNotifyCharacteristicChanged(false):false!"
      }
    }
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testSetNotifyCharacteristicChangedManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testSetNotifyCharacteristicChangedManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testGetDeviceNameCallbackManager(): String {
  try {
    let deviceName = ""
    let message = "getDeviceNameCallback test"
    if ( !gattClientInstance ) {
      message = "NO_GATT_CLIENT_OBJECT";
      prompt.showToast( { message : message } )
      return message;
    }
    //currentClick = 15;
    console.log( "gattClientInstance.getDeviceName,callback start-----------------" )
    gattClientInstance.getDeviceName( ( err , asyncRemoteDeviceName ) => {
      message = "return code:"
      message += JSON.stringify( err );
      message += ",and DeviceName is: "
      message += JSON.stringify( asyncRemoteDeviceName );
      deviceName = JSON.stringify( asyncRemoteDeviceName )
      AppStorage.SetOrCreate( "deviceName" , deviceName )
      prompt.showToast( { message : message } )
    } );
    console.log( "gattClientInstance.getDeviceName,callback end-----------------" )
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testGetDeviceNameCallbackManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testGetDeviceNameCallbackManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testGetDeviceNamePromiseManager(): String {
  try {
    let deviceName = ""
    let message = "getDeviceNamePromise test"
    if ( !gattClientInstance ) {
      message = "NO_GATT_CLIENT_OBJECT";
      prompt.showToast( { message : message } )
      return message;
    }
    console.log( "gattClientInstance.getDeviceName,promise start-----------------" )
    gattClientInstance.getDeviceName().then( ( peripheralDeviceName ) => {
      message = "getDeviceName()Promise:" + peripheralDeviceName;
      deviceName = peripheralDeviceName
      AppStorage.SetOrCreate( "deviceName" , deviceName )
      prompt.showToast( { message : message } )
    } ).catch( err => {
      console.log( "testGetDeviceNamePromiseManager err:" + err )
    } );
    console.log( "gattClientInstance.getDeviceName,promise end-----------------" )
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testGetDeviceNamePromiseManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testGetDeviceNamePromiseManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testGetRssiValueCallbackManager(): String {
  try {
    let Rssi = ""
    let message = "getRssiValueCallback test"
    if ( !gattClientInstance ) {
      message = "NO_GATT_CLIENT_OBJECT";
      prompt.showToast( { message : message } )
      return message;
    }

    message = "getRssiValue callback";
    console.log( "gattClientInstance.getRssiValue,callback start-----------------" )
    gattClientInstance.getRssiValue( ( err , cbRssi ) => {
      message = "return code:"
      message += JSON.stringify( err );
      message += ",and RSSI="
      message += JSON.stringify( cbRssi );
      Rssi = JSON.stringify( cbRssi );
      AppStorage.SetOrCreate( "Rssi" , Rssi )
      prompt.showToast( { message : message } )
    } );
    console.log( "gattClientInstance.getRssiValue,callback end-----------------" )
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testGetRssiValueCallbackManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testGetRssiValueCallbackManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testGetRssiValuePromiseManager(): String {
  try {
    let Rssi = ""
    let message = "getRssiValuePromise test"
    if ( !gattClientInstance ) {
      message = "NO_GATT_CLIENT_OBJECT";
      prompt.showToast( { message : message } )
      return message;
    }
    message = "getRssiValue promise:";
    console.log( "gattClientInstance.getRssiValue,promise start-----------------" )
    gattClientInstance.getRssiValue().then( ( rssiValue ) => {
      message += 'rssi' + JSON.stringify( rssiValue )
      Rssi = JSON.stringify( rssiValue )
      AppStorage.SetOrCreate( "Rssi" , Rssi )
      prompt.showToast( { message : message } )
    } ).catch( err => {
      console.log( "testGetRssiValuePromiseManager err:" + err )
    } );
    console.log( "gattClientInstance.getRssiValue,promise  end-----------------" )
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testGetRssiValuePromiseManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testGetRssiValuePromiseManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}