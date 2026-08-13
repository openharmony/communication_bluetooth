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
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HID_DEVICE_SERVICE_IMPL_H
#define HID_DEVICE_SERVICE_IMPL_H



#include <cstring>
#include <list>
#include <map>
#include <vector>

#include "app_mgr_interface.h"
#include "application_state_observer_stub.h"
#include "iremote_object.h"
#include "bluetooth_types.h"
#include "system_ability_definition.h"
#include "iservice_registry.h"

#include "base_def.h"
#include "base_observer_list.h"
#include "context.h"
#include "btcommon/message.h"
#include "raw_address.h"
#include "hid_device_service_impl_interface.h"
#include "common_util.h"
#include "adapter_device_config.h"
#include "bluetooth.h"
#include "bt_hd.h"
#include "adapter_manager.h"

#include "if_system_ability_manager.h"
#include "system_ability.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace Bluetooth {

class HidAppStateObserver {
public:
    HidAppStateObserver() = default;
    void setBundleName(std::string bundleName);
    std::string getBundleName();
    bool SubscribeHidAppState();
    bool UnSubscribeHidAppState();
    sptr<AppExecFwk::IAppMgr> GetAppMgrProxy();

private:
    class HidAppStateAwareObserver : public AppExecFwk::ApplicationStateObserverStub {
    public:
        void OnForegroundApplicationChanged(const AppExecFwk::AppStateData &appStateData) override;
        void setBundleName(std::string bundleName);
        std::string getBundleName();
    private:
        inline bool ValidateAppStateData(const AppExecFwk::AppStateData &appStateData);
        std::string bundleName_{};
    };

    std::mutex mutex_{};
    sptr<HidAppStateAwareObserver> hidAppStateAwareObserver_;

    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(HidAppStateObserver);
};

class HidDeviceServiceImpl : public HidDeviceServiceImplInterface {
public:
    HidDeviceServiceImpl();
    ~HidDeviceServiceImpl() override;

    static HidDeviceServiceImpl *GetServiceImpl();

    void RegisterObserver(std::shared_ptr<IHidDeviceObserver> &hidDeviceObserver) override;
    void DeregisterObserver(std::shared_ptr<IHidDeviceObserver> &hidDeviceObserver) override;
    int Connect(const bluetooth::RawAddress &device) override;
    int Disconnect(const bluetooth::RawAddress &device) override;
    int RegisterHidDevice(BluetoothHidDeviceSdp sdp, BluetoothHidDeviceQos inQos,
            BluetoothHidDeviceQos outQos, std::string bundleName) override;
    int UnregisterHidDevice() override;
    int32_t SendReport(int id, const std::vector<uint8_t> &data) override;
    int32_t ReplyReport(uint8_t type, int id, const std::vector<uint8_t> &data) override;
    int32_t ReportError(uint8_t type) override;
    bool IsAppForeground(std::string bundleName);
    void SetAppState(bool isForeground);
    bool GetAppState();

    static void OnAppStatusChanged(BLUEDROID::RawAddress* bd_addr, bthd_application_state_t state);
    static void OnConnectionStateChanged(BLUEDROID::RawAddress* bd_addr, bthd_connection_state_t state);
    static void OnGetReport(uint8_t type, uint8_t id, uint16_t buffer_size);
    static void OnInterruptDataReceived(uint8_t report_id, uint16_t len, uint8_t* p_data);
    static void OnSetProtocol(uint8_t protocol);
    static void OnSetReport(uint8_t type, uint8_t id, uint16_t len, uint8_t* p_data);
    static void OnVirtualCableUnplug();

private:

    static int ConvertAppStatusChangedFromBluedroid(bthd_application_state_t status);
    static int ConvertConnectStateFromBluedroid(bthd_connection_state_t state);
    void OnAppStatusChangedInner(int state);
    void OnConnectionStateChangedInner(bluetooth::RawAddress rawAddr, int state);
    void OnGetReportInner(int type, int id, uint16_t buffer_size);
    void OnInterruptDataInner(int report_id, std::vector<uint8_t> data);
    void OnSetProtocolInner(int protocol);
    void OnSetReportInner(int type, int id, std::vector<uint8_t> data);
    void OnVirtualCableUnplugInner();
    void ProcessRegisterHIdDeviceEvent(BluetoothHidDeviceSdp sdp,
        BluetoothHidDeviceQos inQos, BluetoothHidDeviceQos outQos);

    std::vector<OHOS::AppExecFwk::AppStateData> GetForegroundApplications();
    std::shared_ptr<IHidDeviceObserver> hidDeviceObserver_ = nullptr;
    std::shared_ptr<HidAppStateObserver> hidAppStateObserver_ = nullptr;
    std::atomic<bool> IsAppForeground_ {true};
    bthd_interface_t* bluetoothHidDeviceInterface = nullptr;
};

}  // namespace Bluetooth
}  // namespace OHOS
#endif  // HID_DEVICE_SERVICE_IMPL_H
