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
 * Gatt Server Test Page Of Bluetooth test
 */
import { TitleComponent } from '../../../../Component/titleComponent';
import HeadComponent from '../../../../component/headcomponent';
import BluetoothModel from '../../../model/BluetoothModel';
import Router from '@system.router';
import BluetoothDevice from '../../../model/BluetoothDevice'
import BluetoothDeviceController from '../../../controller/BluetoothDeviceController';
import ConfigData from '../../../../Utils/ConfigData';
import LogUtil from '../../../../Utils/LogUtil';
import bluetooth from '@ohos.bluetooth';


const NO_GATT_Server_OBJECT = "No GattServer object, please CreateGattServer firstly!";


@Entry
@Component
struct GattServer {
  @State message: string = 'GattServer'
  @State currentClick: number = -1;

  private connectStateMsg: string = 'initial';
  private btOnConnectState: string = 'on:connectStateChange';
  private isConnectStateChangeClick: boolean = false;

  private characteristicReadMsg: string = 'initial';
  private btOnCharacteristicRead: string = 'on:characteristicRead';
  private isCharacteristicReadClick: boolean = false;

  private characteristicWriteMsg: string = 'initial';
  private btOnCharacteristicWrite: string = 'on:characteristicWrite';
  private isCharacteristicWriteClick: boolean = false;

  private descriptorReadMsg: string = 'initial';
  private btOnDescriptorRead: string = 'on:descriptorRead';
  private isDescriptorReadClick: boolean = false;

  private descriptorWriteMsg: string = 'initial';
  private btOnDescriptorWrite: string = 'on:descriptorWrite';
  private isDescriptorWriteClick: boolean = false;

 //----------all @state -> private-------------------

 // GATT service
  private curServiceUUID:         string = '00001877-0000-1000-8000-00805F9B34FB';
  private curServiceIsPrimary:    boolean= true;
  private curCharacteristicUUID:  string = '00002BE0-0000-1000-8000-00805F9B34FB';
  private curCharacteristicValue: string = 'OHOS:BLE NANJING CHINA ';
  private curDescriptorUUID:      string = '00002902-0000-1000-8000-00805F9B34FB';
  private curDescriptorValue:     string = 'Descriptor';

  private curGattService: bluetooth.GattService = {
    serviceUuid:'',
    isPrimary: true,
    characteristics:[],
    includeServices:[]
  }

  // Notify Characteristic
  private notifyCharacteristic_serviceUuid:string         = '00001810-0000-1000-8000-00805F9B34FB';
  private notifyCharacteristic_characteristicUuid:string  = '00001821-0000-1000-8000-00805F9B34FB';
  private notifyCharacteristic_characteristicValue:string = 'test';
  private notifyCharacteristic_confirm:boolean            =  false;
  private notifyCharacteristicDeviceId:string             = '88:36:CF:09:C1:90';
  private notifyCharacteristic_notifyCharacteristic:bluetooth.NotifyCharacteristic = {
    serviceUuid: '00001810-0000-1000-8000-00805F9B34FB',
    characteristicUuid: '00001821-0000-1000-8000-00805F9B34FB',
    characteristicValue: string2ArrayBuffer("999"),
    confirm: false
  }

  // ServerResponse
  private serverResponse_deviceId:string = '88:36:CF:09:C1:90';
  private serverResponse_transId:string = '1';
  private serverResponse_status:string = '0';
  private serverResponse_offset:string = '0';
  private serverResponse_value:string 	= 'respond value';

  private serverResponse:bluetooth.ServerResponse ={
    deviceId: '88:36:CF:09:C1:90',
    transId: 1,
    status: 0,
    offset: 0,
    value: string2ArrayBuffer("respond value")
};

  // Advertising input parameters:
  /*settings*/
  private cbxAdvSettings:boolean =false;
  private cbxAdvResponse:boolean =false;

  private cbxInterval:boolean = false;
  private advSettings_interval:string = "1600"; //default: 1s

  private cbxTxPower:boolean = false;
  private advSettings_txPower:string = "-7"; //default: -7 dbm, range:-127~1

  private cbxConnectable:boolean = true;
  private advSettings_connectable:boolean = true; //default: true
 // @State selectAdvSettings_connectableIndex:number = 0;

  /* Adv Data*/
  private advDataServiceUuids_serviceUuid:string = "000054F6-0000-1000-8000-00805f9b34fb";
  private advDataManufactureData_manufactureId:string = "7456";
  private advDataManufactureData_manufactureValue:string = "CHN";
  private advDataServiceData_serviceUuid:string = "000054F6-0000-1000-8000-00805f9b34fb";
  private advDataServiceData_serviceValue:string = "AdvValue";

  /*Adv Response*/
  private advResponseServiceUuids_serviceUuid:string = "000054F6-0000-1000-8000-00805f9b34fb";
  private advResponseManufactureData_manufactureId:string = "6547";
  private advResponseManufactureData_manufactureValue:string = "CHN";
  private advResponseServiceData_serviceUuid:string = "000054F6-0000-1000-8000-00805f9b34fb";
  private advResponseServiceData_serviceValue:string = "AdvValue";



  private curAdvSettings: bluetooth.AdvertiseSetting = {
    interval:1600,  //500ms
    txPower:-7 ,
    connectable:true
  };
  private curAdvData: bluetooth.AdvertiseData = {
    serviceUuids:["00003344-0000-1000-8000-00805f9b34fb","00008899-0000-1000-8000-00805f9b34fb"],
    manufactureData:[{
                       manufactureId:7456,
                       manufactureValue:string2ArrayBuffer("OHOS CHN")
                     }],
    serviceData:[
                 {
                   serviceUuid:"000054F6-0000-1000-8000-00805f9b34fb",
                   serviceValue:string2ArrayBuffer("OhosBle!")
                 },
                 {
                   serviceUuid:"00009999-0000-1000-8000-00805f9b34fb",
                   serviceValue:string2ArrayBuffer("dudu_tiger")
                 }]
  };
  private curAdvResponse:bluetooth.AdvertiseData ={
    serviceUuids:[//"000054F6-0000-1000-8000-00805f9b34fb",
                  "00009999-0000-1000-8000-00805f9b34fb"],
    manufactureData:[{
                       manufactureId:6547,
                       manufactureValue:string2ArrayBuffer("OHOS CHN")
                     }],
    serviceData:[{
                   serviceUuid:"000054F6-0000-1000-8000-00805f9b34fb",
                   serviceValue:string2ArrayBuffer("OhosBle!")
                 },
                 {
                   serviceUuid:"00007456-0000-1000-8000-00805f9b34fb",
                   serviceValue:string2ArrayBuffer("OhosBle")
                 }]
  };


//------------@State->private------------------------------

  private gattServerInstance = null;

  private gattServerExist = false;



  //@StorageLink('on_BLEConnectionStateChange')

  build() {

      Column() {
        HeadComponent({ headName: 'GattServer Test', isActive: true });
        Stack().height("0.5vp").backgroundColor("#000000");

      Column() {
        Text("Input Gatt Service Data:")
          .margin({top: "1vp"})
          .padding("1vp")
          .fontSize("15vp")
          .fontColor("#4444444")
          .height("30vp");
        Scroll(){
          Column() {
            Flex({ direction: FlexDirection.Row }) {
              Text("ServiceUUID:").fontSize("15vp").width("150vp")
              TextInput({ text: this.curServiceUUID, placeholder: "input service UUID" })
                .fontSize("15vp")
                .onChange((strInput) => {
                  this.curServiceUUID = strInput;
                })
                .margin( { top: "1vp", left: "5vp", right: "50vp" } )
            }
            .height(30)
            .width('100%')
            .padding(5)
            .backgroundColor(0xAFEEEE);

            Flex({ direction: FlexDirection.Column }) {
              Row() {
                Text("CCC UUID:").fontSize("15vp")
                TextInput({ text: this.curCharacteristicUUID, placeholder: "input characteristic UUID" })
                  .fontSize("15vp")
                  .onChange((strInput) => {
                    this.curCharacteristicUUID = strInput;
                  })
                  .margin( { top: "1vp", left: "5vp", right: "50vp" } )
              }
              .margin( { top: "1vp", left: "5vp", right: "50vp" } )

              Row() {
                Text("CCC Value:").fontSize("15vp")
                TextInput({ text: this.curCharacteristicValue, placeholder: "input characteristic Value" })
                  .fontSize("15vp")
                  .onChange((strInput) => {
                    this.curCharacteristicValue = strInput;
                  })
                  .margin( { top: "1vp", left: "5vp", right: "50vp" } )
              }
              .margin( { top: "1vp", left: "5vp", right: "50vp" } )
            }
            .height(60)
            .width('100%')
            .padding(5)
            .backgroundColor(0xAFEEEE);

            Flex({ direction: FlexDirection.Column }) {
              Row() {
                Text("Descriptor UUID:").fontSize("15vp")
                TextInput({ text: this.curDescriptorUUID, placeholder: "input descriptor UUID" })
                  .fontSize("15vp")
                  .onChange((strInput) => {
                    this.curDescriptorUUID = strInput;
                  })
                  .margin( { top: "1vp", left: "5vp", right: "50vp" } )
              }
              .margin( { top: "1vp", left: "5vp", right: "50vp" } )
              Row() {
                Text("Descriptor Value:").fontSize("15vp")
                TextInput({ text: this.curDescriptorValue, placeholder: "input descriptor Value" })
                  .fontSize("15vp")
                  .onChange((strInput) => {
                    this.curDescriptorValue = strInput;
                  })
                  .margin( { top: "1vp", left: "5vp", right: "50vp" } )
              }
              .margin( { top: "1vp", left: "5vp", right: "50vp" } )
            }
            .height(60)
            .width('100%')
            .padding(5)
            .backgroundColor(0xAFEEEE);

          }
        }.height("80vp")
        Stack().height("0.25vp").backgroundColor("#000000");
        Text("Input advertiser data:")
          .margin({top: "1vp"})
          .padding("5vp")
          .fontSize("15vp")
          .fontColor("#4444444")
          .height("30vp");
        Stack().height("0.25vp").backgroundColor("#000000");
        Scroll(){
          Column() {
            Flex({ direction: FlexDirection.Row }){
              Checkbox({ name: 'AdvSettings', group: 'checkboxGroup1' })
                .select(this.cbxAdvSettings)
                .selectedColor(0x39a2db)
                .onChange((value) => {
                  if(value)
                  {
                    this.cbxAdvSettings =true;
                  }
                  else
                  {
                    this.cbxAdvSettings =false;
                  }

                  }).width("15vp").margin( { left: "1vp" } )
                Text("settings:")
                  .margin({top: "5vp",left: "5vp"})
                  .padding("5vp")
                  .fontSize("15vp")
                  .fontColor("#4444444")
                ;
              }
              .height(40)
              .width('100%')
              .padding(5);
              Flex({ direction: FlexDirection.Row }){
                Row(){
                  Checkbox({ name: 'interval', group: 'checkboxGroup1' })
                    .select(this.cbxInterval)
                    .selectedColor(0x39a2db)
                    .onChange((value) => {
                      if(value)
                      {
                        this.cbxInterval =true;
                      }
                      else
                      {
                        this.cbxInterval =false;
                      }
                    }).width("15vp").margin( { left: "1vp" } )
                  Text("interval").fontSize("10vp").width("40vp")
                  TextInput({ text: this.advSettings_interval, placeholder: "32~1600" })
                    .fontSize("10vp")
                    .onChange((strInput) => {
                      this.advSettings_interval = strInput;
                    })
                    .width("60vp")
                }
                .margin( { top: "10vp", left: "5vp", right: "20vp" } )
                .alignItems(VerticalAlign.Center)
                Row(){
                  Checkbox({ name: 'txPower', group: 'checkboxGroup' })
                    .select(this.cbxTxPower)
                    .selectedColor(0x39a2db)
                    .onChange((value) => {
                      if(value)
                      {
                        this.cbxTxPower =true;
                      }
                      else
                      {
                        this.cbxTxPower =false;
                      }
                    }).width("15vp").margin( { left: "1vp" } )
                  Text("txPower").fontSize("10vp").width("42vp")
                  TextInput({ text: this.advSettings_txPower.toString(), placeholder: "input txPower" })
                    .fontSize("10vp")
                    .onChange((strInput) => {
                      this.advSettings_txPower = strInput;
                    })
                    .width("60vp")
                }
                .margin( { top: "10vp", left: "5vp", right: "20vp" } )
                .alignItems(VerticalAlign.Center)
                Row(){
                  Checkbox({ name: 'connectable', group: 'checkboxGroup' })
                    .select(this.cbxConnectable)
                    .selectedColor(0x39a2db)
                    .onChange((value) => {
                      if(value)
                      {
                        this.cbxConnectable =true;
                      }
                      else
                      {
                        this.cbxConnectable =false;
                      }
                      this.advSettings_connectable = this.cbxConnectable;

                    }).width("15vp").margin( { left: "1vp" } )
                  Text("connectable").fontSize("10vp").width("60vp")
                }
                .margin( { top: "1vp", left: "1vp", right: "20vp" } )
                .alignItems(VerticalAlign.Center)
              }
              .height(50)
              .width('100%')
              .padding(5)
              .backgroundColor(0xAFEEEE);

              Text("Adv Data:")
                .margin({top: "5vp",left: "5vp"})
                .padding("5vp")
                .fontSize("15vp")
                .fontColor("#4444444");
              Flex({ direction: FlexDirection.Column }){
                Row(){
                  Text("serviceUuid").fontSize("16vp")//.width("40vp")
                  TextInput({ text: this.advDataServiceUuids_serviceUuid, placeholder: "input serviceUuid" })
                    .fontSize("10vp")
                    .onChange((strInput) => {
                      this.advDataServiceUuids_serviceUuid = strInput;
                    })
                  // .width("60vp")
                }
                Column(){
                  Text("manufactureData").fontSize("16vp")//.width("42vp")
                  Row(){
                    Text("Id").fontSize("15vp").width("42vp")
                    TextInput({ text: this.advDataManufactureData_manufactureId, placeholder: "input manufactureId(less than 8 digi)" })
                      .fontSize("10vp")
                      .onChange((strInput) => {
                        this.advDataManufactureData_manufactureId  = strInput;
                      })
                      .width("200vp")
                    Text("Value").fontSize("15vp").width("42vp")
                    TextInput({ text: this.advDataManufactureData_manufactureValue, placeholder: "input manufactureValue(less than 8 char)" })
                      .fontSize("10vp")
                      .onChange((strInput) => {
                        this.advDataManufactureData_manufactureValue  = strInput;
                      })
                      .width("200vp")
                  }
                }
                Column() {
                  Text("serviceData").fontSize("16vp")//.width("42vp")
                  Row(){
                    Row(){
                      Text("serviceUuid").fontSize("15vp")
                      TextInput({ text: ''+this.advDataServiceData_serviceUuid, placeholder: "input serviceUuid" })
                        .fontSize("10vp")
                        .onChange((strInput) => {
                          this.advDataServiceData_serviceUuid  = strInput;
                        })
                        .width("200vp")
                    }
                    Row(){
                      Text("serviceValue").fontSize("15vp")
                      TextInput({ text: this.advDataServiceData_serviceValue, placeholder: "input manufactureValue(less than 8 char)" })
                        .fontSize("10vp")
                        .onChange((strInput) => {
                          this.advDataServiceData_serviceValue = strInput;
                        })
                        .width("200vp")
                    }
                  }
                }

              }
              .height(160)
              .width('100%')
              .padding(5)
              .backgroundColor(0xAFEEEE);

              Flex({ direction: FlexDirection.Row }){
                Checkbox({ name: 'AdvResponse', group: 'checkboxGroup1' })
                  .select(this.cbxAdvResponse)
                  .selectedColor(0x39a2db)
                  .onChange((value) => {
                    if(value)
                    {
                      this.cbxAdvResponse =true;
                    }
                    else
                    {
                      this.cbxAdvResponse =false;
                    }

                  }).width("20vp").margin( { left: "1vp" } )
                Text("Response:")
                  .margin({top: "5vp",left: "5vp"})
                  .padding("5vp")
                  .fontSize("15vp")
                  .fontColor("#4444444");
              }
              .height(40)
              .width('100%')
              .padding(5);
              Flex({ direction: FlexDirection.Column }){
                Row(){
                  Text("serviceUuid").fontSize("16vp")//.width("40vp")
                  TextInput({ text: this.advResponseServiceUuids_serviceUuid, placeholder: "input serviceUuid" })
                    .fontSize("10vp")
                    .onChange((strInput) => {
                      this.advResponseServiceUuids_serviceUuid = strInput;
                    })
                  // .width("60vp")
                }
                Column(){
                  Text("manufactureData").fontSize("16vp")//.width("42vp")
                  Row(){
                    Text("Id").fontSize("15vp").width("42vp")
                    TextInput({ text:this.advResponseManufactureData_manufactureId, placeholder: "input manufactureId(less than 8 char)" })
                      .fontSize("10vp")
                      .onChange((strInput) => {
                        this.advResponseManufactureData_manufactureId  = strInput;
                      })
                      .width("200vp")
                    Text("Value").fontSize("15vp").width("42vp")
                    TextInput({ text: this.advResponseManufactureData_manufactureValue, placeholder: "input manufactureValue(less than 8 char)" })
                      .fontSize("10vp")
                      .onChange((strInput) => {
                        this.advResponseManufactureData_manufactureValue  = strInput;
                      })
                      .width("200vp")
                  }
                }
                Column() {
                  Text("serviceData").fontSize("16vp")//.width("42vp")
                  Row(){
                    Row(){
                      Text("serviceUuid").fontSize("15vp")
                      TextInput({ text: ''+this.advResponseServiceData_serviceUuid, placeholder: "input serviceUuid" })
                        .fontSize("10vp")
                        .onChange((strInput) => {
                          this.advResponseServiceData_serviceUuid  = strInput;
                        })
                        .width("200vp")
                    }
                    Row(){
                      Text("serviceValue").fontSize("15vp")
                      TextInput({ text: this.advResponseServiceData_serviceValue, placeholder: "input manufactureValue(less than 8 char)" })
                        .fontSize("10vp")
                        .onChange((strInput) => {
                          this.advResponseServiceData_serviceValue = strInput;
                        })
                        .width("200vp")
                    }
                  }
                }

              }
              .height(160)
              .width('100%')
              .padding(5)
              .backgroundColor(0xAFEEEE);


            }
          }.height("80vp")
          Stack().height("0.25vp").backgroundColor("#000000");
          Column(){
            Text("Result:" + this.message)
              .fontSize("15vp")
              .margin( { top: "1vp", left: "10vp", right: "20vp" } )
              .textAlign(TextAlign.Start)
              .backgroundColor($r("sys.color.ohos_id_color_foreground_contrary"))
            Text("ConnectionStateChange:" + this.connectStateMsg)
              .fontSize("15vp")
              .margin( { top: "1vp", left: "10vp", right: "20vp" } )
              .textAlign(TextAlign.Start)
              .backgroundColor($r("sys.color.ohos_id_color_foreground_contrary"))
          }
          Row(){
            Text("characteristicRead:" + this.characteristicReadMsg)
              .fontSize("15vp")
              .margin( { top: "1vp", left: "10vp", right: "20vp" } )
              .textAlign(TextAlign.Start)
              .backgroundColor($r("sys.color.ohos_id_color_foreground_contrary"))
            Text("characteristicWrite:" + this.characteristicWriteMsg)
              .fontSize("15vp")
              .margin( { top: "1vp", left: "10vp", right: "20vp" } )
              .textAlign(TextAlign.Start)
              .backgroundColor($r("sys.color.ohos_id_color_foreground_contrary"))
          }
          Row(){
            Text("descriptorRead:" + this.descriptorReadMsg)
              .fontSize("15vp")
              .margin( { top: "1vp", left: "10vp", right: "20vp" } )
              .textAlign(TextAlign.Start)
              .backgroundColor($r("sys.color.ohos_id_color_foreground_contrary"))
            Text("descriptorWrite:\n" + this.descriptorWriteMsg)
              .fontSize("15vp")
              .margin( { top: "1vp", left: "10vp", right: "20vp" } )
              .textAlign(TextAlign.Start)
              .backgroundColor($r("sys.color.ohos_id_color_foreground_contrary"))
          }

          Stack().height("0.25vp").backgroundColor("#000000");
          Scroll() {
            List({ space: 1 }) {
              ListItem() {
                TitleComponent({
                  title: "CreateGattServer()",
                  bgColor: this.currentClick === 99 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 99;
                let btState = bluetooth.getState();
                if (btState === bluetooth.BluetoothState.STATE_ON) {
                  if (this.gattServerExist)
                  {
                    this.message = "GattServer exists!";
                    return;
                  }
                  this.gattServerInstance = BluetoothModel.createGattServer();
                  this.message = "CreateGattServer():succeeds!";
                  this.gattServerExist = true;
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
                  title: "startAdvertising(...): void",
                  bgColor: this.currentClick === 0 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 0;
                let btState = bluetooth.getState();
                if (btState === bluetooth.BluetoothState.STATE_ON) {
                  if (!this.gattServerExist) {
                    this.message = NO_GATT_Server_OBJECT;
                  }
                  this.message = "startAdvertising:build input parameter In!";
                  this.buildAdvSettings();
                  this.buildAdvData();
                  this.buildAdvResponse();

                let setting = this.curAdvSettings;

                let advData = this.curAdvData;
                let advResponse = this.curAdvResponse;
                this.message = "startAdvertising:build input parameter Out!";

                if(!this.cbxAdvResponse)
                {
                  this.gattServerInstance.startAdvertising(setting,advData);
                }
                else
                {
                  this.gattServerInstance.startAdvertising(setting,advData,advResponse);
                }

                  this.message = "startAdvertising succeeds!";
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
                  title: "stopAdvertising(): void",
                  bgColor: this.currentClick === 1 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 1;
                this.message = "stopAdvertising():";
                let btState = bluetooth.getState();
                if (btState == bluetooth.BluetoothState.STATE_ON) {
                  if(!this.gattServerExist)
                  {
                    this.message = NO_GATT_Server_OBJECT;
                    return;
                  }
                  this.gattServerInstance.stopAdvertising();
                  this.message = "stopAdvertising() succeeds.";
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
                  title: "addService(service: GattService): boolean",
                  bgColor: this.currentClick === 2 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 2;
                let btStatus = bluetooth.getState();
                if (btStatus === bluetooth.BluetoothState.STATE_OFF) {
                  this.message = "BT is not enabled!";
                  return;
                }

                if(!this.gattServerExist)
                {
                  this.message = NO_GATT_Server_OBJECT;
                  return;
                }
                // build service
                this.buildGattService();
                let rv = this.gattServerInstance.addService(this.curGattService);
                this.message = "result: addService(),"+rv;

              })

              ListItem() {
                TitleComponent({
                  title: "removeService(serviceUuid: string): boolean",
                  bgColor: this.currentClick === 3 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 3;
                this.message = "removeService(...):";
                let btState = bluetooth.getState();
                if (btState == bluetooth.BluetoothState.STATE_ON) {
                  if(!this.gattServerExist)
                  {
                    this.message = NO_GATT_Server_OBJECT;
                    return;
                  }

                  this.message = "result: removeService() ";
                  if(this.gattServerInstance.removeService(this.curServiceUUID))
                  {
                    this.message += 'true';
                  }
                  else
                  {
                    this.message += 'false';
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
                  title: "close(): void",
                  bgColor: this.currentClick === 4 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 4;
                let btState = bluetooth.getState();
                if (btState == bluetooth.BluetoothState.STATE_ON) {
                  if(!this.gattServerExist)
                  {
                    this.message = NO_GATT_Server_OBJECT;
                    return;
                  }
                  this.gattServerInstance.close();
                  this.message = "GattServer closed: succeeds";
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
                  title: this.btOnConnectState,
                  bgColor: this.currentClick === 5 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 5;
                let btStatus = bluetooth.getState();
                if (btStatus === bluetooth.BluetoothState.STATE_OFF) {
                  this.message = "BT is not enabled!";
                  return;
                }
                if (this.isConnectStateChangeClick) {
                  this.gattServerInstance.off("connectStateChange", () => {
                  });
                  this.message = 'connectStateChange is off!';
                  this.isConnectStateChangeClick = false;
                  this.btOnConnectState = 'on:connectStateChange'
                  return;
                }
                this.isConnectStateChangeClick = true;
                this.btOnConnectState = 'off:connectStateChange'

                this.gattServerInstance.on('connectStateChange', (data) => {
                  this.connectStateMsg = "deviceId:" + data.deviceId + ",connectState:" + data.state ;
                })
                this.message = 'connectStateChange is on';
              })

              ListItem() {
                TitleComponent({
                  title: this.btOnCharacteristicRead,
                  bgColor: this.currentClick === 6 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 6;
                let btStatus = bluetooth.getState();
                if (btStatus === bluetooth.BluetoothState.STATE_OFF) {
                  this.message = "BT is not enabled!";
                  return;
                }
                if (this.isCharacteristicReadClick) {
                  this.gattServerInstance.off("characteristicRead", () => {
                  });
                  this.message = 'characteristicRead is off!';
                  this.isCharacteristicReadClick = false;
                  this.btOnCharacteristicRead = 'on:characteristicRead'
                  return;
                }
                this.isCharacteristicReadClick = true;
                this.btOnCharacteristicRead = 'off:characteristicRead'

                this.gattServerInstance.on('characteristicRead', (characteristicReadReq) => {
                  this.characteristicReadMsg = 'characteristicRead received!';
                  // client address
                  let clientDeviceId = characteristicReadReq.deviceId;
                  // transId
                  let transId = characteristicReadReq.transId;
                  // the byte offset of the start position for reading characteristic value
                  let offset = characteristicReadReq.offset;
                  // service/characteristic Uuid
                  let characteristicUuid = characteristicReadReq.characteristicUuid;
                  let serviceUuid = characteristicReadReq.serviceUuid;

                  this.characteristicReadMsg += 'characteristicRead input parameters: deviceId = ' + clientDeviceId+'\n';
                  this.characteristicReadMsg += 'transId = '+ transId+'\n';
                  this.characteristicReadMsg +="characteristicUuid = "+ characteristicUuid+'\n';
                  this.characteristicReadMsg +="serviceUuid = "+ serviceUuid+'\n';
                  // build data for response
                  let arrayBufferCCC = string2ArrayBuffer('ResponseForCharacteristicRead');


                  let serverResponse = {deviceId: clientDeviceId, transId: transId, status: 0, offset: offset, value:arrayBufferCCC};

                  let ret = this.gattServerInstance.sendResponse(serverResponse);
                  let rvMsg = 'failed';
                  if (ret) {
                    rvMsg = 'succeeded';
                  }
                  this.characteristicReadMsg = 'characteristicRead sendResponse:'+ rvMsg;
                })
                this.message = 'characteristicRead on!'
              })

              ListItem() {
                TitleComponent({
                  title: this.btOnCharacteristicWrite,
                  bgColor: this.currentClick === 7 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 7;
                let btStatus = bluetooth.getState();
                if (btStatus === bluetooth.BluetoothState.STATE_OFF) {
                  this.message = "BT is not enabled!";
                  return;
                }
                if (this.isCharacteristicWriteClick) {
                  this.gattServerInstance.off("characteristicWrite", () => {
                  });
                  this.message = 'characteristicWrite is off!';
                  this.isCharacteristicWriteClick = false;
                  this.btOnCharacteristicWrite = 'on:characteristicWrite'
                  return;
                }
                this.isCharacteristicWriteClick = true;
                this.btOnCharacteristicWrite = 'off:characteristicWrite'

                this.gattServerInstance.on('characteristicWrite', (characteristicWriteReq) => {
                  this.btOnCharacteristicWrite = 'off:characteristicWrite'
                  this.characteristicWriteMsg = '';

                  let arrayBufferCCC = string2ArrayBuffer("characteristicWriteForResponse");

                  // client address
                  let deviceId = characteristicWriteReq.deviceId;
                  let transId = characteristicWriteReq.transId;
                  let offset = characteristicWriteReq.offset;
                  /** Whether this request should be pending for later operation */
                  let isPrep = characteristicWriteReq.isPrep;
                  /** Whether the remote client need a response */
                  let needRsp = characteristicWriteReq.needRsp;
                  /** Indicates the value to be written */
                  let value =  new Uint8Array(characteristicWriteReq.value);
                  let characteristicUuid = characteristicWriteReq.characteristicUuid;
                  let serviceUuid = characteristicWriteReq.serviceUuid;


                  this.characteristicWriteMsg += "The characteristicWriteReq input parameters: deviceId ="+ deviceId;
                  this.characteristicWriteMsg += "transId =" + transId;
                  this.characteristicWriteMsg += "offset ="+ offset;
                  this.characteristicWriteMsg += "isPrep ="+ isPrep;
                  this.characteristicWriteMsg += "needRsp ="+ needRsp;
                  this.characteristicWriteMsg += "value ="+ JSON.stringify(value);
                  this.characteristicWriteMsg += "characteristicUuid =" + characteristicUuid;
                  this.characteristicWriteMsg += "serviceUuid ="+ serviceUuid;
                  let serverResponse = {deviceId: deviceId, transId: transId, status: 0, offset: offset, value:arrayBufferCCC};
                  // sendResponse
                  if(needRsp)
                  {
                    let ret = this.gattServerInstance.sendResponse(serverResponse);
                    if (ret) {
                      this.message = 'characteristicWrite sendResponse successfully';
                    } else {
                      this.message = 'characteristicWrite sendResponse failed';
                    }
                  }

                  this.notifyCharacteristic_notifyCharacteristic = {
                    serviceUuid: serviceUuid,
                    characteristicUuid:  characteristicUuid,
                    characteristicValue: string2ArrayBuffer("Value4notifyCharacteristic"),
                    confirm: true //notification:true; indication:false;
                  }
                  let notifyCharacteristic = this.notifyCharacteristic_notifyCharacteristic;

                  if (this.gattServerInstance.notifyCharacteristicChanged(deviceId, notifyCharacteristic))
                  {
                     this.message= "notifyCharacteristicChanged successfully!";
                  }
                  else
                  {
                     this.message= "notifyCharacteristicChanged failed!";
                  }
                })
                this.message = 'characteristicWrite is on';
              })

              ListItem() {
                TitleComponent({
                  title: this.btOnDescriptorRead,
                  bgColor: this.currentClick === 8 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 8;
                let btStatus = bluetooth.getState();
                if (btStatus === bluetooth.BluetoothState.STATE_OFF) {
                  this.message = "BT is not enabled!";
                  return;
                }
                if (this.isDescriptorReadClick) {
                  this.gattServerInstance.off("descriptorRead", () => {
                  });
                  this.message = 'descriptorRead is off!';
                  this.isDescriptorReadClick = false;
                  this.btOnDescriptorRead = 'on:descriptorRead'
                  return;
                }
                this.isDescriptorReadClick = true;
                this.btOnDescriptorRead = 'off:descriptorRead'

                this.gattServerInstance.on('descriptorRead', (descriptorReadReq) => {
                  this.btOnDescriptorRead = 'off:descriptorRead'
                  this.descriptorReadMsg = "descriptorReadIn!! \n";
                  let deviceId = descriptorReadReq.deviceId;
                  let transId = descriptorReadReq.transId;
                  /** Indicates the byte offset of the start position for reading characteristic value */
                  let offset = descriptorReadReq.offset;
                  let descriptorUuid = descriptorReadReq.descriptorUuid;
                  let characteristicUuid = descriptorReadReq.characteristicUuid;
                  let serviceUuid = descriptorReadReq.serviceUuid;
                  let arrayBufferDesc = string2ArrayBuffer("Response4descriptorRead");
                  let serverResponse = {deviceId: deviceId, transId: transId, status: 0, offset: offset, value:arrayBufferDesc}
                  let ret = this.gattServerInstance.sendResponse(serverResponse);
                  if (ret) {
                     this.descriptorReadMsg +='descriptorRead sendResponse successfully!!';
                  } else {
                    this.descriptorReadMsg +='descriptorRead sendResponse failed!!';
                  }
                })
                this.message = 'descriptorRead is on';
              })

              ListItem() {
                TitleComponent({
                  title: this.btOnDescriptorWrite,
                  bgColor: this.currentClick === 9 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 9;
                let btStatus = bluetooth.getState();
                if (btStatus === bluetooth.BluetoothState.STATE_OFF) {
                  this.message = "BT is not enabled!";
                  return;
                }
                if (this.isDescriptorWriteClick) {
                  this.gattServerInstance.off("descriptorWrite", () => {
                  });
                  this.message = 'descriptorWrite is off!';
                  this.isDescriptorWriteClick = false;
                  this.btOnDescriptorWrite = 'on:descriptorWrite'
                  return;
                }
                this.isDescriptorWriteClick = true;
                this.btOnDescriptorWrite = 'off:descriptorWrite'

                this.gattServerInstance.on('descriptorWrite', (descriptorWriteReq) => {
                  this.btOnDescriptorWrite = 'off:descriptorWrite'
                  this.descriptorWriteMsg = "descriptorWriteIn";
                  this.descriptorWriteMsg+="The descriptorWriteReq client address: deviceId is "+ descriptorWriteReq.deviceId;

                  let deviceId = descriptorWriteReq.deviceId;
                  let transId = descriptorWriteReq.transId;
                  /** Indicates the byte offset of the start position for writing descriptor value */
                  let offset = descriptorWriteReq.offset;
                  /** Whether this request should be pending for later operation */
                  let isPrep = descriptorWriteReq.isPrep;
                  /** Whether the remote client need a response */
                  let needRsp = descriptorWriteReq.needRsp;
                  /** Indicates the value to be written */
                  let value = new Uint8Array(descriptorWriteReq.value);
                  let descriptorUuid = descriptorWriteReq.descriptorUuid;
                  let characteristicUuid = descriptorWriteReq.characteristicUuid;
                  let serviceUuid = descriptorWriteReq.serviceUuid;

                  let arrayBufferDesc = new ArrayBuffer(8);
                  let descValue = new Uint8Array(arrayBufferDesc);
                  descValue[0] = value[0];
                  let serverResponse = {deviceId: deviceId, transId: transId, status: 0, offset: offset, value:arrayBufferDesc};

                  if(needRsp)
                  {
                    let ret = this.gattServerInstance.sendResponse(serverResponse);
                    if (ret) {
                      this.message = 'descriptorWrite sendResponse successfully';
                    } else {
                      this.message = 'descriptorWrite sendResponse failed';
                    }
                  }
                  this.descriptorWriteMsg+= "\n"+'descriptorWrite Callback finished!';
                })
                this.message = 'descriptorWrite is on';
              })

            }
            .margin({ bottom: "80vp" })
          }
          .width(ConfigData.WH_100_100)
          .align(Alignment.TopStart)
          .layoutWeight(1)
          .margin({ top: $r('sys.float.ohos_id_card_margin_middle') })
        }
        .visibility(Visibility.Visible);
      }
      .width('100%')

  }
  private buildAdvSettings()
  {
    if(this.cbxAdvSettings)
    {
      if(this.cbxInterval)
      {
        this.curAdvSettings.interval = parseInt(this.advSettings_interval);
      }
      else
      {
        this.curAdvSettings.interval = 1600;
      }
      if(this.cbxTxPower)
      {
        this.curAdvSettings.txPower = parseInt(this.advSettings_txPower);
      }
      else
      {
        this.curAdvSettings.txPower = -7;
      }

      this.curAdvSettings.connectable = this.advSettings_connectable;

    }
    else
    {
      this.curAdvSettings.connectable = true;
    }
  }

  private buildAdvData()
  {
  let serviceUuidInstance = this.advDataServiceUuids_serviceUuid;
  this.curAdvData.serviceUuids.length = 0;
  this.curAdvData.serviceUuids.push(serviceUuidInstance);

  let manufactureIdTmp = parseInt(this.advDataManufactureData_manufactureId);
  let manufactureValueTmp= this.advDataManufactureData_manufactureValue;
  let manufactureDataInstance = {
                           manufactureId:manufactureIdTmp,
                           manufactureValue:string2ArrayBuffer(manufactureValueTmp)};
  this.curAdvData.manufactureData.length = 0;
  this.curAdvData.manufactureData.push(manufactureDataInstance);

  let serviceUuid4Service = this.advDataServiceData_serviceUuid;
  let serviceValue4Service = this.advDataServiceData_serviceValue;
  let serviceDataInstance = {
                               serviceUuid:serviceUuid4Service,
                               serviceValue:string2ArrayBuffer(serviceValue4Service)
                            };
  this.curAdvData.serviceData.length = 0;
  this.curAdvData.serviceData.push(serviceDataInstance);
  }

  private buildAdvResponse()
  {
    if(!this.cbxAdvResponse)
    {
      return;
    }
    let serviceUuidInstance = this.advResponseServiceUuids_serviceUuid ;
    this.curAdvResponse.serviceUuids.length = 0;
    this.curAdvResponse.serviceUuids.push(serviceUuidInstance);

    let manufactureIdTmp = parseInt(this.advResponseManufactureData_manufactureId);
    let manufactureValueTmp= this.advResponseManufactureData_manufactureValue;
    let manufactureDataInstance = {
      manufactureId:manufactureIdTmp,
      manufactureValue:string2ArrayBuffer(manufactureValueTmp)};

    this.curAdvResponse.manufactureData.length=0;
    this.curAdvResponse.manufactureData.push(manufactureDataInstance);

    let serviceUuid4Service = this.advResponseServiceData_serviceUuid;
    let serviceValue4Service = this.advResponseServiceData_serviceValue;
    let serviceDataInstance = {
      serviceUuid:serviceUuid4Service,
      serviceValue:string2ArrayBuffer(serviceValue4Service)
    };

    this.curAdvResponse.serviceData.length = 0;
    this.curAdvResponse.serviceData.push(serviceDataInstance);
  }

  private buildGattService()
  {
    // build descriptors
    let descriptors = [];
    let descriptor = {
      serviceUuid:        this.curServiceUUID,
      characteristicUuid: this.curCharacteristicUUID,
      descriptorUuid:     this.curDescriptorUUID,
      descriptorValue:    string2ArrayBuffer(this.curDescriptorValue)
    };
    descriptors.push(descriptor);

      // build characteristics
    let characteristics = [];
    let cccValue = this.curCharacteristicValue;
    let characteristic = {
        serviceUuid: this.curServiceUUID,
        characteristicUuid: this.curCharacteristicUUID,
        characteristicValue:string2ArrayBuffer(cccValue),
        descriptors:descriptors
    };

    characteristics.push(characteristic);
    this.curGattService.serviceUuid     = this.curServiceUUID;
    this.curGattService.characteristics = characteristics;
    this.curGattService.isPrimary       = this.curServiceIsPrimary;
    this.curGattService.includeServices = [];
  }

  private buildNotifyCharacteristic()
  {
    this.notifyCharacteristic_notifyCharacteristic = {
      serviceUuid: this.notifyCharacteristic_serviceUuid,
      characteristicUuid:  this.notifyCharacteristic_characteristicUuid,
      characteristicValue: string2ArrayBuffer(this.notifyCharacteristic_characteristicValue),
      confirm: this.notifyCharacteristic_confirm
    }
  }

  private buildServerResponse()
  {
     this.serverResponse = {
       deviceId: this.serverResponse_deviceId,
       transId: parseInt(this.serverResponse_transId),
       status: parseInt(this.serverResponse_status),
       offset: parseInt(this.serverResponse_offset),
       value:  string2ArrayBuffer(this.serverResponse_value)
     }
  }
}

/**
* string to ArrayBuffer
* @param {*} str: the string to covert 
* @return: ArrayBuffer
*/
function string2ArrayBuffer (str) {
  let array = new Uint8Array(str.length);
  for(var i = 0; i < str.length; i++) {
    array[i] = str.charCodeAt(i);
  }
  return array.buffer
}
