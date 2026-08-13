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

#ifndef BLUETOOTH_OPP_SERVER_H
#define BLUETOOTH_OPP_SERVER_H

#include "bluetooth_opp_stub.h"
#include "bluetooth_types.h"
#include "i_bluetooth_opp.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothOppServer : public BluetoothOppStub {
public:
    /**
     * @brief constructor
     * @details constructor
     */
    explicit BluetoothOppServer();

    /**
     * @brief deconstructor
     * @details deconstructor
     */
    ~BluetoothOppServer() override;

    /**
     * @brief register observer
     * @details register observer for the service of phone book
     * @param observer the pointer that point to a IBluetoothOppObserver
     * @return Status
     */
    void RegisterObserver(const sptr<IBluetoothOppObserver> &observer) override;

    /**
     * @brief deregister observer
     * @details deregister observer for the service of phone book
     * @param observer the pointer that point to a IBluetoothOppObserver
     * @return Status
     */
    void DeregisterObserver(const sptr<IBluetoothOppObserver> &observer) override;

    /**
     * @brief get the remote devices
     * @details get the remote device with the specified states
     * @param states states
     * @return Status
     */
    int32_t GetDevicesByStates(const std::vector<int32_t> &states,
        std::vector<BluetoothRawAddress> &rawDevices) override;

    /**
     * @brief get the state of device
     * @details get the state with the specified remote device
     * @param device  remote device
     * @return Status
     */
    int32_t GetDeviceState(const BluetoothRawAddress &device, int32_t &state) override;

    /**
     * @brief SendFile
     * @details SendFile to remote device
     * @param device  remote device
     * @param filePaths  filePaths
     * @param mimeTypes  mimeTypes
     * @param result  result
     * @return Status
     */
    int32_t SendFile(const std::string &device,
        const std::vector<BluetoothIOppTransferFileHolder> &fileHolders, bool& result) override;

    /**
     * @brief Set IncomingFile Confirmation or not
     *
     * @param accept accept or not.
     * @param fd accept fileFd,
     * @return result.
     */
    int32_t SetIncomingFileConfirmation(bool accept, int fd) override;

    /**
     * @brief GetCurrent TransferInformation.
     *
     * @param transferInformation GetCurrent Transfer Information.
     * @return result.
     */
    int32_t GetCurrentTransferInformation(BluetoothIOppTransferInformation &transferInformation) override;

    /**
     * @brief CancelTransfer
     *
     * @return result.
     */
    int32_t CancelTransfer() override;

    /**
     * @brief Set LastReceivedFileUri
     *
     * @param uri LastReceivedFileUri bluetooth service not open it
     * @return result.
     */
    int32_t SetLastReceivedFileUri(const std::string &uri) override;

private:
    BLUETOOTH_DECLARE_IMPL();
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothOppServer);
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // BLUETOOTH_OPP_SERVER_H
