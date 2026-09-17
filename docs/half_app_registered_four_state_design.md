# 蓝牙开关四态（STATE_HALF_APP_REGISTERED）设计文档

| 项目 | 内容 |
| --- | --- |
| 主题 | 蓝牙开关三态扩展为四态，新增 HALF_APP_REGISTERED（半开 + 注册应用专属 BLE）态 |
| 状态 | 已实现 |
| 涉及模块 | service(common)、server、ipc、frameworks/inner、interfaces/inner_api |

## 1. 背景与需求

蓝牙开关原有三态：

| 状态 | 属性值 | 含义 |
| --- | --- | --- |
| STATE_ON | `persist.bluetooth.switch_enable = "1"` | BLE + BR 双栈开启，全部功能可用 |
| STATE_OFF | `"0"` | 双栈关闭 |
| STATE_HALF | `"2"` | 受限半开：仅 BLE 开启，BR 对上层隐藏 |

新增第四态 **STATE_HALF_APP_REGISTERED**（属性值 `"3"`）：

- 底层 BLE、BR 双栈**均保持开启**（便于快速切回全开，不重启协议栈）；
- 对上层**仅放行 BLE 功能**，BR 的发现/配对/连接对所有应用一律拒绝；
- BLE 功能仅允许"打开蓝牙的应用"（owner，记录调用方包名，持久化于
  `persist.bluetooth.half_app_registered_owner`）及系统应用使用，其他应用静默视为蓝牙关闭。

## 2. 状态机设计

状态机集中在独立文件中管理：

- 头文件：`services/bluetooth/service/include/bluetooth_switch_state_machine.h`
- 实现：`services/bluetooth/service/src/common/bluetooth_switch_state_machine.cpp`
- 类：`BluetoothSwitchStateMachine`（单例）

### 2.1 状态机图（线性模型，9 条合法边）

```mermaid
stateDiagram-v2
    [*] --> STATE_OFF : 开机 InitFromProperty()

    STATE_OFF : STATE_OFF "0"\n双栈关闭
    STATE_ON : STATE_ON "1"\n双栈开启 全功能
    STATE_HALF : STATE_HALF "2"\n受限半开 仅BLE栈
    STATE_HALF_APP_REGISTERED : STATE_HALF_APP_REGISTERED "3"\n双栈开启 仅owner可用BLE

    OFF --> ON : EnableBle（全开）
    OFF --> HALF : EnableBluetoothToRestrictMode
    OFF --> HALF_APP_REGISTERED : EnableBluetoothToHalfAppRegisteredMode

    HALF --> ON : EnablebluetoothFromRestricted（升级）
    HALF --> OFF : DisableBt（关闭）

    HALF_APP_REGISTERED --> ON : EnableBluetoothFromHalfAppRegisteredMode（升级）
    HALF_APP_REGISTERED --> OFF : DisableBt（关闭）
    HALF_APP_REGISTERED --> HALF : EnableBluetoothToRestrictMode（降半开，栈不重启）

    ON --> OFF : DisableBt

    STATE_OFF --> [*] : 双栈全关 卸载SA
```

迁移规则说明：

- 开启从 OFF 出发有三条路（ON / HALF / HALF_APP_REGISTERED）；
- 两个受限态（HALF、HALF_APP_REGISTERED）只能**升级为 ON** 或**关闭为 OFF**；
- HALF_APP_REGISTERED 因双栈已在运行，额外允许**降级为 HALF**（只改上层的可见性/权限，不动协议栈）；
- 禁止：ON→HALF、ON→HALF_APP_REGISTERED（不允许从全开直接降级，watch 等旧路径经
  `SetBluetoothRestrictedFlag` 的 fallback 兼容，不走迁移表）、HALF→HALF_APP_REGISTERED；
- 非法迁移打 HILOG 并返回 `BT_ERR_INVALID_STATE`；同态迁移为幂等 no-op；
  `EnterHalfAppRegisteredMode` 重复进入仅刷新 owner；
- `SyncState()` 仅更新内存态不落盘，用于 disable 路径（属性 "0" 由双栈全关时的卸载流程写入）。

### 2.2 主要接口

| 接口 | 说明 |
| --- | --- |
| `TransitionTo(target)` | 合法迁移 + 持久化（同态幂等） |
| `EnterHalfAppRegisteredMode(ownerName)` | 进入 HALF_APP_REGISTERED 并记录 owner（同态幂等，仅刷新 owner） |
| `SyncState(state)` | 仅内存同步，不落盘 |
| `InitFromProperty()` | 重启恢复状态与 owner |
| `IsHalfAppRegisteredMode() / IsBluetoothRestricted()` | 状态查询 |
| `IsOwnerAccessible(callingName)` | owner 匹配判断（不含系统应用放行） |
| `IsBrAllowed()` | BR 功能是否放行（仅 HALF_APP_REGISTERED 为 false） |

`AdapterManager` 对外封装 `IsBleAccessible(callingName) = IsSystemHap() || IsOwnerAccessible(callingName)`，状态机本身不依赖 PermissionManager，便于独立单测。

`AdapterManager::EnableBluetoothToHalfAppRegisteredMode` 在发起开栈动作前预检查当前状态必须为
OFF 或 HALF_APP_REGISTERED（重入刷新 owner），否则直接返回 `BT_ERR_INVALID_STATE`，避免"栈已开始拉起但状态置入被拒"的不一致。

## 3. 关键时序图

### 3.1 进入 HALF_APP_REGISTERED（从全关）

```mermaid
sequenceDiagram
    autonumber
    participant App as 应用(未来owner)
    participant Host as BluetoothHost(framework)
    participant SM as BluetoothSwitchModule
    participant Proxy as BluetoothHostProxy(IPC)
    participant Srv as BluetoothHostServer
    participant AM as AdapterManager
    participant BSM as BluetoothSwitchStateMachine
    participant Stk as BLE/BR协议栈

    App->>Host: EnableBluetoothToHalfAppRegisteredMode(name)
    Host->>SM: ProcessBluetoothSwitchEvent(ENABLE_BLUETOOTH_TO_HALF_APP_REGISTERED)
    SM->>Proxy: EnableBluetoothToHalfAppRegisteredMode(callingName)
    Proxy->>Srv: BT_ENABLE_BLUETOOTH_TO_HALF_APP_REGISTERED_MODE
    alt fusion 场景且非允许调用方
        Srv-->>SM : 转移给融合互联 BT_ERR_SWITCH_OP_TRANSFERRED
    else 直接处理
        Srv->>AM: EnableBluetoothToHalfAppRegisteredMode(realCallingName)
        AM->>BSM : 预检状态∈{OFF, HALF_APP_REGISTERED}
        AM->>Stk: Enable(ADAPTER_BLE)（BLE开启后自动拉起BR）
        AM->>BSM: EnterHalfAppRegisteredMode(callingName) 记录owner/属性="3"
        AM-->>Srv: BT_NO_ERROR
        Srv-->>SM: BT_NO_ERROR
        Note over SM : isBtSwitchProcessing_=true 等待完成事件
        Stk-->>AM: BR STATE_TURN_ON
        AM->>AM: OnAdapterStateChange 拦截BR-on上报
        AM->>SM : V2通知 STATE_HALF_APP_REGISTERED
        SM->>SM : ProcessBluetoothHalfAppRegisteredEvent 结束开关动作
    end
```

### 3.2 HALF_APP_REGISTERED 下功能鉴权

```mermaid
sequenceDiagram
    autonumber
    participant Other as 其他应用
    participant Owner as owner应用/系统应用
    participant Srv as Server(各服务入口)
    participant AM as AdapterManager
    participant BSM as BluetoothSwitchStateMachine

    Note over Other,Srv : BLE 扫描/广播/GATT连接
    Other->>Srv : StartScan / StartAdvertising / Connect
    Srv->>AM : IsHalfAppRegisteredMode() && !IsBleAccessible(callingName)
    AM->>BSM : IsOwnerAccessible(callingName)
    BSM-->>AM : false（非owner）
    AM-->>Srv : 拒绝（静默视为蓝牙未开启）
    Owner->>Srv : 同类请求
    Srv->>AM : IsBleAccessible(callingName)
    AM-->>Srv : true（owner或系统应用）
    Srv-->>Owner : 正常执行

    Note over Other,Srv : BR 发现/配对/扫描模式
    Other->>Srv : StartBtDiscovery / Pair / SetBtScanMode
    Srv->>AM : IsBrAllowed()
    AM-->>Srv : false（HALF_APP_REGISTERED 下恒为 false）
    Srv-->>Other : BT_ERR_RESTRICT_STATE
```

### 3.3 HALF_APP_REGISTERED 升级为全开

```mermaid
sequenceDiagram
    autonumber
    participant App as 应用
    participant Host as BluetoothHost(framework)
    participant SM as BluetoothSwitchModule
    participant Srv as BluetoothHostServer
    participant AM as AdapterManager
    participant BSM as BluetoothSwitchStateMachine

    App->>Host: EnableBle / enableBluetooth
    Host->>SM: ENABLE_BLUETOOTH
    SM->>Srv: EnableBt
    Srv->>AM: Enable(ADAPTER_BLE)
    AM->>Srv : IsHalfAppRegisteredMode()==true
    Srv->>AM : EnableBluetoothFromHalfAppRegisteredMode(callingName)
    AM->>BSM : TransitionTo(STATE_ON)（清owner/属性="1"）
    Note over AM : 双栈已在运行，无需重启栈
    AM->>SM : 补发 BR STATE_TURN_ON 公共事件 + V2 STATE_ON
    AM->>AM : 恢复扫描模式/自动连接
    SM->>SM : BLUETOOTH_ON 结束开关动作
```

## 4. 代码改动清单

| 层 | 文件 | 改动 |
| --- | --- | --- |
| 公共定义 | `frameworks/inner/ipc/common/bt_def.h` | `BluetoothSwitchState` 新增 `STATE_HALF_APP_REGISTERED`；`BluetoothTransferredSwitchAction` 新增 `TRANS_ACTION_ENABLE_BLUETOOTH_TO_HALF_APP_REGISTERED` |
| 状态机 | `services/bluetooth/service/include/bluetooth_switch_state_machine.h`、`src/common/bluetooth_switch_state_machine.cpp` | 新增四态状态机（独立文件），9 边线性迁移表 |
| service | `adapter_manager.{h,cpp}`、`interface_adapter_manager.h` | `EnableBluetoothToHalfAppRegisteredMode`（含入态预检）/ `EnableBluetoothFromHalfAppRegisteredMode` / `IsHalfAppRegisteredMode` / `IsBleAccessible` / `IsBrAllowed`；`OnAdapterStateChange` 拦截 BR-on 上报改发 STATE_HALF_APP_REGISTERED；`Initialize` 重启恢复；restrict 标志同步走状态机 |
| 功能拦截 | `classic_adapter.cpp`、`bluetooth_ble_central_manager_server.cpp`、`bluetooth_ble_advertiser_server.cpp`、`gatt_client_service.cpp` | BR 全禁 + BLE owner 鉴权 |
| IPC | `bluetooth_service_ipc_interface_code.h`、`i_bluetooth_host.h`、`bluetooth_host_proxy.{h,cpp}`、`bluetooth_host_stub.{h,cpp}` | 新增 `BT_ENABLE_BLUETOOTH_TO_HALF_APP_REGISTERED_MODE` 接口码及代理/桩 |
| server | `bluetooth_host_server.{h,cpp}` | 服务端接口 + fusion 转移 + `GetBtState` 可见性控制 + `EnableBt` 升级路径 |
| framework | `bluetooth_switch_module.{h,cpp}`、`bluetooth_host.cpp`、`interfaces/inner_api/include/bluetooth_host.h` | SwitchModule 新增事件/缓存重放规则；`BluetoothHost::EnableBluetoothToHalfAppRegisteredMode` 内部 API |
| 测试 | `test/unittest/inner/bluetooth_switch_state_machine_test.cpp`、`BUILD.gn` | 状态机单测（9 条合法迁移/非法迁移拒绝/同态幂等/owner 鉴权/重启恢复/属性映射） |

## 5. 状态可见性矩阵

| 调用方 | STATE_ON | STATE_HALF | STATE_HALF_APP_REGISTERED |
| --- | --- | --- | --- |
| 系统应用 | ON | BLE ON | BLE ON |
| owner 应用 | ON | OFF（弹框流程） | BLE ON |
| 其他三方应用 | ON | OFF | OFF（静默） |

## 6. 验证方式

- 单测：`bluetooth_switch_state_machine_test`
- 编译：`hb build bluetooth -i`
- 手工用例：三方应用 A 调 `EnableBluetoothToHalfAppRegisteredMode` → A 可扫描/GATT，B 扫描/广播/GATT 失败且 `getBtState` 为 OFF；A 或设置开蓝牙 → 全开；重启后 HALF_APP_REGISTERED 与 owner 恢复。
