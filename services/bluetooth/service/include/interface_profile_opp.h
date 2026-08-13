/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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

#ifndef INTERFACE_PROFILE_OPP_H
#define INTERFACE_PROFILE_OPP_H

#include "interface_profile.h"
#include "opp_transfer_information.h"

namespace OHOS {
namespace bluetooth {
/**
 * @brief obsever for pbap opp server
 * when opp server occur event, call these
 */
class IOppObserver {
public:

    /**
     * @brief deconstructor
     * @details deconstructor
     * @return
     * @since 6
     */
    virtual ~IOppObserver(){};

    /**
     * @brief  Receive a incoming file request.
     *
     * @param  info file information
     */
    virtual void OnReceiveIncomingFile(const IOppTransferInformation &info) {}
    /**
     * @brief  Notify the transfer's state changed.
     *
     * @param  info file information
     */
    virtual void OnTransferStateChange(const IOppTransferInformation &info) {}

    /**
     * @brief  Opp Continuous Task.
     *
     * @param  action connect or close
     */
    virtual void OppContinuousTask(const std::string &action) {}
};

/**
 * @brief opp service
 * opp service
 */
class IProfileOpp : public IProfile {
public:
    /**
     * @brief register observer
     *
     * @param observer the pointer that point to a OppObserver
     * @return void
     */
    virtual void RegisterObserver(IOppObserver &observer) = 0;

    /**
     * @brief deregister observer
     *
     * @param observer the pointer that point to a OppObserver
     * @return void
     */
    virtual void DeregisterObserver(IOppObserver &observer) = 0;

    /**
     * @brief SendFile
     *
     * @param device the destinst device
     * @param fileHolders sendFile infos
     * @return errorCode
     */
    virtual int SendFile(const RawAddress &device, const std::vector<IOppTransferFileHolder> &fileHolders) = 0;
    
    /**
     * @brief Get remote device list which are in the specified states.
     *
     * @param states  List of remote device states.
     * @return Returns the list of devices.
     */
    virtual std::vector<RawAddress> GetDevicesByStates(std::vector<int> states) = 0;

    /**
     * @brief Get Device State
     *
     * @param device  Remote device object.
     * @return Returns the connection state of the remote device.
     */
    virtual int GetDeviceState(const RawAddress &device) = 0;

    /**
     * @brief SetIncomingFileConfirmation.
     *
     * @param accept  accept or not.
     * @param fd  accept file fd.
     * @return result of Confirmation set.
     */
    virtual int SetIncomingFileConfirmation(const bool accept, int fd) = 0;

    /**
     * @brief Get Current TransferInformation.
     *
     * @return Current TransferInformation.
     */
    virtual IOppTransferInformation GetCurrentTransferInformation() = 0;

    /**
     * @brief CancelTransfer.
     *
     * @return cancel result.
     */
    virtual int CancelTransfer() = 0;

    /**
     * @brief Set LastReceived FileUri.
     *
     * @return result of Set LastReceived FileUri.
     */
    virtual int SetLastReceivedFileUri(const std::string &uri) = 0;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // INTERFACE_PROFILE_OPP_H