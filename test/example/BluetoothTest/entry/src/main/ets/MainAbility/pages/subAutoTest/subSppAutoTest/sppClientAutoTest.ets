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
import ConfigData from '../../../../Utils/ConfigData';
import router from '@system.router';
import { TestData } from '../../../model/testData'
import { PageTitle } from '../../../../Component/pageTitle';
import { TestImageDisplay } from '../../../../Component/testImageDisplay';
import { AutoContentTable } from '../../../../Component/autoContentTable';
import {initSppClientAutoData} from '../../../model/autoTestDataModels';
import BluetoothModel from '../../../model/bluetoothModel'
import bluetooth from '@ohos.bluetooth';
import bt from '@ohos.bluetooth';
import * as BrInterface from '../../../model/brInterface'
import * as SppClientInterface from '../../../model/sppClientInterface'
import prompt from '@system.prompt';
/**
 *  SppClientAutoTest Page Of Bluetooth test
 */
@Entry
@Component
struct SppClientAutoTest {
  @State message: string = 'BrAutoTest';
  @State textMessage: string = 'null';
  private testItem: TestData = router.getParams().testId;
  @StorageLink ("autoTestMessage") autoTestMessage:string =""
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
            AutoContentTable({ testItem: this.testItem, autoItems: initSppClientAutoData() })
          }
          .width("100%")
          .height(260)
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
          let message = ""
          autoTestMessage += BrInterface.testEnableBluetooth() + "\n"                           // 开蓝牙

          autoTestMessage += SppClientInterface.testSetSppBluetoothScanMode() + "\n"            // 设置spp扫描模式

          autoTestMessage += BrInterface.testGetState() + "\n"                                  //设置状态

          autoTestMessage += BrInterface.testGetBluetoothScanMode() + "\n"                      //获取扫描模式

          autoTestMessage += BrInterface.testGetBtConnectionState() + "\n"                      //获取当前连接状态

          autoTestMessage += SppClientInterface.testSppClientConnectSafe() + "\n"               //spp客户端连接（安全模式）

          autoTestMessage += SppClientInterface.testSppClientConnectUnSafe() +"\n"              //spp客户端连接（非安全模式）

          autoTestMessage += BrInterface.testPairDevice() +"\n"                                 //配对设备

          autoTestMessage += SppClientInterface.testSppClientWrite() +"\n"                      //spp客户端写入

          autoTestMessage += SppClientInterface.testSppClientReadOn() +"\n"                     //spp客户端读取打开

          autoTestMessage += SppClientInterface.testSppClientReadOff() +"\n"                    //spp客户端读取关闭

          autoTestMessage += SppClientInterface.testSppClientClose() +"\n"                      //spp客户端关闭

          autoTestMessage += BrInterface.testDisableBluetooth() +"\n"                           //关蓝牙
          message ="测试完成"
          console.log(message)
          AppStorage.SetOrCreate("autoTestMessage",autoTestMessage)
          prompt.showToast({message: message})
          return message
        })
      }
      .backgroundColor($r("sys.color.ohos_id_color_sub_background"))
      .width(ConfigData.WH_100_100)
      .height(ConfigData.WH_100_100);
    }
  }
}