var _2a33209e6af7922da1521a8a828bfbd0;
/******/ (() => { // webpackBootstrap
/******/ 	"use strict";
/******/ 	var __webpack_modules__ = ({

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/pages/subManualApiTest/subBrTest/deviceFound.ets?entry":
/*!***********************************************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/pages/subManualApiTest/subBrTest/deviceFound.ets?entry ***!
  \***********************************************************************************************************************************************************/
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
const ConfigData_1 = __importDefault(__webpack_require__(/*! ../../../../Utils/ConfigData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Utils/ConfigData.ts"));
__webpack_require__(/*! ../../../../component/headcomponent */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/headcomponent.ets");
const EntryComponent_1 = __importDefault(__webpack_require__(/*! ../../../../component/EntryComponent */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/EntryComponent.ets"));
const LogUtil_1 = __importDefault(__webpack_require__(/*! ../../../../Utils/LogUtil */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Utils/LogUtil.ts"));
const BluetoothDeviceController_1 = __importDefault(__webpack_require__(/*! ../../../controller/BluetoothDeviceController */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/controller/BluetoothDeviceController.ts"));
__webpack_require__(/*! ../../../model/BluetoothDevice */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/BluetoothDevice.ts");
var _system_router_1  = globalThis.requireNativeModule('system.router');
const BluetoothModel_1 = __webpack_require__(/*! ../../../model/BluetoothModel */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/BluetoothModel.ts");
const ImageAnimatorComponent_1 = __importDefault(__webpack_require__(/*! ../../../../component/ImageAnimatorComponent */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/ImageAnimatorComponent.ets"));
const pageTitle_1 = __webpack_require__(/*! ../../../../Component/pageTitle */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Component/pageTitle.ets");
/**
 * BR Test Page Of Bluetooth test
 */
const PAIRED_ITEM_NUMBER = 3;
const PAGE_URI_BLUETOOTH_PAIRED_DEVICE_INFO = 'pages/subManualApiTest/subBrTest/BluetoothPairedDeviceInfo';
class deviceFound extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.PAGE_TAG = ConfigData_1.default.TAG + 'Bluetooth page ';
        this.deviceController = new BluetoothDeviceController_1.default();
        this.__message = new ObservedPropertySimple('BrTest', this, "message");
        this.__bluetoothSwitch = new ObservedPropertySimple(false, this, "bluetoothSwitch");
        this.__scanSwitch = new ObservedPropertySimple(false, this, "scanSwitch");
        this.__isOn = AppStorage.GetOrCreate().setAndLink("bluetoothIsOn", false, this);
        this.__enabled = AppStorage.GetOrCreate().setAndLink("bluetoothToggleEnabled", true, this);
        this.__localName = AppStorage.GetOrCreate().setAndLink("bluetoothLocalName", '', this);
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.PAGE_TAG !== undefined) {
            this.PAGE_TAG = params.PAGE_TAG;
        }
        if (params.deviceController !== undefined) {
            this.deviceController = params.deviceController;
        }
        if (params.message !== undefined) {
            this.message = params.message;
        }
        if (params.bluetoothSwitch !== undefined) {
            this.bluetoothSwitch = params.bluetoothSwitch;
        }
        if (params.scanSwitch !== undefined) {
            this.scanSwitch = params.scanSwitch;
        }
    }
    aboutToBeDeleted() {
        this.__message.aboutToBeDeleted();
        this.__bluetoothSwitch.aboutToBeDeleted();
        this.__scanSwitch.aboutToBeDeleted();
        this.__isOn.aboutToBeDeleted();
        this.__enabled.aboutToBeDeleted();
        this.__localName.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id());
    }
    get message() {
        return this.__message.get();
    }
    set message(newValue) {
        this.__message.set(newValue);
    }
    get bluetoothSwitch() {
        return this.__bluetoothSwitch.get();
    }
    set bluetoothSwitch(newValue) {
        this.__bluetoothSwitch.set(newValue);
    }
    get scanSwitch() {
        return this.__scanSwitch.get();
    }
    set scanSwitch(newValue) {
        this.__scanSwitch.set(newValue);
    }
    get isOn() {
        return this.__isOn.get();
    }
    set isOn(newValue) {
        this.__isOn.set(newValue);
    }
    get enabled() {
        return this.__enabled.get();
    }
    set enabled(newValue) {
        this.__enabled.set(newValue);
    }
    get localName() {
        return this.__localName.get();
    }
    set localName(newValue) {
        this.__localName.set(newValue);
    }
    aboutToAppear() {
        LogUtil_1.default.log(this.PAGE_TAG + 'aboutToAppear in : isOn = ' + this.isOn);
        this.deviceController
            .initData()
            .subscribe();
        LogUtil_1.default.log(this.PAGE_TAG + 'aboutToAppear out : isOn = ' + this.isOn);
    }
    onPageShow() {
        LogUtil_1.default.log(this.PAGE_TAG + 'onPageShow in : localName = ' + this.localName);
        this.deviceController.getLocalName();
        AppStorage.SetOrCreate('btDeviceController', this.deviceController);
        LogUtil_1.default.log(this.PAGE_TAG + 'onPageShow out : localName = ' + this.localName);
    }
    //  aboutToDisappear(): void {
    //    this.deviceController.unsubscribe();
    //  }
    render() {
        Column.create();
        Column.backgroundColor({ "id": 125829132, "type": 10001, params: [] });
        Column.width(ConfigData_1.default.WH_100_100);
        Column.height(ConfigData_1.default.WH_100_100);
        GridContainer.create({ columns: 12, sizeType: SizeType.Auto, gutter: vp2px(1) === 2 ? '12vp' : '0vp', margin: vp2px(1) === 2 ? '24vp' : '0vp' });
        GridContainer.width(ConfigData_1.default.WH_100_100);
        GridContainer.height(ConfigData_1.default.WH_100_100);
        Row.create({});
        Row.width(ConfigData_1.default.WH_100_100);
        Row.height(ConfigData_1.default.WH_100_100);
        Column.create();
        Column.width(ConfigData_1.default.WH_100_100);
        Column.height(ConfigData_1.default.WH_100_100);
        Column.useSizeType({
            xs: { span: 0, offset: 0 }, sm: { span: 0, offset: 0 },
            md: { span: 0, offset: 0 }, lg: { span: 2, offset: 0 }
        });
        Column.pop();
        Column.create({ space: '12vp' });
        Column.padding({
            left: { "id": 16777365, "type": 10002, params: [] },
            right: { "id": 16777365, "type": 10002, params: [] },
            bottom: { "id": 16777368, "type": 10002, params: [] }
        });
        Column.width(ConfigData_1.default.WH_100_100);
        Column.height(ConfigData_1.default.WH_100_100);
        Column.useSizeType({
            xs: { span: 12, offset: 0 }, sm: { span: 12, offset: 0 },
            md: { span: 12, offset: 0 }, lg: { span: 8, offset: 2 }
        });
        let earlierCreatedChild_2 = this.findChildById("2");
        if (earlierCreatedChild_2 == undefined) {
            View.create(new pageTitle_1.PageTitle("2", this, { detail: '扫描蓝牙设备' }));
        }
        else {
            earlierCreatedChild_2.updateWithValueParams({
                detail: '扫描蓝牙设备'
            });
            if (!earlierCreatedChild_2.needsUpdate()) {
                earlierCreatedChild_2.markStatic();
            }
            View.create(earlierCreatedChild_2);
        }
        //            HeadComponent({ headName: '扫描蓝牙设备', isActive: true });
        Text.create({ "id": 16777244, "type": 10003, params: [] });
        //            HeadComponent({ headName: '扫描蓝牙设备', isActive: true });
        Text.width(ConfigData_1.default.WH_100_100);
        //            HeadComponent({ headName: '扫描蓝牙设备', isActive: true });
        Text.fontSize({ "id": 16777381, "type": 10002, params: [] });
        //            HeadComponent({ headName: '扫描蓝牙设备', isActive: true });
        Text.fontColor({ "id": 16777292, "type": 10001, params: [] });
        //            HeadComponent({ headName: '扫描蓝牙设备', isActive: true });
        Text.visibility(this.isOn ? Visibility.Visible : Visibility.None);
        //            HeadComponent({ headName: '扫描蓝牙设备', isActive: true });
        Text.padding({
            left: { "id": 16777365, "type": 10002, params: [] },
            top: { "id": 16777374, "type": 10002, params: [] },
            bottom: { "id": 125829737, "type": 10002, params: [] }
        });
        //            HeadComponent({ headName: '扫描蓝牙设备', isActive: true });
        Text.pop();
        Scroll.create();
        Scroll.width(ConfigData_1.default.WH_100_100);
        Scroll.align(Alignment.TopStart);
        Scroll.layoutWeight(1);
        Scroll.margin({ top: { "id": 125829737, "type": 10002, params: [] } });
        Column.create();
        Column.width(ConfigData_1.default.WH_100_100);
        Column.margin({ bottom: { "id": 125829729, "type": 10002, params: [] } });
        let earlierCreatedChild_3 = this.findChildById("3");
        if (earlierCreatedChild_3 == undefined) {
            View.create(new DeviceNameComponent("3", this, {
                enabled: this.__isOn,
                localName: this.__localName
            }));
        }
        else {
            earlierCreatedChild_3.updateWithValueParams({});
            View.create(earlierCreatedChild_3);
        }
        If.create();
        if (this.isOn) {
            If.branchId(0);
            let earlierCreatedChild_4 = this.findChildById("4");
            if (earlierCreatedChild_4 == undefined) {
                View.create(new PairedDeviceComponent("4", this, {
                    controller: this.deviceController
                }));
            }
            else {
                earlierCreatedChild_4.updateWithValueParams({
                    controller: this.deviceController
                });
                View.create(earlierCreatedChild_4);
            }
            let earlierCreatedChild_5 = this.findChildById("5");
            if (earlierCreatedChild_5 == undefined) {
                View.create(new AvailableDeviceComponent("5", this, {
                    controller: this.deviceController
                }));
            }
            else {
                earlierCreatedChild_5.updateWithValueParams({
                    controller: this.deviceController
                });
                View.create(earlierCreatedChild_5);
            }
        }
        If.pop();
        Column.pop();
        Scroll.pop();
        Column.pop();
        Column.create();
        Column.width(ConfigData_1.default.WH_100_100);
        Column.height(ConfigData_1.default.WH_100_100);
        Column.useSizeType({
            xs: { span: 0, offset: 12 }, sm: { span: 0, offset: 12 },
            md: { span: 0, offset: 12 }, lg: { span: 2, offset: 10 }
        });
        Column.pop();
        Row.pop();
        GridContainer.pop();
        Column.pop();
    }
}
class DeviceNameComponent extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.__enabled = new SynchedPropertySimpleTwoWay(params.enabled, this, "enabled");
        this.__localName = new SynchedPropertySimpleTwoWay(params.localName, this, "localName");
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
    }
    aboutToBeDeleted() {
        this.__enabled.aboutToBeDeleted();
        this.__localName.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id());
    }
    get enabled() {
        return this.__enabled.get();
    }
    set enabled(newValue) {
        this.__enabled.set(newValue);
    }
    get localName() {
        return this.__localName.get();
    }
    set localName(newValue) {
        this.__localName.set(newValue);
    }
    render() {
        Row.create();
        Row.width(ConfigData_1.default.WH_100_100);
        Row.height({ "id": 16777509, "type": 10002, params: [] });
        Row.borderRadius({ "id": 16777421, "type": 10002, params: [] });
        Row.padding({ "id": 16777370, "type": 10002, params: [] });
        Row.backgroundColor({ "id": 16777306, "type": 10001, params: [] });
        let earlierCreatedChild_6 = this.findChildById("6");
        if (earlierCreatedChild_6 == undefined) {
            View.create(new EntryComponent_1.default("6", this, {
                settingIcon: '',
                settingTitle: { "id": 16777230, "type": 10003, params: [] },
                settingSummary: null,
                settingValue: this.localName,
                settingArrow: { "id": 16777546, "type": 20000, params: [] },
                settingArrowStyle: '',
                settingUri: '',
                enabled: this.enabled,
                height: ({ "id": 16777505, "type": 10002, params: [] }),
                fontSize: ({ "id": 16777382, "type": 10002, params: [] })
            }));
        }
        else {
            earlierCreatedChild_6.updateWithValueParams({
                settingIcon: '',
                settingTitle: { "id": 16777230, "type": 10003, params: [] },
                settingSummary: null,
                settingValue: this.localName,
                settingArrow: { "id": 16777546, "type": 20000, params: [] },
                settingArrowStyle: '',
                settingUri: '',
                enabled: this.enabled,
                height: ({ "id": 16777505, "type": 10002, params: [] }),
                fontSize: ({ "id": 16777382, "type": 10002, params: [] })
            });
            View.create(earlierCreatedChild_6);
        }
        Row.pop();
    }
}
class PairedDeviceComponent extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.TAG_PAGE = ConfigData_1.default.TAG + 'PairedDeviceComponent ';
        this.controller = undefined;
        this.__pairedDevices = AppStorage.GetOrCreate().setAndLink("bluetoothPairedDevices", [], this);
        this.__isTouched = new ObservedPropertySimple(false, this, "isTouched");
        this.__hide = new ObservedPropertySimple(true, this, "hide");
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.TAG_PAGE !== undefined) {
            this.TAG_PAGE = params.TAG_PAGE;
        }
        if (params.controller !== undefined) {
            this.controller = params.controller;
        }
        if (params.isTouched !== undefined) {
            this.isTouched = params.isTouched;
        }
        if (params.hide !== undefined) {
            this.hide = params.hide;
        }
    }
    aboutToBeDeleted() {
        this.__pairedDevices.aboutToBeDeleted();
        this.__isTouched.aboutToBeDeleted();
        this.__hide.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id());
    }
    get pairedDevices() {
        return this.__pairedDevices.get();
    }
    set pairedDevices(newValue) {
        this.__pairedDevices.set(newValue);
    }
    get isTouched() {
        return this.__isTouched.get();
    }
    set isTouched(newValue) {
        this.__isTouched.set(newValue);
    }
    get hide() {
        return this.__hide.get();
    }
    set hide(newValue) {
        this.__hide.set(newValue);
    }
    aboutToAppear() {
        if (this.controller) {
            // bind component and initialize
            this.controller.bindComponent(this)
                .bindProperties(["pairedDevices"])
                .initData();
        }
    }
    render() {
        Column.create();
        If.create();
        if (this.pairedDevices && this.pairedDevices.length > 0) {
            If.branchId(0);
            // paired devices title
            Row.create();
            // paired devices title
            Row.width(ConfigData_1.default.WH_100_100);
            // paired devices title
            Row.padding({
                left: { "id": 16777365, "type": 10002, params: [] },
                top: { "id": 16777360, "type": 10002, params: [] },
                bottom: { "id": 16777378, "type": 10002, params: [] }
            });
            Text.create({ "id": 16777235, "type": 10003, params: [] });
            Text.fontSize({ "id": 16777381, "type": 10002, params: [] });
            Text.fontColor({ "id": 16777292, "type": 10001, params: [] });
            Text.pop();
            // paired devices title
            Row.pop();
            List.create();
            List.divider({
                strokeWidth: 1,
                color: { "id": 16777288, "type": 10001, params: [] },
                startMargin: { "id": 16777508, "type": 10002, params: [] },
                endMargin: { "id": 16777465, "type": 10002, params: [] }
            });
            List.backgroundColor({ "id": 16777306, "type": 10001, params: [] });
            List.borderRadius({ "id": 16777421, "type": 10002, params: [] });
            // paired devices list
            ForEach.create("8", this, ObservedObject.GetRawObject(this.pairedDevices), (item, index) => {
                If.create();
                if (index < PAIRED_ITEM_NUMBER || !this.hide) {
                    If.branchId(0);
                    ListItem.create();
                    Row.create();
                    Row.width(ConfigData_1.default.WH_100_100);
                    Row.borderRadius({ "id": 16777421, "type": 10002, params: [] });
                    Row.padding({ "id": 16777370, "type": 10002, params: [] });
                    Row.backgroundColor({ "id": 16777306, "type": 10001, params: [] });
                    Row.onClick(() => {
                        this.itemClicked(item);
                    });
                    let earlierCreatedChild_7 = this.findChildById("7");
                    if (earlierCreatedChild_7 == undefined) {
                        View.create(new EntryComponent_1.default("7", this, {
                            settingIcon: getDeviceIconPath(item.deviceType),
                            settingTitle: item.deviceName,
                            settingSummary: getConnectionStateText(item),
                            settingValue: '',
                            settingArrow: { "id": 16777546, "type": 20000, params: [] },
                            settingArrowStyle: 'bluetooth',
                            settingUri: '',
                            titleFontColor: this.isHeadPhoneConnected(item) ? { "id": 16777282, "type": 10001, params: [] } : { "id": 125829210, "type": 10001, params: [] },
                            image_wh: { "id": 16777482, "type": 10002, params: [] },
                            height: getConnectionStateText(item) == null ? { "id": 16777509, "type": 10002, params: [] } : ({ "id": 16777513, "type": 10002, params: [] }),
                            fontSize: ({ "id": 16777382, "type": 10002, params: [] }),
                            onArrowClick: () => {
                                LogUtil_1.default.info(this.TAG_PAGE + 'item go detail : ' + JSON.stringify(item));
                                this.gotoPairedDeviceInfo(item);
                            }
                        }));
                    }
                    else {
                        earlierCreatedChild_7.updateWithValueParams({
                            settingIcon: getDeviceIconPath(item.deviceType),
                            settingTitle: item.deviceName,
                            settingSummary: getConnectionStateText(item),
                            settingValue: '',
                            settingArrow: { "id": 16777546, "type": 20000, params: [] },
                            settingArrowStyle: 'bluetooth',
                            settingUri: '',
                            titleFontColor: this.isHeadPhoneConnected(item) ? { "id": 16777282, "type": 10001, params: [] } : { "id": 125829210, "type": 10001, params: [] },
                            image_wh: { "id": 16777482, "type": 10002, params: [] },
                            height: getConnectionStateText(item) == null ? { "id": 16777509, "type": 10002, params: [] } : ({ "id": 16777513, "type": 10002, params: [] }),
                            fontSize: ({ "id": 16777382, "type": 10002, params: [] }),
                            onArrowClick: () => {
                                LogUtil_1.default.info(this.TAG_PAGE + 'item go detail : ' + JSON.stringify(item));
                                this.gotoPairedDeviceInfo(item);
                            }
                        });
                        View.create(earlierCreatedChild_7);
                    }
                    Row.pop();
                    ListItem.pop();
                }
                If.pop();
                If.create();
                if ((this.hide && index === PAIRED_ITEM_NUMBER) || //more
                    (!this.hide && index >= PAIRED_ITEM_NUMBER && index == this.pairedDevices.length - 1)) { //put_away
                    If.branchId(0);
                    ListItem.create();
                    Stack.create({ alignContent: Alignment.Center });
                    Stack.height({ "id": 16777505, "type": 10002, params: [] });
                    Stack.backgroundColor({ "id": 125829123, "type": 10001, params: [] });
                    Stack.padding({ "id": 16777370, "type": 10002, params: [] });
                    Stack.create({ alignContent: Alignment.Center });
                    Stack.width(ConfigData_1.default.WH_100_100);
                    Stack.height({ "id": 16777505, "type": 10002, params: [] });
                    Stack.borderRadius({ "id": 16777420, "type": 10002, params: [] });
                    Stack.backgroundColor(this.isTouched ? { "id": 16777287, "type": 10001, params: [] } : { "id": 125829123, "type": 10001, params: [] });
                    Stack.onTouch((event) => {
                        if (event.type === TouchType.Down) {
                            this.isTouched = true;
                        }
                        if (event.type === TouchType.Up) {
                            this.isTouched = false;
                        }
                    });
                    Stack.onClick(() => {
                        this.hide = !this.hide;
                    });
                    Text.create(this.hide ? { "id": 16777261, "type": 10003, params: [] } : { "id": 16777266, "type": 10003, params: [] });
                    Text.fontColor({ "id": 16777287, "type": 10001, params: [] });
                    Text.fontSize({ "id": 16777381, "type": 10002, params: [] });
                    Text.pop();
                    Stack.pop();
                    Stack.pop();
                    ListItem.pop();
                }
                If.pop();
            }, item => JSON.stringify(item));
            // paired devices list
            ForEach.pop();
            List.pop();
        }
        If.pop();
        Column.pop();
    }
    /**
     * Whether headphone connected.
     * @param item device
     * @return headphone connected or not
     */
    isHeadPhoneConnected(item) {
        return item.deviceType === BluetoothModel_1.DeviceType.HEADPHONE && item.connectionState === BluetoothModel_1.DeviceState.STATE_CONNECTED;
    }
    /**
     * Goto paired device Info
     * @param item device
     */
    gotoPairedDeviceInfo(item) {
        if (item.connectionState != BluetoothModel_1.DeviceState.STATE_CONNECTING
            && item.connectionState != BluetoothModel_1.DeviceState.STATE_DISCONNECTING) {
            LogUtil_1.default.info(this.TAG_PAGE + 'item right icon on click : ' + JSON.stringify(item));
            let param = { 'bluetoothDevice': JSON.stringify(item) };
            _system_router_1.push({
                uri: PAGE_URI_BLUETOOTH_PAIRED_DEVICE_INFO,
                params: param
            });
        }
    }
    /**
     * Item clicked
     * @param item device
     */
    itemClicked(item) {
        switch (item.connectionState) {
            case BluetoothModel_1.DeviceState.STATE_CONNECTED:
                this.showDisconnectDialog(item.deviceName, () => {
                    this.controller.disconnect(item.deviceId);
                });
                break;
            case BluetoothModel_1.DeviceState.STATE_DISCONNECTED:
                if (!this.controller.connect(item.deviceId)) {
                    this.showConnectFailedDialog();
                }
                break;
        }
    }
    /**
     * Connect Failed Dialog
     */
    showConnectFailedDialog() {
        showDialog({ "id": 16777228, "type": 10003, params: [] }, { "id": 16777229, "type": 10003, params: [] }, { "id": 16777233, "type": 10003, params: [] });
    }
    /**
     * Disconnect Dialog
     */
    showDisconnectDialog(deviceName, callback) {
        AlertDialog.show({
            title: { "id": 16777231, "type": 10003, params: [] },
            message: { "id": 16777232, "type": 10003, params: [deviceName] },
            primaryButton: {
                value: { "id": 16777250, "type": 10003, params: [] },
                action: () => {
                    LogUtil_1.default.info(ConfigData_1.default.TAG + 'Closed callbacks');
                }
            },
            secondaryButton: {
                value: { "id": 16777253, "type": 10003, params: [] },
                action: () => {
                    LogUtil_1.default.info(ConfigData_1.default.TAG + `AlertDialog success:`);
                    callback();
                }
            },
            alignment: DialogAlignment.Bottom
        });
    }
}
class DiscoveringAnimatorComponent extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
    }
    aboutToBeDeleted() {
        SubscriberManager.Get().delete(this.id());
    }
    render() {
        Column.create();
        Row.create();
        let earlierCreatedChild_9 = this.findChildById("9");
        if (earlierCreatedChild_9 == undefined) {
            View.create(new ImageAnimatorComponent_1.default("9", this, {
                imageWidth: { "id": 16777501, "type": 10002, params: [] },
                imageHeight: { "id": 16777501, "type": 10002, params: [] }
            }));
        }
        else {
            earlierCreatedChild_9.updateWithValueParams({
                imageWidth: { "id": 16777501, "type": 10002, params: [] },
                imageHeight: { "id": 16777501, "type": 10002, params: [] }
            });
            if (!earlierCreatedChild_9.needsUpdate()) {
                earlierCreatedChild_9.markStatic();
            }
            View.create(earlierCreatedChild_9);
        }
        Row.pop();
        Column.pop();
    }
}
class AvailableDeviceComponent extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.TAG_PAGE = ConfigData_1.default.TAG + 'AvailableDeviceComponent ';
        this.controller = undefined;
        this.__isDeviceDiscovering = new ObservedPropertySimple(false, this, "isDeviceDiscovering");
        this.__availableDevices = AppStorage.GetOrCreate().setAndLink("bluetoothAvailableDevices", [], this);
        this.__pairPinCode = new ObservedPropertySimple('', this, "pairPinCode");
        this.pairingDevice = undefined;
        this.__pairDeviceInfo = new ObservedPropertySimple('未配对', this, "pairDeviceInfo");
        this.pairDialog = new CustomDialogController({
            builder: () => {
                let jsDialog = new PairDialog("13", this, {
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
        if (params.TAG_PAGE !== undefined) {
            this.TAG_PAGE = params.TAG_PAGE;
        }
        if (params.controller !== undefined) {
            this.controller = params.controller;
        }
        if (params.isDeviceDiscovering !== undefined) {
            this.isDeviceDiscovering = params.isDeviceDiscovering;
        }
        if (params.pairPinCode !== undefined) {
            this.pairPinCode = params.pairPinCode;
        }
        if (params.pairingDevice !== undefined) {
            this.pairingDevice = params.pairingDevice;
        }
        if (params.pairDeviceInfo !== undefined) {
            this.pairDeviceInfo = params.pairDeviceInfo;
        }
        if (params.pairDialog !== undefined) {
            this.pairDialog = params.pairDialog;
        }
    }
    aboutToBeDeleted() {
        this.__isDeviceDiscovering.aboutToBeDeleted();
        this.__availableDevices.aboutToBeDeleted();
        this.__pairPinCode.aboutToBeDeleted();
        this.__pairDeviceInfo.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id());
    }
    get isDeviceDiscovering() {
        return this.__isDeviceDiscovering.get();
    }
    set isDeviceDiscovering(newValue) {
        this.__isDeviceDiscovering.set(newValue);
    }
    get availableDevices() {
        return this.__availableDevices.get();
    }
    set availableDevices(newValue) {
        this.__availableDevices.set(newValue);
    }
    get pairPinCode() {
        return this.__pairPinCode.get();
    }
    set pairPinCode(newValue) {
        this.__pairPinCode.set(newValue);
    }
    get pairDeviceInfo() {
        return this.__pairDeviceInfo.get();
    }
    set pairDeviceInfo(newValue) {
        this.__pairDeviceInfo.set(newValue);
    }
    aboutToAppear() {
        if (this.controller) {
            // bind component and initialize
            this.controller.bindComponent(this)
                .bindProperties(["isDeviceDiscovering", "availableDevices", "pairPinCode"])
                .initData();
            this.controller.startBluetoothDiscovery();
        }
    }
    render() {
        Column.create();
        Row.create();
        Row.width(ConfigData_1.default.WH_100_100);
        // available devices title
        Text.create({ "id": 16777227, "type": 10003, params: [] });
        // available devices title
        Text.fontSize({ "id": 16777381, "type": 10002, params: [] });
        // available devices title
        Text.fontColor({ "id": 16777292, "type": 10001, params: [] });
        // available devices title
        Text.width(ConfigData_1.default.WH_100_100);
        // available devices title
        Text.padding({
            left: { "id": 16777365, "type": 10002, params: [] },
            top: { "id": 16777360, "type": 10002, params: [] },
            bottom: { "id": 16777378, "type": 10002, params: [] }
        });
        // available devices title
        Text.pop();
        Blank.create();
        Blank.pop();
        If.create();
        // bluetooth discovering
        if (this.isDeviceDiscovering) {
            If.branchId(0);
            let earlierCreatedChild_10 = this.findChildById("10");
            if (earlierCreatedChild_10 == undefined) {
                View.create(new DiscoveringAnimatorComponent("10", this, {}));
            }
            else {
                earlierCreatedChild_10.updateWithValueParams({});
                if (!earlierCreatedChild_10.needsUpdate()) {
                    earlierCreatedChild_10.markStatic();
                }
                View.create(earlierCreatedChild_10);
            }
        }
        If.pop();
        Row.pop();
        Row.create();
        Row.width(ConfigData_1.default.WH_100_100);
        // available devices title
        Text.create('配对状态：' + this.pairDeviceInfo);
        // available devices title
        Text.fontSize({ "id": 16777381, "type": 10002, params: [] });
        // available devices title
        Text.fontColor({ "id": 16777292, "type": 10001, params: [] });
        // available devices title
        Text.width(ConfigData_1.default.WH_100_100);
        // available devices title
        Text.padding({
            left: { "id": 16777365, "type": 10002, params: [] },
            top: { "id": 16777360, "type": 10002, params: [] },
            bottom: { "id": 16777378, "type": 10002, params: [] }
        });
        // available devices title
        Text.pop();
        Row.pop();
        If.create();
        if (this.availableDevices && this.availableDevices.length >= 1) {
            If.branchId(0);
            List.create();
            List.backgroundColor({ "id": 16777306, "type": 10001, params: [] });
            List.borderRadius({ "id": 16777421, "type": 10002, params: [] });
            List.divider({
                strokeWidth: 1,
                color: { "id": 16777288, "type": 10001, params: [] },
                startMargin: { "id": 16777508, "type": 10002, params: [] },
                endMargin: { "id": 16777465, "type": 10002, params: [] }
            });
            // paired devices list
            ForEach.create("12", this, ObservedObject.GetRawObject(this.availableDevices), (item) => {
                ListItem.create();
                Row.create();
                Row.width(ConfigData_1.default.WH_100_100);
                Row.borderRadius({ "id": 16777421, "type": 10002, params: [] });
                Row.padding({ "id": 16777370, "type": 10002, params: [] });
                Row.backgroundColor({ "id": 16777306, "type": 10001, params: [] });
                Row.onClick(() => {
                    LogUtil_1.default.info(this.TAG_PAGE + 'item on click : ' + JSON.stringify(item));
                    this.pairDevice(item);
                });
                let earlierCreatedChild_11 = this.findChildById("11");
                if (earlierCreatedChild_11 == undefined) {
                    View.create(new EntryComponent_1.default("11", this, {
                        settingIcon: getDeviceIconPath(item.deviceType),
                        settingTitle: item.deviceName ? item.deviceName : item.deviceId,
                        settingSummary: getConnectionStateText(item),
                        settingValue: '',
                        settingArrow: null,
                        settingArrowStyle: '',
                        settingUri: '',
                        image_wh: { "id": 16777482, "type": 10002, params: [] },
                        height: getConnectionStateText(item) == null ? { "id": 16777509, "type": 10002, params: [] } : ({ "id": 16777513, "type": 10002, params: [] }),
                        fontSize: ({ "id": 16777382, "type": 10002, params: [] }),
                    }));
                }
                else {
                    earlierCreatedChild_11.updateWithValueParams({
                        settingIcon: getDeviceIconPath(item.deviceType),
                        settingTitle: item.deviceName ? item.deviceName : item.deviceId,
                        settingSummary: getConnectionStateText(item),
                        settingValue: '',
                        settingArrow: null,
                        settingArrowStyle: '',
                        settingUri: '',
                        image_wh: { "id": 16777482, "type": 10002, params: [] },
                        height: getConnectionStateText(item) == null ? { "id": 16777509, "type": 10002, params: [] } : ({ "id": 16777513, "type": 10002, params: [] }),
                        fontSize: ({ "id": 16777382, "type": 10002, params: [] })
                    });
                    View.create(earlierCreatedChild_11);
                }
                Row.pop();
                ListItem.pop();
            }, item => JSON.stringify(item));
            // paired devices list
            ForEach.pop();
            List.pop();
        }
        else {
            If.branchId(1);
            Row.create();
            // Scanning...
            Text.create({ "id": 16777268, "type": 10003, params: [] });
            // Scanning...
            Text.fontSize({ "id": 16777384, "type": 10002, params: [] });
            // Scanning...
            Text.textCase(TextCase.UpperCase);
            // Scanning...
            Text.pop();
            Row.pop();
        }
        If.pop();
        Column.pop();
    }
    /**
     * Pair device
     * @param device
     */
    pairDevice(device) {
        this.controller.pair(device.deviceId, (pinCode) => {
            LogUtil_1.default.info(this.TAG_PAGE + 'pairDevice success callback : pinCode = ' + pinCode);
            // show pair pin dialog
            this.pairPinCode = pinCode;
            this.pairingDevice = device;
            AppStorage.SetOrCreate('pairedDeviceInfo', device);
            AppStorage.SetOrCreate('pairedDeviceId', device.deviceId);
            this.pairDialog.open();
            this.pairDeviceInfo = 'pairDeviceID: ' + device.deviceId + ' true';
        }, () => {
            this.pairDeviceInfo = 'pairDeviceID: ' + device.deviceId + ' false';
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
 * Get device icon resource
 * @param type
 * @return device icon path
 */
function getDeviceIconPath(deviceType) {
    let path = "/res/image/ic_bluetooth_device.svg";
    switch (deviceType) {
        case BluetoothModel_1.DeviceType.HEADPHONE:
            path = "/res/image/ic_device_earphone_hero.svg";
            break;
        case BluetoothModel_1.DeviceType.PHONE:
            path = "/res/image/ic_public_devices_phone.svg";
            break;
        case BluetoothModel_1.DeviceType.WATCH:
            path = "/res/image/ic_device_watch.svg";
            break;
        case BluetoothModel_1.DeviceType.COMPUTER:
            path = "/res/image/ic_device_matebook.svg";
            break;
    }
    return path;
}
/**
 * Get connection state text
 * @param connectionState
 */
function getConnectionStateText(device) {
    let stateText = null;
    switch (device.connectionState) {
        case BluetoothModel_1.DeviceState.STATE_DISCONNECTED:
            stateText = null;
            break;
        case BluetoothModel_1.DeviceState.STATE_CONNECTING:
            stateText = { "id": 16777241, "type": 10003, params: [] };
            break;
        case BluetoothModel_1.DeviceState.STATE_CONNECTED:
            if (device.deviceType === BluetoothModel_1.DeviceType.HEADPHONE) {
                stateText = { "id": 16777240, "type": 10003, params: [] };
            }
            else {
                stateText = null;
            }
            break;
        case BluetoothModel_1.DeviceState.STATE_DISCONNECTING:
            stateText = { "id": 16777242, "type": 10003, params: [] };
            break;
        case BluetoothModel_1.DeviceState.STATE_PAIRING:
            stateText = { "id": 16777243, "type": 10003, params: [] };
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
loadDocument(new deviceFound("1", undefined, {}));


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/ComponentConfig.ets":
/*!**********************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/ComponentConfig.ets ***!
  \**********************************************************************************************************************/
/***/ ((__unused_webpack_module, exports) => {


Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.ComponentConfig = void 0;
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
class ComponentConfig {
    constructor() {
        this.WH_100_100 = '100%';
        this.WH_30_100 = '30%';
        this.WH_33_100 = '33%';
        this.WH_35_100 = '35%';
        this.WH_40_100 = '40%';
        this.WH_45_100 = '45%';
        this.WH_50_100 = '50%';
        this.WH_55_100 = '55%';
        this.WH_83_100 = '83%';
        this.WH_90_100 = '90%';
        this.value_20 = 20;
        this.font_20 = 20;
        this.MAX_LINES_1 = 1;
        this.MAX_LINES_2 = 2;
        this.MAX_LINES_3 = 3;
        this.DURATION_TIME = 200;
    }
}
exports.ComponentConfig = ComponentConfig;
let componentConfig = new ComponentConfig();
exports["default"] = componentConfig;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/ImageAnimatorComponent.ets":
/*!*****************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/ImageAnimatorComponent.ets ***!
  \*****************************************************************************************************************************/
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
const ComponentConfig_1 = __importDefault(__webpack_require__(/*! ./ComponentConfig */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/ComponentConfig.ets"));
class ImageAnimatorComponent extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.imageWidth = 0;
        this.imageHeight = 0;
        this.whtl = ComponentConfig_1.default.value_20;
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.imageWidth !== undefined) {
            this.imageWidth = params.imageWidth;
        }
        if (params.imageHeight !== undefined) {
            this.imageHeight = params.imageHeight;
        }
        if (params.whtl !== undefined) {
            this.whtl = params.whtl;
        }
    }
    aboutToBeDeleted() {
        SubscriberManager.Get().delete(this.id());
    }
    render() {
        Column.create();
        ImageAnimator.create();
        ImageAnimator.images([
            {
                src: 'res/image/xxxhdpi/ic_loading01.png',
                duration: ComponentConfig_1.default.DURATION_TIME,
                width: this.whtl,
                height: this.whtl,
                top: this.whtl,
                left: this.whtl
            },
            {
                src: 'res/image/xxxhdpi/ic_loading02.png',
                duration: ComponentConfig_1.default.DURATION_TIME,
                width: this.whtl,
                height: this.whtl,
                top: this.whtl,
                left: this.whtl
            },
            {
                src: 'res/image/xxxhdpi/ic_loading03.png',
                duration: ComponentConfig_1.default.DURATION_TIME,
                width: this.whtl,
                height: this.whtl,
                top: this.whtl,
                left: this.whtl
            },
            {
                src: 'res/image/xxxhdpi/ic_loading04.png',
                duration: ComponentConfig_1.default.DURATION_TIME,
                width: this.whtl,
                height: this.whtl,
                top: this.whtl,
                left: this.whtl
            },
            {
                src: 'res/image/xxxhdpi/ic_loading05.png',
                duration: ComponentConfig_1.default.DURATION_TIME,
                width: this.whtl,
                height: this.whtl,
                top: this.whtl,
                left: this.whtl
            },
            {
                src: 'res/image/hdpi/ic_loading06.png',
                duration: ComponentConfig_1.default.DURATION_TIME,
                width: this.whtl,
                height: this.whtl,
                top: this.whtl,
                left: this.whtl
            }
        ]);
        ImageAnimator.width(this.imageWidth);
        ImageAnimator.height(this.imageHeight);
        ImageAnimator.iterations(-1);
        ImageAnimator.fixedSize(true);
        ImageAnimator.reverse(true);
        ImageAnimator.fillMode(FillMode.None);
        ImageAnimator.state(AnimationStatus.Running);
        Column.pop();
    }
}
exports["default"] = ImageAnimatorComponent;


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
/******/ 			"./pages/subManualApiTest/subBrTest/deviceFound": 0
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
/******/ 	var __webpack_exports__ = __webpack_require__.O(undefined, ["commons"], () => (__webpack_require__("../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/pages/subManualApiTest/subBrTest/deviceFound.ets?entry")))
/******/ 	__webpack_exports__ = __webpack_require__.O(__webpack_exports__);
/******/ 	_2a33209e6af7922da1521a8a828bfbd0 = __webpack_exports__;
/******/ 	
/******/ })()
;
//# sourceMappingURL=deviceFound.js.map