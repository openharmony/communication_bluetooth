/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "sdp_found_event_publish.h"
#include "bluetooth.h"
#include "service_util.h"
#include "bt_uuid.h"
#include "log.h"
#include "bluetooth_common_event_helper.h"

namespace OHOS {
namespace bluetooth {
static const std::string COMMON_EVENT_BLUETOOTH_MAPMAS_SDP_FOUND =
    "usual.event.bluetooth.sdp.MAPMAS_FOUND";
static const std::string COMMON_EVENT_BLUETOOTH_MAPMNS_SDP_FOUND =
    "usual.event.bluetooth.sdp.MAPMNS_FOUND";
static const std::string COMMON_EVENT_BLUETOOTH_PBAPPSE_SDP_FOUND =
    "usual.event.bluetooth.sdp.PBAPPSE_FOUND";
static const std::string COMMON_EVENT_BLUETOOTH_OPPOPS_SDP_FOUND =
    "usual.event.bluetooth.sdp.OPPOPS_FOUND";
static const std::string COMMON_EVENT_BLUETOOTH_SAP_SDP_FOUND =
    "usual.event.bluetooth.sdp.SAP_FOUND";
static const std::string COMMON_EVENT_BLUETOOTH_DIP_SDP_FOUND =
    "usual.event.bluetooth.sdp.DIP_FOUND";
static const std::string COMMON_EVENT_BLUETOOTH_COMMON_SDP_FOUND =
    "usual.event.bluetooth.sdp.COMMON_FOUND";

static const int MAX_RAW_DATA_BUF = 4096;
static const int MAX_FORMATS_LENGTH = 15;
static const int MAX_SERVICE_NAME_LEN = 21;

static const Uuid UUID_MAP_MAS = Uuid::ConvertFrom16Bits(0x1132);
static const Uuid UUID_MAP_MNS = Uuid::ConvertFrom16Bits(0x1133);
static const Uuid UUID_PBAP_PSE = Uuid::ConvertFrom16Bits(0x112F);
static const Uuid UUID_OBEX_OBJECT_PUSH = Uuid::ConvertFrom16Bits(0x1105);
static const Uuid UUID_SAP = Uuid::ConvertFrom16Bits(0x112D);
static const Uuid UUID_DIP = Uuid::ConvertFrom16Bits(0x1200);
static const std::unordered_map<std::string, std::function<bool(int32_t, const std::string &,
    const std::string &, bool, BluetoothSdpRecord *)>> g_handleMap = {
    {UUID_MAP_MAS.ToString(), SdpFoundEventPublishHelper::PublishMapMasSdpFoundEvent},
    {UUID_MAP_MNS.ToString(), SdpFoundEventPublishHelper::PublishMapMnsSdpFoundEvent},
    {UUID_PBAP_PSE.ToString(), SdpFoundEventPublishHelper::PublishPbapPseSdpFoundEvent},
    {UUID_OBEX_OBJECT_PUSH.ToString(), SdpFoundEventPublishHelper::PublishObexObjectPushSdpFoundEvent},
    {UUID_SAP.ToString(), SdpFoundEventPublishHelper::PublishSapSdpFoundEvent},
    {UUID_DIP.ToString(), SdpFoundEventPublishHelper::PublishDipSdpFoundEvent},
};

template<typename SdpContainer, typename SdpDataType>
static SdpContainer CopyDataFromStack(const SdpDataType* buffer, int len, int maxLen)
{
    SdpContainer results;
    if (buffer == nullptr || len <= 0) {
        return results;
    }
    int uLen = len > maxLen ? maxLen : len;
    results.resize(uLen);
    std::copy(buffer, buffer + uLen, results.begin());
    return results;
}

static void SetCommonParams(int32_t status, const std::string &address,
    const std::string &uuid, OHOS::AAFwk::Want& want)
{
    want.SetParam("status", static_cast<int>(status));
    want.SetParam("address", address);
    want.SetParam("uuid", uuid);
}

bool SdpFoundEventPublishHelper::PublishSdpFoundEvent(int32_t status, const std::string &address,
    const std::string &uuid, bool moreResults, BluetoothSdpRecord *record)
{
    if (record == nullptr) {
        HILOGE("record is error");
        return false;
    }
    auto it = g_handleMap.find(uuid);
    if (it != g_handleMap.end()) {
        return it->second(status, address, uuid, moreResults, record);
    } else {
        return PublishCommonSdpFoundEvent(status, address, uuid, moreResults, record);
    }
}

bool SdpFoundEventPublishHelper::PublishMapMasSdpFoundEvent(int32_t status, const std::string &address,
    const std::string &uuid, bool moreResults, BluetoothSdpRecord* record)
{
    HILOGD("PublishMapMasSdpFoundEvent");
    OHOS::AAFwk::Want want;
    want.SetAction(COMMON_EVENT_BLUETOOTH_MAPMAS_SDP_FOUND);
    SetCommonParams(status, address, uuid, want);
    want.SetParam("instanceId", static_cast<int>(record->mas.masInstanceId));
    want.SetParam("l2capPsm", static_cast<int>(record->mas.hdr.l2capPsm));
    want.SetParam("channel", static_cast<int>(record->mas.hdr.rfcommChannelNumber));
    want.SetParam("version", static_cast<int>(record->mas.hdr.profileVersion));
    want.SetParam("features", static_cast<int>(record->mas.supportedFeatures));
    want.SetParam("messageType", static_cast<int>(record->mas.supportedMessageTypes));
    want.SetParam("serviceName", CopyDataFromStack<std::string, char>(record->mas.hdr.serviceName,
        record->hdr.serviceNameLength, MAX_SERVICE_NAME_LEN));
    want.SetParam("moreResults", moreResults);
    return BluetoothHelper::BluetoothCommonEventHelper::PublishEventWithManagePermission(want, false);
}

bool SdpFoundEventPublishHelper::PublishMapMnsSdpFoundEvent(int32_t status, const std::string &address,
    const std::string &uuid, bool moreResults, BluetoothSdpRecord* record)
{
    HILOGD("PublishMapMnsSdpFoundEvent");
    OHOS::AAFwk::Want want;
    want.SetAction(COMMON_EVENT_BLUETOOTH_MAPMNS_SDP_FOUND);
    SetCommonParams(status, address, uuid, want);
    want.SetParam("l2capPsm", static_cast<int>(record->mns.hdr.l2capPsm));
    want.SetParam("channel", static_cast<int>(record->mns.hdr.rfcommChannelNumber));
    want.SetParam("version", static_cast<int>(record->mns.hdr.profileVersion));
    want.SetParam("features", static_cast<int>(record->mns.supportedFeatures));
    want.SetParam("serviceName", CopyDataFromStack<std::string, char>(record->mns.hdr.serviceName,
        record->hdr.serviceNameLength, MAX_SERVICE_NAME_LEN));
    want.SetParam("moreResults", moreResults);
    return BluetoothHelper::BluetoothCommonEventHelper::PublishEventWithManagePermission(want, false);
}

bool SdpFoundEventPublishHelper::PublishPbapPseSdpFoundEvent(int32_t status, const std::string &address,
    const std::string &uuid, bool moreResults, BluetoothSdpRecord* record)
{
    HILOGD("PublishPbapPseSdpFoundEvent");
    OHOS::AAFwk::Want want;
    want.SetAction(COMMON_EVENT_BLUETOOTH_PBAPPSE_SDP_FOUND);
    SetCommonParams(status, address, uuid, want);
    want.SetParam("l2capPsm", static_cast<int>(record->pse.hdr.l2capPsm));
    want.SetParam("channel", static_cast<int>(record->pse.hdr.rfcommChannelNumber));
    want.SetParam("version", static_cast<int>(record->pse.hdr.profileVersion));
    want.SetParam("features", static_cast<int>(record->pse.supportedFeatures));
    want.SetParam("repositories", static_cast<int>(record->pse.supportedRepositories));
    want.SetParam("serviceName", CopyDataFromStack<std::string, char>(record->pse.hdr.serviceName,
        record->hdr.serviceNameLength, MAX_SERVICE_NAME_LEN));
    want.SetParam("moreResults", moreResults);
    return BluetoothHelper::BluetoothCommonEventHelper::PublishEventWithManagePermission(want, false);
}

bool SdpFoundEventPublishHelper::PublishObexObjectPushSdpFoundEvent(int32_t status, const std::string &address,
    const std::string &uuid, bool moreResults, BluetoothSdpRecord* record)
{
    HILOGD("PublishObexObjectPushSdpFoundEvent");
    OHOS::AAFwk::Want want;
    want.SetAction(COMMON_EVENT_BLUETOOTH_OPPOPS_SDP_FOUND);
    SetCommonParams(status, address, uuid, want);
    want.SetParam("l2capPsm", static_cast<int>(record->ops.hdr.l2capPsm));
    want.SetParam("channel", static_cast<int>(record->ops.hdr.rfcommChannelNumber));
    want.SetParam("version", static_cast<int>(record->ops.hdr.profileVersion));
    want.SetParam("serviceName", CopyDataFromStack<std::string, char>(record->ops.hdr.serviceName,
        record->hdr.serviceNameLength, MAX_SERVICE_NAME_LEN));
    want.SetParam("formatsList", CopyDataFromStack<std::vector<char>, uint8_t>(
        record->ops.supportedFormatsList, record->ops.supportedFormatsListLen, MAX_FORMATS_LENGTH));
    want.SetParam("moreResults", moreResults);
    return BluetoothHelper::BluetoothCommonEventHelper::PublishEventWithManagePermission(want, false);
}

bool SdpFoundEventPublishHelper::PublishSapSdpFoundEvent(int32_t status, const std::string &address,
    const std::string &uuid, bool moreResults, BluetoothSdpRecord* record)
{
    HILOGD("PublishSapSdpFoundEvent");
    OHOS::AAFwk::Want want;
    want.SetAction(COMMON_EVENT_BLUETOOTH_SAP_SDP_FOUND);
    SetCommonParams(status, address, uuid, want);
    want.SetParam("channel", static_cast<int>(record->sap.hdr.rfcommChannelNumber));
    want.SetParam("version", static_cast<int>(record->sap.hdr.profileVersion));
    want.SetParam("serviceName", CopyDataFromStack<std::string, char>(record->sap.hdr.serviceName,
        record->hdr.serviceNameLength, MAX_SERVICE_NAME_LEN));
    want.SetParam("moreResults", moreResults);
    return BluetoothHelper::BluetoothCommonEventHelper::PublishEventWithManagePermission(want, false);
}

bool SdpFoundEventPublishHelper::PublishDipSdpFoundEvent(int32_t status, const std::string &address,
    const std::string &uuid, bool moreResults, BluetoothSdpRecord* record)
{
    HILOGD("PublishDipSdpFoundEvent");
    OHOS::AAFwk::Want want;
    want.SetAction(COMMON_EVENT_BLUETOOTH_DIP_SDP_FOUND);
    SetCommonParams(status, address, uuid, want);
    want.SetParam("specId", static_cast<int>(record->dip.specId));
    want.SetParam("vendor", static_cast<int>(record->dip.vendor));
    want.SetParam("vendorIdSource", static_cast<int>(record->dip.vendorIdSource));
    want.SetParam("product", static_cast<int>(record->dip.product));
    want.SetParam("version", static_cast<int>(record->dip.version));
    want.SetParam("primaryRecord", record->dip.primaryRecord);
    want.SetParam("moreResults", moreResults);
    return BluetoothHelper::BluetoothCommonEventHelper::PublishEventWithManagePermission(want, false);
}

bool SdpFoundEventPublishHelper::PublishCommonSdpFoundEvent(int32_t status, const std::string &address,
    const std::string &uuid, bool moreResults, BluetoothSdpRecord* record)
{
    HILOGD("PublishCommonSdpFoundEvent");
    OHOS::AAFwk::Want want;
    want.SetAction(COMMON_EVENT_BLUETOOTH_COMMON_SDP_FOUND);
    SetCommonParams(status, address, uuid, want);
    want.SetParam("recordDataSize", static_cast<int>(record->hdr.user1PtrLen));
    want.SetParam("recordData", CopyDataFromStack<std::vector<char>, uint8_t>(record->hdr.user1Ptr,
        record->hdr.user1PtrLen, MAX_RAW_DATA_BUF));
    return BluetoothHelper::BluetoothCommonEventHelper::PublishEventWithManagePermission(want, false);
}
}  // namespace bluetooth
}  // namespace OHOS