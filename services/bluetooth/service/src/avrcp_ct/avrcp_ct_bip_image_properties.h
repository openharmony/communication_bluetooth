/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef AVRCP_CT_BIP_IMAGE_PROPERTIES_H
#define AVRCP_CT_BIP_IMAGE_PROPERTIES_H

#include <string>
#include <vector>
#include <cstdint>

namespace OHOS {
namespace bluetooth {

inline constexpr uint8_t BIP_IMAGE_FORMAT_NATIVE = 0x00;  // SCHEME_NATIVE：不发 descriptor，信号 native 格式
inline constexpr uint8_t BIP_IMAGE_FORMAT_JPEG = 0x01;
inline constexpr uint8_t BIP_IMAGE_FORMAT_PNG = 0x02;

inline constexpr uint16_t BIP_IMAGE_WIDTH = 200;
inline constexpr uint16_t BIP_IMAGE_HEIGHT = 200;

struct BipImageDescriptor {
    uint8_t format = BIP_IMAGE_FORMAT_JPEG;
    uint16_t width = 0;
    uint16_t height = 0;
};

struct BipImage {
    std::vector<uint8_t> data;
};

class BipImageProperties {
public:
    static constexpr const char* VERSION = "1.0";

    BipImageProperties() = default;
    ~BipImageProperties() = default;

    void SetImageHandle(const std::string& handle) { imageHandle_ = handle; }
    std::string GetImageHandle() const { return imageHandle_; }

    void SetHasThumbnailFormat(bool has) { hasThumbnailFormat_ = has; }
    bool HasThumbnailFormat() const { return hasThumbnailFormat_; }

    bool ParseFromData(const std::vector<uint8_t>& data);
    bool IsValid() const;

    std::string ToString() const;

private:
    std::string imageHandle_;
    std::string version_ = VERSION;
    bool hasThumbnailFormat_ = false;
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // AVRCP_CT_BIP_IMAGE_PROPERTIES_H