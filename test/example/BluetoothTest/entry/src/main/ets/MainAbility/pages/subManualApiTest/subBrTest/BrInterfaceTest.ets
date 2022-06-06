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
import { TitleComponent } from '../../../../Component/titleComponent';
import HeadComponent from '../../../../component/headcomponent';
import BluetoothModel from '../../../model/BluetoothModel';

@Entry
@Component
struct Index {
  @State message: string = "";
  @State isConnection: boolean = false;
  private stateBT: string = "";
  @State deviceId: string = "F0:5C:D5:8B:A5:8A";

  build() {
    Row() {
      Column() {
        HeadComponent({headName: $r('app.string.br_test_title'), isActive: true});
        Stack().height("0.5vp").backgroundColor("#000000");
        Column() {
          Text("请点击需要测试的方法")
            .margin({top: "20vp"})
            .padding("5vp")
            .fontSize("15vp")
            .fontColor("#4444444");
          List() {
            ListItem() {
              TitleComponent({ title: "getState" });
            }.padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
            .onClick((event) => {
              let ret = BluetoothModel.getState();
              switch (ret) {
                case 0:
                  this.stateBT = 'STATE_OFF';
                  break;
                case 1:
                  this.stateBT = 'STATE_TURNING_ON';
                  break;
                case 2:
                  this.stateBT = 'STATE_ON';
                  break;
                case 3:
                  this.stateBT = 'STATE_TURNING_OFF';
                  break;
                case 4:
                  this.stateBT = 'STATE_BLE_TURNING_ON';
                  break;
                case 5:
                  this.stateBT = 'STATE_BLE_ON';
                  break;
                case 6:
                  this.stateBT = 'STATE_BLE_TURNING_OFF';
                  break;
                default:
                  this.stateBT = '';
                  break;
              }
              this.message = "当前蓝牙的状态是：" + this.stateBT;
            })
            ListItem() {
              TitleComponent({ title: "getLocalName" });
            }.padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
            .onClick((event) => {
              let ret = BluetoothModel.getLocalName();
              this.message = "本设备名称是：" + ret;
            })
            ListItem() {
              TitleComponent({ title: "getBluetoothScanMode" });
            }.padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
            .onClick((event) => {
              let ret = BluetoothModel.getBluetoothScanMode();
              this.message = "蓝牙当前的扫描模式是: " + ret;
            })
          }.margin(20);
          Text(this.message)
            .textAlign(TextAlign.Start)
            .backgroundColor($r("sys.color.ohos_id_color_foreground_contrary"))
        }
        .visibility(Visibility.Visible);
      }
      .width('100%')
    }
    .backgroundColor($r("sys.color.ohos_id_color_sub_background"))
    .height('100%')
    .alignItems(VerticalAlign.Top)
  }
}