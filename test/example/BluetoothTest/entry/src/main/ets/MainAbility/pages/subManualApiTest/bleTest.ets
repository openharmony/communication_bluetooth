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
 * BLE Test Page Of Bluetooth test
 */
import { TitleComponent } from '../../../Component/titleComponent';
import HeadComponent from '../../../component/headcomponent';
import EntryComponent from '../../../component/EntryComponent';
import BluetoothModel from '../../model/BluetoothModel';
// import Router from '@system.router';
import BluetoothDevice from '../../model/BluetoothDevice'
import BluetoothDeviceController from '../../controller/BluetoothDeviceController';
import ConfigData from '../../../Utils/ConfigData';
import LogUtil from '../../../Utils/LogUtil';
import bluetooth from '@ohos.bluetooth';


@Entry
@Component
struct BleTest {
  @State message: string = 'BleTest Result';
  @State peripheralDeviceId: string  = '88:36:CF:09:C1:90'; 
  @State currentClick: number = -1;
  @State btOnBLEDeviceFind: string = 'on("BLEDeviceFind"): void';
  @State isBLEDeviceFindClick: boolean = false;


  // input ble scan parameters:
  /*ScanFilter*/
  @State cbxBleScanFilter: boolean = false;
  @State txtScanFilter_deviceId: string = "88:36:CF:09:C1:90"; 

  @State txtScanFilter_name: string = "dudu-tiger"; 

  @State txtScanFilter_serviceUuid: string = "0000180A-0000-1000-8000-00805f9b34fb"; 
  @State curScanFilters: Array<bluetooth.ScanFilter> = [];

  /*ScanOptions*/
  @State  cbxBleScanOptions: boolean = false;
  @State txtScanOptions_interval: string = "0"; 

  @State txtScanOptions_dutyMode: number = 0;//bluetooth.ScanDuty.SCAN_MODE_LOW_POWER; 
  @State rd3ScanOptions_dutyModeChecked:boolean = false;
  @State rd2ScanOptions_dutyModeChecked:boolean = false;
  @State rd1ScanOptions_dutyModeChecked:boolean = true;

  @State txtScanOptions_matchMode: number = 1;
  @State rd1ScanOptions_matchModeChecked:boolean = true;
  @State rd2ScanOptions_matchModeChecked:boolean = false;

  @State curScanOptions: bluetooth.ScanOptions = {
    interval: 0,
    dutyMode: 0,
    matchMode: 1
  }


  //@StorageLink('gattServerHandle') gattServerHandle: bluetooth.GattServer = null;
  //@StorageLink('gattClientHandle') gattClientHandle: bluetooth.GattClientDevice = null;
  @StorageLink('bleAvailableDevices') availBleDeviceIds: string[] = [];
  @StorageLink('OnBLEDeviceFind') On_off_BLEDeviceFind: boolean = false;
  private gattServerInstance = null;
  private gattClientDeviceInstance = null;

  build() {
    Row() {

      Column() {
        HeadComponent({ headName: 'BLE Test', isActive: true });
        Stack().height("0.5vp").backgroundColor("#000000");
        Row() {
          Text("peripheral deviceId:").fontSize("15vp")
          TextInput({ text: this.peripheralDeviceId, placeholder: "input peripheral deviceId." })
            .fontSize("15vp")
            .onChange((strInput) => {
              this.peripheralDeviceId = strInput;
            })
        }
        .margin({ top: "0vp", left: "20vp", right: "20vp" })
        .alignItems(VerticalAlign.Center)

        Stack().height("0.25vp").backgroundColor("#000000");

        Text("Input bleScan parameters:")
          .margin({ top: "1vp" })
          .padding("5vp")
          .fontSize("15vp")
          .fontColor("#4444444")
          .height("30vp");
        Stack().height("0.25vp").backgroundColor("#000000");

        Column() {
          Column() {
            Row() {
              Checkbox({ name: 'bleScanFilter', group: 'checkboxGroup99' }) //
                .select(this.cbxBleScanFilter)
                .selectedColor(0x39a2db)
                .onChange((value) => {
                  if (value) {
                    this.cbxBleScanFilter = true;
                  }
                  else {
                    this.cbxBleScanFilter = false;
                  }
                })
                .width("15vp")
                .margin({ left: "1vp" })
              Text("BLE ScanFilter:")
                .margin({ top: "5vp", left: "5vp" })
                .padding("5vp")
                .fontSize("15vp")
                .fontColor("#4444444")
              ;
            }
            .height(40)
            .width('100%')
            .padding(5);
            Column() {
              Row() {
                Row(){
                  Text("deviceId").fontSize("10vp").width("10%");
                  TextInput({ text: this.txtScanFilter_deviceId, placeholder: "input deviceId" })
                    .fontSize("10vp")
                    .onChange((strInput) => {
                      this.txtScanFilter_deviceId = strInput;
                    })
                    .margin({ top: "0vp", right: "30vp" })
                    .width("25%");
                }
                .margin({ top: "0vp", left: "5vp", right: "20vp" })
                .justifyContent(FlexAlign.Start)
                .alignItems(VerticalAlign.Center)

                Row(){
                  Text("name").fontSize("10vp").width("10%");
                  TextInput({ text: this.txtScanFilter_name, placeholder: "input name" })
                    .fontSize("10vp")
                    .onChange((strInput) => {
                      this.txtScanFilter_name = strInput;
                    })
                    .margin({ top: "1vp", right: "30vp" })
                }
                .margin({ top: "0vp", left: "5vp", right: "20vp" })
                .justifyContent(FlexAlign.Start)
                .alignItems(VerticalAlign.Center)
              }
              .margin({ top: "0vp", left: "5vp", right: "20vp" })
              .justifyContent(FlexAlign.Start)
              .alignItems(VerticalAlign.Top)

              Row() {
                Text("serviceUuid").fontSize("10vp").width("10%");
                TextInput({ text: this.txtScanFilter_serviceUuid, placeholder: "input serviceUuid" })
                  .fontSize("10vp")
                  .onChange((strInput) => {
                    this.txtScanFilter_serviceUuid = strInput;
                  })
                  .margin({ top: "1vp", right: "40vp" })
              }
              .margin({ top: "0vp", left: "5vp", right: "40vp" })
            }
            .height(100)
            .width('100%')
            .padding(5)
            .backgroundColor(0xAFEEEE);
          }

          Column() {
            Row() {
              Checkbox({ name: 'bleScanOptions', group: 'checkboxGroup95' })
                .select(this.cbxBleScanOptions)
                .selectedColor(0x39a2db)
                .onChange((value) => {
                  if (value) {
                    this.cbxBleScanOptions = true;
                  } else {
                    this.cbxBleScanOptions = false;
                  }
                })
                .width("15vp")
                .margin({ left: "1vp" })
              Text("BLE Scan Options:")
                .margin({ top: "5vp", left: "5vp" })
                .padding("5vp")
                .fontSize("15vp")
                .fontColor("#4444444")
            }
            .height(40)
            .width('100%')
            .padding(5);
            Row() {
              Row() {
                Text("interval").fontSize("10vp").width("40vp")
                TextInput({ text: this.txtScanOptions_interval, placeholder: "input interval" })
                  .fontSize("10vp")
                  .onChange((strInput) => {
                    this.txtScanOptions_interval = strInput;
                  }).width("60vp")
              }
              .margin({ top: "10vp", left: "5vp", right: "20vp" })
              .alignItems(VerticalAlign.Center)

              Row() {
                Text("dutyMode").fontSize("10vp").width("60vp")
                Row(){
                  Column() {
                    Flex({ direction: FlexDirection.Column,alignItems: ItemAlign.Start }) {
                      Row() {
                        Radio({ value: 'Rd1', group: 'radioGroup' }).checked(this.rd1ScanOptions_dutyModeChecked)
                          .height(20)
                          .width(20)
                          .onChange((value: boolean) => {
                            if(value){
                              this.txtScanOptions_dutyMode = 0;//bluetooth.ScanDuty.SCAN_MODE_LOW_POWER;
                              this.rd1ScanOptions_dutyModeChecked =true;
                              this.rd2ScanOptions_dutyModeChecked =false;
                              this.rd3ScanOptions_dutyModeChecked =false;
                            }
                          })
                        Text('LOW_POWER').fontSize("10vp")
                      }
                      Row() {
                        Radio({ value: 'Rd2', group: 'radioGroup' }).checked(this.rd2ScanOptions_dutyModeChecked)
                          .height(20)
                          .width(20)
                          .onChange((value: boolean) => {
                            if(value) {
                              this.txtScanOptions_dutyMode = 1;//bluetooth.ScanDuty.SCAN_MODE_BALANCED;
                              this.rd1ScanOptions_dutyModeChecked = false;
                              this.rd2ScanOptions_dutyModeChecked = true;
                              this.rd3ScanOptions_dutyModeChecked = false;
                            }

                          })
                        Text('BALANCED').fontSize("10vp")
                      }
                      Row() {
                        Radio({ value: 'Rd3', group: 'radioGroup' }).checked(this.rd3ScanOptions_dutyModeChecked)
                          .height(20)
                          .width(20)
                          .onChange((value: boolean) => {
                            if(value)
                            {
                              this.txtScanOptions_dutyMode = 2; //bluetooth.ScanDuty.SCAN_MODE_LOW_LATENCY;
                              this.rd1ScanOptions_dutyModeChecked = false;
                              this.rd2ScanOptions_dutyModeChecked = false;
                              this.rd3ScanOptions_dutyModeChecked = true;
                            }

                          })
                        Text('LOW_LATENCY').fontSize("10vp")
                      }
                    }
//                    Select([
//                      { value: 'LOW_POWER' },
//                      { value: 'BALANCED' },
//                      { value: 'LOW_LATENCY' }])
//                      .selected(this.selectScanOptions_dutyModeIndex)
//                      .font({ size: 8, weight: 100, family: 'serif', style: FontStyle.Normal })
//                      .selectedOptionFont({ size: 10, weight: 100, family: 'serif', style: FontStyle.Normal })
//                      .optionFont({ size: 8, weight: 100, family: 'serif', style: FontStyle.Normal })
//                      .onSelect((index: number) => {
//                        if (index === 0) {
//                          this.txtScanOptions_dutyMode = bluetooth.ScanDuty.SCAN_MODE_LOW_POWER
//                        }
//                        else if (index === 1) {
//                          this.txtScanOptions_dutyMode = bluetooth.ScanDuty.SCAN_MODE_BALANCED
//                        }
//                        else {
//                          this.txtScanOptions_dutyMode = bluetooth.ScanDuty.SCAN_MODE_LOW_LATENCY
//                        }
//                        this.selectScanOptions_dutyModeIndex = index
//                      })
                  }
                }

              }
              .margin({ top: "10vp", left: "1vp", right: "1vp" })
              .alignItems(VerticalAlign.Center)

              Row() {
                Text("matchMode").fontSize("10vp").width("60vp")
                Column() {
                  Flex({ direction: FlexDirection.Column,alignItems: ItemAlign.Start }) {
                    Row() {
                      Radio({ value: 'Rd1', group: 'radioGroupMatchMode' }).checked(this.rd1ScanOptions_matchModeChecked)
                        .height(20)
                        .width(20)
                        .onChange((value: boolean) => {
                          if(value){
                            this.txtScanOptions_matchMode = 1;//bluetooth.MatchMode.MATCH_MODE_AGGRESSIVE;
                            this.rd1ScanOptions_matchModeChecked =true;
                            this.rd2ScanOptions_matchModeChecked =false;
                          }
                        })
                      Text('MATCH_MODE_AGGRESSIVE').fontSize("10vp")
                    }
                    Row() {
                      Radio({ value: 'Rd2', group: 'radioGroupMatchMode' }).checked(this.rd2ScanOptions_matchModeChecked)
                        .height(20)
                        .width(20)
                        .onChange((value: boolean) => {
                          if(value) {
                            this.txtScanOptions_matchMode = 2;//bluetooth.MatchMode.MATCH_MODE_STICKY;
                            this.rd1ScanOptions_matchModeChecked = false;
                            this.rd2ScanOptions_matchModeChecked = true;
                          }

                        })
                      Text('MATCH_MODE_STICKY').fontSize("10vp")
                    }
                  }
//                                    Select([
//                                      { value: 'MATCH_MODE_AGGRESSIVE', icon: '' },
//                                      { value: 'MATCH_MODE_STICKY', icon: '' }])
//                                      .selected(this.selectScanOptions_matchModeIndex)
//                                      .font({ size: 8, weight: 100, family: 'serif', style: FontStyle.Normal })
//                                      .selectedOptionFont({ size: 10, weight: 100, family: 'serif', style: FontStyle.Normal })
//                                      .optionFont({ size: 8, weight: 100, family: 'serif', style: FontStyle.Normal })
//                                      .onSelect((index: number) => {
//                                        if (index === 0) {
//                                          this.txtScanOptions_matchMode = bluetooth.MatchMode.MATCH_MODE_AGGRESSIVE;
//                                        }
//                                        else {
//                                          this.txtScanOptions_matchMode = bluetooth.MatchMode.MATCH_MODE_STICKY;
//                                        }
//                                        this.selectScanOptions_matchModeIndex = index;
//                                      })
//                                      .width("80vp").height("40")
                }
              }
              .margin({ top: "10vp", left: "5vp", right: "20vp" })
              .alignItems(VerticalAlign.Center)
            }
            .height(80)
            .width('100%')
            .padding(5)
            .backgroundColor(0xAFEEEE);
          }.height("100vp")
        }

        Stack().height("0.25vp").backgroundColor("#000000");

        Column() {
          Text(this.message)
            .textAlign(TextAlign.Start)
            .backgroundColor($r("sys.color.ohos_id_color_foreground_contrary"))
            .margin({ top: "20vp" })
            .padding("5vp")
            .fontSize("15vp")
            .fontColor("#4444444")

          List({ space: 1 }) {
            ListItem() {
              TitleComponent({
                title: "createGattServer()",
                bgColor: this.currentClick === 0 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
              });
            }
            .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
            .onClick(() => {
              this.currentClick = 0;

              let btState = bluetooth.getState();
              if (btState == bluetooth.BluetoothState.STATE_ON) {
                this.message = "createGattServer starts";
                if (!this.gattServerInstance) {
                  this.gattServerInstance = BluetoothModel.createGattServer();
                  this.message = "createGattServer: succeeds.";
                }
                else {
                  this.message = "GattServer has existed!";
                }
              }
              else if (btState == bluetooth.BluetoothState.STATE_OFF) {
                this.message = "BT is not enabled!";
              }
              else {
                this.message = "switching,please operate it later!";
              }
            })

            ListItem() {
              TitleComponent({
                title: "createGattClientDevice(deviceId: string)",
                bgColor: this.currentClick === 1 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
              });
            }
            .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
            .onClick(() => {
              this.currentClick = 1;

              let btState = bluetooth.getState();
              if (btState == bluetooth.BluetoothState.STATE_ON) {

                if (!this.gattClientDeviceInstance) {
                  this.message = "createGattClientDevice starts";
                  this.gattClientDeviceInstance = BluetoothModel.createGattClient(this.peripheralDeviceId);
                  this.message = "createGattClientDevice:succeeds.";
                }
                else {
                  this.message = "createGattClientDevice has existed!";
                }
              }
              else if (btState == bluetooth.BluetoothState.STATE_OFF) {
                this.message = "BT is not enabled!";
              }
              else {
                this.message = "switching,please operate it later!";
              }
            })

            ListItem() {
              TitleComponent({
                title: "getConnectedBLEDevices()",
                bgColor: this.currentClick === 2 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
              });
            }
            .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
            .onClick(() => {
              this.currentClick = 2;
              let connectedBleDevices = BluetoothModel.getConnectedBLEDevices();
              this.message = "getConnectedBLEDevices£º" + JSON.stringify(connectedBleDevices);

              console.log(JSON.stringify(connectedBleDevices));
            })


            ListItem() {
              TitleComponent({
                title: "startBLEScan(...)",
                bgColor: this.currentClick === 3 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
              });
            }
            .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
            .onClick(() => {
              this.currentClick = 3;
              let btState = bluetooth.getState();
              if (btState == bluetooth.BluetoothState.STATE_OFF) {
                this.message = "BT is not enabled!";
              }

              // build input parameters:

              this.buildBleScanFilters();
              this.availBleDeviceIds = [];
              AppStorage.SetOrCreate('bluetoothAvailableDevices', this.availBleDeviceIds);
              if (!this.cbxBleScanOptions) {
                if (this.cbxBleScanFilter && this.curScanFilters.length > 0) {
                  BluetoothModel.startBLEScan(this.curScanFilters);
                  this.message = "startBLEScan Filters:"+JSON.stringify(this.curScanFilters);
                }
                else
                {
                  BluetoothModel.startBLEScan(null);
                  this.message = "startBLEScan(null)";
                }
              }
              else
              {
                this.buildBleScanOption();
                this.message = "startBLEScan Filters:"+JSON.stringify(this.curScanFilters) + "\n Options:"+JSON.stringify(this.curScanOptions);
                BluetoothModel.startBLEScan(this.curScanFilters, this.curScanOptions);

              }
              this.message = "startBLEScan succeeds."
            })


            ListItem() {
              TitleComponent({
                title: "stopBLEScan(): void",
                bgColor: this.currentClick === 4 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
              });
            }
            .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
            .onClick(() => {
              this.currentClick = 4;
              BluetoothModel.stopBLEScan();
              this.message = "stopBLEScan OK!";
            })

            ListItem() {
              TitleComponent({
                title: this.btOnBLEDeviceFind,
                bgColor: this.currentClick === 5 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
              });
            }
            .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
            .onClick(() => {
              if (this.isBLEDeviceFindClick) {
                BluetoothModel.unsubscribeBLEDeviceFind();
                this.message = 'BLEDeviceFind is off(turn it on by re-clicking)';
                this.btOnBLEDeviceFind = 'on:BLEDeviceFind'
                AppStorage.SetOrCreate('on_BLEDeviceFind', this.btOnBLEDeviceFind);
                AppStorage.SetOrCreate('OnBLEDeviceFind', false);
                this.isBLEDeviceFindClick = false;
                return;
              }
              this.isBLEDeviceFindClick = true;
              this.currentClick = 5;
              this.btOnBLEDeviceFind = 'off:BLEDeviceFind'
              this.subscribeBLEDeviceFind();
              AppStorage.SetOrCreate('on_BLEDeviceFind', this.btOnBLEDeviceFind);
              AppStorage.SetOrCreate('OnBLEDeviceFind', true);
              this.message = 'BLEDeviceFind is on(turn it off by re-clicking)';
            })

          }.margin(20);
          Stack().height("0.5vp").backgroundColor("#000000");

          if (this.isBLEDeviceFindClick) {
            if (this.availBleDeviceIds && this.availBleDeviceIds.length >= 1) {
              Scroll() {
                List() {
                  ForEach(this.availBleDeviceIds, (item: string) => {
                    ListItem() {
                      Row() {
                        EntryComponent({
                          settingIcon: "/res/image/ic_bluetooth_device.svg",
                          settingTitle: item,
                          settingSummary: '',
                          settingValue: '',
                          settingArrow: '',
                          settingArrowStyle: '',
                          settingUri: '',
                          image_wh: $r('app.float.wh_value_24'),
                          height: $r('app.float.wh_value_56'),
                          fontSize: ($r('app.float.font_16')),
                        });
                      }
                      .width(ConfigData.WH_100_100)
                      .borderRadius($r("app.float.radius_24"))
                      .padding($r('app.float.distance_4'))
                      .backgroundColor($r("app.color.white_bg_color"))
                      .onClick(() => {
                        this.peripheralDeviceId = item;
                      })
                    }
                  }, item => item);
                }
                .backgroundColor($r("app.color.white_bg_color"))
                .borderRadius($r("app.float.radius_24"))
                .divider({
                  strokeWidth: 1,
                  color: $r('app.color.color_E3E3E3_grey'),
                  startMargin: $r('app.float.wh_value_52'),
                  endMargin: $r('app.float.wh_value_12')
                })
                // .visibility();
              }
            }
            else {
              Row() {
                // Scanning...
                Text($r('app.string.scanning'))
                  .fontSize($r('app.float.font_20'))
                  .textCase(TextCase.UpperCase);
              }
            }
          }
        }
        .visibility(Visibility.Visible);
      }

    }
    .backgroundColor($r("sys.color.ohos_id_color_sub_background"))
    .height('100%')
    .alignItems(VerticalAlign.Top)
  }

  /**
* subscribeBLEDeviceFind
*/
  private subscribeBLEDeviceFind() {
    BluetoothModel.subscribeBLEDeviceFind((deviceItems: Array<bluetooth.ScanResult>) => {
      LogUtil.log("BLE Test:" + 'available ble devices changed.');
      deviceItems?.forEach(deviceItem => {
        let device = this.availBleDeviceIds.find((availableBleDevice) => {
          return availableBleDevice === deviceItem.deviceId
        })
        if (!device) {
          let newDevice = deviceItem.deviceId;
          LogUtil.log("BLE Test:" + 'available bluetooth new device : ' + newDevice);
          this.availBleDeviceIds.push(newDevice);
          LogUtil.log("BLE Test:" + 'available bluetooth new device pushed. availableDevices length = ' + this.availBleDeviceIds.length);
        }
      })
      AppStorage.SetOrCreate('bluetoothAvailableDevices', this.availBleDeviceIds);
    });
  }

  private buildBleScanFilters() {
    this.curScanFilters = [];
    if (this.cbxBleScanFilter) {
      let bleScanFilter = {}

      if (this.txtScanFilter_deviceId.length > 0 && this.txtScanFilter_name.length > 0 && this.txtScanFilter_serviceUuid.length > 0) {
        bleScanFilter = {
          deviceId: this.txtScanFilter_deviceId,
          name: this.txtScanFilter_name,
          serviceUuid: this.txtScanFilter_serviceUuid
        }
        this.curScanFilters.push(bleScanFilter);
      }
      if (this.txtScanFilter_deviceId === '' && this.txtScanFilter_name === '' && this.txtScanFilter_serviceUuid.length > 0) {
        bleScanFilter = {
          serviceUuid: this.txtScanFilter_serviceUuid
        }
        this.curScanFilters.push(bleScanFilter);
      }

      if (this.txtScanFilter_deviceId === '' && this.txtScanFilter_name.length > 0 && this.txtScanFilter_serviceUuid === '') {
        bleScanFilter = {
          name: this.txtScanFilter_name
        }
        this.curScanFilters.push(bleScanFilter);
      }
      if (this.txtScanFilter_deviceId.length > 0 && this.txtScanFilter_name === '' && this.txtScanFilter_serviceUuid === '') {
        bleScanFilter = {
          deviceId: this.txtScanFilter_deviceId
        }
        this.curScanFilters.push(bleScanFilter);
      }
      if (this.txtScanFilter_deviceId === '' && this.txtScanFilter_name.length > 0 && this.txtScanFilter_serviceUuid.length > 0) {
        bleScanFilter = {
          name: this.txtScanFilter_name,
          serviceUuid: this.txtScanFilter_serviceUuid
        }
        this.curScanFilters.push(bleScanFilter);
      }
      if (this.txtScanFilter_deviceId.length > 0 && this.txtScanFilter_name === '' && this.txtScanFilter_serviceUuid.length > 0) {
        bleScanFilter = {
          deviceId: this.txtScanFilter_deviceId,
          serviceUuid: this.txtScanFilter_serviceUuid
        }
        this.curScanFilters.push(bleScanFilter);
      }
      if (this.txtScanFilter_deviceId.length > 0 && this.txtScanFilter_name.length > 0 && this.txtScanFilter_serviceUuid === '') {
        bleScanFilter = {
          deviceId: this.txtScanFilter_deviceId,
          name: this.txtScanFilter_name
        }
        this.curScanFilters.push(bleScanFilter);
      }
    }
  }

  private buildBleScanOption() {
    if (this.cbxBleScanOptions) {
      this.curScanOptions = {
        interval:  parseInt(this.txtScanOptions_interval),
        dutyMode:  this.txtScanOptions_dutyMode,
        matchMode: this.txtScanOptions_matchMode}
    }
  }
}