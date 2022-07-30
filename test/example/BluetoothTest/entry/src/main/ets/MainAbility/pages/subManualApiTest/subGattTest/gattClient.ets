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

/**
 * Gatt Client Test Page Of Bluetooth test
 */

import { TitleComponent } from '../../../../Component/titleComponent';
import HeadComponent from '../../../../component/headcomponent';
import BluetoothModel from '../../../model/BluetoothModel';
import Router from '@system.router';
import BluetoothDeviceController from '../../../controller/BluetoothDeviceController';
import ConfigData from '../../../../Utils/ConfigData';
import LogUtil from '../../../../Utils/LogUtil';
import bluetooth from '@ohos.bluetooth';

const NO_GATT_CLIENT_OBJECT = "No GattClient object, please CreateGattClient firstly!";


@Entry
@Component
struct GattClient {
  @State message: string = 'GattClient'
  @State currentClick: number = -1;

  @State btOnBLEConnectionStateChange: string = 'on:BLEConnectionStateChange';
  @State isOnBLEConnectionStateChangeClick: boolean = false;
  @State bleConnectionStateInfo: string = "Disconnected";
  @State bleConnectionState: number = 0;

  @State btOnBLECharacteristicChange: string = 'on:BLECharacteristicChange';

  @State isOnBLECharacteristicChangeClick: boolean = false;
  @State bleCharacteristicChangeInfo:string = '';
  @State serviceUUID: string = '00001877-0000-1000-8000-00805F9B34FB';

  @State characteristicUUID:  string = '00002BE0-0000-1000-8000-00805F9B34FB';
  @State characteristicValue: string = 'CccValue';
  @State descriptorUUID:      string = '00002902-0000-1000-8000-00805F9B34FB';
  @State descriptorValue:     string = 'DesValue';

  @State mtuSize:number = 128;


  // @State  gattClientInstance:bluetooth.GattClientDevice = null;
  private gattClientInstance=null;
  private peripheralDeviceId = '88:36:CF:09:C1:90';// '8C:7A:3D:7D:DF:DD';RedMi//
  private gattClientExist = false;



  build() {
    Column() {
          HeadComponent({ headName: 'GattClient Test', isActive: true });
          Stack().height("0.5vp").backgroundColor("#000000");
          Row() {
            Text("peripheral deviceId:").fontSize("15vp")
            TextInput( { text: this.peripheralDeviceId, placeholder: "input peripheral deviceId." } )
              .fontSize("10vp")
              .onChange((strInput) => {
                this.peripheralDeviceId = strInput;
              })
          }
          .margin( { top: "1vp", left: "20vp", right: "20vp" } )
          .alignItems(VerticalAlign.Center)
          Column() {
            Stack().height("0.25vp").backgroundColor("#000000");
            Column() {
              Row() {
                Text("Service UUID:").fontSize("15vp")
                TextInput({ text: this.serviceUUID, placeholder: "input Service UUID" })
                  .fontSize("10vp")
                  .onChange((strInput) => {
                    this.serviceUUID = strInput;
                  })
              }
              .margin( { top: "1vp", left: "20vp", right: "20vp" } )
              .alignItems(VerticalAlign.Center)
              Row() {
                Text("Characteristic UUID:").fontSize("15vp")
                TextInput({ text: this.characteristicUUID, placeholder: "input Characteristic UUID" })
                  .fontSize("10vp")
                  .onChange((strInput) => {
                    this.characteristicUUID = strInput;
                  })
              }
              .margin( { top: "1vp", left: "20vp", right: "20vp" } )
              .alignItems(VerticalAlign.Center)
              Row() {
                Text("Characteristic Value:").fontSize("15vp")
                TextInput({ text: this.characteristicUUID, placeholder: "Characteristic Value" })
                  .fontSize("10vp")
                  .onChange((strInput) => {
                    this.characteristicValue = strInput;
                  })
              }
              .margin( { top: "1vp", left: "20vp", right: "20vp" } )
              .alignItems(VerticalAlign.Center)
              Row() {
                Text("Descriptor UUID:").fontSize("15vp")
                TextInput({ text: this.descriptorUUID, placeholder: "input descriptor UUID" })
                  .fontSize("10vp")
                  .onChange((strInput) => {
                    this.descriptorUUID = strInput;
                  })
              }
              .margin( { top: "1vp", left: "20vp", right: "20vp" } )
              .alignItems(VerticalAlign.Center)
              Row() {
                Text("Descriptor Value:").fontSize("15vp")
                TextInput({ text: this.descriptorUUID, placeholder: "descriptor Value" })
                  .fontSize("10vp")
                  .onChange((strInput) => {
                    this.descriptorValue = strInput;
                  })
              }
              .margin( { top: "1vp", left: "20vp", right: "20vp" } )
              .alignItems(VerticalAlign.Center)
            }
            Stack().height("0.25vp").backgroundColor("#000000");
            Column(){
              Text("result:" + this.message)
                .fontSize("15vp")
                .margin( { top: "1vp", left: "20vp", right: "20vp" } )
                .textAlign(TextAlign.Start)
                .backgroundColor($r("sys.color.ohos_id_color_foreground_contrary"))
              Text("bleConnectionStateChange:" + this.bleConnectionStateInfo)
                .fontSize("15vp")
                .margin( { top: "1vp", left: "20vp", right: "20vp" } )
                .textAlign(TextAlign.Start)
                .backgroundColor($r("sys.color.ohos_id_color_foreground_contrary"))
              Text("BLECharacteristicChange:" + this.bleCharacteristicChangeInfo)
                .fontSize("15vp")
                .margin( { top: "1vp", left: "20vp", right: "20vp" } )
                .textAlign(TextAlign.Start)
                .backgroundColor($r("sys.color.ohos_id_color_foreground_contrary"))
            }.height("20%")

            Stack().height("0.25vp").backgroundColor("#000000");

            Text("Click your wanted method below to verify:")
              .margin({top: "1vp"})
              .padding("5vp")
              .fontSize("15vp")
              .fontColor("#4444444");

            Scroll() {
              List({ space: 1 }) {
                ListItem() {
                  TitleComponent({
                    title: "CreateGattClient(...)",
                    bgColor: this.currentClick === 99 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                  });
                }
                .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
                .onClick(() => {
                  this.currentClick = 99;
                  let btState = bluetooth.getState();
                  if (btState === bluetooth.BluetoothState.STATE_ON) {
                    if (this.gattClientExist)
                    {
                      this.message = "GattClient exists!";
                      return;
                    }
                    this.message = "CreateGattClient: turning on BLEDeviceFind";

                    bluetooth.BLE.on("BLEDeviceFind",(scanResult)=>{
                      scanResult.forEach(device => {
                        var devId = device.deviceId;  //88:36:CF:09:C1:90
                        var rssi = device.rssi;
                        var data = device.data;

                        bluetooth.BLE.stopBLEScan();
                        // create ble client instance
                        if(!this.gattClientExist)
                        {
                          this.gattClientInstance = BluetoothModel.createGattClient(devId);
                          this.message = "createGattClient():succeeds ";
                          this.gattClientExist = true;
                        }

                        if(devId==this.peripheralDeviceId)
                        {
                          bluetooth.BLE.stopBLEScan();
                          // create ble client instance
                          if(!this.gattClientExist)
                          {
                            this.gattClientInstance = BluetoothModel.createGattClient(devId);
                            this.message = "createGattClient():succeeds ";
                            this.gattClientExist = true;
                          }

                        }
                      })
                    });
                    if(this.peripheralDeviceId.length >15 )
                    {
                      this.message = "startBLEScan with filter,peripheralDeviceId="+this.peripheralDeviceId;
                      let bleScanFilter = {
                        deviceId: this.peripheralDeviceId
                        //serviceUuid:'000054F6-0000-1000-8000-00805f9b34fb'
                      }
                      let bleScanFilters = [];
                      bleScanFilters.push(bleScanFilter);
                      let bleScanOptions =
                      {
                        interval: 0,
                        dutyMode: bluetooth.ScanDuty.SCAN_MODE_BALANCED,
                        matchMode: bluetooth.MatchMode.MATCH_MODE_AGGRESSIVE
                      }
                      bluetooth.BLE.startBLEScan(bleScanFilters,bleScanOptions);
                    }
                    else
                    {
                      this.message = "Be sure the correct MacId!";

                    }

                  }
                  else if (btState === bluetooth.BluetoothState.STATE_OFF) {
                    this.message = "BT is not enabled!";
                  }
                  else
                  {
                    this.message = "switching,please operate it later!";
                  }
                })

                ListItem() {
                  TitleComponent({
                    title: "connect(): boolean",
                    bgColor: this.currentClick === 0 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                  })
                }
                .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
                .onClick(() => {
                  let btState = bluetooth.getState();
                  if (btState === bluetooth.BluetoothState.STATE_ON) {
                    if (this.gattClientExist) {
                      this.currentClick = 0;
                      if(this.bleConnectionState == 0)
                      {
                          let rv = this.gattClientInstance.connect();
                          this.message = "GattClient.connect(): " + rv;
                      }
                      else
                      {
                        this.message = "this device is not disconnected!";
                        return;
                      }
                    }
                    else
                    {
                      this.message = NO_GATT_CLIENT_OBJECT;
                    }

                  }
                  else if (btState === bluetooth.BluetoothState.STATE_OFF) {
                    this.message = "BT is not enabled!";
                  }
                  else
                  {
                    this.message = "switching,please operate it later!";
                  }
                })

                ListItem() {
                  TitleComponent({
                    title: "disconnect(): boolean",
                    bgColor: this.currentClick === 1 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                  });
                }
                .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
                .onClick(() => {

                  this.message = "GattClient.disconnect() result";
                  let btState = bluetooth.getState();
                  if (btState == bluetooth.BluetoothState.STATE_ON) {
                    if(!this.gattClientExist)
                    {
                      this.message = NO_GATT_CLIENT_OBJECT;
                      return;
                    }
                    this.currentClick = 1;
                    if (this.bleConnectionState === 2) {
                      let rv = this.gattClientInstance.disconnect();
                      this.message = "GattClient.disconnect(): " + rv;
                    }
                    else
                    {
                      this.message = " BLE is not connected and no need to disconnect it ";
                    }

                  }
                  else if (btState == bluetooth.BluetoothState.STATE_OFF) {
                    this.message = "BT is not enabled!";
                  }
                  else
                  {
                    this.message = "switching,please operate it later!";
                  }
                })

                ListItem() {
                  TitleComponent({
                    title: this.btOnBLEConnectionStateChange,
                    bgColor: this.currentClick === 2 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                  });
                }
                .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
                .onClick(() => {
                  let btStatus = bluetooth.getState();
                  if (btStatus === bluetooth.BluetoothState.STATE_OFF) {
                    this.message = "BT is not enabled!";
                    return;
                  }

                  if(!this.gattClientExist)
                  {
                    this.message = NO_GATT_CLIENT_OBJECT;
                    return;
                  }

                  if (this.isOnBLEConnectionStateChangeClick) {
                    this.gattClientInstance.off('BLEConnectionStateChange', () => {
                    });
                    this.message = 'BLEConnectionStateChange is off(turn it on by re-clicking)';
                    this.isOnBLEConnectionStateChangeClick = false;
                    this.btOnBLEConnectionStateChange = 'on:BLEConnectionStateChange'
                    // AppStorage.SetOrCreate('on_BLEConnectionStateChange', this.btOnBLEConnectionStateChange);
                    return;
                  }
                  this.isOnBLEConnectionStateChangeClick = true;
                  this.currentClick = 2;
                  this.btOnBLEConnectionStateChange = 'off:BLEConnectionStateChange'
                  // AppStorage.SetOrCreate('on_BLEConnectionStateChange', this.btOnBLEConnectionStateChange);
                  this.gattClientInstance.on('BLEConnectionStateChange', (data) => {
                    this.bleConnectionState = data.state;
                    // AppStorage.SetOrCreate('on_BLEConnectionStateChange', this.btOnBLEConnectionStateChange);
                    switch(this.bleConnectionState)
                    {
                      case bluetooth.ProfileConnectionState.STATE_DISCONNECTED:
                         this.bleConnectionStateInfo = "DISCONNECTED";
                      break;
                      case bluetooth.ProfileConnectionState.STATE_CONNECTING:
                        this.bleConnectionStateInfo ="CONNECTING";
                        break;
                      case bluetooth.ProfileConnectionState.STATE_CONNECTED:
                        this.bleConnectionStateInfo ="STATE_CONNECTED";
                            break;
                      case bluetooth.ProfileConnectionState.STATE_DISCONNECTING:
                        this.bleConnectionStateInfo ="STATE_DISCONNECTING";
                      break;
                      default:
                        this.bleConnectionStateInfo = "undefined";

                    }
                  })
                  this.message = 'BLEConnectionStateChange is on';
                })

                ListItem() {
                  TitleComponent({
                    title: "close(): boolean",
                    bgColor: this.currentClick === 3 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                  });
                }
                .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
                .onClick(() => {

                  this.message = "close():";
                  let btState = bluetooth.getState();
                  if (btState == bluetooth.BluetoothState.STATE_ON) {
                    if(!this.gattClientExist)
                    {
                      this.message = NO_GATT_CLIENT_OBJECT;
                      return;
                    }
                    this.currentClick = 3;
                    let rv = this.gattClientInstance.close();
                    if (rv)
                    {
                        this.gattClientExist = false;
                    }
                    this.message = "GattClient.close():" + rv;

                  }
                  else if (btState == bluetooth.BluetoothState.STATE_OFF) {
                    this.message = "BT is not enabled!";
                  }
                  else
                  {
                    this.message = "switching,please operate it later!";
                  }
                })

                ListItem() {
                  TitleComponent({
                    title: "getServices(callback:...)",
                    bgColor: this.currentClick === 4 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                  });
                }
                .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
                .onClick(() => {
                  let btState = bluetooth.getState();
                  if (btState == bluetooth.BluetoothState.STATE_ON) {
                    if(!this.gattClientExist)
                    {
                      this.message = NO_GATT_CLIENT_OBJECT;
                      return;
                    }
                    this.currentClick = 4;
                    this.gattClientInstance.getServices((code, gattServices) =>
                    {
                        if (code.code != 0)
                        {
                          this.message = "getServices(call back) error code:" + code.code;
                          return;
                        }
                        else {
                          this.message =  "getServices(Callback):invoke succeeds! \n "
                          this.message = "bluetooth services size is " + gattServices.length+'\n';
                          for (let i = 0; i < gattServices.length; i++) {
                            this.message += 'bluetooth serviceUuid is ' + gattServices[i].serviceUuid + '\n';
                          }
                        }
                      });

                  }
                  else if (btState == bluetooth.BluetoothState.STATE_OFF) {
                    this.message = "BT is not enabled!";
                  }
                  else
                  {
                    this.message = "switching,please operate it later!";
                  }
                })


                ListItem() {
                  TitleComponent({
                    title: "getServices(): Promise",
                    bgColor: this.currentClick === 5 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                  });
                }
                .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
                .onClick(() => {
                  if(!this.gattClientExist)
                  {
                    this.message = NO_GATT_CLIENT_OBJECT;
                    return;
                  }
                  this.currentClick = 5;
                  this.message = "getServices promise:starts";
                  let btState = bluetooth.getState();
                  if (btState == bluetooth.BluetoothState.STATE_ON) {
                    this.gattClientInstance.getServices().then(gattServices => {
                      this.message =  "getServicesPromise successfully:\n"
                      this.message = "bluetooth services size is " + gattServices.length;
                      for (let i = 0; i < gattServices.length; i++) {
                        this.message += 'bluetooth serviceUuid is ' + gattServices[i].serviceUuid + '\n';
                      }
                    })
                    this.message = "getServices(promise):invoke succeeds!";
                  }
                  else if (btState == bluetooth.BluetoothState.STATE_OFF) {
                    this.message = "BT is not enabled!";
                  }
                  else
                  {
                    this.message = "switching,please operate it later!";
                  }
                })

                ListItem() {
                  TitleComponent({
                    title: "readCharacteristicValue(...,callback): void",
                    bgColor: this.currentClick === 6 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                  });
                }
                .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
                .onClick(() => {
                  let btState = bluetooth.getState();
                  if (btState === bluetooth.BluetoothState.STATE_OFF) {
                    this.message = "BT is not enabled!";
                    return;
                  }
                  if(!this.gattClientExist)
                  {
                    this.message = NO_GATT_CLIENT_OBJECT;
                    return;
                  }
                  this.currentClick = 6;

                  let bleCharacteristicDataIn = {
                    serviceUuid: this.serviceUUID,
                    characteristicUuid: this.characteristicUUID
                  };
                  this.gattClientInstance.readCharacteristicValue(bleCharacteristicDataIn, (err,bleCharacteristicDataOut)=>{
                    if (err.code != 0) {
                      this.message = "readCharacteristicValue error code:" + err.code;
                      return;
                    }
                    this.message ="readCharacteristicValue callback: \n";
                    this.message +='bluetooth characteristic uuid:' + bleCharacteristicDataOut.characteristicUuid+"\n";

                    let value = new Uint8Array(bleCharacteristicDataOut.characteristicValue);
                    for(let i = 0; i <bleCharacteristicDataOut.characteristicValue.byteLength;i++)
                    {
                      this.message += 'bluetooth characteristic value: ' + value[i];
                    }
                  });
                  this.message = 'readCharacteristicValue(callback):invoke succeeds!';
                })

                ListItem() {
                  TitleComponent({
                    title: "readCharacteristicValue(...): Promise",
                    bgColor: this.currentClick === 7 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                  });
                }
                .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
                .onClick(() => {
                  let btState = bluetooth.getState();
                  if (btState === bluetooth.BluetoothState.STATE_OFF) {
                    this.message = "BT is not enabled!";
                    return;
                  }
                  if(!this.gattClientExist)
                  {
                    this.message = NO_GATT_CLIENT_OBJECT;
                    return;
                  }
                  this.currentClick = 7;

                  let bleCharacteristicDataIn = {
                    serviceUuid: this.serviceUUID,
                    characteristicUuid: this.characteristicUUID
                  };
                  this.gattClientInstance.readCharacteristicValue(bleCharacteristicDataIn).then((bleCharacteristicDataOut)=>{
                    this.message = "readCharacteristicValue promise:\n"
                    this.message +='bluetooth characteristic uuid:' + bleCharacteristicDataOut.characteristicUuid+"\n";

                    let value = new Uint8Array(bleCharacteristicDataOut.characteristicValue);
                    for(let i = 0; i <bleCharacteristicDataOut.characteristicValue.byteLength;i++)
                    {
                      this.message += 'bluetooth characteristic value: ' + value[i];
                    }
                  });
                  this.message = 'readCharacteristicValue(promise):invoke succeeds!';
                })

                ListItem() {
                  TitleComponent({
                    title: "readDescriptorValue(...,callback)",
                    bgColor: this.currentClick === 8 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                  });
                }
                .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
                .onClick(() => {
                  let btState = bluetooth.getState();
                  if (btState === bluetooth.BluetoothState.STATE_OFF) {
                    this.message = "BT is not enabled!";
                    return;
                  }
                  if(!this.gattClientExist)
                  {
                    this.message = NO_GATT_CLIENT_OBJECT;
                    return;
                  }
                  this.currentClick = 8;

                  let bufferDesc = new ArrayBuffer(8);
                  let descV = new Uint8Array(bufferDesc);
                  descV[0] = 88;
                  let descriptorIn = {
                    serviceUuid: this.serviceUUID,
                    characteristicUuid: this.characteristicUUID,
                    descriptorUuid: this.descriptorUUID, descriptorValue: bufferDesc};
                  this.gattClientInstance.readDescriptorValue(descriptorIn,(err,descriptorOut)=>{
                    if (err.code != 0) {
                      return;
                    }
                    this.message = 'readDescriptorValue(callback):descriptor uuid=' + descriptorOut.descriptorUuid+"\n";
                    let value = new Uint8Array(descriptorOut.descriptorValue);
                    for(let i = 0; i <descriptorOut.descriptorValue.byteLength;i++)
                    {
                      this.message += 'descriptor value: ' + value[i]+"\n";
                    }
                  });

                  this.message = 'readDescriptorValue(callback):invoke succeeds!';
                })

                ListItem() {
                  TitleComponent({
                    title: "readDescriptorValue(...): Promise",
                    bgColor: this.currentClick === 9 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                  });
                }
                .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
                .onClick(() => {
                  let btState = bluetooth.getState();
                  if (btState === bluetooth.BluetoothState.STATE_OFF) {
                    this.message = "BT is not enabled!";
                    return;
                  }
                  if(!this.gattClientExist)
                  {
                    this.message = NO_GATT_CLIENT_OBJECT;
                    return;
                  }
                  this.currentClick = 9;

                  let bufferDesc = new ArrayBuffer(8);
                  let descV = new Uint8Array(bufferDesc);
                  descV[0] = 99;
                  let descriptorIn = {
                    serviceUuid: this.serviceUUID,
                    characteristicUuid: this.characteristicUUID,
                    descriptorUuid: this.descriptorUUID, descriptorValue: bufferDesc};
                  this.gattClientInstance.readDescriptorValue(descriptorIn).then((descriptorOut)=>{
                    this.message = 'readDescriptorValue(promise):descriptor uuid=' + descriptorOut.descriptorUuid+"\n";
                    let value = new Uint8Array(descriptorOut.descriptorValue);
                    for(let i = 0; i <descriptorOut.descriptorValue.byteLength;i++)
                    {
                      this.message += 'descriptor value: ' + value[i]+"\n";
                    }
                  });

                  this.message = 'readDescriptorValue(promise):invoke succeeds!';
                })

                ListItem() {
                  TitleComponent({
                    title: "writeCharacteristicValue(...): boolean",
                    bgColor: this.currentClick === 10 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                  });
                }
                .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
                .onClick(() => {
                  let btState = bluetooth.getState();
                  if (btState === bluetooth.BluetoothState.STATE_OFF) {
                    this.message = "BT is not enabled!";
                    return;
                  }
                  if(!this.gattClientExist)
                  {
                    this.message = NO_GATT_CLIENT_OBJECT;
                    return;
                  }
                  this.currentClick = 10;

                  let bufferCCC = string2ArrayBuffer(this.characteristicValue);

                  let characteristic = { serviceUuid: this.serviceUUID,
                    characteristicUuid: this.characteristicUUID,
                    characteristicValue: bufferCCC
                  };
                  if(this.gattClientInstance.writeCharacteristicValue(characteristic))
                  {
                    this.message = 'writeCharacteristicValue succeeds.';
                  }
                  else
                  {
                    this.message = 'writeCharacteristicValue failed!';
                  }

                })

                ListItem() {
                  TitleComponent({
                    title: "writeDescriptorValue(...): boolean",
                    bgColor: this.currentClick === 11 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                  });
                }
                .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
                .onClick(() => {
                  let btState = bluetooth.getState();
                  if (btState === bluetooth.BluetoothState.STATE_OFF) {
                    this.message = "BT is not enabled!";
                    return;
                  }
                  if(!this.gattClientExist)
                  {
                    this.message = NO_GATT_CLIENT_OBJECT;
                    return;
                  }
                  this.currentClick = 11;

                  let bufferDesc = string2ArrayBuffer(this.descriptorValue);
                  let descriptor = {serviceUuid: this.serviceUUID,
                    characteristicUuid: this.characteristicUUID,
                    descriptorUuid: this.descriptorUUID,
                    descriptorValue: bufferDesc};
                  if(this.gattClientInstance.writeDescriptorValue(descriptor))
                  {this.message = 'writeDescriptorValue succeeds.';}
                  else
                  {this.message = 'writeDescriptorValue,failed!';}
                })

                ListItem() {
                  TitleComponent({
                    title: this.btOnBLECharacteristicChange,
                    bgColor: this.currentClick === 12 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                  });
                }
                .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
                .onClick(() => {
                  let btStatus = bluetooth.getState();
                  if (btStatus === bluetooth.BluetoothState.STATE_OFF) {
                    this.message = "BT is not enabled!";
                    return;
                  }
                  if(!this.gattClientExist)
                  {
                    this.message = NO_GATT_CLIENT_OBJECT;
                    return;
                  }
                  this.currentClick =12;
                  if (this.isOnBLECharacteristicChangeClick) {
                    this.gattClientInstance.off('BLECharacteristicChange', () => {
                    });
                    this.message = 'BLECharacteristicChange is off, re-clicking for turning it on!';
                    this.isOnBLECharacteristicChangeClick = false;
                    this.btOnBLECharacteristicChange = 'on:BLECharacteristicChange'
                    // AppStorage.SetOrCreate('on_BLEConnectionStateChange', this.btOnBLEConnectionStateChange);
                    return;
                  }
                  this.isOnBLECharacteristicChangeClick = true;
                  this.btOnBLECharacteristicChange = 'off:BLECharacteristicChange'

                  this.gattClientInstance.on('BLECharacteristicChange', (data) => {
                    this.btOnBLECharacteristicChange = 'off:BLECharacteristicChange'
                    this.bleCharacteristicChangeInfo = 'BLECharacteristicChange:\n';

                    let serviceUuid = data.serviceUuid;
                    let characteristicUuid = data.characteristicUuid;
                    this.bleCharacteristicChangeInfo += "serviceUuid:"+serviceUuid+"\n";
                    this.bleCharacteristicChangeInfo += "characteristicUuid:" + characteristicUuid+"\n";
                    let value = new Uint8Array(data.characteristicValue);
                    for(let i = 0; i <data.characteristicValue.byteLength;i++)
                    {
                      this.bleCharacteristicChangeInfo +="\n"+'value:'+value[i]
                    }
                  })
                  this.message = 'BLECharacteristicChange is on(setNotifyCharacteristicChanged firstly)';
                })

                ListItem() {
                  TitleComponent({
                    title: "setBLEMtuSize(mtu: number): boolean",
                    bgColor: this.currentClick === 13 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                  });
                }
                .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
                .onClick(() => {

                  let btStatus = bluetooth.getState();
                  if (btStatus === bluetooth.BluetoothState.STATE_OFF) {
                    this.message = "BT is not enabled!";
                    return;
                  }
                  if(!this.gattClientExist)
                  {
                    this.message = NO_GATT_CLIENT_OBJECT;
                    return;
                  }
                  this.currentClick = 13;
                  this.message = 'setBLEMtuSize: \n';
                  this.mtuSize = 256;
                  let rv = this.gattClientInstance.setBLEMtuSize(this.mtuSize);
                  this.message += "result = "+rv;
                })

                ListItem() {
                  TitleComponent({
                    title: "setNotifyCharacteristicChanged(...): boolean",
                    bgColor: this.currentClick === 14 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                  });
                }
                .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
                .onClick(() => {
                  let btStatus = bluetooth.getState();
                  if (btStatus === bluetooth.BluetoothState.STATE_OFF) {
                    this.message = "BT is not enabled!";
                    return;
                  }
                  if(!this.gattClientExist)
                  {
                    this.message = NO_GATT_CLIENT_OBJECT;
                    return;
                  }
                  this.currentClick = 14;

                  let descriptors = [];
                  let bufferCCC = new ArrayBuffer(8);
                  let cccV = new Uint8Array(bufferCCC);
                  cccV[0] = 66;
                  let descriptor = {
                    serviceUuid: this.serviceUUID,
                    characteristicUuid: this.characteristicUUID,
                    descriptorUuid: this.descriptorUUID,
                    descriptorValue: string2ArrayBuffer(this.descriptorValue)}
                  descriptors.push(descriptor);

                  let characteristicIn = {
                    serviceUuid: this.serviceUUID,
                    characteristicUuid: this.characteristicUUID,
                    characteristicValue: bufferCCC,
                    descriptors:descriptors};
                  if(this.gattClientInstance.setNotifyCharacteristicChanged(characteristicIn,true))
                  {
                    this.message = "setNotifyCharacteristicChanged(true):succeeds."
                  }
                  else
                  {
                    this.message = "setNotifyCharacteristicChanged(true):failed!"
                    if(this.gattClientInstance.setNotifyCharacteristicChanged(characteristicIn,false))
                    {
                      this.message = "setNotifyCharacteristicChanged(false):true!"
                    }
                    else
                    {
                      this.message = "setNotifyCharacteristicChanged(false):false!"
                    }
                  }
                })

                ListItem() {
                  TitleComponent({
                    title: "getDeviceName(callback): void",
                    bgColor: this.currentClick === 15 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                  });
                }
                .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
                .onClick(() => {
                  if(!this.gattClientExist)
                  {
                    this.message = NO_GATT_CLIENT_OBJECT;
                    return;
                  }
                  this.currentClick =15;
                  this.gattClientInstance.getDeviceName( (err,asyncRemoteDeviceName) => {
                    this.message = "return code:"
                    this.message += JSON.stringify(err);
                    this.message +=",and DeviceName is: "
                    this.message += JSON.stringify(asyncRemoteDeviceName);
                  });
                })

                ListItem() {
                  TitleComponent({
                    title: "getDeviceName(): Promise<string>",
                    bgColor: this.currentClick === 16 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                  });
                }
                .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
                .onClick(() => {

                  if(!this.gattClientExist)
                  {
                    this.message = NO_GATT_CLIENT_OBJECT;
                    return;
                  }
                  this.currentClick = 16;
                  let rv =  this.gattClientInstance.getDeviceName().then((peripheralDeviceName) =>
                  {
                    this.message = "getDeviceName()Promise:"+ peripheralDeviceName;
                  });
                })

                ListItem() {
                  TitleComponent({
                    title: "getRssiValue(callback): void",
                    bgColor: this.currentClick === 17 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                  });
                }
                .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
                .onClick(() => {
                  if(!this.gattClientExist)
                  {
                    this.message = NO_GATT_CLIENT_OBJECT;
                    return;
                  }
                  this.currentClick = 17;
                  this.message ="getRssiValue callback:\n";
                  this.gattClientInstance.getRssiValue((err,cbRssi) => {
                    this.message = "return code:"
                    this.message += JSON.stringify(err);
                    this.message +=",and RSSI="
                    this.message += JSON.stringify(cbRssi);
                  });
                })

                ListItem() {
                  TitleComponent({
                    title: "getRssiValue(): Promise<number>",
                    bgColor: this.currentClick === 18 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                  });
                }
                .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
                .onClick(() => {
                  this.currentClick = 18;
                  if(!this.gattClientExist)
                  {
                    this.message = NO_GATT_CLIENT_OBJECT;
                    return;
                  }
                  this.message ="getRssiValue promise:\n";
                  let rssi = this.gattClientInstance.getRssiValue().then((rssiValue) => {
                    this.message +='rssi' + JSON.stringify(rssiValue)
                  })
                })
              }
              .margin({ bottom: "150vp" })
            }
            .width(ConfigData.WH_100_100)
            .align(Alignment.TopStart)
            .layoutWeight(1)
            .margin({ top: $r('sys.float.ohos_id_card_margin_middle') })
            .height("40%")
          }
          .visibility(Visibility.Visible);

    }
    .backgroundColor($r("sys.color.ohos_id_color_sub_background"))
    .height('100%')
  }
}

/**
* string to ArrayBuffer
* @param {*} str, the string object to    
* @return:  ArrayBuffer
*/
function string2ArrayBuffer (str) {
  let array = new Uint8Array(str.length);
  for(var i = 0; i < str.length; i++) {
    array[i] = str.charCodeAt(i);
  }
  return array.buffer
}
// example: sleep(500).then(()=>{/*TODO after sleeping*/});
function sleep (time) {
  return new Promise((resolve) => setTimeout(resolve, time));
}
