/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
#include "securec.h"
#include "bluetooth_gatt_characteristic.h"
#include "bluetooth_gatt_descriptor.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {

GattCharacteristic::GattCharacteristic(const UUID uuid, int permissions, int properties)
    : writeType_(properties & Propertie::AUTHENTICATED_SIGNED_WRITES ? WriteType::SIGNED
                 : properties & Propertie::WRITE_WITHOUT_RESPONSE    ? WriteType::NO_RESPONSE
                                                                     : WriteType::DEFAULT),
      handle_(0),
      permissions_(permissions),
      properties_(properties),
      service_(nullptr),
      value_(nullptr),
      length_(0),
      descriptors_(),
      uuid_(uuid)
{
    HILOGI("enter, (3 parameters)");
}

GattCharacteristic::GattCharacteristic(const UUID uuid, uint16_t handle, const int permissions, const int properties)
    : writeType_(properties & Propertie::AUTHENTICATED_SIGNED_WRITES ? WriteType::SIGNED
                 : properties & Propertie::WRITE_WITHOUT_RESPONSE    ? WriteType::NO_RESPONSE
                                                                     : WriteType::DEFAULT),
      handle_(handle),
      permissions_(permissions),
      properties_(properties),
      service_(nullptr),
      value_(nullptr),
      length_(0),
      descriptors_(),
      uuid_(uuid)
{
    HILOGI("enter, (4 parameters)");
}

GattCharacteristic::GattCharacteristic(const GattCharacteristic &src)
    : writeType_(src.writeType_),
      handle_(src.handle_),
      permissions_(src.permissions_),
      properties_(src.properties_),
      service_(src.service_),
      value_(nullptr),
      length_(src.length_),
      descriptors_(),
      uuid_(src.uuid_)
{
    if (nullptr != src.value_ && 0 != length_) {
        value_ = std::make_unique<uint8_t[]>(length_);
        (void)memcpy_s(value_.get(), length_, src.value_.get(), length_);
    } else {
        value_.reset(nullptr);
        length_ = 0;
    }

    for (auto &desc : src.descriptors_) {
        AddDescriptor(desc);
    }
}

GattCharacteristic &GattCharacteristic::operator=(const GattCharacteristic &src)
{
    if (this != &src) {
        uuid_ = src.uuid_;
        permissions_ = src.permissions_;
        properties_ = src.properties_;
        service_ = src.service_;
        handle_ = src.handle_;
        length_ = src.length_;
        writeType_ = src.writeType_;

        if (nullptr != src.value_ && 0 != length_) {
            value_ = std::make_unique<uint8_t[]>(length_);
            (void)memcpy_s(value_.get(), length_, src.value_.get(), length_);
        } else {
            value_.reset(nullptr);
            length_ = 0;
        }

        descriptors_ = src.descriptors_;
    }
    return *this;
}

GattCharacteristic::GattCharacteristic(GattCharacteristic &&src)
    : writeType_(src.writeType_),
      handle_(src.handle_),
      permissions_(src.permissions_),
      properties_(src.properties_),
      service_(src.service_),
      value_(std::move(src.value_)),
      length_(src.length_),
      descriptors_(),
      uuid_(src.uuid_)
{
    for (auto &desc : src.descriptors_) {
        descriptors_.insert(descriptors_.end(), std::move(desc))->characteristic_ = this;
    }
}

void GattCharacteristic::AddDescriptor(const GattDescriptor &descriptor)
{
    HILOGI("enter");
    descriptors_.insert(descriptors_.end(), descriptor)->characteristic_ = this;
}

GattDescriptor *GattCharacteristic::GetDescriptor(const UUID &uuid)
{
    HILOGI("enter");
    for (auto &desc : descriptors_) {
        if (desc.GetUuid().Equals(uuid)) {
            return &desc;
        }
    }
    return nullptr;
}

std::vector<GattDescriptor> &GattCharacteristic::GetDescriptors()
{
    HILOGI("enter");
    return descriptors_;
}

uint16_t GattCharacteristic::GetHandle() const
{
    HILOGI("handle: 0x%{public}04X", handle_);
    return handle_;
}

int GattCharacteristic::GetPermissions() const
{
    HILOGI("permissions: %{public}d", permissions_);
    return permissions_;
}

int GattCharacteristic::GetProperties() const
{
    HILOGI("properties: %{public}d", properties_);
    return properties_;
}

GattService *GattCharacteristic::GetService() const
{
    HILOGI("enter");
    return service_;
}

const UUID &GattCharacteristic::GetUuid() const
{
    HILOGI("uuid: %{public}s", uuid_.ToString().c_str());
    return uuid_;
}

const std::unique_ptr<uint8_t[]> &GattCharacteristic::GetValue(size_t *size) const
{
    HILOGI("value size: %{public}zu", length_);
    *size = length_;
    return value_;
}

int GattCharacteristic::GetWriteType() const
{
    HILOGI("type: %{public}d", writeType_);
    return writeType_;
}

int GattCharacteristic::SetWriteType(int type)
{
    HILOGI("enter, type: %{public}d", type);
    if (type != WriteType::DEFAULT && type != WriteType::NO_RESPONSE && type != WriteType::SIGNED) {
        HILOGE("Invalid parameter");
        return GattStatus::INVALID_PARAMETER;
    }

    writeType_ = type;
    return GattStatus::GATT_SUCCESS;
}

void GattCharacteristic::SetValue(const uint8_t *values, const size_t length)
{
    HILOGI("enter, value length: %{public}zu", length);
    if (nullptr == values || 0 == length) {
        return;
    }
    value_ = std::make_unique<uint8_t[]>(length);
    length_ = length;
    (void)memcpy_s(value_.get(), length, values, length);
}

}  // namespace Bluetooth
}  // namespace OHOS
