# ohos-bluetoothTool

## 项目概述

ohos-bluetoothTool 是一个用于蓝牙设备管理的命令行工具，封装了 OpenHarmony BluetoothHost 核心接口，提供蓝牙开关控制和状态查询功能。

## 功能列表

- ✅ 开启蓝牙
- ✅ 关闭蓝牙
- ✅ 查询蓝牙状态
- ✅ 查询经典蓝牙和BLE启用状态

## 目录结构

```
tools/ohos-bluetoothTool/
├── src/
│   ├── main.cpp              # CLI入口，参数解析和命令分发
│   ├── bluetooth_tool.cpp    # BluetoothHost接口封装
│   ├── bluetooth_tool.h      # 核心功能头文件
│   ├── command_executor.cpp  # 命令执行器
│   ├── command_executor.h    # 命令执行器头文件
│   ├── utils.cpp             # 工具函数
│   └── utils.h               # 工具函数头文件
├── docs/
│   ├── README.md             # 详细使用文档
│   └── DESIGN.md             # 设计文档
├── tests/
│   └ TEST.md                 # 测试文档
├── BUILD.gn                  # GN构建配置
├── config.json               # 工具描述文件
└── README.md                 # 项目说明（本文件）
```

## 构建方法

### 编译命令

```bash
# 在 OpenHarmony 源码根目录执行
./build.sh --product-name <product> --build-target ohos-bluetoothTool
```

### 安装位置

编译后可执行文件安装至：
```
/system/bin/cli_tool/executable/ohos-bluetoothTool
```

## 使用方法

### 基本用法

```bash
ohos-bluetoothTool <command> [options]
```

### 命令列表

| 命令 | 说明 | 示例 |
|------|------|------|
| enable-bt | 开启蓝牙 | `ohos-bluetoothTool enable-bt` |
| disable-bt | 关闭蓝牙 | `ohos-bluetoothTool disable-bt` |
| get-state | 查询状态 | `ohos-bluetoothTool get-state` |
| help | 显示帮助 | `ohos-bluetoothTool help` |

### 查看帮助

```bash
# 全量帮助
ohos-bluetoothTool --help

# 单命令帮助
ohos-bluetoothTool enable-bt --help
```

## 输出格式

所有命令输出均为单行 JSON 格式（stdout），日志和帮助信息输出至 stderr。

**成功示例**：
```json
{"type":"result","status":"success","data":{"message":"Bluetooth enabled successfully"}}
```

**失败示例**：
```json
{"type":"result","status":"failed","errCode":"ERR_BT_ENABLE_FAILED","errMsg":"Failed to enable Bluetooth","suggestion":"Please check if Bluetooth service is running..."}
```

## 权限要求

- **ohos.permission.ACCESS_BLUETOOTH**：蓝牙访问权限

## 依赖说明

- **BluetoothHostService**：蓝牙系统服务（SA）
- **BluetoothHost**：inner_kits层接口
- **libhilog**：日志库
- **ipc_single**：IPC通信库

## 详细文档

- 使用文档：[docs/README.md](docs/README.md)
- 设计文档：[docs/DESIGN.md](docs/DESIGN.md)
- 测试文档：[tests/TEST.md](tests/TEST.md)

## 适用场景

- 系统运维：远程控制蓝牙开关
- 故障诊断：查询蓝牙适配器状态
- 自动化测试：脚本化蓝牙管理
- 设备调试：快速切换蓝牙状态

## 不适用场景

- 蓝牙设备配对管理
- 蓝牙连接管理
- 蓝牙服务发现
- 蓝牙数据传输

## 许可证

Apache License 2.0