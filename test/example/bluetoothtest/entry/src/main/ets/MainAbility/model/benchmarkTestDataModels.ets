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
import {
  TestBenchmark ,
  BenchmarkCategory
} from './testData'
import {
  testOnStateChange ,
  testEnableBluetooth ,
  testGetState ,
  testSetLocalName ,
  testGetLocalName ,
  testSetBluetoothScanMode ,
  testGetBluetoothScanMode
} from './brInterface'
import {
  testCreateGattClient ,
  testConnect ,
  testOnBLEConnectionStateChange ,
  testGetServicesCallback ,
  testReadCharacteristicValueCallback ,
  testWriteCharacteristicValue ,
  testOnBLECharacteristicChange ,
  testSetNotifyCharacteristicChanged
} from './gattClientInterface'
import {
  testCreateGattServer ,
  testStartAdvertising ,
  testAddService ,
  testOnConnectStateChange ,
  testOnCharacteristicRead ,
  testOnCharacteristicWrite
} from './gattServerInterface'
import { testSppClientConnectSafe } from './sppClientInterface'


export const BrBenchmarkTestCase: [] = [
  {
    'name' : '注册状态变化' ,
    'detail' : '1' ,
    'method' : 'on.stateChange' ,
    'api' : testOnStateChange ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.BrBenchmark
  },
  {
    'name' : '开蓝牙' ,
    'detail' : '2' ,
    'method' : 'enableBluetooth' ,
    'api' : testEnableBluetooth ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.BrBenchmark
  },
  {
    'name' : '获取状态' ,
    'detail' : '3' ,
    'method' : 'getState' ,
    'api' : testGetState ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.BrBenchmark
  },
  {
    'name' : '设置本地名称' ,
    'detail' : '4' ,
    'method' : 'setLocalName' ,
    'api' : testSetLocalName ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.BrBenchmark
  },
  {
    'name' : '获取本地名称' ,
    'detail' : '5' ,
    'method' : 'getLocalName' ,
    'api' : testGetLocalName ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.BrBenchmark
  },
  {
    'name' : '设置扫描模式' ,
    'detail' : '6' ,
    'method' : 'setBluetoothScanMode' ,
    'api' : testSetBluetoothScanMode ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.BrBenchmark
  },
  {
    'name' : '获取扫描模式' ,
    'detail' : '7' ,
    'method' : 'getBluetoothScanMode' ,
    'api' : testGetBluetoothScanMode ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.BrBenchmark
  },
]

export const BleBenchmarkTestCase: [] = [
  {
    'name' : '注册状态变化' ,
    'detail' : '1' ,
    'method' : 'on.stateChange' ,
    'api' : testOnStateChange ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.BleBenchmark
  },
  {
    'name' : '开蓝牙' ,
    'detail' : '2' ,
    'method' : 'enableBluetooth' ,
    'api' : testEnableBluetooth ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.BleBenchmark
  },
  {
    'name' : '获取状态' ,
    'detail' : '3' ,
    'method' : 'getState' ,
    'api' : testGetState ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.BleBenchmark
  },
  {
    'name' : '设置本地名称' ,
    'detail' : '4' ,
    'method' : 'setLocalName' ,
    'api' : testSetLocalName ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.BleBenchmark
  },
  {
    'name' : '获取本地名称' ,
    'detail' : '5' ,
    'method' : 'getLocalName' ,
    'api' : testGetLocalName ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.BleBenchmark
  },
  {
    'name' : '设置扫描模式' ,
    'detail' : '6' ,
    'method' : 'setBluetoothScanMode' ,
    'api' : testSetBluetoothScanMode ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.BleBenchmark
  },
  {
    'name' : '获取扫描模式' ,
    'detail' : '7' ,
    'method' : 'getBluetoothScanMode' ,
    'api' : testGetBluetoothScanMode ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.BleBenchmark
  },

]

export const GattClientBenchmarkTestCase: [] = [
  {
    'name' : '创建Gatt客户端' ,
    'detail' : '1' ,
    'method' : 'createGattClient' ,
    'api' : testCreateGattClient ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.GattClientBenchmark
  },
  {
    'name' : 'Ble连接状态' ,
    'detail' : '2' ,
    'method' : 'on.BleConnectionStateChange' ,
    'api' : testOnBLEConnectionStateChange ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.GattClientBenchmark
  },
  {
    'name' : '连接' ,
    'detail' : '3' ,
    'method' : 'connect' ,
    'api' : testConnect ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.GattClientBenchmark
  },
  {
    'name' : '设置通知特性更改' ,
    'detail' : '4' ,
    'method' : 'setNotifyCharacteristicChanged' ,
    'api' : testSetNotifyCharacteristicChanged ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.GattClientBenchmark
  },
  {
    'name' : 'Ble特征值变化' ,
    'detail' : '5' ,
    'method' : 'on.BleCharacteristicChange' ,
    'api' : testOnBLECharacteristicChange ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.GattClientBenchmark
  },
  {
    'name' : '获取服务callback' ,
    'detail' : '6' ,
    'method' : 'getServices(Callback)' ,
    'api' : testGetServicesCallback ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.GattClientBenchmark
  },
  {
    'name' : '读取特征值callback' ,
    'detail' : '7' ,
    'method' : 'readCharacteristicValue(callback)' ,
    'api' : testReadCharacteristicValueCallback ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.GattClientBenchmark
  },
  {
    'name' : '写入特征值' ,
    'detail' : '8' ,
    'method' : 'writeCharacteristicValue' ,
    'api' : testWriteCharacteristicValue ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.GattClientBenchmark
  },
]

export const GattServerBenchmarkTestCase: [] = [
  {
    'name' : '创建Gatt服务器' ,
    'detail' : '1' ,
    'method' : 'createGattServer' ,
    'api' : testCreateGattServer ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.GattServerBenchmark
  },
  {
    'name' : '添加服务' ,
    'detail' : '2' ,
    'method' : 'addService' ,
    'api' : testAddService ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.GattServerBenchmark
  },
  {
    'name' : '连接状态' ,
    'detail' : '3' ,
    'method' : 'on.connectStateChange' ,
    'api' : testOnConnectStateChange ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.GattServerBenchmark
  },
  {
    'name' : '开始广播' ,
    'detail' : '4' ,
    'method' : 'startAdvertising' ,
    'api' : testStartAdvertising ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.GattServerBenchmark
  },
  {
    'name' : '读取特征值' ,
    'detail' : '5' ,
    'method' : 'on.characteristicRead' ,
    'api' : testOnCharacteristicRead ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.GattServerBenchmark
  },
  {
    'name' : '写入特征值' ,
    'detail' : '6' ,
    'method' : 'on.characteristicWrite' ,
    'api' : testOnCharacteristicWrite ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.GattServerBenchmark
  },
]

export const SppClientBenchmarkTestCase: [] = [
  {
    'name' : '注册状态变化' ,
    'detail' : '1' ,
    'method' : 'on.stateChange' ,
    'api' : testOnStateChange ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.SppClientBenchmark
  },
  {
    'name' : '开蓝牙' ,
    'detail' : '2' ,
    'method' : 'enableBluetooth' ,
    'api' : testEnableBluetooth ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.SppClientBenchmark
  },
  {
    'name' : '获取状态' ,
    'detail' : '3' ,
    'method' : 'getState' ,
    'api' : testGetState ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.SppClientBenchmark
  },
  {
    'name' : '设置本地名称' ,
    'detail' : '4' ,
    'method' : 'setLocalName' ,
    'api' : testSetLocalName ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.SppClientBenchmark
  },
  {
    'name' : '获取本地名称' ,
    'detail' : '5' ,
    'method' : 'getLocalName' ,
    'api' : testGetLocalName ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.SppClientBenchmark
  },
  {
    'name' : '设置SPP扫描模式' ,
    'detail' : '6' ,
    'method' : 'setBluetoothScanMode' ,
    'api' : testSetBluetoothScanMode ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.SppClientBenchmark
  },
  {
    'name' : '获取扫描模式' ,
    'detail' : '7' ,
    'method' : 'getBluetoothScanMode' ,
    'api' : testGetBluetoothScanMode ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.SppClientBenchmark
  },
  {
    'name' : 'Spp客户端连接(安全模式)' ,
    'detail' : '8' ,
    'method' : 'sppConnect' ,
    'api' : testSppClientConnectSafe ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.SppClientBenchmark
  },
]

export const SppServerBenchmarkTestCase: [] = [
  {
    'name' : '注册状态变化' ,
    'detail' : '1' ,
    'method' : 'on.stateChange' ,
    'api' : testOnStateChange ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.SppServerBenchmark
  },
  {
    'name' : '开蓝牙' ,
    'detail' : '2' ,
    'method' : 'enableBluetooth' ,
    'api' : testEnableBluetooth ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.SppServerBenchmark
  },
  {
    'name' : '获取状态' ,
    'detail' : '3' ,
    'method' : 'getState' ,
    'api' : testGetState ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.SppServerBenchmark
  },
  {
    'name' : '设置本地名称' ,
    'detail' : '4' ,
    'method' : 'setLocalName' ,
    'api' : testSetLocalName ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.SppServerBenchmark
  },
  {
    'name' : '获取本地名称' ,
    'detail' : '5' ,
    'method' : 'getLocalName' ,
    'api' : testGetLocalName ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.SppServerBenchmark
  },
  {
    'name' : '设置SPP扫描模式' ,
    'detail' : '6' ,
    'method' : 'setBluetoothScanMode' ,
    'api' : testSetBluetoothScanMode ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.SppServerBenchmark
  },
  {
    'name' : '获取扫描模式' ,
    'detail' : '7' ,
    'method' : 'getBluetoothScanMode' ,
    'api' : testGetBluetoothScanMode ,
    'result' : 'None' ,
    'category' : BenchmarkCategory.SppServerBenchmark
  },
]


export function initBrBenchmarkData(): Array<TestBenchmark> {
  let TestBenchmarkArray: Array<TestBenchmark> = []
  BrBenchmarkTestCase.forEach( item => {
    TestBenchmarkArray.push( new TestBenchmark(item.name , item.detail , item.method , item.api , item.result , item.Category) );
  } )
  return TestBenchmarkArray;
}

export function initBleBenchmarkData(): Array<TestBenchmark> {
  let TestBenchmarkArray: Array<TestBenchmark> = []
  BleBenchmarkTestCase.forEach( item => {
    TestBenchmarkArray.push( new TestBenchmark(item.name , item.detail , item.method , item.api , item.result , item.Category) );
  } )
  return TestBenchmarkArray;
}

export function initGattClientBenchmarkData(): Array<TestBenchmark> {
  let TestBenchmarkArray: Array<TestBenchmark> = []
  GattClientBenchmarkTestCase.forEach( item => {
    TestBenchmarkArray.push( new TestBenchmark(item.name , item.detail , item.method , item.api , item.result , item.Category) );
  } )
  return TestBenchmarkArray;
}

export function initGattServerBenchmarkData(): Array<TestBenchmark> {
  let TestBenchmarkArray: Array<TestBenchmark> = []
  GattServerBenchmarkTestCase.forEach( item => {
    TestBenchmarkArray.push( new TestBenchmark(item.name , item.detail , item.method , item.api , item.result , item.Category) );
  } )
  return TestBenchmarkArray;
}

export function initSppClientBenchmarkData(): Array<TestBenchmark> {
  let TestBenchmarkArray: Array<TestBenchmark> = []
  SppClientBenchmarkTestCase.forEach( item => {
    TestBenchmarkArray.push( new TestBenchmark(item.name , item.detail , item.method , item.api , item.result , item.Category) );
  } )
  return TestBenchmarkArray;
}

export function initSppServerBenchmarkData(): Array<TestBenchmark> {
  let TestBenchmarkArray: Array<TestBenchmark> = []
  SppServerBenchmarkTestCase.forEach( item => {
    TestBenchmarkArray.push( new TestBenchmark(item.name , item.detail , item.method , item.api , item.result , item.Category) );
  } )
  return TestBenchmarkArray;
}