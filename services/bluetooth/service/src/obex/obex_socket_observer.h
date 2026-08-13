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

#ifndef OBEX_SOCKET_OBSERVER_H
#define OBEX_SOCKET_OBSERVER_H

#include <list>
#include "obex_socket_device.h"

namespace OHOS {
namespace bluetooth {
/**
 * @brief Represents basic profile for each profile service, including the common functions.
 *
 * @since 6
 */
class IObexSocketObserver {
public:
    /**
     * @brief A destructor used to delete the <b>IObexSocketObserver</b> instance.
     *
     * @since 6
     */
    virtual ~IObexSocketObserver() = default;

    /**
     * @brief The callback function to notify Connect device.
     *
     * @param device Remote device address.
     * @return Returns Result for connect operation.
     * @since 6
     */
    virtual void OnConnect(std::shared_ptr<ObexSocketDevice> &device) = 0;
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // OBEX_SOCKET_OBSERVER_H