# ohos-bluetoothTool

## 概述

ohos-bluetoothTool 是一个用于蓝牙设备管理的命令行工具，提供开启、关闭蓝牙以及查询蓝牙状态的功能。该工具封装了 OpenHarmony BluetoothHost 的核心接口，适用于运维诊断和蓝牙状态查询场景。

## 功能列表

- 开启蓝牙（BLE模式）
- 关闭蓝牙
- 查询蓝牙开关状态
- 查询经典蓝牙和BLE的启用状态

## 依赖

- 系统能力：BluetoothHost（inner_kits层）
- 权限：ohos.permission.ACCESS_BLUETOOTH
- 服务：BluetoothHostService（SA）

## 基本用法

```bash
ohos-bluetoothTool <command> [options]
```

## 命令列表

| 命令 | 说明 | 参数 | 权限 | 前置依赖 |
|------|------|------|------|----------|
| enable-bt | 开启蓝牙（BLE模式） | 无 | ohos.permission.ACCESS_BLUETOOTH | 无 |
| disable-bt | 关闭蓝牙 | 无 | ohos.permission.ACCESS_BLUETOOTH | 无 |
| get-state | 获取蓝牙开关状态 | 无 | ohos.permission.ACCESS_BLUETOOTH | 无 |
| help | 显示帮助信息 | [command] | 无 | 无 |

**前置依赖说明**：
- **无**：所有命令均可独立执行，无需前置条件

## 示例

```bash
# 示例 1：开启蓝牙（无前置依赖）
ohos-bluetoothTool enable-bt

# 示例 2：关闭蓝牙（无前置依赖）
ohos-bluetoothTool disable-bt

# 示例 3：查询蓝牙状态（无前置依赖）
ohos-bluetoothTool get-state

# 示例 4：查看工具帮助
ohos-bluetoothTool --help

# 示例 5：查看特定命令帮助
ohos-bluetoothTool enable-bt --help
```

## 输出格式

所有命令输出均为单行 JSON 格式：

**成功示例**：
```json
{"type":"result","status":"success","data":{"message":"Bluetooth enabled successfully"}}
```

**失败示例**：
```json
{"type":"result","status":"failed","errCode":"ERR_BT_ENABLE_FAILED","errMsg":"Failed to enable Bluetooth","suggestion":"Please check if Bluetooth service is running and you have ohos.permission.ACCESS_BLUETOOTH permission"}
```

## 状态码说明

| 状态码 | 状态名称 | 说明 |
|--------|---------|------|
| 0 | STATE_OFF | 蓝牙已关闭 |
| 1 | STATE_TURNING_ON | 蓝牙正在开启 |
| 2 | STATE_ON | 经典蓝牙已开启 |
| 3 | STATE_TURNING_OFF | 蓝牙正在关闭 |
| 4 | STATE_BLE_TURNING_ON | BLE正在开启 |
| 5 | STATE_BLE_ON | BLE已开启 |
| 6 | STATE_BLE_TURNING_OFF | BLE正在关闭 |

## 错误码说明

| 错误码 | 说明 |
|--------|------|
| ERR_BT_ENABLE_FAILED | 开启蓝牙失败 |
| ERR_BT_DISABLE_FAILED | 关闭蓝牙失败 |
| ERR_INVALID_COMMAND | 无效的命令名称 |

## 使用场景

- 系统运维：远程控制蓝牙设备开关
- 故障诊断：查询蓝牙适配器状态
- 自动化测试：脚本化蓝牙状态管理
- 设备调试：快速切换蓝牙状态