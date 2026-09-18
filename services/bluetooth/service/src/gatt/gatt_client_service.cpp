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
#define LOG_TAG "bt_service_gatt_client"
#endif

#include "gatt_client_service.h"

#include <algorithm>
#include <functional>
#include <future>
#include <list>
#include <map>
#include <mutex>
#include <set>

#include "bt_chr_ue_manager.h"
#include "bt_stack_config.h"
#include "bt_ble_interface.h"
#include "class_creator.h"
#include "gatt_client_application.h"
#include "gatt_service_base.h"
#include "interface_adapter_manager.h"
#include "bluetooth_connection_manager.h"
#include "stack_adapter.h"
#include "hw_bt_chr.h"
#include "hw_bt_hwif.h"
#include "log.h"
#include "semaphore_utils.h"
#include "thread_util.h"
#include "permission_manager.h"
#include "ipc_skeleton.h"
#include "bluetooth_errorcode.h"
#include "control_intercept_plugin.h"
#include "bluetooth_state_manager.h"

namespace OHOS {
namespace bluetooth {
static constexpr int GATT_CLIENT_WAIT_TIMEOUT = BLUETOOTH_GATT_CLIENT_TIMEOUT;
static constexpr uint32_t WAIT_TIMEOUT_MS = 1000; // 1000ms
class GattClientService::GattClientServiceRegisterObserver : public BluetoothGattInterface::GattClientObserver {
public:
    struct Context {
        RawAddress addr;
        uint8_t transport;
        bool isShared;
        std::shared_ptr<std::promise<int>> promise;
        std::weak_ptr<IGattClientCallback> callback;
    };

    using RegisterCallback = std::function<void(int status, int clientIf, const Context &context)>;

    explicit GattClientServiceRegisterObserver(const Context &context, RegisterCallback cb)
        : context_(context), callback_(cb)
    {}

    ~GattClientServiceRegisterObserver() override
    {}

    void RegisterClientCallback(int status, int clientIf, const Uuid &appUuid) override
    {
        DoInGattThread([status, clientIf, callback = this->callback_, context = std::ref(this->context_)]() {
            callback(status, clientIf, context);
        });
    }
    bool isRegsitedCallBackPromiseUsed = false;
private:
    Context context_;
    RegisterCallback callback_;
};

struct GattClientService::impl : public GattServiceBase {
    explicit impl(GattClientService &service);
    ~impl() override;

    void Connect(int appId, bool autoConnect, const std::string &pkgName, int uid, bool isAllowed);
    bool CheckRunningStateAndAppId(int appId);
    bool IsValidApplication(int appId)
    {
        std::lock_guard<std::mutex> lock(mutex);
        return appIds.find(appId) != appIds.end();
    }
    std::string GetAddressByAppId(int appId)
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (appIds.count(appId)) {
            return appIds[appId];
        }
        return "";
    }
    std::list<std::shared_ptr<GattClientApplication>>::iterator GetApplicationIter(int appId)
    {
        return std::find_if(clients.begin(), clients.end(),
            [appId](const std::shared_ptr<GattClientApplication> app) { return app->GetAppId() == appId; });
    }
    std::list<std::shared_ptr<GattClientApplication>>::iterator GetApplicationIterByAddr(RawAddress device)
    {
        return std::find_if(clients.begin(), clients.end(),
            [device](const std::shared_ptr<GattClientApplication> app) { return app->GetAddress() == device;});
    }

    void RegisterApplication(std::weak_ptr<IGattClientCallback> callback, const RawAddress &addr, uint8_t transport,
        std::shared_ptr<std::promise<int>> promise, bool isShared);
    void RegisterClientCallback(int status, int clientIf, const GattClientServiceRegisterObserver::Context &context);

    void WriteCharacteristic(int appId, uint16_t handle, std::vector<uint8_t> value);
    void WriteCharacteristicNoRespond(int appId, uint16_t handle, std::vector<uint8_t> value);

    // global mutex
    std::mutex mutex;
    // used in server and inner thread, Used to determine whether the application is valid.
    std::map<int, std::string> appIds;

    // !!! The following variables are only used in inner thread.
    // Client Application
    std::list<std::shared_ptr<GattClientApplication>> clients {};
    GattClientService &self;
    std::shared_ptr<GattClientServiceRegisterObserver> registerObserver = nullptr;
    const BtgattClientInterface *btIfGattClient = nullptr;
    // used in write no respond lock
    std::mutex writeMutex;
};

GattClientService::GattClientService()
    : utility::Context(PROFILE_NAME_GATT_CLIENT, "5.0.0"), pimpl(std::make_unique<GattClientService::impl>(*this))
{}

GattClientService::~GattClientService()
{}

GattClientService::impl::impl(GattClientService &service)
    : mutex(), appIds(), clients(), self(service), registerObserver(nullptr), btIfGattClient(&btgattClientInterface)
{
}

GattClientService::impl::~impl()
{
}

void GattClientService::impl::Connect(int appId, bool autoConnect, const std::string &pkgName, int uid, bool isAllowed)
{
    auto iter = GetApplicationIter(appId);
    if (iter != clients.end()) {
        (*iter)->KeepBleScan(pkgName, uid);
        BluetoothConnectionManager::GetInstance()->RecordBleAclConnect((*iter)->GetAddress(), isAllowed);
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_BLE_CLIENT_CONN,
            (*iter)->GetAddress().GetAddress(), (*iter)->GetAppId(), pkgName);
        (*iter)->Connect(autoConnect);
        BtChrBleConnData((*iter)->GetAddress().GetAddress(), CHR_BLE_GATT, pkgName);
        BtChrAddConnSceneInfo((*iter)->GetAddress().GetAddress(), PAIR_TYPE_USER_CONNECT, pkgName, 0);
    }
}

utility::Context *GattClientService::GetContext()
{
    return this;
}

std::list<RawAddress> GattClientService::GetConnectDevices()
{
    return std::list<RawAddress>();
}

int GattClientService::Connect(const RawAddress &device)
{
    return 0;
}

int GattClientService::Disconnect(const RawAddress &device)
{
    return 0;
}

int GattClientService::GetConnectState()
{
    auto connectState = BluetoothStateManager::GetInstance()->GetProfileConnectState(PROFILE_NAME_GATT_CLIENT);
    return (connectState == BTConnectState::CONNECTED ? PROFILE_STATE_CONNECTED : PROFILE_STATE_DISCONNECTED);
}

int GattClientService::GetMaxConnectNum()
{
    return 0;
}

void GattClientService::SetBtifInterface(const BtgattClientInterface *interface)
{
    pimpl->btIfGattClient = interface;
}

void GattClientService::Enable()
{
    if (pimpl->InRunningState()) {
        HILOGE("ProfileService: %{public}s is already start up!", Name().c_str());
        return;
    }

    DoInGattThread([this]() {
        pimpl->clients.clear();
        pimpl->Start();
        GetContext()->OnEnable(PROFILE_NAME_GATT_CLIENT, true);
    });
}

void GattClientService::Disable()
{
    if (!pimpl->InRunningState()) {
        HILOGE("ProfileService: %{public}s is already shut down!", Name().c_str());
        return;
    }

    DoInGattThread([this]() {
        pimpl->Stop();
        GetContext()->OnDisable(PROFILE_NAME_GATT_CLIENT, true);
        pimpl->registerObserver = nullptr;
        std::lock_guard<std::mutex> lock(pimpl->mutex);
        pimpl->appIds.clear();
    });
}

void GattClientService::impl::RegisterClientCallback(int status, int clientIf,
    const GattClientServiceRegisterObserver::Context &ctx)
{
    HILOGI("status: %{public}d, clientIf: %{public}d", status, clientIf);
    int ret = 0;
    if (registerObserver == nullptr || registerObserver->isRegsitedCallBackPromiseUsed) {
        btIfGattClient->unregisterClient(clientIf);
        HILOGI("this promise is used status: %{public}d, clientIf: %{public}d", status, clientIf);
        return;
    }
    if (status != GATT_SUCCESS) {
        ret = GattStatus::GATT_FAILURE;
    } else {
        // Remove apps that are not cleaned up when Bluetooth is turned off.
        for (auto it = clients.begin(); it != clients.end();) {
            if ((*it)->GetAppId() == clientIf) {
                HILOGE("App (%{public}d) is not cleared", clientIf);
                it = clients.erase(it);
            } else {
                ++it;
            }
        }

        ret = clientIf;
        // Register success.
        GattDevice device(ctx.addr, ctx.transport);
        std::shared_ptr<GattClientApplication> client = std::make_shared<GattClientApplication>(clientIf,
            btIfGattClient, device, ctx.callback);
        BluetoothGattInterface::GetInstance()->AddGattClientObserver(client);
        clients.emplace(clients.end(), client);

        std::lock_guard<std::mutex> lock(mutex);
        appIds[clientIf] = ctx.addr.GetAddress();
    }

    if (ctx.promise) {
        ctx.promise->set_value(ret);
        registerObserver->isRegsitedCallBackPromiseUsed = true;
    }
    registerObserver.reset();
}

void GattClientService::impl::RegisterApplication(std::weak_ptr<IGattClientCallback> callback,
    const RawAddress &addr, uint8_t transport, std::shared_ptr<std::promise<int>> promise, bool isShared)
{
    for (auto &client : clients) {
        if (!isShared && (client->GetAddress() == addr && client->GetTransport() == transport)) {
            HILOGE("Already registered");
            promise->set_value(GattStatus::GATT_FAILURE);
            return;
        }
    }

    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;
    // Create an observer, and only listen to "register_client_cb". Destory in the callback.
    GattClientServiceRegisterObserver::Context ctx = {addr, transport, isShared, promise, callback};
    auto observer = std::make_shared<GattClientServiceRegisterObserver>(ctx,
        [this](int status, int clientIf, const GattClientServiceRegisterObserver::Context &context) {
            this->RegisterClientCallback(status, clientIf, context);
        });
    if (btIfGattClient == nullptr || btIfGattClient->registerClient == nullptr) {
        // Open stack currently exports empty btgattClientInterface; avoid null call SEGV.
        HILOGE("GATT client interface unavailable");
        promise->set_value(GattStatus::REQUEST_NOT_SUPPORT);
        return;
    }

    BluetoothGattInterface::GetInstance()->AddGattClientObserver(observer);
    std::swap(registerObserver, observer);

    // not support eatt transport now
    int ret = btIfGattClient->registerClient(Uuid::Random(), false);
    if (ret != BT_STATUS_SUCCESS) {
        HILOGE("Register client failed, ret: %{public}d", ret);
        // Release and delete the observer.
        registerObserver.reset();
        promise->set_value(GattStatus::GATT_FAILURE);
        return;
    }
}

int GattClientService::RegisterApplication(std::weak_ptr<IGattClientCallback> callback,
    const RawAddress &addr, uint8_t transport)
{
    if (!pimpl->InRunningState()) {
        HILOGE("not start up");
        return GattStatus::REQUEST_NOT_SUPPORT;
    }

    auto promise = std::make_shared<std::promise<int>>();
    std::future<int> future = promise->get_future();

    DoInGattThread([this, callback, addr, transport, promise]() {
        pimpl->RegisterApplication(callback, addr, transport, promise, false);
    });

    auto status = future.wait_for(std::chrono::seconds(GATT_CLIENT_WAIT_TIMEOUT));
    if (status != std::future_status::ready) {
        HILOGE("timeout");
        return GattStatus::GATT_FAILURE;
    }
    return future.get();
}

int GattClientService::RegisterSharedApplication(std::weak_ptr<IGattClientCallback> callback, const RawAddress &addr,
    uint8_t transport)
{
    if (!pimpl->InRunningState()) {
        HILOGE("not start up");
        return GattStatus::REQUEST_NOT_SUPPORT;
    }

    auto promise = std::make_shared<std::promise<int>>();
    std::future<int> future = promise->get_future();

    DoInGattThread([this, callback, addr, transport, promise]() {
        pimpl->RegisterApplication(callback, addr, transport, promise, true);
    });
    auto status = future.wait_for(std::chrono::seconds(GATT_CLIENT_WAIT_TIMEOUT));
    if (status != std::future_status::ready) {
        HILOGE("timeout");
        return GattStatus::GATT_FAILURE;
    }
    return future.get();
}

bool GattClientService::impl::CheckRunningStateAndAppId(int appId)
{
    if (!InRunningState()) {
        HILOGE("not start up");
        return false;
    }
    if (!IsValidApplication(appId)) {
        HILOGE("Application is invalid, appId: %{public}d", appId);
        return false;
    }
    return true;
}

int GattClientService::DeregisterApplication(int appId)
{
    HILOGI("appId: %{public}d", appId);
    if (!pimpl->CheckRunningStateAndAppId(appId)) {
        return GattStatus::REQUEST_NOT_SUPPORT;
    }

    auto promise = std::make_shared<std::promise<int>>();
    auto future = promise->get_future();

    DoInGattThread([this, appId, promise]() {
        auto iter = pimpl->GetApplicationIter(appId);
        if (iter != pimpl->clients.end()) {
            pimpl->clients.erase(iter);
            std::lock_guard<std::mutex> lock(pimpl->mutex);
            pimpl->appIds.erase(appId);
        }

        promise->set_value(GattStatus::GATT_SUCCESS);
    });

    if (future.wait_for(std::chrono::seconds(GATT_CLIENT_WAIT_TIMEOUT)) != std::future_status::ready) {
        HILOGE("timeout");
        return GattStatus::GATT_FAILURE;
    }
    return future.get();
}

int GattClientService::Connect(int appId, bool autoConnect)
{
    HILOGI("appId: %{public}d, autoConnect: %{public}d", appId, autoConnect);
    if (!pimpl->CheckRunningStateAndAppId(appId)) {
        return GattStatus::REQUEST_NOT_SUPPORT;
    }
    ControlInterceptMessage msg {
        .addr = pimpl->GetAddressByAppId(appId),
        .isSystemHap = Bluetooth::PermissionManager::IsSystemHap(),
        .isNativeCaller = Bluetooth::PermissionManager::IsNativeCaller(),
        .pid = IPCSkeleton::GetCallingPid(),
        .uid = IPCSkeleton::GetCallingUid(),
    };
    if (!ControlInterceptIsAllowedGattConn(msg)) {
        return GattStatus::REQUEST_NOT_SUPPORT;
    }

    std::string callingname = Bluetooth::PermissionManager::GetCallingName();
    bool isAllowed = BluetoothConnectionManager::GetInstance()->IsNativeAppOrSystemHap();
    HILOGI("callingName(%{public}s); IsNativeApp : %{public}d", callingname.c_str(), isAllowed);
    if (IAdapterManager::GetInstance()->IsBluetoothRestricted()) {
        if (!isAllowed) {
            HILOGW("just allow system hap and native app connect gatt");
            return GattStatus::GATT_FAILURE;
        }
    }
    if (IAdapterManager::GetInstance()->IsBleOwnerOnlyMode() &&
        !IAdapterManager::GetInstance()->IsBleAccessible(IPCSkeleton::GetCallingPid())) {
        HILOGW("BLE owner only mode, refuse gatt connect from non-owner pid");
        return GattStatus::GATT_FAILURE;
    }

    std::string pkgName = Bluetooth::PermissionManager::GetCallingName();
    int uid = OHOS::IPCSkeleton::GetCallingUid();
    DoInGattThread([this, appId, autoConnect, pkgName, uid, isAllowed]() {
        pimpl->Connect(appId, autoConnect, pkgName, uid, isAllowed);
    });

    return GattStatus::GATT_SUCCESS;
}

int GattClientService::Disconnect(int appId)
{
    HILOGI("appId: %{public}d", appId);
    if (!pimpl->CheckRunningStateAndAppId(appId)) {
        return GattStatus::REQUEST_NOT_SUPPORT;
    }

    DoInGattThread([this, appId]() {
        auto iter = pimpl->GetApplicationIter(appId);
        if (iter != pimpl->clients.end()) {
            (*iter)->Disconnect();
        }
    });

    return GattStatus::GATT_SUCCESS;
}

int GattClientService::DiscoveryServices(int appId)
{
    HILOGI("appId: %{public}d", appId);
    if (!pimpl->CheckRunningStateAndAppId(appId)) {
        return GattStatus::REQUEST_NOT_SUPPORT;
    }

    DoInGattThread(
        [this, appId]() {
            auto iter = pimpl->GetApplicationIter(appId);
            if (iter != pimpl->clients.end()) {
                (*iter)->DiscoveryServices();
            }
        });

    return GattStatus::GATT_SUCCESS;
}

std::vector<Service> GattClientService::GetServices(int appId)
{
    HILOGI("appId: %{public}d", appId);
    if (!pimpl->CheckRunningStateAndAppId(appId)) {
        return std::vector<Service>();
    }

    auto promise = std::make_shared<std::promise<std::vector<Service>>>();
    std::future<std::vector<Service>> future = promise->get_future();

    DoInGattThread(
        [this, appId, promise]() {
            auto iter = pimpl->GetApplicationIter(appId);
            if (iter != pimpl->clients.end()) {
                auto v = (*iter)->GetServices();
                promise->set_value(std::move(v));
            }
        });

    auto status = future.wait_for(std::chrono::seconds(GATT_CLIENT_WAIT_TIMEOUT));
    if (status != std::future_status::ready) {
        HILOGE("timeout");
        return std::vector<Service>();
    }
    return future.get();
}

int GattClientService::ReadCharacteristic(int appId, const Characteristic &characteristic)
{
    HILOGI("appId: %{public}d, handle: %{public}#x", appId, characteristic.handle_);
    if (!pimpl->CheckRunningStateAndAppId(appId)) {
        return GattStatus::REQUEST_NOT_SUPPORT;
    }

    DoInGattThread(
        [this, appId, handle = characteristic.handle_]() {
            auto iter = pimpl->GetApplicationIter(appId);
            if (iter != pimpl->clients.end()) {
                (*iter)->ReadCharacteristic(handle);
            }
        });

    return GattStatus::GATT_SUCCESS;
}

int GattClientService::ReadCharacteristicByUuid(int appId, const Uuid &uuid, int32_t startHandle, int32_t endHandle)
{
    HILOGI("appId: %{public}d", appId);
    if (!pimpl->CheckRunningStateAndAppId(appId)) {
        return GattStatus::REQUEST_NOT_SUPPORT;
    }

    DoInGattThread(
        [this, appId, uuid, startHandle, endHandle]() {
            auto iter = pimpl->GetApplicationIter(appId);
            if (iter != pimpl->clients.end()) {
                (*iter)->ReadCharacteristicByUuid(uuid, startHandle, endHandle);
            }
        });

    return GattStatus::GATT_SUCCESS;
}

void GattClientService::impl::WriteCharacteristic(int appId, uint16_t handle, std::vector<uint8_t> value)
{
    DoInGattThread(
        [this, appId, handle, value = std::move(value)]() {
            auto iter = GetApplicationIter(appId);
            if (iter != clients.end()) {
                (*iter)->WriteCharacteristic(handle, std::move(value));
            }
        });
}

void GattClientService::impl::WriteCharacteristicNoRespond(int appId, uint16_t handle, std::vector<uint8_t> value)
{
    // It need lock because this function will be called in multi thread.
    std::lock_guard<std::mutex> lock(writeMutex);

    auto semaphore = std::make_shared<utility::Semaphore>(0);

    DoInGattThread(
        [this, appId, handle, semaphore, value = std::move(value)]() {
            auto iter = GetApplicationIter(appId);
            if (iter != clients.end()) {
                (*iter)->WriteCharacteristicNoRespond(handle, std::move(value), semaphore);
            }
        });

    if (!semaphore->WaitForMs(WAIT_TIMEOUT_MS)) {
        HILOGE("timeout");
    }
}

int GattClientService::WriteCharacteristic(int appId, Characteristic &characteristic, bool withoutRespond)
{
    HILOGD("appId: %{public}d, handle: %{public}#x, withoutRespond: %{public}d",
        appId, characteristic.handle_, withoutRespond);
    if (!pimpl->CheckRunningStateAndAppId(appId)) {
        return GattStatus::REQUEST_NOT_SUPPORT;
    }
    ControlInterceptMessage msg {
        .addr = pimpl->GetAddressByAppId(appId),
        .pid = IPCSkeleton::GetCallingPid(),
        .uid = IPCSkeleton::GetCallingUid(),
    };
    if (!ControlInterceptIsAllowedGattWrite(msg)) {
        return GattStatus::REQUEST_NOT_SUPPORT;
    }

    const uint8_t *p = characteristic.value_.get();
    std::vector<uint8_t> value(p, p + characteristic.length_);
    if (withoutRespond) {
        pimpl->WriteCharacteristicNoRespond(appId, characteristic.handle_, std::move(value));
    } else {
        pimpl->WriteCharacteristic(appId, characteristic.handle_, std::move(value));
    }

    return GattStatus::GATT_SUCCESS;
}

int GattClientService::SignedWriteCharacteristic(int appId, Characteristic &characteristic)
{
    HILOGE("Unsupported");
    return GattStatus::REQUEST_NOT_SUPPORT;
}

int GattClientService::ReadDescriptor(int appId, const Descriptor &descriptor)
{
    HILOGI("appId: %{public}d, handle: %{public}#x", appId, descriptor.handle_);
    if (!pimpl->CheckRunningStateAndAppId(appId)) {
        return GattStatus::REQUEST_NOT_SUPPORT;
    }

    DoInGattThread(
        [this, appId, handle = descriptor.handle_]() {
            auto iter = pimpl->GetApplicationIter(appId);
            if (iter != pimpl->clients.end()) {
                (*iter)->ReadDescriptor(handle);
            }
        });

    return GattStatus::GATT_SUCCESS;
}

int GattClientService::WriteDescriptor(int appId, Descriptor &descriptor)
{
    HILOGI("appId: %{public}d, handle: %{public}#x", appId, descriptor.handle_);
    if (!pimpl->CheckRunningStateAndAppId(appId)) {
        return GattStatus::REQUEST_NOT_SUPPORT;
    }
    ControlInterceptMessage msg {
        .addr = pimpl->GetAddressByAppId(appId),
        .pid = IPCSkeleton::GetCallingPid(),
        .uid = IPCSkeleton::GetCallingUid(),
    };
    if (!ControlInterceptIsAllowedGattWrite(msg)) {
        return GattStatus::REQUEST_NOT_SUPPORT;
    }

    const uint8_t *p = descriptor.value_.get();
    std::vector<uint8_t> value(p, p + descriptor.length_);
    DoInGattThread(
        [this, appId, handle = descriptor.handle_, value = std::move(value)]() {
            auto iter = pimpl->GetApplicationIter(appId);
            if (iter != pimpl->clients.end()) {
                (*iter)->WriteDescriptor(handle, value);
            }
        });

    return GattStatus::GATT_SUCCESS;
}

int GattClientService::RequestExchangeMtu(int appId, int mtu)
{
    HILOGD("appId: %{public}d, mtu: %{public}d", appId, mtu);
    if (!pimpl->CheckRunningStateAndAppId(appId)) {
        return GattStatus::REQUEST_NOT_SUPPORT;
    }

    DoInGattThread(
        [this, appId, mtu]() {
            auto iter = pimpl->GetApplicationIter(appId);
            if (iter != pimpl->clients.end()) {
                (*iter)->RequestExchangeMtu(mtu);
            }
        });

    return GattStatus::GATT_SUCCESS;
}

std::vector<GattDevice> GattClientService::GetAllDevice()
{
    auto promise = std::make_shared<std::promise<std::vector<GattDevice>>>();
    std::future<std::vector<GattDevice>> future = promise->get_future();

    DoInGattThread(
        [this, promise]() {
            std::set<GattDevice> set;

            for (auto iter = pimpl->clients.begin(); iter != pimpl->clients.end(); iter++) {
                uint8_t addrType;
                BtGetAddressType((*iter)->GetAddress(), &addrType);
                set.emplace((*iter)->GetAddress(), ServiceUtil::AddrTypeFromStack(addrType),
                    (*iter)->GetTransport(), (*iter)->GetConnState());
            }
            std::vector<GattDevice> v(set.begin(), set.end());
            promise->set_value(std::move(v));
        });

    auto status = future.wait_for(std::chrono::seconds(GATT_CLIENT_WAIT_TIMEOUT));
    if (status != std::future_status::ready) {
        HILOGE("timeout");
        return std::vector<GattDevice>();
    }

    return future.get();
}

int GattClientService::RequestConnectionPriority(int appId, int connPriority)
{
    HILOGI("appId: %{public}d, priority: %{public}d", appId, connPriority);
    if (!pimpl->CheckRunningStateAndAppId(appId)) {
        return Bluetooth::BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED;
    }

    DoInGattThread(
        [this, appId, connPriority]() {
            auto iter = pimpl->GetApplicationIter(appId);
            if (iter != pimpl->clients.end()) {
                (*iter)->RequestConnectionPriority(connPriority);
            }
        });

    return Bluetooth::BT_NO_ERROR;
}

int GattClientService::RequestFastestConn(const RawAddress &addr)
{
    HILOGI("add accelerated connection address");
    ControlInterceptMessage msg {
        .addr = addr.GetAddress(),
        .isSystemHap = Bluetooth::PermissionManager::IsSystemHap(),
        .isNativeCaller = Bluetooth::PermissionManager::IsNativeCaller(),
        .pid = IPCSkeleton::GetCallingPid(),
        .uid = IPCSkeleton::GetCallingUid(),
    };
    if (!ControlInterceptIsAllowedFastestGattConn(msg)) {
        return GATT_ERROR;
    }

    DoInGattThread([addr]() {
        const BtInterface *btInterface = nullptr;
        int ret = hal_util_load_bt_library(&btInterface);
        if (ret != BT_STATUS_SUCCESS || btInterface == nullptr) {
            HILOGE("Load bluetooth library failed");
            return;
        }

        const BthwifInterface *bthwif =
            reinterpret_cast<const BthwifInterface*>(btInterface->getProfileInterface(BT_VENDER_INTERFACE_ID));
        if (bthwif == nullptr) {
            HILOGE("bthwif is null");
            return;
        }
        bthwif->leConnectFastest(addr);
    });
    return GattStatus::GATT_SUCCESS;
}

int GattClientService::ReadRemoteRssiValue(int appId)
{
    HILOGI("appId: %{public}d", appId);
    if (!pimpl->CheckRunningStateAndAppId(appId)) {
        return GattStatus::REQUEST_NOT_SUPPORT;
    }
    DoInGattThread([this, appId]() {
        auto iter = pimpl->GetApplicationIter(appId);
        if (iter != pimpl->clients.end()) {
            (*iter)->ReadRemoteRssiValue(appId);
        }
    });

    return GattStatus::GATT_SUCCESS;
}

int GattClientService::RequestNotification(int32_t appId, uint16_t characterhandle, bool enable)
{
    if (!pimpl->CheckRunningStateAndAppId(appId)) {
        return GattStatus::REQUEST_NOT_SUPPORT;
    }
    DoInGattThread([this, appId, characterhandle, enable]() {
        auto iter = pimpl->GetApplicationIter(appId);
        if (iter != pimpl->clients.end()) {
            (*iter)->RequestNotification(characterhandle, enable);
        }
    });
    return GattStatus::GATT_SUCCESS;
}

int GattClientService::GetConnectedState(const RawAddress &address)
{
    const std::string deviceId = address.GetAddress();
    std::vector<std::string> serverDevices = BluetoothStateManager::GetInstance()->GetServerDeviceList();
    for (const std::string &serverDevice : serverDevices) {
        if (serverDevice == deviceId) {
            return static_cast<int>(BTConnectState::CONNECTED);
        }
    }
    return static_cast<int>(BTConnectState::DISCONNECTED);
}

int GattClientService::SetPhy(int32_t appId, int32_t txPhy, int32_t rxPhy, int32_t phyOptions)
{
    HILOGI("appId: %{public}d", appId);
    if (!pimpl->CheckRunningStateAndAppId(appId)) {
        return GattStatus::REQUEST_NOT_SUPPORT;
    }

    if (phyOptions < static_cast<int32_t>(BLE_PHY_CODED_NO_PREFERRED) ||
        phyOptions > static_cast<int32_t>(BLE_PHY_CODED_S8)) {
        HILOGE("Invalid parameters");
        return Bluetooth::BT_ERR_INVALID_PARAM;
    }
    if ((txPhy < static_cast<int32_t>(BLE_PHY_1M) || txPhy > static_cast<int32_t>(BLE_PHY_CODED)) ||
        (rxPhy < static_cast<int32_t>(BLE_PHY_1M) || rxPhy > static_cast<int32_t>(BLE_PHY_CODED))) {
        HILOGE("Invalid parameters");
        return Bluetooth::BT_ERR_INVALID_PARAM;
    }
    DoInGattThread([this, appId, txPhy, rxPhy, phyOptions]() {
        auto iter = pimpl->GetApplicationIter(appId);
        if (iter != pimpl->clients.end()) {
            (*iter)->SetPhy(txPhy, rxPhy, phyOptions);
        }
    });
    return GattStatus::GATT_SUCCESS;
}
 
int GattClientService::ReadPhy(int32_t appId)
{
    HILOGI("appId: %{public}d", appId);
    if (!pimpl->CheckRunningStateAndAppId(appId)) {
        return GattStatus::REQUEST_NOT_SUPPORT;
    }
    DoInGattThread([this, appId]() {
        auto iter = pimpl->GetApplicationIter(appId);
        if (iter != pimpl->clients.end()) {
            (*iter)->ReadPhy();
        }
    });
    return GattStatus::GATT_SUCCESS;
}

REGISTER_CLASS_CREATOR(GattClientService);
}  // namespace bluetooth
}  // namespace OHOS
