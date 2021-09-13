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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "gatt_defines.h"
#include "bt_def.h"
#include "gatt_database.h"
#include "common_mock_all.h"
#include <algorithm>
#include <functional>
#include <optional>

using namespace bluetooth;

TEST(gatt_database_test, AddService_test1)
{
    uint16_t handle = 65535;
    uint16_t starthandle = 65535;
    uint16_t endHandle = 65535;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C7");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);
    GattDatabase gattdatabase;

    int result = gattdatabase.AddService(service);
    EXPECT_EQ(result, GATT_SUCCESS);
};

TEST(gatt_database_test, AddService_test2)
{
    uint16_t handle = 0x13;
    uint16_t starthandle = 0x2;
    uint16_t endHandle = 0x3;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C1");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);

    uint16_t include_handle = 0x15;
    uint16_t include_starthandle = 0x1;
    uint16_t include_endHandle = 0x1;
    const bluetooth::Uuid include_uuid = Uuid::ConvertFromString("3F28CB71571C1");
    struct bluetooth::Service include_service(include_uuid, include_handle, include_starthandle, include_endHandle);
    service.includeServices_.push_back(include_service);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("10001");
    uint16_t ccc_handle = 0x13;
    int ccc_properties = 10;
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties);

    uint16_t desc_handle = 0x13;
    const bluetooth::Uuid desc_uuid = Uuid::ConvertFromString("10001");
    int desc_permissions = 10;
    bluetooth::Descriptor desc(desc_handle, desc_uuid, desc_permissions);
    ccc.descriptors_.push_back(desc);
    service.characteristics_.push_back(ccc);

    GattDatabase gattdatabase1;
    int result = gattdatabase1.AddService(service);
    EXPECT_EQ(result, GATT_SUCCESS);
};

TEST(gatt_database_test, DeleteService_test1)
{
    uint16_t handle = 0x13;
    uint16_t starthandle = 0x2;
    uint16_t endHandle = 0x3;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C72");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);
    GattDatabase gattdatabase;

    int result = gattdatabase.AddService(service);
    EXPECT_EQ(result, GATT_SUCCESS);

    uint16_t handle_delete = 1;
    int result_delete = gattdatabase.DeleteService(handle_delete);
    EXPECT_EQ(result_delete, GATT_SUCCESS);

    uint16_t handle_delete2 = 0x13;
    int result_delete2 = gattdatabase.DeleteService(handle_delete2);
    EXPECT_EQ(result_delete2, INVALID_PARAMETER);
}

TEST(gatt_database_test, DeleteService_test2)
{
    uint16_t handle = 0x13;
    uint16_t starthandle = 0x2;
    uint16_t endHandle = 0x3;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C2");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);

    uint16_t include_handle = 0x15;
    uint16_t include_starthandle = 0x1;
    uint16_t include_endHandle = 0x1;
    const bluetooth::Uuid include_uuid = Uuid::ConvertFromString("3F28CB71571C2");
    struct bluetooth::Service include_service(include_uuid, include_handle, include_starthandle, include_endHandle);
    service.includeServices_.push_back(include_service);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("10002");
    uint16_t ccc_handle = 0x13;
    int ccc_properties = 11;
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties);

    uint16_t desc_handle = 0x13;
    const bluetooth::Uuid desc_uuid = Uuid::ConvertFromString("10002");
    int desc_permissions = 11;
    bluetooth::Descriptor desc(desc_handle, desc_uuid, desc_permissions);
    ccc.descriptors_.push_back(desc);
    service.characteristics_.push_back(ccc);

    GattDatabase gattdatabase2;
    gattdatabase2.AddService(service);

    uint16_t deleteHandle1 = 1;
    int result_delete2 = gattdatabase2.DeleteService(deleteHandle1);
    EXPECT_EQ(result_delete2, REFERENCED_BY_OTHER_SERVICE);
}

TEST(gatt_database_test, DeleteService_test3)  
{
    uint16_t handle = 65535;
    uint16_t starthandle = 0x2;
    uint16_t endHandle = 0x3;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C3");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("10003");
    uint16_t ccc_handle = 0x13;
    int ccc_properties = 10;
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties);

    uint16_t desc_handle = 0x13;
    const bluetooth::Uuid desc_uuid = Uuid::ConvertFromString("10003");
    int desc_permissions = 10;
    bluetooth::Descriptor desc(desc_handle, desc_uuid, desc_permissions);
    ccc.descriptors_.push_back(desc);
    service.characteristics_.push_back(ccc);

    GattDatabase gattdatabase3;
    gattdatabase3.AddService(service);

    uint16_t deleteHandle = 1;
    int result_delete2 = gattdatabase3.DeleteService(deleteHandle);
    EXPECT_EQ(result_delete2, GATT_SUCCESS);
}

TEST(gatt_database_test, GetIncludeService)
{
    uint16_t handle = 0x13;
    uint16_t starthandle = 0x2;
    uint16_t endHandle = 0x3;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C4");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);
    uint16_t include_handle = 0x13;
    uint16_t include_starthandle = 0x2;
    uint16_t include_endHandle = 0x3;
    const bluetooth::Uuid include_uuid = Uuid::ConvertFromString("3F28CB71571C4");
    struct bluetooth::Service include_service(include_uuid, include_handle, include_starthandle, include_endHandle);
    service.includeServices_.push_back(include_service);
    GattDatabase gattdatabase4;
    gattdatabase4.AddService(service);

    uint16_t serviceHandle1 = 2;
    uint16_t serviceHandle2 = 0x13;
    gattdatabase4.GetIncludeService(serviceHandle1);
    gattdatabase4.GetIncludeService(serviceHandle2);
}

TEST(gatt_database_test, GetIncludeServices)
{
    uint16_t handle = 10000;
    uint16_t starthandle = 0x2;
    uint16_t endHandle = 0x3;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C5");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);
    GattDatabase gattdatabase5;
    gattdatabase5.AddService(service);

    uint16_t serviceHandle1 = 1;
    uint16_t serviceHandle2 = 10000;
    gattdatabase5.GetIncludeServices(serviceHandle1);
    gattdatabase5.GetIncludeServices(serviceHandle2);
}

TEST(gatt_database_test, GetCharacteristics)
{
    uint16_t handle = 10000;
    uint16_t starthandle = 0x2;
    uint16_t endHandle = 0x3;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C6");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);
    GattDatabase gattdatabase6;
    gattdatabase6.AddService(service);

    uint16_t serviceHandle1 = 1;
    uint16_t serviceHandle2 = 10000;
    gattdatabase6.GetCharacteristics(serviceHandle1);
    gattdatabase6.GetCharacteristics(serviceHandle2);
}

TEST(gatt_database_test, GetService)
{
    uint16_t handle = 10000;
    uint16_t starthandle = 0x2;
    uint16_t endHandle = 0x3;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C77");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);
    GattDatabase gattdatabase;
    gattdatabase.AddService(service);

    uint16_t handle_getsvc1 = 10000;
    gattdatabase.GetService(handle_getsvc1);

    uint16_t handle_getsvc2 = 1;
    gattdatabase.GetService(handle_getsvc2);
}

TEST(gatt_database_test, GetCharacteristic)
{
    uint16_t handle = 0x13;
    uint16_t starthandle = 0x2;
    uint16_t endHandle = 0x3;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C8");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("10008");
    uint16_t ccc_handle = 0x13;
    int ccc_properties = 10;
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties);
    service.characteristics_.push_back(ccc);

    GattDatabase gattdatabase8;
    int result = gattdatabase8.AddService(service);
    EXPECT_EQ(result, GATT_SUCCESS);

    uint16_t valueHandle1 = 3;
    gattdatabase8.GetCharacteristic(valueHandle1);

    uint16_t valueHandle2 = 0x13;
    gattdatabase8.GetCharacteristic(valueHandle2);
}

TEST(gatt_database_test, GetDescriptor)
{
    uint16_t handle = 0x13;
    uint16_t starthandle = 0x2;
    uint16_t endHandle = 0x3;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C9");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("10009");
    uint16_t ccc_handle = 0x13;
    int ccc_properties = 10;
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties);

    uint16_t desc_handle = 0x13;
    const bluetooth::Uuid desc_uuid = Uuid::ConvertFromString("10009");
    int desc_permissions = 10;
    bluetooth::Descriptor desc(desc_handle, desc_uuid, desc_permissions);
    ccc.descriptors_.push_back(desc);
    service.characteristics_.push_back(ccc);

    GattDatabase gattdatabase9;
    gattdatabase9.AddService(service);

    uint16_t valueHandle1 = 4;
    uint16_t valueHandle2 = 0x13;
    gattdatabase9.GetDescriptor(valueHandle1);
    gattdatabase9.GetDescriptor(valueHandle2);
}

TEST(gatt_database_test, GetDescriptors)
{
    uint16_t handle = 0x13;
    uint16_t starthandle = 0x2;
    uint16_t endHandle = 0x3;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C10");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("10010");
    uint16_t ccc_handle = 0x13;
    int ccc_properties = 10;
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties);

    uint16_t desc_handle = 0x13;
    const bluetooth::Uuid desc_uuid = Uuid::ConvertFromString("10010");
    int desc_permissions = 10;
    bluetooth::Descriptor desc(desc_handle, desc_uuid, desc_permissions);
    ccc.descriptors_.push_back(desc);
    service.characteristics_.push_back(ccc);

    GattDatabase gattdatabase10;
    gattdatabase10.AddService(service);

    uint16_t characteristicsHandle1 = 2;
    uint16_t characteristicsHandle2 = 0x13;
    gattdatabase10.GetDescriptors(characteristicsHandle1);
    gattdatabase10.GetDescriptors(characteristicsHandle2);
}

TEST(gatt_database_test, SetValueByHandle)
{
    uint16_t handle = 0x13;
    uint16_t starthandle = 0x2;
    uint16_t endHandle = 0x3;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C11");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("10011");
    uint16_t ccc_handle = 0x13;
    int ccc_properties = 10;
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties);
    service.characteristics_.push_back(ccc);

    GattDatabase gattdatabase11;
    gattdatabase11.AddService(service);

    uint16_t attribute_handle1 = 3;
    uint16_t attribute_handle2 = 0x13;
    size_t length = 2;
    uint8_t *value = new uint8_t[length];
    GattDatabase::AttributeValue attributevalue;
    attributevalue.SetValue(value, length);
    int result1 = gattdatabase11.SetValueByHandle(attribute_handle1, attributevalue);
    int result2 = gattdatabase11.SetValueByHandle(attribute_handle2, attributevalue);
    EXPECT_EQ(result1, GATT_SUCCESS);
    EXPECT_EQ(result2, HANDLE_NOT_FOUND);

    delete[] value;
}

TEST(gatt_database_test, CheckLegalityOfServiceDefinition_test1)
{
    uint16_t check_handle = 0x13;
    struct bluetooth::Service check_service1(check_handle);

    uint16_t include_handle = 0x15;
    uint16_t include_starthandle = 0x1;
    uint16_t include_endHandle = 0x2;
    const bluetooth::Uuid include_uuid = Uuid::ConvertFromString("3F28CB71571C5");
    struct bluetooth::Service include_service(include_uuid, include_handle, include_starthandle, include_endHandle);
    check_service1.includeServices_.push_back(include_service);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("100012");
    uint16_t ccc_handle = 0x13;
    int ccc_properties = 10;
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties);

    uint16_t desc_handle = 0x13;
    const bluetooth::Uuid desc_uuid = Uuid::ConvertFromString("100012");
    int desc_permissions = 10;
    bluetooth::Descriptor desc(desc_handle, desc_uuid, desc_permissions);
    ccc.descriptors_.push_back(desc);
    check_service1.characteristics_.push_back(ccc);

    GattDatabase gattdatabase12;
    int result = gattdatabase12.CheckLegalityOfServiceDefinition(check_service1);
    EXPECT_EQ(result, INCLUDE_SERVICE_NOT_FOUND);
}

TEST(gatt_database_test, CheckLegalityOfServiceDefinition_test2)
{
    uint16_t handle = 0x13;
    uint16_t starthandle = 0x2;
    uint16_t endHandle = 0x3;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C13");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);
    GattDatabase gattdatabase13;
    gattdatabase13.AddService(service);

    uint16_t check_handle = 0x13;
    struct bluetooth::Service check_service2(check_handle);

    uint16_t include_handle = 0x15;
    uint16_t include_starthandle = 0x1;
    uint16_t include_endHandle = 0x2;
    const bluetooth::Uuid include_uuid = Uuid::ConvertFromString("3F28CB71571C13");
    struct bluetooth::Service include_service(include_uuid, include_handle, include_starthandle, include_endHandle);
    check_service2.includeServices_.push_back(include_service);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("100013");
    uint16_t ccc_handle = 0x13;
    int ccc_properties = 10;
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties);

    uint16_t desc_handle = 0x13;
    const bluetooth::Uuid desc_uuid = Uuid::ConvertFromString("100013");
    int desc_permissions = 10;
    bluetooth::Descriptor desc(desc_handle, desc_uuid, desc_permissions);
    ccc.descriptors_.push_back(desc);
    check_service2.characteristics_.push_back(ccc);

    int result = gattdatabase13.CheckLegalityOfServiceDefinition(check_service2);
    EXPECT_EQ(result, INVALID_CHARACTERISTIC_DATA);
}

TEST(gatt_database_test, CheckLegalityOfServiceDefinition_test3)
{
    uint16_t handle = 0x13;
    uint16_t starthandle = 0x2;
    uint16_t endHandle = 0x3;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C14");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);
    GattDatabase gattdatabase14;
    gattdatabase14.AddService(service);

    uint16_t check_handle = 0x13;
    struct bluetooth::Service check_service3(check_handle);

    uint16_t include_handle = 0x15;
    uint16_t include_starthandle = 0x1;
    uint16_t include_endHandle = 0x2;
    const bluetooth::Uuid include_uuid = Uuid::ConvertFromString("3F28CB7157115");
    struct bluetooth::Service include_service(include_uuid, include_handle, include_starthandle, include_endHandle);
    check_service3.includeServices_.push_back(include_service);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("100014");
    uint16_t ccc_handle = 0x13;
    int ccc_properties = 0x02;
    int ccc_permissions = 10;
    size_t ccc_length = 2;
    const uint8_t *ccc_value = new uint8_t[ccc_length];
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties, ccc_permissions, ccc_value, ccc_length);

    uint16_t desc_handle = 0x13;
    const bluetooth::Uuid desc_uuid = Uuid::ConvertFromString("100014");
    int desc_permissions = 10;
    bluetooth::Descriptor desc(desc_handle, desc_uuid, desc_permissions);
    ccc.descriptors_.push_back(desc);
    check_service3.characteristics_.push_back(ccc);

    int result = gattdatabase14.CheckLegalityOfServiceDefinition(check_service3);
    EXPECT_EQ(result, INVALID_CHARACTERISTIC);
}

TEST(gatt_database_test, CheckLegalityOfServiceDefinition_test4)
{
    uint16_t handle = 0x13;
    uint16_t starthandle = 0x2;
    uint16_t endHandle = 0x3;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C15");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);
    GattDatabase gattdatabase15;
    gattdatabase15.AddService(service);

    uint16_t check_handle = 0x13;
    struct bluetooth::Service check_service4(check_handle);

    uint16_t include_handle = 0x15;
    uint16_t include_starthandle = 0x1;
    uint16_t include_endHandle = 0x2;
    const bluetooth::Uuid include_uuid = Uuid::ConvertFromString("3F28CB71571C51");
    struct bluetooth::Service include_service(include_uuid, include_handle, include_starthandle, include_endHandle);
    check_service4.includeServices_.push_back(include_service);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("100015");
    uint16_t ccc_handle = 0x13;
    int ccc_properties = 0x02;
    int ccc_permissions = 13;
    size_t ccc_length = 1;
    const uint8_t *ccc_value = new uint8_t[ccc_length];
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties, ccc_permissions, ccc_value, ccc_length);

    uint16_t desc_handle = 0x13;
    const bluetooth::Uuid desc_uuid = Uuid::ConvertFromString("100015");
    int desc_permissions = 10;
    bluetooth::Descriptor desc(desc_handle, desc_uuid, desc_permissions);
    ccc.descriptors_.push_back(desc);
    check_service4.characteristics_.push_back(ccc);

    int result = gattdatabase15.CheckLegalityOfServiceDefinition(check_service4);
    EXPECT_EQ(result, INVALID_CHARACTERISTIC_DESCRIPTOR_DATA);
}

TEST(gatt_database_test, CheckLegalityOfServiceDefinition_test5)
{
    uint16_t handle = 0x13;
    uint16_t starthandle = 0x2;
    uint16_t endHandle = 0x3;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C16");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);
    GattDatabase gattdatabase16;
    gattdatabase16.AddService(service);

    uint16_t check_handle = 0x13;
    struct bluetooth::Service check_service5(check_handle);

    uint16_t include_handle = 0x15;
    uint16_t include_starthandle = 0x1;
    uint16_t include_endHandle = 0x2;
    const bluetooth::Uuid include_uuid = Uuid::ConvertFromString("3F28CB71571C56");
    struct bluetooth::Service include_service(include_uuid, include_handle, include_starthandle, include_endHandle);
    check_service5.includeServices_.push_back(include_service);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("100016");
    uint16_t ccc_handle = 0x13;
    int ccc_properties = 0x01;
    int ccc_permissions = 3;
    size_t ccc_length = 2;
    const uint8_t *ccc_value = new uint8_t[ccc_length];
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties, ccc_permissions, ccc_value, ccc_length);

    uint16_t desc_handle = 0x13;
    const bluetooth::Uuid desc_uuid = Uuid::ConvertFromString("100016");
    int desc_permissions = 10;
    size_t desc_length = 1;
    const uint8_t *desc_value = new uint8_t[desc_length];
    bluetooth::Descriptor desc(desc_uuid, desc_handle, desc_permissions, desc_value, desc_length);
    ccc.descriptors_.push_back(desc);
    check_service5.characteristics_.push_back(ccc);

    int result = gattdatabase16.CheckLegalityOfServiceDefinition(check_service5);
    EXPECT_EQ(result, INVALID_CHARACTERISTIC_DESCRIPTOR);
}

TEST(gatt_database_test, CheckLegalityOfServiceDefinition_test6)
{
    uint16_t handle = 0x13;
    uint16_t starthandle = 0x2;
    uint16_t endHandle = 0x3;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C17");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);
    GattDatabase gattdatabase17;
    gattdatabase17.AddService(service);

    uint16_t check_handle = 0x13;
    struct bluetooth::Service check_service6(check_handle);

    uint16_t include_handle = 0x15;
    uint16_t include_starthandle = 0x1;
    uint16_t include_endHandle = 0x2;
    const bluetooth::Uuid include_uuid = Uuid::ConvertFromString("3F28CB71571C57");
    struct bluetooth::Service include_service(include_uuid, include_handle, include_starthandle, include_endHandle);
    check_service6.includeServices_.push_back(include_service);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("100017");
    uint16_t ccc_handle = 0x13;
    int ccc_properties = 0x10;
    int ccc_permissions = 3;
    size_t ccc_length = 5;
    const uint8_t *ccc_value = new uint8_t[ccc_length];
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties, ccc_permissions, ccc_value, ccc_length);

    uint16_t desc_handle = 0x13;
    const bluetooth::Uuid desc_uuid = Uuid::ConvertFromString("100017");
    int desc_permissions = 10;
    size_t desc_length = 2;
    const uint8_t *desc_value = new uint8_t[desc_length];
    bluetooth::Descriptor desc(desc_uuid, desc_handle, desc_permissions, desc_value, desc_length);
    ccc.descriptors_.push_back(desc);
    check_service6.characteristics_.push_back(ccc);

    int result = gattdatabase17.CheckLegalityOfServiceDefinition(check_service6);
    EXPECT_EQ(result, INVALID_CHARACTERISTIC_DESCRIPTOR);
}

TEST(gatt_database_test, CheckLegalityOfServiceDefinition_test7)
{
    uint16_t handle = 0x13;
    uint16_t starthandle = 0x2;
    uint16_t endHandle = 0x3;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C18");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);
    GattDatabase gattdatabase18;
    gattdatabase18.AddService(service);

    uint16_t check_handle = 0x13;
    struct bluetooth::Service check_service(check_handle);

    uint16_t include_handle = 0x15;
    uint16_t include_starthandle = 0x1;
    uint16_t include_endHandle = 0x2;
    const bluetooth::Uuid include_uuid = Uuid::ConvertFromString("3F28CB71571C58");
    struct bluetooth::Service include_service(include_uuid, include_handle, include_starthandle, include_endHandle);
    check_service.includeServices_.push_back(include_service);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("100018");
    uint16_t ccc_handle = 0x13;
    int ccc_properties = 0x80;
    int ccc_permissions = 3;
    size_t ccc_length = 2;
    const uint8_t *ccc_value = new uint8_t[ccc_length];
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties, ccc_permissions, ccc_value, ccc_length);

    uint16_t desc_handle = 0x13;
    const bluetooth::Uuid desc_uuid = Uuid::ConvertFromString("100018");
    int desc_permissions = 10;
    size_t desc_length = 3;
    const uint8_t *desc_value = new uint8_t[desc_length];
    bluetooth::Descriptor desc(desc_uuid, desc_handle, desc_permissions, desc_value, desc_length);
    ccc.descriptors_.push_back(desc);
    check_service.characteristics_.push_back(ccc);

    int result = gattdatabase18.CheckLegalityOfServiceDefinition(check_service);
    EXPECT_EQ(result, INVALID_CHARACTERISTIC_DESCRIPTOR);
}

TEST(gatt_database_test, CheckLegalityOfServiceDefinition_test8)
{
    uint16_t handle = 0x13;
    uint16_t starthandle = 0x2;
    uint16_t endHandle = 0x3;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C19");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);
    GattDatabase gattdatabase19;
    gattdatabase19.AddService(service);

    uint16_t check_handle = 0x13;
    struct bluetooth::Service check_service(check_handle);

    uint16_t include_handle = 0x15;
    uint16_t include_starthandle = 0x1;
    uint16_t include_endHandle = 0x2;
    const bluetooth::Uuid include_uuid = Uuid::ConvertFromString("3F28CB71571C59");
    struct bluetooth::Service include_service(include_uuid, include_handle, include_starthandle, include_endHandle);
    check_service.includeServices_.push_back(include_service);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("100019");
    uint16_t ccc_handle = 0x13;
    int ccc_properties = 0x08;
    int ccc_permissions = 3;
    size_t ccc_length = 2;
    const uint8_t *ccc_value = new uint8_t[ccc_length];
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties, ccc_permissions, ccc_value, ccc_length);

    uint16_t desc_handle = 0x13;
    const bluetooth::Uuid desc_uuid = Uuid::ConvertFrom16Bits(0x2901);
    int desc_permissions = 10;
    size_t desc_length = 2;
    const uint8_t *desc_value = new uint8_t[desc_length];
    bluetooth::Descriptor descriptor1(desc_uuid, desc_handle, desc_permissions, desc_value, desc_length);
    bluetooth::Descriptor descriptor2(desc_uuid, desc_handle, desc_permissions, desc_value, desc_length);
    ccc.descriptors_.push_back(descriptor1);
    ccc.descriptors_.push_back(descriptor2);
    check_service.characteristics_.push_back(ccc);

    int result = gattdatabase19.CheckLegalityOfServiceDefinition(check_service);
    EXPECT_EQ(result, INVALID_CHARACTERISTIC_DESCRIPTOR);
}

TEST(gatt_database_test, CheckLegalityOfServiceDefinition_test9)
{
    uint16_t handle = 0x13;
    uint16_t starthandle = 0x2;
    uint16_t endHandle = 0x3;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C20");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);
    GattDatabase gattdatabase20;
    gattdatabase20.AddService(service);

    uint16_t check_handle = 0x13;
    struct bluetooth::Service check_service(check_handle);

    uint16_t include_handle = 0x15;
    uint16_t include_starthandle = 0x1;
    uint16_t include_endHandle = 0x2;
    const bluetooth::Uuid include_uuid = Uuid::ConvertFromString("3F28CB71571C25");
    struct bluetooth::Service include_service(include_uuid, include_handle, include_starthandle, include_endHandle);
    check_service.includeServices_.push_back(include_service);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("10001");
    uint16_t ccc_handle = 0x13;
    int ccc_properties = 0x08;
    int ccc_permissions = 3;
    size_t ccc_length = 2;
    const uint8_t *ccc_value = new uint8_t[ccc_length];
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties, ccc_permissions, ccc_value, ccc_length);

    uint16_t desc_handle = 0x13;
    const bluetooth::Uuid desc_uuid = Uuid::ConvertFrom16Bits(0x2904);
    int desc_permissions = 10;
    size_t desc_length = 2;
    const uint8_t *desc_value = new uint8_t[desc_length];
    bluetooth::Descriptor desc1(desc_uuid, desc_handle, desc_permissions, desc_value, desc_length);
    bluetooth::Descriptor desc2(desc_uuid, desc_handle, desc_permissions, desc_value, desc_length);
    ccc.descriptors_.push_back(desc1);
    ccc.descriptors_.push_back(desc2);
    check_service.characteristics_.push_back(ccc);

    int result = gattdatabase20.CheckLegalityOfServiceDefinition(check_service);
    EXPECT_EQ(result, INVALID_CHARACTERISTIC_DESCRIPTOR);
}

TEST(gatt_database_test, CheckLegalityOfServiceDefinition)
{
    uint16_t handle = 0x13;
    uint16_t starthandle = 0x2;
    uint16_t endHandle = 0x3;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C21");
    struct bluetooth::Service service(uuid, handle, starthandle, endHandle);
    GattDatabase gattdatabase21;
    gattdatabase21.AddService(service);

    uint16_t check_handle = 0x13;
    struct bluetooth::Service check_service(check_handle);

    uint16_t include_handle = 0x15;
    uint16_t include_starthandle = 0x1;
    uint16_t include_endHandle = 0x2;
    const bluetooth::Uuid include_uuid = Uuid::ConvertFromString("3F28CB71571C21");
    struct bluetooth::Service include_service(include_uuid, include_handle, include_starthandle, include_endHandle);
    check_service.includeServices_.push_back(include_service);

    const bluetooth::Uuid ccc_uuid = Uuid::ConvertFromString("100021");
    uint16_t ccc_handle = 0x13;
    int ccc_properties = 0x02;
    int ccc_permissions = 3;
    size_t ccc_length = 2;
    const uint8_t *ccc_value = new uint8_t[ccc_length];
    bluetooth::Characteristic ccc(ccc_uuid, ccc_handle, ccc_properties, ccc_permissions, ccc_value, ccc_length);

    uint16_t desc_handle = 0x13;
    const bluetooth::Uuid desc_uuid = Uuid::ConvertFromString("100021");
    int desc_permissions = 10;
    size_t desc_length = 3;
    const uint8_t *desc_value = new uint8_t[desc_length];
    bluetooth::Descriptor desc(desc_uuid, desc_handle, desc_permissions, desc_value, desc_length);
    ccc.descriptors_.push_back(desc);
    check_service.characteristics_.push_back(ccc);

    int result = gattdatabase21.CheckLegalityOfServiceDefinition(check_service);
    EXPECT_EQ(result, GATT_SUCCESS);
}

TEST(gatt_database_test, DeleteService)
{
    uint16_t handle1 = 3;
    uint16_t starthandle1 = 2;
    uint16_t endHandle1 = 1;
    const bluetooth::Uuid uuid = Uuid::ConvertFromString("3F28CB71571C22");
    struct bluetooth::Service service1(uuid, handle1, starthandle1, endHandle1);
    uint16_t handle2 = 0x13;
    uint16_t starthandle2 = 0x13;
    uint16_t endHandle2 = 0x13;
    struct bluetooth::Service service2(uuid, handle2, starthandle2, endHandle2);
    uint16_t handle3 = 5;
    uint16_t starthandle3 = 5;
    uint16_t endHandle3 = 5;
    struct bluetooth::Service service3(uuid, handle3, starthandle3, endHandle3);
    GattDatabase gattdatabase;
    gattdatabase.AddService(service1);
    gattdatabase.AddService(service2);
    gattdatabase.AddService(service3);

    uint16_t handle_delete = 1;
    gattdatabase.DeleteService(handle_delete);

    uint16_t handle_delete2 = 2;
    gattdatabase.DeleteService(handle_delete2);

    uint16_t handle_delete3 = 3;
    gattdatabase.DeleteService(handle_delete3);
}
