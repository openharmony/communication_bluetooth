/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#ifndef BLUETOOTH_STATE_MANAGER_H
#define BLUETOOTH_STATE_MANAGER_H

#include <list>
#include "log.h"
#include "bt_def.h"
#include "safe_map.h"  // SafeMap

namespace OHOS {
namespace bluetooth {
struct DeviceConnectState {
    // <profileId, BTConnectState::CONNECTED>
    SafeMap<std::string, BTConnectState> profileConnectStateMap;
};
class BluetoothStateManager {
public:

    BluetoothStateManager();
    ~BluetoothStateManager();

    static BluetoothStateManager *GetInstance();
    void SetRemoteDeviceHfpAgState(const std::string &address, int state);
    void EraseRemoteDeviceHfpAgState(const std::string &address);
    bool FindRemoteDeviceHfpAgStateByAddr(const std::string &address, int &state);
    template <typename K, typename V>
    std::list<std::string> FindRemoteDeviceByHfpConnState(const std::function<bool(const K, V&)> itorFunc)
    {
        std::list<std::string> addressList;
        hfpAgStatemachineState_.Iterate([&addressList, itorFunc](const std::string &address, int state) {
            if (itorFunc(address, state)) {
                addressList.emplace_back(address);
            }
        });
        return addressList;
    }

    void SetRemoteDevicePanState(const std::string &address, int state);
    void EraseRemoteDevicePanState(const std::string &address);
    bool FindRemoteDevicePanStateByAddr(const std::string &address, int &state);
    template <typename K, typename V>
    std::list<std::string> FindRemoteDeviceByPanConnState(const std::function<bool(const K, V&)> itorFunc)
    {
        std::list<std::string> addressList;
        panStatemachineState_.Iterate([&addressList, itorFunc](const std::string &address, int state) {
            if (itorFunc(address, state)) {
                addressList.emplace_back(address);
            }
        });
        return addressList;
    }

    void AddDeviceProfileConnectState(const std::string &profileId, const std::string &address, BTConnectState state);
    void RemoveDeviceConnectState(const std::string &address);
    void RemoveDeviceProfileConnectState(const std::string &profileId, const std::string &address);
    BTConnectState GetDeviceProfileConnectState(const std::string &profileId, const std::string &address);
    BTConnectState GetProfileConnectState(const std::string &profileId);

    /**
    * @brief 获取已连接的所有gatt client设备地址。
    *
    * @return 获取本机设备作为gatt server端时，已连接的所有对端gatt client设备地址列表。
    */
    std::vector<std::string> GetClientDeviceList();
    /**
    * @brief 获取已连接的所有gatt server设备地址。
    *
    * @return 获取本机设备作为gatt client端时，已连接的所有对端gatt server设备地址列表。
    */
    std::vector<std::string> GetServerDeviceList();
    void AddClientDeviceList(std::string addr);
    void AddServerDeviceList(std::string addr);
    void RemoveServerDeviceList(std::string addr);
    void RemoveClientDeviceList(std::string addr);

private:
    int ConvertHfpAgState(const std::string &state);
    SafeMap<std::string, int> hfpAgStatemachineState_ {};

    SafeMap<std::string, int> panStatemachineState_ {};
    // <address, DeviceConnectState>
    SafeMap<std::string, DeviceConnectState> deviceConnectStateMap_ {};

    std::unordered_map<std::string, int> clientDeviceCountMap_ {};
    std::unordered_map<std::string, int> serverDeviceCountMap_ {};
    std::vector<std::string> clientDeviceList_ {};
    std::vector<std::string> serverDeviceList_ {};
    std::mutex clientDeviceListMutex_;
    std::mutex serverDeviceListMutex_;
};
} // namespace bluetooth
} // namespace OHOS

#endif //BLUETOOTH_STATE_MANAGER_H