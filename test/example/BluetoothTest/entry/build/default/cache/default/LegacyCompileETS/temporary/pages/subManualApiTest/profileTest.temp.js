var _2a33209e6af7922da1521a8a828bfbd0;
/******/ (() => { // webpackBootstrap
/******/ 	"use strict";
/******/ 	var __webpack_modules__ = ({

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Component/subEntryComponent.ets":
/*!************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Component/subEntryComponent.ets ***!
  \************************************************************************************************************************/
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

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/pages/subManualApiTest/profileTest.ets?entry":
/*!*************************************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/pages/subManualApiTest/profileTest.ets?entry ***!
  \*************************************************************************************************************************************************/
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
__webpack_require__(/*! ../../../Component/subEntryComponent */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Component/subEntryComponent.ets");
__webpack_require__(/*! ../../../component/headcomponent */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/headcomponent.ets");
var _system_router_1  = globalThis.requireNativeModule('system.router');
__webpack_require__(/*! @system.prompt */ "../../api/@system.prompt.d.ts");
__webpack_require__(/*! ../../model/TestData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/TestData.ets");
const TestDataModels_1 = __webpack_require__(/*! ../../model/TestDataModels */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/TestDataModels.ets");
const pageTitle_1 = __webpack_require__(/*! ../../../Component/pageTitle */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Component/pageTitle.ets");
const testImageDisplay_1 = __webpack_require__(/*! ../../../Component/testImageDisplay */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Component/testImageDisplay.ets");
const contentTable_1 = __webpack_require__(/*! ../../../Component/contentTable */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Component/contentTable.ets");
class Index extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.deviceId = _system_router_1.getParams().deviceId;
        this.testItem = _system_router_1.getParams().testId;
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.deviceId !== undefined) {
            this.deviceId = params.deviceId;
        }
        if (params.testItem !== undefined) {
            this.testItem = params.testItem;
        }
    }
    aboutToBeDeleted() {
        SubscriberManager.Get().delete(this.id());
    }
    render() {
        Column.create();
        Stack.create({ alignContent: Alignment.TopStart });
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
        let earlierCreatedChild_4 = this.findChildById("4");
        if (earlierCreatedChild_4 == undefined) {
            View.create(new contentTable_1.ContentTable("4", this, { testItem: this.testItem, apiItems: TestDataModels_1.initProfileApiData() }));
        }
        else {
            earlierCreatedChild_4.updateWithValueParams({
                testItem: this.testItem, apiItems: TestDataModels_1.initProfileApiData()
            });
            View.create(earlierCreatedChild_4);
        }
        Column.pop();
    }
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
/******/ 			"./pages/subManualApiTest/profileTest": 0
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
/******/ 	var __webpack_exports__ = __webpack_require__.O(undefined, ["commons"], () => (__webpack_require__("../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/pages/subManualApiTest/profileTest.ets?entry")))
/******/ 	__webpack_exports__ = __webpack_require__.O(__webpack_exports__);
/******/ 	_2a33209e6af7922da1521a8a828bfbd0 = __webpack_exports__;
/******/ 	
/******/ })()
;
//# sourceMappingURL=profileTest.js.map