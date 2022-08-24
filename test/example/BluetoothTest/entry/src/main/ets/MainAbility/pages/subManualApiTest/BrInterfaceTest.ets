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
import { TitleComponent } from '../../../Component/titleComponent';
import HeadComponent from '../../../component/headcomponent';
import BluetoothModel from '../../model/BluetoothModel';
import Router from '@system.router';
import BluetoothDevice from '../../model/BluetoothDevice'
import BluetoothDeviceController from '../../controller/BluetoothDeviceController';
import ConfigData from '../../../Utils/ConfigData';
import LogUtil from '../../../Utils/LogUtil';
import bluetooth from '@ohos.bluetooth';

const PAGE_URI_DEVICE_NAME = 'pages/subManualApiTest/subBrTest/deviceName';
const PAGE_URI_SET_SCAN_MODE = 'pages/subManualApiTest/subBrTest/setScanMode';
const PAGE_URI_DEVICE_FOUND_MODE = 'pages/subManualApiTest/subBrTest/deviceFound';
const PAGE_URI_PROFILE_CONN_STATE = 'pages/subManualApiTest/subBrTest/profileConnState';
const PAGE_URI_PROFILE_CONN_STATE_BY_ID = 'pages/subManualApiTest/subBrTest/searchProfileConnStateById';
const PAGE_URI_PAIRED_DEVICES = "pages/subManualApiTest/subBrTest/pairedDevices";

@Entry
@Component
struct Index {
  private TAG_PAGE = ConfigData.TAG + 'Paired_Device ';
  @State message: string = "";
  @State btStateMessage: string = "";
  @State pinMessage: string = "";
  @State bondStateMessage: string = "";
  @State btEnable: boolean = false;
  @State isConnection: boolean = false;
  @StorageLink('setLocalNameRet') setLocalNameRet: boolean = false;
  @StorageLink('setScanModeRet') setScanModeRet: boolean = false;
  private stateBT: string = "";
  @State deviceId: string = '';
  @State currentClick: number = -1;
  @StorageLink('pairedDeviceInfo') pairingDevice: BluetoothDevice = new BluetoothDevice;
  @StorageLink('pairedDeviceId') pairedDeviceId: string = '';
  @StorageLink('btDeviceController') controller: BluetoothDeviceController = new BluetoothDeviceController;
  @State pairPinCode: string = '';
  @State btOnState: string = 'on:stateChange';
  @State btPinRequired: string = 'on:pinRequired';
  @State btBondStateChange: string = 'on:bondStateChange';
  @State isStateChangeClick: boolean = false;
  @State isPinRequiredClick: boolean = false;
  @State isBondStateChangeClick: boolean = false;

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

  aboutToAppear() {
    this.btEnable = BluetoothModel.isStateOn();
    AppStorage.SetOrCreate('bluetoothIsOn', this.btEnable);
  }

  build() {
    Row() {
      Column() {
        HeadComponent({ headName: 'BR接口测试', isActive: true });
        Stack().height("0.5vp").backgroundColor("#000000");
        Row() {
          Text("目标设备MAC：").fontSize("20vp")
          TextInput( { text: this.deviceId, placeholder: "请输入即将连接的设备MAC" } )
          .fontSize("20vp")
          .onChange((str) => {
            this.deviceId = str;
          })
          .width('70%')
        }
        .margin( { top: "20vp", left: "20vp", right: "20vp" } )
        .alignItems(VerticalAlign.Center)

        Scroll() {
          Column() {
            List({ space: 1 }) {
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

              ListItem() {
                TitleComponent({
                  title: "getState",
                  bgColor: this.currentClick === 2 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 2;
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

              ListItem() {
                TitleComponent({
                  title: "getBtConnectionState",
                  bgColor: this.currentClick === 3 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 3;
                let ret = BluetoothModel.getBtConnectionState();
                this.message = "当前连接状态是：" + ret;
              })

              ListItem() {
                TitleComponent({
                  title: "setLocalName",
                  bgColor: this.currentClick === 4 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 4;
                if (this.btEnable) {
                  Router.push({ uri: PAGE_URI_DEVICE_NAME });
                  this.message = "设置SCAN MODE " + this.setScanModeRet ? '成功' : '失败';
                } else {
                  this.message = "蓝牙未使能";
                }
              })

              ListItem() {
                TitleComponent({
                  title: "getLocalName",
                  bgColor: this.currentClick === 5 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 5;
                let ret = BluetoothModel.getLocalName();
                this.message = "本设备名称是：" + ret;
              })

              ListItem() {
                TitleComponent({
                  title: "setBluetoothScanMode",
                  bgColor: this.currentClick === 6 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 6;
                if (this.btEnable) {
                  Router.push({ uri: PAGE_URI_SET_SCAN_MODE });
                  this.message = "setBluetoothScanMode执行" + this.setLocalNameRet ? '成功' : '失败';
                } else {
                  this.message = "蓝牙未使能";
                }
              })

              ListItem() {
                TitleComponent({
                  title: "getBluetoothScanMode",
                  bgColor: this.currentClick === 7 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 7;
                let ret = BluetoothModel.getBluetoothScanMode();
                this.message = "蓝牙当前的扫描模式是: " + ret;
              })

              ListItem() {
                TitleComponent({
                  title: "startBluetoothDiscovery",
                  bgColor: this.currentClick === 8 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                LogUtil.info(this.TAG_PAGE + 'startBluetoothDiscovery 111');
                if (!this.btEnable) {
                  this.message = '蓝牙未使能';
                  return;
                }
                this.currentClick = 8;
                Router.push({ uri: PAGE_URI_DEVICE_FOUND_MODE });
              })

              ListItem() {
                TitleComponent({
                  title: "stopBluetoothDiscovery",
                  bgColor: this.currentClick === 9 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 9;
                let ret = BluetoothModel.stopBluetoothDiscovery();
                this.message = "stopBluetoothDiscovery 执行: " + ret;
              })

              ListItem() {
                TitleComponent({
                  title: "pairDevice",
                  bgColor: this.currentClick === 10 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 10;
                if (!this.btEnable) {
                  this.message = '蓝牙未使能';
                  return;
                }
                if (this.deviceId == '') {
                  this.message = "请输入目标设备的MAC";
                  return;
                } else {
                  this.pairDevice(this.deviceId);
                }
              })

              ListItem() {
                TitleComponent({
                  title: "getRemoteDeviceName",
                  bgColor: this.currentClick === 11 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 11;
                if (!this.btEnable) {
                  this.message = '蓝牙未使能';
                  return;
                }
                if (this.deviceId == '') {
                  this.message = "请输入目标设备的MAC";
                  return;
                } else {
                  let ret = BluetoothModel.getDeviceName(this.deviceId);
                  this.message = '配对设备为' + ret;
                }
              })

              ListItem() {
                TitleComponent({
                  title: "getRemoteDeviceClass",
                  bgColor: this.currentClick === 12 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 12;
                if (!this.btEnable) {
                  this.message = '蓝牙未使能';
                  return;
                }
                if (this.deviceId == '') {
                  this.message = "请输入目标设备的MAC";
                  return;
                } else {
                  let ret = BluetoothModel.getDeviceType(this.deviceId);
                  this.message = '依据majorClass解析的设备类型是' + ret;
                }
              })

              ListItem() {
                TitleComponent({
                  title: "getPairedDevices",
                  bgColor: this.currentClick === 13 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 13;
                let ret = BluetoothModel.getPairedDeviceIds();
                if (ret.length == 0) {
                  this.message = '当前没有任何配对设备';
                  return;
                }
                AppStorage.SetOrCreate('pairedDevices', ret);
                Router.push({ uri: PAGE_URI_PAIRED_DEVICES })
              })

              ListItem() {
                TitleComponent({
                  title: "cancelPairedDevice",
                  bgColor: this.currentClick === 14 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 14;
                if (!this.btEnable) {
                  this.message = '蓝牙未使能';
                  return;
                }
                if (this.deviceId == '') {
                  this.message = "请输入目标设备的MAC";
                  return;
                } else {
                  let ret = BluetoothModel.unpairDevice(this.deviceId);
                  this.message = '设备 "' + this.deviceId + '" 断开：' + ret;
                }
              })

              ListItem() {
                TitleComponent({
                  title: "getProfileConnState1",
                  bgColor: this.currentClick === 15 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 15;
                Router.push({ uri: PAGE_URI_PROFILE_CONN_STATE })
                this.message = '';
              })

              ListItem() {
                TitleComponent({
                  title: "getProfileConnState2",
                  bgColor: this.currentClick === 16 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                this.currentClick = 16;
                Router.push({ uri: PAGE_URI_PROFILE_CONN_STATE_BY_ID })
                this.message = '';
              })

              ListItem() {
                TitleComponent({
                  title: this.btOnState,
                  stateChangeFlag: true,
                  bgColor: this.currentClick === 17 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                if (this.isStateChangeClick) {
                  bluetooth.off('stateChange', () => {})
                  this.message = 'on:stateChange监听已关闭，再次点击将开启监听';
                  this.btOnState = 'on:stateChange'
                  AppStorage.SetOrCreate('on_stateChange', this.btOnState);
                  this.isStateChangeClick = false;
                  return;
                }
                this.isStateChangeClick = true;
                this.currentClick = 17;
                this.btOnState = 'off:stateChange'
                AppStorage.SetOrCreate('on_stateChange', this.btOnState);
                this.btStateMessage = 'STATE: ';
                bluetooth.on('stateChange', (data) => {
                  this.btStateMessage = 'STATE: ';
                  switch (data) {
                    case 0:
                      this.btStateMessage += 'STATE_OFF';
                      break;
                    case 1:
                      this.btStateMessage += 'STATE_TURNING_ON';
                      break;
                    case 2:
                      this.btStateMessage += 'STATE_ON';
                      break;
                    case 3:
                      this.btStateMessage += 'STATE_TURNING_OFF';
                      break;
                    case 4:
                      this.btStateMessage += 'STATE_BLE_TURNING_ON';
                      break;
                    case 5:
                      this.btStateMessage += 'STATE_BLE_ON';
                      break;
                    case 6:
                      this.btStateMessage += 'STATE_BLE_TURNING_OFF';
                      break;
                    default:
                      this.btStateMessage += '未知状态';
                      break;
                  }
                })
                this.message = 'on:stateChange监听已启动，再次点击将关闭监听。';
              })

              ListItem() {
                TitleComponent({
                  title: this.btPinRequired,
                  pinRequiredFlag: true,
                  bgColor: this.currentClick === 18 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                if (!this.btEnable) {
                  this.message = "蓝牙未使能";
                  return;
                }
                if (this.isPinRequiredClick) {
                  bluetooth.off('pinRequired', () => {

                  })
                  this.message = 'on:pinRequired监听已关闭，再次点击将开启监听';
                  this.isPinRequiredClick = false;
                  this.btPinRequired = 'on:pinRequired';
                  AppStorage.SetOrCreate('on_pinRequired', this.btPinRequired);
                  return;
                }
                this.isPinRequiredClick = true;
                this.currentClick = 18;
                this.btPinRequired = 'off:pinRequired';
                AppStorage.SetOrCreate('on_pinRequired', this.btPinRequired);
                this.pinMessage = 'PIN: ';
                bluetooth.on('pinRequired', (data) => {
                  this.pinMessage = 'PIN: ';
                  this.pinMessage += data.pinCode;
                })
                this.message = 'on:pinRequired监听已启动，再次点击将关闭监听。';
              })

              ListItem() {
                TitleComponent({
                  title: this.btBondStateChange,
                  bondStateChangeFlag: true,
                  bgColor: this.currentClick === 19 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')
                });
              }
              .padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
              .onClick(() => {
                if (!this.btEnable) {
                  this.message = "蓝牙未使能";
                  return;
                }
                if (this.isBondStateChangeClick) {
                  bluetooth.off('bondStateChange', () => {
                  })
                  this.message = 'on:bondStateChange监听已关闭，再次点击将开启监听';
                  this.isBondStateChangeClick = false;
                  this.btBondStateChange = 'on:bondStateChange'
                  AppStorage.SetOrCreate('on_bondStateChange', this.btBondStateChange);
                  return;
                }
                this.isBondStateChangeClick = true;
                this.currentClick = 19;
                this.btBondStateChange = 'off:bondStateChange'
                AppStorage.SetOrCreate('on_bondStateChange', this.btBondStateChange);
                this.bondStateMessage = 'bondState: ';
                bluetooth.on('bondStateChange', (data) => {
                  this.bondStateMessage = 'bondState: ';
                  this.bondStateMessage += data.state;
                })
                this.message = 'on:bondStateChange监听已启动，再次点击将关闭监听';
              })

            }
            .margin(20)
          }
          .width('100%')
          .visibility(Visibility.Visible)
        }
        .height('70%')

        Text(this.message)
          .fontSize('15vp')
          .textAlign(TextAlign.Start)
          .backgroundColor($r("sys.color.ohos_id_color_foreground_contrary"))
          .padding({ top: $r('app.float.distance_12'), bottom: $r('app.float.distance_12') })
        Text(this.btStateMessage)
          .fontSize('15vp')
          .textAlign(TextAlign.Start)
          .backgroundColor($r("sys.color.ohos_id_color_foreground_contrary"))
        Text(this.pinMessage)
          .fontSize('15vp')
          .textAlign(TextAlign.Start)
          .backgroundColor($r("sys.color.ohos_id_color_foreground_contrary"))
        Text(this.bondStateMessage)
          .fontSize('15vp')
          .textAlign(TextAlign.Start)
          .backgroundColor($r("sys.color.ohos_id_color_foreground_contrary"))
      }
      .height('100%')
    }
    .backgroundColor($r("sys.color.ohos_id_color_sub_background"))
    .height('100%')
    .alignItems(VerticalAlign.Top)
  }

  /**
 * Pair device
 * @param device
 */
  pairDevice(deviceId: string) {
    this.controller.pair(deviceId, (pinCode: string) => {
      LogUtil.info(this.TAG_PAGE + 'pairDevice success callback : pinCode = ' + pinCode);
      // show pair pin dialog
      this.pairPinCode = pinCode
      this.pairingDevice.deviceId = deviceId;
      this.pairingDevice.deviceName = deviceId;
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

