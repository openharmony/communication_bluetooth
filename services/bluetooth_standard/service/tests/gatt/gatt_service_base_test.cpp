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

#include "gatt_service_base.h"
#include "gatt_connection_manager.h"
#include "interface_profile.h"
#include "common_mock_all.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace bluetooth;

TEST(gatt_service_base_test, ConvertConnectionState)
{
    std::string connecting_state = "CONNECTING";
    std::string connected_state = "CONNECTED";
    std::string disconnecting_state = "DISCONNECTING";
    std::string disconnected_state = "1111";
    class bluetooth::GattServiceBase gattservicebase;
    int connecting_result = gattservicebase.ConvertConnectionState(connecting_state);
    int connected_result = gattservicebase.ConvertConnectionState(connected_state);
    int disconnecting_result = gattservicebase.ConvertConnectionState(disconnecting_state);
    int disconnected_result = gattservicebase.ConvertConnectionState(disconnected_state);
    EXPECT_EQ(connecting_result, 0);
    EXPECT_EQ(connected_result, 1);
    EXPECT_EQ(disconnecting_result, 2);
    EXPECT_EQ(disconnected_result, 3);
}

TEST(gatt_service_base_test, BuildBuffer)
{
    size_t length = 3;
    uint8_t *value = new uint8_t[length];
    class bluetooth::GattServiceBase gattservicebase;
    gattservicebase.BuildBuffer(value, length);
}

TEST(gatt_service_base_test, MoveToSharedPtr)
{
    size_t length = 2;
    std::unique_ptr<uint8_t[]> value = std::make_unique<uint8_t[]>(length);
    class bluetooth::GattServiceBase gattservicebase;
    gattservicebase.MoveToGattValue(value);
}

TEST(gatt_service_base_test, BuildSharedPtr)
{
    size_t length = 3;
    uint8_t *value = new uint8_t[length - 1];
    class bluetooth::GattServiceBase gattservicebase;
    gattservicebase.BuildGattValue(value, length);
}