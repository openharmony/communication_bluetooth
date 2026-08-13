/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef BT_CHR_HCI_PROCESS_DATA_H
#define BT_CHR_HCI_PROCESS_DATA_H

#include <iostream>
#include <memory>
#include <string>
#include <mutex>
#include "safe_map.h"

namespace OHOS {
namespace bluetooth {

class BtChrHciProcessData {
public:
    static BtChrHciProcessData& GetInstance();
    void SaveImportantHciProcessData(const std::string& peerAddr, const std::string& processCode);
    const std::string GetImportantHciProcessData(const std::string& peerAddr);

private:
    BtChrHciProcessData();
    ~BtChrHciProcessData();
    void ClearHciDataByPeerAddr(const std::string& peerAddr);
    void SubmitClearTask(const std::string& peerAddr);

private:
    // <key:peerAddr, val:HCICODE[]>
    SafeMap<std::string, std::string> processDataMap_ {};
    SafeMap<std::string, int64_t> timestampMap_ {};
};
}  // namespace bluetooth
}  // namespace OHOS
#endif /* BT_CHR_HCI_PROCESS_DATA_H */