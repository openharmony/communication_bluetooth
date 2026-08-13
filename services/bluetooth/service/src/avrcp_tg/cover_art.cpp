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
#define LOG_TAG "bt_service_cover_art"
#endif

#include "openssl/sha.h"

#include "cover_art.h"
#include "log.h"
#include "log_utils.h"
#include "image_packer.h"
#include "media_errors.h"

namespace OHOS {
namespace bluetooth {
constexpr uint8_t IMAGE_SUCCESS = 0;
constexpr uint32_t IMAGE_SIZE = 200;
constexpr int32_t DEFAULT_BUFFER_SIZE = 1 * 200 * 200;

CoverArt::CoverArt(const std::shared_ptr<Media::PixelMap> &pixelMap)
{
    if (pixelMap != nullptr) {
        int32_t errorCode = Media::ERR_MEDIA_INVALID_VALUE;
        std::shared_ptr<Media::PixelMap> image = pixelMap->Clone(errorCode);
        image_ = std::move(image);
        if (errorCode != Media::SUCCESS || image_ == nullptr) {
            HILOGE("clone pixelMap failed");
            return;
        }
        int32_t originalPixelMapBytes = image_->GetByteCount();
        if (originalPixelMapBytes > DEFAULT_BUFFER_SIZE) {
            int32_t originSize = originalPixelMapBytes;
            float scaleRatio = sqrt(
                static_cast<float>(DEFAULT_BUFFER_SIZE) / static_cast<float>(originalPixelMapBytes));
            image_->scale(scaleRatio, scaleRatio);
            originalPixelMapBytes = image_->GetByteCount();
            HILOGI_TIME_LIMIT(__func__, "CoverArt imgBufferSize exceeds limited: %{public}d scaled to %{public}d",
                originSize, originalPixelMapBytes);
        }
    }
}

std::shared_ptr<Media::PixelMap> CoverArt::GetImage()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return image_;
}

static bool IsDescriptorValid(std::shared_ptr<BipImageDescriptor> &descriptor)
{
    std::shared_ptr<BipEncoding> encoding = descriptor->GetEncoding();
    std::shared_ptr<Media::PixelMap> pixel = descriptor->GetPixel();
    if (encoding == nullptr || pixel == nullptr) {
        return false;
    }
    std::string encodingStr = encoding->GetEncodingStr();
    if (encodingStr.compare("JPEG") == 0 && pixel->GetWidth() == IMAGE_SIZE && pixel->GetHeight() == IMAGE_SIZE) {
        return true;
    }
    return false;
}

std::shared_ptr<Media::PixelMap> CoverArt::GetImage(std::shared_ptr<BipImageDescriptor> &descriptor)
{
    if (descriptor == nullptr || !IsDescriptorValid(descriptor)) {
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    return image_;
}

std::string CoverArt::GetImageHandle()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return imageHandle_;
}

void CoverArt::SetImageHandle(const std::string &handle)
{
    std::lock_guard<std::mutex> lock(mutex_);
    imageHandle_ = handle;
}

// 32 bytes SHA-256 hash -> 64 bits hex
static std::string Sha256DigestToHex(const unsigned char *digest)
{
    const char hexChars[] = "0123456789abcdef";
    std::string hexStr;
    hexStr.reserve(SHA256_DIGEST_LENGTH * 2);  //  pre-allocated 64 bytes
    for (auto i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        unsigned char byte = digest[i];
        hexStr.push_back(hexChars[(byte >> 4) & 0x0F]);
        hexStr.push_back(hexChars[byte & 0x0F]);
    }
    return hexStr;
}

std::string CoverArt::GetImageHash(const std::shared_ptr<Media::PixelMap> image)
{
    CHECK_AND_RETURN_LOG_RET(image, "", "image is nullptr");
    SHA256_CTX ctx;
    unsigned char digest[SHA256_DIGEST_LENGTH] = {0}; 
    // Calculate the SHA-256 hash of the input string
    std::vector<uint8_t> buff;
    if (!image->EncodeTlv(buff)) { // get pixelMap encode tlv
        HILOGE("PixelMap encodeTlv failed");
        return "";
    }
    if (!SHA256_Init(&ctx)) {
        HILOGE("SHA256_Init failed");
        return "";
    }
    if (!SHA256_Update(&ctx, buff.data(), buff.size())) {
        HILOGE("SHA256_Update failed");
        return "";
    }
    if (!SHA256_Final(digest, &ctx)) {
        HILOGE("SHA256_Final failed");
        return "";
    }
    return Sha256DigestToHex(digest);
}

// Obtain JPEG binary stream.
void CoverArt::GetImageData(const std::shared_ptr<Media::PixelMap> image, std::vector<uint8_t> &data)
{
    CHECK_AND_RETURN_LOG(image, "image is nullptr");
    Media::ImagePacker pack;
    std::vector<uint8_t> outputTempVec(DEFAULT_BUFFER_SIZE);
    uint8_t* outputData = outputTempVec.data();
    int64_t packedSize = 0;
    Media::PackOption option; // Encode Option Parameters
    option.format = "image/jpeg";

    // start packing
    uint32_t startpc = pack.StartPacking(outputData, DEFAULT_BUFFER_SIZE, option);
    CHECK_AND_RETURN_LOG(startpc == IMAGE_SUCCESS, "packing image is nullptr");
    // add image data
    pack.AddImage(*image);
    // packaging complete and obtain the actual data size.
    uint32_t result = pack.FinalizePacking(packedSize);
    if (result != IMAGE_SUCCESS) {
        HILOGE("packing image error");
        return;
    }
    if (packedSize < 0 || (packedSize > DEFAULT_BUFFER_SIZE)) {
        HILOGE("packed size invalid");
        return;
    }
    data.assign(outputData, outputData + packedSize);
}

// Get the set of image properties that the cover artwork can be turned into.
std::shared_ptr<BipImageProperties> CoverArt::GetImageProperties()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::shared_ptr<BipImageProperties> bipImageProperties = std::make_shared<BipImageProperties>();
    CHECK_AND_RETURN_LOG_RET(bipImageProperties, nullptr, "bip image properties is null");
    std::shared_ptr<BipEncoding> encoding = std::make_shared<BipEncoding>("JPEG");
    Media::InitializationOptions options;
    options.size.width = IMAGE_SIZE;
    options.size.height = IMAGE_SIZE;
    std::shared_ptr<Media::PixelMap> pixel = Media::PixelMap::Create(options); // Create an empty pixelmap.
    CHECK_AND_RETURN_LOG_RET(pixel, bipImageProperties, "pixel is null");

    std::shared_ptr<BipImageFormat> bipImageNativeFormat = std::make_shared<BipImageFormat>(
        FORMAT_NATIVE, encoding, pixel, -1, nullptr); // type: native-image
    bipImageProperties->AddNativeFormat(bipImageNativeFormat);
    encoding = std::make_shared<BipEncoding>("PNG");
    std::shared_ptr<BipImageFormat> bipImageVarintFormat = std::make_shared<BipImageFormat>(
        FORMAT_VARIANT, encoding, pixel, -1, nullptr); // type: variant-image
    bipImageProperties->AddVariantFormat(bipImageVarintFormat); // only for bqb test.
    return bipImageProperties;
}
}  // namespace bluetooth
}  // namespace OHOS