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

#ifndef OBEX_TLV_H
#define OBEX_TLV_H

#include <cstdint>
#include <vector>
#include <memory>
#include <algorithm>
#include <optional>
#include <endian.h>

namespace OHOS {
namespace bluetooth {

constexpr uint8_t TLV_UINT8_LENGTH = 1;
constexpr uint8_t TLV_UINT16_LENGTH = 2;
constexpr uint8_t TLV_UINT32_LENGTH = 4;
constexpr uint8_t TLV_UINT64_LENGTH = 8;

class TlvTriplet {
public:
    virtual ~TlvTriplet() = default;
    explicit TlvTriplet(const uint8_t tagId, const uint8_t len, const uint8_t *val, const uint8_t unitLen = 1);
    explicit TlvTriplet(const uint8_t tagId, const uint8_t val);
    explicit TlvTriplet(const uint8_t tagId, const uint16_t val);
    explicit TlvTriplet(const uint8_t tagId, const uint32_t val);
    explicit TlvTriplet(const uint8_t tagId, const uint64_t val);
    explicit TlvTriplet(const TlvTriplet &tlvTriplet);
    uint8_t GetTagId() const;
    uint8_t GetLen() const;
    uint8_t GetUnitLen() const;
    const uint8_t *GetVal() const;
    uint16_t GetUint16() const;
    uint32_t GetUint32() const;
    uint64_t GetUint64() const;

private:
    TlvTriplet();
    uint8_t tagId_ = 0;
    uint8_t len_ = 0;
    std::vector<uint8_t> val_ {};
    uint8_t unitLen_ = 1;
};

class ObexTlvParamters {
public:
    ObexTlvParamters() = default;
    virtual ~ObexTlvParamters() = default;
    ObexTlvParamters(const ObexTlvParamters &other);
    ObexTlvParamters &operator=(const ObexTlvParamters &other);
    ObexTlvParamters(ObexTlvParamters &&other) noexcept = default;
    ObexTlvParamters &operator=(ObexTlvParamters &&other) noexcept = default;
    void AppendTlvtriplet(const TlvTriplet &tlvTriplet);
    const std::vector<std::unique_ptr<TlvTriplet>> &GetTlvTriplets() const;
    const TlvTriplet *GetTlvtriplet(const uint8_t tagId) const;
    std::vector<uint8_t> ToBytes() const;
    static std::optional<ObexTlvParamters> ParseFromBytes(const uint8_t *data, size_t length);

protected:
    std::vector<std::unique_ptr<TlvTriplet>> tlvTriplets_ {};
};

class ObexDigestChallenge : public ObexTlvParamters {
public:
    static const uint8_t NONCE = 0x00;
    static const uint8_t OPTIONS = 0x01;
    static const uint8_t REALM = 0x02;

    const TlvTriplet *GetNonce() const;
    const TlvTriplet *GetOptions() const;
    const TlvTriplet *GetRealm() const;
    void AppendNonce(const uint8_t *nonce, const uint8_t length);
    void AppendOptions(const uint8_t options);
    void AppendRealm(const uint8_t *realm, const uint8_t length);
};

class ObexDigestResponse : public ObexTlvParamters {
public:
    static const uint8_t REQUEST_DIGEST = 0x00;
    static const uint8_t USER_ID = 0x01;
    static const uint8_t NONCE = 0x02;
    static const uint8_t MAX_USER_ID_LEN = 20;

    const TlvTriplet *GetRequestDigest() const;
    const TlvTriplet *GetUserId() const;
    const TlvTriplet *GetNonce() const;
    void AppendRequestDigest(const uint8_t *requestDigest, const uint8_t length);
    void AppendUserId(const uint8_t *userId, const uint8_t length);
    void AppendNonce(const uint8_t *nonce, const uint8_t length);
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // OBEX_TLV_H
