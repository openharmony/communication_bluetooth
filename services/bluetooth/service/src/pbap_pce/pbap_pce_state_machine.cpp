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
#ifndef LOG_TAG
#define LOG_TAG "bt_pbap_pce_state_machine"
#endif

#include "pbap_pce_state_machine.h"
#include <codecvt>
#include <fstream>
#include <locale>
#include <sstream>
#include <vector>
#include "pbap_pce_app_params.h"
#include "pbap_pce_connected_state.h"
#include "pbap_pce_connecting_state.h"
#include "pbap_pce_disconnected_state.h"
#include "pbap_pce_disconnecting_state.h"
#include "pbap_pce_service_impl.h"
#include "power_manager.h"
 #include "bt_def.h"

namespace OHOS {
namespace bluetooth {
PbapPceStateMachine::PbapPceStateMachine(const RawAddress &dev, PbapPceServiceImpl &pceService, PbapPceSdp &pbapPceSdp,
    std::shared_ptr<IPbapPceObserver> observer)
    : device_(dev),
      pceService_(&pceService),
      pbapPceSdp_(pbapPceSdp),
      observer_(std::move(observer)),
      targetState_(PCE_TARGET_STATE_UNKNOWN)
{
    LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    std::unique_ptr<State> disconnected =
        std::make_unique<PceDisconnectedState>(PCE_DISCONNECTED_STATE, *this, observer_);
    std::unique_ptr<State> connecting =
        std::make_unique<PceConnectingState>(PCE_CONNECTING_STATE, *this, observer_);
    std::unique_ptr<State> disconnecting =
        std::make_unique<PceDisconnectingState>(PCE_DISCONNECTING_STATE, *this, observer_);
    std::unique_ptr<State> connected = std::make_unique<PceConnectedState>(PCE_CONNECTED_STATE, *this, observer_);
    Move(disconnected);
    Move(connecting);
    Move(disconnecting);
    Move(connected);

    InitState(PCE_CONNECTING_STATE);
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
}

void PbapPceStateMachine::PullPhoneBook(const PbapPceAppParams &appParams)
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    
    if (pceObexClient_ == nullptr) {
        PBAP_PCE_LOG_ERROR("%{public}s end, pceObexClient_ is null", __PRETTY_FUNCTION__);
        return;
    }
    
    // 1. 创建 OBEX GET 请求头
    ObexHeader pbReqHeader;
    
    pbReqHeader.SetConnectionId(connectId_);
    pbReqHeader.SetType("x-bt/phonebook");
    pbReqHeader.SetName(appParams.GetName());
    
    std::u16string u16Name(appParams.GetName().begin(), appParams.GetName().end());
    SetReqVcardFileName(u16Name);
    
    appParams.AddToObexHeader(pbReqHeader);
    
    bool requestSize = (appParams.GetMaxListCount() == 0);
    
    SetPowerStatusBusy(true);
    
    pceObexClient_->Get(pbReqHeader, 
                        requestSize ? PCE_REQ_PULLPHONEBOOKSIZE : PCE_REQ_PULLPHONEBOOK);
    
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
}

int PbapPceStateMachine::GetTargetState() const
{
    return targetState_;
}

void PbapPceStateMachine::SetTargetState(int targetState)
{
    targetState_ = targetState;
}

void PbapPceStateMachine::TransitTargetState()
{
    PBAP_PCE_LOG_INFO("%{public}s start target=%{public}d", __PRETTY_FUNCTION__, GetTargetState());
    if (GetTargetState() == PCE_TARGET_STATE_CONNECTED) {
        GetPceService().Connect(GetDevice());
        SetTargetState(PCE_TARGET_STATE_UNKNOWN);
    } else if (GetTargetState() == PCE_TARGET_STATE_DISCONNECTED) {
        GetPceService().Disconnect(GetDevice());
        SetTargetState(PCE_TARGET_STATE_UNKNOWN);
    }
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
}

bool PbapPceStateMachine::IsBusy() const
{
    PBAP_PCE_LOG_DEBUG("%{public}s start", __PRETTY_FUNCTION__);
    bool ret = false;
    if (pceObexClient_ != nullptr) {
        ret = pceObexClient_->IsBusy();
    } else {
        PBAP_PCE_LOG_ERROR("%{public}s end, pceObexClient_ OR pceObexClient_->observer_ is null", __PRETTY_FUNCTION__);
    }
    PBAP_PCE_LOG_DEBUG("%{public}s end", __PRETTY_FUNCTION__);
    return ret;
}

bool PbapPceStateMachine::SetBusy(bool isBusy)
{
    PBAP_PCE_LOG_DEBUG("%{public}s start", __PRETTY_FUNCTION__);
    bool ret = false;
    if (pceObexClient_ != nullptr) {
        pceObexClient_->SetBusy(isBusy);
        ret = true;
    } else {
        PBAP_PCE_LOG_ERROR("%{public}s end, pceObexClient_ OR pceObexClient_->observer_ is null", __PRETTY_FUNCTION__);
    }
    PBAP_PCE_LOG_DEBUG("%{public}s end", __PRETTY_FUNCTION__);
    return ret;
}

void PbapPceStateMachine::SetPowerStatusBusy(bool isBusy)
{
    PBAP_PCE_LOG_DEBUG("%{public}s start", __PRETTY_FUNCTION__);
    if (powerStatusBusy_ == isBusy) {
        return;
    }
    powerStatusBusy_ = isBusy;
    if (isBusy) {
        HILOGI("[%{public}s] PowerStatusUpdate -> BUSY", GetEncryptAddr(device_.GetAddress()).c_str());
        IPowerManager::GetInstance()->StatusUpdate(RequestStatus::BUSY, PROFILE_NAME_PBAP_PCE, device_);
    } else {
        HILOGI("[%{public}s] PowerStatusUpdate -> IDLE", GetEncryptAddr(device_.GetAddress()).c_str());
        IPowerManager::GetInstance()->StatusUpdate(RequestStatus::IDLE, PROFILE_NAME_PBAP_PCE, device_);
    }
    PBAP_PCE_LOG_DEBUG("%{public}s end", __PRETTY_FUNCTION__);
    return;
}

int PbapPceStateMachine::AbortDownloading()
{
    PBAP_PCE_LOG_DEBUG("%{public}s start", __PRETTY_FUNCTION__);
    if (pceObexClient_ == nullptr) {
        PBAP_PCE_LOG_ERROR("%{public}s end, pceObexClient_ is null", __PRETTY_FUNCTION__);
        return RET_BAD_STATUS;
    }
    PBAP_PCE_LOG_DEBUG("%{public}s end", __PRETTY_FUNCTION__);
    SetPowerStatusBusy(true);
    return pceObexClient_->Abort();
}

void PbapPceStateMachine::SetConnectId(uint32_t connectId)
{
    connectId_ = connectId;
}

int PbapPceStateMachine::ReqDisconnect(bool withObexReq) const
{
    PBAP_PCE_LOG_DEBUG("%{public}s start", __PRETTY_FUNCTION__);
    if ((pceObexClient_ == nullptr)) {
        PBAP_PCE_LOG_ERROR("%{public}s end, pceObexClient_  is null", __PRETTY_FUNCTION__);
        return RET_BAD_STATUS;
    }
    pceObexClient_->Disconnect(withObexReq);
    PBAP_PCE_LOG_DEBUG("%{public}s end", __PRETTY_FUNCTION__);
    return RET_NO_ERROR;
}

void PbapPceStateMachine::ForceCloseObexClient()
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    if (pceObexClient_ == nullptr) {
        PBAP_PCE_LOG_WARN("%{public}s pceObexClient_ is null, skip", __PRETTY_FUNCTION__);
        return;
    }
    pceObexClient_->ForceClose();
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
}

void PbapPceStateMachine::ResetObexClient()
{
    PBAP_PCE_LOG_INFO("%{public}s start, pceObexClient_=%{public}p", __PRETTY_FUNCTION__, pceObexClient_.get());
    pceObexClient_ = nullptr;
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
}

bool PbapPceStateMachine::SplitFilePath(
    const std::u16string &filePath, std::u16string &retPath, std::u16string &retFileName)
{
    bool ret = false;
    size_t pos = filePath.find_last_of(u'/');
    if (pos != std::u16string::npos) {
        retPath = filePath.substr(0, pos);
        retFileName = filePath.substr(pos + 1);
        ret = true;
    }
    return ret;
}

std::string PbapPceStateMachine::Replace(const std::string &str, const std::string &oldStr, const std::string &newStr)
{
    std::string strRet = str;
    for (std::string::size_type pos = 0; pos != std::string::npos; pos += newStr.length()) {
        pos = strRet.find(oldStr, pos);
        if (pos != std::string::npos) {
            strRet.replace(pos, oldStr.length(), newStr);
        } else {
            break;
        }
    }
    return strRet;
}

std::string PbapPceStateMachine::U16ToStr(const std::u16string &str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
    return converter.to_bytes(str);
}

void PbapPceStateMachine::HandlePhoneBookSizeActionCompleted(const utility::Message &msg)
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__); 
    auto &device = GetDevice();
    int respCode = msg.arg1_;
    if (observer_) {
        observer_->OnActionCompleted(device, static_cast<int>(respCode), PBAP_ACTION_PULLPHONEBOOKSIZE);
    }
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
}

void PbapPceStateMachine::HandlePullPhoneBookActionCompleted(const utility::Message &msg)
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    auto &device = GetDevice();
    int respCode = msg.arg1_;
    if (observer_) {
        observer_->OnActionCompleted(device, static_cast<int>(respCode), PBAP_ACTION_PULLPHONEBOOK);
    }
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
}

void PbapPceStateMachine::HandleAbortDownloadingActionCompleted(const utility::Message &msg)
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    auto &device = GetDevice();
    int respCode = msg.arg1_;
    if (observer_) {
        observer_->OnActionCompleted(device, static_cast<int>(respCode), PBAP_ACTION_ABORT);
    }
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
}

void PbapPceStateMachine::TryReleasePbapMsg(const utility::Message &msg) const
{
    if (msg.what_ == PCE_SDP_FINISH) {
        std::unique_ptr<PbapPceHeaderSdpMsg> sdpMsg(static_cast<PbapPceHeaderSdpMsg *>(msg.arg2_));
        PBAP_PCE_LOG_ERROR("%{public}s msg.what_=[%{public}d] release memory", __PRETTY_FUNCTION__, msg.what_);
    }
}

const RawAddress &PbapPceStateMachine::GetDevice() const
{
    return this->device_;
}

PbapPceServiceImpl &PbapPceStateMachine::GetPceService() const
{
    return *pceService_;
}

PbapPceObexClient *PbapPceStateMachine::GetObexClient() const
{
    return pceObexClient_.get();
}

std::shared_ptr<PbapPceObexClient> PbapPceStateMachine::GetObexClientPtr() const
{
    return pceObexClient_;
}

void PbapPceStateMachine::SetVersionNumber(uint16_t versionNumber)
{
    versionNumber_ = versionNumber;
}

uint16_t PbapPceStateMachine::GetVersionNumber() const
{
    return versionNumber_;
}

void PbapPceStateMachine::SetSupportedRes(uint8_t supportedRes)
{
    supportedRes_ = supportedRes;
}

uint8_t PbapPceStateMachine::GetSupportedRes() const
{
    return supportedRes_;
}

void PbapPceStateMachine::SetSupportedFeature(uint32_t supportedFeature)
{
    supportedFeature_ = supportedFeature;
}

uint32_t PbapPceStateMachine::GetSupportedFeature() const
{
    return supportedFeature_;
}

bool PbapPceStateMachine::IsGoepL2capPSM() const
{
    return isGoepL2capPSM_;
}

void PbapPceStateMachine::SetGoepL2capPSM(bool goepL2capPSM)
{
    isGoepL2capPSM_ = goepL2capPSM;
}

bool PbapPceStateMachine::GetFeatureFlag() const
{
    return featureFlag_;
}

void PbapPceStateMachine::SetFeatureFlag(bool featureFlag)
{
    featureFlag_ = featureFlag;
}

PbapPceSdp &PbapPceStateMachine::GetSdp() const
{
    return pbapPceSdp_;
}

void PbapPceStateMachine::SetUserId(const std::string &userId)
{
    this->userId_ = userId;
}

const std::string &PbapPceStateMachine::GetUserId() const
{
    return userId_;
}

const std::u16string &PbapPceStateMachine::GetCurrentPath() const
{
    return currentPath_;
}

void PbapPceStateMachine::SetCurrentPath(const std::u16string &path)
{
    currentPath_ = path;
}

std::string PbapPceStateMachine::GetDownloadFileName() const
{
    std::u16string vcardPath, vcardFileName;
    SplitFilePath(GetReqVcardFileName(), vcardPath, vcardFileName);
    std::string device = Replace(GetDevice().GetAddress(), ":", "_");
    std::string strPath = Replace(U16ToStr(vcardPath), "/", "_");
    std::string strVcardFileName = Replace(U16ToStr(vcardFileName), "/", "_");
    std::string strRet = device + strPath + strVcardFileName;
    return strRet;
}

const std::u16string &PbapPceStateMachine::GetReqVcardFileName() const
{
    return reqVcardFileName_;
}

void PbapPceStateMachine::SetReqVcardFileName(const std::u16string &reqVcardFileName)
{
    reqVcardFileName_ = reqVcardFileName;
}

PbapPceStateMachine::~PbapPceStateMachine()
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
}

void PbapPceStateMachine::CreatePceObexClient(const PbapPceObexConfig &obexConfig)
{
    PBAP_PCE_LOG_INFO("%{public}s start, addr=%{public}s, rfcomm=%{public}u, l2capPsm=%{public}u, "
        "isL2capPSM=%{public}d, mtu=%{public}u", __PRETTY_FUNCTION__,
        GetEncryptAddr(obexConfig.addr).c_str(), obexConfig.rfCommChannel, obexConfig.l2capPsm,
        obexConfig.isL2capPSM, obexConfig.mtu);
    if (pceService_ == nullptr) {
        PBAP_PCE_LOG_ERROR("pceService_ == nullptr");
        return;
    }
    this->pceObexClient_ = std::make_shared<PbapPceObexClient>(obexConfig, *pceService_);
    PBAP_PCE_LOG_INFO("%{public}s end, pceObexClient_=%{public}p", __PRETTY_FUNCTION__, pceObexClient_.get());
}
}  // namespace bluetooth
}  // namespace OHOS
