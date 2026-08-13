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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_avrcp_tg_bip_server"
#endif

#include "avrcp_tg_bip_obex_server.h"
#include "bluetooth_hw_interface.h"
#include "avrcp_tg_bip_message.h"
#include "avrcp_cover_art_storage.h"
#include "avrcp_bip_service.h"
#include "bip_image_descriptor.h"
#include "common_util.h"

#include <libxml/parser.h>
#include <libxml/tree.h>

namespace OHOS {
namespace bluetooth {
constexpr uint8_t HEADER_ID_IMG_HANDLE = 0x30;
constexpr uint8_t HEADER_ID_IMG_DESCRIPTOR = 0x71;
constexpr uint8_t HANDLE_SIZE = 7;
const std::string TYPE_GET_LINKED_THUMBNAIL = "x-bt/img-thm";
const std::string TYPE_GET_IMAGE_PROPERTIES = "x-bt/img-properties";
const std::string TYPE_GET_IMAGE = "x-bt/img-img";
const std::vector<uint8_t> BLUETOOTH_UUID_AVRCP_COVER_ART = {
    0x71, 0x63, 0xDD, 0x54, 0x4A, 0x7E, 0x11, 0xE2,
    0xB4, 0x7C, 0x00, 0x50, 0xC2, 0x49, 0x00, 0x48
};

AvrcpBipObexServer::AvrcpBipObexServer(std::string &device, const std::weak_ptr<BipService> &bipService)
    : deviceAddr_(device), bipService_(bipService)
{}

static bool IsImageHandleValid(const std::string &handle)
{
    if (handle.empty() || handle.size() != HANDLE_SIZE) {
        return false;
    }

    for (auto c : handle) {
        if (!std::isdigit(c)) {
            return false;
        }
    }
    return true;
}

static std::string Utf16BeToUtf8String(const std::vector<uint8_t> &bytes)
{
    CHECK_AND_RETURN_LOG_RET(!(bytes.empty() || bytes.size() % 2), "", "invalid handle");
    std::string result;
    result.reserve(bytes.size() * 3 / 2);

    for (size_t i = 0; i < bytes.size(); i += 2) {
        uint16_t codeUnit = (static_cast<uint16_t>(bytes[i]) << 8) | bytes[i+1];
        if (codeUnit < 0x80) {
            result.push_back(static_cast<char>(codeUnit));
        } else if (codeUnit < 0x800) {
            result.push_back(static_cast<char>(0xC0 | (codeUnit >> 6)));
            result.push_back(static_cast<char>(0x80 | (codeUnit & 0x3F)));
        } else {
            result.push_back(static_cast<char>(0xE0 | (codeUnit >> 12)));
            result.push_back(static_cast<char>(0x80 | ((codeUnit >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (codeUnit & 0x3F)));
        }
    }
    return result;
}

int AvrcpBipObexServer::HandleGetImageProperties(ObexHeader &request, ObexHeader &reply,
    std::vector<uint8_t> &output, bool &sendBodyHeader)
{
    HILOGI("get image properties start!");
    std::vector<uint8_t> imageHandle = request.GetHeader(HEADER_ID_IMG_HANDLE);
    CHECK_AND_RETURN_LOG_RET(!imageHandle.empty(), ResponseCodes::OBEX_HTTP_BAD_REQUEST, "image handle is null");
    
    std::string imageHandleStr = Utf16BeToUtf8String(imageHandle);
    imageHandleStr.erase(std::remove(imageHandleStr.begin(), imageHandleStr.end(), '\0'), imageHandleStr.end());
    CHECK_AND_RETURN_LOG_RET(IsImageHandleValid(imageHandleStr), ResponseCodes::OBEX_HTTP_PRECON_FAILED,
        "invalid image handle");
    auto coverArtStorage = AvrcpCoverArtStorage::GetInstance();
    CHECK_AND_RETURN_LOG_RET(coverArtStorage, ResponseCodes::OBEX_HTTP_PRECON_FAILED,
        "avrcpCoverArtStorage is null");
    std::shared_ptr<CoverArt> image = coverArtStorage->GetImage(imageHandleStr);
    if (image == nullptr) {
        HILOGE("No image stored at handle");
        return ResponseCodes::OBEX_HTTP_PRECON_FAILED;
    }
    std::shared_ptr<BipImageProperties> properties = image->GetImageProperties();
    CHECK_AND_RETURN_LOG_RET(properties, ResponseCodes::OBEX_HTTP_PRECON_FAILED, "image no properties");
    properties->SetImageHandle(imageHandleStr);
    std::string data = properties->Encode();
    output.insert(output.end(), data.begin(), data.end());
    return ResponseCodes::OBEX_HTTP_OK;
}

static bool ParseImageNodeAttributes(xmlNodePtr node, std::shared_ptr<BipEncoding> &encoding,
    std::shared_ptr<Media::PixelMap> &pixel, int32_t &size, int32_t &maxSize)
{
    xmlChar *enc = xmlGetProp(node, BAD_CAST "encoding");
    if (enc != nullptr) {
        encoding = std::make_shared<BipEncoding>(std::string(reinterpret_cast<const char *>(enc)));
        xmlFree(enc);
    }
    xmlChar *pxl = xmlGetProp(node, BAD_CAST "pixel");
    if (pxl != nullptr) {
        std::string pixelStr(reinterpret_cast<const char *>(pxl));
        auto pos = pixelStr.find('*');
        if (pos != std::string::npos) {
            Media::InitializationOptions options;
            if (!ConvertStrToDigit(pixelStr.substr(0, pos), options.size.width)) {
                xmlFree(pxl);
                return false;
            }
            if (!ConvertStrToDigit(pixelStr.substr(pos + 1), options.size.height)) {
                xmlFree(pxl);
                return false;
            }
            pixel = Media::PixelMap::Create(options);
        }
        xmlFree(pxl);
    }
    xmlChar *sz = xmlGetProp(node, BAD_CAST "size");
    if (sz != nullptr) {
        if (!ConvertStrToDigit(std::string(reinterpret_cast<const char *>(sz)), size)) {
            xmlFree(sz);
            return false;
        }
        xmlFree(sz);
    }
    xmlChar *msz = xmlGetProp(node, BAD_CAST "maxsize");
    if (msz != nullptr) {
        if (!ConvertStrToDigit(std::string(reinterpret_cast<const char *>(msz)), maxSize)) {
            xmlFree(msz);
            return false;
        }
        xmlFree(msz);
    }
    return true;
}

static std::shared_ptr<BipImageDescriptor> ParseImageDescriptor(const std::vector<uint8_t> &data)
{
    if (data.empty()) {
        return nullptr;
    }
    std::string xmlStr(data.begin(), data.end());
    std::string endTag = "</image-descriptor>";
    auto endPos = xmlStr.find(endTag);
    if (endPos != std::string::npos) {
        xmlStr = xmlStr.substr(0, endPos + endTag.length());
    }
    xmlDocPtr doc = xmlReadMemory(xmlStr.c_str(), xmlStr.size(), "", nullptr, XML_PARSE_NOBLANKS);
    CHECK_AND_RETURN_LOG_RET(doc != nullptr, nullptr, "xmlReadMemory failed");

    std::shared_ptr<BipEncoding> encoding = nullptr;
    std::shared_ptr<Media::PixelMap> pixel = nullptr;
    int32_t size = -1;
    int32_t maxSize = -1;

    xmlNodePtr root = xmlDocGetRootElement(doc);
    for (xmlNodePtr node = root->children; node; node = node->next) {
        if (xmlStrcmp(node->name, BAD_CAST "image") == 0) {
            if (!ParseImageNodeAttributes(node, encoding, pixel, size, maxSize)) {
                xmlFreeDoc(doc);
                return nullptr;
            }
        }
    }
    xmlFreeDoc(doc);
    return std::make_shared<BipImageDescriptor>(encoding, pixel, size, maxSize);
}

int AvrcpBipObexServer::HandleGetImage(ObexHeader &request, ObexHeader &reply,
    std::vector<uint8_t> &output, bool &sendBodyHeader)
{
    HILOGI("get image start!");
    std::vector<uint8_t> imageHandle = request.GetHeader(HEADER_ID_IMG_HANDLE);
    std::vector<uint8_t> descriptorVec = request.GetHeader(HEADER_ID_IMG_DESCRIPTOR);
    CHECK_AND_RETURN_LOG_RET(!imageHandle.empty(), ResponseCodes::OBEX_HTTP_BAD_REQUEST, "image handle is null");
    std::shared_ptr<BipImageDescriptor> descriptor = nullptr;
    if (!descriptorVec.empty()) {
        descriptor = ParseImageDescriptor(descriptorVec);
    }
    std::string imageHandleStr = Utf16BeToUtf8String(imageHandle);
    imageHandleStr.erase(std::remove(imageHandleStr.begin(), imageHandleStr.end(), '\0'), imageHandleStr.end());
    CHECK_AND_RETURN_LOG_RET(IsImageHandleValid(imageHandleStr), ResponseCodes::OBEX_HTTP_PRECON_FAILED,
        "invalid image handle");
    auto coverArtStorage = AvrcpCoverArtStorage::GetInstance();
    CHECK_AND_RETURN_LOG_RET(coverArtStorage, ResponseCodes::OBEX_HTTP_PRECON_FAILED,
        "avrcpCoverArtStorage is null");
    std::shared_ptr<CoverArt> coverArt = coverArtStorage->GetImage(imageHandleStr);
    if (coverArt == nullptr) {
        HILOGE("No coverArt stored at handle");
        return ResponseCodes::OBEX_HTTP_NOT_FOUND;
    }
    std::shared_ptr<Media::PixelMap> image = nullptr;
    if (descriptor == nullptr) {
        image = coverArt->GetImage();
    } else {
        image = coverArt->GetImage(descriptor);
    }
    CHECK_AND_RETURN_LOG_RET(image, ResponseCodes::OBEX_HTTP_PRECON_FAILED,
        "Failed to serialize image with given format");
    std::vector<uint8_t> data;
    coverArt->GetImageData(image, data);
    output.insert(output.end(), data.begin(), data.end());
    return ResponseCodes::OBEX_HTTP_OK;
}

int AvrcpBipObexServer::OnConnect(ObexHeader &request, ObexHeader &reply)
{
    std::vector<uint8_t> uuid = request.GetHeader(TARGET);
    if (uuid != BLUETOOTH_UUID_AVRCP_COVER_ART) {
        HILOGE("onConnect - uuid didn't match. Not Acceptable.");
        return ResponseCodes::OBEX_HTTP_NOT_ACCEPTABLE;
    }
    reply.SetHeader(WHO, uuid);
    auto service = bipService_.lock();
    if (service) {
        service->SetBipClientStatus(RawAddress(deviceAddr_), true);
    }
    return ResponseCodes::OBEX_HTTP_OK;
}

void AvrcpBipObexServer::OnClose(bool isThreadStart)
{
    HILOGI("isThreadStart is %{public}d", isThreadStart);
    BipMessage event(BIP_DISCONNECT_EVT);
    event.dev_ = deviceAddr_;
    auto service = bipService_.lock();
    if (service) {
        service->PostEvent(event);
    }
}

int AvrcpBipObexServer::OnGet(ObexHeader &request, ObexHeader &reply,
    std::vector<uint8_t> &output, bool &sendBodyHeader)
{
    std::string type = request.GetType();
    if (type == TYPE_GET_LINKED_THUMBNAIL) {
        // Thumbnails and detailed images are processed in the same way.
        return HandleGetImage(request, reply, output, sendBodyHeader);
    } else if (type == TYPE_GET_IMAGE_PROPERTIES) {
        return HandleGetImageProperties(request, reply, output, sendBodyHeader);
    } else if (type == TYPE_GET_IMAGE) {
        return HandleGetImage(request, reply, output, sendBodyHeader);
    } else {
        return ResponseCodes::OBEX_HTTP_BAD_REQUEST;
    }
}

int AvrcpBipObexServer::OnPut(ObexHeader &request, ObexHeader &reply,
    std::vector<uint8_t> &input, bool &requestFinished)
{
    return ResponseCodes::OBEX_HTTP_NOT_IMPLEMENTED;
}

int AvrcpBipObexServer::OnDisconnect(ObexHeader &request, ObexHeader &reply)
{
    HILOGI("enter!");
    auto service = bipService_.lock();
    if (service) {
        service->SetBipClientStatus(RawAddress(deviceAddr_), false);
    }
    return ResponseCodes::OBEX_HTTP_OK;
}

int AvrcpBipObexServer::OnAbort(ObexHeader &request, ObexHeader &reply)
{
    return ResponseCodes::OBEX_HTTP_NOT_IMPLEMENTED;
}

int AvrcpBipObexServer::OnSetPath(ObexHeader &request, ObexHeader &reply, bool backup, bool create)
{
    return ResponseCodes::OBEX_HTTP_NOT_IMPLEMENTED;
}

int AvrcpBipObexServer::OnDelete(ObexHeader &request, ObexHeader &reply)
{
    return ResponseCodes::OBEX_HTTP_OK;
}

void AvrcpBipObexServer::SetConnectionId(int connectionId)
{
    connectionId_ = connectionId;
}

int AvrcpBipObexServer::GetConnectionId()
{
    return connectionId_;
}

} // namespace bluetooth
} // namespace OHOS