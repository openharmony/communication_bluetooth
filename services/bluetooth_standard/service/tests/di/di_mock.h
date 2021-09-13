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

#ifndef DI_MOCK_H
#define DI_MOCK_H
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "interface_profile.h"
#include "di_service.h"
#include "sdp.h"

class SDPMock {
public:
    MOCK_METHOD0(SDP_CreateServiceRecord, uint32_t());
    MOCK_METHOD0(SDP_AddServiceClassIdList, int());
    MOCK_METHOD0(SDP_AddAttribute, int());
    MOCK_METHOD0(SDP_RegisterServiceRecord, int());
    MOCK_METHOD0(SDP_DeregisterServiceRecord, int());
    MOCK_METHOD0(SDP_DestroyServiceRecord, int());
};

class XmlMock {
public:
    MOCK_METHOD0(Load, bool());
    MOCK_METHOD0(GetValue, bool());
};

class DIServiceTest : public testing::Test {
public:
    bluetooth::DIService *diService_;
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

class IContextCallbackMock : public utility::IContextCallback {
public:
    ~IContextCallbackMock() = default;
    MOCK_METHOD2(OnEnable, void(const std::string &name, bool ret));
    MOCK_METHOD2(OnDisable, void(const std::string &name, bool ret));
};

#endif