
/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
import ConfigData from '../../../../Utils/ConfigData';
import BluetoothModel from '../../../model/BluetoothModel';
import BluetoothDevice from '../../../model/BluetoothDevice'
import BluetoothDeviceController from '../../../controller/BluetoothDeviceController';

let MODEL = "bluetoothModel:";

@Entry
@Component
struct SppServer {
  @State message: string = "";
  @State deviceId: string = AppStorage.Get('inputDeviceId');
  @State dataReadMessage: string = "";
  private stateBT: string = "";
  @State writeData: any = '';
  private uuid: string = AppStorage.Get('inputUuid');
  private serverNumber: number = -1;
  private clientNumber: number = -1;
  private sppOption: any ={};
  private TAG_PAGE = ConfigData.TAG + 'Paired_Device ';
  @State btEnable: boolean = false;
  @State currentClick: number = -1;
  @StorageLink('pairedDeviceInfo') pairingDevice: BluetoothDevice = new BluetoothDevice;
  @StorageLink('btDeviceController') controller: BluetoothDeviceController = new BluetoothDeviceController;
  @State pairPinCode: string = '';

  pairDialog: CustomDialogController = new CustomDialogController({
    builder: PairDialog({
      deviceName: this.pairingDevice.deviceName,
      pinCode: this.pairPinCode,
      action: (accept: boolean) => {
        this.confirmPairing(accept);
      }
    }),
    alignment: DialogAlignment.Bottom,
    autoCancel: true
  });

  build() {
    Row() {
      Column() {
        Column() {
          HeadComponent({ headName: $r('app.string.spp_server_test_title'), isActive: true });
          Stack().height("0.5vp").backgroundColor("#000000");
          Row() {
            Text("目标设备MAC：").fontSize("20vp")
            TextInput( { text: this.deviceId, placeholder: "请输入mac地址" } )
              .fontSize("20vp")
              .onChange((str) => {
                this.deviceId = str;
                AppStorage.SetOrCreate('inputDeviceId', str)
              })
              .width('70%')
          }
          .margin( { top: "20vp", left: "20vp", right: "20vp" } )
          .alignItems(VerticalAlign.Center)
          Row() {
            Text("目标设备UUID：").fontSize("20vp")
            TextInput( { text: this.uuid, placeholder: "请输入uuid" } )
              .fontSize("20vp")
              .onChange((str) => {
                this.uuid = str;
                AppStorage.SetOrCreate('inputUuid', str)
              })
              .width('70%')
          }
          .margin( { top: "20vp", left: "20vp", right: "20vp" } )
          .alignItems(VerticalAlign.Center)
          Row() {
            Text("写入目标设备数据：").fontSize("20vp")
            TextInput( { text: this.writeData, placeholder: "请输入写入的数据" } )
              .fontSize("20vp")
              .onChange((str) => {this.writeData = this.strToArrayBuffer(str);})
              .width('70%')
          }
          .margin( { top: "20vp", left: "20vp", right: "20vp" } )
          .alignItems(VerticalAlign.Center)

          Row() {
            Text("读取的数据为：").fontSize("20vp")
            Text(this.dataReadMessage)
              .fontSize("20vp")
              .width('70%')
          }
          .margin( { top: "20vp", left: "20vp", right: "20vp" } )
          .alignItems(VerticalAlign.Center)

          Scroll() {
            Column() {
            List({ space: 1 }) {
              //使能蓝牙
              ListItem() {
                TitleComponent({
                  title: "enableBluetooth",
                  bgColor: this.currentClick === 0 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 0;
                if (this.btEnable) {
                  this.message = '蓝牙已经使能';
                  return;
                }
                let ret = BluetoothModel.enableBluetooth();
                this.btEnable = ret;
                AppStorage.SetOrCreate('bluetoothIsOn', this.btEnable);
                this.message = "蓝牙使能执行结果：" + ret;
              })

              //去使能蓝牙
              ListItem() {
                TitleComponent({
                  title: "disableBluetooth",
                  bgColor: this.currentClick === 1 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 1;
                if (!this.btEnable) {
                  this.message = '蓝牙还未使能';
                  return;
                }
                let ret = BluetoothModel.disableBluetooth();
                this.btEnable = false;
                AppStorage.SetOrCreate('bluetoothIsOn', this.btEnable);
                this.message = "蓝牙去使能执行结果：" + ret;
              })

              //设置扫描方式为可发现可连接
              ListItem() {
                TitleComponent({
                  title: "setBluetoothScanMode",
                  bgColor: this.currentClick === 2 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 2;
                if (!this.btEnable) {
                  this.message = '蓝牙还未使能';
                  return;
                }
                let ret = BluetoothModel.setBluetoothScanMode(4, 0);
                if (ret) {
                  this.message = "设置成功"
                } else {
                  this.message = "设置失败"
                }
              })

              //查询蓝牙状态
              ListItem() {
                TitleComponent({
                  title: "getState",
                  bgColor: this.currentClick === 3 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 3;
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
                    this.stateBT = '未知状态';
                    break;
                }
                this.message = "当前蓝牙的状态是：" + this.stateBT;
              })

              //查询当前扫描方式
              ListItem() {
                TitleComponent({
                  title: "getBluetoothScanMode",
                  bgColor: this.currentClick === 4 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 4;
                let ret = BluetoothModel.getBluetoothScanMode();
                this.message = "蓝牙当前扫描方式为：" + ret;
              })

              //查询当前连接状态
              ListItem() {
                TitleComponent({
                  title: "getBtConnectionState",
                  bgColor: this.currentClick === 5 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 5;
                let ret = BluetoothModel.getBtConnectionState();
                this.message = "当前连接状态是：" + ret;
              })

              //服务端(安全模式)连接
              ListItem() {
                TitleComponent({ title: "spp_server_create(安全模式)" });
              }
              .padding({
                top: $r('app.float.distance_2'),
                bottom: $r('app.float.distance_2')
              })
              .onClick(() => {
                console.log('spp server socketId(safe mode) create! ');
                if (this.uuid == '') {
                  this.message = 'uuid为空';
                  return;
                }
                // 创建蓝牙服务器侦听套接字, 返回服务端ID
                bluetooth.sppListen('BluetoothDevice', {uuid: this.uuid, secure: true, type: 0}, (code,serverSocketID) => {
                  console.log('bluetooth error code: ' + code.code);
                  if (code.code == 0) {
                    console.log('spp server: serverSocketID(safe mode) - ' + serverSocketID);
                    this.serverNumber = serverSocketID;
                    this.message = "创建server成功";
                  }
                });
              })

              //服务端(非安全模式)连接
              ListItem() {
                TitleComponent({ title: "spp_server_create(非安全模式)" });
              }
              .padding({
                top: $r('app.float.distance_2'),
                bottom: $r('app.float.distance_2')
              })
              .onClick(() => {
                console.log('spp server socketId(non-safe mode) create! ');
                if (this.uuid == '') {
                  this.message = 'uuid为空';
                  return;
                }
                // 创建蓝牙服务器侦听套接字, 返回服务端ID
                bluetooth.sppListen('BluetoothDevice', {uuid: this.uuid, secure: false, type: 0}, (code,serverSocketID) => {
                  console.log('bluetooth error code: ' + code.code);
                  if (code.code == 0) {
                    console.log('spp server: serverSocketID(non-safe mode) - ' + serverSocketID);
                    this.serverNumber = serverSocketID;
                    this.message = "创建server成功";
                  }
                });
              })

              //服务端接受操作
              ListItem() {
                TitleComponent({ title: "spp_server_accept" });
              }
              .padding({
                top: $r('app.float.distance_2'),
                bottom: $r('app.float.distance_2')
              })
              .onClick(() => {
                // 连接客户端，返回客户端ID
                this.message = "等待客户端连接";
                bluetooth.sppAccept(this.serverNumber, (code,clientSocketID) => {
                  if (code.code == 0) {
                    console.log('spp server: clientSocketID - ' + clientSocketID);
                    this.clientNumber = clientSocketID;
                    this.message = "接受客户端成功";
                    bluetooth.on('sppRead', this.clientNumber, this.dataRead.bind(this));
                  }
                });
              })

              //服务端写数据传给客户端
              ListItem() {
                TitleComponent({ title: "spp_client_write" });
              }.padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                LogUtil.info(MODEL + 'sppWrite clientNumber: ' + JSON.stringify(this.clientNumber));
                const sppWriteSuccess = bluetooth.sppWrite(this.clientNumber, this.writeData);
                if (sppWriteSuccess) {
                  console.log('sppWrite successfully');
                  this.message = "写入成功 ";
                } else {
                  console.log('sppWrite failed');
                  this.message = "写入失败";
                }
              })

              //服务端读操作打开
              ListItem() {
                TitleComponent({ title: "spp_server_readOn" });
              }
              .padding({
                top: $r('app.float.distance_2'),
                bottom: $r('app.float.distance_2')
              })
              .onClick(() => {
                  bluetooth.on('sppRead', this.clientNumber, this.dataRead.bind(this));
              })

              //服务端读操作关闭
              ListItem() {
                TitleComponent({ title: "spp_server_readOff" });
              }
              .padding({
                top: $r('app.float.distance_2'),
                bottom: $r('app.float.distance_2')
              })
              .onClick(() => {
                bluetooth.off('sppRead', this.clientNumber, this.dataRead.bind(this));
              })

              //关闭服务端serversocket
              ListItem() {
                TitleComponent({ title: "spp_server_close" });
              }
              .padding({
                top: $r('app.float.distance_2'),
                bottom: $r('app.float.distance_2')
              })
              .onClick(() => {
                LogUtil.info(MODEL + 'sppCloseServerSocket serverNumber: ' + this.serverNumber)
                bluetooth.sppCloseServerSocket(this.serverNumber);
                this.message = "关闭服务端";
              })

              //关闭服务端accept回调的clientsocket
              ListItem() {
                TitleComponent({ title: "spp_client_close" });
              }
              .padding({
                top: $r('app.float.distance_2'),
                bottom: $r('app.float.distance_2')
              })
              .onClick(() => {
                LogUtil.info(MODEL + 'sppCloseClientSocket serverNumber: ' + this.clientNumber)
                bluetooth.sppCloseClientSocket(this.clientNumber);
                this.message = "关闭client端";
              })
            }.margin(20);
            }
            .width('100%')
            .visibility(Visibility.Visible)
          }.height('50%')
          Text("接口调用结果：" + this.message)
          .textAlign(TextAlign.Start)
          .backgroundColor($r("sys.color.ohos_id_color_foreground_contrary"))
        }
      }
      .width('100%')
    }
    .backgroundColor($r("sys.color.ohos_id_color_sub_background"))
    .height('100%')
    .alignItems(VerticalAlign.Top)
  }

  /**
  * 字符串转arrayBuffer
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

  /**
  * Pair device
  * @param device
  */
  pairDevice(deviceId: string) {
    this.controller.pair(this.deviceId, (pinCode: string) => {
      LogUtil.info(this.TAG_PAGE + 'pairDevice success callback : pinCode = ' + pinCode);
      // show pair pin dialog
      this.pairPinCode = pinCode
      this.pairingDevice.deviceId = this.deviceId;
      this.pairingDevice.deviceName = this.deviceId;
      this.pairDialog.open();
    }, () => {
      LogUtil.info(this.TAG_PAGE + 'pairDevice error callback');
      this.showPairFailedDialog()
    })
  }

  /**
  * Confirm pairing
  */
  confirmPairing(accept: boolean) {
    LogUtil.info(this.TAG_PAGE + 'confirmPairing : pairingDevice + ' + JSON.stringify(this.pairingDevice));
    this.controller.confirmPairing(this.pairingDevice.deviceId, accept);
  }

  /**
  * Show pair failed dialog
  */
  showPairFailedDialog() {
    this.showPairingFailedDialog()
  }

  /**
  * Show device illegal prompt dialog
  */
  private showDeviceIllegalPromptDialog(action) {
    AlertDialog.show({
      message: $r("app.string.bluetooth_address_error_msg"),
      primaryButton: {
        value: $r('app.string.cancel'),
        action: () => {
          LogUtil.info(this.TAG_PAGE + 'Closed callbacks');
        }
      },
      secondaryButton: {
        value: $r('app.string.bluetooth_pair_button'),
        action: () => {
          action();
          LogUtil.info(this.TAG_PAGE + `AlertDialog success:`);
        }
      },
      alignment: DialogAlignment.Bottom
    })
  }

  /**
   * Show connect Failed Dialog
   */
  private showConnectFailedDialog() {
    showDialog(
    $r("app.string.bluetooth_connect_failed"),
    $r("app.string.bluetooth_connect_failed_msg"),
    $r("app.string.bluetooth_know_button")
    );
  }

  /**
   * Show pairing failed title Dialog
   */
  private showPairingFailedDialog() {
    showDialog(
    $r("app.string.pairing_failed_title"),
    $r("app.string.pairing_failed_message"),
    $r("app.string.bluetooth_know_button")
    );
  }

  dataRead(dataBuffer) {
    const data = new Uint8Array(dataBuffer);
    console.log('spp bluetooth read is: ' + data);
    if(data != null) {
      this.dataReadMessage = "读取成功" + JSON.stringify(data);
    } else {
      this.dataReadMessage = "读取失败";
    }
  }
}

/**
 * Pair dialog
 */
@CustomDialog
struct PairDialog {
  dialogController: CustomDialogController;
  action: (accept: boolean) => void;
  private deviceName: string = undefined;
  private pinCode: string;

  aboutToAppear(): void {
    LogUtil.log(ConfigData.TAG + `bluetooth PairDialog aboutToAppear pinCode = ${this.pinCode}`)
  }

  build() {
    Column() {
      Text($r('app.string.bluetooth_pairing_request'))
        .fontSize($r('app.float.font_22'))
        .height($r('app.float.wh_value_60'))
        .width(ConfigData.WH_100_100)
        .padding({
          left: $r('app.float.distance_24'),
          top: $r('app.float.distance_14'),
          bottom: $r('app.float.distance_14')
        })
      Column() {
        if (this.pinCode) {
          Text($r('app.string.bluetooth_pairing_intelligent_device_hit', this.deviceName, this.deviceName))
            .fontSize($r('app.float.font_16'))
            .width(ConfigData.WH_100_100)
            .margin({
              top: $r('app.float.distance_14'),
              bottom: $r('app.float.distance_14')
            })

          Text(`${this.pinCode}`)
            .fontSize($r('app.float.pinCode_font_size'))
            .fontWeight(FontWeight.Bolder)
            .width(ConfigData.WH_100_100)
            .textAlign(TextAlign.Center)
            .margin({
              top: $r('app.float.item_common_horizontal_margin'),
              bottom: $r('app.float.item_common_horizontal_margin')
            })

        } else {
          Text($r('app.string.bluetooth_pairing_media_device_hit'))
            .fontSize($r('app.float.font_16'))
            .width(ConfigData.WH_100_100)
            .margin({
              bottom: $r('app.float.switch_summary_margin')
            })

          Text(this.deviceName)
            .fontSize($r('app.float.font_16'))
            .width(ConfigData.WH_100_100)
            .fontWeight(FontWeight.Bold)
        }

        // button
        Flex({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {

          Button() {
            Text($r('app.string.cancel'))
              .fontSize($r('app.float.font_16'))
              .fontColor(Color.Blue)
          }
          .backgroundColor($r("app.color.white_bg_color"))
          .width($r("app.float.wh_value_160"))
          .height($r("app.float.wh_value_40"))
          .flexGrow(1)
          .onClick(() => {
            this.dialogController.close();
            this.action(false);
          })

          Divider()
            .height($r("app.float.wh_value_24"))
            .strokeWidth(0.5)
            .vertical(true)
            .color(($r("app.color.color_E3E3E3_grey")))

          Button() {
            Text($r('app.string.bluetooth_pair_button'))
              .fontSize($r('app.float.font_16'))
              .fontColor(Color.Blue)
          }
          .backgroundColor($r("app.color.white_bg_color"))
          .width($r("app.float.wh_value_160"))
          .height($r("app.float.wh_value_40"))
          .flexGrow(1)
          .onClick(() => {
            this.dialogController.close();
            this.action(true);
          })
        }
        .width(ConfigData.WH_100_100)
        .height($r('app.float.wh_value_60'));
      }
      .width(ConfigData.WH_100_100)
      .padding({
        left: $r('app.float.distance_24'),
        right: $r('app.float.distance_24')
      })
    }
    .padding({
      left: $r('app.float.distance_24'),
      right: $r('app.float.distance_24')
    })
    .width(ConfigData.WH_100_100);
  }
}
/**
 * Pair mode prompt
 * @param dialogTitle Dialog title
 * @param dialogMessage Dialog message
 * @param buttonValue Dialog buttonValue
 */
function showDialog(dialogTitle: string | Resource, dialogMessage: string | Resource, buttonValue: string | Resource){
  AlertDialog.show({
    title: dialogTitle,
    message: dialogMessage,
    confirm: {
      value: buttonValue,
      action: () => {
        LogUtil.info('Button-clicking callback')
      }
    },
    cancel: () => {
      LogUtil.info('Closed callbacks')
    },
    alignment: DialogAlignment.Bottom
  })
}

