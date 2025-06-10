/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "bluetooth_log.h"
#include "napi_event_subscribe_module.h"

using namespace testing;
using namespace testing::ext;

static size_t g_napiGetCbInfoArgc = 0;
void NapiGetCbInfoMockSetArgc(size_t argc)
{
    g_napiGetCbInfoArgc = argc;
}
napi_status napi_get_cb_info(napi_env env, napi_callback_info cbinfo, size_t* argc, napi_value* argv,
    napi_value* this_arg, void** data)
{
    *argc = g_napiGetCbInfoArgc;
    return napi_ok;
}
napi_status napi_close_handle_scope(napi_env env, napi_handle_scope scope)
{
    return napi_ok;
}

namespace OHOS {
namespace Bluetooth {
void NapiMockClear(void);

class NapiEventSubscribeModuleTest : public testing::Test {
public:
    NapiEventSubscribeModuleTest() = default;
    ~NapiEventSubscribeModuleTest() = default;

    static void SetUpTestSuite(void) {}
    static void TearDownTestSuite(void) {}
    void SetUp()
    {
        NapiMockClear();
    }
    void TearDown() {}

    napi_env env_;
    napi_callback_info info_;
};

static std::string g_napiParseStringStr = "";
static bool g_napiParseStringIsString = false;

void NapiParseStringMock(std::string str, bool isString)
{
    g_napiParseStringStr = str;
    g_napiParseStringIsString = isString;
}

napi_status NapiParseString(napi_env env, napi_value value, std::string &outStr)
{
    outStr = g_napiParseStringStr;
    return g_napiParseStringIsString ? napi_ok : napi_invalid_arg;
}

static bool g_napiIsFunction = false;

napi_status NapiIsFunctionMock(bool isFunction)
{
    g_napiIsFunction = isFunction;
    return napi_ok;
}

napi_status NapiIsFunction(napi_env env, napi_value value)
{
    return g_napiIsFunction ? napi_ok : napi_invalid_arg;
}

NapiCallback::NapiCallback(napi_env env, napi_value callback) {}
NapiCallback::~NapiCallback() {}

static int g_napiCallFunctionCount = 0;
void NapiCallback::CallFunction(const std::shared_ptr<NapiNativeObject> &object)
{
    g_napiCallFunctionCount++;
}
void NapiCallback::CallFunction(int errCode, const std::shared_ptr<NapiNativeObject> &object) {}
napi_env NapiCallback::GetNapiEnv(void)
{
    return env_;
}
std::string NapiCallback::ToLogString(void) const
{
    return "";
}

static bool g_napiCallbackIsEqual = false;
void NapiCallbackIsEqualMock(bool equal)
{
    g_napiCallbackIsEqual = equal;
}
bool NapiCallback::Equal(napi_env env, napi_value &callback) const
{
    return g_napiCallbackIsEqual;
}

int DoInJsMainThread(std::function<void(void)> func)
{
    func();
    return 0;
}

int DoInJsMainThread(napi_env env, std::function<void(void)> func)
{
    func();
    return 0;
}

void NapiMockClear(void)
{
    g_napiGetCbInfoArgc = 0;
    g_napiParseStringStr = "";
    g_napiParseStringIsString = false;
    g_napiIsFunction = false;
    g_napiCallbackIsEqual = false;
    g_napiCallFunctionCount = 0;
}

static const char * const TEST_CALLBACK_NAME = "testCallback";

/**
 * @tc.name: NapiEventSubscribeModuleTest_001
 * @tc.desc: Test register and deregister normal
 * @tc.type: FUNC
 */
HWTEST_F(NapiEventSubscribeModuleTest, NapiEventSubscribeModuleTest_001, TestSize.Level0)
{
    HILOGI("NapiEventSubscribeModuleTest_001 enter");
    NapiEventSubscribeModule subscribe(TEST_CALLBACK_NAME, "testModule");

    NapiGetCbInfoMockSetArgc(2);
    NapiParseStringMock(TEST_CALLBACK_NAME, true);

    NapiIsFunctionMock(true);
    NapiCallbackIsEqualMock(false);

    napi_status ret = subscribe.Register(env_, info_);
    EXPECT_EQ(ret, napi_ok);
    EXPECT_EQ(subscribe.eventSubscribeMap_.ReadVal(TEST_CALLBACK_NAME).size(), 1);

    NapiGetCbInfoMockSetArgc(2);
    NapiParseStringMock(TEST_CALLBACK_NAME, true);

    NapiIsFunctionMock(true);
    NapiCallbackIsEqualMock(true);

    ret = subscribe.Deregister(env_, info_);
    EXPECT_EQ(ret, napi_ok);
    EXPECT_EQ(subscribe.eventSubscribeMap_.ReadVal(TEST_CALLBACK_NAME).size(), 0);

    HILOGI("NapiEventSubscribeModuleTest_001 end");
}

/**
 * @tc.name: NapiEventSubscribeModuleTest_002
 * @tc.desc: Test register the type callback, and the same callback Repeated
 * @tc.type: FUNC
 */
HWTEST_F(NapiEventSubscribeModuleTest, NapiEventSubscribeModuleTest_002, TestSize.Level0)
{
    HILOGI("NapiEventSubscribeModuleTest_002 enter");
    NapiEventSubscribeModule subscribe(TEST_CALLBACK_NAME, "testModule");

    NapiGetCbInfoMockSetArgc(2);
    NapiParseStringMock(TEST_CALLBACK_NAME, true);

    NapiIsFunctionMock(true);
    NapiCallbackIsEqualMock(false);

    napi_status ret = subscribe.Register(env_, info_);
    EXPECT_EQ(ret, napi_ok);
    EXPECT_EQ(subscribe.eventSubscribeMap_.ReadVal(TEST_CALLBACK_NAME).size(), 1);

    // Mock the same callback
    NapiCallbackIsEqualMock(true);
    ret = subscribe.Register(env_, info_);
    EXPECT_EQ(ret, napi_ok);
    EXPECT_EQ(subscribe.eventSubscribeMap_.ReadVal(TEST_CALLBACK_NAME).size(), 1);

    HILOGI("NapiEventSubscribeModuleTest_002 end");
}

/**
 * @tc.name: NapiEventSubscribeModuleTest_003
 * @tc.desc: Test register the type callback, and multi callback
 * @tc.type: FUNC
 */
HWTEST_F(NapiEventSubscribeModuleTest, NapiEventSubscribeModuleTest_003, TestSize.Level0)
{
    HILOGI("NapiEventSubscribeModuleTest_003 enter");
    NapiEventSubscribeModule subscribe(TEST_CALLBACK_NAME, "testModule");

    NapiGetCbInfoMockSetArgc(2);
    NapiParseStringMock(TEST_CALLBACK_NAME, true);

    NapiIsFunctionMock(true);
    NapiCallbackIsEqualMock(false);

    napi_status ret = subscribe.Register(env_, info_);
    EXPECT_EQ(ret, napi_ok);
    EXPECT_EQ(subscribe.eventSubscribeMap_.ReadVal(TEST_CALLBACK_NAME).size(), 1);

    ret = subscribe.Register(env_, info_);
    EXPECT_EQ(ret, napi_ok);
    EXPECT_EQ(subscribe.eventSubscribeMap_.ReadVal(TEST_CALLBACK_NAME).size(), 2);

    HILOGI("NapiEventSubscribeModuleTest_003 end");
}

/**
 * @tc.name: NapiEventSubscribeModuleTest_004
 * @tc.desc: Test publish event
 * @tc.type: FUNC
 */
HWTEST_F(NapiEventSubscribeModuleTest, NapiEventSubscribeModuleTest_004, TestSize.Level0)
{
    HILOGI("NapiEventSubscribeModuleTest_004 enter");
    NapiEventSubscribeModule subscribe(TEST_CALLBACK_NAME, "testModule");

    NapiGetCbInfoMockSetArgc(2);
    NapiParseStringMock(TEST_CALLBACK_NAME, true);

    NapiIsFunctionMock(true);
    NapiCallbackIsEqualMock(false);

    napi_status ret = subscribe.Register(env_, info_);
    EXPECT_EQ(ret, napi_ok);
    EXPECT_EQ(subscribe.eventSubscribeMap_.ReadVal(TEST_CALLBACK_NAME).size(), 1);

    subscribe.PublishEvent(TEST_CALLBACK_NAME, nullptr);
    EXPECT_EQ(g_napiCallFunctionCount, 1);

    HILOGI("NapiEventSubscribeModuleTest_004 end");
}

/**
 * @tc.name: NapiEventSubscribeModuleTest_005
 * @tc.desc: Test register failed
 * @tc.type: FUNC
 */
HWTEST_F(NapiEventSubscribeModuleTest, NapiEventSubscribeModuleTest_005, TestSize.Level0)
{
    HILOGI("NapiEventSubscribeModuleTest_005 enter");
    NapiEventSubscribeModule subscribe(TEST_CALLBACK_NAME, "testModule");

    // Invalid params size
    NapiGetCbInfoMockSetArgc(1);
    napi_status ret = subscribe.Register(env_, info_);
    EXPECT_EQ(ret, napi_invalid_arg);

    // Invalid argv[0] string type
    NapiGetCbInfoMockSetArgc(2);
    NapiParseStringMock(TEST_CALLBACK_NAME, false);
    ret = subscribe.Register(env_, info_);
    EXPECT_EQ(ret, napi_invalid_arg);

    // Invalid type name
    NapiGetCbInfoMockSetArgc(2);
    NapiParseStringMock("errCallbackName", true);
    ret = subscribe.Register(env_, info_);
    EXPECT_EQ(ret, napi_invalid_arg);

    // Invalid argv[1] function type
    NapiGetCbInfoMockSetArgc(2);
    NapiParseStringMock(TEST_CALLBACK_NAME, true);
    NapiIsFunctionMock(false);
    ret = subscribe.Register(env_, info_);
    EXPECT_EQ(ret, napi_invalid_arg);

    HILOGI("NapiEventSubscribeModuleTest_005 end");
}

/**
 * @tc.name: NapiEventSubscribeModuleTest_006
 * @tc.desc: Test register failed
 * @tc.type: FUNC
 */
HWTEST_F(NapiEventSubscribeModuleTest, NapiEventSubscribeModuleTest_006, TestSize.Level0)
{
    HILOGI("NapiEventSubscribeModuleTest_006 enter");
    NapiEventSubscribeModule subscribe(TEST_CALLBACK_NAME, "testModule");

    // Invalid params size
    NapiGetCbInfoMockSetArgc(3);
    napi_status ret = subscribe.Deregister(env_, info_);
    EXPECT_EQ(ret, napi_invalid_arg);

    NapiGetCbInfoMockSetArgc(0);
    ret = subscribe.Deregister(env_, info_);
    EXPECT_EQ(ret, napi_invalid_arg);

    // Invalid argv[0] string type
    NapiGetCbInfoMockSetArgc(2);
    NapiParseStringMock(TEST_CALLBACK_NAME, false);
    ret = subscribe.Deregister(env_, info_);
    EXPECT_EQ(ret, napi_invalid_arg);

    // Invalid type name
    NapiGetCbInfoMockSetArgc(2);
    NapiParseStringMock("errCallbackName", true);
    ret = subscribe.Deregister(env_, info_);
    EXPECT_EQ(ret, napi_invalid_arg);

    // Invalid argv[1] function type
    NapiGetCbInfoMockSetArgc(2);
    NapiParseStringMock(TEST_CALLBACK_NAME, true);
    NapiIsFunctionMock(false);
    ret = subscribe.Register(env_, info_);
    EXPECT_EQ(ret, napi_invalid_arg);

    // The callback is not registered
    NapiGetCbInfoMockSetArgc(2);
    NapiParseStringMock(TEST_CALLBACK_NAME, true);
    NapiIsFunctionMock(true);
    NapiCallbackIsEqualMock(true);
    ret = subscribe.Deregister(env_, info_);
    EXPECT_EQ(ret, napi_ok);

    HILOGI("NapiEventSubscribeModuleTest_006 end");
}

/**
 * @tc.name: NapiEventSubscribeModuleTest_007
 * @tc.desc: Test publish event failed
 * @tc.type: FUNC
 */
HWTEST_F(NapiEventSubscribeModuleTest, NapiEventSubscribeModuleTest_007, TestSize.Level0)
{
    HILOGI("NapiEventSubscribeModuleTest_007 enter");
    NapiEventSubscribeModule subscribe(TEST_CALLBACK_NAME, "testModule");

    // Invalid eventName
    NapiGetCbInfoMockSetArgc(2);
    NapiParseStringMock(TEST_CALLBACK_NAME, true);

    NapiIsFunctionMock(true);
    NapiCallbackIsEqualMock(false);

    napi_status ret = subscribe.Register(env_, info_);
    EXPECT_EQ(ret, napi_ok);
    EXPECT_EQ(subscribe.eventSubscribeMap_.ReadVal(TEST_CALLBACK_NAME).size(), 1);

    subscribe.PublishEvent("errCallbackName", nullptr);
    EXPECT_EQ(g_napiCallFunctionCount, 0);

    // Empty callback in map
    NapiGetCbInfoMockSetArgc(2);
    NapiParseStringMock(TEST_CALLBACK_NAME, true);

    NapiIsFunctionMock(true);
    NapiCallbackIsEqualMock(true);

    ret = subscribe.Deregister(env_, info_);
    EXPECT_EQ(ret, napi_ok);
    EXPECT_EQ(subscribe.eventSubscribeMap_.ReadVal(TEST_CALLBACK_NAME).size(), 0);

    subscribe.PublishEvent(TEST_CALLBACK_NAME, nullptr);
    EXPECT_EQ(g_napiCallFunctionCount, 0);

    HILOGI("NapiEventSubscribeModuleTest_007 end");
}

/**
 * @tc.name: NapiEventSubscribeModuleTest_008
 * @tc.desc: Test Deregister all callback
 * @tc.type: FUNC
 */
HWTEST_F(NapiEventSubscribeModuleTest, NapiEventSubscribeModuleTest_008, TestSize.Level0)
{
    HILOGI("NapiEventSubscribeModuleTest_008 enter");
    NapiEventSubscribeModule subscribe(TEST_CALLBACK_NAME, "testModule");

    NapiGetCbInfoMockSetArgc(2);
    NapiParseStringMock(TEST_CALLBACK_NAME, true);

    NapiIsFunctionMock(true);
    NapiCallbackIsEqualMock(false);

    napi_status ret = subscribe.Register(env_, info_);
    EXPECT_EQ(ret, napi_ok);
    EXPECT_EQ(subscribe.eventSubscribeMap_.ReadVal(TEST_CALLBACK_NAME).size(), 1);

    ret = subscribe.Register(env_, info_);
    EXPECT_EQ(ret, napi_ok);
    EXPECT_EQ(subscribe.eventSubscribeMap_.ReadVal(TEST_CALLBACK_NAME).size(), 2);

    NapiGetCbInfoMockSetArgc(1);
    NapiParseStringMock(TEST_CALLBACK_NAME, true);

    ret = subscribe.Deregister(env_, info_);
    EXPECT_EQ(ret, napi_ok);
    EXPECT_EQ(subscribe.eventSubscribeMap_.ReadVal(TEST_CALLBACK_NAME).size(), 0);

    HILOGI("NapiEventSubscribeModuleTest_008 end");
}

/**
 * @tc.name: NapiEventSubscribeModuleTest_009
 * @tc.desc: Test publish a event with multi callback
 * @tc.type: FUNC
 */
HWTEST_F(NapiEventSubscribeModuleTest, NapiEventSubscribeModuleTest_009, TestSize.Level0)
{
    HILOGI("NapiEventSubscribeModuleTest_009 enter");
    napi_env env;
    napi_value callback = nullptr;
    NapiEventSubscribeModule subscribe(TEST_CALLBACK_NAME, "testModule");

    subscribe.eventSubscribeMap_.ChangeValueByLambda(TEST_CALLBACK_NAME, [env, callback](auto & callbackVec) {
        auto napiCallback = std::make_shared<NapiCallback>(env, callback);
        callbackVec.push_back(napiCallback);
        napiCallback = std::make_shared<NapiCallback>(env, callback);
        callbackVec.push_back(napiCallback);
    });

    subscribe.PublishEvent(TEST_CALLBACK_NAME, nullptr);
    EXPECT_EQ(g_napiCallFunctionCount, 2);

    HILOGI("NapiEventSubscribeModuleTest_009 end");
}
}  // namespace Bluetooth
}  // namespace OHOS