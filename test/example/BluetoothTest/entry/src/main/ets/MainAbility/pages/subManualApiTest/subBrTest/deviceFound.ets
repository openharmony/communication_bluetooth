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
import ConfigData from '../../../../Utils/ConfigData';
import HeadComponent from '../../../../component/headcomponent';
import EntryComponent from '../../../../component/EntryComponent';
import LogUtil from '../../../../Utils/LogUtil';
import BluetoothDeviceController from '../../../controller/BluetoothDeviceController';
import BluetoothDevice from '../../../model/BluetoothDevice'
import Router from '@system.router';
import { DeviceType, DeviceState } from '../../../model/BluetoothModel';
import ImageAnimatorComponent from '../../../../component/ImageAnimatorComponent';
/**
 * BR Test Page Of Bluetooth test
 */
const PAIRED_ITEM_NUMBER = 3;
const PAGE_URI_BLUETOOTH_PAIRED_DEVICE_INFO = 'pages/subManualApiTest/subBrTest/BluetoothPairedDeviceInfo';

@Entry
@Component
struct deviceFound {
  private PAGE_TAG = ConfigData.TAG + 'Bluetooth page '
  private deviceController: BluetoothDeviceController = new BluetoothDeviceController();
  @State message: string = 'BrTest';
  @State bluetoothSwitch: boolean = false;
  @State scanSwitch: boolean = false;

  @StorageLink('bluetoothIsOn') isOn: boolean = false;
  @StorageLink('bluetoothToggleEnabled') enabled: boolean = true;
  @StorageLink('bluetoothLocalName') localName: string = '';

  aboutToAppear(): void {
    LogUtil.log(this.PAGE_TAG + 'aboutToAppear in : isOn = ' + this.isOn )
    this.deviceController
      .initData()
      .subscribe();
    LogUtil.log(this.PAGE_TAG + 'aboutToAppear out : isOn = ' + this.isOn )
  }

  onPageShow(): void {
    LogUtil.log(this.PAGE_TAG + 'onPageShow in : localName = ' + this.localName )
    this.deviceController.getLocalName();
    AppStorage.SetOrCreate('btDeviceController', this.deviceController);
    LogUtil.log(this.PAGE_TAG + 'onPageShow out : localName = ' + this.localName )
  }

//  aboutToDisappear(): void {
//    this.deviceController.unsubscribe();
//  }

  build() {
    Column() {
      GridContainer({ columns: 12, sizeType: SizeType.Auto, gutter: vp2px(1) === 2 ? '12vp' : '0vp', margin: vp2px(1) === 2 ? '24vp' : '0vp' }) {
        Row({}) {
          Column() {
          }
          .width(ConfigData.WH_100_100)
          .height(ConfigData.WH_100_100)
          .useSizeType({
            xs: { span: 0, offset: 0 }, sm: { span: 0, offset: 0 },
            md: { span: 0, offset: 0 }, lg: { span: 2, offset: 0 }
          });
          Column({space: '12vp'}) {
            HeadComponent({ headName: '扫描蓝牙设备', isActive: true });

            Text($r('app.string.bluetooth_visible_to_nearby'))
              .width(ConfigData.WH_100_100)
              .fontSize($r('app.float.font_14'))
              .fontColor($r('app.color.font_color_182431'))
              .visibility(this.isOn ? Visibility.Visible : Visibility.None)
              .padding({
                left: $r('app.float.distance_24'),
                top: $r('app.float.distance_8'),
                bottom: $r('sys.float.ohos_id_card_margin_middle')
              })

            Scroll() {
              Column() {
                DeviceNameComponent({
                  enabled: $isOn,
                  localName: $localName
                })

                if (this.isOn) {
                  PairedDeviceComponent({
                    controller: this.deviceController
                  })

                  AvailableDeviceComponent({
                    controller: this.deviceController
                  })

                }
              }
              .width(ConfigData.WH_100_100)
              .margin({ bottom: $r('sys.float.ohos_id_default_padding_bottom_fixed') })
            }
            .width(ConfigData.WH_100_100)
            .align(Alignment.TopStart)
            .layoutWeight(1)
            .margin({ top: $r('sys.float.ohos_id_card_margin_middle') })
          }
          .padding({
            left: $r('app.float.distance_24'),
            right: $r('app.float.distance_24'),
            bottom: $r('app.float.distance_32')
          })
          .width(ConfigData.WH_100_100)
          .height(ConfigData.WH_100_100)
          .useSizeType({
            xs: { span: 12, offset: 0 }, sm: { span: 12, offset: 0 },
            md: { span: 12, offset: 0 }, lg: { span: 8, offset: 2 }
          });

          Column() {
          }
          .width(ConfigData.WH_100_100)
          .height(ConfigData.WH_100_100)
          .useSizeType({
            xs: { span: 0, offset: 12 }, sm: { span: 0, offset: 12 },
            md: { span: 0, offset: 12 }, lg: { span: 2, offset: 10 }
          })
        }
        .width(ConfigData.WH_100_100)
        .height(ConfigData.WH_100_100);
      }
      .width(ConfigData.WH_100_100)
      .height(ConfigData.WH_100_100);
    }
    .backgroundColor($r("sys.color.ohos_id_color_sub_background"))
    .width(ConfigData.WH_100_100)
    .height(ConfigData.WH_100_100);
  }
}



/**
 * Device name component
 */
@Component
struct DeviceNameComponent {
  @Link enabled: boolean;
  @Link localName: string;

  build() {
    Row() {
      EntryComponent({
        settingIcon: '',
        settingTitle: $r('app.string.bluetooth_device_name'),
        settingSummary: '',
        settingValue: this.localName,
        settingArrow: $r("app.media.ic_settings_arrow"),
        settingArrowStyle: '',
        settingUri: '',
        enabled: this.enabled,
        height: ($r('app.float.wh_value_48')),
        fontSize: ($r('app.float.font_16'))
      })
    }
    .width(ConfigData.WH_100_100)
    .height($r('app.float.wh_value_56'))
    .borderRadius($r("app.float.radius_24"))
    .padding($r('app.float.distance_4'))
    .backgroundColor($r("app.color.white_bg_color"));
  }
}

/**
 * Paired device component
 */
@Component
struct PairedDeviceComponent {
  private TAG_PAGE = ConfigData.TAG + 'PairedDeviceComponent ';

  private controller: BluetoothDeviceController;

  @StorageLink('bluetoothPairedDevices') pairedDevices: BluetoothDevice[] = [];
  @State isTouched: boolean = false;
  @State hide: boolean = true;

  aboutToAppear(): void{
    if (this.controller) {
      // bind component and initialize
      this.controller.bindComponent(this)
        .bindProperties(["pairedDevices"])
        .initData();
    }
  }

  build() {
    Column() {
      if (this.pairedDevices && this.pairedDevices.length > 0) {
        // paired devices title
        Row() {
          Text($r('app.string.bluetooth_paired_devices'))
            .fontSize($r('app.float.font_14'))
            .fontColor($r('app.color.font_color_182431'))
        }
        .width(ConfigData.WH_100_100)
        .padding({
          left: $r('app.float.distance_24'),
          top: $r('app.float.distance_19_5'),
          bottom: $r('app.float.distance_9_5')
        })

        List() {
          // paired devices list
          ForEach(this.pairedDevices, (item: BluetoothDevice, index: number) => {

            if (index < PAIRED_ITEM_NUMBER || !this.hide) {
              ListItem() {
                Row() {
                  EntryComponent({
                    settingIcon: getDeviceIconPath(item.deviceType),
                    settingTitle: item.deviceName,
                    settingSummary: getConnectionStateText(item),
                    settingValue: '',
                    settingArrow: $r("app.media.ic_settings_arrow"),
                    settingArrowStyle: 'bluetooth',
                    settingUri: '',
                    titleFontColor: this.isHeadPhoneConnected(item) ? $r("app.color.bluetooth_text_color_highlight") : $r("sys.color.ohos_id_color_text_primary"),
                    image_wh: $r('app.float.wh_value_24'),
                    height: getConnectionStateText(item) == '' ? $r('app.float.wh_value_56') : ($r('app.float.wh_value_64')),
                    fontSize: ($r('app.float.font_16')),
                    onArrowClick: () => {
                      LogUtil.info(this.TAG_PAGE + 'item go detail : ' + JSON.stringify(item));
                      this.gotoPairedDeviceInfo(item);
                    }
                  });
                }
                .width(ConfigData.WH_100_100)
                .borderRadius($r("app.float.radius_24"))
                .padding($r('app.float.distance_4'))
                .backgroundColor($r("app.color.white_bg_color"))
                .onClick(() => {
                  this.itemClicked(item);
                })
              }
            }

            if ((this.hide && index === PAIRED_ITEM_NUMBER) || //more
            (!this.hide && index >= PAIRED_ITEM_NUMBER && index == this.pairedDevices.length - 1)) { //put_away
              ListItem() {
                Stack({ alignContent: Alignment.Center }) {
                  Stack({ alignContent: Alignment.Center }) {
                    Text(this.hide ? $r('app.string.more') : $r('app.string.put_away'))
                      .fontColor($r('app.color.color_D8D8D8_grey'))
                      .fontSize($r('app.float.font_14'))
                  }
                  .width(ConfigData.WH_100_100)
                  .height($r("app.float.wh_value_48"))
                  .borderRadius($r("app.float.radius_20"))
                  .backgroundColor(this.isTouched ? $r("app.color.color_D8D8D8_grey") : $r("sys.color.ohos_id_color_foreground_contrary"))
                  .onTouch((event: TouchEvent) => {
                    if (event.type === TouchType.Down) {
                      this.isTouched = true;
                    }
                    if (event.type === TouchType.Up) {
                      this.isTouched = false;
                    }
                  })
                  .onClick(() => {
                    this.hide = !this.hide;
                  })
                }
                .height($r("app.float.wh_value_48"))
                .backgroundColor($r("sys.color.ohos_id_color_foreground_contrary"))
                .padding($r("app.float.distance_4"))
              }
            }
          }, item => JSON.stringify(item));
        }
        .divider({
          strokeWidth: 1,
          color: $r('app.color.color_E3E3E3_grey'),
          startMargin: $r('app.float.wh_value_52'),
          endMargin: $r('app.float.wh_value_12')
        })
        .backgroundColor($r("app.color.white_bg_color"))
        .borderRadius($r("app.float.radius_24"))
      }
    }
  }

  /**
   * Whether headphone connected.
   * @param item device
   * @return headphone connected or not
   */
  private isHeadPhoneConnected(item: BluetoothDevice): boolean {
    return item.deviceType === DeviceType.HEADPHONE && item.connectionState === DeviceState.STATE_CONNECTED
  }

  /**
   * Goto paired device Info
   * @param item device
   */

  private gotoPairedDeviceInfo(item: BluetoothDevice) {
    if (item.connectionState != DeviceState.STATE_CONNECTING
    && item.connectionState != DeviceState.STATE_DISCONNECTING) {
      LogUtil.info(this.TAG_PAGE + 'item right icon on click : ' + JSON.stringify(item));
      let param = { 'bluetoothDevice': JSON.stringify(item) }
      Router.push({
        uri: PAGE_URI_BLUETOOTH_PAIRED_DEVICE_INFO,
        params: param
      });
    }
  }

  /**
   * Item clicked
   * @param item device
   */
  private itemClicked(item: BluetoothDevice) {
    switch (item.connectionState) {
      case DeviceState.STATE_CONNECTED:
        this.showDisconnectDialog(item.deviceName, () => {
          this.controller.disconnect(item.deviceId)
        });
        break;
      case DeviceState.STATE_DISCONNECTED:
        if (!this.controller.connect(item.deviceId)) {
          this.showConnectFailedDialog();
        }
        break;
    }
  }

  /**
   * Connect Failed Dialog
   */
  private showConnectFailedDialog() {
    showDialog(
    $r("app.string.bluetooth_connect_failed"),
    $r("app.string.bluetooth_connect_failed_msg"),
    $r("app.string.bluetooth_know_button")
    );
  }

  /**
   * Disconnect Dialog
   */
  showDisconnectDialog(deviceName, callback) {
    AlertDialog.show({
      title: $r('app.string.bluetooth_disconnect'),
      message: $r("app.string.bluetooth_disconnect_device", deviceName),
      primaryButton: {
        value: $r('app.string.cancel'),
        action: () => {
          LogUtil.info(ConfigData.TAG + 'Closed callbacks');
        }
      },
      secondaryButton: {
        value: $r('app.string.confirm'),
        action: () => {
          LogUtil.info(ConfigData.TAG + `AlertDialog success:`);
          callback();
        }
      },
      alignment: DialogAlignment.Bottom
    })
  }

}

/**
 * Discovering animator component
 */
@Component
struct DiscoveringAnimatorComponent {
  build() {
    Column() {
      Row() {
        ImageAnimatorComponent({
          imageWidth: $r('app.float.wh_value_40'),
          imageHeight: $r('app.float.wh_value_40') })
      }
    }
  }
}

/**
 * Available device component
 */
@Component
struct AvailableDeviceComponent {
  private TAG_PAGE = ConfigData.TAG + 'AvailableDeviceComponent ';

  private controller: BluetoothDeviceController;

  @State isDeviceDiscovering: boolean = false;
  @StorageLink('bluetoothAvailableDevices') availableDevices: BluetoothDevice[] = [];

  @State pairPinCode: string = '';

  private pairingDevice: BluetoothDevice = undefined;

  @State pairDeviceInfo: string = '未配对';

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

  aboutToAppear(): void{
    if (this.controller) {

      // bind component and initialize
      this.controller.bindComponent(this)
        .bindProperties(["isDeviceDiscovering", "availableDevices", "pairPinCode"])
        .initData();

      this.controller.startBluetoothDiscovery();
    }
  }

  build() {
    Column() {
      Row() {
        // available devices title
        Text($r('app.string.bluetooth_available_devices'))
          .fontSize($r('app.float.font_14'))
          .fontColor($r('app.color.font_color_182431'))
          .width(ConfigData.WH_100_100)
          .padding({
            left: $r('app.float.distance_24'),
            top: $r('app.float.distance_19_5'),
            bottom: $r('app.float.distance_9_5')
          })
        Blank()

        // bluetooth discovering
        if (this.isDeviceDiscovering) {
          DiscoveringAnimatorComponent()
        }
      }
      .width(ConfigData.WH_100_100)

      Row() {
        // available devices title
        Text('配对状态：' + this.pairDeviceInfo)
          .fontSize($r('app.float.font_14'))
          .fontColor($r('app.color.font_color_182431'))
          .width(ConfigData.WH_100_100)
          .padding({
            left: $r('app.float.distance_24'),
            top: $r('app.float.distance_19_5'),
            bottom: $r('app.float.distance_9_5')
          })
      }
      .width(ConfigData.WH_100_100)

      if (this.availableDevices && this.availableDevices.length >= 1) {
        List() {
          // paired devices list
          ForEach(this.availableDevices, (item: BluetoothDevice) => {
            ListItem() {
              Row() {
                EntryComponent({
                  settingIcon: getDeviceIconPath(item.deviceType),
                  settingTitle: item.deviceName ? item.deviceName : item.deviceId,
                  settingSummary: getConnectionStateText(item),
                  settingValue: '',
                  settingArrow: '',
                  settingArrowStyle: '',
                  settingUri: '',
                  image_wh: $r('app.float.wh_value_24'),
                  height: getConnectionStateText(item) == '' ? $r('app.float.wh_value_56') :  ($r('app.float.wh_value_64')),
                  fontSize: ($r('app.float.font_16')),
                });
              }
              .width(ConfigData.WH_100_100)
              .borderRadius($r("app.float.radius_24"))
              .padding($r('app.float.distance_4'))
              .backgroundColor($r("app.color.white_bg_color"))
              .onClick(() => {
                LogUtil.info(this.TAG_PAGE + 'item on click : ' + JSON.stringify(item));
                this.pairDevice(item)
              })
            }
          }, item => JSON.stringify(item));
        }
        .backgroundColor($r("app.color.white_bg_color"))
        .borderRadius($r("app.float.radius_24"))
        .divider({
          strokeWidth: 1,
          color: $r('app.color.color_E3E3E3_grey'),
          startMargin: $r('app.float.wh_value_52'),
          endMargin: $r('app.float.wh_value_12')
        })

      } else {
        Row() {
          // Scanning...
          Text($r('app.string.scanning'))
            .fontSize($r('app.float.font_20'))
            .textCase(TextCase.UpperCase);
        }
      }
    }
  }

  /**
   * Pair device
   * @param device
   */
  pairDevice(device: BluetoothDevice) {
    this.controller.pair(device.deviceId, (pinCode: string) => {
      LogUtil.info(this.TAG_PAGE + 'pairDevice success callback : pinCode = ' + pinCode);
      // show pair pin dialog
      this.pairPinCode = pinCode;
      this.pairingDevice = device;
      AppStorage.SetOrCreate('pairedDeviceInfo', device);
      AppStorage.SetOrCreate('pairedDeviceId', device.deviceId);
      this.pairDialog.open();
      this.pairDeviceInfo = 'pairDeviceID: ' + device.deviceId  + ' true';
    }, () => {
      this.pairDeviceInfo = 'pairDeviceID: ' + device.deviceId  + ' false';
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
 * Get device icon resource
 * @param type
 * @return device icon path
 */
function getDeviceIconPath(deviceType: string): string {
  let path: string = "/res/image/ic_bluetooth_device.svg";
  switch (deviceType) {
    case DeviceType.HEADPHONE:
      path = "/res/image/ic_device_earphone_hero.svg";
      break;

    case DeviceType.PHONE:
      path = "/res/image/ic_public_devices_phone.svg";
      break;

    case DeviceType.WATCH:
      path = "/res/image/ic_device_watch.svg";
      break;

    case DeviceType.COMPUTER:
      path = "/res/image/ic_device_matebook.svg";
      break;
  }
  return path
}

/**
 * Get connection state text
 * @param connectionState
 */
function getConnectionStateText(device: BluetoothDevice): string | Resource {
  let stateText: string | Resource = '';
  switch (device.connectionState) {
    case DeviceState.STATE_DISCONNECTED:
      stateText = '';
      break;

    case DeviceState.STATE_CONNECTING:
      stateText = $r('app.string.bluetooth_state_connecting');
      break;

    case DeviceState.STATE_CONNECTED:
      if (device.deviceType === DeviceType.HEADPHONE) {
        stateText = $r('app.string.bluetooth_state_connected');
      } else {
        stateText = '';
      }
      break;

    case DeviceState.STATE_DISCONNECTING:
      stateText = $r('app.string.bluetooth_state_disconnecting');
      break;

    case DeviceState.STATE_PAIRING:
      stateText = $r('app.string.bluetooth_state_pairing');
      break;

  }
  return stateText;
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