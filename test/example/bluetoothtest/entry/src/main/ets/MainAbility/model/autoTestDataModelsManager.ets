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
  TestAutoManager ,
  AutoCategoryManager
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
  subscribeBluetoothDeviceFindManager ,
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
} from './brManagerInterface'
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
import BluetoothModelManager from './BluetoothModelManager'

export const BrManagerAutoTestCase: [] = [
  {
    'name' : '注册状态变化Manager' ,
    'detail' : '注册状态变化' ,
    'method' : 'on.stateChangeManager' ,
    'api' : testOnStateChangeManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.BrAutoManager
  },
  {
    'name' : '注册pin码变化Manager' ,
    'detail' : '注册pin码变化' ,
    'method' : 'on.pinRequiredManager' ,
    'api' : testOnPinRequiredManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.BrAutoManager
  },
  {
    'name' : '注册绑定状态变化Manager' ,
    'detail' : '注册绑定状态变化' ,
    'method' : 'on.bondStateChange' ,
    'api' : testOnBondStateChangeManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.BrAutoManager
  },
  {
    'name' : '开蓝牙Manager' ,
    'detail' : '使能蓝牙' ,
    'method' : 'enableBluetoothManager' ,
    'api' : testEnableBluetoothManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.BrAutoManager
  },
  {
    'name' : '获取状态Manager' ,
    'detail' : '获取蓝牙状态' ,
    'method' : 'getStateManager' ,
    'api' : testGetStateManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.BrAutoManager
  },
  {
    'name' : '获取当前连接状态Manager' ,
    'detail' : '获取当前蓝牙连接状态' ,
    'method' : 'getBtConnectionStateManager' ,
    'api' : testGetBtConnectionStateManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.BrAutoManager
  },
  {
    'name' : '设置本地名称Manager' ,
    'detail' : '设置蓝牙本地名称' ,
    'method' : 'setLocalNameManager' ,
    'api' : testSetLocalNameManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.BrAutoManager
  },
  {
    'name' : '获取本地名称Manager' ,
    'detail' : '获取蓝牙本地名称' ,
    'method' : 'getLocalNameManager' ,
    'api' : testGetLocalNameManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.BrAutoManager
  },
  {
    'name' : '设置扫描模式Manager' ,
    'detail' : '设置蓝牙扫描模式' ,
    'method' : 'setBluetoothScanModeManager' ,
    'api' : testSetBluetoothScanModeManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.BrAutoManager
  },
  {
    'name' : '获取扫描模式Manager' ,
    'detail' : '获取蓝牙扫描模式' ,
    'method' : 'getBluetoothScanModeManager' ,
    'api' : testGetBluetoothScanModeManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.BrAutoManager
  },
  {
    'name' : '订阅蓝牙发现Manager' ,
    'detail' : '订阅发现蓝牙设备' ,
    'method' : 'subscribeBluetoothDeviceFindManager' ,
    'api' : subscribeBluetoothDeviceFindManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.BrAutoManager
  },
  {
    'name' : '开始蓝牙发现Manager' ,
    'detail' : '开始发现' ,
    'method' : 'startBluetoothDiscoveryManager' ,
    'api' : BluetoothModelManager.startBluetoothDiscovery ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.BrAutoManager
  },
  {
    'name' : '停止蓝牙发现Manager' ,
    'detail' : '停止发现' ,
    'method' : 'stopBluetoothDiscoveryManager' ,
    'api' : testStopBluetoothDiscoveryManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.BrAutoManager
  },
  {
    'name' : '配对设备Manager' ,
    'detail' : '进行蓝牙设备配对' ,
    'method' : 'pairDeviceManager' ,
    'api' : testPairDeviceManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.BrAutoManager
  },
  {
    'name' : '获取远程设备名称Manager' ,
    'detail' : '获取配对蓝牙设备的名称' ,
    'method' : 'getDeviceNameManager' ,
    'api' : testGetRemoteDeviceNameManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.BrAutoManager
  },
  {
    'name' : '获取远程设备类型Manager' ,
    'detail' : '获取配对蓝牙设备的类型' ,
    'method' : 'getDeviceTypeManager' ,
    'api' : testGetRemoteDeviceClassManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.BrAutoManager
  },
  {
    'name' : '获取配对设备Manager' ,
    'detail' : '获取配对蓝牙设备的信息' ,
    'method' : 'getPairedDeviceIdsManager' ,
    'api' : testGetPairedDevicesManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.BrAutoManager
  },
  {
    'name' : '取消配对设备Manager' ,
    'detail' : '取消蓝牙设备配对' ,
    'method' : 'unpairDeviceManager' ,
    'api' : testUnPairedDeviceManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.BrAutoManager
  },
  /*
    {
    'name': '获取配置文件连接状态Manager1',
    'detail': '获取配置文件的连接状态',
    'method': 'getProfileConnStateManager',
    'api':testGetProfileConnStateManager1,
    'result': 'None',
    'category':AutoCategoryManager.BrAutoManager
    },
    {
    'name': '获取配置文件连接状态Manager2',
    'detail': '获取设备的配置文件状态',
    'method': 'getDeviceStateManager',
    'api':testGetProfileConnStateManager2,
    'result': 'None',
    'category':AutoCategoryManager.BrAutoManager
  },
  */
  {
    'name' : '关蓝牙Manager' ,
    'detail' : '去使能蓝牙' ,
    'method' : 'disableBluetoothManager' ,
    'api' : testDisableBluetoothManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.BrAutoManager
  },
]

export const BleManagerAutoTestCase: [] = [
  {
    'name' : '开蓝牙Manager' ,
    'detail' : '使能蓝牙' ,
    'method' : 'enableBluetoothManager' ,
    'api' : testEnableBluetoothManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.BleAutoManager
  },
  {
    'name' : '获取状态Manager' ,
    'detail' : '获取蓝牙状态' ,
    'method' : 'getStateManager' ,
    'api' : testGetStateManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.BleAutoManager
  },
  {
    'name' : '设置本地名称Manager' ,
    'detail' : '设置蓝牙本地名称' ,
    'method' : 'setLocalNameManager' ,
    'api' : testSetLocalNameManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.BleAutoManager
  },
  {
    'name' : '获取本地名称Manager' ,
    'detail' : '获取蓝牙本地名称' ,
    'method' : 'getLocalNameManager' ,
    'api' : testGetLocalNameManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.BleAutoManager
  },
  {
    'name' : '设置扫描模式Manager' ,
    'detail' : '设置蓝牙扫描模式' ,
    'method' : 'setBluetoothScanModeManager' ,
    'api' : testSetBluetoothScanModeManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.BleAutoManager
  },
  {
    'name' : '获取扫描模式Manager' ,
    'detail' : '获取蓝牙扫描模式' ,
    'method' : 'getBluetoothScanModeManager' ,
    'api' : testGetBluetoothScanModeManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.BleAutoManager
  },
  /*
    {
    'name': '创建Gatt服务器Manager',
    'detail': '创建Gatt服务器',
    'method': 'createGattServerManager',
    'api':testCreateGattServerDevice,
    'result': 'None',
    'category':AutoCategoryManager.BleAutoManager
    },
    {
    'name': '创建Gatt客户端Manager',
    'detail': '创建Gatt客户端',
    'method': 'createGattClientManager',
    'api':testCreateGattClientDevice,
    'result': 'None',
    'category':AutoCategoryManager.BleAutoManager
  },
  */
  {
    'name' : '获取连接的Ble设备Manager' ,
    'detail' : '获取连接的Ble设备信息' ,
    'method' : 'getConnectedBleDevices' ,
    'api' : testGetConnectedBLEDevicesManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.BleAutoManager
  },
  {
    'name' : '注册Ble查找状态变化Manager' ,
    'detail' : '注册Ble设备查找状态变化' ,
    'method' : 'on.BleDeviceFindManager' ,
    'api' : testOnBLEDeviceFindManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.BleAutoManager
  },
  {
    'name' : '开始Ble扫描Manager' ,
    'detail' : '开始扫描Ble设备' ,
    'method' : 'startBleScanManager' ,
    'api' : testStartBLEScanManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.BleAutoManager
  },
  {
    'name' : '停止Ble扫描Manager' ,
    'detail' : '停止扫描' ,
    'method' : 'stopBleScanManager' ,
    'api' : testStopBLEScanManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.BleAutoManager
  },
  {
    'name' : '关蓝牙Manager' ,
    'detail' : '去使能蓝牙' ,
    'method' : 'disableBluetoothManager' ,
    'api' : testDisableBluetoothManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.BleAutoManager
  },
]

export const GattClientManagerAutoTestCase: [] = [
  {
    'name' : '开蓝牙Manager' ,
    'detail' : '使能蓝牙' ,
    'method' : 'enableBluetoothManager' ,
    'api' : testEnableBluetoothManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattClientAutoManager
  },
  {
    'name' : '获取状态Manager' ,
    'detail' : '获取蓝牙状态' ,
    'method' : 'getStateManager' ,
    'api' : testGetStateManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattClientAutoManager
  },
  {
    'name' : '设置本地名称Manager' ,
    'detail' : '设置蓝牙本地名称' ,
    'method' : 'setLocalNameManager' ,
    'api' : testSetLocalNameManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattClientAutoManager
  },
  {
    'name' : '获取本地名称Manager' ,
    'detail' : '获取蓝牙本地名称' ,
    'method' : 'getLocalNameManager' ,
    'api' : testGetLocalNameManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattClientAutoManager
  },
  {
    'name' : '设置扫描模式Manager' ,
    'detail' : '设置蓝牙扫描模式' ,
    'method' : 'setBluetoothScanModeManager' ,
    'api' : testSetBluetoothScanModeManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattClientAutoManager
  },
  {
    'name' : '获取扫描模式Manager' ,
    'detail' : '获取蓝牙扫描模式' ,
    'method' : 'getBluetoothScanModeManager' ,
    'api' : testGetBluetoothScanModeManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattClientAutoManager
  },
  {
    'name' : '创造Gatt客户端Manager' ,
    'detail' : '创造Gatt客户端' ,
    'method' : 'createGattClientManager' ,
    'api' : testCreateGattClientManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattClientAutoManager
  },
  {
    'name' : 'Ble连接状态Manager' ,
    'detail' : '查看Ble连接状态' ,
    'method' : 'on.BleConnectionStateChangeManager' ,
    'api' : testOnBLEConnectionStateChangeManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattClientAutoManager
  },
  {
    'name' : '连接Manager' ,
    'detail' : 'Gatt连接' ,
    'method' : 'connectManager' ,
    'api' : testConnectManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattClientAutoManager
  },
  {
    'name' : '获取服务callbackManager' ,
    'detail' : '获取服务callback' ,
    'method' : 'getServices(Callback)Manager' ,
    'api' : testGetServicesCallbackManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattClientAutoManager
  },
  {
    'name' : '获取服务promiseManager' ,
    'detail' : '获取服务promise' ,
    'method' : 'getServices(promise)Manager' ,
    'api' : testGetServicesPromiseManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattClientAutoManager
  },
  {
    'name' : '读取特征值callbackManager' ,
    'detail' : '读取特征值callback' ,
    'method' : 'readCharacteristicValue(callback)Manager' ,
    'api' : testReadCharacteristicValueCallbackManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattClientAutoManager
  },
  {
    'name' : '读取特征值promiseManager' ,
    'detail' : '读取特征值promise' ,
    'method' : 'readCharacteristicValue(promise)Manager' ,
    'api' : testReadCharacteristicValuePromiseManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattClientAutoManager
  },
  {
    'name' : '读取描述符值callbackManager' ,
    'detail' : '读取描述符值callback' ,
    'method' : 'readDescriptorValue(callback)Manager' ,
    'api' : testReadDescriptorValueCallbackManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattClientAutoManager
  },
  {
    'name' : '读取描述符值promiseManager' ,
    'detail' : '读取描述符值promise' ,
    'method' : 'readDescriptorValue(promise)Manager' ,
    'api' : testReadDescriptorValuePromiseManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattClientAutoManager
  },
  {
    'name' : '写入特征值Manager' ,
    'detail' : '写入特征值' ,
    'method' : 'writeCharacteristicValueManager' ,
    'api' : testWriteCharacteristicValueManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattClientAutoManager
  },
  {
    'name' : '写入描述符值Manager' ,
    'detail' : '写入描述符值' ,
    'method' : 'writeDescriptorValueManager' ,
    'api' : testWriteDescriptorValueManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattClientAutoManager
  },
  {
    'name' : 'Ble特征值变化Manager' ,
    'detail' : '查看Ble特征值变化' ,
    'method' : 'on.BleCharacteristicChangeManager' ,
    'api' : testOnBLECharacteristicChangeManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattClientAutoManager
  },
  {
    'name' : '设定BleMtu尺寸Manager' ,
    'detail' : '设定BleMtu尺寸' ,
    'method' : 'setBleMtuSizeManager' ,
    'api' : testSetBLEMtuSizeManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattClientAutoManager
  },
  {
    'name' : '设置通知特性更改Manager' ,
    'detail' : '设置通知特性更改' ,
    'method' : 'setNotifyCharacteristicChangedManager' ,
    'api' : testSetNotifyCharacteristicChangedManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattClientAutoManager
  },
  {
    'name' : '获取设备名称callbackManager' ,
    'detail' : '获取设备名称callback' ,
    'method' : 'getDeviceName(callback)Manager' ,
    'api' : testGetDeviceNameCallbackManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattClientAutoManager
  },
  {
    'name' : '获取设备名称promiseManager' ,
    'detail' : '获取设备名称promise' ,
    'method' : 'getDeviceName(Promise)Manager' ,
    'api' : testGetDeviceNamePromiseManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattClientAutoManager
  },
  {
    'name' : '获取Rssi值callbackManager' ,
    'detail' : '获取Rssi值callback' ,
    'method' : 'getRssiValue(Callback)Manager' ,
    'api' : testGetRssiValueCallbackManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattClientAutoManager
  },
  {
    'name' : '获取Rssi值promiseManager' ,
    'detail' : '获取Rssi值promise' ,
    'method' : 'getRssiValue(Promise)Manager' ,
    'api' : testGetRssiValuePromiseManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattClientAutoManager
  },
  {
    'name' : '取消连接Manager' ,
    'detail' : '取消Gatt连接' ,
    'method' : 'disconnectManager' ,
    'api' : testDisconnectManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattClientAutoManager
  },
  {
    'name' : '关闭Manager' ,
    'detail' : '关闭Gatt客户端' ,
    'method' : 'closeManager' ,
    'api' : testGattClientCloseManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattClientAutoManager
  },
  {
    'name' : '关蓝牙Manager' ,
    'detail' : '去使能蓝牙' ,
    'method' : 'disableBluetoothManager' ,
    'api' : testDisableBluetoothManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattClientAutoManager
  },
]

export const GattServerManagerAutoTestCase: [] = [
  {
    'name' : '开蓝牙Manager' ,
    'detail' : '使能蓝牙' ,
    'method' : 'enableBluetoothManager' ,
    'api' : testEnableBluetoothManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattServerAutoManager
  },
  {
    'name' : '获取状态Manager' ,
    'detail' : '获取蓝牙状态' ,
    'method' : 'getStateManager' ,
    'api' : testGetStateManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattServerAutoManager
  },
  {
    'name' : '设置本地名称Manager' ,
    'detail' : '设置蓝牙本地名称' ,
    'method' : 'setLocalNameManager' ,
    'api' : testSetLocalNameManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattServerAutoManager
  },
  {
    'name' : '获取本地名称Manager' ,
    'detail' : '获取蓝牙本地名称' ,
    'method' : 'getLocalNameManager' ,
    'api' : testGetLocalNameManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattServerAutoManager
  },
  {
    'name' : '设置扫描模式Manager' ,
    'detail' : '设置蓝牙扫描模式' ,
    'method' : 'setBluetoothScanModeManager' ,
    'api' : testSetBluetoothScanModeManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattServerAutoManager
  },
  {
    'name' : '获取扫描模式Manager' ,
    'detail' : '获取蓝牙扫描模式' ,
    'method' : 'getBluetoothScanModeManager' ,
    'api' : testGetBluetoothScanModeManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattServerAutoManager
  },
  {
    'name' : '创建Gatt服务器Manager' ,
    'detail' : '创建Gatt服务器' ,
    'method' : 'createGattServerManager' ,
    'api' : testCreateGattServerManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattServerAutoManager
  },
  {
    'name' : '添加服务Manager' ,
    'detail' : '添加Gatt服务' ,
    'method' : 'addServiceManager' ,
    'api' : testAddServiceManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattServerAutoManager
  },
  {
    'name' : '开始广播Manager' ,
    'detail' : '开始广播播报' ,
    'method' : 'startAdvertisingManager' ,
    'api' : testStartAdvertisingManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattServerAutoManager
  },
  {
    'name' : '连接状态Manager' ,
    'detail' : '查看Gatt连接状态' ,
    'method' : 'on.connectStateChangeManager' ,
    'api' : testOnConnectStateChangeManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattServerAutoManager
  },
  {
    'name' : '读取特征值Manager' ,
    'detail' : '读取特征值' ,
    'method' : 'on.characteristicReadManager' ,
    'api' : testOnCharacteristicReadManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattServerAutoManager
  },
  {
    'name' : '写入特征值Manager' ,
    'detail' : '写入特征值' ,
    'method' : 'on.characteristicWriteManager' ,
    'api' : testOnCharacteristicWriteManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattServerAutoManager
  },
  {
    'name' : '读取描述符Manager' ,
    'detail' : '读取描述符' ,
    'method' : 'on.descriptorReadManager' ,
    'api' : testOnDescriptorReadManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattServerAutoManager
  },
  {
    'name' : '写入描述符Manager' ,
    'detail' : '写入描述符' ,
    'method' : 'on.descriptorWriteManager' ,
    'api' : testOnDescriptorWriteManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattServerAutoManager
  },
  {
    'name' : '停止广播Manager' ,
    'detail' : '停止广播播报' ,
    'method' : 'stopAdvertisingManager' ,
    'api' : testStopAdvertisingManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattServerAutoManager
  },
  {
    'name' : '删除服务Manager' ,
    'detail' : '删除Gatt服务' ,
    'method' : 'removeServiceManager' ,
    'api' : testRemoveServiceManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattServerAutoManager
  },
  {
    'name' : '关闭Manager' ,
    'detail' : '关闭Gatt服务器' ,
    'method' : 'closeManager' ,
    'api' : testGattServerCloseManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattServerAutoManager
  },
  {
    'name' : '关蓝牙Manager' ,
    'detail' : '去使能蓝牙' ,
    'method' : 'disableBluetoothManager' ,
    'api' : testDisableBluetoothManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.GattServerAutoManager
  },
]

export const SppClientManagerAutoTestCase: [] = [
  {
    'name' : '开蓝牙' ,
    'detail' : '使能蓝牙' ,
    'method' : 'enableBluetooth' ,
    'api' : testEnableBluetoothManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.SppClientAutoManager
  },
  {
    'name' : '设置SPP扫描模式' ,
    'detail' : '设置SPP扫描模式' ,
    'method' : 'setBluetoothScanMode' ,
    'api' : testSetBluetoothScanModeManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.SppClientAutoManager
  },
  {
    'name' : '设置本地名称' ,
    'detail' : '设置蓝牙本地名称' ,
    'method' : 'setLocalName' ,
    'api' : testSetLocalNameManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.SppClientAutoManager
  },
  {
    'name' : '获取本地名称' ,
    'detail' : '获取蓝牙本地名称' ,
    'method' : 'getLocalName' ,
    'api' : testGetLocalNameManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.SppClientAutoManager
  },
  {
    'name' : '获取状态' ,
    'detail' : '获取蓝牙状态' ,
    'method' : 'getState' ,
    'api' : testGetStateManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.SppClientAutoManager
  },
  {
    'name' : '获取扫描模式' ,
    'detail' : '获取蓝牙扫描模式' ,
    'method' : 'getBluetoothScanMode' ,
    'api' : testGetBluetoothScanModeManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.SppClientAutoManager
  },
  {
    'name' : 'Spp客户端连接(安全模式)' ,
    'detail' : 'Spp客户端(安全模式)连接' ,
    'method' : 'sppConnect' ,
    'api' : testSppClientConnectSafeManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.SppClientAutoManager
  },
  {
    'name' : 'Spp客户端连接(非安全模式)' ,
    'detail' : 'Spp客户端(非安全模式)连接' ,
    'method' : 'sppConnect' ,
    'api' : testSppClientConnectUnSafeManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.SppClientAutoManager
  },
  {
    'name' : '配对设备' ,
    'detail' : '进行蓝牙设备配对' ,
    'method' : 'pairDevice' ,
    'api' : testPairDeviceManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.SppClientAutoManager
  },
  {
    'name' : 'Spp客户端写入' ,
    'detail' : 'Spp客户端写入' ,
    'method' : 'sppWrite' ,
    'api' : testSppClientWriteManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.SppClientAutoManager
  },
  {
    'name' : 'Spp客户端读取打开' ,
    'detail' : 'Spp客户端读取打开' ,
    'method' : 'on.sppRead' ,
    'api' : testSppClientReadOnManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.SppClientAutoManager
  },
  {
    'name' : 'Spp客户端读取关闭' ,
    'detail' : 'Spp客户端读取关闭' ,
    'method' : 'off.sppRead' ,
    'api' : testSppClientReadOffManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.SppClientAutoManager
  },
  {
    'name' : 'Spp客户端关闭' ,
    'detail' : 'Spp客户端关闭' ,
    'method' : 'sppCloseClientSocket' ,
    'api' : testSppClientCloseManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.SppClientAutoManager
  },
  {
    'name' : '关蓝牙' ,
    'detail' : '去使能蓝牙' ,
    'method' : 'disableBluetooth' ,
    'api' : testDisableBluetoothManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.SppClientAutoManager
  },
]

export const SppServerManagerAutoTestCase: [] = [
  {
    'name' : '开蓝牙Manager' ,
    'detail' : '使能蓝牙' ,
    'method' : 'enableBluetoothManager' ,
    'api' : testEnableBluetoothManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.SppServerAutoManager
  },
  {
    'name' : '设置SPP扫描模式Manager' ,
    'detail' : '设置SPP扫描模式' ,
    'method' : 'setBluetoothScanModeManager' ,
    'api' : testSetBluetoothScanModeManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.SppServerAutoManager
  },
  {
    'name' : '注册pin码变化Manager' ,
    'detail' : '注册pin码变化' ,
    'method' : 'on.pinRequiredManager' ,
    'api' : testOnPinRequiredManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.SppServerAutoManager
  },
  {
    'name' : '设置本地名称Manager' ,
    'detail' : '设置蓝牙本地名称' ,
    'method' : 'setLocalNameManager' ,
    'api' : testSetLocalNameManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.SppServerAutoManager
  },
  {
    'name' : '获取本地名称Manager' ,
    'detail' : '获取蓝牙本地名称' ,
    'method' : 'getLocalNameManager' ,
    'api' : testGetLocalNameManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.SppServerAutoManager
  },
  {
    'name' : '获取状态Manager' ,
    'detail' : '获取蓝牙状态' ,
    'method' : 'getStateManager' ,
    'api' : testGetStateManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.SppServerAutoManager
  },
  {
    'name' : '获取扫描模式Manager' ,
    'detail' : '获取蓝牙扫描模式' ,
    'method' : 'getBluetoothScanModeManager' ,
    'api' : testGetBluetoothScanModeManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.SppServerAutoManager
  },
  {
    'name' : 'Spp服务器创建(安全模式)Manager' ,
    'detail' : 'Spp服务器(安全模式)创建' ,
    'method' : 'sppListenManager' ,
    'api' : testSppServerCreateSafeManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.SppServerAutoManager
  },
  {
    'name' : 'Spp服务器创建(非安全模式)Manager' ,
    'detail' : 'Spp服务器(非安全模式)创建' ,
    'method' : 'sppListenManager' ,
    'api' : testSppServerCreateUnSafeManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.SppServerAutoManager
  },
  {
    'name' : 'Spp服务器写入Manager' ,
    'detail' : 'Spp服务器写入' ,
    'method' : 'sppWriteManager' ,
    'api' : testSppServerWriteManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.SppServerAutoManager
  },
  {
    'name' : 'Spp服务器接受Manager' ,
    'detail' : 'Spp服务器接受' ,
    'method' : 'sppAcceptManager' ,
    'api' : testSppServerAcceptManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.SppServerAutoManager
  },
  {
    'name' : 'Spp服务器读取打开Manager' ,
    'detail' : 'Spp服务器读取打开' ,
    'method' : 'on.sppReadManager' ,
    'api' : testSppServerReadOnManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.SppServerAutoManager
  },
  {
    'name' : 'Spp服务器读取关闭Manager' ,
    'detail' : 'Spp服务器读取关闭' ,
    'method' : 'off.sppReadManager' ,
    'api' : testSppServerReadOffManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.SppServerAutoManager
  },
  {
    'name' : 'Spp服务器关闭Manager' ,
    'detail' : 'Spp服务器关闭' ,
    'method' : 'sppCloseServerSocketManager' ,
    'api' : testSppCloseServerSocketManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.SppServerAutoManager
  },
  {
    'name' : 'Spp客户端关闭Manager' ,
    'detail' : 'Spp客户端关闭' ,
    'method' : 'sppCloseClientSocketManager' ,
    'api' : testSppCloseClientSocketManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.SppServerAutoManager
  },
  {
    'name' : '关蓝牙Manager' ,
    'detail' : '去使能蓝牙' ,
    'method' : 'disableBluetoothManager' ,
    'api' : testDisableBluetoothManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.SppServerAutoManager
  },
]

export const A2dpSourceProfileManagerAutoTestCase: [] = [
  {
    'name' : '开蓝牙Manager' ,
    'detail' : '使能蓝牙' ,
    'method' : 'enableBluetoothManager' ,
    'api' : testEnableBluetoothManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.A2dpSourceAutoManager
  },
  {
    'name' : '获取状态Manager' ,
    'detail' : '获取蓝牙状态' ,
    'method' : 'getStateManager' ,
    'api' : testGetStateManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.A2dpSourceAutoManager
  },
  {
    'name' : '设置本地名称Manager' ,
    'detail' : '设置蓝牙本地名称' ,
    'method' : 'setLocalNameManager' ,
    'api' : testSetLocalNameManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.A2dpSourceAutoManager
  },
  {
    'name' : '获取本地名称Manager' ,
    'detail' : '获取蓝牙本地名称' ,
    'method' : 'getLocalNameManager' ,
    'api' : testGetLocalNameManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.A2dpSourceAutoManager
  },
  {
    'name' : '设置扫描模式Manager' ,
    'detail' : '设置蓝牙扫描模式' ,
    'method' : 'setBluetoothScanModeManager' ,
    'api' : testSetBluetoothScanModeManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.A2dpSourceAutoManager
  },
  {
    'name' : '获取扫描模式Manager' ,
    'detail' : '获取蓝牙扫描模式' ,
    'method' : 'getBluetoothScanModeManager' ,
    'api' : testGetBluetoothScanModeManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.A2dpSourceAutoManager
  },
  {
    'name' : '连接Manager' ,
    'detail' : 'Profile连接' ,
    'method' : 'connectManager' ,
    'api' : testProfileConnectManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.A2dpSourceAutoManager
  },
  {
    'name' : '获取播放状态Manager' ,
    'detail' : '获取Profile播放状态' ,
    'method' : 'getPlayingStateManager' ,
    'api' : testProfileGetPlayingStateManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.A2dpSourceAutoManager
  },
  {
    'name' : '获取连接设备Manager' ,
    'detail' : '获取Profile连接设备' ,
    'method' : 'getConnectionDevicesManager' ,
    'api' : testProfileGetConnectionDevicesManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.A2dpSourceAutoManager
  },
  {
    'name' : '获取设备状态Manager' ,
    'detail' : '获取Profile设备状态' ,
    'method' : 'getDeviceStateManager' ,
    'api' : testProfileGetDeviceStateManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.A2dpSourceAutoManager
  },
  {
    'name' : '取消连接Manager' ,
    'detail' : '取消Profile连接' ,
    'method' : 'disconnectManager' ,
    'api' : testProfileDisConnectManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.A2dpSourceAutoManager
  },
]

export const HandsFreeAudioGatewayProfileManagerAutoTestCase: [] = [
  {
    'name' : '开蓝牙Manager' ,
    'detail' : '使能蓝牙' ,
    'method' : 'enableBluetoothManager' ,
    'api' : testEnableBluetoothManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.HandsFreeAudioGatewayAutoManager
  },
  {
    'name' : '获取状态Manager' ,
    'detail' : '获取蓝牙状态' ,
    'method' : 'getStateManager' ,
    'api' : testGetStateManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.HandsFreeAudioGatewayAutoManager
  },
  {
    'name' : '设置本地名称Manager' ,
    'detail' : '设置蓝牙本地名称' ,
    'method' : 'setLocalNameManager' ,
    'api' : testSetLocalNameManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.HandsFreeAudioGatewayAutoManager
  },
  {
    'name' : '获取本地名称Manager' ,
    'detail' : '获取蓝牙本地名称' ,
    'method' : 'getLocalNameManager' ,
    'api' : testGetLocalNameManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.HandsFreeAudioGatewayAutoManager
  },
  {
    'name' : '设置扫描模式Manager' ,
    'detail' : '设置蓝牙扫描模式' ,
    'method' : 'setBluetoothScanModeManager' ,
    'api' : testSetBluetoothScanModeManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.HandsFreeAudioGatewayAutoManager
  },
  {
    'name' : '获取扫描模式Manager' ,
    'detail' : '获取蓝牙扫描模式' ,
    'method' : 'getBluetoothScanModeManager' ,
    'api' : testGetBluetoothScanModeManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.HandsFreeAudioGatewayAutoManager
  },
  {
    'name' : '连接Manager' ,
    'detail' : 'Profile连接' ,
    'method' : 'connectManager' ,
    'api' : testProfileConnectManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.HandsFreeAudioGatewayAutoManager
  },
  {
    'name' : '获取连接设备Manager' ,
    'detail' : '获取Profile连接设备' ,
    'method' : 'getConnectionDevicesManager' ,
    'api' : testProfileGetConnectionDevicesManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.HandsFreeAudioGatewayAutoManager
  },
  {
    'name' : '获取设备状态Manager' ,
    'detail' : '获取Profile设备状态' ,
    'method' : 'getDeviceStateManager' ,
    'api' : testProfileGetDeviceStateManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.HandsFreeAudioGatewayAutoManager
  },
  {
    'name' : '取消连接Manager' ,
    'detail' : '取消Profile连接' ,
    'method' : 'disconnectManager' ,
    'api' : testProfileDisConnectManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.HandsFreeAudioGatewayAutoManager
  },
]

export const HitHostProfileManagerAutoTestCase: [] = [
  {
    'name' : '开蓝牙Manager' ,
    'detail' : '使能蓝牙' ,
    'method' : 'enableBluetoothManager' ,
    'api' : testEnableBluetoothManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.HitHostAutoManager
  },
  {
    'name' : '获取状态Manager' ,
    'detail' : '获取蓝牙状态' ,
    'method' : 'getStateManager' ,
    'api' : testGetStateManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.HitHostAutoManager
  },
  {
    'name' : '设置本地名称Manager' ,
    'detail' : '设置蓝牙本地名称' ,
    'method' : 'setLocalNameManager' ,
    'api' : testSetLocalNameManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.HitHostAutoManager
  },
  {
    'name' : '获取本地名称Manager' ,
    'detail' : '获取蓝牙本地名称' ,
    'method' : 'getLocalNameManager' ,
    'api' : testGetLocalNameManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.HitHostAutoManager
  },
  {
    'name' : '设置扫描模式Manager' ,
    'detail' : '设置蓝牙扫描模式' ,
    'method' : 'setBluetoothScanModeManager' ,
    'api' : testSetBluetoothScanModeManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.HitHostAutoManager
  },
  {
    'name' : '获取扫描模式Manager' ,
    'detail' : '获取蓝牙扫描模式' ,
    'method' : 'getBluetoothScanModeManager' ,
    'api' : testGetBluetoothScanModeManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.HitHostAutoManager
  },
  {
    'name' : '连接Manager' ,
    'detail' : 'Profile连接' ,
    'method' : 'connectManager' ,
    'api' : testProfileConnectManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.HitHostAutoManager
  },
  {
    'name' : '获取连接设备Manager' ,
    'detail' : '获取Profile连接设备' ,
    'method' : 'getConnectionDevicesManager' ,
    'api' : testProfileGetConnectionDevicesManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.HitHostAuto
  },
  {
    'name' : '获取设备状态Manager' ,
    'detail' : '获取Profile设备状态' ,
    'method' : 'getDeviceStateManager' ,
    'api' : testProfileGetDeviceStateManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.HitHostAutoManager
  },
  {
    'name' : '取消连接Manager' ,
    'detail' : '取消Profile连接' ,
    'method' : 'disconnectManager' ,
    'api' : testProfileDisConnectManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.HitHostAutoManager
  },
]

export const PanNetworkProfileManagerAutoTestCase: [] = [
  {
    'name' : '开蓝牙Manager' ,
    'detail' : '使能蓝牙' ,
    'method' : 'enableBluetoothManager' ,
    'api' : testEnableBluetoothManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.PanNetworkAutoManager
  },
  {
    'name' : '获取状态Manager' ,
    'detail' : '获取蓝牙状态' ,
    'method' : 'getStateManager' ,
    'api' : testGetStateManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.PanNetworkAutoManager
  },
  {
    'name' : '设置本地名称Manager' ,
    'detail' : '设置蓝牙本地名称' ,
    'method' : 'setLocalNameManager' ,
    'api' : testSetLocalNameManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.PanNetworkAutoManager
  },
  {
    'name' : '获取本地名称Manager' ,
    'detail' : '获取蓝牙本地名称' ,
    'method' : 'getLocalNameManager' ,
    'api' : testGetLocalNameManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.PanNetworkAutoManager
  },
  {
    'name' : '设置扫描模式Manager' ,
    'detail' : '设置蓝牙扫描模式' ,
    'method' : 'setBluetoothScanModeManager' ,
    'api' : testSetBluetoothScanModeManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.PanNetworkAutoManager
  },
  {
    'name' : '获取扫描模式Manager' ,
    'detail' : '获取蓝牙扫描模式' ,
    'method' : 'getBluetoothScanModeManager' ,
    'api' : testGetBluetoothScanModeManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.PanNetworkAutoManager
  },
  {
    'name' : '获取连接设备Manager' ,
    'detail' : '获取Profile连接设备' ,
    'method' : 'getConnectionDevicesManager' ,
    'api' : testProfileGetConnectionDevicesManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.PanNetworkAutoManager
  },
  {
    'name' : '获取设备状态Manager' ,
    'detail' : '获取Profile设备状态' ,
    'method' : 'getDeviceStateManager' ,
    'api' : testProfileGetDeviceStateManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.PanNetworkAutoManager
  },
  {
    'name' : '取消连接Manager' ,
    'detail' : '取消Profile连接' ,
    'method' : 'disconnectManager' ,
    'api' : testProfileDisConnectManager ,
    'result' : 'None' ,
    'category' : AutoCategoryManager.PanNetworkAutoManager
  },
]

export function initBrManagerAutoData(): Array<TestAutoManager> {
  let TestAutoArray: Array<TestAutoManager> = []
  BrManagerAutoTestCase.forEach( item => {
    TestAutoArray.push( new TestAutoManager(item.name , item.detail , item.method , item.api , item.result , item.Category) );
  } )
  return TestAutoArray;
}

export function initBleManagerAutoData(): Array<TestAutoManager> {
  let TestAutoArray: Array<TestAutoManager> = []
  BleManagerAutoTestCase.forEach( item => {
    TestAutoArray.push( new TestAutoManager(item.name , item.detail , item.method , item.api , item.result , item.Category) );
  } )
  return TestAutoArray;
}

export function initGattClientManagerAutoData(): Array<TestAutoManager> {
  let TestAutoArray: Array<TestAutoManager> = []
  GattClientManagerAutoTestCase.forEach( item => {
    TestAutoArray.push( new TestAutoManager(item.name , item.detail , item.method , item.api , item.result , item.Category) );
  } )
  return TestAutoArray;
}

export function initGattServerManagerAutoData(): Array<TestAutoManager> {
  let TestAutoArray: Array<TestAutoManager> = []
  GattServerManagerAutoTestCase.forEach( item => {
    TestAutoArray.push( new TestAutoManager(item.name , item.detail , item.method , item.api , item.result , item.Category) );
  } )
  return TestAutoArray;
}

export function initSppClientManagerAutoData(): Array<TestAutoManager> {
  let TestAutoArray: Array<TestAutoManager> = []
  SppClientManagerAutoTestCase.forEach( item => {
    TestAutoArray.push( new TestAutoManager(item.name , item.detail , item.method , item.api , item.result , item.Category) );
  } )
  return TestAutoArray;
}

export function initSppServerManagerAutoData(): Array<TestAutoManager> {
  let TestAutoArray: Array<TestAutoManager> = []
  SppServerManagerAutoTestCase.forEach( item => {
    TestAutoArray.push( new TestAutoManager(item.name , item.detail , item.method , item.api , item.result , item.Category) );
  } )
  return TestAutoArray;
}

export function initA2dpSourceProfileManagerAutoData(): Array<TestAutoManager> {
  let TestAutoArray: Array<TestAutoManager> = []
  A2dpSourceProfileManagerAutoTestCase.forEach( item => {
    TestAutoArray.push( new TestAutoManager(item.name , item.detail , item.method , item.api , item.result , item.Category) );
  } )
  return TestAutoArray;
}

export function initHandsFreeAudioGatewayProfileManagerAutoData(): Array<TestAutoManager> {
  let TestAutoArray: Array<TestAutoManager> = []
  HandsFreeAudioGatewayProfileManagerAutoTestCase.forEach( item => {
    TestAutoArray.push( new TestAutoManager(item.name , item.detail , item.method , item.api , item.result , item.Category) );
  } )
  return TestAutoArray;
}

export function initHitHostProfileManagerAutoData(): Array<TestAutoManager> {
  let TestAutoArray: Array<TestAutoManager> = []
  HitHostProfileManagerAutoTestCase.forEach( item => {
    TestAutoArray.push( new TestAutoManager(item.name , item.detail , item.method , item.api , item.result , item.Category) );
  } )
  return TestAutoArray;
}

export function initPanNetworkProfileManagerAutoData(): Array<TestAutoManager> {
  let TestAutoArray: Array<TestAutoManager> = []
  PanNetworkProfileManagerAutoTestCase.forEach( item => {
    TestAutoArray.push( new TestAutoManager(item.name , item.detail , item.method , item.api , item.result , item.Category) );
  } )
  return TestAutoArray;
}

