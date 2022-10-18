var _840471a2ef2afe531ee6aea9267b2e21;
/******/ (() => { // webpackBootstrap
/******/ 	"use strict";
/******/ 	var __webpack_modules__ = ({

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/pages/subManualApiTest/subBrTest/profileConnState.ets?entry":
/*!********************************************************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/pages/subManualApiTest/subBrTest/profileConnState.ets?entry ***!
  \********************************************************************************************************************************************************************/
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
// @ts-nocheck
const ConfigData_1 = __importDefault(__webpack_require__(/*! ../../../../Utils/ConfigData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Utils/ConfigData.ts"));
const pageTitle_1 = __webpack_require__(/*! ../../../../Component/pageTitle */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Component/pageTitle.ets");
const titleComponent_1 = __webpack_require__(/*! ../../../../Component/titleComponent */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Component/titleComponent.ets");
class profileConnState extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.__message = new ObservedPropertySimple("请点击选择PROFILE TYPE", this, "message");
        this.__currentClick = new ObservedPropertySimple(-1, this, "currentClick");
        this.__profileId = new ObservedPropertySimple(-1, this, "profileId");
        this.__profileType = new ObservedPropertySimple('', this, "profileType");
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
        if (params.profileType !== undefined) {
            this.profileType = params.profileType;
        }
    }
    aboutToBeDeleted() {
        this.__message.aboutToBeDeleted();
        this.__currentClick.aboutToBeDeleted();
        this.__profileId.aboutToBeDeleted();
        this.__profileType.aboutToBeDeleted();
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
    get profileType() {
        return this.__profileType.get();
    }
    set profileType(newValue) {
        this.__profileType.set(newValue);
    }
    render() {
        Column.create();
        Column.debugLine("pages/subManualApiTest/subBrTest/profileConnState.ets(31:5)");
        Column.padding({ left: { "id": 16777372, "type": 10002, params: [] }, right: { "id": 16777372, "type": 10002, params: [] } });
        Column.width("100%");
        Column.height(ConfigData_1.default.WH_100_100);
        Column.backgroundColor({ "id": 16777302, "type": 10001, params: [] });
        let earlierCreatedChild_2 = this.findChildById("2");
        if (earlierCreatedChild_2 == undefined) {
            View.create(new pageTitle_1.PageTitle("2", this, { detail: '选择PROFILE ID' }));
        }
        else {
            earlierCreatedChild_2.updateWithValueParams({
                detail: '选择PROFILE ID'
            });
            if (!earlierCreatedChild_2.needsUpdate()) {
                earlierCreatedChild_2.markStatic();
            }
            View.create(earlierCreatedChild_2);
        }
        Column.create();
        Column.debugLine("pages/subManualApiTest/subBrTest/profileConnState.ets(33:7)");
        List.create();
        List.debugLine("pages/subManualApiTest/subBrTest/profileConnState.ets(34:9)");
        List.height(400);
        ListItem.create();
        ListItem.debugLine("pages/subManualApiTest/subBrTest/profileConnState.ets(35:11)");
        ListItem.height(80);
        ListItem.borderRadius(10);
        let earlierCreatedChild_3 = this.findChildById("3");
        if (earlierCreatedChild_3 == undefined) {
            View.create(new titleComponent_1.TitleComponent("3", this, {
                title: "SELECT_PROFILE_ID",
                bgColor: { "id": 16777304, "type": 10001, params: [] },
                fontSize: '17vp',
            }));
        }
        else {
            earlierCreatedChild_3.updateWithValueParams({
                title: "SELECT_PROFILE_ID",
                bgColor: { "id": 16777304, "type": 10001, params: [] },
                fontSize: '17vp'
            });
            View.create(earlierCreatedChild_3);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.debugLine("pages/subManualApiTest/subBrTest/profileConnState.ets(45:11)");
        ListItem.padding({ top: 10 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            this.currentClick = 0;
            this.profileId = 1;
        });
        let earlierCreatedChild_4 = this.findChildById("4");
        if (earlierCreatedChild_4 == undefined) {
            View.create(new titleComponent_1.TitleComponent("4", this, {
                title: "A2DP_SOURCE",
                fontSize: '15vp',
                bgColor: this.currentClick === 0 ? { "id": 16777298, "type": 10001, params: [] } : { "id": 16777313, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_4.updateWithValueParams({
                title: "A2DP_SOURCE",
                fontSize: '15vp',
                bgColor: this.currentClick === 0 ? { "id": 16777298, "type": 10001, params: [] } : { "id": 16777313, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_4);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.debugLine("pages/subManualApiTest/subBrTest/profileConnState.ets(59:11)");
        ListItem.padding({ top: 20 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            this.currentClick = 1;
            this.profileId = 4;
        });
        let earlierCreatedChild_5 = this.findChildById("5");
        if (earlierCreatedChild_5 == undefined) {
            View.create(new titleComponent_1.TitleComponent("5", this, {
                title: "HANDS_FREE_AUDIO_GATEWAY",
                fontSize: '15vp',
                bgColor: this.currentClick === 1 ? { "id": 16777298, "type": 10001, params: [] } : { "id": 16777313, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_5.updateWithValueParams({
                title: "HANDS_FREE_AUDIO_GATEWAY",
                fontSize: '15vp',
                bgColor: this.currentClick === 1 ? { "id": 16777298, "type": 10001, params: [] } : { "id": 16777313, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_5);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.debugLine("pages/subManualApiTest/subBrTest/profileConnState.ets(74:11)");
        ListItem.padding({ top: 20 });
        ListItem.height(80);
        ListItem.borderRadius(10);
        ListItem.onClick(() => {
            this.currentClick = 2;
            this.profileId = 6;
        });
        let earlierCreatedChild_6 = this.findChildById("6");
        if (earlierCreatedChild_6 == undefined) {
            View.create(new titleComponent_1.TitleComponent("6", this, {
                title: "HID_HOST",
                fontSize: '15vp',
                bgColor: this.currentClick === 2 ? { "id": 16777298, "type": 10001, params: [] } : { "id": 16777313, "type": 10001, params: [] }
            }));
        }
        else {
            earlierCreatedChild_6.updateWithValueParams({
                title: "HID_HOST",
                fontSize: '15vp',
                bgColor: this.currentClick === 2 ? { "id": 16777298, "type": 10001, params: [] } : { "id": 16777313, "type": 10001, params: [] }
            });
            View.create(earlierCreatedChild_6);
        }
        ListItem.pop();
        List.pop();
        Column.pop();
        Text.create(this.message);
        Text.debugLine("pages/subManualApiTest/subBrTest/profileConnState.ets(91:7)");
        Text.textAlign(TextAlign.Center);
        Text.fontSize('17fp');
        Text.height(60);
        Text.width("100%");
        Text.backgroundColor({ "id": 16777304, "type": 10001, params: [] });
        Text.pop();
        Row.create();
        Row.debugLine("pages/subManualApiTest/subBrTest/profileConnState.ets(99:7)");
        Row.width("80%");
        Column.create();
        Column.debugLine("pages/subManualApiTest/subBrTest/profileConnState.ets(100:9)");
        Column.padding({ right: 10 });
        Text.create("取消");
        Text.debugLine("pages/subManualApiTest/subBrTest/profileConnState.ets(101:11)");
        Text.textAlign(TextAlign.Center);
        Text.fontSize('17fp');
        Text.height(60);
        Text.width(100);
        Text.borderRadius(10);
        Text.borderColor({ "id": 16777304, "type": 10001, params: [] });
        Text.backgroundColor({ "id": 16777312, "type": 10001, params: [] });
        Text.pop();
        Column.pop();
        Column.create();
        Column.debugLine("pages/subManualApiTest/subBrTest/profileConnState.ets(111:9)");
        Column.padding({ left: 50 });
        Text.create("确认");
        Text.debugLine("pages/subManualApiTest/subBrTest/profileConnState.ets(112:11)");
        Text.textAlign(TextAlign.Center);
        Text.fontSize('17fp');
        Text.height(60);
        Text.width(100);
        Text.borderRadius(10);
        Text.borderColor({ "id": 16777304, "type": 10001, params: [] });
        Text.backgroundColor({ "id": 16777312, "type": 10001, params: [] });
        Text.pop();
        Column.pop();
        Row.pop();
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
        Column.debugLine("pages/subManualApiTest/subBrTest/profileConnState.ets(141:5)");
        Column.width(ConfigData_1.default.WH_100_100);
        Text.create(this.text);
        Text.debugLine("pages/subManualApiTest/subBrTest/profileConnState.ets(142:7)");
        Text.fontSize({ "id": 16777389, "type": 10002, params: [] });
        Text.fontWeight(FontWeight.Medium);
        Text.lineHeight({ "id": 16777487, "type": 10002, params: [] });
        Text.fontColor({ "id": 16777298, "type": 10001, params: [] });
        Text.textAlign(TextAlign.Center);
        Text.width(100);
        Text.height({ "id": 16777508, "type": 10002, params: [] });
        Text.borderRadius({ "id": 16777427, "type": 10002, params: [] });
        Text.backgroundColor(!this.isTouched ? { "id": 16777296, "type": 10001, params: [] } : { "id": 125829123, "type": 10001, params: [] });
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
loadDocument(new profileConnState("1", undefined, {}));


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
/******/ 			"./pages/subManualApiTest/subBrTest/profileConnState": 0
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
/******/ 	var __webpack_exports__ = __webpack_require__.O(undefined, ["commons"], () => (__webpack_require__("../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/pages/subManualApiTest/subBrTest/profileConnState.ets?entry")))
/******/ 	__webpack_exports__ = __webpack_require__.O(__webpack_exports__);
/******/ 	_840471a2ef2afe531ee6aea9267b2e21 = __webpack_exports__;
/******/ 	
/******/ })()
;
//# sourceMappingURL=profileConnState.js.map