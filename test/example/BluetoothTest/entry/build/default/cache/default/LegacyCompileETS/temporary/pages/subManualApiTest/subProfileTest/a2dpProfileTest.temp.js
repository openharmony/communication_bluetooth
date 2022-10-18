var _2a33209e6af7922da1521a8a828bfbd0;
/******/ (() => { // webpackBootstrap
/******/ 	"use strict";
/******/ 	var __webpack_modules__ = ({

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/BTProfileModel.ets":
/*!*****************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/BTProfileModel.ets ***!
  \*****************************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {


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
const ConfigData_1 = __importDefault(__webpack_require__(/*! ../../Utils/ConfigData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Utils/ConfigData.ts"));
const BaseModel_1 = __importDefault(__webpack_require__(/*! ../../Utils/BaseModel */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Utils/BaseModel.ts"));
var _ohos_bluetooth_1  = globalThis.requireNapi('bluetooth') || (isSystemplugin('bluetooth', 'ohos') ? globalThis.ohosplugin.bluetooth : isSystemplugin('bluetooth', 'system') ? globalThis.systemplugin.bluetooth : undefined);
class BTProfileModel extends BaseModel_1.default {
    constructor() {
        super();
        this.TAG = ConfigData_1.default.TAG + 'BTProfileModel';
        this.profiles = new Array();
        this.PROFILE_A2DP_SOURCE = 1;
        this.PROFILE_HANDS_FREE_AUDIO_GATEWAY = 4;
        this.PROFILE_HID_HOST = 6;
        this.BLUETOOTH_STATE_OFF = 2;
        console.log('bt_test BTProfileModel start');
        this.profiles[this.PROFILE_A2DP_SOURCE] = _ohos_bluetooth_1.getProfile(this.PROFILE_A2DP_SOURCE);
        this.profiles[this.PROFILE_HANDS_FREE_AUDIO_GATEWAY] = _ohos_bluetooth_1.getProfile(this.PROFILE_HANDS_FREE_AUDIO_GATEWAY);
        this.profiles[this.PROFILE_HID_HOST] = _ohos_bluetooth_1.getProfile(this.PROFILE_HID_HOST);
        console.log('bt_test BTProfileModel end');
    }
    hasProfile(type) {
        if (!this.profiles[type]) {
            return false;
        }
        return true;
    }
    getA2DPProfile() {
        if (this.profiles[this.PROFILE_A2DP_SOURCE] != null) {
            return this.profiles[this.PROFILE_A2DP_SOURCE];
        }
        return null;
    }
    getHandsProfile() {
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

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/pages/subManualApiTest/subProfileTest/a2dpProfileTest.ets?entry":
/*!********************************************************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/pages/subManualApiTest/subProfileTest/a2dpProfileTest.ets?entry ***!
  \********************************************************************************************************************************************************************/
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
var _system_prompt_1  = isSystemplugin('prompt', 'system') ? globalThis.systemplugin.prompt : globalThis.requireNapi('prompt');
var _system_router_1  = globalThis.requireNativeModule('system.router');
const titleComponent_1 = __webpack_require__(/*! ../../../../Component/titleComponent */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Component/titleComponent.ets");
const headcomponent_1 = __importDefault(__webpack_require__(/*! ../../../../component/headcomponent */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/component/headcomponent.ets"));
const BTProfileModel_1 = __importDefault(__webpack_require__(/*! ../../../model/BTProfileModel */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/model/BTProfileModel.ets"));
const ConfigData_1 = __importDefault(__webpack_require__(/*! ../../../../Utils/ConfigData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Utils/ConfigData.ts"));
const LogUtil_1 = __importDefault(__webpack_require__(/*! ../../../../Utils/LogUtil */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/Utils/LogUtil.ts"));
class Index extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.__hasProfile = new ObservedPropertySimple(false, this, "hasProfile");
        this.__message = new ObservedPropertySimple("", this, "message");
        this.__isConnection = new ObservedPropertySimple(false, this, "isConnection");
        this.__connectionStateInfo = new ObservedPropertySimple("", this, "connectionStateInfo");
        this.__title = new ObservedPropertyObject({ "id": 16777218, "type": 10003, params: [] }, this, "title");
        this.__profileExist = new ObservedPropertySimple("", this, "profileExist");
        this.__profileNotExist = new ObservedPropertySimple("", this, "profileNotExist");
        this.__profileType = new ObservedPropertySimple(0, this, "profileType");
        this.__bluetoothState = new ObservedPropertySimple(0, this, "bluetoothState");
        this.__btConnectionState = new ObservedPropertySimple(0, this, "btConnectionState");
        this.pageType = 0;
        this.deviceId = "";
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.hasProfile !== undefined) {
            this.hasProfile = params.hasProfile;
        }
        if (params.message !== undefined) {
            this.message = params.message;
        }
        if (params.isConnection !== undefined) {
            this.isConnection = params.isConnection;
        }
        if (params.connectionStateInfo !== undefined) {
            this.connectionStateInfo = params.connectionStateInfo;
        }
        if (params.title !== undefined) {
            this.title = params.title;
        }
        if (params.profileExist !== undefined) {
            this.profileExist = params.profileExist;
        }
        if (params.profileNotExist !== undefined) {
            this.profileNotExist = params.profileNotExist;
        }
        if (params.profileType !== undefined) {
            this.profileType = params.profileType;
        }
        if (params.bluetoothState !== undefined) {
            this.bluetoothState = params.bluetoothState;
        }
        if (params.btConnectionState !== undefined) {
            this.btConnectionState = params.btConnectionState;
        }
        if (params.pageType !== undefined) {
            this.pageType = params.pageType;
        }
        if (params.deviceId !== undefined) {
            this.deviceId = params.deviceId;
        }
    }
    aboutToBeDeleted() {
        this.__hasProfile.aboutToBeDeleted();
        this.__message.aboutToBeDeleted();
        this.__isConnection.aboutToBeDeleted();
        this.__connectionStateInfo.aboutToBeDeleted();
        this.__title.aboutToBeDeleted();
        this.__profileExist.aboutToBeDeleted();
        this.__profileNotExist.aboutToBeDeleted();
        this.__profileType.aboutToBeDeleted();
        this.__bluetoothState.aboutToBeDeleted();
        this.__btConnectionState.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id());
    }
    get hasProfile() {
        return this.__hasProfile.get();
    }
    set hasProfile(newValue) {
        this.__hasProfile.set(newValue);
    }
    get message() {
        return this.__message.get();
    }
    set message(newValue) {
        this.__message.set(newValue);
    }
    get isConnection() {
        return this.__isConnection.get();
    }
    set isConnection(newValue) {
        this.__isConnection.set(newValue);
    }
    get connectionStateInfo() {
        return this.__connectionStateInfo.get();
    }
    set connectionStateInfo(newValue) {
        this.__connectionStateInfo.set(newValue);
    }
    get title() {
        return this.__title.get();
    }
    set title(newValue) {
        this.__title.set(newValue);
    }
    get profileExist() {
        return this.__profileExist.get();
    }
    set profileExist(newValue) {
        this.__profileExist.set(newValue);
    }
    get profileNotExist() {
        return this.__profileNotExist.get();
    }
    set profileNotExist(newValue) {
        this.__profileNotExist.set(newValue);
    }
    get profileType() {
        return this.__profileType.get();
    }
    set profileType(newValue) {
        this.__profileType.set(newValue);
    }
    get bluetoothState() {
        return this.__bluetoothState.get();
    }
    set bluetoothState(newValue) {
        this.__bluetoothState.set(newValue);
    }
    get btConnectionState() {
        return this.__btConnectionState.get();
    }
    set btConnectionState(newValue) {
        this.__btConnectionState.set(newValue);
    }
    aboutToAppear() {
        LogUtil_1.default.log(ConfigData_1.default.TAG + "xzyj: aboutToAppear a2dp: deviceId " + JSON.stringify(this.deviceId));
    }
    render() {
        Row.create();
        Row.backgroundColor({ "id": 125829132, "type": 10001, params: [] });
        Row.height('100%');
        Row.alignItems(VerticalAlign.Top);
        Column.create();
        Column.width('100%');
        let earlierCreatedChild_2 = this.findChildById("2");
        if (earlierCreatedChild_2 == undefined) {
            View.create(new headcomponent_1.default("2", this, { headName: this.title, isActive: true }));
        }
        else {
            earlierCreatedChild_2.updateWithValueParams({
                headName: this.title, isActive: true
            });
            View.create(earlierCreatedChild_2);
        }
        Stack.create();
        Stack.height("0.5vp");
        Stack.backgroundColor("#000000");
        Stack.pop();
        Text.create(this.hasProfile ? this.profileExist : this.profileNotExist);
        Text.margin({ top: "20vp" });
        Text.fontSize("20vp");
        Text.fontColor(this.hasProfile ? "#66ccff" : "#ff0000");
        Text.border({ width: 0.5 });
        Text.padding(5);
        Text.fontStyle(FontStyle.Italic);
        Text.pop();
        Column.create();
        Column.visibility(this.hasProfile ? Visibility.Visible : Visibility.Hidden);
        Row.create();
        Row.margin({ top: "20vp", left: "20vp", right: "20vp" });
        Row.alignItems(VerticalAlign.Center);
        Text.create("现在正在连接的设备为：");
        Text.fontSize("20vp");
        Text.pop();
        TextInput.create({ text: JSON.stringify(this.deviceId), placeholder: "请输入需要连接的deviceId" });
        TextInput.fontSize("20vp");
        TextInput.onChange((str) => {
            this.deviceId = str;
        });
        Row.pop();
        Text.create("请点击需要测试的方法");
        Text.margin({ top: "20vp" });
        Text.padding("5vp");
        Text.fontSize("15vp");
        Text.fontColor("#4444444");
        Text.pop();
        List.create();
        List.margin(20);
        ListItem.create();
        ListItem.padding({ top: { "id": 16777361, "type": 10002, params: [] }, bottom: { "id": 16777361, "type": 10002, params: [] } });
        ListItem.onClick((event) => {
            if (this.btConnectionState == 1 || this.btConnectionState == 2) {
                _system_prompt_1.showToast({ message: this.btConnectionState == 1 ? "该设备正在连接中" : "该设备已连接" });
                this.message = "";
                return;
            }
            let ret = BTProfileModel_1.default.getProfile(this.profileType).connect(this.deviceId);
            if (ret) {
                this.btConnectionState = 1;
            }
            this.message = "connect ret：" + ret;
        });
        let earlierCreatedChild_3 = this.findChildById("3");
        if (earlierCreatedChild_3 == undefined) {
            View.create(new titleComponent_1.TitleComponent("3", this, { title: "connect" }));
        }
        else {
            earlierCreatedChild_3.updateWithValueParams({
                title: "connect"
            });
            View.create(earlierCreatedChild_3);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: { "id": 16777361, "type": 10002, params: [] }, bottom: { "id": 16777361, "type": 10002, params: [] } });
        ListItem.onClick((event) => {
            if (this.btConnectionState == 0 || this.btConnectionState == 3) {
                _system_prompt_1.showToast({ message: this.btConnectionState == 0 ? "该设备已断开" : "该设备正在断开连接" });
                this.message = "";
                return;
            }
            let ret = BTProfileModel_1.default.getProfile(this.profileType).disconnect(this.deviceId);
            if (ret) {
                this.btConnectionState = 3;
            }
            this.message = "disconnect ret：" + ret;
        });
        let earlierCreatedChild_4 = this.findChildById("4");
        if (earlierCreatedChild_4 == undefined) {
            View.create(new titleComponent_1.TitleComponent("4", this, { title: "disconnect" }));
        }
        else {
            earlierCreatedChild_4.updateWithValueParams({
                title: "disconnect"
            });
            View.create(earlierCreatedChild_4);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: { "id": 16777361, "type": 10002, params: [] }, bottom: { "id": 16777361, "type": 10002, params: [] } });
        ListItem.onClick((event) => {
            let ret = BTProfileModel_1.default.getProfile(this.profileType).getPlayingState(this.deviceId);
            this.message = "getPlayingState ret: " + ret;
        });
        ListItem.visibility(this.pageType == 0 ? Visibility.Visible : Visibility.None);
        let earlierCreatedChild_5 = this.findChildById("5");
        if (earlierCreatedChild_5 == undefined) {
            View.create(new titleComponent_1.TitleComponent("5", this, { title: "getPlayingState" }));
        }
        else {
            earlierCreatedChild_5.updateWithValueParams({
                title: "getPlayingState"
            });
            View.create(earlierCreatedChild_5);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: { "id": 16777361, "type": 10002, params: [] }, bottom: { "id": 16777361, "type": 10002, params: [] } });
        ListItem.onClick((event) => {
            let ret = BTProfileModel_1.default.getProfile(this.profileType).getConnectionDevices();
            this.message = "getConnectionDevices ret: " + ret;
        });
        let earlierCreatedChild_6 = this.findChildById("6");
        if (earlierCreatedChild_6 == undefined) {
            View.create(new titleComponent_1.TitleComponent("6", this, { title: "getConnectionDevices" }));
        }
        else {
            earlierCreatedChild_6.updateWithValueParams({
                title: "getConnectionDevices"
            });
            View.create(earlierCreatedChild_6);
        }
        ListItem.pop();
        ListItem.create();
        ListItem.padding({ top: { "id": 16777361, "type": 10002, params: [] }, bottom: { "id": 16777361, "type": 10002, params: [] } });
        ListItem.onClick((event) => {
            let ret = BTProfileModel_1.default.getProfile(this.profileType).getDeviceState(this.deviceId);
            this.message = "getDeviceState ret: " + ret;
        });
        let earlierCreatedChild_7 = this.findChildById("7");
        if (earlierCreatedChild_7 == undefined) {
            View.create(new titleComponent_1.TitleComponent("7", this, { title: "getDeviceState" }));
        }
        else {
            earlierCreatedChild_7.updateWithValueParams({
                title: "getDeviceState"
            });
            View.create(earlierCreatedChild_7);
        }
        ListItem.pop();
        List.pop();
        Text.create("接口调用结果： " + this.message);
        Text.textAlign(TextAlign.Start);
        Text.backgroundColor({ "id": 125829123, "type": 10001, params: [] });
        Text.pop();
        Text.create("connectionStateChange： " + this.connectionStateInfo);
        Text.margin({ top: "20vp" });
        Text.textAlign(TextAlign.Start);
        Text.backgroundColor({ "id": 125829123, "type": 10001, params: [] });
        Text.pop();
        Column.pop();
        Column.pop();
        Row.pop();
    }
    onPageShow() {
        this.bluetoothState = BTProfileModel_1.default.getBluetoothState();
        if (this.bluetoothState != BTProfileModel_1.default.BLUETOOTH_STATE_OFF) {
            this.profileNotExist = "蓝牙未打开，请先开启蓝牙后再使用！";
            return;
        }
        this.hasProfile = BTProfileModel_1.default.hasProfile(BTProfileModel_1.default.PROFILE_A2DP_SOURCE);
        console.log("bt_test pageType: " + this.pageType);
        switch (this.pageType) {
            case 0:
                this.title = { "id": 16777218, "type": 10003, params: [] };
                this.profileExist = "A2DP exist";
                this.profileNotExist = "A2DP does not exist";
                this.profileType = BTProfileModel_1.default.PROFILE_A2DP_SOURCE;
                break;
            case 1:
                this.title = { "id": 16777258, "type": 10003, params: [] };
                this.profileExist = "HANDS exist";
                this.profileNotExist = "HANDS does not exist";
                this.profileType = BTProfileModel_1.default.PROFILE_HANDS_FREE_AUDIO_GATEWAY;
                break;
            case 2:
                this.title = { "id": 16777259, "type": 10003, params: [] };
                this.profileExist = "HID HOST exist";
                this.profileNotExist = "HID HOST does not exist";
                this.profileType = BTProfileModel_1.default.PROFILE_HID_HOST;
                break;
            default:
                _system_router_1.back();
        }
        this.hasProfile = BTProfileModel_1.default.hasProfile(this.profileType);
        if (this.hasProfile) {
            console.log("bt_test registered connectionStateChange type: " + this.profileType);
            BTProfileModel_1.default.getProfile(this.profileType).on("connectionStateChange", (date) => {
                this.connectionStateInfo = JSON.stringify(date);
                this.btConnectionState = date.state;
            });
        }
    }
    onPageHide() {
        if (this.hasProfile) {
            BTProfileModel_1.default.getProfile(this.profileType).off("connectionStateChange");
        }
        console.log("bt_test Ability end");
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
/******/ 			"./pages/subManualApiTest/subProfileTest/a2dpProfileTest": 0
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
/******/ 	var __webpack_exports__ = __webpack_require__.O(undefined, ["commons"], () => (__webpack_require__("../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/pages/subManualApiTest/subProfileTest/a2dpProfileTest.ets?entry")))
/******/ 	__webpack_exports__ = __webpack_require__.O(__webpack_exports__);
/******/ 	_2a33209e6af7922da1521a8a828bfbd0 = __webpack_exports__;
/******/ 	
/******/ })()
;
//# sourceMappingURL=a2dpProfileTest.js.map