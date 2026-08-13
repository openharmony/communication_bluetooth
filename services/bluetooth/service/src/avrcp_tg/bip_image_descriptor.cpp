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
#define LOG_TAG "bt_service_bip_image_descriptor"
#endif

#include "bip_image_descriptor.h"

namespace OHOS {
namespace bluetooth {
std::shared_ptr<BipEncoding> BipImageDescriptor::GetEncoding()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return encoding_;
}

std::shared_ptr<Media::PixelMap> BipImageDescriptor::GetPixel()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return pixel_;
}

int32_t BipImageDescriptor::GetSize()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return size_;
}

int32_t BipImageDescriptor::GetMaxSize()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return maxSize_;
}
}  // namespace bluetooth
}  // namespace OHOS