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

#ifndef COVER_ART_H
#define COVER_ART_H

#include <map>
#include <mutex>
#include <string>

#include "bip_image_properties.h"
#include "bip_image_descriptor.h"

namespace OHOS {
namespace bluetooth {

class CoverArt {
public:
    explicit CoverArt(const std::shared_ptr<Media::PixelMap> &image);
    ~CoverArt(){};

    std::string GetImageHandle();
    std::shared_ptr<Media::PixelMap> GetImage();
    std::shared_ptr<Media::PixelMap> GetImage(std::shared_ptr<BipImageDescriptor> &descriptor);
    void SetImageHandle(const std::string &handle);
    void GetImageData(const std::shared_ptr<Media::PixelMap> image, std::vector<uint8_t> &data);
    std::shared_ptr<BipImageProperties> GetImageProperties();
    std::string GetImageHash(const std::shared_ptr<Media::PixelMap> image);

private:
    std::mutex mutex_ {};
    std::string imageHandle_ = "";
    std::shared_ptr<Media::PixelMap> image_ = nullptr;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // COVER_ART_H