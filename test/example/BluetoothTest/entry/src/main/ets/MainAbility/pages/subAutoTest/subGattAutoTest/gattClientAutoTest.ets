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
import {initGattClientAutoData} from '../../../model/autoTestDataModels';
import BluetoothModel from '../../../model/bluetoothModel'
import bluetooth from '@ohos.bluetooth';
import bt from '@ohos.bluetooth';
import * as GattClientInterface from '../../../model/gattClientInterface'
import prompt from '@system.prompt';
/**
 *  GattClientAutoTest Page Of Bluetooth test
 */
@Entry
@Component
struct GattClientAutoTest {
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
            AutoContentTable({ testItem: this.testItem, autoItems: initGattClientAutoData() })
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
          autoTestMessage += GattClientInterface.testCreateGattClient() + "\n"                 // 创建Gatt客户端

          autoTestMessage += GattClientInterface.testConnect() + "\n"                         // 连接

          autoTestMessage += GattClientInterface.testBLEConnectionStateChange() + "\n"         //BLE连接状态变化

          autoTestMessage += GattClientInterface.testGetServicesCallback() + "\n"              //获取服务callback

          autoTestMessage += GattClientInterface.testGetServicesPromise() + "\n"              //获取服务Promise

          autoTestMessage += GattClientInterface.testReadCharacteristicValueCallback() + "\n"  //读取特征值callback

          autoTestMessage += GattClientInterface.testReadCharacteristicValuePromise() + "\n"  //读取特征值promise

          autoTestMessage += GattClientInterface.testReadDescriptorValueCallback()+ "\n"       //读取描述符值callback

          autoTestMessage += GattClientInterface.testReadDescriptorValuePromise()+ "\n"       //读取描述符值promise

          autoTestMessage += GattClientInterface.testWriteCharacteristicValue() +"\n"           //写入特征值

          autoTestMessage += GattClientInterface.testWriteDescriptorValue() +"\n"               //写入描述符值

          autoTestMessage += GattClientInterface.testBLECharacteristicChange() +"\n"            //BLE特征值变化

          autoTestMessage += GattClientInterface.testSetBLEMtuSize() +"\n"                      //设定BLEMtu尺寸

          autoTestMessage += GattClientInterface.testSetNotifyCharacteristicChanged() +"\n"       //设置通知特性更改

          autoTestMessage += GattClientInterface.testGetDeviceNameCallback() +"\n"                //获取设备名称callback

          autoTestMessage += GattClientInterface.testGetDeviceNamePromise() +"\n"                //获取设备名称promise

          autoTestMessage += GattClientInterface.testGetRssiValueCallback() +"\n"                //获取Rssi值callback

          autoTestMessage += GattClientInterface.testGetRssiValuePromise() +"\n"                //获取Rssi值promise

          autoTestMessage += GattClientInterface.testDisconnect() +"\n"                        //取消连接

          autoTestMessage += GattClientInterface.testGattClientClose() +"\n"                  //关闭客户端

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