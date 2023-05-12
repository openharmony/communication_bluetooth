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
  TestDataManager ,
  TestApiManager ,
  CategoryManager ,
  CaseCategoryManager
} from './testDataManager'
import {
  testEnableBluetoothManager ,
  testDisableBluetoothManager ,
  testGetStateManager ,
  testGetBtConnectionStateManager ,
  testSetLocalNameManager ,
  testGetLocalNameManager ,
  testSetBluetoothScanModeManager ,
  testGetBluetoothScanModeManager ,
  testStartBluetoothDiscoveryManager ,
  testStopBluetoothDiscoveryManager ,
  testPairDeviceManager ,
  testGetRemoteDeviceNameManager ,
  testGetRemoteDeviceClassManager ,
  testGetPairedDevicesManager ,
  testUnPairedDeviceManager ,
  testGetProfileConnStateManager1 ,
  testGetProfileConnStateManager2 ,
  testOnStateChangeManager ,
  testOnPinRequiredManager ,
  testOnBondStateChangeManager
} from "./brManagerInterface"
import {
  testCreateGattServerDeviceManager ,
  testCreateGattClientDeviceManager ,
  testGetConnectedBLEDevicesManager ,
  testStartBLEScanManager ,
  testStopBLEScanManager ,
  testOnBLEDeviceFindManager
} from './bleManagerInterface'
import {
  testCreateGattClientManager ,
  testConnectManager ,
  testDisconnectManager ,
  testOnBLEConnectionStateChangeManager ,
  testGattClientCloseManager ,
  testGetServicesCallbackManager ,
  testGetServicesPromiseManager ,
  testReadCharacteristicValueCallbackManager ,
  testReadCharacteristicValuePromiseManager ,
  testReadDescriptorValueCallbackManager ,
  testReadDescriptorValuePromiseManager ,
  testWriteCharacteristicValueManager ,
  testWriteDescriptorValueManager ,
  testOnBLECharacteristicChangeManager ,
  testSetBLEMtuSizeManager ,
  testSetNotifyCharacteristicChangedManager ,
  testGetDeviceNameCallbackManager ,
  testGetDeviceNamePromiseManager ,
  testGetRssiValueCallbackManager ,
  testGetRssiValuePromiseManager
} from './gattClientManagerInterface'
import {
  testCreateGattServerManager ,
  testStartAdvertisingManager ,
  testStopAdvertisingManager ,
  testAddServiceManager ,
  testRemoveServiceManager ,
  testGattServerCloseManager ,
  testOnConnectStateChangeManager ,
  testOnCharacteristicReadManager ,
  testOnCharacteristicWriteManager ,
  testOnDescriptorReadManager ,
  testOnDescriptorWriteManager
} from './gattServerManagerInterface'
import {
  testSppClientConnectSafeManager ,
  testSppClientConnectUnSafeManager ,
  testSppClientWriteManager ,
  testSppClientReadOnManager ,
  testSppClientReadOffManager ,
  testSppClientCloseManager
} from './sppClientManagerInterface'
import {
  testSppServerCreateSafeManager ,
  testSppServerCreateUnSafeManager ,
  testSppServerAcceptManager ,
  testSppServerWriteManager ,
  testSppServerReadOnManager ,
  testSppServerReadOffManager ,
  testSppCloseServerSocketManager ,
  testSppCloseClientSocketManager
} from './sppServerManagerInterface'
import {
  testProfileConnectManager ,
  testProfileDisConnectManager ,
  testProfileGetPlayingStateManager ,
  testProfileGetConnectionDevicesManager ,
  testProfileGetDeviceStateManager
} from './a2dpSinkProfileManagerInterface'

const BRManagerTestCase: [] = [
  {
    'name' : '开蓝牙Manager' ,
    'detail' : '使能蓝牙' ,
    'method' : 'enableBluetoothManager' ,
    'api' : testEnableBluetoothManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.BRManager
  },
  {
    'name' : '关蓝牙Manager' ,
    'detail' : '去使能蓝牙' ,
    'method' : 'disableBluetoothManager' ,
    'api' : testDisableBluetoothManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.BRManager
  },
  {
    'name' : '获取状态Manager' ,
    'detail' : '获取蓝牙状态' ,
    'method' : 'getStateManager' ,
    'api' : testGetStateManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.BRManager
  },
  {
    'name' : '获取当前连接状态Manager' ,
    'detail' : '获取当前蓝牙连接状态' ,
    'method' : 'getBtConnectionStateManager' ,
    'api' : testGetBtConnectionStateManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.BRManager
  },
  {
    'name' : '设置本地名称Manager' ,
    'detail' : '设置蓝牙本地名称' ,
    'method' : 'setLocalNameManager' ,
    'api' : testSetLocalNameManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.BRManager
  },
  {
    'name' : '获取本地名称Manager' ,
    'detail' : '获取蓝牙本地名称' ,
    'method' : 'getLocalNameManager' ,
    'api' : testGetLocalNameManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.BRManager
  },
  {
    'name' : '设置扫描模式Manager' ,
    'detail' : '设置蓝牙扫描模式' ,
    'method' : 'setBluetoothScanModeManager' ,
    'api' : testSetBluetoothScanModeManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.BRManager
  },
  {
    'name' : '获取扫描模式Manager' ,
    'detail' : '获取蓝牙扫描模式' ,
    'method' : 'getBluetoothScanModeManager' ,
    'api' : testGetBluetoothScanModeManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.BRManager
  },
  {
    'name' : '开始蓝牙发现Manager' ,
    'detail' : '发现可配对蓝牙设备' ,
    'method' : 'startBluetoothDiscoveryManager' ,
    'api' : testStartBluetoothDiscoveryManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.BRManager
  },
  {
    'name' : '停止蓝牙发现Manager' ,
    'detail' : '停止发现' ,
    'method' : 'stopBluetoothDiscoveryManager' ,
    'api' : testStopBluetoothDiscoveryManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.BRManager
  },
  {
    'name' : '配对设备Manager' ,
    'detail' : '进行蓝牙设备配对' ,
    'method' : 'pairDeviceManager' ,
    'api' : testPairDeviceManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.BRManager
  },
  {
    'name' : '取消配对设备Manager' ,
    'detail' : '取消蓝牙设备配对' ,
    'method' : 'unpairDeviceManager' ,
    'api' : testUnPairedDeviceManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.BRManager
  },
  {
    'name' : '获取远程设备名称Manager' ,
    'detail' : '获取配对蓝牙设备的名称' ,
    'method' : 'getDeviceNameManager' ,
    'api' : testGetRemoteDeviceNameManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.BRManager
  },
  {
    'name' : '获取远程设备类型Manager' ,
    'detail' : '获取配对蓝牙设备的类型' ,
    'method' : 'getDeviceTypeManager' ,
    'api' : testGetRemoteDeviceClassManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.BRManager
  },
  {
    'name' : '获取配对设备Manager' ,
    'detail' : '获取配对蓝牙设备的信息' ,
    'method' : 'getPairedDeviceIdsManager' ,
    'api' : testGetPairedDevicesManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.BRManager
  },
  {
    'name' : '获取配置文件连接状态Manager1' ,
    'detail' : '获取配置文件的连接状态' ,
    'method' : 'getProfileConnStateManager' ,
    'api' : testGetProfileConnStateManager1 ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.BRManager
  },
  {
    'name' : '获取配置文件连接状态Manager2' ,
    'detail' : '获取设备的配置文件状态' ,
    'method' : 'getDeviceStateManager' ,
    'api' : testGetProfileConnStateManager2 ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.BRManager
  },
  {
    'name' : '注册状态变化Manager' ,
    'detail' : '注册状态变化' ,
    'method' : 'on.stateChangeManager' ,
    'api' : testOnStateChangeManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.BRManager
  },
  {
    'name' : '注册pin码变化Manager' ,
    'detail' : '注册pin码变化' ,
    'method' : 'on.pinRequiredManager' ,
    'api' : testOnPinRequiredManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.BRManager
  },
  {
    'name' : '注册绑定状态变化Manager' ,
    'detail' : '注册绑定状态变化' ,
    'method' : 'on.bondStateChangeManager' ,
    'api' : testOnBondStateChangeManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.BRManager
  }
]

const BleManagerTestCase: any[] = [
  {
    'name' : '创建Gatt服务器Manager' ,
    'detail' : '创建GattManager服务器' ,
    'method' : 'createGattServerManager' ,
    'api' : testCreateGattServerDeviceManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.BLEManager
  },
  {
    'name' : '创建Gatt客户端Manager' ,
    'detail' : '创建GattManager客户端' ,
    'method' : 'createGattClientManager' ,
    'api' : testCreateGattClientDeviceManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.BLEManager
  },
  {
    'name' : '获取连接的BLE设备Manager' ,
    'detail' : '获取连接的BLE设备Manager信息' ,
    'method' : 'getConnectedBLEDevicesManager' ,
    'api' : testGetConnectedBLEDevicesManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.BLEManager
  },
  {
    'name' : '开始BLE扫描Manager' ,
    'detail' : '开始扫描BLEManager设备' ,
    'method' : 'startBLEScanManager' ,
    'api' : testStartBLEScanManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.BLEManager
  },
  {
    'name' : '停止BLE扫描Manager' ,
    'detail' : '停止Manager扫描' ,
    'method' : 'stopBLEScanManager' ,
    'api' : testStopBLEScanManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.BLEManager
  },
  {
    'name' : '注册BLE查找状态变化Manager' ,
    'detail' : '注册vBLE设备查找状态变化Manager' ,
    'method' : 'on.BLEDeviceFindManager' ,
    'api' : testOnBLEDeviceFindManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.BLEManager
  }
]

const GattManagerTestComposition: any[] = [
  {
    'name' : 'GattClientManager' ,
    'detail' : 'Gatt客户端Manager测试' ,
    'url' : 'pages/subManualApiTestManager/subGattTestManager/gattClientManager' ,
    'image' : $r( 'app.media.Gatt' ) ,
    'category' : CategoryManager.GattManager
  },
  {
    'name' : 'GattServerManager' ,
    'detail' : 'Gatt服务器Manager测试' ,
    'url' : 'pages/subManualApiTestManager/subGattTestManager/gattServerManager' ,
    'image' : $r( 'app.media.Gatt' ) ,
    'category' : CategoryManager.GattManager
  },
]

const SppManagerTestComposition: any[] = [
  {
    'name' : 'SppClientManager' ,
    'detail' : 'Spp客户端Manager测试' ,
    'url' : 'pages/subManualApiTestManager/subSppTestManager/sppClientManager' ,
    'image' : $r( 'app.media.spp' ) ,
    'category' : CategoryManager.SPPManager
  },
  {
    'name' : 'SppServerManager' ,
    'detail' : 'Spp服务器Manager测试' ,
    'url' : 'pages/subManualApiTestManager/subSppTestManager/sppServerManager' ,
    'image' : $r( 'app.media.spp' ) ,
    'category' : CategoryManager.SPPManager
  },
]

const GattClientManagerTestCase: any[] = [
  {
    'name' : '创造Gatt客户端Manager' ,
    'detail' : '创造Gatt客户端' ,
    'method' : 'createGattClientManager' ,
    'api' : testCreateGattClientManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattClientManager
  },
  {
    'name' : '连接Manager' ,
    'detail' : 'Gatt连接' ,
    'method' : 'connectManager' ,
    'api' : testConnectManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattClientManager
  },
  {
    'name' : '取消连接Manager' ,
    'detail' : '取消Gatt连接' ,
    'method' : 'disconnectManager' ,
    'api' : testDisconnectManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattClientManager
  },
  {
    'name' : 'BLE连接状态Manager' ,
    'detail' : '查看BLE连接状态' ,
    'method' : 'on.BLEConnectionStateChangeManager' ,
    'api' : testOnBLEConnectionStateChangeManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattClientManager
  },
  {
    'name' : '关闭Manager' ,
    'detail' : '关闭Gatt客户端' ,
    'method' : 'closeManager' ,
    'api' : testGattClientCloseManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattClientManager
  },
  {
    'name' : '获取服务callbackManager' ,
    'detail' : '获取服务callback' ,
    'method' : 'getServices(callback)Manager' ,
    'api' : testGetServicesCallbackManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattClientManager
  },
  {
    'name' : '获取服务promiseManager' ,
    'detail' : '获取服务promise' ,
    'method' : 'getServices(promise)Manager' ,
    'api' : testGetServicesPromiseManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattClientManager
  },
  {
    'name' : '读取特征值callbackManager' ,
    'detail' : '读取特征值callback' ,
    'method' : 'readCharacteristicValue(callback)Manager' ,
    'api' : testReadCharacteristicValueCallbackManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattClientManager
  },
  {
    'name' : '读取特征值promiseManager' ,
    'detail' : '读取特征值promise' ,
    'method' : 'readCharacteristicValue(promise)Manager' ,
    'api' : testReadCharacteristicValuePromiseManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattClientManager
  },
  {
    'name' : '读取描述符值callbackManager' ,
    'detail' : '读取描述符值callback' ,
    'method' : 'readDescriptorValue(callback)Manager' ,
    'api' : testReadDescriptorValueCallbackManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattClientManager
  },
  {
    'name' : '读取描述符值promiseManager' ,
    'detail' : '读取描述符值promise' ,
    'method' : 'readDescriptorValue(promise)Manager' ,
    'api' : testReadDescriptorValuePromiseManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattClientManager
  },
  {
    'name' : '写入特征值Manager' ,
    'detail' : '写入特征值' ,
    'method' : 'writeCharacteristicValueManager' ,
    'api' : testWriteCharacteristicValueManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattClientManager
  },
  {
    'name' : '写入描述符值Manager' ,
    'detail' : '写入描述符值' ,
    'method' : 'writeDescriptorValueManager' ,
    'api' : testWriteDescriptorValueManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattClientManager
  },
  {
    'name' : 'BLE特征值变化Manager' ,
    'detail' : '查看BLE特征值变化' ,
    'method' : 'on.BLECharacteristicChangeManager' ,
    'api' : testOnBLECharacteristicChangeManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattClientManager
  },
  {
    'name' : '设定BLEMtu尺寸Manager' ,
    'detail' : '设定BLEMtu尺寸' ,
    'method' : 'setBLEMtuSizeManager' ,
    'api' : testSetBLEMtuSizeManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattClientManager
  },
  {
    'name' : '设置通知特性更改Manager' ,
    'detail' : '设置通知特性更改' ,
    'method' : 'setNotifyCharacteristicChangedManager' ,
    'api' : testSetNotifyCharacteristicChangedManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattClientManager
  },
  {
    'name' : '获取设备名称callbackManager' ,
    'detail' : '获取设备名称callback' ,
    'method' : 'getDeviceName(callback)Manager' ,
    'api' : testGetDeviceNameCallbackManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattClientManager
  },
  {
    'name' : '获取设备名称promiseManager' ,
    'detail' : '获取设备名称promise' ,
    'method' : 'getDeviceName(promise)Manager' ,
    'api' : testGetDeviceNamePromiseManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattClientManager
  },
  {
    'name' : '获取Rssi值callbackManager' ,
    'detail' : '获取Rssi值callback' ,
    'method' : 'getRssiValue(callback)Manager' ,
    'api' : testGetRssiValueCallbackManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattClientManager
  },
  {
    'name' : '获取Rssi值promiseManager' ,
    'detail' : '获取Rssi值promise' ,
    'method' : 'getRssiValue(promise)Manager' ,
    'api' : testGetRssiValuePromiseManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattClientManager
  },
]

const GattServerManagerTestCase: any[] = [
  {
    'name' : '创建Gatt服务器Manager' ,
    'detail' : '创建Gatt服务器' ,
    'method' : 'createGattServerManager' ,
    'api' : testCreateGattServerManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattServerManager
  },
  {
    'name' : '添加服务Manager' ,
    'detail' : '添加Gatt服务' ,
    'method' : 'addServiceManager' ,
    'api' : testAddServiceManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattServerManager
  },
  {
    'name' : '删除服务Manager' ,
    'detail' : '删除Gatt服务' ,
    'method' : 'removeServiceManager' ,
    'api' : testRemoveServiceManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattServerManager
  },
  {
    'name' : '开始广播Manager' ,
    'detail' : '开始广播播报' ,
    'method' : 'startAdvertisingManager' ,
    'api' : testStartAdvertisingManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattServerManager
  },
  {
    'name' : '停止广播Manager' ,
    'detail' : '停止广播播报' ,
    'method' : 'stopAdvertisingManager' ,
    'api' : testStopAdvertisingManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattServerManager
  },
  {
    'name' : '关闭Manager' ,
    'detail' : '关闭Gatt服务器' ,
    'method' : 'closeManager' ,
    'api' : testGattServerCloseManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattServerManager
  },
  {
    'name' : '连接状态Manager' ,
    'detail' : '查看Gatt连接状态' ,
    'method' : 'on.connectStateChangeManager' ,
    'api' : testOnConnectStateChangeManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattServerManager
  },
  {
    'name' : '读取特征值Manager' ,
    'detail' : '读取特征值' ,
    'method' : 'on.characteristicReadManager' ,
    'api' : testOnCharacteristicReadManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattServerManager
  },
  {
    'name' : '写入特征值Manager' ,
    'detail' : '写入特征值' ,
    'method' : 'on.characteristicWriteManager' ,
    'api' : testOnCharacteristicWriteManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattServerManager
  },
  {
    'name' : '读取描述符Manager' ,
    'detail' : '读取描述符' ,
    'method' : 'on.descriptorRead' ,
    'api' : testOnDescriptorReadManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattServerManager
  },
  {
    'name' : '写入描述符Manager' ,
    'detail' : '写入描述符' ,
    'method' : 'on.descriptorWrite' ,
    'api' : testOnDescriptorWriteManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.GattServerManager
  },
]

const SppClientManagerTestCase: any[] = [
  {
    'name' : '开蓝牙Manager' ,
    'detail' : '使能蓝牙' ,
    'method' : 'enableBluetoothManager' ,
    'api' : testEnableBluetoothManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.SPPClientManager
  },
  {
    'name' : '关蓝牙Manager' ,
    'detail' : '去使能蓝牙' ,
    'method' : 'disableBluetoothManager' ,
    'api' : testDisableBluetoothManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.SPPClientManager
  },
  {
    'name' : '设置SPP扫描模式Manager' ,
    'detail' : '设置SPP扫描模式' ,
    'method' : 'setBluetoothScanModeManager' ,
    'api' : testSetBluetoothScanModeManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.SPPClientManager
  },
  {
    'name' : '获取状态Manager' ,
    'detail' : '获取蓝牙状态' ,
    'method' : 'getStateManager' ,
    'api' : testGetStateManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.SPPClientManager
  },
  {
    'name' : '获取扫描模式Manager' ,
    'detail' : '获取蓝牙扫描模式' ,
    'method' : 'getBluetoothScanModeManager' ,
    'api' : testGetBluetoothScanModeManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.SPPClientManager
  },
  {
    'name' : '获取当前连接状态Manager' ,
    'detail' : '获取当前蓝牙连接状态' ,
    'method' : 'getBtConnectionStateManager' ,
    'api' : testGetBtConnectionStateManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.SPPClientManager
  },
  {
    'name' : 'Spp客户端连接(安全模式)Manager' ,
    'detail' : 'Spp客户端(安全模式)连接' ,
    'method' : 'sppConnectManager' ,
    'api' : testSppClientConnectSafeManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.SPPClientManager
  },
  {
    'name' : 'Spp客户端连接(非安全模式)Manager' ,
    'detail' : 'Spp客户端(非安全模式)连接' ,
    'method' : 'sppConnectManager' ,
    'api' : testSppClientConnectUnSafeManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.SPPClientManager
  },
  {
    'name' : '配对设备Manager' ,
    'detail' : '进行蓝牙设备配对' ,
    'method' : 'pairDeviceManager' ,
    'api' : testPairDeviceManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.SPPClientManager
  },
  {
    'name' : 'Spp客户端写入Manager' ,
    'detail' : 'Spp客户端写入' ,
    'method' : 'sppWriteManager' ,
    'api' : testSppClientWriteManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.SPPClientManager
  },
  {
    'name' : 'Spp客户端读取打开Manager' ,
    'detail' : 'Spp客户端读取打开' ,
    'method' : 'on.sppReadManager' ,
    'api' : testSppClientReadOnManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.SPPClientManager
  },
  {
    'name' : 'Spp客户端读取关闭Manager' ,
    'detail' : 'Spp客户端读取关闭' ,
    'method' : 'off.sppReadManager' ,
    'api' : testSppClientReadOffManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.SPPClientManager
  },
  {
    'name' : 'Spp客户端关闭Manager' ,
    'detail' : 'Spp客户端关闭' ,
    'method' : 'sppCloseClientSocketManager' ,
    'api' : testSppClientCloseManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.SPPClientManager
  },
]

const SppServerManagerTestCase: any[] = [
  {
    'name' : '开蓝牙Manager' ,
    'detail' : '使能蓝牙' ,
    'method' : 'enableBluetoothManager' ,
    'api' : testEnableBluetoothManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.SPPServerManager
  },
  {
    'name' : '关蓝牙Manager' ,
    'detail' : '去使能蓝牙' ,
    'method' : 'disableBluetoothManager' ,
    'api' : testDisableBluetoothManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.SPPServerManager
  },
  {
    'name' : '设置SPP扫描模式Manager' ,
    'detail' : '设置SPP扫描模式' ,
    'method' : 'setBluetoothScanModeManager' ,
    'api' : testSetBluetoothScanModeManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.SPPServerManager
  },
  {
    'name' : '获取状态Manager' ,
    'detail' : '获取蓝牙状态' ,
    'method' : 'getStateManager' ,
    'api' : testGetStateManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.SPPServerManager
  },
  {
    'name' : '获取扫描模式Manager' ,
    'detail' : '获取蓝牙扫描模式' ,
    'method' : 'getBluetoothScanModeManager' ,
    'api' : testGetBluetoothScanModeManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.SPPServerManager
  },
  {
    'name' : '获取当前连接状态Manager' ,
    'detail' : '获取当前蓝牙连接状态' ,
    'method' : 'getBtConnectionStateManager' ,
    'api' : testGetBtConnectionStateManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.SPPServerManager
  },
  {
    'name' : 'Spp服务器创建(安全模式)Manager' ,
    'detail' : 'Spp服务器(安全模式)创建' ,
    'method' : 'sppListenManager' ,
    'api' : testSppServerCreateSafeManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.SPPServerManager
  },
  {
    'name' : 'Spp服务器创建(非安全模式)Manager' ,
    'detail' : 'Spp服务器(非安全模式)创建' ,
    'method' : 'sppListenManager' ,
    'api' : testSppServerCreateUnSafeManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.SPPServerManager
  },
  {
    'name' : '注册pin码变化Manager' ,
    'detail' : '注册pin码变化' ,
    'method' : 'on.pinRequiredManager' ,
    'api' : testOnPinRequiredManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.SPPServerManager
  },
  //  { 'name': 'SPP客户端写入', 'detail': '使能蓝牙', 'method': 'spp_Client_Write', 'api': testSppClientWriteManager, 'result': 'None', 'category': CaseCategoryManager.SPPServer },
  {
    'name' : 'Spp服务器写入Manager' ,
    'detail' : 'Spp服务器写入' ,
    'method' : 'sppWriteManager' ,
    'api' : testSppServerWriteManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.SPPServerManager
  },
  {
    'name' : 'Spp服务器接受Manager' ,
    'detail' : 'Spp服务器接受' ,
    'method' : 'sppAcceptManager' ,
    'api' : testSppServerAcceptManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.SPPServerManager
  },
  {
    'name' : 'Spp服务器读取打开Manager' ,
    'detail' : 'Spp服务器读取打开' ,
    'method' : 'on.sppReadManager' ,
    'api' : testSppServerReadOnManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.SPPServerManager
  },
  {
    'name' : 'Spp服务器读取关闭Manager' ,
    'detail' : 'Spp服务器读取关闭' ,
    'method' : 'off.sppReadManager' ,
    'api' : testSppServerReadOffManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.SPPServerManager
  },
  {
    'name' : 'Spp服务器关闭Manager' ,
    'detail' : 'Spp服务器关闭' ,
    'method' : 'sppCloseServerSocketManager' ,
    'api' : testSppCloseServerSocketManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.SPPServerManager
  },
  {
    'name' : 'Spp客户端关闭Manager' ,
    'detail' : 'Spp客户端关闭' ,
    'method' : 'sppCloseClientSocketManager' ,
    'api' : testSppCloseClientSocketManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.SPPServerManager
  },
]

const A2dpSourceProfileManagerTestCase: any [] = [
  {
    'name' : '连接Manager' ,
    'detail' : 'Profile连接Manager' ,
    'method' : 'connect' ,
    'api' : testProfileConnectManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.ProfileA2dpSourceManager
  },
  {
    'name' : '取消连接Manager' ,
    'detail' : '取消Profile连接Manager' ,
    'method' : 'disconnect' ,
    'api' : testProfileDisConnectManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.ProfileA2dpSourceManager
  },
  {
    'name' : '获取播放状态Manager' ,
    'detail' : '获取Profile播放状态Manager' ,
    'method' : 'getPlayingState' ,
    'api' : testProfileGetPlayingStateManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.ProfileA2dpSourceManager
  },
  {
    'name' : '获取连接设备Manager' ,
    'detail' : '获取Profile连接设备Manager' ,
    'method' : 'getConnectionDevices' ,
    'api' : testProfileGetConnectionDevicesManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.ProfileA2dpSourceManager
  },
  {
    'name' : '获取设备状态Manager' ,
    'detail' : '获取Profile设备状态Manager' ,
    'method' : 'getDeviceState' ,
    'api' : testProfileGetDeviceStateManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.ProfileA2dpSourceManager
  },
]

const HandsFreeAudioGatewayProfileManagerTestCase: any[] = [
  {
    'name' : '连接Manager' ,
    'detail' : 'Profile连接Manager' ,
    'method' : 'connect' ,
    'api' : testProfileConnectManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.ProfileHandsFreeAudioGatewayManager
  },
  {
    'name' : '取消连接Manager' ,
    'detail' : '取消Profile连接Manager' ,
    'method' : 'disconnect' ,
    'api' : testProfileDisConnectManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.ProfileHandsFreeAudioGatewayManager
  },
  {
    'name' : '获取连接设备Manager' ,
    'detail' : '获取Profile连接设备Manager' ,
    'method' : 'getConnectionDevices' ,
    'api' : testProfileGetConnectionDevicesManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.ProfileHandsFreeAudioGatewayManager ,
  },
  {
    'name' : '获取设备状态Manager' ,
    'detail' : '获取Profile设备状态Manager' ,
    'method' : 'getDeviceState' ,
    'api' : testProfileGetDeviceStateManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.ProfileHandsFreeAudioGatewayManager ,
  },
]

const HitHostProfileManagerTestCase: any[] = [
  {
    'name' : '连接Manager' ,
    'detail' : 'Profile连接Manager' ,
    'method' : 'connect' ,
    'api' : testProfileConnectManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.ProfileHitHostManager
  },
  {
    'name' : '取消连接Manager' ,
    'detail' : '取消Profile连接Manager' ,
    'method' : 'disconnect' ,
    'api' : testProfileDisConnectManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.ProfileHitHostManager
  },
  {
    'name' : '获取连接设备Manager' ,
    'detail' : '获取Profile连接设备Manager' ,
    'method' : 'getConnectionDevices' ,
    'api' : testProfileGetConnectionDevicesManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.ProfileHitHostManager
  },
  {
    'name' : '获取设备状态Manager' ,
    'detail' : '获取Profile设备状态Manager' ,
    'method' : 'getDeviceState' ,
    'api' : testProfileGetDeviceStateManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.ProfileHitHostManager
  },
]

const PanNetworkProfileManagerTestCase: any[] = [
  {
    'name' : '取消连接Manager' ,
    'detail' : '取消Profile连接Manager' ,
    'method' : 'disconnect' ,
    'api' : testProfileDisConnectManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.ProfilePanNetworkManager
  },
  {
    'name' : '获取连接设备Manager' ,
    'detail' : '获取Profile连接设备Manager' ,
    'method' : 'getConnectionDevices' ,
    'api' : testProfileGetConnectionDevicesManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.ProfilePanNetworkManager
  },
  {
    'name' : '获取设备状态Manager' ,
    'detail' : '获取Profile设备状态Manager' ,
    'method' : 'getDeviceState' ,
    'api' : testProfileGetDeviceStateManager ,
    'result' : 'None' ,
    'category' : CaseCategoryManager.ProfilePanNetworkManager
  },
]

const GattManagerAutoTestComposition: any[] = [
  {
    'name' : 'GattClientAutoTestManager' ,
    'detail' : 'GattClientManager自动测试' ,
    'url' : 'pages/subAutoTestManager/subGattAutoTestManager/gattClientAutoTestManager' ,
    'image' : $r( "app.media.AutoTestIcon" ) ,
    'category' : CategoryManager.GattAutoTestManager
  },
  {
    'name' : 'GattServerAutoTestManager' ,
    'detail' : 'GattServerManager自动测试' ,
    'url' : 'pages/subAutoTestManager/subGattAutoTestManager/gattServerAutoTestManager' ,
    'image' : $r( "app.media.AutoTestIcon" ) ,
    'category' : CategoryManager.GattAutoTestManager
  },
]

const SppManagerAutoTestComposition: any[] = [
  {
    'name' : 'SppClientAutoTestManager' ,
    'detail' : 'SppClientManager自动测试' ,
    'url' : 'pages/subAutoTestManager/subSppAutoTestManager/sppClientAutoTestManager' ,
    'image' : $r( "app.media.AutoTestIcon" ) ,
    'category' : CategoryManager.SppAutoTestManager
  },
  {
    'name' : 'SppServerAutoTest' ,
    'detail' : 'SppServer自动测试' ,
    'url' : 'pages/subAutoTest/subSppAutoTest/sppServerAutoTest' ,
    'image' : $r( "app.media.AutoTestIcon" ) ,
    'category' : CategoryManager.SppAutoTestManager
  },
]

const ProfileManagerAutoTestComposition: any[] = [
  {
    'name' : 'A2dpSourceAutoTestManager' ,
    'detail' : 'A2dpSourceManager自动测试' ,
    'url' : 'pages/subAutoTestManager/subProfileAutoTestManager/a2dpSourceAutoTestManager' ,
    'image' : $r( "app.media.AutoTestIcon" ) ,
    'category' : CategoryManager.ProfileAutoTestManager
  },
  {
    'name' : 'HFAudioGatewayAutoTestManager' ,
    'detail' : 'HFAudioGatewayManager自动测试' ,
    'url' : 'pages/subAutoTestManager/subProfileAutoTestManager/handsFreeAudioGatewayAutoTestManager' ,
    'image' : $r( "app.media.AutoTestIcon" ) ,
    'category' : CategoryManager.ProfileAutoTestManager
  },
  {
    'name' : 'HitHostAutoTestManager' ,
    'detail' : 'HitHostManager自动测试' ,
    'url' : 'pages/subAutoTestManager/subProfileAutoTestManager/hitHostAutoTestManager' ,
    'image' : $r( "app.media.AutoTestIcon" ) ,
    'category' : CategoryManager.ProfileAutoTestManager
  },
  {
    'name' : 'PanNetworkAutoTestManager' ,
    'detail' : 'PanNetworkManager自动测试' ,
    'url' : 'pages/subAutoTestManager/subProfileAutoTestManager/panNetworkAutoTestManager' ,
    'image' : $r( "app.media.AutoTestIcon" ) ,
    'category' : CategoryManager.ProfileAutoTestManager
  },
]


export function initBRManagerApiData(): Array<TestApiManager> {
  let TestApiArray: Array<TestApiManager> = []
  BRManagerTestCase.forEach( item => {
    TestApiArray.push( new TestApiManager(item.name , item.detail , item.method , item.api , item.result , item.category) );
  } )
  return TestApiArray;
}

export function initBleManagerApiData(): Array<TestApiManager> {
  let TestApiArray: Array<TestApiManager> = []
  BleManagerTestCase.forEach( item => {
    TestApiArray.push( new TestApiManager(item.name , item.detail , item.method , item.api , item.result , item.category) );
  } )
  return TestApiArray;
}

export function initGattManagerApiTestData(): Array<TestDataManager> {
  let TestDataArray: Array<TestDataManager> = []
  GattManagerTestComposition.forEach( item => {
    TestDataArray.push( new TestDataManager(item.name , item.detail , item.url , item.image , item.category) );
  } )
  return TestDataArray;
}

export function initSppManagerApiTestData(): Array<TestDataManager> {
  let TestDataArray: Array<TestDataManager> = []
  SppManagerTestComposition.forEach( item => {
    TestDataArray.push( new TestDataManager(item.name , item.detail , item.url , item.image , item.category) );
  } )
  return TestDataArray;
}

export function initGattClientManagerApiData(): Array<TestApiManager> {
  let TestApiArray: Array<TestApiManager> = []
  GattClientManagerTestCase.forEach( item => {
    TestApiArray.push( new TestApiManager(item.name , item.detail , item.method , item.api , item.result , item.category) );
  } )
  return TestApiArray;
}

export function initGattServerManagerApiData(): Array<TestApiManager> {
  let TestApiArray: Array<TestApiManager> = []
  GattServerManagerTestCase.forEach( item => {
    TestApiArray.push( new TestApiManager(item.name , item.detail , item.method , item.api , item.result , item.category) );
  } )
  return TestApiArray;
}

export function initSppClientManagerApiData(): Array<TestApiManager> {
  let TestApiArray: Array<TestApiManager> = []
  SppClientManagerTestCase.forEach( item => {
    TestApiArray.push( new TestApiManager(item.name , item.detail , item.method , item.api , item.result , item.category) );
  } )
  return TestApiArray;
}

export function initSppServerManagerApiData(): Array<TestApiManager> {
  let TestApiArray: Array<TestApiManager> = []
  SppServerManagerTestCase.forEach( item => {
    TestApiArray.push( new TestApiManager(item.name , item.detail , item.method , item.api , item.result , item.category) );
  } )
  return TestApiArray;
}

export function initA2dpSourceProfileManagerApiData(): Array<TestApiManager> {
  let TestApiArray: Array<TestApiManager> = []
  A2dpSourceProfileManagerTestCase.forEach( item => {
    TestApiArray.push( new TestApiManager(item.name , item.detail , item.method , item.api , item.result , item.category) );
  } )
  return TestApiArray;
}

export function initHandsFreeAudioGatewayProfileManagerApiData(): Array<TestApiManager> {
  let TestApiArray: Array<TestApiManager> = []
  HandsFreeAudioGatewayProfileManagerTestCase.forEach( item => {
    TestApiArray.push( new TestApiManager(item.name , item.detail , item.method , item.api , item.result , item.category) );
  } )
  return TestApiArray;
}

export function initHitHostProfileManagerApiData(): Array<TestApiManager> {
  let TestApiArray: Array<TestApiManager> = []
  HitHostProfileManagerTestCase.forEach( item => {
    TestApiArray.push( new TestApiManager(item.name , item.detail , item.method , item.api , item.result , item.category) );
  } )
  return TestApiArray;
}

export function initPanNetworkProfileManagerApiData(): Array<TestApiManager> {
  let TestApiArray: Array<TestApiManager> = []
  PanNetworkProfileManagerTestCase.forEach( item => {
    TestApiArray.push( new TestApiManager(item.name , item.detail , item.method , item.api , item.result , item.category) );
  } )
  return TestApiArray;
}

export function initGattManagerAutoTestData(): Array<TestDataManager> {
  let TestDataArray: Array<TestDataManager> = []
  GattManagerAutoTestComposition.forEach( item => {
    TestDataArray.push( new TestDataManager(item.name , item.detail , item.url , item.image , item.category) );
  } )
  return TestDataArray;
}

export function initSppManagerAutoTestData(): Array<TestDataManager> {
  let TestDataArray: Array<TestDataManager> = []
  SppManagerAutoTestComposition.forEach( item => {
    TestDataArray.push( new TestDataManager(item.name , item.detail , item.url , item.image , item.category) );
  } )
  return TestDataArray;
}

export function initProfileManagerAutoTestData(): Array<TestDataManager> {
  let TestDataArray: Array<TestDataManager> = []
  ProfileManagerAutoTestComposition.forEach( item => {
    TestDataArray.push( new TestDataManager(item.name , item.detail , item.url , item.image , item.category) );
  } )
  return TestDataArray;
}