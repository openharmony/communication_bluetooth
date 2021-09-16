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
    HILOGI("GattCharacteristic::GattCharacteristic (3 parameters) starts");
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
    HILOGI("GattCharacteristic::GattCharacteristic (4 parameters) starts");
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
        memcpy(value_.get(), src.value_.get(), length_);
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
            memcpy(value_.get(), src.value_.get(), length_);
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
    HILOGI("GattCharacteristic::AddDescriptor starts");
    descriptors_.insert(descriptors_.end(), descriptor)->characteristic_ = this;
}

GattDescriptor *GattCharacteristic::GetDescriptor(const UUID &uuid)
{
    HILOGI("GattCharacteristic::GetDescriptor starts");
    for (auto &desc : descriptors_) {
        if (desc.GetUuid().Equals(uuid)) {
            return &desc;
        }
    }
    return nullptr;
}

std::vector<GattDescriptor> &GattCharacteristic::GetDescriptors()
{
    HILOGI("GattCharacteristic::GetDescriptors starts");
    return descriptors_;
}

uint16_t GattCharacteristic::GetHandle() const
{
    HILOGI("GattCharacteristic::GetHandle starts");
    return handle_;
}

int GattCharacteristic::GetPermissions() const
{
    HILOGI("GattCharacteristic::GetPermissions starts");
    return permissions_;
}

int GattCharacteristic::GetProperties() const
{
    HILOGI("GattCharacteristic::GetProperties starts");
    return properties_;
}

GattService *GattCharacteristic::GetService() const
{
    HILOGI("GattCharacteristic::GetService starts");
    return service_;
}

const UUID &GattCharacteristic::GetUuid() const
{
    HILOGI("GattCharacteristic::GetUuid starts");
    return uuid_;
}

const std::unique_ptr<uint8_t[]> &GattCharacteristic::GetValue(size_t *size) const
{
    HILOGI("GattCharacteristic::GetValue starts");
    *size = length_;
    return value_;
}

int GattCharacteristic::GetWriteType() const
{
    HILOGI("GattCharacteristic::GetWriteType starts");
    return writeType_;
}

int GattCharacteristic::SetWriteType(int type)
{
    HILOGI("GattCharacteristic::SetWriteType starts");
    if (type != WriteType::DEFAULT && type != WriteType::NO_RESPONSE && type != WriteType::SIGNED) {
        return GattStatus::INVALID_PARAMETER;
    }
    writeType_ = type;
    return GattStatus::GATT_SUCCESS;
}

void GattCharacteristic::SetValue(const uint8_t *values, const size_t length)
{
    HILOGI("GattCharacteristic::SetValue starts");
    if (nullptr == values || 0 == length) {
        return;
    }
    value_ = std::make_unique<uint8_t[]>(length);
    length_ = length;
    memcpy(value_.get(), values, length);
}

}  // namespace Bluetooth
}  // namespace OHOS
