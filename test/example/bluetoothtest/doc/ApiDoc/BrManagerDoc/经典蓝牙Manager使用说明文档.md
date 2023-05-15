## 经典蓝牙Manager使用说明文档

​		本文档主要介绍蓝牙专项测试程序的经典蓝牙部分(ohos.bluetoothManager.d.ts)的功能使用说明。

#### 从主界面跳转到经典蓝牙部分



#### 经典蓝牙的主要接口(ohos.bluetoothManager.d.ts)

|           API名称            |               method名称               |                           所需参数                           |         返回值         | 备注 |
| :--------------------------: | :------------------------------------: | :----------------------------------------------------------: | :--------------------: | ---- |
|       enableBluetooth        |                开启蓝牙                |                              ()                              |          void          |      |
|       disableBluetooth       |                关闭蓝牙                |                              ()                              |          void          |      |
|         getLocalName         |          获取蓝牙本地设备名称          |                              ()                              |         string         |      |
|           getState           |            获取蓝牙开关状态            |                              ()                              |     BluetoothState     |      |
|     getBtConnectionState     |     获取蓝牙本端的Profile连接状态      |                              ()                              | ProfileConnectionState |      |
|         setLocalName         |          设置蓝牙本地设备名称          |                        (name: string)                        |          void          |      |
|          pairDevice          |              发起蓝牙配对              |                      (deviceId: string)                      |          void          |      |
|  getProfileConnectionState   | 依据ProfileId获取指定profile的连接状态 |                    (profileId: ProfileId)                    | ProfileConnectionState |      |
|      cancelPairedDevice      |           删除配对的远程设备           |                      (deviceId: string)                      |          void          |      |
|     getRemoteDeviceName      |         获取对端蓝牙设备的名称         |                      (deviceId: string)                      |         string         |      |
|     getRemoteDeviceClass     |         获取对端蓝牙设备的类别         |                      (deviceId: string)                      |      DeviceClass       |      |
|       getPairedDevices       |            获取蓝牙配对列表            |                              ()                              |     Array<string>      |      |
|     setBluetoothScanMode     |            设置蓝牙扫描模式            |              (mode: ScanMode, duration: number)              |          void          |      |
|     getBluetoothScanMode     |            获取蓝牙扫描模式            |                              ()                              |        ScanMode        |      |
|   startBluetoothDiscovery    |              开启蓝牙扫描              |                              ()                              |          void          |      |
|    stopBluetoothDiscovery    |              关闭蓝牙扫描              |                              ()                              |          void          |      |
| setDevicePairingConfirmation |          设置设备配对请求确认          |              (device: string, accept: boolean)               |          void          |      |
|  on.('bluetoothDeviceFind')  |        订阅蓝牙设备发现上报事件        | on(type: “bluetoothDeviceFind”, callback: Callback<Array<string>>) |          void          |      |
| off.('bluetoothDeviceFind')  |      取消订阅蓝牙设备发现上报事件      | off(type: “bluetoothDeviceFind”, callback?: Callback<Array<string>>) |          void          |      |
|      on.('pinRequired')      |     订阅远端蓝牙设备的配对请求事件     | on(type: “pinRequired”, callback: Callback<PinRequiredParam>) |          void          |      |
|     off.('pinRequired')      |   取消订阅远端蓝牙设备的配对请求事件   | off(type: “pinRequired”, callback?: Callback<PinRequiredParam>) |          void          |      |
|    on.('bondStateChange')    |        订阅蓝牙配对状态改变事件        | on(type: “bondStateChange”, callback: Callback<BondStateParam>) |          void          |      |
|   off.('bondStateChange')    |      取消订阅蓝牙配对状态改变事件      | off(type: “bondStateChange”, callback?: Callback<BondStateParam>) |          void          |      |
|      on.('stateChange')      |        订阅蓝牙连接状态改变事件        | on(type: “stateChange”, callback: Callback<BluetoothState>)  |          void          |      |
|     off.('stateChange')      |      取消订阅蓝牙连接状态改变事件      | off(type: “stateChange”, callback?: Callback<BluetoothState>) |          void          |      |
|                              |                                        |                                                              |                        |      |

>**接口所需的参数配置：**
>
>|  参数名   |   类型    | 必填 |                       说明                       |
>| :-------: | :-------: | :--: | :----------------------------------------------: |
>| ProfileId | profileId |  是  | 表示profile的枚举值，例如：PROFILE_A2DP_SOURCE。 |
>
>
>
>|  参数名  |   类型   | 必填 |                           说明                            |
>| :------: | :------: | :--: | :-------------------------------------------------------: |
>|   mode   | ScanMode |  是  |                      蓝牙扫描模式。                       |
>| duration |  number  |  是  | 设备可被发现的持续时间，单位为毫秒；设置为0则持续可发现。 |
>
>
>
>#### ScanMode  枚举，扫描模式
>
>| 名称                                       | 值   | 说明                    |
>| :----------------------------------------- | :--- | :---------------------- |
>| SCAN_MODE_NONE                             | 0    | 没有扫描模式。          |
>| SCAN_MODE_CONNECTABLE                      | 1    | 可连接扫描模式。        |
>| SCAN_MODE_GENERAL_DISCOVERABLE             | 2    | general发现模式。       |
>| SCAN_MODE_LIMITED_DISCOVERABLE             | 3    | limited发现模式。       |
>| SCAN_MODE_CONNECTABLE_GENERAL_DISCOVERABLE | 4    | 可连接general发现模式。 |
>| SCAN_MODE_CONNECTABLE_LIMITED_DISCOVERABLE | 5    | 可连接limited发现模式。 |
>
>
>
>|  参数名  | 类型   | 必填 |                        说明                         |
>| :------: | ------ | :--: | :-------------------------------------------------: |
>| deviceId | string |  是  | 表示配对的远端设备地址，例如：“XX:XX:XX:XX:XX:XX”。 |
>
>
>
>| 参数名 |  类型   | 必填 |                     说明                      |
>| :----: | :-----: | :--: | :-------------------------------------------: |
>| device | string  |  是  | 表示远端设备地址，例如：“XX:XX:XX:XX:XX:XX”。 |
>| accept | boolean |  是  |   接受配对请求设置为true，否则设置为false。   |
>
>
>
>**返回值：**
>
>#### BluetoothState   枚举，蓝牙开关状态
>
>| 名称                  | 值   | 说明                          |
>| :-------------------- | :--- | :---------------------------- |
>| STATE_OFF             | 0    | 表示蓝牙已关闭。              |
>| STATE_TURNING_ON      | 1    | 表示蓝牙正在打开。            |
>| STATE_ON              | 2    | 表示蓝牙已打开。              |
>| STATE_TURNING_OFF     | 3    | 表示蓝牙正在关闭。            |
>| STATE_BLE_TURNING_ON  | 4    | 表示蓝牙正在打开LE-only模式。 |
>| STATE_BLE_ON          | 5    | 表示蓝牙正处于LE-only模式。   |
>| STATE_BLE_TURNING_OFF | 6    | 表示蓝牙正在关闭LE-only模式。 |
>
>
>
>#### ProfileConnectionState    枚举蓝牙设备的profile连接状态
>
>| 名称                | 值   | 说明                  |
>| :------------------ | :--- | :-------------------- |
>| STATE_DISCONNECTED  | 0    | 表示profile已断连。   |
>| STATE_CONNECTING    | 1    | 表示profile正在连接。 |
>| STATE_CONNECTED     | 2    | 表示profile已连接。   |
>| STATE_DISCONNECTING | 3    | 表示profile正在断连。 |
>
>
>
>#### DeviceClass   描述蓝牙设备的类别
>
>| 名称            | 类型                | 可读 | 可写 | 说明                             |
>| :-------------- | :------------------ | :--- | :--- | :------------------------------- |
>| majorClass      | **MajorClass**      | 是   | 否   | 表示蓝牙设备主要类别的枚举。     |
>| majorMinorClass | **MajorMinorClass** | 是   | 否   | 表示主要次要蓝牙设备类别的枚举。 |
>| classOfDevice   | number              | 是   | 否   | 表示设备类别。                   |
>
>
>
>#### MajorClass    枚举，蓝牙设备主要类别
>
>| 名称                | 值     | 说明                 |
>| :------------------ | :----- | :------------------- |
>| MAJOR_MISC          | 0x0000 | 表示杂项设备。       |
>| MAJOR_COMPUTER      | 0x0100 | 表示计算机设备。     |
>| MAJOR_PHONE         | 0x0200 | 表示手机设备。       |
>| MAJOR_NETWORKING    | 0x0300 | 表示网络设备。       |
>| MAJOR_AUDIO_VIDEO   | 0x0400 | 表示音频和视频设备。 |
>| MAJOR_PERIPHERAL    | 0x0500 | 表示外围设备。       |
>| MAJOR_IMAGING       | 0x0600 | 表示成像设备。       |
>| MAJOR_WEARABLE      | 0x0700 | 表示可穿戴设备。     |
>| MAJOR_TOY           | 0x0800 | 表示玩具设备。       |
>| MAJOR_HEALTH        | 0x0900 | 表示健康设备。       |
>| MAJOR_UNCATEGORIZED | 0x1F00 | 表示未分类设备。     |
>
>
>
>#### MajorMinorClass     枚举，主要次要蓝牙设备类别
>
>| 名称                                      | 值     | 说明                           |
>| :---------------------------------------- | :----- | :----------------------------- |
>| COMPUTER_UNCATEGORIZED                    | 0x0100 | 表示未分类计算机设备。         |
>| COMPUTER_DESKTOP                          | 0x0104 | 表示台式计算机设备。           |
>| COMPUTER_SERVER                           | 0x0108 | 表示服务器设备。               |
>| COMPUTER_LAPTOP                           | 0x010C | 表示便携式计算机设备。         |
>| COMPUTER_HANDHELD_PC_PDA                  | 0x0110 | 表示手持式计算机设备。         |
>| COMPUTER_PALM_SIZE_PC_PDA                 | 0x0114 | 表示掌上电脑设备。             |
>| COMPUTER_WEARABLE                         | 0x0118 | 表示可穿戴计算机设备。         |
>| COMPUTER_TABLET                           | 0x011C | 表示平板电脑设备。             |
>| PHONE_UNCATEGORIZED                       | 0x0200 | 表示未分类手机设备。           |
>| PHONE_CELLULAR                            | 0x0204 | 表示便携式手机设备。           |
>| PHONE_CORDLESS                            | 0x0208 | 表示无线电话设备。             |
>| PHONE_SMART                               | 0x020C | 表示智能手机设备。             |
>| PHONE_MODEM_OR_GATEWAY                    | 0x0210 | 表示调制解调器或网关手机设备。 |
>| PHONE_ISDN                                | 0x0214 | 表示ISDN手机设备。             |
>| NETWORK_FULLY_AVAILABLE                   | 0x0300 | 表示网络完全可用设备。         |
>| NETWORK_1_TO_17_UTILIZED                  | 0x0320 | 表示使用网络1到17设备。        |
>| NETWORK_17_TO_33_UTILIZED                 | 0x0340 | 表示使用网络17到33设备。       |
>| NETWORK_33_TO_50_UTILIZED                 | 0x0360 | 表示使用网络33到50设备。       |
>| NETWORK_60_TO_67_UTILIZED                 | 0x0380 | 表示使用网络60到67设备。       |
>| NETWORK_67_TO_83_UTILIZED                 | 0x03A0 | 表示使用网络67到83设备。       |
>| NETWORK_83_TO_99_UTILIZED                 | 0x03C0 | 表示使用网络83到99设备。       |
>| NETWORK_NO_SERVICE                        | 0x03E0 | 表示网络无服务设备。           |
>| AUDIO_VIDEO_UNCATEGORIZED                 | 0x0400 | 表示未分类音频视频设备。       |
>| AUDIO_VIDEO_WEARABLE_HEADSET              | 0x0404 | 表示可穿戴式音频视频设备。     |
>| AUDIO_VIDEO_HANDSFREE                     | 0x0408 | 表示免提音频视频设备。         |
>| AUDIO_VIDEO_MICROPHONE                    | 0x0410 | 表示麦克风音频视频设备。       |
>| AUDIO_VIDEO_LOUDSPEAKER                   | 0x0414 | 表示扬声器音频视频设备。       |
>| AUDIO_VIDEO_HEADPHONES                    | 0x0418 | 表示头戴式音频视频设备。       |
>| AUDIO_VIDEO_PORTABLE_AUDIO                | 0x041C | 表示便携式音频视频设备。       |
>| AUDIO_VIDEO_CAR_AUDIO                     | 0x0420 | 表示汽车音频视频设备。         |
>| AUDIO_VIDEO_SET_TOP_BOX                   | 0x0424 | 表示机顶盒音频视频设备。       |
>| AUDIO_VIDEO_HIFI_AUDIO                    | 0x0428 | 表示高保真音响设备。           |
>| AUDIO_VIDEO_VCR                           | 0x042C | 表示录像机音频视频设备。       |
>| AUDIO_VIDEO_VIDEO_CAMERA                  | 0x0430 | 表示照相机音频视频设备。       |
>| AUDIO_VIDEO_CAMCORDER                     | 0x0434 | 表示摄像机音频视频设备。       |
>| AUDIO_VIDEO_VIDEO_MONITOR                 | 0x0438 | 表示监视器音频视频设备。       |
>| AUDIO_VIDEO_VIDEO_DISPLAY_AND_LOUDSPEAKER | 0x043C | 表示视频显示器和扬声器设备。   |
>| AUDIO_VIDEO_VIDEO_CONFERENCING            | 0x0440 | 表示音频视频会议设备。         |
>| AUDIO_VIDEO_VIDEO_GAMING_TOY              | 0x0448 | 表示游戏玩具音频视频设备。     |
>| PERIPHERAL_NON_KEYBOARD_NON_POINTING      | 0x0500 | 表示非键盘非指向外围设备。     |
>| PERIPHERAL_KEYBOARD                       | 0x0540 | 表示外设键盘设备。             |
>| PERIPHERAL_POINTING_DEVICE                | 0x0580 | 表示定点装置外围设备。         |
>| PERIPHERAL_KEYBOARD_POINTING              | 0x05C0 | 表示键盘指向外围设备。         |
>| PERIPHERAL_UNCATEGORIZED                  | 0x0500 | 表示未分类外围设备。           |
>| PERIPHERAL_JOYSTICK                       | 0x0504 | 表示周边操纵杆设备。           |
>| PERIPHERAL_GAMEPAD                        | 0x0508 | 表示周边游戏板设备。           |
>| PERIPHERAL_REMOTE_CONTROL                 | 0x05C0 | 表示远程控制外围设备。         |
>| PERIPHERAL_SENSING_DEVICE                 | 0x0510 | 表示外围传感设备设备。         |
>| PERIPHERAL_DIGITIZER_TABLET               | 0x0514 | 表示外围数字化仪平板电脑设备。 |
>| PERIPHERAL_CARD_READER                    | 0x0518 | 表示外围读卡器设备。           |
>| PERIPHERAL_DIGITAL_PEN                    | 0x051C | 表示外设数码笔设备。           |
>| PERIPHERAL_SCANNER_RFID                   | 0x0520 | 表示射频识别扫描仪外围设备。   |
>| PERIPHERAL_GESTURAL_INPUT                 | 0x0522 | 表示手势输入外围设备。         |
>| IMAGING_UNCATEGORIZED                     | 0x0600 | 表示未分类的图像设备。         |
>| IMAGING_DISPLAY                           | 0x0610 | 表示图像显示设备。             |
>| IMAGING_CAMERA                            | 0x0620 | 表示成像照相机设备。           |
>| IMAGING_SCANNER                           | 0x0640 | 表示成像扫描仪设备。           |
>| IMAGING_PRINTER                           | 0x0680 | 表示成像打印机设备。           |
>| WEARABLE_UNCATEGORIZED                    | 0x0700 | 表示未分类的可穿戴设备。       |
>| WEARABLE_WRIST_WATCH                      | 0x0704 | 表示可穿戴腕表设备。           |
>| WEARABLE_PAGER                            | 0x0708 | 表示可穿戴寻呼机设备。         |
>| WEARABLE_JACKET                           | 0x070C | 表示夹克可穿戴设备。           |
>| WEARABLE_HELMET                           | 0x0710 | 表示可穿戴头盔设备。           |
>| WEARABLE_GLASSES                          | 0x0714 | 表示可穿戴眼镜设备。           |
>| TOY_UNCATEGORIZED                         | 0x0800 | 表示未分类的玩具设备。         |
>| TOY_ROBOT                                 | 0x0804 | 表示玩具机器人设备。           |
>| TOY_VEHICLE                               | 0x0808 | 表示玩具车设备。               |
>| TOY_DOLL_ACTION_FIGURE                    | 0x080C | 表示人形娃娃玩具设备。         |
>| TOY_CONTROLLER                            | 0x0810 | 表示玩具控制器设备。           |
>| TOY_GAME                                  | 0x0814 | 表示玩具游戏设备。             |
>| HEALTH_UNCATEGORIZED                      | 0x0900 | 表示未分类健康设备。           |
>| HEALTH_BLOOD_PRESSURE                     | 0x0904 | 表示血压健康设备。             |
>| HEALTH_THERMOMETER                        | 0x0908 | 表示温度计健康设备。           |
>| HEALTH_WEIGHING                           | 0x090C | 表示体重健康设备。             |
>| HEALTH_GLUCOSE                            | 0x0910 | 表示葡萄糖健康设备。           |
>| HEALTH_PULSE_OXIMETER                     | 0x0914 | 表示脉搏血氧仪健康设备。       |
>| HEALTH_PULSE_RATE                         | 0x0918 | 表示脉搏率健康设备。           |
>| HEALTH_DATA_DISPLAY                       | 0x091C | 表示数据显示健康设备。         |
>| HEALTH_STEP_COUNTER                       | 0x0920 | 表示阶梯计数器健康设备。       |
>| HEALTH_BODY_COMPOSITION_ANALYZER          | 0x0924 | 表示身体成分分析仪健康设备。   |
>| HEALTH_PEAK_FLOW_MONITOR                  | 0x0928 | 表示湿度计健康设备。           |
>| HEALTH_MEDICATION_MONITOR                 | 0x092C | 表示药物监视仪健康设备。       |
>| HEALTH_KNEE_PROSTHESIS                    | 0x0930 | 表示膝盖假肢健康设备。         |
>| HEALTH_ANKLE_PROSTHESIS                   | 0x0934 | 表示脚踝假肢健康设备。         |
>| HEALTH_GENERIC_HEALTH_MANAGER             | 0x0938 | 表示通用健康管理设备。         |
>| HEALTH_PERSONAL_MOBILITY_DEVICE           | 0x093C | 表示个人移动健康设备。         |
>
>
>
>#### PinRequiredParam  描述配对请求参数
>
>| 名称     | 类型   | 可读 | 可写 | 说明                 |
>| :------- | :----- | :--- | :--- | :------------------- |
>| deviceId | string | 是   | 否   | 表示要配对的设备ID。 |
>| pinCode  | string | 是   | 否   | 表示要配对的密钥。   |
>
>
>
>#### BondStateParam   描述配对状态参数
>
>| 名称     | 类型      | 可读 | 可写 | 说明                 |
>| :------- | :-------- | :--- | :--- | :------------------- |
>| deviceId | string    | 是   | 否   | 表示要配对的设备ID。 |
>| state    | BondState | 是   | 否   | 表示配对设备的状态。 |
>
>
>
>#### StateChangeParam    描述profile状态改变参数
>
>| 名称     | 类型                       | 可读 | 可写 | 说明                            |
>| :------- | :------------------------- | :--- | :--- | :------------------------------ |
>| deviceId | string                     | 是   | 否   | 表示蓝牙设备地址。              |
>| state    | **ProfileConnectionState** | 是   | 否   | 表示蓝牙设备的profile连接状态。 |
>
>| 类型          | 说明                       |
>| :------------ | :------------------------- |
>| Array<string> | 已配对蓝牙设备的地址列表。 |



#### 功能

1. 开/关蓝牙

   **"蓝牙打开"是其他功能测试的前提**

   - 使用指导：点击后，在设备上启动/关闭蓝牙；根据设备的蓝牙情况，显示返回信息。

   - 限制条件：

     - 返回值为void
     - @throws { BusinessError } 201 - Permission denied.
      * @throws { BusinessError } 801 - Capability not supported.
      * @throws { BusinessError } 2900001 - Service stopped.
      * @throws { BusinessError } 2900099 - Operation failed.

   - 验证方法：可在设备的设置中查看蓝牙的开关情况

     

2. 获取状态

   - 使用指导：点击后，基于蓝牙的开关状态，判断蓝牙当时的状态。

   - 限制条件：

     - 若本地蓝牙已关闭，返回值为0，显示信息为"STATE_OFF"；
     - 若本地蓝牙正在打开，返回值为1，显示信息为"STATE_TURNING_ON"；
     - 若本地蓝牙已打开，返回值为2，显示信息为"STATE_ON"；
     - 若本地蓝牙正在关闭，返回值为3，显示信息为"STATE_TURNING_OFF"；
     - 若本地蓝牙正在打开LE模式，返回值为4，显示信息为"STATE_BLE_TURNING_ON"；
     - 若本地蓝牙处于仅LE模式，返回值为5，显示信息为"STATE_BLE_ON"；
     - 若本地蓝牙正在关闭仅LE模式，返回值为6，显示信息为"STATE_BLE_TURNING_OFF"。
     - @throws｛BusinessError｝201-权限被拒绝。
     - @throws｛BusinessError｝801-不支持功能。
     - @throws｛BusinessError｝2900001-服务已停止。
     - @throws｛BusinessError｝2900099-操作失败。

   - 验证方法：可在设备设置中查看蓝牙的当前状态

     

3. 获取当前连接状态

   - 使用指导：点击后，基于设备蓝牙的连接情况，判断蓝牙的连接状态。

   - 限制条件：

     - 若本地蓝牙未连接，返回值为0，显示信息为"STATE_DISCONNECTED"
     - 若本地蓝牙正在连接，返回值为1，显示信息为"STATE_CONNECTING"
     - 若本地蓝牙已连接，返回值为2，显示信息为"STATE_CONNECTED"
     - 若本地蓝牙正在断开连接，返回值为3，显示信息为"STATE_DISCONNECTING"
     - @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 801 - Capability not supported.
     * @throws { BusinessError } 2900001 - Service stopped.
     * @throws { BusinessError } 2900003 - Bluetooth switch is off.
     - @throws { BusinessError } 2900099 - Operation failed

   - 验证方法：可在设备设置中查看蓝牙的连接情况

     

4. 设置本地名称---->获取本地名称

   - 使用指导：点击"设置本地名称"后，会弹出弹窗，可以设置本地蓝牙名称，也可以保持默认值不变。点击"获取本地名称"，会在信息显示处显示本地当前蓝牙名称。

   - 限制条件：

     - 若更改了本地名称，获取时会显示修改后的名称；如果未修改，会直接显示默认名称。
     - @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 401 - Invalid parameter.
     * @throws { BusinessError } 801 - Capability not supported.
     * @throws { BusinessError } 2900001 - Service stopped.
     * @throws { BusinessError } 2900003 - Bluetooth switch is off.
     * @throws { BusinessError } 2900099 - Operation failed.

   - 验证方法：可在设备设置中查看本设备蓝牙名称；或是让其他蓝牙设备搜索发现本地设备蓝牙。

     

5. 设置扫描模式---->获取扫描模式

   - 使用指导：点击"设置扫描模式"后，会弹出弹窗，可以选择本地蓝牙扫描模式，设置延迟时间，也可以保持默认选项不变。点击"获取扫描模式"，会在信息显示处显示本地当前蓝牙扫描模式和延迟时间。

   - 限制条件：扫描模式共五种（保持默认选项:第四个 即可）

     - SCAN_MODE_CONNECTABLE（扫描模式可连接） 
     - SCAN_MODE_GENERAL_DISCOVERABLE（扫描模式通用可发现）
     - SCAN_MODE_LIMITED_DISCOVERABLE（扫描模式受限可发现）
     - **SCAN_MODE_CONNECTABLE_GENERAL_DISCOVERABLE（扫描模式可连接通用可发现）**
     - SCAN_MODE_CONNECTABLE_LIMITED_DISCOVERABLE（扫描模式可连接受限可发现）
     - @throws { BusinessError } 201 - Permission denied.
     - @throws { BusinessError } 401 - Invalid parameter.
     - @throws { BusinessError } 801 - Capability not supported.
     - @throws { BusinessError } 2900001 - Service stopped.
     - @throws { BusinessError } 2900003 - Bluetooth switch is off.
     - @throws { BusinessError } 2900099 - Operation failed.

   - 验证方法：选择不同的扫描模式，用其他蓝牙设备进行连接、发现来验证是否可被连接，发现与受限。

     

6. 开始蓝牙发现--->停止蓝牙发现

   - 使用指导：点击后，可以开始/停止蓝牙的搜索发现。

     - 点击"开始蓝牙发现"，进行跳转页面，会显示已发现的可配对设备和已配对设备。
     - 配对列表中会显示设备的名称，MAC地址和配对状态（0：未配对，1：配对中。2：配对成功）。
     - 点击可配对设备列表中的设备进行配对，配对成功后会显示在已配对设备的列表中。
     - 点击已配对设备列表中的取消按钮，可以取消该设备的配对。
     - @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 801 - Capability not supported.
     * @throws { BusinessError } 2900001 - Service stopped.
     * @throws { BusinessError } 2900003 - Bluetooth switch is off.
     * @throws { BusinessError } 2900099 - Operation failed.

   - 限制条件：蓝牙必须打开。

   - 验证方法：蓝牙是否可以正常发现与配对；在设备设置中查看已发现与已配对的设备。

     

7. 配对设备--->取消配对设备

   - 使用指导：

     - "配对设备"：显示当前配对结果，结果为成功或失败。
     - "取消配对设备"：取消当前的配对。

   - 限制条件：

     - 只有配对成功后，"配对设备"的返回信息才会显示"成功"，才能成功取消配对设备。
     - @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 202 - Non-system applications are not allowed to use system APIs.
     * @throws { BusinessError } 401 - Invalid parameter.
     * @throws { BusinessError } 801 - Capability not supported.
     * @throws { BusinessError } 2900001 - Service stopped.
     * @throws { BusinessError } 2900003 - Bluetooth switch is off.
     * @throws { BusinessError } 2900099 - Operation failed.

   - 验证方法：在设备设置中查看设备配对与否。

     

8. 获取远程设备名称/类型

   - 使用指导：点击后，可以获取当前配对设备的名称和设备类型，并在对应功能的返回信息处显示。

   - 限制条件：

     - 必须要与设备配对成功，才能显示配对设备的名称与类型。
     - @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 401 - Invalid parameter.
     * @throws { BusinessError } 801 - Capability not supported.
     * @throws { BusinessError } 2900001 - Service stopped.
     * @throws { BusinessError } 2900003 - Bluetooth switch is off.
     * @throws { BusinessError } 2900099 - Operation failed.

   - 验证方法：在设备设置中查看配对设备的名称和类型。

     

9. 获取配对设备列表

   - 使用指导：点击后，获取已与当前设备配对的蓝牙设备的列表。

   - 限制条件：

     - 必须要与设备配对成功，才能显示配对设备的MAC地址。
     - @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 801 - Capability not supported.
     * @throws { BusinessError } 2900001 - Service stopped.
     * @throws { BusinessError } 2900003 - Bluetooth switch is off.
     - @throws { BusinessError } 2900099 - Operation failed.

   - 验证方法：在"开始蓝牙发现"功能中查看配对设备的MAC地址。

     ​                                                            

10. 依据ProfileId获取指定profile的连接状态

    - 使用指导：

      - 点击"依据ProfileId获取指定profile的连接状态"，可获取指定profile的连接状态。

      - 点击"获取配置文件连接状态2"，可获取设备的配置文件状态。

      - 调用的方法不同，状态结果选项与"获取当前连接状态"的状态选项一样，同为四个。

       - 限制条件：

         - 需要进行连接配对。
         - @throws { BusinessError } 201 - Permission denied.
         - @throws { BusinessError } 401 - Invalid parameter.
         - @throws { BusinessError } 801 - Capability not supported.
         - @throws { BusinessError } 2900001 - Service stopped.
         - @throws { BusinessError } 2900003 - Bluetooth switch is off.
         - @throws { BusinessError } 2900004 - Profile is not supported.
         - @throws { BusinessError } 2900099 - Operation failed.

       - 验证方法：查看连接profile的状态。

         

11. 注册蓝牙设备发现 / 注册状态变化  / 注册pin码变化 / 注册绑定变化

    - 使用指导：都为回调函数，用来监听相关类型事件的变化，并弹窗显示信息。

      > 注册蓝牙设备发现：
      >
      > - 订阅蓝牙状态更改时报告的事件。
      >
      > - type为要侦听的蓝牙发现事件的类型。
      >
      >
      > - callback回调用于侦听Bluetooth状态事件。
      >
      > 注册状态变化：
      >
      > - 订阅蓝牙状态更改时报告的事件。
      >
      > - type为要侦听的Bluetooth状态更改事件的类型。
      >
      >
      > - callback回调用于侦听Bluetooth状态事件。
      >
      > 注册pin码变化：
      >
      > - 订阅来自远程蓝牙设备的配对请求事件。
      > - type要侦听的配对请求事件的类型。
      > - callback回调用于侦听配对请求事件。
      >
      > 注册绑定变化：
      >
      > - 订阅绑定远程蓝牙设备时报告的事件。
      > - type为要侦听的绑定状态事件的类型。
      > - callback回调用于侦听绑定状态事件。

    - 限制条件：

      - 需要在相关类型事件发生改变前，开启监听。
      - @throws { BusinessError } 201 - Permission denied.
      - @throws { BusinessError } 401 - Invalid parameter.
      - @throws { BusinessError } 801 - Capability not supported.
      - @throws { BusinessError } 2900099 - Operation failed.

    - 验证方法：在事件变化后，查看是否有弹窗信息显示。