/**
 * Copyright (c) 2022 Shenzhen Kaihong Digital Industry Development Co., Ltd.
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

import prompt from '@system.prompt';
import router from '@system.router';
import { PageTitle } from '../../Component/pageTitle';
import { TestImageDisplay } from '../../Component/testImageDisplay';
import { TestData } from '../../MainAbility/model/testData'
import { ScenarioContentTable } from '../../Component/scenarioContentTable'
import {
  BleFoundManagerScenarioTestCase ,
  initBleFoundManagerScenarioData
} from '../../MainAbility/model/scenarioTestDataModelManager'
import * as BLEManagerInterface from '../../MainAbility/model/bleManagerInterface'
import * as BrManagerInterface from '../../MainAbility/model/brManagerInterface'

/**
 * BLE found Scenario Manager Test Page Of Bluetooth test
 */

@Entry
@Component
struct BleFoundScenarioTestManager {
  @State message: string = 'BleFoundScenarioTestManager';
  private testItem: TestData = router.getParams().testId;
  @State changeIndex: number = - 1
  @StorageLink( "bleFoundScenarioMessage" ) bleFoundScenarioMessage: string = ""

  aboutToAppear() {
    AppStorage.SetOrCreate( 'txtScanFilterDeviceId' , "10:2C:6B:11:B3:17" ); // 6c:96:d7:3d:87:6f
    AppStorage.SetOrCreate( 'txtScanFilterName' , "dudu-tiger" );
    AppStorage.SetOrCreate( 'txtScanFilterServiceUuid' , "0000180A-0000-1000-8000-00805f9b34fb" );
    AppStorage.SetOrCreate( 'txtScanOptions_interval' , "0" );
    AppStorage.SetOrCreate( 'txtScanOptionsDutyMode' , '0' );
    AppStorage.SetOrCreate( 'txtScanOptionsMatchMode' , '0' );
    AppStorage.SetOrCreate( 'cbxBleScanFilter' , true );
    AppStorage.SetOrCreate( 'cbxBleScanOptions' , true );
  }

  build() {
    Column() {
      Stack( { alignContent : Alignment.TopStart } ) {
        TestImageDisplay( { testItem : this.testItem } )
        PageTitle( { testItem : this.testItem } )
      }

      Stack().height( "1vp" ).backgroundColor( "#000000" );
      Column() {
        Scroll() {
          Column() {
            Text( "场景流程验证:" )
              .fontSize( "17vp" )
              .margin( { top : "10vp" , bottom : "10vp" , left : "20vp" } )
              .textAlign( TextAlign.Start )
              .width( "100%" )
            ScenarioContentTable( {
              testItem : this.testItem ,
              scenarioItems : initBleFoundManagerScenarioData() ,
              //            testMessage: this.testMessage,  优化时修改
              changeIndex : this.changeIndex
            } )
          }
          .width( "100%" )
          .height( 280 )
        }
        .scrollBarWidth( 10 )
        .scrollBar( BarState.Auto )

        Stack().height( "1vp" ).backgroundColor( "#000000" );

        Scroll() {
          Column() {
            Text( "流程日志:" )
              .fontSize( "17vp" )
              .margin( { top : "8vp" , bottom : "8vp" , left : "20vp" } )
              .textAlign( TextAlign.Start )
              .width( "100%" )
            List() {
              ListItem() {
                Text( this.bleFoundScenarioMessage )
                  .fontSize( "17vp" )
                  .margin( { top : "5vp" , left : "30vp" , right : "10vp" } )
                  .textAlign( TextAlign.Start )
                  .width( "90%" )
              }
            }
            .height( "80%" )
            .width( "90%" )
            .backgroundColor( $r( "sys.color.ohos_id_color_foreground_contrary" ) )
          }
          .width( "100%" )
          .height( 260 )
        }
        .scrollBarWidth( 10 )
        .scrollBar( BarState.On )
      }
      .height( 500 )
      .width( "100%" )
      .backgroundColor( $r( "sys.color.ohos_id_color_sub_background" ) )


      Stack().height( "1vp" ).backgroundColor( "#000000" );
      Column() {
        Flex( { alignItems : ItemAlign.Center , justifyContent : FlexAlign.SpaceBetween } ) {
          Button( { type : ButtonType.Normal , stateEffect : true } ) {
            Text( '场景测试开始' ).fontSize( "24vp" ).fontColor( 0xffffff ).margin( { left : 5 , right : 5 } )
          }
          .borderRadius( 8 )
          .backgroundColor( $r( "app.color.blue" ) )
          .width( 100 )
          .height( 66 )
          .margin( { top : "15vp" , left : "20vp" } )
          .align( Alignment.Start )
          .onClick( async( event: ClickEvent ) => {
            prompt.showToast( { message : '开始BleFound场景测试' } )
            let self = this
            //            const arr = [0, 1, 2, 3, 4, 5, 6, 7]
            let bleFoundScenarioMessage = ""
            //拼装数组
            var bleScenarioArray = [ { name : "testSetLocalNameManager" , para : [ "DaYuBlue" ] },
                                     { name : "testSetBluetoothScanModeManager" , para : [ 4, 0 ] } ]
            let Interface = BleFoundManagerScenarioTestCase.map( i => {
              let para = bleScenarioArray.some( itm => itm.name === i.api.name ) ? bleScenarioArray.find( itm => itm.name === i.api.name ).para : []
              return { name : i.api.name , para }
            } )

            //循环调用
            async function forLoop() {
              for ( let m = 0 ; m < Interface.length ; m ++ ) {
                bleFoundScenarioMessage += BleFoundManagerScenarioTestCase[m]["api"]( ... Interface[m].para ) + "\n"
                self.changeIndex = m
                await sleep( 1 )
                AppStorage.SetOrCreate( "bleFoundScenarioMessage" , bleFoundScenarioMessage )
              }
            }

            forLoop()

          } )

          Button( { type : ButtonType.Normal , stateEffect : true } ) {
            Row() {
              Text( '场景测试结束' ).fontSize( "24vp" ).fontColor( 0xffffff ).margin( { left : 5 , right : 5 } )
            }.alignItems( VerticalAlign.Start )
          }
          .borderRadius( 8 )
          .backgroundColor( $r( "app.color.blue" ) )
          .width( 100 )
          .height( 66 )
          .margin( { top : "15vp" , left : "10vp" , right : "10vp" } )
          .align( Alignment.Start )
          .onClick( async( event: ClickEvent ) => {
            let self = this
            const arr = [ 0, 1, 2, 3, 4, 5, 6, 7 ]
            let bleFoundScenarioMessage = ""
            // 停止BLE扫描
            bleFoundScenarioMessage += BLEManagerInterface.testStopBLEScanManager() + "\n"
            console.log( "testStopBLEScan success" )
            //            self.changeIndex = arr[6]
            await sleep( 1 )
            // 关闭注册BLE查找状态变化
            bleFoundScenarioMessage += BLEManagerInterface.testOnBLEDeviceFindManager() + "\n"
            console.log( "testOffBLEDeviceFind success" )
            await sleep( 1 )
            //关闭注册状态变化
            bleFoundScenarioMessage += BrManagerInterface.testOnStateChangeManager() + "\n"
            console.log( "testOffStateChange success" )
            await sleep( 1 )
            //关蓝牙
            bleFoundScenarioMessage += BrManagerInterface.testDisableBluetoothManager() + "\n"
            console.log( "testDisableBluetooth success" )
            //            self.changeIndex = arr[7]
            await sleep( 1 )
            AppStorage.SetOrCreate( "bleFoundScenarioMessage" , bleFoundScenarioMessage )
          } )

          Button( { type : ButtonType.Normal , stateEffect : true } ) {
            Row() {
              Text( '清空日志' ).fontSize( "24vp" ).fontColor( 0xffffff ).margin( { left : 5 , right : 5 } )
            }.alignItems( VerticalAlign.Start )
          }
          .borderRadius( 8 )
          .backgroundColor( $r( "app.color.moon" ) )
          .width( 80 )
          .height( 66 )
          .margin( { top : "15vp" , right : "20vp" } )
          .align( Alignment.Start )
          .onClick( async( event: ClickEvent ) => {
            let bleFoundScenarioMessage = ""
            AppStorage.SetOrCreate( "bleFoundScenarioMessage" , bleFoundScenarioMessage )
          } )
        }
        .backgroundColor( $r( "sys.color.ohos_id_color_sub_background" ) )
        .width( "100%" )
      }
      .backgroundColor( $r( "sys.color.ohos_id_color_sub_background" ) )
      .width( "100%" )
      .height( "100%" );
    }
  }
}

function sleep( time ) {
  return new Promise(( resolve , reject ) => {
    setTimeout( () => {
      resolve()
    } , time * 1000 )
  })
}