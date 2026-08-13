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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_bip_image_properties"
#endif

#include "bip_image_properties.h"

namespace OHOS {
namespace bluetooth {

void BipImageProperties::SetImageHandle(const std::string &handle)
{
    std::lock_guard<std::mutex> lock(propertiesLock_);
    imageHandle_ = handle;
}

void BipImageProperties::SetFriendlyName(const std::string &friendlyName)
{
    std::lock_guard<std::mutex> lock(propertiesLock_);
    friendlyName_ = friendlyName;
}

void BipImageProperties::AddNativeFormat(std::shared_ptr<BipImageFormat> format)
{
    std::lock_guard<std::mutex> lock(propertiesLock_);
    if (format != nullptr && format->GetType() == FORMAT_NATIVE) {
        nativeFormats_.push_back(format);
    }
}

void BipImageProperties::AddVariantFormat(std::shared_ptr<BipImageFormat> format)
{
    std::lock_guard<std::mutex> lock(propertiesLock_);
    if (format != nullptr && format->GetType() == FORMAT_VARIANT) {
        variantFormats_.push_back(format);
    }
}

void BipImageProperties::EncodeNativeFormats(std::string &xml)
{
    for (const auto &format : nativeFormats_) {
        std::shared_ptr<BipEncoding> encoding = format->GetEncoding();
        std::shared_ptr<Media::PixelMap> pixelMap = format->GetPixel();
        if (encoding == nullptr || pixelMap == nullptr) {
            continue;
        }
        std::string encodingStr = encoding->GetEncodingStr();
        Media::ImageInfo imageInfo;
        pixelMap->GetImageInfo(imageInfo);
        int32_t width = static_cast<int32_t>(imageInfo.size.width);
        int32_t height = static_cast<int32_t>(imageInfo.size.height);
        int32_t size = format->GetSize();
        xml.append("<native encoding=\"").append(encodingStr).append("\""); // required
        xml.append(" pixel=\"")
            .append(std::to_string(width)) // required
            .append("*")
            .append(std::to_string(height)) // required
            .append("\"");
        if (size >= 0) {
            xml.append(" size=\"").append(std::to_string(size)).append("\""); // implied
        }
        xml.append(" />\n");
    }
}

void BipImageProperties::EncodeVariantFormats(std::string &xml)
{
    for (const auto &format : variantFormats_) {
        std::shared_ptr<BipEncoding> encoding = format->GetEncoding();
        std::shared_ptr<Media::PixelMap> pixelMap = format->GetPixel();
        if (encoding == nullptr || pixelMap == nullptr) {
            continue;
        }
        std::string encodingStr = encoding->GetEncodingStr();
        Media::ImageInfo imageInfo;
        pixelMap->GetImageInfo(imageInfo);
        int32_t width = static_cast<int32_t>(imageInfo.size.width);
        int32_t height = static_cast<int32_t>(imageInfo.size.height);
        xml.append("<variant encoding=\"").append(encodingStr).append("\""); // required
        xml.append(" pixel=\"")
            .append(std::to_string(width)) // required
            .append("*")
            .append(std::to_string(height)) // required
            .append("\"");
        int32_t maxSize = 200;
        xml.append(" maxSize=\"").append(std::to_string(maxSize)).append("\""); // implied
        xml.append(" />\n");
    }
}

std::string BipImageProperties::Encode()
{
    std::lock_guard<std::mutex> lock(propertiesLock_);
    std::string xml = "<?xml version='1.0' encoding='utf-8' standalone='yes' ?>\n"; // fixed
    xml.append("<image-properties version=\"1.0\"");
    if (!imageHandle_.empty()) {
        xml.append(" handle=\"").append(imageHandle_).append("\""); // required
    }
    if (!friendlyName_.empty()) {
        xml.append(" friendly-name=\"").append(friendlyName_).append("\""); // implied
    }
    xml.append(">\n    ");
    EncodeNativeFormats(xml);
    EncodeVariantFormats(xml);
    xml.append("</image-properties>\n");
    return xml;
}
}  // namespace bluetooth
}  // namespace OHOS