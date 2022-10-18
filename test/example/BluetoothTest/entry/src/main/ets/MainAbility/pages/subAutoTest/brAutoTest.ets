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

import ConfigData from '../../../Utils/ConfigData';
import router from '@system.router';
import { TestData } from '../../model/testData'
import { PageTitle } from '../../../Component/pageTitle';
import { TestImageDisplay } from '../../../Component/testImageDisplay';
import {initBrAutoData} from '../../model/autoTestDataModels'
import { AutoContentTable } from '../../../Component/autoContentTable'
import BluetoothModel from '../../model/bluetoothModel'
import bluetooth from '@ohos.bluetooth';
import bt from '@ohos.bluetooth';
import * as BRInterface from '../../model/brInterface'
import prompt from '@system.prompt';
import emitter from '@ohos.events.emitter'
/**
 *  BrAutoTest Page Of Bluetooth test
 */
@Entry
@Component
struct BrAutoTest {
  @State message: string = 'BrAutoTest';
  @State textMessage: string = 'null';
  private testItem: TestData = router.getParams().testId;
  @StorageLink ("autoTestMessage") autoTestMessage:string ="";

  @State changIndex:number = -1;
  @State testMessage:string="";
  @State testState:boolean = false;

  build() {
    Column() {
      Stack({ alignContent: Alignment.TopStart }) {
        TestImageDisplay({ testItem: this.testItem })
        PageTitle({ testItem: this.testItem })
      }
      Stack().height("1vp").backgroundColor("#000000");
      Column() {
        Scroll() {
          Column() {
            Text("API验证:")
              .fontSize("17vp")
              .margin({ top: "10vp", bottom:"10vp",left: "10vp", right: "280vp" })
              .textAlign(TextAlign.Start)
            AutoContentTable({ testItem: this.testItem, autoItems: initBrAutoData(),changeIndex:this.changIndex,testMessage:this.testMessage,testState:this.testState})
            //              BrTestAuto({ testItems: this.testItems.filter(item => (item.AutoCategory === AutoCategory.BrAuto)) })
            //            List(){
            //              ListItem(){
            //                Row(){
            //                  Row() {
            //                    Image($r('app.media.bluetooth'))
            //                      .objectFit(ImageFit.Contain)
            //                      .autoResize(false)
            //                      .height(23)
            //                      .width(23)
            //                  }
            //                  .width("10%")
            //                  .padding({left: 10})
            //                  Row() {
            //                    Text("enableBluetooth")
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp", left: "10vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("70%")
            //                  Row() {
            //                    Text(this.textMessage)
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("20%")
            //                }
            //              }
            //              ListItem(){
            //                Row(){
            //                  Row() {
            //                    Image($r('app.media.bluetooth'))
            //                      .objectFit(ImageFit.Contain)
            //                      .autoResize(false)
            //                      .height(23)
            //                      .width(23)
            //                  }
            //                  .width("10%")
            //                  .padding({left: 10})
            //                  Row() {
            //                    Text("disableBluetooth")
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp", left: "10vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("70%")
            //                  Row() {
            //                    Text(this.textMessage)
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("20%")
            //                }
            //              }
            //              ListItem(){
            //                Row(){
            //                  Row() {
            //                    Image($r('app.media.bluetooth'))
            //                      .objectFit(ImageFit.Contain)
            //                      .autoResize(false)
            //                      .height(23)
            //                      .width(23)
            //                  }
            //                  .width("10%")
            //                  .padding({left: 10})
            //                  Row() {
            //                    Text("getState")
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp", left: "10vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("70%")
            //                  Row() {
            //                    Text(this.textMessage)
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("20%")
            //                }
            //              }
            //              ListItem(){
            //                Row(){
            //                    Row() {
            //                      Image($r('app.media.bluetooth'))
            //                        .objectFit(ImageFit.Contain)
            //                        .autoResize(false)
            //                        .height(23)
            //                        .width(23)
            //                    }
            //                    .width("10%")
            //                    .padding({left: 10})
            //                  Row() {
            //                    Text("getBtConnectionState")
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp", left: "10vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("70%")
            //                  Row() {
            //                    Text(this.textMessage)
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("20%")
            //                }
            //              }
            //              ListItem(){
            //                Row(){
            //                  Row() {
            //                    Image($r('app.media.bluetooth'))
            //                      .objectFit(ImageFit.Contain)
            //                      .autoResize(false)
            //                      .height(23)
            //                      .width(23)
            //                  }
            //                  .width("10%")
            //                  .padding({left: 10})
            //                  Row() {
            //                    Text("setLocalName")
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp", left: "10vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("70%")
            //                  Row() {
            //                    Text(this.textMessage)
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("20%")
            //                }
            //              }
            //              ListItem(){
            //                Row(){
            //                  Row() {
            //                    Image($r('app.media.bluetooth'))
            //                      .objectFit(ImageFit.Contain)
            //                      .autoResize(false)
            //                      .height(23)
            //                      .width(23)
            //                  }
            //                  .width("10%")
            //                  .padding({left: 10})
            //                  Row() {
            //                    Text("getLocalName")
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp", left: "10vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("70%")
            //                  Row() {
            //                    Text(this.textMessage)
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("20%")
            //                }
            //              }
            //              ListItem(){
            //                Row(){
            //                  Row() {
            //                    Image($r('app.media.bluetooth'))
            //                      .objectFit(ImageFit.Contain)
            //                      .autoResize(false)
            //                      .height(23)
            //                      .width(23)
            //                  }
            //                  .width("10%")
            //                  .padding({left: 10})
            //                  Row() {
            //                    Text("setBluetoothScanMode")
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp", left: "10vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("70%")
            //                  Row() {
            //                    Text(this.textMessage)
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("20%")
            //                }
            //              }
            //              ListItem(){
            //                Row(){
            //                  Row() {
            //                    Image($r('app.media.bluetooth'))
            //                      .objectFit(ImageFit.Contain)
            //                      .autoResize(false)
            //                      .height(23)
            //                      .width(23)
            //                  }
            //                  .width("10%")
            //                  .padding({left: 10})
            //                  Row() {
            //                    Text("getBluetoothScanMode")
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp", left: "10vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("70%")
            //                  Row() {
            //                    Text(this.textMessage)
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("20%")
            //                }
            //              }
            //              ListItem(){
            //                Row(){
            //                  Row() {
            //                    Image($r('app.media.bluetooth'))
            //                      .objectFit(ImageFit.Contain)
            //                      .autoResize(false)
            //                      .height(23)
            //                      .width(23)
            //                  }
            //                  .width("10%")
            //                  .padding({left: 10})
            //                  Row() {
            //                    Text("startBluetoothDiscovery")
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp", left: "10vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("70%")
            //                  Row() {
            //                    Text(this.textMessage)
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("20%")
            //                }
            //              }
            //              ListItem(){
            //                Row(){
            //                  Row() {
            //                    Image($r('app.media.bluetooth'))
            //                      .objectFit(ImageFit.Contain)
            //                      .autoResize(false)
            //                      .height(23)
            //                      .width(23)
            //                  }
            //                  .width("10%")
            //                  .padding({left: 10})
            //                  Row() {
            //                    Text("stopBluetoothDiscovery")
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp", left: "10vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("70%")
            //                  Row() {
            //                    Text(this.textMessage)
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("20%")
            //                }
            //              }
            //              ListItem(){
            //                Row(){
            //                  Row() {
            //                    Image($r('app.media.bluetooth'))
            //                      .objectFit(ImageFit.Contain)
            //                      .autoResize(false)
            //                      .height(23)
            //                      .width(23)
            //                  }
            //                  .width("10%")
            //                  .padding({left: 10})
            //                  Row() {
            //                    Text("pairDevice")
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp", left: "10vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("70%")
            //                  Row() {
            //                    Text(this.textMessage)
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("20%")
            //                }
            //              }
            //              ListItem(){
            //                Row(){
            //                  Row() {
            //                    Image($r('app.media.bluetooth'))
            //                      .objectFit(ImageFit.Contain)
            //                      .autoResize(false)
            //                      .height(23)
            //                      .width(23)
            //                  }
            //                  .width("10%")
            //                  .padding({left: 10})
            //                  Row() {
            //                    Text("unpairDevice")
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp", left: "10vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("70%")
            //                  Row() {
            //                    Text(this.textMessage)
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("20%")
            //                }
            //              }
            //              ListItem(){
            //                Row(){
            //                  Row() {
            //                    Image($r('app.media.bluetooth'))
            //                      .objectFit(ImageFit.Contain)
            //                      .autoResize(false)
            //                      .height(23)
            //                      .width(23)
            //                  }
            //                  .width("10%")
            //                  .padding({left: 10})
            //                  Row() {
            //                    Text("getDeviceName")
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp", left: "10vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("70%")
            //                  Row() {
            //                    Text(this.textMessage)
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("20%")
            //                }
            //              }
            //              ListItem(){
            //                Row(){
            //                  Row() {
            //                    Image($r('app.media.bluetooth'))
            //                      .objectFit(ImageFit.Contain)
            //                      .autoResize(false)
            //                      .height(23)
            //                      .width(23)
            //                  }
            //                  .width("10%")
            //                  .padding({left: 10})
            //                  Row() {
            //                    Text("getDeviceType")
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp", left: "10vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("70%")
            //                  Row() {
            //                    Text(this.textMessage)
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("20%")
            //                }
            //              }
            //              ListItem(){
            //                Row(){
            //                  Row() {
            //                    Image($r('app.media.bluetooth'))
            //                      .objectFit(ImageFit.Contain)
            //                      .autoResize(false)
            //                      .height(23)
            //                      .width(23)
            //                  }
            //                  .width("10%")
            //                  .padding({left: 10})
            //                  Row() {
            //                    Text("getPairedDeviceIds")
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp", left: "10vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("70%")
            //                  Row() {
            //                    Text(this.textMessage)
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("20%")
            //                }
            //              }
            //              ListItem(){
            //                Row(){
            //                  Row() {
            //                    Image($r('app.media.bluetooth'))
            //                      .objectFit(ImageFit.Contain)
            //                      .autoResize(false)
            //                      .height(23)
            //                      .width(23)
            //                  }
            //                  .width("10%")
            //                  .padding({left: 10})
            //                  Row() {
            //                    Text("getProfileConnState")
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp", left: "10vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("70%")
            //                  Row() {
            //                    Text(this.textMessage)
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("20%")
            //                }
            //              }
            //              ListItem(){
            //                Row(){
            //                  Row() {
            //                    Image($r('app.media.bluetooth'))
            //                      .objectFit(ImageFit.Contain)
            //                      .autoResize(false)
            //                      .height(23)
            //                      .width(23)
            //                  }
            //                  .width("10%")
            //                  .padding({left: 10})
            //                  Row() {
            //                    Text("getDeviceState")
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp", left: "10vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("70%")
            //                  Row() {
            //                    Text(this.textMessage)
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("20%")
            //                }
            //              }
            //              ListItem(){
            //                Row(){
            //                  Row() {
            //                    Image($r('app.media.bluetooth'))
            //                      .objectFit(ImageFit.Contain)
            //                      .autoResize(false)
            //                      .height(23)
            //                      .width(23)
            //                  }
            //                  .width("10%")
            //                  .padding({left: 10})
            //                  Row() {
            //                    Text("on.stateChange")
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp", left: "10vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("70%")
            //                  Row() {
            //                    Text(this.textMessage)
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("20%")
            //                }
            //              }
            //              ListItem(){
            //                Row(){
            //                  Row() {
            //                    Image($r('app.media.bluetooth'))
            //                      .objectFit(ImageFit.Contain)
            //                      .autoResize(false)
            //                      .height(23)
            //                      .width(23)
            //                  }
            //                  .width("10%")
            //                  .padding({left: 10})
            //                  Row() {
            //                    Text("on.pinRequired")
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp", left: "10vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("70%")
            //                  Row() {
            //                    Text(this.textMessage)
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("20%")
            //                }
            //              }
            //              ListItem(){
            //                Row(){
            //                  Row() {
            //                    Image($r('app.media.bluetooth'))
            //                      .objectFit(ImageFit.Contain)
            //                      .autoResize(false)
            //                      .height(23)
            //                      .width(23)
            //                  }
            //                  .width("10%")
            //                  .padding({left: 10})
            //                  Row() {
            //                    Text("on.bondStateChange")
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp", left: "10vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("70%")
            //                  Row() {
            //                    Text(this.textMessage)
            //                      .fontSize("17vp")
            //                      .margin({ top: "3vp", bottom: "3vp" })
            //                      .textAlign(TextAlign.Start)
            //                  }
            //                  .height(30)
            //                  .width("20%")
            //                }
            //              }
            //            }
            //            .height("80%")
          }
          .width("100%")
          .height(300)
        }
        .scrollBarWidth(10)
        .scrollBar(BarState.Auto)

        Stack().height("1vp").backgroundColor("#000000");

        Scroll() {
          Column() {
            Text("Debug日志:")
              .fontSize("17vp")
              .margin({ top: "8vp", bottom: "8vp", left: "10vp", right: "258vp" })
              .textAlign(TextAlign.Start)
            List(){
              ListItem(){
                Text("log:" + "\n"  + this.autoTestMessage)
                  .fontSize("17vp")
                  .margin({ top: "5vp", left: "30vp", right: "10vp" })
                  .textAlign(TextAlign.Start)
              }
            }
            .height("80%")
          }
          .width("100%")
          .height(260)
        }
        .scrollBarWidth(10)
        .scrollBar(BarState.Auto)

      }
      .height(520)
      .width("100%")
      .backgroundColor($r("sys.color.ohos_id_color_sub_background"))


      Stack().height("1vp").backgroundColor("#000000");
      Column() {
        Button({ type: ButtonType.Normal, stateEffect: true }) {
          Row() {
            Text('自动测试').fontSize("24vp").fontColor(0xffffff).margin({ left: 5, right: 5 })
          }.alignItems(VerticalAlign.Center)
        }
        .borderRadius(8)
        .backgroundColor($r("app.color.blue"))
        .width(180)
        .height(50)
        .margin({ top: "15vp"})
        .onClick((event: ClickEvent) => {
          AlertDialog.show({ message: 'AutoTest is successful' })
          let autoTestMessage =""


          this.changIndex=0
          //强制刷新
          this.changIndex=1
//          this.changIndex=2
//          this.changIndex=3

          let message= BRInterface.testEnableBluetooth()
          autoTestMessage += message + "\n"                  // 开蓝牙
          if(message&&message.length>0){
            this.changIndex=0
            this.testState=true
            this.testMessage = message

          }

         message = BRInterface.testGetState()                     // 设置状态
          autoTestMessage += message + "\n"                  // 开蓝牙
          if(message&&message.length>0){
            this.changIndex=1
            this.testState=true
            this.testMessage = message
          }
          autoTestMessage += BRInterface.testSetLocalName() + "\n"                     //设置本地名称

          autoTestMessage += BRInterface.testGetLocalName() + "\n"                     //获取本地名称

          autoTestMessage += BRInterface.testSetBluetoothScanMode() + "\n"             //设置扫描模式

          autoTestMessage += BRInterface.testGetBluetoothScanMode() + "\n"             //获取扫描模式

          autoTestMessage += BRInterface.testStartBluetoothDiscovery() +"\n"           //开始蓝牙发现

          autoTestMessage += BRInterface.testStopBluetoothDiscovery() +"\n"            //停止蓝牙发现

          autoTestMessage += BRInterface.testPairDevice() +"\n"                        //配对设备

          autoTestMessage += BRInterface.testGetBtConnectionState() +"\n"              //获取当前连接状态

          autoTestMessage += BRInterface.testGetRemoteDeviceName() +"\n"               //获取远程设备名称

          autoTestMessage += BRInterface.testGetRemoteDeviceClass() +"\n"               //获取远程设备类型

          autoTestMessage += BRInterface.testGetPairedDevices() +"\n"               //获取配对设备MAC

          autoTestMessage += BRInterface.testCancelPairedDevice() +"\n"                //取消配对

          autoTestMessage += BRInterface.testDisableBluetooth() +"\n"                  //关蓝牙
          AppStorage.SetOrCreate("autoTestMessage",autoTestMessage)

          let msg = ""
          msg ="测试完成"
          console.log(msg)
          prompt.showToast({message: msg})

        })
      }
      .backgroundColor($r("sys.color.ohos_id_color_sub_background"))
      .width(ConfigData.WH_100_100)
      .height(ConfigData.WH_100_100);
    }
  }
}