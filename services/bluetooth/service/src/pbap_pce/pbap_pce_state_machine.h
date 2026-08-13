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

#ifndef PCE_STATE_MACHINE_H
#define PCE_STATE_MACHINE_H
#include <cstdint>
#include <cstring>
#include <memory>
#include "../obex/obex_header.h"
#include "../obex/obex_tlv.h"
#include "interface_profile_pbap_pce.h"
#include "log.h"
#include "btcommon/message.h"
#include "pbap_pce_def.h"
#include "pbap_pce_sdp.h"
#include "pbap_pce_header_msg.h"
#include "pbap_pce_obex_client.h"
#include "pbap_pce_parameter.h"
#include "raw_address.h"
#include "btcommon/state_machine.h"
#include "bt_def.h"
#include "pbap_pce_app_params.h"

namespace OHOS {
namespace bluetooth {

class PbapPceServiceImpl;

class PbapPceStateMachine : public utility::StateMachine {
public:
    /**
     * @brief constructor
     * @details constructor
     * @param dev remote device
     * @param pceService pce service
     * @param pbapPceSdp sdp service
     * @param observerMgrList pce observer list
     * @return
     */
    explicit PbapPceStateMachine(const RawAddress &dev, PbapPceServiceImpl &pceService, PbapPceSdp &pbapPceSdp,
        std::shared_ptr<IPbapPceObserver> observer);

    /**
     * @brief destructor
     * @details destructor
     */
    virtual ~PbapPceStateMachine();

    /**
     * @brief get remote device
     * @details get remote device
     * @return RawAddress&
     */
    const RawAddress &GetDevice() const;

    /**
     * @brief get pce service
     * @details get pce service
     * @return Reference to PbapPceService
     */
    PbapPceServiceImpl &GetPceService() const;

    /**
     * @brief get pce obex client
     * @details get pce obex client
     * @return Reference to PbapPceObexClient
     */
    PbapPceObexClient *GetObexClient() const;
    std::shared_ptr<PbapPceObexClient> GetObexClientPtr() const;

    /**
     * @brief Set Version Number
     * @details Set Version Number
     * @param versionNumber version Number
     * @return
     */
    void SetVersionNumber(uint16_t versionNumber);

    /**
     * @brief get version number
     * @details get version number
     * @return uint16_t
     */
    uint16_t GetVersionNumber() const;

    /**
     * @brief Set supported repositories
     * @details Set supported repositories
     * @param supportedRes supported repositories
     * @return
     */
    void SetSupportedRes(uint8_t supportedRes);

    /**
     * @brief get supported repositories
     * @details get supported repositories
     * @return uint8_t
     */
    uint8_t GetSupportedRes() const;

    /**
     * @brief Set supported features
     * @details Set supported features
     * @param supportedRes supported features
     * @return
     */
    void SetSupportedFeature(uint32_t supportedFeature);

    /**
     * @brief get supported features
     * @details get supported features
     * @return uint32_t
     */
    uint32_t GetSupportedFeature() const;

    /**
     * @brief get rfcomm or psm
     * @details get rfcomm or psm
     * @return bool
     */
    bool IsGoepL2capPSM() const;

    /**
     * @brief Set rfcomm or psm
     * @details Set rfcomm or psm
     * @param goepL2capPSM rfcomm or psm
     * @return
     */
    void SetGoepL2capPSM(bool goepL2capPSM);

    bool GetFeatureFlag() const;

    /**
     * @brief Set feature flag
     * @details Set feature flag
     * @param featureFlag
     * @return
     */
    void SetFeatureFlag(bool featureFlag);

    /**
     * @brief get sdp
     * @details get sdp
     * @return PbapPceSdp reference
     */
    PbapPceSdp &GetSdp() const;

    /**
     * @brief Create Pce Obex Client
     * @details Create Pce Obex Client
     * @param obexConfig
     * @return
     */
    void CreatePceObexClient(const PbapPceObexConfig &obexConfig);

    /**
     * @brief set userId
     * @details set userId
     * @param userId userId
     */
    void SetUserId(const std::string &userId);

    /**
     * @brief get userId
     * @details get userId
     * @return userId
     */
    const std::string &GetUserId() const;

    /**
     * @brief PullPhoneBook
     * @details PullPhoneBook
     */
    void PullPhoneBook(const PbapPceAppParams &param);

    /**
     * @brief IsBusy
     * @details Is Busy
     */
    bool IsBusy() const;

    /**
     * @brief SetBusy
     * @details SetBusy
     */
    bool SetBusy(bool isBusy);

    /**
     * @brief SetPowerStatusBusy
     * @details SetPowerStatusBusy
     */
    void SetPowerStatusBusy(bool isBusy);

    /**
     * @brief AbortDownloading
     * @details AbortDownloading
     */
    int AbortDownloading();

    /**
     * @brief SetConnectId
     * @details SetConnectId
     */
    void SetConnectId(uint32_t connectId);

    /**
     * @brief send obex disconnect request
     * @param withObexReq is send obex disconnect request
     * @return int Request processing result:0:succeeded -1:failed
     */
    int ReqDisconnect(bool withObexReq = true) const;

    /**
     * @brief 强制关闭 obex client（shutdown socket），用于断连时唤醒阻塞的下载线程。
     * @details 调用 PbapPceObexClient::ForceClose()，绕过 OBEX DISCONNECT 协议，
     *          直接 shutdown+close socket，并发送 PCE_OBEX_DISCONNECTED 消息。
     */
    void ForceCloseObexClient();

    /**
     * @brief 释放 obex client 引用，供 DisconnectedState 清理。
     * @details 将 pceObexClient_ 置空，确保重连时 CreatePceObexClient 新建对象。
     *          ffrt 下载线程持有的 shared_ptr 仍保持 obexClient 存活，直到 lambda 退出。
     */
    void ResetObexClient();

    /**
     * @brief Get current virtual phonebook path
     * @details when SetPhoneBook action succeeded, save the path
     * @return path
     */
    const std::u16string &GetCurrentPath() const;

    /**
     * @brief SetCurrentPath
     * @details when SetPhoneBook action succeeded, call this
     * @param path
     */
    void SetCurrentPath(const std::u16string &path);

    /**
     * @brief GetReqVcardFileName
     * @details Get Requested Vcard FileName for local DB
     */
    const std::u16string &GetReqVcardFileName() const;

    /**
     * @brief GetDownloadFileName
     * @details Get Download FileName for ObexFileBodyObject
     */
    std::string GetDownloadFileName() const;

    /**
     * @brief SetCurrentPath
     * @details when SetPhoneBook action succeeded, call this
     * @param path
     */
    void SetReqVcardFileName(const std::u16string &reqVcardFileName);

public:
    void HandlePhoneBookSizeActionCompleted(const utility::Message &msg);
    void HandlePullPhoneBookActionCompleted(const utility::Message &msg);
    void HandleAbortDownloadingActionCompleted(const utility::Message &msg);
    void TryReleasePbapMsg(const utility::Message &msg) const;
    // void RemoveBTMLogging() const;
    void TransitTargetState();
    int GetTargetState() const;
    void SetTargetState(int targetState);

private:
    static bool SplitFilePath(const std::u16string &filePath, std::u16string &retPath, std::u16string &retFileName);
    static std::string Replace(const std::string &str, const std::string &oldStr, const std::string &newStr);
    static std::string U16ToStr(const std::u16string &str);

private:
    RawAddress device_;                                           // remote address
    PbapPceServiceImpl *pceService_ {nullptr};                        // pce service
    std::shared_ptr<PbapPceObexClient> pceObexClient_ {nullptr};  // obex client

    uint16_t versionNumber_ = 0;     // received from pse version number
    uint8_t supportedRes_ = 0;       // received from pse Supported Repositories
    uint32_t supportedFeature_ = 0;  // received from pse Supported feature
    bool featureFlag_ = false;       // true:can received feature; false:cannot received feature

    bool isGoepL2capPSM_ = false;     // rfcomm or psm
    std::string password_ = "0000";   // user input password
    std::string userId_ = "";         // default password
    uint32_t connectId_ {0};          // session connectId from PceConnectedState state

    PbapPceSdp &pbapPceSdp_;                                // sdp
    std::shared_ptr<IPbapPceObserver> observer_;            // observer
    std::atomic_bool powerStatusBusy_ {false};              // powerStatusBusy_
    std::u16string currentPath_ = u"";                      // current path in sever
    std::u16string reqVcardFileName_ = u"";                 // requested vcard file name
    std::atomic_int targetState_ {0};                       // target State
    BT_DISALLOW_COPY_AND_ASSIGN(PbapPceStateMachine);
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // PCE_STATE_MACHINE_H
