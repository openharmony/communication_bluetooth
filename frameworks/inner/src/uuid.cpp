/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "uuid.h"

#include <cerrno>

#define CONSTANT_ZERO 0
#define CONSTANT_FOUR 4
#define CONSTANT_EIGHT 8
#define CONSTANT_TWELVE 12
#define CONSTANT_SIXTEEN 16
#define CONSTANT_TWENTY 20

namespace OHOS {
namespace Bluetooth {
const std::regex uuidRegex("^[0-9a-fA-F]{8}-([0-9a-fA-F]{4}-){3}[0-9a-fA-F]{12}$");

UUID::UUID(const long mostSigBits, const long leastSigBits)
{
    this->uuid_[15] = static_cast<uint8_t>(leastSigBits & 0x00000000000000FF); // 15是uuid的数组下标
    this->uuid_[14] = static_cast<uint8_t>((leastSigBits & 0x000000000000FF00) >> 8); // 14是uuid的数组下标，右移8位
    // 13是uuid的数组下标，右移16位
    this->uuid_[13] = static_cast<uint8_t>((leastSigBits & 0x0000000000FF0000) >> 16);
    // 12是uuid的数组下标，右移24位
    this->uuid_[12] = static_cast<uint8_t>((leastSigBits & 0x00000000FF000000) >> 24);
    // 11是uuid的数组下标，右移32位
    this->uuid_[11] = static_cast<uint8_t>((leastSigBits & 0x000000FF00000000) >> 32);
    // 10是uuid的数组下标，右移40位
    this->uuid_[10] = static_cast<uint8_t>((leastSigBits & 0x0000FF0000000000) >> 40);
    this->uuid_[9] = static_cast<uint8_t>((leastSigBits & 0x00FF000000000000) >> 48); // 9是uuid的数组下标，右移48位
    this->uuid_[8] = static_cast<uint8_t>((leastSigBits & 0xFF00000000000000) >> 56); // 8是uuid的数组下标，右移56位
    this->uuid_[7] = static_cast<uint8_t>(mostSigBits & 0x00000000000000FF); // 7是uuid的数组下标，右移8位
    this->uuid_[6] = static_cast<uint8_t>((mostSigBits & 0x000000000000FF00) >> 8); // 6是uuid的数组下标，右移8位
    this->uuid_[5] = static_cast<uint8_t>((mostSigBits & 0x0000000000FF0000) >> 16); // 5是uuid的数组下标，右移16位
    this->uuid_[4] = static_cast<uint8_t>((mostSigBits & 0x00000000FF000000) >> 24); // 4是uuid的数组下标，右移24位
    this->uuid_[3] = static_cast<uint8_t>((mostSigBits & 0x000000FF00000000) >> 32); // 3是uuid的数组下标，右移32位
    this->uuid_[2] = static_cast<uint8_t>((mostSigBits & 0x0000FF0000000000) >> 40); // 2是uuid的数组下标，右移40位
    this->uuid_[1] = static_cast<uint8_t>((mostSigBits & 0x00FF000000000000) >> 48); // 1是uuid的数组下标，右移48位
    this->uuid_[0] = static_cast<uint8_t>((mostSigBits & 0xFF00000000000000) >> 56); // 0是uuid的数组下标，右移56位
}

UUID UUID::FromString(const std::string &name)
{
    UUID ret;
    if (name.empty()) {
        return ret;
    }
    std::string tmp = name;
    std::size_t pos = tmp.find("-");

    while (pos != std::string::npos) {
        tmp.replace(pos, 1, "");
        pos = tmp.find("-");
    }

    for (std::size_t i = 0; (i + 1) < tmp.length();) {
        errno = 0;
        char *endptr = nullptr;
        long int num = std::strtol(tmp.substr(i, 2).c_str(), &endptr, 16); // 16: hexadecimal
        if (errno == ERANGE) {
            return ret;
        }
        ret.uuid_[i / 2] = static_cast<int>(num); // i / 2作为uuid的数组下标
        i += 2; // for 循环中，每轮增加2
    }

    return ret;
}

UUID UUID::RandomUUID()
{
    UUID random;

    struct timeval tv;
    struct timezone tz;
    struct tm randomTime;
    unsigned int randNum = 0;

    rand_r(&randNum);
    gettimeofday(&tv, &tz);
    localtime_r(&tv.tv_sec, &randomTime);
    random.uuid_[15] = static_cast<uint8_t>(tv.tv_usec & 0x00000000000000FF); // 15是uuid的数组下标
    random.uuid_[14] = static_cast<uint8_t>((tv.tv_usec & 0x000000000000FF00) >> 8); // 14是uuid的数组下标，右移8位
    random.uuid_[13] = static_cast<uint8_t>((tv.tv_usec & 0x0000000000FF0000) >> 16); // 13是uuid的数组下标，右移16位
    random.uuid_[12] = static_cast<uint8_t>((tv.tv_usec & 0x00000000FF000000) >> 24); // 12是uuid的数组下标，右移24位
    random.uuid_[10] = static_cast<uint8_t>((tv.tv_usec & 0x000000FF00000000) >> 32); // 10是uuid的数组下标，右移32位
    random.uuid_[9] = static_cast<uint8_t>((tv.tv_usec & 0x0000FF0000000000) >> 40); // 9是uuid的数组下标，右移40位
    random.uuid_[8] = static_cast<uint8_t>((tv.tv_usec & 0x00FF000000000000) >> 48); // 8是uuid的数组下标，右移48位
    random.uuid_[7] = static_cast<uint8_t>((tv.tv_usec & 0xFF00000000000000) >> 56); // 7是uuid的数组下标，右移56位
    // 6是uuid的数组下标
    random.uuid_[6] = static_cast<uint8_t>((randomTime.tm_sec + static_cast<int>(randNum)) & 0xFF);
    // 5是uuid的数组下标，右移8位
    random.uuid_[5] = static_cast<uint8_t>((randomTime.tm_min + (randNum >> 8)) & 0xFF);
    // 4是uuid的数组下标，右移16位
    random.uuid_[4] = static_cast<uint8_t>((randomTime.tm_hour + (randNum >> 16)) & 0xFF);
    // 3是uuid的数组下标，右移24位
    random.uuid_[3] = static_cast<uint8_t>((randomTime.tm_mday + (randNum >> 24)) & 0xFF);
    random.uuid_[2] = static_cast<uint8_t>(randomTime.tm_mon & 0xFF); // 2是uuid的数组下标
    random.uuid_[1] = static_cast<uint8_t>(randomTime.tm_year & 0xFF); // 1是uuid的数组下标
    random.uuid_[0] = static_cast<uint8_t>((randomTime.tm_year & 0xFF00) >> 8); // 0是uuid的数组下标，右移8位
    return random;
}

std::string UUID::ToString() const
{
    std::string tmp = "";
    std::string ret = "";
    static const char *hex = "0123456789ABCDEF";

    for (auto it = this->uuid_.begin(); it != this->uuid_.end(); it++) {
        tmp.push_back(hex[(((*it) >> 4) & 0xF)]); // 右移4位
        tmp.push_back(hex[(*it) & 0xF]);
    }
    // ToString操作，8， 4， 12， 16，20 作为截取字符的开始位置或截取长度
    ret = tmp.substr(CONSTANT_ZERO, CONSTANT_EIGHT) + "-" +
            tmp.substr(CONSTANT_EIGHT, CONSTANT_FOUR) + "-" +
            tmp.substr(CONSTANT_TWELVE, CONSTANT_FOUR) + "-" +
            tmp.substr(CONSTANT_SIXTEEN, CONSTANT_FOUR) + "-" +
            tmp.substr(CONSTANT_TWENTY);

    return ret;
}

int UUID::CompareTo(const UUID &val) const
{
    UUID tmp = val;
    return this->ToString().compare(tmp.ToString());
}

bool UUID::Equals(const UUID &val) const
{
    for (int i = 0; i < UUID::UUID128_BYTES_LEN; i++) {
        if (this->uuid_[i] != val.uuid_[i]) {
            return false;
        }
    }
    return true;
}

uint64_t UUID::GetLeastSignificantBits() const
{
    uint64_t leastSigBits = 0;
    for (int i = UUID::UUID128_BYTES_LEN / 2; i < UUID::UUID128_BYTES_LEN; i++) { // uuid长度/2作为i初始值
        leastSigBits = (leastSigBits << 8) | (uuid_[i] & 0xFF); // 左移8位
    }
    return leastSigBits;
}

uint64_t UUID::GetMostSignificantBits() const
{
    uint64_t mostSigBits = 0;
    for (int i = 0 / 2; i < UUID::UUID128_BYTES_LEN / 2; i++) { // uuid长度/2作为i最大值
        mostSigBits = (mostSigBits << 8) | (uuid_[i] & 0xFF); // 左移8位
    }
    return mostSigBits;
}

UUID UUID::ConvertFrom128Bits(const std::array<uint8_t, UUID::UUID128_BYTES_LEN> &name)
{
    UUID tmp;
    for (int i = 0; i < UUID::UUID128_BYTES_LEN; i++) {
        tmp.uuid_[i] = name[i];
    }
    return tmp;
}

std::array<uint8_t, UUID::UUID128_BYTES_LEN> UUID::ConvertTo128Bits() const
{
    std::array<uint8_t, UUID::UUID128_BYTES_LEN> uuid;

    for (int i = 0; i < UUID::UUID128_BYTES_LEN; i++) {
        uuid[i] = uuid_[i];
    }

    return uuid;
}

bool UUID::operator==(const UUID &rhs) const
{
    return uuid_ == rhs.uuid_;
}

bool UUID::operator<(const UUID &uuid) const
{
    return !Equals(uuid);
}

bool IsValidUuid(std::string uuid)
{
    return regex_match(uuid, uuidRegex);
}
}
}