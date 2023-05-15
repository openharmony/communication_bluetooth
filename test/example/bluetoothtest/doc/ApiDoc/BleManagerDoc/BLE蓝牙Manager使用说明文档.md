## BLE蓝牙使用说明文档

​		本文档主要介绍了蓝牙专项测试程序的BLE蓝牙部分(ohos.bluetoothManager.d.ts)的功能使用说明。

#### 从主界面跳转到BLE蓝牙部分

#### setting界面

点击"switch"按钮 ，设置本设备的BLE参数与配置

>"BLE扫描过滤参数"，"BLE扫描配置" 默认都为true，是勾选的；值也是默认填好的。**如果不需要更改，无需打开BleFilter界面**。
>
>外设MAC地址填了默认值，可根据自己的连接需求更改。
>
>BLE扫描过滤参数包括：
>
>- 本机MAC地址。
>- 本机name。
>- 本机的serviceUUid。
>
>BLE扫描配置包括：
>
>- interval值 （默认值：0）
>
>- dutyMode：(默认值：LOW_POWER )
>
> > - LOW_POWER
> > - BALANCED
> > - LOW_LATENCY
>
>- matchMode:(默认值: MATCH_MODE_AGGRESSIVE)
>
> >- MATCH_MODE_AGGRESSIVE
> >- MATCH_MODE_STICKY



#### BLE蓝牙的主要接口(ohos.bluetoothManager.d.ts)

|         method名称          |        API名称         |                           所需参数                           |    返回值     | 备注 |
| :-------------------------: | :--------------------: | :----------------------------------------------------------: | :-----------: | :--: |
|         开始BLE扫描         |      startBLEScan      |     (filters: Array<ScanFilter>, options?: ScanOptions)      |     void      |      |
|         停止BLE扫描         |      stopBLEScan       |                              ()                              |     void      |      |
|   订阅BLE设备发现上报事件   |     on. BLEDevice      | (type: “BLEDeviceFind”, callback: Callback<Array<ScanResult>>) |     void      |      |
| 取消订阅BLE设备发现上报事件 |   off.BLEDeviceFind    | (type: “BLEDeviceFind”, callback?: Callback<Array<ScanResult>>) |     void      |      |
|      获取连接的BLE设备      | getConnectedBLEDevices |                              ()                              | Array<string> |      |



注：此处的"创建Gatt服务器"，"创建Gatt客户端"功能只是写了个壳子，其主要功能在Gatt界面中。



>| 参数名  |         类型          | 必填 |                             说明                             |
>| :-----: | :-------------------: | :--- | :----------------------------------------------------------: |
>| filters | Array<**ScanFilter**> | 是   | 表示扫描结果过滤策略集合，如果不使用过滤的方式，该参数设置为null。 |
>| options |    **ScanOptions**    | 否   |                表示扫描的参数配置，可选参数。                |
>
>
>
>#### ScanFilter   扫描过滤参数
>
>| 名称                        | 类型        | 可读 | 可写 | 说明                                                         |
>| :-------------------------- | :---------- | :--- | :--- | :----------------------------------------------------------- |
>| deviceId                    | string      | 是   | 是   | 表示过滤的BLE设备地址，例如：“XX:XX:XX:XX:XX:XX”。           |
>| name                        | string      | 是   | 是   | 表示过滤的BLE设备名。                                        |
>| serviceUuid                 | string      | 是   | 是   | 表示过滤包含该UUID服务的设备，例如：00001888-0000-1000-8000-00805f9b34fb。 |
>| serviceUuidMask             | string      | 是   | 是   | 表示过滤包含该UUID服务掩码的设备，例如：FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF。 |
>| serviceSolicitationUuid     | string      | 是   | 是   | 表示过滤包含该UUID服务请求的设备，例如：00001888-0000-1000-8000-00805F9B34FB。 |
>| serviceSolicitationUuidMask | string      | 是   | 是   | 表示过滤包含该UUID服务请求掩码的设备，例如：FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF。 |
>| serviceData                 | ArrayBuffer | 是   | 是   | 表示过滤包含该服务相关数据的设备，例如：[0x90,0x00,0xF1,0xF2]。 |
>| serviceDataMask             | ArrayBuffer | 是   | 是   | 表示过滤包含该服务相关数据掩码的设备，例如：[0xFF,0xFF,0xFF,0xFF]。 |
>| manufactureId               | number      | 是   | 是   | 表示过滤包含该制造商ID的设备，例如：0x0006。                 |
>| manufactureData             | ArrayBuffer | 是   | 是   | 表示过滤包含该制造商相关数据的设备，例如：[0x1F,0x2F,0x3F]。 |
>| manufactureDataMask         | ArrayBuffer | 是   | 是   | 表示过滤包含该制造商相关数据掩码的设备，例如：[0xFF,0xFF,0xFF]。 |
>
>
>
>#### ScanOptions   扫描的配置参数
>
>| 名称      | 类型          | 可读 | 可写 | 说明                                                    |
>| :-------- | :------------ | :--- | :--- | :------------------------------------------------------ |
>| interval  | number        | 是   | 是   | 表示扫描结果上报延迟时间，默认值为0。                   |
>| dutyMode  | **ScanDuty**  | 是   | 是   | 表示扫描模式，默认值为SCAN_MODE_LOW_POWER。             |
>| matchMode | **MatchMode** | 是   | 是   | 表示硬件的过滤匹配模式，默认值为MATCH_MODE_AGGRESSIVE。 |
>
>
>
>#### ScanDuty   枚举，扫描模式
>
>|         名称          |  值  |           说明           |
>| :-------------------: | :--: | :----------------------: |
>|  SCAN_MODE_LOW_POWER  |  0   | 表示低功耗模式，默认值。 |
>|  SCAN_MODE_BALANCED   |  1   |      表示均衡模式。      |
>| SCAN_MODE_LOW_LATENCY |  2   |     表示低延迟模式。     |
>
>
>
>#### MatchMode   枚举，硬件过滤匹配模式
>
>|         名称          |  值  |                             说明                             |
>| :-------------------: | :--: | :----------------------------------------------------------: |
>| MATCH_MODE_AGGRESSIVE |  1   | 表示硬件上报扫描结果门限较低，比如扫描到的功率较低或者一段时间扫描到的次数较少也触发上报，默认值。 |
>|   MATCH_MODE_STICKY   |  2   | 表示硬件上报扫描结果门限较高，更高的功率门限以及扫描到多次才会上报。 |
>
>
>
>#### ScanResult    扫描结果上报数据
>
>|   名称   |    类型     | 可读 | 可写 |                       说明                        |
>| :------: | :---------: | :--: | :--: | :-----------------------------------------------: |
>| deviceId |   string    |  是  |  否  | 表示扫描到的设备地址，例如：“XX:XX:XX:XX:XX:XX”。 |
>|   rssi   |   number    |  是  |  否  |            表示扫描到的设备的rssi值。             |
>|   data   | ArrayBuffer |  是  |  否  |          表示扫描到的设备发送的广播包。           |



#### 功能

**"蓝牙打开"是其他功能测试的前提**

1. 开始/停止BLE扫描

   - 使用指导：

     > 开始BLE扫描：
     >
     > - 开始扫描具有筛选器的指定BLE设备。
     > - filters表示用于筛选出指定设备的筛选器列表。
     > - options指示扫描的参数，如果用户未指定值，将使用默认值。
     >
     > 停止BLE扫描。

   - 限制条件：

     - 只能扫描到指定BLE设备。
     - 如果不想使用筛选器，请将此参数filters设置为｛@code null｝。
     - ｛interval｝设置为0，
     - ｛扫描选项#dutyMode｝设置成｛SCAN_MODE_LOW_POWER｝并且{ScanOptions#matchMode}设置为{ MATCH_MODE_AGGRESSIVE}
     - @throws { BusinessError } 201 - Permission denied.
     - @throws { BusinessError } 401 - Invalid parameter.
     - @throws { BusinessError } 801 - Capability not supported.
     - @throws { BusinessError } 2900001 - Service stopped.
     - @throws { BusinessError } 2900003 - Bluetooth switch is off.
     - @throws { BusinessError } 2900099 - Operation failed.

   - 验证方法：根据弹窗消息查看已扫描到的BLE

     

2. BLE设备查找变化

   - 使用指导：为回调函数，用来监听相关类型事件的变化，并弹窗显示信息。

     - 订阅BLE扫描结果。
     - type为要侦听的扫描结果事件的类型。
     - callback回调用于侦听扫描结果事件。
     - @throws { BusinessError } 201 - Permission denied.
     - @throws { BusinessError } 401 - Invalid parameter.
     - @throws { BusinessError } 801 - Capability not supported.
     - @throws { BusinessError } 2900099 - Operation failed.

   - 限制条件：需要在相关类型事件发生改变前，开启监听。

   - 验证方法：在事件变化后，查看是否有弹窗信息显示。

     

3. 获取连接的BLE设备

   - 使用指导：点击后，可获取到处于连接状态的设备列表。
   - 限制条件：
     - 开始BLE扫描，并发现了可连接的设备，连接后，才可以获取此设备的相关信息。
     - @throws { BusinessError } 201 - Permission denied.
     - @throws { BusinessError } 801 - Capability not supported.
     - @throws { BusinessError } 2900001 - Service stopped.
     - @throws { BusinessError } 2900003 - Bluetooth switch is off.
     - @throws { BusinessError } 2900099 - Operation failed.
   - 验证方法：在设备设置中，查看已经连接的设备。