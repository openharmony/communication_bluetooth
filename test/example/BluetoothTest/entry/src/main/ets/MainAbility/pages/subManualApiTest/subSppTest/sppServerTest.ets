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

let MODEL = "bluetoothModel:";

@Entry
@Component
struct SppServer {
  @State isConnection: boolean = false;
  private stateBT: string = "";
  @State sppOption: any = {uuid: '00001810-0000-1000-8000-00805F9B34FB', secure: false, type: 0};
  private serverNumber: number = 0;
  private clientNumber: number = 0;

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
            ListItem() {
              TitleComponent({ title: "spp_server_listen" });
            }
            .padding({
              top: $r('app.float.distance_2'),
              bottom: $r('app.float.distance_2')
            })
            .onClick(() => {
              this.sppListenOrAccept()
            })

            ListItem() {
              TitleComponent({ title: "spp_server_close" });
            }
            .padding({
              top: $r('app.float.distance_2'),
              bottom: $r('app.float.distance_2')
            })
            .onClick(() => {
              this.sppCloseServer()
            })
          }
          .margin(20);
        }
      }
      .width('100%')
    }
    .backgroundColor($r("sys.color.ohos_id_color_sub_background"))
    .height('100%')
    .alignItems(VerticalAlign.Top)
  }

  sppListenOrAccept() {
    console.log('spp server listen start! ');
    // 创建蓝牙服务器侦听套接字, 返回服务端ID
    bluetooth.sppListen('BluetoothDevice', this.sppOption, (code,serverSocketID) => {
      console.log('bluetooth error code: ' + code.code);
      if (code.code == 0) {
        console.log('spp server: serverSocketID - ' + serverSocketID);
        this.serverNumber = serverSocketID;
      }
      // 连接客户端，返回客户端ID
      bluetooth.sppAccept(this.serverNumber, (code,clientSocketID) => {
        if (code.code == 0) {
          this.clientNumber = clientSocketID;
          console.log('spp server: clientSocketID - ' + clientSocketID);
          bluetooth.on('sppRead', this.clientNumber, this.dataRead.bind(this));
        }
      });
    });
  }

  //关闭服务端
  sppCloseServer() {
    LogUtil.info(MODEL + 'sppCloseServerSocket serverNumber: ' + this.serverNumber)
    bluetooth.sppCloseServerSocket(this.serverNumber);
    bluetooth.off('sppRead', this.clientNumber, this.dataRead.bind(this));
  }

 dataRead(dataBuffer) {
    const data = new Uint8Array(dataBuffer);
    console.log('spp bluetooth read deviceID is: ' + data);
  }
}



