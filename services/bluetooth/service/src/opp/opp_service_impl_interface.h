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

#ifndef OPP_SERVICE_IMPL_INTERFACE_H
#define OPP_SERVICE_IMPL_INTERFACE_H

#include "raw_address.h"
#include "interface_profile_opp.h"
#include "obex_socket_device.h"
#include "bt_uuid.h"

namespace OHOS {
namespace bluetooth {
class OppServiceImplInterface {
public:
    OppServiceImplInterface() = default;
    virtual ~OppServiceImplInterface() = default;

    /**
     * @brief register observer
     *
     * @param observer the pointer that point to a OppObserver
     * @return void
     */
    virtual void RegisterObserver(std::shared_ptr<IOppObserver> &observer) = 0;

    /**
     * @brief deregister observer
     *
     * @return void
     */
    virtual void DeregisterObserver() = 0;

        /**
     * @brief get the remote devices with the specified states
     *
     * @param states states
     * @return std::vector remote devices
     */
    virtual std::vector<RawAddress> GetDevicesByStates(const std::vector<int32_t> &states) = 0;

    /**
     * @brief get the state of remote device
     *
     * @param device  remote device
     * @return int state
     */
    virtual int32_t GetDeviceState(const RawAddress &device) = 0;

    /**
     * @brief GetCurrentTransferInformation
     *
     * @return IOppTransferInformation
     */
    virtual IOppTransferInformation GetCurrentTransferInformation() = 0;

    /**
     * @brief sdp found result callback
     *
     */
    virtual void SdpFoundCallback(const OppSdpFoundRecord &record) = 0;

    /**
     * @brief SendFile
     *
     * @param device the destinst device
     * @param fileHolders sendFile infos
     * @return errorCode
     */
    virtual int SendFile(const RawAddress &device, const std::vector<IOppTransferFileHolder> &fileHolders) = 0;

    /**
     * @brief CancelTransfer
     *
     * @return result
     */
    virtual int CancelTransfer() = 0;

    /**
     * @brief RemoveAllDeviceTransfer
     * @param device remove device
     *
     * @return result
     */
    virtual void RemoveAllDeviceTransfer(const std::string &device) = 0;

    /**
     * @brief SetIncomingFileConfirmation
     *
     * @param accept accept or not
     * @param fd  accept file fd.
     * @return result
     */
    virtual int SetIncomingFileConfirmation(const bool accept, int fd) = 0;

    /**
     * @brief SetLastReceivedFileUri
     *
     * @param uri last received fileuri
     * @return result
     */
    virtual int SetLastReceivedFileUri(const std::string &uri) = 0;

    /**
     * @brief HandlerConnectEvt
     *
     * @param socketDevice socketDevice
     * @return result
     */
    virtual void HandlerConnectEvt(std::shared_ptr<ObexSocketDevice> &socketDevice) = 0;

    /**
     * @brief DllRegisterConnectSocketFunc
     *
     */
    virtual void DllRegisterConnectSocketFunc(const std::function<int(const std::string &addr, const Uuid &uuid,
        int securityFlag, int type, int psm)> &connectSocketFunc) = 0;

    /**
     * @brief DllRegisterUnloadOppFunc
     *
     */
    virtual void DllRegisterUnloadOppFunc(const std::function<bool()> &unloadOppFunc) = 0;

    /**
     * @brief DllRegisterCreateOppSdpFunc
     *
     */
    virtual void DllRegisterCreateOppSdpFunc(const std::function<void(const std::string &addr)> &createOppSdpFunc) = 0;

    /**
     * @brief DllRegisterOppContinuousTaskFunc
     *
     */
    virtual void DllRegisterOppContinuousTaskFunc(const std::function<void(const std::string &action)>
        &oppContinuousTaskFunc) = 0;

    /**
     * @brief DllRegisterIsAllowedOppReadFunc
     *
     */
    virtual void DllRegisterIsAllowedOppReadFunc(const std::function<bool(const std::string &addr)>
        &isAllowedOppReadFunc) = 0;

    /**
     * @brief DllRegisterAclDisConnEventFunc
     *
     */
    virtual void DllRegisterAclDisConnEventFunc(const std::function<void(const std::string &addr)>
        &aclDisConnEventFunc) = 0;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // OPP_SERVICE_IMPL_INTERFACE_H