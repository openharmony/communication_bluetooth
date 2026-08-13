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

#ifndef PBAP_PCE_OBEX_CLIENT_H
#define PBAP_PCE_OBEX_CLIENT_H

#include <cstdint>
#include <cstring>
#include <fstream>
#include <memory>
#include <mutex>
#include "../obex/obex_body.h"
#include "../obex/obex_client_session.h"
#include "../obex/obex_header.h"
#include "../obex/obex_socket_device.h"
#include "../obex/obex_tlv.h"
#include "obex_response_handler.h"
#include "pbap_pce_app_params.h"
#include "pbap_pce_datashare_helper.h"
#include "pbap_pce_header_msg.h"
#include "interface_profile.h"
#include "interface_profile_manager.h"
#include "interface_profile_socket.h"
#include "common_util.h"

namespace OHOS {
namespace bluetooth {

class PbapPceServiceImpl;

/// download feature
static const uint32_t PBAP_PCE_FEATURES_DOWNLOAD = 1 << 0;
/// browsing feature
static const uint32_t PBAP_PCE_FEATURES_BROWSING = 1 << 1;
/// database identifier feature
static const uint32_t PBAP_PCE_FEATURES_DATABASE_IDENTIFIER = 1 << 2;
/// folder version counter feature
static const uint32_t PBAP_PCE_FEATURES_FOLDER_VERSION_COUNTERS = 1 << 3;
/// vcard selecting feature
static const uint32_t PBAP_PCE_FEATURES_VCARD_SELECTING = 1 << 4;
/// enhanced missed calls feature
static const uint32_t PBAP_PCE_FEATURES_ENHANCED_MISSED_CALLS = 1 << 5;
/// uci property feature
static const uint32_t PBAP_PCE_FEATURES_X_BT_UCI_VCARD_PROPERTY = 1 << 6;
/// uid property feature
static const uint32_t PBAP_PCE_FEATURES_X_BT_UID_VCARD_PROPERTY = 1 << 7;
/// contact referencing feature
static const uint32_t PBAP_PCE_FEATURES_CONTACT_REFERENCING = 1 << 8;
/// contact image feature
static const uint32_t PBAP_PCE_FEATURES_DEFAULT_CONTACT_IMAGE_FORMAT = 1 << 9;

/// uuid length
static const uint8_t PBAP_PCE_SERVICE_UUID_LEN = 16;

/// pbap uuid
static constexpr uint8_t PBAP_PCE_SERVICE_UUID[PBAP_PCE_SERVICE_UUID_LEN] = {
    0x79, 0x61, 0x35, 0xf0, 0xf0, 0xc5, 0x11, 0xd8, 0x09, 0x66, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66
};

// PBAP v1.2.3 Sec. 7.1.2
static const int PBAP_PCE_SUPPORTED_REPOS_LOCALPB = 1 << 0;
static const int PBAP_PCE_SUPPORTED_REPOS_SIMCARD = 1 << 1;
static const int PBAP_PCE_SUPPORTED_REPOS_FAV = 1 << 3;

//call log name 
static const std::string PATH_PB = "telecom/pb";      // 联系人电话本
static const std::string PATH_MCH = "telecom/mch";    // 未接来电
static const std::string PATH_ICH = "telecom/ich";    // 已接来电
static const std::string PATH_OCH = "telecom/och";    // 已拨电话
static const std::string PATH_CCH = "telecom/cch";    // 全部通话
static const std::string PATH_TEL = "telecom/tel";    // 电话本

// Download constants (aligned with design doc §5.2.1.3)
static const uint16_t PBAP_PCE_DEFAULT_BATCH_SIZE = 250;   // 每批最多250条联系人
static const uint16_t PBAP_PCE_CALLLOG_MAX_COUNT = 100;    // 通话记录每类型最多100条
static const uint8_t PBAP_PCE_VCARD_FORMAT_30 = 1;         // vCard 3.0格式

// vCard file suffix
static const std::string VCARD_FILE_SUFFIX = ".vcf";

class MemoryBodyObject : public ObexBodyObject {
public:
    MemoryBodyObject() = default;
    ~MemoryBodyObject() override = default;

    void OpenFile(const int fileFd) override {}
    int Close() override { return 0; }
    size_t Read(uint8_t *buf, size_t bufLen) override { return 0; }
    size_t Write(const uint8_t *buf, size_t bufLen) override;
    bool IsReadFinished() override { return true; }

    const std::vector<uint8_t> &GetData() const { return data_; }
    void Clear() { data_.clear(); }

private:
    std::vector<uint8_t> data_;
};

class PbapPceObexClient : public ObexResponseHandler {
public:
    explicit PbapPceObexClient(const PbapPceObexConfig &config, PbapPceServiceImpl &pceService);
    virtual ~PbapPceObexClient();

    int Connect(std::shared_ptr<PbapPceObexClient> self, bool supported);
    int Disconnect(bool withObexReq = true);
    int Get(const ObexHeader &req, int reqMsgType, std::shared_ptr<ObexBodyObject> writer = nullptr);
    int Abort();

    uint16_t PullPhoneBookSize(const std::string &path);
    int PullCalllog(const std::string &path, uint16_t offset, uint16_t count);
    int PullPhoneBookBatch(const std::string &path, uint16_t offset, uint16_t count, bool isFinish);
    void DownloadContactsAndCallLogs();

    /// Parse vCard body and write call logs to database.
    /// @param bodyData vCard byte data from OBEX response.
    /// @param path PBAP path (MCH/ICH/OCH) to determine call type.
    /// @return 0 on success, negative on failure.
    int ParseAndInsertCallLogs(const std::vector<uint8_t> &bodyData, const std::string &path);

    /// Parse vCard body and write contacts to database.
    /// @param bodyData vCard byte data from OBEX response.
    /// @param isFinish Whether this is the last batch of the current sync round.
    /// @return 0 on success, negative on failure.
    int ParseAndInsertContacts(const std::vector<uint8_t> &bodyData, bool isFinish);

    ObexClientSession *GetClientSession() const;
    const std::string &GetRemoteAddress() const;

    void SetBusy(bool isBusy);
    bool IsBusy() const;
    void SetAbort(bool isAbort);

    struct PhoneBookActionInfo {
        int operationId_ = 0;
        int reqMsgType_ = 0;
        std::u16string path_ = u"";
        uint8_t flags_ = 0;
        bool isAbort = false;
    };

    bool GetPhoneBookActionInfo(int operationId, int &retReqMsgType, std::u16string &retPath, uint8_t &retFlags) const;
    void SetPhoneBookActionInfo(int operationId, int reqMsgType, const std::u16string &path = u"",
        uint8_t flags = 0, bool isBusy = false);

    void OnConnectResp(ObexHeader &reply, uint8_t responseCode) override;
    void OnPutDateAvailableResp(ObexHeader &reply, uint8_t responseCode) override;
    void OnGetDateAvailableResp(ObexHeader &reply, uint8_t responseCode) override;
    void OnDisConnectResp(ObexHeader &reply, uint8_t responseCode) override;
    void OnAbortResp() override;

    void ObexConnectFailed();

    /// 强制关闭底层 socket，唤醒阻塞的下载线程。
    /// 用于断连流程，绕过 OBEX DISCONNECT 协议，直接 shutdown+close，
    /// 并发送 PCE_OBEX_DISCONNECTED 消息以推动状态机流转。
    void ForceClose();

private:
    std::shared_ptr<ObexSocketDevice> RecvSocketDevice(SocketType socketType);
    void CloseTransport();
    int CreateSocketConnection();
    void HandleConnectSuccess(ObexHeader &reply);
    void NotifyConnectFailed();
    void CloseSocket();
    void BuildConnectHeader(ObexHeader& header, bool supported);
    void BuildGetHeader(ObexHeader &header, const std::string &path, const PbapPceAppParams &appParams);
    void ExtractBodyFromResponse(ObexHeader &resp, MemoryBodyObject &writer);
    void NotifyDownloadComplete();

    int m_socketFd;
    int currentReqMsgType_;
    PbapPceObexConfig obexConfig_;
    PbapPceServiceImpl &pceService_;
    std::shared_ptr<ObexClientSession> clientSession_ = nullptr;
    ObexHeader connectHeader_;
    int32_t connectionId_ = -1;
    bool isObexConnected_ = false;
    std::atomic_bool isBusy_ {false};
    std::atomic_bool isAbort_ {false};
    bool isWaitingSendAbort_ = false;
    int socketFd_ = -1;
    std::mutex socketFdLock_;
    std::shared_ptr<ObexBodyObject> currentWriter_ = nullptr;
    PhoneBookActionInfo phoneBookActionInfo_ {};
    int lastReqMsgType_ = 0;
    std::weak_ptr<PbapPceObexClient> selfRef_;
    std::atomic_bool isBatchDownloading_ {false};
    std::atomic_bool isDisconnectRequested_ {false};  // 断连请求标志，下载循环检测后主动退出
    std::unique_ptr<PbapPceDataShareHelper> dataShareHelper_ = nullptr;
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // PBAP_PCE_OBEX_CLIENT_H
