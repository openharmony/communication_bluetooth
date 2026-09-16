# 调试与日志定位

## 一、日志域命名规律（按层过滤）

代码中 `LOG_TAG` 遵循 `bt_<层>_<模块>` 命名，可直接按关键字过滤 hilog：

| 层 | LOG_TAG 规律 | 实例（OPP 链路） |
| --- | --- | --- |
| NAPI | `bt_napi_<模块>` | bt_napi_opp、bt_napi_connection |
| C++ API | `bt_fwk_<模块>` | bt_fwk_opp |
| IPC proxy/stub | `bt_ipc_<模块>_proxy` / `_stub` | bt_ipc_opp_proxy |
| Server | `bt_server_<模块>` | bt_server_opp |
| Service 业务 | `bt_service_<模块>_<文件>` | bt_service_opp_service_impl、bt_service_opp_statemachine |
| 应用层（bluetooth_ui） | 能力名标识 | BluetoothServiceAbility、BluetoothShare 等 |

抓取示例：

```bash
hilog | grep "bt_napi_opp"        # 按单域过滤
hilog | grep -E "bt_fwk_opp|bt_ipc_opp|bt_server_opp"  # 按链路过滤
```

## 二、HCI 日志

- 以二进制原文件提供（`.cfa` / `btsnoop` / `hci.log` 等），分析前先转换为可读文本。
- **判读原则（第一判据）**：
  - 有请求无响应 → 问题在响应方（接收端）；
  - 请求未发出 → 问题在发起方（发送端）；
  - 响应带异常码 → 按 HCI/协议响应码表直接定位拒因；
  - 本机 hilog 仅作辅助验证，不作为定责依据。

## 三、bluetoothTool（CLI 调试工具）

`tools/ohos-bluetoothTool`，安装位置 `/system/bin/cli_tool/executable/ohos-bluetoothTool`。

| 命令 | 说明 |
| --- | --- |
| `ohos-bluetoothTool enable-bt` | 开启蓝牙 |
| `ohos-bluetoothTool disable-bt` | 关闭蓝牙 |
| `ohos-bluetoothTool get-state` | 查询状态（stdout 输出单行 JSON） |
| `ohos-bluetoothTool help` | 帮助 |

- 构建：`./build.sh --product-name <product> --build-target ohos-bluetoothTool`（源码根目录执行）
- 权限：`ohos.permission.ACCESS_BLUETOOTH`
- 适用：开关控制、状态查询、脚本化调试；不覆盖配对/连接/传输（用应用或 SDK 验证）
