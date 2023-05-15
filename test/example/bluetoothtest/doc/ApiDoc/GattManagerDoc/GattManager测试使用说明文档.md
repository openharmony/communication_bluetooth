## GattManager测试使用说明文档

​		本文档主要介绍蓝牙专项测试程序的Gatt部分(ohos.bluetoothManager.d.ts)的功能使用说明。

### 从主界面跳转到Gatt蓝牙部分

Gatt测试分为Client测试和Server测试

### Gatt客户端

选择GattClient测试，进行Gatt客户端的测试

#### GattClientSetting界面

点击"switch"图标，可以在setting界面修改MAC地址, ServerUUID, characteristicUuid, descriptorUuid的值。已经设置了默认值。

>Gatt_ClientSetting中参数有：
>
>​	外围设备MAC
>
>​	服务UUID
>
>​	特征值UUID，特征值Value
>
>​	描述符UUID，描述符Value

#### GattClient的主要接口(ohos.bluetoothManager.d.ts)

|              method名称              |              API名称              |                           所需参数                           |           返回值            | 备注 |
| :----------------------------------: | :-------------------------------: | :----------------------------------------------------------: | :-------------------------: | :--: |
|            创建GattClient            |      createGattClientDevice       |                      (deviceId: string)                      |      GattClientDevice       |      |
|                 连接                 |              connect              |                              ()                              |            void             |      |
|               取消连接               |            disconnect             |                              ()                              |            void             |      |
|   订阅蓝牙低功耗设备的连接状态变化   |   on.BLEConnectionStatecChange    | (type: "BLEConnectionStateChange", callback: Callback<BLEConnectChangedState>) |            void             |      |
| 取消订阅蓝牙低功耗设备的连接状态变化 |   off.BLEConnectionStateChange    | (type: “BLEConnectionStateChange”, callback?: Callback<BLEConnectChangedState>) |            void             |      |
|                 关闭                 |               close               |                              ()                              |            void             |      |
|           获取服务callback           |       getServices(callback)       |        (callback: AsyncCallback<Array<GattService>>)         |            void             |      |
|           获取服务promise            |        getServices(pomise)        |                              ()                              | Promise<Array<GattService>> |      |
|          读取特征值callback          | readCharacteristicValue(callback) | (characteristic: BLECharacteristic, callback: AsyncCallback<BLECharacteristic>) |            void             |      |
|          读取特征值promise           |  readCharacteristicValue(pomise)  |             (characteristic: BLECharacteristic)              | Promise<BLECharacteristic>  |      |
|         读取描述符值callback         |  readDescripitorValue(callback)   | (descriptor: BLEDescriptor, callback: AsyncCallback<BLEDescriptor>) |            void             |      |
|         读取描述符值promise          |   readDescripitorValue(promise)   |                 (descriptor: BLEDescriptor)                  |   Promise<BLEDescriptor>    |      |
|              写入特征值              |     writeCharacteristicValue      |             (characteristic: BLECharacteristic)              |            void             |      |
|             写入描述符值             |       writeDescripitorValue       |                 (descriptor: BLEDescriptor)                  |            void             |      |
|            BLE特征值变化             |    on.BLECharacteristicChange     | (type: "BLECharacteristicChange", callback: Callback<BLECharacteristic>) |            void             |      |
|            设定BLEMtu尺寸            |           setBLEMtuSize           |                        (mtu: number)                         |            void             |      |
|           设置通知特性更改           |   setNotifyCharateristicChanged   |     (characteristic: BLECharacteristic, enable: boolean)     |            void             |      |
|         获取设备名称callback         |      getDeviceName(callback)      |              (callback: AsyncCallback<string>)               |            void             |      |
|         获取设备名称promise          |      getDeviceName(promise)       |                              ()                              |       Promise<string>       |      |
|          获取Rssi值callback          |      getRssiValue(callback)       |              (callback: AsyncCallback<number>)               |            void             |      |
|          获取Rssi值promise           |       getRssiValue(promise)       |                              ()                              |       Promise<number>       |      |

>**接口所需的参数配置：**
>
>#### BLEConnectChangedState    描述Gatt profile连接状态 
>
>| 名称     | 类型                       | 可读 | 可写 | 说明                                          |
>| :------- | :------------------------- | :--- | :--- | :-------------------------------------------- |
>| deviceId | string                     | 是   | 否   | 表示远端设备地址，例如：“XX:XX:XX:XX:XX:XX”。 |
>| state    | **ProfileConnectionState** | 是   | 是   | 表示BLE连接状态的枚举。                       |
>
>
>
>#### ProfileConnectionState     枚举，蓝牙设备的profile连接状态
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
>#### GattService    描述service的接口参数定义
>
>| 名称            | 类型                         | 可读 | 可写 | 说明                                                         |
>| :-------------- | :--------------------------- | :--- | :--- | :----------------------------------------------------------- |
>| serviceUuid     | string                       | 是   | 是   | 特定服务（service）的UUID，例如：00001888-0000-1000-8000-00805f9b34fb。 |
>| isPrimary       | boolean                      | 是   | 是   | 如果是主服务设置为true，否则设置为false。                    |
>| characteristics | Array<**BLECharacteristic**> | 是   | 是   | 当前服务包含的特征列表。                                     |
>| includeServices | Array<**GattService**>       | 是   | 是   | 当前服务依赖的其它服务。                                     |
>
>
>
>#### BLECharacteristic    描述characteristic的接口参数定义 
>
>| 名称                | 类型                     | 可读 | 可写 | 说明                                                         |
>| :------------------ | :----------------------- | :--- | :--- | :----------------------------------------------------------- |
>| serviceUuid         | string                   | 是   | 是   | 特定服务（service）的UUID，例如：00001888-0000-1000-8000-00805f9b34fb。 |
>| characteristicUuid  | string                   | 是   | 是   | 特定特征（characteristic）的UUID，例如：00002a11-0000-1000-8000-00805f9b34fb。 |
>| characteristicValue | ArrayBuffer              | 是   | 是   | 特征对应的二进制值。                                         |
>| descriptors         | Array<**BLEDescriptor**> | 是   | 是   | 特定特征的描述符列表。                                       |
>
>
>
>#### BLEDescriptor   描述descriptor的接口参数定义
>
>| 名称               | 类型        | 可读 | 可写 | 说明                                                         |
>| :----------------- | :---------- | :--- | :--- | :----------------------------------------------------------- |
>| serviceUuid        | string      | 是   | 是   | 特定服务（service）的UUID，例如：00001888-0000-1000-8000-00805f9b34fb。 |
>| characteristicUuid | string      | 是   | 是   | 特定特征（characteristic）的UUID，例如：00002a11-0000-1000-8000-00805f9b34fb。 |
>| descriptorUuid     | string      | 是   | 是   | 描述符（descriptor）的UUID，例如：00002902-0000-1000-8000-00805f9b34fb。 |
>| descriptorValue    | ArrayBuffer | 是   | 是   | 描述符对应的二进制值。                                       |



#### Gatt_Client测试功能

**"蓝牙打开"是其他功能测试的前提**

1. 创造GattClientDevice

   - 使用指导：点击"创造GattClient"后，创建一个JavaScript网关客户端设备实例。
   - 限制条件：
     - 蓝牙状态要打开，且GattClient不存在实例，并且会判断是否输入了MAC地址和MAC地址长度正确与否。
     - @throws { BusinessError } 401 - Invalid parameter.
     * @throws { BusinessError } 801 - Capability not supported.
   - 验证方法：尝试可以去连接GattServer端的设备。

2. 连接/取消连接

   - 使用指导：

     > 连接：连接到BLE外围设备。如果连接进程启动，返回信息为true；否则返回false。
     >
     > 订阅“BLEConnectionStateChange”事件（即"ble连接状态"）可以返回连接状态。
     >
     > 取消连接：断开或停止与BLE外围设备的持续连接。如果断开连接过程开始，返回信息为true；否则返回false。

   - 限制条件：

     - 只有蓝牙打开且连接成功后，"连接"的返回信息才会显示"成功"，才能成功取消连接设备。
     - @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 801 - Capability not supported.
     * @throws { BusinessError } 2900001 - Service stopped.
     * @throws { BusinessError } 2900003 - Bluetooth switch is off.
     - @throws { BusinessError } 2900099 - Operation failed.

   - 验证方法：在设备设置上查看连接状态。如果成功连接正在advertise on的服务端，服务端会有连接响应。

3. ble连接状态

   - 使用指导：为回调函数，用来监听相关类型事件的变化，并弹窗显示信息。

     > - 订阅客户端连接状态更改事件。
     > - type为要侦听的连接状态更改事件的类型。
     > - callback回调用于侦听连接状态更改事件。

   - 限制条件：

     - 需要在相关类型事件发生改变前，开启监听。且需要蓝牙打开，gatt客户端实例存在。
     - @throws { BusinessError } 201 - Permission denied.
     - @throws { BusinessError } 801 - Capability not supported.

   - 验证方法：在事件变化后，查看是否有弹窗信息显示，或直接查看其BLE连接状态。

4. 关闭

   - 使用指导：禁用BLE外围设备。
   - 限制条件：
     - 创立了Gatt客户端实例后，此方法注销设备并清除已注册的callback和handle。
     - @throws { BusinessError } 201 - Permission denied.
     - @throws { BusinessError } 801 - Capability not supported.
     - @throws { BusinessError } 2900001 - Service stopped.
     - @throws { BusinessError } 2900099 - Operation failed.
   - 验证方法：客户端已经关闭，无法连接server端。

5. 获取服务callback/promise

   - 使用指导：点击后，开始发现服务。返回的信息为BLE外围设备的服务GattService列表。
   - 限制条件：
     - Gatt客户端实例存在，连接设备的code不为0，ServerUUid正确。
     - @throws { BusinessError } 201 - Permission denied.
     - @throws { BusinessError } 401 - Invalid parameter.
     - @throws { BusinessError } 801 - Capability not supported.
     - @throws { BusinessError } 2900001 - Service stopped.
     - @throws { BusinessError } 2900099 - Operation failed.
   - 验证方法：查看返回信息与连接的服务是否相同。

6. 读取特征值callback/promise

   - 使用指导：点击后，读取BLE外围设备的特性。characteristic表示要读取的特征。
   - 限制条件：
     - Gatt客户端实例存在，连接设备的errcode为0，特征值UUid正确。
     - @throws { BusinessError } 201 - Permission denied.
     - @throws { BusinessError } 401 - Invalid parameter.
     - @throws { BusinessError } 801 - Capability not supported.
     - @throws { BusinessError } 2900001 - Service stopped.
     - @throws { BusinessError } 2901000 - Read forbidden.
     - @throws { BusinessError } 2900099 - Operation failed.
   - 验证方法：查看返回信息与连接的特征值是否相同。

7. 读取描述符值callback/promise

   - 使用指导：点击后，读取BLE外围设备的描述符。descriptor表示要读取的描述符。
   - 限制条件：
     - Gatt客户端实例存在，连接设备的errcode为0，描述符UUid正确。
     - @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 401 - Invalid parameter.
     * @throws { BusinessError } 801 - Capability not supported.
     * @throws { BusinessError } 2900001 - Service stopped.
     * @throws { BusinessError } 2901000 - Read forbidden.
     * @throws { BusinessError } 2900099 - Operation failed.
   - 验证方法：查看返回信息与连接的描述符值是否相同。

8. 写入特征值

   - 使用指导：写入BLE外围设备的特性。characteristic表示要写入的特征。如果成功写入特征，返回true；否则返回false。
   - 限制条件：
     - Gatt客户端实例存在，有写入的特征值。
     - @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 401 - Invalid parameter.
     * @throws { BusinessError } 801 - Capability not supported.
     * @throws { BusinessError } 2900001 - Service stopped.
     * @throws { BusinessError } 2901001 - Write forbidden.
     * @throws { BusinessError } 2900099 - Operation failed.
   - 验证方法：用验证程序查看写入的特征值。

9. 写入描述符值

   - 使用指导：写入BLE外围设备的描述符。descriptor指示要写入的描述符。如果描述符写入成功，返回true；否则返回false。
   - 限制条件：
     - Gatt客户端实例存在，有写入的描述符值。
     - @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 401 - Invalid parameter.
     * @throws { BusinessError } 801 - Capability not supported.
     * @throws { BusinessError } 2900001 - Service stopped.
     * @throws { BusinessError } 2901001 - Write forbidden.
     - @throws { BusinessError } 2900099 - Operation failed.
   - 验证方法：用验证程序查看写入的描述符值。

10. BLE特征值变化

    - 使用指导：为回调函数，用来监听相关类型事件的变化，并弹窗显示信息。

      >订阅特征值更改事件。
      >
      >type为要侦听的特征值更改事件的类型。
      >
      >callback回调用于侦听特征值更改事件。

    - 限制条件：

      - 需要在相关类型事件发生改变前，开启监听。
      - @throws { BusinessError } 201 - Permission denied.
      * @throws { BusinessError } 801 - Capability not supported.

    - 验证方法：在事件变化后，查看是否有弹窗信息显示。

11. 设定BLEMtu尺寸

    - 使用指导：设置BLE外围设备的mtu大小。mtu最大传输单位。如果设置mtu成功，返回信息为true；否则返回false。
    - 限制条件：
      - 蓝牙需要打开，Gatt客户端实例存在。
      - @throws { BusinessError } 201 - Permission denied.
      * @throws { BusinessError } 401 - Invalid parameter.
      * @throws { BusinessError } 801 - Capability not supported.
      * @throws { BusinessError } 2900001 - Service stopped.
      - @throws { BusinessError } 2900099 - Operation failed.
    - 验证方法：查看显示结果。

12. 设置通知特性更改

    - 使用指导：启用或禁用值更改时的特征通知。enable指定是否启用特征通知。值为true时表示通知已启用，值为false时表示通知已禁用。如果特性通知被启用，返回true；否则返回false。
    - 限制条件：
      - 蓝牙需要打开，Gatt客户端实例存在，且需要传入UUid值。
      - @throws { BusinessError } 201 - Permission denied.
      * @throws { BusinessError } 401 - Invalid parameter.
      * @throws { BusinessError } 801 - Capability not supported.
      * @throws { BusinessError } 2900001 - Service stopped.
      - @throws { BusinessError } 2900099 - Operation failed.
    - 验证方法：查看特征值的变化。

13. 获取设备名称_callback/promise

    - 使用指导：获取BLE外围设备的名称。如果获得，返回名称的字符串表示形式；如果无法获取名称或名称不存在，则返回null。
    - 限制条件：Gatt客户端实例存在，传入的Name值要正确。
    - 验证方法：查看显示的结果。

14. 获取Rssi值_callback/promise

    - 使用指导：获取此BLE外围设备的RSSI值。return返回RSSI值。
    - 限制条件：
      - Gatt客户端实例存在，传入的Rssi值要正确。
      - @throws { BusinessError } 201 - Permission denied.
      - @throws { BusinessError } 401 - Invalid parameter.
      - @throws { BusinessError } 801 - Capability not supported.
      - @throws { BusinessError } 2900099 - Operation failed.
    - 验证方法：查看显示的结果。

---

### Gatt服务器

#### GattServerSetting界面

点击"switch"图标，同样可以在setting界面修改MAC地址,ServerUUID,characteristicUuid,descriptorUuid的值。并且可以设置广播配置。默认值已经设置好，默认"设置"和"connectable"为已勾选的true，其他为false，如果测试时需要，可以手动设置"interval","txPower","Response"

#### GattServer中的主要接口(ohos.bluetoothManager.d.ts)

|   method名称   |        API名称         |                           所需参数                           |   返回值   | 备注 |
| :------------: | :--------------------: | :----------------------------------------------------------: | :--------: | :--: |
| 创建Gatt服务器 |    createGattServer    |                              ()                              | GattServer |      |
|    添加服务    |       addService       |                    (service: GattService)                    |    void    |      |
|    删除服务    |     removeService      |                    (serviceUuid: string)                     |    void    |      |
|    开始广播    |    startAdvertising    | (setting: AdvertiseSetting, advData: AdvertiseData, advResponse?: AdvertiseData) |    void    |      |
|    停止广播    |    stopAdvertising     |                              ()                              |    void    |      |
|      关闭      |         close          |                              ()                              |    void    |      |
|    连接状态    | on.connectStateChange  | (type: "connectStateChange", callback: Callback<BLEConnectChangedState>) |    void    |      |
|   读取特征值   | on.characteristicRead  | (type: "characteristicRead", callback: Callback<CharacteristicReadReq>) |    void    |      |
|   写入特征值   | on.characteristicWrite | (type: "characteristicWrite", callback: Callback<CharacteristicWriteReq>) |    void    |      |
|   读取描述符   |   on.descriptorRead    | (type: "descriptorRead", callback: Callback<DescriptorReadReq>) |    void    |      |
|   写入描述符   |   on.descriptorWrite   | (type: "descriptorWrite", callback: Callback<DescriptorWriteReq>) |    void    |      |

>**接口所需的参数配置：**
>
>| 参数名      | 类型                 | 必填 | 说明                      |
>| :---------- | :------------------- | :--- | :------------------------ |
>| setting     | **AdvertiseSetting** | 是   | BLE广播的相关参数。       |
>| advData     | **AdvertiseData**    | 是   | BLE广播包内容。           |
>| advResponse | **AdvertiseData**    | 否   | BLE回复扫描请求回复响应。 |
>
>
>
>#### AdvertiseSetting   描述蓝牙低功耗设备发送广播的参数
>
>| 名称        | 类型    | 可读 | 可写 | 说明                                                         |
>| :---------- | :------ | :--- | :--- | :----------------------------------------------------------- |
>| interval    | number  | 是   | 是   | 表示广播间隔，最小值设置32个slot表示20ms，最大值设置16384个slot，默认值设置为1600个slot表示1s。 |
>| txPower     | number  | 是   | 是   | 表示发送功率，最小值设置-127，最大值设置1，默认值设置-7，单位dbm。 |
>| connectable | boolean | 是   | 是   | 表示是否是可连接广播，默认值设置为true。                     |
>
>
>
>#### AdvertiseData  描述BLE广播数据包的内容
>
>| 名称            | 类型                       | 可读 | 可写 | 说明                               |
>| :-------------- | :------------------------- | :--- | :--- | :--------------------------------- |
>| serviceUuids    | Array<string>              | 是   | 是   | 表示要广播的服务 UUID 列表。       |
>| manufactureData | Array**<ManufactureData>** | 是   | 是   | 表示要广播的广播的制造商信息列表。 |
>| serviceData     | Array**<ServiceData>**     | 是   | 是   | 表示要广播的服务数据列表。         |
>
>
>
>#### ManufactureData  描述BLE广播数据包的内容
>
>| 名称             | 类型        | 可读 | 可写 | 说明                            |
>| :--------------- | :---------- | :--- | :--- | :------------------------------ |
>| manufactureId    | number      | 是   | 是   | 表示制造商的ID，由蓝牙SIG分配。 |
>| manufactureValue | ArrayBuffer | 是   | 是   | 表示制造商发送的制造商数据。    |
>
>
>
>#### ServiceData  描述广播包中服务数据内容
>
>| 名称         | 类型        | 可读 | 可写 | 说明             |
>| :----------- | :---------- | :--- | :--- | :--------------- |
>| serviceUuid  | string      | 是   | 是   | 表示服务的UUID。 |
>| serviceValue | ArrayBuffer | 是   | 是   | 表示服务数据。   |

#### Gatt_Server测试功能

1. 创建Gatt服务器

   - 使用指导：创建JavaScript网关服务器实例。return返回JavaScript网关服务器实例code网关服务器。

   - 限制条件：蓝牙状态要打开，且GattServer不存在实例。

   - 验证方法：可以尝试用Client端去搜索本设备的GattServer端。

     

2. 添加/删除服务

   - 使用指导：

     >添加服务：
     >
     >- 添加要承载的指定服务，添加的服务及其特性由本地设备提供。
     >
     >- service表示要添加的服务。
     >
     >- 如果添加了服务，返回 true；否则返回 false。
     >
     >删除服务：
     >
     >- 从该设备提供的GATT服务列表中删除指定的服务。
     >
     >- serviceUuid表示要删除的服务的UUID。
     >
     >- 如果服务被删除，返回 true；否则返回 false。

   - 限制条件：

     - Gatt服务端实例要存在，并且要传入UUid和value的值。
     - @throws { BusinessError } 201 - Permission denied.
     - @throws { BusinessError } 401 - Invalid parameter.
     - @throws { BusinessError } 801 - Capability not supported.
     - @throws { BusinessError } 2900001 - Service stopped.
     - @throws { BusinessError } 2900003 - Bluetooth switch is off.
     - @throws { BusinessError } 2900099 - Operation failed.

   - 验证方法：使用验证程序查看服务是否添加/删除成功。

     

3. 开始/停止广播

   - 使用指导：

     >开始广播：
     >
     >- 启动BLE广播。
     >- setting指示BLE广播的设置。如果需要使用默认值，将此参数设置为null。
     >- advData表示广播数据。advResponse表示与广播数据关联的扫描响应。
     >
     >停止广播：停止BLE广播。

   - 限制条件：

     - 配置页面中勾选了"√"，并传入了正确的配置信息。
     - @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 401 - Invalid parameter.
     * @throws { BusinessError } 801 - Capability not supported.
     * @throws { BusinessError } 2900001 - Service stopped.
     * @throws { BusinessError } 2900003 - Bluetooth switch is off.
     - @throws { BusinessError } 2900099 - Operation failed.

   - 验证方法：查看显示信息，使用client端设备去尝试搜索广播。

     

4. 关闭

   - 使用指导：关闭此{GattServer}对象并注销其回调。

   - 限制条件：

     - Server端存在，才可以关闭。
     - @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 801 - Capability not supported.
     * @throws { BusinessError } 2900001 - Service stopped.
     * @throws { BusinessError } 2900003 - Bluetooth switch is off.
     * @throws { BusinessError } 2900099 - Operation failed.

   - 验证方法：Server端关闭，无法搜索到本设备的Server端。

     

5. 连接状态

   - 使用指导：为回调函数，用来监听相关类型事件的变化，并弹窗显示信息。

     > 订阅服务器连接状态更改事件。
     >
     > type为要侦听的连接状态更改事件的类型。
     >
     > callback回调用于侦听连接状态更改事件。

   - 限制条件：

     - 需要在相关类型事件发生改变前，开启监听。
     - @throws { BusinessError } 201 - Permission denied.
     - @throws { BusinessError } 401 - Invalid parameter.
     - @throws { BusinessError } 801 - Capability not supported.

   - 验证方法：在事件变化后，查看是否有弹窗信息显示。

     

6. 读取/写入特征值

   - 使用指导：为回调函数，用来监听相关类型事件的变化，并弹窗显示信息。

     >读取特征值：
     >
     >- 订阅特征读取事件。
     >- type为要侦听的特征读取事件的类型。
     >- callback回调用于侦听特征读取事件。
     >
     >写入特征值：
     >
     >- 订阅特征写入事件。
     >- type为要侦听的特征写入事件的类型。
     >- callback回调用于侦听特征写入事件。

   - 限制条件：

     - 需要在相关类型事件发生改变前，开启监听。
     - @throws { BusinessError } 201 - Permission denied.
     - @throws { BusinessError } 401 - Invalid parameter.
     - @throws { BusinessError } 801 - Capability not supported.

   - 验证方法：在事件变化后，查看是否有弹窗信息显示。

     

7. 读取/写入描述符

   - 使用指导：为回调函数，用来监听相关类型事件的变化，并弹窗显示信息。

     >读取描述符值：
     >
     >- 订阅描述符写入事件。
     >- type为要侦听的描述符写入事件的类型。
     >- callback为回调用于侦听描述符写入事件。
     >
     >写入描述符值：
     >
     >- 订阅描述符读取事件。
     >- type为要侦听的描述符读取事件的类型。
     >- callback回调用于侦听描述符读取事件。

   - 限制条件：

     - 需要在相关类型事件发生改变前，开启监听。
     - @throws { BusinessError } 201 - Permission denied.
     * @throws { BusinessError } 401 - Invalid parameter.
     * @throws { BusinessError } 801 - Capability not supported.

   - 验证方法：在事件变化后，查看是否有弹窗信息显示。