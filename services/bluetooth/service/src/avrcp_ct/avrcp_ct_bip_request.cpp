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
#define LOG_TAG "avrcp_ct_bip_request"
#endif

#include "avrcp_ct_bip_request.h"
#include "log.h"
#include "obex_def.h"

namespace OHOS {
namespace bluetooth {

RequestGetImageProperties::RequestGetImageProperties(const std::string &imageHandle)
{
    imageHandle_ = imageHandle;
}

int RequestGetImageProperties::GetType() const
{
    return TYPE_GET_IMAGE_PROPERTIES;
}

std::string RequestGetImageProperties::ToString() const
{
    return "RequestGetImageProperties { handle=" + imageHandle_ + " }";
}

bool RequestGetImageProperties::Execute(std::shared_ptr<ObexClientSession> clientSession)
{
    HILOGI("Processing GetImageProperties request: %{public}s", imageHandle_.c_str());

    ObexHeader reqHeader;
    std::string typeStr{BipRequest::HEAD_TYPE_GET_IMAGE_PROPERTIES};
    std::vector<uint8_t> typeVec(typeStr.begin(), typeStr.end());
    reqHeader.SetHeader(TYPE, typeVec);
    reqHeader.SetHeader(BipRequest::HEADER_ID_IMG_HANDLE,
                        std::vector<uint8_t>(imageHandle_.begin(), imageHandle_.end()));

    ObexHeader outHeader;
    std::vector<uint8_t> reqData;
    reqHeader.GenerateHeaderValue(reqData);

    if (!clientSession) {
        HILOGE("clientSession is null");
        responseCode_ = -1;
        return false;
    }

    std::vector<uint8_t> propertiesData;
    int iterations = 0;

    do {
        clientSession->SendRequest(OBEX_OPCODE_GET, reqData, outHeader);
        responseCode_ = clientSession->GetLastRespCd();

        if (responseCode_ != OBEX_HTTP_OK && responseCode_ != OBEX_HTTP_CONTINUE) {
            HILOGE("GetImageProperties failed, responseCode=0x%{public}02X", responseCode_);
            properties_.ParseFromData(propertiesData);
            return false;
        }

        uint16_t bodyLen = 0;
        auto body = outHeader.GetBody(bodyLen);
        uint16_t endBodyLen = 0;
        auto endBody = outHeader.GetEndOfBody(endBodyLen);
        if (bodyLen > 0) {
            propertiesData.insert(propertiesData.end(), body.begin(), body.end());
        }
        if (endBodyLen > 0) {
            propertiesData.insert(propertiesData.end(), endBody.begin(), endBody.end());
        }

        if (responseCode_ == OBEX_HTTP_CONTINUE) {
            HILOGD("Received OBEX_HTTP_CONTINUE, continuing to receive...");
        }

        ++iterations;
    } while (responseCode_ == OBEX_HTTP_CONTINUE && iterations < MAX_CONTINUE_ITERATIONS);

    if (iterations >= MAX_CONTINUE_ITERATIONS) {
        HILOGW("Max continuation iterations reached, truncating data");
    }

    properties_.ParseFromData(propertiesData);
    HILOGI("GetImageProperties complete for handle: %{public}s, responseCode=0x%{public}02X, %{public}s",
           imageHandle_.c_str(), responseCode_, properties_.ToString().c_str());
    return true;
}

RequestGetImage::RequestGetImage(const std::string &imageHandle, const BipImageDescriptor &descriptor)
{
    imageHandle_ = imageHandle;
    descriptor_ = descriptor;
}

int RequestGetImage::GetType() const
{
    return TYPE_GET_IMAGE;
}

std::string RequestGetImage::ToString() const
{
    return "RequestGetImage { handle=" + imageHandle_ + ", format=" + std::to_string(descriptor_.format) + ", " +
           std::to_string(descriptor_.width) + "x" + std::to_string(descriptor_.height) + " }";
}

bool RequestGetImage::Execute(std::shared_ptr<ObexClientSession> clientSession)
{
    HILOGI("Processing GetImage request: %{public}s", imageHandle_.c_str());

    ObexHeader reqHeader;
    std::string typeStr{BipRequest::HEAD_TYPE_GET_IMAGE};
    std::vector<uint8_t> typeVec(typeStr.begin(), typeStr.end());
    reqHeader.SetHeader(TYPE, typeVec);
    reqHeader.SetHeader(BipRequest::HEADER_ID_IMG_HANDLE,
                        std::vector<uint8_t>(imageHandle_.begin(), imageHandle_.end()));

    // 对齐双框架 BipImageDescriptor.serialize()：SCHEME_NATIVE 时不发 IMG_DESCRIPTOR header（信号 native 格式），
    // SCHEME_THUMBNAIL 时按 descriptor_ 字段动态生成 XML（避免硬编码 200*200）
    if (descriptor_.format != BIP_IMAGE_FORMAT_NATIVE) {
        std::string encodingStr;
        switch (descriptor_.format) {
            case BIP_IMAGE_FORMAT_PNG:  encodingStr = "PNG";  break;
            case BIP_IMAGE_FORMAT_JPEG:
            default:                    encodingStr = "JPEG"; break;
        }
        std::string xml = "<image-descriptor version=\"1.0\">"
                          "<image encoding=\"" + encodingStr + "\""
                          " pixel=\"" + std::to_string(descriptor_.width) + "*"
                          + std::to_string(descriptor_.height) + "\"/>"
                          "</image-descriptor>";
        std::vector<uint8_t> descriptorBytes(xml.begin(), xml.end());
        reqHeader.SetHeader(BipRequest::HEADER_ID_IMG_DESCRIPTOR, descriptorBytes);
    }

    ObexHeader outHeader;
    std::vector<uint8_t> reqData;
    reqHeader.GenerateHeaderValue(reqData);

    if (!clientSession) {
        HILOGE("clientSession is null");
        responseCode_ = -1;
        return false;
    }

    clientSession->SendRequest(OBEX_OPCODE_GET, reqData, outHeader);
    responseCode_ = clientSession->GetLastRespCd();

    if (responseCode_ != OBEX_HTTP_OK && responseCode_ != OBEX_HTTP_CONTINUE) {
        HILOGE("GetImage failed, responseCode=0x%{public}02X", responseCode_);
        return false;
    }

    uint16_t bodyLen = 0;
    auto body = outHeader.GetBody(bodyLen);
    uint16_t endBodyLen = 0;
    auto endBodyData = outHeader.GetEndOfBody(endBodyLen);
    std::vector<uint8_t> imageData;
    if (bodyLen > 0) {
        imageData.insert(imageData.end(), body.begin(), body.end());
    }
    if (endBodyLen > 0) {
        imageData.insert(imageData.end(), endBodyData.begin(), endBodyData.end());
    }

    image_.data = imageData;
    HILOGI("Image download complete: responseCode=0x%{public}02X, size=%{public}zu", responseCode_, imageData.size());
    return true;
}

}  // namespace bluetooth
}  // namespace OHOS