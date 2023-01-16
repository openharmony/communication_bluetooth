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
import ConfigData from '../../../../Utils/ConfigData';
import HeadComponent from '../../../../Component/headComponent';
import BluetoothModel from '../../../model/BluetoothModel';
import Router from '@system.router'
import { TitleComponent } from '../../../../Component/titleComponent';
import InputMethod from '@ohos.inputMethod';
import bluetooth from '@ohos.bluetooth';
/**
 * about phone
 */
@Entry
@Component
struct setScanMode {
  @State message: string = "";
  private duration: string = '0';
  private deviceInfo: any = '';
  private aboutDeviceList: any[] = [];
  private scanMode: number = 0;
  @State currentClick: number = -1;

  onPageShow() : void {
    this.currentClick = bluetooth.getBluetoothScanMode();
  }

  build() {
    Column(){
      GridContainer({columns:12, sizeType: SizeType.Auto, gutter: vp2px(1) === 2 ? '12vp' : '0vp', margin: vp2px(1) === 2 ? '24vp' : '0vp'}) {
        Row({}) {
          Column() {
          }
          .width(ConfigData.WH_100_100)
          .height(ConfigData.WH_100_100)
          .useSizeType({
            xs: { span: 0, offset: 0 }, sm: { span: 0, offset: 0 },
            md: { span: 0, offset: 0 }, lg: { span: 2, offset: 0 }
          });

          Column() {
            HeadComponent({ headName: $r('app.string.bluetooth_scan_mode_type'), isActive: true })

            Divider()
              .padding({left: $r('sys.float.ohos_id_card_margin_start'), right: $r('sys.float.ohos_id_card_margin_end')})

            Column(){

              List() {
                ListItem() {
                  TitleComponent({
                    title: "SCAN_MODE_SELECT",
                    bgColor: $r('app.color.66ccff')});
                }.padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
                .onClick(() => {
                })
                ListItem() {
                  TitleComponent({
                    title: "SCAN_MODE_NONE",
                    fontSize: '15vp',
                    bgColor: this.currentClick === 0 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')});
                }.padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
                .onClick(() => {
                  this.currentClick = 0;
                })
                ListItem() {
                  TitleComponent({
                    title: "CONNECTABLE",
                    fontSize: '15vp',
                    bgColor: this.currentClick === 1 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')});
                }.padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
                .onClick(() => {
                  this.currentClick = 1;
                })
                ListItem() {
                  TitleComponent({
                    title: "GENERAL_DISCOVERABLE",
                    fontSize: '15vp',
                    bgColor: this.currentClick === 2 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')});
                }.padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
                .onClick(() => {
                  this.currentClick = 2;
                })
                ListItem() {
                  TitleComponent({
                    title: "LIMITED_DISCOVERABLE",
                    fontSize: '15vp',
                    bgColor: this.currentClick === 3 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')});
                }.padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
                .onClick(() => {
                  this.currentClick = 3;
                })
                ListItem() {
                  TitleComponent({
                    title: "CONNECTABLE_GENERAL_DISCOVERABLE",
                    fontSize: '14vp',
                    bgColor: this.currentClick === 4 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')});
                }.padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
                .onClick(() => {
                  this.currentClick = 4;
                })
                ListItem() {
                  TitleComponent({
                    title: "CONNECTABLE_LIMITED_DISCOVERABLE",
                    fontSize: '15vp',
                    bgColor: this.currentClick === 5 ? $r('app.color.font_color_007DFF') : $r('app.color.white_bg_color')});
                }.padding({ top: $r('app.float.distance_2'), bottom: $r('app.float.distance_2') })
                .onClick(() => {
                  this.currentClick = 5;
                })
              }
              Blank()
            }

            Row() {
              Text('duration')
                .textAlign(TextAlign.Center)
                .fontSize('20fp')
                .fontColor($r('app.color.font_color_182431'))
                .fontWeight(FontWeight.Medium)
              TextInput({ text: this.duration})
                .placeholderFont({size: $r("app.float.font_18"), weight:FontWeight.Normal, style: FontStyle.Normal})
                .type(InputType.Normal)
                .enterKeyType(EnterKeyType.Done)
                .caretColor(Color.Blue)
                .height($r("app.float.wh_value_40"))
                .borderRadius(0)
                .backgroundColor($r('app.color.color_00000000_transparent'))
                .onChange((value: string) => {
                  console.info(ConfigData.TAG + 'device name changed to: ' + JSON.stringify(value));
                  this.duration = value;
                })
                .onSubmit((enterKey) => {
                  InputMethod.getInputMethodController().stopInput().then((ret) => {
                    console.info( `${ConfigData.TAG}, enterType: ${enterKey}, stopInput: ${ret}`);
                  });
                })
            }
            .padding($r('app.float.distance_15'))
            .width('100%')

            Text(this.message)
              .textAlign(TextAlign.Start)
              .fontSize('20fp')
              .backgroundColor($r("app.color.66ccff"))

            Flex({direction: FlexDirection.Row, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }){
              Row(){
                ButtonComponent({text: $r("app.string.cancel"), onClick: () => {
                  Router.back();
                }})
              }
              .width($r("app.float.deviceName_button_width"))
              .margin({right: $r("app.float.distance_12")})
              Row(){
                ButtonComponent({text: $r("app.string.confirm"), onClick: () => {
                  if(this.currentClick === -1) {
                    this.message = '请点击选择SCAN MODE';
                    return;
                  }
                  this.message = '';
                  let dur = parseInt(this.duration);
                  let parseRet = isNaN(dur);
                  let ret = BluetoothModel.setBluetoothScanMode(this.currentClick, !parseRet ? dur : 0);
                  AppStorage.SetOrCreate('setScanModeRet', ret);
                  Router.back();
                }})
              }.width($r("app.float.deviceName_button_width"))
            }
          }
          .padding({left: $r('app.float.distance_24'), right: $r('app.float.distance_24')})
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


@Component
struct ButtonComponent {
  @State isTouched: boolean = false;
  private text;
  private onClick = () => {};

  build() {
    Column() {
      Text(this.text)
        .fontSize($r('app.float.font_16'))
        .fontWeight(FontWeight.Medium)
        .lineHeight($r('app.float.wh_value_22'))
        .fontColor($r('app.color.font_color_007DFF'))
        .textAlign(TextAlign.Center)
        .width($r('app.float.deviceName_button_width'))
        .height($r("app.float.wh_value_40"))
        .borderRadius($r('app.float.radius_20'))
        .backgroundColor(!this.isTouched ? $r("app.color.color_button_grey") : $r("sys.color.ohos_id_color_foreground_contrary"))
        .onTouch((event: TouchEvent) => {
          if (event.type === TouchType.Down) {
            this.isTouched = true;
          }
          if (event.type === TouchType.Up) {
            this.isTouched = false;
          }
        })
        .onClick(() => {
          this.onClick();
        })
    }
    .width(ConfigData.WH_100_100)
    .align(Alignment.Center);
  }
}
