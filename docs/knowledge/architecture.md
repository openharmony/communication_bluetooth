# 蓝牙仓架构：关键机制与流程枢纽

> 本文只记录跨模块的机制性知识（设计模式与流程入口），不重复目录分层与模块位置（见 AGENTS.md）。

## 一、关键设计机制

1. **单例 + 代理访问**：应用侧 `BluetoothHost::GetDefaultHost()` 是全局唯一入口；内部经 IPC proxy 访问服务端 `BluetoothHostServer`，应用无感跨进程。
2. **SA 生命周期**：SA 1130 按需启动（`sa_profile/1130.json` 的 start-on-demand 参数/事件触发，如 `persist.bluetooth.switch_enable`）；空闲时由 `AdapterManager` 的卸载定时器机制回收（`UnLoadBluetoothSystemAbility`）。
3. **状态机体系**：全仓共用 `utility::StateMachine` 基类（嵌套 `State` 类 + `Transition` 迁移）；adapter 与每个 profile 各自实现业务状态机（如 `AdapterStateMachine`、`OppStateMachine`）。
4. **观察者 + 应用容器**：服务端用 `BluetoothApplicationContainer`（按 pid/uid 管理注册应用）与 `RemoteObserverList`（跨进程观察者列表）向多个应用分发事件。
5. **Profile 独立动态库**：大 profile（OPP/MAP/PBAP/HearingAid/HID Device 等）编译为独立 .so 并按需装卸；开关联动由 `config/BUILD.gn` 的 feature_flags 控制。
6. **线程与定时器抽象**：`ThreadUtil`、`TimerManager` 提供 ffrt / 原生双实现，由 `bluetooth_ffrt_enable` 编译期切换（`BtRecursiveMutex` 宏同理）。

## 二、核心流程入口（速查）

| 流程 | 入口 | 关键枢纽 |
| --- | --- | --- |
| 蓝牙开关 | NAPI host.enableBluetooth → `BluetoothHostServer::EnableBt` | `AdapterManager::Enable/Disable`（联动各 profile 启停：`OnProfileServicesEnableComplete`） |
| 设备配对/连接 | connection 模块 API → server → `ClassicAdapter` / `BleAdapter` | `ConnectStrategyManager`、各 profile 状态机 |
| 事件订阅分发 | app `on/off` → observer 注册 | `BluetoothObserverApplicationContainer`、`RemoteObserverList` |
| OPP 文件传输 | opp NAPI → `BluetoothOppServer` → `OppService` | `OppStateMachine`、OBEX 适配（opp_obex_*） |

## 三、新增模块/文档时的基线

- 新增 profile 走读文档建议结构：目录职责 → 关键类 → 状态机/数据流 → 相邻层交互点 → 错误码与日志关键字。
- 新增机制说明必须落到"类/文件"级别，避免只写概念。
