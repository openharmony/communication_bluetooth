(globalThis["webpackChunk_840471a2ef2afe531ee6aea9267b2e21"] = globalThis["webpackChunk_840471a2ef2afe531ee6aea9267b2e21"] || []).push([["commons"],{

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Component/autoContentTable.ets":
/*!***************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Component/autoContentTable.ets ***!
  \***************************************************************************************************************************/
/***/ ((__unused_webpack_module, exports, __webpack_require__) => {

"use strict";

Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.AutoContentTable = void 0;
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
/**
 *  ContentTable Component Page Of Bluetooth test
 */
__webpack_require__(/*! ../MainAbility/model/testData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/testData.ets");
__webpack_require__(/*! @ohos.events.emitter */ "../../api/@ohos.events.emitter.d.ts");
__webpack_require__(/*! @system.prompt */ "../../api/@system.prompt.d.ts");
class AutoContentTable extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.testItem = undefined;
        this.__currentAutoItem = new ObservedPropertyObject({}, this, "currentAutoItem");
        this.__autoItems = new ObservedPropertyObject([], this, "autoItems");
        this.__changeIndex = new SynchedPropertySimpleOneWay(params.changeIndex, this, "changeIndex");
        this.__testMessage = new SynchedPropertySimpleOneWay(params.testMessage, this, "testMessage");
        this.__testState = new SynchedPropertySimpleOneWay(params.testState, this, "testState");
        this.__localName = new ObservedPropertySimple('dayu', this, "localName");
        this.__currentIndex = new ObservedPropertySimple(1, this, "currentIndex");
        this.__pairedMac = AppStorage.GetOrCreate().setAndLink("pairedMac", "", this);
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.testItem !== undefined) {
            this.testItem = params.testItem;
        }
        if (params.currentAutoItem !== undefined) {
            this.currentAutoItem = params.currentAutoItem;
        }
        if (params.autoItems !== undefined) {
            this.autoItems = params.autoItems;
        }
        this.changeIndex = params.changeIndex;
        this.testMessage = params.testMessage;
        this.testState = params.testState;
        if (params.localName !== undefined) {
            this.localName = params.localName;
        }
        if (params.currentIndex !== undefined) {
            this.currentIndex = params.currentIndex;
        }
    }
    aboutToBeDeleted() {
        this.__currentAutoItem.aboutToBeDeleted();
        this.__autoItems.aboutToBeDeleted();
        this.__changeIndex.aboutToBeDeleted();
        this.__testMessage.aboutToBeDeleted();
        this.__testState.aboutToBeDeleted();
        this.__localName.aboutToBeDeleted();
        this.__currentIndex.aboutToBeDeleted();
        this.__pairedMac.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id());
    }
    get currentAutoItem() {
        return this.__currentAutoItem.get();
    }
    set currentAutoItem(newValue) {
        this.__currentAutoItem.set(newValue);
    }
    get autoItems() {
        return this.__autoItems.get();
    }
    set autoItems(newValue) {
        this.__autoItems.set(newValue);
    }
    get changeIndex() {
        return this.__changeIndex.get();
    }
    set changeIndex(newValue) {
        this.__changeIndex.set(newValue);
    }
    get testMessage() {
        return this.__testMessage.get();
    }
    set testMessage(newValue) {
        this.__testMessage.set(newValue);
    }
    get testState() {
        return this.__testState.get();
    }
    set testState(newValue) {
        this.__testState.set(newValue);
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
    get pairedMac() {
        return this.__pairedMac.get();
    }
    set pairedMac(newValue) {
        this.__pairedMac.set(newValue);
    }
    IngredientItem(autoItem, index) {
        Stack.create();
        Stack.debugLine("../Component/autoContentTable.ets(38:5)");
        Stack.height(40);
        Stack.borderRadius(10);
        Stack.margin({ top: 3, bottom: 3 });
        Stack.backgroundColor({ "id": 16777312, "type": 10001, params: [] });
        Flex.create();
        Flex.debugLine("../Component/autoContentTable.ets(39:7)");
        Flex.create({ alignItems: ItemAlign.Center });
        Flex.debugLine("../Component/autoContentTable.ets(40:9)");
        Flex.height(30);
        Flex.width("10%");
        Flex.padding({ left: 10 });
        Image.create({ "id": 16777551, "type": 20000, params: [] });
        Image.debugLine("../Component/autoContentTable.ets(41:11)");
        Image.objectFit(ImageFit.Contain);
        Image.autoResize(false);
        Image.height(26);
        Image.width(25);
        Flex.pop();
        Flex.create({ direction: FlexDirection.Column, alignItems: ItemAlign.Start });
        Flex.debugLine("../Component/autoContentTable.ets(50:9)");
        Flex.height(30);
        Flex.layoutWeight(6);
        Row.create();
        Row.debugLine("../Component/autoContentTable.ets(51:11)");
        Row.height(30);
        Row.width("70%");
        Text.create(autoItem.method);
        Text.debugLine("../Component/autoContentTable.ets(52:13)");
        Text.fontSize("17vp");
        Text.margin({ top: "3vp", bottom: "3vp", left: "10vp" });
        Text.textAlign(TextAlign.Start);
        Text.height(40);
        Text.width(200);
        Text.pop();
        Row.pop();
        Flex.pop();
        Flex.create({ alignItems: ItemAlign.Center });
        Flex.debugLine("../Component/autoContentTable.ets(64:9)");
        Flex.layoutWeight(3);
        Flex.padding({ left: 10 });
        Text.create(this.changeIndex >= 0 ? this.getCurrentState(index) : autoItem.result);
        Text.debugLine("../Component/autoContentTable.ets(65:11)");
        Text.margin({ left: "10vp" });
        Text.fontSize("17vp");
        Text.flexGrow(1);
        Text.pop();
        Flex.pop();
        Flex.pop();
        Stack.pop();
    }
    getCurrentState(index) {
        //    if(this.changeIndex = -1) return this.apiItems[index].result
        this.currentIndex = this.changeIndex;
        if (index === this.currentIndex) {
            console.log("点击了");
            this.currentMessage = this.testMessage;
            if (this.testState) {
                return "Pass";
            }
            else {
                return "Fail";
            }
        }
        return this.autoItems[index].result;
    }
    render() {
        Column.create();
        Column.debugLine("../Component/autoContentTable.ets(100:5)");
        Column.height("80%");
        Column.padding({ top: 5, right: 10, left: 10 });
        Scroll.create();
        Scroll.debugLine("../Component/autoContentTable.ets(101:7)");
        Scroll.scrollBarWidth(20);
        Flex.create({
            direction: FlexDirection.Column,
            justifyContent: FlexAlign.SpaceBetween,
            alignItems: ItemAlign.Start
        });
        Flex.debugLine("../Component/autoContentTable.ets(102:9)");
        ForEach.create("1", this, ObservedObject.GetRawObject(this.autoItems), (item, index) => {
            this.IngredientItem(item, index);
        });
        ForEach.pop();
        Flex.pop();
        Scroll.pop();
        Column.pop();
    }
}
exports.AutoContentTable = AutoContentTable;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Component/contentTable.ets":
/*!***********************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Component/contentTable.ets ***!
  \***********************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.ContentTable = exports.InputDialog = exports.SetScanModeDialog = exports.GetProfileConnSateDialog = void 0;
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
/**
 *  ContentTable Component Page Of Bluetooth test
 */
__webpack_require__(/*! ../MainAbility/model/testData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/testData.ets");
var _system_prompt_1  = isSystemplugin('prompt', 'system') ? globalThis.systemplugin.prompt : globalThis.requireNapi('prompt');
class GetProfileConnSateDialog extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.hitText = undefined;
        this.getProfileList = [
            { value: 'A2DP_SOURCE' },
            { value: 'HANDS_FREE_AUDIO_GATEWAY' },
            { value: 'HID_HOST' }
        ];
        this.__currentClick = new ObservedPropertySimple(1, this, "currentClick");
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
        if (params.getProfileList !== undefined) {
            this.getProfileList = params.getProfileList;
        }
        if (params.currentClick !== undefined) {
            this.currentClick = params.currentClick;
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
        this.__currentClick.aboutToBeDeleted();
        this.__textValue.aboutToBeDeleted();
        this.__inputValue.aboutToBeDeleted();
        this.__apiItem.aboutToBeDeleted();
        this.__currentIndex.aboutToBeDeleted();
        this.__changeIndex.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id());
    }
    get currentClick() {
        return this.__currentClick.get();
    }
    set currentClick(newValue) {
        this.__currentClick.set(newValue);
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
        Column.debugLine("../Component/contentTable.ets(42:5)");
        Text.create(this.hitText);
        Text.debugLine("../Component/contentTable.ets(43:7)");
        Text.fontSize(20);
        Text.margin({ top: 10, bottom: 10 });
        Text.pop();
        Select.create(this.getProfileList);
        Select.debugLine("../Component/contentTable.ets(46:7)");
        Select.selected(this.currentClick);
        Select.value(this.getProfileList[this.currentClick].value);
        Select.font({ size: 20 });
        Select.selectedOptionFont({ size: 20 });
        Select.optionFont({ size: 17 });
        Select.height(80);
        Select.width("90%");
        Select.onSelect((index) => {
            console.info("Select:" + index);
            this.currentClick = index;
        });
        Select.pop();
        Flex.create({ justifyContent: FlexAlign.SpaceAround });
        Flex.debugLine("../Component/contentTable.ets(59:7)");
        Flex.margin({ bottom: 10 });
        Button.createWithLabel('取消');
        Button.debugLine("../Component/contentTable.ets(60:9)");
        Button.onClick(() => {
            this.controller.close();
            this.cancel();
        });
        Button.backgroundColor(0xffffff);
        Button.fontColor(Color.Black);
        Button.pop();
        Button.createWithLabel('确认');
        Button.debugLine("../Component/contentTable.ets(66:9)");
        Button.onClick(() => {
            this.inputValue = this.currentClick;
            this.changeIndex = this.currentIndex;
            //TODO: 设备调试时候需要打开
            if (true) {
                let message = "";
                message = this.apiItem.api(this.currentClick, this.pairedMac);
                this.apiItem.result = this.getProfileList[this.currentClick].value + "状态:" + message;
                console.info("this.apiItem.result:" + this.apiItem.result);
            }
            else {}
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
exports.GetProfileConnSateDialog = GetProfileConnSateDialog;
class SetScanModeDialog extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.hitText = undefined;
        this.scanModeList = [
            { value: 'SCAN_MODE_NONE' },
            { value: 'CONNECTABLE' },
            { value: 'GENERAL_DISCOVERABLE' },
            { value: 'LIMITED_DISCOVERABLE' },
            { value: 'CONNECTABLE_GENERAL_DISCOVERABLE' },
            { value: 'CONNECTABLE_LIMITED_DISCOVERABLE' }
        ];
        this.__currentClick = new ObservedPropertySimple(4, this, "currentClick");
        this.__textValue = new SynchedPropertySimpleTwoWay(params.textValue, this, "textValue");
        this.__inputValue = new SynchedPropertySimpleTwoWay(params.inputValue, this, "inputValue");
        this.__apiItem = new SynchedPropertyObjectTwoWay(params.apiItem, this, "apiItem");
        this.__currentIndex = new SynchedPropertySimpleTwoWay(params.currentIndex, this, "currentIndex");
        this.__changeIndex = new SynchedPropertySimpleTwoWay(params.changeIndex, this, "changeIndex");
        this.__duration = new ObservedPropertySimple(0, this, "duration");
        this.controller = undefined;
        this.cancel = undefined;
        this.confirm = undefined;
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.hitText !== undefined) {
            this.hitText = params.hitText;
        }
        if (params.scanModeList !== undefined) {
            this.scanModeList = params.scanModeList;
        }
        if (params.currentClick !== undefined) {
            this.currentClick = params.currentClick;
        }
        if (params.duration !== undefined) {
            this.duration = params.duration;
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
        this.__currentClick.aboutToBeDeleted();
        this.__textValue.aboutToBeDeleted();
        this.__inputValue.aboutToBeDeleted();
        this.__apiItem.aboutToBeDeleted();
        this.__currentIndex.aboutToBeDeleted();
        this.__changeIndex.aboutToBeDeleted();
        this.__duration.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id());
    }
    get currentClick() {
        return this.__currentClick.get();
    }
    set currentClick(newValue) {
        this.__currentClick.set(newValue);
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
    get duration() {
        return this.__duration.get();
    }
    set duration(newValue) {
        this.__duration.set(newValue);
    }
    setController(ctr) {
        this.controller = ctr;
    }
    render() {
        Column.create();
        Column.debugLine("../Component/contentTable.ets(115:5)");
        Text.create(this.hitText);
        Text.debugLine("../Component/contentTable.ets(116:7)");
        Text.fontSize(20);
        Text.margin({ top: 10, bottom: 10 });
        Text.pop();
        Select.create(this.scanModeList);
        Select.debugLine("../Component/contentTable.ets(119:7)");
        Select.selected(this.currentClick);
        Select.value(this.scanModeList[this.currentClick].value);
        Select.font({ size: 12 });
        Select.selectedOptionFont({ size: 12 });
        Select.optionFont({ size: 12 });
        Select.height(60);
        Select.width("90%");
        Select.onSelect((index) => {
            console.info("Select:" + index);
            this.currentClick = index;
        });
        Select.pop();
        TextInput.create({
            placeholder: '请输入延迟时间',
            text: this.duration.toString(),
            type: InputType.Number,
        });
        TextInput.debugLine("../Component/contentTable.ets(131:7)");
        TextInput.height(60);
        TextInput.width('90%');
        TextInput.onChange((value) => {
            if (value == "") {
                this.duration = 0;
            }
            else {
                this.duration = parseInt(value) == nil ? 0 : parseInt(value);
            }
        });
        Flex.create({ justifyContent: FlexAlign.SpaceAround });
        Flex.debugLine("../Component/contentTable.ets(144:7)");
        Flex.margin({ bottom: 10 });
        Button.createWithLabel('取消');
        Button.debugLine("../Component/contentTable.ets(145:9)");
        Button.onClick(() => {
            this.controller.close();
            this.cancel();
        });
        Button.backgroundColor(0xffffff);
        Button.fontColor(Color.Black);
        Button.pop();
        Button.createWithLabel('确认');
        Button.debugLine("../Component/contentTable.ets(151:9)");
        Button.onClick(() => {
            this.inputValue = this.currentClick;
            this.changeIndex = this.currentIndex;
            //TODO: 设备调试时候需要打开
            if (true) {
                let message = {};
                message = this.apiItem.api(this.currentClick, this.duration);
                //              this.apiItem.result = message.mod.toString() + message.duration.toString()
                this.apiItem.result = "类型:" + this.scanModeList[message.mod].value + "延迟:" + message.duration.toString();
                console.info("this.apiItem.result:" + this.apiItem.result);
            }
            else {}
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
exports.SetScanModeDialog = SetScanModeDialog;
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
        Column.debugLine("../Component/contentTable.ets(191:5)");
        Text.create(this.hitText);
        Text.debugLine("../Component/contentTable.ets(192:7)");
        Text.fontSize(20);
        Text.margin({ top: 10, bottom: 10 });
        Text.pop();
        TextInput.create({ placeholder: '', text: this.textValue });
        TextInput.debugLine("../Component/contentTable.ets(195:7)");
        TextInput.height(60);
        TextInput.width('90%');
        TextInput.onChange((value) => {
            this.textValue = value;
        });
        Flex.create({ justifyContent: FlexAlign.SpaceAround });
        Flex.debugLine("../Component/contentTable.ets(202:7)");
        Flex.margin({ bottom: 10 });
        Button.createWithLabel('取消');
        Button.debugLine("../Component/contentTable.ets(203:9)");
        Button.onClick(() => {
            this.controller.close();
            this.cancel();
        });
        Button.backgroundColor(0xffffff);
        Button.fontColor(Color.Black);
        Button.pop();
        Button.createWithLabel('确认');
        Button.debugLine("../Component/contentTable.ets(209:9)");
        Button.onClick(() => {
            this.inputValue = this.textValue;
            this.changeIndex = this.currentIndex;
            //TODO: 设备调试时候需要打开
            let message = "";
            if (true) {
                message = this.apiItem.api(this.textValue);
                this.apiItem.result = message;
            }
            else {}
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
exports.InputDialog = InputDialog;
class ContentTable extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.testItem = undefined;
        this.__currentApiItem = new ObservedPropertyObject({}, this, "currentApiItem");
        this.__apiItems = new ObservedPropertyObject([], this, "apiItems");
        this.__changeIndex = new ObservedPropertySimple(-1, this, "changeIndex");
        this.__localName = new ObservedPropertySimple('dayu', this, "localName");
        this.__currentIndex = new ObservedPropertySimple(1, this, "currentIndex");
        this.__pairedMac = AppStorage.GetOrCreate().setAndLink("pairedMac", "", this);
        this.getProfileConnSateDialog = new CustomDialogController({
            builder: () => {
                let jsDialog = new GetProfileConnSateDialog("2", this, {
                    hitText: "设置配置类型",
                    cancel: this.onCancel,
                    confirm: this.onAccept,
                    textValue: this.__localName,
                    inputValue: this.__localName,
                    currentIndex: this.__currentIndex,
                    apiItem: this.__currentApiItem,
                    changeIndex: this.__changeIndex,
                });
                jsDialog.setController(this.getProfileConnSateDialog);
                View.create(jsDialog);
            },
            cancel: this.onCancel(),
            autoCancel: true
        }, this);
        this.setScanModelDialog = new CustomDialogController({
            builder: () => {
                let jsDialog = new SetScanModeDialog("3", this, {
                    hitText: "设置扫描模式",
                    cancel: this.onCancel,
                    confirm: this.onAccept,
                    textValue: this.__localName,
                    inputValue: this.__localName,
                    currentIndex: this.__currentIndex,
                    apiItem: this.__currentApiItem,
                    changeIndex: this.__changeIndex,
                });
                jsDialog.setController(this.setScanModelDialog);
                View.create(jsDialog);
            },
            cancel: this.onCancel(),
            autoCancel: true
        }, this);
        this.inputDialog = new CustomDialogController({
            builder: () => {
                let jsDialog = new InputDialog("4", this, {
                    hitText: "请设置本地名称",
                    cancel: this.onCancel,
                    confirm: this.onAccept,
                    textValue: this.__localName,
                    inputValue: this.__localName,
                    currentIndex: this.__currentIndex,
                    apiItem: this.__currentApiItem,
                    changeIndex: this.__changeIndex,
                });
                jsDialog.setController(this.inputDialog);
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
        if (params.getProfileConnSateDialog !== undefined) {
            this.getProfileConnSateDialog = params.getProfileConnSateDialog;
        }
        if (params.setScanModelDialog !== undefined) {
            this.setScanModelDialog = params.setScanModelDialog;
        }
        if (params.inputDialog !== undefined) {
            this.inputDialog = params.inputDialog;
        }
    }
    aboutToBeDeleted() {
        this.__currentApiItem.aboutToBeDeleted();
        this.__apiItems.aboutToBeDeleted();
        this.__changeIndex.aboutToBeDeleted();
        this.__localName.aboutToBeDeleted();
        this.__currentIndex.aboutToBeDeleted();
        this.__pairedMac.aboutToBeDeleted();
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
    get pairedMac() {
        return this.__pairedMac.get();
    }
    set pairedMac(newValue) {
        this.__pairedMac.set(newValue);
    }
    onCancel() {
        console.info('取消');
    }
    onAccept(text) {
        console.info('input text:', text);
    }
    IngredientItem(apiItem, index) {
        Stack.create();
        Stack.debugLine("../Component/contentTable.ets(295:5)");
        Stack.height(80);
        Stack.borderRadius(10);
        Stack.margin({ top: 5, bottom: 5 });
        Stack.backgroundColor({ "id": 16777312, "type": 10001, params: [] });
        Stack.onClick(() => {
            switch (apiItem.method) {
                case "setLocalName":
                    {
                        this.currentApiItem = apiItem;
                        this.currentIndex += 1;
                        this.inputDialog.open();
                    }
                    break;
                case "setBluetoothScanMode":
                    {
                        this.currentApiItem = apiItem;
                        this.currentIndex += 1;
                        this.setScanModelDialog.open();
                    }
                    break;
                case "pairDevice":
                    {
                        let message = apiItem.api(this.pairedMac);
                        apiItem.result = message;
                        this.currentIndex += 1;
                        this.changeIndex = this.currentIndex;
                        _system_prompt_1.showToast({ message: message });
                    }
                    break;
                case "unpairDevice":
                    {
                        let message = apiItem.api(this.pairedMac);
                        apiItem.result = message;
                        this.currentIndex += 1;
                        this.changeIndex = this.currentIndex;
                        _system_prompt_1.showToast({ message: message });
                    }
                    break;
                case "getDeviceName":
                    {
                        let message = apiItem.api(this.pairedMac);
                        apiItem.result = message;
                        this.currentIndex += 1;
                        this.changeIndex = this.currentIndex;
                        _system_prompt_1.showToast({ message: message });
                    }
                    break;
                case "getDeviceType":
                    {
                        let message = apiItem.api(this.pairedMac);
                        apiItem.result = message;
                        this.currentIndex += 1;
                        this.changeIndex = this.currentIndex;
                        _system_prompt_1.showToast({ message: message });
                    }
                    break;
                case "getProfileConnState":
                    {
                        this.currentApiItem = apiItem;
                        this.currentIndex += 1;
                        this.getProfileConnSateDialog.open();
                    }
                    break;
                case "getDeviceState":
                    {
                        this.currentApiItem = apiItem;
                        this.currentIndex += 1;
                        this.getProfileConnSateDialog.open();
                    }
                    break;
                default:
                    {
                        let message = apiItem.api();
                        apiItem.result = message;
                        this.currentIndex += 1;
                        this.changeIndex = this.currentIndex;
                        _system_prompt_1.showToast({ message: message });
                    }
                    break;
            }
        });
        Flex.create();
        Flex.debugLine("../Component/contentTable.ets(296:7)");
        Flex.create({ alignItems: ItemAlign.Center });
        Flex.debugLine("../Component/contentTable.ets(297:9)");
        Flex.layoutWeight(1);
        Flex.padding({ left: 10 });
        Image.create({ "id": 16777548, "type": 20000, params: [] });
        Image.debugLine("../Component/contentTable.ets(298:11)");
        Image.objectFit(ImageFit.Contain);
        Image.autoResize(false);
        Image.height(30);
        Image.width(30);
        Flex.pop();
        Flex.create({ direction: FlexDirection.Column, alignItems: ItemAlign.Start });
        Flex.debugLine("../Component/contentTable.ets(306:9)");
        Flex.layoutWeight(6);
        Flex.width("80%");
        Text.create(apiItem.name);
        Text.debugLine("../Component/contentTable.ets(307:11)");
        Text.fontSize(17.4);
        Text.fontWeight(FontWeight.Bold);
        Text.layoutWeight(1);
        Text.padding({ left: 10, top: 15 });
        Text.pop();
        Text.create(apiItem.method);
        Text.debugLine("../Component/contentTable.ets(312:11)");
        Text.fontSize(17.4);
        Text.fontWeight(FontWeight.Bold);
        Text.layoutWeight(1);
        Text.padding({ left: 10, top: 5 });
        Text.pop();
        Flex.pop();
        Flex.create({ alignItems: ItemAlign.Center });
        Flex.debugLine("../Component/contentTable.ets(320:9)");
        Flex.layoutWeight(3);
        Flex.padding({ left: 25 });
        Text.create(this.changeIndex >= 0 ? this.getCurrentState(index) : apiItem.result);
        Text.debugLine("../Component/contentTable.ets(321:11)");
        Text.fontSize(17.4);
        Text.flexGrow(1);
        Text.pop();
        Flex.pop();
        Flex.pop();
        Stack.pop();
    }
    getCurrentState(index) {
        return this.apiItems[index].result;
    }
    render() {
        Column.create();
        Column.debugLine("../Component/contentTable.ets(418:5)");
        Column.height("66%");
        Column.padding({ top: 5, right: 10, left: 10 });
        Scroll.create();
        Scroll.debugLine("../Component/contentTable.ets(419:7)");
        Scroll.scrollBarWidth(20);
        Flex.create({
            direction: FlexDirection.Column,
            justifyContent: FlexAlign.SpaceBetween,
            alignItems: ItemAlign.Start
        });
        Flex.debugLine("../Component/contentTable.ets(420:9)");
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

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Component/pageTitle.ets":
/*!********************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Component/pageTitle.ets ***!
  \********************************************************************************************************************/
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
__webpack_require__(/*! ../MainAbility/model/testData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/testData.ets");
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
        Flex.debugLine("../Component/pageTitle.ets(26:5)");
        Flex.height(61);
        Flex.backgroundColor('#FFedf2f5');
        Flex.padding({ top: 23, bottom: 15, left: 18 });
        Flex.borderColor({ "id": 16777287, "type": 10001, params: [] });
        Flex.backgroundColor({ "id": 16777288, "type": 10001, params: [] });
        Flex.onClick(() => {
            _system_router_1.back();
        });
        Image.create({ "id": 16777534, "type": 20000, params: [] });
        Image.debugLine("../Component/pageTitle.ets(27:7)");
        Image.width("10%");
        Image.height("100%");
        Text.create(this.testItem != null ? this.testItem.detail : this.detail);
        Text.debugLine("../Component/pageTitle.ets(30:7)");
        Text.fontSize(21.8);
        Text.margin({ left: 17.4 });
        Text.pop();
        Flex.pop();
    }
}
exports.PageTitle = PageTitle;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Component/testImageDisplay.ets":
/*!***************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Component/testImageDisplay.ets ***!
  \***************************************************************************************************************************/
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
__webpack_require__(/*! ../MainAbility/model/testData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/testData.ets");
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
        Stack.debugLine("../Component/testImageDisplay.ets(24:5)");
        Stack.backgroundColor('#FFedf2f5');
        Stack.height(160);
        Flex.create({ alignItems: ItemAlign.Start });
        Flex.debugLine("../Component/testImageDisplay.ets(25:7)");
        Stack.create();
        Stack.debugLine("../Component/testImageDisplay.ets(26:9)");
        Stack.width("20%");
        Image.create(this.testItem.image);
        Image.debugLine("../Component/testImageDisplay.ets(27:11)");
        Image.height(80);
        Image.width(80);
        Image.margin({ left: 20, bottom: 10 });
        Stack.pop();
        Column.create();
        Column.debugLine("../Component/testImageDisplay.ets(33:9)");
        Column.width("80%");
        Column.padding({ left: 20 });
        Text.create(this.testItem.name);
        Text.debugLine("../Component/testImageDisplay.ets(34:11)");
        Text.fontSize(26);
        Text.fontWeight(500);
        Text.margin({ left: 26, top: 10 });
        Text.width("90%");
        Text.pop();
        Text.create(this.testItem.detail);
        Text.debugLine("../Component/testImageDisplay.ets(40:11)");
        Text.fontSize(17);
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

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Component/testListItems.ets":
/*!************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Component/testListItems.ets ***!
  \************************************************************************************************************************/
/***/ ((__unused_webpack_module, exports, __webpack_require__) => {

"use strict";

Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.TestListItem = void 0;
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
/**
 * Test-ListItem
 */
__webpack_require__(/*! ../MainAbility/model/testData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/testData.ets");
class TestListItem extends View {
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
        Navigator.create({ target: this.testItem.url });
        Navigator.debugLine("../Component/testListItems.ets(26:5)");
        Navigator.params({ testId: this.testItem });
        Navigator.margin({ right: 10, left: 10, top: 10 });
        Navigator.padding({ left: 15, right: 15 });
        Navigator.borderRadius(10);
        Navigator.backgroundColor({ "id": 16777312, "type": 10001, params: [] });
        Flex.create({ justifyContent: FlexAlign.Start, alignItems: ItemAlign.Center });
        Flex.debugLine("../Component/testListItems.ets(27:7)");
        Flex.height(85);
        Row.create();
        Row.debugLine("../Component/testListItems.ets(28:9)");
        Row.backgroundColor({ "id": 16777312, "type": 10001, params: [] });
        Row.width("30%");
        Image.create(this.testItem.image);
        Image.debugLine("../Component/testListItems.ets(29:11)");
        Image.objectFit(ImageFit.Contain);
        Image.autoResize(false);
        Image.height(60);
        Image.width(60);
        Row.pop();
        Text.create(this.testItem.name);
        Text.debugLine("../Component/testListItems.ets(38:9)");
        Text.fontSize(20);
        Text.flexGrow(1);
        Text.margin({ left: 12 });
        Text.width("45%");
        Text.pop();
        Text.create(this.testItem.detail);
        Text.debugLine("../Component/testListItems.ets(43:9)");
        Text.fontSize(17);
        Text.margin({ left: 10, right: 5 });
        Text.width("45%");
        Text.pop();
        Flex.pop();
        Navigator.pop();
    }
}
exports.TestListItem = TestListItem;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Component/titleComponent.ets":
/*!*************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Component/titleComponent.ets ***!
  \*************************************************************************************************************************/
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
const ConfigData_1 = __importDefault(__webpack_require__(/*! ../Utils/ConfigData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Utils/ConfigData.ts"));
class TitleComponent extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.title = undefined;
        this.fontSize = '30vp';
        this.stateChangeFlag = false;
        this.pinRequiredFlag = false;
        this.bondStateChangeFlag = false;
        this.__bgColor = new ObservedPropertyObject({ "id": 16777298, "type": 10001, params: [] }, this, "bgColor");
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
        Column.debugLine("../Component/titleComponent.ets(35:5)");
        Column.padding({ "id": 16777377, "type": 10002, params: [] });
        Column.height("100vp");
        Column.borderRadius({ "id": 16777426, "type": 10002, params: [] });
        Column.backgroundColor({ "id": 125829123, "type": 10001, params: [] });
        Flex.create({ justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center });
        Flex.debugLine("../Component/titleComponent.ets(36:7)");
        Flex.height(80);
        Flex.width(ConfigData_1.default.WH_100_100);
        Flex.backgroundColor(ObservedObject.GetRawObject(this.bgColor));
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
        Text.create(this.stateChangeFlag ? this.state : (this.pinRequiredFlag ? this.pin : (this.bondStateChangeFlag ? this.bondState : this.title)));
        Text.debugLine("../Component/titleComponent.ets(37:9)");
        Text.textAlign(TextAlign.Center);
        Text.width("100%");
        Text.height(70);
        Text.fontSize(this.fontSize);
        Text.fontColor({ "id": 16777299, "type": 10001, params: [] });
        Text.fontWeight(FontWeight.Medium);
        Text.pop();
        Flex.pop();
        Column.pop();
    }
}
exports.TitleComponent = TitleComponent;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/controller/BluetoothDeviceController.ts":
/*!************************************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/controller/BluetoothDeviceController.ts ***!
  \************************************************************************************************************************************************/
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
const bluetoothModel_1 = __importStar(__webpack_require__(/*! ../model/bluetoothModel */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/bluetoothModel.ts"));
const ConfigData_1 = __importDefault(__webpack_require__(/*! ../../Utils/ConfigData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Utils/ConfigData.ts"));
const LogUtil_1 = __importDefault(__webpack_require__(/*! ../../Utils/LogUtil */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Utils/LogUtil.ts"));
const bluetoothDevice_1 = __importDefault(__webpack_require__(/*! ../model/bluetoothDevice */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/bluetoothDevice.ts"));
const BaseSettingsController_1 = __importDefault(__webpack_require__(/*! ../../component/controller/BaseSettingsController */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/component/controller/BaseSettingsController.ts"));
__webpack_require__(/*! ../../component/controller/ISettingsController */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/component/controller/ISettingsController.ts");
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
        let isOn = bluetoothModel_1.default.isStateOn();
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
        bluetoothModel_1.default.unsubscribeStateChange();
        bluetoothModel_1.default.unsubscribeBluetoothDeviceFind();
        bluetoothModel_1.default.unsubscribeBondStateChange();
        bluetoothModel_1.default.unsubscribeDeviceStateChange();
        return this;
    }
    subscribePinRequired(success) {
        // start listening pinCode
        bluetoothModel_1.default.subscribePinRequired((pinRequiredParam) => {
            LogUtil_1.default.log(this.TAG + 'bluetooth subscribePinRequired callback. pinRequiredParam = ' + JSON.stringify(pinRequiredParam));
            this.pairPinCode = pinRequiredParam.pinCode;
            if (success) {
                success(pinRequiredParam.deviceId, pinRequiredParam.pinCode);
            }
        });
    }
    /**
     * Set toggle value
     */
    toggleValue(isOn) {
        this.enabled = false;
        AppStorage.SetOrCreate('bluetoothToggleEnabled', this.enabled);
        LogUtil_1.default.log(this.TAG + 'afterCurrentValueChanged bluetooth state isOn = ' + this.isOn);
        if (isOn) {
            bluetoothModel_1.default.enableBluetooth();
        }
        else {
            bluetoothModel_1.default.disableBluetooth();
        }
    }
    /**
     * Get Local Name
     */
    getLocalName() {
        AppStorage.SetOrCreate('bluetoothLocalName', bluetoothModel_1.default.getLocalName());
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
        if (device && device.connectionState === bluetoothModel_1.DeviceState.STATE_PAIRING) {
            LogUtil_1.default.log(this.TAG + `bluetooth no Aavailable device or device(${deviceId}) is already pairing.`);
            return;
        }
        // start pairing
        const result = bluetoothModel_1.default.pairDevice(deviceId);
        LogUtil_1.default.log(this.TAG + 'bluetooth pairDevice result = ' + result);
        if (!result) {
            AppStorage.SetOrCreate('pairedDeviceId', '');
            bluetoothModel_1.default.unsubscribePinRequired(() => LogUtil_1.default.log(this.TAG + 'available pinRequired unsubscribed.'));
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
            this.getAvailableDevice(deviceId).connectionState = bluetoothModel_1.DeviceState.STATE_PAIRING;
            this.forceRefresh(this.availableDevices);
            AppStorage.SetOrCreate('bluetoothAvailableDevices', this.availableDevices);
        }
        // set paring confirmation
        bluetoothModel_1.default.setDevicePairingConfirmation(deviceId, accept);
    }
    /**
     * Connect device.
     * @param deviceId device id
     */
    connect(deviceId) {
        return bluetoothModel_1.default.connectDevice(deviceId);
    }
    /**
     * disconnect device.
     * @param deviceId device id
     */
    disconnect(deviceId) {
        return bluetoothModel_1.default.disconnectDevice(deviceId);
    }
    /**
     * Unpair device.
     * @param deviceId device id
     */
    unpair(deviceId) {
        const result = bluetoothModel_1.default.unpairDevice(deviceId);
        LogUtil_1.default.log(this.TAG + 'bluetooth paired device unpair. result = ' + result);
        this.refreshPairedDevices();
        return result;
    }
    /**
     * Refresh paired devices.
     */
    refreshPairedDevices() {
        let deviceIds = bluetoothModel_1.default.getPairedDeviceIds();
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
        console.info("subscribeStateChange");
        bluetoothModel_1.default.subscribeStateChange((isOn) => {
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
        console.info("subscribeBluetoothDeviceFind");
        bluetoothModel_1.default.subscribeBluetoothDeviceFind((deviceIds) => {
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
        console.info("subscribeBondStateChange");
        bluetoothModel_1.default.subscribeBondStateChange((data) => {
            LogUtil_1.default.log(this.TAG + 'bluetooth subscribeBondStateChange callback.');
            //paired devices
            if (data.bondState !== bluetoothModel_1.BondState.BOND_STATE_BONDING) {
                this.refreshPairedDevices();
            }
            //available devices
            if (data.bondState == bluetoothModel_1.BondState.BOND_STATE_BONDING) {
                // case bonding
                // do nothing and still listening
                LogUtil_1.default.log(this.TAG + 'bluetooth continue listening bondStateChange.');
            }
            else if (data.bondState == bluetoothModel_1.BondState.BOND_STATE_INVALID) {
                // case failed
                this.getAvailableDevice(data.deviceId).connectionState = bluetoothModel_1.DeviceState.STATE_DISCONNECTED;
                this.forceRefresh(this.availableDevices);
                AppStorage.SetOrCreate('bluetoothAvailableDevices', this.availableDevices);
                this.showConnectFailedDialog();
            }
            else if (data.bondState == bluetoothModel_1.BondState.BOND_STATE_BONDED) {
                // case success
                LogUtil_1.default.log(this.TAG + 'bluetooth bonded : remove device.');
                this.removeAvailableDevice(data.deviceId);
                bluetoothModel_1.default.connectDevice(data.deviceId);
            }
        });
    }
    /**
     * Subscribe device connect state change
     */
    subscribeDeviceConnectStateChange() {
        console.info("subscribeDeviceConnectStateChange");
        bluetoothModel_1.default.subscribeDeviceStateChange((data) => {
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
        let device = new bluetoothDevice_1.default();
        device.deviceId = deviceId;
        device.deviceName = bluetoothModel_1.default.getDeviceName(deviceId);
        device.deviceType = bluetoothModel_1.default.getDeviceType(deviceId);
        device.setProfiles(bluetoothModel_1.default.getDeviceState(deviceId));
        console.info("getDevice");
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
        arr.push(new bluetoothDevice_1.default());
        arr.pop();
    }
    /**
     * Start bluetooth discovery.
     */
    startBluetoothDiscovery() {
        this.isDeviceDiscovering = true;
        bluetoothModel_1.default.startBluetoothDiscovery();
        console.info("startBluetoothDiscovery");
        this.discoveryStopTimeoutId = setTimeout(() => {
            this.stopBluetoothDiscovery();
        }, DISCOVERY_DURING_TIME);
    }
    /**
     * Stop bluetooth discovery.
     */
    stopBluetoothDiscovery() {
        console.info("stopBluetoothDiscovery");
        this.isDeviceDiscovering = false;
        bluetoothModel_1.default.stopBluetoothDiscovery();
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
        console.info("getAvailableDevice:", this.availableDevices.length);
        for (let i = 0; i < this.availableDevices.length; i++) {
            if (this.availableDevices[i].deviceId === deviceId) {
                return this.availableDevices[i];
            }
        }
        return null;
        //    this.availableDevices.forEach(device => {
        //      console.info("device.deviceId: "+device.deviceId)
        //      console.info("input.deviceId:  "+deviceId)
        //      console.info("input.localeCompare:  " + device.deviceId.localeCompare(deviceId).toString())
        //      if (device.deviceId.localeCompare(deviceId) == 0) {
        //        return device;
        //      }
        //    })
    }
    /**
     * Remove available device.
     *
     * @param deviceId device id
     */
    removeAvailableDevice(deviceId) {
        console.info("removeAvailableDevice");
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
            title: { "id": 16777231, "type": 10003, params: [] },
            message: { "id": 16777232, "type": 10003, params: [] },
            confirm: {
                value: { "id": 16777236, "type": 10003, params: [] },
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

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/a2dpSinkProfileInterface.ets":
/*!*******************************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/a2dpSinkProfileInterface.ets ***!
  \*******************************************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.testProfileGetDeviceState = exports.testProfileGetConnectionDevices = exports.testProfileGetPlayingState = exports.testProfileDisConnect = exports.testProfileConnect = void 0;
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
/**
 * Profile A2DP Test Page Of Bluetooth test
 */
var _system_prompt_1  = isSystemplugin('prompt', 'system') ? globalThis.systemplugin.prompt : globalThis.requireNapi('prompt');
const btProfileModel_1 = __importDefault(__webpack_require__(/*! ./btProfileModel */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/btProfileModel.ets"));
function testProfileConnect() {
    let message = "ProfileConnect";
    let textMessage = "";
    let btConnectionState = AppStorage.Get("btConnectionState");
    let deviceId = AppStorage.Get("deviceId");
    let profileType = AppStorage.Get("profileType");
    if (btConnectionState == 1 || btConnectionState == 2) {
        _system_prompt_1.showToast({ message: btConnectionState == 1 ? "该设备正在连接中" : "该设备已连接" });
        return message;
    }
    let ret = btProfileModel_1.default.getProfile(profileType).connect(deviceId);
    if (ret) {
        btConnectionState = 1;
    }
    textMessage = "connect ret：" + ret;
    message = "connect ret：" + ret;
    AppStorage.SetOrCreate("textMessage", textMessage);
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testProfileConnect = testProfileConnect;
function testProfileDisConnect() {
    let message = "ProfileDisConnect";
    let textMessage = "";
    let btConnectionState = AppStorage.Get("btConnectionState");
    let deviceId = AppStorage.Get("deviceId");
    let profileType = AppStorage.Get("profileType");
    if (btConnectionState == 0 || btConnectionState == 3) {
        _system_prompt_1.showToast({ message: btConnectionState == 0 ? "该设备已断开" : "该设备正在断开连接" });
        return message;
    }
    let ret = btProfileModel_1.default.getProfile(profileType).disconnect(deviceId);
    if (ret) {
        btConnectionState = 3;
    }
    textMessage = "disconnect ret：" + ret;
    AppStorage.SetOrCreate("textMessage", textMessage);
    message = "disconnect ret：" + ret;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testProfileDisConnect = testProfileDisConnect;
function testProfileGetPlayingState() {
    let message = "";
    let textMessage = "";
    let deviceId = AppStorage.Get("deviceId");
    let profileType = AppStorage.Get("profileType");
    _system_prompt_1.showToast({ message: profileType });
    console.info("profileType:" + profileType);
    let profile = btProfileModel_1.default.getProfile(parseInt(profileType));
    let ret = -1;
    if (profile) {
        ret = profile.getPlayingState(deviceId);
    }
    textMessage = "getPlayingState ret: " + ret;
    message = "getPlayingState ret: " + ret;
    AppStorage.SetOrCreate("textMessage", textMessage);
    _system_prompt_1.showToast({ message: message });
    return message;
    //visibility(this.pageType == 0 ? Visibility.Visible : Visibility.None)
}
exports.testProfileGetPlayingState = testProfileGetPlayingState;
function testProfileGetConnectionDevices() {
    let message = "";
    let textMessage = "";
    let profileType = AppStorage.Get("profileType");
    let ret = btProfileModel_1.default.getProfile(profileType).getConnectionDevices();
    textMessage = "getConnectionDevices ret: " + ret;
    message = "getConnectionDevices ret: " + ret;
    AppStorage.SetOrCreate("textMessage", textMessage);
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testProfileGetConnectionDevices = testProfileGetConnectionDevices;
function testProfileGetDeviceState() {
    let message = "";
    let textMessage = "";
    let deviceId = AppStorage.Get("deviceId");
    let profileType = AppStorage.Get("profileType");
    let ret = btProfileModel_1.default.getProfile(profileType).getDeviceState(deviceId);
    textMessage = "getDeviceState ret: " + ret;
    message = "getDeviceState ret: " + ret;
    AppStorage.SetOrCreate("textMessage", textMessage);
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testProfileGetDeviceState = testProfileGetDeviceState;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/autoTestDataModels.ets":
/*!*************************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/autoTestDataModels.ets ***!
  \*************************************************************************************************************************************/
/***/ ((__unused_webpack_module, exports, __webpack_require__) => {

"use strict";

Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.initPanNetworkProfileAutoData = exports.initHitHostProfileAutoData = exports.initHandsFreeAudioGatewayProfileAutoData = exports.initA2dpSourceProfileAutoData = exports.initSppServerAutoData = exports.initSppClientAutoData = exports.initGattServerAutoData = exports.initGattClientAutoData = exports.initBleAutoData = exports.initBrAutoData = void 0;
const testData_1 = __webpack_require__(/*! ./testData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/testData.ets");
const BrAutoTest = [
    { 'name': '开蓝牙', 'detail': '使能蓝牙', 'method': 'enableBluetooth', 'result': 'None', 'category': testData_1.AutoCategory.BrAuto },
    { 'name': '关蓝牙', 'detail': '去使能蓝牙', 'method': 'disableBluetooth', 'result': 'None', 'category': testData_1.AutoCategory.BrAuto },
    { 'name': '设置状态', 'detail': '设置蓝牙状态', 'method': 'getState', 'result': 'None', 'category': testData_1.AutoCategory.BrAuto },
    { 'name': '获取当前连接状态', 'detail': '获取当前蓝牙连接状态', 'method': 'getBtConnectionState', 'result': 'None', 'category': testData_1.AutoCategory.BrAuto },
    { 'name': '设置本地名称', 'detail': '设置蓝牙本地名称', 'method': 'setLocalName', 'result': 'None', 'category': testData_1.AutoCategory.BrAuto },
    { 'name': '获取本地名称', 'detail': '获取蓝牙本地名称', 'method': 'getLocalName', 'result': 'None', 'category': testData_1.AutoCategory.BrAuto },
    { 'name': '设置扫描模式', 'detail': '设置蓝牙扫描模式', 'method': 'setBluetoothScanMode', 'result': 'None', 'category': testData_1.AutoCategory.BrAuto },
    { 'name': '获取扫描模式', 'detail': '获取蓝牙扫描模式', 'method': 'getBluetoothScanMode', 'result': 'None', 'category': testData_1.AutoCategory.BrAuto },
    { 'name': '开始蓝牙发现', 'detail': '发现可配对蓝牙设备', 'method': 'startBluetoothDiscovery', 'result': 'None', 'category': testData_1.AutoCategory.BrAuto },
    { 'name': '停止蓝牙发现', 'detail': '停止发现', 'method': 'stopBluetoothDiscovery', 'result': 'None', 'category': testData_1.AutoCategory.BrAuto },
    { 'name': '配对设备', 'detail': '进行蓝牙设备配对', 'method': 'pairDevice', 'result': 'None', 'category': testData_1.AutoCategory.BrAuto },
    { 'name': '取消配对设备', 'detail': '取消蓝牙设备配对', 'method': 'unpairDevice', 'result': 'None', 'category': testData_1.AutoCategory.BrAuto },
    { 'name': '获取远程设备名称', 'detail': '获取配对蓝牙设备的名称', 'method': 'getDeviceName', 'result': 'None', 'category': testData_1.AutoCategory.BrAuto },
    { 'name': '获取远程设备类型', 'detail': '获取配对蓝牙设备的类型', 'method': 'getDeviceType', 'result': 'None', 'category': testData_1.AutoCategory.BrAuto },
    { 'name': '获取配对设备', 'detail': '获取配对蓝牙设备的信息', 'method': 'getPairedDeviceIds', 'result': 'None', 'category': testData_1.AutoCategory.BrAuto },
    { 'name': '获取配置文件连接状态1', 'detail': '获取配置文件的连接状态', 'method': 'getProfileConnState', 'result': 'None', 'category': testData_1.AutoCategory.BrAuto },
    { 'name': '获取配置文件连接状态2', 'detail': '获取设备的配置文件状态', 'method': 'getDeviceState', 'result': 'None', 'category': testData_1.AutoCategory.BrAuto },
    { 'name': '注册状态变化', 'detail': '注册状态变化', 'method': 'on.stateChange', 'result': 'None', 'category': testData_1.AutoCategory.BrAuto },
    { 'name': '注册pin码变化', 'detail': '注册pin码变化', 'method': 'on.pinRequired', 'result': 'None', 'category': testData_1.AutoCategory.BrAuto },
    { 'name': '注册绑定状态变化', 'detail': '注册绑定状态变化', 'method': 'on.bondStateChange', 'result': 'None', 'category': testData_1.AutoCategory.BrAuto }
];
const BleAutoTest = [
    { 'name': '创建Gatt服务器', 'detail': '创建Gatt服务器', 'method': 'createGattServer', 'result': 'None', 'category': testData_1.AutoCategory.BleAuto },
    { 'name': '创建Gatt客户端', 'detail': '创建Gatt客户端', 'method': 'createGattClient', 'result': 'None', 'category': testData_1.AutoCategory.BleAuto },
    { 'name': '获取连接的Ble设备', 'detail': '获取连接的Ble设备信息', 'method': 'getConnectedBleDevices', 'result': 'None', 'category': testData_1.AutoCategory.BleAuto },
    { 'name': '开始Ble扫描', 'detail': '开始扫描Ble设备', 'method': 'startBleScan', 'result': 'None', 'category': testData_1.AutoCategory.BleAuto },
    { 'name': '停止Ble扫描', 'detail': '停止扫描', 'method': 'stopBleScan', 'result': 'None', 'category': testData_1.AutoCategory.BleAuto },
    { 'name': '注册Ble查找状态变化', 'detail': '注册Ble设备查找状态变化', 'method': 'on.BleDeviceFind', 'result': 'None', 'category': testData_1.AutoCategory.BleAuto }
];
//const GattTestCase: any[]  = [
//  { 'name': 'GattClient测试', 'detail': 'Gatt客户端测试', 'method': 'gattClient', 'result': 'None', 'category': AutoCategory.Gatt },
//  { 'name': 'GattServer测试', 'detail': 'Gatt服务器测试', 'method': 'gattServer', 'result': 'None', 'category': AutoCategory.Gatt },
//]
//const SppTestCase: any[]  = [
//  { 'name': 'SppClient测试', 'detail': 'Spp客户端测试', 'method': 'sppClient', 'result': 'None', 'category': AutoCategory.SPP },
//  { 'name': 'SppServer测试', 'detail': 'Spp服务器测试', 'method': 'sppServer', 'result': 'None', 'category': AutoCategory.SPP },
//]
//const ProfileTestCase: any[]  = [
//  { 'name': 'A2dpSink配置', 'detail': 'A2dpSink配置测试', 'method': 'A2dpSinkProfile', 'result': 'None', 'category': AutoCategory.Profile },
//  { 'name': 'A2dpSource配置', 'detail': 'A2dpSource配置测试', 'method': 'A2dpSourceProfile', 'result': 'None', 'category': AutoCategory.Profile },
//  { 'name': 'AvrcpCT配置', 'detail': 'AvrcpCT配置测试', 'method': 'AvrcpProfile',  'result': 'None', 'category': AutoCategory.Profile },
//  { 'name': 'AvrcpTG配置', 'detail': 'AvrcpTG配置测试', 'method': 'AvrcpProfile',  'result': 'None', 'category': AutoCategory.Profile },
//  { 'name': 'HandsFreeAudioGateway配置', 'detail': 'HandsFreeAudioGateway配置测试', 'method': 'HandsFreeAudioGatewayProfile',  'result': 'None', 'category': AutoCategory.Profile },
//  { 'name': 'HandsFreeUnit配置', 'detail': 'HandsFreeUnit配置测试', 'method': 'HandsFreeUnitProfile', 'result': 'None', 'category': AutoCategory.Profile },
//  { 'name': 'HidHost配置', 'detail': 'HidHost配置测试', 'method': 'HidHostProfile', 'result': 'None', 'category': AutoCategory.Profile },
//  { 'name': 'PanNetwork配置', 'detail': 'PanNetwork配置测试', 'method': 'PanNetworkProfile', 'result': 'None', 'category': AutoCategory.Profile },
//  { 'name': 'PbapClient配置', 'detail': 'PbapClient配置测试', 'method': 'PbapClientProfile',  'result': 'None', 'category': AutoCategory.Profile },
//  { 'name': 'PbapServer配置', 'detail': 'PbapServer配置测试', 'method': 'PbapServerProfile', 'result': 'None', 'category': AutoCategory.Profile },
//]
const GattClientAutoTest = [
    { 'name': '创造Gatt客户端', 'detail': '创造Gatt客户端', 'method': 'createGattClient', 'result': 'None', 'category': testData_1.AutoCategory.GattClientAuto },
    { 'name': '连接', 'detail': 'Gatt连接', 'method': 'connect', 'result': 'None', 'category': testData_1.AutoCategory.GattClientAuto },
    { 'name': '取消连接', 'detail': '取消Gatt连接', 'method': 'disconnect', 'result': 'None', 'category': testData_1.AutoCategory.GattClientAuto },
    { 'name': 'Ble连接状态', 'detail': '查看Ble连接状态', 'method': 'on.BleConnectionStateChange', 'result': 'None', 'category': testData_1.AutoCategory.GattClientAuto },
    { 'name': '关闭', 'detail': '关闭Gatt客户端', 'method': 'close', 'result': 'None', 'category': testData_1.AutoCategory.GattClientAuto },
    { 'name': '获取服务callback', 'detail': '获取服务callback', 'method': 'getServices(Callback)', 'result': 'None', 'category': testData_1.AutoCategory.GattClientAuto },
    { 'name': '获取服务promise', 'detail': '获取服务promise', 'method': 'getServices(promise)', 'result': 'None', 'category': testData_1.AutoCategory.GattClientAuto },
    { 'name': '读取特征值callback', 'detail': '读取特征值callback', 'method': 'readCharacteristicValue(callback)', 'result': 'None', 'category': testData_1.AutoCategory.GattClientAuto },
    { 'name': '读取特征值promise', 'detail': '读取特征值promise', 'method': 'readCharacteristicValue(promise)', 'result': 'None', 'category': testData_1.AutoCategory.GattClientAuto },
    { 'name': '读取描述符值callback', 'detail': '读取描述符值callback', 'method': 'readDescriptorValue(callback)', 'result': 'None', 'category': testData_1.AutoCategory.GattClientAuto },
    { 'name': '读取描述符值promise', 'detail': '读取描述符值promise', 'method': 'readDescriptorValue(promise)', 'result': 'None', 'category': testData_1.AutoCategory.GattClientAuto },
    { 'name': '写入特征值', 'detail': '写入特征值', 'method': 'writeCharacteristicValue', 'result': 'None', 'category': testData_1.AutoCategory.GattClientAuto },
    { 'name': '写入描述符值', 'detail': '写入描述符值', 'method': 'writeDescriptorValue', 'result': 'None', 'category': testData_1.AutoCategory.GattClientAuto },
    { 'name': 'Ble特征值变化', 'detail': '查看Ble特征值变化', 'method': 'on.BleCharacteristicChange', 'result': 'None', 'category': testData_1.AutoCategory.GattClientAuto },
    { 'name': '设定BleMtu尺寸', 'detail': '设定BleMtu尺寸', 'method': 'setBleMtuSize', 'result': 'None', 'category': testData_1.AutoCategory.GattClientAuto },
    { 'name': '设置通知特性更改', 'detail': '设置通知特性更改', 'method': 'setNotifyCharacteristicChanged', 'result': 'None', 'category': testData_1.AutoCategory.GattClientAuto },
    { 'name': '获取设备名称callback', 'detail': '获取设备名称callback', 'method': 'getDeviceName()callback', 'result': 'None', 'category': testData_1.AutoCategory.GattClientAuto },
    { 'name': '获取设备名称promise', 'detail': '获取设备名称promise', 'method': 'getDeviceName()Promise', 'result': 'None', 'category': testData_1.AutoCategory.GattClientAuto },
    { 'name': '获取Rssi值callback', 'detail': '获取Rssi值callback', 'method': 'getRssiValue()Callback', 'result': 'None', 'category': testData_1.AutoCategory.GattClientAuto },
    { 'name': '获取Rssi值promise', 'detail': '获取Rssi值promise', 'method': 'getRssiValue()Promise', 'result': 'None', 'category': testData_1.AutoCategory.GattClientAuto },
];
const GattServerAutoTest = [
    { 'name': '创建Gatt服务器', 'detail': '创建Gatt服务器', 'method': 'createGattServer', 'result': 'None', 'category': testData_1.AutoCategory.GattServerAuto },
    { 'name': '添加服务', 'detail': '添加Gatt服务', 'method': 'addService', 'result': 'None', 'category': testData_1.AutoCategory.GattServerAuto },
    { 'name': '删除服务', 'detail': '删除Gatt服务', 'method': 'removeService', 'result': 'None', 'category': testData_1.AutoCategory.GattServerAuto },
    { 'name': '开始广播', 'detail': '开始广播播报', 'method': 'startAdvertising', 'result': 'None', 'category': testData_1.AutoCategory.GattServerAuto },
    { 'name': '停止广播', 'detail': '停止广播播报', 'method': 'stopAdvertising', 'result': 'None', 'category': testData_1.AutoCategory.GattServerAuto },
    { 'name': '关闭', 'detail': '关闭Gatt服务器', 'method': 'close', 'result': 'None', 'category': testData_1.AutoCategory.GattServerAuto },
    { 'name': '连接状态', 'detail': '查看Gatt连接状态', 'method': 'on.connectStateChange', 'result': 'None', 'category': testData_1.AutoCategory.GattServerAuto },
    { 'name': '读取特征值', 'detail': '读取特征值', 'method': 'on.characteristicRead', 'result': 'None', 'category': testData_1.AutoCategory.GattServerAuto },
    { 'name': '写入特征值', 'detail': '写入特征值', 'method': 'on.characteristicWrite', 'result': 'None', 'category': testData_1.AutoCategory.GattServerAuto },
    { 'name': '读取描述符', 'detail': '读取描述符', 'method': 'on.descriptorRead', 'result': 'None', 'category': testData_1.AutoCategory.GattServerAuto },
    { 'name': '写入描述符', 'detail': '写入描述符', 'method': 'on.descriptorWrite', 'result': 'None', 'category': testData_1.AutoCategory.GattServerAuto },
];
const SppClientAutoTest = [
    { 'name': '开蓝牙', 'detail': '使能蓝牙', 'method': 'enableBluetooth', 'result': 'None', 'category': testData_1.AutoCategory.SppClientAuto },
    { 'name': '关蓝牙', 'detail': '去使能蓝牙', 'method': 'disableBluetooth', 'result': 'None', 'category': testData_1.AutoCategory.SppClientAuto },
    { 'name': '设置SPP扫描模式', 'detail': '设置SPP扫描模式', 'method': 'setBluetoothScanMode', 'result': 'None', 'category': testData_1.AutoCategory.SppClientAuto },
    { 'name': '设置状态', 'detail': '设置蓝牙状态', 'method': 'getState', 'result': 'None', 'category': testData_1.AutoCategory.SppClientAuto },
    { 'name': '获取扫描模式', 'detail': '获取蓝牙扫描模式', 'method': 'getBluetoothScanMode', 'result': 'None', 'category': testData_1.AutoCategory.SppClientAuto },
    { 'name': '获取当前连接状态', 'detail': '获取当前蓝牙连接状态', 'method': 'getBtConnectionState', 'result': 'None', 'category': testData_1.AutoCategory.SppClientAuto },
    { 'name': 'Spp客户端连接(安全模式)', 'detail': 'Spp客户端(安全模式)连接', 'method': 'sppConnect', 'result': 'None', 'category': testData_1.AutoCategory.SppClientAuto },
    { 'name': 'Spp客户端连接(非安全模式)', 'detail': 'Spp客户端(非安全模式)连接', 'method': 'sppConnect', 'result': 'None', 'category': testData_1.AutoCategory.SppClientAuto },
    { 'name': '配对设备', 'detail': '进行蓝牙设备配对', 'method': 'pairDevice', 'result': 'None', 'category': testData_1.AutoCategory.SppClientAuto },
    { 'name': 'Spp客户端写入', 'detail': 'Spp客户端写入', 'method': 'sppWrite', 'result': 'None', 'category': testData_1.AutoCategory.SppClientAuto },
    { 'name': 'Spp客户端读取打开', 'detail': 'Spp客户端读取打开', 'method': 'on.sppRead', 'result': 'None', 'category': testData_1.AutoCategory.SppClientAuto },
    { 'name': 'Spp客户端读取关闭', 'detail': 'Spp客户端读取关闭', 'method': 'off.sppRead', 'result': 'None', 'category': testData_1.AutoCategory.SppClientAuto },
    { 'name': 'Spp客户端关闭', 'detail': 'Spp客户端关闭', 'method': 'sppCloseClientSocket', 'result': 'None', 'category': testData_1.AutoCategory.SppClientAuto },
];
const SppServerAutoTest = [
    { 'name': '开蓝牙', 'detail': '使能蓝牙', 'method': 'enableBluetooth', 'result': 'None', 'category': testData_1.AutoCategory.SppServerAuto },
    { 'name': '关蓝牙', 'detail': '去使能蓝牙', 'method': 'disableBluetooth', 'result': 'None', 'category': testData_1.AutoCategory.SppServerAuto },
    { 'name': '设置SPP扫描模式', 'detail': '设置SPP扫描模式', 'method': 'setBluetoothScanMode', 'result': 'None', 'category': testData_1.AutoCategory.SppServerAuto },
    { 'name': '设置状态', 'detail': '设置蓝牙状态', 'method': 'getState', 'result': 'None', 'category': testData_1.AutoCategory.SppServerAuto },
    { 'name': '获取扫描模式', 'detail': '获取蓝牙扫描模式', 'method': 'getBluetoothScanMode', 'result': 'None', 'category': testData_1.AutoCategory.SppServerAuto },
    { 'name': '获取当前连接状态', 'detail': '获取当前蓝牙连接状态', 'method': 'getBtConnectionState', 'result': 'None', 'category': testData_1.AutoCategory.SppServerAuto },
    { 'name': 'Spp服务器创建(安全模式)', 'detail': 'Spp服务器(安全模式)创建', 'method': 'sppListen', 'result': 'None', 'category': testData_1.AutoCategory.SppServerAuto },
    { 'name': 'Spp服务器创建(非安全模式)', 'detail': 'Spp服务器(非安全模式)创建', 'method': 'sppListen', 'result': 'None', 'category': testData_1.AutoCategory.SppServerAuto },
    { 'name': '注册pin码变化', 'detail': '注册pin码变化', 'method': 'on.pinRequired', 'result': 'None', 'category': testData_1.AutoCategory.SppServerAuto },
    //  { 'name': 'SPP客户端写入', 'detail': '使能蓝牙', 'method': 'spp_Client_Write', 'api': testSPP_Client_Write, 'result': 'None', 'category': AutoCategory.SppServerAuto },
    { 'name': 'Spp服务器写入', 'detail': 'Spp服务器写入', 'method': 'sppWrite', 'result': 'None', 'category': testData_1.AutoCategory.SppServerAuto },
    { 'name': 'Spp服务器接受', 'detail': 'Spp服务器接受', 'method': 'sppAccept', 'result': 'None', 'category': testData_1.AutoCategory.SppServerAuto },
    { 'name': 'Spp服务器读取打开', 'detail': 'Spp服务器读取打开', 'method': 'on.sppRead', 'result': 'None', 'category': testData_1.AutoCategory.SppServerAuto },
    { 'name': 'Spp服务器读取关闭', 'detail': 'Spp服务器读取关闭', 'method': 'off.sppRead', 'result': 'None', 'category': testData_1.AutoCategory.SppServerAuto },
    { 'name': 'Spp服务器关闭', 'detail': 'Spp服务器关闭', 'method': 'sppCloseServerSocket', 'result': 'None', 'category': testData_1.AutoCategory.SppServerAuto },
    { 'name': 'Spp客户端关闭', 'detail': 'Spp客户端关闭', 'method': 'sppCloseClientSocket', 'result': 'None', 'category': testData_1.AutoCategory.SppServerAuto },
];
//const A2dpSinkProfileAutoTest: any[]  = [
//  { 'name': '连接', 'detail': 'Profile连接', 'method': 'connect',  'result': 'None', 'category': AutoCategory.A2dpSinkAuto },
//  { 'name': '取消连接', 'detail': '取消Profile连接', 'method': 'disconnect',  'result': 'None', 'category': AutoCategory.A2dpSinkAuto },
//  { 'name': '获取播放状态', 'detail': '获取Profile播放状态', 'method': 'getPlayingState',  'result': 'None', 'category': AutoCategory.A2dpSinkAuto },
//  { 'name': '获取连接设备', 'detail': '获取Profile连接设备', 'method': 'getConnectionDevices', 'result': 'None', 'category': AutoCategory.A2dpSinkAuto },
//  { 'name': '获取设备状态', 'detail': '获取Profile设备状态', 'method': 'getDeviceState',  'result': 'None', 'category': AutoCategory.A2dpSinkAuto },
//]
const A2dpSourceProfileAutoTest = [
    { 'name': '连接', 'detail': 'Profile连接', 'method': 'connect', 'result': 'None', 'category': testData_1.AutoCategory.A2dpSourceAuto },
    { 'name': '取消连接', 'detail': '取消Profile连接', 'method': 'disconnect', 'result': 'None', 'category': testData_1.AutoCategory.A2dpSourceAuto },
    { 'name': '获取播放状态', 'detail': '获取Profile播放状态', 'method': 'getPlayingState', 'result': 'None', 'category': testData_1.AutoCategory.A2dpSourceAuto },
    { 'name': '获取连接设备', 'detail': '获取Profile连接设备', 'method': 'getConnectionDevices', 'result': 'None', 'category': testData_1.AutoCategory.A2dpSourceAuto },
    { 'name': '获取设备状态', 'detail': '获取Profile设备状态', 'method': 'getDeviceState', 'result': 'None', 'category': testData_1.AutoCategory.A2dpSourceAuto },
];
//const AvrcpCTProfileAutoTest:any []=[
//  { 'name': '连接', 'detail': 'Profile连接', 'method': 'connect',  'result': 'None', 'category': AutoCategory.AvrcpCTAuto },
//  { 'name': '取消连接', 'detail': '取消Profile连接', 'method': 'disconnect',  'result': 'None', 'category': AutoCategory.AvrcpCTAuto },
//  { 'name': '获取连接设备', 'detail': '获取Profile连接设备', 'method': 'getConnectionDevices', 'result': 'None', 'category': AutoCategory.AvrcpCTAuto },
//  { 'name': '获取设备状态', 'detail': '获取Profile设备状态', 'method': 'getDeviceState',  'result': 'None', 'category': AutoCategory.AvrcpCTAuto },
//]
//const AvrcpTGProfileAutoTest:any []=[
//  { 'name': '连接', 'detail': 'Profile连接', 'method': 'connect', 'result': 'None', 'category': AutoCategory.AvrcpTGAuto },
//  { 'name': '取消连接', 'detail': '取消Profile连接', 'method': 'disconnect',  'result': 'None', 'category': AutoCategory.AvrcpTGAuto },
//  { 'name': '获取连接设备', 'detail': '获取Profile连接设备', 'method': 'getConnectionDevices', 'result': 'None', 'category': AutoCategory.AvrcpTGAuto },
//  { 'name': '获取设备状态', 'detail': '获取Profile设备状态', 'method': 'getDeviceState', 'result': 'None', 'category': AutoCategory.AvrcpTGAuto },
//]
const HandsFreeAudioGatewayProfileAutoTest = [
    { 'name': '连接', 'detail': 'Profile连接', 'method': 'connect', 'result': 'None', 'category': testData_1.AutoCategory.HandsFreeAudioGatewayAuto },
    { 'name': '取消连接', 'detail': '取消Profile连接', 'method': 'disconnect', 'result': 'None', 'category': testData_1.AutoCategory.HandsFreeAudioGatewayAuto },
    { 'name': '获取连接设备', 'detail': '获取Profile连接设备', 'method': 'getConnectionDevices', 'result': 'None', 'category': testData_1.AutoCategory.HandsFreeAudioGatewayAuto, },
    { 'name': '获取设备状态', 'detail': '获取Profile设备状态', 'method': 'getDeviceState', 'result': 'None', 'category': testData_1.AutoCategory.HandsFreeAudioGatewayAuto, },
];
//const HandsFreeUnitProfileAutoTest: any[] =[
//  { 'name': '连接', 'detail': 'Profile连接', 'method': 'connect', 'result': 'None', 'category': AutoCategory.HandsFreeUnitAuto },
//  { 'name': '取消连接', 'detail': '取消Profile连接', 'method': 'disconnect',  'result': 'None', 'category': AutoCategory.HandsFreeUnitAuto },
//  { 'name': '连接Sco', 'detail': '连接Sco', 'method': 'connectSco',  'result': 'None', 'category': AutoCategory.HandsFreeUnitAuto },
//  { 'name': '获取Sco状态', 'detail': '获取Sco状态', 'method': 'getScoState',  'result': 'None', 'category': AutoCategory.HandsFreeUnitAuto },
//  { 'name': '取消连接Sco', 'detail': '取消连接Sco', 'method': 'disconnectSco',  'result': 'None', 'category': AutoCategory.HandsFreeUnitAuto },
//  { 'name': '发送DTMF', 'detail': '发送DTMF', 'method': 'sendDTMF', 'result': 'None', 'category': AutoCategory.HandsFreeUnitAuto },
//  { 'name': '获取连接设备', 'detail': '获取Profile连接设备', 'method': 'getConnectionDevices', 'result': 'None', 'category': AutoCategory.HandsFreeUnitAuto },
//  { 'name': '获取设备状态', 'detail': '获取Profile设备状态', 'method': 'getDeviceState',  'result': 'None', 'category': AutoCategory.HandsFreeUnitAuto },
//]
const HitHostProfileAutoTest = [
    { 'name': '连接', 'detail': 'Profile连接', 'method': 'connect', 'result': 'None', 'category': testData_1.AutoCategory.HitHostAuto },
    { 'name': '取消连接', 'detail': '取消Profile连接', 'method': 'disconnect', 'result': 'None', 'category': testData_1.AutoCategory.HitHostAuto },
    { 'name': '获取连接设备', 'detail': '获取Profile连接设备', 'method': 'getConnectionDevices', 'result': 'None', 'category': testData_1.AutoCategory.HitHostAuto },
    { 'name': '获取设备状态', 'detail': '获取Profile设备状态', 'method': 'getDeviceState', 'result': 'None', 'category': testData_1.AutoCategory.HitHostAuto },
];
const PanNetworkProfileAutoTest = [
    { 'name': '取消连接', 'detail': '取消Profile连接', 'method': 'disconnect', 'result': 'None', 'category': testData_1.AutoCategory.PanNetworkAuto },
    { 'name': '获取连接设备', 'detail': '获取Profile连接设备', 'method': 'getConnectionDevices', 'result': 'None', 'category': testData_1.AutoCategory.PanNetworkAuto },
    { 'name': '获取设备状态', 'detail': '获取Profile设备状态', 'method': 'getDeviceState', 'result': 'None', 'category': testData_1.AutoCategory.PanNetworkAuto },
    { 'name': '设置网络共享', 'detail': '设置网络数据共享', 'method': 'setTethering', 'result': 'None', 'category': testData_1.AutoCategory.PanNetworkAuto },
    { 'name': '网络共享状态打开', 'detail': '网络共享状态打开', 'method': 'isTetheringOn', 'result': 'None', 'category': testData_1.AutoCategory.PanNetworkAuto },
];
//const PbapClientProfileAutoTest:any[] =[
//  { 'name': '连接', 'detail': 'Profile连接', 'method': 'connect',  'result': 'None', 'category': AutoCategory.PbapClientAuto },
//  { 'name': '取消连接', 'detail': '取消Profile连接', 'method': 'disconnect', 'result': 'None', 'category': AutoCategory.PbapClientAuto },
//  { 'name': '获取连接设备', 'detail': '获取Profile连接设备', 'method': 'getConnectionDevices', 'result': 'None', 'category': AutoCategory.PbapClientAuto },
//  { 'name': '获取设备状态', 'detail': '获取Profile设备状态', 'method': 'getDeviceState',  'result': 'None', 'category': AutoCategory.PbapClientAuto },
//]
//
//const PbapServerProfileAutoTest:any[] =[
//  { 'name': '取消连接', 'detail': '取消Profile连接', 'method': 'disconnect',  'result': 'None', 'category': AutoCategory.PbapServerAuto },
//  { 'name': '获取连接设备', 'detail': '获取Profile连接设备', 'method': 'getConnectionDevices',  'result': 'None', 'category': AutoCategory.PbapServerAuto },
//  { 'name': '获取设备状态', 'detail': '获取Profile设备状态', 'method': 'getDeviceState', 'result': 'None', 'category': AutoCategory.PbapServerAuto },
//]
function initBrAutoData() {
    let TestAutoArray = [];
    BrAutoTest.forEach(item => {
        TestAutoArray.push(new testData_1.TestAuto(item.name, item.detail, item.method, item.result, item.Category));
    });
    return TestAutoArray;
}
exports.initBrAutoData = initBrAutoData;
function initBleAutoData() {
    let TestAutoArray = [];
    BleAutoTest.forEach(item => {
        TestAutoArray.push(new testData_1.TestAuto(item.name, item.detail, item.method, item.result, item.Category));
    });
    return TestAutoArray;
}
exports.initBleAutoData = initBleAutoData;
function initGattClientAutoData() {
    let TestAutoArray = [];
    GattClientAutoTest.forEach(item => {
        TestAutoArray.push(new testData_1.TestAuto(item.name, item.detail, item.method, item.result, item.Category));
    });
    return TestAutoArray;
}
exports.initGattClientAutoData = initGattClientAutoData;
function initGattServerAutoData() {
    let TestAutoArray = [];
    GattServerAutoTest.forEach(item => {
        TestAutoArray.push(new testData_1.TestAuto(item.name, item.detail, item.method, item.result, item.Category));
    });
    return TestAutoArray;
}
exports.initGattServerAutoData = initGattServerAutoData;
function initSppClientAutoData() {
    let TestAutoArray = [];
    SppClientAutoTest.forEach(item => {
        TestAutoArray.push(new testData_1.TestAuto(item.name, item.detail, item.method, item.result, item.Category));
    });
    return TestAutoArray;
}
exports.initSppClientAutoData = initSppClientAutoData;
function initSppServerAutoData() {
    let TestAutoArray = [];
    SppServerAutoTest.forEach(item => {
        TestAutoArray.push(new testData_1.TestAuto(item.name, item.detail, item.method, item.result, item.Category));
    });
    return TestAutoArray;
}
exports.initSppServerAutoData = initSppServerAutoData;
function initA2dpSourceProfileAutoData() {
    let TestAutoArray = [];
    A2dpSourceProfileAutoTest.forEach(item => {
        TestAutoArray.push(new testData_1.TestAuto(item.name, item.detail, item.method, item.result, item.Category));
    });
    return TestAutoArray;
}
exports.initA2dpSourceProfileAutoData = initA2dpSourceProfileAutoData;
function initHandsFreeAudioGatewayProfileAutoData() {
    let TestAutoArray = [];
    HandsFreeAudioGatewayProfileAutoTest.forEach(item => {
        TestAutoArray.push(new testData_1.TestAuto(item.name, item.detail, item.method, item.result, item.Category));
    });
    return TestAutoArray;
}
exports.initHandsFreeAudioGatewayProfileAutoData = initHandsFreeAudioGatewayProfileAutoData;
function initHitHostProfileAutoData() {
    let TestAutoArray = [];
    HitHostProfileAutoTest.forEach(item => {
        TestAutoArray.push(new testData_1.TestAuto(item.name, item.detail, item.method, item.result, item.Category));
    });
    return TestAutoArray;
}
exports.initHitHostProfileAutoData = initHitHostProfileAutoData;
function initPanNetworkProfileAutoData() {
    let TestAutoArray = [];
    PanNetworkProfileAutoTest.forEach(item => {
        TestAutoArray.push(new testData_1.TestAuto(item.name, item.detail, item.method, item.result, item.Category));
    });
    return TestAutoArray;
}
exports.initPanNetworkProfileAutoData = initPanNetworkProfileAutoData;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/bleInterface.ets":
/*!*******************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/bleInterface.ets ***!
  \*******************************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.testOnBLEDeviceFind = exports.testStopBLEScan = exports.testStartBLEScan = exports.testGetConnectedBLEDevices = exports.testCreateGattClientDevice = exports.testCreateGattServerDevice = void 0;
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
/**
 * BLE Test Page Of Bluetooth test
 */
var _system_prompt_1  = isSystemplugin('prompt', 'system') ? globalThis.systemplugin.prompt : globalThis.requireNapi('prompt');
const bluetoothModel_1 = __importDefault(__webpack_require__(/*! ./bluetoothModel */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/bluetoothModel.ts"));
var _ohos_bluetooth_1  = globalThis.requireNapi('bluetooth') || (isSystemplugin('bluetooth', 'ohos') ? globalThis.ohosplugin.bluetooth : isSystemplugin('bluetooth', 'system') ? globalThis.systemplugin.bluetooth : undefined);
let gattServerInstance = null;
function testCreateGattServerDevice() {
    let message = "success";
    let btState = _ohos_bluetooth_1.getState();
    if (btState == _ohos_bluetooth_1.BluetoothState.STATE_ON) {
        message = "createGattServer starts";
        if (!gattServerInstance) {
            gattServerInstance = bluetoothModel_1.default.createGattServer();
            message = "createGattServer: succeeds.";
        }
        else {
            message = "GattServer existed!";
        }
    }
    else if (btState == _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
        message = "BT is not enabled!";
    }
    else {
        message = "switching,please wait!";
    }
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testCreateGattServerDevice = testCreateGattServerDevice;
let gattClientDeviceInstance = null;
function testCreateGattClientDevice() {
    let message = "success";
    let btState = _ohos_bluetooth_1.getState();
    let peripheralDeviceId = AppStorage.Get('peripheralDeviceId');
    if (btState == _ohos_bluetooth_1.BluetoothState.STATE_ON) {
        message = "createGattClient starts";
        if (!gattClientDeviceInstance) {
            gattClientDeviceInstance = bluetoothModel_1.default.createGattClient(peripheralDeviceId);
            message = "createGattClient:succeeds.";
        }
        else {
            message = "gattclient existed!";
        }
    }
    else if (btState == _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
        message = "BT is not enabled!";
    }
    else {
        message = "switching,please wait!";
    }
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testCreateGattClientDevice = testCreateGattClientDevice;
function testGetConnectedBLEDevices() {
    let connectedBleDevices = bluetoothModel_1.default.getConnectedBLEDevices();
    let message = "getConnectedBLEDevices" + JSON.stringify(connectedBleDevices);
    console.log("getConnectedBLEDevices" + JSON.stringify(connectedBleDevices));
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testGetConnectedBLEDevices = testGetConnectedBLEDevices;
function testStartBLEScan() {
    let btState = _ohos_bluetooth_1.getState();
    let message = "success";
    if (btState == _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
        message = "BT is not enabled!";
        return message;
    }
    let txtScanFilter_deviceId = AppStorage.Get('txtScanFilterDeviceId');
    let txtScanFilter_name = AppStorage.Get('txtScanFilterName');
    let txtScanFilter_serviceUuid = AppStorage.Get('txtScanFilterServiceUuid');
    let txtScanOptions_interval = AppStorage.Get('txtScanOptions_interval');
    let txtScanOptions_dutyMode = AppStorage.Get('txtScanOptionsDutyMode');
    let txtScanOptions_matchMode = AppStorage.Get('txtScanOptionsMatchMode');
    let cbxBleScanFilter = AppStorage.Get('cbxBleScanFilter');
    let cbxBleScanOptions = AppStorage.Get('cbxBleScanOptions');
    let curScanFilters = [];
    let bleScanFilter = {
        deviceId: txtScanFilter_deviceId,
        name: txtScanFilter_name,
        serviceUuid: txtScanFilter_serviceUuid
    };
    let curScanOptions = {
        interval: parseInt(txtScanOptions_interval),
        dutyMode: parseInt(txtScanOptions_dutyMode),
        matchMode: parseInt(txtScanOptions_matchMode)
    };
    if (cbxBleScanFilter) {
        curScanFilters.push(bleScanFilter);
    }
    if (!cbxBleScanOptions) {
        if (cbxBleScanFilter) {
            bluetoothModel_1.default.startBLEScan(curScanFilters);
            message = "startBLEScan Filters:" + JSON.stringify(curScanFilters);
        }
        else {
            bluetoothModel_1.default.startBLEScan(null);
            message = "startBLEScan(null)";
        }
    }
    else {
        bluetoothModel_1.default.startBLEScan(curScanFilters, curScanOptions);
        message = "startBLEScan Filters:" + JSON.stringify(curScanFilters) + "\n Options:" + JSON.stringify(curScanOptions);
    }
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testStartBLEScan = testStartBLEScan;
function testStopBLEScan() {
    bluetoothModel_1.default.stopBLEScan();
    let message = "stopBLEScan OK!";
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testStopBLEScan = testStopBLEScan;
let bleDeviceFind = false;
function testOnBLEDeviceFind() {
    let message = "testOnBLEDeviceFind";
    if (bleDeviceFind) {
        bluetoothModel_1.default.unsubscribeBLEDeviceFind();
        message = 'offBLEDeviceFind';
        _system_prompt_1.showToast({ message: message });
    }
    else {
        bluetoothModel_1.default.subscribeBLEDeviceFind((deviceItems) => {
            LogUtil.log("BLE Test:" + 'available ble devices changed.');
            let message = "";
            let availBleDeviceIds = [];
            deviceItems === null || deviceItems === void 0 ? void 0 : deviceItems.forEach(deviceItem => {
                let newDevice = deviceItem.deviceId;
                message += "mac:" + newDevice;
                LogUtil.log("BLE Test:" + 'available bluetooth new device : ' + newDevice);
                availBleDeviceIds.push(newDevice);
            });
            AppStorage.SetOrCreate('bluetoothAvailableDevices', availBleDeviceIds);
            _system_prompt_1.showToast({ message: message });
        });
        message = "onBLEDeviceFind";
        _system_prompt_1.showToast({ message: message });
    }
    bleDeviceFind = !bleDeviceFind;
    return message;
}
exports.testOnBLEDeviceFind = testOnBLEDeviceFind;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/bluetoothDevice.ts":
/*!*********************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/bluetoothDevice.ts ***!
  \*********************************************************************************************************************************/
/***/ ((__unused_webpack_module, exports, __webpack_require__) => {

"use strict";

Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.Profile = void 0;
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
/**
 * BT Device Page Of Bluetooth test
 */
const bluetoothModel_1 = __webpack_require__(/*! ./bluetoothModel */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/bluetoothModel.ts");
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
            this.connectionState = bluetoothModel_1.DeviceState.STATE_CONNECTED;
        }
        else if (countStateConnecting > 0) {
            this.connectionState = bluetoothModel_1.DeviceState.STATE_CONNECTING;
        }
        else {
            this.connectionState = bluetoothModel_1.DeviceState.STATE_DISCONNECTED;
        }
    }
}
exports["default"] = BluetoothDevice;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/bluetoothModel.ts":
/*!********************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/bluetoothModel.ts ***!
  \********************************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.BluetoothModel = exports.BluetoothErrorCode = exports.DeviceState = exports.DeviceType = exports.BondState = exports.ProfileCode = void 0;
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
/**
 * BT MODEL Page Of Bluetooth test
 */
const LogUtil_1 = __importDefault(__webpack_require__(/*! ../../Utils/LogUtil */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Utils/LogUtil.ts"));
const BaseModel_1 = __importDefault(__webpack_require__(/*! ../../Utils/BaseModel */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Utils/BaseModel.ts"));
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
        let deviceType = '';
        let deviceClass = _ohos_bluetooth_1.getRemoteDeviceClass(deviceId);
        deviceType = "Major:" + deviceClass.majorClass.toString();
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
        //  return bluetooth.cancelPairedDevice(deviceId);
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

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/brInterface.ets":
/*!******************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/brInterface.ets ***!
  \******************************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.testOnBondStateChange = exports.testOnPinRequired = exports.testOnStateChange = exports.testGetProfileConnState2 = exports.testGetProfileConnState1 = exports.testCancelPairedDevice = exports.testGetPairedDevices = exports.testGetRemoteDeviceClass = exports.testGetRemoteDeviceName = exports.testPairDevice = exports.testStopBluetoothDiscovery = exports.testStartBluetoothDiscovery = exports.testGetBluetoothScanMode = exports.testSetBluetoothScanMode = exports.testGetLocalName = exports.testSetLocalName = exports.testGetBtConnectionState = exports.testGetState = exports.testDisableBluetooth = exports.testEnableBluetooth = exports.testInputBluetooth = void 0;
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
/**
 *BR Test Page Of Bluetooth test
 */
var _system_prompt_1  = isSystemplugin('prompt', 'system') ? globalThis.systemplugin.prompt : globalThis.requireNapi('prompt');
const bluetoothModel_1 = __importDefault(__webpack_require__(/*! ./bluetoothModel */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/bluetoothModel.ts"));
var _system_router_1  = globalThis.requireNativeModule('system.router');
const BluetoothDeviceController_1 = __importDefault(__webpack_require__(/*! ../controller/BluetoothDeviceController */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/controller/BluetoothDeviceController.ts"));
var _ohos_bluetooth_1  = globalThis.requireNapi('bluetooth') || (isSystemplugin('bluetooth', 'ohos') ? globalThis.ohosplugin.bluetooth : isSystemplugin('bluetooth', 'system') ? globalThis.systemplugin.bluetooth : undefined);
var _ohos_bluetooth_2  = globalThis.requireNapi('bluetooth') || (isSystemplugin('bluetooth', 'ohos') ? globalThis.ohosplugin.bluetooth : isSystemplugin('bluetooth', 'system') ? globalThis.systemplugin.bluetooth : undefined);
const PAGE_URI_DEVICE_FOUND_MODE = 'pages/subManualApiTest/subBrTest/deviceFound';
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
let btEnable = false;
function testEnableBluetooth() {
    let message = "";
    if (btEnable) {
        message = '蓝牙已经使能';
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    let ret = bluetoothModel_1.default.enableBluetooth();
    btEnable = true;
    AppStorage.SetOrCreate('bluetoothIsOn', btEnable);
    message = "蓝牙使能执行结果：" + ret;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testEnableBluetooth = testEnableBluetooth;
function testDisableBluetooth() {
    let message = "";
    //  if (!btEnable) {
    //    message = '蓝牙还未使能';
    //    prompt.showToast({message: message})
    //    return message
    //  }
    let ret = bluetoothModel_1.default.disableBluetooth();
    btEnable = false;
    AppStorage.SetOrCreate('bluetoothIsOn', btEnable);
    message = "蓝牙去使能执行结果：" + ret;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testDisableBluetooth = testDisableBluetooth;
function testGetState() {
    let ret = bluetoothModel_1.default.getState();
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
    let ret = bluetoothModel_1.default.getBtConnectionState();
    let message = "当前连接状态是：" + ret;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testGetBtConnectionState = testGetBtConnectionState;
function testSetLocalName(name) {
    let ret = bluetoothModel_1.default.setLocalName(name);
    let message = "设置：" + ret + "设备名称为：" + name;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testSetLocalName = testSetLocalName;
function testGetLocalName() {
    let ret = bluetoothModel_1.default.getLocalName();
    let message = "本设备名称是：" + ret;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testGetLocalName = testGetLocalName;
function testSetBluetoothScanMode(mode, dur) {
    let retObj = { mod: 0, duration: -1 };
    let ret = bluetoothModel_1.default.setBluetoothScanMode(mode, dur);
    //  let ret = true   调试使用
    if (ret) {
        AppStorage.SetOrCreate('setScanModeRet', ret);
        retObj.mod = mode;
        retObj.duration = dur;
    }
    else {
        console.info("BluetoothModel.setBluetoothScanMode failed!");
    }
    console.info("BluetoothModel.setBluetoothScanMode success!", retObj);
    let message = "setBluetoothScanMode执行" + ret ? "成功" : 0;
    _system_prompt_1.showToast({ message: message });
    return retObj;
}
exports.testSetBluetoothScanMode = testSetBluetoothScanMode;
function testGetBluetoothScanMode() {
    let ret = bluetoothModel_1.default.getBluetoothScanMode();
    let message = "模式: " + ret;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testGetBluetoothScanMode = testGetBluetoothScanMode;
function testStartBluetoothDiscovery() {
    let message = "";
    let btEnable = AppStorage.Get('bluetoothIsOn');
    if (!btEnable) {
        message = '蓝牙还未使能';
        return message;
    }
    else {
        _system_router_1.push({ uri: PAGE_URI_DEVICE_FOUND_MODE });
        message = "蓝牙发现成功";
    }
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testStartBluetoothDiscovery = testStartBluetoothDiscovery;
function testStopBluetoothDiscovery() {
    let ret = bluetoothModel_1.default.stopBluetoothDiscovery();
    let message = "stopBluetoothDiscovery 执行: " + ret;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testStopBluetoothDiscovery = testStopBluetoothDiscovery;
function testPairDevice(deviceId) {
    let controller = new BluetoothDeviceController_1.default;
    let ret = _ohos_bluetooth_1.pairDevice(deviceId);
    let message = "";
    controller.pair(deviceId, (pinCode) => {
        _system_prompt_1.showToast({ message: pinCode });
        AppStorage.SetOrCreate('pairedMac', deviceId);
    }, () => {
        _system_prompt_1.showToast({ message: "testPairDevice Failed" });
    });
    message = "配对：" + deviceId + ret ? "成功" : 0;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testPairDevice = testPairDevice;
function testGetRemoteDeviceName(deviceId) {
    let ret = bluetoothModel_1.default.getDeviceName(deviceId);
    let message = '远程设备名:' + ret;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testGetRemoteDeviceName = testGetRemoteDeviceName;
function testGetRemoteDeviceClass(deviceId) {
    //  let deviceId= "12312fsdf";
    let ret = bluetoothModel_1.default.getDeviceType(deviceId);
    let message = '远程class名:' + ret;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testGetRemoteDeviceClass = testGetRemoteDeviceClass;
function testGetPairedDevices() {
    let ret = bluetoothModel_1.default.getPairedDeviceIds();
    let message = "";
    console.info("BluetoothModel.getPairedDeviceIds:" + ret);
    if (ret.length == 0) {
        message = '当前没有任何配对设备';
    }
    else {
        message = "设备为：" + ret[0];
        AppStorage.SetOrCreate('pairedMac', ret[0]);
    }
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testGetPairedDevices = testGetPairedDevices;
function testCancelPairedDevice(deviceId) {
    //  let deviceId = "123123"
    let ret = bluetoothModel_1.default.unpairDevice(deviceId);
    let message = " 设备 " + deviceId + " 断开：" + ret;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testCancelPairedDevice = testCancelPairedDevice;
function testGetProfileConnState1(profileId) {
    let ret = bluetoothModel_1.default.getProfileConnState(profileId);
    let message = ret;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testGetProfileConnState1 = testGetProfileConnState1;
function testGetProfileConnState2(profileId, deviceId) {
    let ret = _ohos_bluetooth_2.getProfile(profileId).getDeviceState(deviceId);
    let stateProfile = "";
    console.log('state is -' + ret);
    switch (ret) {
        case 0:
            stateProfile = 'STATE_DISCONNECTED';
            break;
        case 1:
            stateProfile = 'STATE_CONNECTING';
            break;
        case 2:
            stateProfile = 'STATE_CONNECTED';
            break;
        case 3:
            stateProfile = 'STATE_DISCONNECTING';
            break;
        default:
            stateProfile = '未知';
            break;
    }
    let message = stateProfile;
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
            }
            _system_prompt_1.showToast({ message: "状态:" + btStateMessage });
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
            _system_prompt_1.showToast({ message: "mac:" + data.deviceId + "pin:" + data.pinCode });
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
            _system_prompt_1.showToast({ message: "mac:" + data.deviceId + "state:" + data.state });
        });
        message = "onBondStateChange";
    }
    bondStateFlag = !bondStateFlag;
    return message;
}
exports.testOnBondStateChange = testOnBondStateChange;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/btProfileModel.ets":
/*!*********************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/btProfileModel.ets ***!
  \*********************************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.BTProfileModel = void 0;
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
 * BT ProfileModel Test Page Of Bluetooth test
 */
const ConfigData_1 = __importDefault(__webpack_require__(/*! ../../Utils/ConfigData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Utils/ConfigData.ts"));
const BaseModel_1 = __importDefault(__webpack_require__(/*! ../../Utils/BaseModel */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Utils/BaseModel.ts"));
var _ohos_bluetooth_1  = globalThis.requireNapi('bluetooth') || (isSystemplugin('bluetooth', 'ohos') ? globalThis.ohosplugin.bluetooth : isSystemplugin('bluetooth', 'system') ? globalThis.systemplugin.bluetooth : undefined);
class BTProfileModel extends BaseModel_1.default {
    constructor() {
        super();
        this.TAG = ConfigData_1.default.TAG + 'BTProfileModel';
        this.profiles = new Array();
        this.PROFILE_A2DP_SOURCE = 1;
        this.PROFILE_HANDS_FREE_AUDIO_GATEWAY = 4;
        this.PROFILE_HID_HOST = 6;
        this.PROFILE_PAN_NETWORK = 7;
        this.BLUETOOTH_STATE_OFF = 0;
    }
    initProfile() {
        console.log('bt_test BTProfileModel start');
        this.profiles[this.PROFILE_A2DP_SOURCE] = _ohos_bluetooth_1.getProfile(this.PROFILE_A2DP_SOURCE);
        this.profiles[this.PROFILE_HANDS_FREE_AUDIO_GATEWAY] = _ohos_bluetooth_1.getProfile(this.PROFILE_HANDS_FREE_AUDIO_GATEWAY);
        this.profiles[this.PROFILE_HID_HOST] = _ohos_bluetooth_1.getProfile(this.PROFILE_HID_HOST);
        this.profiles[this.PROFILE_PAN_NETWORK] = _ohos_bluetooth_1.getProfile(this.PROFILE_PAN_NETWORK);
        console.log('bt_test BTProfileModel end');
    }
    hasProfile(type) {
        if (!this.profiles[type]) {
            return false;
        }
        return true;
    }
    getA2DPSourceProfile() {
        if (this.profiles[this.PROFILE_A2DP_SOURCE] != null) {
            return this.profiles[this.PROFILE_A2DP_SOURCE];
        }
        return null;
    }
    getHandsFreeAudioGatewayProfile() {
        if (this.profiles[this.PROFILE_HANDS_FREE_AUDIO_GATEWAY] != null) {
            return this.profiles[this.PROFILE_HANDS_FREE_AUDIO_GATEWAY];
        }
        return null;
    }
    getHidHostProfile() {
        if (this.profiles[this.PROFILE_HID_HOST] != null) {
            return this.profiles[this.PROFILE_HID_HOST];
        }
        return null;
    }
    getPanNetworkProfile() {
        if (this.profiles[this.PROFILE_PAN_NETWORK] != null) {
            return this.profiles[this.PROFILE_PAN_NETWORK];
        }
        return null;
    }
    getProfile(type) {
        if (this.profiles[type] != null) {
            return this.profiles[type];
        }
        return null;
    }
    getBluetoothState() {
        return _ohos_bluetooth_1.getState();
    }
}
exports.BTProfileModel = BTProfileModel;
let btProfileModel = new BTProfileModel();
exports["default"] = btProfileModel;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/gattClientInterface.ets":
/*!**************************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/gattClientInterface.ets ***!
  \**************************************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.testGetRssiValuePromise = exports.testGetRssiValueCallback = exports.testGetDeviceNamePromise = exports.testGetDeviceNameCallback = exports.testSetNotifyCharacteristicChanged = exports.testSetBLEMtuSize = exports.testBLECharacteristicChange = exports.testWriteDescriptorValue = exports.testWriteCharacteristicValue = exports.testReadDescriptorValuePromise = exports.testReadDescriptorValueCallback = exports.testReadCharacteristicValuePromise = exports.testReadCharacteristicValueCallback = exports.testGetServicesPromise = exports.testGetServicesCallback = exports.testGattClientClose = exports.testBLEConnectionStateChange = exports.testDisconnect = exports.testConnect = exports.testCreateGattClient = void 0;
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
/**
 * GattClient Test Of Bluetooth test
 */
var _system_prompt_1  = isSystemplugin('prompt', 'system') ? globalThis.systemplugin.prompt : globalThis.requireNapi('prompt');
var _ohos_bluetooth_1  = globalThis.requireNapi('bluetooth') || (isSystemplugin('bluetooth', 'ohos') ? globalThis.ohosplugin.bluetooth : isSystemplugin('bluetooth', 'system') ? globalThis.systemplugin.bluetooth : undefined);
const bluetoothModel_1 = __importDefault(__webpack_require__(/*! ./bluetoothModel */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/bluetoothModel.ts"));
let gattClientInstance = null;
function testCreateGattClient() {
    let message = "";
    let peripheralDeviceId = AppStorage.Get('peripheralDeviceId');
    let btState = _ohos_bluetooth_1.getState();
    if (btState === _ohos_bluetooth_1.BluetoothState.STATE_ON) {
        if (gattClientInstance) {
            message = "GattClient exists!";
            _system_prompt_1.showToast({ message: message });
            return message;
        }
        //注册回调
        _ohos_bluetooth_1.BLE.on("BLEDeviceFind", (scanResult) => {
            scanResult.forEach(device => {
                var devId = device.deviceId; //88:36:CF:09:C1:90
                var rssi = device.rssi;
                var data = device.data;
                _system_prompt_1.showToast({ message: "findDevice:" + devId });
                if (devId == peripheralDeviceId) {
                    _ohos_bluetooth_1.BLE.stopBLEScan();
                    // create ble client instance
                    if (!gattClientInstance) {
                        gattClientInstance = bluetoothModel_1.default.createGattClient(devId);
                        message = "createGattClient():succeeds ";
                    }
                }
            });
        });
        if (peripheralDeviceId && peripheralDeviceId.length > 15) {
            gattClientInstance = bluetoothModel_1.default.createGattClient(peripheralDeviceId.toUpperCase());
            if (gattClientInstance) {
                message = "createGattClient(): " + peripheralDeviceId;
            }
            else {
                message = "createGattClient():fail ";
            }
        }
        else {
            message = "peripheralDeviceId invalid! ";
        }
        _system_prompt_1.showToast({ message: message });
    }
    else if (btState === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
        message = "BT is not enabled!";
        _system_prompt_1.showToast({ message: message });
    }
    else {
        message = "switching,please wait";
        _system_prompt_1.showToast({ message: message });
    }
    return message;
}
exports.testCreateGattClient = testCreateGattClient;
let bleConnectionState = 0;
function testConnect() {
    let message = "waiting";
    let btState = _ohos_bluetooth_1.getState();
    if (btState === _ohos_bluetooth_1.BluetoothState.STATE_ON) {
        if (gattClientInstance) {
            if (bleConnectionState == 0) {
                let ret = gattClientInstance.connect();
                message = "GattClient.connect(): " + ret;
                bleConnectionState = 2;
            }
            else {
                message = "this device is not disconnected!";
            }
        }
        else {
            message = "NO_GATT_CLIENT_OBJECT";
        }
    }
    else if (btState === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
        message = "BT is not enabled!";
    }
    else {
        message = "switching,please wait!";
    }
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testConnect = testConnect;
function testDisconnect() {
    let message = "GattClient.disconnect";
    let btState = _ohos_bluetooth_1.getState();
    if (btState == _ohos_bluetooth_1.BluetoothState.STATE_ON) {
        if (!gattClientInstance) {
            message = "NO_GATT_CLIENT_OBJECT";
            _system_prompt_1.showToast({ message: message });
            return message;
        }
        if (bleConnectionState === 2) {
            let ret = gattClientInstance.disconnect();
            message = "GattClient.disconnect(): " + ret;
        }
        else {
            message = " BLE is not connected and no need to disconnect it ";
        }
    }
    else if (btState == _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
        message = "BT is not enabled!";
    }
    else {
        message = "switching,please wait!";
    }
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testDisconnect = testDisconnect;
let BLEConnectionStateChange = false;
function testBLEConnectionStateChange() {
    let message = "waiting";
    let btStatus = _ohos_bluetooth_1.getState();
    if (btStatus === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
        message = "BT is not enabled!";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    if (!gattClientInstance) {
        message = "NO_GATT_CLIENT_OBJECT";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    message = "testOnBLEConnectionStateChange";
    if (BLEConnectionStateChange) {
        gattClientInstance.off('BLEConnectionStateChange', () => {
            _system_prompt_1.showToast({ message: "off:BLEConnectionStateChange" });
        });
        message = "offBLEConnectionStateChange";
    }
    else {
        gattClientInstance.on('BLEConnectionStateChange', (bleConnectionState) => {
            switch (bleConnectionState) {
                case _ohos_bluetooth_1.ProfileConnectionState.STATE_DISCONNECTED:
                    bleConnectionStateInfo = "DISCONNECTED";
                    break;
                case _ohos_bluetooth_1.ProfileConnectionState.STATE_CONNECTING:
                    bleConnectionStateInfo = "CONNECTING";
                    break;
                case _ohos_bluetooth_1.ProfileConnectionState.STATE_CONNECTED:
                    bleConnectionStateInfo = "STATE_CONNECTED";
                    break;
                case _ohos_bluetooth_1.ProfileConnectionState.STATE_DISCONNECTING:
                    bleConnectionStateInfo = "STATE_DISCONNECTING";
                    break;
                default:
                    bleConnectionStateInfo = "undefined";
                    break;
            }
            AppStorage.SetOrCreate('bleConnectionStateInfo', bleConnectionStateInfo);
            _system_prompt_1.showToast({ message: "状态:" + bleConnectionStateInfo });
        });
        message = "onBLEConnectionStateChange";
    }
    BLEConnectionStateChange = !BLEConnectionStateChange;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testBLEConnectionStateChange = testBLEConnectionStateChange;
function testGattClientClose() {
    let message = "GattClientClose";
    let btState = _ohos_bluetooth_1.getState();
    if (btState == _ohos_bluetooth_1.BluetoothState.STATE_ON) {
        if (!gattClientInstance) {
            message = "NO_GATT_CLIENT_OBJECT";
            _system_prompt_1.showToast({ message: message });
            return message;
        }
        let ret = gattClientInstance.close();
        if (ret) {
            gattClientInstance = null;
        }
        message = "GattClient.close():" + ret;
    }
    else if (btState == _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
        message = "BT is not enabled!";
    }
    else {
        message = "switching,please wait!";
    }
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testGattClientClose = testGattClientClose;
function testGetServicesCallback() {
    let message = "getServices(call back)";
    let btState = _ohos_bluetooth_1.getState();
    if (btState == _ohos_bluetooth_1.BluetoothState.STATE_ON) {
        if (!gattClientInstance) {
            message = "NO_GATT_CLIENT_OBJECT";
            _system_prompt_1.showToast({ message: message });
            return message;
        }
        gattClientInstance.getServices((code, gattServices) => {
            if (code.code != 0) {
                message = "getServices(call back) error code:" + code.code;
                _system_prompt_1.showToast({ message: message });
                return message;
            }
            else {
                message = "getServices(Callback):invoke succeeds! \n ";
                message = "bluetooth services size is " + gattServices.length + '\n';
                for (let i = 0; i < gattServices.length; i++) {
                    message += 'bluetooth serviceUuid is ' + gattServices[i].serviceUuid + '\n';
                }
                _system_prompt_1.showToast({ message: message });
                AppStorage.SetOrCreate("getServices", message);
                console.info("getServices(Callback)", message);
            }
        });
    }
    else if (btState == _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
        message = "BT is not enabled!";
    }
    else {
        message = "switching,please wait!";
    }
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testGetServicesCallback = testGetServicesCallback;
function testGetServicesPromise() {
    let message = "getServices(promise)";
    if (!gattClientInstance) {
        message = "NO_GATT_CLIENT_OBJECT";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    message = "getServices promise:starts";
    let btState = _ohos_bluetooth_1.getState();
    if (btState == _ohos_bluetooth_1.BluetoothState.STATE_ON) {
        gattClientInstance.getServices().then(gattServices => {
            message = "getServicesPromise successfully:\n";
            message = "bluetooth services size is " + gattServices.length;
            for (let i = 0; i < gattServices.length; i++) {
                message += 'bluetooth serviceUuid is ' + gattServices[i].serviceUuid + '\n';
            }
            _system_prompt_1.showToast({ message: message });
        });
        message = "getServices(promise):invoke succeeds!";
    }
    else if (btState == _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
        message = "BT is not enabled!";
    }
    else {
        message = "switching,please wait!";
    }
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testGetServicesPromise = testGetServicesPromise;
function testReadCharacteristicValueCallback() {
    let serviceUUID = AppStorage.Get('serviceUUID');
    let characteristicUUID = AppStorage.Get('characteristicUUID');
    let message = "readCharacteristicValue(callback)";
    let btState = _ohos_bluetooth_1.getState();
    if (btState === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
        message = "BT is not enabled!";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    if (!gattClientInstance) {
        message = "NO_GATT_CLIENT_OBJECT";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    let bleCharacteristicDataIn = {
        serviceUuid: serviceUUID,
        characteristicUuid: characteristicUUID
    };
    gattClientInstance.readCharacteristicValue(bleCharacteristicDataIn, (err, bleCharacteristicDataOut) => {
        if (err.code != 0) {
            message = "readCharacteristicValue error code:" + err.code + serviceUUID;
            console.info(message);
            _system_prompt_1.showToast({ message: message });
            return message;
        }
        message = "readCharacteristicValue callback:";
        message += 'bluetooth characteristic uuid:' + bleCharacteristicDataOut.characteristicUuid + "\n";
        let value = new Uint8Array(bleCharacteristicDataOut.characteristicValue);
        for (let i = 0; i < bleCharacteristicDataOut.characteristicValue.byteLength; i++) {
            message += 'bluetooth characteristic value: ' + value[i];
        }
        AppStorage.SetOrCreate("read_CharacteristicValue", message);
        _system_prompt_1.showToast({ message: message });
    });
    message = 'readCharacteristicValue(callback):invoke succeeds!';
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testReadCharacteristicValueCallback = testReadCharacteristicValueCallback;
function testReadCharacteristicValuePromise() {
    let serviceUUID = AppStorage.Get('serviceUUID');
    let characteristicUUID = AppStorage.Get('characteristicUUID');
    let message = "readCharacteristicValue(promise)";
    let btState = _ohos_bluetooth_1.getState();
    if (btState === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
        message = "BT is not enabled!";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    if (!gattClientInstance) {
        message = "NO_GATT_CLIENT_OBJECT";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    let bleCharacteristicDataIn = {
        serviceUuid: serviceUUID,
        characteristicUuid: characteristicUUID
    };
    gattClientInstance.readCharacteristicValue(bleCharacteristicDataIn).then((bleCharacteristicDataOut) => {
        message = "readCharacteristicValue promise:\n";
        message += 'bluetooth characteristic uuid:' + bleCharacteristicDataOut.characteristicUuid + "\n";
        let value = new Uint8Array(bleCharacteristicDataOut.characteristicValue);
        for (let i = 0; i < bleCharacteristicDataOut.characteristicValue.byteLength; i++) {
            message += 'bluetooth characteristic value: ' + value[i];
        }
        _system_prompt_1.showToast({ message: message });
    });
    message = 'readCharacteristicValue(promise):invoke succeeds!';
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testReadCharacteristicValuePromise = testReadCharacteristicValuePromise;
function testReadDescriptorValueCallback() {
    let serviceUUID = AppStorage.Get('serviceUUID');
    let characteristicUUID = AppStorage.Get('characteristicUUID');
    let descriptorUUID = AppStorage.Get('descriptorUUID');
    let message = "readDescriptorValue(callback)";
    let btState = _ohos_bluetooth_1.getState();
    if (btState === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
        message = "BT is not enabled!";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    if (!gattClientInstance) {
        message = "NO_GATT_CLIENT_OBJECT";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    let bufferDesc = new ArrayBuffer(8);
    let descV = new Uint8Array(bufferDesc);
    descV[0] = 88;
    let descriptorIn = {
        serviceUuid: serviceUUID,
        characteristicUuid: characteristicUUID,
        descriptorUuid: descriptorUUID,
        descriptorValue: bufferDesc
    };
    gattClientInstance.readDescriptorValue(descriptorIn, (err, descriptorOut) => {
        let message = "";
        if (err.code != 0) {
            message = "readDescriptorValue error" + err.code.toString();
            _system_prompt_1.showToast({ message: message });
            return message;
        }
        message = 'readDescriptorValue(callback):descriptor uuid=' + descriptorOut.descriptorUuid + "\n";
        let value = new Uint8Array(descriptorOut.descriptorValue);
        for (let i = 0; i < descriptorOut.descriptorValue.byteLength; i++) {
            message += 'descriptor value: ' + value[i] + "\n";
        }
        _system_prompt_1.showToast({ message: message });
    });
    message = 'readDescriptorValue(callback):invoke succeeds!';
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testReadDescriptorValueCallback = testReadDescriptorValueCallback;
function testReadDescriptorValuePromise() {
    let serviceUUID = AppStorage.Get('serviceUUID');
    let characteristicUUID = AppStorage.Get('characteristicUUID');
    let descriptorUUID = AppStorage.Get('descriptorUUID');
    let message = "readDescriptorValue(promise)";
    let btState = _ohos_bluetooth_1.getState();
    if (btState === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
        message = "BT is not enabled!";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    if (!gattClientInstance) {
        message = "NO_GATT_CLIENT_OBJECT";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    let bufferDesc = new ArrayBuffer(8);
    let descV = new Uint8Array(bufferDesc);
    descV[0] = 99;
    let descriptorIn = {
        serviceUuid: serviceUUID,
        characteristicUuid: characteristicUUID,
        descriptorUuid: descriptorUUID,
        descriptorValue: bufferDesc
    };
    gattClientInstance.readDescriptorValue(descriptorIn).then((descriptorOut) => {
        message = 'readDescriptorValue(promise):descriptor uuid=' + descriptorOut.descriptorUuid + "\n";
        let value = new Uint8Array(descriptorOut.descriptorValue);
        for (let i = 0; i < descriptorOut.descriptorValue.byteLength; i++) {
            message += 'descriptor value: ' + value[i] + "\n";
        }
        _system_prompt_1.showToast({ message: message });
    });
    message = 'readDescriptorValue(promise):invoke succeeds!';
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testReadDescriptorValuePromise = testReadDescriptorValuePromise;
function testWriteCharacteristicValue() {
    let serviceUUID = AppStorage.Get('serviceUUID');
    let characteristicUUID = AppStorage.Get('characteristicUUID');
    let characteristicValue = AppStorage.Get('characteristicValue');
    let message = "writeCharacteristicValue";
    let btState = _ohos_bluetooth_1.getState();
    if (btState === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
        message = "BT is not enabled!";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    if (!gattClientInstance) {
        message = "NO_GATT_CLIENT_OBJECT";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    let bufferCCC = string2ArrayBuffer(characteristicValue);
    let characteristic = {
        serviceUuid: serviceUUID,
        characteristicUuid: characteristicUUID,
        characteristicValue: bufferCCC
    };
    if (gattClientInstance.writeCharacteristicValue(characteristic)) {
        message = 'writeCharacteristicValue succeeds.';
    }
    else {
        message = 'writeCharacteristicValue failed!';
    }
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testWriteCharacteristicValue = testWriteCharacteristicValue;
function string2ArrayBuffer(str) {
    let array = new Uint8Array(str.length);
    for (var i = 0; i < str.length; i++) {
        array[i] = str.charCodeAt(i);
    }
    return array.buffer;
}
function testWriteDescriptorValue() {
    let serviceUUID = AppStorage.Get('serviceUUID');
    let characteristicUUID = AppStorage.Get('characteristicUUID');
    let descriptorUUID = AppStorage.Get('descriptorUUID');
    let descriptorValue = AppStorage.Get('descriptorValue');
    let message = "writeDescriptorValue";
    let btState = _ohos_bluetooth_1.getState();
    if (btState === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
        message = "BT is not enabled!";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    if (!gattClientInstance) {
        message = "NO_GATT_CLIENT_OBJECT";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    let bufferDesc = string2ArrayBuffer(descriptorValue);
    let descriptor = {
        serviceUuid: serviceUUID,
        characteristicUuid: characteristicUUID,
        descriptorUuid: descriptorUUID,
        descriptorValue: bufferDesc
    };
    if (gattClientInstance.writeDescriptorValue(descriptor)) {
        message = 'writeDescriptorValue succeeds.';
    }
    else {
        message = 'writeDescriptorValue,failed!';
    }
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testWriteDescriptorValue = testWriteDescriptorValue;
//回调函数
let BLECharacteristicChange = false;
function testBLECharacteristicChange() {
    let message = "跳转成功";
    let btStatus = _ohos_bluetooth_1.getState();
    if (btStatus === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
        message = "BT is not enabled!";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    if (!gattClientInstance) {
        message = "NO_GATT_CLIENT_OBJECT";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    if (BLECharacteristicChange) {
        gattClientInstance.off('BLECharacteristicChange', () => {
            _system_prompt_1.showToast({ message: "on:BLECharacteristicChange" });
        });
        message = "offBLECharacteristicChange";
    }
    else {
        gattClientInstance.on('BLECharacteristicChange', (data) => {
            let serviceUuid = data.serviceUuid;
            let characteristicUuid = data.characteristicUuid;
            bleCharacteristicChangeInfo += "serviceUuid:" + serviceUuid + "\n";
            bleCharacteristicChangeInfo += "characteristicUuid:" + characteristicUuid + "\n";
            let value = new Uint8Array(data.characteristicValue);
            for (let i = 0; i < data.characteristicValue.byteLength; i++) {
                bleCharacteristicChangeInfo += "\n" + 'value:' + value[i];
            }
            AppStorage.SetOrCreate("bleCharacteristicChangeInfo", bleCharacteristicChangeInfo);
            _system_prompt_1.showToast({ message: "on:BLECharacteristicChange" });
        });
        message = "onBLECharacteristicChange";
    }
    BLECharacteristicChange = !BLECharacteristicChange;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testBLECharacteristicChange = testBLECharacteristicChange;
function testSetBLEMtuSize() {
    let message = "setBLEMtuSize";
    let btStatus = _ohos_bluetooth_1.getState();
    if (btStatus === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
        message = "BT is not enabled!";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    if (!gattClientInstance) {
        message = "NO_GATT_CLIENT_OBJECT";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    message = 'setBLEMtuSize: \n';
    let mtuSize = 256;
    let ret = gattClientInstance.setBLEMtuSize(mtuSize);
    message += "result = " + ret;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testSetBLEMtuSize = testSetBLEMtuSize;
function testSetNotifyCharacteristicChanged() {
    let serviceUUID = AppStorage.Get('serviceUUID');
    let characteristicUUID = AppStorage.Get('characteristicUUID');
    let descriptorUUID = AppStorage.Get('descriptorUUID');
    let descriptorValue = AppStorage.Get('descriptorValue');
    let message = "setNotifyCharacteristicChanged(true)";
    let btStatus = _ohos_bluetooth_1.getState();
    if (btStatus === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
        message = "BT is not enabled!";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    if (!gattClientInstance) {
        message = "NO_GATT_CLIENT_OBJECT";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    let descriptors = [];
    let bufferCCC = new ArrayBuffer(8);
    let cccV = new Uint8Array(bufferCCC);
    cccV[0] = 66;
    let descriptor = {
        serviceUuid: serviceUUID,
        characteristicUuid: characteristicUUID,
        descriptorUuid: descriptorUUID,
        descriptorValue: string2ArrayBuffer(descriptorValue)
    };
    descriptors.push(descriptor);
    let characteristicIn = {
        serviceUuid: serviceUUID,
        characteristicUuid: characteristicUUID,
        characteristicValue: bufferCCC,
        descriptors: descriptors
    };
    if (gattClientInstance.setNotifyCharacteristicChanged(characteristicIn, true)) {
        message = "setNotifyCharacteristicChanged(true):succeeds.";
    }
    else {
        message = "setNotifyCharacteristicChanged(true):failed!";
        if (gattClientInstance.setNotifyCharacteristicChanged(characteristicIn, false)) {
            message = "setNotifyCharacteristicChanged(false):true!";
        }
        else {
            message = "setNotifyCharacteristicChanged(false):false!";
        }
    }
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testSetNotifyCharacteristicChanged = testSetNotifyCharacteristicChanged;
function testGetDeviceNameCallback() {
    let deviceName = "";
    let message = "getDeviceName()Callback";
    if (!gattClientInstance) {
        message = "NO_GATT_CLIENT_OBJECT";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    //currentClick = 15;
    gattClientInstance.getDeviceName((err, asyncRemoteDeviceName) => {
        message = "return code:";
        message += JSON.stringify(err);
        message += ",and DeviceName is: ";
        message += JSON.stringify(asyncRemoteDeviceName);
        deviceName = JSON.stringify(asyncRemoteDeviceName);
        AppStorage.SetOrCreate("deviceName", deviceName);
        _system_prompt_1.showToast({ message: message });
    });
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testGetDeviceNameCallback = testGetDeviceNameCallback;
function testGetDeviceNamePromise() {
    let deviceName = "";
    let message = "getDeviceName()Promise";
    if (!gattClientInstance) {
        message = "NO_GATT_CLIENT_OBJECT";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    gattClientInstance.getDeviceName().then((peripheralDeviceName) => {
        message = "getDeviceName()Promise:" + peripheralDeviceName;
        deviceName = peripheralDeviceName;
        AppStorage.SetOrCreate("deviceName", deviceName);
        _system_prompt_1.showToast({ message: message });
    });
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testGetDeviceNamePromise = testGetDeviceNamePromise;
function testGetRssiValueCallback() {
    let Rssi = "";
    let message = "getRssiValue callback";
    if (!gattClientInstance) {
        message = "NO_GATT_CLIENT_OBJECT";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    message = "getRssiValue callback";
    gattClientInstance.getRssiValue((err, cbRssi) => {
        message = "return code:";
        message += JSON.stringify(err);
        message += ",and RSSI=";
        message += JSON.stringify(cbRssi);
        Rssi = JSON.stringify(cbRssi);
        AppStorage.SetOrCreate("Rssi", Rssi);
        _system_prompt_1.showToast({ message: message });
    });
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testGetRssiValueCallback = testGetRssiValueCallback;
function testGetRssiValuePromise() {
    let Rssi = "";
    let message = "getRssiValue promise";
    if (!gattClientInstance) {
        message = "NO_GATT_CLIENT_OBJECT";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    message = "getRssiValue promise:";
    gattClientInstance.getRssiValue().then((rssiValue) => {
        message += 'rssi' + JSON.stringify(rssiValue);
        Rssi = JSON.stringify(rssiValue);
        AppStorage.SetOrCreate("Rssi", Rssi);
        _system_prompt_1.showToast({ message: message });
    });
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testGetRssiValuePromise = testGetRssiValuePromise;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/gattInterface.ets":
/*!********************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/gattInterface.ets ***!
  \********************************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports) {

"use strict";

var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.testGattServer = exports.testGattClient = void 0;
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
/**
 * Gatt Page Of Bluetooth test
 */
var _system_router_1  = globalThis.requireNativeModule('system.router');
var _system_prompt_1  = isSystemplugin('prompt', 'system') ? globalThis.systemplugin.prompt : globalThis.requireNapi('prompt');
const PAGE_URI_GET_GATT_CLIENT = 'pages/subManualApiTest/subGattTest/gattClient';
const PAGE_URI_GET_GATT_SERVER = 'pages/subManualApiTest/subGattTest/gattServer';
function testGattClient() {
    _system_router_1.push({ uri: PAGE_URI_GET_GATT_CLIENT });
    let message = "跳转成功";
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testGattClient = testGattClient;
function testGattServer() {
    _system_router_1.push({ uri: PAGE_URI_GET_GATT_SERVER });
    let message = "跳转成功";
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testGattServer = testGattServer;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/gattServerInterface.ets":
/*!**************************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/gattServerInterface.ets ***!
  \**************************************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.testOnDescriptorWrite = exports.testOnDescriptorRead = exports.testOnCharacteristicWrite = exports.testOnCharacteristicRead = exports.testOnConnectStateChange = exports.testGattServerClose = exports.testStopAdvertising = exports.testStartAdvertising = exports.testRemoveService = exports.testAddService = exports.testCreateGattServer = void 0;
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
var _system_prompt_1  = isSystemplugin('prompt', 'system') ? globalThis.systemplugin.prompt : globalThis.requireNapi('prompt');
var _ohos_bluetooth_1  = globalThis.requireNapi('bluetooth') || (isSystemplugin('bluetooth', 'ohos') ? globalThis.ohosplugin.bluetooth : isSystemplugin('bluetooth', 'system') ? globalThis.systemplugin.bluetooth : undefined);
const bluetoothModel_1 = __importDefault(__webpack_require__(/*! ./bluetoothModel */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/bluetoothModel.ts"));
let gattServerInstance = null;
function testCreateGattServer() {
    let btState = _ohos_bluetooth_1.getState();
    let message = "createGattServer";
    if (btState === _ohos_bluetooth_1.BluetoothState.STATE_ON) {
        if (gattServerInstance) {
            message = "GattServer exists!";
            _system_prompt_1.showToast({ message: message });
            return message;
        }
        gattServerInstance = bluetoothModel_1.default.createGattServer();
        message = "CreateGattServer():succeeds!";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    else if (btState === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
        message = "BT is not enabled!";
    }
    else {
        message = "switching,wait!";
    }
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testCreateGattServer = testCreateGattServer;
function testAddService() {
    let message = "addService";
    let btStatus = _ohos_bluetooth_1.getState();
    if (btStatus === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
        message = "BT is not enabled!";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    if (!gattServerInstance) {
        message = "NO_GATT_Server_OBJECT";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    // build service
    // 调这个本地函数 buildGattService();
    let curServiceUUID = AppStorage.Get("curServiceUUID");
    let curCharacteristicUUID = AppStorage.Get("curCharacteristicUUID");
    let curDescriptorUUID = AppStorage.Get("curDescriptorUUID");
    let curDescriptorValue = AppStorage.Get("curDescriptorValue");
    let curCharacteristicValue = AppStorage.Get("curCharacteristicValue");
    let isPrimary = AppStorage.Get("isPrimary");
    let descriptors = [];
    let descriptor = {
        serviceUuid: curServiceUUID,
        characteristicUuid: curCharacteristicUUID,
        descriptorUuid: curDescriptorUUID,
        descriptorValue: string2ArrayBuffer(curDescriptorValue)
    };
    descriptors.push(descriptor);
    // build characteristics
    let characteristics = [];
    let characteristic = {
        serviceUuid: curServiceUUID,
        characteristicUuid: curCharacteristicUUID,
        characteristicValue: string2ArrayBuffer(curCharacteristicValue),
        descriptors: descriptors
    };
    characteristics.push(characteristic);
    let curGattService = {
        serviceUuid: curServiceUUID,
        characteristics: characteristics,
        isPrimary: isPrimary,
        includeServices: []
    };
    let ret = gattServerInstance.addService(curGattService);
    message = "result: addService()," + ret;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testAddService = testAddService;
function testRemoveService() {
    let message = "removeService()";
    let curServiceUUID = AppStorage.Get("curServiceUUID");
    let btState = _ohos_bluetooth_1.getState();
    if (btState == _ohos_bluetooth_1.BluetoothState.STATE_ON) {
        if (!gattServerInstance) {
            message = "NO_GATT_Server_OBJECT";
            _system_prompt_1.showToast({ message: message });
            return message;
        }
        message = "result: removeService() ";
        if (gattServerInstance.removeService(curServiceUUID)) {
            message += 'true';
        }
        else {
            message += 'false';
        }
    }
    else if (btState == _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
        message = "BT is not enabled!";
    }
    else {
        message = "switching,please wait!";
    }
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testRemoveService = testRemoveService;
let curAdvData = {
    serviceUuids: ["00003344-0000-1000-8000-00805f9b34fb", "00008899-0000-1000-8000-00805f9b34fb"],
    manufactureData: [{
            manufactureId: 7456,
            manufactureValue: string2ArrayBuffer("OHOS CHN")
        }],
    serviceData: [{
            serviceUuid: "000054F6-0000-1000-8000-00805f9b34fb",
            serviceValue: string2ArrayBuffer("OhosBle!")
        }, {
            serviceUuid: "00009999-0000-1000-8000-00805f9b34fb",
            serviceValue: string2ArrayBuffer("dudu_tiger")
        }]
};
let curAdvResponse = {
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
        }, {
            serviceUuid: "00007456-0000-1000-8000-00805f9b34fb",
            serviceValue: string2ArrayBuffer("OhosBle")
        }]
};
function testStartAdvertising() {
    let message = "startAdvertising";
    let btState = _ohos_bluetooth_1.getState();
    if (btState === _ohos_bluetooth_1.BluetoothState.STATE_ON) {
        if (!gattServerInstance) {
            message = "NO_GATT_Server_OBJECT";
            _system_prompt_1.showToast({ message: message });
            return message;
        }
        message = "startAdvertising:build input parameter In!";
        // 调三个本地函数
        // buildAdvSettings();
        let cbxAdvSettings = AppStorage.Get("cbxAdvSettings");
        let cbxInterval = AppStorage.Get("cbxInterval");
        let cbxTxPower = AppStorage.Get("cbxTxPower");
        let curAdvSettings = {
            interval: 800,
            txPower: -7,
            connectable: true
        };
        if (cbxAdvSettings) {
            if (cbxInterval) {
                let advSettings_interval = AppStorage.Get("advSettings_interval");
                curAdvSettings.interval = parseInt(advSettings_interval);
            }
            if (cbxTxPower) {
                let advSettings_txPower = AppStorage.Get("advSettings_txPower");
                curAdvSettings.txPower = parseInt(advSettings_txPower);
            }
            let advSettings_connectable = AppStorage.Get("advSettings_connectable");
            curAdvSettings.connectable = advSettings_connectable;
        }
        //    buildAdvData();
        let advDataServiceUuids_serviceUuid = AppStorage.Get("advDataServiceUuids_serviceUuid");
        let advDataManufactureData_manufactureId = AppStorage.Get("advDataManufactureData_manufactureId");
        let advDataManufactureData_manufactureValue = AppStorage.Get("advDataManufactureData_manufactureValue");
        let advDataServiceData_serviceUuid = AppStorage.Get("advDataServiceData_serviceUuid");
        let advDataServiceData_serviceValue = AppStorage.Get("advDataServiceData_serviceValue");
        let advDataServiceUuidList = [];
        let advDataManufactureDataList = [];
        let advDataServiceDataList = [];
        let advDataManufactureDataInstance = {
            advDataManufactureId: parseInt(advDataManufactureData_manufactureId),
            advDataManufactureValue: string2ArrayBuffer(advDataManufactureData_manufactureValue)
        };
        let advDataServiceDataInstance = {
            advDataServiceUuid: advDataServiceData_serviceUuid,
            advDataServiceValue: string2ArrayBuffer(advDataServiceData_serviceValue)
        };
        let curAdvData = {
            serviceUuids: advDataServiceUuidList.push(advDataServiceUuids_serviceUuid),
            manufactureData: advDataManufactureDataList.push(advDataManufactureDataInstance),
            serviceData: advDataServiceDataList.push(advDataServiceDataInstance)
        };
        //    buildAdvResponse();
        let advResponseServiceUuids_serviceUuid = AppStorage.Get("advResponseServiceUuids_serviceUuid");
        let advResponseManufactureData_manufactureId = AppStorage.Get("advResponseManufactureData_manufactureId");
        let advResponseManufactureData_manufactureValue = AppStorage.Get("advResponseManufactureData_manufactureValue");
        let advResponseServiceData_serviceUuid = AppStorage.Get("advResponseServiceData_serviceUuid");
        let advResponseServiceData_serviceValue = AppStorage.Get("advResponseServiceData_serviceValue");
        let advResponseServiceUuidList = [];
        let advResponseManufactureDataList = [];
        let advResponseServiceDataList = [];
        let advResponseManufactureDataInstance = {
            manufactureId: parseInt(advResponseManufactureData_manufactureId),
            manufactureValue: string2ArrayBuffer(advResponseManufactureData_manufactureValue)
        };
        let advResponseServiceDataInstance = {
            serviceUuid: advResponseServiceData_serviceUuid,
            serviceValue: string2ArrayBuffer(advResponseServiceData_serviceValue)
        };
        let curAdvResponse = {
            serviceUuids: advResponseServiceUuidList.push(advResponseServiceUuids_serviceUuid),
            manufactureData: advResponseManufactureDataList.push(advResponseManufactureDataInstance),
            serviceData: advResponseServiceDataList.push(advResponseServiceDataInstance)
        };
        let setting = curAdvSettings;
        let advData = curAdvData;
        let advResponse = curAdvResponse;
        let cbxAdvResponse = AppStorage.Get("cbxAdvResponse");
        message = "startAdvertising:build input parameter Out!";
        if (!cbxAdvResponse) {
            gattServerInstance.startAdvertising(setting, advData);
        }
        else {
            gattServerInstance.startAdvertising(setting, advData, advResponse);
        }
        message = "startAdvertising succeeds!" + cbxAdvSettings + cbxAdvResponse;
    }
    else if (btState === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
        message = "BT is not enabled!";
    }
    else {
        message = "switching,please wait!";
    }
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testStartAdvertising = testStartAdvertising;
function testStopAdvertising() {
    let message = "stopAdvertising";
    let btState = _ohos_bluetooth_1.getState();
    if (btState == _ohos_bluetooth_1.BluetoothState.STATE_ON) {
        if (!gattServerInstance) {
            message = "NO_GATT_Server_OBJECT";
            _system_prompt_1.showToast({ message: message });
            return message;
        }
        gattServerInstance.stopAdvertising();
        message = "stopAdvertising() succeeds.";
    }
    else if (btState == _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
        message = "BT is not enabled!";
    }
    else {
        message = "switching,please wait!";
    }
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testStopAdvertising = testStopAdvertising;
function testGattServerClose() {
    let message = "GattServer closed";
    let btState = _ohos_bluetooth_1.getState();
    if (btState == _ohos_bluetooth_1.BluetoothState.STATE_ON) {
        if (!gattServerInstance) {
            message = "NO_GATT_Server_OBJECT";
            _system_prompt_1.showToast({ message: message });
            return message;
        }
        gattServerInstance.close();
        message = "GattServer closed: succeeds";
    }
    else if (btState == _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
        message = "BT is not enabled!";
    }
    else {
        message = "switching,please wait!";
    }
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testGattServerClose = testGattServerClose;
//回调函数
let connectStateChange = false;
function testOnConnectStateChange() {
    let message = "connectStateChange!";
    let btStatus = _ohos_bluetooth_1.getState();
    if (btStatus === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
        message = "BT is not enabled!";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    if (connectStateChange) {
        gattServerInstance.off("connectStateChange", () => {
        });
        message = "ConnectStateChange is off!";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    else {
        gattServerInstance.on('connectStateChange', (data) => {
            connectStateMsg = "deviceId:" + data.deviceId + ",connectState:" + data.state;
            AppStorage.SetOrCreate("connect_StateMsg", data.state);
            _system_prompt_1.showToast({ message: connectStateMsg });
        });
        message = 'onConnectStateChange is on!';
    }
    connectStateChange = !connectStateChange;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testOnConnectStateChange = testOnConnectStateChange;
//回调函数
let characteristicRead = false;
function testOnCharacteristicRead() {
    let message = "characteristicRead!";
    let btStatus = _ohos_bluetooth_1.getState();
    if (btStatus === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
        message = "BT is not enabled!";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    if (characteristicRead) {
        gattServerInstance.off("characteristicRead", () => {
        });
        message = "characteristicRead is off!";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    else {
        gattServerInstance.on('characteristicRead', (characteristicReadReq) => {
            let characteristicReadMsg = 'characteristicRead received!';
            // client address
            let clientDeviceId = characteristicReadReq.deviceId;
            // transId
            let transId = characteristicReadReq.transId;
            // the byte offset of the start position for reading characteristic value
            let offset = characteristicReadReq.offset;
            // service/characteristic Uuid
            let characteristicUuid = characteristicReadReq.characteristicUuid;
            let serviceUuid = characteristicReadReq.serviceUuid;
            characteristicReadMsg += 'characteristicRead input parameters: deviceId = ' + clientDeviceId + '\n';
            characteristicReadMsg += 'transId = ' + transId + '\n';
            characteristicReadMsg += "characteristicUuid = " + characteristicUuid + '\n';
            characteristicReadMsg += "serviceUuid = " + serviceUuid + '\n';
            AppStorage.SetOrCreate("characteristicReadMsg", characteristicReadMsg); // build data for response
            let arrayBufferCCC = string2ArrayBuffer('dayu');
            let serverResponse = {
                deviceId: clientDeviceId,
                transId: transId,
                status: 0,
                offset: offset,
                value: arrayBufferCCC
            };
            let ret = gattServerInstance.sendResponse(serverResponse);
            message = 'characteristicRead sendResponse:' + ret;
            _system_prompt_1.showToast({ message: message });
        });
        message = 'characteristicRead is on!';
    }
    characteristicRead = !characteristicRead;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testOnCharacteristicRead = testOnCharacteristicRead;
//回调函数
let characteristicWrite = false;
function testOnCharacteristicWrite() {
    let message = "notifyCharacteristicChanged successfully";
    let btStatus = _ohos_bluetooth_1.getState();
    if (btStatus === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
        message = "BT is not enabled!";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    if (characteristicWrite) {
        gattServerInstance.off("characteristicWrite", () => { });
        message = "characteristicWrite is off!";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    else {
        gattServerInstance.on('characteristicWrite', (characteristicWriteReq) => {
            btOnCharacteristicWrite = 'off:characteristicWrite';
            characteristicWriteMsg = '';
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
            characteristicWriteMsg += "The characteristicWriteReq input parameters: deviceId =" + deviceId;
            characteristicWriteMsg += "transId =" + transId;
            characteristicWriteMsg += "offset =" + offset;
            characteristicWriteMsg += "isPrep =" + isPrep;
            characteristicWriteMsg += "needRsp =" + needRsp;
            characteristicWriteMsg += "value =" + JSON.stringify(value);
            characteristicWriteMsg += "characteristicUuid =" + characteristicUuid;
            characteristicWriteMsg += "serviceUuid =" + serviceUuid;
            AppStorage.SetOrCreate("characteristicWriteMsg", characteristicWriteMsg);
            let serverResponse = {
                deviceId: deviceId,
                transId: transId,
                status: 0,
                offset: offset,
                value: arrayBufferCCC
            };
            // sendResponse
            if (needRsp) {
                let ret = gattServerInstance.sendResponse(serverResponse);
                message = 'characteristicWrite sendResponse:' + ret;
                _system_prompt_1.showToast({ message: message });
            }
            notifyCharacteristic_notifyCharacteristic = {
                serviceUuid: serviceUuid,
                characteristicUuid: characteristicUuid,
                characteristicValue: string2ArrayBuffer("Value4notifyCharacteristic"),
                confirm: true //notification:true; indication:false;
            };
            let notifyCharacteristic = notifyCharacteristic_notifyCharacteristic;
            if (gattServerInstance.notifyCharacteristicChanged(deviceId, notifyCharacteristic)) {
                message = "notifyCharacteristicChanged successfully!";
            }
            else {
                message = "notifyCharacteristicChanged failed!";
            }
        });
        message = 'characteristicWrite is on';
    }
    characteristicWrite = !characteristicWrite;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testOnCharacteristicWrite = testOnCharacteristicWrite;
//回调函数
let descriptorRead = false;
function testOnDescriptorRead() {
    let message = "DescriptorRead";
    let btStatus = _ohos_bluetooth_1.getState();
    if (btStatus === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
        message = "BT is not enabled!";
        return message;
    }
    if (descriptorRead) {
        gattServerInstance.off("descriptorRead", () => { });
        message = 'descriptorRead is off!';
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    else {
        gattServerInstance.on('descriptorRead', (descriptorReadReq) => {
            btOnDescriptorRead = 'off:descriptorRead';
            descriptorReadMsg = "descriptorReadIn!!";
            let deviceId = descriptorReadReq.deviceId;
            let transId = descriptorReadReq.transId;
            /** Indicates the byte offset of the start position for reading characteristic value */
            let offset = descriptorReadReq.offset;
            let descriptorUuid = descriptorReadReq.descriptorUuid;
            let characteristicUuid = descriptorReadReq.characteristicUuid;
            let serviceUuid = descriptorReadReq.serviceUuid;
            let arrayBufferDesc = string2ArrayBuffer("Response4descriptorRead");
            let serverResponse = {
                deviceId: deviceId,
                transId: transId,
                status: 0,
                offset: offset,
                value: arrayBufferDesc
            };
            let ret = gattServerInstance.sendResponse(serverResponse);
            message = 'descriptorRead sendResponse:' + ret;
            descriptorReadMsg += message;
            AppStorage.SetOrCreate("descriptorReadMsg", descriptorReadMsg);
            _system_prompt_1.showToast({ message: message });
        });
        message = 'descriptorRead is on';
    }
    descriptorRead = !descriptorRead;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testOnDescriptorRead = testOnDescriptorRead;
//回调函数
let descriptorWrite = false;
function testOnDescriptorWrite() {
    let message = "descriptorWrite";
    let btStatus = _ohos_bluetooth_1.getState();
    if (btStatus === _ohos_bluetooth_1.BluetoothState.STATE_OFF) {
        message = "BT is not enabled!";
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    if (descriptorWrite) {
        gattServerInstance.off("descriptorWrite", () => { });
        message = 'descriptorWrite is off!';
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    else {
        gattServerInstance.on('descriptorWrite', (descriptorWriteReq) => {
            btOnDescriptorWrite = 'off:descriptorWrite';
            descriptorWriteMsg = "descriptorWriteIn";
            descriptorWriteMsg += "The descriptorWriteReq client address: deviceId is " + descriptorWriteReq.deviceId;
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
            let serverResponse = {
                deviceId: deviceId,
                transId: transId,
                status: 0,
                offset: offset,
                value: arrayBufferDesc
            };
            if (needRsp) {
                let ret = gattServerInstance.sendResponse(serverResponse);
                message = 'descriptorWrite sendResponse:' + ret;
                _system_prompt_1.showToast({ message: message });
            }
            descriptorWriteMsg += 'descriptorWrite Callback finished!';
        });
        message = 'descriptorWrite is on';
    }
    descriptorWrite = !descriptorWrite;
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testOnDescriptorWrite = testOnDescriptorWrite;
function string2ArrayBuffer(str) {
    let array = new Uint8Array(str.length);
    for (var i = 0; i < str.length; i++) {
        array[i] = str.charCodeAt(i);
    }
    return array.buffer;
}


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/profileInterface.ets":
/*!***********************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/profileInterface.ets ***!
  \***********************************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports) {

"use strict";

var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.testPanNetworkProfile = exports.testHidHostProfile = exports.testHandsFreeAudioGatewayProfile = exports.testA2dpSourceProfile = void 0;
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
var _system_router_1  = globalThis.requireNativeModule('system.router');
var _system_prompt_1  = isSystemplugin('prompt', 'system') ? globalThis.systemplugin.prompt : globalThis.requireNapi('prompt');
const PAGE_URI_GET_A2DP_SOURCE_PROFILE = 'pages/subManualApiTest/subProfileTest/a2dpSourceProfileTest';
const PAGE_URI_GET_HANDS_FREE_AUDIO_GATEWAY_PROFILE = 'pages/subManualApiTest/subProfileTest/handsFreeAudioGatewayProfileTest';
const PAGE_URI_GET_HID_HOST_PROFILE = 'pages/subManualApiTest/subProfileTest/hidHostProfileTest';
const PAGE_URI_GET_Pan_NETWORK_PROFILE = 'pages/subManualApiTest/subProfileTest/panNetworkProfileTest';
function testA2dpSourceProfile() {
    _system_router_1.push({ uri: PAGE_URI_GET_A2DP_SOURCE_PROFILE });
    let message = "跳转成功";
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testA2dpSourceProfile = testA2dpSourceProfile;
function testHandsFreeAudioGatewayProfile() {
    _system_router_1.push({ uri: PAGE_URI_GET_HANDS_FREE_AUDIO_GATEWAY_PROFILE });
    let message = "跳转成功";
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testHandsFreeAudioGatewayProfile = testHandsFreeAudioGatewayProfile;
function testHidHostProfile() {
    _system_router_1.push({ uri: PAGE_URI_GET_HID_HOST_PROFILE });
    let message = "跳转成功";
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testHidHostProfile = testHidHostProfile;
function testPanNetworkProfile() {
    _system_router_1.push({ uri: PAGE_URI_GET_Pan_NETWORK_PROFILE });
    let message = "跳转成功";
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testPanNetworkProfile = testPanNetworkProfile;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/sppClientInterface.ets":
/*!*************************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/sppClientInterface.ets ***!
  \*************************************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.testSppClientClose = exports.testSppClientReadOff = exports.testSppClientReadOn = exports.testSppClientWrite = exports.testSppClientConnectUnSafe = exports.testSppClientConnectSafe = void 0;
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
var _system_prompt_1  = isSystemplugin('prompt', 'system') ? globalThis.systemplugin.prompt : globalThis.requireNapi('prompt');
__webpack_require__(/*! ./bluetoothModel */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/bluetoothModel.ts");
var _ohos_bluetooth_1  = globalThis.requireNapi('bluetooth') || (isSystemplugin('bluetooth', 'ohos') ? globalThis.ohosplugin.bluetooth : isSystemplugin('bluetooth', 'system') ? globalThis.systemplugin.bluetooth : undefined);
//export function testSetSppBluetoothScanMode(mode: number, dur: number):String{
//  let message = "蓝牙还未使能"
////  let btEnable = AppStorage.get('bluetoothIsOn')
////  if (!btEnable) {
////    message = '蓝牙还未使能';
////    return;
////  }.
//  let ret = BluetoothModel.setBluetoothScanMode(4, 0);
//  if (ret) {
//    message = "设置成功"
//  } else {
//    message = "设置失败"
//  }
//  prompt.showToast({message: message})
//  return message
//}
let clientNumber = -1;
function testSppClientConnectSafe() {
    let message = "SppClientConnectSafe!";
    console.info('spp server socketId(safe mode) create! ');
    let uuid = AppStorage.Get('inputUuid');
    let deviceId = AppStorage.Get('inputDeviceId');
    console.info(MODEL + 'sppConnect: ' + deviceId);
    _ohos_bluetooth_1.sppConnect(deviceId, {
        uuid: uuid,
        secure: true,
        type: 0
    }, (code, socketID) => {
        if (code.code != 0) {
            message = "客户端连接失败!";
            console.info(MODEL + 'sppConnect callback socketNumber(safe mode): ' + code.code);
            _system_prompt_1.showToast({ message: message });
            return message;
        }
        acceptClientSocket(socketID);
        console.info(MODEL + 'sppConnect callback socketNumber(safe mode): ' + socketID);
        message = "客户端连接成功!";
        _system_prompt_1.showToast({ message: message });
        return message;
    });
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testSppClientConnectSafe = testSppClientConnectSafe;
let MODEL = "bluetoothModel:";
function testSppClientConnectUnSafe() {
    let message = "SppClientConnectUnSafe!";
    let uuid = AppStorage.Get('inputUuid');
    let deviceId = AppStorage.Get('inputDeviceId');
    _ohos_bluetooth_1.sppConnect(deviceId, {
        uuid: uuid,
        secure: false,
        type: 0
    }, (code, socketID) => {
        if (code.code != 0) {
            message = "客户端连接失败!";
            _system_prompt_1.showToast({ message: message });
            return message;
        }
        acceptClientSocket(socketID);
        console.info(MODEL + 'sppConnect callback socketNumber(non-safe mode): ' + socketID);
        message = "客户端连接成功!";
        _system_prompt_1.showToast({ message: message });
        return message;
    });
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testSppClientConnectUnSafe = testSppClientConnectUnSafe;
function strToArrayBuffer(str) {
    let array = new Uint8Array(str.length);
    for (var i = 0; i < str.length; i++) {
        array[i] = str.charCodeAt(i);
    }
    return array.buffer;
}
function testSppClientWrite() {
    let message = "SppClientWrite";
    console.info(MODEL + 'sppWrite clientNumber: ' + JSON.stringify(clientNumber));
    let writeData = AppStorage.Get("writeData");
    let writeArrayBuffer = strToArrayBuffer(writeData);
    const sppWriteSuccess = _ohos_bluetooth_1.sppWrite(clientNumber, writeArrayBuffer);
    if (sppWriteSuccess) {
        console.info('sppWrite successfully');
        message = "写入成功 ";
    }
    else {
        console.info('sppWrite failed');
        message = "写入失败";
    }
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testSppClientWrite = testSppClientWrite;
function testSppClientReadOn() {
    let message = "SppClientReadOn";
    _ohos_bluetooth_1.on('sppRead', clientNumber, (dataBuffer) => {
        const data = new Uint8Array(dataBuffer);
        console.info('spp bluetooth read is: ' + data);
        if (data != null) {
            message = "读取成功" + JSON.stringify(data);
        }
        else {
            message = "读取失败";
        }
        AppStorage.SetOrCreate("dataReadMessage", message);
        _system_prompt_1.showToast({ message: message });
        return message;
    });
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testSppClientReadOn = testSppClientReadOn;
function testSppClientReadOff() {
    let message = "SppClientReadOff";
    _ohos_bluetooth_1.off('sppRead', clientNumber, (dataBuffer) => {
        const data = new Uint8Array(dataBuffer);
        console.info('spp bluetooth read is: ' + data);
        if (data != null) {
            message = "读取成功" + JSON.stringify(data);
        }
        else {
            message = "读取失败";
        }
        AppStorage.SetOrCreate("dataReadMessage", message);
        _system_prompt_1.showToast({ message: message });
        return message;
    });
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testSppClientReadOff = testSppClientReadOff;
function testSppClientClose() {
    let message = "SppClientClose";
    sppClientClose();
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testSppClientClose = testSppClientClose;
function acceptClientSocket(number) {
    console.info('spp clientSocket socketID: ' + number);
    // 获取的clientNumber用作服务端后续读/写操作socket的id。
    clientNumber = number;
}
function sppClientClose() {
    console.info(MODEL + 'sppCloseClientSocket clientNumber: ' + JSON.stringify(clientNumber));
    _ohos_bluetooth_1.sppCloseClientSocket(clientNumber);
    let message = '关闭成功 clientNumber' + clientNumber;
    _system_prompt_1.showToast({ message: message });
    return message;
}


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/sppInterface.ets":
/*!*******************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/sppInterface.ets ***!
  \*******************************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports) {

"use strict";

var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.testSppServer = exports.testSppClient = void 0;
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
var _system_router_1  = globalThis.requireNativeModule('system.router');
var _system_prompt_1  = isSystemplugin('prompt', 'system') ? globalThis.systemplugin.prompt : globalThis.requireNapi('prompt');
const PAGE_URI_GET_SPP_CLIENT_TEST = 'pages/subManualApiTest/subSppTest/sppClientTest';
const PAGE_URI_GET_SPP_SERVER_TEST = 'pages/subManualApiTest/subSppTest/sppServerTest';
function testSppClient() {
    _system_router_1.push({ uri: PAGE_URI_GET_SPP_CLIENT_TEST });
    let message = "跳转成功";
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testSppClient = testSppClient;
function testSppServer() {
    _system_router_1.push({ uri: PAGE_URI_GET_SPP_SERVER_TEST });
    let message = "跳转成功";
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testSppServer = testSppServer;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/sppServerInterface.ets":
/*!*************************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/sppServerInterface.ets ***!
  \*************************************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.testSppCloseClientSocket = exports.testSppServerClose = exports.testSppServerReadOff = exports.testSppServerReadOn = exports.testSppServerWrite = exports.testSppServerAccept = exports.testSppServerCreateUnSafe = exports.testSppServerCreateSafe = void 0;
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
var _system_prompt_1  = isSystemplugin('prompt', 'system') ? globalThis.systemplugin.prompt : globalThis.requireNapi('prompt');
var _ohos_bluetooth_1  = globalThis.requireNapi('bluetooth') || (isSystemplugin('bluetooth', 'ohos') ? globalThis.ohosplugin.bluetooth : isSystemplugin('bluetooth', 'system') ? globalThis.systemplugin.bluetooth : undefined);
__webpack_require__(/*! @ohos.bluetooth */ "../../api/@ohos.bluetooth.d.ts");
function testSppServerCreateSafe() {
    let message = "SPP_Server_Connect_Safe";
    console.info('spp server socketId(safe mode) create! ');
    let uuid = AppStorage.Get('inputUuid');
    if (uuid == '') {
        message = 'uuid为空';
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    // 创建蓝牙服务器侦听套接字, 返回服务端ID
    _ohos_bluetooth_1.sppListen('BluetoothDevice', { uuid: uuid, secure: true, type: 0 }, (code, serverSocketID) => {
        console.info('bluetooth error code: ' + code.code);
        if (code.code == 0) {
            console.info('spp server: serverSocketID(safe mode) - ' + serverSocketID);
            serverNumber = serverSocketID;
            message = "创建server成功";
            _system_prompt_1.showToast({ message: message });
            return message;
        }
    });
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testSppServerCreateSafe = testSppServerCreateSafe;
let serverNumber = -1;
function testSppServerCreateUnSafe() {
    let message = "SPP_Server_Connect_UnSafe";
    let uuid = AppStorage.Get('inputUuid');
    console.info('spp server socketId(non-safe mode) create! ');
    if (uuid == '') {
        message = 'uuid为空';
        _system_prompt_1.showToast({ message: message });
        return message;
    }
    // 创建蓝牙服务器侦听套接字, 返回服务端ID
    _ohos_bluetooth_1.sppListen('BluetoothDevice', { uuid: uuid, secure: false, type: 0 }, (code, serverSocketID) => {
        console.info('bluetooth error code: ' + code.code);
        if (code.code == 0) {
            console.info('spp server: serverSocketID(non-safe mode) - ' + serverSocketID);
            serverNumber = serverSocketID;
            message = "创建server成功";
        }
    });
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testSppServerCreateUnSafe = testSppServerCreateUnSafe;
let clientNumber = -1;
function testSppServerAccept() {
    let message = "等待客户端连接";
    _ohos_bluetooth_1.sppAccept(serverNumber, (code, clientSocketID) => {
        if (code.code == 0) {
            console.info('spp server: clientSocketID - ' + clientSocketID);
            clientNumber = clientSocketID;
            message = "接受客户端成功";
            _ohos_bluetooth_1.on('sppRead', clientNumber, (dataBuffer) => {
                const data = new Uint8Array(dataBuffer);
                console.info('spp bluetooth read is: ' + data);
                if (data != null) {
                    message = "读取成功" + JSON.stringify(data);
                }
                else {
                    message = "读取失败";
                }
                AppStorage.SetOrCreate("dataReadMessage", message);
            });
        }
        _system_prompt_1.showToast({ message: message });
        return message;
    });
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testSppServerAccept = testSppServerAccept;
function testSppServerWrite() {
    let message = "SPP_Server_Write";
    console.info(MODEL + 'sppWrite clientNumber: ' + JSON.stringify(clientNumber));
    let writeData = AppStorage.Get("writeData");
    let writeArrayBuffer = strToArrayBuffer(writeData);
    const sppWriteSuccess = _ohos_bluetooth_1.sppWrite(clientNumber, writeArrayBuffer);
    if (sppWriteSuccess) {
        console.info('sppWrite successfully');
        message = "写入成功 ";
    }
    else {
        console.info('sppWrite failed');
        message = "写入失败";
    }
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testSppServerWrite = testSppServerWrite;
function testSppServerReadOn() {
    let message = "bluetooth.on('sppRead')";
    _ohos_bluetooth_1.on('sppRead', clientNumber, (dataBuffer) => {
        const data = new Uint8Array(dataBuffer);
        console.info('spp bluetooth read is: ' + data);
        if (data != null) {
            message = "读取成功" + JSON.stringify(data);
        }
        else {
            message = "读取失败";
        }
        AppStorage.SetOrCreate("dataReadMessage", message);
        _system_prompt_1.showToast({ message: message });
        return message;
    });
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testSppServerReadOn = testSppServerReadOn;
function testSppServerReadOff() {
    let message = "bluetooth.off('sppRead')";
    _ohos_bluetooth_1.off('sppRead', clientNumber, (dataBuffer) => {
        const data = new Uint8Array(dataBuffer);
        console.info('spp bluetooth read is: ' + data);
        if (data != null) {
            message = "读取成功" + JSON.stringify(data);
        }
        else {
            message = "读取失败";
        }
        AppStorage.SetOrCreate("dataReadMessage", message);
        _system_prompt_1.showToast({ message: message });
        return message;
    });
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testSppServerReadOff = testSppServerReadOff;
//关闭服务端serversocket
let MODEL = "bluetoothModel:";
function testSppServerClose() {
    let message = "SppServerClose";
    console.info(MODEL + 'sppCloseServerSocket serverNumber: ' + serverNumber);
    _ohos_bluetooth_1.sppCloseServerSocket(serverNumber);
    message = "关闭服务端";
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testSppServerClose = testSppServerClose;
//关闭服务端accept回调的clientsocket
function testSppCloseClientSocket() {
    let message = "SppCloseClientSocket";
    console.info(MODEL + 'sppCloseClientSocket serverNumber: ' + clientNumber);
    _ohos_bluetooth_1.sppCloseClientSocket(clientNumber);
    message = "关闭客户端";
    _system_prompt_1.showToast({ message: message });
    return message;
}
exports.testSppCloseClientSocket = testSppCloseClientSocket;
function strToArrayBuffer(str) {
    let array = new Uint8Array(str.length);
    for (var i = 0; i < str.length; i++) {
        array[i] = str.charCodeAt(i);
    }
    return array.buffer;
}


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/testData.ets":
/*!***************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/testData.ets ***!
  \***************************************************************************************************************************/
/***/ ((__unused_webpack_module, exports) => {

"use strict";

Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.TestAuto = exports.AutoCategory = exports.TestApi = exports.CaseCategory = exports.TestData = exports.Category = void 0;
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
    Category[Category["GattAutoTest"] = 4] = "GattAutoTest";
    Category[Category["SppAutoTest"] = 5] = "SppAutoTest";
    Category[Category["ProfileAutoTest"] = 6] = "ProfileAutoTest";
    Category[Category["Profile"] = 7] = "Profile";
    Category[Category["BenchMark"] = 8] = "BenchMark";
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
    CaseCategory[CaseCategory["GattClient"] = 5] = "GattClient";
    CaseCategory[CaseCategory["GattServer"] = 6] = "GattServer";
    CaseCategory[CaseCategory["SPPClient"] = 7] = "SPPClient";
    CaseCategory[CaseCategory["SPPServer"] = 8] = "SPPServer";
    CaseCategory[CaseCategory["ProfileA2dpSink"] = 9] = "ProfileA2dpSink";
    CaseCategory[CaseCategory["ProfileA2dpSource"] = 10] = "ProfileA2dpSource";
    CaseCategory[CaseCategory["ProfileAvrcpCT"] = 11] = "ProfileAvrcpCT";
    CaseCategory[CaseCategory["ProfileAvrcpTG"] = 12] = "ProfileAvrcpTG";
    CaseCategory[CaseCategory["ProfileHandsFreeAudioGateway"] = 13] = "ProfileHandsFreeAudioGateway";
    CaseCategory[CaseCategory["ProfileHandsFreeUnit"] = 14] = "ProfileHandsFreeUnit";
    CaseCategory[CaseCategory["ProfileHitHost"] = 15] = "ProfileHitHost";
    CaseCategory[CaseCategory["ProfilePanNetwork"] = 16] = "ProfilePanNetwork";
    CaseCategory[CaseCategory["ProfilePbapClient"] = 17] = "ProfilePbapClient";
    CaseCategory[CaseCategory["ProfilePbapServer"] = 18] = "ProfilePbapServer";
    CaseCategory[CaseCategory["GattAutoTest"] = 19] = "GattAutoTest";
    CaseCategory[CaseCategory["SppAutoTest"] = 20] = "SppAutoTest";
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
var AutoCategory;
(function (AutoCategory) {
    AutoCategory[AutoCategory["BrAuto"] = 0] = "BrAuto";
    AutoCategory[AutoCategory["BleAuto"] = 1] = "BleAuto";
    AutoCategory[AutoCategory["GattClientAuto"] = 2] = "GattClientAuto";
    AutoCategory[AutoCategory["GattServerAuto"] = 3] = "GattServerAuto";
    AutoCategory[AutoCategory["SppClientAuto"] = 4] = "SppClientAuto";
    AutoCategory[AutoCategory["SppServerAuto"] = 5] = "SppServerAuto";
    AutoCategory[AutoCategory["A2dpSinkAuto"] = 6] = "A2dpSinkAuto";
    AutoCategory[AutoCategory["A2dpSourceAuto"] = 7] = "A2dpSourceAuto";
    AutoCategory[AutoCategory["AvrcpCTAuto"] = 8] = "AvrcpCTAuto";
    AutoCategory[AutoCategory["AvrcpTGAuto"] = 9] = "AvrcpTGAuto";
    AutoCategory[AutoCategory["HandsFreeAudioGatewayAuto"] = 10] = "HandsFreeAudioGatewayAuto";
    AutoCategory[AutoCategory["HandsFreeUnitAuto"] = 11] = "HandsFreeUnitAuto";
    AutoCategory[AutoCategory["HitHostAuto"] = 12] = "HitHostAuto";
    AutoCategory[AutoCategory["PanNetworkAuto"] = 13] = "PanNetworkAuto";
    AutoCategory[AutoCategory["PbapClientAuto"] = 14] = "PbapClientAuto";
    AutoCategory[AutoCategory["PbapServerAuto"] = 15] = "PbapServerAuto";
})(AutoCategory = exports.AutoCategory || (exports.AutoCategory = {}));
class TestAuto {
    constructor(name, detail, method, result, category) {
        this.id = `${NextId++}`;
        this.name = name;
        this.detail = detail;
        this.method = method;
        this.result = result;
        this.category = category;
    }
}
exports.TestAuto = TestAuto;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/testDataModels.ets":
/*!*********************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/testDataModels.ets ***!
  \*********************************************************************************************************************************/
/***/ ((__unused_webpack_module, exports, __webpack_require__) => {

"use strict";

Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.initProfileAutoTestData = exports.initSppAutoTestData = exports.initGattAutoTestData = exports.initPanNetworkProfileApiData = exports.initHitHostProfileApiData = exports.initHandsFreeAudioGatewayProfileApiData = exports.initA2dpSourceProfileApiData = exports.initSppServerApiData = exports.initSppClientApiData = exports.initGattServerApiData = exports.initGattClientApiData = exports.initProfileApiData = exports.initSppApiData = exports.initGattApiData = exports.initBleApiData = exports.initBRApiData = exports.initTestData = void 0;
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
const testData_1 = __webpack_require__(/*! ./testData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/testData.ets");
const brInterface_1 = __webpack_require__(/*! ./brInterface */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/brInterface.ets");
const bleInterface_1 = __webpack_require__(/*! ./bleInterface */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/bleInterface.ets");
const gattInterface_1 = __webpack_require__(/*! ./gattInterface */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/gattInterface.ets");
const sppInterface_1 = __webpack_require__(/*! ./sppInterface */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/sppInterface.ets");
const profileInterface_1 = __webpack_require__(/*! ./profileInterface */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/profileInterface.ets");
const gattClientInterface_1 = __webpack_require__(/*! ./gattClientInterface */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/gattClientInterface.ets");
const gattServerInterface_1 = __webpack_require__(/*! ./gattServerInterface */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/gattServerInterface.ets");
const sppClientInterface_1 = __webpack_require__(/*! ./sppClientInterface */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/sppClientInterface.ets");
const sppServerInterface_1 = __webpack_require__(/*! ./sppServerInterface */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/sppServerInterface.ets");
const a2dpSinkProfileInterface_1 = __webpack_require__(/*! ./a2dpSinkProfileInterface */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/a2dpSinkProfileInterface.ets");
__webpack_require__(/*! @ohos.bluetooth */ "../../api/@ohos.bluetooth.d.ts");
const BRTestCase = [
    { 'name': '开蓝牙', 'detail': '使能蓝牙', 'method': 'enableBluetooth', 'api': brInterface_1.testEnableBluetooth, 'result': 'None', 'category': testData_1.CaseCategory.BR },
    { 'name': '关蓝牙', 'detail': '去使能蓝牙', 'method': 'disableBluetooth', 'api': brInterface_1.testDisableBluetooth, 'result': 'None', 'category': testData_1.CaseCategory.BR },
    { 'name': '设置状态', 'detail': '设置蓝牙状态', 'method': 'getState', 'api': brInterface_1.testGetState, 'result': 'None', 'category': testData_1.CaseCategory.BR },
    { 'name': '获取当前连接状态', 'detail': '获取当前蓝牙连接状态', 'method': 'getBtConnectionState', 'api': brInterface_1.testGetBtConnectionState, 'result': 'None', 'category': testData_1.CaseCategory.BR },
    { 'name': '设置本地名称', 'detail': '设置蓝牙本地名称', 'method': 'setLocalName', 'api': brInterface_1.testSetLocalName, 'result': 'None', 'category': testData_1.CaseCategory.BR },
    { 'name': '获取本地名称', 'detail': '获取蓝牙本地名称', 'method': 'getLocalName', 'api': brInterface_1.testGetLocalName, 'result': 'None', 'category': testData_1.CaseCategory.BR },
    { 'name': '设置扫描模式', 'detail': '设置蓝牙扫描模式', 'method': 'setBluetoothScanMode', 'api': brInterface_1.testSetBluetoothScanMode, 'result': 'None', 'category': testData_1.CaseCategory.BR },
    { 'name': '获取扫描模式', 'detail': '获取蓝牙扫描模式', 'method': 'getBluetoothScanMode', 'api': brInterface_1.testGetBluetoothScanMode, 'result': 'None', 'category': testData_1.CaseCategory.BR },
    { 'name': '开始蓝牙发现', 'detail': '发现可配对蓝牙设备', 'method': 'startBluetoothDiscovery', 'api': brInterface_1.testStartBluetoothDiscovery, 'result': 'None', 'category': testData_1.CaseCategory.BR },
    { 'name': '停止蓝牙发现', 'detail': '停止发现', 'method': 'stopBluetoothDiscovery', 'api': brInterface_1.testStopBluetoothDiscovery, 'result': 'None', 'category': testData_1.CaseCategory.BR },
    { 'name': '配对设备', 'detail': '进行蓝牙设备配对', 'method': 'pairDevice', 'api': brInterface_1.testPairDevice, 'result': 'None', 'category': testData_1.CaseCategory.BR },
    { 'name': '取消配对设备', 'detail': '取消蓝牙设备配对', 'method': 'unpairDevice', 'api': brInterface_1.testCancelPairedDevice, 'result': 'None', 'category': testData_1.CaseCategory.BR },
    { 'name': '获取远程设备名称', 'detail': '获取配对蓝牙设备的名称', 'method': 'getDeviceName', 'api': brInterface_1.testGetRemoteDeviceName, 'result': 'None', 'category': testData_1.CaseCategory.BR },
    { 'name': '获取远程设备类型', 'detail': '获取配对蓝牙设备的类型', 'method': 'getDeviceType', 'api': brInterface_1.testGetRemoteDeviceClass, 'result': 'None', 'category': testData_1.CaseCategory.BR },
    { 'name': '获取配对设备', 'detail': '获取配对蓝牙设备的信息', 'method': 'getPairedDeviceIds', 'api': brInterface_1.testGetPairedDevices, 'result': 'None', 'category': testData_1.CaseCategory.BR },
    { 'name': '获取配置文件连接状态1', 'detail': '获取配置文件的连接状态', 'method': 'getProfileConnState', 'api': brInterface_1.testGetProfileConnState1, 'result': 'None', 'category': testData_1.CaseCategory.BR },
    { 'name': '获取配置文件连接状态2', 'detail': '获取设备的配置文件状态', 'method': 'getDeviceState', 'api': brInterface_1.testGetProfileConnState2, 'result': 'None', 'category': testData_1.CaseCategory.BR },
    { 'name': '注册状态变化', 'detail': '注册状态变化', 'method': 'on.stateChange', 'api': brInterface_1.testOnStateChange, 'result': 'None', 'category': testData_1.CaseCategory.BR },
    { 'name': '注册pin码变化', 'detail': '注册pin码变化', 'method': 'on.pinRequired', 'api': brInterface_1.testOnPinRequired, 'result': 'None', 'category': testData_1.CaseCategory.BR },
    { 'name': '注册绑定状态变化', 'detail': '注册绑定状态变化', 'method': 'on.bondStateChange', 'api': brInterface_1.testOnBondStateChange, 'result': 'None', 'category': testData_1.CaseCategory.BR }
];
const BleTestCase = [
    { 'name': '创建Gatt服务器', 'detail': '创建Gatt服务器', 'method': 'createGattServer', 'api': bleInterface_1.testCreateGattServerDevice, 'result': 'None', 'category': testData_1.CaseCategory.BLE },
    { 'name': '创建Gatt客户端', 'detail': '创建Gatt客户端', 'method': 'createGattClient', 'api': bleInterface_1.testCreateGattClientDevice, 'result': 'None', 'category': testData_1.CaseCategory.BLE },
    { 'name': '获取连接的BLE设备', 'detail': '获取连接的BLE设备信息', 'method': 'getConnectedBLEDevices', 'api': bleInterface_1.testGetConnectedBLEDevices, 'result': 'None', 'category': testData_1.CaseCategory.BLE },
    { 'name': '开始BLE扫描', 'detail': '开始扫描BLE设备', 'method': 'startBLEScan', 'api': bleInterface_1.testStartBLEScan, 'result': 'None', 'category': testData_1.CaseCategory.BLE },
    { 'name': '停止BLE扫描', 'detail': '停止扫描', 'method': 'stopBLEScan', 'api': bleInterface_1.testStopBLEScan, 'result': 'None', 'category': testData_1.CaseCategory.BLE },
    { 'name': '注册BLE查找状态变化', 'detail': '注册BLE设备查找状态变化', 'method': 'on.BLEDeviceFind', 'api': bleInterface_1.testOnBLEDeviceFind, 'result': 'None', 'category': testData_1.CaseCategory.BLE }
];
const GattTestCase = [
    { 'name': 'GattClient测试', 'detail': 'Gatt客户端测试', 'method': 'gattClient', 'api': gattInterface_1.testGattClient, 'result': 'None', 'category': testData_1.CaseCategory.Gatt },
    { 'name': 'GattServer测试', 'detail': 'Gatt服务器测试', 'method': 'gattServer', 'api': gattInterface_1.testGattServer, 'result': 'None', 'category': testData_1.CaseCategory.Gatt },
];
const SppTestCase = [
    { 'name': 'SppClient测试', 'detail': 'Spp客户端测试', 'method': 'sppClient', 'api': sppInterface_1.testSppClient, 'result': 'None', 'category': testData_1.CaseCategory.SPP },
    { 'name': 'SppServer测试', 'detail': 'Spp服务器测试', 'method': 'sppServer', 'api': sppInterface_1.testSppServer, 'result': 'None', 'category': testData_1.CaseCategory.SPP },
];
const ProfileTestCase = [
    { 'name': 'A2dpSource配置', 'detail': 'A2dpSource配置测试', 'method': 'A2dpSourceProfile', 'api': profileInterface_1.testA2dpSourceProfile, 'result': 'None', 'category': testData_1.CaseCategory.Profile },
    { 'name': 'HandsFreeAudioGateway配置', 'detail': 'HandsFreeAudioGateway配置测试', 'method': 'HandsFreeAudioGatewayProfile', 'api': profileInterface_1.testHandsFreeAudioGatewayProfile, 'result': 'None', 'category': testData_1.CaseCategory.Profile },
    { 'name': 'HidHost配置', 'detail': 'HidHost配置测试', 'method': 'HidHostProfile', 'api': profileInterface_1.testHidHostProfile, 'result': 'None', 'category': testData_1.CaseCategory.Profile },
    { 'name': 'PanNetwork配置', 'detail': 'PanNetwork配置测试', 'method': 'PanNetworkProfile', 'api': profileInterface_1.testPanNetworkProfile, 'result': 'None', 'category': testData_1.CaseCategory.Profile },
];
const GattClientTestCase = [
    { 'name': '创造Gatt客户端', 'detail': '创造Gatt客户端', 'method': 'createGattClient', 'api': gattClientInterface_1.testCreateGattClient, 'result': 'None', 'category': testData_1.CaseCategory.GattClient },
    { 'name': '连接', 'detail': 'Gatt连接', 'method': 'connect', 'api': gattClientInterface_1.testConnect, 'result': 'None', 'category': testData_1.CaseCategory.GattClient },
    { 'name': '取消连接', 'detail': '取消Gatt连接', 'method': 'disconnect', 'api': gattClientInterface_1.testDisconnect, 'result': 'None', 'category': testData_1.CaseCategory.GattClient },
    { 'name': 'BLE连接状态', 'detail': '查看BLE连接状态', 'method': 'on.BLEConnectionStateChange', 'api': gattClientInterface_1.testBLEConnectionStateChange, 'result': 'None', 'category': testData_1.CaseCategory.GattClient },
    { 'name': '关闭', 'detail': '关闭Gatt客户端', 'method': 'close', 'api': gattClientInterface_1.testGattClientClose, 'result': 'None', 'category': testData_1.CaseCategory.GattClient },
    { 'name': '获取服务callback', 'detail': '获取服务callback', 'method': 'getServices(Callback)', 'api': gattClientInterface_1.testGetServicesCallback, 'result': 'None', 'category': testData_1.CaseCategory.GattClient },
    { 'name': '获取服务promise', 'detail': '获取服务promise', 'method': 'getServices(promise)', 'api': gattClientInterface_1.testGetServicesPromise, 'result': 'None', 'category': testData_1.CaseCategory.GattClient },
    { 'name': '读取特征值callback', 'detail': '读取特征值callback', 'method': 'readCharacteristicValue(callback)', 'api': gattClientInterface_1.testReadCharacteristicValueCallback, 'result': 'None', 'category': testData_1.CaseCategory.GattClient },
    { 'name': '读取特征值promise', 'detail': '读取特征值promise', 'method': 'readCharacteristicValue(promise)', 'api': gattClientInterface_1.testReadCharacteristicValuePromise, 'result': 'None', 'category': testData_1.CaseCategory.GattClient },
    { 'name': '读取描述符值callback', 'detail': '读取描述符值callback', 'method': 'readDescriptorValue(callback)', 'api': gattClientInterface_1.testReadDescriptorValueCallback, 'result': 'None', 'category': testData_1.CaseCategory.GattClient },
    { 'name': '读取描述符值promise', 'detail': '读取描述符值promise', 'method': 'readDescriptorValue(promise)', 'api': gattClientInterface_1.testReadDescriptorValuePromise, 'result': 'None', 'category': testData_1.CaseCategory.GattClient },
    { 'name': '写入特征值', 'detail': '写入特征值', 'method': 'writeCharacteristicValue', 'api': gattClientInterface_1.testWriteCharacteristicValue, 'result': 'None', 'category': testData_1.CaseCategory.GattClient },
    { 'name': '写入描述符值', 'detail': '写入描述符值', 'method': 'writeDescriptorValue', 'api': gattClientInterface_1.testWriteDescriptorValue, 'result': 'None', 'category': testData_1.CaseCategory.GattClient },
    { 'name': 'BLE特征值变化', 'detail': '查看BLE特征值变化', 'method': 'on.BLECharacteristicChange', 'api': gattClientInterface_1.testBLECharacteristicChange, 'result': 'None', 'category': testData_1.CaseCategory.GattClient },
    { 'name': '设定BLEMtu尺寸', 'detail': '设定BLEMtu尺寸', 'method': 'setBLEMtuSize', 'api': gattClientInterface_1.testSetBLEMtuSize, 'result': 'None', 'category': testData_1.CaseCategory.GattClient },
    { 'name': '设置通知特性更改', 'detail': '设置通知特性更改', 'method': 'setNotifyCharacteristicChanged', 'api': gattClientInterface_1.testSetNotifyCharacteristicChanged, 'result': 'None', 'category': testData_1.CaseCategory.GattClient },
    { 'name': '获取设备名称callback', 'detail': '获取设备名称callback', 'method': 'getDeviceName()callback', 'api': gattClientInterface_1.testGetDeviceNameCallback, 'result': 'None', 'category': testData_1.CaseCategory.GattClient },
    { 'name': '获取设备名称promise', 'detail': '获取设备名称promise', 'method': 'getDeviceName()Promise', 'api': gattClientInterface_1.testGetDeviceNamePromise, 'result': 'None', 'category': testData_1.CaseCategory.GattClient },
    { 'name': '获取Rssi值callback', 'detail': '获取Rssi值callback', 'method': 'getRssiValue()Callback', 'api': gattClientInterface_1.testGetRssiValueCallback, 'result': 'None', 'category': testData_1.CaseCategory.GattClient },
    { 'name': '获取Rssi值promise', 'detail': '获取Rssi值promise', 'method': 'getRssiValue()Promise', 'api': gattClientInterface_1.testGetRssiValuePromise, 'result': 'None', 'category': testData_1.CaseCategory.GattClient },
];
const GattServerTestCase = [
    { 'name': '创建Gatt服务器', 'detail': '创建Gatt服务器', 'method': 'createGattServer', 'api': gattServerInterface_1.testCreateGattServer, 'result': 'None', 'category': testData_1.CaseCategory.GattServer },
    { 'name': '添加服务', 'detail': '添加Gatt服务', 'method': 'addService', 'api': gattServerInterface_1.testAddService, 'result': 'None', 'category': testData_1.CaseCategory.GattServer },
    { 'name': '删除服务', 'detail': '删除Gatt服务', 'method': 'removeService', 'api': gattServerInterface_1.testRemoveService, 'result': 'None', 'category': testData_1.CaseCategory.GattServer },
    { 'name': '开始广播', 'detail': '开始广播播报', 'method': 'startAdvertising', 'api': gattServerInterface_1.testStartAdvertising, 'result': 'None', 'category': testData_1.CaseCategory.GattServer },
    { 'name': '停止广播', 'detail': '停止广播播报', 'method': 'stopAdvertising', 'api': gattServerInterface_1.testStopAdvertising, 'result': 'None', 'category': testData_1.CaseCategory.GattServer },
    { 'name': '关闭', 'detail': '关闭Gatt服务器', 'method': 'close', 'api': gattServerInterface_1.testGattServerClose, 'result': 'None', 'category': testData_1.CaseCategory.GattServer },
    { 'name': '连接状态', 'detail': '查看Gatt连接状态', 'method': 'on.connectStateChange', 'api': gattServerInterface_1.testOnConnectStateChange, 'result': 'None', 'category': testData_1.CaseCategory.GattServer },
    { 'name': '读取特征值', 'detail': '读取特征值', 'method': 'on.characteristicRead', 'api': gattServerInterface_1.testOnCharacteristicRead, 'result': 'None', 'category': testData_1.CaseCategory.GattServer },
    { 'name': '写入特征值', 'detail': '写入特征值', 'method': 'on.characteristicWrite', 'api': gattServerInterface_1.testOnCharacteristicWrite, 'result': 'None', 'category': testData_1.CaseCategory.GattServer },
    { 'name': '读取描述符', 'detail': '读取描述符', 'method': 'on.descriptorRead', 'api': gattServerInterface_1.testOnDescriptorRead, 'result': 'None', 'category': testData_1.CaseCategory.GattServer },
    { 'name': '写入描述符', 'detail': '写入描述符', 'method': 'on.descriptorWrite', 'api': gattServerInterface_1.testOnDescriptorWrite, 'result': 'None', 'category': testData_1.CaseCategory.GattServer },
];
const SppClientTestCase = [
    { 'name': '开蓝牙', 'detail': '使能蓝牙', 'method': 'enableBluetooth', 'api': brInterface_1.testEnableBluetooth, 'result': 'None', 'category': testData_1.CaseCategory.SPPClient },
    { 'name': '关蓝牙', 'detail': '去使能蓝牙', 'method': 'disableBluetooth', 'api': brInterface_1.testDisableBluetooth, 'result': 'None', 'category': testData_1.CaseCategory.SPPClient },
    { 'name': '设置SPP扫描模式', 'detail': '设置SPP扫描模式', 'method': 'setBluetoothScanMode', 'api': brInterface_1.testSetBluetoothScanMode, 'result': 'None', 'category': testData_1.CaseCategory.SPPClient },
    { 'name': '设置状态', 'detail': '设置蓝牙状态', 'method': 'getState', 'api': brInterface_1.testGetState, 'result': 'None', 'category': testData_1.CaseCategory.SPPClient },
    { 'name': '获取扫描模式', 'detail': '获取蓝牙扫描模式', 'method': 'getBluetoothScanMode', 'api': brInterface_1.testGetBluetoothScanMode, 'result': 'None', 'category': testData_1.CaseCategory.SPPClient },
    { 'name': '获取当前连接状态', 'detail': '获取当前蓝牙连接状态', 'method': 'getBtConnectionState', 'api': brInterface_1.testGetBtConnectionState, 'result': 'None', 'category': testData_1.CaseCategory.SPPClient },
    { 'name': 'Spp客户端连接(安全模式)', 'detail': 'Spp客户端(安全模式)连接', 'method': 'sppConnect', 'api': sppClientInterface_1.testSppClientConnectSafe, 'result': 'None', 'category': testData_1.CaseCategory.SPPClient },
    { 'name': 'Spp客户端连接(非安全模式)', 'detail': 'Spp客户端(非安全模式)连接', 'method': 'sppConnect', 'api': sppClientInterface_1.testSppClientConnectUnSafe, 'result': 'None', 'category': testData_1.CaseCategory.SPPClient },
    { 'name': '配对设备', 'detail': '进行蓝牙设备配对', 'method': 'pairDevice', 'api': brInterface_1.testPairDevice, 'result': 'None', 'category': testData_1.CaseCategory.SPPClient },
    { 'name': 'Spp客户端写入', 'detail': 'Spp客户端写入', 'method': 'sppWrite', 'api': sppClientInterface_1.testSppClientWrite, 'result': 'None', 'category': testData_1.CaseCategory.SPPClient },
    { 'name': 'Spp客户端读取打开', 'detail': 'Spp客户端读取打开', 'method': 'on.sppRead', 'api': sppClientInterface_1.testSppClientReadOn, 'result': 'None', 'category': testData_1.CaseCategory.SPPClient },
    { 'name': 'Spp客户端读取关闭', 'detail': 'Spp客户端读取关闭', 'method': 'off.sppRead', 'api': sppClientInterface_1.testSppClientReadOff, 'result': 'None', 'category': testData_1.CaseCategory.SPPClient },
    { 'name': 'Spp客户端关闭', 'detail': 'Spp客户端关闭', 'method': 'sppCloseClientSocket', 'api': sppClientInterface_1.testSppClientClose, 'result': 'None', 'category': testData_1.CaseCategory.SPPClient },
];
const SppServerTestCase = [
    { 'name': '开蓝牙', 'detail': '使能蓝牙', 'method': 'enableBluetooth', 'api': brInterface_1.testEnableBluetooth, 'result': 'None', 'category': testData_1.CaseCategory.SPPServer },
    { 'name': '关蓝牙', 'detail': '去使能蓝牙', 'method': 'disableBluetooth', 'api': brInterface_1.testDisableBluetooth, 'result': 'None', 'category': testData_1.CaseCategory.SPPServer },
    { 'name': '设置SPP扫描模式', 'detail': '设置SPP扫描模式', 'method': 'setBluetoothScanMode', 'api': brInterface_1.testSetBluetoothScanMode, 'result': 'None', 'category': testData_1.CaseCategory.SPPServer },
    { 'name': '设置状态', 'detail': '设置蓝牙状态', 'method': 'getState', 'api': brInterface_1.testGetState, 'result': 'None', 'category': testData_1.CaseCategory.SPPServer },
    { 'name': '获取扫描模式', 'detail': '获取蓝牙扫描模式', 'method': 'getBluetoothScanMode', 'api': brInterface_1.testGetBluetoothScanMode, 'result': 'None', 'category': testData_1.CaseCategory.SPPServer },
    { 'name': '获取当前连接状态', 'detail': '获取当前蓝牙连接状态', 'method': 'getBtConnectionState', 'api': brInterface_1.testGetBtConnectionState, 'result': 'None', 'category': testData_1.CaseCategory.SPPServer },
    { 'name': 'Spp服务器创建(安全模式)', 'detail': 'Spp服务器(安全模式)创建', 'method': 'sppListen', 'api': sppServerInterface_1.testSppServerCreateSafe, 'result': 'None', 'category': testData_1.CaseCategory.SPPServer },
    { 'name': 'Spp服务器创建(非安全模式)', 'detail': 'Spp服务器(非安全模式)创建', 'method': 'sppListen', 'api': sppServerInterface_1.testSppServerCreateUnSafe, 'result': 'None', 'category': testData_1.CaseCategory.SPPServer },
    { 'name': '注册pin码变化', 'detail': '注册pin码变化', 'method': 'on.pinRequired', 'api': brInterface_1.testOnPinRequired, 'result': 'None', 'category': testData_1.CaseCategory.SPPServer },
    //  { 'name': 'SPP客户端写入', 'detail': '使能蓝牙', 'method': 'spp_Client_Write', 'api': testSPP_Client_Write, 'result': 'None', 'category': CaseCategory.SPPServer },
    { 'name': 'Spp服务器写入', 'detail': 'Spp服务器写入', 'method': 'sppWrite', 'api': sppServerInterface_1.testSppServerWrite, 'result': 'None', 'category': testData_1.CaseCategory.SPPServer },
    { 'name': 'Spp服务器接受', 'detail': 'Spp服务器接受', 'method': 'sppAccept', 'api': sppServerInterface_1.testSppServerAccept, 'result': 'None', 'category': testData_1.CaseCategory.SPPServer },
    { 'name': 'Spp服务器读取打开', 'detail': 'Spp服务器读取打开', 'method': 'on.sppRead', 'api': sppServerInterface_1.testSppServerReadOn, 'result': 'None', 'category': testData_1.CaseCategory.SPPServer },
    { 'name': 'Spp服务器读取关闭', 'detail': 'Spp服务器读取关闭', 'method': 'off.sppRead', 'api': sppServerInterface_1.testSppServerReadOff, 'result': 'None', 'category': testData_1.CaseCategory.SPPServer },
    { 'name': 'Spp服务器关闭', 'detail': 'Spp服务器关闭', 'method': 'sppCloseServerSocket', 'api': sppServerInterface_1.testSppServerClose, 'result': 'None', 'category': testData_1.CaseCategory.SPPServer },
    { 'name': 'Spp客户端关闭', 'detail': 'Spp客户端关闭', 'method': 'sppCloseClientSocket', 'api': sppServerInterface_1.testSppCloseClientSocket, 'result': 'None', 'category': testData_1.CaseCategory.SPPServer },
];
//const A2dpSinkProfileTestCase: any[]  = [
//  { 'name': '连接', 'detail': 'Profile连接', 'method': 'connect', 'api': testProfileConnect, 'result': 'None', 'category': CaseCategory.ProfileA2dpSink },
//  { 'name': '取消连接', 'detail': '取消Profile连接', 'method': 'disconnect', 'api': testProfileDisConnect, 'result': 'None', 'category': CaseCategory.ProfileA2dpSink },
//  { 'name': '获取播放状态', 'detail': '获取Profile播放状态', 'method': 'getPlayingState', 'api': testProfileGetPlayingState, 'result': 'None', 'category': CaseCategory.ProfileA2dpSink },
//  { 'name': '获取连接设备', 'detail': '获取Profile连接设备', 'method': 'getConnectionDevices', 'api': testProfileGetConnectionDevices, 'result': 'None', 'category': CaseCategory.ProfileA2dpSink },
//  { 'name': '获取设备状态', 'detail': '获取Profile设备状态', 'method': 'getDeviceState', 'api': testProfileGetDeviceState, 'result': 'None', 'category': CaseCategory.ProfileA2dpSink },
//]
const A2dpSourceProfileTestCase = [
    { 'name': '连接', 'detail': 'Profile连接', 'method': 'connect', 'api': a2dpSinkProfileInterface_1.testProfileConnect, 'result': 'None', 'category': testData_1.CaseCategory.ProfileA2dpSource },
    { 'name': '取消连接', 'detail': '取消Profile连接', 'method': 'disconnect', 'api': a2dpSinkProfileInterface_1.testProfileDisConnect, 'result': 'None', 'category': testData_1.CaseCategory.ProfileA2dpSource },
    { 'name': '获取播放状态', 'detail': '获取Profile播放状态', 'method': 'getPlayingState', 'api': a2dpSinkProfileInterface_1.testProfileGetPlayingState, 'result': 'None', 'category': testData_1.CaseCategory.ProfileA2dpSource },
    { 'name': '获取连接设备', 'detail': '获取Profile连接设备', 'method': 'getConnectionDevices', 'api': a2dpSinkProfileInterface_1.testProfileGetConnectionDevices, 'result': 'None', 'category': testData_1.CaseCategory.ProfileA2dpSource },
    { 'name': '获取设备状态', 'detail': '获取Profile设备状态', 'method': 'getDeviceState', 'api': a2dpSinkProfileInterface_1.testProfileGetDeviceState, 'result': 'None', 'category': testData_1.CaseCategory.ProfileA2dpSource },
];
//const AvrcpCTProfileTestCase:any []=[
//  { 'name': '连接', 'detail': 'Profile连接', 'method': 'connect', 'api': testProfileConnect, 'result': 'None', 'category': CaseCategory.ProfileAvrcpCT },
//  { 'name': '取消连接', 'detail': '取消Profile连接', 'method': 'disconnect', 'api': testProfileDisConnect, 'result': 'None', 'category': CaseCategory.ProfileAvrcpCT },
//  { 'name': '获取连接设备', 'detail': '获取Profile连接设备', 'method': 'getConnectionDevices', 'api': testProfileGetConnectionDevices, 'result': 'None', 'category': CaseCategory.ProfileAvrcpCT },
//  { 'name': '获取设备状态', 'detail': '获取Profile设备状态', 'method': 'getDeviceState', 'api': testProfileGetDeviceState, 'result': 'None', 'category': CaseCategory.ProfileAvrcpCT },
//]
//
//const AvrcpTGProfileTestCase:any []=[
//  { 'name': '连接', 'detail': 'Profile连接', 'method': 'connect', 'api': testProfileConnect, 'result': 'None', 'category': CaseCategory.ProfileAvrcpTG },
//  { 'name': '取消连接', 'detail': '取消Profile连接', 'method': 'disconnect', 'api': testProfileDisConnect, 'result': 'None', 'category': CaseCategory.ProfileAvrcpTG },
//  { 'name': '获取连接设备', 'detail': '获取Profile连接设备', 'method': 'getConnectionDevices', 'api': testProfileGetConnectionDevices, 'result': 'None', 'category': CaseCategory.ProfileAvrcpTG },
//  { 'name': '获取设备状态', 'detail': '获取Profile设备状态', 'method': 'getDeviceState', 'api': testProfileGetDeviceState, 'result': 'None', 'category': CaseCategory.ProfileAvrcpTG },
//]
const HandsFreeAudioGatewayProfileTestCase = [
    { 'name': '连接', 'detail': 'Profile连接', 'method': 'connect', 'api': a2dpSinkProfileInterface_1.testProfileConnect, 'result': 'None', 'category': testData_1.CaseCategory.ProfileHandsFreeAudioGateway },
    { 'name': '取消连接', 'detail': '取消Profile连接', 'method': 'disconnect', 'api': a2dpSinkProfileInterface_1.testProfileDisConnect, 'result': 'None', 'category': testData_1.CaseCategory.ProfileHandsFreeAudioGateway },
    { 'name': '获取连接设备', 'detail': '获取Profile连接设备', 'method': 'getConnectionDevices', 'api': a2dpSinkProfileInterface_1.testProfileGetConnectionDevices, 'result': 'None', 'category': testData_1.CaseCategory.ProfileHandsFreeAudioGateway, },
    { 'name': '获取设备状态', 'detail': '获取Profile设备状态', 'method': 'getDeviceState', 'api': a2dpSinkProfileInterface_1.testProfileGetDeviceState, 'result': 'None', 'category': testData_1.CaseCategory.ProfileHandsFreeAudioGateway, },
];
//const HandsFreeUnitProfileTestCase: any[] =[
//  { 'name': '连接', 'detail': 'Profile连接', 'method': 'connect', 'api': testProfileConnect, 'result': 'None', 'category': CaseCategory.ProfileHandsFreeUnit },
//  { 'name': '取消连接', 'detail': '取消Profile连接', 'method': 'disconnect', 'api': testProfileDisConnect, 'result': 'None', 'category': CaseCategory.ProfileHandsFreeUnit },
//  { 'name': '连接Sco', 'detail': '连接Sco', 'method': 'connectSco', 'api': testConnectSco, 'result': 'None', 'category': CaseCategory.ProfileHandsFreeUnit },
//  { 'name': '获取Sco状态', 'detail': '获取Sco状态', 'method': 'getScoState', 'api': testGetScoState, 'result': 'None', 'category': CaseCategory.ProfileHandsFreeUnit },
//  { 'name': '取消连接Sco', 'detail': '取消连接Sco', 'method': 'disconnectSco', 'api': testDisconnectSco, 'result': 'None', 'category': CaseCategory.ProfileHandsFreeUnit },
//  { 'name': '发送DTMF', 'detail': '发送DTMF', 'method': 'sendDTMF', 'api': testSendDTMF, 'result': 'None', 'category': CaseCategory.ProfileHandsFreeUnit },
//  { 'name': '获取连接设备', 'detail': '获取Profile连接设备', 'method': 'getConnectionDevices', 'api': testProfileGetConnectionDevices, 'result': 'None', 'category': CaseCategory.ProfileHandsFreeUnit },
//  { 'name': '获取设备状态', 'detail': '获取Profile设备状态', 'method': 'getDeviceState', 'api': testProfileGetDeviceState, 'result': 'None', 'category': CaseCategory.ProfileHandsFreeUnit },
//]
const HitHostProfileTestCase = [
    { 'name': '连接', 'detail': 'Profile连接', 'method': 'connect', 'api': a2dpSinkProfileInterface_1.testProfileConnect, 'result': 'None', 'category': testData_1.CaseCategory.ProfileHitHost },
    { 'name': '取消连接', 'detail': '取消Profile连接', 'method': 'disconnect', 'api': a2dpSinkProfileInterface_1.testProfileDisConnect, 'result': 'None', 'category': testData_1.CaseCategory.ProfileHitHost },
    { 'name': '获取连接设备', 'detail': '获取Profile连接设备', 'method': 'getConnectionDevices', 'api': a2dpSinkProfileInterface_1.testProfileGetConnectionDevices, 'result': 'None', 'category': testData_1.CaseCategory.ProfileHitHost },
    { 'name': '获取设备状态', 'detail': '获取Profile设备状态', 'method': 'getDeviceState', 'api': a2dpSinkProfileInterface_1.testProfileGetDeviceState, 'result': 'None', 'category': testData_1.CaseCategory.ProfileHitHost },
];
const PanNetworkProfileTestCase = [
    { 'name': '取消连接', 'detail': '取消Profile连接', 'method': 'disconnect', 'api': a2dpSinkProfileInterface_1.testProfileDisConnect, 'result': 'None', 'category': testData_1.CaseCategory.ProfilePanNetwork },
    { 'name': '获取连接设备', 'detail': '获取Profile连接设备', 'method': 'getConnectionDevices', 'api': a2dpSinkProfileInterface_1.testProfileGetConnectionDevices, 'result': 'None', 'category': testData_1.CaseCategory.ProfilePanNetwork },
    { 'name': '获取设备状态', 'detail': '获取Profile设备状态', 'method': 'getDeviceState', 'api': a2dpSinkProfileInterface_1.testProfileGetDeviceState, 'result': 'None', 'category': testData_1.CaseCategory.ProfilePanNetwork },
    //  { 'name': '设置网络共享', 'detail': '设置网络数据共享', 'method': 'setTethering', 'api': testSetTethering, 'result': 'None', 'category': CaseCategory.ProfilePanNetwork },
    //  { 'name': '网络共享状态打开', 'detail': '网络共享状态打开', 'method': 'isTetheringOn', 'api': testIsTetheringOn, 'result': 'None', 'category': CaseCategory.ProfilePanNetwork },
];
//const PbapClientProfileTestCase:any[] =[
//  { 'name': '连接', 'detail': 'Profile连接', 'method': 'connect', 'api': testProfileConnect, 'result': 'None', 'category': CaseCategory.ProfilePbapClient },
//  { 'name': '取消连接', 'detail': '取消Profile连接', 'method': 'disconnect', 'api': testProfileDisConnect, 'result': 'None', 'category': CaseCategory.ProfilePbapClient },
//  { 'name': '获取连接设备', 'detail': '获取Profile连接设备', 'method': 'getConnectionDevices', 'api': testProfileGetConnectionDevices, 'result': 'None', 'category': CaseCategory.ProfilePbapClient },
//  { 'name': '获取设备状态', 'detail': '获取Profile设备状态', 'method': 'getDeviceState', 'api': testProfileGetDeviceState, 'result': 'None', 'category': CaseCategory.ProfilePbapClient },
//]
//
//const PbapServerProfileTestCase:any[] =[
//  { 'name': '取消连接', 'detail': '取消Profile连接', 'method': 'disconnect', 'api': testProfileDisConnect, 'result': 'None', 'category': CaseCategory.ProfilePbapServer },
//  { 'name': '获取连接设备', 'detail': '获取Profile连接设备', 'method': 'getConnectionDevices', 'api': testProfileGetConnectionDevices, 'result': 'None', 'category': CaseCategory.ProfilePbapServer },
//  { 'name': '获取设备状态', 'detail': '获取Profile设备状态', 'method': 'getDeviceState', 'api': testProfileGetDeviceState, 'result': 'None', 'category': CaseCategory.ProfilePbapServer },
//]
const TestComposition = [
    { 'name': 'Classic', 'detail': '经典蓝牙测试', 'url': 'pages/subManualApiTest/BrInterfaceTest', 'image': { "id": 16777539, "type": 20000, params: [] }, 'category': testData_1.Category.API },
    { 'name': 'BLE', 'detail': '低功耗蓝牙测试', 'url': 'pages/subManualApiTest/bleTest', 'image': { "id": 16777547, "type": 20000, params: [] }, 'category': testData_1.Category.API },
    { 'name': 'Gatt', 'detail': 'GATT测试', 'url': 'pages/subManualApiTest/gattTest', 'image': { "id": 16777540, "type": 20000, params: [] }, 'category': testData_1.Category.API },
    { 'name': 'SPP', 'detail': 'SPP测试', 'url': 'pages/subManualApiTest/sppTest', 'image': { "id": 16777558, "type": 20000, params: [] }, 'category': testData_1.Category.API },
    { 'name': 'Scenario', 'detail': '场景测试', 'image': { "id": 16777542, "type": 20000, params: [] }, 'category': testData_1.Category.Scenario },
    { 'name': 'BrAutoTest', 'detail': 'Br自动测试', 'url': 'pages/subAutoTest/brAutoTest', 'image': { "id": 16777532, "type": 20000, params: [] }, 'category': testData_1.Category.AutoTest },
    { 'name': 'BleAutoTest', 'detail': 'Ble自动测试', 'url': 'pages/subAutoTest/bleAutoTest', 'image': { "id": 16777532, "type": 20000, params: [] }, 'category': testData_1.Category.AutoTest },
    { 'name': 'GattAutoTest', 'detail': 'Gatt自动测试', 'url': 'pages/subAutoTest/gattAutoTest', 'image': { "id": 16777532, "type": 20000, params: [] }, 'category': testData_1.Category.AutoTest },
    { 'name': 'SppAutoTest', 'detail': 'Spp自动测试', 'url': 'pages/subAutoTest/sppAutoTest', 'image': { "id": 16777532, "type": 20000, params: [] }, 'category': testData_1.Category.AutoTest },
    { 'name': 'ProfileAutoTest', 'detail': 'Profile自动测试', 'url': 'pages/subAutoTest/profileAutoTest', 'image': { "id": 16777532, "type": 20000, params: [] }, 'category': testData_1.Category.AutoTest },
    { 'name': 'Profile', 'detail': 'Profile测试', 'url': 'pages/subManualApiTest/profileTest', 'image': { "id": 16777557, "type": 20000, params: [] }, 'category': testData_1.Category.Profile },
    { 'name': 'Benchmark', 'detail': '性能测试', 'image': { "id": 16777544, "type": 20000, params: [] }, 'category': testData_1.Category.BenchMark },
];
const GattAutoTestComposition = [
    { 'name': 'GattClientAutoTest', 'detail': 'GattClient自动测试', 'url': 'pages/subAutoTest/subGattAutoTest/gattClientAutoTest', 'image': { "id": 16777532, "type": 20000, params: [] }, 'category': testData_1.Category.GattAutoTest },
    { 'name': 'GattServerAutoTest', 'detail': 'GattServer自动测试', 'url': 'pages/subAutoTest/subGattAutoTest/gattServerAutoTest', 'image': { "id": 16777532, "type": 20000, params: [] }, 'category': testData_1.Category.GattAutoTest },
];
const SppAutoTestComposition = [
    { 'name': 'SppClientAutoTest', 'detail': 'SppClient自动测试', 'url': 'pages/subAutoTest/subSppAutoTest/sppClientAutoTest', 'image': { "id": 16777532, "type": 20000, params: [] }, 'category': testData_1.Category.SppAutoTest },
    { 'name': 'SppServerAutoTest', 'detail': 'SppServer自动测试', 'url': 'pages/subAutoTest/subSppAutoTest/sppServerAutoTest', 'image': { "id": 16777532, "type": 20000, params: [] }, 'category': testData_1.Category.SppAutoTest },
];
const ProfileAutoTestComposition = [
    { 'name': 'A2dpSourceAutoTest', 'detail': 'A2dpSource自动测试', 'url': 'pages/subAutoTest/subProfileAutoTest/a2dpSourceAutoTest', 'image': { "id": 16777532, "type": 20000, params: [] }, 'category': testData_1.Category.ProfileAutoTest },
    { 'name': 'HandsFreeAudioGatewayAutoTest', 'detail': 'HandsFreeAudioGateway自动测试', 'url': 'pages/subAutoTest/subProfileAutoTest/handsFreeAudioGatewayAutoTest', 'image': { "id": 16777532, "type": 20000, params: [] }, 'category': testData_1.Category.ProfileAutoTest },
    { 'name': 'HitHostAutoTest', 'detail': 'HitHost自动测试', 'url': 'pages/subAutoTest/subProfileAutoTest/hitHostAutoTest', 'image': { "id": 16777532, "type": 20000, params: [] }, 'category': testData_1.Category.ProfileAutoTest },
    { 'name': 'PanNetworkAutoTest', 'detail': 'PanNetwork自动测试', 'url': 'pages/subAutoTest/subProfileAutoTest/panNetworkAutoTest', 'image': { "id": 16777532, "type": 20000, params: [] }, 'category': testData_1.Category.ProfileAutoTest },
];
function initTestData() {
    let TestDataArray = [];
    TestComposition.forEach(item => {
        TestDataArray.push(new testData_1.TestData(item.name, item.detail, item.url, item.image, item.category));
    });
    return TestDataArray;
}
exports.initTestData = initTestData;
function initBRApiData() {
    let TestApiArray = [];
    BRTestCase.forEach(item => {
        TestApiArray.push(new testData_1.TestApi(item.name, item.detail, item.method, item.api, item.result, item.category));
    });
    return TestApiArray;
}
exports.initBRApiData = initBRApiData;
function initBleApiData() {
    let TestApiArray = [];
    BleTestCase.forEach(item => {
        TestApiArray.push(new testData_1.TestApi(item.name, item.detail, item.method, item.api, item.result, item.category));
    });
    return TestApiArray;
}
exports.initBleApiData = initBleApiData;
function initGattApiData() {
    let TestApiArray = [];
    GattTestCase.forEach(item => {
        TestApiArray.push(new testData_1.TestApi(item.name, item.detail, item.method, item.api, item.result, item.category));
    });
    return TestApiArray;
}
exports.initGattApiData = initGattApiData;
function initSppApiData() {
    let TestApiArray = [];
    SppTestCase.forEach(item => {
        TestApiArray.push(new testData_1.TestApi(item.name, item.detail, item.method, item.api, item.result, item.category));
    });
    return TestApiArray;
}
exports.initSppApiData = initSppApiData;
function initProfileApiData() {
    let TestApiArray = [];
    ProfileTestCase.forEach(item => {
        TestApiArray.push(new testData_1.TestApi(item.name, item.detail, item.method, item.api, item.result, item.category));
    });
    return TestApiArray;
}
exports.initProfileApiData = initProfileApiData;
function initGattClientApiData() {
    let TestApiArray = [];
    GattClientTestCase.forEach(item => {
        TestApiArray.push(new testData_1.TestApi(item.name, item.detail, item.method, item.api, item.result, item.category));
    });
    return TestApiArray;
}
exports.initGattClientApiData = initGattClientApiData;
function initGattServerApiData() {
    let TestApiArray = [];
    GattServerTestCase.forEach(item => {
        TestApiArray.push(new testData_1.TestApi(item.name, item.detail, item.method, item.api, item.result, item.category));
    });
    return TestApiArray;
}
exports.initGattServerApiData = initGattServerApiData;
function initSppClientApiData() {
    let TestApiArray = [];
    SppClientTestCase.forEach(item => {
        TestApiArray.push(new testData_1.TestApi(item.name, item.detail, item.method, item.api, item.result, item.category));
    });
    return TestApiArray;
}
exports.initSppClientApiData = initSppClientApiData;
function initSppServerApiData() {
    let TestApiArray = [];
    SppServerTestCase.forEach(item => {
        TestApiArray.push(new testData_1.TestApi(item.name, item.detail, item.method, item.api, item.result, item.category));
    });
    return TestApiArray;
}
exports.initSppServerApiData = initSppServerApiData;
function initA2dpSourceProfileApiData() {
    let TestApiArray = [];
    A2dpSourceProfileTestCase.forEach(item => {
        TestApiArray.push(new testData_1.TestApi(item.name, item.detail, item.method, item.api, item.result, item.category));
    });
    return TestApiArray;
}
exports.initA2dpSourceProfileApiData = initA2dpSourceProfileApiData;
function initHandsFreeAudioGatewayProfileApiData() {
    let TestApiArray = [];
    HandsFreeAudioGatewayProfileTestCase.forEach(item => {
        TestApiArray.push(new testData_1.TestApi(item.name, item.detail, item.method, item.api, item.result, item.category));
    });
    return TestApiArray;
}
exports.initHandsFreeAudioGatewayProfileApiData = initHandsFreeAudioGatewayProfileApiData;
function initHitHostProfileApiData() {
    let TestApiArray = [];
    HitHostProfileTestCase.forEach(item => {
        TestApiArray.push(new testData_1.TestApi(item.name, item.detail, item.method, item.api, item.result, item.category));
    });
    return TestApiArray;
}
exports.initHitHostProfileApiData = initHitHostProfileApiData;
function initPanNetworkProfileApiData() {
    let TestApiArray = [];
    PanNetworkProfileTestCase.forEach(item => {
        TestApiArray.push(new testData_1.TestApi(item.name, item.detail, item.method, item.api, item.result, item.category));
    });
    return TestApiArray;
}
exports.initPanNetworkProfileApiData = initPanNetworkProfileApiData;
function initGattAutoTestData() {
    let TestDataArray = [];
    GattAutoTestComposition.forEach(item => {
        TestDataArray.push(new testData_1.TestData(item.name, item.detail, item.url, item.image, item.category));
    });
    return TestDataArray;
}
exports.initGattAutoTestData = initGattAutoTestData;
function initSppAutoTestData() {
    let TestDataArray = [];
    SppAutoTestComposition.forEach(item => {
        TestDataArray.push(new testData_1.TestData(item.name, item.detail, item.url, item.image, item.category));
    });
    return TestDataArray;
}
exports.initSppAutoTestData = initSppAutoTestData;
function initProfileAutoTestData() {
    let TestDataArray = [];
    ProfileAutoTestComposition.forEach(item => {
        TestDataArray.push(new testData_1.TestData(item.name, item.detail, item.url, item.image, item.category));
    });
    return TestDataArray;
}
exports.initProfileAutoTestData = initProfileAutoTestData;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Utils/BaseModel.ts":
/*!***************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Utils/BaseModel.ts ***!
  \***************************************************************************************************************/
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

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Utils/ConfigData.ts":
/*!****************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Utils/ConfigData.ts ***!
  \****************************************************************************************************************/
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

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Utils/LogUtil.ts":
/*!*************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Utils/LogUtil.ts ***!
  \*************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.LogUtil = void 0;
const BaseModel_1 = __importDefault(__webpack_require__(/*! ./BaseModel */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Utils/BaseModel.ts"));
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
        console.info(msg);
        if (LogLevel.DEBUG >= LOG_LEVEL) {
            console.info(msg);
        }
    }
    log(msg) {
        console.info(msg);
        if (LogLevel.INFO >= LOG_LEVEL) {
            console.info(msg);
        }
    }
    info(msg) {
        console.info(msg);
        if (LogLevel.INFO >= LOG_LEVEL) {
            console.info(msg);
        }
    }
    warn(msg) {
        console.info(msg);
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

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/component/controller/BaseSettingsController.ts":
/*!*******************************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/component/controller/BaseSettingsController.ts ***!
  \*******************************************************************************************************************************************/
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
__webpack_require__(/*! ./ISettingsController */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/component/controller/ISettingsController.ts");
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

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/component/controller/ISettingsController.ts":
/*!****************************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/component/controller/ISettingsController.ts ***!
  \****************************************************************************************************************************************/
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

/***/ "../../api/@ohos.bluetooth.d.ts":
/*!**************************************!*\
  !*** ../../api/@ohos.bluetooth.d.ts ***!
  \**************************************/
/***/ (() => {



/***/ }),

/***/ "../../api/@ohos.events.emitter.d.ts":
/*!*******************************************!*\
  !*** ../../api/@ohos.events.emitter.d.ts ***!
  \*******************************************/
/***/ (() => {



/***/ }),

/***/ "../../api/@system.prompt.d.ts":
/*!*************************************!*\
  !*** ../../api/@system.prompt.d.ts ***!
  \*************************************/
/***/ (() => {



/***/ })

}]);
          globalThis["__common_module_cache___840471a2ef2afe531ee6aea9267b2e21"] = globalThis["__common_module_cache___840471a2ef2afe531ee6aea9267b2e21"] || {};
          globalThis["webpackChunk_840471a2ef2afe531ee6aea9267b2e21"].forEach((item)=> {
            Object.keys(item[1]).forEach((element) => {
              globalThis["__common_module_cache___840471a2ef2afe531ee6aea9267b2e21"][element] = null;
            })
          });
//# sourceMappingURL=commons.js.map