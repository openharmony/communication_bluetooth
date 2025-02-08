/*
 * Copyright (C) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_BLUETOOTH_STANDARD_SOCKET_INTERFACE_H
#define OHOS_BLUETOOTH_STANDARD_SOCKET_INTERFACE_H

#include <string>

#include "bluetooth_service_ipc_interface_code.h"
#include "../common/bt_uuid.h"
#include "bluetooth_socket_coc.h"
#include "iremote_broker.h"
#include "i_bluetooth_socket_observer.h"

namespace OHOS {
namespace Bluetooth {
struct ListenSocketParam {
    std::string name;
    bluetooth::Uuid uuid;
    int32_t securityFlag;
    int32_t type;
    sptr<IBluetoothServerSocketObserver> observer;
};

struct ConnectSocketParam {
    std::string addr;
    bluetooth::Uuid uuid;
    int32_t securityFlag;
    int32_t type;
    int32_t psm;
};

class IBluetoothSocket : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.ipc.IBluetoothSocket");

    virtual int Connect(ConnectSocketParam &param, int &fd) = 0;
    virtual int Listen(ListenSocketParam &param, int &fd) = 0;
    virtual int DeregisterServerObserver(const sptr<IBluetoothServerSocketObserver> &observer) = 0;
    virtual int RegisterClientObserver(const BluetoothRawAddress &dev, const bluetooth::Uuid uuid,
        const sptr<IBluetoothClientSocketObserver> &observer) = 0;
    virtual int DeregisterClientObserver(const BluetoothRawAddress &dev, const bluetooth::Uuid uuid,
        const sptr<IBluetoothClientSocketObserver> &observer) = 0;
    virtual int UpdateCocConnectionParams(const BluetoothSocketCocInfo &info) = 0;
    virtual int IsAllowSocketConnect(int socketType, const std::string &addr, bool &isAllowed) = 0;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_SOCKET_INTERFACE_H