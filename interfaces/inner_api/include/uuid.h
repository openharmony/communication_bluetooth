/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

/**
 * @addtogroup Bluetooth
 * @{
 *
 * @brief Defines uuid for framework.
 *
 * @since 6
 */

/**
 * @file uuid.h
 *
 * @brief framework uuid interface.
 *
 * @since 6
 */

#ifndef DUMMY_UUID_H
#define DUMMY_UUID_H

#include "sys/time.h"
#include <string>
#include <array>
#include <ctime>
#include <regex>

namespace OHOS {
namespace Bluetooth {

/**
 * @brief This class provides framework uuid.
 *
 * @since 6
 */
class UUID {
public:
    //128 bits uuid length
    const static int UUID128_BYTES_LEN = 16;

    /**
     * @brief A constructor used to create an <b>UUID</b> instance.
     *
     * @since 6
     */
    UUID(){};

    /**
     * @brief A destructor used to delete the <b>UUID</b> instance.
     *
     * @since 6
     */
    ~UUID(){};

    /**
     * @brief A constructor used to create an <b>UUID</b> instance. Constructor a new UUID using most significant 64
     * bits and least significant 64 bits.
     *
     * @param[in] mostSigBits  : The most significant 64 bits of UUID.
     * @param[in] leastSigBits : The least significant 64 bits of UUID.
     * @since 6
     */
    UUID(const long mostSigBits, const long leastSigBits);

    /**
     * @brief A constructor used to create an <b>UUID</b> instance. Constructor a new UUID from string.
     *
     * @param[in] name : The value of string to create UUID.
     *      for example : "00000000-0000-1000-8000-00805F9B34FB"
     * @return Returns a specified UUID.
     * @since 6
     */
    static UUID FromString(const std::string &name);

    /**
     * @brief A constructor used to create an <b>UUID</b> instance. Constructor a new random UUID.
     *
     * @return Returns a random UUID.
     * @since 6
     */
    static UUID RandomUUID();

    /**
     * @brief Convert UUID to string.
     *
     * @return Returns a String object representing this UUID.
     * @since 6
     */
    std::string ToString() const;

    /**
     * @brief Compares this UUID with the specified UUID.
     *
     * @param[in] val : UUID which this UUID is to be compared.
     * @return Returns <b> <0 </b> if this UUID is less than compared UUID;
     *         returns <b> =0 </b> if this UUID is equal to compared UUID;
     *         returns <b> >0 </b> if this UUID is greater than compared UUID.
     * @since 6
     */
    int CompareTo(const UUID &val) const;

    /**
     * @brief Compares this object to the specified object.
     *
     * @param[in] val : UUID which this UUID is to be compared.
     * @return Returns <b>true</b> if this UUID is the same as compared UUID;
     *         returns <b>false</b> if this UUID is not the same as compared UUID.
     * @since 6
     */
    bool Equals(const UUID &val) const;

    /**
     * @brief Returns the least significant 64 bits of this UUID's 128 bit value.
     *
     * @return Retruns the least significant 64 bits of this UUID's 128 bit value.
     * @since 6
     */
    uint64_t GetLeastSignificantBits() const;

    /**
     * @brief Returns the most significant 64 bits of this UUID's 128 bit value.
     *
     * @return Returns the most significant 64 bits of this UUID's 128 bit value.
     * @since 6
     */
    uint64_t GetMostSignificantBits() const;

    /**
     * @brief Constructor a new UUID from uint8_t array.
     *
     * @param[in] name : The 128 bits value for a UUID.
     * @return Returns a specified UUID.
     * @since 6
     */
    static UUID ConvertFrom128Bits(const std::array<uint8_t, UUID128_BYTES_LEN> &name);

    /**
     * @brief Returns uint8_t array from UUID.
     *
     * @return returns a specified array.
     * @since 6
     */
    std::array<uint8_t, UUID128_BYTES_LEN> ConvertTo128Bits() const;

    /**
     * @brief Compare two UUID whether are same or not.
     *
     * @param rhs Compared UUID instance.
     * @return Returns <b>true</b> if this UUID is the same as compared UUID;
     *         returns <b>false</b> if this UUID is not the same as compared UUID.
     */
    bool operator==(const UUID &rhs) const;

    /**
     * @brief In order to use the object key in the map object, overload the operator <.
     * @param[in] uuid : UUID object.
     * @return @c bool : If the object uuid is the same, return true, otherwise return false.
     */
    bool operator<(const UUID &uuid) const;

private:
    std::array<uint8_t, UUID128_BYTES_LEN> uuid_ = {0x00};
};

/**
 * @brief This class provides framework ParcelUuid.
 *
 * @since 6
 */
using ParcelUuid = UUID;

bool IsValidUuid(std::string uuid);

} // namespace Bluetooth
} // namespace OHOS

#endif  //DUMMY_UUID_H