var _2a33209e6af7922da1521a8a828bfbd0;
/******/ (() => { // webpackBootstrap
var __webpack_exports__ = {};
/*!****************************************************************************************************************************!*\
  !*** ../../../../../gitee/commbluetooth/test/example/BluetoothTest/entry/src/main/ets/MainAbility/pages/profile.ets?entry ***!
  \****************************************************************************************************************************/
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
        Text.create(this.message);
        Text.fontSize({ "id": 16777384, "type": 10002, params: [] });
        Text.lineHeight({ "id": 16777480, "type": 10002, params: [] });
        Text.fontColor({ "id": 16777292, "type": 10001, params: [] });
        Text.fontWeight(FontWeight.Medium);
        Text.margin({ left: { "id": 16777374, "type": 10002, params: [] } });
        Text.textAlign(TextAlign.Start);
        Text.pop();
        Column.pop();
    }
}
loadDocument(new Profile("1", undefined, {}));

_2a33209e6af7922da1521a8a828bfbd0 = __webpack_exports__;
/******/ })()
;
//# sourceMappingURL=profile.js.map