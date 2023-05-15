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
 *BR Manager Test Page Of Bluetooth test
 */

import prompt from '@system.prompt';
import BluetoothModelManager from './BluetoothModelManager'
import Router from '@system.router';
import BluetoothDeviceController from '../controller/BluetoothDeviceController';
import bluetoothManager from '@ohos.bluetoothManager';

const PAGE_URI_DEVICE_FOUND_MODE = 'pages/subManualApiTest/subBrTest/deviceFound';

export function testInputBluetoothManager(): string {
  //  let ret = btApi.enableBluetooth();
  //  let btManagerEnable = ret;
  //  AppStorage.SetOrCreate('bluetoothManagerIsOn', btManagerEnable);
  //  let message = "蓝牙使能执行结果：" + ret;
  let message = "testInputBluetooth"
  prompt.showToast( { message : message } )
  return message
}

let btManagerEnable = false

export function testEnableBluetoothManager() {
  try {
    let message = "enableBluetooth test"
    if ( btManagerEnable ) {
      message = '蓝牙已经使能';
    } else {
      let ret = BluetoothModelManager.enableBluetooth();
      if ( ret ) {
        message = "蓝牙已打开或打开失败,请确认"
      }
      btManagerEnable = true;
      AppStorage.SetOrCreate( 'bluetoothManagerIsOn' , btManagerEnable );
    }
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testEnableBluetoothManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testEnableBluetoothManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testDisableBluetoothManager(): string {
  try {
    let message = "disableBluetooth test"
    if ( !btManagerEnable ) {
      message = '蓝牙还未使能';
    } else {
      let ret = BluetoothModelManager.disableBluetooth();
      if ( ret ) {
        message = "蓝牙已关闭或关闭失败,请确认"
      }
      btManagerEnable = false;
      AppStorage.SetOrCreate( 'bluetoothManagerIsOn' , btManagerEnable );
    }
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testDisableBluetoothManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testDisableBluetoothManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testGetStateManager(): string {
  try {
    let ret = BluetoothModelManager.getState();
    let stateManagerBT = "getState test"
    switch ( ret ) {
      case 0:
        stateManagerBT = 'STATE_OFF';
        break;
      case 1:
        stateManagerBT = 'STATE_TURNING_ON';
        break;
      case 2:
        stateManagerBT = 'STATE_ON';
        break;
      case 3:
        stateManagerBT = 'STATE_TURNING_OFF';
        break;
      case 4:
        stateManagerBT = 'STATE_BLE_TURNING_ON';
        break;
      case 5:
        stateManagerBT = 'STATE_BLE_ON';
        break;
      case 6:
        stateManagerBT = 'STATE_BLE_TURNING_OFF';
        break;
      default:
        stateManagerBT = '未知状态';
        break;
    }
    let message = "当前蓝牙的状态是：" + stateManagerBT;
    if ( ret == 0 || ret == 3 ) {
      btManagerEnable = false
    } else if ( ret == 1 || ret == 2 ) {
      btManagerEnable = true
    } else {
      console.log( "查看bt状态" )
    }
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testGetStateManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testGetStateManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testGetBtConnectionStateManager(): string {
  try {
    let ret = BluetoothModelManager.getBtConnectionState();
    let message = "当前连接状态是：" + ret;
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testGetBtConnectionStateManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testGetBtConnectionStateManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testSetLocalNameManager( name: string ): string {
  try {
    let ret = BluetoothModelManager.setLocalName( name );
    let message = "设置：" + ret + "设备名称为：" + name;
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testSetLocalNameManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testSetLocalNameManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testGetLocalNameManager(): string {
  try {
    let ret = BluetoothModelManager.getLocalName();
    let message = "本设备名称是：" + ret;
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testGetLocalNameManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testGetLocalNameManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testSetBluetoothScanModeManager( mode: number , dur: number ): {} {
  try {
    let retObj = { mod : 0 , duration : - 1 }
    let ret = BluetoothModelManager.setBluetoothScanMode( mode , dur );
    //  let ret = true   调试使用
    if ( ret ) {
      AppStorage.SetOrCreate( 'setScanModeRet' , ret );
      retObj.mod = mode;
      retObj.duration = dur;
    } else {
      console.log( "BluetoothModelManager.setBluetoothScanMode failed!" )
    }
    console.log( "BluetoothModelManager.setBluetoothScanMode success!" + JSON.stringify( retObj ) )
    let message = ret ? "setBluetoothScanMode执行成功" + JSON.stringify( retObj ) : "setBluetoothScanMode执行失败"
    prompt.showToast( { message : message } )
    return retObj
  } catch( e ) {
    console.log( `Failed to testSetBluetoothScanModeManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testSetBluetoothScanModeManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testGetBluetoothScanModeManager(): string {
  try {
    let ret = BluetoothModelManager.getBluetoothScanMode();
    let message = "模式: " + ret;
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testGetBluetoothScanModeManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testGetBluetoothScanModeManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testStartBluetoothDiscoveryManager() {
  try {
    let message = "startBluetoothDiscovery test"
    let btManagerEnable = AppStorage.Get( 'bluetoothManagerIsOn' )
    if ( !btManagerEnable ) {
      message = '蓝牙还未使能';
      return message;
    } else {
      Router.push( { uri : PAGE_URI_DEVICE_FOUND_MODE } );
      message = "蓝牙发现成功"
    }
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testStartBluetoothDiscoveryManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testStartBluetoothDiscoveryManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testStopBluetoothDiscoveryManager(): string {
  try {
    let ret = BluetoothModelManager.stopBluetoothDiscovery();
    let message = "stopBluetoothDiscovery 执行：" + ret;
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testStopBluetoothDiscoveryManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testStopBluetoothDiscoveryManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testPairDeviceManager( deviceId: string ): string {
  try {
    let controller = new BluetoothDeviceController;
    let ret = bluetoothManager.pairDevice( deviceId );
    let message = ""
    controller.pair( deviceId , ( pinCode: string ) => {
      prompt.showToast( { message : pinCode } )
      AppStorage.SetOrCreate( 'pairedMac' , deviceId );
    } , () => {
      prompt.showToast( { message : "testPairDevice Failed" } )
    } )
    message = "配对：" + deviceId + ret ? "成功" : "失败"
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testPairDeviceManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testPairDeviceManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testUnPairedDeviceManager( deviceId: string ): string {
  //  let deviceId = "123123"
  try {
    let ret = BluetoothModelManager.unpairDevice( deviceId );
    let message = " 设备 " + deviceId + " 断开：" + ret;
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testUnPairedDeviceManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testUnPairedDeviceManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testGetRemoteDeviceNameManager( deviceId: string ): string {
  try {
    let ret = BluetoothModelManager.getDeviceName( deviceId );
    let message = '远程设备名:' + ret;
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testGetRemoteDeviceNameManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testGetRemoteDeviceNameManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testGetRemoteDeviceClassManager( deviceId: string ): string {
  //  let deviceId= "12312fsdf";
  try {
    let ret = BluetoothModelManager.getDeviceType( deviceId );
    let message = '远程class名:' + ret;
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testGetRemoteDeviceClassManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testGetRemoteDeviceClassManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testGetPairedDevicesManager(): string {
  try {
    let ret = BluetoothModelManager.getPairedDeviceIds();
    let message = "getPairedDevices test"
    console.log( "BluetoothModelManager.getPairedDeviceIds:" + ret )
    if ( ret.length == 0 ) {
      message = '当前没有任何配对设备';
    } else {
      message = "设备为：" + ret[0]
      AppStorage.SetOrCreate( 'pairedMac' , ret[0] );
    }
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testGetPairedDevicesManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testGetPairedDevicesManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}


export function testGetProfileConnStateManager1( profileId: number ): string {
  try {
    let ret = BluetoothModelManager.getProfileConnState( profileId );
    let message = ret
    prompt.showToast( { message : message } )
    console.log( "BluetoothModelManager.testGetProfileConnState1:" + ret )
    return message
  } catch( e ) {
    console.log( `Failed to testGetProfileConnStateManager1. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testGetProfileConnStateManager1. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testGetProfileConnStateManager2( profileId: number , deviceId: string ): string {
  try {
    let ret = bluetoothManager.getProfileInstance( profileId ).getDeviceState( deviceId );
    let stateProfile = ""
    console.log( 'testGetProfileConnState2 state is ' + ret );
    switch ( ret ) {
      case 0:
        stateProfile = 'STATE_DISCONNECTED';
        break;
      case 1:
        stateProfile = 'STATE_CONNECTING';
        break;
      case 2:
        stateProfile = 'STATE_CONNECTED';
        break;
      case 3:
        stateProfile = 'STATE_DISCONNECTING';
        break;
      default:
        stateProfile = '未知';
        break;
    }
    let message = stateProfile;
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testGetProfileConnStateManager2. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testGetProfileConnStateManager2. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

let stateChangeManager = false

export function testOnStateChangeManager(): string {
  try {
    let message = "onStateChange test"
    console.log( 'testOnStateChange test' );
    if ( stateChangeManager ) {
      bluetoothManager.off( 'stateChange' , () => {
        prompt.showToast( { message : "on.stateChange监听已关闭" } )
      } )
      message = "关闭注册状态变化"
      console.log( 'testOffStateChange test' );
    } else {
      bluetoothManager.on( 'stateChange' , ( data: bluetoothManager.BluetoothState ) => {
        let btStateMessageManager = ""
        switch ( data ) {
          case 0:
            btStateMessageManager += 'STATE_OFF';
            break;
          case 1:
            btStateMessageManager += 'STATE_TURNING_ON';
            break;
          case 2:
            btStateMessageManager += 'STATE_ON';
            break;
          case 3:
            btStateMessageManager += 'STATE_TURNING_OFF';
            break;
          case 4:
            btStateMessageManager += 'STATE_BLE_TURNING_ON';
            break;
          case 5:
            btStateMessageManager += 'STATE_BLE_ON';
            break;
          case 6:
            btStateMessageManager += 'STATE_BLE_TURNING_OFF';
            break;
          default:
            btStateMessageManager += '未知状态';
            break;
        }
        prompt.showToast( { message : "状态:" + btStateMessageManager } )
      } )
      message = "打开注册状态变化"
      prompt.showToast( { message : message } )
    }
    stateChangeManager = !stateChangeManager
    return message
  } catch( e ) {
    console.log( `Failed to testOnStateChangeManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testOnStateChangeManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

let pinRequiredManager = false

export function testOnPinRequiredManager(): string {
  try {
    let message = "onPinRequired test"
    console.log( 'testOnPinRequired test' );
    if ( pinRequiredManager ) {
      bluetoothManager.off( 'pinRequired' , () => {
        prompt.showToast( { message : "on.pinRequired监听已关闭" } )
      } )
      message = "关闭注册pin码变化"
    } else {
      bluetoothManager.on( 'pinRequired' , ( data: bluetoothManager.PinRequiredParam ) => {
        prompt.showToast( { message : "mac:" + data.deviceId + "pin:" + data.pinCode } )
      } )
      message = "打开注册pin码变化"
    }
    pinRequiredManager = !pinRequiredManager
    return message
  } catch( e ) {
    console.log( `Failed to testOnPinRequiredManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testOnPinRequiredManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

let bondStateFlagManager = false

export function testOnBondStateChangeManager(): string {
  try {
    let message = "onBondStateChange test"
    console.log( 'testOnBondStateChange test' );
    if ( bondStateFlagManager ) {
      bluetoothManager.off( 'bondStateChange' , () => {
        prompt.showToast( { message : "on.bondStateChange监听已关闭" } )
      } )
      message = "关闭注册绑定状态变化"
    } else {
      bluetoothManager.on( 'bondStateChange' , ( data: bluetoothManager.BondStateParam ) => {
        prompt.showToast( { message : "mac:" + data.deviceId + "state:" + data.state } )
      } )
      message = "打开注册绑定状态变化"
    }
    bondStateFlagManager = !bondStateFlagManager
    return message
  } catch( e ) {
    console.log( `Failed to testOnBondStateChangeManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testOnBondStateChangeManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function subscribeBluetoothDeviceFindManager(): string {
  try {
    let message = "subscribeBluetoothDeviceFind test"
    console.log( "subscribeBluetoothDeviceFind" )
    BluetoothModelManager.subscribeBluetoothDeviceFind( ( deviceIds: Array<string> ) => {
      console.log( 'available bluetooth devices changed.' );
      deviceIds?.forEach( async( deviceId ) => {
        console.log( 'available bluetooth find [' + deviceId + '] ' )
        let message = ( 'available bluetooth find [' + deviceId + ']  ' )
        prompt.showToast( { message : message } )
        AppStorage.SetOrCreate( 'bluetoothAvailableDevices' , message );
      } )
    } );
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to subscribeBluetoothDeviceFindManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to subscribeBluetoothDeviceFindManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}