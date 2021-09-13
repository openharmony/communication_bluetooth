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

#ifndef SOCKET_GAP_MOCK_H
#define SOCKET_GAP_MOCK_H

#include "base_def.h"
#include "gap_if.h"

/**
 * @brief The bluetooth subsystem.
 */
namespace bluetooth {
class SocketGapMock {
public:
    /**
     * @brief Constructor.
     */
    SocketGapMock() = default;

    /**
     * @brief Destructor.
     */
    virtual ~SocketGapMock()
    {}

    /**
     * @brief Register Service Security
     *
     * @param scn server channel number
     * @param isServer is server or not
     * @param securityFlag require the connection to be encrypted and authenticated.
     * @return int
     */
    int RegisterServiceSecurity(const BtAddr *addr, GAP_SecMultiplexingProtocol protocolId, uint16_t scn, bool isServer,
        int securityFlag, GAP_Service serviceId);

    /**
     * @brief UnRegister Service Security
     *
     * @param serviceId service id
     * @return int
     */
    int UnregisterSecurity(const BtAddr *addr, uint16_t scn, GAP_Service serviceId, bool isServer);
};
}  // namespace bluetooth
#endif  // SOCKET_GAP_MOCK_H
