var _2a33209e6af7922da1521a8a828bfbd0;
/******/ (() => { // webpackBootstrap
/******/ 	"use strict";
/******/ 	var __webpack_modules__ = ({

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/pages/subManualApiTest/subGattTest/gattClient.ets?entry":
/*!************************************************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/pages/subManualApiTest/subGattTest/gattClient.ets?entry ***!
  \************************************************************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {


var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
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
/**
 * Gatt Client Test Page Of Bluetooth test
 */
const titleComponent_1 = __webpack_require__(/*! ../../../../Component/titleComponent */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Component/titleComponent.ets");
__webpack_require__(/*! ../../../../component/headcomponent */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/headcomponent.ets");
const BluetoothModel_1 = __importDefault(__webpack_require__(/*! ../../../model/BluetoothModel */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/BluetoothModel.ts"));
__webpack_require__(/*! @system.router */ "../../api/@system.router.d.ts");
__webpack_require__(/*! ../../../controller/BluetoothDeviceController */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/controller/BluetoothDeviceController.ts");
const ConfigData_1 = __importDefault(__webpack_require__(/*! ../../../../Utils/ConfigData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Utils/ConfigData.ts"));
__webpack_require__(/*! ../../../../Utils/LogUtil */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Utils/LogUtil.ts");
var _ohos_bluetooth_1  = globalThis.requireNapi('bluetooth') || (isSystemplugin('bluetooth', 'ohos') ? globalThis.ohosplugin.bluetooth : isSystemplugin('bluetooth', 'system') ? globalThis.systemplugin.bluetooth : undefined);
const pageTitle_1 = __webpack_require__(/*! ../../../../Component/pageTitle */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Component/pageTitle.ets");
const NO_GATT_CLIENT_OBJECT = "No GattClient object, please CreateGattClient firstly!";
class GattClient extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.__message = new ObservedPropertySimple('GattClient', this, "message");
        this.__currentClick = new ObservedPropertySimple(-1, this, "currentClick");
        this.__btOnBLEConnectionStateChange = new ObservedPropertySimple('on:BLEConnectionStateChange', this, "btOnBLEConnectionStateChange");
        this.__isOnBLEConnectionStateChangeClick = new ObservedPropertySimple(false, this, "isOnBLEConnectionStateChangeClick");
        this.__bleConnectionStateInfo = new ObservedPropertySimple("Disconnected", this, "bleConnectionStateInfo");
        this.__bleConnectionState = new ObservedPropertySimple(0, this, "bleConnectionState");
        this.__btOnBLECharacteristicChange = new ObservedPropertySimple('on:BLECharacteristicChange', this, "btOnBLECharacteristicChange");
        this.__isOnBLECharacteristicChangeClick = new ObservedPropertySimple(false, this, "isOnBLECharacteristicChangeClick");
        this.__bleCharacteristicChangeInfo = new ObservedPropertySimple('', this, "bleCharacteristicChangeInfo");
        this.__serviceUUID = new ObservedPropertySimple('00001877-0000-1000-8000-00805F9B34FB', this, "serviceUUID");
        this.__characteristicUUID = new ObservedPropertySimple('00002BE0-0000-1000-8000-00805F9B34FB', this, "characteristicUUID");
        this.__characteristicValue = new ObservedPropertySimple('CccValue', this, "characteristicValue");
        this.__descriptorUUID = new ObservedPropertySimple('00002902-0000-1000-8000-00805F9B34FB', this, "descriptorUUID");
        this.__descriptorValue = new ObservedPropertySimple('DesValue', this, "descriptorValue");
        this.__mtuSize = new ObservedPropertySimple(128, this, "mtuSize");
        this.gattClientInstance = null;
        this.peripheralDeviceId = '88:36:CF:09:C1:90';
        this.gattClientExist = false;
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.message !== undefined) {
            this.message = params.message;
        }
        if (params.currentClick !== undefined) {
            this.currentClick = params.currentClick;
        }
        if (params.btOnBLEConnectionStateChange !== undefined) {
            this.btOnBLEConnectionStateChange = params.btOnBLEConnectionStateChange;
        }
        if (params.isOnBLEConnectionStateChangeClick !== undefined) {
            this.isOnBLEConnectionStateChangeClick = params.isOnBLEConnectionStateChangeClick;
        }
        if (params.bleConnectionStateInfo !== undefined) {
            this.bleConnectionStateInfo = params.bleConnectionStateInfo;
        }
        if (params.bleConnectionState !== undefined) {
            this.bleConnectionState = params.bleConnectionState;
        }
        if (params.btOnBLECharacteristicChange !== undefined) {
            this.btOnBLECharacteristicChange = params.btOnBLECharacteristicChange;
        }
        if (params.isOnBLECharacteristicChangeClick !== undefined) {
            this.isOnBLECharacteristicChangeClick = params.isOnBLECharacteristicChangeClick;
        }
        if (params.bleCharacteristicChangeInfo !== undefined) {
            this.bleCharacteristicChangeInfo = params.bleCharacteristicChangeInfo;
        }
        if (params.serviceUUID !== undefined) {
            this.serviceUUID = params.serviceUUID;
        }
        if (params.characteristicUUID !== undefined) {
            this.characteristicUUID = params.characteristicUUID;
        }
        if (params.characteristicValue !== undefined) {
            this.characteristicValue = params.characteristicValue;
        }
        if (params.descriptorUUID !== undefined) {
            this.descriptorUUID = params.descriptorUUID;
        }
        if (params.descriptorValue !== undefined) {
            this.descriptorValue = params.descriptorValue;
        }
        if (params.mtuSize !== undefined) {
            this.mtuSize = params.mtuSize;
        }
        if (params.gattClientInstance !== undefined) {
            this.gattClientInstance = params.gattClientInstance;
        }
        if (params.peripheralDeviceId !== undefined) {
            this.peripheralDeviceId = params.peripheralDeviceId;
        }
        if (params.gattClientExist !== undefined) {
            this.gattClientExist = params.gattClientExist;
        }
    }
    aboutToBeDeleted() {
        this.__message.aboutToBeDeleted();
        this.__currentClick.aboutToBeDeleted();
        this.__btOnBLEConnectionStateChange.aboutToBeDeleted();
        this.__isOnBLEConnectionStateChangeClick.aboutToBeDeleted();
        this.__bleConnectionStateInfo.aboutToBeDeleted();
        this.__bleConnectionState.aboutToBeDeleted();
        this.__btOnBLECharacteristicChange.aboutToBeDeleted();
        this.__isOnBLECharacteristicChangeClick.aboutToBeDeleted();
        this.__bleCharacteristicChangeInfo.aboutToBeDeleted();
        this.__serviceUUID.aboutToBeDeleted();
        this.__characteristicUUID.aboutToBeDeleted();
        this.__characteristicValue.aboutToBeDeleted();
        this.__descriptorUUID.aboutToBeDeleted();
        this.__descriptorValue.aboutToBeDeleted();
        this.__mtuSize.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id());
    }
    get message() {
        return this.__message.get();
    }
    set message(newValue) {
        this.__message.set(newValue);
    }
    get currentClick() {
        return this.__currentClick.get();
    }
    set currentClick(newValue) {
        this.__currentClick.set(newValue);
    }
    get btOnBLEConnectionStateChange() {
        return this.__btOnBLEConnectionStateChange.get();
    }
    set btOnBLEConnectionStateChange(newValue) {
        this.__btOnBLEConnectionStateChange.set(newValue);
    }
    get isOnBLEConnectionStateChangeClick() {
        return this.__isOnBLEConnectionStateChangeClick.get();
    }
    set isOnBLEConnectionStateChangeClick(newValue) {
        this.__isOnBLEConnectionStateChangeClick.set(newValue);
    }
    get bleConnectionStateInfo() {
        return this.__bleConnectionStateInfo.get();
    }
    set bleConnectionStateInfo(newValue) {
        this.__bleConnectionStateInfo.set(newValue);
    }
    get bleConnectionState() {
        return this.__bleConnectionState.get();
    }
    set bleConnectionState(newValue) {
        this.__bleConnectionState.set(newValue);
    }
    get btOnBLECharacteristicChange() {
        return this.__btOnBLECharacteristicChange.get();
    }
    set btOnBLECharacteristicChange(newValue) {
        this.__btOnBLECharacteristicChange.set(newValue);
    }
    get isOnBLECharacteristicChangeClick() {
        return this.__isOnBLECharacteristicChangeClick.get();
    }
    set isOnBLECharacteristicChangeClick(newValue) {
        this.__isOnBLECharacteristicChangeClick.set(newValue);
    }
    get bleCharacteristicChangeInfo() {
        return this.__bleCharacteristicChangeInfo.get();
    }
    set bleCharacteristicChangeInfo(newValue) {
        this.__bleCharacteristicChangeInfo.set(newValue);
    }
    get serviceUUID() {
        return this.__serviceUUID.get();
    }
    set serviceUUID(newValue) {
        this.__serviceUUID.set(newValue);
    }
    get characteristicUUID() {
        return this.__characteristicUUID.get();
    }
    set characteristicUUID(newValue) {
        this.__characteristicUUID.set(newValue);
    }
    get characteristicValue() {
        return this.__characteristicValue.get();
    }
    set characteristicValue(newValue) {
        this.__characteristicValue.set(newValue);
    }
    get descriptorUUID() {
        return this.__descriptorUUID.get();
    }
    set descriptorUUID(newValue) {
        this.__descriptorUUID.set(newValue);
    }
    get descriptorValue() {
        return this.__descriptorValue.get();
    }
    set descriptorValue(newValue) {
        this.__descriptorValue.set(newValue);
    }
    get mtuSize() {
        return this.__mtuSize.get();
    }
    set mtuSize(newValue) {
        this.__mtuSize.set(newValue);
    }
    render() {
        Column.create();
        Column.backgroundColor({ "id": 125829132, "type": 10001, params: [] });
        Column.height('100%');
        let earlierCreatedChild_2 = this.findChildById("2");
        if (earlierCreatedChild_2 == undefined) {
            View.create(new pageTitle_1.PageTitle("2", this, { detail: 'Gatt_Client测试' }));
        }
        else {
            earlierCreatedChild_2.updateWithValueParams({
                detail: 'Gatt_Client测试'
            });
            if (!earlierCreatedChild_2.needsUpdate()) {
                earlierCreatedChild_2.markStatic();
            }
            View.create(earlierCreatedChild_2);
        }
        //          HeadComponent({ headName: 'GattClient Test', isActive: true });
        Stack.create();
        //          HeadComponent({ headName: 'GattClient Test', isActive: true });
        Stack.height("0.5vp");
        //          HeadComponent({ headName: 'GattClient Test', isActive: true });
        Stack.backgroundColor("#000000");
        //          HeadComponent({ headName: 'GattClient Test', isActive: true });
        Stack.pop();
        Row.create();
        Row.backgroundColor({ "id": 16777297, "type": 10001, params: [] });
        Row.padding(5);
        Row.justifyContent(FlexAlign.Start);
        Row.alignItems(VerticalAlign.Center);
        Text.create("peripheral deviceId:");
        Text.fontSize(14);
        Text.width(60);
        Text.pop();
        TextInput.create({ text: this.peripheralDeviceId, placeholder: "input peripheral deviceId." });
        TextInput.fontSize("10vp");
        TextInput.onChange((strInput) => {
            this.peripheralDeviceId = strInput;
        });
        TextInput.width("80%");
        TextInput.borderRadius(1);
        Row.pop();
        //        .margin( { top: "1vp", left: "20vp", right: "20vp" } )
        Column.create();
        //        .margin( { top: "1vp", left: "20vp", right: "20vp" } )
        Column.visibility(Visibility.Visible);
        Stack.create();
        Stack.height("0.25vp");
        Stack.backgroundColor("#000000");
        Stack.pop();
        Column.create();
        Row.create();
        Row.padding(5);
        Row.justifyContent(FlexAlign.Start);
        Row.alignItems(VerticalAlign.Center);
        Row.backgroundColor({ "id": 16777302, "type": 10001, params: [] });
        Text.create("Service UUID:");
        Text.fontSize(12);
        Text.width(60);
        Text.pop();
        TextInput.create({ text: this.serviceUUID, placeholder: "input Service UUID" });
        TextInput.fontSize("10vp");
        TextInput.onChange((strInput) => {
            this.serviceUUID = strInput;
        });
        TextInput.width("80%");
        TextInput.borderRadius(1);
        Row.pop();
        //            .margin( { top: "1vp", left: "20vp", right: "20vp" } )     
        Row.create();
        //            .margin( { top: "1vp", left: "20vp", right: "20vp" } )     
        Row.backgroundColor({ "id": 16777302, "type": 10001, params: [] });
        //            .margin( { top: "1vp", left: "20vp", right: "20vp" } )     
        Row.padding(5);
        //            .margin( { top: "1vp", left: "20vp", right: "20vp" } )     
        Row.justifyContent(FlexAlign.Start);
        //            .margin( { top: "1vp", left: "20vp", right: "20vp" } )     
        Row.alignItems(VerticalAlign.Center);
        Text.create("Characteristic UUID:");
        Text.fontSize(11);
        Text.width(60);
        Text.pop();
        TextInput.create({ text: this.characteristicUUID, placeholder: "input Characteristic UUID" });
        TextInput.fontSize("10vp");
        TextInput.onChange((strInput) => {
            this.characteristicUUID = strInput;
        });
        TextInput.width("80%");
        TextInput.borderRadius(1);
        //            .margin( { top: "1vp", left: "20vp", right: "20vp" } )     
        Row.pop();
        //            .margin( { top: "1vp", left: "20vp", right: "20vp" } )
        Row.create();
        //            .margin( { top: "1vp", left: "20vp", right: "20vp" } )
        Row.backgroundColor({ "id": 16777302, "type": 10001, params: [] });
        //            .margin( { top: "1vp", left: "20vp", right: "20vp" } )
        Row.padding(5);
        //            .margin( { top: "1vp", left: "20vp", right: "20vp" } )
        Row.justifyContent(FlexAlign.Start);
        //            .margin( { top: "1vp", left: "20vp", right: "20vp" } )
        Row.alignItems(VerticalAlign.Center);
        Text.create("Characteristic Value:");
        Text.fontSize(12);
        Text.width(60);
        Text.pop();
        TextInput.create({ text: this.characteristicUUID, placeholder: "Characteristic Value" });
        TextInput.fontSize("10vp");
        TextInput.onChange((strInput) => {
            this.characteristicValue = strInput;
        });
        TextInput.width("80%");
        TextInput.borderRadius(1);
        //            .margin( { top: "1vp", left: "20vp", right: "20vp" } )
        Row.pop();
        //            .margin( { top: "1vp", left: "20vp", right: "20vp" } )
        Row.create();
        //            .margin( { top: "1vp", left: "20vp", right: "20vp" } )
        Row.backgroundColor({ "id": 16777302, "type": 10001, params: [] });
        //            .margin( { top: "1vp", left: "20vp", right: "20vp" } )
        Row.padding(5);
        //            .margin( { top: "1vp", left: "20vp", right: "20vp" } )
        Row.justifyContent(FlexAlign.Start);
        //            .margin( { top: "1vp", left: "20vp", right: "20vp" } )
        Row.alignItems(VerticalAlign.Center);
        Text.create("Descriptor UUID:");
        Text.fontSize(12);
        Text.width(60);
        Text.pop();
        TextInput.create({ text: this.descriptorUUID, placeholder: "input descriptor UUID" });
        TextInput.fontSize("10vp");
        TextInput.onChange((strInput) => {
            this.descriptorUUID = strInput;
        });
        TextInput.width("80%");
        TextInput.borderRadius(1);
        //            .margin( { top: "1vp", left: "20vp", right: "20vp" } )
        Row.pop();
        //            .margin( { top: "1vp", left: "20vp", right: "20vp" } )
        Row.create();
        //            .margin( { top: "1vp", left: "20vp", right: "20vp" } )
        Row.backgroundColor({ "id": 16777302, "type": 10001, params: [] });
        //            .margin( { top: "1vp", left: "20vp", right: "20vp" } )
        Row.padding(5);
        //            .margin( { top: "1vp", left: "20vp", right: "20vp" } )
        Row.justifyContent(FlexAlign.Start);
        //            .margin( { top: "1vp", left: "20vp", right: "20vp" } )
        Row.alignItems(VerticalAlign.Center);
        Text.create("Descriptor Value:");
        Text.fontSize(12);
        Text.width(60);
        Text.pop();
        TextInput.create({ text: this.descriptorUUID, placeholder: "descriptor Value" });
        TextInput.fontSize("10vp");
        TextInput.onChange((strInput) => {
            this.descriptorValue = strInput;
        });
        TextInput.width("80%");
        TextInput.borderRadius(1);
        //            .margin( { top: "1vp", left: "20vp", right: "20vp" } )
        Row.pop();
        Column.pop();
        Stack.create();
        Stack.height("0.25vp");
        Stack.backgroundColor("#000000");
        Stack.pop();
        Column.create();
        Column.height("12%");
        Text.create("result:" + this.message);
        Text.fontSize("15vp");
        Text.margin({ top: "5vp", left: "20vp", right: "20vp" });
        Text.textAlign(TextAlign.Start);
        Text.backgroundColor({ "id": 125829123, "type": 10001, params: [] });
        Text.pop();
        Text.create("bleConnectionStateChange:" + this.bleConnectionStateInfo);
        Text.fontSize("15vp");
        Text.margin({ top: "5vp", left: "20vp", right: "20vp" });
        Text.textAlign(TextAlign.Start);
        Text.backgroundColor({ "id": 125829123, "type": 10001, params: [] });
        Text.pop();
        Text.create("BLECharacteristicChange:" + this.bleCharacteristicChangeInfo);
        Text.fontSize("15vp");
        Text.margin({ top: "5vp", left: "20vp", right: "20vp" });
        Text.textAlign(TextAlign.Start);
        Text.backgroundColor({ "id": 125829123, "type": 10001, params: [] });
        Text.pop();
        Column.pop();
        Stack.create();
        Stack.height("0.25vp");
        Stack.backgroundColor("#000000");
        Stack.pop();
        Text.create("Click your wanted method below to verify:");
        Text.margin({ top: "20vp" });
        Text.padding("5vp");
        Text.fontSize("15vp");
        Text.fontWeight(FontWeight.Bold);
        Text.fontColor("#4444444");
        Text.backgroundColor({ "id": 16777297, "type": 10001, params: [] });
        Text.pop();
        Scroll.create();
        Scroll.width(ConfigData_1.default.WH_100_100);
        Scroll.align(Alignment.TopStart);
        Scroll.layoutWeight(1);
        Scroll.margin({ top: { "id": 125829737, "type": 10002, params: [] } });
        Scroll.height("40%");
        List.create({ space: 1 });
        List.margin({ bottom: "150vp" });
        ListItem.create();
        ListItem.padding({ top: 10 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            this.currentClick = 99;
            let btState = _ohos_bluetooth_1.getState();
            if (btState === _ohos_bluetooth_1.BluetoothState.STATE_ON) {
                if (this.gattClientExist) {
                    this.message = "GattClient exists!";
                    return;
                }
                this.message = "CreateGattClient: turning on BLEDeviceFind";
                _ohos_bluetooth_1.BLE.on("BLEDeviceFind", (scanResult) => {
                    scanResult.forEach(device => {
                        var devId = device.deviceId; //88:36:CF:09:C1:90
                        var rssi = device.rssi;
                        var data = device.data;
                        _ohos_bluetooth_1.BLE.stopBLEScan();
                        // create ble client instance
                        if (!this.gattClientExist) {
                            this.gattClientInstance = BluetoothModel_1.default.createGattClient(devId);
                            this.message = "createGattClient():succeeds ";
                            this.gattClientExist = true;
                        }
                        if (devId == this.peripheralDeviceId) {
                            _ohos_bluetooth_1.BLE.stopBLEScan();
                            // create ble client instance
                            if (!this.gattClientExist) {
                                this.gattClientInstance = BluetoothModel_1.default.createGattClient(devId);
                                this.message = "createGattClient():succeeds ";
                                this.gattClientExist = true;
                            }
                        }
                    });
                });
                if (this.peripheralDeviceId.length > 15) {
                    this.message = "startBLEScan with filter,peripheralDeviceId=" + this.peripheralDeviceId;
                    let bleScanFilter = {
                        deviceId: this.peripheralDeviceId
                        //serviceUuid:'000054F6-0000-1000-8000-00805f9b34fb'
                    };
                    let bleScanFilters = [];
                    bleScanFilters.push(bleScanFilter);
                    let bleScanOptions = {
                        interval: 0,
                        dutyMode: _ohos_bluetooth_1.ScanDuty.SCAN_MODE_BALANCED,
                        matchMode: _ohos_bluetooth_1.MatchMode.MATCH_MODE_AGGRESSIVE
                    };
                    _ohos_bluetooth_1.BLE.startBLEScan(bleScanFilters, bleScanOptions);
                }
                else {
                    this.message = "Be sure the correct MacId!";
                }
            }
            else if (btState === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
                this.message = "BT is not enabled!";
            }
            else {
                this.message = "switching,please operate it later!";
            }
        });
        let earlierCreatedChild_3 = this.findChildById("3");
        if (earlierCreatedChild_3 == undefined) {
            View.create(new titleComponent_1.TitleComponent("3", this, {
                title: "CreateGattClient", fontSize: '17vp',
                bgColor: this.currentClick === 99 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_3.updateWithValueParams({
                title: "CreateGattClient", fontSize: '17vp',
                bgColor: this.currentClick === 99 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_3);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 20 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            let btState = _ohos_bluetooth_1.getState();
            if (btState === _ohos_bluetooth_1.BluetoothState.STATE_ON) {
                if (this.gattClientExist) {
                    this.currentClick = 0;
                    if (this.bleConnectionState == 0) {
                        let rv = this.gattClientInstance.connect();
                        this.message = "GattClient.connect(): " + rv;
                    }
                    else {
                        this.message = "this device is not disconnected!";
                        return;
                    }
                }
                else {
                    this.message = NO_GATT_CLIENT_OBJECT;
                }
            }
            else if (btState === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
                this.message = "BT is not enabled!";
            }
            else {
                this.message = "switching,please operate it later!";
            }
        });
        let earlierCreatedChild_4 = this.findChildById("4");
        if (earlierCreatedChild_4 == undefined) {
            View.create(new titleComponent_1.TitleComponent("4", this, {
                title: "connect", fontSize: '17vp',
                bgColor: this.currentClick === 0 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_4.updateWithValueParams({
                title: "connect", fontSize: '17vp',
                bgColor: this.currentClick === 0 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_4);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 20 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            this.message = "GattClient.disconnect() result";
            let btState = _ohos_bluetooth_1.getState();
            if (btState == _ohos_bluetooth_1.BluetoothState.STATE_ON) {
                if (!this.gattClientExist) {
                    this.message = NO_GATT_CLIENT_OBJECT;
                    return;
                }
                this.currentClick = 1;
                if (this.bleConnectionState === 2) {
                    let rv = this.gattClientInstance.disconnect();
                    this.message = "GattClient.disconnect(): " + rv;
                }
                else {
                    this.message = " BLE is not connected and no need to disconnect it ";
                }
            }
            else if (btState == _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
                this.message = "BT is not enabled!";
            }
            else {
                this.message = "switching,please operate it later!";
            }
        });
        let earlierCreatedChild_5 = this.findChildById("5");
        if (earlierCreatedChild_5 == undefined) {
            View.create(new titleComponent_1.TitleComponent("5", this, {
                title: "disconnect", fontSize: '17vp',
                bgColor: this.currentClick === 1 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_5.updateWithValueParams({
                title: "disconnect", fontSize: '17vp',
                bgColor: this.currentClick === 1 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_5);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 20 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            let btStatus = _ohos_bluetooth_1.getState();
            if (btStatus === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
                this.message = "BT is not enabled!";
                return;
            }
            if (!this.gattClientExist) {
                this.message = NO_GATT_CLIENT_OBJECT;
                return;
            }
            if (this.isOnBLEConnectionStateChangeClick) {
                this.gattClientInstance.off('BLEConnectionStateChange', () => {
                });
                this.message = 'BLEConnectionStateChange is off(turn it on by re-clicking)';
                this.isOnBLEConnectionStateChangeClick = false;
                this.btOnBLEConnectionStateChange = 'on:BLEConnectionStateChange';
                // AppStorage.SetOrCreate('on_BLEConnectionStateChange', this.btOnBLEConnectionStateChange);
                return;
            }
            this.isOnBLEConnectionStateChangeClick = true;
            this.currentClick = 2;
            this.btOnBLEConnectionStateChange = 'off:BLEConnectionStateChange';
            // AppStorage.SetOrCreate('on_BLEConnectionStateChange', this.btOnBLEConnectionStateChange);
            this.gattClientInstance.on('BLEConnectionStateChange', (data) => {
                this.bleConnectionState = data.state;
                // AppStorage.SetOrCreate('on_BLEConnectionStateChange', this.btOnBLEConnectionStateChange);
                switch (this.bleConnectionState) {
                    case _ohos_bluetooth_1.ProfileConnectionState.STATE_DISCONNECTED:
                        this.bleConnectionStateInfo = "DISCONNECTED";
                        break;
                    case _ohos_bluetooth_1.ProfileConnectionState.STATE_CONNECTING:
                        this.bleConnectionStateInfo = "CONNECTING";
                        break;
                    case _ohos_bluetooth_1.ProfileConnectionState.STATE_CONNECTED:
                        this.bleConnectionStateInfo = "STATE_CONNECTED";
                        break;
                    case _ohos_bluetooth_1.ProfileConnectionState.STATE_DISCONNECTING:
                        this.bleConnectionStateInfo = "STATE_DISCONNECTING";
                        break;
                    default:
                        this.bleConnectionStateInfo = "undefined";
                }
            });
            this.message = 'BLEConnectionStateChange is on';
        });
        let earlierCreatedChild_6 = this.findChildById("6");
        if (earlierCreatedChild_6 == undefined) {
            View.create(new titleComponent_1.TitleComponent("6", this, {
                title: this.btOnBLEConnectionStateChange, fontSize: '17vp',
                bgColor: this.currentClick === 2 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_6.updateWithValueParams({
                title: this.btOnBLEConnectionStateChange, fontSize: '17vp',
                bgColor: this.currentClick === 2 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_6);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 20 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            this.message = "close():";
            let btState = _ohos_bluetooth_1.getState();
            if (btState == _ohos_bluetooth_1.BluetoothState.STATE_ON) {
                if (!this.gattClientExist) {
                    this.message = NO_GATT_CLIENT_OBJECT;
                    return;
                }
                this.currentClick = 3;
                let rv = this.gattClientInstance.close();
                if (rv) {
                    this.gattClientExist = false;
                }
                this.message = "GattClient.close():" + rv;
            }
            else if (btState == _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
                this.message = "BT is not enabled!";
            }
            else {
                this.message = "switching,please operate it later!";
            }
        });
        let earlierCreatedChild_7 = this.findChildById("7");
        if (earlierCreatedChild_7 == undefined) {
            View.create(new titleComponent_1.TitleComponent("7", this, {
                title: "close", fontSize: '17vp',
                bgColor: this.currentClick === 3 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_7.updateWithValueParams({
                title: "close", fontSize: '17vp',
                bgColor: this.currentClick === 3 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_7);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 20 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            let btState = _ohos_bluetooth_1.getState();
            if (btState == _ohos_bluetooth_1.BluetoothState.STATE_ON) {
                if (!this.gattClientExist) {
                    this.message = NO_GATT_CLIENT_OBJECT;
                    return;
                }
                this.currentClick = 4;
                this.gattClientInstance.getServices((code, gattServices) => {
                    if (code.code != 0) {
                        this.message = "getServices(call back) error code:" + code.code;
                        return;
                    }
                    else {
                        this.message = "getServices(Callback):invoke succeeds! \n ";
                        this.message = "bluetooth services size is " + gattServices.length + '\n';
                        for (let i = 0; i < gattServices.length; i++) {
                            this.message += 'bluetooth serviceUuid is ' + gattServices[i].serviceUuid + '\n';
                        }
                    }
                });
            }
            else if (btState == _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
                this.message = "BT is not enabled!";
            }
            else {
                this.message = "switching,please operate it later!";
            }
        });
        let earlierCreatedChild_8 = this.findChildById("8");
        if (earlierCreatedChild_8 == undefined) {
            View.create(new titleComponent_1.TitleComponent("8", this, {
                title: "getServices", fontSize: '17vp',
                bgColor: this.currentClick === 4 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_8.updateWithValueParams({
                title: "getServices", fontSize: '17vp',
                bgColor: this.currentClick === 4 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_8);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 20 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            if (!this.gattClientExist) {
                this.message = NO_GATT_CLIENT_OBJECT;
                return;
            }
            this.currentClick = 5;
            this.message = "getServices promise:starts";
            let btState = _ohos_bluetooth_1.getState();
            if (btState == _ohos_bluetooth_1.BluetoothState.STATE_ON) {
                this.gattClientInstance.getServices().then(gattServices => {
                    this.message = "getServicesPromise successfully:\n";
                    this.message = "bluetooth services size is " + gattServices.length;
                    for (let i = 0; i < gattServices.length; i++) {
                        this.message += 'bluetooth serviceUuid is ' + gattServices[i].serviceUuid + '\n';
                    }
                });
                this.message = "getServices(promise):invoke succeeds!";
            }
            else if (btState == _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
                this.message = "BT is not enabled!";
            }
            else {
                this.message = "switching,please operate it later!";
            }
        });
        let earlierCreatedChild_9 = this.findChildById("9");
        if (earlierCreatedChild_9 == undefined) {
            View.create(new titleComponent_1.TitleComponent("9", this, {
                title: "getServices", fontSize: '17vp',
                bgColor: this.currentClick === 5 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_9.updateWithValueParams({
                title: "getServices", fontSize: '17vp',
                bgColor: this.currentClick === 5 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_9);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 20 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            let btState = _ohos_bluetooth_1.getState();
            if (btState === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
                this.message = "BT is not enabled!";
                return;
            }
            if (!this.gattClientExist) {
                this.message = NO_GATT_CLIENT_OBJECT;
                return;
            }
            this.currentClick = 6;
            let bleCharacteristicDataIn = {
                serviceUuid: this.serviceUUID,
                characteristicUuid: this.characteristicUUID
            };
            this.gattClientInstance.readCharacteristicValue(bleCharacteristicDataIn, (err, bleCharacteristicDataOut) => {
                if (err.code != 0) {
                    this.message = "readCharacteristicValue error code:" + err.code;
                    return;
                }
                this.message = "readCharacteristicValue callback: \n";
                this.message += 'bluetooth characteristic uuid:' + bleCharacteristicDataOut.characteristicUuid + "\n";
                let value = new Uint8Array(bleCharacteristicDataOut.characteristicValue);
                for (let i = 0; i < bleCharacteristicDataOut.characteristicValue.byteLength; i++) {
                    this.message += 'bluetooth characteristic value: ' + value[i];
                }
            });
            this.message = 'readCharacteristicValue(callback):invoke succeeds!';
        });
        let earlierCreatedChild_10 = this.findChildById("10");
        if (earlierCreatedChild_10 == undefined) {
            View.create(new titleComponent_1.TitleComponent("10", this, {
                title: "readCharacteristicValue", fontSize: '17vp',
                bgColor: this.currentClick === 6 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_10.updateWithValueParams({
                title: "readCharacteristicValue", fontSize: '17vp',
                bgColor: this.currentClick === 6 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_10);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 20 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            let btState = _ohos_bluetooth_1.getState();
            if (btState === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
                this.message = "BT is not enabled!";
                return;
            }
            if (!this.gattClientExist) {
                this.message = NO_GATT_CLIENT_OBJECT;
                return;
            }
            this.currentClick = 7;
            let bleCharacteristicDataIn = {
                serviceUuid: this.serviceUUID,
                characteristicUuid: this.characteristicUUID
            };
            this.gattClientInstance.readCharacteristicValue(bleCharacteristicDataIn).then((bleCharacteristicDataOut) => {
                this.message = "readCharacteristicValue promise:\n";
                this.message += 'bluetooth characteristic uuid:' + bleCharacteristicDataOut.characteristicUuid + "\n";
                let value = new Uint8Array(bleCharacteristicDataOut.characteristicValue);
                for (let i = 0; i < bleCharacteristicDataOut.characteristicValue.byteLength; i++) {
                    this.message += 'bluetooth characteristic value: ' + value[i];
                }
            });
            this.message = 'readCharacteristicValue(promise):invoke succeeds!';
        });
        let earlierCreatedChild_11 = this.findChildById("11");
        if (earlierCreatedChild_11 == undefined) {
            View.create(new titleComponent_1.TitleComponent("11", this, {
                title: "readCharacteristicValue", fontSize: '17vp',
                bgColor: this.currentClick === 7 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_11.updateWithValueParams({
                title: "readCharacteristicValue", fontSize: '17vp',
                bgColor: this.currentClick === 7 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_11);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 20 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            let btState = _ohos_bluetooth_1.getState();
            if (btState === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
                this.message = "BT is not enabled!";
                return;
            }
            if (!this.gattClientExist) {
                this.message = NO_GATT_CLIENT_OBJECT;
                return;
            }
            this.currentClick = 8;
            let bufferDesc = new ArrayBuffer(8);
            let descV = new Uint8Array(bufferDesc);
            descV[0] = 88;
            let descriptorIn = {
                serviceUuid: this.serviceUUID,
                characteristicUuid: this.characteristicUUID,
                descriptorUuid: this.descriptorUUID, descriptorValue: bufferDesc
            };
            this.gattClientInstance.readDescriptorValue(descriptorIn, (err, descriptorOut) => {
                if (err.code != 0) {
                    return;
                }
                this.message = 'readDescriptorValue(callback):descriptor uuid=' + descriptorOut.descriptorUuid + "\n";
                let value = new Uint8Array(descriptorOut.descriptorValue);
                for (let i = 0; i < descriptorOut.descriptorValue.byteLength; i++) {
                    this.message += 'descriptor value: ' + value[i] + "\n";
                }
            });
            this.message = 'readDescriptorValue(callback):invoke succeeds!';
        });
        let earlierCreatedChild_12 = this.findChildById("12");
        if (earlierCreatedChild_12 == undefined) {
            View.create(new titleComponent_1.TitleComponent("12", this, {
                title: "readDescriptorValue", fontSize: '17vp',
                bgColor: this.currentClick === 8 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_12.updateWithValueParams({
                title: "readDescriptorValue", fontSize: '17vp',
                bgColor: this.currentClick === 8 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_12);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 20 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            let btState = _ohos_bluetooth_1.getState();
            if (btState === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
                this.message = "BT is not enabled!";
                return;
            }
            if (!this.gattClientExist) {
                this.message = NO_GATT_CLIENT_OBJECT;
                return;
            }
            this.currentClick = 9;
            let bufferDesc = new ArrayBuffer(8);
            let descV = new Uint8Array(bufferDesc);
            descV[0] = 99;
            let descriptorIn = {
                serviceUuid: this.serviceUUID,
                characteristicUuid: this.characteristicUUID,
                descriptorUuid: this.descriptorUUID, descriptorValue: bufferDesc
            };
            this.gattClientInstance.readDescriptorValue(descriptorIn).then((descriptorOut) => {
                this.message = 'readDescriptorValue(promise):descriptor uuid=' + descriptorOut.descriptorUuid + "\n";
                let value = new Uint8Array(descriptorOut.descriptorValue);
                for (let i = 0; i < descriptorOut.descriptorValue.byteLength; i++) {
                    this.message += 'descriptor value: ' + value[i] + "\n";
                }
            });
            this.message = 'readDescriptorValue(promise):invoke succeeds!';
        });
        let earlierCreatedChild_13 = this.findChildById("13");
        if (earlierCreatedChild_13 == undefined) {
            View.create(new titleComponent_1.TitleComponent("13", this, {
                title: "readDescriptorValue", fontSize: '17vp',
                bgColor: this.currentClick === 9 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_13.updateWithValueParams({
                title: "readDescriptorValue", fontSize: '17vp',
                bgColor: this.currentClick === 9 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_13);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 20 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            let btState = _ohos_bluetooth_1.getState();
            if (btState === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
                this.message = "BT is not enabled!";
                return;
            }
            if (!this.gattClientExist) {
                this.message = NO_GATT_CLIENT_OBJECT;
                return;
            }
            this.currentClick = 10;
            let bufferCCC = string2ArrayBuffer(this.characteristicValue);
            let characteristic = { serviceUuid: this.serviceUUID,
                characteristicUuid: this.characteristicUUID,
                characteristicValue: bufferCCC
            };
            if (this.gattClientInstance.writeCharacteristicValue(characteristic)) {
                this.message = 'writeCharacteristicValue succeeds.';
            }
            else {
                this.message = 'writeCharacteristicValue failed!';
            }
        });
        let earlierCreatedChild_14 = this.findChildById("14");
        if (earlierCreatedChild_14 == undefined) {
            View.create(new titleComponent_1.TitleComponent("14", this, {
                title: "writeCharacteristicValue", fontSize: '17vp',
                bgColor: this.currentClick === 10 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_14.updateWithValueParams({
                title: "writeCharacteristicValue", fontSize: '17vp',
                bgColor: this.currentClick === 10 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_14);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 20 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            let btState = _ohos_bluetooth_1.getState();
            if (btState === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
                this.message = "BT is not enabled!";
                return;
            }
            if (!this.gattClientExist) {
                this.message = NO_GATT_CLIENT_OBJECT;
                return;
            }
            this.currentClick = 11;
            let bufferDesc = string2ArrayBuffer(this.descriptorValue);
            let descriptor = { serviceUuid: this.serviceUUID,
                characteristicUuid: this.characteristicUUID,
                descriptorUuid: this.descriptorUUID,
                descriptorValue: bufferDesc };
            if (this.gattClientInstance.writeDescriptorValue(descriptor)) {
                this.message = 'writeDescriptorValue succeeds.';
            }
            else {
                this.message = 'writeDescriptorValue,failed!';
            }
        });
        let earlierCreatedChild_15 = this.findChildById("15");
        if (earlierCreatedChild_15 == undefined) {
            View.create(new titleComponent_1.TitleComponent("15", this, {
                title: "writeDescriptorValue", fontSize: '17vp',
                bgColor: this.currentClick === 11 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_15.updateWithValueParams({
                title: "writeDescriptorValue", fontSize: '17vp',
                bgColor: this.currentClick === 11 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_15);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 20 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            let btStatus = _ohos_bluetooth_1.getState();
            if (btStatus === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
                this.message = "BT is not enabled!";
                return;
            }
            if (!this.gattClientExist) {
                this.message = NO_GATT_CLIENT_OBJECT;
                return;
            }
            this.currentClick = 12;
            if (this.isOnBLECharacteristicChangeClick) {
                this.gattClientInstance.off('BLECharacteristicChange', () => {
                });
                this.message = 'BLECharacteristicChange is off, re-clicking for turning it on!';
                this.isOnBLECharacteristicChangeClick = false;
                this.btOnBLECharacteristicChange = 'on:BLECharacteristicChange';
                // AppStorage.SetOrCreate('on_BLEConnectionStateChange', this.btOnBLEConnectionStateChange);
                return;
            }
            this.isOnBLECharacteristicChangeClick = true;
            this.btOnBLECharacteristicChange = 'off:BLECharacteristicChange';
            this.gattClientInstance.on('BLECharacteristicChange', (data) => {
                this.btOnBLECharacteristicChange = 'off:BLECharacteristicChange';
                this.bleCharacteristicChangeInfo = 'BLECharacteristicChange:\n';
                let serviceUuid = data.serviceUuid;
                let characteristicUuid = data.characteristicUuid;
                this.bleCharacteristicChangeInfo += "serviceUuid:" + serviceUuid + "\n";
                this.bleCharacteristicChangeInfo += "characteristicUuid:" + characteristicUuid + "\n";
                let value = new Uint8Array(data.characteristicValue);
                for (let i = 0; i < data.characteristicValue.byteLength; i++) {
                    this.bleCharacteristicChangeInfo += "\n" + 'value:' + value[i];
                }
            });
            this.message = 'BLECharacteristicChange is on(setNotifyCharacteristicChanged firstly)';
        });
        let earlierCreatedChild_16 = this.findChildById("16");
        if (earlierCreatedChild_16 == undefined) {
            View.create(new titleComponent_1.TitleComponent("16", this, {
                title: this.btOnBLECharacteristicChange, fontSize: '17vp',
                bgColor: this.currentClick === 12 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_16.updateWithValueParams({
                title: this.btOnBLECharacteristicChange, fontSize: '17vp',
                bgColor: this.currentClick === 12 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_16);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 20 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            let btStatus = _ohos_bluetooth_1.getState();
            if (btStatus === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
                this.message = "BT is not enabled!";
                return;
            }
            if (!this.gattClientExist) {
                this.message = NO_GATT_CLIENT_OBJECT;
                return;
            }
            this.currentClick = 13;
            this.message = 'setBLEMtuSize: \n';
            this.mtuSize = 256;
            let rv = this.gattClientInstance.setBLEMtuSize(this.mtuSize);
            this.message += "result = " + rv;
        });
        let earlierCreatedChild_17 = this.findChildById("17");
        if (earlierCreatedChild_17 == undefined) {
            View.create(new titleComponent_1.TitleComponent("17", this, {
                title: "setBLEMtuSize", fontSize: '17vp',
                bgColor: this.currentClick === 13 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_17.updateWithValueParams({
                title: "setBLEMtuSize", fontSize: '17vp',
                bgColor: this.currentClick === 13 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_17);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 20 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            let btStatus = _ohos_bluetooth_1.getState();
            if (btStatus === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
                this.message = "BT is not enabled!";
                return;
            }
            if (!this.gattClientExist) {
                this.message = NO_GATT_CLIENT_OBJECT;
                return;
            }
            this.currentClick = 14;
            let descriptors = [];
            let bufferCCC = new ArrayBuffer(8);
            let cccV = new Uint8Array(bufferCCC);
            cccV[0] = 66;
            let descriptor = {
                serviceUuid: this.serviceUUID,
                characteristicUuid: this.characteristicUUID,
                descriptorUuid: this.descriptorUUID,
                descriptorValue: string2ArrayBuffer(this.descriptorValue)
            };
            descriptors.push(descriptor);
            let characteristicIn = {
                serviceUuid: this.serviceUUID,
                characteristicUuid: this.characteristicUUID,
                characteristicValue: bufferCCC,
                descriptors: descriptors
            };
            if (this.gattClientInstance.setNotifyCharacteristicChanged(characteristicIn, true)) {
                this.message = "setNotifyCharacteristicChanged(true):succeeds.";
            }
            else {
                this.message = "setNotifyCharacteristicChanged(true):failed!";
                if (this.gattClientInstance.setNotifyCharacteristicChanged(characteristicIn, false)) {
                    this.message = "setNotifyCharacteristicChanged(false):true!";
                }
                else {
                    this.message = "setNotifyCharacteristicChanged(false):false!";
                }
            }
        });
        let earlierCreatedChild_18 = this.findChildById("18");
        if (earlierCreatedChild_18 == undefined) {
            View.create(new titleComponent_1.TitleComponent("18", this, {
                title: "setNotifyCharacteristicChanged", fontSize: '17vp',
                bgColor: this.currentClick === 14 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_18.updateWithValueParams({
                title: "setNotifyCharacteristicChanged", fontSize: '17vp',
                bgColor: this.currentClick === 14 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_18);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 20 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            if (!this.gattClientExist) {
                this.message = NO_GATT_CLIENT_OBJECT;
                return;
            }
            this.currentClick = 15;
            this.gattClientInstance.getDeviceName((err, asyncRemoteDeviceName) => {
                this.message = "return code:";
                this.message += JSON.stringify(err);
                this.message += ",and DeviceName is: ";
                this.message += JSON.stringify(asyncRemoteDeviceName);
            });
        });
        let earlierCreatedChild_19 = this.findChildById("19");
        if (earlierCreatedChild_19 == undefined) {
            View.create(new titleComponent_1.TitleComponent("19", this, {
                title: "getDeviceName", fontSize: '17vp',
                bgColor: this.currentClick === 15 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_19.updateWithValueParams({
                title: "getDeviceName", fontSize: '17vp',
                bgColor: this.currentClick === 15 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_19);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 20 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            if (!this.gattClientExist) {
                this.message = NO_GATT_CLIENT_OBJECT;
                return;
            }
            this.currentClick = 16;
            let rv = this.gattClientInstance.getDeviceName().then((peripheralDeviceName) => {
                this.message = "getDeviceName()Promise:" + peripheralDeviceName;
            });
        });
        let earlierCreatedChild_20 = this.findChildById("20");
        if (earlierCreatedChild_20 == undefined) {
            View.create(new titleComponent_1.TitleComponent("20", this, {
                title: "getDeviceName", fontSize: '17vp',
                bgColor: this.currentClick === 16 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_20.updateWithValueParams({
                title: "getDeviceName", fontSize: '17vp',
                bgColor: this.currentClick === 16 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_20);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 20 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            if (!this.gattClientExist) {
                this.message = NO_GATT_CLIENT_OBJECT;
                return;
            }
            this.currentClick = 17;
            this.message = "getRssiValue callback:\n";
            this.gattClientInstance.getRssiValue((err, cbRssi) => {
                this.message = "return code:";
                this.message += JSON.stringify(err);
                this.message += ",and RSSI=";
                this.message += JSON.stringify(cbRssi);
            });
        });
        let earlierCreatedChild_21 = this.findChildById("21");
        if (earlierCreatedChild_21 == undefined) {
            View.create(new titleComponent_1.TitleComponent("21", this, {
                title: "getRssiValue", fontSize: '17vp',
                bgColor: this.currentClick === 17 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_21.updateWithValueParams({
                title: "getRssiValue", fontSize: '17vp',
                bgColor: this.currentClick === 17 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_21);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 20 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            this.currentClick = 18;
            if (!this.gattClientExist) {
                this.message = NO_GATT_CLIENT_OBJECT;
                return;
            }
            this.message = "getRssiValue promise:\n";
            let rssi = this.gattClientInstance.getRssiValue().then((rssiValue) => {
                this.message += 'rssi' + JSON.stringify(rssiValue);
            });
        });
        let earlierCreatedChild_22 = this.findChildById("22");
        if (earlierCreatedChild_22 == undefined) {
            View.create(new titleComponent_1.TitleComponent("22", this, {
                title: "getRssiValue", fontSize: '17vp',
                bgColor: this.currentClick === 18 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_22.updateWithValueParams({
                title: "getRssiValue", fontSize: '17vp',
                bgColor: this.currentClick === 18 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_22);
        }
        ListItem.pop();
        List.pop();
        Scroll.pop();
        //        .margin( { top: "1vp", left: "20vp", right: "20vp" } )
        Column.pop();
        Column.pop();
    }
}
/**
* string to ArrayBuffer
* @param {*} str, the string object to
* @return:  ArrayBuffer
*/
function string2ArrayBuffer(str) {
    let array = new Uint8Array(str.length);
    for (var i = 0; i < str.length; i++) {
        array[i] = str.charCodeAt(i);
    }
    return array.buffer;
}
// example: sleep(500).then(()=>{/*TODO after sleeping*/});
function sleep(time) {
    return new Promise((resolve) => setTimeout(resolve, time));
}
loadDocument(new GattClient("1", undefined, {}));


/***/ })

/******/ 	});
/************************************************************************/
/******/ 	// The module cache
/******/ 	var __webpack_module_cache__ = {};
/******/ 	
/******/ 	// The require function
/******/ 	function __webpack_require__(moduleId) {
/******/ 		var commonCachedModule = globalThis["__common_module_cache___2a33209e6af7922da1521a8a828bfbd0"] ? globalThis["__common_module_cache___2a33209e6af7922da1521a8a828bfbd0"][moduleId]: null;
/******/ 		if (commonCachedModule) { return commonCachedModule.exports; }
/******/ 		// Check if module is in cache
/******/ 		var cachedModule = __webpack_module_cache__[moduleId];
/******/ 		if (cachedModule !== undefined) {
/******/ 			return cachedModule.exports;
/******/ 		}
/******/ 		// Create a new module (and put it into the cache)
/******/ 		var module = __webpack_module_cache__[moduleId] = {
/******/ 			// no module.id needed
/******/ 			// no module.loaded needed
/******/ 			exports: {}
/******/ 		};
/******/ 	
/******/ 		if (globalThis["__common_module_cache___2a33209e6af7922da1521a8a828bfbd0"] && moduleId.indexOf("?name=") < 0 && Object.keys(globalThis["__common_module_cache___2a33209e6af7922da1521a8a828bfbd0"]).indexOf(moduleId) >= 0) {
/******/ 		  globalThis["__common_module_cache___2a33209e6af7922da1521a8a828bfbd0"][moduleId] = module;
/******/ 		}
/******/ 		__webpack_modules__[moduleId].call(module.exports, module, module.exports, __webpack_require__);
/******/ 	
/******/ 		// Return the exports of the module
/******/ 		return module.exports;
/******/ 	}
/******/ 	
/******/ 	// expose the modules object (__webpack_modules__)
/******/ 	__webpack_require__.m = __webpack_modules__;
/******/ 	
/************************************************************************/
/******/ 	/* webpack/runtime/chunk loaded */
/******/ 	(() => {
/******/ 		var deferred = [];
/******/ 		__webpack_require__.O = (result, chunkIds, fn, priority) => {
/******/ 			if(chunkIds) {
/******/ 				priority = priority || 0;
/******/ 				for(var i = deferred.length; i > 0 && deferred[i - 1][2] > priority; i--) deferred[i] = deferred[i - 1];
/******/ 				deferred[i] = [chunkIds, fn, priority];
/******/ 				return;
/******/ 			}
/******/ 			var notFulfilled = Infinity;
/******/ 			for (var i = 0; i < deferred.length; i++) {
/******/ 				var [chunkIds, fn, priority] = deferred[i];
/******/ 				var fulfilled = true;
/******/ 				for (var j = 0; j < chunkIds.length; j++) {
/******/ 					if ((priority & 1 === 0 || notFulfilled >= priority) && Object.keys(__webpack_require__.O).every((key) => (__webpack_require__.O[key](chunkIds[j])))) {
/******/ 						chunkIds.splice(j--, 1);
/******/ 					} else {
/******/ 						fulfilled = false;
/******/ 						if(priority < notFulfilled) notFulfilled = priority;
/******/ 					}
/******/ 				}
/******/ 				if(fulfilled) {
/******/ 					deferred.splice(i--, 1)
/******/ 					var r = fn();
/******/ 					if (r !== undefined) result = r;
/******/ 				}
/******/ 			}
/******/ 			return result;
/******/ 		};
/******/ 	})();
/******/ 	
/******/ 	/* webpack/runtime/hasOwnProperty shorthand */
/******/ 	(() => {
/******/ 		__webpack_require__.o = (obj, prop) => (Object.prototype.hasOwnProperty.call(obj, prop))
/******/ 	})();
/******/ 	
/******/ 	/* webpack/runtime/jsonp chunk loading */
/******/ 	(() => {
/******/ 		// no baseURI
/******/ 		
/******/ 		// object to store loaded and loading chunks
/******/ 		// undefined = chunk not loaded, null = chunk preloaded/prefetched
/******/ 		// [resolve, reject, Promise] = chunk loading, 0 = chunk loaded
/******/ 		var installedChunks = {
/******/ 			"./pages/subManualApiTest/subGattTest/gattClient": 0
/******/ 		};
/******/ 		
/******/ 		// no chunk on demand loading
/******/ 		
/******/ 		// no prefetching
/******/ 		
/******/ 		// no preloaded
/******/ 		
/******/ 		// no HMR
/******/ 		
/******/ 		// no HMR manifest
/******/ 		
/******/ 		__webpack_require__.O.j = (chunkId) => (installedChunks[chunkId] === 0);
/******/ 		
/******/ 		// install a JSONP callback for chunk loading
/******/ 		var webpackJsonpCallback = (parentChunkLoadingFunction, data) => {
/******/ 			var [chunkIds, moreModules, runtime] = data;
/******/ 			// add "moreModules" to the modules object,
/******/ 			// then flag all "chunkIds" as loaded and fire callback
/******/ 			var moduleId, chunkId, i = 0;
/******/ 			if(chunkIds.some((id) => (installedChunks[id] !== 0))) {
/******/ 				for(moduleId in moreModules) {
/******/ 					if(__webpack_require__.o(moreModules, moduleId)) {
/******/ 						__webpack_require__.m[moduleId] = moreModules[moduleId];
/******/ 					}
/******/ 				}
/******/ 				if(runtime) var result = runtime(__webpack_require__);
/******/ 			}
/******/ 			if(parentChunkLoadingFunction) parentChunkLoadingFunction(data);
/******/ 			for(;i < chunkIds.length; i++) {
/******/ 				chunkId = chunkIds[i];
/******/ 				if(__webpack_require__.o(installedChunks, chunkId) && installedChunks[chunkId]) {
/******/ 					installedChunks[chunkId][0]();
/******/ 				}
/******/ 				installedChunks[chunkId] = 0;
/******/ 			}
/******/ 			return __webpack_require__.O(result);
/******/ 		}
/******/ 		
/******/ 		var chunkLoadingGlobal = globalThis["webpackChunk_2a33209e6af7922da1521a8a828bfbd0"] = globalThis["webpackChunk_2a33209e6af7922da1521a8a828bfbd0"] || [];
/******/ 		chunkLoadingGlobal.forEach(webpackJsonpCallback.bind(null, 0));
/******/ 		chunkLoadingGlobal.push = webpackJsonpCallback.bind(null, chunkLoadingGlobal.push.bind(chunkLoadingGlobal));
/******/ 	})();
/******/ 	
/************************************************************************/
/******/ 	
/******/ 	// startup
/******/ 	// Load entry module and return exports
/******/ 	// This entry module depends on other loaded chunks and execution need to be delayed
/******/ 	var __webpack_exports__ = __webpack_require__.O(undefined, ["commons"], () => (__webpack_require__("../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/pages/subManualApiTest/subGattTest/gattClient.ets?entry")))
/******/ 	__webpack_exports__ = __webpack_require__.O(__webpack_exports__);
/******/ 	_2a33209e6af7922da1521a8a828bfbd0 = __webpack_exports__;
/******/ 	
/******/ })()
;
//# sourceMappingURL=gattClient.js.map