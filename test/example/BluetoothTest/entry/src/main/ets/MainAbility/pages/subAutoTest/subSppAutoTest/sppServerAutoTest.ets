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
import {initSppServerAutoData} from '../../../model/autoTestDataModels';
import BluetoothModel from '../../../model/bluetoothModel'
import bluetooth from '@ohos.bluetooth';
import bt from '@ohos.bluetooth';
import * as SPPServerInterface from '../../../model/sppServerInterface'
import * as BRInterface from '../../../model/brInterface'
import prompt from '@system.prompt';
/**
 *  SppServerAutoTest Page Of Bluetooth test
 */
@Entry
@Component
struct SppServerAutoTest {
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
            AutoContentTable({ testItem: this.testItem, autoItems: initSppServerAutoData() })
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
          autoTestMessage += BRInterface.testEnableBluetooth() + "\n"                 // 开蓝牙

          autoTestMessage += BRInterface.testSetBluetoothScanMode() + "\n"             //设置扫描模式

          autoTestMessage += BRInterface.testGetState() + "\n"                         // 设置状态

          autoTestMessage += BRInterface.testGetBluetoothScanMode() + "\n"             //获取扫描模式

          autoTestMessage += BRInterface.testGetBtConnectionState() + "\n"             //获取当前连接状态

          autoTestMessage += SPPServerInterface.testSppServerCreateSafe() + "\n"       //spp服务器创建（安全模式）

          autoTestMessage += SPPServerInterface.testSppServerCreateUnSafe() +"\n"      //spp服务器创建（非安全模式）

          autoTestMessage += BRInterface.testOnPinRequired() +"\n"                     //注册pin码变化

          autoTestMessage += SPPServerInterface.testSppServerWrite() +"\n"             //spp服务器写入

          autoTestMessage += SPPServerInterface.testSppServerAccept() +"\n"            //spp服务器接受

          autoTestMessage += SPPServerInterface.testSppServerReadOn() +"\n"            //spp服务器读取打开

          autoTestMessage += SPPServerInterface.testSppServerReadOff() +"\n"           //spp服务器读取关闭

          autoTestMessage += SPPServerInterface.testSppServerClose() +"\n"             //spp服务器关闭

          autoTestMessage += SPPServerInterface.testSppCloseClientSocket() +"\n"       //spp客户端关闭

          autoTestMessage += BRInterface.testDisableBluetooth() +"\n"                  //关蓝牙

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