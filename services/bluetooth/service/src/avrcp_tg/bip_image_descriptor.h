
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

#ifndef BIP_IMAGE_DESCRIPTOR_H
#define BIP_IMAGE_DESCRIPTOR_H

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "bip_encoding.h"
#include "bip_transformation.h"
#include "pixel_map.h"

namespace OHOS {
namespace bluetooth {

class BipImageDescriptor {
public:
    explicit BipImageDescriptor(std::shared_ptr<BipEncoding> encoding, std::shared_ptr<Media::PixelMap> pixel,
        int32_t size, int32_t maxSize)
        : encoding_(encoding), pixel_(pixel), size_(size), maxSize_(maxSize)
    {};
    ~BipImageDescriptor(){};
    std::shared_ptr<BipEncoding> GetEncoding();
    std::shared_ptr<Media::PixelMap> GetPixel();
    int32_t GetSize();
    int32_t GetMaxSize();
private:
    std::mutex mutex_ {};
    std::shared_ptr<BipEncoding> encoding_ = nullptr;
    std::shared_ptr<Media::PixelMap> pixel_ = nullptr;
    int32_t size_ = -1;
    int32_t maxSize_ = -1;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // BIP_IMAGE_DESCRIPTOR_H