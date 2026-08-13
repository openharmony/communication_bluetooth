/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions andpbap
 * limitations under the License.
 */

#ifndef HID_DEVICE_SERVICE_H
#define HID_DEVICE_SERVICE_H

#include "app_mgr_interface.h"
#include "application_state_observer_stub.h"
#include "iremote_object.h"
#include "bluetooth_types.h"
#include "system_ability_definition.h"
#include "iservice_registry.h"

#include "base_def.h"
#include "hid_device_message.h"
#include "base_observer_list.h"
#include "btcommon/dynamic_library_loader.h"
#include "context.h"
#include "interface_profile_hid_device.h"
#include "hid_device_service_impl_interface.h"
#include "hid_device_defines.h"
#include "raw_address.h"
#include "bluetooth_hid_device_info.h"
#include "bt_hd.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace Bluetooth {


class HidDeviceService : public IProfileHidDevice, public utility::Context {
public:
    static constexpr char const *DEFAULT_LIB_NAME = "libbthiddevice.z.so";
    static constexpr char const *DEFAULT_LIB_CREATE_FUNC_NAME = "CreateHidDeviceServiceImplInterface";
    static constexpr char const *DEFAULT_LIB_DESTROY_FUNC_NAME = "DestroyHidDeviceServiceImplInterface";
    static constexpr uint32_t DEFAULT_UNLOAD_TIMER_MS = 600000;

    BaseObserverList<IHidDeviceObserver>& GetHidDeviceObservers();
    static HidDeviceService *GetService();
    explicit HidDeviceService();
    ~HidDeviceService() override;
    void LoadHidDeviceManagerInterfaceLib(void);
    utility::Context *GetContext() override;
    void Enable() override;
    void Disable() override;
    int Connect(const bluetooth::RawAddress &device) override;
    int Disconnect(const bluetooth::RawAddress &device) override;
    int DisconnectHidDevice() override;
    std::list<bluetooth::RawAddress> GetConnectDevices() override;
    int GetConnectState() override;
    int GetConnectionState(const bluetooth::RawAddress &device) override;
    int GetMaxConnectNum() override;
    bool IsRemoteHidHostSupported(const bluetooth::RawAddress &device);
    int RegisterHidDevice(BluetoothHidDeviceSdp sdp, BluetoothHidDeviceQos inQos,
       BluetoothHidDeviceQos outQos) override;

    int UnregisterHidDevice() override;
    int AutoUnregisterHidDevice() override;

    int32_t SendReport(int id, const std::vector<uint8_t> &data) override;
    int32_t ReplyReport(uint8_t type, int id, const std::vector<uint8_t> &data) override;
    int32_t ReportError(uint8_t type) override;
    int SetConnectStrategy(const bluetooth::RawAddress &device, int strategy) override;
    int GetConnectStrategy(const bluetooth::RawAddress &device) override;
    void SetIsForeground(bool isForeground);
    bool GetIsForeground();
    void SetIsConnected(bool isConnected);
    bool IsConnected();
    void SetAddress(const std::string &device);
    void SetHidDeviceAppInfoEmpty();
    bool IsAppRegistered();

    void PostEvent(const HidDeviceMessage &event);
    void ProcessEvent(const HidDeviceMessage &event);
    void StartUp();
    void ShutDown();
    void ProcessUnregisterHidDeviceEvent(const HidDeviceMessage &event);
    void ProcessConnectEvent(const HidDeviceMessage &event);
    void ProcessDisconnectEvent(const HidDeviceMessage &event);
    void ProcessSendReportEvent(const HidDeviceMessage &event);
    void ProcessReplyReportEvent(const HidDeviceMessage &event);
    void ProcessReportErrorEvent(const HidDeviceMessage &event);
    void HidProcessBtChrEvent(int state);

    void RegisterObserver(IHidDeviceObserver &hidDeviceObserver) override;
    void DeregisterObserver(IHidDeviceObserver &hidDeviceObserver) override;

    BT_DISALLOW_COPY_AND_ASSIGN(HidDeviceService);

private:
    HidDeviceAppInfo GetHidDeviceAppInfo();
    std::shared_ptr<HidDeviceServiceImplInterface> serviceManagerIntfImpl_ = nullptr;
    bluetooth::CxxDynamicLibraryLoader<HidDeviceServiceImplInterface> loader_;
    
    std::shared_ptr<IHidDeviceObserver> hidDeviceObserver_ = nullptr;
    BaseObserverList<IHidDeviceObserver> hidDeviceObservers_ {};
    bluetooth::RawAddress device_;
    HidDeviceAppInfo hidDeviceAppInfo_ {0, "", ""};
    bool isLoaded_ {false};
    const int HIDDEVICETYPE = 1;
    std::atomic<bool> isForeground_ {true};
    std::atomic<bool> isConnected_ {false};
    std::mutex hidDeviceAppInfoMutex_ {};
    std::mutex loadHidDeviceMutex_ {};
    std::mutex hidDeviceMutex_ {};
};

class HidDeviceObserver : public IHidDeviceObserver {
public:
    ~HidDeviceObserver() override = default;
    void OnAppStatusChanged(int state) override;
    void OnConnectionStateChanged(bluetooth::RawAddress device, int state) override;
    void OnGetReport(int type, int id, uint16_t buffer_size) override;
    void OnInterruptDataReceived(int report_id, std::vector<uint8_t> data) override;
    void OnSetProtocol(int protocol) override;
    void OnSetReport(int type, int id, std::vector<uint8_t> data) override;
    void OnVirtualCableUnplug() override;
};

}  // namespace Bluetooth
}  // namespace OHOS
#endif  // HID_DEVICE_SERVICE_H