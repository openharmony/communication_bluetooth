# 蓝牙组件指引

## 项目定位

本仓库对应 OpenHarmony `foundation/communication/bluetooth`。优先按这些目录定位问题：

- `bluetooth_ui`：蓝牙文件分享。
- `frameworks/`、`interfaces/`：Native、NAPI、ETS 和 C API。
- `tools/ohos-bluetoothTool`：蓝牙CLI工具。
- `test/`、`test/fuzztest/`：单元测试和 fuzz 目标。

## 构建和验证

构建命令从 OpenHarmony 源码根目录执行，不在本子目录执行。

优先使用独立编译命令：

```bash
hb build bluetooth -i
```

若有整仓代码环境，可执行下述命令：

```bash
./build.sh --product-name rk3568 --build-target input --ccache
prebuilts/build-tools/linux-x86/bin/ninja -C out/rk3568 bluetooth
```

提交使用 `git commit -s`，并新增 `Co-Authored-By: Agent` 信息。

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

## 知识索引

稳定背景知识放在 `docs/knowledge/`。改动前按场景读取对应文件：

| 场景                   | 先读                           |
| :--------------------- | :----------------------------- |
| NAPI、ETS、NDK接口开发 | `docs/knowledge/interfaces.md` |

## 项目约束

- 不要违反代码的层次结构。
- git commit 里的 Signed-off-by 必须使用系统自带的。







