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

#ifndef PBAP_PCE_HEADER_MSG_H
#define PBAP_PCE_HEADER_MSG_H

#include <cstdint>
#include <cstring>
#include <memory>
#include "../obex/obex_header.h"
#include "../obex/obex_tlv.h"
#include "btcommon/message.h"
#include "pbap_pce_def.h"

namespace OHOS {
namespace bluetooth {
/// pbap pce state mathine message
enum PbapPcesmMessage {
    PCE_SDP_FINISH = 0x1001,             // sdp search success
    PCE_SDP_FAILED,                      // sdp search failed
    PCE_GAP_FINISH,                      // gap security success
    PCE_GAP_FAILED,                      // gap security failed
    PCE_REQ_DISCONNECTED,                // disconnect request
    PCE_PASSWORD_INPUT,                  // when device password is input by user
    PCE_OBEX_CONNECTED,                  // obex connected
    PCE_OBEX_CONNECT_FAILED,             // obex connect failed
    PCE_TRANSPORT_FAILED,                // transport failed
    PCE_OBEX_DISCONNECTED,               // obex disconnected
    PCE_DISCONNECT_TIMEOUT,              // disconnect timeout (3s)
    PCE_CONNECT_TIMEOUT,                 // connect timeout (10s)
    PCE_REQ_SET_TARGET_STATE,            // request setting target state
    PCE_REQ_TRANSIT_TARGET_STATE,        // transit target state
    PCE_REQ_PULLPHONEBOOKSIZE,           // pull phonebooksize
    PCE_PULLPHONEBOOKSIZE_COMPLETED,     // pull phonebooksize completed
    PCE_REQ_PULLPHONEBOOK,               // pull phonebook
    PCE_PULLPHONEBOOK_COMPLETED,         // pull phonebook completed
    PCE_REQ_SETPHONEBOOK,                // set phonebook
    PCE_SETPHONEBOOK_COMPLETED,          // pull setphonebook completed
    PCE_REQ_PULLVCARDLISTINGSIZE,        // pull vcardlisting size
    PCE_PULLVCARDLISTINGSIZE_COMPLETED,  // pull vcardlisting size completed
    PCE_REQ_PULLVCARDLISTING,            // pull vcard listing
    PCE_PULLVCARDLISTING_COMPLETED,      // pull pullvcardlisting completed
    PCE_REQ_PULLVCARDENTRY,              // pull vcard entry
    PCE_PULLVCARDENTRY_COMPLETED,        // pull pullvcardentry completed
    PCE_REQ_ABORTDOWNLOADING,            // abort downloading
    PCE_ABORTDOWNLOADING_COMPLETED,      // abort downloading completed
    PCE_DOWNLOAD_COMPLETE,               // auto-download of contacts and call logs completed
};

/**
 * @brief pbap pce message
 * pbap pce message
 */
struct PbapPceObexConfig {
    std::string addr {};
    uint16_t l2capLocalPsm = 0;
    uint16_t l2capPsm = 0;
    uint16_t rfCommChannel = 0;
    uint16_t mtu = 0;
    int32_t fileCount = 0;
    bool isL2capPSM = false;
    bool isSupportSrm = false;
    bool isSupportReliableSession = false;
    std::string serviceUUID {};
};

class PbapPceHeaderSdpMsg {
public:
    explicit PbapPceHeaderSdpMsg(PbapPceObexConfig &obexConfig, uint16_t versionNumber, uint8_t supportedRes,
        uint32_t supportedFeature, bool featureFlag)
        : obexConfig_(obexConfig),
          versionNumber_(versionNumber),
          supportedRes_(supportedRes),
          supportedFeature_(supportedFeature),
          featureFlag_(featureFlag)
    {}
    PbapPceHeaderSdpMsg(const PbapPceHeaderSdpMsg &other) = default;
    PbapPceHeaderSdpMsg &operator=(const PbapPceHeaderSdpMsg &other) = default;
    virtual ~PbapPceHeaderSdpMsg() = default;

    const PbapPceObexConfig &GetObexClientConfig() const
    {
        return obexConfig_;
    }

    uint16_t GetVersionNumber() const
    {
        return versionNumber_;
    }

    uint8_t GetSupportedRes() const
    {
        return supportedRes_;
    }

    uint32_t GetSupportedFeature() const
    {
        return supportedFeature_;
    }

    bool GetFeatureFlag() const
    {
        return featureFlag_;
    }

private:
    PbapPceObexConfig obexConfig_ {};
    uint16_t versionNumber_ = 0;
    uint8_t supportedRes_ = 0;
    uint32_t supportedFeature_ = 0;
    bool featureFlag_ = false;
};

/**
 * @brief pbap pce obex message
 * pbap pce obex message
 */
class PbapPceObexMessage {
public:
    explicit PbapPceObexMessage(const std::string &remoteAddr, const ObexHeader &resp)
        : remoteAddr_(remoteAddr), resp_(resp)
    {}

    virtual ~PbapPceObexMessage() = default;

    const std::string &GetRemoteAddress() const
    {
        return remoteAddr_;
    }

    const ObexHeader &GetObexHeader() const
    {
        return resp_;
    }

private:
    std::string remoteAddr_;
    ObexHeader resp_;
};

/**
 * @brief pbap pce obex message for action
 * pbap pce message for action
 */
class PbapPceActionObexMessage : public PbapPceObexMessage {
public:
    explicit PbapPceActionObexMessage(
        const std::string &remoteAddr, const ObexHeader &resp, const std::u16string &name, uint8_t flags)
        : PbapPceObexMessage(remoteAddr, resp), name_(name), flags_(flags)
    {}

    ~PbapPceActionObexMessage() override = default;

    std::u16string GetName() const
    {
        return name_;
    }

    uint8_t GetFlags() const
    {
        return flags_;
    }

private:
    std::u16string name_;
    uint8_t flags_;
};

/**
 * @brief Pbap Pce PullPhoneBookMsg for interthread
 * Pbap Pce PullPhoneBook Msg
 */
class PbapPcePullPhoneBookMsg {
public:
    explicit PbapPcePullPhoneBookMsg() = default;
    virtual ~PbapPcePullPhoneBookMsg() = default;

    std::u16string GetName() const
    {
        return name_;
    }

    void SetName(const std::u16string& name)
    {
        name_ = name;
    }

    uint64_t GetPropertySelector() const
    {
        return propertySelector_;
    }

    void SetPropertySelector(uint64_t selector)
    {
        propertySelector_ = selector;
    }

    uint8_t GetFormat() const
    {
        return format_;
    }

    void SetFormat(uint8_t fmt)
    {
        format_ = fmt;
    }

    uint16_t GetMaxListCount() const
    {
        return maxListCount_;
    }

    void SetMaxListCount(uint16_t max)
    {
        maxListCount_ = max;
    }

    uint16_t GetListStartOffset() const
    {
        return listStartOffset_;
    }

    void SetListStartOffset(uint16_t offset)
    {
        listStartOffset_ = offset;
    }

    uint8_t GetResetNewMissedCalls() const
    {
        return resetNewMissedCalls_;
    }

    void SetResetNewMissedCalls(uint8_t reset)
    {
        resetNewMissedCalls_ = reset;
    }

    uint64_t GetvCardSelector() const
    {
        return vCardSelector_;
    }

    void SetvCardSelector(uint64_t selector)
    {
        vCardSelector_ = selector;
    }

    uint8_t GetvCardSelectorOp() const
    {
        return vCardSelectorOp_;
    }

    void SetvCardSelectorOp(uint8_t op)
    {
        vCardSelectorOp_ = op;
    }

    uint64_t GetSpecifiedBitset() const
    {
        return specifiedBitset_;
    }

    void SetSpecifiedBitset(uint64_t bitSet)
    {
        specifiedBitset_ = bitSet;
    }

private:
    std::u16string name_ = u"";
    uint64_t propertySelector_ = 0ULL;
    uint8_t format_ = 0;
    uint16_t maxListCount_ = 0;
    uint16_t listStartOffset_ = 0;
    uint8_t resetNewMissedCalls_ = 0;
    uint64_t vCardSelector_ = 0ULL;
    uint8_t vCardSelectorOp_ = 0;
    uint64_t specifiedBitset_ = 0ULL;
};

/**
 * @brief Pbap Pce SetPhoneBookMsg for interthread
 * Pbap Pce SetPhoneBook Msg
 */
class PbapPceSetPhoneBookMsg {
public:
    explicit PbapPceSetPhoneBookMsg(const std::u16string &name, uint8_t flag) : name_(name), flag_(flag)
    {}
    virtual ~PbapPceSetPhoneBookMsg() = default;

    std::u16string GetName() const
    {
        return name_;
    }

    uint8_t GetFlag() const
    {
        return flag_;
    }

private:
    std::u16string name_ = u"";
    uint8_t flag_ = 0;
};

/**
 * @brief Pbap Pce PullvCardListing Msg for interthread
 * Pbap Pce PullvCardListing Msg
 */
class PbapPcePullvCardListingMsg {
public:
    explicit PbapPcePullvCardListingMsg() = default;
    virtual ~PbapPcePullvCardListingMsg() = default;

    std::u16string GetName() const
    {
        return name_;
    }

    void SetName(const std::u16string& name)
    {
        name_ = name;
    }

    uint8_t GetOrder() const
    {
        return order_;
    }

    void SetOrder(uint8_t order)
    {
        order_ = order;
    }

    std::string GetSearchValue() const
    {
        return searchValue_;
    }

    void SetSearchValue(const std::string& val)
    {
        searchValue_ = val;
    }

    uint8_t GetSearchProperty() const
    {
        return searchProperty_;
    }

    void SetSearchProperty(uint8_t prop)
    {
        searchProperty_ = prop;
    }

    uint16_t GetMaxListCount() const
    {
        return maxListCount_;
    }

    void SetMaxListCount(uint16_t max)
    {
        maxListCount_ = max;
    }

    uint16_t GetListStartOffset() const
    {
        return listStartOffset_;
    }

    void SetListStartOffset(uint16_t offset)
    {
        listStartOffset_ = offset;
    }

    uint8_t GetResetNewMissedCalls() const
    {
        return resetNewMissedCalls_;
    }

    void SetResetNewMissedCalls(uint8_t reset)
    {
        resetNewMissedCalls_ = reset;
    }

    uint64_t GetvCardSelector() const
    {
        return vCardSelector_;
    }

    void SetvCardSelector(uint64_t selector)
    {
        vCardSelector_ = selector;
    }

    uint8_t GetvCardSelectorOp() const
    {
        return vCardSelectorOp_;
    }

    void SetvCardSelectorOp(uint8_t op)
    {
        vCardSelectorOp_ = op;
    }

    uint64_t GetSpecifiedBitset() const
    {
        return specifiedBitset_;
    }

    void SetSpecifiedBitset(uint64_t bitSet)
    {
        specifiedBitset_ = bitSet;
    }

private:
    std::u16string name_ = u"";
    uint8_t order_ = 0;
    std::string searchValue_ = "";
    uint8_t searchProperty_ = 0;
    uint16_t maxListCount_ = 0;
    uint16_t listStartOffset_ = 0;
    uint8_t resetNewMissedCalls_ = 0;
    uint64_t vCardSelector_ = 0ULL;
    uint8_t vCardSelectorOp_ = 0;
    uint64_t specifiedBitset_ = 0ULL;
};

/**
 * @brief Pbap Pce PullvCardEntry Msg for interthread
 * Pbap Pce PullvCardEntry Msg
 */
class PbapPcePullvCardEntryMsg {
public:
    PbapPcePullvCardEntryMsg() = default;

    explicit PbapPcePullvCardEntryMsg(
        const std::u16string &name, uint64_t propertySelector, uint8_t format, uint64_t specifiedBitset)
        : name_(name), propertySelector_(propertySelector), format_(format), specifiedBitset_(specifiedBitset)
    {}

    virtual ~PbapPcePullvCardEntryMsg() = default;

    std::u16string GetName() const
    {
        return name_;
    }

    uint64_t GetPropertySelector() const
    {
        return propertySelector_;
    }

    uint8_t GetFormat() const
    {
        return format_;
    }

    uint64_t GetSpecifiedBitset() const
    {
        return specifiedBitset_;
    }

    void SetName(const std::u16string& name)
    {
        name_ = name;
    }

    void SetPropertySelector(uint64_t selector)
    {
        propertySelector_ = selector;
    }

    void SetFormat(uint8_t format)
    {
        format_ = format;
    }

    void SetSpecifiedBitset(uint64_t bitSet)
    {
        specifiedBitset_ = bitSet;
    }

private:
    std::u16string name_ = u"";
    uint64_t propertySelector_ = 0ULL;
    uint8_t format_ = 0;
    uint64_t specifiedBitset_ = 0ULL;
};

/**
 * @brief Pbap Pce abort action Msg for interthread
 * Pbap Pce abort action Msg
 */
class PbapPceAbortMsg {
public:
    explicit PbapPceAbortMsg(const std::u16string &name) : name_(name)
    {}

    virtual ~PbapPceAbortMsg() = default;

    std::u16string GetName() const
    {
        return name_;
    }

private:
    std::u16string name_;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // PBAP_PCE_HEADER_MSG_H
