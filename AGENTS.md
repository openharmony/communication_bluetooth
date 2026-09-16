# 蓝牙组件指引

## 项目定位

本仓库对应 OpenHarmony `foundation/communication/bluetooth`，实现系统蓝牙服务（SA 1130）及其全部对外接口。其中四类应用接口（NAPI/ETS/FFI/Native）面向所有加载了 SDK 的应用开放。

- `bluetooth_ui/`：蓝牙系统应用 HAP（本仓自带的系统应用，作为普通应用通过 SDK 公开接口调用所需能力；当前承载文件分享等功能，后续可扩展）。
- `frameworks/js/napi/`、`frameworks/ets/taihe/`、`frameworks/cj/`、`frameworks/c_api/`、`interfaces/`：五类接口（平级）——NAPI（JS 应用）、ETS（ArkTS 应用）、FFI（仓颉应用）、Native（原生应用）、C++ API（面向系统部件/平台开发）。
- `idl/`：IDL 接口定义。
- `frameworks/inner/`：所有接口的 IPC proxy 端。
- `services/bluetooth/server/`：所有接口的 IPC stub 端（蓝牙服务入口）。
- `services/bluetooth/service/`：核心业务（ble/classic/gatt/各 profile）。
- `stack_adapter/`：协议栈适配。
- `services/bluetooth/hardware/`：HDI 适配（芯片驱动通道）。
- `tools/ohos-bluetoothTool/`：蓝牙 CLI 调试工具。
- `test/`：单元测试（`unit_test`）。

### 多语言接口形态

**仅当任务涉及对外接口（新增 / 修改 / 定位接口）时读取分析**；不涉及对外接口的改动可跳过本节。

NAPI / ETS（ArkTS）/ FFI（仓颉）/ Native（C）/ C++ API 五类接口平级，各自面向一类使用者：NAPI 给 JS 应用，ETS 给 ArkTS 应用，FFI 给仓颉应用，Native 给原生应用，C++ API 给系统部件与平台开发。四类应用接口（NAPI/ETS/FFI/Native）的实现基于 C++ API 底座。

| 接口形态 | 服务对象 | 代码标识 |
| --- | --- | --- |
| NAPI | JS 应用 | `napi_bluetooth_*` |
| ETS | ArkTS 应用 | `*.taihe` + impl |
| FFI | 仓颉应用 | `FfiBluetooth*`（`FFI_EXPORT`） |
| Native | 原生应用（C/C++） | `OH_Bluetooth_*`（`frameworks/c_api` 导出） |
| C++ API | 系统部件、平台开发（platformsdk） | `bluetooth_*.h`（`BluetoothHost` 等） |

判断改哪个目录，看**调用方是谁**（应用按语言/运行时区分；系统部件直用 C++ API）。涉及接口开发（新增/修改接口）时，先向开发者确认接口面向的使用者，再对照"按任务类型定位代码"定位目录，不要自行假设。

### 按任务类型定位代码

| 任务类型 | 对应位置 |
| --- | --- |
| 修改应用层 UI | `bluetooth_ui/entry/src/main/ets/` |
| 修改 NAPI 接口 | `frameworks/js/napi/src/<模块>/`（napi_bluetooth_*.cpp），规范见 `docs/knowledge/interfaces.md` |
| 修改 ETS 接口 | `frameworks/ets/taihe/<模块>/`（*.taihe + 实现） |
| 修改仓颉接口 | `frameworks/cj/<模块>/` |
| 修改 Native 接口 | `frameworks/c_api/src/` + `interfaces/c_api/include/oh_bluetooth.h` |
| 修改 C++ API | `interfaces/inner_api/include/bluetooth_*.h` + `frameworks/inner/src/` |
| 修改 IPC 协议 | `frameworks/inner/ipc/interface/`（接口码/接口定义）、`ipc/src/`（proxy/stub）、`ipc/parcel/`（序列化） |
| 修改 IPC stub 端 | `services/bluetooth/server/src/bluetooth_*_server.cpp` |
| 新增/修改 Profile 业务 | `services/bluetooth/service/src/<profile>/`，并在 `service/BUILD.gn` 注册源文件 |
| 修改协议栈适配 | `services/bluetooth/btchr/`、`stack_adapter*/` |
| 修改 Profile 开关/平台宏 | `bluetooth.gni`（declare_args）+ `config/BUILD.gn`（feature_flags → 宏） |
| 修改 SA 注册/自启动 | `sa_profile/1130.json`、`services/bluetooth/etc/` |
| 新增单元测试 | `test/unittest/{napi,inner,taihe,cj}/`，并注册到对应 BUILD.gn 的 `group("unittest")` |

### 嵌套指引

本仓库无目录级嵌套 AGENTS.md；所有任务级指引通过本文件与 `docs/knowledge/` 场景文档提供。

## 构建和验证

构建命令从 OpenHarmony 源码根目录执行，不在本子目录执行。

单模块编译：

```bash
# 注意：-T 传的是 gn target 名（如 btframework）或完整 label（路径:target），
# 组件名（bluetooth）不是 ninja target，会报 unknown target！
hb build -T btframework -T bluetooth_hap
hb build -T foundation/communication/bluetooth/frameworks/js/napi:bluetooth
```

优先使用独立编译命令：

```bash
hb build bluetooth -i
```

若有整仓代码环境，可执行下述命令：

```bash
./build.sh --product-name rk3568 --build-target input --ccache
prebuilts/build-tools/linux-x86/bin/ninja -C out/rk3568 bluetooth
```

### 关键 gn target

| target | 位置 | 产物/说明 |
| --- | --- | --- |
| `btframework` | `frameworks/inner` | libbtframework.so：C++ API + IPC + C 适配（平台 SDK） |
| `bluetooth` / `bluetoothmanager` | `frameworks/js/napi` | NAPI 模块库 |
| `btservice` | `services/bluetooth/service` | libbtservice.so：核心服务（ble/classic/gatt 等） |
| `bluetooth_server` | `services/bluetooth/server` | libbluetooth_server.so：服务端入口 |
| `btopp` 等 profile 库 | `services/bluetooth/service/BUILD.gn` | 各 profile 独立动态库：btopp / btmap_mse / btpbap_pse / btbas / bthearing_aid_impl / bthiddevice / btavrcp_media / btfast_scan / btpan |
| `bluetooth_hap` | `bluetooth_ui` | Bluetooth.hap（系统应用） |
| `*_static` | 各模块 | 单测用静态库（标注 "Only used for unitest"） |

### 完成标准

任务被认为完成，当且仅当：

1. **代码改动已提交** - 按"提交规范"提交；
2. **构建通过** - 执行对应构建命令（`hb build bluetooth -i` 或对应 gn target）；
3. **相关测试通过** - 对应单元测试通过（如有）；
4. **接口改动已核对** - 错误码与 d.ts @throws 对齐、HA 打点正常（见 `docs/knowledge/interfaces.md`）；
5. **真机链路提供证据** - 涉及连接/传输/音频/配对等真实设备链路的改动，附验证证据（日志或截图）。

### 如果无法运行验证

明确说明无法运行的原因，列出推荐的验证步骤供人工执行，显式标记需要人工验证的部分。

### 完成报告格式

报告应包含：改动摘要（文件列表、改动点）、验证结果（构建/测试输出）、风险评估（API 兼容性、行为变化）、未完成事项。

## 知识索引

稳定背景知识放在 `docs/knowledge/`。改动前按场景读取：

### 场景与路径路由

| 场景 | 先读文档 |
| --- | --- |
| NAPI、ETS、Native 接口开发（错误码、HA 打点、异步接口） | `docs/knowledge/interfaces.md` |
| 跨模块机制与关键流程入口（状态机/观察者/SA 生命周期） | `docs/knowledge/architecture.md` |
| 日志定位、HCI 日志、bluetoothTool 调试 | `docs/knowledge/debug.md` |
| 单元测试编写与运行 | `docs/knowledge/test.md` |
| 历史问题案例（缺陷模式与自查规则，改动前对照） | `docs/knowledge/key_history_issue/` |

### 开始编辑前

在修改代码前，按以下顺序确认：

1. 对照"按任务类型定位代码"确定任务类别；
2. 读取"场景与路径路由"对应的知识文档；
3. 核对"项目约束"，确认不违反红线；
4. 声明："本次修改 X，已阅读 Y 文档，遵循 Z 约束"。

## 需求开发流程

**仅当同时满足两个前提时适用**：① 属于需求开发；② 需求涉及对外接口。其余改动（缺陷修复、内部调整等）不适用。

适用时按以下顺序推进，再进入实现：

1. **梳理对外 API**：先明确需求涉及哪些对外 API（对照"多语言接口形态"判断面向的使用者）；
2. **确认是否已有定稿 API**：直接向开发者询问——有定稿契约（d.ts / IDL / 头文件）则以定稿为准，不要自行设计；
3. **未定稿则先设计 API 层**：接口签名、错误码范围、面向的使用者，经开发者确认后再实现。

## 问题分析与案例沉淀

每次完成问题分析（检视意见核查、缺陷定位、故障排查等）后，当问题被接纳上库时，执行以下收尾流程：

1. **总结 issue 案例**：按 `docs/knowledge/key_history_issue/` 已有案例的模板总结本次问题——教训式文件名 + frontmatter（KP 编号顺延）+ 陷阱模式 + 高风险代码区域 + 检查规则 + 正/反模式；
2. **征询固化**：向开发者确认是否需要将案例固化上库；
3. **确认后提交**：仅在开发者确认后按"提交规范"提交，未确认前不提交。

案例要求：落到"文件/类/函数"级；检查规则可执行（grep/步骤）；同类教训合并进已有案例，不重复建卡。

## 项目约束

### 架构与层次

- 不要违反代码的层次结构（NAPI/C API → IPC → server → service）。不要在接口层夹带业务逻辑，不要在 service 层绕过 server 直连应用。

### 编码约定

- NAPI 新增接口必须使用 `NAPI_BT_CONTEXT` 声明错误码范围（自动携带 HA 打点），不要单独定义 `NapiHaEventUtils` 对象；错误码范围必须与接口 d.ts 的 @throws 声明一致（401 默认支持）。
- 异步（Promise）接口使用 `NapiAsyncWorkFactory` / `CREATE_ASYNC_WORK_WITH_CONTEXT` 实现。
- 新接口不允许使用不带 `NUM` 的 `NAPI_BT_ASSERT` 宏；错误码路径使用 `NAPI_BT_ASSERT_NUM_RETURN_VERIFY` 等 NUM 系列宏。
- 错误码统一使用 `interfaces/inner_api/include/bluetooth_errorcode.h` 的 `BtErrCode`，不要硬编码数字；不要新增 `NO_ERROR`/`ERROR`/`ERR_INVALID_STATE` 等废弃别名。
- HILOG 日志不能明文打印蓝牙地址：使用 `GET_ENCRYPT_ADDR` / `GET_ENCRYPT_RAW_ADDR` / `GET_ENCRYPT_GATT_ADDR`（底层 `GetEncryptAddr`）。

### 公共 API 约束

**Do not（禁止）：**
- 修改已发布 NAPI/ArkTS/C API 的签名、参数类型、返回值类型；
- 修改已有 API 的错误码（除非明确标注废弃）；
- 删除或重命名已有公共 API。

**Ask before（修改前必须确认）：**
- 新增公共 API：确认 d.ts 声明（interface_sdk-js）、错误码范围、权限要求；
- 修改已有 API 的行为语义（异步/同步、阻塞/非阻塞）。

### 安全与权限边界

**Do not（禁止）：**
- 绕过已有权限检查（PermissionItem / IsSystemCaller 判定）；
- 在未校验的场景使用跨进程传递的文件描述符；
- 在日志中输出敏感信息（地址、设备标识等，统一走匿名化宏）。

**Ask before（修改前必须确认）：**
- 修改 `services/bluetooth/service/src/permission/` 的权限项配置；
- 涉及多用户/账户隔离的改动。

### 协议与兼容

**Do not（禁止）：**
- 修改 IPC 接口码、parcel 序列化顺序（`frameworks/inner/ipc/parcel/`）；
- 修改已有 IDL（`idl/`、`*.taihe`）的接口签名；
- 在 IPC 边界丢弃错误码语义（错误码需透传到应用层）。

**Ask before（修改前必须确认）：**
- 新增 IPC 接口或修改接口码：确认跨版本兼容性。

### 生成代码边界

**Do not（禁止）：**
- 直接修改 IDL/ETS 编译器生成的代码。

**正确做法：**
- 修改源头定义：`idl/*.idl`（hearingaid）、`frameworks/ets/taihe/<模块>/*.taihe`；
- `frameworks/inner/ipc/` 下的 proxy/stub 为手写维护（非生成），修改时同步四处：接口码（`ipc/interface/bluetooth_service_ipc_interface_code.h`）、接口定义（`ipc/interface/i_bluetooth_*.h`）、proxy/stub（`ipc/src/`）、parcel（`ipc/parcel/`）。

### 提交规范

使用 `git commit -s`，并新增 `Co-Authored-By: Agent` 信息。Signed-off-by 必须使用系统自带的（git config 的用户信息）。

```
# git commit 信息模板

refactor(bluetooth): split TaiheBuildOobDataFromTaihe and remove magic numbers

Split the oversized TaiheBuildOobDataFromTaihe into focused helpers and
replace magic numbers (2/16) with named constants to satisfy
G.CNS.02 and G.FUD.05.

Co-Authored-By: Agent
Change-Id: I3eea26405ea4c8551ea8621aeb8e8e09672c8b15
Signed-off-by: zyjjjjjjjjjjjjjjjjj <zhangyijiu1@huawei.com>
```
