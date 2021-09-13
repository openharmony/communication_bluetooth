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

#include <stdlib.h>
#include <iostream>
#include "dispatcher.h"
#include "gtest/gtest.h"
#include <future>
#include <chrono>
#include <unistd.h>

class DispatcherTest : public testing::Test {
public:
    virtual void SetUp()
    {}
    virtual void TearDown()
    {}
};

// Tests Dispatcher's c'tor which accepts a string.
TEST_F(DispatcherTest, Dispatcher_Constructor)
{
    std::string testString("Testing");
    utility::Dispatcher *dispatcher;

    // Test normal case.
    dispatcher = new utility::Dispatcher(testString);
    EXPECT_TRUE(dispatcher != nullptr);
    EXPECT_EQ(testString, dispatcher->Name());
    delete dispatcher;

    // Test when the string is empty.
    testString = "";
    dispatcher = new utility::Dispatcher(testString);
    EXPECT_TRUE(dispatcher != nullptr);
    EXPECT_EQ(testString, dispatcher->Name());
    delete dispatcher;

    // The case that two dispatchers have the same name is accepted.
    testString = "for test";
    dispatcher = new utility::Dispatcher(testString);
    utility::Dispatcher *dispatcher_2 = new utility::Dispatcher(testString);
    EXPECT_EQ(dispatcher_2->Name(), dispatcher->Name());
    delete dispatcher;
    delete dispatcher_2;
}

void TestTask(void *flag)
{
    *(int *)flag += 1;
}

// Tests Post Task function.
TEST_F(DispatcherTest, Dispatcher_PostTask)
{
    std::string testString("Testing");
    utility::Dispatcher *dispatcher;
    dispatcher = new utility::Dispatcher(testString);
    dispatcher->Initialize();

    int testflag = 0;
    auto f1 = std::bind(TestTask, &testflag);
    dispatcher->PostTask(f1);
    sleep(1);
    EXPECT_EQ(1, testflag);

    dispatcher->Uninitialize();
    delete dispatcher;
}
