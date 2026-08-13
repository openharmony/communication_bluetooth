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
const BLUEDROID::bluetooth::Uuid UUID_MAP_MAS = BLUEDROID::bluetooth::Uuid::From16Bit(0x1132);
const BLUEDROID::bluetooth::Uuid UUID_MAP_MNS = BLUEDROID::bluetooth::Uuid::From16Bit(0x1133);
const BLUEDROID::bluetooth::Uuid UUID_OBEX_OBJECT_PUSH = BLUEDROID::bluetooth::Uuid::From16Bit(0x1105);
static BtRecursiveMutex sdpGetInstanceMutex_ {};

SdpAdapter &SdpAdapter::GetInstance()
{
    std::lock_guard<BtRecursiveMutex> lock(sdpGetInstanceMutex_);
    static SdpAdapter instance;
    return instance;
}
static void sdp_search_callback(bt_status_t status, const BLUEDROID::RawAddress &bd_addr,
    const BLUEDROID::bluetooth::Uuid &uuid_in, int count, bluetooth_sdp_record *records)
{
    for (int i = 0; i < count || i == 0; i++) {
        // when status = 1 (BTA_SDP_FAILURE), records is defult(not null), count = 0.
        bluetooth_sdp_record *record = &records[i];
        SdpFoundEventPublishHelper::PublishSdpFoundEvent(static_cast<int32_t>(status), bd_addr.ToString(),
            uuid_in.ToString(), (i < (count - 1)) ? true : false, record);
        if (uuid_in.ToString() == UUID_MAP_MNS.ToString()) {
            if (SdpAdapter::GetInstance().GetMapMnsSdpFoundCallback() == nullptr) {
                return;
            }
            MapMnsSdpFoundRecord foundRecord{static_cast<int32_t>(status),
                record->mns.hdr.rfcomm_channel_number,
                record->mns.hdr.l2cap_psm,
                record->mns.hdr.profile_version,
                record->mns.supported_features,
                bd_addr.ToString(),
                UUID_MAP_MNS.ToString(),
                record->hdr.service_name_length > 0 ? record->mas.hdr.service_name : "",
                i < (count - 1) ? true : false};
            SdpAdapter::GetInstance().GetMapMnsSdpFoundCallback()(foundRecord);
        } else if (uuid_in.ToString() == UUID_OBEX_OBJECT_PUSH.ToString()) {
            if (SdpAdapter::GetInstance().GetOppSdpFoundCallback() == nullptr) {
                return;
            }
            OppSdpFoundRecord foundRecord{static_cast<int32_t>(status),
                record->ops.hdr.rfcomm_channel_number,
                record->ops.hdr.l2cap_psm,
                record->ops.hdr.profile_version,
                ServiceUtil::AddrFromBluedroid(bd_addr).GetAddress(),
                UUID_OBEX_OBJECT_PUSH.ToString(),
                record->ops.hdr.service_name_length > 0 ? record->ops.hdr.service_name : "",
                i < (count - 1) ? true : false};
            SdpAdapter::GetInstance().GetOppSdpFoundCallback()(foundRecord);
        }
    }
}
btsdp_callbacks_t g_bluetoothSdpCallbacks = {sizeof(g_bluetoothSdpCallbacks), sdp_search_callback};

int32_t SdpAdapter::CreatePbapPseSdpRecord(const PbapPseSdpCreateParam &param)
{
    if (!bluetoothSdpInterface_) {
        return -1;
    }
    bluetooth_sdp_record record = {};
    record.pse.hdr.type = SDP_TYPE_PBAP_PSE;
    if (param.name != nullptr) {
        record.pse.hdr.service_name = const_cast<char *>(param.name);
        record.pse.hdr.service_name_length = strlen(param.name);
    } else {
        record.pse.hdr.service_name = nullptr;
        record.pse.hdr.service_name_length = 0;
    }
    record.pse.hdr.rfcomm_channel_number = param.channel;
    record.pse.hdr.l2cap_psm = param.l2capPsm;
    record.pse.hdr.profile_version = param.version;

    record.pse.supported_features = param.features;
    record.pse.supported_repositories = param.supportedRepositories;

    int32_t handle = -1;
    int32_t ret = bluetoothSdpInterface_->create_sdp_record(&record, &handle);
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
    bluetooth_sdp_record record = {};
    record.mas.hdr.type = SDP_TYPE_MAP_MAS;
    if (param.name != nullptr) {
        record.mas.hdr.service_name = const_cast<char *>(param.name);
        record.mas.hdr.service_name_length = strlen(param.name);
    } else {
        record.mas.hdr.service_name = nullptr;
        record.mas.hdr.service_name_length = 0;
    }
    record.mas.hdr.rfcomm_channel_number = param.channel;
    record.mas.hdr.l2cap_psm = param.l2capPsm;
    record.mas.hdr.profile_version = param.version;

    record.mas.mas_instance_id = param.masId;
    record.mas.supported_features = param.features;
    record.mas.supported_message_types = param.msgTypes;

    int32_t handle = -1;
    int32_t ret = bluetoothSdpInterface_->create_sdp_record(&record, &handle);
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
    bluetooth_sdp_record record = {};
    record.ops.hdr.type = SDP_TYPE_OPP_SERVER;
    if (param.name != nullptr) {
        record.ops.hdr.service_name = param.name;
        record.ops.hdr.service_name_length = strlen(param.name);
    } else {
        record.ops.hdr.service_name = nullptr;
        record.ops.hdr.service_name_length = 0;
    }
    record.ops.hdr.rfcomm_channel_number = param.channel;
    record.ops.hdr.l2cap_psm = param.l2capPsm;
    record.ops.hdr.profile_version = param.version;

    int32_t handle = -1;
    int32_t ret = bluetoothSdpInterface_->create_sdp_record(&record, &handle);
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

    int32_t ret = bluetoothSdpInterface_->remove_sdp_record(handle);
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
    BLUEDROID::RawAddress rawAddress;
    BLUEDROID::RawAddress::FromString(address, rawAddress);
    int32_t ret = bluetoothSdpInterface_->sdp_search(&rawAddress, BLUEDROID::bluetooth::Uuid::FromString(uuid));
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
    bt_interface_t *bluetoothInterface = AdapterManager::GetInstance()->getBluetoothInterface();
    if (bluetoothInterface == nullptr) {
        HILOGE("bluetoothInterface in nullptr");
        return;
    }
    bluetoothSdpInterface_ = static_cast<btsdp_interface_t *>(
        const_cast<void *>(bluetoothInterface->get_profile_interface(BT_PROFILE_SDP_CLIENT_ID)));
    if (bluetoothSdpInterface_ != nullptr) {
        bluetoothSdpInterface_->init(&g_bluetoothSdpCallbacks);
    } else {
        HILOGE("bluetoothSdpInterface_ in nullptr");
    }
}
}  // namespace bluetooth
}  // namespace OHOS