/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef MAP_MSE_FOLDER_H
#define MAP_MSE_FOLDER_H

#include <string>
#include <memory>
#include <map>

namespace OHOS {
namespace bluetooth {
const std::string MAP_MSE_FOLDER_NAME_INBOX = "inbox";
const std::string MAP_MSE_FOLDER_NAME_SENT = "sent";
const std::string MAP_MSE_FOLDER_NAME_OUTBOX = "outbox";
const std::string MAP_MSE_FOLDER_NAME_DRAFT = "draft";
const std::string MAP_MSE_FOLDER_NAME_DELETED = "deleted";
const std::string MAP_MSE_FOLDER_NAME_OTHER = "other";

class MapMseFolder : public std::enable_shared_from_this<MapMseFolder> {
public:
    MapMseFolder(const std::string &name, std::shared_ptr<MapMseFolder> parent) : name_(name), parent_(parent){};
    std::shared_ptr<MapMseFolder> AddFolder(const std::string &name);
    std::shared_ptr<MapMseFolder> AddSmsMmsFolder(const std::string &name);
    std::shared_ptr<MapMseFolder> GetSubFolder(const std::string &name);
    std::shared_ptr<MapMseFolder> GetParent();
    std::shared_ptr<MapMseFolder> GetRoot();
    std::string GetName();
    std::string GetEncodeData(int32_t offset, int32_t count);
    int32_t GetSubFolderSize();
    void SetIgnore(bool ignore);
    bool GetIgnore();

private:
    std::string name_;
    std::shared_ptr<MapMseFolder> parent_;
    std::map<std::string, std::shared_ptr<MapMseFolder>> subFolders_;
    bool ignore_ = false;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif