var _2a33209e6af7922da1521a8a828bfbd0;
/******/ (() => { // webpackBootstrap
/******/ 	"use strict";
/******/ 	var __webpack_modules__ = ({

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/pages/subManualApiTest/subBrTest/pairedDevices.ets?entry":
/*!*************************************************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/pages/subManualApiTest/subBrTest/pairedDevices.ets?entry ***!
  \*************************************************************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {


var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
/**
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
__webpack_require__(/*! ../../../../Component/headComponent */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Component/headComponent.ets");
var _system_router_1  = globalThis.requireNativeModule('system.router');
const titleComponent_1 = __webpack_require__(/*! ../../../../Component/titleComponent */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Component/titleComponent.ets");
const pageTitle_1 = __webpack_require__(/*! ../../../../Component/pageTitle */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Component/pageTitle.ets");
class pairedDevices extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.__message = new ObservedPropertySimple("", this, "message");
        this.duration = '0';
        this.deviceInfo = '';
        this.aboutDeviceList = [];
        this.scanMode = 0;
        this.__pairedDevices = AppStorage.GetOrCreate().setAndLink("pairedDevices", [], this);
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.message !== undefined) {
            this.message = params.message;
        }
        if (params.duration !== undefined) {
            this.duration = params.duration;
        }
        if (params.deviceInfo !== undefined) {
            this.deviceInfo = params.deviceInfo;
        }
        if (params.aboutDeviceList !== undefined) {
            this.aboutDeviceList = params.aboutDeviceList;
        }
        if (params.scanMode !== undefined) {
            this.scanMode = params.scanMode;
        }
    }
    aboutToBeDeleted() {
        this.__message.aboutToBeDeleted();
        this.__pairedDevices.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id());
    }
    get message() {
        return this.__message.get();
    }
    set message(newValue) {
        this.__message.set(newValue);
    }
    get pairedDevices() {
        return this.__pairedDevices.get();
    }
    set pairedDevices(newValue) {
        this.__pairedDevices.set(newValue);
    }
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
        Column.create();
        Column.padding({ left: { "id": 16777365, "type": 10002, params: [] }, right: { "id": 16777365, "type": 10002, params: [] } });
        Column.width(ConfigData_1.default.WH_100_100);
        Column.height(ConfigData_1.default.WH_100_100);
        Column.useSizeType({
            xs: { span: 12, offset: 0 }, sm: { span: 12, offset: 0 },
            md: { span: 12, offset: 0 }, lg: { span: 8, offset: 2 }
        });
        let earlierCreatedChild_2 = this.findChildById("2");
        if (earlierCreatedChild_2 == undefined) {
            View.create(new pageTitle_1.PageTitle("2", this, { detail: '已配对设备' }));
        }
        else {
            earlierCreatedChild_2.updateWithValueParams({
                detail: '已配对设备'
            });
            if (!earlierCreatedChild_2.needsUpdate()) {
                earlierCreatedChild_2.markStatic();
            }
            View.create(earlierCreatedChild_2);
        }
        //            HeadComponent({ headName: '已配对设备', isActive: true })
        Divider.create();
        //            HeadComponent({ headName: '已配对设备', isActive: true })
        Divider.padding({ left: { "id": 125829735, "type": 10002, params: [] }, right: { "id": 125829736, "type": 10002, params: [] } });
        Column.create();
        List.create();
        ForEach.create("4", this, ObservedObject.GetRawObject(this.pairedDevices), (item) => {
            ListItem.create();
            ListItem.padding({ top: { "id": 16777361, "type": 10002, params: [] }, bottom: { "id": 16777361, "type": 10002, params: [] } });
            let earlierCreatedChild_3 = this.findChildById("3");
            if (earlierCreatedChild_3 == undefined) {
                View.create(new titleComponent_1.TitleComponent("3", this, { title: item }));
            }
            else {
                earlierCreatedChild_3.updateWithValueParams({
                    title: item
                });
                View.create(earlierCreatedChild_3);
            }
            ListItem.pop();
        }, item => JSON.stringify(item));
        ForEach.pop();
        List.pop();
        Blank.create();
        Blank.pop();
        Column.pop();
        Flex.create({ direction: FlexDirection.Row, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center });
        Row.create();
        Row.width({ "id": 16777348, "type": 10002, params: [] });
        Row.margin({ right: { "id": 16777352, "type": 10002, params: [] } });
        let earlierCreatedChild_5 = this.findChildById("5");
        if (earlierCreatedChild_5 == undefined) {
            View.create(new ButtonComponent("5", this, { text: { "id": 16777250, "type": 10003, params: [] }, onClick: () => {
                    _system_router_1.back();
                } }));
        }
        else {
            earlierCreatedChild_5.updateWithValueParams({
                text: { "id": 16777250, "type": 10003, params: [] },
                onClick: () => {
                    _system_router_1.back();
                }
            });
            View.create(earlierCreatedChild_5);
        }
        Row.pop();
        Row.create();
        Row.width({ "id": 16777348, "type": 10002, params: [] });
        let earlierCreatedChild_6 = this.findChildById("6");
        if (earlierCreatedChild_6 == undefined) {
            View.create(new ButtonComponent("6", this, { text: { "id": 16777253, "type": 10003, params: [] }, onClick: () => {
                    _system_router_1.back();
                } }));
        }
        else {
            earlierCreatedChild_6.updateWithValueParams({
                text: { "id": 16777253, "type": 10003, params: [] },
                onClick: () => {
                    _system_router_1.back();
                }
            });
            View.create(earlierCreatedChild_6);
        }
        Row.pop();
        Flex.pop();
        Column.pop();
        Row.pop();
        GridContainer.pop();
        Column.pop();
    }
}
class ButtonComponent extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.__isTouched = new ObservedPropertySimple(false, this, "isTouched");
        this.text = undefined;
        this.onClick = () => { };
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.isTouched !== undefined) {
            this.isTouched = params.isTouched;
        }
        if (params.text !== undefined) {
            this.text = params.text;
        }
        if (params.onClick !== undefined) {
            this.onClick = params.onClick;
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
        Column.create();
        Column.width(ConfigData_1.default.WH_100_100);
        Column.align(Alignment.Center);
        Text.create(this.text);
        Text.fontSize({ "id": 16777382, "type": 10002, params: [] });
        Text.fontWeight(FontWeight.Medium);
        Text.lineHeight({ "id": 16777480, "type": 10002, params: [] });
        Text.fontColor({ "id": 16777291, "type": 10001, params: [] });
        Text.textAlign(TextAlign.Center);
        Text.width({ "id": 16777348, "type": 10002, params: [] });
        Text.height({ "id": 16777501, "type": 10002, params: [] });
        Text.borderRadius({ "id": 16777420, "type": 10002, params: [] });
        Text.backgroundColor(!this.isTouched ? { "id": 16777289, "type": 10001, params: [] } : { "id": 125829123, "type": 10001, params: [] });
        Text.onTouch((event) => {
            if (event.type === TouchType.Down) {
                this.isTouched = true;
            }
            if (event.type === TouchType.Up) {
                this.isTouched = false;
            }
        });
        Text.onClick(() => {
            this.onClick();
        });
        Text.pop();
        Column.pop();
    }
}
loadDocument(new pairedDevices("1", undefined, {}));


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
/******/ 			"./pages/subManualApiTest/subBrTest/pairedDevices": 0
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
/******/ 	var __webpack_exports__ = __webpack_require__.O(undefined, ["commons"], () => (__webpack_require__("../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/pages/subManualApiTest/subBrTest/pairedDevices.ets?entry")))
/******/ 	__webpack_exports__ = __webpack_require__.O(__webpack_exports__);
/******/ 	_2a33209e6af7922da1521a8a828bfbd0 = __webpack_exports__;
/******/ 	
/******/ })()
;
//# sourceMappingURL=pairedDevices.js.map