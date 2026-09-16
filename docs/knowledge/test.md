# 单元测试

## 一、测试域与注册位置

入口：`test/BUILD.gn` 的 `group("unit_test")`，聚合四个域（每个域内为 `group("unittest")`）：

| 域 | 目录 | 现有用例（示例） | 被测对象 |
| --- | --- | --- | --- |
| inner | `test/unittest/inner` | `bluetooth_uuid_module_test` | `frameworks/inner/src/uuid.cpp` |
| napi | `test/unittest/napi` | `bluetooth_napi_event_subscribe_module_test` | `frameworks/js/napi/src/common/napi_event_subscribe_module.cpp` |
| taihe | `test/unittest/taihe` | `bluetooth_taihe_utils_module_test` | `frameworks/ets/taihe/common/taihe_bluetooth_utils.cpp` |
| cj | `test/unittest/cj` | `bluetooth_cj_connection_common_module_test` | `frameworks/cj/connection/src/bluetooth_connection_common.cpp` |

选择原则：改哪层的公共逻辑，就近放入对应域；对 NAPI 之外无法独立编译的模块，直接引用 `FRAMWORK_DIR` 源文件参与编译。

## 二、新增用例模板

在对应域 `BUILD.gn` 中新增 `ohos_unittest` 目标：

```gn
ohos_unittest("bluetooth_xxx_module_test") {
    module_out_path = module_output_path
    sources = [
        "$FRAMWORK_DIR/<被测源文件路径>.cpp",
        "xxx_test.cpp",
    ]
    configs = [ ":module_private_config" ]   # 含 private=public，可测私有成员
    external_deps = [
        "c_utils:utils",
        "hilog:libhilog",
        "googletest:gmock_main",
        "googletest:gtest_main",
    ]
}
```

要点：

- 注册到域内 `group("unittest")` 的 deps，注意外层有 `if (is_phone_product)` 条件（非 phone 形态不编译）；
- 复杂依赖（需要链接库）参考 `services/bluetooth/service/BUILD.gn` 中的 `*_static` 目标（"Only used for unitest" 注释的静态库）；
- 测试代码可访问 `private/protected` 成员（`module_private_config` 中的宏放开），可用于验证内部状态。

## 三、编译

从 OpenHarmony 源码根目录执行：

```bash
prebuilts/build-tools/linux-x86/bin/ninja -C out/rk3568 bluetooth_uuid_module_test
```

产物输出到 `out/rk3568/tests/unittest/bluetooth/bluetooth/` 下，由单测框架调度执行。
