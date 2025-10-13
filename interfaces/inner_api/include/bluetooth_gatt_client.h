/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

/**
 * @addtogroup Bluetooth
 * @{
 *
 * @brief Defines a bluetooth system that provides basic bluetooth connection and profile functions,
 *        including A2DP, AVRCP, BLE, GATT, HFP, MAP, PBAP, and SPP, etc.
 *
 * @since 6
 *
 */

/**
 * @file bluetooth_gatt_client.h
 *
 * @brief Bluetooth gatt client interface.
 *
 * @since 6
 *
 */

#ifndef BLUETOOTH_GATT_CLIENT_H
#define BLUETOOTH_GATT_CLIENT_H

#include "bluetooth_def.h"
#include "bluetooth_gatt_service.h"
#include "bluetooth_remote_device.h"
#include <memory>

namespace OHOS {
namespace Bluetooth {

/**
 * @brief Class for GattClientCallback functions.
 *
 * @since 6
 *
 */
class GattClientCallback {
public:
    /**
     * @brief The function to OnConnectionStateChanged.
     *
     * @param connectionState callback of gattClientCallback.
     * @param ret ret of GattClientCallback.
     * @since 6
     *
     */
    virtual void OnConnectionStateChanged(int connectionState, int ret)
    {}

    /**
     * @brief The function to OnConnectionStateChanged.
     *
     * @param connectionState callback of gattClientCallback.
     * @param ret ret of GattClientCallback.
     * @since 20
     *
     */
    virtual void OnConnectionStateChangedWithReason(int connectionState, int ret, int disconnectReason)
    {}

    /**
     * @brief The function to OnServicesChanged.
     *
     * @since 22
     *
     */
    virtual void OnServicesChanged()
    {}

    /**
     * @brief The function to OnCharacteristicChanged.
     *
     * @param characteristic Characteristic object to changed.
     * @since 6
     *
     */
    virtual void OnCharacteristicChanged(const GattCharacteristic &characteristic)
    {}

    /**
     * @brief The function to OnCharacteristicReadResult.
     *
     * @param characteristic Characteristic object.
     * @param ret ret of GattClientCallback.
     * @since 6
     *
     */
    virtual void OnCharacteristicReadResult(const GattCharacteristic &characteristic, int ret)
    {}

    virtual void OnReadRemoteRssiValueResult(int rssi, int status)
    {}
    /**
     * @brief The function to OnCharacteristicWriteResult.
     *
     * @param characteristic Characteristic object. The value attribute is the initial value 0.
     * @param ret ret of GattClientCallback.
     * @since 6
     *
     */
    virtual void OnCharacteristicWriteResult(const GattCharacteristic &characteristic, int ret)
    {}

    /**
     * @brief The function to OnDescriptorReadResult.
     *
     * @param descriptor descriptor object.
     * @param ret ret of GattClientCallback.
     * @since 6
     *
     */
    virtual void OnDescriptorReadResult(const GattDescriptor &descriptor, int ret)
    {}

    /**
     * @brief The function to OnDescriptorWriteResult.
     *
     * @param descriptor descriptor object.
     * @param ret ret of GattClientCallback.
     * @since 6
     *
     */
    virtual void OnDescriptorWriteResult(const GattDescriptor &descriptor, int ret)
    {}

    /**
     * @brief The function to OnMtuUpdate.
     *
     * @param mtu mtu to update.
     * @param ret ret of GattClientCallback.
     * @since 6
     *
     */
    virtual void OnMtuUpdate(int mtu, int ret)
    {}

    /**
     * @brief The function to OnServicesDiscovered.
     *
     * @param status Status object.
     * @since 6
     *
     */
    virtual void OnServicesDiscovered(int status)
    {}

    /**
     * @brief The function to OnConnectionParameterChanged.
     *
     * @param interval interval object.
     * @param latency latency object.
     * @param timeout timeout object.
     * @param status status object.
     * @since 6
     *
     */
    virtual void OnConnectionParameterChanged(int interval, int latency, int timeout, int status)
    {}

    /**
     * @brief The function to OnSetNotifyCharacteristic.
     *
     * @param status status object.
     * @since 6
     *
     */
    virtual void OnSetNotifyCharacteristic(const GattCharacteristic &characteristic, int status)
    {}

    /**
     * @brief A destructor of GattClientCallback.
     *
     * @since 6
     *
     */
    virtual ~GattClientCallback()
    {}
};

/**
 * @brief Class for GattClient functions.
 *
 * @since 6
 *
 */
class BLUETOOTH_API GattClient : public std::enable_shared_from_this<GattClient> {
public:

    /**
     * @brief init gattClient.
     *
     * @return init   api init result.
     * @since 6
     *
     */
    bool Init();

    /**
     * @brief The function to Connect.
     *
     * @param callback callback of gattClientCallback.
     * @param isAutoConnect isAutoConnect of GattClient.
     * @param transport transport of GattClient.
     * @return int   api accept status.
     * @since 6
     *
     */
    int Connect(std::weak_ptr<GattClientCallback> callback, bool isAutoConnect, int transport);

    /**
     * @brief The function to request connection priority.
     *
     * @param connPriority connPriority of GattClient.
     * @return int   api accept status.
     * @since 6
     *
     */
    int RequestConnectionPriority(int connPriority);

    /**
     * @brief The function to request fastest connection.
     *
     * @return int   api accept status.
     */
    int RequestFastestConn();

    /**
     * @brief The function to disconnect.
     *
     * @return int   api accept status.
     * @since 6
     *
     */
    int Disconnect();

    /**
     * @brief The function to close.
     *
     * @return int.
     * @since 6
     *
     */
    int Close();

    /**
     * @brief The function to discover services.
     *
     * @return int    api accept status.
     * @since 6
     *
     */
    int DiscoverServices();

    /**
     * @brief The function to get service.
     *
     * @param uuid uuid of GattClient.
     * @return service.
     * @since 6
     *
     */
    std::optional<std::reference_wrapper<GattService>> GetService(const UUID &uuid);

    /**
     * @brief The function to get service.
     *
     * @return list of services.
     * @since 6
     *
     */
    std::vector<GattService> &GetService();

    /**
     * @brief The function to read characteristic.
     *
     * @param characteristic Characteristic object.
     * @return int read characteristic.
     * @since 6
     *
     */
    int ReadCharacteristic(GattCharacteristic &characteristic);

    /**
     * @brief The function to read descriptor.
     *
     * @param descriptor descriptor object.
     * @return int read descriptor.
     * @since 6
     *
     */
    int ReadDescriptor(GattDescriptor &descriptor);

    /**
     * @brief The function to RequestBleMtuSize.
     *
     * @param mtu mtu of GattClient.
     * @return int request ble mtu size.
     * @since 6
     *
     */
    int RequestBleMtuSize(int mtu);

    /**
     * @brief The function to SetNotifyCharacteristic.
     *
     * @param characteristic characteristic object.
     * @param enable enable of GattClient.
     * @return result of #GATT_STATUS.
     * @since 6
     *
     */
    int SetNotifyCharacteristic(GattCharacteristic &characteristic, bool enable);

    /**
     * @brief The function to SetNotifyCharacteristic version 2, this funtion will not trigger onDespritor callback.
     *
     * @param characteristic characteristic object.
     * @param enable enable of GattClient.
     * @return result of #GATT_STATUS.
     * @since 13
     *
     */
    int SetNotifyCharacteristicV2(GattCharacteristic &characteristic, bool enable);

    /**
     * @brief The function to SetIndicateCharacteristic.
     *
     * @param characteristic characteristic object.
     * @param enable enable of GattClient.
     * @return result of #GATT_STATUS.
     * @since 6
     *
     */
    int SetIndicateCharacteristic(GattCharacteristic &characteristic, bool enable);

    /**
     * @brief The function to write characteristic.
     *
     * @param characteristic characteristic object.
     * @return int write characteristic.
     * @since 6
     *
     */
    int WriteCharacteristic(GattCharacteristic &characteristic);

    /**
     * @brief The function to write characteristic.
     *
     * @param characteristic characteristic object.
     * @param value characteristic value.
     * @return int write characteristic.
     * @since 6
     *
     */
    int WriteCharacteristic(GattCharacteristic &characteristic, std::vector<uint8_t> value);

    /**
     * @brief The function to write characteristic.
     *
     * @param descriptor descriptor object.
     * @return int write descriptor.
     * @since 6
     *
     */
    int WriteDescriptor(GattDescriptor &descriptor);

    int ReadRemoteRssiValue();
    /**
     * @brief A constructor of GattClient.
     *
     * @param device Remote device object.
     * @since 6
     *
     */
    explicit GattClient(const BluetoothRemoteDevice &device);

    /**
     * @brief A destructor of GattClient.
     *
     * @since 6
     *
     */
    ~GattClient();

    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(GattClient);

private:
    BLUETOOTH_DECLARE_IMPL();

    int SetNotifyCharacteristicInner(GattCharacteristic &characteristic, bool enable,
        const std::vector<uint8_t> &descriptorValue);
};
} // namespace Bluetooth
} // namespace OHOS
#endif  // BLUETOOTH_GATT_CLIENT_H
