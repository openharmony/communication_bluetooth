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

#ifndef MAP_MSE_SERVICE_IMPL_H
#define MAP_MSE_SERVICE_IMPL_H

#include <map>
#include "bt_def.h"
#include "base_def.h"
#include "context.h"
#include "btcommon/message.h"
#include "btcommon/timer_manager.h"
#include "raw_address.h"
#include "map_mse_masinstance.h"
#include "map_mse_service_impl_interface.h"
#include "common_util.h"
#include "thread_util.h"
#include "adapter_device_config.h"
#include "safe_map.h"

namespace OHOS {
namespace bluetooth {
class MapMseServiceImpl : public MapMseServiceImplInterface {
public:
    std::function<int()> getDbIdentifierFunc_ = nullptr;
    std::function<int(const std::string &addr, const Uuid &uuid, int securityFlag, int type,
        int psm)> connectSocketFunc_ = nullptr;
    std::function<void(const bool isNeedReconnClient)> setIsNeedReconnClientFunc_ = nullptr;

    MapMseServiceImpl() = default;
    ~MapMseServiceImpl() override = default;

    void StartUp(const MapMnsSdpFoundRecord &record,
        std::shared_ptr<ObexSocketDevice> &socketDevice, const bool isReconnClient) override;

    void RegisterObserver(std::shared_ptr<IMapMseObserver> &observer) override;

    void DeregisterObserver(std::shared_ptr<IMapMseObserver> &observer) override;

    int Disconnect(const RawAddress &device) override;

    void OnClose(int32_t masInstanceId) override;

    std::list<RawAddress> GetConnectDevices() override;

    int32_t GetConnectState() override;

    int GetDeviceState(const RawAddress &device) override;

    void DllRegisterConnectSocketFunc(const std::function<int(const std::string &addr, const Uuid &uuid,
        int securityFlag, int type, int psm)> &connectSocketFunc) override;

    void DllRegisterGetDbIndetifierFunc(const std::function<int()> &func) override;

    void DllSetIsNeedReconnClientFunc(const std::function<void(const bool isNeedReconnClientFunc)> &func) override;
private:
    void DealOnClose(int32_t masInstanceId);
    void NotifyStateChanged(RawAddress address, int32_t state);

    std::string connectDevice_ = "";
    int32_t connectState_ = static_cast<int32_t>(BTConnectState::DISCONNECTED);

    std::shared_ptr<MapMseMnsClient> mnsClient_ = nullptr;
    std::shared_ptr<ObexSocketDevice> socketDevice_ = nullptr;
    std::shared_ptr<IMapMseObserver> mapMseConnectionObserver_;
    std::mutex socketDeviceLock_;
    SafeMap<int32_t, std::shared_ptr<MapMseMasInstance>> masInstanceMap_{};
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // MAP_MSE_SERVICE_IMPL_H
