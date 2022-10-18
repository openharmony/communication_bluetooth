var _840471a2ef2afe531ee6aea9267b2e21;
/******/ (() => { // webpackBootstrap
/******/ 	"use strict";
/******/ 	var __webpack_modules__ = ({

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/pages/manualApiTestPage.ets?entry":
/*!******************************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/pages/manualApiTestPage.ets?entry ***!
  \******************************************************************************************************************************************/
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
const ConfigData_1 = __importDefault(__webpack_require__(/*! ../../Utils/ConfigData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Utils/ConfigData.ts"));
const headercomponent_1 = __importDefault(__webpack_require__(/*! ../../component/headercomponent */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/component/headercomponent.ets"));
const subEntryComponent_1 = __webpack_require__(/*! ../../component/subEntryComponent */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/component/subEntryComponent.ets");
class ManualApiTestPage extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.__message = new ObservedPropertySimple('ManualApiTest', this, "message");
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.message !== undefined) {
            this.message = params.message;
        }
    }
    aboutToBeDeleted() {
        this.__message.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id());
    }
    get message() {
        return this.__message.get();
    }
    set message(newValue) {
        this.__message.set(newValue);
    }
    render() {
        Column.create();
        Column.debugLine("pages/manualApiTestPage.ets(28:5)");
        Column.backgroundColor({ "id": 125829132, "type": 10001, params: [] });
        Column.width(ConfigData_1.default.WH_100_100);
        Column.height(ConfigData_1.default.WH_100_100);
        GridContainer.create({ columns: 12, sizeType: SizeType.Auto, gutter: vp2px(1) === 2 ? '12vp' : '0vp', margin: vp2px(1) === 2 ? '24vp' : '0vp' });
        GridContainer.debugLine("pages/manualApiTestPage.ets(29:7)");
        GridContainer.width(ConfigData_1.default.WH_100_100);
        GridContainer.height(ConfigData_1.default.WH_100_100);
        Row.create({});
        Row.debugLine("pages/manualApiTestPage.ets(30:9)");
        Row.width(ConfigData_1.default.WH_100_100);
        Row.height(ConfigData_1.default.WH_100_100);
        Column.create();
        Column.debugLine("pages/manualApiTestPage.ets(31:11)");
        Column.width(ConfigData_1.default.WH_100_100);
        Column.height(ConfigData_1.default.WH_100_100);
        Column.useSizeType({
            xs: { span: 0, offset: 0 }, sm: { span: 0, offset: 0 },
            md: { span: 0, offset: 0 }, lg: { span: 2, offset: 0 }
        });
        Column.pop();
        Column.create();
        Column.debugLine("pages/manualApiTestPage.ets(40:11)");
        Column.padding({ left: { "id": 16777372, "type": 10002, params: [] }, right: { "id": 16777372, "type": 10002, params: [] } });
        Column.width(ConfigData_1.default.WH_100_100);
        Column.height(ConfigData_1.default.WH_100_100);
        Column.useSizeType({
            xs: { span: 12, offset: 0 }, sm: { span: 12, offset: 0 },
            md: { span: 12, offset: 0 }, lg: { span: 8, offset: 2 }
        });
        let earlierCreatedChild_2 = this.findChildById("2");
        if (earlierCreatedChild_2 == undefined) {
            View.create(new headercomponent_1.default("2", this, { headName: { "id": 16777221, "type": 10003, params: [] }, isActive: true }));
        }
        else {
            earlierCreatedChild_2.updateWithValueParams({
                headName: { "id": 16777221, "type": 10003, params: [] },
                isActive: true
            });
            View.create(earlierCreatedChild_2);
        }
        Column.create({ space: '12vp' });
        Column.debugLine("pages/manualApiTestPage.ets(42:13)");
        Column.width(ConfigData_1.default.WH_100_100);
        List.create();
        List.debugLine("pages/manualApiTestPage.ets(43:15)");
        ListItem.create();
        ListItem.debugLine("pages/manualApiTestPage.ets(44:17)");
        ListItem.padding({ top: { "id": 16777368, "type": 10002, params: [] }, bottom: { "id": 16777368, "type": 10002, params: [] } });
        let earlierCreatedChild_3 = this.findChildById("3");
        if (earlierCreatedChild_3 == undefined) {
            View.create(new subEntryComponent_1.SubEntryComponent("3", this, { targetPage: 'pages/subManualApiTest/BrInterfaceTest', title: { "id": 16777249, "type": 10003, params: [] } }));
        }
        else {
            earlierCreatedChild_3.updateWithValueParams({
                targetPage: 'pages/subManualApiTest/BrInterfaceTest',
                title: { "id": 16777249, "type": 10003, params: [] }
            });
            View.create(earlierCreatedChild_3);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.debugLine("pages/manualApiTestPage.ets(48:17)");
        ListItem.padding({ top: { "id": 16777368, "type": 10002, params: [] }, bottom: { "id": 16777368, "type": 10002, params: [] } });
        let earlierCreatedChild_4 = this.findChildById("4");
        if (earlierCreatedChild_4 == undefined) {
            View.create(new subEntryComponent_1.SubEntryComponent("4", this, { targetPage: "pages/subManualApiTest/bleTest", title: { "id": 16777227, "type": 10003, params: [] } }));
        }
        else {
            earlierCreatedChild_4.updateWithValueParams({
                targetPage: "pages/subManualApiTest/bleTest",
                title: { "id": 16777227, "type": 10003, params: [] }
            });
            View.create(earlierCreatedChild_4);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.debugLine("pages/manualApiTestPage.ets(52:17)");
        ListItem.padding({ top: { "id": 16777368, "type": 10002, params: [] }, bottom: { "id": 16777368, "type": 10002, params: [] } });
        let earlierCreatedChild_5 = this.findChildById("5");
        if (earlierCreatedChild_5 == undefined) {
            View.create(new subEntryComponent_1.SubEntryComponent("5", this, { targetPage: "pages/subManualApiTest/gattTest", title: { "id": 16777260, "type": 10003, params: [] } }));
        }
        else {
            earlierCreatedChild_5.updateWithValueParams({
                targetPage: "pages/subManualApiTest/gattTest",
                title: { "id": 16777260, "type": 10003, params: [] }
            });
            View.create(earlierCreatedChild_5);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.debugLine("pages/manualApiTestPage.ets(56:17)");
        ListItem.padding({ top: { "id": 16777368, "type": 10002, params: [] }, bottom: { "id": 16777368, "type": 10002, params: [] } });
        let earlierCreatedChild_6 = this.findChildById("6");
        if (earlierCreatedChild_6 == undefined) {
            View.create(new subEntryComponent_1.SubEntryComponent("6", this, { targetPage: "pages/subManualApiTest/sppTest", title: { "id": 16777279, "type": 10003, params: [] } }));
        }
        else {
            earlierCreatedChild_6.updateWithValueParams({
                targetPage: "pages/subManualApiTest/sppTest",
                title: { "id": 16777279, "type": 10003, params: [] }
            });
            View.create(earlierCreatedChild_6);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.debugLine("pages/manualApiTestPage.ets(60:17)");
        ListItem.padding({ top: { "id": 16777368, "type": 10002, params: [] }, bottom: { "id": 16777368, "type": 10002, params: [] } });
        let earlierCreatedChild_7 = this.findChildById("7");
        if (earlierCreatedChild_7 == undefined) {
            View.create(new subEntryComponent_1.SubEntryComponent("7", this, { targetPage: "pages/subManualApiTest/profileTest", title: { "id": 16777272, "type": 10003, params: [] } }));
        }
        else {
            earlierCreatedChild_7.updateWithValueParams({
                targetPage: "pages/subManualApiTest/profileTest",
                title: { "id": 16777272, "type": 10003, params: [] }
            });
            View.create(earlierCreatedChild_7);
        }
        ListItem.pop();
        List.pop();
        Column.pop();
        Column.pop();
        Column.create();
        Column.debugLine("pages/manualApiTestPage.ets(76:11)");
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
loadDocument(new ManualApiTestPage("1", undefined, {}));


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/component/headercomponent.ets":
/*!**************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/component/headercomponent.ets ***!
  \**************************************************************************************************************************/
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
const ConfigData_1 = __importDefault(__webpack_require__(/*! ../Utils/ConfigData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Utils/ConfigData.ts"));
var _system_router_1  = globalThis.requireNativeModule('system.router');
class HeadTableComponent extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.isActive = true;
        this.icBackIsVisibility = true;
        this.headName = '';
        this.__isTouch = new ObservedPropertySimple(false, this, "isTouch");
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.isActive !== undefined) {
            this.isActive = params.isActive;
        }
        if (params.icBackIsVisibility !== undefined) {
            this.icBackIsVisibility = params.icBackIsVisibility;
        }
        if (params.headName !== undefined) {
            this.headName = params.headName;
        }
        if (params.isTouch !== undefined) {
            this.isTouch = params.isTouch;
        }
    }
    aboutToBeDeleted() {
        this.__isTouch.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id());
    }
    get isTouch() {
        return this.__isTouch.get();
    }
    set isTouch(newValue) {
        this.__isTouch.set(newValue);
    }
    render() {
        Row.create();
        Row.debugLine("../component/headercomponent.ets(30:5)");
        Row.width(ConfigData_1.default.WH_100_100);
        Row.padding({ left: { "id": 16777526, "type": 10002, params: [] } });
        Row.height({ "id": 16777516, "type": 10002, params: [] });
        Row.alignItems(VerticalAlign.Center);
        Row.align(Alignment.Start);
        Stack.create({ alignContent: Alignment.Center });
        Stack.debugLine("../component/headercomponent.ets(31:7)");
        Stack.margin({ left: { "id": 16777507, "type": 10002, params: [] }, right: { "id": 16777477, "type": 10002, params: [] } });
        Stack.backgroundColor(this.isTouch ? { "id": 16777295, "type": 10001, params: [] } : { "id": 16777290, "type": 10001, params: [] });
        Stack.visibility(this.icBackIsVisibility ? Visibility.Visible : Visibility.None);
        Stack.onClick(() => {
            _system_router_1.back();
        });
        Stack.onTouch((event) => {
            if (event.type === TouchType.Down) {
                this.isTouch = true;
            }
            if (event.type === TouchType.Up) {
                this.isTouch = false;
            }
        });
        Image.create({ "id": 16777552, "type": 20000, params: [] });
        Image.debugLine("../component/headercomponent.ets(32:9)");
        Image.width({ "id": 16777500, "type": 10002, params: [] });
        Image.height({ "id": 16777500, "type": 10002, params: [] });
        Stack.pop();
        Text.create(this.headName);
        Text.debugLine("../component/headercomponent.ets(51:7)");
        Text.fontSize({ "id": 16777394, "type": 10002, params: [] });
        Text.lineHeight({ "id": 16777503, "type": 10002, params: [] });
        Text.fontFamily('HarmonyHeiTi-Bold');
        Text.fontWeight(FontWeight.Bold);
        Text.fontColor({ "id": 16777299, "type": 10001, params: [] });
        Text.maxLines(ConfigData_1.default.MAX_LINES_1);
        Text.textOverflow({ overflow: TextOverflow.Ellipsis });
        Text.textAlign(TextAlign.Start);
        Text.margin({ top: { "id": 16777474, "type": 10002, params: [] }, bottom: { "id": 16777476, "type": 10002, params: [] } });
        Text.pop();
        Row.pop();
    }
}
exports["default"] = HeadTableComponent;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/component/subEntryComponent.ets":
/*!****************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/component/subEntryComponent.ets ***!
  \****************************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {


var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.SubEntryComponentWithEndText = exports.SubEntryComponent = void 0;
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
const ConfigData_1 = __importDefault(__webpack_require__(/*! ../Utils/ConfigData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Utils/ConfigData.ts"));
class SubEntryComponent extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.targetPage = undefined;
        this.title = undefined;
        this.__isTouched = new ObservedPropertySimple(false, this, "isTouched");
        this.date = null;
        this.deviceId = null;
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.targetPage !== undefined) {
            this.targetPage = params.targetPage;
        }
        if (params.title !== undefined) {
            this.title = params.title;
        }
        if (params.isTouched !== undefined) {
            this.isTouched = params.isTouched;
        }
        if (params.date !== undefined) {
            this.date = params.date;
        }
        if (params.deviceId !== undefined) {
            this.deviceId = params.deviceId;
        }
    }
    aboutToBeDeleted() {
        this.__isTouched.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id());
    }
    get isTouched() {
        return this.__isTouched.get();
    }
    set isTouched(newValue) {
        this.__isTouched.set(newValue);
    }
    render() {
        Navigator.create({ target: this.targetPage });
        Navigator.debugLine("../component/subEntryComponent.ets(29:5)");
        Navigator.params({ date: this.date, deviceId: this.deviceId });
        Navigator.padding({ "id": 16777377, "type": 10002, params: [] });
        Navigator.height({ "id": 16777467, "type": 10002, params: [] });
        Navigator.borderRadius({ "id": 16777428, "type": 10002, params: [] });
        Navigator.backgroundColor({ "id": 125829123, "type": 10001, params: [] });
        Flex.create({ justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center });
        Flex.debugLine("../component/subEntryComponent.ets(30:7)");
        Flex.height(ConfigData_1.default.WH_100_100);
        Flex.width(ConfigData_1.default.WH_100_100);
        Flex.borderRadius({ "id": 16777426, "type": 10002, params: [] });
        Flex.linearGradient(this.isTouched ? {
            angle: 90,
            direction: GradientDirection.Right,
            colors: [[{ "id": 16777284, "type": 10001, params: [] }, 0.0], [{ "id": 16777285, "type": 10001, params: [] }, 1.0]]
        } : {
            angle: 90,
            direction: GradientDirection.Right,
            colors: [[{ "id": 125829123, "type": 10001, params: [] }, 1], [{ "id": 125829123, "type": 10001, params: [] }, 1]]
        });
        Flex.onTouch((event) => {
            if (event.type === TouchType.Down) {
                this.isTouched = true;
            }
            if (event.type === TouchType.Up) {
                this.isTouched = false;
            }
        });
        Row.create();
        Row.debugLine("../component/subEntryComponent.ets(31:9)");
        Text.create(this.title);
        Text.debugLine("../component/subEntryComponent.ets(32:11)");
        Text.fontSize({ "id": 16777396, "type": 10002, params: [] });
        Text.lineHeight({ "id": 16777501, "type": 10002, params: [] });
        Text.fontColor({ "id": 16777299, "type": 10001, params: [] });
        Text.fontWeight(FontWeight.Medium);
        Text.margin({ left: { "id": 16777381, "type": 10002, params: [] } });
        Text.textAlign(TextAlign.Start);
        Text.height({ "id": 16777501, "type": 10002, params: [] });
        Text.pop();
        Row.pop();
        Image.create(('app.media.ic_settings_arrow'));
        Image.debugLine("../component/subEntryComponent.ets(41:9)");
        Image.width({ "id": 16777472, "type": 10002, params: [] });
        Image.height({ "id": 16777467, "type": 10002, params: [] });
        Image.margin({ right: { "id": 16777381, "type": 10002, params: [] } });
        Flex.pop();
        Navigator.pop();
    }
}
exports.SubEntryComponent = SubEntryComponent;
class SubEntryComponentWithEndText extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.__isTouched = new ObservedPropertySimple(false, this, "isTouched");
        this.__endText = new SynchedPropertySimpleOneWay(params.endText, this, "endText");
        this.targetPage = undefined;
        this.title = undefined;
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.isTouched !== undefined) {
            this.isTouched = params.isTouched;
        }
        this.endText = params.endText;
        if (params.targetPage !== undefined) {
            this.targetPage = params.targetPage;
        }
        if (params.title !== undefined) {
            this.title = params.title;
        }
    }
    aboutToBeDeleted() {
        this.__isTouched.aboutToBeDeleted();
        this.__endText.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id());
    }
    get isTouched() {
        return this.__isTouched.get();
    }
    set isTouched(newValue) {
        this.__isTouched.set(newValue);
    }
    get endText() {
        return this.__endText.get();
    }
    set endText(newValue) {
        this.__endText.set(newValue);
    }
    render() {
        Navigator.create({ target: this.targetPage });
        Navigator.debugLine("../component/subEntryComponent.ets(86:5)");
        Navigator.padding({ "id": 16777377, "type": 10002, params: [] });
        Navigator.height({ "id": 16777516, "type": 10002, params: [] });
        Navigator.borderRadius({ "id": 16777428, "type": 10002, params: [] });
        Navigator.backgroundColor({ "id": 125829123, "type": 10001, params: [] });
        Flex.create({ justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center });
        Flex.debugLine("../component/subEntryComponent.ets(87:7)");
        Flex.height(ConfigData_1.default.WH_100_100);
        Flex.width(ConfigData_1.default.WH_100_100);
        Flex.borderRadius({ "id": 16777427, "type": 10002, params: [] });
        Flex.linearGradient(this.isTouched ? {
            angle: 90,
            direction: GradientDirection.Right,
            colors: [[{ "id": 16777284, "type": 10001, params: [] }, 0.0], [{ "id": 16777285, "type": 10001, params: [] }, 1.0]]
        } : {
            angle: 90,
            direction: GradientDirection.Right,
            colors: [[{ "id": 125829123, "type": 10001, params: [] }, 1], [{ "id": 125829123, "type": 10001, params: [] }, 1]]
        });
        Flex.onTouch((event) => {
            if (event.type === TouchType.Down) {
                this.isTouched = true;
            }
            if (event.type === TouchType.Up) {
                this.isTouched = false;
            }
        });
        Row.create();
        Row.debugLine("../component/subEntryComponent.ets(88:9)");
        Text.create(this.title);
        Text.debugLine("../component/subEntryComponent.ets(89:11)");
        Text.fontSize({ "id": 16777389, "type": 10002, params: [] });
        Text.lineHeight({ "id": 16777487, "type": 10002, params: [] });
        Text.fontWeight(FontWeight.Medium);
        Text.fontColor({ "id": 16777299, "type": 10001, params: [] });
        Text.margin({ left: { "id": 16777381, "type": 10002, params: [] } });
        Text.textAlign(TextAlign.Start);
        Text.pop();
        Row.pop();
        Row.create();
        Row.debugLine("../component/subEntryComponent.ets(98:9)");
        Text.create(this.endText);
        Text.debugLine("../component/subEntryComponent.ets(99:11)");
        Text.fontSize({ "id": 16777388, "type": 10002, params: [] });
        Text.lineHeight({ "id": 16777480, "type": 10002, params: [] });
        Text.fontWeight(FontWeight.Regular);
        Text.fontColor({ "id": 125829216, "type": 10001, params: [] });
        Text.margin({ right: { "id": 16777377, "type": 10002, params: [] } });
        Text.textAlign(TextAlign.End);
        Text.pop();
        Image.create(('app.media.ic_settings_arrow'));
        Image.debugLine("../component/subEntryComponent.ets(106:11)");
        Image.width({ "id": 16777472, "type": 10002, params: [] });
        Image.height({ "id": 16777489, "type": 10002, params: [] });
        Image.margin({ right: { "id": 16777381, "type": 10002, params: [] } });
        Row.pop();
        Flex.pop();
        Navigator.pop();
    }
}
exports.SubEntryComponentWithEndText = SubEntryComponentWithEndText;


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
/******/ 			"./pages/manualApiTestPage": 0
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
/******/ 	var __webpack_exports__ = __webpack_require__.O(undefined, ["commons"], () => (__webpack_require__("../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/pages/manualApiTestPage.ets?entry")))
/******/ 	__webpack_exports__ = __webpack_require__.O(__webpack_exports__);
/******/ 	_840471a2ef2afe531ee6aea9267b2e21 = __webpack_exports__;
/******/ 	
/******/ })()
;
//# sourceMappingURL=manualApiTestPage.js.map