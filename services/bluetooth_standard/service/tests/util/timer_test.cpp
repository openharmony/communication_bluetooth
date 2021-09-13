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
#include <functional>
#include <thread>
#include "timer.h"
#include "gtest/gtest.h"
#include <sys/select.h>

class TimerTest : public testing::Test {
public:
    virtual void SetUp()
    {}
    virtual void TearDown()
    {}
};

static void sleepMs(unsigned int timeMs)
{
    struct timeval tval;
    tval.tv_sec = timeMs / 1000;
    tval.tv_usec = (timeMs * 1000) % 1000000;
    select(0, NULL, NULL, NULL, &tval);
}

void TestCallback(void *flag)
{
    *(int *)flag += 1;
}

// Test the normal cases
TEST_F(TimerTest, Timer_normal1)
{
    int testflag = 0;
    auto f1 = std::bind(TestCallback, &testflag);
    utility::Timer *timer = new utility::Timer(f1);

    EXPECT_TRUE(timer->Start(100, false));
    sleepMs(300);
    EXPECT_EQ(1, testflag);
    delete timer;
}

TEST_F(TimerTest, Timer_normal2)
{
    int testflag = 0;
    auto f1 = std::bind(TestCallback, &testflag);
    utility::Timer *timer = new utility::Timer(f1);

    timer->Start(100, true);
    sleepMs(210);
    EXPECT_EQ(2, testflag);
    delete timer;
}

TEST_F(TimerTest, Timer_normal3)
{
    int testflag = 0;
    auto f1 = std::bind(TestCallback, &testflag);
    utility::Timer *timer = new utility::Timer(f1);

    timer->Start(100, true);
    sleepMs(50);
    int cnt = 1;
    while (cnt < 10) {
        sleepMs(100);
        EXPECT_EQ(cnt, testflag);
        cnt++;
    }

    delete timer;
}

TEST_F(TimerTest, Timer_normal4)
{
    int testflag = 0;
    auto f1 = std::bind(TestCallback, &testflag);
    auto f2 = std::bind(TestCallback, &testflag);
    utility::Timer *timer1 = new utility::Timer(f1);
    utility::Timer *timer2 = new utility::Timer(f2);
    timer1->Start(100, false);
    timer2->Start(100, false);
    sleepMs(110);
    delete timer1;
    delete timer2;
    EXPECT_EQ(2, testflag);
}

TEST_F(TimerTest, Timer_normal5)
{
    int testflag = 0;
    auto f1 = std::bind(TestCallback, &testflag);
    auto f2 = std::bind(TestCallback, &testflag);
    utility::Timer *timer1 = new utility::Timer(f1);
    utility::Timer *timer2 = new utility::Timer(f2);
    timer1->Start(100, false);
    std::thread thread = std::thread(&utility::Timer::Start, timer2, 100, true);
    thread.join();
    sleepMs(210);
    EXPECT_EQ(3, testflag);
    delete timer1;
    delete timer2;
}

TEST_F(TimerTest, Timer_normal6)
{
    int testflag = 0;
    auto f1 = std::bind(TestCallback, &testflag);
    auto f2 = std::bind(TestCallback, &testflag);
    utility::Timer *timer1 = new utility::Timer(f1);
    utility::Timer *timer2 = new utility::Timer(f2);
    timer1->Start(100, true);
    std::thread thread = std::thread(&utility::Timer::Start, timer2, 100, true);
    thread.join();
    sleepMs(310);
    EXPECT_EQ(6, testflag);
    delete timer1;
    delete timer2;
}

TEST_F(TimerTest, Timer_normal7)
{
    int testflag = 0;
    auto f1 = std::bind(TestCallback, &testflag);
    utility::Timer *timer = new utility::Timer(f1);
    timer->Start(100, true);
    sleepMs(210);
    timer->Stop();
    sleepMs(100);
    EXPECT_EQ(2, testflag);

    timer->Start(200, true);
    sleepMs(410);
    EXPECT_EQ(4, testflag);

    timer->Start(100, true);
    sleepMs(410);
    EXPECT_EQ(8, testflag);

    timer->Start(100, false);
    sleepMs(210);
    EXPECT_EQ(9, testflag);

    timer->Start(200, false);
    sleepMs(110);
    EXPECT_EQ(9, testflag);
    sleepMs(110);
    EXPECT_EQ(10, testflag);

    delete timer;
}

// Before timeout callback executed, call stop.
TEST_F(TimerTest, Timer_Stop1)
{
    int testflag = 0;
    auto f1 = std::bind(TestCallback, &testflag);
    utility::Timer *timer = new utility::Timer(f1);
    timer->Start(100, false);
    timer->Stop();
    sleepMs(200);
    EXPECT_EQ(0, testflag);
    delete timer;
}

TEST_F(TimerTest, Timer_Stop2)
{
    int testflag = 0;
    auto f1 = std::bind(TestCallback, &testflag);
    utility::Timer *timer = new utility::Timer(f1);
    timer->Start(100, true);
    timer->Stop();
    sleepMs(110);
    EXPECT_EQ(0, testflag);
    delete timer;
}

// Before timeout callback executed, delete Timer object.
TEST_F(TimerTest, Timer_delete1)
{
    int testflag = 0;
    auto f1 = std::bind(TestCallback, &testflag);
    utility::Timer *timer = new utility::Timer(f1);
    timer->Start(100, false);
    delete timer;
    EXPECT_EQ(0, testflag);
}

TEST_F(TimerTest, Timer_delete2)
{
    int testflag = 0;
    auto f1 = std::bind(TestCallback, &testflag);
    utility::Timer *timer = new utility::Timer(f1);
    timer->Start(100, true);
    delete timer;
    sleepMs(110);
    EXPECT_EQ(0, testflag);
}

// Set alarm with the time = 0.
TEST_F(TimerTest, Timer_error)
{
    int testflag = 0;
    auto f1 = std::bind(TestCallback, &testflag);
    utility::Timer *timer = new utility::Timer(f1);
    timer->Start(0, false);
    sleepMs(100);
    EXPECT_EQ(0, testflag);
    delete timer;
}
