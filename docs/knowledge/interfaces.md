# 接口实现知识

本文只记录 NAPI、ETS、Native 接口实现的相关知识。

## 接口分层链路

d.ts 契约（interface_sdk-js，仓外）→ NAPI（`frameworks/js/napi/src/<模块>/`）→ C++ API（`frameworks/inner/src/`）→ IPC（`frameworks/inner/ipc/`）→ Server（`services/bluetooth/server/src/`）→ Service（`services/bluetooth/service/src/`）

- ETS(ArkTS) 接口：`frameworks/ets/taihe/<模块>/`
- Native(C) 接口：`frameworks/c_api/`，头文件 `interfaces/c_api/include/oh_bluetooth.h`

## 项目约束

- 新增接口需实现HA打点和错误码范围强校验，使用宏 `NAPI_BT_CONTEXT`，不要单独定义 `NapiHaEventUtils` 对象。
- 错误码范围需和接口定义保持一致，（401错误码默认支持），接口定义可从OpenHarmony根路径 `interface/sdk-js/*bluetooth*.d.ts` 获取。

```
例如接口错误码范围为：

   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Non-system applications are not allowed to use system APIs.
   * @throws { BusinessError } 801 - Capability not supported.
   * @throws { BusinessError } 2900001 - Service stopped.
   * @throws { BusinessError } 2900003 - Bluetooth disabled.
   * @throws { BusinessError } 2900099 - Operation failed.

则该接口仅支持返回错误码[201, 202, 401, 801, 2900001, 2900003, 2900099]
```

- 异步接口`Promise`接口使用`NapiAsyncWorkFactory`实现，使用宏 `CREATE_ASYNC_WORK_WITH_CONTEXT`。
- 新接口不允许使用不带 `NUM` 的 `NAPI_BT_ASSERT`的宏。

## 错误码机制

- 唯一源：`interfaces/inner_api/include/bluetooth_errorcode.h`（枚举 `BtErrCode`）
  - 公共错误码：201 / 202 / 203 / 401 / 801
  - 业务错误码：2900000 段。各域分段：GATT 2901000+、SPP 2901050+、HFP 2902000+、BLE 2902050+、OPP 2903001+、HID 2903050+；2900099 内部错误、2900100 IPC 失败
- 处理链（`frameworks/js/napi/src/common/napi_bluetooth_error.cpp`）：
  1. `NAPI_BT_CONTEXT` 声明本接口的 validErrCodes 白名单；
  2. 抛错路径 `HandleSyncErrNumAdapter` → `ProcessErrCode` 白名单校验；
  3. **不在白名单的错误码会被收敛为 2900099**（应用侧丢失错误语义）——实现中可能返回的每个错误码都必须写进该接口的 validErrCodes；
  4. 白名单内的错误码还需要在 `napiErrMsgMap` 中配置文案，**否则 errMsg 为空导致该错误不抛出（静默失败）**。
- 错误码必须以 number 类型抛出（`GenerateBusinessError` 走 `napi_create_int32`）；使用 NUM 系列宏。`HandleSyncErr*`（非 Num 版本）为历史 string 形式，不要在新接口中使用。
- 异步回调路径同样经过 `ProcessErrCode`（`frameworks/js/napi/src/common/napi_async_work.cpp`），白名单不全会同样被收敛。
- **SDK 版本适配**：`GetSDKAdaptedStatusCode`（`napi_bluetooth_utils.cpp`）对 SDK_VERSION_20 之前应用将新引入的状态码收敛为 `BT_ERR_INTERNAL_ERROR`——新增/调整错误码需评估旧版本应用兼容。

## HA 打点

- `NAPI_BT_CONTEXT(env, apiName, validErrCodes)` 在栈上创建 `ApiContext`（含 `NapiHaEventUtils`），并将白名单绑定到上下文。
- 同步抛错路径（`HandleSyncErrNumAdapter`）与异步完成回调（`NapiAsyncWork`）会自动 `WriteErrCode` 上报。
- 无需打点的接口使用 `NAPI_BT_CONTEXT_WITHOUT_HA`。

## 参数校验

- 遵循 d.ts 声明做类型与范围校验；非法参数返回 `BT_ERR_INVALID_PARAM`(401)。
- 枚举类参数需显式校验合法值集合/范围，不要直接 `static_cast` 使用者传入的数值。
- 地址参数使用 `IsValidAddress` 校验格式。
- 参数解析统一放在 `CheckXxxParam` / `ParseXxx` 函数中，返回 `napi_status`；错误码由 `NAPI_BT_ASSERT_NUM_RETURN_VERIFY` 等 NUM 宏系列统一抛出。

## 参考实现

- 接口类模板与参数解析：`frameworks/js/napi/src/opp/napi_bluetooth_opp.cpp`
- 错误处理核心：`frameworks/js/napi/include/napi_bluetooth_error.h`、`frameworks/js/napi/src/common/napi_bluetooth_error.cpp`
- 宏定义：`NAPI_BT_CONTEXT`、`CREATE_ASYNC_WORK_WITH_CONTEXT`、`NAPI_BT_ASSERT_NUM_*` 均在 `napi_bluetooth_error.h`
