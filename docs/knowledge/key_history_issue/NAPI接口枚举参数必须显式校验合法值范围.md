---
name: KP-001 NAPI 接口枚举参数必须显式校验合法值范围
description: NAPI 参数解析中对枚举类型参数只做 number 类型检查后直接 static_cast，不做合法值校验时，非法枚举值会被静默接受或在下游以非预期错误码失败，必须在接口入口显式校验并抛参数错误
type: project
recallCount: 0
---

# KP-001: NAPI 接口枚举参数必须显式校验合法值范围

## 严重级别
major

## 来源
fusion 仓 partnerAgent：`isDeviceBound` / `isDeviceControlEnabled` 等 6 个接口的 `addressType` / `rawAddressType` 传入非法枚举值仍可正常调用（检视意见核查发现）

**问题详情**：
- 严重级别：一般（Major）——参数校验缺失，违反 API 契约
- 影响范围：共用同一解析函数 `NapiParsePartnerDeviceAddress` 的全部 6 个接口（bindDevice / unbindDevice / isDeviceBound / enableDeviceControl / disableDeviceControl / isDeviceControlEnabled）
- 出现概率：必现（传入非法枚举值即可触发）
- 故障表现：不抛 401；无绑定设备时静默成功返回，有绑定设备时以 IPC 层错误收敛为内部错误码

## 问题回顾

应用传入非法枚举值（如 `addressType: 0/3/99`）。合法定义域为 `BluetoothAddressType { VIRTUAL=1, REAL=2 }`、`BluetoothRawAddressType { PUBLIC=0, RANDOM=1 }`。

实际链路：
1. NAPI 解析层仅做 number 类型检查，任意 int32 直接 `static_cast` 放行；
2. 无绑定设备时 inner client 短路 `return FCM_NO_ERROR`，非法值静默通过，应用完全无感；
3. 有绑定设备走了 IPC 时，服务端反序列化（Unmarshalling）有范围校验会拒绝，但表现为 IPC 层错误（收敛为内部错误码 34900099），**不是语义正确的 401**。

即：非法值要么静默成功，要么以错误码语义丢失的方式失败，唯一该拦的 NAPI 入口没有拦。

## 代码陷阱模式

**陷阱：NAPI 参数解析对枚举参数只做"类型检查（number）"后直接 `static_cast<Enum>(value)`，不做合法值范围校验。**

此陷阱的隐蔽性在于：
1. 合法值调用路径功能完全正常，常规自测通过；
2. 非法值的行为依赖下游路径——设备未绑定时短路成功，测试环境若未构造完整数据（如未先绑定设备），无法观察到异常；
3. 下游某层可能有防御性校验（如 IPC 反序列化），让人误以为"已有校验"——但拦截点错误码语义丢失，且存在覆盖不到的路径（短路路径不经过它）。

## 高风险代码区域

### 1. NAPI 参数解析函数（缺口所在）
- **文件**：`partner_agent/frameworks/napi/src/napi_partner_device_agent.cpp`
- `NapiParsePartnerDeviceAddress`：`address` 有 `IsValidAddress` 校验，但 `addressType` / `rawAddressType` 仅 number 检查后 `static_cast`——**不对称**，同类字段有的校验有的不校验时最容易被漏
- 该函数被 6 个接口共用，一处缺失全链路放行

### 2. inner client 短路路径（掩盖问题）
- **文件**：`partner_agent/frameworks/inner/src/partner_device_agent.cpp`
- `IsDeviceBound` / `IsDeviceControlEnabled`：无绑定设备时直接 `return FCM_NO_ERROR`（读 persist 参数判断），非法参数不被任何代码检查

### 3. IPC 反序列化（有校验但拦截表现错误）
- **文件**：`partner_agent/idl/src/partner_device_address.cpp`
- `Unmarshalling` 对枚举值有范围校验，非法返回 nullptr——但错误码经白名单收敛后应用侧丢失语义（非 401）

### 4. 蓝牙仓同源位置（同类风险自检）
- 蓝牙仓 NAPI 解析中的 `static_cast<枚举>`（`frameworks/js/napi/src/`，各模块的 Parse/Check 函数）
- 蓝牙仓已有正面范例：`napi_bluetooth_utils.cpp` 的 `ParseAddressInfoParam` 对 addressType/rawAddressType 显式校验后返回 `napi_invalid_arg`

## 检查规则

### Step 1: 搜索可疑的枚举强转
```bash
# 在 NAPI 解析函数中查找对使用者入参的类枚举强转
grep -r -n "static_cast<.*Type>\|static_cast<.*Enum>" frameworks/js/napi/src --include="*.cpp"
```

### Step 2: 逐一核对枚举合法定义域
对每个枚举参数回答：
1. 枚举的合法值域是什么？（注意值可能不连续、起点非 0，如 VIRTUAL=1、PUBLIC=0）
2. 解析函数是否做了范围/集合校验？
3. 同一结构体/对象中其余字段是否有校验？（不对称往往就是缺口）

### Step 3: 校验收敛路径
1. 非法值是否在入口即抛参数错误（401）？
2. 该错误码是否在接口的 validErrCodes 白名单内？
3. 用"先构造完整上下文（如先绑设备）再传非法值"的方式复测——避免被短路路径掩盖

## 正确模式

```cpp
NAPI_FCM_CALL_RETURN(NapiParseInt32Object(env, bluetoothAddressValue, "addressType", addressType));
NAPI_FCM_RETURN_IF(addressType < static_cast<int32_t>(BluetoothAddressType::VIRTUAL) ||
    addressType > static_cast<int32_t>(BluetoothAddressType::REAL),
    "Invalid addressType", napi_invalid_arg);   // → 上层断言统一抛 401
```

## 反面模式

```cpp
NAPI_FCM_CALL_RETURN(NapiParseInt32Object(env, bluetoothAddressValue, "addressType", addressType));
// 只查了 number 类型，非法值直接造型放行
outAddr = PartnerDeviceAddress(address, static_cast<BluetoothAddressType>(addressType));
```

## 关联知识

- 蓝牙仓参数校验规范与错误码机制：`../interfaces.md`（参数校验 / 错误码机制节）
- 修复要点：校验失败返回 `napi_invalid_arg` 后，上层 `NAPI_BT_ASSERT_NUM_RETURN_VERIFY` 自动抛 401；确认 401 已在各接口 validErrCodes 白名单内
