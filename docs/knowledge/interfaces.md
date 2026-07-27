# 接口实现知识

本文只记录NAPI、ETS接口实现的相关知识。



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
