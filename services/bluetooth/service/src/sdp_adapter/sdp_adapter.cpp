/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_sdp_adapter"
#endif

#include "sdp_adapter.h"
#include "bluetooth.h"
#include "adapter_manager.h"
#include "log.h"
#include "bt_uuid.h"
#include "raw_address.h"
#include "sdp_found_event_publish.h"

namespace OHOS {
namespace bluetooth {
const Uuid UUID_MAP_MAS = Uuid::ConvertFrom16Bits(0x1132);
const Uuid UUID_MAP_MNS = Uuid::ConvertFrom16Bits(0x1133);
const Uuid UUID_OBEX_OBJECT_PUSH = Uuid::ConvertFrom16Bits(0x1105);
static BtRecursiveMutex sdpGetInstanceMutex_ {};

SdpAdapter &SdpAdapter::GetInstance()
{
    std::lock_guard<BtRecursiveMutex> lock(sdpGetInstanceMutex_);
    static SdpAdapter instance;
    return instance;
}
static void sdp_search_callback(BtStackStatus status, const STACK::RawAddress &bdAddr,
    const Uuid &uuid_in, int count, BluetoothSdpRecord *records)
{
    for (int i = 0; i < count || i == 0; i++) {
        // when status = 1 (BTA_SDP_FAILURE), records is defult(not null), count = 0.
        BluetoothSdpRecord *record = &records[i];
        SdpFoundEventPublishHelper::PublishSdpFoundEvent(static_cast<int32_t>(status), bdAddr.ToString(),
            uuid_in.ToString(), (i < (count - 1)) ? true : false, record);
        if (uuid_in.ToString() == UUID_MAP_MNS.ToString()) {
            if (SdpAdapter::GetInstance().GetMapMnsSdpFoundCallback() == nullptr) {
                return;
            }
            MapMnsSdpFoundRecord foundRecord{static_cast<int32_t>(status),
                record->mns.hdr.rfcommChannelNumber,
                record->mns.hdr.l2capPsm,
                record->mns.hdr.profileVersion,
                record->mns.supportedFeatures,
                bdAddr.ToString(),
                UUID_MAP_MNS.ToString(),
                record->hdr.serviceNameLength > 0 ? record->mas.hdr.serviceName : "",
                i < (count - 1) ? true : false};
            SdpAdapter::GetInstance().GetMapMnsSdpFoundCallback()(foundRecord);
        } else if (uuid_in.ToString() == UUID_OBEX_OBJECT_PUSH.ToString()) {
            if (SdpAdapter::GetInstance().GetOppSdpFoundCallback() == nullptr) {
                return;
            }
            OppSdpFoundRecord foundRecord{static_cast<int32_t>(status),
                record->ops.hdr.rfcommChannelNumber,
                record->ops.hdr.l2capPsm,
                record->ops.hdr.profileVersion,
                ServiceUtil::AddrFromStack(bdAddr).GetAddress(),
                UUID_OBEX_OBJECT_PUSH.ToString(),
                record->ops.hdr.serviceNameLength > 0 ? record->ops.hdr.serviceName : "",
                i < (count - 1) ? true : false};
            SdpAdapter::GetInstance().GetOppSdpFoundCallback()(foundRecord);
        }
    }
}
BtsdpCallbacks g_bluetoothSdpCallbacks = {sizeof(g_bluetoothSdpCallbacks), sdp_search_callback};

int32_t SdpAdapter::CreatePbapPseSdpRecord(const PbapPseSdpCreateParam &param)
{
    if (!bluetoothSdpInterface_) {
        return -1;
    }
    BluetoothSdpRecord record = {};
    record.pse.hdr.type = SDP_TYPE_PBAP_PSE;
    if (param.name != nullptr) {
        record.pse.hdr.serviceName = const_cast<char *>(param.name);
        record.pse.hdr.serviceNameLength = strlen(param.name);
    } else {
        record.pse.hdr.serviceName = nullptr;
        record.pse.hdr.serviceNameLength = 0;
    }
    record.pse.hdr.rfcommChannelNumber = param.channel;
    record.pse.hdr.l2capPsm = param.l2capPsm;
    record.pse.hdr.profileVersion = param.version;

    record.pse.supportedFeatures = param.features;
    record.pse.supportedRepositories = param.supportedRepositories;

    int32_t handle = -1;
    int32_t ret = bluetoothSdpInterface_->createSdpRecord(&record, &handle);
    if (ret != BT_STATUS_SUCCESS) {
        HILOGE("create_sdp_record error");
    }

    return handle;
}

int32_t SdpAdapter::CreateMapMasSdpRecord(const MapMasSdpCreateParam &param)
{
    if (!bluetoothSdpInterface_) {
        return -1;
    }
    BluetoothSdpRecord record = {};
    record.mas.hdr.type = SDP_TYPE_MAP_MAS;
    if (param.name != nullptr) {
        record.mas.hdr.serviceName = const_cast<char *>(param.name);
        record.mas.hdr.serviceNameLength = strlen(param.name);
    } else {
        record.mas.hdr.serviceName = nullptr;
        record.mas.hdr.serviceNameLength = 0;
    }
    record.mas.hdr.rfcommChannelNumber = param.channel;
    record.mas.hdr.l2capPsm = param.l2capPsm;
    record.mas.hdr.profileVersion = param.version;

    record.mas.masInstanceId = param.masId;
    record.mas.supportedFeatures = param.features;
    record.mas.supportedMessageTypes = param.msgTypes;

    int32_t handle = -1;
    int32_t ret = bluetoothSdpInterface_->createSdpRecord(&record, &handle);
    if (ret != BT_STATUS_SUCCESS) {
        HILOGE("create_sdp_record error");
    }

    return handle;
}

int32_t SdpAdapter::CreateOppSdpRecord(const OppSdpCreateParam &param)
{
    if (!bluetoothSdpInterface_) {
        return -1;
    }
    BluetoothSdpRecord record = {};
    record.ops.hdr.type = SDP_TYPE_OPP_SERVER;
    if (param.name != nullptr) {
        record.ops.hdr.serviceName = param.name;
        record.ops.hdr.serviceNameLength = strlen(param.name);
    } else {
        record.ops.hdr.serviceName = nullptr;
        record.ops.hdr.serviceNameLength = 0;
    }
    record.ops.hdr.rfcommChannelNumber = param.channel;
    record.ops.hdr.l2capPsm = param.l2capPsm;
    record.ops.hdr.profileVersion = param.version;

    int32_t handle = -1;
    int32_t ret = bluetoothSdpInterface_->createSdpRecord(&record, &handle);
    if (ret != BT_STATUS_SUCCESS) {
        HILOGE("create_sdp_record error");
    }
    return handle;
}

bool SdpAdapter::RemoveSdpRecord(int32_t handle)
{
    if (!bluetoothSdpInterface_) {
        return false;
    }

    int32_t ret = bluetoothSdpInterface_->removeSdpRecord(handle);
    if (ret != BT_STATUS_SUCCESS) {
        HILOGE("remove_sdp_record error");
        return false;
    }
    return true;
}

bool SdpAdapter::StartRemoteSdpSearch(const std::string &address, const std::string &uuid)
{
    if (!bluetoothSdpInterface_) {
        return false;
    }
    STACK::RawAddress rawAddress;
    STACK::RawAddress::FromString(address, rawAddress);
    int32_t ret = bluetoothSdpInterface_->sdpSearch(&rawAddress, Uuid::ConvertFromString(uuid));
    if (ret != BT_STATUS_SUCCESS) {
        HILOGE("sdp_search error");
        return false;
    }
    return true;
}

void SdpAdapter::SetMapMnsSdpFoundCallback(const MapMnsSdpFoundCallback &callback)
{
    mapMnsSdpFoundCallback_ = callback;
}

MapMnsSdpFoundCallback SdpAdapter::GetMapMnsSdpFoundCallback()
{
    return mapMnsSdpFoundCallback_;
}

void SdpAdapter::SetOppSdpFoundCallback(const OppSdpFoundCallback &callback)
{
    oppSdpFoundCallback_ = callback;
}

OppSdpFoundCallback SdpAdapter::GetOppSdpFoundCallback()
{
    return oppSdpFoundCallback_;
}

SdpAdapter::SdpAdapter()
{
    BtInterface *bluetoothInterface = AdapterManager::GetInstance()->getBluetoothInterface();
    if (bluetoothInterface == nullptr) {
        HILOGE("bluetoothInterface in nullptr");
        return;
    }
    bluetoothSdpInterface_ = static_cast<BtsdpInterface *>(
        const_cast<void *>(bluetoothInterface->getProfileInterface(BT_PROFILE_SDP_CLIENT_ID)));
    if (bluetoothSdpInterface_ != nullptr) {
        bluetoothSdpInterface_->init(&g_bluetoothSdpCallbacks);
    } else {
        HILOGE("bluetoothSdpInterface_ in nullptr");
    }
}
}  // namespace bluetooth
}  // namespace OHOS
