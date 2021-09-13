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

#include "gatt_cache.h"
#include "common_mock_all.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace bluetooth;

extern AdapterManagerMock *g_amMock_;

TEST(gatt_cache_test, AddService_test)
{
    uint16_t handle = 0x13;
    uint16_t endHandle = 1;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00805F9B34FC");
    struct bluetooth::GattCache::Service service(true, handle, endHandle, uuid);
    bluetooth::GattCache gattcache;

    uint16_t handle2 = 0x13;
    const bluetooth::Uuid uuid2 = Uuid::ConvertFromString("00001830-0000-1000-8000-11111F9B33FC");
    struct bluetooth::GattCache::Service service2(true, handle2, endHandle, uuid2);

    gattcache.AddService(service);
    gattcache.AddService(service2);
}

TEST(gatt_cache_test, Clear)
{
    bluetooth::GattCache gattcache;
    gattcache.Clear();
}

TEST(gatt_cache_test, AddIncludeService_test1)
{
    uint16_t handle1 = 0x16;
    uint16_t endHandle1 = 1;
    const bluetooth::Uuid uuid1 = Uuid::ConvertFromString("0000001");
    struct bluetooth::GattCache::Service service(true, handle1, endHandle1, uuid1);
    bluetooth::GattCache gattcache;
    gattcache.AddService(service);

    uint16_t handle = 0x16;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830");
    uint16_t includehandle = 0x16;
    uint16_t startHandle = 0x13;
    uint16_t endHandle = 0x16;
    struct bluetooth::GattCache::IncludeService includeservice(includehandle, startHandle, endHandle, uuid);
    int result = gattcache.AddIncludeService(handle, includeservice);
    EXPECT_EQ(0, result);

    struct bluetooth::GattCache::IncludeService includeservice1(includehandle, startHandle, endHandle);
    int result1 = gattcache.AddIncludeService(handle, includeservice1);
    EXPECT_EQ(0, result1);

    struct bluetooth::GattCache::IncludeService includeservice2(uuid);
    int result2 = gattcache.AddIncludeService(handle, includeservice2);
    EXPECT_EQ(0, result2);

    struct bluetooth::Service src(handle);
    struct bluetooth::GattCache::IncludeService includeservice3(src);
    int result3 = gattcache.AddIncludeService(handle, includeservice3);
    EXPECT_EQ(0, result3);
}

TEST(gatt_cache_test, AddIncludeService_test2)
{
    uint16_t handle = 0x16;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830");
    uint16_t includehandle = 0x16;
    uint16_t startHandle = 0x13;
    uint16_t endHandle = 0x16;
    struct bluetooth::GattCache::IncludeService includeservice(includehandle, startHandle, endHandle, uuid);
    bluetooth::GattCache gattcache;
    int result = gattcache.AddIncludeService(handle, includeservice);
    EXPECT_EQ(-20, result);
}

TEST(gatt_cache_test, AddCharacteristic_test)
{
    uint16_t serviceHandle = 0x13;
    uint16_t handle = 0x16;
    uint8_t properties = 1;
    uint16_t valueHandle = 0x16;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00686F9B34Fk");

    bluetooth::GattCache::Characteristic characteristic(handle, properties, valueHandle, uuid);
    bluetooth::GattCache gattcache;
    int result = gattcache.AddCharacteristic(serviceHandle, characteristic);
    EXPECT_EQ(-20, result);
}

TEST(gatt_cache_test, AddDescriptor_test)
{
    uint16_t cccHandle = 0x16;
    uint16_t handle = 0x17;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00686F9B34Fk");

    bluetooth::GattCache::Descriptor descriptor(handle, uuid);
    bluetooth::GattCache gattcache;
    int result = gattcache.AddDescriptor(cccHandle, descriptor);
    EXPECT_EQ(-20, result);
}

TEST(gatt_cache_test, GATT_SUCCESS_test)
{
    uint16_t handle1 = 0x13;
    uint16_t endHandle1 = 1;
    const bluetooth::Uuid uuid1 = Uuid::ConvertFromString("0000001");
    struct bluetooth::GattCache::Service service(true, handle1, endHandle1, uuid1);
    bluetooth::GattCache gattcache;
    gattcache.AddService(service);

    uint16_t serviceHandle = 0x13;
    uint16_t handle = 0x16;
    uint8_t properties = 1;
    uint16_t valueHandle = 0x16;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("00001830-0000-1000-8000-00686F9B34Fk");
    bluetooth::GattCache::Characteristic characteristic(handle, properties, valueHandle, uuid);
    int result1 = gattcache.AddCharacteristic(serviceHandle, characteristic);
    EXPECT_EQ(0, result1);
    int result2 = gattcache.AddCharacteristic(serviceHandle, characteristic);
    EXPECT_EQ(0, result2);

    uint16_t cccHandle = 0x16;
    uint16_t handle_d = 0x16;
    const bluetooth::Uuid uuid_d = Uuid::ConvertFromString("00001830023");
    bluetooth::GattCache::Descriptor descriptor(handle_d, uuid_d);
    int result = gattcache.AddDescriptor(cccHandle, descriptor);
    EXPECT_EQ(0, result);

    gattcache.GetCharacteristic(valueHandle);
    gattcache.GetDescriptor(valueHandle);
    gattcache.GetIncludeServices(serviceHandle);
    gattcache.GetCharacteristics(serviceHandle);

    uint16_t characteristicsHandle = 0x16;
    gattcache.GetDescriptors(characteristicsHandle);
}

TEST(gatt_cache_test, GetCharacteristic_test)
{
    uint16_t valueHandle = 0x16;

    bluetooth::GattCache gattcache;
    gattcache.GetCharacteristic(valueHandle);
}

TEST(gatt_cache_test, GetDescriptor_test)
{
    uint16_t valueHandle = 0x16;

    bluetooth::GattCache gattcache;
    gattcache.GetDescriptor(valueHandle);
}

TEST(gatt_cache_test, GetIncludeServices_test)
{
    uint16_t serviceHandle = 0x13;
    bluetooth::GattCache gattcache;
    gattcache.GetIncludeServices(serviceHandle);
}

TEST(gatt_cache_test, GetCharacteristics_test)
{
    uint16_t serviceHandle = 0x13;
    bluetooth::GattCache gattcache;
    gattcache.GetCharacteristics(serviceHandle);
}

TEST(gatt_cache_test, GetDescriptors_test)
{
    uint16_t characteristicsHandle = 0x13;
    bluetooth::GattCache gattcache;
    gattcache.GetDescriptors(characteristicsHandle);
}

TEST(gatt_cache_test, GetServices_test)
{
    bluetooth::GattCache gattcache;
    gattcache.GetServices();
}

TEST(gatt_cache_test, StoredToFile_test)
{
    uint16_t handle1 = 0x03;
    uint16_t endHandle1 = 1;
    const bluetooth::Uuid uuid1 = Uuid::ConvertFromString("0000001");
    struct bluetooth::GattCache::Service service(true, handle1, endHandle1, uuid1);
    bluetooth::GattCache gattcache;
    gattcache.AddService(service);

    const bluetooth::Uuid include_uuid1 = Uuid::ConvertFromString("0000003");
    uint16_t include_includehandle1 = 0x06;
    uint16_t include_startHandle1 = 0x03;
    uint16_t include_endHandle1 = 0x06;
    struct bluetooth::GattCache::IncludeService includeservice(
        include_includehandle1, include_startHandle1, include_endHandle1, include_uuid1);
    gattcache.AddIncludeService(0x03, includeservice);

    uint16_t ccc_handle1 = 0x06;
    uint8_t ccc_properties1 = 1;
    uint16_t ccc_valueHandle1 = 0x16;
    const bluetooth::Uuid ccc_uuid1 = Uuid::ConvertFromString("00001830-0000-1000-8000-00686F9B3434");
    bluetooth::GattCache::Characteristic characteristic(ccc_handle1, ccc_properties1, ccc_valueHandle1, ccc_uuid1);
    gattcache.AddCharacteristic(0x03, characteristic);

    uint16_t desc_handle1 = 0x17;
    const bluetooth::Uuid desc_uuid1 = Uuid::ConvertFromString("00001830-0000-1000-8000-00686F9B3400");
    bluetooth::GattCache::Descriptor descriptor(desc_handle1, desc_uuid1);
    gattcache.AddDescriptor(0x06, descriptor);

    GattDevice dev(RawAddress("00:00:00:00:00:01"), GATT_TRANSPORT_TYPE_CLASSIC);
    EXPECT_EQ(GATT_SUCCESS, gattcache.StoredToFile(dev));
    EXPECT_EQ(GATT_SUCCESS, gattcache.LoadFromFile(dev));
}

TEST(gatt_cache_test, GetCharacteristicEndHandle_test)
{
    uint16_t handle1 = 0x23;
    uint16_t endHandle1 = 0x01;
    const bluetooth::Uuid uuid1 = Uuid::ConvertFromString("00000011");
    struct bluetooth::GattCache::Service service(true, handle1, endHandle1, uuid1);
    bluetooth::GattCache gattcache;
    gattcache.AddService(service);

    uint16_t ccc_handle1 = 0x26;
    uint8_t ccc_properties1 = 1;
    uint16_t ccc_valueHandle1 = 0x16;
    const bluetooth::Uuid ccc_uuid1 = Uuid::ConvertFromString("00001830-0000-1000-8000-00686F9B3411");
    bluetooth::GattCache::Characteristic characteristic(ccc_handle1, ccc_properties1, ccc_valueHandle1, ccc_uuid1);
    gattcache.AddCharacteristic(0x23, characteristic);

    uint16_t serviceHandle = 0x23;
    uint16_t characteristicsHandle = 0x26;
    gattcache.GetCharacteristicEndHandle(serviceHandle,characteristicsHandle);
}

