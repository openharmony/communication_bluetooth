/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef BIP_IMAGE_FORMAT_H
#define BIP_IMAGE_FORMAT_H

#include <memory>
#include <mutex>
#include <string>

#include "bip_encoding.h"
#include "bip_transformation.h"
#include "pixel_map.h"

namespace OHOS {
namespace bluetooth {
const int32_t FORMAT_NATIVE = 0;
const int32_t FORMAT_VARIANT = 1;

class BipImageFormat {
public: 
    explicit BipImageFormat(int32_t type, std::shared_ptr<BipEncoding> encoding, std::shared_ptr<Media::PixelMap> pixel,
        int32_t size, std::shared_ptr<BipTransformation> transformation)
        : type_(type), encoding_(encoding), pixel_(pixel), size_(size), transformation_(transformation)
    {};
    ~BipImageFormat(){};
    std::shared_ptr<BipEncoding> GetEncoding();
    std::shared_ptr<Media::PixelMap> GetPixel();
    int32_t GetSize();
    int32_t GetType();
private:
    std::mutex mutex_ {};
    int32_t type_= 0;
    std::shared_ptr<BipEncoding> encoding_ = nullptr;
    std::shared_ptr<Media::PixelMap> pixel_ = nullptr;
    int32_t size_ = 0;
    std::shared_ptr<BipTransformation> transformation_ = nullptr;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // BIP_IMAGE_FORMAT_H