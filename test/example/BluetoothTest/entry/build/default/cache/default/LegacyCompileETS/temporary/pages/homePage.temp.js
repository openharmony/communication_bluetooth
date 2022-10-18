var _2a33209e6af7922da1521a8a828bfbd0;
/******/ (() => { // webpackBootstrap
/******/ 	"use strict";
/******/ 	var __webpack_modules__ = ({

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/pages/homePage.ets?entry":
/*!*****************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/pages/homePage.ets?entry ***!
  \*****************************************************************************************************************************/
/***/ ((__unused_webpack_module, exports, __webpack_require__) => {


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
// import ConfigData from '../../Utils/ConfigData';
// import {SubEntryComponent} from '../../component/subentrycomponent';
const TestData_1 = __webpack_require__(/*! ../model/TestData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/TestData.ets");
const TestDataModels_1 = __webpack_require__(/*! ../model/TestDataModels */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/TestDataModels.ets");
const mainPageTitle_1 = __webpack_require__(/*! ../../component/mainPageTitle */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/mainPageTitle.ets");
const manualApiTest_1 = __webpack_require__(/*! ../../component/manualApiTest */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/manualApiTest.ets");
const manualSceneTest_1 = __webpack_require__(/*! ../../component/manualSceneTest */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/manualSceneTest.ets");
const autoTest_1 = __webpack_require__(/*! ../../component/autoTest */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/autoTest.ets");
const profileTest_1 = __webpack_require__(/*! ../../component/profileTest */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/profileTest.ets");
class TestCategory extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.testItems = undefined;
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.testItems !== undefined) {
            this.testItems = params.testItems;
        }
    }
    aboutToBeDeleted() {
        SubscriberManager.Get().delete(this.id());
    }
    render() {
        Stack.create();
        Tabs.create();
        Tabs.barWidth(280);
        Tabs.barHeight(60);
        Tabs.barMode(BarMode.Scrollable);
        Tabs.backgroundColor({ "id": 16777305, "type": 10001, params: [] });
        TabContent.create();
        TabContent.tabBar({ "id": 16777219, "type": 10003, params: [] });
        let earlierCreatedChild_2 = this.findChildById("2");
        if (earlierCreatedChild_2 == undefined) {
            //          TestList({ testItems: this.testItems.filter(item => (item.category === Category.API)) })
            View.create(new manualApiTest_1.ManualApiTest("2", this, { testItems: this.testItems.filter(item => (item.category === TestData_1.Category.API)) }));
        }
        else {
            earlierCreatedChild_2.updateWithValueParams({
                testItems: this.testItems.filter(item => (item.category === TestData_1.Category.API))
            });
            if (!earlierCreatedChild_2.needsUpdate()) {
                earlierCreatedChild_2.markStatic();
            }
            View.create(earlierCreatedChild_2);
        }
        TabContent.pop();
        TabContent.create();
        TabContent.tabBar({ "id": 16777269, "type": 10003, params: [] });
        let earlierCreatedChild_3 = this.findChildById("3");
        if (earlierCreatedChild_3 == undefined) {
            //          TestList({ testItems: this.testItems.filter(item => (item.category === Category.Scenario)) })
            View.create(new manualSceneTest_1.ManualSceneTest("3", this, { testItems: this.testItems.filter(item => (item.category === TestData_1.Category.Scenario)) }));
        }
        else {
            earlierCreatedChild_3.updateWithValueParams({
                testItems: this.testItems.filter(item => (item.category === TestData_1.Category.Scenario))
            });
            if (!earlierCreatedChild_3.needsUpdate()) {
                earlierCreatedChild_3.markStatic();
            }
            View.create(earlierCreatedChild_3);
        }
        TabContent.pop();
        TabContent.create();
        TabContent.tabBar({ "id": 16777222, "type": 10003, params: [] });
        let earlierCreatedChild_4 = this.findChildById("4");
        if (earlierCreatedChild_4 == undefined) {
            //          TestList({ testItems: this.testItems.filter(item => (item.category === Category.AutoTest)) })
            View.create(new autoTest_1.AutoTest("4", this, { testItems: this.testItems.filter(item => (item.category === TestData_1.Category.AutoTest)) }));
        }
        else {
            earlierCreatedChild_4.updateWithValueParams({
                testItems: this.testItems.filter(item => (item.category === TestData_1.Category.AutoTest))
            });
            if (!earlierCreatedChild_4.needsUpdate()) {
                earlierCreatedChild_4.markStatic();
            }
            View.create(earlierCreatedChild_4);
        }
        TabContent.pop();
        TabContent.create();
        TabContent.tabBar({ "id": 16777264, "type": 10003, params: [] });
        let earlierCreatedChild_5 = this.findChildById("5");
        if (earlierCreatedChild_5 == undefined) {
            //          TestList({ testItems: this.testItems.filter(item => (item.category === Category.Profile)) })
            View.create(new profileTest_1.ProfileTest("5", this, { testItems: this.testItems.filter(item => (item.category === TestData_1.Category.AutoTest)) }));
        }
        else {
            earlierCreatedChild_5.updateWithValueParams({
                testItems: this.testItems.filter(item => (item.category === TestData_1.Category.AutoTest))
            });
            if (!earlierCreatedChild_5.needsUpdate()) {
                earlierCreatedChild_5.markStatic();
            }
            View.create(earlierCreatedChild_5);
        }
        TabContent.pop();
        Tabs.pop();
        Stack.pop();
    }
}
class HomePage extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.__message = new ObservedPropertySimple('Hello World', this, "message");
        this.testItems = TestDataModels_1.initTestData();
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.message !== undefined) {
            this.message = params.message;
        }
        if (params.testItems !== undefined) {
            this.testItems = params.testItems;
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
        let earlierCreatedChild_6 = this.findChildById("6");
        if (earlierCreatedChild_6 == undefined) {
            View.create(new mainPageTitle_1.MainPageTitle("6", this, {}));
        }
        else {
            earlierCreatedChild_6.updateWithValueParams({});
            if (!earlierCreatedChild_6.needsUpdate()) {
                earlierCreatedChild_6.markStatic();
            }
            View.create(earlierCreatedChild_6);
        }
        Stack.create({ alignContent: Alignment.TopEnd });
        Stack.height('100%');
        let earlierCreatedChild_7 = this.findChildById("7");
        if (earlierCreatedChild_7 == undefined) {
            View.create(new TestCategory("7", this, { testItems: this.testItems }));
        }
        else {
            earlierCreatedChild_7.updateWithValueParams({
                testItems: this.testItems
            });
            if (!earlierCreatedChild_7.needsUpdate()) {
                earlierCreatedChild_7.markStatic();
            }
            View.create(earlierCreatedChild_7);
        }
        Image.create({ "id": 16777538, "type": 20000, params: [] });
        Image.height(24);
        Image.width(24);
        Image.margin({ top: 15, right: 10 });
        Image.onClick(() => {
        });
        Stack.pop();
        Column.pop();
    }
}
loadDocument(new HomePage("1", undefined, {}));


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/autoTest.ets":
/*!***************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/autoTest.ets ***!
  \***************************************************************************************************************/
/***/ ((__unused_webpack_module, exports, __webpack_require__) => {


Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.AutoTest = void 0;
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
const testList_1 = __webpack_require__(/*! ./testList */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/testList.ets");
class AutoTest extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.testItems = undefined;
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.testItems !== undefined) {
            this.testItems = params.testItems;
        }
    }
    aboutToBeDeleted() {
        SubscriberManager.Get().delete(this.id());
    }
    render() {
        Column.create();
        let earlierCreatedChild_1 = this.findChildById("1");
        if (earlierCreatedChild_1 == undefined) {
            View.create(new testList_1.TestList("1", this, { testItems: this.testItems }));
        }
        else {
            earlierCreatedChild_1.updateWithValueParams({
                testItems: this.testItems
            });
            if (!earlierCreatedChild_1.needsUpdate()) {
                earlierCreatedChild_1.markStatic();
            }
            View.create(earlierCreatedChild_1);
        }
        Column.pop();
    }
}
exports.AutoTest = AutoTest;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/mainPageTitle.ets":
/*!********************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/mainPageTitle.ets ***!
  \********************************************************************************************************************/
/***/ ((__unused_webpack_module, exports) => {


Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.MainPageTitle = void 0;
class MainPageTitle extends View {
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
        Flex.create({ alignItems: ItemAlign.Start });
        Flex.height(80);
        Flex.backgroundColor({ "id": 16777281, "type": 10001, params: [] });
        Flex.padding({ top: 12, bottom: 5, left: 28.3 });
        Text.create('蓝牙调试助手');
        Text.fontSize(26);
        Text.margin({ left: 17.4 });
        Text.padding({ top: 13 });
        Text.pop();
        Flex.pop();
    }
}
exports.MainPageTitle = MainPageTitle;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/manualApiTest.ets":
/*!********************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/manualApiTest.ets ***!
  \********************************************************************************************************************/
/***/ ((__unused_webpack_module, exports, __webpack_require__) => {


Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.ManualApiTest = void 0;
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
const testList_1 = __webpack_require__(/*! ./testList */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/testList.ets");
class ManualApiTest extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.testItems = undefined;
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.testItems !== undefined) {
            this.testItems = params.testItems;
        }
    }
    aboutToBeDeleted() {
        SubscriberManager.Get().delete(this.id());
    }
    render() {
        Column.create();
        let earlierCreatedChild_1 = this.findChildById("1");
        if (earlierCreatedChild_1 == undefined) {
            View.create(new testList_1.TestList("1", this, { testItems: this.testItems }));
        }
        else {
            earlierCreatedChild_1.updateWithValueParams({
                testItems: this.testItems
            });
            if (!earlierCreatedChild_1.needsUpdate()) {
                earlierCreatedChild_1.markStatic();
            }
            View.create(earlierCreatedChild_1);
        }
        Column.pop();
    }
}
exports.ManualApiTest = ManualApiTest;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/manualSceneTest.ets":
/*!**********************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/manualSceneTest.ets ***!
  \**********************************************************************************************************************/
/***/ ((__unused_webpack_module, exports, __webpack_require__) => {


Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.ManualSceneTest = void 0;
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
const testList_1 = __webpack_require__(/*! ./testList */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/testList.ets");
class ManualSceneTest extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.testItems = undefined;
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.testItems !== undefined) {
            this.testItems = params.testItems;
        }
    }
    aboutToBeDeleted() {
        SubscriberManager.Get().delete(this.id());
    }
    render() {
        Column.create();
        let earlierCreatedChild_1 = this.findChildById("1");
        if (earlierCreatedChild_1 == undefined) {
            View.create(new testList_1.TestList("1", this, { testItems: this.testItems }));
        }
        else {
            earlierCreatedChild_1.updateWithValueParams({
                testItems: this.testItems
            });
            if (!earlierCreatedChild_1.needsUpdate()) {
                earlierCreatedChild_1.markStatic();
            }
            View.create(earlierCreatedChild_1);
        }
        Column.pop();
    }
}
exports.ManualSceneTest = ManualSceneTest;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/profileTest.ets":
/*!******************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/profileTest.ets ***!
  \******************************************************************************************************************/
/***/ ((__unused_webpack_module, exports, __webpack_require__) => {


Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.ProfileTest = void 0;
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
const testList_1 = __webpack_require__(/*! ./testList */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/testList.ets");
class ProfileTest extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.testItems = undefined;
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.testItems !== undefined) {
            this.testItems = params.testItems;
        }
    }
    aboutToBeDeleted() {
        SubscriberManager.Get().delete(this.id());
    }
    render() {
        Column.create();
        let earlierCreatedChild_1 = this.findChildById("1");
        if (earlierCreatedChild_1 == undefined) {
            View.create(new testList_1.TestList("1", this, { testItems: this.testItems }));
        }
        else {
            earlierCreatedChild_1.updateWithValueParams({
                testItems: this.testItems
            });
            if (!earlierCreatedChild_1.needsUpdate()) {
                earlierCreatedChild_1.markStatic();
            }
            View.create(earlierCreatedChild_1);
        }
        Column.pop();
    }
}
exports.ProfileTest = ProfileTest;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/testList.ets":
/*!***************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/testList.ets ***!
  \***************************************************************************************************************/
/***/ ((__unused_webpack_module, exports, __webpack_require__) => {


Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.TestList = void 0;
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
const testListItem_1 = __webpack_require__(/*! ./testListItem */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/testListItem.ets");
class TestList extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.testItems = undefined;
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.testItems !== undefined) {
            this.testItems = params.testItems;
        }
    }
    aboutToBeDeleted() {
        SubscriberManager.Get().delete(this.id());
    }
    render() {
        Column.create();
        List.create();
        List.height('93%');
        List.backgroundColor({ "id": 16777295, "type": 10001, params: [] });
        ForEach.create("2", this, ObservedObject.GetRawObject(this.testItems), item => {
            ListItem.create();
            let earlierCreatedChild_1 = this.findChildById("1");
            if (earlierCreatedChild_1 == undefined) {
                View.create(new testListItem_1.TestListItem("1", this, { testItem: item }));
            }
            else {
                earlierCreatedChild_1.updateWithValueParams({
                    testItem: item
                });
                if (!earlierCreatedChild_1.needsUpdate()) {
                    earlierCreatedChild_1.markStatic();
                }
                View.create(earlierCreatedChild_1);
            }
            ListItem.pop();
        }, item => item.id.toString());
        ForEach.pop();
        List.pop();
        Column.pop();
    }
}
exports.TestList = TestList;


/***/ }),

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/testListItem.ets":
/*!*******************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/testListItem.ets ***!
  \*******************************************************************************************************************/
/***/ ((__unused_webpack_module, exports, __webpack_require__) => {


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
__webpack_require__(/*! ../MainAbility/model/TestData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/TestData.ets");
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
        Navigator.params({ testId: this.testItem });
        Navigator.margin({ right: 10, left: 10, top: 10 });
        Navigator.padding({ left: 15, right: 15 });
        Navigator.borderRadius(10);
        Navigator.backgroundColor({ "id": 16777305, "type": 10001, params: [] });
        Flex.create({ justifyContent: FlexAlign.Start, alignItems: ItemAlign.Center });
        Flex.height(85);
        Row.create();
        Row.backgroundColor('#FFf1f3f5');
        Row.margin({ right: 16 });
        Image.create(this.testItem.image);
        Image.objectFit(ImageFit.Contain);
        Image.autoResize(false);
        Image.height(60);
        Image.width(60);
        Row.pop();
        Text.create(this.testItem.name);
        Text.fontSize(18);
        Text.flexGrow(1);
        Text.pop();
        Text.create(this.testItem.detail);
        Text.fontSize(16);
        Text.pop();
        Flex.pop();
        Navigator.pop();
    }
}
exports.TestListItem = TestListItem;


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
/******/ 			"./pages/homePage": 0
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
/******/ 	var __webpack_exports__ = __webpack_require__.O(undefined, ["commons"], () => (__webpack_require__("../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/pages/homePage.ets?entry")))
/******/ 	__webpack_exports__ = __webpack_require__.O(__webpack_exports__);
/******/ 	_2a33209e6af7922da1521a8a828bfbd0 = __webpack_exports__;
/******/ 	
/******/ })()
;
//# sourceMappingURL=homePage.js.map