/*
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

import { TestAuto, AutoCategory } from './testData'
const BrAutoTest: any[]  = [
  { 'name': '开蓝牙', 'detail': '使能蓝牙', 'method': 'enableBluetooth', 'result': 'None', 'category': AutoCategory.BrAuto },
  { 'name': '关蓝牙', 'detail': '去使能蓝牙', 'method': 'disableBluetooth', 'result': 'None', 'category': AutoCategory.BrAuto },
  { 'name': '设置状态', 'detail': '设置蓝牙状态', 'method': 'getState', 'result': 'None', 'category': AutoCategory.BrAuto },
  { 'name': '获取当前连接状态', 'detail': '获取当前蓝牙连接状态', 'method': 'getBtConnectionState',  'result': 'None', 'category': AutoCategory.BrAuto },
  { 'name': '设置本地名称', 'detail': '设置蓝牙本地名称', 'method': 'setLocalName', 'result': 'None', 'category': AutoCategory.BrAuto },
  { 'name': '获取本地名称', 'detail': '获取蓝牙本地名称', 'method': 'getLocalName', 'result': 'None', 'category': AutoCategory.BrAuto },
  { 'name': '设置扫描模式', 'detail': '设置蓝牙扫描模式', 'method': 'setBluetoothScanMode', 'result': 'None', 'category': AutoCategory.BrAuto },
  { 'name': '获取扫描模式', 'detail': '获取蓝牙扫描模式', 'method': 'getBluetoothScanMode', 'result': 'None', 'category': AutoCategory.BrAuto },
  { 'name': '开始蓝牙发现', 'detail': '发现可配对蓝牙设备', 'method': 'startBluetoothDiscovery', 'result': 'None', 'category': AutoCategory.BrAuto },
  { 'name': '停止蓝牙发现', 'detail': '停止发现', 'method': 'stopBluetoothDiscovery', 'result': 'None', 'category': AutoCategory.BrAuto },
  { 'name': '配对设备', 'detail': '进行蓝牙设备配对', 'method': 'pairDevice', 'result': 'None', 'category': AutoCategory.BrAuto },
  { 'name': '取消配对设备', 'detail': '取消蓝牙设备配对', 'method': 'unpairDevice', 'result': 'None', 'category': AutoCategory.BrAuto },
  { 'name': '获取远程设备名称', 'detail': '获取配对蓝牙设备的名称', 'method': 'getDeviceName', 'result': 'None', 'category': AutoCategory.BrAuto },
  { 'name': '获取远程设备类型', 'detail': '获取配对蓝牙设备的类型', 'method': 'getDeviceType',  'result': 'None', 'category': AutoCategory.BrAuto },
  { 'name': '获取配对设备', 'detail': '获取配对蓝牙设备的信息', 'method': 'getPairedDeviceIds', 'result': 'None', 'category': AutoCategory.BrAuto },
  { 'name': '获取配置文件连接状态1', 'detail': '获取配置文件的连接状态', 'method': 'getProfileConnState', 'result': 'None', 'category': AutoCategory.BrAuto },
  { 'name': '获取配置文件连接状态2', 'detail': '获取设备的配置文件状态', 'method': 'getDeviceState', 'result': 'None', 'category': AutoCategory.BrAuto },
  { 'name': '注册状态变化', 'detail': '注册状态变化', 'method': 'on.stateChange',  'result': 'None', 'category': AutoCategory.BrAuto },
  { 'name': '注册pin码变化', 'detail': '注册pin码变化', 'method': 'on.pinRequired','result': 'None', 'category': AutoCategory.BrAuto },
  { 'name': '注册绑定状态变化', 'detail': '注册绑定状态变化', 'method': 'on.bondStateChange',  'result': 'None', 'category': AutoCategory.BrAuto }
]

const BleAutoTest: any[]  = [
  { 'name': '创建Gatt服务器', 'detail': '创建Gatt服务器', 'method': 'createGattServer',  'result': 'None', 'category': AutoCategory.BleAuto },
  { 'name': '创建Gatt客户端', 'detail': '创建Gatt客户端', 'method': 'createGattClient',  'result': 'None', 'category': AutoCategory.BleAuto },
  { 'name': '获取连接的Ble设备', 'detail': '获取连接的Ble设备信息', 'method': 'getConnectedBleDevices', 'result': 'None', 'category': AutoCategory.BleAuto },
  { 'name': '开始Ble扫描', 'detail': '开始扫描Ble设备', 'method': 'startBleScan',  'result': 'None', 'category': AutoCategory.BleAuto },
  { 'name': '停止Ble扫描', 'detail': '停止扫描', 'method': 'stopBleScan',  'result': 'None', 'category': AutoCategory.BleAuto },
  { 'name': '注册Ble查找状态变化', 'detail': '注册Ble设备查找状态变化', 'method': 'on.BleDeviceFind',  'result': 'None', 'category': AutoCategory.BleAuto }
]

//const GattTestCase: any[]  = [
//  { 'name': 'GattClient测试', 'detail': 'Gatt客户端测试', 'method': 'gattClient', 'result': 'None', 'category': AutoCategory.Gatt },
//  { 'name': 'GattServer测试', 'detail': 'Gatt服务器测试', 'method': 'gattServer', 'result': 'None', 'category': AutoCategory.Gatt },
//]
//const SppTestCase: any[]  = [
//  { 'name': 'SppClient测试', 'detail': 'Spp客户端测试', 'method': 'sppClient', 'result': 'None', 'category': AutoCategory.SPP },
//  { 'name': 'SppServer测试', 'detail': 'Spp服务器测试', 'method': 'sppServer', 'result': 'None', 'category': AutoCategory.SPP },
//]
//const ProfileTestCase: any[]  = [
//  { 'name': 'A2dpSink配置', 'detail': 'A2dpSink配置测试', 'method': 'A2dpSinkProfile', 'result': 'None', 'category': AutoCategory.Profile },
//  { 'name': 'A2dpSource配置', 'detail': 'A2dpSource配置测试', 'method': 'A2dpSourceProfile', 'result': 'None', 'category': AutoCategory.Profile },
//  { 'name': 'AvrcpCT配置', 'detail': 'AvrcpCT配置测试', 'method': 'AvrcpProfile',  'result': 'None', 'category': AutoCategory.Profile },
//  { 'name': 'AvrcpTG配置', 'detail': 'AvrcpTG配置测试', 'method': 'AvrcpProfile',  'result': 'None', 'category': AutoCategory.Profile },
//  { 'name': 'HandsFreeAudioGateway配置', 'detail': 'HandsFreeAudioGateway配置测试', 'method': 'HandsFreeAudioGatewayProfile',  'result': 'None', 'category': AutoCategory.Profile },
//  { 'name': 'HandsFreeUnit配置', 'detail': 'HandsFreeUnit配置测试', 'method': 'HandsFreeUnitProfile', 'result': 'None', 'category': AutoCategory.Profile },
//  { 'name': 'HidHost配置', 'detail': 'HidHost配置测试', 'method': 'HidHostProfile', 'result': 'None', 'category': AutoCategory.Profile },
//  { 'name': 'PanNetwork配置', 'detail': 'PanNetwork配置测试', 'method': 'PanNetworkProfile', 'result': 'None', 'category': AutoCategory.Profile },
//  { 'name': 'PbapClient配置', 'detail': 'PbapClient配置测试', 'method': 'PbapClientProfile',  'result': 'None', 'category': AutoCategory.Profile },
//  { 'name': 'PbapServer配置', 'detail': 'PbapServer配置测试', 'method': 'PbapServerProfile', 'result': 'None', 'category': AutoCategory.Profile },
//]

const GattClientAutoTest: any[] =[
  { 'name': '创造Gatt客户端', 'detail': '创造Gatt客户端', 'method': 'createGattClient', 'result': 'None', 'category': AutoCategory.GattClientAuto },
  { 'name': '连接', 'detail': 'Gatt连接', 'method': 'connect', 'result': 'None', 'category': AutoCategory.GattClientAuto },
  { 'name': '取消连接', 'detail': '取消Gatt连接', 'method': 'disconnect',  'result': 'None', 'category': AutoCategory.GattClientAuto },
  { 'name': 'Ble连接状态', 'detail': '查看Ble连接状态', 'method': 'on.BleConnectionStateChange',  'result': 'None', 'category': AutoCategory.GattClientAuto },
  { 'name': '关闭', 'detail': '关闭Gatt客户端', 'method': 'close',  'result': 'None', 'category': AutoCategory.GattClientAuto },
  { 'name': '获取服务callback', 'detail': '获取服务callback', 'method': 'getServices(Callback)',  'result': 'None', 'category': AutoCategory.GattClientAuto },
  { 'name': '获取服务promise', 'detail': '获取服务promise', 'method': 'getServices(promise)',  'result': 'None', 'category': AutoCategory.GattClientAuto },
  { 'name': '读取特征值callback', 'detail': '读取特征值callback', 'method': 'readCharacteristicValue(callback)', 'result': 'None', 'category': AutoCategory.GattClientAuto },
  { 'name': '读取特征值promise', 'detail': '读取特征值promise', 'method': 'readCharacteristicValue(promise)',  'result': 'None', 'category': AutoCategory.GattClientAuto },
  { 'name': '读取描述符值callback', 'detail': '读取描述符值callback', 'method': 'readDescriptorValue(callback)', 'result': 'None', 'category': AutoCategory.GattClientAuto },
  { 'name': '读取描述符值promise', 'detail': '读取描述符值promise', 'method': 'readDescriptorValue(promise)',  'result': 'None', 'category': AutoCategory.GattClientAuto },
  { 'name': '写入特征值', 'detail': '写入特征值', 'method': 'writeCharacteristicValue',  'result': 'None', 'category': AutoCategory.GattClientAuto },
  { 'name': '写入描述符值', 'detail': '写入描述符值', 'method': 'writeDescriptorValue', 'result': 'None', 'category': AutoCategory.GattClientAuto },
  { 'name': 'Ble特征值变化', 'detail': '查看Ble特征值变化', 'method': 'on.BleCharacteristicChange', 'result': 'None', 'category': AutoCategory.GattClientAuto },
  { 'name': '设定BleMtu尺寸', 'detail': '设定BleMtu尺寸', 'method': 'setBleMtuSize', 'result': 'None', 'category': AutoCategory.GattClientAuto },
  { 'name': '设置通知特性更改', 'detail': '设置通知特性更改', 'method': 'setNotifyCharacteristicChanged','result': 'None', 'category': AutoCategory.GattClientAuto },
  { 'name': '获取设备名称callback', 'detail': '获取设备名称callback', 'method': 'getDeviceName()callback',  'result': 'None', 'category': AutoCategory.GattClientAuto },
  { 'name': '获取设备名称promise', 'detail': '获取设备名称promise', 'method': 'getDeviceName()Promise', 'result': 'None', 'category': AutoCategory.GattClientAuto },
  { 'name': '获取Rssi值callback', 'detail': '获取Rssi值callback', 'method': 'getRssiValue()Callback',  'result': 'None', 'category': AutoCategory.GattClientAuto },
  { 'name': '获取Rssi值promise', 'detail': '获取Rssi值promise', 'method': 'getRssiValue()Promise',  'result': 'None', 'category': AutoCategory.GattClientAuto },
]

const GattServerAutoTest: any[]  = [
  { 'name': '创建Gatt服务器', 'detail': '创建Gatt服务器', 'method': 'createGattServer',  'result': 'None', 'category': AutoCategory.GattServerAuto },
  { 'name': '添加服务', 'detail': '添加Gatt服务', 'method': 'addService', 'result': 'None', 'category': AutoCategory.GattServerAuto },
  { 'name': '删除服务', 'detail': '删除Gatt服务', 'method': 'removeService',  'result': 'None', 'category': AutoCategory.GattServerAuto },
  { 'name': '开始广播', 'detail': '开始广播播报', 'method': 'startAdvertising',  'result': 'None', 'category': AutoCategory.GattServerAuto },
  { 'name': '停止广播', 'detail': '停止广播播报', 'method': 'stopAdvertising',  'result': 'None', 'category': AutoCategory.GattServerAuto },
  { 'name': '关闭', 'detail': '关闭Gatt服务器', 'method': 'close', 'result': 'None', 'category': AutoCategory.GattServerAuto },
  { 'name': '连接状态', 'detail': '查看Gatt连接状态', 'method': 'on.connectStateChange',  'result': 'None', 'category': AutoCategory.GattServerAuto },
  { 'name': '读取特征值', 'detail': '读取特征值', 'method': 'on.characteristicRead','result': 'None', 'category': AutoCategory.GattServerAuto },
  { 'name': '写入特征值', 'detail': '写入特征值', 'method': 'on.characteristicWrite',  'result': 'None', 'category': AutoCategory.GattServerAuto },
  { 'name': '读取描述符', 'detail': '读取描述符', 'method': 'on.descriptorRead',  'result': 'None', 'category': AutoCategory.GattServerAuto },
  { 'name': '写入描述符', 'detail': '写入描述符', 'method': 'on.descriptorWrite', 'result': 'None', 'category': AutoCategory.GattServerAuto },
]

const SppClientAutoTest: any[]  = [
  { 'name': '开蓝牙', 'detail': '使能蓝牙', 'method': 'enableBluetooth',  'result': 'None', 'category': AutoCategory.SppClientAuto },
  { 'name': '关蓝牙', 'detail': '去使能蓝牙', 'method': 'disableBluetooth',  'result': 'None', 'category': AutoCategory.SppClientAuto },
  { 'name': '设置SPP扫描模式', 'detail': '设置SPP扫描模式', 'method': 'setBluetoothScanMode',  'result': 'None', 'category': AutoCategory.SppClientAuto },
  { 'name': '设置状态', 'detail': '设置蓝牙状态', 'method': 'getState',  'result': 'None', 'category': AutoCategory.SppClientAuto },
  { 'name': '获取扫描模式', 'detail': '获取蓝牙扫描模式', 'method': 'getBluetoothScanMode','result': 'None', 'category': AutoCategory.SppClientAuto },
  { 'name': '获取当前连接状态', 'detail': '获取当前蓝牙连接状态', 'method': 'getBtConnectionState',  'result': 'None', 'category': AutoCategory.SppClientAuto },
  { 'name': 'Spp客户端连接(安全模式)', 'detail': 'Spp客户端(安全模式)连接', 'method': 'sppConnect', 'result': 'None', 'category': AutoCategory.SppClientAuto },
  { 'name': 'Spp客户端连接(非安全模式)', 'detail': 'Spp客户端(非安全模式)连接', 'method': 'sppConnect','result': 'None', 'category': AutoCategory.SppClientAuto },
  { 'name': '配对设备', 'detail': '进行蓝牙设备配对', 'method': 'pairDevice', 'result': 'None', 'category': AutoCategory.SppClientAuto },
  { 'name': 'Spp客户端写入', 'detail': 'Spp客户端写入', 'method': 'sppWrite',  'result': 'None', 'category': AutoCategory.SppClientAuto },
  { 'name': 'Spp客户端读取打开', 'detail': 'Spp客户端读取打开', 'method': 'on.sppRead', 'result': 'None', 'category': AutoCategory.SppClientAuto },
  { 'name': 'Spp客户端读取关闭', 'detail': 'Spp客户端读取关闭', 'method': 'off.sppRead', 'result': 'None', 'category': AutoCategory.SppClientAuto },
  { 'name': 'Spp客户端关闭', 'detail': 'Spp客户端关闭', 'method': 'sppCloseClientSocket', 'result': 'None', 'category': AutoCategory.SppClientAuto },
]

const SppServerAutoTest: any[]  = [
  { 'name': '开蓝牙', 'detail': '使能蓝牙', 'method': 'enableBluetooth',  'result': 'None', 'category': AutoCategory.SppServerAuto },
  { 'name': '关蓝牙', 'detail': '去使能蓝牙', 'method': 'disableBluetooth', 'result': 'None', 'category': AutoCategory.SppServerAuto },
  { 'name': '设置SPP扫描模式', 'detail': '设置SPP扫描模式', 'method': 'setBluetoothScanMode',  'result': 'None', 'category': AutoCategory.SppServerAuto },
  { 'name': '设置状态', 'detail': '设置蓝牙状态', 'method': 'getState',  'result': 'None', 'category': AutoCategory.SppServerAuto },
  { 'name': '获取扫描模式', 'detail': '获取蓝牙扫描模式', 'method': 'getBluetoothScanMode',  'result': 'None', 'category': AutoCategory.SppServerAuto },
  { 'name': '获取当前连接状态', 'detail': '获取当前蓝牙连接状态', 'method': 'getBtConnectionState',  'result': 'None', 'category': AutoCategory.SppServerAuto },
  { 'name': 'Spp服务器创建(安全模式)', 'detail': 'Spp服务器(安全模式)创建', 'method': 'sppListen',  'result': 'None', 'category': AutoCategory.SppServerAuto },
  { 'name': 'Spp服务器创建(非安全模式)', 'detail': 'Spp服务器(非安全模式)创建', 'method': 'sppListen',  'result': 'None', 'category': AutoCategory.SppServerAuto },
  { 'name': '注册pin码变化', 'detail': '注册pin码变化', 'method': 'on.pinRequired',  'result': 'None', 'category': AutoCategory.SppServerAuto },
  //  { 'name': 'SPP客户端写入', 'detail': '使能蓝牙', 'method': 'spp_Client_Write', 'api': testSPP_Client_Write, 'result': 'None', 'category': AutoCategory.SppServerAuto },
  { 'name': 'Spp服务器写入', 'detail': 'Spp服务器写入', 'method': 'sppWrite', 'result': 'None', 'category': AutoCategory.SppServerAuto },
  { 'name': 'Spp服务器接受', 'detail': 'Spp服务器接受', 'method': 'sppAccept',  'result': 'None', 'category': AutoCategory.SppServerAuto },
  { 'name': 'Spp服务器读取打开', 'detail': 'Spp服务器读取打开', 'method': 'on.sppRead',  'result': 'None', 'category': AutoCategory.SppServerAuto },
  { 'name': 'Spp服务器读取关闭', 'detail': 'Spp服务器读取关闭', 'method': 'off.sppRead',  'result': 'None', 'category': AutoCategory.SppServerAuto },
  { 'name': 'Spp服务器关闭', 'detail': 'Spp服务器关闭', 'method': 'sppCloseServerSocket',  'result': 'None', 'category': AutoCategory.SppServerAuto },
  { 'name': 'Spp客户端关闭', 'detail': 'Spp客户端关闭', 'method': 'sppCloseClientSocket', 'result': 'None', 'category': AutoCategory.SppServerAuto },
]

//const A2dpSinkProfileAutoTest: any[]  = [
//  { 'name': '连接', 'detail': 'Profile连接', 'method': 'connect',  'result': 'None', 'category': AutoCategory.A2dpSinkAuto },
//  { 'name': '取消连接', 'detail': '取消Profile连接', 'method': 'disconnect',  'result': 'None', 'category': AutoCategory.A2dpSinkAuto },
//  { 'name': '获取播放状态', 'detail': '获取Profile播放状态', 'method': 'getPlayingState',  'result': 'None', 'category': AutoCategory.A2dpSinkAuto },
//  { 'name': '获取连接设备', 'detail': '获取Profile连接设备', 'method': 'getConnectionDevices', 'result': 'None', 'category': AutoCategory.A2dpSinkAuto },
//  { 'name': '获取设备状态', 'detail': '获取Profile设备状态', 'method': 'getDeviceState',  'result': 'None', 'category': AutoCategory.A2dpSinkAuto },
//]
const A2dpSourceProfileAutoTest:any []=[
  { 'name': '连接', 'detail': 'Profile连接', 'method': 'connect', 'result': 'None', 'category': AutoCategory.A2dpSourceAuto },
  { 'name': '取消连接', 'detail': '取消Profile连接', 'method': 'disconnect', 'result': 'None', 'category': AutoCategory.A2dpSourceAuto },
  { 'name': '获取播放状态', 'detail': '获取Profile播放状态', 'method': 'getPlayingState',  'result': 'None', 'category': AutoCategory.A2dpSourceAuto},
  { 'name': '获取连接设备', 'detail': '获取Profile连接设备', 'method': 'getConnectionDevices', 'result': 'None', 'category': AutoCategory.A2dpSourceAuto },
  { 'name': '获取设备状态', 'detail': '获取Profile设备状态', 'method': 'getDeviceState', 'result': 'None', 'category': AutoCategory.A2dpSourceAuto},
]

//const AvrcpCTProfileAutoTest:any []=[
//  { 'name': '连接', 'detail': 'Profile连接', 'method': 'connect',  'result': 'None', 'category': AutoCategory.AvrcpCTAuto },
//  { 'name': '取消连接', 'detail': '取消Profile连接', 'method': 'disconnect',  'result': 'None', 'category': AutoCategory.AvrcpCTAuto },
//  { 'name': '获取连接设备', 'detail': '获取Profile连接设备', 'method': 'getConnectionDevices', 'result': 'None', 'category': AutoCategory.AvrcpCTAuto },
//  { 'name': '获取设备状态', 'detail': '获取Profile设备状态', 'method': 'getDeviceState',  'result': 'None', 'category': AutoCategory.AvrcpCTAuto },
//]

//const AvrcpTGProfileAutoTest:any []=[
//  { 'name': '连接', 'detail': 'Profile连接', 'method': 'connect', 'result': 'None', 'category': AutoCategory.AvrcpTGAuto },
//  { 'name': '取消连接', 'detail': '取消Profile连接', 'method': 'disconnect',  'result': 'None', 'category': AutoCategory.AvrcpTGAuto },
//  { 'name': '获取连接设备', 'detail': '获取Profile连接设备', 'method': 'getConnectionDevices', 'result': 'None', 'category': AutoCategory.AvrcpTGAuto },
//  { 'name': '获取设备状态', 'detail': '获取Profile设备状态', 'method': 'getDeviceState', 'result': 'None', 'category': AutoCategory.AvrcpTGAuto },
//]

const HandsFreeAudioGatewayProfileAutoTest: any[]  = [
  { 'name': '连接', 'detail': 'Profile连接', 'method': 'connect',  'result': 'None', 'category': AutoCategory.HandsFreeAudioGatewayAuto },
  { 'name': '取消连接', 'detail': '取消Profile连接', 'method': 'disconnect',  'result': 'None', 'category': AutoCategory.HandsFreeAudioGatewayAuto },
  { 'name': '获取连接设备', 'detail': '获取Profile连接设备', 'method': 'getConnectionDevices', 'result': 'None', 'category': AutoCategory.HandsFreeAudioGatewayAuto, },
  { 'name': '获取设备状态', 'detail': '获取Profile设备状态', 'method': 'getDeviceState',  'result': 'None', 'category': AutoCategory.HandsFreeAudioGatewayAuto, },
]

//const HandsFreeUnitProfileAutoTest: any[] =[
//  { 'name': '连接', 'detail': 'Profile连接', 'method': 'connect', 'result': 'None', 'category': AutoCategory.HandsFreeUnitAuto },
//  { 'name': '取消连接', 'detail': '取消Profile连接', 'method': 'disconnect',  'result': 'None', 'category': AutoCategory.HandsFreeUnitAuto },
//  { 'name': '连接Sco', 'detail': '连接Sco', 'method': 'connectSco',  'result': 'None', 'category': AutoCategory.HandsFreeUnitAuto },
//  { 'name': '获取Sco状态', 'detail': '获取Sco状态', 'method': 'getScoState',  'result': 'None', 'category': AutoCategory.HandsFreeUnitAuto },
//  { 'name': '取消连接Sco', 'detail': '取消连接Sco', 'method': 'disconnectSco',  'result': 'None', 'category': AutoCategory.HandsFreeUnitAuto },
//  { 'name': '发送DTMF', 'detail': '发送DTMF', 'method': 'sendDTMF', 'result': 'None', 'category': AutoCategory.HandsFreeUnitAuto },
//  { 'name': '获取连接设备', 'detail': '获取Profile连接设备', 'method': 'getConnectionDevices', 'result': 'None', 'category': AutoCategory.HandsFreeUnitAuto },
//  { 'name': '获取设备状态', 'detail': '获取Profile设备状态', 'method': 'getDeviceState',  'result': 'None', 'category': AutoCategory.HandsFreeUnitAuto },
//]

const HitHostProfileAutoTest:any[] =[
  { 'name': '连接', 'detail': 'Profile连接', 'method': 'connect', 'result': 'None', 'category': AutoCategory.HitHostAuto },
  { 'name': '取消连接', 'detail': '取消Profile连接', 'method': 'disconnect','result': 'None', 'category': AutoCategory.HitHostAuto },
  { 'name': '获取连接设备', 'detail': '获取Profile连接设备', 'method': 'getConnectionDevices',  'result': 'None', 'category': AutoCategory.HitHostAuto },
  { 'name': '获取设备状态', 'detail': '获取Profile设备状态', 'method': 'getDeviceState', 'result': 'None', 'category': AutoCategory.HitHostAuto },
]

const PanNetworkProfileAutoTest:any[] =[
  { 'name': '取消连接', 'detail': '取消Profile连接', 'method': 'disconnect','result': 'None', 'category': AutoCategory.PanNetworkAuto },
  { 'name': '获取连接设备', 'detail': '获取Profile连接设备', 'method': 'getConnectionDevices',  'result': 'None', 'category': AutoCategory.PanNetworkAuto },
  { 'name': '获取设备状态', 'detail': '获取Profile设备状态', 'method': 'getDeviceState',  'result': 'None', 'category': AutoCategory.PanNetworkAuto },
  { 'name': '设置网络共享', 'detail': '设置网络数据共享', 'method': 'setTethering',  'result': 'None', 'category': AutoCategory.PanNetworkAuto },
  { 'name': '网络共享状态打开', 'detail': '网络共享状态打开', 'method': 'isTetheringOn',  'result': 'None', 'category': AutoCategory.PanNetworkAuto },
]

//const PbapClientProfileAutoTest:any[] =[
//  { 'name': '连接', 'detail': 'Profile连接', 'method': 'connect',  'result': 'None', 'category': AutoCategory.PbapClientAuto },
//  { 'name': '取消连接', 'detail': '取消Profile连接', 'method': 'disconnect', 'result': 'None', 'category': AutoCategory.PbapClientAuto },
//  { 'name': '获取连接设备', 'detail': '获取Profile连接设备', 'method': 'getConnectionDevices', 'result': 'None', 'category': AutoCategory.PbapClientAuto },
//  { 'name': '获取设备状态', 'detail': '获取Profile设备状态', 'method': 'getDeviceState',  'result': 'None', 'category': AutoCategory.PbapClientAuto },
//]
//
//const PbapServerProfileAutoTest:any[] =[
//  { 'name': '取消连接', 'detail': '取消Profile连接', 'method': 'disconnect',  'result': 'None', 'category': AutoCategory.PbapServerAuto },
//  { 'name': '获取连接设备', 'detail': '获取Profile连接设备', 'method': 'getConnectionDevices',  'result': 'None', 'category': AutoCategory.PbapServerAuto },
//  { 'name': '获取设备状态', 'detail': '获取Profile设备状态', 'method': 'getDeviceState', 'result': 'None', 'category': AutoCategory.PbapServerAuto },
//]

export function initBrAutoData(): Array<TestAuto> {
  let TestAutoArray: Array<TestAuto> = []
  BrAutoTest.forEach(item => {
    TestAutoArray.push(new TestAuto(item.name, item.detail,  item.method, item.result, item.Category));
  })
  return TestAutoArray;
}

export function initBleAutoData(): Array<TestAuto> {
  let TestAutoArray: Array<TestAuto> = []
  BleAutoTest.forEach(item => {
    TestAutoArray.push(new TestAuto(item.name, item.detail, item.method, item.result, item.Category));
  })
  return TestAutoArray;
}

export function initGattClientAutoData(): Array<TestAuto> {
  let TestAutoArray: Array<TestAuto> = []
  GattClientAutoTest.forEach(item => {
    TestAutoArray.push(new TestAuto(item.name, item.detail,  item.method, item.result, item.Category));
  })
  return TestAutoArray;
}

export function initGattServerAutoData(): Array<TestAuto> {
  let TestAutoArray: Array<TestAuto> = []
  GattServerAutoTest.forEach(item => {
    TestAutoArray.push(new TestAuto(item.name, item.detail, item.method, item.result, item.Category));
  })
  return TestAutoArray;
}

export function initSppClientAutoData(): Array<TestAuto> {
  let TestAutoArray: Array<TestAuto> = []
  SppClientAutoTest.forEach(item => {
    TestAutoArray.push(new TestAuto(item.name, item.detail,  item.method, item.result, item.Category));
  })
  return TestAutoArray;
}

export function initSppServerAutoData(): Array<TestAuto> {
  let TestAutoArray: Array<TestAuto> = []
  SppServerAutoTest.forEach(item => {
    TestAutoArray.push(new TestAuto(item.name, item.detail,  item.method, item.result, item.Category));
  })
  return TestAutoArray;
}


export function initA2dpSourceProfileAutoData(): Array<TestAuto> {
  let TestAutoArray: Array<TestAuto> = []
  A2dpSourceProfileAutoTest.forEach(item => {
    TestAutoArray.push(new TestAuto(item.name, item.detail,  item.method, item.result, item.Category));
  })
  return TestAutoArray;
}


export function initHandsFreeAudioGatewayProfileAutoData(): Array<TestAuto> {
  let TestAutoArray: Array<TestAuto> = []
  HandsFreeAudioGatewayProfileAutoTest.forEach(item => {
    TestAutoArray.push(new TestAuto(item.name, item.detail,  item.method, item.result, item.Category));
  })
  return TestAutoArray;
}


export function initHitHostProfileAutoData(): Array<TestAuto> {
  let TestAutoArray: Array<TestAuto> = []
  HitHostProfileAutoTest.forEach(item => {
    TestAutoArray.push(new TestAuto(item.name, item.detail,  item.method, item.result, item.Category));
  })
  return TestAutoArray;
}

export function initPanNetworkProfileAutoData(): Array<TestAuto> {
  let TestAutoArray: Array<TestAuto> = []
  PanNetworkProfileAutoTest.forEach(item => {
    TestAutoArray.push(new TestAuto(item.name, item.detail,  item.method, item.result, item.Category));
  })
  return TestAutoArray;
}

