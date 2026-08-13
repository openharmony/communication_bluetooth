/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"){}
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
#define LOG_TAG "bt_service_map_mse_floder"
#endif

#include "map_mse_folder.h"
#include <vector>
#include "log.h"
#include "bluetooth_code_convert_utils.h"

namespace OHOS {
namespace bluetooth {
std::shared_ptr<MapMseFolder> MapMseFolder::AddFolder(const std::string &name)
{
    if (!subFolders_.count(name)) {
        subFolders_[name] = std::make_shared<MapMseFolder>(name, shared_from_this());
    }
    return subFolders_[name];
}

std::shared_ptr<MapMseFolder> MapMseFolder::AddSmsMmsFolder(const std::string &name)
{
    return AddFolder(name);
}

std::shared_ptr<MapMseFolder> MapMseFolder::GetSubFolder(const std::string &name)
{
    if (subFolders_.count(name)) {
        return subFolders_[name];
    }
    return nullptr;
}

std::shared_ptr<MapMseFolder> MapMseFolder::GetParent()
{
    return parent_;
}

std::shared_ptr<MapMseFolder> MapMseFolder::GetRoot()
{
    std::shared_ptr<MapMseFolder> root = shared_from_this();
    while (root->parent_ != nullptr) {
        root = root->parent_;
    }
    return root;
}

std::string MapMseFolder::GetName()
{
    return name_;
}

std::string MapMseFolder::GetEncodeData(int32_t offset, int32_t count)
{
    std::vector<std::shared_ptr<MapMseFolder>> folders;
    for (auto it = subFolders_.begin(); it != subFolders_.end(); ++it) {
        folders.push_back(it->second);
    }

    if (offset > static_cast<int32_t>(folders.size())) {
        HILOGE("ofset > folders.size()");
        return "";
    }
    int stopIndex = offset + count;
    if (stopIndex > static_cast<int32_t>(folders.size())) {
        stopIndex = static_cast<int32_t>(folders.size());
    }
    std::string xml = "<?xml version='1.0' encoding='utf-8' standalone='yes' ?>";
    xml.append("<folder-listing version=\"1.0\">");
    for (int i = offset; i < stopIndex; i++) {
        xml.append("<folder name=\"");
        xml.append(folders[i]->name_);
        xml.append("\"/>");
    }
    xml.append("</folder-listing>");
    std::string utf8xml = BluetoothCodeConvertUtils::AsciiToUtf8(xml);
    return utf8xml;
}

int32_t MapMseFolder::GetSubFolderSize()
{
    return static_cast<int32_t>(subFolders_.size());
}

void MapMseFolder::SetIgnore(bool ignore)
{
    ignore_ = ignore;
}

bool MapMseFolder::GetIgnore()
{
    return ignore_;
}
}  // namespace bluetooth
}  // namespace OHOS