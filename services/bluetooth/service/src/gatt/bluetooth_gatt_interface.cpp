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
#define LOG_TAG "bt_service_gatt"
#endif

#include "bluetooth_gatt_interface.h"
#include "bt_def.h"
#include "hal_util.h"
#include "hisysevent.h"
#include "log.h"
#include "common_util.h"
#include "bt_hwif.h"
#include "util/safe_vector.h"
#include "thread_util.h"
#ifdef RES_SCHED_SUPPORT
#include "res_type.h"
#include "res_sched_client.h"
#endif

namespace OHOS {
namespace bluetooth {
#define WPTR_CBACK_GATT_THREAD_SERVER_OBSERVER(cbWptr, func, ...)                      \
do {                                                                                   \
    DoInGattThread([cbWptr, __VA_ARGS__]() {                                           \
        std::shared_ptr<GattServerObserver> cbSptr = (cbWptr).lock();                  \
        if (cbSptr) {                                                                  \
            cbSptr->func(__VA_ARGS__);                                                 \
        } else {                                                                       \
            BluetoothGattInterface::GetInstance()->ClearGattServerInValidObserver();   \
        }});                                                                           \
} while (0)

#define WPTR_CBACK_GATT_THREAD_CLIENT_OBSERVER(cbWptr, func, ...)                      \
do {                                                                                   \
    DoInGattThread([cbWptr, __VA_ARGS__]() {                                           \
        std::shared_ptr<GattClientObserver> cbSptr = (cbWptr).lock();                  \
        if (cbSptr) {                                                                  \
            cbSptr->func(__VA_ARGS__);                                                 \
        } else {                                                                       \
            BluetoothGattInterface::GetInstance()->ClearGattClientInValidObserver();   \
        }});                                                                           \
} while (0)

#define WPTR_CBACK_GATT_SERVER_OBSERVER(cbWptr, func, ...)                             \
do {                                                                                   \
    std::shared_ptr<GattServerObserver> cbSptr = (cbWptr).lock();                      \
    if (cbSptr) {                                                                      \
        cbSptr->func(__VA_ARGS__);                                                     \
    } else {                                                                           \
        DoInGattThread(std::bind([]() {                                                \
            BluetoothGattInterface::GetInstance()->ClearGattServerInValidObserver();}));      \
    }                                                                                  \
} while (0)

#define WPTR_CBACK_GATT_CLIENT_OBSERVER(cbWptr, func, ...)                             \
do {                                                                                   \
    std::shared_ptr<GattClientObserver> cbSptr = (cbWptr).lock();                      \
    if (cbSptr) {                                                                      \
        cbSptr->func(__VA_ARGS__);                                                     \
    } else {                                                                           \
        DoInGattThread(std::bind([]() {                                                \
            BluetoothGattInterface::GetInstance()->ClearGattClientInValidObserver();}));      \
    }                                                                                  \
} while (0)

using ScannerObserver = BluetoothGattInterface::ScannerObserver;
using GattServerObserver = BluetoothGattInterface::GattServerObserver;
using GattClientObserver = BluetoothGattInterface::GattClientObserver;

// BaseObserverList is thread safety
static std::unique_ptr<BaseObserverList<ScannerObserver>> g_scannerObservers = nullptr;
static utility::SafeVector<std::weak_ptr<GattServerObserver>> g_gattServerObservers;
static utility::SafeVector<std::weak_ptr<GattClientObserver>> g_gattClientObservers;

BluetoothGattInterface::BluetoothGattInterface(void) : gattInterface_(nullptr)
{
    g_scannerObservers = std::make_unique<BaseObserverList<ScannerObserver>>();
}

BluetoothGattInterface* BluetoothGattInterface::GetInstance(void)
{
    static BluetoothGattInterface singleton;
    return &singleton;
}

void BluetoothGattInterface::AddScannerObserver(ScannerObserver &observer)
{
    if (g_scannerObservers) {
        g_scannerObservers->Register(observer);
    }
}

void BluetoothGattInterface::RemoveScannerObserver(ScannerObserver &observer)
{
    if (g_scannerObservers) {
        g_scannerObservers->Deregister(observer);
    }
}

void BluetoothGattInterface::AddGattServerObserver(std::shared_ptr<GattServerObserver> observer)
{
    g_gattServerObservers.Push(observer);
}

void BluetoothGattInterface::RemoveGattServerObserver(std::shared_ptr<GattServerObserver> observer)
{
    g_gattServerObservers.EraseIf([observer](std::weak_ptr<GattServerObserver> &serverObs) {
        std::shared_ptr<GattServerObserver> sptr = serverObs.lock();
        return sptr == observer;
    });
}

void BluetoothGattInterface::ClearGattServerInValidObserver()
{
    g_gattServerObservers.EraseIf([](std::weak_ptr<GattServerObserver> &serverObs) {
        return serverObs.lock() == nullptr;
    });
}

void BluetoothGattInterface::AddGattClientObserver(std::shared_ptr<GattClientObserver> observer)
{
    g_gattClientObservers.Push(observer);
}

void BluetoothGattInterface::RemoveGattClientObserver(std::shared_ptr<GattClientObserver> observer)
{
    g_gattClientObservers.EraseIf([observer](std::weak_ptr<GattClientObserver> &clientObs) {
        std::shared_ptr<GattClientObserver> sptr = clientObs.lock();
        return sptr == observer;
    });
}

void BluetoothGattInterface::ClearGattClientInValidObserver()
{
    g_gattClientObservers.EraseIf([](std::weak_ptr<GattClientObserver> &clientObs) {
        return clientObs.lock() == nullptr;
    });
}

static void ScanResultCallback(
    uint16_t bleEvtType, uint8_t addrType, STACK::RawAddress *bda,
    uint8_t blePrimaryPhy, uint8_t bleSecondaryPhy,
    uint8_t bleAdvertisingSid, int8_t bleTxPower, int8_t rssi,
    uint16_t blePeriodicAdvInt,
    std::vector<uint8_t> advData)
{
    if (g_scannerObservers) {
        g_scannerObservers->ForEach([bleEvtType, addrType, bda, rssi, advData](ScannerObserver &observer) {
            observer.ScanResultCallback(bleEvtType, addrType, bda, rssi, advData);
        });
    }
}

static void BatchScanReportCallback(int clientIf, int status, int reportFormat, int numRecords,
    std::vector<uint8_t> data)
{
    if (g_scannerObservers) {
        g_scannerObservers->ForEach([clientIf, status, reportFormat, numRecords, data](ScannerObserver &observer) {
            observer.BatchScanReportCallback(clientIf, status, reportFormat, numRecords, data);
        });
    }
}

static void BatchScanThresholdCallback(int clientIf)
{
    // BT chip actively request to report results when storage reaches threshold
    if (g_scannerObservers) {
        g_scannerObservers->ForEach([clientIf](ScannerObserver &observer) {
            observer.BatchScanThresholdCallback(clientIf);
        });
    }
}

static void TrackAdvEventCallback(btgatt_track_adv_info_t *advTrackInfo)
{
    std::vector<uint8_t> advData{};
    if (advTrackInfo->adv_pkt_len > 0) {
        advData.insert(advData.begin(), advTrackInfo->p_adv_pkt_data,
            advTrackInfo->p_adv_pkt_data + advTrackInfo->adv_pkt_len);
    }
    if (advTrackInfo->scan_rsp_len > 0) {
        advData.insert(advData.begin() + advTrackInfo->adv_pkt_len, advTrackInfo->p_scan_rsp_data,
            advTrackInfo->p_scan_rsp_data + advTrackInfo->scan_rsp_len);
    }

    TrackAdvBaseInfo info{
        advTrackInfo->client_if,
        advTrackInfo->advertiser_state,
        advTrackInfo->addr_type,
        advTrackInfo->rssi_value,
    };

    if (g_scannerObservers) {
        g_scannerObservers->ForEach(
            [info, addr = advTrackInfo->bd_addr, advData](ScannerObserver &observer) {
            observer.TrackAdvFoundLostCallback(info, addr, advData);
        });
    }
}

static void RegisterServerCallback(int status, int serverIf, const Uuid &appUuid)
{
    g_gattServerObservers.ForEach([status, serverIf, appUuid](std::weak_ptr<GattServerObserver> observer) {
        WPTR_CBACK_GATT_SERVER_OBSERVER(observer, RegisterServerCallback, status, serverIf, appUuid);
    });
}

static void ReportDataToRss(const std::string &address, int state, const std::string &role, int connectif, int status)
{
#ifdef RES_SCHED_SUPPORT
    std::unordered_map<std::string, std::string> payload;
    payload["ADDRESS"] = address;
    payload["STATE"] = std::to_string(state);
    payload["ROLE"] = role;
    payload["CONNECTIF"] = std::to_string(connectif);
    payload["STATUS"] = std::to_string(status);
    ResourceSchedule::ResSchedClient::GetInstance().ReportData(
        OHOS::ResourceSchedule::ResType::RES_TYPE_BT_SERVICE_EVENT,
        OHOS::ResourceSchedule::ResType::BtServiceEvent::GATT_CONNECT_STATE,
        payload);
#endif
}

static void ConnectionCallback(int connId, int serverIf, int connected, const STACK::RawAddress &bda, int reason)
{
    HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::BT_SERVICE, "GATT_CONNECT_STATE",
        OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC,  "ADDRESS", GetEncryptAddr(bda.ToString()),
        "STATE", connected, "ROLE", "server", "CONNECTIF", serverIf,
        "STATUS", -1);
    ReportDataToRss(GetEncryptAddr(bda.ToString()), connected, "server", serverIf, -1);
    g_gattServerObservers.ForEach(
        [connId, serverIf, connected, bda, reason](std::weak_ptr<GattServerObserver> observer) {
            WPTR_CBACK_GATT_THREAD_SERVER_OBSERVER(
                observer, ConnectionCallback, connId, serverIf, connected, bda, reason);
        });
}

static void ServiceAddedCallback(int status, int serverIf, std::vector<btgatt_db_element_t> service)
{
    g_gattServerObservers.ForEach([status, serverIf, service](std::weak_ptr<GattServerObserver> observer) {
        WPTR_CBACK_GATT_THREAD_SERVER_OBSERVER(observer, ServiceAddedCallback, status, serverIf, service);
    });
}

static void ServiceStoppedCallback(int status, int serverIf, int serviceHandle)
{
    g_gattServerObservers.ForEach([status, serverIf, serviceHandle](std::weak_ptr<GattServerObserver> observer) {
        WPTR_CBACK_GATT_THREAD_SERVER_OBSERVER(observer, ServiceStoppedCallback, status, serverIf, serviceHandle);
    });
}

static void ServiceDeletedCallback(int status, int serverIf, int serviceHandle)
{
    g_gattServerObservers.ForEach([status, serverIf, serviceHandle](std::weak_ptr<GattServerObserver> observer) {
        WPTR_CBACK_GATT_THREAD_SERVER_OBSERVER(observer, ServiceDeletedCallback, status, serverIf, serviceHandle);
    });
}

static void RequestReadCharacteristicCallback(int connId, int transId, const STACK::RawAddress &bda, int attrHandle,
    int offset, bool isLong)
{
    g_gattServerObservers.ForEach(
        [connId, transId, bda, attrHandle, offset, isLong](std::weak_ptr<GattServerObserver> observer) {
            WPTR_CBACK_GATT_THREAD_SERVER_OBSERVER(observer, RequestReadCharacteristicCallback,
                connId, transId, bda, attrHandle, offset, isLong);
    });
}

static void RequestReadDescriptorCallback(int connId, int transId, const STACK::RawAddress &bda, int attrHandle,
    int offset, bool isLong)
{
    g_gattServerObservers.ForEach(
        [connId, transId, bda, attrHandle, offset, isLong](std::weak_ptr<GattServerObserver> observer) {
            WPTR_CBACK_GATT_THREAD_SERVER_OBSERVER(observer, RequestReadDescriptorCallback,
                connId, transId, bda, attrHandle, offset, isLong);
    });
}

static void RequestWriteCharacteristicCallback(int connId, int transId, const STACK::RawAddress &bda,
    int attrHandle, int offset, bool needRsp, bool isPrep, std::vector<uint8_t> value)
{
    g_gattServerObservers.ForEach(
        [connId, transId, bda, attrHandle, offset, needRsp, isPrep, value](std::weak_ptr<GattServerObserver> obs) {
            WPTR_CBACK_GATT_THREAD_SERVER_OBSERVER(obs, RequestWriteCharacteristicCallback,
                connId, transId, bda, attrHandle, offset, needRsp, isPrep, value);
    });
}

static void RequestWriteDescriptorCallback(int connId, int transId, const STACK::RawAddress &bda, int attrHandle,
    int offset, bool needRsp, bool isPrep, std::vector<uint8_t> value)
{
    g_gattServerObservers.ForEach(
        [connId, transId, bda, attrHandle, offset, needRsp, isPrep, value](std::weak_ptr<GattServerObserver> obs) {
            WPTR_CBACK_GATT_THREAD_SERVER_OBSERVER(obs, RequestWriteDescriptorCallback,
                connId, transId, bda, attrHandle, offset, needRsp, isPrep, value);
    });
}

static void RequestExecWriteCallback(int connId, int transId, const STACK::RawAddress &bda, int execWrite)
{
    g_gattServerObservers.ForEach([connId, transId, bda, execWrite](std::weak_ptr<GattServerObserver> observer) {
        WPTR_CBACK_GATT_THREAD_SERVER_OBSERVER(observer, RequestExecWriteCallback,
                connId, transId, bda, execWrite);
    });
}

static void ResponseConfirmationCallback(int status, int handle)
{
    g_gattServerObservers.ForEach([status, handle](std::weak_ptr<GattServerObserver> observer) {
        WPTR_CBACK_GATT_THREAD_SERVER_OBSERVER(observer, ResponseConfirmationCallback, status, handle);
    });
}

static void IndicationSentCallback(int connId, int status)
{
    g_gattServerObservers.ForEach([connId, status](std::weak_ptr<GattServerObserver> observer) {
        WPTR_CBACK_GATT_THREAD_SERVER_OBSERVER(observer, IndicationSentCallback, connId, status);
    });
}

static void CongestionCallback(int connId, bool congested)
{
    g_gattServerObservers.ForEach([connId, congested](std::weak_ptr<GattServerObserver> observer) {
        WPTR_CBACK_GATT_THREAD_SERVER_OBSERVER(observer, CongestionCallback, connId, congested);
    });
}

static void MtuChangedCallback(int connId, int mtu)
{
    g_gattServerObservers.ForEach([connId, mtu](std::weak_ptr<GattServerObserver> observer) {
        WPTR_CBACK_GATT_THREAD_SERVER_OBSERVER(observer, MtuChangedCallback, connId, mtu);
    });
}

static void PhyUpdatedCallback(int connId, uint8_t txPhy, uint8_t rxPhy, uint8_t status)
{
    g_gattServerObservers.ForEach([connId, txPhy, rxPhy, status](std::weak_ptr<GattServerObserver> observer) {
        WPTR_CBACK_GATT_THREAD_SERVER_OBSERVER(observer, PhyUpdatedCallback, connId, txPhy, rxPhy, status);
    });
}

static void ServerConnUpdatedCallback(int connId, uint16_t interval, uint16_t latency, uint16_t timeout, uint8_t status)
{
    g_gattServerObservers.ForEach(
        [connId, interval, latency, timeout, status](std::weak_ptr<GattServerObserver> observer) {
            WPTR_CBACK_GATT_THREAD_SERVER_OBSERVER(observer,
                ConnUpdatedCallback, connId, interval, latency, timeout, status);
        });
}

static void RegisterClientCallback(int status, int clientIf, const Uuid &appUuid)
{
    g_gattClientObservers.ForEach([status, clientIf, appUuid](std::weak_ptr<GattClientObserver> observer) {
        WPTR_CBACK_GATT_THREAD_CLIENT_OBSERVER(observer, RegisterClientCallback, status, clientIf, appUuid);
    });
}

static void ConnectCallback(int connId, int status, int clientIf, const STACK::RawAddress &bda)
{
    HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::BT_SERVICE, "GATT_CONNECT_STATE",
        OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC,  "ADDRESS", GetEncryptAddr(bda.ToString()),
        "STATE", static_cast<int>(BTConnectState::CONNECTED), "ROLE", "client", "CONNECTIF", clientIf,
        "STATUS", status);
    ReportDataToRss(GetEncryptAddr(bda.ToString()), static_cast<int>(BTConnectState::CONNECTED),
        "client", clientIf, status);
    g_gattClientObservers.ForEach([connId, status, clientIf, bda](std::weak_ptr<GattClientObserver> observer) {
        WPTR_CBACK_GATT_THREAD_CLIENT_OBSERVER(observer, ConnectCallback, connId, status, clientIf, bda);
    });
}

static void DisconnectCallback(int connId, int status, int clientIf, const STACK::RawAddress &bda, int reason)
{
    HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::BT_SERVICE, "GATT_CONNECT_STATE",
        OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC,  "ADDRESS", GetEncryptAddr(bda.ToString()),
        "STATE", static_cast<int>(BTConnectState::DISCONNECTED), "ROLE", "client", "CONNECTIF", clientIf,
        "STATUS", status);
    ReportDataToRss(GetEncryptAddr(bda.ToString()), static_cast<int>(BTConnectState::DISCONNECTED),
        "client", clientIf, status);
    g_gattClientObservers.ForEach([connId, status, clientIf, bda, reason](std::weak_ptr<GattClientObserver> observer) {
        WPTR_CBACK_GATT_THREAD_CLIENT_OBSERVER(observer, DisconnectCallback, connId, status, clientIf, bda, reason);
    });
}

static void CancelOpenCallback(int connId, int status, int clientIf, const STACK::RawAddress &bda)
{
    g_gattClientObservers.ForEach([connId, status, clientIf, bda](std::weak_ptr<GattClientObserver> observer) {
        WPTR_CBACK_GATT_THREAD_CLIENT_OBSERVER(observer, CancelOpenCallback, connId, status, clientIf, bda);
    });
}

static void SearchCompleteCallback(int connId, int status)
{
    g_gattClientObservers.ForEach(
        [connId, status](std::weak_ptr<GattClientObserver> observer) {
            WPTR_CBACK_GATT_THREAD_CLIENT_OBSERVER(observer, SearchCompleteCallback, connId, status);
        });
}

static void RegisterForNotificationCallback(int connId, int registered, int status, uint16_t handle)
{
    g_gattClientObservers.ForEach(
        [connId, registered, status, handle](std::weak_ptr<GattClientObserver> observer) {
            WPTR_CBACK_GATT_THREAD_CLIENT_OBSERVER(observer, RegisterForNotificationCallback,
                connId, registered, status, handle);
        });
}

static void NotifyCallback(int connId, const btgatt_notify_params_t &data)
{
    g_gattClientObservers.ForEach([connId, data](std::weak_ptr<GattClientObserver> observer) {
        WPTR_CBACK_GATT_CLIENT_OBSERVER(observer, NotifyCallback, connId, data);
    });
}

static void ReadCharacteristicCallback(int connId, int status, btgatt_read_params_t *pData)
{
    g_gattClientObservers.ForEach([connId, status, pData](std::weak_ptr<GattClientObserver> observer) {
        WPTR_CBACK_GATT_CLIENT_OBSERVER(observer, ReadCharacteristicCallback, connId, status, pData);
    });
}

static void WriteCharacteristicCallback(int connId, int status, uint16_t handle, const btgatt_rsp_params_t &rspContext)
{
    g_gattClientObservers.ForEach([connId, status, handle, rspContext](std::weak_ptr<GattClientObserver> observer) {
        WPTR_CBACK_GATT_THREAD_CLIENT_OBSERVER(observer, WriteCharacteristicCallback, connId, status, handle,
            rspContext);
    });
}

static void ReadDescriptorCallback(int connId, int status, const btgatt_read_params_t &data)
{
    g_gattClientObservers.ForEach([connId, status, data](std::weak_ptr<GattClientObserver> observer) {
        WPTR_CBACK_GATT_CLIENT_OBSERVER(observer, ReadDescriptorCallback, connId, status, data);
    });
}

static void WriteDescriptorCallback(int connId, int status, uint16_t handle)
{
    g_gattClientObservers.ForEach([connId, status, handle](std::weak_ptr<GattClientObserver> observer) {
        WPTR_CBACK_GATT_THREAD_CLIENT_OBSERVER(observer, WriteDescriptorCallback, connId, status, handle);
    });
}

static void ExecuteWriteCallback(int connId, int status)
{
    g_gattClientObservers.ForEach([connId, status](std::weak_ptr<GattClientObserver> observer) {
        WPTR_CBACK_GATT_THREAD_CLIENT_OBSERVER(observer, ExecuteWriteCallback, connId, status);
    });
}

static void ConfigureMtuCallback(int connId, int status, int mtu)
{
    g_gattClientObservers.ForEach([connId, status, mtu](std::weak_ptr<GattClientObserver> observer) {
        WPTR_CBACK_GATT_THREAD_CLIENT_OBSERVER(observer, ConfigureMtuCallback, connId, status, mtu);
    });
}

static void ReadRemoteRssiValueCallback(int clientIf, const STACK::RawAddress &bda, int rssi, int status)
{
    g_gattClientObservers.ForEach([clientIf, bda, rssi, status](std::weak_ptr<GattClientObserver> observer) {
        WPTR_CBACK_GATT_THREAD_CLIENT_OBSERVER(observer, ReadRemoteRssiValueCallback, clientIf, bda, rssi, status);
    });
}

static void GetGattDbCallback(int connId, const btgatt_db_element_t *db, int count)
{
    g_gattClientObservers.ForEach([connId, db, count](std::weak_ptr<GattClientObserver> observer) {
        WPTR_CBACK_GATT_CLIENT_OBSERVER(observer, GetGattDbCallback, connId, db, count);
    });
}

static void ServicesRemovedCallback(int connId, uint16_t startHandle, uint16_t endHandle)
{
    g_gattClientObservers.ForEach([connId, startHandle, endHandle](std::weak_ptr<GattClientObserver> observer) {
        WPTR_CBACK_GATT_THREAD_CLIENT_OBSERVER(observer, ServicesRemovedCallback, connId, startHandle, endHandle);
    });
}

static void ServicesAddedCallback(int connId, const btgatt_db_element_t &added, int addedCount)
{
    g_gattClientObservers.ForEach([connId, added, addedCount](std::weak_ptr<GattClientObserver> observer) {
        WPTR_CBACK_GATT_CLIENT_OBSERVER(observer, ServicesAddedCallback, connId, added, addedCount);
    });
}

static void ClientConnUpdatedCallback(int connId, uint16_t interval, uint16_t latency, uint16_t timeout, uint8_t status)
{
    g_gattClientObservers.ForEach(
        [connId, interval, latency, timeout, status](std::weak_ptr<GattClientObserver> observer) {
            WPTR_CBACK_GATT_THREAD_CLIENT_OBSERVER(observer, ConnUpdatedCallback,
                connId, interval, latency, timeout, status);
        });
}

static void ServicesChangedCallback(int connId)
{
    g_gattClientObservers.ForEach(
        [connId](std::weak_ptr<GattClientObserver> observer) {
            WPTR_CBACK_GATT_THREAD_CLIENT_OBSERVER(observer, ServicesChangedCallback, connId);
        });
}

static void ClientPhyUpdatedCallback(int connId, uint8_t txPhy, uint8_t rxPhy, uint8_t status)
{
    g_gattClientObservers.ForEach(
        [connId, txPhy, rxPhy, status](std::weak_ptr<GattClientObserver> observer) {
            WPTR_CBACK_GATT_THREAD_CLIENT_OBSERVER(observer, PhyUpdatedCallback,
                connId, txPhy, rxPhy, status);
        });
}

static const btgatt_scanner_callbacks_t g_gattScannerCallbacks = {
    ScanResultCallback,
    BatchScanReportCallback,  // batchscan_reports_cb
    BatchScanThresholdCallback,  // batchscan_threshold_cb
    TrackAdvEventCallback,  // track_adv_event_cb
};

static const btgatt_server_callbacks_t g_gattServerCallbacks = {
    RegisterServerCallback,
    ConnectionCallback,
    ServiceAddedCallback,
    ServiceStoppedCallback,
    ServiceDeletedCallback,
    RequestReadCharacteristicCallback,
    RequestReadDescriptorCallback,
    RequestWriteCharacteristicCallback,
    RequestWriteDescriptorCallback,
    RequestExecWriteCallback,
    ResponseConfirmationCallback,
    IndicationSentCallback,
    CongestionCallback,
    MtuChangedCallback,
    PhyUpdatedCallback,
    ServerConnUpdatedCallback,
};

static const btgatt_client_callbacks_t g_gattClientCallbacks = {
    RegisterClientCallback,
    ConnectCallback,
    DisconnectCallback,
    CancelOpenCallback,
    SearchCompleteCallback,
    RegisterForNotificationCallback,
    NotifyCallback,
    ReadCharacteristicCallback,
    WriteCharacteristicCallback,
    ReadDescriptorCallback,
    WriteDescriptorCallback,
    ExecuteWriteCallback,
    ReadRemoteRssiValueCallback,  // read_remote_rssi_cb
    ConfigureMtuCallback,
    nullptr,  // congestion_cb
    GetGattDbCallback,
    ServicesRemovedCallback,
    ServicesAddedCallback,
    ClientPhyUpdatedCallback,  // phy_updated_cb
    ClientConnUpdatedCallback,
    ServicesChangedCallback,  // service_changed_cb
};

static const btgatt_callbacks_t g_GattCallbacks = {
    sizeof(btgatt_callbacks_t), &g_gattClientCallbacks, &g_gattServerCallbacks,
    &g_gattScannerCallbacks,
};

bool BluetoothGattInterface::Initialize(const btgatt_interface_t *gattInterface)
{
    if (!gattInterface) {
        HILOGE("gattInterface is nullptr");
        return false;
    }
    bt_status_t status = gattInterface->init(&g_GattCallbacks);
    if (status != BT_STATUS_SUCCESS) {
        HILOGE("Failed to init gatt interface, (%{public}s)", bt_status_text(status).c_str());
        return false;
    }

    gattInterface_ = gattInterface;
    return true;
}

}  // namespace bluetooth
}  // namespace OHOS
