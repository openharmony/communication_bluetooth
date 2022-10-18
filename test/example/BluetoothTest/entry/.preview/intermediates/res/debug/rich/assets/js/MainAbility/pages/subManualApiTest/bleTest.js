var _840471a2ef2afe531ee6aea9267b2e21;
/******/ (() => { // webpackBootstrap
/******/ 	"use strict";
/******/ 	var __webpack_modules__ = ({

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Component/bleFilterTable.ets":
/*!*************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Component/bleFilterTable.ets ***!
  \*************************************************************************************************************************/
/***/ ((__unused_webpack_module, exports, __webpack_require__) => {


Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.BleFilterTable = void 0;
/*
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
__webpack_require__(/*! ../MainAbility/model/testData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/testData.ets");
const testDataModels_1 = __webpack_require__(/*! ../MainAbility/model/testDataModels */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/testDataModels.ets");
class BleFilterTable extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.testItem = undefined;
        this.__apiItems = new ObservedPropertyObject(testDataModels_1.initBRApiData(), this, "apiItems");
        this.__changeIndex = new ObservedPropertySimple(-1
        // input ble scan parameters:
        /*ScanFilter*/
        , this, "changeIndex");
        this.__cbxBleScanFilter = new ObservedPropertySimple(false, this, "cbxBleScanFilter");
        this.__txtScanFilter_deviceId = new ObservedPropertySimple("00:1A:7D:DA:71:15", this, "txtScanFilter_deviceId");
        this.__txtScanFilter_name = new ObservedPropertySimple("dudu-tiger", this, "txtScanFilter_name");
        this.__txtScanFilter_serviceUuid = new ObservedPropertySimple("0000180A-0000-1000-8000-00805f9b34fb", this, "txtScanFilter_serviceUuid");
        this.__cbxBleScanOptions = new ObservedPropertySimple(false, this, "cbxBleScanOptions");
        this.__txtScanOptions_interval = new ObservedPropertySimple("0", this, "txtScanOptions_interval");
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.testItem !== undefined) {
            this.testItem = params.testItem;
        }
        if (params.apiItems !== undefined) {
            this.apiItems = params.apiItems;
        }
        if (params.changeIndex !== undefined) {
            this.changeIndex = params.changeIndex;
        }
        if (params.cbxBleScanFilter !== undefined) {
            this.cbxBleScanFilter = params.cbxBleScanFilter;
        }
        if (params.txtScanFilter_deviceId !== undefined) {
            this.txtScanFilter_deviceId = params.txtScanFilter_deviceId;
        }
        if (params.txtScanFilter_name !== undefined) {
            this.txtScanFilter_name = params.txtScanFilter_name;
        }
        if (params.txtScanFilter_serviceUuid !== undefined) {
            this.txtScanFilter_serviceUuid = params.txtScanFilter_serviceUuid;
        }
        if (params.cbxBleScanOptions !== undefined) {
            this.cbxBleScanOptions = params.cbxBleScanOptions;
        }
        if (params.txtScanOptions_interval !== undefined) {
            this.txtScanOptions_interval = params.txtScanOptions_interval;
        }
    }
    aboutToBeDeleted() {
        this.__apiItems.aboutToBeDeleted();
        this.__changeIndex.aboutToBeDeleted();
        this.__cbxBleScanFilter.aboutToBeDeleted();
        this.__txtScanFilter_deviceId.aboutToBeDeleted();
        this.__txtScanFilter_name.aboutToBeDeleted();
        this.__txtScanFilter_serviceUuid.aboutToBeDeleted();
        this.__cbxBleScanOptions.aboutToBeDeleted();
        this.__txtScanOptions_interval.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id());
    }
    get apiItems() {
        return this.__apiItems.get();
    }
    set apiItems(newValue) {
        this.__apiItems.set(newValue);
    }
    get changeIndex() {
        return this.__changeIndex.get();
    }
    set changeIndex(newValue) {
        this.__changeIndex.set(newValue);
    }
    get cbxBleScanFilter() {
        return this.__cbxBleScanFilter.get();
    }
    set cbxBleScanFilter(newValue) {
        this.__cbxBleScanFilter.set(newValue);
    }
    get txtScanFilter_deviceId() {
        return this.__txtScanFilter_deviceId.get();
    }
    set txtScanFilter_deviceId(newValue) {
        this.__txtScanFilter_deviceId.set(newValue);
    }
    get txtScanFilter_name() {
        return this.__txtScanFilter_name.get();
    }
    set txtScanFilter_name(newValue) {
        this.__txtScanFilter_name.set(newValue);
    }
    get txtScanFilter_serviceUuid() {
        return this.__txtScanFilter_serviceUuid.get();
    }
    set txtScanFilter_serviceUuid(newValue) {
        this.__txtScanFilter_serviceUuid.set(newValue);
    }
    get cbxBleScanOptions() {
        return this.__cbxBleScanOptions.get();
    }
    set cbxBleScanOptions(newValue) {
        this.__cbxBleScanOptions.set(newValue);
    }
    get txtScanOptions_interval() {
        return this.__txtScanOptions_interval.get();
    }
    set txtScanOptions_interval(newValue) {
        this.__txtScanOptions_interval.set(newValue);
    }
    getCurrentState(index) {
        //    console.log("currentState:" + index.toString() + this.apiItems[index].result)
        return this.apiItems[index].result;
    }
    aboutToAppear() {
        AppStorage.SetOrCreate('txtScanFilterDeviceId', this.txtScanFilter_deviceId);
        AppStorage.SetOrCreate('txtScanFilterName', this.txtScanFilter_name);
        AppStorage.SetOrCreate('txtScanFilterServiceUuid', this.txtScanFilter_serviceUuid);
        AppStorage.SetOrCreate('txtScanOptions_interval', this.txtScanOptions_interval);
        AppStorage.SetOrCreate('txtScanOptionsDutyMode', 'LOW_POWER');
        AppStorage.SetOrCreate('txtScanOptionsMatchMode', 'MATCH_MODE_AGGRESSIVE');
        AppStorage.SetOrCreate('cbxBleScanFilter', this.cbxBleScanFilter);
        AppStorage.SetOrCreate('cbxBleScanOptions', this.cbxBleScanOptions);
    }
    render() {
        Scroll.create();
        Scroll.debugLine("../Component/bleFilterTable.ets(53:5)");
        Scroll.scrollBarWidth(20);
        Scroll.padding({ bottom: 10 });
        Column.create();
        Column.debugLine("../Component/bleFilterTable.ets(54:7)");
        Column.height("100%");
        Column.backgroundColor({ "id": 16777312, "type": 10001, params: [] });
        Row.create();
        Row.debugLine("../Component/bleFilterTable.ets(55:9)");
        Row.height(30);
        Row.width("100%");
        Row.borderWidth(1);
        Row.borderColor({ "id": 16777302, "type": 10001, params: [] });
        Text.create("BLE扫描过滤参数:");
        Text.debugLine("../Component/bleFilterTable.ets(56:11)");
        Text.fontSize("17vp");
        Text.fontColor("#4444444");
        Text.width("80%");
        Text.padding({ left: 10 });
        Text.pop();
        Checkbox.create({ name: 'bleScanFilter', group: 'checkboxGroup99' });
        Checkbox.debugLine("../Component/bleFilterTable.ets(61:11)");
        Checkbox.select(this.cbxBleScanFilter);
        Checkbox.selectedColor(0x39a2db);
        Checkbox.onChange((value) => {
            if (value) {
                this.cbxBleScanFilter = true;
            }
            else {
                this.cbxBleScanFilter = false;
            }
            AppStorage.SetOrCreate('cbxBleScanFilter', value);
        });
        Checkbox.width(15);
        Checkbox.pop();
        Row.pop();
        Column.create();
        Column.debugLine("../Component/bleFilterTable.ets(79:9)");
        Column.create();
        Column.debugLine("../Component/bleFilterTable.ets(80:11)");
        Column.borderColor({ "id": 16777302, "type": 10001, params: [] });
        Column.borderWidth(1);
        Column.create();
        Column.debugLine("../Component/bleFilterTable.ets(81:13)");
        Column.height(160);
        Column.width('100%');
        Column.padding(5);
        Row.create();
        Row.debugLine("../Component/bleFilterTable.ets(82:15)");
        Row.padding(5);
        Row.justifyContent(FlexAlign.Start);
        Row.alignItems(VerticalAlign.Center);
        Text.create("mac");
        Text.debugLine("../Component/bleFilterTable.ets(83:17)");
        Text.fontSize("17vp");
        Text.width(60);
        Text.pop();
        TextInput.create({ text: this.txtScanFilter_deviceId, placeholder: "input deviceId" });
        TextInput.debugLine("../Component/bleFilterTable.ets(84:17)");
        TextInput.fontSize("15vp");
        TextInput.onChange((strInput) => {
            this.txtScanFilter_deviceId = strInput;
            AppStorage.SetOrCreate('txtScanFilterDeviceId', strInput);
        });
        TextInput.width("80%");
        TextInput.borderRadius(1);
        Row.pop();
        Row.create();
        Row.debugLine("../Component/bleFilterTable.ets(97:15)");
        Row.padding(5);
        Row.justifyContent(FlexAlign.Start);
        Row.alignItems(VerticalAlign.Center);
        Text.create("name");
        Text.debugLine("../Component/bleFilterTable.ets(98:17)");
        Text.fontSize(17);
        Text.width(60);
        Text.pop();
        TextInput.create({ text: this.txtScanFilter_name, placeholder: "input name" });
        TextInput.debugLine("../Component/bleFilterTable.ets(99:17)");
        TextInput.fontSize("15vp");
        TextInput.onChange((strInput) => {
            this.txtScanFilter_name = strInput;
            AppStorage.SetOrCreate('txtScanFilterName', strInput);
        });
        TextInput.width("80%");
        TextInput.borderRadius(1);
        Row.pop();
        Row.create();
        Row.debugLine("../Component/bleFilterTable.ets(112:15)");
        Row.padding(5);
        Row.justifyContent(FlexAlign.Start);
        Row.alignItems(VerticalAlign.Center);
        Text.create("serviceUuid");
        Text.debugLine("../Component/bleFilterTable.ets(113:17)");
        Text.fontSize(17);
        Text.width(60);
        Text.pop();
        TextInput.create({ text: this.txtScanFilter_serviceUuid, placeholder: "input serviceUuid" });
        TextInput.debugLine("../Component/bleFilterTable.ets(114:17)");
        TextInput.fontSize("15vp");
        TextInput.onChange((strInput) => {
            this.txtScanFilter_serviceUuid = strInput;
            AppStorage.SetOrCreate('txtScanFilterServiceUuid', strInput);
        });
        TextInput.width("80%");
        TextInput.borderRadius(1);
        Row.pop();
        Column.pop();
        Column.pop();
        Column.create();
        Column.debugLine("../Component/bleFilterTable.ets(134:11)");
        Column.height(180);
        Column.borderWidth(1);
        Column.borderColor({ "id": 16777302, "type": 10001, params: [] });
        Row.create();
        Row.debugLine("../Component/bleFilterTable.ets(135:13)");
        Row.height(40);
        Row.width('100%');
        Row.padding(5);
        Text.create("BLE扫描配置:");
        Text.debugLine("../Component/bleFilterTable.ets(136:15)");
        Text.padding("5vp");
        Text.fontSize("17vp");
        Text.fontColor("#4444444");
        Text.width("80%");
        Text.pop();
        Checkbox.create({ name: 'bleScanOptions', group: 'checkboxGroup95' });
        Checkbox.debugLine("../Component/bleFilterTable.ets(141:15)");
        Checkbox.select(this.cbxBleScanOptions);
        Checkbox.selectedColor(0x39a2db);
        Checkbox.onChange((value) => {
            if (value) {
                this.cbxBleScanOptions = true;
                this.cbxBleScanFilter = true;
                AppStorage.SetOrCreate('cbxBleScanOptions', this.cbxBleScanOptions);
                AppStorage.SetOrCreate('cbxBleScanFilter', this.cbxBleScanFilter);
            }
            else {
                this.cbxBleScanOptions = false;
                AppStorage.SetOrCreate('cbxBleScanOptions', this.cbxBleScanOptions);
            }
        });
        Checkbox.width("15vp");
        Checkbox.margin({ left: "1vp" });
        Checkbox.pop();
        Row.pop();
        Column.create();
        Column.debugLine("../Component/bleFilterTable.ets(161:13)");
        Column.width('100%');
        Column.padding(5);
        Row.create();
        Row.debugLine("../Component/bleFilterTable.ets(162:15)");
        Row.padding(5);
        Row.justifyContent(FlexAlign.Start);
        Row.alignItems(VerticalAlign.Center);
        Text.create("interval");
        Text.debugLine("../Component/bleFilterTable.ets(163:17)");
        Text.fontSize(17);
        Text.width(80);
        Text.pop();
        TextInput.create({ text: this.txtScanOptions_interval, placeholder: "input interval" });
        TextInput.debugLine("../Component/bleFilterTable.ets(164:17)");
        TextInput.fontSize("17vp");
        TextInput.onChange((strInput) => {
            this.txtScanOptions_interval = strInput;
            AppStorage.SetOrCreate('txtScanOptionsInterval', strInput);
        });
        TextInput.width("80%");
        Row.pop();
        Row.create();
        Row.debugLine("../Component/bleFilterTable.ets(176:15)");
        Row.padding(5);
        Row.justifyContent(FlexAlign.Start);
        Row.alignItems(VerticalAlign.Center);
        Text.create("dutyMode");
        Text.debugLine("../Component/bleFilterTable.ets(177:17)");
        Text.fontSize(17);
        Text.width(80);
        Text.pop();
        Column.create();
        Column.debugLine("../Component/bleFilterTable.ets(178:17)");
        Column.width("80%");
        Select.create([
            { value: 'LOW_POWER' },
            { value: 'BALANCED' },
            { value: 'LOW_LATENCY' }
        ]);
        Select.debugLine("../Component/bleFilterTable.ets(179:19)");
        Select.selected(2);
        Select.value('LOW_POWER');
        Select.font({ size: 17 });
        Select.selectedOptionFont({ size: 17 });
        Select.optionFont({ size: 15 });
        Select.width("80%");
        Select.onSelect((index) => {
            console.info("Select:" + index);
            AppStorage.SetOrCreate('txtScanOptionsDutyMode', index);
        });
        Select.pop();
        Column.pop();
        Row.pop();
        Row.create();
        Row.debugLine("../Component/bleFilterTable.ets(201:15)");
        Row.padding(5);
        Row.justifyContent(FlexAlign.Start);
        Row.alignItems(VerticalAlign.Center);
        Text.create("matchMode");
        Text.debugLine("../Component/bleFilterTable.ets(202:17)");
        Text.fontSize(17);
        Text.width(80);
        Text.pop();
        Column.create();
        Column.debugLine("../Component/bleFilterTable.ets(203:17)");
        Column.width("80%");
        Select.create([
            { value: 'MATCH_MODE_AGGRESSIVE' },
            { value: 'MATCH_MODE_STICKY' }
        ]);
        Select.debugLine("../Component/bleFilterTable.ets(204:19)");
        Select.value('MATCH_MODE_AGGRESSIVE');
        Select.font({ size: 17 });
        Select.selectedOptionFont({ size: 17 });
        Select.optionFont({ size: 15 });
        Select.width("80%");
        Select.onSelect((index) => {
            console.info("Select:" + index);
            AppStorage.SetOrCreate('txtScanOptionsMatchMode', index);
        });
        Select.pop();
        Column.pop();
        Row.pop();
        Column.pop();
        Column.pop();
        Column.pop();
        Column.pop();
        Scroll.pop();
    }
}
exports.BleFilterTable = BleFilterTable;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/pages/subManualApiTest/bleTest.ets?entry":
/*!*************************************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/pages/subManualApiTest/bleTest.ets?entry ***!
  \*************************************************************************************************************************************************/
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
// @ts-nocheck
/**
 * BLE Test Page Of Bluetooth test
 */
const bluetoothModel_1 = __importDefault(__webpack_require__(/*! ../../model/bluetoothModel */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/bluetoothModel.ts"));
var _system_router_1  = globalThis.requireNativeModule('system.router');
const LogUtil_1 = __importDefault(__webpack_require__(/*! ../../../Utils/LogUtil */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Utils/LogUtil.ts"));
__webpack_require__(/*! @ohos.bluetooth */ "../../api/@ohos.bluetooth.d.ts");
const pageTitle_1 = __webpack_require__(/*! ../../../Component/pageTitle */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Component/pageTitle.ets");
const testImageDisplay_1 = __webpack_require__(/*! ../../../Component/testImageDisplay */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Component/testImageDisplay.ets");
const contentTable_1 = __webpack_require__(/*! ../../../Component/contentTable */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Component/contentTable.ets");
__webpack_require__(/*! ../../model/testData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/testData.ets");
const testDataModels_1 = __webpack_require__(/*! ../../model/testDataModels */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/testDataModels.ets");
const bleFilterTable_1 = __webpack_require__(/*! ../../../Component/bleFilterTable */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Component/bleFilterTable.ets");
class BleTest extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.testItem = _system_router_1.getParams().testId;
        this.__showList = new ObservedPropertySimple(true, this, "showList");
        this.__message = new ObservedPropertySimple('BleTest Result', this, "message");
        this.__peripheralDeviceId = new ObservedPropertySimple('00:1A:7D:DA:71:15', this, "peripheralDeviceId");
        this.__currentClick = new ObservedPropertySimple(-1, this, "currentClick");
        this.__btOnBLEDeviceFind = new ObservedPropertySimple('on("BLEDeviceFind"): void', this, "btOnBLEDeviceFind");
        this.__isBLEDeviceFindClick = new ObservedPropertySimple(false, this, "isBLEDeviceFindClick");
        this.__cbxBleScanFilter = new ObservedPropertySimple(false, this, "cbxBleScanFilter");
        this.__txtScanFilter_deviceId = new ObservedPropertySimple("00:1A:7D:DA:71:15", this, "txtScanFilter_deviceId");
        this.__txtScanFilter_name = new ObservedPropertySimple("dudu-tiger", this, "txtScanFilter_name");
        this.__txtScanFilter_serviceUuid = new ObservedPropertySimple("0000180A-0000-1000-8000-00805f9b34fb", this, "txtScanFilter_serviceUuid");
        this.__curScanFilters = new ObservedPropertyObject([], this, "curScanFilters");
        this.__cbxBleScanOptions = new ObservedPropertySimple(false, this, "cbxBleScanOptions");
        this.__txtScanOptions_interval = new ObservedPropertySimple("0", this, "txtScanOptions_interval");
        this.__txtScanOptions_dutyMode = new ObservedPropertySimple(0, this, "txtScanOptions_dutyMode");
        this.__rd3ScanOptions_dutyModeChecked = new ObservedPropertySimple(false, this, "rd3ScanOptions_dutyModeChecked");
        this.__rd2ScanOptions_dutyModeChecked = new ObservedPropertySimple(false, this, "rd2ScanOptions_dutyModeChecked");
        this.__rd1ScanOptions_dutyModeChecked = new ObservedPropertySimple(true, this, "rd1ScanOptions_dutyModeChecked");
        this.__txtScanOptions_matchMode = new ObservedPropertySimple(1, this, "txtScanOptions_matchMode");
        this.__rd1ScanOptions_matchModeChecked = new ObservedPropertySimple(true, this, "rd1ScanOptions_matchModeChecked");
        this.__rd2ScanOptions_matchModeChecked = new ObservedPropertySimple(false, this, "rd2ScanOptions_matchModeChecked");
        this.__curScanOptions = new ObservedPropertyObject({
            interval: 0,
            dutyMode: 0,
            matchMode: 1
        }, this, "curScanOptions");
        this.__availBleDeviceIds = AppStorage.GetOrCreate().setAndLink("bleAvailableDevices", [], this);
        this.__On_off_BLEDeviceFind = AppStorage.GetOrCreate().setAndLink("OnBLEDeviceFind", false, this);
        this.gattServerInstance = null;
        this.gattClientDeviceInstance = null;
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.testItem !== undefined) {
            this.testItem = params.testItem;
        }
        if (params.showList !== undefined) {
            this.showList = params.showList;
        }
        if (params.message !== undefined) {
            this.message = params.message;
        }
        if (params.peripheralDeviceId !== undefined) {
            this.peripheralDeviceId = params.peripheralDeviceId;
        }
        if (params.currentClick !== undefined) {
            this.currentClick = params.currentClick;
        }
        if (params.btOnBLEDeviceFind !== undefined) {
            this.btOnBLEDeviceFind = params.btOnBLEDeviceFind;
        }
        if (params.isBLEDeviceFindClick !== undefined) {
            this.isBLEDeviceFindClick = params.isBLEDeviceFindClick;
        }
        if (params.cbxBleScanFilter !== undefined) {
            this.cbxBleScanFilter = params.cbxBleScanFilter;
        }
        if (params.txtScanFilter_deviceId !== undefined) {
            this.txtScanFilter_deviceId = params.txtScanFilter_deviceId;
        }
        if (params.txtScanFilter_name !== undefined) {
            this.txtScanFilter_name = params.txtScanFilter_name;
        }
        if (params.txtScanFilter_serviceUuid !== undefined) {
            this.txtScanFilter_serviceUuid = params.txtScanFilter_serviceUuid;
        }
        if (params.curScanFilters !== undefined) {
            this.curScanFilters = params.curScanFilters;
        }
        if (params.cbxBleScanOptions !== undefined) {
            this.cbxBleScanOptions = params.cbxBleScanOptions;
        }
        if (params.txtScanOptions_interval !== undefined) {
            this.txtScanOptions_interval = params.txtScanOptions_interval;
        }
        if (params.txtScanOptions_dutyMode !== undefined) {
            this.txtScanOptions_dutyMode = params.txtScanOptions_dutyMode;
        }
        if (params.rd3ScanOptions_dutyModeChecked !== undefined) {
            this.rd3ScanOptions_dutyModeChecked = params.rd3ScanOptions_dutyModeChecked;
        }
        if (params.rd2ScanOptions_dutyModeChecked !== undefined) {
            this.rd2ScanOptions_dutyModeChecked = params.rd2ScanOptions_dutyModeChecked;
        }
        if (params.rd1ScanOptions_dutyModeChecked !== undefined) {
            this.rd1ScanOptions_dutyModeChecked = params.rd1ScanOptions_dutyModeChecked;
        }
        if (params.txtScanOptions_matchMode !== undefined) {
            this.txtScanOptions_matchMode = params.txtScanOptions_matchMode;
        }
        if (params.rd1ScanOptions_matchModeChecked !== undefined) {
            this.rd1ScanOptions_matchModeChecked = params.rd1ScanOptions_matchModeChecked;
        }
        if (params.rd2ScanOptions_matchModeChecked !== undefined) {
            this.rd2ScanOptions_matchModeChecked = params.rd2ScanOptions_matchModeChecked;
        }
        if (params.curScanOptions !== undefined) {
            this.curScanOptions = params.curScanOptions;
        }
        if (params.gattServerInstance !== undefined) {
            this.gattServerInstance = params.gattServerInstance;
        }
        if (params.gattClientDeviceInstance !== undefined) {
            this.gattClientDeviceInstance = params.gattClientDeviceInstance;
        }
    }
    aboutToBeDeleted() {
        this.__showList.aboutToBeDeleted();
        this.__message.aboutToBeDeleted();
        this.__peripheralDeviceId.aboutToBeDeleted();
        this.__currentClick.aboutToBeDeleted();
        this.__btOnBLEDeviceFind.aboutToBeDeleted();
        this.__isBLEDeviceFindClick.aboutToBeDeleted();
        this.__cbxBleScanFilter.aboutToBeDeleted();
        this.__txtScanFilter_deviceId.aboutToBeDeleted();
        this.__txtScanFilter_name.aboutToBeDeleted();
        this.__txtScanFilter_serviceUuid.aboutToBeDeleted();
        this.__curScanFilters.aboutToBeDeleted();
        this.__cbxBleScanOptions.aboutToBeDeleted();
        this.__txtScanOptions_interval.aboutToBeDeleted();
        this.__txtScanOptions_dutyMode.aboutToBeDeleted();
        this.__rd3ScanOptions_dutyModeChecked.aboutToBeDeleted();
        this.__rd2ScanOptions_dutyModeChecked.aboutToBeDeleted();
        this.__rd1ScanOptions_dutyModeChecked.aboutToBeDeleted();
        this.__txtScanOptions_matchMode.aboutToBeDeleted();
        this.__rd1ScanOptions_matchModeChecked.aboutToBeDeleted();
        this.__rd2ScanOptions_matchModeChecked.aboutToBeDeleted();
        this.__curScanOptions.aboutToBeDeleted();
        this.__availBleDeviceIds.aboutToBeDeleted();
        this.__On_off_BLEDeviceFind.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id());
    }
    get showList() {
        return this.__showList.get();
    }
    set showList(newValue) {
        this.__showList.set(newValue);
    }
    get message() {
        return this.__message.get();
    }
    set message(newValue) {
        this.__message.set(newValue);
    }
    get peripheralDeviceId() {
        return this.__peripheralDeviceId.get();
    }
    set peripheralDeviceId(newValue) {
        this.__peripheralDeviceId.set(newValue);
    }
    get currentClick() {
        return this.__currentClick.get();
    }
    set currentClick(newValue) {
        this.__currentClick.set(newValue);
    }
    get btOnBLEDeviceFind() {
        return this.__btOnBLEDeviceFind.get();
    }
    set btOnBLEDeviceFind(newValue) {
        this.__btOnBLEDeviceFind.set(newValue);
    }
    get isBLEDeviceFindClick() {
        return this.__isBLEDeviceFindClick.get();
    }
    set isBLEDeviceFindClick(newValue) {
        this.__isBLEDeviceFindClick.set(newValue);
    }
    get cbxBleScanFilter() {
        return this.__cbxBleScanFilter.get();
    }
    set cbxBleScanFilter(newValue) {
        this.__cbxBleScanFilter.set(newValue);
    }
    get txtScanFilter_deviceId() {
        return this.__txtScanFilter_deviceId.get();
    }
    set txtScanFilter_deviceId(newValue) {
        this.__txtScanFilter_deviceId.set(newValue);
    }
    get txtScanFilter_name() {
        return this.__txtScanFilter_name.get();
    }
    set txtScanFilter_name(newValue) {
        this.__txtScanFilter_name.set(newValue);
    }
    get txtScanFilter_serviceUuid() {
        return this.__txtScanFilter_serviceUuid.get();
    }
    set txtScanFilter_serviceUuid(newValue) {
        this.__txtScanFilter_serviceUuid.set(newValue);
    }
    get curScanFilters() {
        return this.__curScanFilters.get();
    }
    set curScanFilters(newValue) {
        this.__curScanFilters.set(newValue);
    }
    get cbxBleScanOptions() {
        return this.__cbxBleScanOptions.get();
    }
    set cbxBleScanOptions(newValue) {
        this.__cbxBleScanOptions.set(newValue);
    }
    get txtScanOptions_interval() {
        return this.__txtScanOptions_interval.get();
    }
    set txtScanOptions_interval(newValue) {
        this.__txtScanOptions_interval.set(newValue);
    }
    get txtScanOptions_dutyMode() {
        return this.__txtScanOptions_dutyMode.get();
    }
    set txtScanOptions_dutyMode(newValue) {
        this.__txtScanOptions_dutyMode.set(newValue);
    }
    get rd3ScanOptions_dutyModeChecked() {
        return this.__rd3ScanOptions_dutyModeChecked.get();
    }
    set rd3ScanOptions_dutyModeChecked(newValue) {
        this.__rd3ScanOptions_dutyModeChecked.set(newValue);
    }
    get rd2ScanOptions_dutyModeChecked() {
        return this.__rd2ScanOptions_dutyModeChecked.get();
    }
    set rd2ScanOptions_dutyModeChecked(newValue) {
        this.__rd2ScanOptions_dutyModeChecked.set(newValue);
    }
    get rd1ScanOptions_dutyModeChecked() {
        return this.__rd1ScanOptions_dutyModeChecked.get();
    }
    set rd1ScanOptions_dutyModeChecked(newValue) {
        this.__rd1ScanOptions_dutyModeChecked.set(newValue);
    }
    get txtScanOptions_matchMode() {
        return this.__txtScanOptions_matchMode.get();
    }
    set txtScanOptions_matchMode(newValue) {
        this.__txtScanOptions_matchMode.set(newValue);
    }
    get rd1ScanOptions_matchModeChecked() {
        return this.__rd1ScanOptions_matchModeChecked.get();
    }
    set rd1ScanOptions_matchModeChecked(newValue) {
        this.__rd1ScanOptions_matchModeChecked.set(newValue);
    }
    get rd2ScanOptions_matchModeChecked() {
        return this.__rd2ScanOptions_matchModeChecked.get();
    }
    set rd2ScanOptions_matchModeChecked(newValue) {
        this.__rd2ScanOptions_matchModeChecked.set(newValue);
    }
    get curScanOptions() {
        return this.__curScanOptions.get();
    }
    set curScanOptions(newValue) {
        this.__curScanOptions.set(newValue);
    }
    get availBleDeviceIds() {
        return this.__availBleDeviceIds.get();
    }
    set availBleDeviceIds(newValue) {
        this.__availBleDeviceIds.set(newValue);
    }
    get On_off_BLEDeviceFind() {
        return this.__On_off_BLEDeviceFind.get();
    }
    set On_off_BLEDeviceFind(newValue) {
        this.__On_off_BLEDeviceFind.set(newValue);
    }
    aboutToAppear() {
        AppStorage.SetOrCreate("peripheralDeviceId", this.peripheralDeviceId);
        AppStorage.SetOrCreate('txtScanFilterDeviceId', this.txtScanFilter_deviceId);
        AppStorage.SetOrCreate('txtScanFilterName', this.txtScanFilter_name);
        AppStorage.SetOrCreate('txtScanFilterServiceUuid', this.txtScanFilter_serviceUuid);
        AppStorage.SetOrCreate('txtScanOptions_interval', this.txtScanOptions_interval);
        AppStorage.SetOrCreate('txtScanOptionsDutyMode', 'LOW_POWER');
        AppStorage.SetOrCreate('txtScanOptionsMatchMode', 'MATCH_MODE_AGGRESSIVE');
    }
    render() {
        Column.create();
        Column.debugLine("pages/subManualApiTest/bleTest.ets(92:5)");
        Column.alignItems(HorizontalAlign.Center);
        Column.backgroundColor({ "id": 16777302, "type": 10001, params: [] });
        Stack.create({ alignContent: Alignment.TopStart });
        Stack.debugLine("pages/subManualApiTest/bleTest.ets(93:7)");
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
        Column.debugLine("pages/subManualApiTest/bleTest.ets(98:7)");
        Column.align(Alignment.TopStart);
        Row.create();
        Row.debugLine("pages/subManualApiTest/bleTest.ets(99:9)");
        Row.margin({ top: "10vp", left: "20vp", right: "20vp" });
        Row.alignItems(VerticalAlign.Center);
        Row.backgroundColor({ "id": 16777312, "type": 10001, params: [] });
        Row.width("100%");
        Row.height(60);
        Text.create("外设MAC:");
        Text.debugLine("pages/subManualApiTest/bleTest.ets(100:11)");
        Text.fontSize("18vp");
        Text.height(60);
        Text.width("20%");
        Text.padding({ left: 10 });
        Text.pop();
        TextInput.create({ text: this.peripheralDeviceId, placeholder: "input peripheral deviceId." });
        TextInput.debugLine("pages/subManualApiTest/bleTest.ets(105:11)");
        TextInput.fontSize("18vp");
        TextInput.onChange((strInput) => {
            this.peripheralDeviceId = strInput;
            AppStorage.SetOrCreate("peripheralDeviceId", strInput);
        });
        TextInput.width("70%");
        Image.create({ "id": 16777545, "type": 20000, params: [] });
        Image.debugLine("pages/subManualApiTest/bleTest.ets(112:11)");
        Image.height(40);
        Image.width(40);
        Image.padding({ top: 10, bottom: 10, left: 2 });
        Image.onClick(() => {
            this.showList = !this.showList;
        });
        Row.pop();
        Column.pop();
        If.create();
        if (this.showList) {
            If.branchId(0);
            let earlierCreatedChild_4 = this.findChildById("4");
            if (earlierCreatedChild_4 == undefined) {
                View.create(new contentTable_1.ContentTable("4", this, { testItem: this.testItem, apiItems: testDataModels_1.initBleApiData() }));
            }
            else {
                earlierCreatedChild_4.updateWithValueParams({
                    testItem: this.testItem, apiItems: testDataModels_1.initBleApiData()
                });
                View.create(earlierCreatedChild_4);
            }
        }
        else {
            If.branchId(1);
            let earlierCreatedChild_5 = this.findChildById("5");
            if (earlierCreatedChild_5 == undefined) {
                View.create(new bleFilterTable_1.BleFilterTable("5", this, { testItem: this.testItem, }));
            }
            else {
                earlierCreatedChild_5.updateWithValueParams({
                    testItem: this.testItem
                });
                View.create(earlierCreatedChild_5);
            }
        }
        If.pop();
        Column.pop();
    }
    /**
    * subscribeBLEDeviceFind
    */
    subscribeBLEDeviceFind() {
        //回调函数里的动作是过滤，已经过滤好了就不用再考虑了。
        bluetoothModel_1.default.subscribeBLEDeviceFind((deviceItems) => {
            LogUtil_1.default.log("BLE Test:" + 'available ble devices changed.');
            deviceItems === null || deviceItems === void 0 ? void 0 : deviceItems.forEach(deviceItem => {
                let device = this.availBleDeviceIds.find((availableBleDevice) => {
                    return availableBleDevice === deviceItem.deviceId;
                });
                if (!device) {
                    let newDevice = deviceItem.deviceId;
                    LogUtil_1.default.log("BLE Test:" + 'available bluetooth new device : ' + newDevice);
                    this.availBleDeviceIds.push(newDevice);
                    LogUtil_1.default.log("BLE Test:" + 'available bluetooth new device pushed. availableDevices length = ' + this.availBleDeviceIds.length);
                }
            });
            AppStorage.SetOrCreate('bluetoothAvailableDevices', this.availBleDeviceIds);
        });
    }
    buildBleScanFilters() {
        this.curScanFilters = [];
        if (this.cbxBleScanFilter) {
            let bleScanFilter = {};
            if (this.txtScanFilter_deviceId.length > 0 && this.txtScanFilter_name.length > 0 && this.txtScanFilter_serviceUuid.length > 0) {
                bleScanFilter = {
                    deviceId: this.txtScanFilter_deviceId,
                    name: this.txtScanFilter_name,
                    serviceUuid: this.txtScanFilter_serviceUuid
                };
                this.curScanFilters.push(bleScanFilter);
            }
            if (this.txtScanFilter_deviceId === '' && this.txtScanFilter_name === '' && this.txtScanFilter_serviceUuid.length > 0) {
                bleScanFilter = {
                    serviceUuid: this.txtScanFilter_serviceUuid
                };
                this.curScanFilters.push(bleScanFilter);
            }
            if (this.txtScanFilter_deviceId === '' && this.txtScanFilter_name.length > 0 && this.txtScanFilter_serviceUuid === '') {
                bleScanFilter = {
                    name: this.txtScanFilter_name
                };
                this.curScanFilters.push(bleScanFilter);
            }
            if (this.txtScanFilter_deviceId.length > 0 && this.txtScanFilter_name === '' && this.txtScanFilter_serviceUuid === '') {
                bleScanFilter = {
                    deviceId: this.txtScanFilter_deviceId
                };
                this.curScanFilters.push(bleScanFilter);
            }
            if (this.txtScanFilter_deviceId === '' && this.txtScanFilter_name.length > 0 && this.txtScanFilter_serviceUuid.length > 0) {
                bleScanFilter = {
                    name: this.txtScanFilter_name,
                    serviceUuid: this.txtScanFilter_serviceUuid
                };
                this.curScanFilters.push(bleScanFilter);
            }
            if (this.txtScanFilter_deviceId.length > 0 && this.txtScanFilter_name === '' && this.txtScanFilter_serviceUuid.length > 0) {
                bleScanFilter = {
                    deviceId: this.txtScanFilter_deviceId,
                    serviceUuid: this.txtScanFilter_serviceUuid
                };
                this.curScanFilters.push(bleScanFilter);
            }
            if (this.txtScanFilter_deviceId.length > 0 && this.txtScanFilter_name.length > 0 && this.txtScanFilter_serviceUuid === '') {
                bleScanFilter = {
                    deviceId: this.txtScanFilter_deviceId,
                    name: this.txtScanFilter_name
                };
                this.curScanFilters.push(bleScanFilter);
            }
        }
    }
    buildBleScanOption() {
        if (this.cbxBleScanOptions) {
            this.curScanOptions = {
                interval: parseInt(this.txtScanOptions_interval),
                dutyMode: this.txtScanOptions_dutyMode,
                matchMode: this.txtScanOptions_matchMode
            };
        }
    }
}
loadDocument(new BleTest("1", undefined, {}));


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
/******/ 			"./pages/subManualApiTest/bleTest": 0
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
/******/ 	var __webpack_exports__ = __webpack_require__.O(undefined, ["commons"], () => (__webpack_require__("../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/pages/subManualApiTest/bleTest.ets?entry")))
/******/ 	__webpack_exports__ = __webpack_require__.O(__webpack_exports__);
/******/ 	_840471a2ef2afe531ee6aea9267b2e21 = __webpack_exports__;
/******/ 	
/******/ })()
;
//# sourceMappingURL=bleTest.js.map