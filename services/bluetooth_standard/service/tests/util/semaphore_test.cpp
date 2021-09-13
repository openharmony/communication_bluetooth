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

#include "semaphore.h"
#include <stdlib.h>
#include <iostream>
#include "gtest/gtest.h"
#include <unistd.h>
#include <future>
#include <chrono>

class SemaphoreTest : public testing::Test {
public:
    virtual void SetUp()
    {}
    virtual void TearDown()
    {}
};

// Test Post and Wait
TEST_F(SemaphoreTest, Semaphore_PostWait_Normal)
{
    utility::Semaphore *sem1 = new utility::Semaphore(0);

    sem1->Post();
    sem1->Post();
    sem1->Wait();
    sem1->Wait();
    sem1->Post();
    sem1->Wait();

    delete sem1;
}

// Test the normal case of TryWait
TEST_F(SemaphoreTest, Semaphore_TryWait_Normal)
{
    utility::Semaphore *sem2 = new utility::Semaphore(2);

    EXPECT_TRUE(sem2->TryWait());
    EXPECT_TRUE(sem2->TryWait());
    sem2->Post();
    EXPECT_TRUE(sem2->TryWait());

    delete sem2;
}

// Test TryWait when the semaphore val is zero
TEST_F(SemaphoreTest, Semaphore_Zero_TryWait)
{
    utility::Semaphore *sem3 = new utility::Semaphore(0);

    sem3->Post();
    sem3->Wait();
    EXPECT_FALSE(sem3->TryWait());

    delete sem3;
}

// Test Wait when the semaphore val is zero.
TEST_F(SemaphoreTest, Semaphore_Zero_Wait)
{
    utility::Semaphore *sem = new utility::Semaphore(0);

    std::future<void> fut = std::async(std::launch::async, &utility::Semaphore::Wait, sem);
    std::chrono::seconds span(1);
    EXPECT_TRUE(fut.wait_for(span) == std::future_status::timeout);

    sem->Post();

    std::chrono::milliseconds span_2(3);
    EXPECT_TRUE(fut.wait_for(span_2) == std::future_status::ready);

    delete sem;
}
