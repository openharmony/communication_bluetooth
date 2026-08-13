/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PBAP_PSE_OBEX_SERVER_H
#define PBAP_PSE_OBEX_SERVER_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "pbap_pse_appparam.h"
#include "pbap_pse_def.h"
#include "pbap_pse_vcard_manager.h"
#include "../obex/obex_def.h"
#include "../obex/obex_request_handler.h"

namespace OHOS {
namespace bluetooth {
class PbapPseServiceImpl;
class PbapPseObexServer : public ObexRequestHandler {
public:
    explicit PbapPseObexServer(std::string &device, PbapPseServiceImpl *pbapPseServiceImpl);
    virtual ~PbapPseObexServer() {}

    int OnGet(ObexHeader &request, ObexHeader &reply, std::vector<uint8_t> &output, bool &sendBodyHeader) override;
    int OnPut(ObexHeader &request, ObexHeader &reply, std::vector<uint8_t> &input, bool &requestFinished) override;
    int OnConnect(ObexHeader &request, ObexHeader &reply) override;
    int OnDisconnect(ObexHeader &request, ObexHeader &reply) override;
    int OnSetPath(ObexHeader &request, ObexHeader &reply, bool backup, bool create) override;
    int OnDelete(ObexHeader &request, ObexHeader &reply) override;
    int OnAbort(ObexHeader &request, ObexHeader &reply) override;
    void OnClose(bool isThreadStart) override;
    void SetConnectionId(int connectionId) override;
    int GetConnectionId() override;

private:
    bool IsLegalPath(const std::string &path);
    bool IsNameMatchTarget(const std::string &name, const std::string &target);

    int PullVcardListing(PbapPseAppParam &appParam, ObexHeader &reply,
        std::vector<uint8_t> &output, bool &sendBodyHeader);
    int SendVcardListingXml(ObexHeader &reply, PbapPseAppParam &appParam, std::vector<uint8_t> &output);
    void CreatePhoneBookList(PbapPseAppParam &appParam, std::string &xml);
    void CreateCallHistoryList(PbapPseAppParam &appParam, std::string &xml);

    int PullVcardEntry(PbapPseAppParam &appParam, ObexHeader &reply,
        std::string &name, std::vector<uint8_t> &output, bool &sendBodyHeader);
    int PullPhonebook(PbapPseAppParam &appParam, ObexHeader &reply, std::string &name,
        std::vector<uint8_t> &output, bool &sendBodyHeader);
    int GetContentTypeForNoNameOrVcard(bool validName, std::string &type, int &contentType);
    int GetContentTypeAndCounter(std::string &name, PbapPseAppParam &appParam);
    int32_t GetRawContactIdByNameIndex(int32_t index, uint8_t orderBy);

    bool CheckNameIncludeVcf(std::string &name);
    int32_t HandleAppParaForResponse(int32_t size, ObexHeader &reply);
    void AppendNewMissdCallData(std::vector<uint8_t> &retValue);
    void AppendAppHeaderData(uint8_t tag, uint8_t length, std::vector<uint8_t> &data, std::vector<uint8_t> &retValue);

    bool GetVcardEntryIndex(std::string &name, int32_t &index);
    bool CalcCallLogParam(PbapPseAppParam &appParam, CallLogSelectParam &param);

    void CreateNumSearchType(std::vector<std::string> &nameList, std::vector<std::string> &names,
        std::vector<std::string> &selectNameList, std::vector<uint32_t> &selectNameListPos);
    void CreateOtherSearchType(PbapPseAppParam &appParam, std::vector<std::string> &nameList,
        std::vector<std::string> &selectNameList, std::vector<uint32_t> &selectNameListPos);
    bool CheckAccessAuthorization();

private:
    std::string deviceAddr_;
    PbapPseAppParam connectAppParam_;
    PbabPseVcardManager *mVcardManager_ = nullptr;
    PbapPseServiceImpl *pbapPseServiceImpl_ = nullptr;
    std::string currentPath_ = "";
    // The number of indexes in the phone book.
    bool needPhonebookSize_ = false;
    bool needNewMissedCallsNum_ = false;
    int32_t connectionId_ = -1;
};
} // namespace bluetooth
} // namespace OHOS
#endif // PBAP_PSE_OBEX_SERVER_H