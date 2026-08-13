/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef BT_FUNC_HOOK_H
#define BT_FUNC_HOOK_H

#include "bt_config.h"
#include "service_util.h"

#ifdef __cplusplus
extern "C" {
#endif
/**********  Function hook  ***************/
/**
 * @brief 蓝牙打开钩子函数声明
 *
 * 当蓝牙打开时，会调用此函数。
 */
extern void OnBluetoothOnHook(void);

/**
 * @brief 蓝牙关闭钩子函数声明
 *
 * 当蓝牙关闭时，会调用此函数。
 */
extern void OnBluetoothOffHook(void);

/**
 * @brief 蓝牙初始化钩子函数声明
 *
 * 当蓝牙初始化时，会调用此函数。
 */
extern void OnBluetoothInitHook(void);

/**
 * @brief 判断是否允许自动连接的钩子函数声明
 *
 * @return 返回true表示允许自动连接，否则不允许。
 */
extern bool IsAutoConnectEnabledHook(void);

/**
 * @brief 蓝牙ACL断开原因钩子函数声明
 *
 * 当蓝牙设备断开连接时，会调用此函数。
 *
 * @param device 设备地址
 * @param reason 断开原因
 */
extern void OnAclDisconnectReasonHook(const OHOS::bluetooth::RawAddress &device, int reason);

/**
 * @brief 判断是否允许启用HFP AG呼叫的钩子函数声明
 *
 * @return 返回true表示允许启用HFP AG呼叫，否则不允许。
 */
extern bool IsAllowHfpAgCallHook(std::string address);

/**
 * @brief 判断是否允许连接HFP AG SCO的钩子函数声明
 *
 * @return 返回true表示允许连接HFP AG SCO，否则不允许。
 */
extern bool IsAllowConnectHfpAgScoHook(void);

/**
 * @brief 蓝牙ble高速模式设置最小连接间隔的钩子函数声明
 *
 * 当业务模块调用Gatt的接口设置高速模式时会调用此函数。
 *
 * @return 返回最interval配置string
 */
extern const char* GetBleMinConnectionIntervalHook(void);

/**
 * @brief 手表退出STR模式后是否自动打开蓝牙
 *
 * 当拉起蓝牙进程初始化adapter manager时会调用该函数
 *
 * @return 返回true表示自动打开蓝牙，否则不打开
 */
extern bool IsEnableBluetoothAfterExitStrModeHook(void);

/**
 * @brief 判断蓝牙是否弹配对弹框
 *
 * @param device 设备地址
 *
 * @return 返回true表示弹配对弹框，否则不弹框
 */
BT_FUNC_HOOK bool IsAllowPairDialogHook(const OHOS::bluetooth::RawAddress &device);

/**
 * @brief 手表退出STR模式时处理异常的函数
 *
 * @param callerName 调用enable的进程名称
 *
 * @return 返回0表示退出STR模式无异常，正常打开蓝牙，否则重新执行进入STR模式关闭AP蓝牙的流程
 */
extern int StrModeAbnormalExitHandleHook(const OHOS::bluetooth::BTTransport transport);

/**
 * @brief 取消STR模式切换事件注册
 *
 * 当卸载蓝牙的SA时调用该函数
 */
extern void OnBluetoothDestroyHook(void);

/**
 * @brief 判断是否要立即停止A2DP播放音乐
 * @param hasSessionPlaying 当前播放状态
 * @param id 播放ID
 * @return 当卸载蓝牙的SA时调用该函数
 */
extern bool QuickStopOffloadPlayHook(void);

/**
 * @brief 手表拨号时判断是否要立即拨号
 * @param address 拨号的设备
 * @param hfpState 当前hfp连接状态
 * @param number 拨号的电话号码
 * @return 是否直接拨号
 */
extern bool StartDialHook(std::string address, const int hfpState, const std::string &number);

/**
 * @brief 手表蓝牙进入到turning off状态
 */
extern void BluetoothTurningOffHook(void);
/**********  Function hook  ***************/
#ifdef __cplusplus
}
#endif

#endif // BT_FUNC_HOOK_H
