/**
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
 * SPP Test Page Of Bluetooth test
 */
import { TitleComponent } from '../../../../Component/titleComponent';
import HeadComponent from '../../../../component/headcomponent';
import bluetooth from '@ohos.bluetooth';
import LogUtil from '../../../../Utils/LogUtil';

let MODEL = "bluetoothModel:"

@Entry
@Component
struct SppClient {
  @State message: string = "";
  @State isConnection: boolean = false;
  private stateBT: string = "";
//  @State deviceId: string = "08:FB:EA:1C:06:D7";
  @State deviceId: string = "08:FB:EA:7A:68:53";
  private sppOption: any = {uuid: '00001810-0000-1000-8000-00805F9B34FB', secure: false, type: 0};
  private serverNumber: number = -1;
  private clientNumber: number = -1;
  private sppWriteText: string = '';

  build() {
    Row() {
      Column() {
        HeadComponent({headName: $r('app.string.spp_test_title'), isActive: true});
        Stack().height("0.5vp").backgroundColor("#000000");
        Column() {
          Text("请点击需要测试的方法")
            .margin({top: "20vp"})
            .padding("5vp")
            .fontSize("15vp")
            .fontColor("#4444444");

          List() {
            //spp客户端
            ListItem() {
              TitleComponent({ title: "spp_client_test" });
            }
            // 客户端连接
            ListItem() {
              TitleComponent({ title: "spp_client_connect" });
            }.padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
            .onClick(() => {
              this.sppClientConnect()
            })

            // 将设备ID传到服务端
            ListItem() {
              TitleComponent({ title: "spp_client_write" });
            }.padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
            .onClick(() => {
              this.sppClientWrite()
            })

            //关闭客户端
            ListItem() {
              TitleComponent({ title: "spp_client_close" });
            }.padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
            .onClick(() => {
              this.sppClientClose()
            })

          }.margin(20);
          Text(this.message)
            .textAlign(TextAlign.Start)
            .backgroundColor($r("sys.color.ohos_id_color_foreground_contrary"))

          TitleComponent({ title: this.sppWriteText })
        }
        .visibility(Visibility.Visible);
      }
      .width('100%')
    }
    .backgroundColor($r("sys.color.ohos_id_color_sub_background"))
    .height('100%')
    .alignItems(VerticalAlign.Top)
  }

  sppClientConnect() {
    bluetooth.sppConnect(
      this.deviceId,
      this.sppOption,
      (code,socketID) => {
        if (code.code != 0) {
          return;
        }
        this.acceptClientSocket(socketID);
        LogUtil.info(MODEL + 'sppConnect callback socketNumber: ' + socketID);
      }
    );
  }

  sppClientWrite() {
    LogUtil.info(MODEL + 'sppWrite clientNumber: ' + JSON.stringify(this.clientNumber));
    const sppWriteSuccess = bluetooth.sppWrite(
      this.clientNumber,
    this.strToArrayBuffer(this.deviceId)
    );
    if (sppWriteSuccess) {
      console.log('sppWrite successfully');
      this.sppWriteText = 'write success';
    } else {
      console.log('sppWrite failed');
      this.sppWriteText = 'write false';
    }
  }

  sppClientClose() {
    LogUtil.info(MODEL + 'sppCloseClientSocket clientNumber: ' + JSON.stringify(this.clientNumber))
    bluetooth.sppCloseClientSocket(this.clientNumber);
  }

  /**
   * 获取的clientNumber用作客户端后续读/写操作socket的id
   * @param number
   */
  acceptClientSocket(number) {
    console.log('spp clientSocket socketID: ' + number);
    // 获取的clientNumber用作服务端后续读/写操作socket的id。
    this.clientNumber = number;
  }

  dataRead(dataBuffer) {
    let data = new Uint8Array(dataBuffer);
    console.log('spp bluetooth read data is: ' + data[0]);
  }

  /**
  * 字符串转arrayBuffrt
  * @param {*} str 需要转换的字符串
  * @returns 返回 arrayBuffer
  */
  strToArrayBuffer (str) {
    let array = new Uint8Array(str.length);
    for(var i = 0; i < str.length; i++) {
      array[i] = str.charCodeAt(i);
    }
    return array.buffer
  }
}
