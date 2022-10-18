var _2a33209e6af7922da1521a8a828bfbd0;
/******/ (() => { // webpackBootstrap
/******/ 	"use strict";
/******/ 	var __webpack_modules__ = ({

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/pages/subManualApiTest/subSppTest/sppServerTest.ets?entry":
/*!**************************************************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/pages/subManualApiTest/subSppTest/sppServerTest.ets?entry ***!
  \**************************************************************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {


var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
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
const titleComponent_1 = __webpack_require__(/*! ../../../../Component/titleComponent */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Component/titleComponent.ets");
__webpack_require__(/*! ../../../../component/headcomponent */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/headcomponent.ets");
var _ohos_bluetooth_1  = globalThis.requireNapi('bluetooth') || (isSystemplugin('bluetooth', 'ohos') ? globalThis.ohosplugin.bluetooth : isSystemplugin('bluetooth', 'system') ? globalThis.systemplugin.bluetooth : undefined);
const LogUtil_1 = __importDefault(__webpack_require__(/*! ../../../../Utils/LogUtil */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Utils/LogUtil.ts"));
const ConfigData_1 = __importDefault(__webpack_require__(/*! ../../../../Utils/ConfigData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Utils/ConfigData.ts"));
const BluetoothModel_1 = __importDefault(__webpack_require__(/*! ../../../model/BluetoothModel */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/BluetoothModel.ts"));
const BluetoothDevice_1 = __importDefault(__webpack_require__(/*! ../../../model/BluetoothDevice */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/BluetoothDevice.ts"));
const BluetoothDeviceController_1 = __importDefault(__webpack_require__(/*! ../../../controller/BluetoothDeviceController */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/controller/BluetoothDeviceController.ts"));
const pageTitle_1 = __webpack_require__(/*! ../../../../Component/pageTitle */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Component/pageTitle.ets");
let MODEL = "bluetoothModel:";
class SppServer extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.__message = new ObservedPropertySimple("", this, "message");
        this.__deviceId = new ObservedPropertySimple(AppStorage.Get('inputDeviceId'), this, "deviceId");
        this.__dataReadMessage = new ObservedPropertySimple("", this, "dataReadMessage");
        this.stateBT = "";
        this.__writeData = new ObservedPropertyObject('', this, "writeData");
        this.uuid = AppStorage.Get('inputUuid');
        this.serverNumber = -1;
        this.clientNumber = -1;
        this.sppOption = {};
        this.TAG_PAGE = ConfigData_1.default.TAG + 'Paired_Device ';
        this.__btEnable = new ObservedPropertySimple(false, this, "btEnable");
        this.__currentClick = new ObservedPropertySimple(-1, this, "currentClick");
        this.__pairingDevice = AppStorage.GetOrCreate().setAndLink("pairedDeviceInfo", new BluetoothDevice_1.default, this);
        this.__controller = AppStorage.GetOrCreate().setAndLink("btDeviceController", new BluetoothDeviceController_1.default, this);
        this.__pairPinCode = new ObservedPropertySimple('', this, "pairPinCode");
        this.pairDialog = new CustomDialogController({
            builder: () => {
                let jsDialog = new PairDialog("17", this, {
                    deviceName: this.pairingDevice.deviceName,
                    pinCode: this.pairPinCode,
                    action: (accept) => {
                        this.confirmPairing(accept);
                    }
                });
                jsDialog.setController(this.pairDialog);
                View.create(jsDialog);
            },
            alignment: DialogAlignment.Bottom,
            autoCancel: true
        }, this);
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.message !== undefined) {
            this.message = params.message;
        }
        if (params.deviceId !== undefined) {
            this.deviceId = params.deviceId;
        }
        if (params.dataReadMessage !== undefined) {
            this.dataReadMessage = params.dataReadMessage;
        }
        if (params.stateBT !== undefined) {
            this.stateBT = params.stateBT;
        }
        if (params.writeData !== undefined) {
            this.writeData = params.writeData;
        }
        if (params.uuid !== undefined) {
            this.uuid = params.uuid;
        }
        if (params.serverNumber !== undefined) {
            this.serverNumber = params.serverNumber;
        }
        if (params.clientNumber !== undefined) {
            this.clientNumber = params.clientNumber;
        }
        if (params.sppOption !== undefined) {
            this.sppOption = params.sppOption;
        }
        if (params.TAG_PAGE !== undefined) {
            this.TAG_PAGE = params.TAG_PAGE;
        }
        if (params.btEnable !== undefined) {
            this.btEnable = params.btEnable;
        }
        if (params.currentClick !== undefined) {
            this.currentClick = params.currentClick;
        }
        if (params.pairPinCode !== undefined) {
            this.pairPinCode = params.pairPinCode;
        }
        if (params.pairDialog !== undefined) {
            this.pairDialog = params.pairDialog;
        }
    }
    aboutToBeDeleted() {
        this.__message.aboutToBeDeleted();
        this.__deviceId.aboutToBeDeleted();
        this.__dataReadMessage.aboutToBeDeleted();
        this.__writeData.aboutToBeDeleted();
        this.__btEnable.aboutToBeDeleted();
        this.__currentClick.aboutToBeDeleted();
        this.__pairingDevice.aboutToBeDeleted();
        this.__controller.aboutToBeDeleted();
        this.__pairPinCode.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id());
    }
    get message() {
        return this.__message.get();
    }
    set message(newValue) {
        this.__message.set(newValue);
    }
    get deviceId() {
        return this.__deviceId.get();
    }
    set deviceId(newValue) {
        this.__deviceId.set(newValue);
    }
    get dataReadMessage() {
        return this.__dataReadMessage.get();
    }
    set dataReadMessage(newValue) {
        this.__dataReadMessage.set(newValue);
    }
    get writeData() {
        return this.__writeData.get();
    }
    set writeData(newValue) {
        this.__writeData.set(newValue);
    }
    get btEnable() {
        return this.__btEnable.get();
    }
    set btEnable(newValue) {
        this.__btEnable.set(newValue);
    }
    get currentClick() {
        return this.__currentClick.get();
    }
    set currentClick(newValue) {
        this.__currentClick.set(newValue);
    }
    get pairingDevice() {
        return this.__pairingDevice.get();
    }
    set pairingDevice(newValue) {
        this.__pairingDevice.set(newValue);
    }
    get controller() {
        return this.__controller.get();
    }
    set controller(newValue) {
        this.__controller.set(newValue);
    }
    get pairPinCode() {
        return this.__pairPinCode.get();
    }
    set pairPinCode(newValue) {
        this.__pairPinCode.set(newValue);
    }
    render() {
        Row.create();
        Row.backgroundColor({ "id": 125829132, "type": 10001, params: [] });
        Row.height('100%');
        Row.alignItems(VerticalAlign.Top);
        Column.create();
        Column.width('100%');
        Column.create();
        let earlierCreatedChild_2 = this.findChildById("2");
        if (earlierCreatedChild_2 == undefined) {
            View.create(new pageTitle_1.PageTitle("2", this, { detail: 'SPP_SERVER业务' }));
        }
        else {
            earlierCreatedChild_2.updateWithValueParams({
                detail: 'SPP_SERVER业务'
            });
            if (!earlierCreatedChild_2.needsUpdate()) {
                earlierCreatedChild_2.markStatic();
            }
            View.create(earlierCreatedChild_2);
        }
        //          HeadComponent({ headName: $r('app.string.spp_server_test_title'), isActive: true });
        Stack.create();
        //          HeadComponent({ headName: $r('app.string.spp_server_test_title'), isActive: true });
        Stack.height("0.5vp");
        //          HeadComponent({ headName: $r('app.string.spp_server_test_title'), isActive: true });
        Stack.backgroundColor("#000000");
        //          HeadComponent({ headName: $r('app.string.spp_server_test_title'), isActive: true });
        Stack.pop();
        Row.create();
        Row.backgroundColor({ "id": 16777297, "type": 10001, params: [] });
        Row.padding(5);
        Row.justifyContent(FlexAlign.Start);
        Row.alignItems(VerticalAlign.Center);
        Text.create("目标设备MAC：");
        Text.fontSize(17);
        Text.width(60);
        Text.pop();
        TextInput.create({ text: this.deviceId, placeholder: "请输入mac地址" });
        TextInput.fontSize("14vp");
        TextInput.onChange((str) => {
            this.deviceId = str;
            AppStorage.SetOrCreate('inputDeviceId', str);
        });
        TextInput.width("80%");
        TextInput.borderRadius(1);
        Row.pop();
        Row.create();
        Row.backgroundColor({ "id": 16777302, "type": 10001, params: [] });
        Row.padding(5);
        Row.justifyContent(FlexAlign.Start);
        Row.alignItems(VerticalAlign.Center);
        Text.create("目标设备UUID：");
        Text.fontSize(17);
        Text.width(60);
        Text.pop();
        TextInput.create({ text: this.uuid, placeholder: "请输入uuid" });
        TextInput.fontSize("14vp");
        TextInput.onChange((str) => {
            this.uuid = str;
            AppStorage.SetOrCreate('inputUuid', str);
        });
        TextInput.width("80%");
        TextInput.borderRadius(1);
        Row.pop();
        Row.create();
        Row.backgroundColor({ "id": 16777297, "type": 10001, params: [] });
        Row.padding(5);
        Row.justifyContent(FlexAlign.Start);
        Row.alignItems(VerticalAlign.Center);
        Text.create("写入目标设备数据：");
        Text.fontSize(17);
        Text.width(60);
        Text.pop();
        TextInput.create({ text: this.writeData, placeholder: "请输入写入的数据" });
        TextInput.fontSize("14vp");
        TextInput.onChange((str) => { this.writeData = this.strToArrayBuffer(str); });
        TextInput.width("80%");
        TextInput.borderRadius(1);
        Row.pop();
        Row.create();
        Row.padding(5);
        Row.justifyContent(FlexAlign.Start);
        Row.alignItems(VerticalAlign.Center);
        Text.create("读取的数据为：");
        Text.fontSize("17vp");
        Text.pop();
        Text.create(this.dataReadMessage);
        Text.fontSize("14vp");
        Text.width("80%");
        Text.borderRadius(1);
        Text.pop();
        Row.pop();
        Text.create("接口调用结果：" + this.message);
        Text.fontSize("17vp");
        Text.padding(10);
        Text.textAlign(TextAlign.Start);
        Text.backgroundColor({ "id": 16777297, "type": 10001, params: [] });
        Text.pop();
        Scroll.create();
        Scroll.height('50%');
        Column.create();
        Column.width('100%');
        Column.visibility(Visibility.Visible);
        List.create({ space: 1 });
        List.margin(20);
        //使能蓝牙
        ListItem.create();
        //使能蓝牙
        ListItem.padding({ top: 20 });
        //使能蓝牙
        ListItem.height(80);
        //使能蓝牙
        ListItem.borderRadius(10);
        //使能蓝牙
        ListItem.onClick(() => {
            this.currentClick = 0;
            if (this.btEnable) {
                this.message = '蓝牙已经使能';
                return;
            }
            let ret = BluetoothModel_1.default.enableBluetooth();
            this.btEnable = ret;
            AppStorage.SetOrCreate('bluetoothIsOn', this.btEnable);
            this.message = "蓝牙使能执行结果：" + ret;
        });
        let earlierCreatedChild_3 = this.findChildById("3");
        if (earlierCreatedChild_3 == undefined) {
            View.create(new titleComponent_1.TitleComponent("3", this, {
                title: "enableBluetooth", fontSize: '15vp',
                bgColor: this.currentClick === 0 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_3.updateWithValueParams({
                title: "enableBluetooth", fontSize: '15vp',
                bgColor: this.currentClick === 0 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_3);
        }
        //使能蓝牙
        ListItem.pop();
        //去使能蓝牙
        ListItem.create();
        //去使能蓝牙
        ListItem.padding({ top: 20 });
        //去使能蓝牙
        ListItem.height(80);
        //去使能蓝牙
        ListItem.borderRadius(10);
        //去使能蓝牙
        ListItem.onClick(() => {
            this.currentClick = 1;
            if (!this.btEnable) {
                this.message = '蓝牙还未使能';
                return;
            }
            let ret = BluetoothModel_1.default.disableBluetooth();
            this.btEnable = false;
            AppStorage.SetOrCreate('bluetoothIsOn', this.btEnable);
            this.message = "蓝牙去使能执行结果：" + ret;
        });
        let earlierCreatedChild_4 = this.findChildById("4");
        if (earlierCreatedChild_4 == undefined) {
            View.create(new titleComponent_1.TitleComponent("4", this, {
                title: "disableBluetooth", fontSize: '15vp',
                bgColor: this.currentClick === 1 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_4.updateWithValueParams({
                title: "disableBluetooth", fontSize: '15vp',
                bgColor: this.currentClick === 1 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_4);
        }
        //去使能蓝牙
        ListItem.pop();
        //设置扫描方式为可发现可连接
        ListItem.create();
        //设置扫描方式为可发现可连接
        ListItem.padding({ top: 20 });
        //设置扫描方式为可发现可连接
        ListItem.height(80);
        //设置扫描方式为可发现可连接
        ListItem.borderRadius(10);
        //设置扫描方式为可发现可连接
        ListItem.onClick(() => {
            this.currentClick = 2;
            if (!this.btEnable) {
                this.message = '蓝牙还未使能';
                return;
            }
            let ret = BluetoothModel_1.default.setBluetoothScanMode(4, 0);
            if (ret) {
                this.message = "设置成功";
            }
            else {
                this.message = "设置失败";
            }
        });
        let earlierCreatedChild_5 = this.findChildById("5");
        if (earlierCreatedChild_5 == undefined) {
            View.create(new titleComponent_1.TitleComponent("5", this, {
                title: "setBluetoothScanMode", fontSize: '15vp',
                bgColor: this.currentClick === 2 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_5.updateWithValueParams({
                title: "setBluetoothScanMode", fontSize: '15vp',
                bgColor: this.currentClick === 2 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_5);
        }
        //设置扫描方式为可发现可连接
        ListItem.pop();
        //查询蓝牙状态
        ListItem.create();
        //查询蓝牙状态
        ListItem.padding({ top: 20 });
        //查询蓝牙状态
        ListItem.height(80);
        //查询蓝牙状态
        ListItem.borderRadius(10);
        //查询蓝牙状态
        ListItem.onClick(() => {
            this.currentClick = 3;
            let ret = BluetoothModel_1.default.getState();
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
        });
        let earlierCreatedChild_6 = this.findChildById("6");
        if (earlierCreatedChild_6 == undefined) {
            View.create(new titleComponent_1.TitleComponent("6", this, {
                title: "getState", fontSize: '15vp',
                bgColor: this.currentClick === 3 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_6.updateWithValueParams({
                title: "getState", fontSize: '15vp',
                bgColor: this.currentClick === 3 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_6);
        }
        //查询蓝牙状态
        ListItem.pop();
        //查询当前扫描方式
        ListItem.create();
        //查询当前扫描方式
        ListItem.padding({ top: 20 });
        //查询当前扫描方式
        ListItem.height(80);
        //查询当前扫描方式
        ListItem.borderRadius(10);
        //查询当前扫描方式
        ListItem.onClick(() => {
            this.currentClick = 4;
            let ret = BluetoothModel_1.default.getBluetoothScanMode();
            this.message = "蓝牙当前扫描方式为：" + ret;
        });
        let earlierCreatedChild_7 = this.findChildById("7");
        if (earlierCreatedChild_7 == undefined) {
            View.create(new titleComponent_1.TitleComponent("7", this, {
                title: "getBluetoothScanMode", fontSize: '15vp',
                bgColor: this.currentClick === 4 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_7.updateWithValueParams({
                title: "getBluetoothScanMode", fontSize: '15vp',
                bgColor: this.currentClick === 4 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_7);
        }
        //查询当前扫描方式
        ListItem.pop();
        //查询当前连接状态
        ListItem.create();
        //查询当前连接状态
        ListItem.padding({ top: 20 });
        //查询当前连接状态
        ListItem.height(80);
        //查询当前连接状态
        ListItem.borderRadius(10);
        //查询当前连接状态
        ListItem.onClick(() => {
            this.currentClick = 5;
            let ret = BluetoothModel_1.default.getBtConnectionState();
            this.message = "当前连接状态是：" + ret;
        });
        let earlierCreatedChild_8 = this.findChildById("8");
        if (earlierCreatedChild_8 == undefined) {
            View.create(new titleComponent_1.TitleComponent("8", this, {
                title: "getBtConnectionState", fontSize: '15vp',
                bgColor: this.currentClick === 5 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_8.updateWithValueParams({
                title: "getBtConnectionState", fontSize: '15vp',
                bgColor: this.currentClick === 5 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_8);
        }
        //查询当前连接状态
        ListItem.pop();
        //服务端(安全模式)连接
        ListItem.create();
        //服务端(安全模式)连接
        ListItem.padding({ top: 20 });
        //服务端(安全模式)连接
        ListItem.height(80);
        //服务端(安全模式)连接
        ListItem.borderRadius(10);
        //服务端(安全模式)连接
        ListItem.onClick(() => {
            console.log('spp server socketId(safe mode) create! ');
            if (this.uuid == '') {
                this.message = 'uuid为空';
                return;
            }
            // 创建蓝牙服务器侦听套接字, 返回服务端ID
            _ohos_bluetooth_1.sppListen('BluetoothDevice', { uuid: this.uuid, secure: true, type: 0 }, (code, serverSocketID) => {
                console.log('bluetooth error code: ' + code.code);
                if (code.code == 0) {
                    console.log('spp server: serverSocketID(safe mode) - ' + serverSocketID);
                    this.serverNumber = serverSocketID;
                    this.message = "创建server成功";
                }
            });
        });
        let earlierCreatedChild_9 = this.findChildById("9");
        if (earlierCreatedChild_9 == undefined) {
            View.create(new titleComponent_1.TitleComponent("9", this, {
                title: "spp_server_create(安全模式)", fontSize: '15vp',
                bgColor: this.currentClick === 6 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_9.updateWithValueParams({
                title: "spp_server_create(安全模式)", fontSize: '15vp',
                bgColor: this.currentClick === 6 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_9);
        }
        //服务端(安全模式)连接
        ListItem.pop();
        //服务端(非安全模式)连接
        ListItem.create();
        //服务端(非安全模式)连接
        ListItem.padding({ top: 20 });
        //服务端(非安全模式)连接
        ListItem.height(80);
        //服务端(非安全模式)连接
        ListItem.borderRadius(10);
        //服务端(非安全模式)连接
        ListItem.onClick(() => {
            console.log('spp server socketId(non-safe mode) create! ');
            if (this.uuid == '') {
                this.message = 'uuid为空';
                return;
            }
            // 创建蓝牙服务器侦听套接字, 返回服务端ID
            _ohos_bluetooth_1.sppListen('BluetoothDevice', { uuid: this.uuid, secure: false, type: 0 }, (code, serverSocketID) => {
                console.log('bluetooth error code: ' + code.code);
                if (code.code == 0) {
                    console.log('spp server: serverSocketID(non-safe mode) - ' + serverSocketID);
                    this.serverNumber = serverSocketID;
                    this.message = "创建server成功";
                }
            });
        });
        let earlierCreatedChild_10 = this.findChildById("10");
        if (earlierCreatedChild_10 == undefined) {
            View.create(new titleComponent_1.TitleComponent("10", this, {
                title: "spp_server_create(非安全模式)", fontSize: '15vp',
                bgColor: this.currentClick === 7 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_10.updateWithValueParams({
                title: "spp_server_create(非安全模式)", fontSize: '15vp',
                bgColor: this.currentClick === 7 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_10);
        }
        //服务端(非安全模式)连接
        ListItem.pop();
        //服务端接受操作
        ListItem.create();
        //服务端接受操作
        ListItem.padding({ top: 20 });
        //服务端接受操作
        ListItem.height(80);
        //服务端接受操作
        ListItem.borderRadius(10);
        //服务端接受操作
        ListItem.onClick(() => {
            // 连接客户端，返回客户端ID
            this.message = "等待客户端连接";
            _ohos_bluetooth_1.sppAccept(this.serverNumber, (code, clientSocketID) => {
                if (code.code == 0) {
                    console.log('spp server: clientSocketID - ' + clientSocketID);
                    this.clientNumber = clientSocketID;
                    this.message = "接受客户端成功";
                    _ohos_bluetooth_1.on('sppRead', this.clientNumber, this.dataRead.bind(this));
                }
            });
        });
        let earlierCreatedChild_11 = this.findChildById("11");
        if (earlierCreatedChild_11 == undefined) {
            View.create(new titleComponent_1.TitleComponent("11", this, {
                title: "spp_server_accept", fontSize: '15vp',
                bgColor: this.currentClick === 8 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_11.updateWithValueParams({
                title: "spp_server_accept", fontSize: '15vp',
                bgColor: this.currentClick === 8 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_11);
        }
        //服务端接受操作
        ListItem.pop();
        //服务端写数据传给客户端
        ListItem.create();
        //服务端写数据传给客户端
        ListItem.padding({ top: 20 });
        //服务端写数据传给客户端
        ListItem.height(80);
        //服务端写数据传给客户端
        ListItem.borderRadius(10);
        //服务端写数据传给客户端
        ListItem.onClick(() => {
            LogUtil_1.default.info(MODEL + 'sppWrite clientNumber: ' + JSON.stringify(this.clientNumber));
            const sppWriteSuccess = _ohos_bluetooth_1.sppWrite(this.clientNumber, ObservedObject.GetRawObject(this.writeData));
            if (sppWriteSuccess) {
                console.log('sppWrite successfully');
                this.message = "写入成功 ";
            }
            else {
                console.log('sppWrite failed');
                this.message = "写入失败";
            }
        });
        let earlierCreatedChild_12 = this.findChildById("12");
        if (earlierCreatedChild_12 == undefined) {
            View.create(new titleComponent_1.TitleComponent("12", this, {
                title: "spp_client_write", fontSize: '15vp',
                bgColor: this.currentClick === 9 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_12.updateWithValueParams({
                title: "spp_client_write", fontSize: '15vp',
                bgColor: this.currentClick === 9 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_12);
        }
        //服务端写数据传给客户端
        ListItem.pop();
        //服务端读操作打开
        ListItem.create();
        //服务端读操作打开
        ListItem.padding({ top: 20 });
        //服务端读操作打开
        ListItem.height(80);
        //服务端读操作打开
        ListItem.borderRadius(10);
        //服务端读操作打开
        ListItem.onClick(() => {
            _ohos_bluetooth_1.on('sppRead', this.clientNumber, this.dataRead.bind(this));
        });
        let earlierCreatedChild_13 = this.findChildById("13");
        if (earlierCreatedChild_13 == undefined) {
            View.create(new titleComponent_1.TitleComponent("13", this, {
                title: "spp_server_readOn", fontSize: '15vp',
                bgColor: this.currentClick === 10 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_13.updateWithValueParams({
                title: "spp_server_readOn", fontSize: '15vp',
                bgColor: this.currentClick === 10 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_13);
        }
        //服务端读操作打开
        ListItem.pop();
        //服务端读操作关闭
        ListItem.create();
        //服务端读操作关闭
        ListItem.padding({ top: 20 });
        //服务端读操作关闭
        ListItem.height(80);
        //服务端读操作关闭
        ListItem.borderRadius(10);
        //服务端读操作关闭
        ListItem.onClick(() => {
            _ohos_bluetooth_1.off('sppRead', this.clientNumber, this.dataRead.bind(this));
        });
        let earlierCreatedChild_14 = this.findChildById("14");
        if (earlierCreatedChild_14 == undefined) {
            View.create(new titleComponent_1.TitleComponent("14", this, {
                title: "spp_server_readOff", fontSize: '15vp',
                bgColor: this.currentClick === 11 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_14.updateWithValueParams({
                title: "spp_server_readOff", fontSize: '15vp',
                bgColor: this.currentClick === 11 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_14);
        }
        //服务端读操作关闭
        ListItem.pop();
        //关闭服务端serversocket
        ListItem.create();
        //关闭服务端serversocket
        ListItem.padding({ top: 20 });
        //关闭服务端serversocket
        ListItem.height(80);
        //关闭服务端serversocket
        ListItem.borderRadius(10);
        //关闭服务端serversocket
        ListItem.onClick(() => {
            LogUtil_1.default.info(MODEL + 'sppCloseServerSocket serverNumber: ' + this.serverNumber);
            _ohos_bluetooth_1.sppCloseServerSocket(this.serverNumber);
            this.message = "关闭服务端";
        });
        let earlierCreatedChild_15 = this.findChildById("15");
        if (earlierCreatedChild_15 == undefined) {
            View.create(new titleComponent_1.TitleComponent("15", this, {
                title: "spp_server_close", fontSize: '15vp',
                bgColor: this.currentClick === 12 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_15.updateWithValueParams({
                title: "spp_server_close", fontSize: '15vp',
                bgColor: this.currentClick === 12 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_15);
        }
        //关闭服务端serversocket
        ListItem.pop();
        //关闭服务端accept回调的clientsocket
        ListItem.create();
        //关闭服务端accept回调的clientsocket
        ListItem.padding({ top: 20 });
        //关闭服务端accept回调的clientsocket
        ListItem.height(80);
        //关闭服务端accept回调的clientsocket
        ListItem.borderRadius(10);
        //关闭服务端accept回调的clientsocket
        ListItem.onClick(() => {
            LogUtil_1.default.info(MODEL + 'sppCloseClientSocket serverNumber: ' + this.clientNumber);
            _ohos_bluetooth_1.sppCloseClientSocket(this.clientNumber);
            this.message = "关闭client端";
        });
        let earlierCreatedChild_16 = this.findChildById("16");
        if (earlierCreatedChild_16 == undefined) {
            View.create(new titleComponent_1.TitleComponent("16", this, {
                title: "spp_client_close", fontSize: '15vp',
                bgColor: this.currentClick === 13 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_16.updateWithValueParams({
                title: "spp_client_close", fontSize: '15vp',
                bgColor: this.currentClick === 13 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_16);
        }
        //关闭服务端accept回调的clientsocket
        ListItem.pop();
        List.pop();
        Column.pop();
        Scroll.pop();
        Column.pop();
        Column.pop();
        Row.pop();
    }
    /**
    * 字符串转arrayBuffer
    * @param {*} str 需要转换的字符串
    * @returns 返回 arrayBuffer
    */
    strToArrayBuffer(str) {
        let array = new Uint8Array(str.length);
        for (var i = 0; i < str.length; i++) {
            array[i] = str.charCodeAt(i);
        }
        return array.buffer;
    }
    /**
    * Pair device
    * @param device
    */
    pairDevice(deviceId) {
        this.controller.pair(this.deviceId, (pinCode) => {
            LogUtil_1.default.info(this.TAG_PAGE + 'pairDevice success callback : pinCode = ' + pinCode);
            // show pair pin dialog
            this.pairPinCode = pinCode;
            this.pairingDevice.deviceId = this.deviceId;
            this.pairingDevice.deviceName = this.deviceId;
            this.pairDialog.open();
        }, () => {
            LogUtil_1.default.info(this.TAG_PAGE + 'pairDevice error callback');
            this.showPairFailedDialog();
        });
    }
    /**
    * Confirm pairing
    */
    confirmPairing(accept) {
        LogUtil_1.default.info(this.TAG_PAGE + 'confirmPairing : pairingDevice + ' + JSON.stringify(this.pairingDevice));
        this.controller.confirmPairing(this.pairingDevice.deviceId, accept);
    }
    /**
    * Show pair failed dialog
    */
    showPairFailedDialog() {
        this.showPairingFailedDialog();
    }
    /**
    * Show device illegal prompt dialog
    */
    showDeviceIllegalPromptDialog(action) {
        AlertDialog.show({
            message: { "id": 16777226, "type": 10003, params: [] },
            primaryButton: {
                value: { "id": 16777250, "type": 10003, params: [] },
                action: () => {
                    LogUtil_1.default.info(this.TAG_PAGE + 'Closed callbacks');
                }
            },
            secondaryButton: {
                value: { "id": 16777234, "type": 10003, params: [] },
                action: () => {
                    action();
                    LogUtil_1.default.info(this.TAG_PAGE + `AlertDialog success:`);
                }
            },
            alignment: DialogAlignment.Bottom
        });
    }
    /**
     * Show connect Failed Dialog
     */
    showConnectFailedDialog() {
        showDialog({ "id": 16777228, "type": 10003, params: [] }, { "id": 16777229, "type": 10003, params: [] }, { "id": 16777233, "type": 10003, params: [] });
    }
    /**
     * Show pairing failed title Dialog
     */
    showPairingFailedDialog() {
        showDialog({ "id": 16777263, "type": 10003, params: [] }, { "id": 16777262, "type": 10003, params: [] }, { "id": 16777233, "type": 10003, params: [] });
    }
    dataRead(dataBuffer) {
        const data = new Uint8Array(dataBuffer);
        console.log('spp bluetooth read is: ' + data);
        if (data != null) {
            this.dataReadMessage = "读取成功" + JSON.stringify(data);
        }
        else {
            this.dataReadMessage = "读取失败";
        }
    }
}
class PairDialog extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.dialogController = undefined;
        this.action = undefined;
        this.deviceName = undefined;
        this.pinCode = undefined;
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.dialogController !== undefined) {
            this.dialogController = params.dialogController;
        }
        if (params.action !== undefined) {
            this.action = params.action;
        }
        if (params.deviceName !== undefined) {
            this.deviceName = params.deviceName;
        }
        if (params.pinCode !== undefined) {
            this.pinCode = params.pinCode;
        }
    }
    aboutToBeDeleted() {
        SubscriberManager.Get().delete(this.id());
    }
    setController(ctr) {
        this.dialogController = ctr;
    }
    aboutToAppear() {
        LogUtil_1.default.log(ConfigData_1.default.TAG + `bluetooth PairDialog aboutToAppear pinCode = ${this.pinCode}`);
    }
    render() {
        Column.create();
        Column.padding({
            left: { "id": 16777365, "type": 10002, params: [] },
            right: { "id": 16777365, "type": 10002, params: [] }
        });
        Column.width(ConfigData_1.default.WH_100_100);
        Text.create({ "id": 16777238, "type": 10003, params: [] });
        Text.fontSize({ "id": 16777386, "type": 10002, params: [] });
        Text.height({ "id": 16777511, "type": 10002, params: [] });
        Text.width(ConfigData_1.default.WH_100_100);
        Text.padding({
            left: { "id": 16777365, "type": 10002, params: [] },
            top: { "id": 16777354, "type": 10002, params: [] },
            bottom: { "id": 16777354, "type": 10002, params: [] }
        });
        Text.pop();
        Column.create();
        Column.width(ConfigData_1.default.WH_100_100);
        Column.padding({
            left: { "id": 16777365, "type": 10002, params: [] },
            right: { "id": 16777365, "type": 10002, params: [] }
        });
        If.create();
        if (this.pinCode) {
            If.branchId(0);
            Text.create({ "id": 16777236, "type": 10003, params: [this.deviceName, this.deviceName] });
            Text.fontSize({ "id": 16777382, "type": 10002, params: [] });
            Text.width(ConfigData_1.default.WH_100_100);
            Text.margin({
                top: { "id": 16777354, "type": 10002, params: [] },
                bottom: { "id": 16777354, "type": 10002, params: [] }
            });
            Text.pop();
            Text.create(`${this.pinCode}`);
            Text.fontSize({ "id": 16777416, "type": 10002, params: [] });
            Text.fontWeight(FontWeight.Bolder);
            Text.width(ConfigData_1.default.WH_100_100);
            Text.textAlign(TextAlign.Center);
            Text.margin({
                top: { "id": 16777393, "type": 10002, params: [] },
                bottom: { "id": 16777393, "type": 10002, params: [] }
            });
            Text.pop();
        }
        else {
            If.branchId(1);
            Text.create({ "id": 16777237, "type": 10003, params: [] });
            Text.fontSize({ "id": 16777382, "type": 10002, params: [] });
            Text.width(ConfigData_1.default.WH_100_100);
            Text.margin({
                bottom: { "id": 16777448, "type": 10002, params: [] }
            });
            Text.pop();
            Text.create(this.deviceName);
            Text.fontSize({ "id": 16777382, "type": 10002, params: [] });
            Text.width(ConfigData_1.default.WH_100_100);
            Text.fontWeight(FontWeight.Bold);
            Text.pop();
        }
        If.pop();
        // button
        Flex.create({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center });
        // button
        Flex.width(ConfigData_1.default.WH_100_100);
        // button
        Flex.height({ "id": 16777511, "type": 10002, params: [] });
        Button.createWithChild();
        Button.backgroundColor({ "id": 16777306, "type": 10001, params: [] });
        Button.width({ "id": 16777471, "type": 10002, params: [] });
        Button.height({ "id": 16777501, "type": 10002, params: [] });
        Button.flexGrow(1);
        Button.onClick(() => {
            this.dialogController.close();
            this.action(false);
        });
        Text.create({ "id": 16777250, "type": 10003, params: [] });
        Text.fontSize({ "id": 16777382, "type": 10002, params: [] });
        Text.fontColor(Color.Blue);
        Text.pop();
        Button.pop();
        Divider.create();
        Divider.height({ "id": 16777482, "type": 10002, params: [] });
        Divider.strokeWidth(0.5);
        Divider.vertical(true);
        Divider.color(({ "id": 16777288, "type": 10001, params: [] }));
        Button.createWithChild();
        Button.backgroundColor({ "id": 16777306, "type": 10001, params: [] });
        Button.width({ "id": 16777471, "type": 10002, params: [] });
        Button.height({ "id": 16777501, "type": 10002, params: [] });
        Button.flexGrow(1);
        Button.onClick(() => {
            this.dialogController.close();
            this.action(true);
        });
        Text.create({ "id": 16777234, "type": 10003, params: [] });
        Text.fontSize({ "id": 16777382, "type": 10002, params: [] });
        Text.fontColor(Color.Blue);
        Text.pop();
        Button.pop();
        // button
        Flex.pop();
        Column.pop();
        Column.pop();
    }
}
/**
 * Pair mode prompt
 * @param dialogTitle Dialog title
 * @param dialogMessage Dialog message
 * @param buttonValue Dialog buttonValue
 */
function showDialog(dialogTitle, dialogMessage, buttonValue) {
    AlertDialog.show({
        title: dialogTitle,
        message: dialogMessage,
        confirm: {
            value: buttonValue,
            action: () => {
                LogUtil_1.default.info('Button-clicking callback');
            }
        },
        cancel: () => {
            LogUtil_1.default.info('Closed callbacks');
        },
        alignment: DialogAlignment.Bottom
    });
}
loadDocument(new SppServer("1", undefined, {}));


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
/******/ 			"./pages/subManualApiTest/subSppTest/sppServerTest": 0
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
/******/ 	var __webpack_exports__ = __webpack_require__.O(undefined, ["commons"], () => (__webpack_require__("../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/pages/subManualApiTest/subSppTest/sppServerTest.ets?entry")))
/******/ 	__webpack_exports__ = __webpack_require__.O(__webpack_exports__);
/******/ 	_2a33209e6af7922da1521a8a828bfbd0 = __webpack_exports__;
/******/ 	
/******/ })()
;
//# sourceMappingURL=sppServerTest.js.map