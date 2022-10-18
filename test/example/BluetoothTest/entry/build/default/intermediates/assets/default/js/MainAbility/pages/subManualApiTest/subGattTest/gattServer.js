var _2a33209e6af7922da1521a8a828bfbd0;
/******/ (() => { // webpackBootstrap
/******/ 	"use strict";
/******/ 	var __webpack_modules__ = ({

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/pages/subManualApiTest/subGattTest/gattServer.ets?entry":
/*!************************************************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/pages/subManualApiTest/subGattTest/gattServer.ets?entry ***!
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
 * Gatt Server Test Page Of Bluetooth test
 */
const titleComponent_1 = __webpack_require__(/*! ../../../../Component/titleComponent */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Component/titleComponent.ets");
__webpack_require__(/*! ../../../../component/headcomponent */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/headcomponent.ets");
const BluetoothModel_1 = __importDefault(__webpack_require__(/*! ../../../model/BluetoothModel */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/BluetoothModel.ts"));
__webpack_require__(/*! @system.router */ "../../api/@system.router.d.ts");
__webpack_require__(/*! ../../../model/BluetoothDevice */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/BluetoothDevice.ts");
__webpack_require__(/*! ../../../controller/BluetoothDeviceController */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/controller/BluetoothDeviceController.ts");
const ConfigData_1 = __importDefault(__webpack_require__(/*! ../../../../Utils/ConfigData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Utils/ConfigData.ts"));
__webpack_require__(/*! ../../../../Utils/LogUtil */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Utils/LogUtil.ts");
var _ohos_bluetooth_1  = globalThis.requireNapi('bluetooth') || (isSystemplugin('bluetooth', 'ohos') ? globalThis.ohosplugin.bluetooth : isSystemplugin('bluetooth', 'system') ? globalThis.systemplugin.bluetooth : undefined);
const pageTitle_1 = __webpack_require__(/*! ../../../../Component/pageTitle */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Component/pageTitle.ets");
const NO_GATT_Server_OBJECT = "No GattServer object, please CreateGattServer firstly!";
class GattSetting extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.curServiceUUID = '00001877-0000-1000-8000-00805F9B34FB';
        this.curServiceIsPrimary = true;
        this.curCharacteristicUUID = '00002BE0-0000-1000-8000-00805F9B34FB';
        this.curCharacteristicValue = 'OHOS:BLE NANJING CHINA ';
        this.curDescriptorUUID = '00002902-0000-1000-8000-00805F9B34FB';
        this.curDescriptorValue = 'Descriptor';
        this.cbxAdvSettings = false;
        this.cbxAdvResponse = false;
        this.cbxInterval = false;
        this.advSettings_interval = "1600";
        this.cbxTxPower = false;
        this.advSettings_txPower = "-7";
        this.cbxConnectable = true;
        this.advSettings_connectable = true;
        this.advDataServiceUuids_serviceUuid = "000054F6-0000-1000-8000-00805f9b34fb";
        this.advDataManufactureData_manufactureId = "7456";
        this.advDataManufactureData_manufactureValue = "CHN";
        this.advDataServiceData_serviceUuid = "000054F6-0000-1000-8000-00805f9b34fb";
        this.advDataServiceData_serviceValue = "AdvValue";
        this.advResponseServiceUuids_serviceUuid = "000054F6-0000-1000-8000-00805f9b34fb";
        this.advResponseManufactureData_manufactureId = "6547";
        this.advResponseManufactureData_manufactureValue = "CHN";
        this.advResponseServiceData_serviceUuid = "000054F6-0000-1000-8000-00805f9b34fb";
        this.advResponseServiceData_serviceValue = "AdvValue";
        this.curAdvSettings = {
            interval: 800,
            txPower: -7,
            connectable: true
        };
        this.curAdvData = {
            serviceUuids: ["00003344-0000-1000-8000-00805f9b34fb", "00008899-0000-1000-8000-00805f9b34fb"],
            manufactureData: [{
                    manufactureId: 7456,
                    manufactureValue: string2ArrayBuffer("OHOS CHN")
                }],
            serviceData: [
                {
                    serviceUuid: "000054F6-0000-1000-8000-00805f9b34fb",
                    serviceValue: string2ArrayBuffer("OhosBle!")
                },
                {
                    serviceUuid: "00009999-0000-1000-8000-00805f9b34fb",
                    serviceValue: string2ArrayBuffer("dudu_tiger")
                }
            ]
        };
        this.curAdvResponse = {
            serviceUuids: [
                "00009999-0000-1000-8000-00805f9b34fb"
            ],
            manufactureData: [{
                    manufactureId: 6547,
                    manufactureValue: string2ArrayBuffer("OHOS CHN")
                }],
            serviceData: [{
                    serviceUuid: "000054F6-0000-1000-8000-00805f9b34fb",
                    serviceValue: string2ArrayBuffer("OhosBle!")
                },
                {
                    serviceUuid: "00007456-0000-1000-8000-00805f9b34fb",
                    serviceValue: string2ArrayBuffer("OhosBle")
                }]
        };
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.curServiceUUID !== undefined) {
            this.curServiceUUID = params.curServiceUUID;
        }
        if (params.curServiceIsPrimary !== undefined) {
            this.curServiceIsPrimary = params.curServiceIsPrimary;
        }
        if (params.curCharacteristicUUID !== undefined) {
            this.curCharacteristicUUID = params.curCharacteristicUUID;
        }
        if (params.curCharacteristicValue !== undefined) {
            this.curCharacteristicValue = params.curCharacteristicValue;
        }
        if (params.curDescriptorUUID !== undefined) {
            this.curDescriptorUUID = params.curDescriptorUUID;
        }
        if (params.curDescriptorValue !== undefined) {
            this.curDescriptorValue = params.curDescriptorValue;
        }
        if (params.cbxAdvSettings !== undefined) {
            this.cbxAdvSettings = params.cbxAdvSettings;
        }
        if (params.cbxAdvResponse !== undefined) {
            this.cbxAdvResponse = params.cbxAdvResponse;
        }
        if (params.cbxInterval !== undefined) {
            this.cbxInterval = params.cbxInterval;
        }
        if (params.advSettings_interval !== undefined) {
            this.advSettings_interval = params.advSettings_interval;
        }
        if (params.cbxTxPower !== undefined) {
            this.cbxTxPower = params.cbxTxPower;
        }
        if (params.advSettings_txPower !== undefined) {
            this.advSettings_txPower = params.advSettings_txPower;
        }
        if (params.cbxConnectable !== undefined) {
            this.cbxConnectable = params.cbxConnectable;
        }
        if (params.advSettings_connectable !== undefined) {
            this.advSettings_connectable = params.advSettings_connectable;
        }
        if (params.advDataServiceUuids_serviceUuid !== undefined) {
            this.advDataServiceUuids_serviceUuid = params.advDataServiceUuids_serviceUuid;
        }
        if (params.advDataManufactureData_manufactureId !== undefined) {
            this.advDataManufactureData_manufactureId = params.advDataManufactureData_manufactureId;
        }
        if (params.advDataManufactureData_manufactureValue !== undefined) {
            this.advDataManufactureData_manufactureValue = params.advDataManufactureData_manufactureValue;
        }
        if (params.advDataServiceData_serviceUuid !== undefined) {
            this.advDataServiceData_serviceUuid = params.advDataServiceData_serviceUuid;
        }
        if (params.advDataServiceData_serviceValue !== undefined) {
            this.advDataServiceData_serviceValue = params.advDataServiceData_serviceValue;
        }
        if (params.advResponseServiceUuids_serviceUuid !== undefined) {
            this.advResponseServiceUuids_serviceUuid = params.advResponseServiceUuids_serviceUuid;
        }
        if (params.advResponseManufactureData_manufactureId !== undefined) {
            this.advResponseManufactureData_manufactureId = params.advResponseManufactureData_manufactureId;
        }
        if (params.advResponseManufactureData_manufactureValue !== undefined) {
            this.advResponseManufactureData_manufactureValue = params.advResponseManufactureData_manufactureValue;
        }
        if (params.advResponseServiceData_serviceUuid !== undefined) {
            this.advResponseServiceData_serviceUuid = params.advResponseServiceData_serviceUuid;
        }
        if (params.advResponseServiceData_serviceValue !== undefined) {
            this.advResponseServiceData_serviceValue = params.advResponseServiceData_serviceValue;
        }
        if (params.curAdvSettings !== undefined) {
            this.curAdvSettings = params.curAdvSettings;
        }
        if (params.curAdvData !== undefined) {
            this.curAdvData = params.curAdvData;
        }
        if (params.curAdvResponse !== undefined) {
            this.curAdvResponse = params.curAdvResponse;
        }
    }
    aboutToBeDeleted() {
        SubscriberManager.Get().delete(this.id());
    }
    render() {
        Column.create();
        Text.create("Gatt服务配置:");
        Text.margin({ top: "1vp" });
        Text.padding("1vp");
        Text.fontSize("14vp");
        Text.fontColor("#4444444");
        Text.height("30vp");
        Text.width(200);
        Text.textAlign(TextAlign.Center);
        Text.pop();
        Scroll.create();
        Scroll.height("80vp");
        Column.create();
        Flex.create({ direction: FlexDirection.Row });
        Flex.height(30);
        Flex.width('100%');
        Flex.padding(5);
        Flex.backgroundColor({ "id": 16777297, "type": 10001, params: [] });
        Text.create("ServiceUUID:");
        Text.fontSize(12);
        Text.width(80);
        Text.pop();
        TextInput.create({ text: this.curServiceUUID, placeholder: "input service UUID" });
        TextInput.fontSize("10vp");
        TextInput.onChange((strInput) => {
            this.curServiceUUID = strInput;
        });
        TextInput.width("80%");
        TextInput.borderRadius(1);
        Flex.pop();
        Flex.create({ direction: FlexDirection.Column });
        Flex.height(60);
        Flex.width('100%');
        Flex.padding(5);
        Flex.backgroundColor({ "id": 16777302, "type": 10001, params: [] });
        Row.create();
        Row.padding(5);
        Row.justifyContent(FlexAlign.Start);
        Row.alignItems(VerticalAlign.Center);
        Text.create("CCC UUID:");
        Text.fontSize(12);
        Text.width(60);
        Text.pop();
        TextInput.create({ text: this.curCharacteristicUUID, placeholder: "input characteristic UUID" });
        TextInput.fontSize("10vp");
        TextInput.onChange((strInput) => {
            this.curCharacteristicUUID = strInput;
        });
        TextInput.width("80%");
        TextInput.borderRadius(1);
        Row.pop();
        //.alignItems(VerticalAlign.Center)
        Row.create();
        //.alignItems(VerticalAlign.Center)
        Row.padding(5);
        //.alignItems(VerticalAlign.Center)
        Row.justifyContent(FlexAlign.Start);
        //.alignItems(VerticalAlign.Center)
        Row.alignItems(VerticalAlign.Center);
        Text.create("CCC Value:");
        Text.fontSize(12);
        Text.width(60);
        Text.pop();
        TextInput.create({ text: this.curCharacteristicValue, placeholder: "input characteristic Value" });
        TextInput.fontSize("10vp");
        TextInput.onChange((strInput) => {
            this.curCharacteristicValue = strInput;
        });
        TextInput.width("80%");
        TextInput.borderRadius(1);
        //.alignItems(VerticalAlign.Center)
        Row.pop();
        Flex.pop();
        Flex.create({ direction: FlexDirection.Column });
        Flex.height(60);
        Flex.width('100%');
        Flex.padding(5);
        Flex.backgroundColor({ "id": 16777297, "type": 10001, params: [] });
        Row.create();
        Row.padding(5);
        Row.justifyContent(FlexAlign.Start);
        Row.alignItems(VerticalAlign.Center);
        Text.create("Descriptor UUID:");
        Text.fontSize(12);
        Text.width(60);
        Text.pop();
        TextInput.create({ text: this.curDescriptorUUID, placeholder: "input descriptor UUID" });
        TextInput.fontSize("10vp");
        TextInput.onChange((strInput) => {
            this.curDescriptorUUID = strInput;
        });
        TextInput.width("80%");
        TextInput.borderRadius(1);
        Row.pop();
        Row.create();
        Row.padding(5);
        Row.justifyContent(FlexAlign.Start);
        Row.alignItems(VerticalAlign.Center);
        Text.create("Descriptor Value:");
        Text.fontSize(12);
        Text.width(60);
        Text.pop();
        TextInput.create({ text: this.curDescriptorValue, placeholder: "input descriptor Value" });
        TextInput.fontSize("10vp");
        TextInput.onChange((strInput) => {
            this.curDescriptorValue = strInput;
        });
        TextInput.width("80%");
        TextInput.borderRadius(1);
        Row.pop();
        Flex.pop();
        Column.pop();
        Scroll.pop();
        Stack.create();
        Stack.height("0.25vp");
        Stack.backgroundColor("#000000");
        Stack.pop();
        Text.create("Input advertiser value:");
        Text.margin({ top: "1vp" });
        Text.padding("5vp");
        Text.fontSize("15vp");
        Text.fontColor("#4444444");
        Text.height("30vp");
        Text.width(200);
        Text.pop();
        Stack.create();
        Stack.height("0.25vp");
        Stack.backgroundColor("#000000");
        Stack.pop();
        Scroll.create();
        Scroll.height("160vp");
        Column.create();
        Flex.create({ direction: FlexDirection.Row });
        Flex.height(40);
        Flex.width('100%');
        Flex.padding(5);
        Checkbox.create({ name: 'AdvSettings', group: 'checkboxGroup1' });
        Checkbox.select(this.cbxAdvSettings);
        Checkbox.selectedColor(0x39a2db);
        Checkbox.onChange((value) => {
            if (value) {
                this.cbxAdvSettings = true;
            }
            else {
                this.cbxAdvSettings = false;
            }
        });
        Checkbox.width(15);
        Checkbox.margin({ left: "1vp" });
        Checkbox.pop();
        Text.create("settings:");
        Text.margin({ top: "5vp", left: "5vp" });
        Text.padding("5vp");
        Text.fontSize("15vp");
        Text.fontColor("#4444444");
        Text.pop();
        Flex.pop();
        Flex.create({ direction: FlexDirection.Row });
        Flex.height(50);
        Flex.width('100%');
        Flex.padding(5);
        Flex.backgroundColor(0xAFEEEE);
        Row.create();
        Row.padding(5);
        Row.justifyContent(FlexAlign.Start);
        Row.alignItems(VerticalAlign.Center);
        Checkbox.create({ name: 'interval', group: 'checkboxGroup1' });
        Checkbox.select(this.cbxInterval);
        Checkbox.selectedColor(0x39a2db);
        Checkbox.onChange((value) => {
            if (value) {
                this.cbxInterval = true;
            }
            else {
                this.cbxInterval = false;
            }
        });
        Checkbox.width("15vp");
        Checkbox.margin({ left: "1vp" });
        Checkbox.pop();
        Text.create("interval");
        Text.fontSize(10);
        Text.width(50);
        Text.pop();
        TextInput.create({ text: this.advSettings_interval, placeholder: "32~1600" });
        TextInput.fontSize("10vp");
        TextInput.onChange((strInput) => {
            this.advSettings_interval = strInput;
        });
        TextInput.width("60vp");
        TextInput.borderRadius(1);
        Row.pop();
        Row.create();
        Row.padding(5);
        Row.justifyContent(FlexAlign.Start);
        Row.alignItems(VerticalAlign.Center);
        Checkbox.create({ name: 'txPower', group: 'checkboxGroup' });
        Checkbox.select(this.cbxTxPower);
        Checkbox.selectedColor(0x39a2db);
        Checkbox.onChange((value) => {
            if (value) {
                this.cbxTxPower = true;
            }
            else {
                this.cbxTxPower = false;
            }
        });
        Checkbox.width("15vp");
        Checkbox.margin({ left: "1vp" });
        Checkbox.pop();
        Text.create("txPower");
        Text.fontSize(10);
        Text.width(50);
        Text.pop();
        TextInput.create({ text: this.advSettings_txPower.toString(), placeholder: "input txPower" });
        TextInput.fontSize("10vp");
        TextInput.onChange((strInput) => {
            this.advSettings_txPower = strInput;
        });
        TextInput.width("60vp");
        TextInput.borderRadius(1);
        Row.pop();
        Row.create();
        Row.padding(5);
        Row.justifyContent(FlexAlign.Start);
        Row.alignItems(VerticalAlign.Center);
        Checkbox.create({ name: 'connectable', group: 'checkboxGroup' });
        Checkbox.select(this.cbxConnectable);
        Checkbox.selectedColor(0x39a2db);
        Checkbox.onChange((value) => {
            if (value) {
                this.cbxConnectable = true;
            }
            else {
                this.cbxConnectable = false;
            }
            this.advSettings_connectable = this.cbxConnectable;
        });
        Checkbox.width("15vp");
        Checkbox.margin({ left: "1vp" });
        Checkbox.pop();
        Text.create("connectable");
        Text.fontSize(10);
        Text.width(50);
        Text.pop();
        Row.pop();
        Flex.pop();
        Text.create("Adv Data:");
        Text.margin({ top: "5vp", left: "5vp" });
        Text.padding("5vp");
        Text.fontSize("15vp");
        Text.fontColor("#4444444");
        Text.pop();
        Flex.create({ direction: FlexDirection.Column });
        Flex.height(160);
        Flex.width('100%');
        Flex.padding(5);
        Flex.backgroundColor(0xAFEEEE);
        Row.create();
        Text.create("serviceUuid"); //.width(50)
        Text.fontSize(10);
        Text.pop(); //.width(50)
        TextInput.create({ text: this.advDataServiceUuids_serviceUuid, placeholder: "input serviceUuid" });
        TextInput.fontSize("10vp");
        TextInput.onChange((strInput) => {
            this.advDataServiceUuids_serviceUuid = strInput;
        });
        Row.pop();
        Column.create();
        Text.create("manufactureData"); //.width(60)
        Text.fontSize(12);
        Text.pop(); //.width(60)
        Row.create();
        Text.create("Id");
        Text.fontSize(10);
        Text.width(50);
        Text.pop();
        TextInput.create({ text: this.advDataManufactureData_manufactureId, placeholder: "input manufactureId(less than 8 digi)" });
        TextInput.fontSize("10vp");
        TextInput.onChange((strInput) => {
            this.advDataManufactureData_manufactureId = strInput;
        });
        TextInput.width("200vp");
        Text.create("Value");
        Text.fontSize(10);
        Text.width(50);
        Text.pop();
        TextInput.create({ text: this.advDataManufactureData_manufactureValue, placeholder: "input manufactureValue(less than 8 char)" });
        TextInput.fontSize("10vp");
        TextInput.onChange((strInput) => {
            this.advDataManufactureData_manufactureValue = strInput;
        });
        TextInput.width("200vp");
        Row.pop();
        Column.pop();
        Column.create();
        Text.create("serviceData"); //.width("42vp")
        Text.fontSize(12);
        Text.pop(); //.width("42vp")
        Row.create();
        Row.create();
        Text.create("serviceUuid");
        Text.fontSize(10);
        Text.width(50);
        Text.pop();
        TextInput.create({ text: '' + this.advDataServiceData_serviceUuid, placeholder: "input serviceUuid" });
        TextInput.fontSize("10vp");
        TextInput.onChange((strInput) => {
            this.advDataServiceData_serviceUuid = strInput;
        });
        TextInput.width("200vp");
        Row.pop();
        Row.create();
        Text.create("serviceValue");
        Text.fontSize(10);
        Text.width(50);
        Text.pop();
        TextInput.create({ text: this.advDataServiceData_serviceValue, placeholder: "input manufactureValue(less than 8 char)" });
        TextInput.fontSize("10vp");
        TextInput.onChange((strInput) => {
            this.advDataServiceData_serviceValue = strInput;
        });
        TextInput.width("200vp");
        Row.pop();
        Row.pop();
        Column.pop();
        Flex.pop();
        Flex.create({ direction: FlexDirection.Row });
        Flex.height(40);
        Flex.width('100%');
        Flex.padding(5);
        Checkbox.create({ name: 'AdvResponse', group: 'checkboxGroup1' });
        Checkbox.select(this.cbxAdvResponse);
        Checkbox.selectedColor(0x39a2db);
        Checkbox.onChange((value) => {
            if (value) {
                this.cbxAdvResponse = true;
            }
            else {
                this.cbxAdvResponse = false;
            }
        });
        Checkbox.width(15);
        Checkbox.margin({ left: "1vp" });
        Checkbox.pop();
        Text.create("Response:");
        Text.margin({ top: "5vp", left: "5vp" });
        Text.padding("5vp");
        Text.fontSize("15vp");
        Text.fontColor("#4444444");
        Text.pop();
        Flex.pop();
        Flex.create({ direction: FlexDirection.Column });
        Flex.height(160);
        Flex.width('100%');
        Flex.padding(5);
        Flex.backgroundColor(0xAFEEEE);
        Row.create();
        Text.create("serviceUuid"); //.width("40vp")
        Text.fontSize(15);
        Text.pop(); //.width("40vp")
        TextInput.create({ text: this.advResponseServiceUuids_serviceUuid, placeholder: "input serviceUuid" });
        TextInput.fontSize("10vp");
        TextInput.onChange((strInput) => {
            this.advResponseServiceUuids_serviceUuid = strInput;
        });
        Row.pop();
        Column.create();
        Text.create("manufactureData"); //.width("42vp")
        Text.fontSize(15);
        Text.pop(); //.width("42vp")
        Row.create();
        Text.create("Id");
        Text.fontSize(10);
        Text.width(50);
        Text.pop();
        TextInput.create({ text: this.advResponseManufactureData_manufactureId, placeholder: "input manufactureId(less than 8 char)" });
        TextInput.fontSize("10vp");
        TextInput.onChange((strInput) => {
            this.advResponseManufactureData_manufactureId = strInput;
        });
        TextInput.width("200vp");
        Text.create("Value");
        Text.fontSize(10);
        Text.width(50);
        Text.pop();
        TextInput.create({ text: this.advResponseManufactureData_manufactureValue, placeholder: "input manufactureValue(less than 8 char)" });
        TextInput.fontSize("10vp");
        TextInput.onChange((strInput) => {
            this.advResponseManufactureData_manufactureValue = strInput;
        });
        TextInput.width("200vp");
        Row.pop();
        Column.pop();
        Column.create();
        Text.create("serviceData"); //.width("42vp")
        Text.fontSize(15);
        Text.pop(); //.width("42vp")
        Row.create();
        Row.create();
        Text.create("serviceUuid");
        Text.fontSize("15vp");
        Text.pop();
        TextInput.create({ text: '' + this.advResponseServiceData_serviceUuid, placeholder: "input serviceUuid" });
        TextInput.fontSize("10vp");
        TextInput.onChange((strInput) => {
            this.advResponseServiceData_serviceUuid = strInput;
        });
        TextInput.width("200vp");
        Row.pop();
        Row.create();
        Text.create("serviceValue");
        Text.fontSize(15);
        Text.pop();
        TextInput.create({ text: this.advResponseServiceData_serviceValue, placeholder: "input manufactureValue(less than 8 char)" });
        TextInput.fontSize("10vp");
        TextInput.onChange((strInput) => {
            this.advResponseServiceData_serviceValue = strInput;
        });
        TextInput.width("200vp");
        Row.pop();
        Row.pop();
        Column.pop();
        Flex.pop();
        Column.pop();
        Scroll.pop();
        Stack.create();
        Stack.height("0.25vp");
        Stack.backgroundColor("#000000");
        Stack.pop();
        Column.pop();
    }
}
class GattServer extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.__message = new ObservedPropertySimple('GattServer', this, "message");
        this.__currentClick = new ObservedPropertySimple(-1, this, "currentClick");
        this.__showList = new ObservedPropertySimple(false, this, "showList");
        this.connectStateMsg = 'initial';
        this.btOnConnectState = 'on:connectStateChange';
        this.isConnectStateChangeClick = false;
        this.characteristicReadMsg = 'initial';
        this.btOnCharacteristicRead = 'on:characteristicRead';
        this.isCharacteristicReadClick = false;
        this.characteristicWriteMsg = 'initial';
        this.btOnCharacteristicWrite = 'on:characteristicWrite';
        this.isCharacteristicWriteClick = false;
        this.descriptorReadMsg = 'initial';
        this.btOnDescriptorRead = 'on:descriptorRead';
        this.isDescriptorReadClick = false;
        this.descriptorWriteMsg = 'initial';
        this.btOnDescriptorWrite = 'on:descriptorWrite';
        this.isDescriptorWriteClick = false;
        this.curServiceUUID = '00001877-0000-1000-8000-00805F9B34FB';
        this.curServiceIsPrimary = true;
        this.curCharacteristicUUID = '00002BE0-0000-1000-8000-00805F9B34FB';
        this.curCharacteristicValue = 'OHOS:BLE NANJING CHINA ';
        this.curDescriptorUUID = '00002902-0000-1000-8000-00805F9B34FB';
        this.curDescriptorValue = 'Descriptor';
        this.curGattService = {
            serviceUuid: '',
            isPrimary: true,
            characteristics: [],
            includeServices: []
        };
        this.notifyCharacteristic_serviceUuid = '00001810-0000-1000-8000-00805F9B34FB';
        this.notifyCharacteristic_characteristicUuid = '00001821-0000-1000-8000-00805F9B34FB';
        this.notifyCharacteristic_characteristicValue = 'test';
        this.notifyCharacteristic_confirm = false;
        this.notifyCharacteristicDeviceId = '88:36:CF:09:C1:90';
        this.notifyCharacteristic_notifyCharacteristic = {
            serviceUuid: '00001810-0000-1000-8000-00805F9B34FB',
            characteristicUuid: '00001821-0000-1000-8000-00805F9B34FB',
            characteristicValue: string2ArrayBuffer("999"),
            confirm: false
        };
        this.serverResponse_deviceId = '88:36:CF:09:C1:90';
        this.serverResponse_transId = '1';
        this.serverResponse_status = '0';
        this.serverResponse_offset = '0';
        this.serverResponse_value = 'respond value';
        this.serverResponse = {
            deviceId: '88:36:CF:09:C1:90',
            transId: 1,
            status: 0,
            offset: 0,
            value: string2ArrayBuffer("respond value")
        };
        this.cbxAdvSettings = false;
        this.cbxAdvResponse = false;
        this.cbxInterval = false;
        this.advSettings_interval = "1600";
        this.cbxTxPower = false;
        this.advSettings_txPower = "-7";
        this.cbxConnectable = true;
        this.advSettings_connectable = true;
        this.advDataServiceUuids_serviceUuid = "000054F6-0000-1000-8000-00805f9b34fb";
        this.advDataManufactureData_manufactureId = "7456";
        this.advDataManufactureData_manufactureValue = "CHN";
        this.advDataServiceData_serviceUuid = "000054F6-0000-1000-8000-00805f9b34fb";
        this.advDataServiceData_serviceValue = "AdvValue";
        this.advResponseServiceUuids_serviceUuid = "000054F6-0000-1000-8000-00805f9b34fb";
        this.advResponseManufactureData_manufactureId = "6547";
        this.advResponseManufactureData_manufactureValue = "CHN";
        this.advResponseServiceData_serviceUuid = "000054F6-0000-1000-8000-00805f9b34fb";
        this.advResponseServiceData_serviceValue = "AdvValue";
        this.curAdvSettings = {
            interval: 800,
            txPower: -7,
            connectable: true
        };
        this.curAdvData = {
            serviceUuids: ["00003344-0000-1000-8000-00805f9b34fb", "00008899-0000-1000-8000-00805f9b34fb"],
            manufactureData: [{
                    manufactureId: 7456,
                    manufactureValue: string2ArrayBuffer("OHOS CHN")
                }],
            serviceData: [
                {
                    serviceUuid: "000054F6-0000-1000-8000-00805f9b34fb",
                    serviceValue: string2ArrayBuffer("OhosBle!")
                },
                {
                    serviceUuid: "00009999-0000-1000-8000-00805f9b34fb",
                    serviceValue: string2ArrayBuffer("dudu_tiger")
                }
            ]
        };
        this.curAdvResponse = {
            serviceUuids: [
                "00009999-0000-1000-8000-00805f9b34fb"
            ],
            manufactureData: [{
                    manufactureId: 6547,
                    manufactureValue: string2ArrayBuffer("OHOS CHN")
                }],
            serviceData: [{
                    serviceUuid: "000054F6-0000-1000-8000-00805f9b34fb",
                    serviceValue: string2ArrayBuffer("OhosBle!")
                },
                {
                    serviceUuid: "00007456-0000-1000-8000-00805f9b34fb",
                    serviceValue: string2ArrayBuffer("OhosBle")
                }]
        };
        this.gattServerInstance = null;
        this.gattServerExist = false;
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.message !== undefined) {
            this.message = params.message;
        }
        if (params.currentClick !== undefined) {
            this.currentClick = params.currentClick;
        }
        if (params.showList !== undefined) {
            this.showList = params.showList;
        }
        if (params.connectStateMsg !== undefined) {
            this.connectStateMsg = params.connectStateMsg;
        }
        if (params.btOnConnectState !== undefined) {
            this.btOnConnectState = params.btOnConnectState;
        }
        if (params.isConnectStateChangeClick !== undefined) {
            this.isConnectStateChangeClick = params.isConnectStateChangeClick;
        }
        if (params.characteristicReadMsg !== undefined) {
            this.characteristicReadMsg = params.characteristicReadMsg;
        }
        if (params.btOnCharacteristicRead !== undefined) {
            this.btOnCharacteristicRead = params.btOnCharacteristicRead;
        }
        if (params.isCharacteristicReadClick !== undefined) {
            this.isCharacteristicReadClick = params.isCharacteristicReadClick;
        }
        if (params.characteristicWriteMsg !== undefined) {
            this.characteristicWriteMsg = params.characteristicWriteMsg;
        }
        if (params.btOnCharacteristicWrite !== undefined) {
            this.btOnCharacteristicWrite = params.btOnCharacteristicWrite;
        }
        if (params.isCharacteristicWriteClick !== undefined) {
            this.isCharacteristicWriteClick = params.isCharacteristicWriteClick;
        }
        if (params.descriptorReadMsg !== undefined) {
            this.descriptorReadMsg = params.descriptorReadMsg;
        }
        if (params.btOnDescriptorRead !== undefined) {
            this.btOnDescriptorRead = params.btOnDescriptorRead;
        }
        if (params.isDescriptorReadClick !== undefined) {
            this.isDescriptorReadClick = params.isDescriptorReadClick;
        }
        if (params.descriptorWriteMsg !== undefined) {
            this.descriptorWriteMsg = params.descriptorWriteMsg;
        }
        if (params.btOnDescriptorWrite !== undefined) {
            this.btOnDescriptorWrite = params.btOnDescriptorWrite;
        }
        if (params.isDescriptorWriteClick !== undefined) {
            this.isDescriptorWriteClick = params.isDescriptorWriteClick;
        }
        if (params.curServiceUUID !== undefined) {
            this.curServiceUUID = params.curServiceUUID;
        }
        if (params.curServiceIsPrimary !== undefined) {
            this.curServiceIsPrimary = params.curServiceIsPrimary;
        }
        if (params.curCharacteristicUUID !== undefined) {
            this.curCharacteristicUUID = params.curCharacteristicUUID;
        }
        if (params.curCharacteristicValue !== undefined) {
            this.curCharacteristicValue = params.curCharacteristicValue;
        }
        if (params.curDescriptorUUID !== undefined) {
            this.curDescriptorUUID = params.curDescriptorUUID;
        }
        if (params.curDescriptorValue !== undefined) {
            this.curDescriptorValue = params.curDescriptorValue;
        }
        if (params.curGattService !== undefined) {
            this.curGattService = params.curGattService;
        }
        if (params.notifyCharacteristic_serviceUuid !== undefined) {
            this.notifyCharacteristic_serviceUuid = params.notifyCharacteristic_serviceUuid;
        }
        if (params.notifyCharacteristic_characteristicUuid !== undefined) {
            this.notifyCharacteristic_characteristicUuid = params.notifyCharacteristic_characteristicUuid;
        }
        if (params.notifyCharacteristic_characteristicValue !== undefined) {
            this.notifyCharacteristic_characteristicValue = params.notifyCharacteristic_characteristicValue;
        }
        if (params.notifyCharacteristic_confirm !== undefined) {
            this.notifyCharacteristic_confirm = params.notifyCharacteristic_confirm;
        }
        if (params.notifyCharacteristicDeviceId !== undefined) {
            this.notifyCharacteristicDeviceId = params.notifyCharacteristicDeviceId;
        }
        if (params.notifyCharacteristic_notifyCharacteristic !== undefined) {
            this.notifyCharacteristic_notifyCharacteristic = params.notifyCharacteristic_notifyCharacteristic;
        }
        if (params.serverResponse_deviceId !== undefined) {
            this.serverResponse_deviceId = params.serverResponse_deviceId;
        }
        if (params.serverResponse_transId !== undefined) {
            this.serverResponse_transId = params.serverResponse_transId;
        }
        if (params.serverResponse_status !== undefined) {
            this.serverResponse_status = params.serverResponse_status;
        }
        if (params.serverResponse_offset !== undefined) {
            this.serverResponse_offset = params.serverResponse_offset;
        }
        if (params.serverResponse_value !== undefined) {
            this.serverResponse_value = params.serverResponse_value;
        }
        if (params.serverResponse !== undefined) {
            this.serverResponse = params.serverResponse;
        }
        if (params.cbxAdvSettings !== undefined) {
            this.cbxAdvSettings = params.cbxAdvSettings;
        }
        if (params.cbxAdvResponse !== undefined) {
            this.cbxAdvResponse = params.cbxAdvResponse;
        }
        if (params.cbxInterval !== undefined) {
            this.cbxInterval = params.cbxInterval;
        }
        if (params.advSettings_interval !== undefined) {
            this.advSettings_interval = params.advSettings_interval;
        }
        if (params.cbxTxPower !== undefined) {
            this.cbxTxPower = params.cbxTxPower;
        }
        if (params.advSettings_txPower !== undefined) {
            this.advSettings_txPower = params.advSettings_txPower;
        }
        if (params.cbxConnectable !== undefined) {
            this.cbxConnectable = params.cbxConnectable;
        }
        if (params.advSettings_connectable !== undefined) {
            this.advSettings_connectable = params.advSettings_connectable;
        }
        if (params.advDataServiceUuids_serviceUuid !== undefined) {
            this.advDataServiceUuids_serviceUuid = params.advDataServiceUuids_serviceUuid;
        }
        if (params.advDataManufactureData_manufactureId !== undefined) {
            this.advDataManufactureData_manufactureId = params.advDataManufactureData_manufactureId;
        }
        if (params.advDataManufactureData_manufactureValue !== undefined) {
            this.advDataManufactureData_manufactureValue = params.advDataManufactureData_manufactureValue;
        }
        if (params.advDataServiceData_serviceUuid !== undefined) {
            this.advDataServiceData_serviceUuid = params.advDataServiceData_serviceUuid;
        }
        if (params.advDataServiceData_serviceValue !== undefined) {
            this.advDataServiceData_serviceValue = params.advDataServiceData_serviceValue;
        }
        if (params.advResponseServiceUuids_serviceUuid !== undefined) {
            this.advResponseServiceUuids_serviceUuid = params.advResponseServiceUuids_serviceUuid;
        }
        if (params.advResponseManufactureData_manufactureId !== undefined) {
            this.advResponseManufactureData_manufactureId = params.advResponseManufactureData_manufactureId;
        }
        if (params.advResponseManufactureData_manufactureValue !== undefined) {
            this.advResponseManufactureData_manufactureValue = params.advResponseManufactureData_manufactureValue;
        }
        if (params.advResponseServiceData_serviceUuid !== undefined) {
            this.advResponseServiceData_serviceUuid = params.advResponseServiceData_serviceUuid;
        }
        if (params.advResponseServiceData_serviceValue !== undefined) {
            this.advResponseServiceData_serviceValue = params.advResponseServiceData_serviceValue;
        }
        if (params.curAdvSettings !== undefined) {
            this.curAdvSettings = params.curAdvSettings;
        }
        if (params.curAdvData !== undefined) {
            this.curAdvData = params.curAdvData;
        }
        if (params.curAdvResponse !== undefined) {
            this.curAdvResponse = params.curAdvResponse;
        }
        if (params.gattServerInstance !== undefined) {
            this.gattServerInstance = params.gattServerInstance;
        }
        if (params.gattServerExist !== undefined) {
            this.gattServerExist = params.gattServerExist;
        }
    }
    aboutToBeDeleted() {
        this.__message.aboutToBeDeleted();
        this.__currentClick.aboutToBeDeleted();
        this.__showList.aboutToBeDeleted();
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
    get showList() {
        return this.__showList.get();
    }
    set showList(newValue) {
        this.__showList.set(newValue);
    }
    //@StorageLink('on_BLEConnectionStateChange')
    render() {
        Column.create();
        Column.width('100%');
        let earlierCreatedChild_2 = this.findChildById("2");
        if (earlierCreatedChild_2 == undefined) {
            View.create(new pageTitle_1.PageTitle("2", this, { detail: 'Gatt_Server测试' }));
        }
        else {
            earlierCreatedChild_2.updateWithValueParams({
                detail: 'Gatt_Server测试'
            });
            if (!earlierCreatedChild_2.needsUpdate()) {
                earlierCreatedChild_2.markStatic();
            }
            View.create(earlierCreatedChild_2);
        }
        //        HeadComponent({ headName: 'GattServer Test', isActive: true });
        Row.create();
        Text.create("Gatt测试");
        Text.fontSize("18vp");
        Text.height(60);
        Text.width("80%");
        Text.padding({ left: 10 });
        Text.pop();
        Image.create({ "id": 16777538, "type": 20000, params: [] });
        Image.height(36);
        Image.width(36);
        Image.padding({ top: 10, bottom: 10, left: 2 });
        Image.onClick(() => {
            this.showList = !this.showList;
        });
        //        HeadComponent({ headName: 'GattServer Test', isActive: true });
        Row.pop();
        Column.create();
        Column.visibility(Visibility.Visible);
        If.create();
        if (this.showList) {
            If.branchId(0);
            let earlierCreatedChild_3 = this.findChildById("3");
            if (earlierCreatedChild_3 == undefined) {
                View.create(new GattSetting("3", this, {}));
            }
            else {
                earlierCreatedChild_3.updateWithValueParams({});
                if (!earlierCreatedChild_3.needsUpdate()) {
                    earlierCreatedChild_3.markStatic();
                }
                View.create(earlierCreatedChild_3);
            }
        }
        If.pop();
        Column.create();
        Column.height(50);
        Text.create("Result:" + this.message);
        Text.fontSize("15vp");
        Text.margin({ top: "1vp", left: "10vp", right: "20vp" });
        Text.textAlign(TextAlign.Start);
        Text.backgroundColor({ "id": 125829123, "type": 10001, params: [] });
        Text.pop();
        Text.create("ConnectionStateChange:" + this.connectStateMsg);
        Text.fontSize("15vp");
        Text.margin({ top: "1vp", left: "10vp", right: "20vp" });
        Text.textAlign(TextAlign.Start);
        Text.backgroundColor({ "id": 125829123, "type": 10001, params: [] });
        Text.pop();
        Column.pop();
        Row.create();
        Row.height(40);
        Text.create("characteristicRead:" + this.characteristicReadMsg);
        Text.fontSize("15vp");
        Text.margin({ top: "1vp", left: "10vp", right: "20vp" });
        Text.textAlign(TextAlign.Start);
        Text.backgroundColor({ "id": 125829123, "type": 10001, params: [] });
        Text.pop();
        Text.create("characteristicWrite:" + this.characteristicWriteMsg);
        Text.fontSize("15vp");
        Text.margin({ top: "1vp", left: "10vp", right: "20vp" });
        Text.textAlign(TextAlign.Start);
        Text.backgroundColor({ "id": 125829123, "type": 10001, params: [] });
        Text.pop();
        Row.pop();
        Row.create();
        Row.height(40);
        Text.create("descriptorRead:" + this.descriptorReadMsg);
        Text.fontSize("15vp");
        Text.margin({ top: "1vp", left: "10vp", right: "20vp" });
        Text.textAlign(TextAlign.Start);
        Text.backgroundColor({ "id": 125829123, "type": 10001, params: [] });
        Text.pop();
        Text.create("descriptorWrite:\n" + this.descriptorWriteMsg);
        Text.fontSize("15vp");
        Text.margin({ top: "1vp", left: "10vp", right: "20vp" });
        Text.textAlign(TextAlign.Start);
        Text.backgroundColor({ "id": 125829123, "type": 10001, params: [] });
        Text.pop();
        Row.pop();
        Stack.create();
        Stack.height("0.25vp");
        Stack.backgroundColor("#000000");
        Stack.pop();
        Scroll.create();
        Scroll.width(ConfigData_1.default.WH_100_100);
        Scroll.align(Alignment.TopStart);
        Scroll.layoutWeight(1);
        Scroll.margin({ top: { "id": 125829737, "type": 10002, params: [] } });
        List.create({ space: 1 });
        List.margin({ bottom: "80vp" });
        List.height("70%");
        ListItem.create();
        ListItem.padding({ top: 10 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            this.currentClick = 99;
            let btState = _ohos_bluetooth_1.getState();
            if (btState === _ohos_bluetooth_1.BluetoothState.STATE_ON) {
                if (this.gattServerExist) {
                    this.message = "GattServer exists!";
                    return;
                }
                this.gattServerInstance = BluetoothModel_1.default.createGattServer();
                this.message = "CreateGattServer():succeeds!";
                this.gattServerExist = true;
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
                title: "CreateGattServer", fontSize: '14vp',
                bgColor: this.currentClick === 99 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_4.updateWithValueParams({
                title: "CreateGattServer", fontSize: '14vp',
                bgColor: this.currentClick === 99 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_4);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 10 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            this.currentClick = 0;
            let btState = _ohos_bluetooth_1.getState();
            if (btState === _ohos_bluetooth_1.BluetoothState.STATE_ON) {
                if (!this.gattServerExist) {
                    this.message = NO_GATT_Server_OBJECT;
                }
                this.message = "startAdvertising:build input parameter In!";
                this.buildAdvSettings();
                this.buildAdvData();
                this.buildAdvResponse();
                let setting = this.curAdvSettings;
                let advData = this.curAdvData;
                let advResponse = this.curAdvResponse;
                this.message = "startAdvertising:build input parameter Out!";
                if (!this.cbxAdvSettings) {
                    setting = null;
                }
                if (!this.cbxAdvResponse) {
                    this.gattServerInstance.startAdvertising(setting, advData);
                }
                else {
                    this.gattServerInstance.startAdvertising(setting, advData, advResponse);
                }
                this.message = "startAdvertising succeeds!";
            }
            else if (btState === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
                this.message = "BT is not enabled!";
            }
            else {
                this.message = "switching,please operate it later!";
            }
        });
        let earlierCreatedChild_5 = this.findChildById("5");
        if (earlierCreatedChild_5 == undefined) {
            View.create(new titleComponent_1.TitleComponent("5", this, {
                title: "startAdvertising", fontSize: '14vp',
                bgColor: this.currentClick === 0 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_5.updateWithValueParams({
                title: "startAdvertising", fontSize: '14vp',
                bgColor: this.currentClick === 0 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_5);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 10 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            this.currentClick = 1;
            this.message = "stopAdvertising():";
            let btState = _ohos_bluetooth_1.getState();
            if (btState == _ohos_bluetooth_1.BluetoothState.STATE_ON) {
                if (!this.gattServerExist) {
                    this.message = NO_GATT_Server_OBJECT;
                    return;
                }
                this.gattServerInstance.stopAdvertising();
                this.message = "stopAdvertising() succeeds.";
            }
            else if (btState == _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
                this.message = "BT is not enabled!";
            }
            else {
                this.message = "switching,please operate it later!";
            }
        });
        let earlierCreatedChild_6 = this.findChildById("6");
        if (earlierCreatedChild_6 == undefined) {
            View.create(new titleComponent_1.TitleComponent("6", this, {
                title: "stopAdvertising", fontSize: '14vp',
                bgColor: this.currentClick === 1 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_6.updateWithValueParams({
                title: "stopAdvertising", fontSize: '14vp',
                bgColor: this.currentClick === 1 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_6);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 10 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            this.currentClick = 2;
            let btStatus = _ohos_bluetooth_1.getState();
            if (btStatus === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
                this.message = "BT is not enabled!";
                return;
            }
            if (!this.gattServerExist) {
                this.message = NO_GATT_Server_OBJECT;
                return;
            }
            // build service
            this.buildGattService();
            let rv = this.gattServerInstance.addService(this.curGattService);
            this.message = "result: addService()," + rv;
        });
        let earlierCreatedChild_7 = this.findChildById("7");
        if (earlierCreatedChild_7 == undefined) {
            View.create(new titleComponent_1.TitleComponent("7", this, {
                title: "addService", fontSize: '14vp',
                bgColor: this.currentClick === 2 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_7.updateWithValueParams({
                title: "addService", fontSize: '14vp',
                bgColor: this.currentClick === 2 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_7);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 10 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            this.currentClick = 3;
            this.message = "removeService(...):";
            let btState = _ohos_bluetooth_1.getState();
            if (btState == _ohos_bluetooth_1.BluetoothState.STATE_ON) {
                if (!this.gattServerExist) {
                    this.message = NO_GATT_Server_OBJECT;
                    return;
                }
                this.message = "result: removeService() ";
                if (this.gattServerInstance.removeService(this.curServiceUUID)) {
                    this.message += 'true';
                }
                else {
                    this.message += 'false';
                }
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
                title: "removeService", fontSize: '14vp',
                bgColor: this.currentClick === 3 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_8.updateWithValueParams({
                title: "removeService", fontSize: '14vp',
                bgColor: this.currentClick === 3 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_8);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 10 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            this.currentClick = 4;
            let btState = _ohos_bluetooth_1.getState();
            if (btState == _ohos_bluetooth_1.BluetoothState.STATE_ON) {
                if (!this.gattServerExist) {
                    this.message = NO_GATT_Server_OBJECT;
                    return;
                }
                this.gattServerInstance.close();
                this.message = "GattServer closed: succeeds";
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
                title: "close", fontSize: '14vp',
                bgColor: this.currentClick === 4 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_9.updateWithValueParams({
                title: "close", fontSize: '14vp',
                bgColor: this.currentClick === 4 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_9);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 10 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            this.currentClick = 5;
            let btStatus = _ohos_bluetooth_1.getState();
            if (btStatus === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
                this.message = "BT is not enabled!";
                return;
            }
            if (this.isConnectStateChangeClick) {
                this.gattServerInstance.off("connectStateChange", () => {
                });
                this.message = 'connectStateChange is off!';
                this.isConnectStateChangeClick = false;
                this.btOnConnectState = 'on:connectStateChange';
                return;
            }
            this.isConnectStateChangeClick = true;
            this.btOnConnectState = 'off:connectStateChange';
            this.gattServerInstance.on('connectStateChange', (data) => {
                this.connectStateMsg = "deviceId:" + data.deviceId + ",connectState:" + data.state;
            });
            this.message = 'connectStateChange is on';
        });
        let earlierCreatedChild_10 = this.findChildById("10");
        if (earlierCreatedChild_10 == undefined) {
            View.create(new titleComponent_1.TitleComponent("10", this, {
                title: this.btOnConnectState, fontSize: '14vp',
                bgColor: this.currentClick === 5 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_10.updateWithValueParams({
                title: this.btOnConnectState, fontSize: '14vp',
                bgColor: this.currentClick === 5 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_10);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 10 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            this.currentClick = 6;
            let btStatus = _ohos_bluetooth_1.getState();
            if (btStatus === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
                this.message = "BT is not enabled!";
                return;
            }
            if (this.isCharacteristicReadClick) {
                this.gattServerInstance.off("characteristicRead", () => {
                });
                this.message = 'characteristicRead is off!';
                this.isCharacteristicReadClick = false;
                this.btOnCharacteristicRead = 'on:characteristicRead';
                return;
            }
            this.isCharacteristicReadClick = true;
            this.btOnCharacteristicRead = 'off:characteristicRead';
            this.gattServerInstance.on('characteristicRead', (characteristicReadReq) => {
                this.characteristicReadMsg = 'characteristicRead received!';
                // client address
                let clientDeviceId = characteristicReadReq.deviceId;
                // transId
                let transId = characteristicReadReq.transId;
                // the byte offset of the start position for reading characteristic value
                let offset = characteristicReadReq.offset;
                // service/characteristic Uuid
                let characteristicUuid = characteristicReadReq.characteristicUuid;
                let serviceUuid = characteristicReadReq.serviceUuid;
                this.characteristicReadMsg += 'characteristicRead input parameters: deviceId = ' + clientDeviceId + '\n';
                this.characteristicReadMsg += 'transId = ' + transId + '\n';
                this.characteristicReadMsg += "characteristicUuid = " + characteristicUuid + '\n';
                this.characteristicReadMsg += "serviceUuid = " + serviceUuid + '\n';
                // build data for response
                let arrayBufferCCC = string2ArrayBuffer('ResponseForCharacteristicRead');
                let serverResponse = { deviceId: clientDeviceId, transId: transId, status: 0, offset: offset, value: arrayBufferCCC };
                let ret = this.gattServerInstance.sendResponse(serverResponse);
                let rvMsg = 'failed';
                if (ret) {
                    rvMsg = 'succeeded';
                }
                this.characteristicReadMsg = 'characteristicRead sendResponse:' + rvMsg;
            });
            this.message = 'characteristicRead on!';
        });
        let earlierCreatedChild_11 = this.findChildById("11");
        if (earlierCreatedChild_11 == undefined) {
            View.create(new titleComponent_1.TitleComponent("11", this, {
                title: this.btOnCharacteristicRead, fontSize: '14vp',
                bgColor: this.currentClick === 6 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_11.updateWithValueParams({
                title: this.btOnCharacteristicRead, fontSize: '14vp',
                bgColor: this.currentClick === 6 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_11);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 10 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            this.currentClick = 7;
            let btStatus = _ohos_bluetooth_1.getState();
            if (btStatus === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
                this.message = "BT is not enabled!";
                return;
            }
            if (this.isCharacteristicWriteClick) {
                this.gattServerInstance.off("characteristicWrite", () => {
                });
                this.message = 'characteristicWrite is off!';
                this.isCharacteristicWriteClick = false;
                this.btOnCharacteristicWrite = 'on:characteristicWrite';
                return;
            }
            this.isCharacteristicWriteClick = true;
            this.btOnCharacteristicWrite = 'off:characteristicWrite';
            this.gattServerInstance.on('characteristicWrite', (characteristicWriteReq) => {
                this.btOnCharacteristicWrite = 'off:characteristicWrite';
                this.characteristicWriteMsg = '';
                let arrayBufferCCC = string2ArrayBuffer("characteristicWriteForResponse");
                // client address
                let deviceId = characteristicWriteReq.deviceId;
                let transId = characteristicWriteReq.transId;
                let offset = characteristicWriteReq.offset;
                /** Whether this request should be pending for later operation */
                let isPrep = characteristicWriteReq.isPrep;
                /** Whether the remote client need a response */
                let needRsp = characteristicWriteReq.needRsp;
                /** Indicates the value to be written */
                let value = new Uint8Array(characteristicWriteReq.value);
                let characteristicUuid = characteristicWriteReq.characteristicUuid;
                let serviceUuid = characteristicWriteReq.serviceUuid;
                this.characteristicWriteMsg += "The characteristicWriteReq input parameters: deviceId =" + deviceId;
                this.characteristicWriteMsg += "transId =" + transId;
                this.characteristicWriteMsg += "offset =" + offset;
                this.characteristicWriteMsg += "isPrep =" + isPrep;
                this.characteristicWriteMsg += "needRsp =" + needRsp;
                this.characteristicWriteMsg += "value =" + JSON.stringify(value);
                this.characteristicWriteMsg += "characteristicUuid =" + characteristicUuid;
                this.characteristicWriteMsg += "serviceUuid =" + serviceUuid;
                let serverResponse = { deviceId: deviceId, transId: transId, status: 0, offset: offset, value: arrayBufferCCC };
                // sendResponse
                if (needRsp) {
                    let ret = this.gattServerInstance.sendResponse(serverResponse);
                    if (ret) {
                        this.message = 'characteristicWrite sendResponse successfully';
                    }
                    else {
                        this.message = 'characteristicWrite sendResponse failed';
                    }
                }
                this.notifyCharacteristic_notifyCharacteristic = {
                    serviceUuid: serviceUuid,
                    characteristicUuid: characteristicUuid,
                    characteristicValue: string2ArrayBuffer("Value4notifyCharacteristic"),
                    confirm: true //notification:true; indication:false;
                };
                let notifyCharacteristic = this.notifyCharacteristic_notifyCharacteristic;
                if (this.gattServerInstance.notifyCharacteristicChanged(deviceId, notifyCharacteristic)) {
                    this.message = "notifyCharacteristicChanged successfully!";
                }
                else {
                    this.message = "notifyCharacteristicChanged failed!";
                }
            });
            this.message = 'characteristicWrite is on';
        });
        let earlierCreatedChild_12 = this.findChildById("12");
        if (earlierCreatedChild_12 == undefined) {
            View.create(new titleComponent_1.TitleComponent("12", this, {
                title: this.btOnCharacteristicWrite, fontSize: '14vp',
                bgColor: this.currentClick === 7 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_12.updateWithValueParams({
                title: this.btOnCharacteristicWrite, fontSize: '14vp',
                bgColor: this.currentClick === 7 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_12);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 10 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            this.currentClick = 8;
            let btStatus = _ohos_bluetooth_1.getState();
            if (btStatus === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
                this.message = "BT is not enabled!";
                return;
            }
            if (this.isDescriptorReadClick) {
                this.gattServerInstance.off("descriptorRead", () => {
                });
                this.message = 'descriptorRead is off!';
                this.isDescriptorReadClick = false;
                this.btOnDescriptorRead = 'on:descriptorRead';
                return;
            }
            this.isDescriptorReadClick = true;
            this.btOnDescriptorRead = 'off:descriptorRead';
            this.gattServerInstance.on('descriptorRead', (descriptorReadReq) => {
                this.btOnDescriptorRead = 'off:descriptorRead';
                this.descriptorReadMsg = "descriptorReadIn!! \n";
                let deviceId = descriptorReadReq.deviceId;
                let transId = descriptorReadReq.transId;
                /** Indicates the byte offset of the start position for reading characteristic value */
                let offset = descriptorReadReq.offset;
                let descriptorUuid = descriptorReadReq.descriptorUuid;
                let characteristicUuid = descriptorReadReq.characteristicUuid;
                let serviceUuid = descriptorReadReq.serviceUuid;
                let arrayBufferDesc = string2ArrayBuffer("Response4descriptorRead");
                let serverResponse = { deviceId: deviceId, transId: transId, status: 0, offset: offset, value: arrayBufferDesc };
                let ret = this.gattServerInstance.sendResponse(serverResponse);
                if (ret) {
                    this.descriptorReadMsg += 'descriptorRead sendResponse successfully!!';
                }
                else {
                    this.descriptorReadMsg += 'descriptorRead sendResponse failed!!';
                }
            });
            this.message = 'descriptorRead is on';
        });
        let earlierCreatedChild_13 = this.findChildById("13");
        if (earlierCreatedChild_13 == undefined) {
            View.create(new titleComponent_1.TitleComponent("13", this, {
                title: this.btOnDescriptorRead, fontSize: '14vp',
                bgColor: this.currentClick === 8 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_13.updateWithValueParams({
                title: this.btOnDescriptorRead, fontSize: '14vp',
                bgColor: this.currentClick === 8 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_13);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 10 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            this.currentClick = 9;
            let btStatus = _ohos_bluetooth_1.getState();
            if (btStatus === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
                this.message = "BT is not enabled!";
                return;
            }
            if (this.isDescriptorWriteClick) {
                this.gattServerInstance.off("descriptorWrite", () => {
                });
                this.message = 'descriptorWrite is off!';
                this.isDescriptorWriteClick = false;
                this.btOnDescriptorWrite = 'on:descriptorWrite';
                return;
            }
            this.isDescriptorWriteClick = true;
            this.btOnDescriptorWrite = 'off:descriptorWrite';
            this.gattServerInstance.on('descriptorWrite', (descriptorWriteReq) => {
                this.btOnDescriptorWrite = 'off:descriptorWrite';
                this.descriptorWriteMsg = "descriptorWriteIn";
                this.descriptorWriteMsg += "The descriptorWriteReq client address: deviceId is " + descriptorWriteReq.deviceId;
                let deviceId = descriptorWriteReq.deviceId;
                let transId = descriptorWriteReq.transId;
                /** Indicates the byte offset of the start position for writing descriptor value */
                let offset = descriptorWriteReq.offset;
                /** Whether this request should be pending for later operation */
                let isPrep = descriptorWriteReq.isPrep;
                /** Whether the remote client need a response */
                let needRsp = descriptorWriteReq.needRsp;
                /** Indicates the value to be written */
                let value = new Uint8Array(descriptorWriteReq.value);
                let descriptorUuid = descriptorWriteReq.descriptorUuid;
                let characteristicUuid = descriptorWriteReq.characteristicUuid;
                let serviceUuid = descriptorWriteReq.serviceUuid;
                let arrayBufferDesc = new ArrayBuffer(8);
                let descValue = new Uint8Array(arrayBufferDesc);
                descValue[0] = value[0];
                let serverResponse = { deviceId: deviceId, transId: transId, status: 0, offset: offset, value: arrayBufferDesc };
                if (needRsp) {
                    let ret = this.gattServerInstance.sendResponse(serverResponse);
                    if (ret) {
                        this.message = 'descriptorWrite sendResponse successfully';
                    }
                    else {
                        this.message = 'descriptorWrite sendResponse failed';
                    }
                }
                this.descriptorWriteMsg += "\n" + 'descriptorWrite Callback finished!';
            });
            this.message = 'descriptorWrite is on';
        });
        let earlierCreatedChild_14 = this.findChildById("14");
        if (earlierCreatedChild_14 == undefined) {
            View.create(new titleComponent_1.TitleComponent("14", this, {
                title: this.btOnDescriptorWrite, fontSize: '14vp',
                bgColor: this.currentClick === 9 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_14.updateWithValueParams({
                title: this.btOnDescriptorWrite, fontSize: '14vp',
                bgColor: this.currentClick === 9 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_14);
        }
        ListItem.pop();
        List.pop();
        Scroll.pop();
        Column.pop();
        Column.pop();
    }
    buildAdvSettings() {
        if (this.cbxAdvSettings) {
            if (this.cbxInterval) {
                this.curAdvSettings.interval = parseInt(this.advSettings_interval);
            }
            if (this.cbxTxPower) {
                this.curAdvSettings.txPower = parseInt(this.advSettings_txPower);
            }
            if (this.cbxConnectable) {
                this.curAdvSettings.connectable = this.advSettings_connectable;
            }
        }
    }
    buildAdvData() {
        let serviceUuidInstance = this.advDataServiceUuids_serviceUuid;
        this.curAdvData.serviceUuids.length = 0;
        this.curAdvData.serviceUuids.push(serviceUuidInstance);
        let manufactureIdTmp = parseInt(this.advDataManufactureData_manufactureId);
        let manufactureValueTmp = this.advDataManufactureData_manufactureValue;
        let manufactureDataInstance = {
            manufactureId: manufactureIdTmp,
            manufactureValue: string2ArrayBuffer(manufactureValueTmp)
        };
        this.curAdvData.manufactureData.length = 0;
        this.curAdvData.manufactureData.push(manufactureDataInstance);
        let serviceUuid4Service = this.advDataServiceData_serviceUuid;
        let serviceValue4Service = this.advDataServiceData_serviceValue;
        let serviceDataInstance = {
            serviceUuid: serviceUuid4Service,
            serviceValue: string2ArrayBuffer(serviceValue4Service)
        };
        this.curAdvData.serviceData.length = 0;
        this.curAdvData.serviceData.push(serviceDataInstance);
    }
    buildAdvResponse() {
        if (!this.cbxAdvResponse) {
            return;
        }
        let serviceUuidInstance = this.advResponseServiceUuids_serviceUuid;
        this.curAdvResponse.serviceUuids.length = 0;
        this.curAdvResponse.serviceUuids.push(serviceUuidInstance);
        let manufactureIdTmp = parseInt(this.advResponseManufactureData_manufactureId);
        let manufactureValueTmp = this.advResponseManufactureData_manufactureValue;
        let manufactureDataInstance = {
            manufactureId: manufactureIdTmp,
            manufactureValue: string2ArrayBuffer(manufactureValueTmp)
        };
        this.curAdvResponse.manufactureData.length = 0;
        this.curAdvResponse.manufactureData.push(manufactureDataInstance);
        let serviceUuid4Service = this.advResponseServiceData_serviceUuid;
        let serviceValue4Service = this.advResponseServiceData_serviceValue;
        let serviceDataInstance = {
            serviceUuid: serviceUuid4Service,
            serviceValue: string2ArrayBuffer(serviceValue4Service)
        };
        this.curAdvResponse.serviceData.length = 0;
        this.curAdvResponse.serviceData.push(serviceDataInstance);
    }
    buildGattService() {
        // build descriptors
        let descriptors = [];
        let descriptor = {
            serviceUuid: this.curServiceUUID,
            characteristicUuid: this.curCharacteristicUUID,
            descriptorUuid: this.curDescriptorUUID,
            descriptorValue: string2ArrayBuffer(this.curDescriptorValue)
        };
        descriptors.push(descriptor);
        // build characteristics
        let characteristics = [];
        let cccValue = this.curCharacteristicValue;
        let characteristic = {
            serviceUuid: this.curServiceUUID,
            characteristicUuid: this.curCharacteristicUUID,
            characteristicValue: string2ArrayBuffer(cccValue),
            descriptors: descriptors
        };
        characteristics.push(characteristic);
        this.curGattService.serviceUuid = this.curServiceUUID;
        this.curGattService.characteristics = characteristics;
        this.curGattService.isPrimary = this.curServiceIsPrimary;
        this.curGattService.includeServices = [];
    }
    buildNotifyCharacteristic() {
        this.notifyCharacteristic_notifyCharacteristic = {
            serviceUuid: this.notifyCharacteristic_serviceUuid,
            characteristicUuid: this.notifyCharacteristic_characteristicUuid,
            characteristicValue: string2ArrayBuffer(this.notifyCharacteristic_characteristicValue),
            confirm: this.notifyCharacteristic_confirm
        };
    }
    buildServerResponse() {
        this.serverResponse = {
            deviceId: this.serverResponse_deviceId,
            transId: parseInt(this.serverResponse_transId),
            status: parseInt(this.serverResponse_status),
            offset: parseInt(this.serverResponse_offset),
            value: string2ArrayBuffer(this.serverResponse_value)
        };
    }
}
/**
* string to ArrayBuffer
* @param {*} str: the string to covert
* @return: ArrayBuffer
*/
function string2ArrayBuffer(str) {
    let array = new Uint8Array(str.length);
    for (var i = 0; i < str.length; i++) {
        array[i] = str.charCodeAt(i);
    }
    return array.buffer;
}
loadDocument(new GattServer("1", undefined, {}));


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
/******/ 			"./pages/subManualApiTest/subGattTest/gattServer": 0
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
/******/ 	var __webpack_exports__ = __webpack_require__.O(undefined, ["commons"], () => (__webpack_require__("../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/pages/subManualApiTest/subGattTest/gattServer.ets?entry")))
/******/ 	__webpack_exports__ = __webpack_require__.O(__webpack_exports__);
/******/ 	_2a33209e6af7922da1521a8a828bfbd0 = __webpack_exports__;
/******/ 	
/******/ })()
;
//# sourceMappingURL=gattServer.js.map