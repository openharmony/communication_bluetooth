/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "pbap_pce_sdp.h"
#include <cstring>
#include <memory>
#include "btcommon/message.h"
#include "pbap_pce_header_msg.h"
#include "pbap_pce_app_params.h"
#include "interface_profile.h"
#include "bt_def.h"
#include "log.h"

#ifndef LOG_TAG
#define LOG_TAG "bt_pbap_pce_sdp"
#endif

namespace OHOS {
namespace bluetooth {

PbapPceSdp::PbapPceSdp(SdpResponseCallback callback) : callback_(std::move(callback))
{}

PbapPceSdp::~PbapPceSdp()
{}

int PbapPceSdp::Register()
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    PbapPceSdpCreateParam param = {
        .name = PCE_SERVICE_NAME.c_str(),
        .l2capPsm = PBAP_PCE_LOCAL_GOEP_L2CAP_PSM,
        .version = PBAP_PCE_VERSION_NUMBER,
    };
    sdpHandle_ = SdpAdapter::GetInstance().CreatePbapPceSdpRecord(param);
    if (sdpHandle_ < 0) {
        PBAP_PCE_LOG_ERROR("CreatePbapPceSdpRecord failed");
        return sdpHandle_;
    }
    PBAP_PCE_LOG_INFO("%{public}s end, handle=%{public}d", __PRETTY_FUNCTION__, sdpHandle_);
    return 0;
}

int PbapPceSdp::Deregister() const
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    bool ret = SdpAdapter::GetInstance().RemoveSdpRecord(sdpHandle_);
    if (!ret) {
        PBAP_PCE_LOG_ERROR("RemoveSdpRecord failed");
        return -1;
    }
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
    return 0;
}

int PbapPceSdp::SdpSearch(const std::string &address)
{
    PBAP_PCE_LOG_INFO("%{public}s start, addr=%{public}s", __PRETTY_FUNCTION__,
        GetEncryptAddr(address).c_str());

    SdpAdapter::GetInstance().SetPbapPceSdpFoundCallback(
        [this](const PbapPceSdpFoundRecord &record) {
            OnSdpFound(record);
        });

    bool ret = SdpAdapter::GetInstance().StartRemoteSdpSearch(address, BLUETOOTH_UUID_PBAP_PSE);
    if (!ret) {
        PBAP_PCE_LOG_ERROR("StartRemoteSdpSearch failed");
        return -1;
    }
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
    return 0;
}

void PbapPceSdp::OnSdpFound(const PbapPceSdpFoundRecord &record)
{
    PBAP_PCE_LOG_INFO("%{public}s start, addr=%{public}s, status=%{public}d", __PRETTY_FUNCTION__,
        GetEncryptAddr(record.deviceAddr).c_str(), record.status);
    if (record.status != 0) {
        PBAP_PCE_LOG_ERROR("SDP search failed, status=%{public}d", record.status);
        utility::Message msg(PCE_SDP_FAILED);
        if (callback_) callback_(record.deviceAddr, msg);
        return;
    }

    int32_t rfcommChannel = record.rfcommChannel;
    int32_t l2capPsm = record.l2capPsm;
    uint16_t versionNumber = static_cast<uint16_t>(record.version);
    uint8_t supportedRes = static_cast<uint8_t>(record.supportedRepositories);
    uint32_t supportedFeature = static_cast<uint32_t>(record.features);
    bool featureFlag = (record.features != 0);

    PBAP_PCE_LOG_INFO("%{public}s rfcomm=%{public}d, l2capPsm=%{public}d, version=%{public}u, "
        "supportedRes=%{public}u, supportedFeature=%{public}u", __PRETTY_FUNCTION__,
        rfcommChannel, l2capPsm, versionNumber, supportedRes, supportedFeature);

    bool isGoepL2capPSM = false;
#ifdef PBAP_PCE_ENABLE_L2CAP_PSM
    isGoepL2capPSM = record.hasL2capPsm;
    if (isGoepL2capPSM) {
        PBAP_PCE_LOG_INFO("GOEP_L2CAP_PSM attribute present, l2capPsm=%{public}d", int(l2capPsm));
    } else {
        PBAP_PCE_LOG_INFO("GOEP_L2CAP_PSM attribute absent, falling back to rfcomm=%{public}d", int(rfcommChannel));
    }
#endif

    PbapPceObexConfig obexConfig;
    obexConfig.addr = record.deviceAddr;
    if (isGoepL2capPSM) {
        obexConfig.l2capPsm = static_cast<uint16_t>(l2capPsm);
        obexConfig.l2capLocalPsm = PBAP_PCE_LOCAL_GOEP_L2CAP_PSM;
        obexConfig.isL2capPSM = true;
        obexConfig.isSupportSrm = true;
        PBAP_PCE_LOG_INFO("connect with psm:[%{public}d]", int(l2capPsm));
    } else {
        obexConfig.rfCommChannel = static_cast<uint16_t>(rfcommChannel);
        PBAP_PCE_LOG_INFO("connect with rfcommNo:[%u]", static_cast<unsigned>(rfcommChannel));
    }
    obexConfig.serviceUUID = BLUETOOTH_UUID_PBAP_PSE;

    if (!featureFlag) {
        supportedFeature = PbapPceAppParams::PBAP_SUPPORTED_FEATURES;
    }

    if ((versionNumber != 0) && (supportedRes != 0) && (supportedFeature != 0) &&
        (l2capPsm != 0 || rfcommChannel != 0)) {
        auto sdpMsg = std::make_unique<PbapPceHeaderSdpMsg>(
            obexConfig, versionNumber, supportedRes, supportedFeature, featureFlag);
        void *rawPtr = static_cast<void *>(sdpMsg.release());
        utility::Message msg(PCE_SDP_FINISH, 0, rawPtr);
        PBAP_PCE_LOG_INFO("%{public}s PCE_SDP_FINISH created, arg2=%{public}p, addr=%{public}s, invoking callback",
            __PRETTY_FUNCTION__, rawPtr, GetEncryptAddr(record.deviceAddr).c_str());
        if (callback_) {
            callback_(record.deviceAddr, msg);
        } else {
            PBAP_PCE_LOG_ERROR("%{public}s callback_ is null! sdpMsg will leak, arg2=%{public}p", __PRETTY_FUNCTION__,
                rawPtr);
        }
    } else {
        PBAP_PCE_LOG_ERROR("%{public}s sdp failed, invalid params: version=%{public}u, supportedRes=%{public}u, "
            "supportedFeature=%{public}u, l2capPsm=%{public}d, rfcomm=%{public}d", __PRETTY_FUNCTION__,
            versionNumber, supportedRes, supportedFeature, l2capPsm, rfcommChannel);
        utility::Message msg(PCE_SDP_FAILED);
        if (callback_) callback_(record.deviceAddr, msg);
    }
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
}
}  // namespace bluetooth
}  // namespace OHOS
