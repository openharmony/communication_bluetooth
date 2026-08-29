/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_adapter_manager"
#endif

#include "adapter_manager.h"

#include <atomic>
#include <array>
#include <functional>
#include <future>
#include <if_system_ability_manager.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "adapter_config.h"
#include "adapter_device_config.h"
#include "adapter_device_info.h"
#include "adapter_state_machine.h"
#include "base_def.h"
#include "base_observer_list.h"
#include "bluetooth_airplane_manager.h"
#include "bluetooth_audio_manager.h"
#include "bluetooth_common_event_helper.h"
#include "bluetooth_connection_manager.h"
#include "bluetooth_device_battery_manager.h"
#include "common/bluetooth_hw_interface.h"
#ifdef BLUETOOTH_HIGHPOWERV1_ENABLE
#include "bluetooth_highpower_manager.h"
#endif
#include "bt_chr_dft_exception.h"
#include "bt_chr_transaction_manager.h"
#include "bt_chr_ue_manager.h"
#include "class_creator.h"
#include "classic_config.h"
#include "common_util.h"
#include "ipc_skeleton.h"
#include "connect_strategy_manager.h"
#include "dialog_switch.h"
#include "stack_adapter.h"
#include "log.h"
#include "parameter.h"
#include "permission_manager.h"
#include "power_manager.h"
#include "profile_config.h"
#include "profile_service_manager.h"
#include "remote_device_properties.h"
#include "sys_state_machine.h"
#include "thread_util.h"
#include "bluetooth_audio_framework_adapter.h"
#include "concurrent_task_client.h"
#include "hitrace_meter.h"
#include "control_intercept_plugin.h"
#ifdef EDM_SERVICE_ENABLE
#include "bluetooth_edm_manager.h"
#endif
#ifdef BLUETOOTH_KIA_ENABLE
#include "bluetooth_kia_refuse_policy_manager.h"
#endif
#ifdef FEATURE_MOUSE_FAST_CONNECTION
#include "pc_power_manager.h"
#endif
#include "bt_func_hook.h"
#include "parameters.h"
#include "bluetooth_hw_interface.h"
#include "preferences_manager.h"
#include "bluetooth_datashare_utils.h"
#include "bluetooth_resource_manager.h"
#include "satellite_manager.h"
#include "bluetooth_def.h"
#ifdef BLUETOOTH_FASTSCAN_ENABLE
#include "bluetooth_fastscan_manager_loader.h"
#endif
#include "cloud_device_manager.h"
#include "system_ability_processer.h"
#ifdef CONTEXTHUB_BLE_V3
#include "sensorhub_collaboration_service.h"
#endif
#include "bt_chr_switch.h"
#ifdef FEATURE_MOUSE_FAST_CONNECTION
#include "pc_power_manager.h"
#endif
#ifdef BLUETOOTH_POWER_ON_RECONNECT_FEATURE
#include "power_on_reconnect.h"
#endif
#include "param_wrapper.h"

using namespace OHOS::Bluetooth;

namespace OHOS {
namespace bluetooth {
// data define
constexpr int32_t INITIALIZE_DELAY_MS = 1000;
constexpr int32_t CHECK_DELAY_MS = 500;
constexpr int32_t AUTO_CONNECT_DELAY_MS = 300;
constexpr int32_t BT_ERR_BLUETOOTH_TURN_ON = -1;
constexpr int32_t BT_ERR_BLUETOOTH_TURNING = -2;
const int32_t BLUETOOTH_SWITCH_STATUS_SIZE = 16;
const int32_t SYSTEM_PARAMETER_ERROR_CODE = 0;
const int32_t PROPERTY_VALUE_MAX = 128;
const int BT_SCAN_DURATION_MS = 120000;
const char *INITIALIZE_CHECK_TASK = "InitializeCheckTask";
const char *g_bluetoothSwitchStateOff = "0";
const char *g_bluetoothSwitchStateOn = "1";
const char *BLUETOOTH_SWITCH_STATE_HALF = "2";
const char *g_bluetoothSwitchStatePropertyName = "persist.bluetooth.switch_enable";
const char *CONNECTIVITY_BTCHIPTYPE = "ohos.boot.odm.conn.schiptype";
const char *BETA_VERSION_PROPERTY = "const.logsystem.versiontype";
const char *BETA_VERSION = "beta";
const int32_t HISI_PCM_LOG_ENABLE = 1;
const char *COLLABORATION_SERVICE_PARAM = "persist.bluetooth.collaboration_service";

#ifdef BLUETOOTH_PLUGGABLE_SUPPORTED
static const std::string BLUETOOTH_PLUGGABLE_STATE_EXTRACT = "0";
static const std::string BLUETOOTH_PLUGGABLE_STATE_EMPLACE = "1";
static const std::string BLUETOOTH_PLUGGABLE_STATE = "persist.bluetooth.pluggable.state";
static const char* BLUETOOTH_EMPLACE_ENABLE_STATE = "bluetooth.emplace_enable.state";
#endif

std::shared_ptr<utility::Timer> AdapterManager::UnloadSaTimeout_ = nullptr;
std::vector<std::string> highPowerV2Chip = {"bisheng", "mp12"};
static void NotifyAdapterStateChange(
    BaseObserverList<IAdapterStateObserver> &observers, BTTransport transport, BTStateID state);

// T is BleAdapter or ClassicAdapter
template <typename T>
struct AdapterInfo {
    AdapterInfo(std::shared_ptr<T> instance, std::unique_ptr<AdapterStateMachine> stateMachine)
        : instance(instance), stateMachine(std::move(stateMachine))
    {}
    ~AdapterInfo()
    {}

    std::atomic<BTStateID> state = BTStateID::STATE_TURN_OFF;
    std::shared_ptr<T> instance = nullptr;
    std::unique_ptr<AdapterStateMachine> stateMachine = nullptr;
};

// static function
IAdapterManager *IAdapterManager::GetInstance()
{
    return AdapterManager::GetInstance();
}
AdapterManager *AdapterManager::GetInstance()
{
    static AdapterManager instance;
    return &instance;
}

// impl class
struct AdapterManager::impl {
    impl();
    ~impl();

    BtRecursiveMutex sysStateMutex_ = {};
    std::promise<void> stopPromise_ = {};
    std::promise<void> resetPromise_ = {};
    std::unique_ptr<AdapterInfo<ClassicAdapter>> classicAdapter_ = nullptr;
    std::unique_ptr<AdapterInfo<BleAdapter>> bleAdapter_ = nullptr;
    SysStateMachine sysStateMachine_ = {};
    std::string sysState_ = SYS_STATE_STOPPED;
    std::atomic_bool isFactoryReseting {false};
    std::atomic_bool isRestrictBluetooth {false};
    BaseObserverList<IAdapterStateObserver> adapterObservers_ = {};
    BaseObserverList<ISystemStateObserver> systemObservers_ = {};

    std::atomic_bool isAppCloseBt_ { false };
    std::shared_ptr<SatelliteManager> satelliteManager_ { nullptr };
    std::atomic_bool quietMode_{false}; // if quietMode_ is true, enable bt without AutoConnect

    class AdaptersContextCallback;
    std::unique_ptr<AdaptersContextCallback> contextCallback_ = nullptr;
    BtInterface* bluetoothInterface = nullptr;
    void OnEnable(const std::string &name, bool ret);
    void OnDisable(const std::string &name, bool ret);
    void ProcessMessage(const BTTransport transport, const utility::Message &msg);
    void ProcessMessage(const BTTransport transport, const utility::Message &msg, const std::string &callingName);

    std::mutex initializedMutex_ {};
    bool isInitialized_ = false;  // guarded by initializedMutex_
    std::condition_variable initializedConditionVariable_ {};
    bool WaitAdapterManagerInitializeComplete(void);

    std::mutex thirdpartyOperateBluetoothMutex_ {};
    bool isThirdPartyAppOperateBluetoothSwitch_ = false;
    std::string thirdPartyAppOperateBluetoothSwitchName_ = "";
    void ThirdpartyAppBeginOperateBluetoothSwitch(const std::string &callingName);
    std::string AttemptReplaceThirdpartyAppName(const std::string &callingName);
    void ClearThirdpartyAppOperateBluetoothSwitch();

    BT_DISALLOW_COPY_AND_ASSIGN(impl);
};

class AdapterManager::impl::AdaptersContextCallback : public utility::IContextCallback {
public:
    explicit AdaptersContextCallback(AdapterManager::impl &impl) : impl_(impl){};
    ~AdaptersContextCallback() override = default;

    void OnEnable(const std::string &name, bool ret) override
    {
        HILOGI("name=%{public}s, ret=%{public}d\n", name.c_str(), ret);
        impl_.OnEnable(name, ret);
    }
    void OnDisable(const std::string &name, bool ret) override
    {
        HILOGI("name=%{public}s, ret=%{public}d\n", name.c_str(), ret);
        impl_.OnDisable(name, ret);
    }

private:
    AdapterManager::impl &impl_;
};

AdapterManager::impl::impl()
{
    // context callback create
    contextCallback_ = std::make_unique<AdaptersContextCallback>(*this);
    satelliteManager_ = std::make_shared<SatelliteManager>();
    satelliteManager_->Init();
}

AdapterManager::impl::~impl()
{}

void AdapterManager::impl::OnEnable(const std::string &name, bool ret)
{
    HILOGI("[ADAPTER_MANAGER]adapter_name=%{public}s, ret=%{public}d\n", name.c_str(), ret);
    BTTransport transport = BTTransport::ADAPTER_BREDR;

    if (name.c_str() == ADAPTER_NAME_CLASSIC) {
        transport = BTTransport::ADAPTER_BREDR;
    } else if (name.c_str() == ADAPTER_NAME_BLE) {
        transport = BTTransport::ADAPTER_BLE;
    } else {
        HILOGE("[ADAPTER_MANAGER]adapter_name=%{public}s is warning, transport\n", name.c_str());
    }

    utility::Message msg(AdapterStateMachine::MSG_ADAPTER_ENABLE_CMP, ret ? true : false);
    DoInAdapterManagerThread([this, transport, msg] {this->ProcessMessage(transport, msg);});
}

void AdapterManager::impl::OnDisable(const std::string &name, bool ret)
{
    HILOGI("[ADAPTER_MANAGER]adapter_name=%{public}s, ret=%{public}d\n", name.c_str(), ret);
    BTTransport transport = BTTransport::ADAPTER_BREDR;

    if (name.c_str() == ADAPTER_NAME_CLASSIC) {
        transport = BTTransport::ADAPTER_BREDR;
    } else if (name.c_str() == ADAPTER_NAME_BLE) {
        transport = BTTransport::ADAPTER_BLE;
    } else {
        HILOGE("[ADAPTER_MANAGER]adapter_name=%{public}s is warning, transport\n", name.c_str());
    }

    utility::Message msg(AdapterStateMachine::MSG_ADAPTER_DISABLE_CMP, ret ? true : false);
    DoInAdapterManagerThread([this, transport, msg] {this->ProcessMessage(transport, msg);});
}

void AdapterManager::impl::ProcessMessage(const BTTransport transport, const utility::Message &msg)
{
    if (transport == ADAPTER_BREDR && classicAdapter_ && classicAdapter_->stateMachine) {
        classicAdapter_->stateMachine->ProcessMessage(msg);
        return;
    }
    if (transport == ADAPTER_BLE && bleAdapter_ && bleAdapter_->stateMachine) {
        bleAdapter_->stateMachine->ProcessMessage(msg);
        return;
    }

    HILOGE("transport(%{public}d) failed", transport);
}

void AdapterManager::impl::ProcessMessage(
    const BTTransport transport, const utility::Message &msg, const std::string &callingName)
{
    if (transport == ADAPTER_BLE && bleAdapter_ && bleAdapter_->stateMachine) {
        bleAdapter_->stateMachine->SetCallingName(callingName);
    }
    ProcessMessage(transport, msg);
}

bool AdapterManager::impl::WaitAdapterManagerInitializeComplete(void)
{
    std::unique_lock<std::mutex> lock(initializedMutex_);
    if (isInitialized_) {
        return true;
    }
    if (!initializedConditionVariable_.wait_for(lock, std::chrono::milliseconds(INITIALIZE_DELAY_MS),
        [this]() { return isInitialized_; })) {
        HILOGW("adapter manager initialize failed");
        return false;
    }
    return true;
}

// AdapterManager class
AdapterManager::AdapterManager() : pimpl(std::make_unique<AdapterManager::impl>())
{
    // sys state Machine create
    pimpl->sysStateMachine_.Init(*this);
    paramUpdateManager_ = std::make_shared<ParamUpdateEventManager>();
    dialogResult_ = std::make_shared<DialogResult>();
}

void AdapterManager::ProcessStackEnableCmpMsg()
{
    utility::Message msg(AdapterStateMachine::MSG_STACK_ENABLE_CMP);
    if (GetState(BTTransport::ADAPTER_BREDR) == BTStateID::STATE_TURNING_ON) {
        DoInAdapterManagerThread([this, msg] {this->pimpl.get()->ProcessMessage(BTTransport::ADAPTER_BREDR, msg);});
    }
    if (GetState(BTTransport::ADAPTER_BLE) == BTStateID::STATE_TURNING_ON) {
        DoInAdapterManagerThread([this, msg] {this->pimpl.get()->ProcessMessage(BTTransport::ADAPTER_BLE, msg);});
    }
}

void AdapterManager::ProcessStackDisableCmpMsg()
{
    utility::Message msg(AdapterStateMachine::MSG_STACK_DISABLE_CMP);
    if (GetState(BTTransport::ADAPTER_BREDR) == BTStateID::STATE_TURNING_OFF) {
        DoInAdapterManagerThread([this, msg] {this->pimpl.get()->ProcessMessage(BTTransport::ADAPTER_BREDR, msg);});
    }
    if (GetState(BTTransport::ADAPTER_BLE) == BTStateID::STATE_TURNING_OFF) {
        DoInAdapterManagerThread([this, msg] {this->pimpl.get()->ProcessMessage(BTTransport::ADAPTER_BLE, msg);});
    }
}
void AdapterManager::AdapterStateChangedInner(BtState state)
{
    HILOGI("[ADAPTER_MANAGER]stack bluetooth switch state = %{public}d", state);
    if (state == BT_STATE_OFF) {
        ProcessStackDisableCmpMsg();
        return;
    }
    if (state == BT_STATE_ON) {
        ProcessStackEnableCmpMsg();
        return;
    }
}
void AdapterManager::AdapterStateChangedCb(BtState state)
{
    AdapterManager::GetInstance()->AdapterStateChangedInner(state);
}

void AdapterManager::DeviceFoundCb(int numProperties, BtProperty* properties)
{
    RemoteDeviceProperties* remoteDeviceProperties = RemoteDeviceProperties::GetInstance();
    if (remoteDeviceProperties) {
        remoteDeviceProperties->DeviceFoundCallBack(numProperties, properties);
    }
}

void AdapterManager::DiscoveryStateChangedCb(BtDiscoveryState state)
{
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter) {
        classicAdapter->DiscoveryStateChanged(state);
    }
}

void AdapterManager::BondStateChangedCb(BtStackStatus status, OHOS::bluetooth::RawAddress* bdAddr, BtBondState state)
{
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter) {
        classicAdapter->BondStateChanged(status, bdAddr, state);
        RawAddress rawAddr = *bdAddr;
        BluetoothHelper::BluetoothCommonEventHelper::PublishRemoteDevicePairStateEvent(rawAddr.GetAddress(), state);
    }
}

void AdapterManager::SspRequestCb(OHOS::bluetooth::RawAddress* remoteBdAddr, BtBdname* bdName, uint32_t cod,
    BtSspVariant pairingVariant, uint32_t passKey)
{
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter) {
        classicAdapter->SspRequest(remoteBdAddr, bdName, cod, pairingVariant, passKey);
    }
}

static bool CheckIsControlInterceptAllowedAndDisconnect(const std::string address)
{
    ControlInterceptMessage msg {
        .addr = address,
    };
    if (!ControlInterceptIsAllowedAclConn(msg)) {
        auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
        if (classicAdapter != nullptr) {
            classicAdapter->DisconnectAcl(address);
            return false;
        }
    }
    return true;
}

void AdapterManager::AclStateChangedCb(BtStackStatus status, OHOS::bluetooth::RawAddress* remoteBdAddr, BtAclState state,
    BtHciErrorCode hciReason, BtTransport linkTypeCallback)
{
    CHECK_AND_RETURN_LOG(remoteBdAddr, "wrong addr");
    CHECK_AND_RETURN_LOG(status == BT_STATUS_SUCCESS, "Acl Connection fail, hci error code = %{public}u", hciReason);

    RawAddress device = *remoteBdAddr;
    int linkType = ServiceUtil::LinkTypeFromStack(linkTypeCallback);
    HILOG_COMM_INFO("AclStateChangedCb address %{public}s linkType %{public}d connectState %{public}d "
        "status %{public}d", GetEncryptAddr(device.GetAddress()).c_str(), linkType, state, status);
    if (state == BT_ACL_STATE_DISCONNECTED) {
        ConnectStrategyManager::GetInstance()->EraseDevicePreviouslyConnected(device.GetAddress());
    }
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
    int connectState = HandleAclStateChanged(remoteDevice, device, remoteBdAddr, state);
    BtChrUeManager::GetInstance()->WriteStateChangeUe(CHR_UE_BRCAST_ACL_CONN, device, hciReason,
        linkType, connectState);
    if (connectState == CONNECTION_STATE_CONNECTED) {
        CHECK_AND_RETURN_LOG(CheckIsControlInterceptAllowedAndDisconnect(device.GetAddress()),
            "Restricted by control intercept");
    }
    // disconnect unpaired acl address eg. socket unsafe connect
    std::string unPairedAddr;
    auto connectionManager = BluetoothConnectionManager::GetInstance();

    if (linkType == LINK_TYPE_BREDR) {
        if (connectState == CONNECTION_STATE_CONNECTED) {
            bool isAllowedAclConnect = connectionManager->IsBrLinkAllowed(device.GetAddress(), unPairedAddr);
            CHECK_AND_RETURN_LOG(isAllowedAclConnect, "Acl Connection fail, more than max acl links");
        }
        connectionManager->RecordAclConnect(device.GetAddress(), linkType, connectState, unPairedAddr);
        auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
        if (classicAdapter) {
            classicAdapter->SendAclStateChanged(device, connectState, hciReason, remoteDevice->GetPairedStatus());
        }
    } else if (linkType == LINK_TYPE_LE) {
        auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
        if (classicAdapter && connectState == CONNECTION_STATE_DISCONNECTED) {
            classicAdapter->RemovePendingConnectDevice(device);
        }
        connectionManager->RecordAclConnect(device.GetAddress(), linkType, connectState, unPairedAddr);
        auto bleAdapter = AdapterManager::GetInstance()->GetBleAdapter();
        if (bleAdapter) {
            bleAdapter->SendAclStateChanged(device, connectState, hciReason, remoteDevice->GetPairedStatus());
        }
    } else {
        HILOGE("Unknown link type: %{public}d", linkType);
    }
    BluetoothDeviceBatteryManager::GetInstance()->ProcessAclStateChanged(device.GetAddress(), connectState);
    HandleCloudBondWhenAclStateChange(device, connectState);
}

void AdapterManager::AdapterPropertiesCb(BtStackStatus status, int numProperties, BtProperty* properties)
{
    AdapterProperties* adapterProperties = AdapterProperties::GetInstance();
    adapterProperties->ParseAdapterProps(status, numProperties, properties);
}

void AdapterManager::RemoteDevicePropertiesCb(BtStackStatus status, OHOS::bluetooth::RawAddress* bdAddr,
    int numProperties, BtProperty* properties)
{
    RemoteDeviceProperties* remoteDeviceProperties = RemoteDeviceProperties::GetInstance();
    if (remoteDeviceProperties) {
        remoteDeviceProperties->GetRemoteDevicePropsCallBack(status, bdAddr, numProperties, properties);
    }
}

void AdapterManager::ThreadEvtCb(BtCbThreadEvt evt)
{
}

void AdapterManager::PinRequestCb(OHOS::bluetooth::RawAddress* remoteBdAddr, BtBdname* bdName, uint32_t cod,
    bool min16Digit)
{
    HILOG_COMM_INFO("PinRequestCb enter");
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter) {
        classicAdapter->PinRequest(remoteBdAddr, bdName, cod, min16Digit);
    }
}

void AdapterManager::DutModeRecvCb(uint16_t opcode, uint8_t* buf, uint8_t len)
{
}

void AdapterManager::EnergyInfoCb(BtActivityEnergyInfo* energyInfo, BtUidTraffic* uidData)
{
}

void AdapterManager::SensingStateChangedCb(uint8_t eventId, BtSensingInfo* info)
{
    CHECK_AND_RETURN_LOG(info != nullptr, "fail to get sensing info");
    HILOGI("eventId: %{public}d", eventId);
    SensingInfo sensingInfo(info->addr.GetAddress(), info->uuid.ToString(), info->resourceId, info->interval);
    auto resourceMgr = BluetoothResourceManager::GetInstance();
    if (resourceMgr) {
        resourceMgr->SendSensingStateChanged(eventId, sensingInfo);
    }
}

Bluetooth::BluetoothOobData AdapterManager::BuildBluetoothOobData(const BtStackOobData &data)
{
    BluetoothOobData outData;
    // data.address数组大小为固定值7
    std::vector<uint8_t> addressWithType(data.address, data.address + sizeof(data.address) / sizeof(data.address[0]));
    std::reverse(addressWithType.begin(), addressWithType.begin() + 6); // addressWithType中前6位为小端序的地址，转为大端序
    outData.SetAddressWithType(addressWithType);
    std::vector<uint8_t> confirmHash(data.c, data.c + sizeof(data.c) / sizeof(data.c[0]));
    outData.SetConfirmationHash(confirmHash);
    std::vector<uint8_t> randomHash(data.r, data.r + sizeof(data.r) / sizeof(data.r[0]));
    outData.SetRandomizerHash(randomHash);
    std::string deviceName(reinterpret_cast<const char*>(data.deviceName));
    if (!deviceName.empty()) {
        outData.SetDeviceName(deviceName);
    }
    outData.SetDeviceRole(data.leDeviceRole);
    return outData;
}

void AdapterManager::GenerateLocalOobDataCb(BtTransport transport, BtStackOobData oobData)
{
    HILOGI("transport: %{public}d", ServiceUtil::TransportFromStack(transport));
    int32_t status = BT_STATUS_FAIL;
    BluetoothOobData data;
    if (oobData.isValid) {
        data = BuildBluetoothOobData(oobData);
        status = BT_STATUS_SUCCESS;
    } else {
        HILOGE("invalid oob from stack");
    }
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter) {
        classicAdapter->GenerateLocalOobDataCb(status, data);
    }
}

#ifdef COMMUNICATION_L2
int AdapterManager::AcquireWakeLock(const char* lockName)
{
    return AdapterManager::GetInstance()->AcquireWakeLockCallout(lockName);
}

int AdapterManager::AcquireWakeLockCallout(const char* lockName)
{
    HITRACE_METER(BT_TRACE_TAG);
    HILOGD("acquire_wake_lock");
    auto& powerMgrClient = PowerMgr::PowerMgrClient::GetInstance();
    std::string loclName = lockName;
    if (runningLock_ == nullptr) {
        runningLock_ = powerMgrClient.CreateRunningLock(loclName, PowerMgr::RunningLockType::RUNNINGLOCK_BACKGROUND);
    }
    CHECK_AND_RETURN_LOG_RET(runningLock_ != nullptr, BT_STATUS_FAIL, "runningLock_ is nullptr");
    if (!runningLock_->IsUsed()) {
        runningLock_->Lock();
    }
    return BT_STATUS_SUCCESS;
}

int AdapterManager::ReleaseWakeLock(const char* lockName)
{
    AdapterManager::GetInstance()->ReleaseWakeLockCallout(lockName);
    return BT_STATUS_SUCCESS;
}

int AdapterManager::ReleaseWakeLockCallout(const char* lockName)
{
    std::string loclName = lockName;
    HILOGD("release_wake_lock");
    if (runningLock_ == nullptr) {
        HILOGE("running lock is nullptr");
        return BT_STATUS_FAIL;
    }
    if (runningLock_->IsUsed()) {
        runningLock_->UnLock();
    }
    return BT_STATUS_SUCCESS;
}

BtOsCallouts BluetoothOsCallouts = {
    .size = sizeof(BluetoothOsCallouts),
    .setWakeAlarm = nullptr,
    .acquireWakeLock = AdapterManager::AcquireWakeLock,
    .releaseWakeLock = AdapterManager::ReleaseWakeLock,
};
#endif

BtCallbacks bt_callbacks {
/** set to sizeof(BtCallbacks) */
    .size = sizeof(BtCallbacks),
    .adapterStateChangedCb = AdapterManager::AdapterStateChangedCb,
    .adapterPropertiesCb = AdapterManager::AdapterPropertiesCb,
    .remoteDevicePropertiesCb = AdapterManager::RemoteDevicePropertiesCb,
    .deviceFoundCb = AdapterManager::DeviceFoundCb,
    .discoveryStateChangedCb = AdapterManager::DiscoveryStateChangedCb,
    .pinRequestCb = AdapterManager::PinRequestCb,
    .sspRequestCb = AdapterManager::SspRequestCb,
    .bondStateChangedCb = AdapterManager::BondStateChangedCb,
    .aclStateChangedCb = AdapterManager::AclStateChangedCb,
    .threadEvtCb = AdapterManager::ThreadEvtCb,
    .dutModeRecvCb = AdapterManager::DutModeRecvCb,
    .energyInfoCb = AdapterManager::EnergyInfoCb,
    .generateLocalOobDataCb = AdapterManager::GenerateLocalOobDataCb,
    .sensingStateChangedCb = AdapterManager::SensingStateChangedCb,
};

int AdapterManager::StackInit()
{
    HITRACE_METER(HITRACE_TAG_ZAUDIO);
    const bool startRestricted = false;
    const bool isCommonCriteriaMode = false;
    int configCompareResult = 0;
    bool isAtv = false;
    int status = hal_util_load_bt_library((const BtInterface**)&(pimpl->bluetoothInterface));
    if (status) {
        HILOGE("Failed to open the Bluetooth stack");
        return BT_STATUS_FAIL;
    }
#ifdef QOS_MANAGER_ENABLE
    std::unordered_map<std::string, std::string> payload;
    int32_t AUDIO_PROC_QOS_TABLE = 7;
    payload["groupId"] = std::to_string(AUDIO_PROC_QOS_TABLE);
    payload["pid"] = std::to_string(getpid()); // 这里需要将pid的输入转化为string类型
    OHOS::ConcurrentTask::ConcurrentTaskClient::GetInstance().RequestAuth(payload); // 向concurrent_task服务申请对自己进程鉴权
#endif
    int ret = pimpl->bluetoothInterface->init(
        &bt_callbacks, startRestricted, isCommonCriteriaMode,
        configCompareResult, nullptr, isAtv);
    if (ret != BT_STATUS_SUCCESS) {
        HILOGE("AdapterManager Init fail!");
        return BT_STATUS_FAIL;
    }
#ifdef COMMUNICATION_L2
    ret= pimpl->bluetoothInterface->setOsCallouts(&BluetoothOsCallouts);
    HILOGI("set_os_callouts");
    if (ret != BT_STATUS_SUCCESS) {
        HILOGE("weaklocal Init fail!");
        return BT_STATUS_WAKELOCK_ERROR;
    }
#endif
    return ret;
}

AdapterManager::~AdapterManager()
{}

bool AdapterManager::IsCollaborationModeOn(void) const
{
    std::string value = "";
    BluetoothDataShareQuery(
        SETTINGS_DATASHARE_URI_COLLABORATION_SWITCH, SETTINGS_DATASHARE_KEY_COLLABORATION_SWITCH, value);
    HILOGI("Collaboration switch is %{public}s", value.c_str());
    return value == "1";
}

int AdapterManager::EnableBluetoothFromOffToRestrictMode(const std::string &callingName) const
{
    pimpl->WaitAdapterManagerInitializeComplete();
    int32_t ret = Enable(ADAPTER_BLE, false, callingName);
    
    if (ret == BT_NO_ERROR) {
        SetBluetoothRestrictedFlag(true);
    }

    return ret;
}

static void CollaborationModeChangeCallback(const char *key, const char *value, void *context)
{
    if (key == nullptr || value == nullptr) {
        HILOGE("Invalid key or value");
        return;
    }
    bool openCollaborationWhenBluetoothIsWaitingUnload =
        key == std::string(COLLABORATION_SERVICE_PARAM) &&
        value == std::string("1") &&
        AdapterManager::GetInstance()->IsUnloadBluetoothSaTimerStarted();
    if (openCollaborationWhenBluetoothIsWaitingUnload) {
        HILOGI("persist.bluetooth.collaboration_service change to 1, stop unload bluetooth sa");
        AdapterManager::GetInstance()->StopUnloadBluetoothSaTimer();
        AdapterManager::GetInstance()->EnableBluetoothFromOffToRestrictMode("bluetooth_collaboration");
    }
}

void AdapterManager::Initialize() const
{
#ifdef BLUETOOTH_PLUGGABLE_SUPPORTED
    if (!CheckBluetoothPluggableState()) {
        return;
    }
#endif
#ifdef CONTEXTHUB_BLE_V3
    SensorhubCollaborationService::GetInstance()->Init();
#endif

    int ret = WatchParameter(COLLABORATION_SERVICE_PARAM, CollaborationModeChangeCallback, nullptr);
    if (ret != 0) {
        HILOGE("WatchParameter failed, ret(%{public}d)", ret);
    }

    if (pimpl->satelliteManager_->IsSatelliteControlBtSwitch()) {
        HILOGW("Satellite restrict bluetooth switch, can't auto enable bluetooth");
        return;
    }

    // 自动开蓝牙 - 手表退出STR模式
    if (IsEnableBluetoothAfterExitStrModeHook()) {
        return;
    }

#ifndef BLUETOOTH_FACTORY_MODE
    // 自动开蓝牙 - 重启场景
    const int bluetoothStateHalf = 2;
    int lastBtState = GetIntParameter(g_bluetoothSwitchStatePropertyName, 0);
    if (lastBtState == bluetoothStateHalf) {
        pimpl->isRestrictBluetooth = true;
    }
    if (lastBtState > 0) {
        HILOGI("enable bluetooth lastBtState: %{public}d, 1(on) 2(half)", lastBtState);
        DoInAdapterManagerThread(
            []() { AdapterManager::GetInstance()->Enable(ADAPTER_BLE, false, "bluetooth_reboot"); });
        return;
    }

    // 自动开蓝牙 - 飞行模式关闭，多设备协同打开场景（会和用户手动开蓝牙场景冲突）
    if (IsCollaborationModeOn() && !BluetoothAirplaneManager::GetInstance()->IsAirplaneModeOn()) {
        const int delayTimeMs = 500;  // 500 ms
        ThreadUtil::GetInstance().PostTask(THREAD_ID_ADAPTER_MANAGER, []() {
            if (AdapterManager::GetInstance()->GetState(ADAPTER_BLE) == STATE_TURN_OFF) {
                AdapterManager::GetInstance()->EnableBluetoothFromOffToRestrictMode("bluetooth_reboot");
            }
        }, delayTimeMs, "EnableBluetoothHalfTaskWhenCollaborationOn");
        return;
    }
#endif
}

#ifdef BLUETOOTH_PLUGGABLE_SUPPORTED
bool AdapterManager::CheckBluetoothPluggableState() const
{
    std::string btPluggableState = BLUETOOTH_PLUGGABLE_STATE_EXTRACT;
    int32_t res = system::GetStringParameter(BLUETOOTH_PLUGGABLE_STATE, btPluggableState,
        BLUETOOTH_PLUGGABLE_STATE_EXTRACT);
    bool isSupported = (res == 0 && btPluggableState == BLUETOOTH_PLUGGABLE_STATE_EMPLACE);
    if (!isSupported) {
        HILOGE("bluetooth not supported");
        SetParameter(g_bluetoothSwitchStatePropertyName, g_bluetoothSwitchStateOff);
        PromptUnloadBluetoothSystemAbility();
        return false;
    }
    const int needEnableBt = 1;
    if (GetIntParameter(BLUETOOTH_EMPLACE_ENABLE_STATE, 0) == needEnableBt) {
        SetParameter(g_bluetoothSwitchStatePropertyName, g_bluetoothSwitchStateOn);
    }
    return true;
}
#endif

void AdapterManager::WaitForAllSwitchDependency(void)
{
    auto func = []() {
        DoInAdapterManagerThread([]() { AdapterManager::GetInstance()->InitializeAfterAllDependencyOn(); });
    };
    switchDependency_ = std::make_shared<SwitchDependency>(func);
    switchDependency_->Init();
}

void AdapterManager::InitializeAfterAllDependencyOn(void)
{
    // Bluetooth all dependency is ready, clear it.
    switchDependency_ = nullptr;

    if (paramUpdateManager_ != nullptr) {
        paramUpdateManager_->SubscriberEvent();
    }

    Initialize();
    OnBluetoothInitHook();
    std::lock_guard<std::mutex> lock(pimpl->initializedMutex_);
    pimpl->isInitialized_ = true;
    pimpl->initializedConditionVariable_.notify_all();
}

bool AdapterManager::Start()
{
    if (GetSysState() == SYS_STATE_STARTED) {
        HILOGE("Bluetooth has been started!!");
        return false;
    }

    if (!ClassicConfig::GetInstance().LoadConfigFile()) {
        HILOGE("Load Classic Config File Failed!!");
        PromptUnloadBluetoothSystemAbility();
        return false;
    }

    if (!AdapterConfig::GetInstance()->Load()) {
        HILOGE("Load Config File Failed!!");
        PromptUnloadBluetoothSystemAbility();
        return false;
    }

    if (!ProfileConfig::GetInstance()->Load()) {
        HILOGE("Load Profile Config File Failed!!");
        PromptUnloadBluetoothSystemAbility();
        return false;
    }

    if (StackInit() != BT_STATUS_SUCCESS) {
        HILOGE("Bluetooth Stack Initialize Failed!!");
        PromptUnloadBluetoothSystemAbility();
        return false;
    }

    CreateAdapters();

    ProfileServiceManager::Initialize();

    PowerManager::Initialize();

    RegisterHciResetCallback();

    OnSysStateChange(SYS_STATE_STARTED);

    utility::Message msg(SysStateMachine::MSG_SYS_START_CMP);
    DoInAdapterManagerThread([this, msg] {this->pimpl->sysStateMachine_.ProcessMessage(msg);});

    WaitForAllSwitchDependency();
    SystemAbilityProcesser::Initialize();
    return true;
}

void AdapterManager::Stop() const
{
    if (GetSysState() == SYS_STATE_STOPPED) {
        HILOGI("AdapterManager is stoped");
    } else if (GetSysState() == SYS_STATE_STOPPING) {
        HILOGI("AdapterManager is stoping...");
    } else {
        if (paramUpdateManager_ != nullptr) {
            paramUpdateManager_->UnSubscriberEvent();
        }
        OnBluetoothDestroyHook();
        std::promise<void> stopPromise;
        std::future<void> stopFuture = stopPromise.get_future();
        pimpl->stopPromise_ = std::move(stopPromise);

        utility::Message msg(SysStateMachine::MSG_SYS_STOP_REQ);
        DoInAdapterManagerThread([this, msg] {this->pimpl->sysStateMachine_.ProcessMessage(msg);});
        stopFuture.wait();
    }
}

bool AdapterManager::AdapterStop() const
{
    ProfileServiceManager::Uninitialize();
    IPowerManager::Uninitialize();
    DeregisterHciResetCallback();
    SystemAbilityProcesser::Uninitialize();

    if (pimpl->classicAdapter_) {
        pimpl->classicAdapter_->instance->GetContext()->Uninitialize();
        pimpl->classicAdapter_ = nullptr;
    }
    if (pimpl->bleAdapter_) {
        pimpl->bleAdapter_->instance->GetContext()->Uninitialize();
        pimpl->bleAdapter_ = nullptr;
    }

    /* Unified stack (libbtstack): release HCI/modules so the next StackInit can DmInit again. */
    if (pimpl->bluetoothInterface != nullptr && pimpl->bluetoothInterface->cleanup != nullptr) {
        HILOGI("cleanup bluetooth stack on AdapterStop");
        pimpl->bluetoothInterface->cleanup();
    }

    {
        std::lock_guard<std::mutex> lock(pimpl->initializedMutex_);
        pimpl->isInitialized_ = false;
    }

    utility::Message msg(SysStateMachine::MSG_SYS_STOP_CMP);
    DoInAdapterManagerThread([this, msg] {this->pimpl->sysStateMachine_.ProcessMessage(msg);});

    return true;
}

int32_t AdapterManager::NotifyDialogResult(uint32_t dialogType, bool dialogResult) const
{
    HILOGI("[ADAPTER_MANAGER]dialogType: %{public}u, dialogResult: %{public}d",
        dialogType, dialogResult);
    dialogResult_->NotifyBluetoothSwitchDialogResult(dialogType, dialogResult);
    if (!dialogResult) {
        pimpl->ClearThirdpartyAppOperateBluetoothSwitch();
    }
    return BT_NO_ERROR;
}

void AdapterManager::impl::ThirdpartyAppBeginOperateBluetoothSwitch(const std::string &callingName)
{
    std::lock_guard<std::mutex> lock(thirdpartyOperateBluetoothMutex_);
    isThirdPartyAppOperateBluetoothSwitch_ = true;
    thirdPartyAppOperateBluetoothSwitchName_ = callingName;
}

void AdapterManager::impl::ClearThirdpartyAppOperateBluetoothSwitch()
{
    std::lock_guard<std::mutex> lock(thirdpartyOperateBluetoothMutex_);
    isThirdPartyAppOperateBluetoothSwitch_ = false;
    thirdPartyAppOperateBluetoothSwitchName_ = "";
}

std::string AdapterManager::impl::AttemptReplaceThirdpartyAppName(const std::string &callingName)
{
    std::string ret = callingName;
    std::lock_guard<std::mutex> lock(thirdpartyOperateBluetoothMutex_);
    if (isThirdPartyAppOperateBluetoothSwitch_) {
        isThirdPartyAppOperateBluetoothSwitch_ = false;
        ret = thirdPartyAppOperateBluetoothSwitchName_;
        thirdPartyAppOperateBluetoothSwitchName_ = "";
    }
    return ret;
}

int32_t AdapterManager::EnableInner(const std::string &callingName, const BTTransport transport, bool isAsync,
    bool isUserTriggered) const
{
    if (transport == ADAPTER_BLE && pimpl->satelliteManager_->IsSatelliteControlBtSwitch()) {
        HILOGW("Satellite restrict bluetooth switch, can't enable bluetooth");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!PermissionManager::IsSystemHap() && transport == ADAPTER_BLE &&
        DialogSwitch::RequestBluetoothSwitchDialog(ENABLE_BLUETOOTH)) {
        pimpl->ThirdpartyAppBeginOperateBluetoothSwitch(callingName);
        return dialogResult_->HandleBluetoothSwitchDialogResult(isAsync);
    }
    // during 10s delay to unload bluetooth, enable bluetooth, stop timer
    if (IsUnloadBluetoothSaTimerStarted()) {
        StopUnloadBluetoothSaTimer();
    }

    auto msg = utility::Message(AdapterStateMachine::MSG_USER_ENABLE_REQ);
    DoInAdapterManagerThread([this, transport, msg, callingName] {
        this->pimpl.get()->ProcessMessage(transport, msg, callingName);
    });
    BtChrUeManager::GetInstance()->WriteBtSwitchChangeUe(transport, UE_COMMON_SCENE_CASE0, UE_COMMON_SCENE_CASE0,
        callingName, 0);
    return BT_NO_ERROR;
}

int32_t AdapterManager::Enable(BTTransport transport, bool isAsync, std::string callingName,
    bool isUserTriggered) const
{
    if (!pimpl->WaitAdapterManagerInitializeComplete()) {
        HILOGE("[ADAPTER_MANAGER]Bluetooth cannot be enabled when all dependency is not ready");
        PromptUnloadBluetoothSystemAbility();
        return BT_ERR_INTERNAL_ERROR;
    }
    callingName = pimpl->AttemptReplaceThirdpartyAppName(callingName);
    bool isSystemHap = PermissionManager::IsSystemHap();
    HILOGI("[ADAPTER_MANAGER]start transport is %{public}d, callingName(%{public}s), isSystemHap=%{public}d",
        transport, callingName.c_str(), isSystemHap);
    if (StrModeAbnormalExitHandleHook(transport) != BT_STATUS_SUCCESS) {
        HILOGE("[ADAPTER_MANAGER]Bluetooth cannot be enabled when the watch is in STR mode");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (GetSysState() != SYS_STATE_STARTED) {
        HILOGE("[ADAPTER_MANAGER]AdapterManager system is stoped");
        BtChrUeManager::GetInstance()->WriteBtSwitchChangeUe(transport, UE_COMMON_SCENE_CASE0, UE_COMMON_SCENE_CASE1,
            callingName, 0);
        return BT_ERR_INTERNAL_ERROR;
    }

    if (IsAdpaterNullptrWhenEnable(transport)) {
        return BT_ERR_INTERNAL_ERROR;
    }

    if (GetState(transport) == BTStateID::STATE_TURN_OFF) {
        return EnableInner(callingName, transport, isAsync, isUserTriggered);
    } else if (GetState(transport) == BTStateID::STATE_TURN_ON) {
        // svc/Settings call Enable(BLE). When BLE is already on, continue classic bring-up.
        if (transport == ADAPTER_BLE) {
            int brState = GetState(ADAPTER_BREDR);
            if (brState == BTStateID::STATE_TURN_OFF) {
                HILOGI("[ADAPTER_MANAGER]BLE already on, enable ADAPTER_BREDR");
                return Enable(ADAPTER_BREDR, isAsync, callingName, isUserTriggered);
            }
            if (brState == BTStateID::STATE_TURNING_ON) {
                // Classic profiles may still be completing; treat as in-progress success for svc.
                HILOGI("[ADAPTER_MANAGER]BLE already on, ADAPTER_BREDR is turning on");
                return BT_NO_ERROR;
            }
            // BREDR already on: full bluetooth is on.
            HILOGI("[ADAPTER_MANAGER]BLE and BREDR already on");
            return BT_NO_ERROR;
        }
        HILOGE("[ADAPTER_MANAGER]bluetooth switch state is turn on");
        BtChrUeManager::GetInstance()->WriteBtSwitchChangeUe(transport, UE_COMMON_SCENE_CASE0, UE_COMMON_SCENE_CASE3,
            callingName, 0);
        return BT_ERR_BLUETOOTH_TURN_ON;
    } else {
        HILOGE("[ADAPTER_MANAGER]bluetooth switch state is turning state: %{public}d", GetState(transport));
        BtChrUeManager::GetInstance()->WriteBtSwitchChangeUe(transport, UE_COMMON_SCENE_CASE0, UE_COMMON_SCENE_CASE4,
            callingName, 0);
        return BT_ERR_BLUETOOTH_TURNING;
    }
}

int32_t AdapterManager::DisableInner(BTTransport transport, bool isAsync, std::string callingName) const
{
#ifndef DISABLE_BT_SUPPORTED
    if (!PermissionManager::IsSystemHap() && transport == ADAPTER_BREDR) {
        return BT_ERR_API_NOT_SUPPORT;
    }
#endif
    if (!PermissionManager::IsSystemHap() && transport == ADAPTER_BREDR &&
        DialogSwitch::RequestBluetoothSwitchDialog(DISABLE_BLUETOOTH)) {
        pimpl->ThirdpartyAppBeginOperateBluetoothSwitch(callingName);
        return dialogResult_->HandleBluetoothSwitchDialogResult(isAsync);
    }
    // set RestrictBluetooth false, means when disable bluetooth, no Restricted Bluetooth
    SetBluetoothRestrictedFlagOnly(false);
    if (transport == ADAPTER_BREDR) {
        BluetoothTurningOffHook();
        auto connectionManager = BluetoothConnectionManager::GetInstance();
        connectionManager->RemoveAllAclLinks();
        BtChrWriteSwitchEvent(ChrSwitchEvent(callingName, EVENT_TYPE_BT_DISABLE));
        // 发送私有公共广播，通知有应用关闭蓝牙
        OHOS::AAFwk::Want want;
        want.SetAction("usual.event.bluetooth.DISABLE_BLUETOOTH");
        want.SetParam("callingName", callingName);
        BluetoothHelper::BluetoothCommonEventHelper::PublishEventWithManagePermission(want, false);
    }
    utility::Message msg(AdapterStateMachine::MSG_USER_DISABLE_REQ);
    DoInAdapterManagerThread([this, transport, msg] {this->pimpl.get()->ProcessMessage(transport, msg);});
    BtChrUeManager::GetInstance()->WriteBtSwitchChangeUe(transport, UE_COMMON_SCENE_CASE1, UE_COMMON_SCENE_CASE0,
        callingName, 0);
    return BT_NO_ERROR;
}

int32_t AdapterManager::Disable(BTTransport transport, bool isAsync, std::string callingName,
    bool isUserTriggered) const
{
    callingName = pimpl->AttemptReplaceThirdpartyAppName(callingName);
    bool isSystemHap = PermissionManager::IsSystemHap();
    HILOGI("[ADAPTER_MANAGER]start transport is %{public}d, callingName(%{public}s), isSystemHap=%{public}d",
        transport, callingName.c_str(), isSystemHap);
    if (system::GetBoolParameter("persist.edm.force_enable_bluetooth", false)) {
        HILOGI("Forcibly enable Bluetooth.");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (IsAdpaterNullptrWhenDisable(transport)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (((GetRestrictedState(transport) == BTStateID::STATE_TURN_OFF)) && !PermissionManager::IsSystemHap() &&
        transport == ADAPTER_BREDR && IsBluetoothRestricted()) {
        HILOGI("[AIRPLANE MODE]: Thirdparty application not allow to disablebluetooth when bluetooth is restircted");
        return BT_ERR_INVALID_STATE;
    }
    if (GetState(transport) == BTStateID::STATE_TURN_ON) {
        return DisableInner(transport, isAsync, callingName);
    } else if (GetState(transport) == BTStateID::STATE_TURN_OFF) {
        HILOGE("[ADAPTER_MANAGER]bluetooth switch state is turn off");
        BtChrUeManager::GetInstance()->WriteBtSwitchChangeUe(transport, UE_COMMON_SCENE_CASE1, UE_COMMON_SCENE_CASE2,
            callingName, 0);
        return BT_ERR_INTERNAL_ERROR;
    } else {
        HILOGE("[ADAPTER_MANAGER]bluetooth switch state is turning state: %{public}d", GetState(transport));
        BtChrUeManager::GetInstance()->WriteBtSwitchChangeUe(transport, UE_COMMON_SCENE_CASE1, UE_COMMON_SCENE_CASE3,
            callingName, 0);
        return BT_ERR_INTERNAL_ERROR;
    }
}

bool AdapterManager::FactoryReset() const
{
    if (GetSysState() == SYS_STATE_STARTED) {
        if (pimpl->isFactoryReseting.load()) {
            HILOGE("isFactoryReseting is true");
            return false;
        }
        pimpl->isFactoryReseting = true;

        std::promise<void> resetPromise;
        std::future<void> resetFuture = resetPromise.get_future();
        pimpl->resetPromise_ = std::move(resetPromise);

        utility::Message msg(SysStateMachine::MSG_SYS_FACTORY_RESET_REQ);
        DoInAdapterManagerThread([this, msg] {this->pimpl->sysStateMachine_.ProcessMessage(msg);});
        resetFuture.wait();
        return true;
    } else {
        HILOGI("System state is not started");
        return false;
    }
}

void AdapterManager::HciFailedReset(void *context)
{
    (static_cast<AdapterManager *>(context))->Reset();
}

void AdapterManager::RegisterHciResetCallback()
{
}

void AdapterManager::DeregisterHciResetCallback() const
{
}

void AdapterManager::Reset() const
{
    utility::Message msg(SysStateMachine::MSG_SYS_RESET_REQ);
    DoInAdapterManagerThread([this, msg] {this->pimpl->sysStateMachine_.ProcessMessage(msg);});
}

bool AdapterManager::ClearAllStorage() const
{
    if (!AdapterConfig::GetInstance()->Reload()) {
        return false;
    }
    if (!ProfileConfig::GetInstance()->Reload()) {
        return false;
    }
    if (!AdapterDeviceConfig::GetInstance()->Reload()) {
        return false;
    }
    if (!AdapterDeviceInfo::GetInstance()->Reload()) {
        return false;
    }

    utility::Message msg(SysStateMachine::MSG_SYS_CLEAR_ALL_STORAGE_CMP);
    DoInAdapterManagerThread([this, msg] {this->pimpl->sysStateMachine_.ProcessMessage(msg);});
    return true;
}

BTStateID AdapterManager::GetState(BTTransport transport) const
{
    BTStateID state = BTStateID::STATE_TURN_OFF;
    if (transport == ADAPTER_BREDR && pimpl->classicAdapter_) {
        state = pimpl->classicAdapter_->state.load();
    }
    if (transport == ADAPTER_BLE && pimpl->bleAdapter_) {
        state = pimpl->bleAdapter_->state.load();
    }
    return state;
}

bool AdapterManager::RegisterStateObserver(IAdapterStateObserver &observer) const
{
    return pimpl->adapterObservers_.Register(observer);
}

bool AdapterManager::DeregisterStateObserver(IAdapterStateObserver &observer) const
{
    if (pimpl == nullptr) {
        return false;
    }
    return pimpl->adapterObservers_.Deregister(observer);
}

bool AdapterManager::RegisterSystemStateObserver(ISystemStateObserver &observer) const
{
    return pimpl->systemObservers_.Register(observer);
}

bool AdapterManager::DeregisterSystemStateObserver(ISystemStateObserver &observer) const
{
    if (pimpl == nullptr) {
        return false;
    }
    return pimpl->systemObservers_.Deregister(observer);
}

BTConnectState AdapterManager::GetAdapterConnectState() const
{
    return ProfileServiceManager::GetInstance()->GetProfileServicesConnectState();
}

std::shared_ptr<IAdapterClassic> AdapterManager::GetClassicAdapterInterface(void) const
{
    return pimpl->classicAdapter_ != nullptr ? pimpl->classicAdapter_->instance : nullptr;
}

std::shared_ptr<ClassicAdapter> AdapterManager::GetClassicAdapter(void) const
{
    return pimpl->classicAdapter_ != nullptr ? pimpl->classicAdapter_->instance : nullptr;
}

std::shared_ptr<IAdapterBle> AdapterManager::GetBleAdapterInterface(void) const
{
    return pimpl->bleAdapter_ != nullptr ? pimpl->bleAdapter_->instance : nullptr;
}

std::shared_ptr<BleAdapter> AdapterManager::GetBleAdapter(void) const
{
    return pimpl->bleAdapter_ != nullptr ? pimpl->bleAdapter_->instance : nullptr;
}

void AdapterManager::OnSysStateChange(const std::string &state) const
{
    HITRACE_METER(BT_TRACE_TAG);
    HILOGI("state is %{public}s", state.c_str());

    std::string oldSysState;
    std::string newSysState = state;

    {  // lock start,update systerm state
        std::lock_guard<BtRecursiveMutex> lock(pimpl->sysStateMutex_);
        oldSysState = pimpl->sysState_;
        pimpl->sysState_ = state;
    }  // lock end

    // notify systerm state update
    BTSystemState notifySysState = (newSysState == SYS_STATE_STARTED) ? BTSystemState::ON : BTSystemState::OFF;
    if ((newSysState != oldSysState) && ((newSysState == SYS_STATE_STARTED) || (oldSysState == SYS_STATE_STARTED))) {
        HILOGI("oldSysState is %{public}s, newSysState is %{public}s",
            oldSysState.c_str(), newSysState.c_str());
        pimpl->systemObservers_.ForEach(
            [notifySysState](ISystemStateObserver &observer) { observer.OnSystemStateChange(notifySysState); });
    }
}

std::string AdapterManager::GetSysState() const
{
    std::lock_guard<BtRecursiveMutex> lock(pimpl->sysStateMutex_);
    return pimpl->sysState_;
}

bool AdapterManager::IsBluetoothSysStateStarted() const
{
    if (GetSysState() != SYS_STATE_STARTED) {
        HILOGE("Bluetooth has not been started!");
        return false;
    }
    return true;
}

void AdapterManager::OnSysStateExit(const std::string &state) const
{
    HILOGI("state is %{public}s", state.c_str());

    if (state == SYS_STATE_FRESETTING) {
        pimpl->resetPromise_.set_value();
        pimpl->isFactoryReseting = false;
    } else if (state == SYS_STATE_STOPPING) {
        pimpl->stopPromise_.set_value();
    } else {
        // Nothing to do.
    }
}

static void NotifyAdapterStateChange(
    BaseObserverList<IAdapterStateObserver> &observers, BTTransport transport, BTStateID state)
{
    for (int priority = PRIORITY_BEGIN; priority < PRIORITY_END; priority++) {
        observers.ForEach([transport, state, priority](IAdapterStateObserver &observer) {
            if (observer.GetPriority() == priority) {
                observer.OnStateChange(transport, state);
            }
        });
    }
}
#ifdef BLUETOOTH_KIA_ENABLE
static void NotifyAdapterRefusePolicyChange(
    BaseObserverList<IAdapterStateObserver> &observers, const int32_t pid, const int64_t time)
{
    observers.ForEach([pid, time](IAdapterStateObserver &observer) { observer.OnRefusePolicyChanged(pid, time); });
}
#endif

static void NotifyAdapterStateChangeV2(
    BaseObserverList<IAdapterStateObserver> &observers, int state)
{
    for (int priority = PRIORITY_BEGIN; priority < PRIORITY_END; priority++) {
        observers.ForEach([state, priority](IAdapterStateObserver &observer) {
            if (observer.GetPriority() == priority) {
                observer.OnBluetoothStateChanged(state);
            }
        });
    }
}

void AdapterManager::ExecuteTaskWhenBluetoothOn(void) const
{
    BtChrWriteSwitchEvent(ChrSwitchEvent(EVENT_TYPE_BT_ENABLE_SUCCESS));
    SetHighPower();
    auto classicAdapterInstance = GetClassicAdapterInterface();
#ifdef BLUETOOTH_FASTSCAN_ENABLE
    if (classicAdapterInstance && classicAdapterInstance->GetPairedDevices().size() > 0) {
        DoInLowPriorityThread([]() {BluetoothFastScanManagerLoader::GetInstance().Init();});
    }
#endif
    // delay AUTO_CONNECT_DELAY_MS to auto connect, for the reason that:
    // we need to wait until initialization of the profile service real finished in the stack.
    ThreadUtil::GetInstance().RemoveTask(THREAD_ID_ADAPTER_MANAGER, INITIALIZE_CHECK_TASK);
    PostAutoConnectTask(AUTO_CONNECT_DELAY_MS);
#ifdef BLUETOOTH_POWER_ON_RECONNECT_FEATURE
    PowerOnReconnect::GetInstance()->Initialize();
#endif
    if (IsBluetoothRestricted() && pimpl->classicAdapter_) {
        // change deivce scanmode
        pimpl->classicAdapter_->instance->SetBtScanMode(SCAN_MODE_NONE, 0);
    }
    if (classicAdapterInstance) {
        classicAdapterInstance->InitCarKeyInfo();
    }
#ifdef FEATURE_MOUSE_FAST_CONNECTION
    PowerMgrHelper::GetInstance()->RegisterPowermgrCommonEvent();
#endif
#ifdef EDM_SERVICE_ENABLE
    BluetoothEdmManager::GetInstance().Init();
    HILOGI("BluetoothEdmManager init complete");
#endif
}

void AdapterManager::OnAdapterStateChange(const BTTransport transport, const BTStateID state) const
{
    HITRACE_METER(BT_TRACE_TAG);
    HILOGD("transport is %{public}d state is %{public}d", transport, state);

    if ((transport == ADAPTER_BREDR && pimpl->classicAdapter_ == nullptr) ||
        (transport == ADAPTER_BLE && pimpl->bleAdapter_ == nullptr)) {
        return;
    }
    // must update bluetooth state before check whether need to auto enable/disable bluetooth
    bool ret = UpdateBluetoothState(transport, state);

    UpdateBluetoothSwitchStatus(transport, state);

    if ((transport == ADAPTER_BLE) && (state == STATE_TURN_ON)) {
        HILOGI("enable ADAPTER_BREDR");
        Enable(ADAPTER_BREDR);
    }
    if ((transport == ADAPTER_BREDR) && (state == STATE_TURN_OFF)) {
        HILOGI("disable ADAPTER_BLE");
        Disable(ADAPTER_BLE);
    }

    bool isBrOnInRestrictedMode = IsBluetoothRestricted() && (transport == ADAPTER_BREDR) && state == STATE_TURN_ON;
    bool isNeedReportBrOff = ret && isBrOnInRestrictedMode;
    bool isNeedReportStateChange = ret && !isBrOnInRestrictedMode;
    if (isNeedReportStateChange) {
        if (GetSysState() != SYS_STATE_RESETTING) {
            BluetoothHelper::BluetoothCommonEventHelper::PublishBluetoothStateChangeEvent(state, transport);
            NotifyAdapterStateChange(pimpl->adapterObservers_, transport, state);
            if (transport == ADAPTER_BREDR && state == STATE_TURN_ON) {
                OnBluetoothOnHook();
                NotifyAdapterStateChangeV2(pimpl->adapterObservers_, BluetoothSwitchState::STATE_ON);
            }
        }
    }
    if (isNeedReportBrOff) {
        BluetoothHelper::BluetoothCommonEventHelper::PublishBluetoothStateChangeEvent(STATE_TURN_OFF, ADAPTER_BLE);
        NotifyAdapterStateChange(pimpl->adapterObservers_, ADAPTER_BREDR, STATE_TURN_OFF);
        NotifyAdapterStateChangeV2(pimpl->adapterObservers_, BluetoothSwitchState::STATE_HALF);
        BtChrUeManager::GetInstance()->WriteBtSwitchChangeUe(
            ADAPTER_BREDR, UE_COMMON_SCENE_CASE2, UE_COMMON_SCENE_CASE0, "", 0);
    }

    // notify sys state machine
    int classicState = pimpl->classicAdapter_ ? pimpl->classicAdapter_->state.load() : BTStateID::STATE_TURN_OFF;
    int bleState = pimpl->bleAdapter_ ? pimpl->bleAdapter_->state.load() : BTStateID::STATE_TURN_OFF;

    utility::Message msg(SysStateMachine::MSG_SYS_ADAPTER_STATE_CHANGE_REQ);
    msg.arg1_ = (static_cast<unsigned int>(classicState) << CLASSIC_ENABLE_STATE_BIT) + bleState;
    DoInAdapterManagerThread([this, msg] {this->pimpl->sysStateMachine_.ProcessMessage(msg);});
    UnLoadBluetoothSystemAbility(transport, state);
    if (transport == ADAPTER_BREDR && state == STATE_TURN_ON) {
        ExecuteTaskWhenBluetoothOn();
    }
}

void AdapterManager::SetHighPower() const
{
    const BtInterface *btInterface = nullptr;
    int status = hal_util_load_bt_library(&btInterface);
    if (status != 0) {
        HILOGE("Failed to open the Bluetooth module, status = %{public}d.", status);
        return;
    }
    char btSubChipType[PROPERTY_VALUE_MAX] = {0};
    int ret = GetParameter(CONNECTIVITY_BTCHIPTYPE, nullptr, btSubChipType, PROPERTY_VALUE_MAX - 1); // 1为了防止越界读非'\0'
    if (ret == 0) {
        return;
    }
    std::string chipType(btSubChipType);
    auto iter = find(highPowerV2Chip.begin(), highPowerV2Chip.end(), chipType);
    if (iter != highPowerV2Chip.end()) {
        HILOGI("High Power On");
        btInterface->enableBluetoothHighpower(true);
    }
#ifdef BLUETOOTH_HIGHPOWERV1_ENABLE
    else {
        BluetoothHighPowerManager::GetInstance().Init();
    }
#endif
}

bool AdapterManager::IsBluetoothSwitchEnableFromSystemParameter() const
{
    int state = GetIntParameter(g_bluetoothSwitchStatePropertyName, 0);
    return state > 0;
}

void AdapterManager::UnLoadBluetoothSystemAbility(const BTTransport transport, const BTStateID state) const
{
    if (state != BTStateID::STATE_TURN_OFF) {
        return;
    }
    bool isBrOff = GetState(BTTransport::ADAPTER_BREDR) == BTStateID::STATE_TURN_OFF;
    bool isBleOff = GetState(BTTransport::ADAPTER_BLE) == BTStateID::STATE_TURN_OFF;
    if (transport == BTTransport::ADAPTER_BREDR && !isBleOff) {
        return;
    }
    if (transport == BTTransport::ADAPTER_BLE && !isBrOff) {
        return;
    }
#ifdef BLUETOOTH_POWER_ON_RECONNECT_FEATURE
    PowerOnReconnect::GetInstance()->Uninitialize();
#endif
    OnBluetoothOffHook();
    HILOGI("set persist.bluetooth.switch_enable %{public}s", g_bluetoothSwitchStateOff);
#if defined(DISABLE_BT_SUPPORTED)
    SetParameter(g_bluetoothSwitchStatePropertyName, g_bluetoothSwitchStateOff);
#else
    HILOGI("Bluetooth cannot be disabled!");
#endif
    // Unblock framework BluetoothSwitchModule: without STATE_OFF, isBtSwitchProcessing_
    // stays true and a later UI enable is only cached then dropped on timeout.
    NotifyAdapterStateChangeV2(pimpl->adapterObservers_, BluetoothSwitchState::STATE_OFF);
    BtChrWriteSwitchEvent(ChrSwitchEvent(EVENT_TYPE_BT_DISABLE_SUCCESS));
    BtChrTransactionManager::GetInstance().ReportTransaction();

    if (IsFactoryReset() || pimpl->isAppCloseBt_.load()) {
        pimpl->isAppCloseBt_ = false;
        PromptUnloadBluetoothSystemAbility();
        return;
    }

    HILOGI("bluetooth switch state is off, wait for 10s then unload bluetooth service process");
    StartUnloadBluetoothSaTimer();
}

void AdapterManager::UpdateBluetoothSwitchStatus(const BTTransport transport, const BTStateID state) const
{
    HILOGD("start state is %{public}d, transport = %{public}d", state, transport);
    if (state != BTStateID::STATE_TURN_ON) {
        HILOGD("state is not STATE_TURN_ON, do nothing.");
        return;
    }
    bool isBrOn = GetState(BTTransport::ADAPTER_BREDR) == BTStateID::STATE_TURN_ON;
    bool isBleOn = GetState(BTTransport::ADAPTER_BLE) == BTStateID::STATE_TURN_ON;
    HILOGI("isBrOn = %{public}d, isBleOn = %{public}d", isBrOn, isBleOn);
    if (transport == BTTransport::ADAPTER_BREDR && !isBleOn) {
        return;
    }
    if (transport == BTTransport::ADAPTER_BLE && !isBrOn) {
        return;
    }
    HILOGI("bluetooth switch state is on, Update bluetooth switch status");
    if (!IsBluetoothSwitchEnableFromSystemParameter()) {
        SetParameter(g_bluetoothSwitchStatePropertyName, g_bluetoothSwitchStateOn);
    }
}

void AdapterManager::OnProfileServicesEnableComplete(const BTTransport transport, const bool ret) const
{
    HILOGI("transport is %{public}d, ret is %{public}d", transport, ret);

    utility::Message msg(AdapterStateMachine::MSG_PROFILE_ENABLE_CMP, ret ? true : false);
    DoInAdapterManagerThread([this, transport, msg] {this->pimpl.get()->ProcessMessage(transport, msg);});
}

void AdapterManager::OnProfileServicesDisableComplete(const BTTransport transport, const bool ret) const
{
    HILOGI("transport is %{public}d, ret is %{public}d", transport, ret);

    utility::Message msg(AdapterStateMachine::MSG_PROFILE_DISABLE_CMP, ret ? true : false);
    DoInAdapterManagerThread([this, transport, msg] {this->pimpl.get()->ProcessMessage(transport, msg);});
}

void AdapterManager::OnPairDevicesRemoved(const std::vector<RawAddress> &devices) const
{
    DoInAdapterManagerThread([this, devices] {this->RemoveDeviceProfileConfig(devices);});
}

void AdapterManager::RemoveDeviceProfileConfig(const std::vector<RawAddress> &devices) const
{
    for (auto &device : devices) {
        ProfileConfig::GetInstance()->RemoveAddr(device.GetAddress());
    }
}

namespace {
template <typename T>
std::unique_ptr<AdapterInfo<T>> CreateAdapter(const std::string &adapterSection, const std::string &adapterName,
    utility::IContextCallback &callback)
{
    bool value = false;
    std::unique_ptr<AdapterInfo<T>> adapterInfo = nullptr;
    if (AdapterConfig::GetInstance()->GetValue(adapterSection, PROPERTY_IS_VALID, value) && value) {
        std::shared_ptr<T> adapter(ClassCreator<T>::NewInstance(adapterName));
        auto stateMachine = std::make_unique<AdapterStateMachine>();
        if (adapter && stateMachine) {
            adapter->GetContext()->Initialize();
            adapter->GetContext()->RegisterCallback(callback);
            stateMachine->Init(adapter->GetContext());
            adapterInfo = std::make_unique<AdapterInfo<T>>(adapter, std::move(stateMachine));
        } else {
            HILOGE("Create %{public}s failed", adapterName.c_str());
        }
    }
    return adapterInfo;
}
}  // namespace {}

void AdapterManager::CreateAdapters() const
{
    pimpl->classicAdapter_ = CreateAdapter<ClassicAdapter>(
        SECTION_CLASSIC_ADAPTER, ADAPTER_NAME_CLASSIC, *(pimpl->contextCallback_));
    pimpl->bleAdapter_ = CreateAdapter<BleAdapter>(
        SECTION_BLE_ADAPTER, ADAPTER_NAME_BLE, *(pimpl->contextCallback_));
}

int AdapterManager::GetMaxNumConnectedAudioDevices() const
{
    int value = 0;
    IAdapterConfig *config = AdapterConfig::GetInstance();

    if (!config->GetValue(SECTION_A2DP_SRC_SERVICE, PROPERTY_MAX_CONNECTED_DEVICES, value)) {
        HILOGE("%{public}s not found", SECTION_A2DP_SRC_SERVICE.c_str());
    }
    return value;
}

bool AdapterManager::SetPhonebookPermission(const std::string &address, BTPermissionType permission) const
{
    IProfileConfig *config = ProfileConfig::GetInstance();
    bool tmp = false;
    switch (permission) {
        case BTPermissionType::ACCESS_UNKNOWN:
            return config->RemoveProperty(address, SECTION_PERMISSION, PROPERTY_PHONEBOOK_PERMISSION);
        case BTPermissionType::ACCESS_ALLOWED:
            tmp = true;
            break;
        case BTPermissionType::ACCESS_FORBIDDEN:
            break;
        default:
            return false;
    }
    return config->SetValue(address, SECTION_PERMISSION, PROPERTY_PHONEBOOK_PERMISSION, tmp);
}

BTPermissionType AdapterManager::GetPhonebookPermission(const std::string &address) const
{
    IProfileConfig *config = ProfileConfig::GetInstance();
    bool value = false;

    if (!config->GetValue(address, SECTION_PERMISSION, PROPERTY_PHONEBOOK_PERMISSION, value)) {
        HILOGI("addr: %{public}s %{public}s not found", GetEncryptAddr(address).c_str(),
            PROPERTY_PHONEBOOK_PERMISSION);
        return BTPermissionType::ACCESS_UNKNOWN;
    }

    if (value) {
        return BTPermissionType::ACCESS_ALLOWED;
    } else {
        return BTPermissionType::ACCESS_FORBIDDEN;
    }
}

bool AdapterManager::SetMessagePermission(const std::string &address, BTPermissionType permission) const
{
    IProfileConfig *config = ProfileConfig::GetInstance();
    bool tmp = false;

    switch (permission) {
        case BTPermissionType::ACCESS_UNKNOWN:
            return config->RemoveProperty(address, SECTION_PERMISSION, PROPERTY_MESSAGE_PERMISSION);
        case BTPermissionType::ACCESS_ALLOWED:
            tmp = true;
            break;
        case BTPermissionType::ACCESS_FORBIDDEN:
            break;
        default:
            return false;
    }
    return config->SetValue(address, SECTION_PERMISSION, PROPERTY_MESSAGE_PERMISSION, tmp);
}

BTPermissionType AdapterManager::GetMessagePermission(const std::string &address) const
{
    IProfileConfig *config = ProfileConfig::GetInstance();
    bool value = false;

    if (!config->GetValue(address, SECTION_PERMISSION, PROPERTY_MESSAGE_PERMISSION, value)) {
        HILOGI("%{public}s %{public}s not found", GetEncryptAddr(address).c_str(),
            PROPERTY_MESSAGE_PERMISSION);
        return BTPermissionType::ACCESS_UNKNOWN;
    }

    if (value) {
        return BTPermissionType::ACCESS_ALLOWED;
    } else {
        return BTPermissionType::ACCESS_FORBIDDEN;
    }
}

int AdapterManager::GetPowerMode(const std::string &address) const
{
    RawAddress addr = RawAddress(address);
    return static_cast<int>(IPowerManager::GetInstance()->GetPowerMode(addr));
}

void AdapterManager::UnloadBluetoothSaTimerCallback() const
{
    PromptUnloadBluetoothSystemAbility();
    StopUnloadBluetoothSaTimer();
}

void AdapterManager::StartUnloadBluetoothSaTimer() const
{
    std::lock_guard<std::mutex> lock(unloadSaTimeoutMutex_);
    UnloadSaTimeout_ = std::make_shared<utility::Timer>(
        std::bind(&bluetooth::AdapterManager::UnloadBluetoothSaTimerCallback, this));
    UnloadSaTimeout_->Start(TIMEOUT_UNLOAD_BLUETOOTH_SA);
}

void AdapterManager::StopUnloadBluetoothSaTimer() const
{
    std::lock_guard<std::mutex> lock(unloadSaTimeoutMutex_);
    if (UnloadSaTimeout_ != nullptr) {
        UnloadSaTimeout_->Stop();
    }
    UnloadSaTimeout_ = nullptr;
}

bool AdapterManager::IsUnloadBluetoothSaTimerStarted() const
{
    std::lock_guard<std::mutex> lock(unloadSaTimeoutMutex_);
    return UnloadSaTimeout_ != nullptr;
}

bool AdapterManager::UpdateBluetoothState(const BTTransport transport, const BTStateID state) const
{
    HILOGI(" Update state ,transport is %{public}d state is %{public}d", transport, state);
    if (state == STATE_TURN_ON && IsBetaVersion()) {
        int state = PreferencesManager::Get("CALL_LOG", 0, PreferencesManagerType::CALL_LOG);
        if (state == HISI_PCM_LOG_ENABLE) {
            EnableHisiPcm(true);
        }
    }
    if (transport == ADAPTER_BREDR && pimpl->classicAdapter_ && pimpl->classicAdapter_->state.load() != state) {
        pimpl->classicAdapter_->state = state;
        return true;
    }
    if (transport == ADAPTER_BLE && pimpl->bleAdapter_ && pimpl->bleAdapter_->state.load() != state) {
        pimpl->bleAdapter_->state = state;
        return true;
    }
    if (GetState(ADAPTER_BLE) == STATE_TURNING_ON) {
        ThreadUtil::GetInstance().RemoveTask(THREAD_ID_ADAPTER_MANAGER, "EnableBluetoothHalfTaskWhenCollaborationOn");
    }
    return false;
}

bool AdapterManager::IsBetaVersion() const
{
    return system::GetParameter(BETA_VERSION_PROPERTY, "") == BETA_VERSION;
}

void AdapterManager::EnableHisiPcm(bool state) const
{
    const BthwifInterface *bthwif = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    CHECK_AND_RETURN_LOG(bthwif != nullptr, "bthwif is nullptr");
    bthwif->hwEnableHisiPcm(state);
}

BtInterface* AdapterManager::getBluetoothInterface() const
{
    return pimpl->bluetoothInterface;
}

// only used for TDD use case
void AdapterManager::setBluetoothInterface(BtInterface *interface) const
{
    pimpl->bluetoothInterface = interface;
}

void AdapterManager::PostAutoConnectTask(const int32_t delayMs) const
{
    if (AdapterManager::GetInstance()->GetQuietMode()) {
        HILOGI("quiet enableBt, no need to autoConnect");
        AdapterManager::GetInstance()->SetQuietMode(false);
        return;
    }
    if (!IsAutoConnectEnabledHook()) {
        HILOGI("Auto connect is disabled by product");
        return;
    }
    HITRACE_METER(BT_TRACE_TAG);
    HILOGI("delay %{public}d ms to auto connect", delayMs);
    auto taskFunc = []() {
        ConnectStrategyManager::GetInstance()->AutoConnect();
    };
    ThreadUtil::GetInstance().PostTask(
        THREAD_ID_ADAPTER_MANAGER, taskFunc, delayMs, "BluetoothProfileAutoConnection");
}

bool AdapterManager::IsFactoryReset() const
{
    const char *bluetoothFactoryResetParam = "persist.bluetooth.factoryreset";
    return system::GetBoolParameter(bluetoothFactoryResetParam, false);  // false is the default value
}

void AdapterManager::SetBluetoothRestrictedFlag(bool isBluetoothRestricted) const
{
    pimpl->isRestrictBluetooth = isBluetoothRestricted;
    if (isBluetoothRestricted) {
        SetParameter(g_bluetoothSwitchStatePropertyName, BLUETOOTH_SWITCH_STATE_HALF);
    } else {
        SetParameter(g_bluetoothSwitchStatePropertyName, g_bluetoothSwitchStateOn);
    }
    return;
}

void AdapterManager::SetBluetoothRestrictedFlagOnly(bool isBluetoothRestricted) const
{
    HILOGI("SetBluetoothRestrictedFlagOnly isBluetoothRestricted=%{public}d", isBluetoothRestricted);
    pimpl->isRestrictBluetooth = isBluetoothRestricted;
}

BTStateID AdapterManager::GetRestrictedState(BTTransport transport) const
{
    BTStateID state = BTStateID::STATE_TURN_OFF;
    if (transport == ADAPTER_BREDR && pimpl->classicAdapter_) {
        state = BTStateID::STATE_TURN_OFF;
    }
    if (transport == ADAPTER_BLE && pimpl->bleAdapter_) {
        if (PermissionManager::IsSystemHap() && pimpl->bleAdapter_->state.load() == BTStateID::STATE_TURN_ON) {
            state = BTStateID::STATE_TURN_ON;
        } else {
            state = BTStateID::STATE_TURN_OFF;
        }
    }
    return state;
}

bool AdapterManager::IsThirdpartyNeedBluetoothSwitchDialog() const
{
    if ((GetRestrictedState(ADAPTER_BLE) == BTStateID::STATE_TURN_OFF) && !PermissionManager::IsSystemHap() &&
        IsBluetoothRestricted()) {
        HILOGI("Thirdparty RequestBluetoothSwitchDialog");
        DialogSwitch::RequestBluetoothSwitchDialog(ENABLE_BLUETOOTH);
        return true;
    }
    return false;
}

int32_t AdapterManager::EnablebluetoothFromRestricted(
    std::string callingName, bool isAsync, bool isUserTriggered) const
{
    if (IsThirdpartyNeedBluetoothSwitchDialog()) {
        pimpl->ThirdpartyAppBeginOperateBluetoothSwitch(callingName);
        return dialogResult_->HandleBluetoothSwitchDialogResult(isAsync);
    }
    if (GetState(BTTransport::ADAPTER_BREDR) != BTStateID::STATE_TURN_ON) {
        HILOGW("bluetooth stack is not enable complete, just clear restricted flag");
        SetBluetoothRestrictedFlag(false);
        return BT_NO_ERROR;
    }

    callingName = pimpl->AttemptReplaceThirdpartyAppName(callingName);
    HILOGI("Enable Bluetooth from ble on, set RestrictBluetooth -> false");
    SetBluetoothRestrictedFlag(false);
    BtChrUeManager::GetInstance()->WriteBtSwitchChangeUe(
        ADAPTER_BREDR, UE_COMMON_SCENE_CASE0, UE_COMMON_SCENE_CASE0, callingName, 0);
    // Broadcast br on
    BluetoothHelper::BluetoothCommonEventHelper::PublishBluetoothStateChangeEvent(STATE_TURN_ON, ADAPTER_BREDR);
    NotifyAdapterStateChange(pimpl->adapterObservers_, ADAPTER_BREDR, STATE_TURN_ON);
    NotifyAdapterStateChangeV2(pimpl->adapterObservers_, BluetoothSwitchState::STATE_ON);
    pimpl->classicAdapter_->instance->SetBtScanMode(BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE, BT_SCAN_DURATION_MS);
    // Enable stack hid service
    pimpl->classicAdapter_->instance->ResumeBleStackDevice();
    PostAutoConnectTask(AUTO_CONNECT_DELAY_MS);
    BluetoothConnectionManager::GetInstance()->CheckNeedReportConnectedDevice();

    // 上报一次蓝牙打开成功事件（半开->全开）
    DoInAdapterManagerThread([callingName]() {
        BtChrWriteSwitchEvent(ChrSwitchEvent(callingName, EVENT_TYPE_BT_ENABLE_FROM_HALF_TO_ON));
    });
    return BT_NO_ERROR;
}

int32_t AdapterManager::SatelliteControl(int type, int state) const
{
    return pimpl->satelliteManager_->SatelliteControl(type, state);
}

bool AdapterManager::IsSupportVirtualAutoConnect(const std::string &address) const
{
    return BluetoothAudioManager::GetInstance().GetSharedPreVirtualAutoConnSwitchValue(address);
}

int AdapterManager::SetVirtualAutoConnectType(const std::string &address, int connType, int businessType) const
{
    RawAddress device(address);
    if (connType <= 0 || connType > CONN_REASON_MANUAL_VIRTUAL_CONNECT_PREEMPT_FLAG) {
        HILOGE("auto connect error connType:%{public}d", connType);
        return Bluetooth::BT_ERR_INVALID_PARAM;
    }
    BluetoothConnectionManager::GetInstance()->SetVirtualAutoConnectType(device, connType, businessType);
    return Bluetooth::BT_NO_ERROR;
}

bool AdapterManager::IsBluetoothRestricted() const
{
    return pimpl->isRestrictBluetooth.load();
}

bool AdapterManager::IsAdpaterNullptrWhenEnable(BTTransport transport) const
{
    std::string callingName = PermissionManager::GetCallingName();
    if ((transport == ADAPTER_BREDR && pimpl->classicAdapter_ == nullptr) ||
        (transport == ADAPTER_BLE && pimpl->bleAdapter_ == nullptr)) {
        HILOGE("[ADAPTER_MANAGER]BTTransport not register");
        BtChrUeManager::GetInstance()->WriteBtSwitchChangeUe(transport, UE_COMMON_SCENE_CASE0, UE_COMMON_SCENE_CASE2,
            callingName, 0);
        return true;
    }
    return false;
}

bool AdapterManager::IsAdpaterNullptrWhenDisable(BTTransport transport) const
{
    std::string callingName = PermissionManager::GetCallingName();
    if ((transport == ADAPTER_BREDR && pimpl->classicAdapter_ == nullptr) ||
        (transport == ADAPTER_BLE && pimpl->bleAdapter_ == nullptr)) {
        HILOGE("[ADAPTER_MANAGER]BTTransport not register");
        BtChrUeManager::GetInstance()->WriteBtSwitchChangeUe(transport, UE_COMMON_SCENE_CASE1, UE_COMMON_SCENE_CASE1,
            callingName, 0);
        return true;
    }
    return false;
}

void AdapterManager::PromptUnloadBluetoothSystemAbility() const
{
    HITRACE_METER(BT_TRACE_TAG);
    RemoveParameterWatcher(COLLABORATION_SERVICE_PARAM, nullptr, nullptr);

    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE("get samgr failed");
        return;
    }
    int32_t ret = samgrProxy->UnloadSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);
    if (ret != ERR_OK) {
        HILOGE("unload system ability failed");
        return;
    }
}

void AdapterManager::SetQuietMode(bool isQuietMode) const
{
    pimpl->quietMode_ = isQuietMode;
}

bool AdapterManager::GetQuietMode() const
{
    return pimpl->quietMode_.load();
}

void AdapterManager::SetApplicationDisableBluetoothFlag(bool isAppCloseBt)
{
    pimpl->isAppCloseBt_ = isAppCloseBt;
}

int32_t AdapterManager::UpdateRefusePolicy(const int32_t protocolType,
    const int32_t pid, const int64_t prohibitedSecondsTime) const
{
#ifdef BLUETOOTH_KIA_ENABLE
    int32_t ret = BluetoothRefusePolicyManager::GetInstance().UpdateRefusePolicy(protocolType,
        pid, prohibitedSecondsTime);
    if (protocolType == REFUSE_PROTOCOL_TYPE_SPP && ret == BT_NO_ERROR) {
        NotifyAdapterRefusePolicyChange(pimpl->adapterObservers_, pid, prohibitedSecondsTime);
    }
    return ret;
#else
    return BT_NO_ERROR;
#endif
}

int32_t AdapterManager::EnableBluetoothToRestrictMode(std::string callingName,
    bool isUserTriggered)
{
    if (system::GetBoolParameter("persist.edm.force_enable_bluetooth", false)) {
        HILOGI("Forcibly enable Bluetooth.");
        return BT_ERR_INTERNAL_ERROR;
    }
    pimpl->WaitAdapterManagerInitializeComplete();
    int32_t ret = Enable(ADAPTER_BLE, false, callingName, isUserTriggered);
    if (ret == BT_NO_ERROR) {
        SetBluetoothRestrictedFlag(true);
    }
    return ret;
}

void AdapterManager::SaveConnectionTime(const RawAddress &device)
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    int64_t currentTime = static_cast<int64_t>(tv.tv_sec);
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter) {
        classicAdapter->SetConnectionTime(device, currentTime);
        HILOGI("AclStateChangedCb connectTime %{public}ld", currentTime);
    }
}

void AdapterManager::ResetCloudBondingState(const RawAddress &device)
{
    if (RemoteDeviceProperties::GetInstance()->GetCloudBondState(device) == CLOUD_BOND_BONDING) {
        HILOGI("[CLOUD_PAIR] acl disconnected change to cloud bond none");
        RemoteDeviceProperties::GetInstance()->SetCloudBondState(device, CLOUD_BOND_NONE);
        auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
        if (classicAdapter != nullptr) {
            classicAdapter->NotifyDisconnectState(device);
        }
        BtChrUeManager::GetInstance()->HwChrReportFinishCloudPair(device,
            static_cast<int>(UeCloudPairSceneCode::CONNECT_DOWN_CLOUD_DEV),
            static_cast<int>(UeConnCloudPairSubSceneCode::ACL_CONN_TIMEOUT));
    }
}

bool AdapterManager::StartPairIfInCloudCreateBondState(const RawAddress &device)
{
    if (RemoteDeviceProperties::GetInstance()->GetCloudBondState(device) != CLOUD_BOND_CREATE_BOND) {
        return false;
    }
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter == nullptr) {
        HILOGE("classicAdapter is nullptr.");
        return false;
    }
    HILOGI("[CLOUD_PAIR] %{public}s acl disconnected and create bond", GetEncryptAddr(device.GetAddress()).c_str());
    RemoteDeviceProperties::GetInstance()->SetCloudBondState(device, CLOUD_BOND_BONDED);
    classicAdapter->StartCrediblePair(BT_TRANSPORT_BREDR, device);
    return true;
}

void AdapterManager::HandleCloudBondWhenAclStateChange(const RawAddress &device, int connectionState)
{
    if (connectionState != CONNECTION_STATE_DISCONNECTED ||
        !RemoteDeviceProperties::GetInstance()->IsCloudDevice(device)) {
        return;
    }
    ResetCloudBondingState(device);
    if (StartPairIfInCloudCreateBondState(device)) {
        return;
    }
}

int AdapterManager::HandleAclStateChanged(std::shared_ptr<BluetoothDevice> remoteDevice, const RawAddress &device,
    OHOS::bluetooth::RawAddress *remoteBdAddr, BtAclState state)
{
    int connectState = CONNECTION_STATE_CONNECTED;
    if (remoteDevice == nullptr || remoteBdAddr == nullptr) {
        return connectState;
    }
    if (state == BT_ACL_STATE_CONNECTED) {
        connectState = CONNECTION_STATE_CONNECTED;
        BtChrUpdateDeviceInfo(device.GetAddress(),
            remoteDevice->GetRemoteName(),
            remoteDevice->GetRssi(),
            remoteDevice->GetDeviceClass());
        RemoteDeviceProperties::GetInstance()->GetRemoteDeviceProperty(
            *remoteBdAddr, static_cast<BtPropertyType>(BT_PROPERTY_REMOTE_VERSION_INFO));
        SaveConnectionTime(device);
        CloudDeviceManager::GetInstance()->StopBtAclTimer(device);
    } else {
        connectState = CONNECTION_STATE_DISCONNECTED;
        remoteDevice->SetIsConnectFromLocal(false);
        BluetoothAudioManager::GetInstance().RemoveDisconnectDevice(device.GetAddress());
    }
    remoteDevice->SetAclConnectState(connectState);
    return connectState;
}

void AdapterManager::RegisterVirtualDeviceIdManagerFunc(
    std::function<int32_t(const RawAddress &realAddr, RawAddress &randomAddr)> func)
{
    virtualDeviceIdManageFunc_ = func;
}

int32_t AdapterManager::GetDeviceRandomAddr(const RawAddress &realAddr, RawAddress &randomAddr)
{
    if (virtualDeviceIdManageFunc_) {
        return virtualDeviceIdManageFunc_(realAddr, randomAddr);
    }
    return BT_ERR_INTERNAL_ERROR;
}

int32_t AdapterManager::SetTmpAdvName(const uint32_t tokenId, const std::string &name)
{
    tmpAdvNameMap_.EnsureInsert(tokenId, name);
    return BT_NO_ERROR;
}

std::string AdapterManager::GetTmpAdvName(const uint32_t tokenId)
{
    std::string name = "";
    tmpAdvNameMap_.Find(tokenId, name);
    return name;
}

void AdapterManager::EraseTmpAdvName(const uint32_t tokenId)
{
    tmpAdvNameMap_.Erase(tokenId);
}

#ifdef FEATURE_MOUSE_FAST_CONNECTION
void AdapterManager::SetNeedReconnectActiveDevice() const
{
    PowerMgrHelper::GetInstance()->SetNeedReconnectActiveDevice();
}
#endif
}  // namespace bluetooth
}  // namespace OHOS
