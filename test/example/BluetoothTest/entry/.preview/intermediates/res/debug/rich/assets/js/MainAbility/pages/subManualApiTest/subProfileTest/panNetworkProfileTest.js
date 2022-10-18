var _840471a2ef2afe531ee6aea9267b2e21;
/******/ (() => { // webpackBootstrap
/******/ 	"use strict";
/******/ 	var __webpack_modules__ = ({

/***/ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/pages/subManualApiTest/subProfileTest/panNetworkProfileTest.ets?entry":
/*!******************************************************************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/pages/subManualApiTest/subProfileTest/panNetworkProfileTest.ets?entry ***!
  \******************************************************************************************************************************************************************************/
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {


var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
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
const contentTable_1 = __webpack_require__(/*! ../../../../Component/contentTable */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Component/contentTable.ets");
const testDataModels_1 = __webpack_require__(/*! ../../../model/testDataModels */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/testDataModels.ets");
var _system_router_1  = globalThis.requireNativeModule('system.router');
const btProfileModel_1 = __importDefault(__webpack_require__(/*! ../../../model/btProfileModel */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/model/btProfileModel.ets"));
const ConfigData_1 = __importDefault(__webpack_require__(/*! ../../../../Utils/ConfigData */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Utils/ConfigData.ts"));
const LogUtil_1 = __importDefault(__webpack_require__(/*! ../../../../Utils/LogUtil */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Utils/LogUtil.ts"));
const pageTitle_1 = __webpack_require__(/*! ../../../../Component/pageTitle */ "../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/Component/pageTitle.ets");
class Index extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.__hasProfile = new ObservedPropertySimple(false, this, "hasProfile");
        this.__message = AppStorage.GetOrCreate().setAndLink("textMessage", "", this);
        this.__isConnection = new ObservedPropertySimple(false, this, "isConnection");
        this.__connectionStateInfo = new ObservedPropertySimple("", this, "connectionStateInfo");
        this.__title = new ObservedPropertyObject({ "id": 16777270, "type": 10003, params: [] }, this, "title");
        this.__profileExist = new ObservedPropertySimple("", this, "profileExist");
        this.__profileNotExist = new ObservedPropertySimple("", this, "profileNotExist");
        this.__profileType = new ObservedPropertySimple(0, this, "profileType");
        this.__bluetoothState = new ObservedPropertySimple(0, this, "bluetoothState");
        this.__btConnectionState = new ObservedPropertySimple(0, this, "btConnectionState");
        this.__currentClick = new ObservedPropertySimple(-1, this, "currentClick");
        this.pageType = 7;
        this.deviceId = "8C:5A:F8:25:FB:E9";
        this.updateWithValueParams(params);
    }
    updateWithValueParams(params) {
        if (params.hasProfile !== undefined) {
            this.hasProfile = params.hasProfile;
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
        if (params.currentClick !== undefined) {
            this.currentClick = params.currentClick;
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
        this.__currentClick.aboutToBeDeleted();
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
    get currentClick() {
        return this.__currentClick.get();
    }
    set currentClick(newValue) {
        this.__currentClick.set(newValue);
    }
    aboutToAppear() {
        LogUtil_1.default.log(ConfigData_1.default.TAG + "xzyj: aboutToAppear a2dp: deviceId " + JSON.stringify(this.deviceId));
        AppStorage.SetOrCreate("deviceId", this.deviceId);
        //    btProfileModel.initProfile()
    }
    render() {
        Row.create();
        Row.debugLine("pages/subManualApiTest/subProfileTest/panNetworkProfileTest.ets(48:5)");
        Row.backgroundColor({ "id": 125829132, "type": 10001, params: [] });
        Row.height('100%');
        Row.alignItems(VerticalAlign.Top);
        Column.create();
        Column.debugLine("pages/subManualApiTest/subProfileTest/panNetworkProfileTest.ets(49:7)");
        Column.width('100%');
        let earlierCreatedChild_2 = this.findChildById("2");
        if (earlierCreatedChild_2 == undefined) {
            View.create(new pageTitle_1.PageTitle("2", this, { detail: 'PanNetWork测试' }));
        }
        else {
            earlierCreatedChild_2.updateWithValueParams({
                detail: 'PanNetWork测试'
            });
            if (!earlierCreatedChild_2.needsUpdate()) {
                earlierCreatedChild_2.markStatic();
            }
            View.create(earlierCreatedChild_2);
        }
        Stack.create();
        Stack.debugLine("pages/subManualApiTest/subProfileTest/panNetworkProfileTest.ets(51:9)");
        Stack.height("0.5vp");
        Stack.backgroundColor("#000000");
        Stack.pop();
        Text.create(this.hasProfile ? this.profileExist : this.profileNotExist);
        Text.debugLine("pages/subManualApiTest/subProfileTest/panNetworkProfileTest.ets(52:9)");
        Text.margin({ top: "20vp" });
        Text.fontSize("20vp");
        Text.fontColor(this.hasProfile ? "#66ccff" : "#ff0000");
        Text.border({ width: 0.5 });
        Text.padding(5);
        Text.fontStyle(FontStyle.Italic);
        Text.pop();
        Column.create();
        Column.debugLine("pages/subManualApiTest/subProfileTest/panNetworkProfileTest.ets(59:9)");
        Column.visibility(this.hasProfile ? Visibility.Visible : Visibility.Hidden);
        Row.create();
        Row.debugLine("pages/subManualApiTest/subProfileTest/panNetworkProfileTest.ets(60:11)");
        Row.backgroundColor({ "id": 16777304, "type": 10001, params: [] });
        Row.margin({ top: "20vp", bottom: "20vp", left: "20vp", right: "20vp" });
        Row.alignItems(VerticalAlign.Center);
        Text.create("现在正在连接的设备为：");
        Text.debugLine("pages/subManualApiTest/subProfileTest/panNetworkProfileTest.ets(61:13)");
        Text.fontSize("17");
        Text.width(110);
        Text.pop();
        TextInput.create({ text: JSON.stringify(this.deviceId), placeholder: "请输入需要连接的deviceId" });
        TextInput.debugLine("pages/subManualApiTest/subProfileTest/panNetworkProfileTest.ets(63:13)");
        TextInput.fontSize("17vp");
        TextInput.onChange((str) => {
            this.deviceId = str;
            AppStorage.SetOrCreate("deviceId", str);
        });
        TextInput.width("70%");
        TextInput.borderRadius(1);
        Row.pop();
        Column.create();
        Column.debugLine("pages/subManualApiTest/subProfileTest/panNetworkProfileTest.ets(75:11)");
        Column.backgroundColor({ "id": 125829123, "type": 10001, params: [] });
        Column.width("88%");
        Text.create("接口调用结果： " + this.message);
        Text.debugLine("pages/subManualApiTest/subProfileTest/panNetworkProfileTest.ets(76:13)");
        Text.fontSize("17vp");
        Text.textAlign(TextAlign.Start);
        Text.pop();
        Text.create("connectionStateChange： " + this.connectionStateInfo);
        Text.debugLine("pages/subManualApiTest/subProfileTest/panNetworkProfileTest.ets(79:13)");
        Text.fontSize("17vp");
        Text.margin({ top: "10vp" });
        Text.textAlign(TextAlign.Start);
        Text.pop();
        Column.pop();
        Text.create("请点击需要测试的方法");
        Text.debugLine("pages/subManualApiTest/subProfileTest/panNetworkProfileTest.ets(87:11)");
        Text.margin({ top: "15vp" });
        Text.padding("5vp");
        Text.fontSize("17vp");
        Text.fontColor("#4444444");
        Text.pop();
        Scroll.create();
        Scroll.debugLine("pages/subManualApiTest/subProfileTest/panNetworkProfileTest.ets(92:11)");
        Scroll.height("70%");
        Scroll.scrollable(ScrollDirection.Vertical);
        Scroll.scrollBar(BarState.On);
        Scroll.scrollBarColor(Color.Gray);
        Scroll.scrollBarWidth(20);
        let earlierCreatedChild_3 = this.findChildById("3");
        if (earlierCreatedChild_3 == undefined) {
            View.create(new contentTable_1.ContentTable("3", this, { apiItems: testDataModels_1.initPanNetworkProfileApiData() }));
        }
        else {
            earlierCreatedChild_3.updateWithValueParams({
                apiItems: testDataModels_1.initPanNetworkProfileApiData()
            });
            View.create(earlierCreatedChild_3);
        }
        Scroll.pop();
        Column.pop();
        Column.pop();
        Row.pop();
    }
    onPageShow() {
        this.bluetoothState = btProfileModel_1.default.getBluetoothState();
        if (this.bluetoothState == btProfileModel_1.default.BLUETOOTH_STATE_OFF) {
            this.profileNotExist = "蓝牙未打开，请先开启蓝牙后再使用！";
            return;
        }
        this.hasProfile = btProfileModel_1.default.hasProfile(btProfileModel_1.default.PROFILE_PAN_NETWORK);
        console.log("bt_test pageType: " + this.pageType);
        switch (this.pageType) {
            case 0:
                this.title = { "id": 16777218, "type": 10003, params: [] };
                this.profileExist = "A2DP_SINK exist";
                this.profileNotExist = "A2DP_SINK does not exist";
                this.profileType = btProfileModel_1.default.PROFILE_A2DP_SINK;
                break;
            case 1:
                this.title = { "id": 16777219, "type": 10003, params: [] };
                this.profileExist = "A2DP_SOURCE exist";
                this.profileNotExist = "A2DP_SOURCE does not exist";
                this.profileType = btProfileModel_1.default.PROFILE_A2DP_SOURCE;
                break;
            case 2:
                this.title = { "id": 16777224, "type": 10003, params: [] };
                this.profileExist = "AVRCP_CT exist";
                this.profileNotExist = "AVRCP_CT does not exist";
                this.profileType = btProfileModel_1.default.PROFILE_AVRCP_CT;
                break;
            case 3:
                this.title = { "id": 16777225, "type": 10003, params: [] };
                this.profileExist = "AVRCP_TG exist";
                this.profileNotExist = "AVRCP_TG does not exist";
                this.profileType = btProfileModel_1.default.PROFILE_AVRCP_TG;
                break;
            case 4:
                this.title = { "id": 16777261, "type": 10003, params: [] };
                this.profileExist = "HANDS_FREE_AUDIO_GATEWAY exist";
                this.profileNotExist = "HANDS_FREE_AUDIO_GATEWAY does not exist";
                this.profileType = btProfileModel_1.default.PROFILE_HANDS_FREE_AUDIO_GATEWAY;
                break;
            case 5:
                this.title = { "id": 16777262, "type": 10003, params: [] };
                this.profileExist = "HANDS_FREE_UNIT exist";
                this.profileNotExist = "HANDS_FREE_UNIT does not exist";
                this.profileType = btProfileModel_1.default.PROFILE_HANDS_FREE_UNIT;
                break;
            case 6:
                this.title = { "id": 16777263, "type": 10003, params: [] };
                this.profileExist = "HID HOST exist";
                this.profileNotExist = "HID HOST does not exist";
                this.profileType = btProfileModel_1.default.PROFILE_HID_HOST;
                break;
            case 7:
                this.title = { "id": 16777270, "type": 10003, params: [] };
                this.profileExist = "PAN_NETWORK exist";
                this.profileNotExist = "PAN_NETWORK does not exist";
                this.profileType = btProfileModel_1.default.PROFILE_PAN_NETWORK;
                break;
            case 8:
                this.title = { "id": 16777266, "type": 10003, params: [] };
                this.profileExist = "PBAP_CLIENT exist";
                this.profileNotExist = "PBAP_CLIENT does not exist";
                this.profileType = btProfileModel_1.default.PROFILE_PBAP_CLIENT;
                break;
            case 9:
                this.title = { "id": 16777267, "type": 10003, params: [] };
                this.profileExist = "PBAP_SERVER exist";
                this.profileNotExist = "PBAP_SERVER does not exist";
                this.profileType = btProfileModel_1.default.PROFILE_PBAP_SERVER;
                break;
            default:
                _system_router_1.back();
        }
        AppStorage.SetOrCreate("profileType", this.profileType);
        this.hasProfile = btProfileModel_1.default.hasProfile(this.profileType);
        if (this.hasProfile) {
            console.log("bt_test registered connectionStateChange type: " + this.profileType);
            btProfileModel_1.default.getProfile(this.profileType).on("connectionStateChange", (date) => {
                this.connectionStateInfo = JSON.stringify(date);
                this.btConnectionState = date.state;
                AppStorage.SetOrCreate("btConnectionState", this.btConnectionState);
            });
        }
    }
    onPageHide() {
        if (this.hasProfile) {
            btProfileModel_1.default.getProfile(this.profileType).off("connectionStateChange");
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
/******/ 			"./pages/subManualApiTest/subProfileTest/panNetworkProfileTest": 0
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
/******/ 	var __webpack_exports__ = __webpack_require__.O(undefined, ["commons"], () => (__webpack_require__("../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/pages/subManualApiTest/subProfileTest/panNetworkProfileTest.ets?entry")))
/******/ 	__webpack_exports__ = __webpack_require__.O(__webpack_exports__);
/******/ 	_840471a2ef2afe531ee6aea9267b2e21 = __webpack_exports__;
/******/ 	
/******/ })()
;
//# sourceMappingURL=panNetworkProfileTest.js.map