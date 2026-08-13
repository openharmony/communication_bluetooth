/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef BAS_MESSAGE_H
#define BAS_MESSAGE_H

#include <string>

#include "bas_defines.h"
#include "btcommon/message.h"
#include "securec.h"

namespace OHOS {
namespace bluetooth {
class BasMessage : public utility::Message {
public:
    explicit BasMessage(int what = 0, int arg1 = 0, void *arg2 = nullptr) : utility::Message(what, arg1, arg2)
    {}
    BasMessage(const BasMessage &src) : utility::Message(src.what_, src.arg1_, src.arg2_),
        dev_(src.dev_),
        batteryData_(src.batteryData_),
        data_(nullptr),
        dataLength_(src.dataLength_)
    {
        if ((dataLength_ > 0) && (src.data_ != nullptr)) {
            data_ = std::make_unique<uint8_t[]>(dataLength_);
            if (memcpy_s(data_.get(), dataLength_, src.data_.get(), dataLength_) != EOK) {
                data_.reset(nullptr);
                dataLength_ = 0;
            }
        } else {
            data_.reset(nullptr);
            dataLength_ = 0;
        }
    }
    ~BasMessage() override = default;
    std::string dev_ {""};
    BasBatteryData batteryData_ {};
    std::unique_ptr<uint8_t[]> data_ = nullptr;
    int dataLength_ = 0;

    BasMessage operator=(const BasMessage &src)
    {
        if (this != &src) {
            dev_ = src.dev_;
            batteryData_ = src.batteryData_;
            data_ = nullptr;
            dataLength_ = src.dataLength_;

            if ((dataLength_ > 0) && (src.data_ != nullptr)) {
                data_ = std::make_unique<uint8_t[]>(dataLength_);
                if (memcpy_s(data_.get(), dataLength_, src.data_.get(), dataLength_) != EOK) {
                    data_.reset(nullptr);
                    dataLength_ = 0;
                }
            } else {
                data_.reset(nullptr);
                dataLength_ = 0;
            }
        }
        return *this;
    }
};
}  // namespace bluetooth
}
#endif  // BAS_MESSAGE_H