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

#ifndef AVRCP_CT_BIP_REQUEST_H
#define AVRCP_CT_BIP_REQUEST_H

#include <string>
#include <memory>
#include <vector>
#include <cstdint>

#include "avrcp_ct_bip_image_properties.h"
#include "obex_client_session.h"

namespace OHOS {
namespace bluetooth {

class BipRequest {
public:
    static constexpr const char* HEAD_TYPE_GET_IMAGE_PROPERTIES = "x-bt/img-properties";
    static constexpr const char* HEAD_TYPE_GET_IMAGE = "x-bt/img-img";
    
    static constexpr uint8_t HEADER_ID_IMG_HANDLE = 0x30;
    static constexpr uint8_t HEADER_ID_IMG_DESCRIPTOR = 0x71;

    static constexpr int TYPE_GET_IMAGE_PROPERTIES = 1;
    static constexpr int TYPE_GET_IMAGE = 2;

    static constexpr int MAX_CONTINUE_ITERATIONS = 100;

    virtual ~BipRequest() = default;
    virtual int GetType() const = 0;
    virtual std::string ToString() const = 0;
    virtual bool Execute(std::shared_ptr<ObexClientSession> clientSession) = 0;
    virtual std::string GetImageHandle() const = 0;

    int GetResponseCode() const { return responseCode_; }

protected:
    int responseCode_ = -1;
};

class RequestGetImageProperties : public BipRequest {
public:
    explicit RequestGetImageProperties(const std::string& imageHandle);

    int GetType() const override;
    std::string ToString() const override;
    bool Execute(std::shared_ptr<ObexClientSession> clientSession) override;
    std::string GetImageHandle() const override { return imageHandle_; }

    const BipImageProperties& GetProperties() const { return properties_; }
private:
    std::string imageHandle_;
    BipImageProperties properties_;
};

class RequestGetImage : public BipRequest {
public:
    RequestGetImage(const std::string& imageHandle, const BipImageDescriptor& descriptor);

    int GetType() const override;
    std::string ToString() const override;
    bool Execute(std::shared_ptr<ObexClientSession> clientSession) override;
    std::string GetImageHandle() const override { return imageHandle_; }

    const BipImage& GetImage() const { return image_; }
    const BipImageDescriptor& GetDescriptor() const { return descriptor_; }

private:
    std::string imageHandle_;
    BipImageDescriptor descriptor_;
    BipImage image_;
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // AVRCP_CT_BIP_REQUEST_H