var _840471a2ef2afe531ee6aea9267b2e21;
/******/ (() => { // webpackBootstrap
var __webpack_exports__ = {};
/*!********************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest-bak/entry/src/main/ets/MainAbility/pages/profile.ets?entry ***!
  \********************************************************************************************************************************/
class Profile extends View {
    constructor(compilerAssignedUniqueChildId, parent, params) {
        super(compilerAssignedUniqueChildId, parent);
        this.__message = new ObservedPropertySimple('Profile', this, "message");
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
        Column.debugLine("pages/profile.ets(25:5)");
        Text.create(this.message);
        Text.debugLine("pages/profile.ets(26:7)");
        Text.fontSize({ "id": 16777391, "type": 10002, params: [] });
        Text.lineHeight({ "id": 16777487, "type": 10002, params: [] });
        Text.fontColor({ "id": 16777299, "type": 10001, params: [] });
        Text.fontWeight(FontWeight.Medium);
        Text.margin({ left: { "id": 16777381, "type": 10002, params: [] } });
        Text.textAlign(TextAlign.Start);
        Text.pop();
        Column.pop();
    }
}
loadDocument(new Profile("1", undefined, {}));

_840471a2ef2afe531ee6aea9267b2e21 = __webpack_exports__;
/******/ })()
;
//# sourceMappingURL=profile.js.map