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
 
#ifndef BT_CHR_PKGNAME_MANAGER_H
#define BT_CHR_PKGNAME_MANAGER_H
#include <set>
#include <mutex>
 
namespace OHOS {
namespace bluetooth {
 
enum BtChrBusinessType {
    BUSINESS_TYPE_CALL = 0,
    BUSINESS_TYPE_MEDIA,
    BUSINESS_TYPE_MAX_VALUE = 0xF,
};
 
struct BtChrPkgNameInfo {
    std::string pkgName;
    BtChrBusinessType businessType;
    std::set<uint32_t> streamIds;
    bool operator<(const BtChrPkgNameInfo& other) const {
        if (businessType != other.businessType) {
            return businessType < other.businessType;
        }
        return pkgName < other.pkgName;
    }
 
    bool operator==(const BtChrPkgNameInfo& other) const {
        return businessType == other.businessType && pkgName == other.pkgName;
    }
};
 
class BtChrPkgNameManager {
public:
    static BtChrPkgNameManager& GetInstance();
    void UpdateRenderPkgName(const std::string &bundleName, const bool isRendererRunning,
        const BtChrBusinessType businessType, uint32_t streamId);
    std::string GetRenderPkgName(const BtChrBusinessType businessType);
private:
    BtChrPkgNameManager() = default;
    ~BtChrPkgNameManager() = default;
    std::set<BtChrPkgNameInfo> renderPkgNameSet_;
    std::mutex renderPkgNameSetMutex_;
    void SetRenderPkgName(const BtChrPkgNameInfo& pkgNameInfo);
    void DeleteRenderPkgName(const BtChrPkgNameInfo& pkgNameInfo);
};
 
}  // namespace bluetooth
}  // namespace OHOS
#endif /* BT_CHR_PKGNAME_MANAGER_H */