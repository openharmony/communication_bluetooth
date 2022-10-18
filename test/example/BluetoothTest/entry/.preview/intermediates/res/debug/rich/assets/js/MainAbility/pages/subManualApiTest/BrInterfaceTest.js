var _840471a2ef2afe531ee6aea9267b2e21;
/******/ (() => { // webpackBootstrap
/******/ 	"use strict";
/******/ 	var __webpack_modules__ = ({

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/pages/subManualApiTest/BrInterfaceTest.ets?entry":
/*!*********************************************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/pages/subManualApiTest/BrInterfaceTest.ets?entry ***!
  \*********************************************************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {


var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
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
// @ts-nocheck
const pageTitle_1 = __webpack_require__(/*! ../../../Component/pageTitle */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Component/pageTitle.ets");
const testImageDisplay_1 = __webpack_require__(/*! ../../../Component/testImageDisplay */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Component/testImageDisplay.ets");
const contentTable_1 = __webpack_require__(/*! ../../../Component/contentTable */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Component/contentTable.ets");
const bluetoothModel_1 = __importDefault(__webpack_require__(/*! ../../model/bluetoothModel */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/bluetoothModel.ts"));
var _system_router_1  = globalThis.requireNativeModule('system.router');
const bluetoothDevice_1 = __importDefault(__webpack_require__(/*! ../../model/bluetoothDevice */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/bluetoothDevice.ts"));
const BluetoothDeviceController_1 = __importDefault(__webpack_require__(/*! ../../controller/BluetoothDeviceController */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/controller/BluetoothDeviceController.ts"));
const ConfigData_1 = __importDefault(__webpack_require__(/*! ../../../Utils/ConfigData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Utils/ConfigData.ts"));
const LogUtil_1 = __importDefault(__webpack_require__(/*! ../../../Utils/LogUtil */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Utils/LogUtil.ts"));
const testDataModels_1 = __webpack_require__(/*! ../../../MainAbility/model/testDataModels */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/testDataModels.ets");
__webpack_require__(/*! ../../model/testData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/testData.ets");
class Index extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.TAG_PAGE = ConfigData_1.default.TAG + 'Paired_Device ';
        this.testItem = _system_router_1.getParams().testId;
        this.__message = new ObservedPropertySimple("", this, "message");
        this.__btStateMessage = new ObservedPropertySimple("", this, "btStateMessage");
        this.__pinMessage = new ObservedPropertySimple("", this, "pinMessage");
        this.__bondStateMessage = new ObservedPropertySimple("", this, "bondStateMessage");
        this.__btEnable = new ObservedPropertySimple(false, this, "btEnable");
        this.__isConnection = new ObservedPropertySimple(false, this, "isConnection");
        this.__setLocalNameRet = AppStorage.GetOrCreate().setAndLink("setLocalNameRet", false, this);
        this.__setScanModeRet = AppStorage.GetOrCreate().setAndLink("setScanModeRet", false, this);
        this.stateProfile = "";
        this.stateBT = "";
        this.__deviceId = new ObservedPropertySimple('', this, "deviceId");
        this.__currentClick = new ObservedPropertySimple(-1, this, "currentClick");
        this.__pairingDevice = AppStorage.GetOrCreate().setAndLink("pairedDeviceInfo", new bluetoothDevice_1.default, this);
        this.__pairedDeviceId = AppStorage.GetOrCreate().setAndLink("pairedDeviceId", '', this);
        this.__controller = AppStorage.GetOrCreate().setAndLink("btDeviceController", new BluetoothDeviceController_1.default, this);
        this.__pairedMac = AppStorage.GetOrCreate().setAndLink("pairedMac", "AC:1E:9E:0B:2E:63", this);
        this.__pairPinCode = new ObservedPropertySimple('', this, "pairPinCode");
        this.__btOnState = new ObservedPropertySimple('on:stateChange', this, "btOnState");
        this.__btPinRequired = new ObservedPropertySimple('on:pinRequired', this, "btPinRequired");
        this.__btBondStateChange = new ObservedPropertySimple('on:bondStateChange', this, "btBondStateChange");
        this.__isStateChangeClick = new ObservedPropertySimple(false, this, "isStateChangeClick");
        this.__isPinRequiredClick = new ObservedPropertySimple(false, this, "isPinRequiredClick");
        this.__isBondStateChangeClick = new ObservedPropertySimple(false, this, "isBondStateChangeClick");
        this.pairDialog = new CustomDialogController({
            builder: () => {
                let jsDialog = new PairDialog("5", this, {
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
        if (params.testItem !== undefined) {
            this.testItem = params.testItem;
        }
        if (params.message !== undefined) {
            this.message = params.message;
        }
        if (params.btStateMessage !== undefined) {
            this.btStateMessage = params.btStateMessage;
        }
        if (params.pinMessage !== undefined) {
            this.pinMessage = params.pinMessage;
        }
        if (params.bondStateMessage !== undefined) {
            this.bondStateMessage = params.bondStateMessage;
        }
        if (params.btEnable !== undefined) {
            this.btEnable = params.btEnable;
        }
        if (params.isConnection !== undefined) {
            this.isConnection = params.isConnection;
        }
        if (params.stateProfile !== undefined) {
            this.stateProfile = params.stateProfile;
        }
        if (params.stateBT !== undefined) {
            this.stateBT = params.stateBT;
        }
        if (params.deviceId !== undefined) {
            this.deviceId = params.deviceId;
        }
        if (params.currentClick !== undefined) {
            this.currentClick = params.currentClick;
        }
        if (params.pairPinCode !== undefined) {
            this.pairPinCode = params.pairPinCode;
        }
        if (params.btOnState !== undefined) {
            this.btOnState = params.btOnState;
        }
        if (params.btPinRequired !== undefined) {
            this.btPinRequired = params.btPinRequired;
        }
        if (params.btBondStateChange !== undefined) {
            this.btBondStateChange = params.btBondStateChange;
        }
        if (params.isStateChangeClick !== undefined) {
            this.isStateChangeClick = params.isStateChangeClick;
        }
        if (params.isPinRequiredClick !== undefined) {
            this.isPinRequiredClick = params.isPinRequiredClick;
        }
        if (params.isBondStateChangeClick !== undefined) {
            this.isBondStateChangeClick = params.isBondStateChangeClick;
        }
        if (params.pairDialog !== undefined) {
            this.pairDialog = params.pairDialog;
        }
    }
    aboutToBeDeleted() {
        this.__message.aboutToBeDeleted();
        this.__btStateMessage.aboutToBeDeleted();
        this.__pinMessage.aboutToBeDeleted();
        this.__bondStateMessage.aboutToBeDeleted();
        this.__btEnable.aboutToBeDeleted();
        this.__isConnection.aboutToBeDeleted();
        this.__setLocalNameRet.aboutToBeDeleted();
        this.__setScanModeRet.aboutToBeDeleted();
        this.__deviceId.aboutToBeDeleted();
        this.__currentClick.aboutToBeDeleted();
        this.__pairingDevice.aboutToBeDeleted();
        this.__pairedDeviceId.aboutToBeDeleted();
        this.__controller.aboutToBeDeleted();
        this.__pairedMac.aboutToBeDeleted();
        this.__pairPinCode.aboutToBeDeleted();
        this.__btOnState.aboutToBeDeleted();
        this.__btPinRequired.aboutToBeDeleted();
        this.__btBondStateChange.aboutToBeDeleted();
        this.__isStateChangeClick.aboutToBeDeleted();
        this.__isPinRequiredClick.aboutToBeDeleted();
        this.__isBondStateChangeClick.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id());
    }
    get message() {
        return this.__message.get();
    }
    set message(newValue) {
        this.__message.set(newValue);
    }
    get btStateMessage() {
        return this.__btStateMessage.get();
    }
    set btStateMessage(newValue) {
        this.__btStateMessage.set(newValue);
    }
    get pinMessage() {
        return this.__pinMessage.get();
    }
    set pinMessage(newValue) {
        this.__pinMessage.set(newValue);
    }
    get bondStateMessage() {
        return this.__bondStateMessage.get();
    }
    set bondStateMessage(newValue) {
        this.__bondStateMessage.set(newValue);
    }
    get btEnable() {
        return this.__btEnable.get();
    }
    set btEnable(newValue) {
        this.__btEnable.set(newValue);
    }
    get isConnection() {
        return this.__isConnection.get();
    }
    set isConnection(newValue) {
        this.__isConnection.set(newValue);
    }
    get setLocalNameRet() {
        return this.__setLocalNameRet.get();
    }
    set setLocalNameRet(newValue) {
        this.__setLocalNameRet.set(newValue);
    }
    get setScanModeRet() {
        return this.__setScanModeRet.get();
    }
    set setScanModeRet(newValue) {
        this.__setScanModeRet.set(newValue);
    }
    get deviceId() {
        return this.__deviceId.get();
    }
    set deviceId(newValue) {
        this.__deviceId.set(newValue);
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
    get pairedDeviceId() {
        return this.__pairedDeviceId.get();
    }
    set pairedDeviceId(newValue) {
        this.__pairedDeviceId.set(newValue);
    }
    get controller() {
        return this.__controller.get();
    }
    set controller(newValue) {
        this.__controller.set(newValue);
    }
    get pairedMac() {
        return this.__pairedMac.get();
    }
    set pairedMac(newValue) {
        this.__pairedMac.set(newValue);
    }
    get pairPinCode() {
        return this.__pairPinCode.get();
    }
    set pairPinCode(newValue) {
        this.__pairPinCode.set(newValue);
    }
    get btOnState() {
        return this.__btOnState.get();
    }
    set btOnState(newValue) {
        this.__btOnState.set(newValue);
    }
    get btPinRequired() {
        return this.__btPinRequired.get();
    }
    set btPinRequired(newValue) {
        this.__btPinRequired.set(newValue);
    }
    get btBondStateChange() {
        return this.__btBondStateChange.get();
    }
    set btBondStateChange(newValue) {
        this.__btBondStateChange.set(newValue);
    }
    get isStateChangeClick() {
        return this.__isStateChangeClick.get();
    }
    set isStateChangeClick(newValue) {
        this.__isStateChangeClick.set(newValue);
    }
    get isPinRequiredClick() {
        return this.__isPinRequiredClick.get();
    }
    set isPinRequiredClick(newValue) {
        this.__isPinRequiredClick.set(newValue);
    }
    get isBondStateChangeClick() {
        return this.__isBondStateChangeClick.get();
    }
    set isBondStateChangeClick(newValue) {
        this.__isBondStateChangeClick.set(newValue);
    }
    aboutToAppear() {
        this.btEnable = bluetoothModel_1.default.isStateOn();
        this.controller.subscribePinRequired((deviceId, pinCode) => {
            this.pairPinCode = pinCode;
            this.pairingDevice.deviceId = deviceId;
            this.pairingDevice.deviceName = deviceId;
            this.pairDialog.open();
        });
        AppStorage.SetOrCreate('bluetoothIsOn', this.btEnable);
        AppStorage.SetOrCreate('pairedMac', this.pairedMac);
    }
    init() {
        console.log("init-----------");
    }
    render() {
        Column.create();
        Column.debugLine("pages/subManualApiTest/BrInterfaceTest.ets(88:5)");
        Column.alignItems(HorizontalAlign.Center);
        Column.backgroundColor({ "id": 16777302, "type": 10001, params: [] });
        Stack.create({ alignContent: Alignment.TopStart });
        Stack.debugLine("pages/subManualApiTest/BrInterfaceTest.ets(89:7)");
        let earlierCreatedChild_2 = this.findChildById("2");
        if (earlierCreatedChild_2 == undefined) {
            View.create(new testImageDisplay_1.TestImageDisplay("2", this, { testItem: this.testItem }));
        }
        else {
            earlierCreatedChild_2.updateWithValueParams({
                testItem: this.testItem
            });
            if (!earlierCreatedChild_2.needsUpdate()) {
                earlierCreatedChild_2.markStatic();
            }
            View.create(earlierCreatedChild_2);
        }
        let earlierCreatedChild_3 = this.findChildById("3");
        if (earlierCreatedChild_3 == undefined) {
            View.create(new pageTitle_1.PageTitle("3", this, { testItem: this.testItem }));
        }
        else {
            earlierCreatedChild_3.updateWithValueParams({
                testItem: this.testItem
            });
            if (!earlierCreatedChild_3.needsUpdate()) {
                earlierCreatedChild_3.markStatic();
            }
            View.create(earlierCreatedChild_3);
        }
        Stack.pop();
        Column.create();
        Column.debugLine("pages/subManualApiTest/BrInterfaceTest.ets(93:7)");
        Column.align(Alignment.TopStart);
        Text.create("目标设备MAC：");
        Text.debugLine("pages/subManualApiTest/BrInterfaceTest.ets(94:9)");
        Text.fontSize("20vp");
        Text.height(50);
        Text.width('100%');
        Text.padding({ top: 10, left: 10 });
        Text.pop();
        TextInput.create({ text: this.pairedMac, placeholder: "请输入即将连接的设备MAC" });
        TextInput.debugLine("pages/subManualApiTest/BrInterfaceTest.ets(99:9)");
        TextInput.height(50);
        TextInput.borderStyle(BorderStyle.Dashed);
        TextInput.backgroundColor({ "id": 16777312, "type": 10001, params: [] });
        TextInput.onChange((str) => {
            this.deviceId = str;
        });
        TextInput.width('96%');
        TextInput.padding({ top: 10, left: 10 });
        Column.pop();
        let earlierCreatedChild_4 = this.findChildById("4");
        if (earlierCreatedChild_4 == undefined) {
            View.create(new contentTable_1.ContentTable("4", this, { testItem: this.testItem, apiItems: testDataModels_1.initBRApiData() }));
        }
        else {
            earlierCreatedChild_4.updateWithValueParams({
                testItem: this.testItem, apiItems: testDataModels_1.initBRApiData()
            });
            View.create(earlierCreatedChild_4);
        }
        Column.pop();
    }
    /**
   * Pair device
   * @param device
   */
    pairDevice(deviceId) {
        this.controller.pair(deviceId, (pinCode) => {
            LogUtil_1.default.info(this.TAG_PAGE + 'pairDevice success callback : pinCode = ' + pinCode);
            // show pair pin dialog
            this.pairPinCode = pinCode;
            this.pairingDevice.deviceId = deviceId;
            this.pairingDevice.deviceName = deviceId;
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
            message: { "id": 16777229, "type": 10003, params: [] },
            primaryButton: {
                value: { "id": 16777253, "type": 10003, params: [] },
                action: () => {
                    LogUtil_1.default.info(this.TAG_PAGE + 'Closed callbacks');
                }
            },
            secondaryButton: {
                value: { "id": 16777237, "type": 10003, params: [] },
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
        showDialog({ "id": 16777231, "type": 10003, params: [] }, { "id": 16777232, "type": 10003, params: [] }, { "id": 16777236, "type": 10003, params: [] });
    }
    /**
     * Show pairing failed title Dialog
     */
    showPairingFailedDialog() {
        showDialog({ "id": 16777269, "type": 10003, params: [] }, { "id": 16777268, "type": 10003, params: [] }, { "id": 16777236, "type": 10003, params: [] });
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
        Column.debugLine("pages/subManualApiTest/BrInterfaceTest.ets(211:5)");
        Column.padding({
            left: { "id": 16777372, "type": 10002, params: [] },
            right: { "id": 16777372, "type": 10002, params: [] }
        });
        Column.width(ConfigData_1.default.WH_100_100);
        Text.create({ "id": 16777241, "type": 10003, params: [] });
        Text.debugLine("pages/subManualApiTest/BrInterfaceTest.ets(212:7)");
        Text.fontSize({ "id": 16777393, "type": 10002, params: [] });
        Text.height({ "id": 16777518, "type": 10002, params: [] });
        Text.width(ConfigData_1.default.WH_100_100);
        Text.padding({
            left: { "id": 16777372, "type": 10002, params: [] },
            top: { "id": 16777361, "type": 10002, params: [] },
            bottom: { "id": 16777361, "type": 10002, params: [] }
        });
        Text.pop();
        Column.create();
        Column.debugLine("pages/subManualApiTest/BrInterfaceTest.ets(221:7)");
        Column.width(ConfigData_1.default.WH_100_100);
        Column.padding({
            left: { "id": 16777372, "type": 10002, params: [] },
            right: { "id": 16777372, "type": 10002, params: [] }
        });
        If.create();
        if (this.pinCode) {
            If.branchId(0);
            Text.create({ "id": 16777239, "type": 10003, params: [this.deviceName, this.deviceName] });
            Text.debugLine("pages/subManualApiTest/BrInterfaceTest.ets(223:11)");
            Text.fontSize({ "id": 16777389, "type": 10002, params: [] });
            Text.width(ConfigData_1.default.WH_100_100);
            Text.margin({
                top: { "id": 16777361, "type": 10002, params: [] },
                bottom: { "id": 16777361, "type": 10002, params: [] }
            });
            Text.pop();
            Text.create(`${this.pinCode}`);
            Text.debugLine("pages/subManualApiTest/BrInterfaceTest.ets(231:11)");
            Text.fontSize({ "id": 16777423, "type": 10002, params: [] });
            Text.fontWeight(FontWeight.Bolder);
            Text.width(ConfigData_1.default.WH_100_100);
            Text.textAlign(TextAlign.Center);
            Text.margin({
                top: { "id": 16777400, "type": 10002, params: [] },
                bottom: { "id": 16777400, "type": 10002, params: [] }
            });
            Text.pop();
        }
        else {
            If.branchId(1);
            Text.create({ "id": 16777240, "type": 10003, params: [] });
            Text.debugLine("pages/subManualApiTest/BrInterfaceTest.ets(242:11)");
            Text.fontSize({ "id": 16777389, "type": 10002, params: [] });
            Text.width(ConfigData_1.default.WH_100_100);
            Text.margin({
                bottom: { "id": 16777455, "type": 10002, params: [] }
            });
            Text.pop();
            Text.create(this.deviceName);
            Text.debugLine("pages/subManualApiTest/BrInterfaceTest.ets(249:11)");
            Text.fontSize({ "id": 16777389, "type": 10002, params: [] });
            Text.width(ConfigData_1.default.WH_100_100);
            Text.fontWeight(FontWeight.Bold);
            Text.pop();
        }
        If.pop();
        // button
        Flex.create({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center });
        Flex.debugLine("pages/subManualApiTest/BrInterfaceTest.ets(256:9)");
        // button
        Flex.width(ConfigData_1.default.WH_100_100);
        // button
        Flex.height({ "id": 16777518, "type": 10002, params: [] });
        Button.createWithChild();
        Button.debugLine("pages/subManualApiTest/BrInterfaceTest.ets(258:11)");
        Button.backgroundColor({ "id": 16777313, "type": 10001, params: [] });
        Button.width({ "id": 16777478, "type": 10002, params: [] });
        Button.height({ "id": 16777508, "type": 10002, params: [] });
        Button.flexGrow(1);
        Button.onClick(() => {
            this.dialogController.close();
            this.action(false);
        });
        Text.create({ "id": 16777253, "type": 10003, params: [] });
        Text.debugLine("pages/subManualApiTest/BrInterfaceTest.ets(259:13)");
        Text.fontSize({ "id": 16777389, "type": 10002, params: [] });
        Text.fontColor(Color.Blue);
        Text.pop();
        Button.pop();
        Divider.create();
        Divider.debugLine("pages/subManualApiTest/BrInterfaceTest.ets(272:11)");
        Divider.height({ "id": 16777489, "type": 10002, params: [] });
        Divider.strokeWidth(0.5);
        Divider.vertical(true);
        Divider.color(({ "id": 16777295, "type": 10001, params: [] }));
        Button.createWithChild();
        Button.debugLine("pages/subManualApiTest/BrInterfaceTest.ets(278:11)");
        Button.backgroundColor({ "id": 16777313, "type": 10001, params: [] });
        Button.width({ "id": 16777478, "type": 10002, params: [] });
        Button.height({ "id": 16777508, "type": 10002, params: [] });
        Button.flexGrow(1);
        Button.onClick(() => {
            this.dialogController.close();
            this.action(true);
        });
        Text.create({ "id": 16777237, "type": 10003, params: [] });
        Text.debugLine("pages/subManualApiTest/BrInterfaceTest.ets(279:13)");
        Text.fontSize({ "id": 16777389, "type": 10002, params: [] });
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
loadDocument(new Index("1", undefined, {}));


/***/ })

/******/ 	});
/************************************************************************/
/******/ 	// The module cache
/******/ 	var __webpack_module_cache__ = {};
/******/ 	
/******/ 	// The require function
/******/ 	function __webpack_require__(moduleId) {
/******/ 		var commonCachedModule = globalThis["__common_module_cache___840471a2ef2afe531ee6aea9267b2e21"] ? globalThis["__common_module_cache___840471a2ef2afe531ee6aea9267b2e21"][moduleId]: null;
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
/******/ 		if (globalThis["__common_module_cache___840471a2ef2afe531ee6aea9267b2e21"] && moduleId.indexOf("?name=") < 0 && Object.keys(globalThis["__common_module_cache___840471a2ef2afe531ee6aea9267b2e21"]).indexOf(moduleId) >= 0) {
/******/ 		  globalThis["__common_module_cache___840471a2ef2afe531ee6aea9267b2e21"][moduleId] = module;
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
/******/ 			"./pages/subManualApiTest/BrInterfaceTest": 0
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
/******/ 		var chunkLoadingGlobal = globalThis["webpackChunk_840471a2ef2afe531ee6aea9267b2e21"] = globalThis["webpackChunk_840471a2ef2afe531ee6aea9267b2e21"] || [];
/******/ 		chunkLoadingGlobal.forEach(webpackJsonpCallback.bind(null, 0));
/******/ 		chunkLoadingGlobal.push = webpackJsonpCallback.bind(null, chunkLoadingGlobal.push.bind(chunkLoadingGlobal));
/******/ 	})();
/******/ 	
/************************************************************************/
/******/ 	
/******/ 	// startup
/******/ 	// Load entry module and return exports
/******/ 	// This entry module depends on other loaded chunks and execution need to be delayed
/******/ 	var __webpack_exports__ = __webpack_require__.O(undefined, ["commons"], () => (__webpack_require__("../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/pages/subManualApiTest/BrInterfaceTest.ets?entry")))
/******/ 	__webpack_exports__ = __webpack_require__.O(__webpack_exports__);
/******/ 	_840471a2ef2afe531ee6aea9267b2e21 = __webpack_exports__;
/******/ 	
/******/ })()
;
//# sourceMappingURL=BrInterfaceTest.js.map