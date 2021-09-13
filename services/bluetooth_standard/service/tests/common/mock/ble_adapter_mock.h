/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef BLE_ADAPTER_MOCK_H
#define BLE_ADAPTER_MOCK_H

#include "interface_adapter.h"
#include "context.h"

#include <memory>
#include <map>

/*
 * @brief The Bluetooth subsystem.
 */
namespace bluetooth {
/**
 *  @brief BLE Adpter implementation class
 */
class BleAdapter : public IAdapter, public utility::Context {
public:
    BleAdapter();

    ~BleAdapter();
    utility::Context *GetContext() override;

    void Enable() override;
    void Disable() override;
    void PostEnable() override;

    std::string GetLocalAddress() const override;
    std::string GetLocalName() const override;
    bool SetLocalName(const std::string &name) const override;
    bool SetBondableMode(int mode) const override;
    int GetBondableMode() const override;
    // remote device information
    int GetDeviceType(const RawAddress &device) const override;
    std::string GetDeviceName(const RawAddress &device) const override;
    std::vector<Uuid> GetDeviceUuids(const RawAddress &device) const override;
    // pair
    std::vector<RawAddress> GetPairedDevices() const override;
    bool StartPair(const RawAddress &device) override;
    bool IsBondedFromLocal(const RawAddress &device) const override;
    bool CancelPairing(const RawAddress &device) override;
    bool RemovePair(const RawAddress &device) override;
    bool RemoveAllPairs() override;
    int GetPairState(const RawAddress &device) const override;
    bool SetDevicePairingConfirmation(const RawAddress &device, bool accept) const override;
    bool SetDevicePasskey(const RawAddress &device, int passkey, bool accept) const override;
    bool PairRequestReply(const RawAddress &device, bool accept) const override;
    // other
    bool IsAclConnected(const RawAddress &device) const override;
    bool IsAclEncrypted(const RawAddress &device) const override;

private:
    void EnableNative();
    void DisableNative();
};

};  // namespace bluetooth

#endif  /// BLE_ADAPTER_MOCK_H
