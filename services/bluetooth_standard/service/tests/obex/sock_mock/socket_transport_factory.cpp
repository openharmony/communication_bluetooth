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

#include "log.h"
#include "socket_transport_mock.h"
#include "transport_factory.h"
namespace bluetooth {
std::unique_ptr<DataTransport> TransportFactory::CreateRfcommTransport(
    const RawAddress *addr, uint8_t scn, uint16_t mtu, DataTransportObserver &observer, utility::Dispatcher &dispatcher)
{
    return std::make_unique<MockSockTransport>(addr, scn, scn, mtu, observer, MockSockTransport::TransportType::RFCOMM);
}

std::unique_ptr<DataTransport> TransportFactory::CreateL2capTransport(L2capTransportInfo &createInfo)
{
    return std::make_unique<MockSockTransport>(createInfo.addr,
        createInfo.lpsm,
        createInfo.rpsm,
        createInfo.mtu,
        createInfo.observer,
        MockSockTransport::TransportType::L2CAP);
}
}  // namespace bluetooth
