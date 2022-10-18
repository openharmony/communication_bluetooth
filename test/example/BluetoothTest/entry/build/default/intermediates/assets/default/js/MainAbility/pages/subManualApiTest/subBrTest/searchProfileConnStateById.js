var _2a33209e6af7922da1521a8a828bfbd0;
/******/ (() => { // webpackBootstrap
/******/ 	"use strict";
/******/ 	var __webpack_modules__ = ({

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/pages/subManualApiTest/subBrTest/searchProfileConnStateById.ets?entry":
/*!**************************************************************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/pages/subManualApiTest/subBrTest/searchProfileConnStateById.ets?entry ***!
  \**************************************************************************************************************************************************************************/
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
const titleComponent_1 = __webpack_require__(/*! ../../../../Component/titleComponent */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Component/titleComponent.ets");
var _ohos_bluetooth_1  = globalThis.requireNapi('bluetooth') || (isSystemplugin('bluetooth', 'ohos') ? globalThis.ohosplugin.bluetooth : isSystemplugin('bluetooth', 'system') ? globalThis.systemplugin.bluetooth : undefined);
class profileConnState2 extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.__message = new ObservedPropertySimple("", this, "message");
        this.__currentClick = new ObservedPropertySimple(-1, this, "currentClick");
        this.__profileId = new ObservedPropertySimple(-1, this, "profileId");
        this.stateProfile = "";
        this.__deviceId = new ObservedPropertySimple(AppStorage.Get('inputDeviceId'), this, "deviceId");
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.message !== undefined) {
            this.message = params.message;
        }
        if (params.currentClick !== undefined) {
            this.currentClick = params.currentClick;
        }
        if (params.profileId !== undefined) {
            this.profileId = params.profileId;
        }
        if (params.stateProfile !== undefined) {
            this.stateProfile = params.stateProfile;
        }
        if (params.deviceId !== undefined) {
            this.deviceId = params.deviceId;
        }
    }
    aboutToBeDeleted() {
        this.__message.aboutToBeDeleted();
        this.__currentClick.aboutToBeDeleted();
        this.__profileId.aboutToBeDeleted();
        this.__deviceId.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id());
    }
    get message() {
        return this.__message.get();
    }
    set message(newValue) {
        this.__message.set(newValue);
    }
    get currentClick() {
        return this.__currentClick.get();
    }
    set currentClick(newValue) {
        this.__currentClick.set(newValue);
    }
    get profileId() {
        return this.__profileId.get();
    }
    set profileId(newValue) {
        this.__profileId.set(newValue);
    }
    get deviceId() {
        return this.__deviceId.get();
    }
    set deviceId(newValue) {
        this.__deviceId.set(newValue);
    }
    render() {
        Row.create();
        Row.height('100%');
        Row.alignItems(VerticalAlign.Top);
        Row.backgroundColor({ "id": 16777295, "type": 10001, params: [] });
        Column.create();
        Column.width('100%');
        Row.create();
        Row.backgroundColor({ "id": 16777297, "type": 10001, params: [] });
        Row.margin({ top: "20vp", left: "20vp", right: "20vp" });
        Row.alignItems(VerticalAlign.Center);
        Text.create("目标设备MAC：");
        Text.fontSize("20vp");
        Text.pop();
        TextInput.create({ text: this.deviceId, placeholder: "请输入mac地址" });
        TextInput.fontSize("20vp");
        TextInput.onChange((str) => {
            this.deviceId = str;
            AppStorage.SetOrCreate('inputDeviceId', str);
        });
        TextInput.width('70%');
        Row.pop();
        List.create();
        List.height(300);
        List.margin(20);
        ListItem.create();
        ListItem.padding({ top: 20 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            this.currentClick = 0;
            this.profileId = 1;
            let ret = _ohos_bluetooth_1.getProfile(this.profileId).getDeviceState(this.deviceId);
            console.log('A2DP state is -' + ret);
            switch (ret) {
                case 0:
                    this.stateProfile = 'STATE_DISCONNECTED';
                    break;
                case 1:
                    this.stateProfile = 'STATE_CONNECTING';
                    break;
                case 2:
                    this.stateProfile = 'STATE_CONNECTED';
                    break;
                case 3:
                    this.stateProfile = 'STATE_DISCONNECTING';
                    break;
                case -1:
                    this.stateProfile = 'FAIL';
                    break;
            }
            this.message = "当前A2DP的状态是：" + this.stateProfile;
        });
        let earlierCreatedChild_2 = this.findChildById("2");
        if (earlierCreatedChild_2 == undefined) {
            View.create(new titleComponent_1.TitleComponent("2", this, {
                title: "A2DP_SOURCE",
                fontSize: '15vp',
                bgColor: this.currentClick === 0 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_2.updateWithValueParams({
                title: "A2DP_SOURCE",
                fontSize: '15vp',
                bgColor: this.currentClick === 0 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_2);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 20 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            this.currentClick = 1;
            this.profileId = 4;
            let ret = _ohos_bluetooth_1.getProfile(this.profileId).getDeviceState(this.deviceId);
            console.log('HFP state is -' + ret);
            switch (ret) {
                case 0:
                    this.stateProfile = 'STATE_DISCONNECTED';
                    break;
                case 1:
                    this.stateProfile = 'STATE_CONNECTING';
                    break;
                case 2:
                    this.stateProfile = 'STATE_CONNECTED';
                    break;
                case 3:
                    this.stateProfile = 'STATE_DISCONNECTING';
                    break;
                case -1:
                    this.stateProfile = 'FAIL';
                    break;
            }
            this.message = "当前HFP的状态是：" + this.stateProfile;
        });
        let earlierCreatedChild_3 = this.findChildById("3");
        if (earlierCreatedChild_3 == undefined) {
            View.create(new titleComponent_1.TitleComponent("3", this, {
                title: "HANDS_FREE_AUDIO_GATEWAY",
                fontSize: '15vp',
                bgColor: this.currentClick === 1 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_3.updateWithValueParams({
                title: "HANDS_FREE_AUDIO_GATEWAY",
                fontSize: '15vp',
                bgColor: this.currentClick === 1 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_3);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: 20 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            this.currentClick = 2;
            this.profileId = 6;
            let ret = _ohos_bluetooth_1.getProfile(this.profileId).getDeviceState(this.deviceId);
            console.log('HID state is -' + ret);
            switch (ret) {
                case 0:
                    this.stateProfile = 'STATE_DISCONNECTED';
                    break;
                case 1:
                    this.stateProfile = 'STATE_CONNECTING';
                    break;
                case 2:
                    this.stateProfile = 'STATE_CONNECTED';
                    break;
                case 3:
                    this.stateProfile = 'STATE_DISCONNECTING';
                    break;
                case -1:
                    this.stateProfile = 'FAIL';
                    break;
            }
            this.message = "当前HID的状态是：" + this.stateProfile;
        });
        let earlierCreatedChild_4 = this.findChildById("4");
        if (earlierCreatedChild_4 == undefined) {
            View.create(new titleComponent_1.TitleComponent("4", this, {
                title: "HID_HOST",
                fontSize: '15vp',
                bgColor: this.currentClick === 2 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_4.updateWithValueParams({
                title: "HID_HOST",
                fontSize: '15vp',
                bgColor: this.currentClick === 2 ? { "id": 16777291, "type": 10001, params: [] } : { "id": 16777306, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_4);
        }
        ListItem.pop();
        List.pop();
        Text.create("接口调用结果：" + this.message);
        Text.textAlign(TextAlign.Start);
        Text.fontSize('17fp');
        Text.backgroundColor({ "id": 16777305, "type": 10001, params: [] });
        Text.pop();
        Column.pop();
        Row.pop();
    }
}
loadDocument(new profileConnState2("1", undefined, {}));


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
/******/ 			"./pages/subManualApiTest/subBrTest/searchProfileConnStateById": 0
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
/******/ 	var __webpack_exports__ = __webpack_require__.O(undefined, ["commons"], () => (__webpack_require__("../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/pages/subManualApiTest/subBrTest/searchProfileConnStateById.ets?entry")))
/******/ 	__webpack_exports__ = __webpack_require__.O(__webpack_exports__);
/******/ 	_2a33209e6af7922da1521a8a828bfbd0 = __webpack_exports__;
/******/ 	
/******/ })()
;
//# sourceMappingURL=searchProfileConnStateById.js.map