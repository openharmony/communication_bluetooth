/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef OPP_TRANSFER_INFORMATION_H
#define OPP_TRANSFER_INFORMATION_H

#include <cstdint>
#include <string>
#include "iosfwd"

namespace OHOS {
namespace bluetooth {
class IOppTransferInformation {
public:
    IOppTransferInformation() = default;
    virtual ~IOppTransferInformation() = default;

    int GetId() const;
    void SetId(int id);
    std::string GetFileName() const;
    void SetFileName(const std::string &fileName);
    std::string GetFilePath() const;
    void SetFilePath(const std::string &filePath);
    std::string GetFileType() const;
    void SetFileType(const std::string &fileType);
    std::string GetDeviceName() const;
    void SetDeviceName(const std::string &deviceName);
    std::string GetDeviceAddress() const;
    void SetDeviceAddress(const std::string &deviceAddress);
    int GetDirection() const;
    void SetDirection(int direction);
    int GetStatus() const;
    void SetStatus(int status);
    int GetFailedReason() const;
    void SetFailedReason(int failedReason);
    int GetFileFd() const;
    void SetFileFd(int fileFd);
    uint64_t GetTimeStamp() const;
    void SetTimeStamp(uint64_t timeStamp);
    uint64_t GetCurrentBytes() const;
    void SetCurrentBytes(uint64_t currentBytes);
    uint64_t GetTotalBytes() const;
    void SetTotalBytes(uint64_t totalBytes);
    int GetIsAccepct() const;
    void SetIsAccepct(int isAccept);
    int GetOperationNum() const;
    void SetOperationNum(int operationNum);
    int GetCurrentCount() const;
    void SetCurrentCount(int currentCount);
    int GetTotalCount() const;
    void SetTotalCount(int totalCount);

private:
    int id_ = -1;
    std::string fileName_;
    std::string filePath_;
    std::string fileType_;
    std::string deviceName_;
    std::string deviceAddress_;
    int direction_ = 0;
    int status_ = 0;
    int failedReason_ = 0;
    int fileFd_ = -1;
    int operationNum_ = 0;
    int isAccepct_ = false;
    uint64_t timeStamp_ = 0;
    uint64_t currentBytes_ = 0;
    uint64_t totalBytes_ = 0;
    int currentCount_ = 0;
    int totalCount_ = 0;
};

class IOppTransferFileHolder {
public:
    IOppTransferFileHolder() = default;
    virtual ~IOppTransferFileHolder() = default;
    IOppTransferFileHolder(const std::string &filePath, const int64_t &fileSize, const int32_t &fileFd);
    std::string GetFilePath() const;
    void SetFilePath(const std::string &filePath);
    int64_t GetFileSize() const;
    void SetFileSize(const int64_t &fileSize);
    int32_t GetFileFd() const;
    void SetFileFd(const int32_t &fileFd);
private:
    std::string filePath_ = "";
    int64_t fileSize_ = 0;
    int32_t fileFd_ = 0;
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // OPP_TRANSFER_INFORMATION_H