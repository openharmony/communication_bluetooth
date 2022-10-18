(globalThis["webpackChunk_2a33209e6af7922da1521a8a828bfbd0"] = globalThis["webpackChunk_2a33209e6af7922da1521a8a828bfbd0"] || []).push([["commons"],{

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Component/contentTable.ets":
/*!*******************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Component/contentTable.ets ***!
  \*******************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.ContentTable = void 0;
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
// @ts-nocheck
__webpack_require__(/*! ../MainAbility/model/TestData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/TestData.ets");
__webpack_require__(/*! ../MainAbility/model/TestDataModels */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/TestDataModels.ets");
var _system_prompt_1  = isSystemplugin('prompt', 'system') ? globalThis.systemplugin.prompt : globalThis.requireNapi('prompt');
__webpack_require__(/*! @ohos.bluetooth */ "../../api/@ohos.bluetooth.d.ts");
class InputDialog extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.hitText = undefined;
        this.__textValue = new SynchedPropertySimpleTwoWay(params.textValue, this, "textValue");
        this.__inputValue = new SynchedPropertySimpleTwoWay(params.inputValue, this, "inputValue");
        this.__apiItem = new SynchedPropertyObjectTwoWay(params.apiItem, this, "apiItem");
        this.__currentIndex = new SynchedPropertySimpleTwoWay(params.currentIndex, this, "currentIndex");
        this.__changeIndex = new SynchedPropertySimpleTwoWay(params.changeIndex, this, "changeIndex");
        this.controller = undefined;
        this.cancel = undefined;
        this.confirm = undefined;
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.hitText !== undefined) {
            this.hitText = params.hitText;
        }
        if (params.controller !== undefined) {
            this.controller = params.controller;
        }
        if (params.cancel !== undefined) {
            this.cancel = params.cancel;
        }
        if (params.confirm !== undefined) {
            this.confirm = params.confirm;
        }
    }
    aboutToBeDeleted() {
        this.__textValue.aboutToBeDeleted();
        this.__inputValue.aboutToBeDeleted();
        this.__apiItem.aboutToBeDeleted();
        this.__currentIndex.aboutToBeDeleted();
        this.__changeIndex.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id());
    }
    get textValue() {
        return this.__textValue.get();
    }
    set textValue(newValue) {
        this.__textValue.set(newValue);
    }
    get inputValue() {
        return this.__inputValue.get();
    }
    set inputValue(newValue) {
        this.__inputValue.set(newValue);
    }
    get apiItem() {
        return this.__apiItem.get();
    }
    set apiItem(newValue) {
        this.__apiItem.set(newValue);
    }
    get currentIndex() {
        return this.__currentIndex.get();
    }
    set currentIndex(newValue) {
        this.__currentIndex.set(newValue);
    }
    get changeIndex() {
        return this.__changeIndex.get();
    }
    set changeIndex(newValue) {
        this.__changeIndex.set(newValue);
    }
    setController(ctr) {
        this.controller = ctr;
    }
    render() {
        Column.create();
        Text.create(this.hitText);
        Text.fontSize(20);
        Text.margin({ top: 10, bottom: 10 });
        Text.pop();
        TextInput.create({ placeholder: '', text: this.textValue });
        TextInput.height(60);
        TextInput.width('90%');
        TextInput.onChange((value) => {
            this.textValue = value;
        });
        Flex.create({ justifyContent: FlexAlign.SpaceAround });
        Flex.margin({ bottom: 10 });
        Button.createWithLabel('取消');
        Button.onClick(() => {
            this.controller.close();
            this.cancel();
        });
        Button.backgroundColor(0xffffff);
        Button.fontColor(Color.Black);
        Button.pop();
        Button.createWithLabel('确认');
        Button.onClick(() => {
            this.inputValue = this.textValue;
            this.changeIndex = this.currentIndex;
            //TODO: 设备调试时候需要打开
            let message = "";
            if (false) {}
            else {
                this.apiItem.result = this.textValue;
            }
            this.controller.close();
            this.confirm(this.inputValue);
        });
        Button.backgroundColor(0xffffff);
        Button.fontColor(Color.Red);
        Button.pop();
        Flex.pop();
        Column.pop();
    }
}
class ContentTable extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.testItem = undefined;
        this.__currentApiItem = new ObservedPropertyObject({}, this, "currentApiItem");
        this.__apiItems = new ObservedPropertyObject([], this, "apiItems");
        this.__changeIndex = new ObservedPropertySimple(-1, this, "changeIndex");
        this.__localName = new ObservedPropertySimple('ohosbt', this, "localName");
        this.__currentIndex = new ObservedPropertySimple(-1, this, "currentIndex");
        this.dialogController = new CustomDialogController({
            builder: () => {
                let jsDialog = new InputDialog("2", this, {
                    hitText: "请输入MAC地址",
                    cancel: this.onCancel,
                    confirm: this.onAccept,
                    textValue: this.__localName,
                    inputValue: this.__localName,
                    currentIndex: this.__currentIndex,
                    apiItem: this.__currentApiItem,
                    changeIndex: this.__changeIndex,
                });
                jsDialog.setController(this.dialogController);
                View.create(jsDialog);
            },
            cancel: this.onCancel(),
            autoCancel: true
        }, this);
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.testItem !== undefined) {
            this.testItem = params.testItem;
        }
        if (params.currentApiItem !== undefined) {
            this.currentApiItem = params.currentApiItem;
        }
        if (params.apiItems !== undefined) {
            this.apiItems = params.apiItems;
        }
        if (params.changeIndex !== undefined) {
            this.changeIndex = params.changeIndex;
        }
        if (params.localName !== undefined) {
            this.localName = params.localName;
        }
        if (params.currentIndex !== undefined) {
            this.currentIndex = params.currentIndex;
        }
        if (params.dialogController !== undefined) {
            this.dialogController = params.dialogController;
        }
    }
    aboutToBeDeleted() {
        this.__currentApiItem.aboutToBeDeleted();
        this.__apiItems.aboutToBeDeleted();
        this.__changeIndex.aboutToBeDeleted();
        this.__localName.aboutToBeDeleted();
        this.__currentIndex.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id());
    }
    get currentApiItem() {
        return this.__currentApiItem.get();
    }
    set currentApiItem(newValue) {
        this.__currentApiItem.set(newValue);
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
    get localName() {
        return this.__localName.get();
    }
    set localName(newValue) {
        this.__localName.set(newValue);
    }
    get currentIndex() {
        return this.__currentIndex.get();
    }
    set currentIndex(newValue) {
        this.__currentIndex.set(newValue);
    }
    onCancel() {
        console.info('取消');
    }
    onAccept(text) {
        console.info('input text:', text);
        //    this.currentApiItem.result = "inputtext:"+text+"localname:"+this.localName
        _system_prompt_1.showToast({ message: "localname:" + text });
    }
    IngredientItem(apiItem, index) {
        Stack.create();
        Stack.height(80);
        Stack.borderRadius(10);
        Stack.margin({ top: 5, bottom: 5 });
        Stack.backgroundColor({ "id": 16777305, "type": 10001, params: [] });
        Stack.onClick(() => {
            if (apiItem.method == "setLocalName") {
                this.currentApiItem = apiItem;
                this.currentIndex = index;
                this.dialogController.open();
            }
            else {
                let message = apiItem.api();
                apiItem.result = message;
                this.currentIndex = index;
                _system_prompt_1.showToast({ message: message });
            }
        });
        Flex.create();
        Flex.create({ alignItems: ItemAlign.Center });
        Flex.layoutWeight(1);
        Flex.padding({ left: 10 });
        Image.create({ "id": 16777543, "type": 20000, params: [] });
        Image.objectFit(ImageFit.Contain);
        Image.autoResize(false);
        Image.height(30);
        Image.width(30);
        Flex.pop();
        Flex.create({ direction: FlexDirection.Column, alignItems: ItemAlign.Start });
        Flex.layoutWeight(6);
        Text.create(apiItem.name);
        Text.fontSize(17.4);
        Text.fontWeight(FontWeight.Bold);
        Text.layoutWeight(1);
        Text.padding({ left: 10, top: 15 });
        Text.pop();
        Text.create(apiItem.method);
        Text.fontSize(17.4);
        Text.fontWeight(FontWeight.Bold);
        Text.layoutWeight(1);
        Text.padding({ left: 10, top: 5 });
        Text.pop();
        Flex.pop();
        Flex.create({ alignItems: ItemAlign.Center });
        Flex.layoutWeight(3);
        Flex.padding({ left: 10 });
        Text.create(this.changeIndex >= 0 ? this.getCurrentState(index) : apiItem.result);
        Text.fontSize(17.4);
        Text.flexGrow(1);
        Text.pop();
        Flex.pop();
        Flex.pop();
        Stack.pop();
    }
    getCurrentState(index) {
        //    console.log("currentState:" + index.toString() + this.apiItems[index].result)
        return this.apiItems[index].result;
    }
    render() {
        Column.create();
        Column.height("66%");
        Column.padding({ top: 5, right: 10, left: 10 });
        Scroll.create();
        Scroll.scrollBarWidth(20);
        Flex.create({ direction: FlexDirection.Column, justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Start });
        ForEach.create("1", this, ObservedObject.GetRawObject(this.apiItems), (item, index) => {
            this.IngredientItem(item, index);
        });
        ForEach.pop();
        Flex.pop();
        Scroll.pop();
        Column.pop();
    }
}
exports.ContentTable = ContentTable;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Component/headComponent.ets":
/*!********************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Component/headComponent.ets ***!
  \********************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

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
const ConfigData_1 = __importDefault(__webpack_require__(/*! ../Utils/ConfigData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Utils/ConfigData.ts"));
var _system_router_1  = globalThis.requireNativeModule('system.router');
class HeadComponent extends View {
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
        Row.width(ConfigData_1.default.WH_100_100);
        Row.padding({ left: { "id": 16777519, "type": 10002, params: [] } });
        Row.height({ "id": 16777509, "type": 10002, params: [] });
        Row.alignItems(VerticalAlign.Center);
        Row.align(Alignment.Start);
        Stack.create({ alignContent: Alignment.Center });
        Stack.margin({ left: { "id": 16777500, "type": 10002, params: [] }, right: { "id": 16777470, "type": 10002, params: [] } });
        Stack.backgroundColor(this.isTouch ? { "id": 16777288, "type": 10001, params: [] } : { "id": 16777283, "type": 10001, params: [] });
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
        Image.create({ "id": 16777545, "type": 20000, params: [] });
        Image.width({ "id": 16777493, "type": 10002, params: [] });
        Image.height({ "id": 16777493, "type": 10002, params: [] });
        Stack.pop();
        Text.create(this.headName);
        Text.fontSize({ "id": 16777387, "type": 10002, params: [] });
        Text.lineHeight({ "id": 16777496, "type": 10002, params: [] });
        Text.fontFamily('HarmonyHeiTi-Bold');
        Text.fontWeight(FontWeight.Bold);
        Text.fontColor({ "id": 16777292, "type": 10001, params: [] });
        Text.maxLines(ConfigData_1.default.MAX_LINES_1);
        Text.textOverflow({ overflow: TextOverflow.Ellipsis });
        Text.textAlign(TextAlign.Start);
        Text.margin({ top: { "id": 16777467, "type": 10002, params: [] }, bottom: { "id": 16777469, "type": 10002, params: [] } });
        Text.pop();
        Row.pop();
    }
}
exports["default"] = HeadComponent;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Component/pageTitle.ets":
/*!****************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Component/pageTitle.ets ***!
  \****************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.PageTitle = void 0;
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
__webpack_require__(/*! ../MainAbility/model/TestData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/TestData.ets");
var _system_router_1  = globalThis.requireNativeModule('system.router');
class PageTitle extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.testItem = null;
        this.detail = undefined;
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.testItem !== undefined) {
            this.testItem = params.testItem;
        }
        if (params.detail !== undefined) {
            this.detail = params.detail;
        }
    }
    aboutToBeDeleted() {
        SubscriberManager.Get().delete(this.id());
    }
    render() {
        Flex.create({ alignItems: ItemAlign.Start });
        Flex.height(61);
        Flex.backgroundColor('#FFedf2f5');
        Flex.padding({ top: 23, bottom: 15, left: 18 });
        Flex.borderColor({ "id": 16777280, "type": 10001, params: [] });
        Flex.backgroundColor({ "id": 16777281, "type": 10001, params: [] });
        Flex.onClick(() => {
            _system_router_1.back();
        });
        Image.create({ "id": 16777527, "type": 20000, params: [] });
        Image.width("10%");
        Image.height("100%");
        Text.create(this.testItem != null ? this.testItem.detail : this.detail);
        Text.fontSize(21.8);
        Text.margin({ left: 17.4 });
        Text.pop();
        Flex.pop();
    }
}
exports.PageTitle = PageTitle;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Component/testImageDisplay.ets":
/*!***********************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Component/testImageDisplay.ets ***!
  \***********************************************************************************************************************/
/***/ ((__unused_webpack_module, exports, __webpack_require__) => {

"use strict";

Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.TestImageDisplay = void 0;
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
__webpack_require__(/*! ../MainAbility/model/TestData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/TestData.ets");
class TestImageDisplay extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.testItem = undefined;
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.testItem !== undefined) {
            this.testItem = params.testItem;
        }
    }
    aboutToBeDeleted() {
        SubscriberManager.Get().delete(this.id());
    }
    render() {
        Stack.create({ alignContent: Alignment.BottomStart });
        Stack.backgroundColor('#FFedf2f5');
        Stack.height(160);
        Flex.create({ alignItems: ItemAlign.Start });
        Stack.create();
        Stack.width("20%");
        Image.create(this.testItem.image);
        Image.height(80);
        Image.width(80);
        Image.margin({ left: 20, bottom: 10 });
        Stack.pop();
        Column.create();
        Column.width("80%");
        Column.padding({ left: 20 });
        Text.create(this.testItem.name);
        Text.fontSize(26);
        Text.fontWeight(500);
        Text.margin({ left: 26, top: 10 });
        Text.width("90%");
        Text.pop();
        Text.create(this.testItem.detail);
        Text.fontSize(16);
        Text.fontWeight(500);
        Text.margin({ top: 8 });
        Text.width("80%");
        Text.pop();
        Column.pop();
        Flex.pop();
        Stack.pop();
    }
}
exports.TestImageDisplay = TestImageDisplay;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Component/titleComponent.ets":
/*!*********************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Component/titleComponent.ets ***!
  \*********************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.TitleComponent = void 0;
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
const ConfigData_1 = __importDefault(__webpack_require__(/*! ../Utils/ConfigData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Utils/ConfigData.ts"));
class TitleComponent extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.title = undefined;
        this.fontSize = '30vp';
        this.stateChangeFlag = false;
        this.pinRequiredFlag = false;
        this.bondStateChangeFlag = false;
        this.__bgColor = new ObservedPropertyObject({ "id": 16777291, "type": 10001, params: [] }, this, "bgColor");
        this.__isTouched = new ObservedPropertySimple(false, this, "isTouched");
        this.__state = AppStorage.GetOrCreate().setAndLink("on_stateChange", 'on:stateChange', this);
        this.__pin = AppStorage.GetOrCreate().setAndLink("on_pinRequired", 'on:pinRequired', this);
        this.__bondState = AppStorage.GetOrCreate().setAndLink("on_bondStateChange", 'on:bondStateChange', this);
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.title !== undefined) {
            this.title = params.title;
        }
        if (params.fontSize !== undefined) {
            this.fontSize = params.fontSize;
        }
        if (params.stateChangeFlag !== undefined) {
            this.stateChangeFlag = params.stateChangeFlag;
        }
        if (params.pinRequiredFlag !== undefined) {
            this.pinRequiredFlag = params.pinRequiredFlag;
        }
        if (params.bondStateChangeFlag !== undefined) {
            this.bondStateChangeFlag = params.bondStateChangeFlag;
        }
        if (params.bgColor !== undefined) {
            this.bgColor = params.bgColor;
        }
        if (params.isTouched !== undefined) {
            this.isTouched = params.isTouched;
        }
    }
    aboutToBeDeleted() {
        this.__bgColor.aboutToBeDeleted();
        this.__isTouched.aboutToBeDeleted();
        this.__state.aboutToBeDeleted();
        this.__pin.aboutToBeDeleted();
        this.__bondState.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id());
    }
    get bgColor() {
        return this.__bgColor.get();
    }
    set bgColor(newValue) {
        this.__bgColor.set(newValue);
    }
    get isTouched() {
        return this.__isTouched.get();
    }
    set isTouched(newValue) {
        this.__isTouched.set(newValue);
    }
    get state() {
        return this.__state.get();
    }
    set state(newValue) {
        this.__state.set(newValue);
    }
    get pin() {
        return this.__pin.get();
    }
    set pin(newValue) {
        this.__pin.set(newValue);
    }
    get bondState() {
        return this.__bondState.get();
    }
    set bondState(newValue) {
        this.__bondState.set(newValue);
    }
    render() {
        Column.create();
        Column.padding({ "id": 16777370, "type": 10002, params: [] });
        Column.height("100vp");
        Column.borderRadius({ "id": 16777419, "type": 10002, params: [] });
        Column.backgroundColor({ "id": 125829123, "type": 10001, params: [] });
        Flex.create({ justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center });
        Flex.height(80);
        Flex.width(ConfigData_1.default.WH_100_100);
        Flex.backgroundColor(ObservedObject.GetRawObject(this.bgColor));
        Flex.linearGradient(this.isTouched ? {
            angle: 90,
            direction: GradientDirection.Right,
            colors: [[{ "id": 16777277, "type": 10001, params: [] }, 0.0], [{ "id": 16777278, "type": 10001, params: [] }, 1.0]]
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
        Text.create(this.stateChangeFlag ? this.state : (this.pinRequiredFlag ? this.pin : (this.bondStateChangeFlag ? this.bondState : this.title)));
        Text.textAlign(TextAlign.Center);
        Text.width("100%");
        Text.height(70);
        Text.fontSize(this.fontSize);
        Text.fontColor({ "id": 16777292, "type": 10001, params: [] });
        Text.fontWeight(FontWeight.Medium);
        Text.pop();
        Flex.pop();
        Column.pop();
    }
}
exports.TitleComponent = TitleComponent;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/controller/BluetoothDeviceController.ts":
/*!********************************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/controller/BluetoothDeviceController.ts ***!
  \********************************************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    Object.defineProperty(o, k2, { enumerable: true, get: function() { return m[k]; } });
}) : (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}));
var __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {
    Object.defineProperty(o, "default", { enumerable: true, value: v });
}) : function(o, v) {
    o["default"] = v;
});
var __importStar = (this && this.__importStar) || function (mod) {
    if (mod && mod.__esModule) return mod;
    var result = {};
    if (mod != null) for (var k in mod) if (k !== "default" && Object.prototype.hasOwnProperty.call(mod, k)) __createBinding(result, mod, k);
    __setModuleDefault(result, mod);
    return result;
};
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
// @ts-nocheck
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
const BluetoothModel_1 = __importStar(__webpack_require__(/*! ../model/BluetoothModel */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/BluetoothModel.ts"));
const ConfigData_1 = __importDefault(__webpack_require__(/*! ../../Utils/ConfigData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Utils/ConfigData.ts"));
const LogUtil_1 = __importDefault(__webpack_require__(/*! ../../Utils/LogUtil */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Utils/LogUtil.ts"));
const BluetoothDevice_1 = __importDefault(__webpack_require__(/*! ../model/BluetoothDevice */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/BluetoothDevice.ts"));
const BaseSettingsController_1 = __importDefault(__webpack_require__(/*! ../../component/controller/BaseSettingsController */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/controller/BaseSettingsController.ts"));
__webpack_require__(/*! ../../component/controller/ISettingsController */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/controller/ISettingsController.ts");
const DISCOVERY_DURING_TIME = 30000; // 30'
const DISCOVERY_INTERVAL_TIME = 3000; // 3'
class BluetoothDeviceController extends BaseSettingsController_1.default {
    constructor() {
        super(...arguments);
        this.TAG = ConfigData_1.default.TAG + 'BluetoothDeviceController ';
        //state
        this.isOn = false;
        this.enabled = false;
        // paired devices
        this.pairedDevices = [];
        // available devices
        this.isDeviceDiscovering = false;
        this.availableDevices = [];
        this.pairPinCode = '';
    }
    initData() {
        super.initData();
        let isOn = BluetoothModel_1.default.isStateOn();
        LogUtil_1.default.log(this.TAG + 'initData bluetooth state isOn ' + isOn + ', typeof isOn = ' + typeof (isOn));
        if (isOn) {
            this.refreshPairedDevices();
        }
        LogUtil_1.default.log(this.TAG + 'initData save value to app storage. ');
        this.isOn = new Boolean(isOn).valueOf();
        this.enabled = true;
        AppStorage.SetOrCreate('bluetoothIsOn', this.isOn);
        AppStorage.SetOrCreate('bluetoothToggleEnabled', this.enabled);
        AppStorage.SetOrCreate('bluetoothAvailableDevices', this.availableDevices);
        return this;
    }
    subscribe() {
        LogUtil_1.default.log(this.TAG + 'subscribe bluetooth state isOn ' + this.isOn);
        this.subscribeStateChange();
        this.subscribeBluetoothDeviceFind();
        this.subscribeBondStateChange();
        this.subscribeDeviceConnectStateChange();
        if (this.isOn) {
            this.startBluetoothDiscovery();
        }
        return this;
    }
    unsubscribe() {
        this.stopBluetoothDiscovery();
        if (this.discoveryStartTimeoutId) {
            clearTimeout(this.discoveryStartTimeoutId);
        }
        if (this.discoveryStopTimeoutId) {
            clearTimeout(this.discoveryStopTimeoutId);
        }
        BluetoothModel_1.default.unsubscribeStateChange();
        BluetoothModel_1.default.unsubscribeBluetoothDeviceFind();
        BluetoothModel_1.default.unsubscribeBondStateChange();
        BluetoothModel_1.default.unsubscribeDeviceStateChange();
        return this;
    }
    /**
     * Set toggle value
     */
    toggleValue(isOn) {
        this.enabled = false;
        AppStorage.SetOrCreate('bluetoothToggleEnabled', this.enabled);
        LogUtil_1.default.log(this.TAG + 'afterCurrentValueChanged bluetooth state isOn = ' + this.isOn);
        if (isOn) {
            BluetoothModel_1.default.enableBluetooth();
        }
        else {
            BluetoothModel_1.default.disableBluetooth();
        }
    }
    /**
     * Get Local Name
     */
    getLocalName() {
        AppStorage.SetOrCreate('bluetoothLocalName', BluetoothModel_1.default.getLocalName());
    }
    /**
     * Pair device.
     *
     * @param deviceId device id
     * @param success success callback
     * @param error error callback
     */
    pair(deviceId, success, error) {
        const device = this.getAvailableDevice(deviceId);
        if (device && device.connectionState === BluetoothModel_1.DeviceState.STATE_PAIRING) {
            LogUtil_1.default.log(this.TAG + `bluetooth no Aavailable device or device(${deviceId}) is already pairing.`);
            return;
        }
        // start listening pinCode
        BluetoothModel_1.default.subscribePinRequired((pinRequiredParam) => {
            LogUtil_1.default.log(this.TAG + 'bluetooth subscribePinRequired callback. pinRequiredParam = ' + JSON.stringify(pinRequiredParam));
            this.pairPinCode = pinRequiredParam.pinCode;
            BluetoothModel_1.default.unsubscribePinRequired(() => LogUtil_1.default.log(this.TAG + 'available pinRequired unsubscribed.'));
            if (success) {
                success(this.pairPinCode);
            }
        });
        // start pairing
        const result = BluetoothModel_1.default.pairDevice(deviceId);
        LogUtil_1.default.log(this.TAG + 'bluetooth pairDevice result = ' + result);
        if (!result) {
            AppStorage.SetOrCreate('pairedDeviceId', '');
            BluetoothModel_1.default.unsubscribePinRequired(() => LogUtil_1.default.log(this.TAG + 'available pinRequired unsubscribed.'));
            if (error) {
                error();
            }
        }
    }
    /**
     * Confirm pairing.
     *
     * @param deviceId device id
     * @param accept accept or not
     * @param success success callback
     * @param error error callback
     */
    confirmPairing(deviceId, accept) {
        if (accept) {
            this.getAvailableDevice(deviceId).connectionState = BluetoothModel_1.DeviceState.STATE_PAIRING;
            this.forceRefresh(this.availableDevices);
            AppStorage.SetOrCreate('bluetoothAvailableDevices', this.availableDevices);
        }
        // set paring confirmation
        BluetoothModel_1.default.setDevicePairingConfirmation(deviceId, accept);
    }
    /**
     * Connect device.
     * @param deviceId device id
     */
    connect(deviceId) {
        return BluetoothModel_1.default.connectDevice(deviceId);
    }
    /**
     * disconnect device.
     * @param deviceId device id
     */
    disconnect(deviceId) {
        return BluetoothModel_1.default.disconnectDevice(deviceId);
    }
    /**
     * Unpair device.
     * @param deviceId device id
     */
    unpair(deviceId) {
        const result = BluetoothModel_1.default.unpairDevice(deviceId);
        LogUtil_1.default.log(this.TAG + 'bluetooth paired device unpair. result = ' + result);
        this.refreshPairedDevices();
        return result;
    }
    /**
     * Refresh paired devices.
     */
    refreshPairedDevices() {
        let deviceIds = BluetoothModel_1.default.getPairedDeviceIds();
        let list = [];
        deviceIds.forEach(deviceId => {
            list.push(this.getDevice(deviceId));
        });
        this.pairedDevices = list;
        AppStorage.SetOrCreate('bluetoothPairedDevices', this.pairedDevices);
        LogUtil_1.default.log(this.TAG + 'bluetooth paired devices. list = ' + JSON.stringify(list));
    }
    //---------------------- subscribe ----------------------
    /**
     * Subscribe bluetooth state change
     */
    subscribeStateChange() {
        BluetoothModel_1.default.subscribeStateChange((isOn) => {
            LogUtil_1.default.log(this.TAG + 'bluetooth state changed. isOn = ' + isOn);
            this.isOn = new Boolean(isOn).valueOf();
            this.enabled = true;
            LogUtil_1.default.log(this.TAG + 'bluetooth state changed. save value.');
            this.getLocalName();
            AppStorage.SetOrCreate('bluetoothIsOn', this.isOn);
            AppStorage.SetOrCreate('bluetoothToggleEnabled', this.enabled);
            if (isOn) {
                LogUtil_1.default.log(this.TAG + 'bluetooth state changed. unsubscribe');
                this.startBluetoothDiscovery();
            }
            else {
                LogUtil_1.default.log(this.TAG + 'bluetooth state changed. subscribe');
                this.stopBluetoothDiscovery();
            }
        });
    }
    /**
     * Subscribe device find
     */
    subscribeBluetoothDeviceFind() {
        BluetoothModel_1.default.subscribeBluetoothDeviceFind((deviceIds) => {
            LogUtil_1.default.log(ConfigData_1.default.TAG + 'available bluetooth devices changed.');
            deviceIds === null || deviceIds === void 0 ? void 0 : deviceIds.forEach(deviceId => {
                let device = this.availableDevices.find((availableDevice) => {
                    return availableDevice.deviceId === deviceId;
                });
                LogUtil_1.default.log(this.TAG + 'available bluetooth find [' + deviceId + '] = ' + JSON.stringify(device));
                if (!device) {
                    let newDevice = this.getDevice(deviceId);
                    LogUtil_1.default.log(this.TAG + 'available bluetooth new device : ' + JSON.stringify(newDevice));
                    this.availableDevices.push(newDevice);
                    LogUtil_1.default.log(this.TAG + 'available bluetooth new device pushed. availableDevices length = ' + this.availableDevices.length);
                }
            });
            AppStorage.SetOrCreate('bluetoothAvailableDevices', this.availableDevices);
        });
    }
    /**
     * Subscribe bond state change
     */
    subscribeBondStateChange() {
        BluetoothModel_1.default.subscribeBondStateChange((data) => {
            LogUtil_1.default.log(this.TAG + 'bluetooth subscribeBondStateChange callback.');
            //paired devices
            if (data.bondState !== BluetoothModel_1.BondState.BOND_STATE_BONDING) {
                this.refreshPairedDevices();
            }
            //available devices
            if (data.bondState == BluetoothModel_1.BondState.BOND_STATE_BONDING) {
                // case bonding
                // do nothing and still listening
                LogUtil_1.default.log(this.TAG + 'bluetooth continue listening bondStateChange.');
            }
            else if (data.bondState == BluetoothModel_1.BondState.BOND_STATE_INVALID) {
                // case failed
                this.getAvailableDevice(data.deviceId).connectionState = BluetoothModel_1.DeviceState.STATE_DISCONNECTED;
                this.forceRefresh(this.availableDevices);
                AppStorage.SetOrCreate('bluetoothAvailableDevices', this.availableDevices);
                this.showConnectFailedDialog();
            }
            else if (data.bondState == BluetoothModel_1.BondState.BOND_STATE_BONDED) {
                // case success
                LogUtil_1.default.log(this.TAG + 'bluetooth bonded : remove device.');
                this.removeAvailableDevice(data.deviceId);
                BluetoothModel_1.default.connectDevice(data.deviceId);
            }
        });
    }
    /**
     * Subscribe device connect state change
     */
    subscribeDeviceConnectStateChange() {
        BluetoothModel_1.default.subscribeDeviceStateChange((data) => {
            LogUtil_1.default.log(this.TAG + 'device connection state changed. data = ' + JSON.stringify(data));
            for (let device of this.pairedDevices) {
                if (device.deviceId === data.deviceId) {
                    device.setProfile(data);
                    this.forceRefresh(this.pairedDevices);
                    AppStorage.SetOrCreate('bluetoothPairedDevices', this.pairedDevices);
                    break;
                }
            }
            ;
            LogUtil_1.default.log(this.TAG + 'device connection state changed. pairedDevices = ' + JSON.stringify(this.pairedDevices));
            LogUtil_1.default.log(this.TAG + 'device connection state changed. availableDevices = ' + JSON.stringify(this.availableDevices));
            this.removeAvailableDevice(data.deviceId);
        });
    }
    //---------------------- private ----------------------
    /**
     * Get device by device id.
     * @param deviceId device id
     */
    getDevice(deviceId) {
        let device = new BluetoothDevice_1.default();
        device.deviceId = deviceId;
        device.deviceName = BluetoothModel_1.default.getDeviceName(deviceId);
        device.deviceType = BluetoothModel_1.default.getDeviceType(deviceId);
        device.setProfiles(BluetoothModel_1.default.getDeviceState(deviceId));
        return device;
    }
    /**
       * Force refresh array.
       * Note: the purpose of this function is just trying to fix page (ets) level's bug below,
       *   and should be useless if fixed by the future sdk.
       * Bug Details:
       *   @State is not supported well for Array<CustomClass> type.
       *   In the case that the array item's field value changed, while not its length,
       *   the build method on page will not be triggered!
       */
    forceRefresh(arr) {
        arr.push(new BluetoothDevice_1.default());
        arr.pop();
    }
    /**
     * Start bluetooth discovery.
     */
    startBluetoothDiscovery() {
        this.isDeviceDiscovering = true;
        BluetoothModel_1.default.startBluetoothDiscovery();
        this.discoveryStopTimeoutId = setTimeout(() => {
            this.stopBluetoothDiscovery();
        }, DISCOVERY_DURING_TIME);
    }
    /**
     * Stop bluetooth discovery.
     */
    stopBluetoothDiscovery() {
        this.isDeviceDiscovering = false;
        BluetoothModel_1.default.stopBluetoothDiscovery();
        this.discoveryStartTimeoutId = setTimeout(() => {
            this.startBluetoothDiscovery();
        }, DISCOVERY_INTERVAL_TIME);
    }
    /**
     * Get available device.
     *
     * @param deviceId device id
     */
    getAvailableDevice(deviceId) {
        this.availableDevices.forEach(device => {
            if (device.deviceId === deviceId) {
                return device;
            }
        });
        return null;
    }
    /**
     * Remove available device.
     *
     * @param deviceId device id
     */
    removeAvailableDevice(deviceId) {
        LogUtil_1.default.log(this.TAG + 'removeAvailableDevice : before : availableDevices length = ' + this.availableDevices.length);
        this.availableDevices = this.availableDevices.filter((device) => device.deviceId !== deviceId);
        AppStorage.SetOrCreate('bluetoothAvailableDevices', this.availableDevices);
        LogUtil_1.default.log(this.TAG + 'removeAvailableDevice : after : availableDevices length = ' + this.availableDevices.length);
    }
    /**
     * Connect Failed Dialog
     */
    showConnectFailedDialog() {
        AlertDialog.show({
            title: { "id": 16777228, "type": 10003, params: [] },
            message: { "id": 16777229, "type": 10003, params: [] },
            confirm: {
                value: { "id": 16777233, "type": 10003, params: [] },
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
}
exports["default"] = BluetoothDeviceController;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/BLEInterface.ets":
/*!***************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/BLEInterface.ets ***!
  \***************************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.testOnBLEDeviceFind = exports.testStopBLEScan = exports.testStartBLEScan = exports.testGetConnectedBLEDevices = exports.testCreateGattClientDevice = exports.testCreateGattServer = void 0;
// @ts-nocheck
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
var _system_prompt_1  = isSystemplugin('prompt', 'system') ? globalThis.systemplugin.prompt : globalThis.requireNapi('prompt');
const BluetoothModel_1 = __importDefault(__webpack_require__(/*! ./BluetoothModel */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/BluetoothModel.ts"));
//// import Router from '@system.router';
//import BluetoothDevice from '../../model/BluetoothDevice'
//import BluetoothDeviceController from '../../controller/BluetoothDeviceController';
//import ConfigData from '../../../Utils/ConfigData';
//import LogUtil from '../../../Utils/LogUtil';
var _ohos_bluetooth_1  = globalThis.requireNapi('bluetooth') || (isSystemplugin('bluetooth', 'ohos') ? globalThis.ohosplugin.bluetooth : isSystemplugin('bluetooth', 'system') ? globalThis.systemplugin.bluetooth : undefined);
function testCreateGattServer() {
    //  let btState = bluetooth.getState();
    // if (btState == bluetooth.BluetoothState.STATE_ON){
    //  this.message = "createGattServer starts";
    //    if (!this.gattServerInstance) {
    //      this.gattServerInstance = BluetoothModel.createGattServer();
    //      this.message = "createGattServer: succeeds.";
    //    }
    //    else {
    //      message = "GattServer has existed!";
    //   }
    //  else if (btState == bluetooth.BluetoothState.STATE_OFF) {
    //    this.message = "BT is not enabled!";
    //  }
    //  else {
    //    this.message = "switching,please operate it later!";
    //  }
    let message = "success";
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testCreateGattServer = testCreateGattServer;
function testCreateGattClientDevice() {
    //  let btState = bluetooth.getState();
    // if (btState == bluetooth.BluetoothState.STATE_ON) {
    //
    //    if (!this.gattClientDeviceInstance) {
    //   let message = "createGattClientDevice starts"
    //      this.gattClientDeviceInstance = BluetoothModel.createGattClient(this.peripheralDeviceId);
    //      this.message = "createGattClientDevice:succeeds.";
    //    }
    //    else {
    //      message = "createGattClientDevice has existed!";
    //    }
    //    }
    //  }
    //  else if (btState == bluetooth.BluetoothState.STATE_OFF) {
    //    this.message = "BT is not enabled!";
    //  }
    //  else {
    //    this.message = "switching,please operate it later!";
    //  }
    let message = "success";
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testCreateGattClientDevice = testCreateGattClientDevice;
function testGetConnectedBLEDevices() {
    let connectedBleDevices = BluetoothModel_1.default.getConnectedBLEDevices();
    let message = "getConnectedBLEDevices��" + JSON.stringify(connectedBleDevices);
    //  console.log(JSON.stringify(connectedBleDevices));
    //  let message = "success"
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testGetConnectedBLEDevices = testGetConnectedBLEDevices;
function testStartBLEScan() {
    let btState = _ohos_bluetooth_1.getState();
    //  if (btState == bluetooth.BluetoothState.STATE_OFF) {
    let message = "BT is not enabled!";
    // build input parameters:
    //    this.buildBleScanFilters();
    //    this.availBleDeviceIds = [];
    //    AppStorage.SetOrCreate('bluetoothAvailableDevices', this.availBleDeviceIds);
    //    if (!this.cbxBleScanOptions) {
    //      if (this.cbxBleScanFilter && this.curScanFilters.length > 0) {
    //        BluetoothModel.startBLEScan(this.curScanFilters);
    //        this.message = "startBLEScan Filters:"+JSON.stringify(this.curScanFilters);
    //      }
    //      else
    //      {
    //        BluetoothModel.startBLEScan(null);
    //        this.message = "startBLEScan(null)";
    //      }
    //    }
    //    else
    //    {
    //      this.buildBleScanOption();
    //      this.message = "startBLEScan Filters:"+JSON.stringify(this.curScanFilters) + "\n Options:"+JSON.stringify(this.curScanOptions);
    //      BluetoothModel.startBLEScan(this.curScanFilters, this.curScanOptions);
    //
    //    }
    //    this.message = "startBLEScan succeeds."
    //  })
    //  let message = "success"
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testStartBLEScan = testStartBLEScan;
function testStopBLEScan() {
    BluetoothModel_1.default.stopBLEScan();
    let message = "stopBLEScan OK!";
    //  let message = "success"
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testStopBLEScan = testStopBLEScan;
let bleDeviceFind = false;
function testOnBLEDeviceFind() {
    let message = "testOnBLEDeviceFind";
    if (bleDeviceFind) {
        _ohos_bluetooth_1.off('bleDeviceFind', () => {
            _system_prompt_1.showToast({ message: "BLEDeviceFind is off" });
        });
        message = "offBLEDeviceFind";
    }
    else {
        _ohos_bluetooth_1.on('bleDeviceFind', (data) => {
            _system_prompt_1.showToast({ message: data.state });
        });
        message = "onBLEDeviceFind";
    }
    bleDeviceFind = !bleDeviceFind;
    return message;
    //  let message = "success"
    //  prompt.showToast({message: message})
    //  return message
}
exports.testOnBLEDeviceFind = testOnBLEDeviceFind;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/BRInterface.ets":
/*!**************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/BRInterface.ets ***!
  \**************************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.testOnBondStateChange = exports.testOnPinRequired = exports.testOnStateChange = exports.testGetProfileConnState2 = exports.testGetProfileConnState1 = exports.testCancelPairedDevice = exports.testGetPairedDevices = exports.testGetRemoteDeviceClass = exports.testGetRemoteDeviceName = exports.testPairDevice = exports.testStopBluetoothDiscovery = exports.testStartBluetoothDiscovery = exports.testGetBluetoothScanMode = exports.testSetBluetoothScanMode = exports.testGetLocalName = exports.testSetLocalName = exports.testGetBtConnectionState = exports.testGetState = exports.testDisableBluetooth = exports.testEnableBluetooth = exports.testInputBluetooth = void 0;
// @ts-nocheck
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
__webpack_require__(/*! ./BluetoothModel */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/BluetoothModel.ts");
var _system_prompt_1  = isSystemplugin('prompt', 'system') ? globalThis.systemplugin.prompt : globalThis.requireNapi('prompt');
const BluetoothModel_1 = __importDefault(__webpack_require__(/*! ./BluetoothModel */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/BluetoothModel.ts"));
var _system_router_1  = globalThis.requireNativeModule('system.router');
const BluetoothDeviceController_1 = __importDefault(__webpack_require__(/*! ../controller/BluetoothDeviceController */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/controller/BluetoothDeviceController.ts"));
var _ohos_bluetooth_1  = globalThis.requireNapi('bluetooth') || (isSystemplugin('bluetooth', 'ohos') ? globalThis.ohosplugin.bluetooth : isSystemplugin('bluetooth', 'system') ? globalThis.systemplugin.bluetooth : undefined);
const PAGE_URI_SET_SCAN_MODE = 'pages/subManualApiTest/subBrTest/setScanMode';
const PAGE_URI_DEVICE_FOUND_MODE = 'pages/subManualApiTest/subBrTest/deviceFound';
const PAGE_URI_PROFILE_CONN_STATE = 'pages/subManualApiTest/subBrTest/profileConnState';
const PAGE_URI_PROFILE_CONN_STATE_BY_ID = 'pages/subManualApiTest/subBrTest/searchProfileConnStateById';
function testInputBluetooth() {
    //  let ret = btApi.enableBluetooth();
    //  let btEnable = ret;
    //  AppStorage.SetOrCreate('bluetoothIsOn', btEnable);
    //  let message = "蓝牙使能执行结果：" + ret;
    let message = "testInputBluetooth";
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testInputBluetooth = testInputBluetooth;
function testEnableBluetooth() {
    //  this.currentClick = 0;
    //  if (this.btEnable) {
    //    this.message = '蓝牙已经使能';
    //   return;
    //  }
    let ret = BluetoothModel_1.default.enableBluetooth();
    AppStorage.SetOrCreate('bluetoothIsOn', this.btEnable);
    let message = "蓝牙使能执行结果：" + ret;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testEnableBluetooth = testEnableBluetooth;
function testDisableBluetooth() {
    let ret = BluetoothModel_1.default.disableBluetooth();
    this.btEnable = false;
    AppStorage.SetOrCreate('bluetoothIsOn', this.btEnable);
    let message = "蓝牙去使能执行结果：" + ret;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testDisableBluetooth = testDisableBluetooth;
function testGetState() {
    let ret = BluetoothModel_1.default.getState();
    let stateBT = "";
    switch (ret) {
        case 0:
            stateBT = 'STATE_OFF';
            break;
        case 1:
            stateBT = 'STATE_TURNING_ON';
            break;
        case 2:
            stateBT = 'STATE_ON';
            break;
        case 3:
            stateBT = 'STATE_TURNING_OFF';
            break;
        case 4:
            stateBT = 'STATE_BLE_TURNING_ON';
            break;
        case 5:
            stateBT = 'STATE_BLE_ON';
            break;
        case 6:
            stateBT = 'STATE_BLE_TURNING_OFF';
            break;
        default:
            stateBT = '未知状态';
            break;
    }
    let message = "当前蓝牙的状态是：" + stateBT;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testGetState = testGetState;
function testGetBtConnectionState() {
    let ret = BluetoothModel_1.default.getBtConnectionState();
    let message = "当前连接状态是：" + ret;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testGetBtConnectionState = testGetBtConnectionState;
function testSetLocalName() {
    //  Router.push({ uri: PAGE_URI_DEVICE_NAME });
    let deviceName = "SKH";
    let ret = BluetoothModel_1.default.setLocalName(deviceName);
    //  this.message = "设置SCAN MODE " + this.setScanModeRet ? '成功' : '失败';
    let message = "设置设备名称为：" + ret;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testSetLocalName = testSetLocalName;
function testGetLocalName() {
    let ret = BluetoothModel_1.default.getLocalName();
    let message = "本设备名称是：" + ret;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testGetLocalName = testGetLocalName;
function testSetBluetoothScanMode() {
    _system_router_1.push({ uri: PAGE_URI_SET_SCAN_MODE });
    //  this.message = "setBluetoothScanMode执行" + this.setLocalNameRet ? '成功' : '失败';
    //  let ret = BluetoothModel.setBluetoothScanMode(this.currentClick,!parseRet ? dur : 0);
    //  AppStorage.SetOrCreate('setScanModeRet', ret);
    let message = "跳转成功";
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testSetBluetoothScanMode = testSetBluetoothScanMode;
function testGetBluetoothScanMode() {
    let ret = BluetoothModel_1.default.getBluetoothScanMode();
    let message = "蓝牙当前的扫描模式是: " + ret;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testGetBluetoothScanMode = testGetBluetoothScanMode;
function testStartBluetoothDiscovery() {
    //  LogUtil.info(this.TAG_PAGE + 'startBluetoothDiscovery 111');
    _system_router_1.push({ uri: PAGE_URI_DEVICE_FOUND_MODE });
    //  LogUtil.log(this.PAGE_TAG + 'onPageShow in : localName = ' + this.localName )
    //  this.deviceController.getLocalName();
    //  AppStorage.SetOrCreate('btDeviceController', this.deviceController);
    //  LogUtil.log(this.PAGE_TAG + 'onPageShow out : localName = ' + this.localName )
    //
    let message = "跳转成功";
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testStartBluetoothDiscovery = testStartBluetoothDiscovery;
function testStopBluetoothDiscovery() {
    let ret = BluetoothModel_1.default.stopBluetoothDiscovery();
    let message = "stopBluetoothDiscovery 执行: " + ret;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testStopBluetoothDiscovery = testStopBluetoothDiscovery;
function testPairDevice() {
    let controller = new BluetoothDeviceController_1.default;
    let deviceId = "12312fsdf";
    controller.pair(deviceId, (pinCode) => {
        _system_prompt_1.showToast({ message: pinCode });
    }, () => {
        _system_prompt_1.showToast({ message: "testPairDevice Failed" });
    });
    //(deviceId, (pinCode: string) => {
    //   LogUtil.info(this.TAG_PAGE + 'pairDevice success callback : pinCode = ' + pinCode);
    // show pair pin dialog
    //   this.pairPinCode = pinCode
    //   this.pairingDevice.deviceId = deviceId;
    //   this.pairingDevice.deviceName = deviceId;
    //   this.pairDialog.open();
    // }, () => {
    //   LogUtil.info(this.TAG_PAGE + 'pairDevice error callback');
    //   this.showPairFailedDialog()
    // })
    //  }
    //  let pairDevice(controller.deviceId);
    //  let message = deviceId;
    //  prompt.showToast({message: message})
    return message;
}
exports.testPairDevice = testPairDevice;
function testGetRemoteDeviceName() {
    let deviceId = "12312fsdf";
    let ret = BluetoothModel_1.default.getDeviceName(deviceId);
    let message = '配对设备为' + ret;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testGetRemoteDeviceName = testGetRemoteDeviceName;
function testGetRemoteDeviceClass() {
    let deviceId = "12312fsdf";
    let ret = BluetoothModel_1.default.getDeviceType(deviceId);
    let message = '依据majorClass解析的设备类型是' + ret;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testGetRemoteDeviceClass = testGetRemoteDeviceClass;
function testGetPairedDevices() {
    let ret = BluetoothModel_1.default.getPairedDeviceIds();
    //  if (ret.length == 0) {
    //    this.message = '当前没有任何配对设备';
    //    return;
    //  }
    // AppStorage.SetOrCreate('pairedDevices', ret);
    //  Router.push({ uri: PAGE_URI_PAIRED_DEVICES })
    let message = "获取配对设备为：";
    if (ret == null || len(ret) == 0) {
        message += "空";
    }
    else {
        message += ret;
    }
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testGetPairedDevices = testGetPairedDevices;
function testCancelPairedDevice() {
    let deviceId = "123123";
    let ret = BluetoothModel_1.default.unpairDevice(deviceId);
    let message = '设备 "' + this.deviceId + '" 断开：' + ret;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testCancelPairedDevice = testCancelPairedDevice;
function testGetProfileConnState1() {
    _system_router_1.push({ uri: PAGE_URI_PROFILE_CONN_STATE });
    //  this.message = '';
    //  let ret = BluetoothModel.getProfileConnState(this.profileId);
    //  AppStorage.SetOrCreate('getProfileConnState', ret);
    //  this.message = this.profileType + ' 的连接状态是 ' + ret;
    let message = "跳转成功";
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testGetProfileConnState1 = testGetProfileConnState1;
function testGetProfileConnState2() {
    _system_router_1.push({ uri: PAGE_URI_PROFILE_CONN_STATE_BY_ID });
    //  this.message = '';
    let message = "跳转成功";
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testGetProfileConnState2 = testGetProfileConnState2;
let stateChange = false;
function testOnStateChange() {
    let message = "testOnStateChange";
    if (stateChange) {
        _ohos_bluetooth_1.off('stateChange', () => {
            _system_prompt_1.showToast({ message: "on:stateChange监听已关闭" });
        });
        message = "offStateChange";
    }
    else {
        _ohos_bluetooth_1.on('stateChange', (data) => {
            let btStateMessage = "";
            switch (data) {
                case 0:
                    btStateMessage += 'STATE_OFF';
                    break;
                case 1:
                    btStateMessage += 'STATE_TURNING_ON';
                    break;
                case 2:
                    btStateMessage += 'STATE_ON';
                    break;
                case 3:
                    btStateMessage += 'STATE_TURNING_OFF';
                    break;
                case 4:
                    btStateMessage += 'STATE_BLE_TURNING_ON';
                    break;
                case 5:
                    btStateMessage += 'STATE_BLE_ON';
                    break;
                case 6:
                    btStateMessage += 'STATE_BLE_TURNING_OFF';
                    break;
                default:
                    btStateMessage += '未知状态';
                    break;
                    _system_prompt_1.showToast({ message: data.state });
            }
        });
        message = "onStateChange";
    }
    stateChange = !stateChange;
    return message;
}
exports.testOnStateChange = testOnStateChange;
let pinRequired = false;
function testOnPinRequired() {
    let message = "testOnPinRequired";
    if (pinRequired) {
        _ohos_bluetooth_1.off('pinRequired', () => {
            _system_prompt_1.showToast({ message: "on:pinRequired监听已关闭" });
        });
        message = "offPingRequired";
    }
    else {
        _ohos_bluetooth_1.on('pinRequired', (data) => {
            _system_prompt_1.showToast({ message: data.state });
        });
        message = "onPingRequired";
    }
    pinRequired = !pinRequired;
    return message;
}
exports.testOnPinRequired = testOnPinRequired;
let bondStateFlag = false;
function testOnBondStateChange() {
    let message = "testOnBondStateChange";
    if (bondStateFlag) {
        _ohos_bluetooth_1.off('bondStateChange', () => {
            _system_prompt_1.showToast({ message: "on:bondStateChange监听已关闭" });
        });
        message = "offBondStateChange";
    }
    else {
        _ohos_bluetooth_1.on('bondStateChange', (data) => {
            _system_prompt_1.showToast({ message: data.state });
        });
        message = "onBondStateChange";
    }
    bondStateFlag = !bondStateFlag;
    return message;
}
exports.testOnBondStateChange = testOnBondStateChange;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/BluetoothDevice.ts":
/*!*****************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/BluetoothDevice.ts ***!
  \*****************************************************************************************************************************/
/***/ ((__unused_webpack_module, exports, __webpack_require__) => {

"use strict";

Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.Profile = void 0;
const BluetoothModel_1 = __webpack_require__(/*! ./BluetoothModel */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/BluetoothModel.ts");
class Profile {
    constructor() {
        this.profileId = -1;
        this.profileConnectionState = -1;
    }
}
exports.Profile = Profile;
/**
 * Bluetooth device class
 */
class BluetoothDevice {
    constructor() {
        this.deviceId = '';
        this.deviceName = '';
        this.deviceType = '';
        this.connectionState = 0;
        this.profiles = new Map();
    }
    setProfiles(data) {
        data.forEach((item) => {
            this.setProfile({
                profileId: item.profileId,
                deviceId: this.deviceId,
                profileConnectionState: item.profileConnectionState
            });
        });
    }
    setProfile(data) {
        if (this.deviceId !== data.deviceId) {
            return;
        }
        this.profiles.set(data.profileId, data);
        let countStateDisconnect = 0;
        let countStateConnecting = 0;
        let countStateConnected = 0;
        let countStateDisconnecting = 0;
        this.profiles.forEach((profile, key) => {
            if (profile.profileConnectionState == 0) {
                // 0:the current profile is disconnected
                countStateDisconnect++;
            }
            else if (profile.profileConnectionState == 1) {
                // 1:the current profile is being connected
                countStateConnecting++;
            }
            else if (profile.profileConnectionState == 2) {
                // 2:the current profile is connected
                countStateConnected++;
            }
            else if (profile.profileConnectionState == 3) {
                // 3:the current profile is being disconnected
                countStateDisconnecting++;
            }
        });
        if (countStateConnected > 0 || countStateDisconnecting > 0) {
            this.connectionState = BluetoothModel_1.DeviceState.STATE_CONNECTED;
        }
        else if (countStateConnecting > 0) {
            this.connectionState = BluetoothModel_1.DeviceState.STATE_CONNECTING;
        }
        else {
            this.connectionState = BluetoothModel_1.DeviceState.STATE_DISCONNECTED;
        }
    }
}
exports["default"] = BluetoothDevice;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/BluetoothModel.ts":
/*!****************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/BluetoothModel.ts ***!
  \****************************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.BluetoothModel = exports.BluetoothErrorCode = exports.DeviceState = exports.DeviceType = exports.BondState = exports.ProfileCode = void 0;
const LogUtil_1 = __importDefault(__webpack_require__(/*! ../../Utils/LogUtil */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Utils/LogUtil.ts"));
const BaseModel_1 = __importDefault(__webpack_require__(/*! ../../Utils/BaseModel */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Utils/BaseModel.ts"));
var _ohos_bluetooth_1  = globalThis.requireNapi('bluetooth') || (isSystemplugin('bluetooth', 'ohos') ? globalThis.ohosplugin.bluetooth : isSystemplugin('bluetooth', 'system') ? globalThis.systemplugin.bluetooth : undefined);
var ProfileCode;
(function (ProfileCode) {
    ProfileCode[ProfileCode["CODE_BT_PROFILE_A2DP_SINK"] = 0] = "CODE_BT_PROFILE_A2DP_SINK";
    ProfileCode[ProfileCode["CODE_BT_PROFILE_A2DP_SOURCE"] = 1] = "CODE_BT_PROFILE_A2DP_SOURCE";
    ProfileCode[ProfileCode["CODE_BT_PROFILE_AVRCP_CT"] = 2] = "CODE_BT_PROFILE_AVRCP_CT";
    ProfileCode[ProfileCode["CODE_BT_PROFILE_AVRCP_TG"] = 3] = "CODE_BT_PROFILE_AVRCP_TG";
    ProfileCode[ProfileCode["CODE_BT_PROFILE_HANDS_FREE_AUDIO_GATEWAY"] = 4] = "CODE_BT_PROFILE_HANDS_FREE_AUDIO_GATEWAY";
    ProfileCode[ProfileCode["CODE_BT_PROFILE_HANDS_FREE_UNIT"] = 5] = "CODE_BT_PROFILE_HANDS_FREE_UNIT";
    ProfileCode[ProfileCode["CODE_BT_PROFILE_HID_HOST"] = 6] = "CODE_BT_PROFILE_HID_HOST";
    ProfileCode[ProfileCode["CODE_BT_PROFILE_PAN_NETWORK"] = 7] = "CODE_BT_PROFILE_PAN_NETWORK";
    ProfileCode[ProfileCode["CODE_BT_PROFILE_PBAP_CLIENT"] = 8] = "CODE_BT_PROFILE_PBAP_CLIENT";
    ProfileCode[ProfileCode["CODE_BT_PROFILE_PBAP_SERVER"] = 9] = "CODE_BT_PROFILE_PBAP_SERVER";
})(ProfileCode = exports.ProfileCode || (exports.ProfileCode = {}));
;
var BondState;
(function (BondState) {
    /** Indicate the bond state is invalid */
    BondState[BondState["BOND_STATE_INVALID"] = 0] = "BOND_STATE_INVALID";
    /** Indicate the bond state is bonding */
    BondState[BondState["BOND_STATE_BONDING"] = 1] = "BOND_STATE_BONDING";
    /** Indicate the bond state is bonded*/
    BondState[BondState["BOND_STATE_BONDED"] = 2] = "BOND_STATE_BONDED";
})(BondState = exports.BondState || (exports.BondState = {}));
var DeviceType;
(function (DeviceType) {
    DeviceType["BLUETOOTH"] = "1";
    DeviceType["HEADPHONE"] = "2";
    DeviceType["PHONE"] = "3";
    DeviceType["COMPUTER"] = "4";
    DeviceType["WATCH"] = "5";
})(DeviceType = exports.DeviceType || (exports.DeviceType = {}));
var DeviceState;
(function (DeviceState) {
    /** the device is disconnected */
    DeviceState[DeviceState["STATE_DISCONNECTED"] = 0] = "STATE_DISCONNECTED";
    /** the device is being connected */
    DeviceState[DeviceState["STATE_CONNECTING"] = 1] = "STATE_CONNECTING";
    /** the device is connected */
    DeviceState[DeviceState["STATE_CONNECTED"] = 2] = "STATE_CONNECTED";
    /** the device is being disconnected */
    DeviceState[DeviceState["STATE_DISCONNECTING"] = 3] = "STATE_DISCONNECTING";
    /** the device is available */
    DeviceState[DeviceState["STATE_AVAILABLE"] = 100] = "STATE_AVAILABLE";
    /** the device is pairing */
    DeviceState[DeviceState["STATE_PAIRING"] = 101] = "STATE_PAIRING";
    /** the device is paired */
    DeviceState[DeviceState["STATE_PAIRED"] = 102] = "STATE_PAIRED";
})(DeviceState = exports.DeviceState || (exports.DeviceState = {}));
var BluetoothErrorCode;
(function (BluetoothErrorCode) {
    BluetoothErrorCode[BluetoothErrorCode["SUCCESS"] = -1] = "SUCCESS";
    BluetoothErrorCode[BluetoothErrorCode["HOLD_PAIRING_MODE"] = 1] = "HOLD_PAIRING_MODE";
    BluetoothErrorCode[BluetoothErrorCode["APP_PAIR"] = 2] = "APP_PAIR";
    BluetoothErrorCode[BluetoothErrorCode["PAIR_FAILED"] = 3] = "PAIR_FAILED";
    BluetoothErrorCode[BluetoothErrorCode["DEVICE_ILLEGAL"] = 4] = "DEVICE_ILLEGAL";
    BluetoothErrorCode[BluetoothErrorCode["CONNECT_FAILED"] = 5] = "CONNECT_FAILED";
})(BluetoothErrorCode = exports.BluetoothErrorCode || (exports.BluetoothErrorCode = {}));
var BluetoothState;
(function (BluetoothState) {
    /** Indicates the local Bluetooth is off */
    BluetoothState[BluetoothState["STATE_OFF"] = 0] = "STATE_OFF";
    /** Indicates the local Bluetooth is turning on */
    BluetoothState[BluetoothState["STATE_TURNING_ON"] = 1] = "STATE_TURNING_ON";
    /** Indicates the local Bluetooth is on, and ready for use */
    BluetoothState[BluetoothState["STATE_ON"] = 2] = "STATE_ON";
    /** Indicates the local Bluetooth is turning off */
    BluetoothState[BluetoothState["STATE_TURNING_OFF"] = 3] = "STATE_TURNING_OFF";
    /** Indicates the local Bluetooth is turning LE mode on */
    BluetoothState[BluetoothState["STATE_BLE_TURNING_ON"] = 4] = "STATE_BLE_TURNING_ON";
    /** Indicates the local Bluetooth is in LE only mode */
    BluetoothState[BluetoothState["STATE_BLE_ON"] = 5] = "STATE_BLE_ON";
    /** Indicates the local Bluetooth is turning off LE only mode */
    BluetoothState[BluetoothState["STATE_BLE_TURNING_OFF"] = 6] = "STATE_BLE_TURNING_OFF";
})(BluetoothState || (BluetoothState = {}));
var ScanMode;
(function (ScanMode) {
    /** Indicates the scan mode is none */
    ScanMode[ScanMode["SCAN_MODE_NONE"] = 0] = "SCAN_MODE_NONE";
    /** Indicates the scan mode is connectable */
    ScanMode[ScanMode["SCAN_MODE_CONNECTABLE"] = 1] = "SCAN_MODE_CONNECTABLE";
    /** Indicates the scan mode is general discoverable */
    ScanMode[ScanMode["SCAN_MODE_GENERAL_DISCOVERABLE"] = 2] = "SCAN_MODE_GENERAL_DISCOVERABLE";
    /** Indicates the scan mode is limited discoverable */
    ScanMode[ScanMode["SCAN_MODE_LIMITED_DISCOVERABLE"] = 3] = "SCAN_MODE_LIMITED_DISCOVERABLE";
    /** Indicates the scan mode is connectable and general discoverable */
    ScanMode[ScanMode["SCAN_MODE_CONNECTABLE_GENERAL_DISCOVERABLE"] = 4] = "SCAN_MODE_CONNECTABLE_GENERAL_DISCOVERABLE";
    /** Indicates the scan mode is connectable and limited discoverable */
    ScanMode[ScanMode["SCAN_MODE_CONNECTABLE_LIMITED_DISCOVERABLE"] = 5] = "SCAN_MODE_CONNECTABLE_LIMITED_DISCOVERABLE";
})(ScanMode || (ScanMode = {}));
var ProfileId;
(function (ProfileId) {
    ProfileId[ProfileId["PROFILE_A2DP_SOURCE"] = 1] = "PROFILE_A2DP_SOURCE";
    ProfileId[ProfileId["PROFILE_HANDS_FREE_AUDIO_GATEWAY"] = 4] = "PROFILE_HANDS_FREE_AUDIO_GATEWAY";
})(ProfileId || (ProfileId = {}));
var ProfileConnectionState;
(function (ProfileConnectionState) {
    /** the current profile is disconnected */
    ProfileConnectionState[ProfileConnectionState["STATE_DISCONNECTED"] = 0] = "STATE_DISCONNECTED";
    /** the current profile is being connected */
    ProfileConnectionState[ProfileConnectionState["STATE_CONNECTING"] = 1] = "STATE_CONNECTING";
    /** the current profile is connected */
    ProfileConnectionState[ProfileConnectionState["STATE_CONNECTED"] = 2] = "STATE_CONNECTED";
    /** the current profile is being disconnected */
    ProfileConnectionState[ProfileConnectionState["STATE_DISCONNECTING"] = 3] = "STATE_DISCONNECTING";
})(ProfileConnectionState || (ProfileConnectionState = {}));
class BluetoothModel extends BaseModel_1.default {
    /**
     * constructor
     */
    constructor() {
        super();
        this.TAG = 'BluetoothModel ';
        this.profiles = new Array(10);
        this.canUse = false;
        try {
            LogUtil_1.default.info('bluetooth.getProfile start');
            this.profiles[1] = _ohos_bluetooth_1.getProfile(1);
            this.profiles[4] = _ohos_bluetooth_1.getProfile(4);
            LogUtil_1.default.info('bluetooth.getProfile end');
            this.canUse = true;
        }
        catch (error) {
            LogUtil_1.default.info('bluetooth.getProfile error');
            this.canUse = false;
            LogUtil_1.default.info(`BluetoothModel error: ${JSON.stringify(error)}.`);
        }
    }
    getProfileConnState(profileId) {
        let state = _ohos_bluetooth_1.getProfileConnState(profileId);
        switch (state) {
            case 0:
                return 'STATE_DISCONNECTED';
                break;
            case 1:
                return 'STATE_CONNECTING';
                break;
            case 2:
                return 'STATE_CONNECTED';
                break;
            case 3:
                return 'STATE_DISCONNECTING';
                break;
            default:
                return '未知状态';
                break;
        }
    }
    getBtConnectionState() {
        let connectionState = _ohos_bluetooth_1.getBtConnectionState();
        switch (connectionState) {
            case 0:
                return 'STATE_DISCONNECTED';
                break;
            case 1:
                return 'STATE_CONNECTING';
                break;
            case 2:
                return 'STATE_CONNECTED';
                break;
            case 3:
                return 'STATE_DISCONNECTING';
                break;
            default:
                return '未知状态';
                break;
        }
    }
    /**
     * Get Bluetooth status
     * @return value of bluetooth.BluetoothState type
     */
    getState() {
        let bluetoothState = _ohos_bluetooth_1.getState();
        LogUtil_1.default.info(`${this.TAG} getState: bluetoothState = ${bluetoothState}`);
        return bluetoothState;
    }
    setBluetoothScanMode(mode, duration) {
        return _ohos_bluetooth_1.setBluetoothScanMode(mode, duration);
    }
    getBluetoothScanMode() {
        let scanMode = _ohos_bluetooth_1.getBluetoothScanMode();
        LogUtil_1.default.info(`${this.TAG} getBluetoothScanMode: scanMode = ${scanMode}`);
        switch (scanMode) {
            case 0:
                return 'SCAN_MODE_NONE';
                break;
            case 1:
                return 'SCAN_MODE_CONNECTABLE';
                break;
            case 2:
                return 'SCAN_MODE_GENERAL_DISCOVERABLE';
                break;
            case 3:
                return 'SCAN_MODE_LIMITED_DISCOVERABLE';
                break;
            case 4:
                return 'SCAN_MODE_CONNECTABLE_GENERAL_DISCOVERABLE';
                break;
            case 5:
                return 'SCAN_MODE_CONNECTABLE_LIMITED_DISCOVERABLE';
                break;
            default:
                return '';
                break;
        }
    }
    /**
     * Get Bluetooth switch status
     */
    isStateOn() {
        let result = false;
        let state = _ohos_bluetooth_1.getState();
        LogUtil_1.default.info(`${this.TAG} isStateOn: state = ${state}`);
        switch (state) {
            case BluetoothState.STATE_ON:
                result = true;
                break;
            default:
                break;
        }
        LogUtil_1.default.info(`${this.TAG} isStateOn: bluetoothState = ${result}`);
        return result;
    }
    /**
     * Subscribe Bluetooth switch status Change
     */
    subscribeStateChange(callback) {
        _ohos_bluetooth_1.on('stateChange', (data) => {
            LogUtil_1.default.info(`${this.TAG} subscribeStateChange->stateChange data:${data}`);
            if (callback) {
                switch (data) {
                    case BluetoothState.STATE_ON:
                        _ohos_bluetooth_1.setBluetoothScanMode(4, 0);
                        LogUtil_1.default.info(`${this.TAG} subscribeStateChange->stateChange return: true`);
                        callback(true);
                        break;
                    case BluetoothState.STATE_OFF:
                        LogUtil_1.default.info(`${this.TAG} subscribeStateChange->stateChange return: false`);
                        callback(false);
                        break;
                    default:
                        break;
                }
            }
        });
    }
    /**
     * unsubscribe Bluetooth switch status Change
     */
    unsubscribeStateChange(callback) {
        _ohos_bluetooth_1.off('stateChange', (data) => {
            LogUtil_1.default.info(`${this.TAG} unsubscribeStateChange->stateChange data:${data}`);
            if (callback) {
                let result = false;
                switch (data) {
                    case BluetoothState.STATE_ON:
                        LogUtil_1.default.info(`${this.TAG} unsubscribeStateChange->stateChange return : true`);
                        callback(true);
                        break;
                    case BluetoothState.STATE_OFF:
                        LogUtil_1.default.info(`${this.TAG} unsubscribeStateChange->stateChange return : false`);
                        callback(false);
                        break;
                    default:
                        break;
                }
            }
        });
    }
    /**
     * Turn on Bluetooth
     */
    enableBluetooth() {
        return _ohos_bluetooth_1.enableBluetooth();
    }
    /**
     * Turn off Bluetooth
     */
    disableBluetooth() {
        return _ohos_bluetooth_1.disableBluetooth();
    }
    /**
     * Get local name
     */
    getLocalName() {
        return _ohos_bluetooth_1.getLocalName();
    }
    /**
     * Set local name
     */
    setLocalName(name) {
        return _ohos_bluetooth_1.setLocalName(name);
    }
    /**
     * Get paired device ids
     */
    getPairedDeviceIds() {
        return _ohos_bluetooth_1.getPairedDevices();
    }
    /**
     * Start Bluetooth discovery
     */
    startBluetoothDiscovery() {
        return _ohos_bluetooth_1.startBluetoothDiscovery();
    }
    /**
     * Stop Bluetooth discovery
     */
    stopBluetoothDiscovery() {
        let ret = _ohos_bluetooth_1.stopBluetoothDiscovery();
        this.unsubscribeStateChange();
        this.unsubscribeBluetoothDeviceFind();
        this.unsubscribeBondStateChange();
        this.unsubscribeDeviceStateChange();
        return ret;
    }
    /**
     * Subscribe Bluetooth status Change
     */
    subscribeBluetoothDeviceFind(callback) {
        _ohos_bluetooth_1.on('bluetoothDeviceFind', (data) => {
            LogUtil_1.default.info(`${this.TAG} subscribeBluetoothDeviceFind->deviceFind return:${JSON.stringify(data)}`);
            if (callback) {
                callback(data);
            }
        });
    }
    /**
     * unsubscribe Bluetooth status Change
     */
    unsubscribeBluetoothDeviceFind(callback) {
        _ohos_bluetooth_1.off('bluetoothDeviceFind', (data) => {
            LogUtil_1.default.info(`${this.TAG} unsubscribeBluetoothDeviceFind->deviceFind return:${JSON.stringify(data)}`);
            if (callback) {
                callback(data);
            }
        });
    }
    /**
     * Pair device
     */
    pairDevice(deviceId) {
        return _ohos_bluetooth_1.pairDevice(deviceId);
    }
    /**
     * Subscribe PinRequired
     */
    subscribePinRequired(callback) {
        _ohos_bluetooth_1.on('pinRequired', (data) => {
            LogUtil_1.default.info(`${this.TAG} subscribePinRequired->pinRequired return:${JSON.stringify(data)}`);
            if (callback) {
                callback(data);
            }
        });
    }
    /**
     * Unsubscribe PinRequired
     */
    unsubscribePinRequired(callback) {
        _ohos_bluetooth_1.off('pinRequired', (data) => {
            LogUtil_1.default.info(`${this.TAG} unsubscribePinRequired->pinRequired return: ${JSON.stringify(data)}`);
            if (callback) {
                callback(data);
            }
        });
    }
    /**
     * Set device PairingConfirmation
     */
    setDevicePairingConfirmation(deviceId, accept) {
        return _ohos_bluetooth_1.setDevicePairingConfirmation(deviceId, accept);
    }
    /**
     * Subscribe bondStateChange
     */
    subscribeBondStateChange(callback) {
        _ohos_bluetooth_1.on('bondStateChange', (data) => {
            LogUtil_1.default.info(`${this.TAG} subscribeBondStateChange->bondStateChange data:${JSON.stringify(data)}`);
            if (callback) {
                let result = {
                    deviceId: data.deviceId,
                    bondState: data.state
                };
                LogUtil_1.default.info(`${this.TAG} subscribeBondStateChange->bondStateChange return:${JSON.stringify(result)}`);
                callback(result);
            }
        });
    }
    /**
     * Unsubscribe bondStateChange
     */
    unsubscribeBondStateChange(callback) {
        _ohos_bluetooth_1.off('bondStateChange', (data) => {
            LogUtil_1.default.info(`${this.TAG} unsubscribeBondStateChange->bondStateChange data:${JSON.stringify(data)}`);
            if (callback) {
                let result = {
                    deviceId: data.deviceId,
                    bondState: data.state
                };
                LogUtil_1.default.info(`${this.TAG} unsubscribeBondStateChange->bondStateChange return:${JSON.stringify(result)}`);
                callback(result);
            }
        });
    }
    /**
     * Get device name
     */
    getDeviceName(deviceId) {
        return _ohos_bluetooth_1.getRemoteDeviceName(deviceId);
    }
    /**
     * Get device type
     */
    getDeviceType(deviceId) {
        let deviceType = 'BLUETOOTH';
        let deviceClass = _ohos_bluetooth_1.getRemoteDeviceClass(deviceId);
        switch (deviceClass.majorClass) {
            case 0x0100:
                deviceType = 'COMPUTER';
                break;
            case 0x0400:
                if (deviceClass.majorMinorClass === 0x0418 || deviceClass.majorMinorClass === 0x0404) {
                    deviceType = 'HEADPHONE';
                }
                break;
            case 0x0700:
                if (deviceClass.majorMinorClass === 0x0704) {
                    deviceType = 'WATCH';
                }
                break;
            case 0x0200:
                deviceType = 'PHONE';
                break;
            default:
                deviceType = 'BLUETOOTH';
                break;
        }
        return deviceType;
    }
    /**
     * Get device state
     */
    getDeviceState(deviceId) {
        let result = [];
        for (let i = 0; i < this.profiles.length; i++) {
            if (this.profiles[i]) {
                let state = this.profiles[i].getDeviceState(deviceId);
                result.push({
                    profileId: i,
                    profileConnectionState: state
                });
            }
        }
        return result;
    }
    /**
     * Unpair device
     */
    unpairDevice(deviceId) {
        //        return bluetooth.cancelPairedDevice(deviceId);
        return true;
    }
    /**
     * Connect device
     */
    connectDevice(deviceId) {
        let result = [];
        for (let i = 0; i < this.profiles.length; i++) {
            if (this.profiles[i]) {
                let profile = this.profiles[i];
                let connectRet = profile.connect(deviceId);
                result.push({
                    profileId: i,
                    connectRet: connectRet
                });
            }
        }
        return result;
    }
    /**
     * Disconnect device
     */
    disconnectDevice(deviceId) {
        let result = [];
        for (let i = 0; i < this.profiles.length; i++) {
            let profile = this.profiles[i];
            if (this.profiles[i]) {
                let profileConnectionState = profile.getDeviceState(deviceId);
                let disconnectRet = true;
                LogUtil_1.default.info(`${this.TAG} disconnectDevice deviceId = ${deviceId}, connectionState = ${profileConnectionState}`);
                if (profileConnectionState === 2) {
                    disconnectRet = profile.disconnect(deviceId);
                    LogUtil_1.default.info(`${this.TAG} disconnectDevice call disconnect over. api return =  ${disconnectRet}, deviceId = ${deviceId}`);
                }
                result.push({
                    profileId: i,
                    disconnectRet: disconnectRet
                });
            }
        }
        return result;
    }
    /**
     * Subscribe device connection state Change
     */
    subscribeDeviceStateChange(callback) {
        for (let i = 0; i < this.profiles.length; i++) {
            if (this.profiles[i]) {
                let profile = this.profiles[i];
                profile.on('connectionStateChange', (data) => {
                    LogUtil_1.default.info(`${this.TAG} subscribeDeviceStateChange->connectionStateChange data:${JSON.stringify(data)}`);
                    if (callback) {
                        let result = {
                            profileId: i,
                            deviceId: data.deviceId,
                            profileConnectionState: data.state
                        };
                        LogUtil_1.default.info(`${this.TAG} subscribeDeviceStateChange->connectionStateChange return:${JSON.stringify(result)}`);
                        callback(result);
                    }
                });
            }
        }
    }
    /**
     * unsubscribe device connection state Change
     */
    unsubscribeDeviceStateChange(callback) {
        for (let i = 0; i < this.profiles.length; i++) {
            if (this.profiles[i]) {
                let profile = this.profiles[i];
                profile.off('connectionStateChange', (data) => {
                    LogUtil_1.default.info(`${this.TAG} unsubscribeDeviceStateChange->connectionStateChange data:${JSON.stringify(data)}`);
                    if (callback) {
                        let result = {
                            profileId: i,
                            deviceId: data.deviceId,
                            profileConnectionState: data.state
                        };
                        LogUtil_1.default.info(`${this.TAG} unsubscribeDeviceStateChange->connectionStateChange return:${JSON.stringify(result)}`);
                        callback(result);
                    }
                });
            }
        }
    }
    // BLE public
    createGattServer() {
        return _ohos_bluetooth_1.BLE.createGattServer();
    }
    createGattClient(deviceId) {
        return _ohos_bluetooth_1.BLE.createGattClientDevice(deviceId);
    }
    getConnectedBLEDevices() {
        return _ohos_bluetooth_1.BLE.getConnectedBLEDevices();
    }
    // start BLE scanning
    startBLEScan(filters, options) {
        switch (arguments.length) {
            case 1:
                LogUtil_1.default.info(`${this.TAG} startBLEScan with filters(or null) only `);
                _ohos_bluetooth_1.BLE.startBLEScan(filters);
                break;
            case 2:
                LogUtil_1.default.info(`${this.TAG} startBLEScan with filters and options`);
                _ohos_bluetooth_1.BLE.startBLEScan(filters, options);
                break;
            default:
                LogUtil_1.default.error(`${this.TAG} startBLEScan with unexpected input parameter!`);
        }
    }
    /**
     * Stops BLE scanning.
     */
    stopBLEScan() {
        _ohos_bluetooth_1.BLE.stopBLEScan();
    }
    /**
     * Subscribe BLEDeviceFind
    */
    subscribeBLEDeviceFind(callback) {
        _ohos_bluetooth_1.BLE.on("BLEDeviceFind", (bleDeviceFindData) => {
            LogUtil_1.default.info(`${this.TAG} subscribeBLEDeviceFind->deviceFind return:${JSON.stringify(bleDeviceFindData)}`);
            if (callback) {
                callback(bleDeviceFindData);
            }
        });
    }
    /**
     * unsubscribe BLEDeviceFind
     */
    unsubscribeBLEDeviceFind(callback) {
        _ohos_bluetooth_1.BLE.off('BLEDeviceFind', (bleDeviceFindData) => {
            LogUtil_1.default.info(`${this.TAG} unsubscribeBLEDeviceFind->deviceFind return:${JSON.stringify(bleDeviceFindData)}`);
            if (callback) {
                callback(bleDeviceFindData);
            }
        });
    }
}
exports.BluetoothModel = BluetoothModel;
let bluetoothModel = new BluetoothModel();
exports["default"] = bluetoothModel;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/GATTInterface.ets":
/*!****************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/GATTInterface.ets ***!
  \****************************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports) {

"use strict";

var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.testGATT_Server = exports.testGATT_Client = void 0;
// @ts-nocheck
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
var _system_router_1  = globalThis.requireNativeModule('system.router');
var _system_prompt_1  = isSystemplugin('prompt', 'system') ? globalThis.systemplugin.prompt : globalThis.requireNapi('prompt');
const PAGE_URI_GET_GATT_CLIENT = 'pages/subManualApiTest/subGattTest/gattClient';
const PAGE_URI_GET_GATT_SERVER = 'pages/subManualApiTest/subGattTest/gattServer';
function testGATT_Client() {
    //SubEntryComponent({targetPage: "pages/subManualApiTest/subGattTest/gattClient"
    _system_router_1.push({ uri: PAGE_URI_GET_GATT_CLIENT });
    let message = "跳转成功";
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testGATT_Client = testGATT_Client;
function testGATT_Server() {
    //SubEntryComponent({targetPage: "pages/subManualApiTest/subGattTest/gattServer"
    _system_router_1.push({ uri: PAGE_URI_GET_GATT_SERVER });
    let message = "跳转成功";
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testGATT_Server = testGATT_Server;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/ProfileInterface.ets":
/*!*******************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/ProfileInterface.ets ***!
  \*******************************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports) {

"use strict";

var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.testHID_HOST_Profile = exports.testHANDS_Profile = exports.testA2DP_Profile = void 0;
// @ts-nocheck
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
var _system_router_1  = globalThis.requireNativeModule('system.router');
var _system_prompt_1  = isSystemplugin('prompt', 'system') ? globalThis.systemplugin.prompt : globalThis.requireNapi('prompt');
const PAGE_URI_GET_A2DP_PROFILE = 'pages/subManualApiTest/subProfileTest/a2dpProfileTest';
const PAGE_URI_GET_HANDS_PROFILE = 'pages/subManualApiTest/subProfileTest/a2dpProfileTest';
const PAGE_URI_GET_HID_HOST_PROFILE = 'pages/subManualApiTest/subProfileTest/a2dpProfileTest';
function testA2DP_Profile() {
    //SubEntryComponent({targetPage: "pages/subManualApiTest/subProfileTest/a2dpProfileTest"
    _system_router_1.push({ uri: PAGE_URI_GET_A2DP_PROFILE });
    let message = "跳转成功";
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testA2DP_Profile = testA2DP_Profile;
function testHANDS_Profile() {
    //SubEntryComponent({targetPage: "pages/subManualApiTest/subProfileTest/a2dpProfileTest"
    _system_router_1.push({ uri: PAGE_URI_GET_HANDS_PROFILE });
    let message = "跳转成功";
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testHANDS_Profile = testHANDS_Profile;
function testHID_HOST_Profile() {
    //SubEntryComponent({targetPage: "pages/subManualApiTest/subProfileTest/a2dpProfileTest"
    _system_router_1.push({ uri: PAGE_URI_GET_HID_HOST_PROFILE });
    let message = "跳转成功";
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testHID_HOST_Profile = testHID_HOST_Profile;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/SPPInterface.ets":
/*!***************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/SPPInterface.ets ***!
  \***************************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports) {

"use strict";

var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.testSPP_Server = exports.testSPP_Client = void 0;
// @ts-nocheck
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
var _system_router_1  = globalThis.requireNativeModule('system.router');
var _system_prompt_1  = isSystemplugin('prompt', 'system') ? globalThis.systemplugin.prompt : globalThis.requireNapi('prompt');
const PAGE_URI_GET_SPP_CLIENT_TEST = 'pages/subManualApiTest/subSppTest/sppClientTest';
const PAGE_URI_GET_SPP_SERVER_TEST = 'pages/subManualApiTest/subSppTest/sppServerTest';
function testSPP_Client() {
    //SubEntryComponent({targetPage: "pages/subManualApiTest/subSppTest/sppClientTest"
    _system_router_1.push({ uri: PAGE_URI_GET_SPP_CLIENT_TEST });
    let message = "跳转成功";
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testSPP_Client = testSPP_Client;
function testSPP_Server() {
    //SubEntryComponent({targetPage: "pages/subManualApiTest/subSppTest/sppServerTest"
    _system_router_1.push({ uri: PAGE_URI_GET_SPP_SERVER_TEST });
    let message = "跳转成功";
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testSPP_Server = testSPP_Server;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/TestData.ets":
/*!***********************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/TestData.ets ***!
  \***********************************************************************************************************************/
/***/ ((__unused_webpack_module, exports) => {

"use strict";

Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.TestApi = exports.CaseCategory = exports.TestData = exports.Category = void 0;
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
var Category;
(function (Category) {
    Category[Category["All"] = 0] = "All";
    Category[Category["API"] = 1] = "API";
    Category[Category["Scenario"] = 2] = "Scenario";
    Category[Category["AutoTest"] = 3] = "AutoTest";
    Category[Category["Profile"] = 4] = "Profile";
    Category[Category["BenchMark"] = 5] = "BenchMark";
})(Category = exports.Category || (exports.Category = {}));
let NextId = 0;
class TestData {
    constructor(name, detail, url, image, category) {
        this.id = `${NextId++}`;
        this.name = name;
        this.url = url;
        this.detail = detail;
        this.image = image;
        this.category = category;
    }
}
exports.TestData = TestData;
var CaseCategory;
(function (CaseCategory) {
    CaseCategory[CaseCategory["BR"] = 0] = "BR";
    CaseCategory[CaseCategory["BLE"] = 1] = "BLE";
    CaseCategory[CaseCategory["Gatt"] = 2] = "Gatt";
    CaseCategory[CaseCategory["SPP"] = 3] = "SPP";
    CaseCategory[CaseCategory["Profile"] = 4] = "Profile";
})(CaseCategory = exports.CaseCategory || (exports.CaseCategory = {}));
class TestApi {
    constructor(name, detail, method, api, result, category) {
        this.id = `${NextId++}`;
        this.name = name;
        this.detail = detail;
        this.method = method;
        this.api = api;
        this.result = result;
        this.category = category;
    }
}
exports.TestApi = TestApi;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/TestDataModels.ets":
/*!*****************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/TestDataModels.ets ***!
  \*****************************************************************************************************************************/
/***/ ((__unused_webpack_module, exports, __webpack_require__) => {

"use strict";

Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.initProfileApiData = exports.initSppApiData = exports.initGattApiData = exports.initBleApiData = exports.initBRApiData = exports.initTestData = void 0;
const TestData_1 = __webpack_require__(/*! ./TestData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/TestData.ets");
const BRInterface_1 = __webpack_require__(/*! ./BRInterface */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/BRInterface.ets");
const BLEInterface_1 = __webpack_require__(/*! ./BLEInterface */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/BLEInterface.ets");
const GATTInterface_1 = __webpack_require__(/*! ./GATTInterface */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/GATTInterface.ets");
const SPPInterface_1 = __webpack_require__(/*! ./SPPInterface */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/SPPInterface.ets");
const ProfileInterface_1 = __webpack_require__(/*! ./ProfileInterface */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/ProfileInterface.ets");
__webpack_require__(/*! @ohos.bluetooth */ "../../api/@ohos.bluetooth.d.ts");
const BRTestCase = [
    { 'name': '开蓝牙', 'detail': '使能蓝牙', 'method': 'enableBluetooth', 'api': BRInterface_1.testEnableBluetooth, 'result': 'None', 'category': TestData_1.CaseCategory.BR },
    { 'name': '关蓝牙', 'detail': '使能蓝牙', 'method': 'disableBluetooth', 'api': BRInterface_1.testDisableBluetooth, 'result': 'None', 'category': TestData_1.CaseCategory.BR },
    { 'name': '设置状态', 'detail': '使能蓝牙', 'method': 'getState', 'api': BRInterface_1.testGetState, 'result': 'None', 'category': TestData_1.CaseCategory.BR },
    { 'name': '获取当前连接状态', 'detail': '使能蓝牙', 'method': 'getBtConnectionState', 'api': BRInterface_1.testGetBtConnectionState, 'result': 'None', 'category': TestData_1.CaseCategory.BR },
    { 'name': '设置本地名称', 'detail': '使能蓝牙', 'method': 'setLocalName', 'api': BRInterface_1.testSetLocalName, 'result': 'None', 'category': TestData_1.CaseCategory.BR },
    { 'name': '获取本地名称', 'detail': '使能蓝牙', 'method': 'getLocalName', 'api': BRInterface_1.testGetLocalName, 'result': 'None', 'category': TestData_1.CaseCategory.BR },
    { 'name': '设置扫描模式', 'detail': '使能蓝牙', 'method': 'setBluetoothScanMode', 'api': BRInterface_1.testSetBluetoothScanMode, 'result': 'None', 'category': TestData_1.CaseCategory.BR },
    { 'name': '获取扫描模式', 'detail': '使能蓝牙', 'method': 'getBluetoothScanMode', 'api': BRInterface_1.testGetBluetoothScanMode, 'result': 'None', 'category': TestData_1.CaseCategory.BR },
    { 'name': '开始蓝牙发现', 'detail': '使能蓝牙', 'method': 'startBluetoothDiscovery', 'api': BRInterface_1.testStartBluetoothDiscovery, 'result': 'None', 'category': TestData_1.CaseCategory.BR },
    { 'name': '停止蓝牙发现', 'detail': '使能蓝牙', 'method': 'stopBluetoothDiscovery', 'api': BRInterface_1.testStopBluetoothDiscovery, 'result': 'None', 'category': TestData_1.CaseCategory.BR },
    { 'name': '配对设备', 'detail': '使能蓝牙', 'method': 'PairDevice', 'api': BRInterface_1.testPairDevice, 'result': 'None', 'category': TestData_1.CaseCategory.BR },
    { 'name': '获取远程设备名称', 'detail': '使能蓝牙', 'method': 'getRemoteDeviceName', 'api': BRInterface_1.testGetRemoteDeviceName, 'result': 'None', 'category': TestData_1.CaseCategory.BR },
    { 'name': '获取远程设备类型', 'detail': '使能蓝牙', 'method': 'getRemoteDeviceClass', 'api': BRInterface_1.testGetRemoteDeviceClass, 'result': 'None', 'category': TestData_1.CaseCategory.BR },
    { 'name': '获取配对设备', 'detail': '使能蓝牙', 'method': 'getPairedDevices', 'api': BRInterface_1.testGetPairedDevices, 'result': 'None', 'category': TestData_1.CaseCategory.BR },
    { 'name': '取消配对设备', 'detail': '使能蓝牙', 'method': 'cancelPairedDevice', 'api': BRInterface_1.testCancelPairedDevice, 'result': 'None', 'category': TestData_1.CaseCategory.BR },
    { 'name': '获取配置文件连接状态1', 'detail': '使能蓝牙', 'method': 'getProfileConnState1', 'api': BRInterface_1.testGetProfileConnState1, 'result': 'None', 'category': TestData_1.CaseCategory.BR },
    { 'name': '获取配置文件连接状态2', 'detail': '使能蓝牙', 'method': 'getProfileConnState2', 'api': BRInterface_1.testGetProfileConnState2, 'result': 'None', 'category': TestData_1.CaseCategory.BR },
    { 'name': '注册状态变化', 'detail': '使能蓝牙', 'method': 'bluetooth.on(stateChange)', 'api': BRInterface_1.testOnStateChange, 'result': 'None', 'category': TestData_1.CaseCategory.BR },
    { 'name': '注册pin码变化', 'detail': '使能蓝牙', 'method': 'bluetooth.on(pinRequired)', 'api': BRInterface_1.testOnPinRequired, 'result': 'None', 'category': TestData_1.CaseCategory.BR },
    { 'name': '注册绑定状态变化', 'detail': '使能蓝牙', 'method': 'bluetooth.on(bondStateChange)', 'api': BRInterface_1.testOnBondStateChange, 'result': 'None', 'category': TestData_1.CaseCategory.BR }
];
const BleTestCase = [
    { 'name': '创建Gatt服务器', 'detail': '使能蓝牙', 'method': 'createGattServer', 'api': BLEInterface_1.testCreateGattServer, 'result': 'None', 'category': TestData_1.CaseCategory.BLE },
    { 'name': '创建Gatt客户端设备', 'detail': '使能蓝牙', 'method': 'createGattClientDevice', 'api': BLEInterface_1.testCreateGattClientDevice, 'result': 'None', 'category': TestData_1.CaseCategory.BLE },
    { 'name': '连接到BLE设备', 'detail': '使能蓝牙', 'method': 'getConnectedBLEDevices', 'api': BLEInterface_1.testGetConnectedBLEDevices, 'result': 'None', 'category': TestData_1.CaseCategory.BLE },
    { 'name': '开始BLE扫描', 'detail': '使能蓝牙', 'method': 'startBLEScan', 'api': BLEInterface_1.testStartBLEScan, 'result': 'None', 'category': TestData_1.CaseCategory.BLE },
    { 'name': '停止BLE扫描', 'detail': '使能蓝牙', 'method': 'stopBLEScan', 'api': BLEInterface_1.testStopBLEScan, 'result': 'None', 'category': TestData_1.CaseCategory.BLE },
    { 'name': 'BLE设备查找变化', 'detail': '使能蓝牙', 'method': 'bluetooth.on(BLEDeviceFind)', 'api': BLEInterface_1.testOnBLEDeviceFind, 'result': 'None', 'category': TestData_1.CaseCategory.BLE }
];
const GattTestCase = [
    { 'name': 'GATT_Client测试', 'detail': '使能蓝牙', 'method': 'gatt_Client', 'api': GATTInterface_1.testGATT_Client, 'result': 'None', 'category': TestData_1.CaseCategory.Gatt },
    { 'name': 'GATT_Server测试', 'detail': '使能蓝牙', 'method': 'gatt_Server', 'api': GATTInterface_1.testGATT_Server, 'result': 'None', 'category': TestData_1.CaseCategory.Gatt },
];
const SppTestCase = [
    { 'name': 'SPP_CLIENT测试', 'detail': '使能蓝牙', 'method': 'spp_Client', 'api': SPPInterface_1.testSPP_Client, 'result': 'None', 'category': TestData_1.CaseCategory.SPP },
    { 'name': 'SPP_SERVER测试', 'detail': '使能蓝牙', 'method': 'spp_Server', 'api': SPPInterface_1.testSPP_Server, 'result': 'None', 'category': TestData_1.CaseCategory.SPP },
];
const ProfileTestCase = [
    { 'name': 'A2DP配置文件', 'detail': '使能蓝牙', 'method': 'a2dp_profile', 'api': ProfileInterface_1.testA2DP_Profile, 'result': 'None', 'category': TestData_1.CaseCategory.Profile },
    { 'name': 'HANDS配置文件', 'detail': '使能蓝牙', 'method': 'hands_profile', 'api': ProfileInterface_1.testHANDS_Profile, 'result': 'None', 'category': TestData_1.CaseCategory.Profile },
    { 'name': 'HID_HOST配置文件', 'detail': '使能蓝牙', 'method': 'hid_host_profile', 'api': ProfileInterface_1.testHID_HOST_Profile, 'result': 'None', 'category': TestData_1.CaseCategory.Profile },
];
const TestComposition = [
    //  { 'name': 'API', 'detail': '手动API测试', 'url': 'pages/manualApiTest', 'image': $r('app.media.Apiauto'), 'category': Category.All},
    //  { 'name': 'Scenario', 'detail': '手动场景测试', 'url': 'pages/manualApiTest', 'image': $r('app.media.ScenarioTest'), 'category': Category.All},
    //  { 'name': 'AutoTest', 'detail': '自动测试', 'url': 'pages/manualApiTest', 'image': $r('app.media.AutotestIcon'), 'category': Category.All},
    //  { 'name': 'Config', 'detail': '配置', 'url': 'pages/manualApiTest', 'image': $r('app.media.Setting'), 'category': Category.All},
    { 'name': 'Classic', 'detail': '经典蓝牙测试', 'url': 'pages/subManualApiTest/BrInterfaceTest', 'image': { "id": 16777532, "type": 20000, params: [] }, 'category': TestData_1.Category.API },
    { 'name': 'BLE', 'detail': '低功耗蓝牙测试', 'url': 'pages/subManualApiTest/bleTest', 'image': { "id": 16777540, "type": 20000, params: [] }, 'category': TestData_1.Category.API },
    { 'name': 'Gatt', 'detail': 'GATT测试', 'url': 'pages/subManualApiTest/gattTest', 'image': { "id": 16777533, "type": 20000, params: [] }, 'category': TestData_1.Category.API },
    { 'name': 'SPP', 'detail': 'SPP测试', 'url': 'pages/subManualApiTest/sppTest', 'image': { "id": 16777551, "type": 20000, params: [] }, 'category': TestData_1.Category.API },
    { 'name': 'Profile', 'detail': 'Profile测试', 'url': 'pages/subManualApiTest/profileTest', 'image': { "id": 16777550, "type": 20000, params: [] }, 'category': TestData_1.Category.API },
    { 'name': 'Scenario', 'detail': '场景测试', 'image': { "id": 16777535, "type": 20000, params: [] }, 'category': TestData_1.Category.Scenario },
    { 'name': 'AutoTest', 'detail': '自动测试', 'image': { "id": 16777526, "type": 20000, params: [] }, 'category': TestData_1.Category.AutoTest },
    { 'name': 'Benchmark', 'detail': '性能测试', 'image': { "id": 16777537, "type": 20000, params: [] }, 'category': TestData_1.Category.BenchMark },
];
function initTestData() {
    let TestDataArray = [];
    TestComposition.forEach(item => {
        TestDataArray.push(new TestData_1.TestData(item.name, item.detail, item.url, item.image, item.category));
    });
    return TestDataArray;
}
exports.initTestData = initTestData;
function initBRApiData() {
    let TestApiArray = [];
    BRTestCase.forEach(item => {
        TestApiArray.push(new TestData_1.TestApi(item.name, item.detail, item.method, item.api, item.result, item.category));
    });
    return TestApiArray;
}
exports.initBRApiData = initBRApiData;
function initBleApiData() {
    let TestApiArray = [];
    BleTestCase.forEach(item => {
        TestApiArray.push(new TestData_1.TestApi(item.name, item.detail, item.method, item.api, item.result, item.category));
    });
    return TestApiArray;
}
exports.initBleApiData = initBleApiData;
function initGattApiData() {
    let TestApiArray = [];
    GattTestCase.forEach(item => {
        TestApiArray.push(new TestData_1.TestApi(item.name, item.detail, item.method, item.api, item.result, item.category));
    });
    return TestApiArray;
}
exports.initGattApiData = initGattApiData;
function initSppApiData() {
    let TestApiArray = [];
    SppTestCase.forEach(item => {
        TestApiArray.push(new TestData_1.TestApi(item.name, item.detail, item.method, item.api, item.result, item.category));
    });
    return TestApiArray;
}
exports.initSppApiData = initSppApiData;
function initProfileApiData() {
    let TestApiArray = [];
    ProfileTestCase.forEach(item => {
        TestApiArray.push(new TestData_1.TestApi(item.name, item.detail, item.method, item.api, item.result, item.category));
    });
    return TestApiArray;
}
exports.initProfileApiData = initProfileApiData;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Utils/BaseModel.ts":
/*!***********************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Utils/BaseModel.ts ***!
  \***********************************************************************************************************/
/***/ ((__unused_webpack_module, exports) => {

"use strict";

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
Object.defineProperty(exports, "__esModule", ({ value: true }));
class BaseModel {
    constructor() {
    }
}
exports["default"] = BaseModel;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Utils/ConfigData.ts":
/*!************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Utils/ConfigData.ts ***!
  \************************************************************************************************************/
/***/ ((__unused_webpack_module, exports) => {

"use strict";

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
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.ConfigData = void 0;
class ConfigData {
    constructor() {
        this.TAG = 'Bluetooth_Test ';
        this.WH_100_100 = '100%';
        this.WH_25_100 = '25%';
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
        this.FUNCTION_TYPE_HDC = 4;
    }
}
exports.ConfigData = ConfigData;
let configData = new ConfigData();
exports["default"] = configData;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Utils/LogUtil.ts":
/*!*********************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Utils/LogUtil.ts ***!
  \*********************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.LogUtil = void 0;
const BaseModel_1 = __importDefault(__webpack_require__(/*! ./BaseModel */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Utils/BaseModel.ts"));
let LogLevel = {
    /**
       * debug
       */
    DEBUG: 3,
    /**
       * info
       */
    INFO: 4,
    /**
       * warn
       */
    WARN: 5,
    /**
       * error
       */
    ERROR: 6,
    /**
       * fatal
       */
    FATAL: 7,
};
const LOG_LEVEL = LogLevel.INFO;
/**
 *  log package tool class
 */
class LogUtil extends BaseModel_1.default {
    debug(msg) {
        if (LogLevel.DEBUG >= LOG_LEVEL) {
            console.info(msg);
        }
    }
    log(msg) {
        if (LogLevel.INFO >= LOG_LEVEL) {
            console.log(msg);
        }
    }
    info(msg) {
        if (LogLevel.INFO >= LOG_LEVEL) {
            console.info(msg);
        }
    }
    warn(msg) {
        if (LogLevel.WARN >= LOG_LEVEL) {
            console.warn(msg);
        }
    }
    error(msg) {
        if (LogLevel.ERROR >= LOG_LEVEL) {
            console.error(msg);
        }
    }
}
exports.LogUtil = LogUtil;
let mLogUtil = new LogUtil();
exports["default"] = mLogUtil;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/EntryComponent.ets":
/*!*********************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/EntryComponent.ets ***!
  \*********************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

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
const componentconfig_1 = __importDefault(__webpack_require__(/*! ./componentconfig */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/componentconfig.ets"));
class EntryComponent extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.__isShow = new ObservedPropertyObject(true, this, "isShow");
        this.__settingIcon = new SynchedPropertySimpleOneWay(params.settingIcon, this, "settingIcon");
        this.__endTextIsShow = new ObservedPropertyObject(true, this, "endTextIsShow");
        this.settingTitle = undefined;
        this.__settingSummary = new ObservedPropertyObject(null, this, "settingSummary");
        this.settingValue = undefined;
        this.__settingArrow = new ObservedPropertyObject(null, this, "settingArrow");
        this.__settingArrowStyle = new SynchedPropertySimpleOneWay(params.settingArrowStyle, this, "settingArrowStyle");
        this.settingUri = undefined;
        this.__titleFontColor = new ObservedPropertyObject({ "id": 125829210, "type": 10001, params: [] }, this, "titleFontColor");
        this.enabled = true;
        this.onArrowClick = undefined;
        this.__isTouched = new ObservedPropertySimple(false, this, "isTouched");
        this.height = { "id": 16777516, "type": 10002, params: [] };
        this.image_wh = { "id": 16777507, "type": 10002, params: [] };
        this.fontSize = { "id": 125829684, "type": 10002, params: [] };
        this.valueFontSize = { "id": 125829685, "type": 10002, params: [] };
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.isShow !== undefined) {
            this.isShow = params.isShow;
        }
        this.settingIcon = params.settingIcon;
        if (params.endTextIsShow !== undefined) {
            this.endTextIsShow = params.endTextIsShow;
        }
        if (params.settingTitle !== undefined) {
            this.settingTitle = params.settingTitle;
        }
        if (params.settingSummary !== undefined) {
            this.settingSummary = params.settingSummary;
        }
        if (params.settingValue !== undefined) {
            this.settingValue = params.settingValue;
        }
        if (params.settingArrow !== undefined) {
            this.settingArrow = params.settingArrow;
        }
        this.settingArrowStyle = params.settingArrowStyle;
        if (params.settingUri !== undefined) {
            this.settingUri = params.settingUri;
        }
        if (params.titleFontColor !== undefined) {
            this.titleFontColor = params.titleFontColor;
        }
        if (params.enabled !== undefined) {
            this.enabled = params.enabled;
        }
        if (params.onArrowClick !== undefined) {
            this.onArrowClick = params.onArrowClick;
        }
        if (params.isTouched !== undefined) {
            this.isTouched = params.isTouched;
        }
        if (params.height !== undefined) {
            this.height = params.height;
        }
        if (params.image_wh !== undefined) {
            this.image_wh = params.image_wh;
        }
        if (params.fontSize !== undefined) {
            this.fontSize = params.fontSize;
        }
        if (params.valueFontSize !== undefined) {
            this.valueFontSize = params.valueFontSize;
        }
    }
    aboutToBeDeleted() {
        this.__isShow.aboutToBeDeleted();
        this.__settingIcon.aboutToBeDeleted();
        this.__endTextIsShow.aboutToBeDeleted();
        this.__settingSummary.aboutToBeDeleted();
        this.__settingArrow.aboutToBeDeleted();
        this.__settingArrowStyle.aboutToBeDeleted();
        this.__titleFontColor.aboutToBeDeleted();
        this.__isTouched.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id());
    }
    get isShow() {
        return this.__isShow.get();
    }
    set isShow(newValue) {
        this.__isShow.set(newValue);
    }
    get settingIcon() {
        return this.__settingIcon.get();
    }
    set settingIcon(newValue) {
        this.__settingIcon.set(newValue);
    }
    get endTextIsShow() {
        return this.__endTextIsShow.get();
    }
    set endTextIsShow(newValue) {
        this.__endTextIsShow.set(newValue);
    }
    get settingSummary() {
        return this.__settingSummary.get();
    }
    set settingSummary(newValue) {
        this.__settingSummary.set(newValue);
    }
    get settingArrow() {
        return this.__settingArrow.get();
    }
    set settingArrow(newValue) {
        this.__settingArrow.set(newValue);
    }
    get settingArrowStyle() {
        return this.__settingArrowStyle.get();
    }
    set settingArrowStyle(newValue) {
        this.__settingArrowStyle.set(newValue);
    }
    get titleFontColor() {
        return this.__titleFontColor.get();
    }
    set titleFontColor(newValue) {
        this.__titleFontColor.set(newValue);
    }
    get isTouched() {
        return this.__isTouched.get();
    }
    set isTouched(newValue) {
        this.__isTouched.set(newValue);
    }
    render() {
        Row.create();
        Row.height(this.height);
        Row.width(componentconfig_1.default.WH_100_100);
        Row.padding({ left: { "id": 125829723, "type": 10002, params: [] } });
        Row.borderRadius({ "id": 125829719, "type": 10002, params: [] });
        Row.linearGradient((this.enabled && this.isTouched) ? {
            angle: 90,
            direction: GradientDirection.Right,
            colors: [[{ "id": 16777277, "type": 10001, params: [] }, 0.0], [{ "id": 16777278, "type": 10001, params: [] }, 1.0]]
        } : {
            angle: 90,
            direction: GradientDirection.Right,
            colors: [[{ "id": 125829123, "type": 10001, params: [] }, 1], [{ "id": 125829123, "type": 10001, params: [] }, 1]]
        });
        Row.alignItems(VerticalAlign.Center);
        Row.onTouch((event) => {
            if (event.type === TouchType.Down) {
                this.isTouched = true;
            }
            if (event.type === TouchType.Up) {
                this.isTouched = false;
            }
        });
        Row.create();
        Row.flexShrink(0);
        Row.alignItems(VerticalAlign.Center);
        Row.align(Alignment.Start);
        Row.layoutWeight(1);
        Image.create(this.settingIcon);
        Image.width(this.image_wh);
        Image.height(this.image_wh);
        Image.margin({ right: { "id": 16777459, "type": 10002, params: [] } });
        Image.visibility('' === this.settingIcon ? Visibility.None : Visibility.Visible);
        Image.objectFit(ImageFit.Contain);
        Column.create();
        Column.alignItems(HorizontalAlign.Start);
        Text.create(this.settingTitle);
        Text.fontColor(this.enabled ? this.titleFontColor : { "id": 125829605, "type": 10001, params: [] });
        Text.fontSize(this.fontSize);
        Text.textAlign(TextAlign.Start);
        Text.maxLines(componentconfig_1.default.MAX_LINES_3);
        Text.textOverflow({ overflow: TextOverflow.Ellipsis });
        Text.pop();
        Row.create();
        Text.create({ "id": 16777275, "type": 10003, params: [] });
        Text.fontColor({ "id": 125829216, "type": 10001, params: [] });
        Text.fontSize({ "id": 125829685, "type": 10002, params: [] });
        Text.textAlign(TextAlign.Start);
        Text.maxLines(componentconfig_1.default.MAX_LINES_1);
        Text.textOverflow({ overflow: TextOverflow.Ellipsis });
        Text.visibility('pages/applicationInfo' === this.settingUri ? Visibility.Visible : Visibility.None);
        Text.margin({ top: { "id": 125829738, "type": 10002, params: [] } });
        Text.pop();
        Text.create(this.settingSummary);
        Text.fontColor({ "id": 125829216, "type": 10001, params: [] });
        Text.fontSize({ "id": 125829685, "type": 10002, params: [] });
        Text.fontWeight('sans-serif');
        Text.textAlign(TextAlign.Start);
        Text.maxLines(componentconfig_1.default.MAX_LINES_1);
        Text.textOverflow({ overflow: TextOverflow.Ellipsis });
        Text.visibility(null === this.settingSummary || undefined === this.settingSummary ? Visibility.None : Visibility.Visible);
        Text.margin({ top: { "id": 125829738, "type": 10002, params: [] } });
        Text.pop();
        Row.pop();
        Column.pop();
        Row.pop();
        Row.create();
        Row.alignItems(VerticalAlign.Center);
        Row.align(Alignment.End);
        Text.create(this.settingValue);
        Text.fontSize(this.valueFontSize);
        Text.fontColor({ "id": 125829216, "type": 10001, params: [] });
        Text.fontWeight('sans-serif');
        Text.height({ "id": 16777501, "type": 10002, params: [] });
        Text.margin({ left: { "id": 125829747, "type": 10002, params: [] } });
        Text.align(Alignment.End);
        Text.pop();
        If.create();
        if (!this.settingArrowStyle && this.settingArrow) {
            If.branchId(0);
            Image.create(this.settingArrow);
            Image.visibility(null === this.settingArrow ? Visibility.None : Visibility.Visible);
            Image.width({ "id": 16777465, "type": 10002, params: [] });
            Image.height({ "id": 16777482, "type": 10002, params: [] });
            Image.margin({ left: { "id": 16777370, "type": 10002, params: [] }, right: { "id": 16777374, "type": 10002, params: [] } });
        }
        else if (this.settingArrow) {
            If.branchId(1);
            Image.create(this.settingArrow);
            Image.visibility(null === this.settingArrow ? Visibility.None : Visibility.Visible);
            Image.width({ "id": 16777505, "type": 10002, params: [] });
            Image.height({ "id": 16777505, "type": 10002, params: [] });
            Image.padding({ "id": 16777500, "type": 10002, params: [] });
            Image.margin({ left: { "id": 16777500, "type": 10002, params: [] }, right: { "id": 16777500, "type": 10002, params: [] } });
            Image.borderRadius({ "id": 125829719, "type": 10002, params: [] });
            Image.onClick(this.onArrowClick);
        }
        If.pop();
        Row.pop();
        Row.pop();
    }
}
exports["default"] = EntryComponent;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/componentconfig.ets":
/*!**********************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/componentconfig.ets ***!
  \**********************************************************************************************************************/
/***/ ((__unused_webpack_module, exports) => {

"use strict";

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

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/controller/BaseSettingsController.ts":
/*!***************************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/controller/BaseSettingsController.ts ***!
  \***************************************************************************************************************************************/
/***/ ((__unused_webpack_module, exports, __webpack_require__) => {

"use strict";

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
Object.defineProperty(exports, "__esModule", ({ value: true }));
__webpack_require__(/*! ./ISettingsController */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/controller/ISettingsController.ts");
class BaseSettingsController {
    /**
     * Bind component.
     */
    bindComponent(component) {
        this.getComponent = () => component;
        // set default property values by component
        for (var key in this) {
            if (key in component) {
                this[key] = component[key];
            }
        }
        return this;
    }
    /**
     * Bind component's properties, note that only basic types can be transported.
     * Type like Resource may meet unexpected error.
     * If you want to transport resource string or color, ets. AppStorage is suggested @StorageLink.
     */
    bindProperties(componentProperties, controllerProperties) {
        for (let i = 0; i < componentProperties.length; i++) {
            this.defineBoundProperty(componentProperties[i], controllerProperties ? controllerProperties[i] : componentProperties[i]);
        }
        return this;
    }
    /**
     * Initialize data.
     */
    initData() {
        return this;
    }
    ;
    /**
     * Subscribe listeners.
     */
    subscribe() {
        return this;
    }
    ;
    /**
     * Unsubscribe listeners.
     */
    unsubscribe() {
        return this;
    }
    ;
    /**
     * Define bound properties.
     */
    defineBoundProperty(componentProperty, controllerProperty) {
        let __v = this[controllerProperty];
        Object.defineProperty(this, controllerProperty, {
            get: function () {
                return __v;
            },
            set: function (value) {
                __v = value;
                this.getComponent()[componentProperty] = value;
            }
        });
    }
}
exports["default"] = BaseSettingsController;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/controller/ISettingsController.ts":
/*!************************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/controller/ISettingsController.ts ***!
  \************************************************************************************************************************************/
/***/ ((__unused_webpack_module, exports) => {

"use strict";

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
Object.defineProperty(exports, "__esModule", ({ value: true }));


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/headComponent.ets":
/*!********************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/headComponent.ets ***!
  \********************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

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
const ConfigData_1 = __importDefault(__webpack_require__(/*! ../Utils/ConfigData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Utils/ConfigData.ts"));
var _system_router_1  = globalThis.requireNativeModule('system.router');
class HeadComponent extends View {
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
        Row.width(ConfigData_1.default.WH_100_100);
        Row.padding({ left: { "id": 16777519, "type": 10002, params: [] } });
        Row.height({ "id": 16777509, "type": 10002, params: [] });
        Row.alignItems(VerticalAlign.Center);
        Row.align(Alignment.Start);
        Stack.create({ alignContent: Alignment.Center });
        Stack.margin({ left: { "id": 16777500, "type": 10002, params: [] }, right: { "id": 16777470, "type": 10002, params: [] } });
        Stack.backgroundColor(this.isTouch ? { "id": 16777288, "type": 10001, params: [] } : { "id": 16777283, "type": 10001, params: [] });
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
        Image.create({ "id": 16777545, "type": 20000, params: [] });
        Image.width({ "id": 16777493, "type": 10002, params: [] });
        Image.height({ "id": 16777493, "type": 10002, params: [] });
        Stack.pop();
        Text.create(this.headName);
        Text.fontSize({ "id": 16777387, "type": 10002, params: [] });
        Text.lineHeight({ "id": 16777496, "type": 10002, params: [] });
        Text.fontFamily('HarmonyHeiTi-Bold');
        Text.fontWeight(FontWeight.Bold);
        Text.fontColor({ "id": 16777292, "type": 10001, params: [] });
        Text.maxLines(ConfigData_1.default.MAX_LINES_1);
        Text.textOverflow({ overflow: TextOverflow.Ellipsis });
        Text.textAlign(TextAlign.Start);
        Text.margin({ top: { "id": 16777467, "type": 10002, params: [] }, bottom: { "id": 16777469, "type": 10002, params: [] } });
        Text.pop();
        Row.pop();
    }
}
exports["default"] = HeadComponent;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/headcomponent.ets":
/*!********************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/headcomponent.ets ***!
  \********************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

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
const ConfigData_1 = __importDefault(__webpack_require__(/*! ../Utils/ConfigData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Utils/ConfigData.ts"));
var _system_router_1  = globalThis.requireNativeModule('system.router');
class HeadComponent extends View {
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
        Row.width(ConfigData_1.default.WH_100_100);
        Row.padding({ left: { "id": 16777519, "type": 10002, params: [] } });
        Row.height({ "id": 16777509, "type": 10002, params: [] });
        Row.alignItems(VerticalAlign.Center);
        Row.align(Alignment.Start);
        Stack.create({ alignContent: Alignment.Center });
        Stack.margin({ left: { "id": 16777500, "type": 10002, params: [] }, right: { "id": 16777470, "type": 10002, params: [] } });
        Stack.backgroundColor(this.isTouch ? { "id": 16777288, "type": 10001, params: [] } : { "id": 16777283, "type": 10001, params: [] });
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
        Image.create({ "id": 16777545, "type": 20000, params: [] });
        Image.width({ "id": 16777493, "type": 10002, params: [] });
        Image.height({ "id": 16777493, "type": 10002, params: [] });
        Stack.pop();
        Text.create(this.headName);
        Text.fontSize({ "id": 16777387, "type": 10002, params: [] });
        Text.lineHeight({ "id": 16777496, "type": 10002, params: [] });
        Text.fontFamily('HarmonyHeiTi-Bold');
        Text.fontWeight(FontWeight.Bold);
        Text.fontColor({ "id": 16777292, "type": 10001, params: [] });
        Text.maxLines(ConfigData_1.default.MAX_LINES_1);
        Text.textOverflow({ overflow: TextOverflow.Ellipsis });
        Text.textAlign(TextAlign.Start);
        Text.margin({ top: { "id": 16777467, "type": 10002, params: [] }, bottom: { "id": 16777469, "type": 10002, params: [] } });
        Text.pop();
        Row.pop();
    }
}
exports["default"] = HeadComponent;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/subEntryComponent.ets":
/*!************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/subEntryComponent.ets ***!
  \************************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

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
const ConfigData_1 = __importDefault(__webpack_require__(/*! ../Utils/ConfigData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Utils/ConfigData.ts"));
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
        Navigator.params({ date: this.date, deviceId: this.deviceId });
        Navigator.padding({ "id": 16777370, "type": 10002, params: [] });
        Navigator.height({ "id": 16777460, "type": 10002, params: [] });
        Navigator.borderRadius({ "id": 16777421, "type": 10002, params: [] });
        Navigator.backgroundColor({ "id": 125829123, "type": 10001, params: [] });
        Flex.create({ justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center });
        Flex.height(ConfigData_1.default.WH_100_100);
        Flex.width(ConfigData_1.default.WH_100_100);
        Flex.borderRadius({ "id": 16777419, "type": 10002, params: [] });
        Flex.linearGradient(this.isTouched ? {
            angle: 90,
            direction: GradientDirection.Right,
            colors: [[{ "id": 16777277, "type": 10001, params: [] }, 0.0], [{ "id": 16777278, "type": 10001, params: [] }, 1.0]]
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
        Text.create(this.title);
        Text.fontSize({ "id": 16777389, "type": 10002, params: [] });
        Text.lineHeight({ "id": 16777494, "type": 10002, params: [] });
        Text.fontColor({ "id": 16777292, "type": 10001, params: [] });
        Text.fontWeight(FontWeight.Medium);
        Text.margin({ left: { "id": 16777374, "type": 10002, params: [] } });
        Text.textAlign(TextAlign.Start);
        Text.height({ "id": 16777494, "type": 10002, params: [] });
        Text.pop();
        Row.pop();
        Image.create(('app.media.ic_settings_arrow'));
        Image.width({ "id": 16777465, "type": 10002, params: [] });
        Image.height({ "id": 16777460, "type": 10002, params: [] });
        Image.margin({ right: { "id": 16777374, "type": 10002, params: [] } });
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
        Navigator.padding({ "id": 16777370, "type": 10002, params: [] });
        Navigator.height({ "id": 16777509, "type": 10002, params: [] });
        Navigator.borderRadius({ "id": 16777421, "type": 10002, params: [] });
        Navigator.backgroundColor({ "id": 125829123, "type": 10001, params: [] });
        Flex.create({ justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center });
        Flex.height(ConfigData_1.default.WH_100_100);
        Flex.width(ConfigData_1.default.WH_100_100);
        Flex.borderRadius({ "id": 16777420, "type": 10002, params: [] });
        Flex.linearGradient(this.isTouched ? {
            angle: 90,
            direction: GradientDirection.Right,
            colors: [[{ "id": 16777277, "type": 10001, params: [] }, 0.0], [{ "id": 16777278, "type": 10001, params: [] }, 1.0]]
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
        Text.create(this.title);
        Text.fontSize({ "id": 16777382, "type": 10002, params: [] });
        Text.lineHeight({ "id": 16777480, "type": 10002, params: [] });
        Text.fontWeight(FontWeight.Medium);
        Text.fontColor({ "id": 16777292, "type": 10001, params: [] });
        Text.margin({ left: { "id": 16777374, "type": 10002, params: [] } });
        Text.textAlign(TextAlign.Start);
        Text.pop();
        Row.pop();
        Row.create();
        Text.create(this.endText);
        Text.fontSize({ "id": 16777381, "type": 10002, params: [] });
        Text.lineHeight({ "id": 16777473, "type": 10002, params: [] });
        Text.fontWeight(FontWeight.Regular);
        Text.fontColor({ "id": 125829216, "type": 10001, params: [] });
        Text.margin({ right: { "id": 16777370, "type": 10002, params: [] } });
        Text.textAlign(TextAlign.End);
        Text.pop();
        Image.create(('app.media.ic_settings_arrow'));
        Image.width({ "id": 16777465, "type": 10002, params: [] });
        Image.height({ "id": 16777482, "type": 10002, params: [] });
        Image.margin({ right: { "id": 16777374, "type": 10002, params: [] } });
        Row.pop();
        Flex.pop();
        Navigator.pop();
    }
}
exports.SubEntryComponentWithEndText = SubEntryComponentWithEndText;


/***/ }),

/***/ "../../api/@ohos.bluetooth.d.ts":
/*!**************************************!*\
  !*** ../../api/@ohos.bluetooth.d.ts ***!
  \**************************************/
/***/ (() => {



/***/ }),

/***/ "../../api/@system.prompt.d.ts":
/*!*************************************!*\
  !*** ../../api/@system.prompt.d.ts ***!
  \*************************************/
/***/ (() => {



/***/ }),

/***/ "../../api/@system.router.d.ts":
/*!*************************************!*\
  !*** ../../api/@system.router.d.ts ***!
  \*************************************/
/***/ (() => {



/***/ })

}]);
          globalThis["__common_module_cache___2a33209e6af7922da1521a8a828bfbd0"] = globalThis["__common_module_cache___2a33209e6af7922da1521a8a828bfbd0"] || {};
          globalThis["webpackChunk_2a33209e6af7922da1521a8a828bfbd0"].forEach((item)=> {
            Object.keys(item[1]).forEach((element) => {
              globalThis["__common_module_cache___2a33209e6af7922da1521a8a828bfbd0"][element] = null;
            })
          });
//# sourceMappingURL=commons.js.map