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
#include "fixed_queue.h"
#include "gtest/gtest.h"
#include <future>
#include <chrono>

class FixedQueueTest : public testing::Test {
public:
    virtual void SetUp()
    {}
    virtual void TearDown()
    {}
};

// Tests FixedQueue's c'tor.
TEST_F(FixedQueueTest, FixedQueue_Constructor)
{
    utility::FixedQueue<int> *queue;

    // Test the Default value.
    queue = new utility::FixedQueue<int>;
    EXPECT_TRUE(queue != nullptr);
    delete queue;

    // Test the corner case: size = 0.
    queue = new utility::FixedQueue<int>(0);
    EXPECT_TRUE(queue != nullptr);
    delete queue;

    // Test normal case.
    queue = new utility::FixedQueue<int>(30);
    EXPECT_TRUE(queue != nullptr);
    delete queue;
}

// Tests the normal cases and checks return value.
TEST_F(FixedQueueTest, FixedQueue_Normal)
{
    utility::FixedQueue<int> *queue = new utility::FixedQueue<int>(4);
    int output;

    EXPECT_TRUE(queue->TryPush(5));
    queue->Push(34);
    EXPECT_TRUE(queue->TryPush(17));
    EXPECT_TRUE(queue->TryPush(66));
    EXPECT_TRUE(queue->TryPop(output));
    EXPECT_EQ(5, output);
    queue->Pop(output);
    EXPECT_EQ(34, output);
    queue->Pop(output);
    EXPECT_EQ(17, output);
    queue->Pop(output);
    EXPECT_EQ(66, output);

    delete queue;
}

// Tests TryPop when the queue is Empty.
TEST_F(FixedQueueTest, FixedQueue_Empty_TryPop)
{
    utility::FixedQueue<int> *queue = new utility::FixedQueue<int>;
    int output;

    EXPECT_FALSE(queue->TryPop(output));

    delete queue;
}

// Tests TryPush when the queue is full.
TEST_F(FixedQueueTest, FixedQueue_Full_TryPush)
{
    utility::FixedQueue<int> *queue = new utility::FixedQueue<int>(0);
    int output;

    EXPECT_FALSE(queue->TryPush(output));

    delete queue;
}

// Tests Pop when the queue is Empty and after pushing pop should return.
TEST_F(FixedQueueTest, FixedQueue_Empty_Pop)
{
    utility::FixedQueue<int> *queue = new utility::FixedQueue<int>;
    int output;

    std::future<void> fut = std::async(std::launch::async, &utility::FixedQueue<int>::Pop, queue, std::ref(output));
    std::chrono::seconds span(1);
    EXPECT_TRUE(fut.wait_for(span) == std::future_status::timeout);

    queue->Push(40);

    // Wait for Pop to return.
    std::chrono::milliseconds span_2(3);
    EXPECT_TRUE(fut.wait_for(span_2) == std::future_status::ready);
    EXPECT_EQ(40, output);

    delete queue;
}

// Tests Push when the queue is full and after poping Push should return.
TEST_F(FixedQueueTest, FixedQueue_Full_Push)
{
    utility::FixedQueue<int> *queue = new utility::FixedQueue<int>(3);

    // Push until the queue is full.
    queue->Push(15);
    queue->Push(30);
    queue->Push(45);

    int forPush = 20;
    int output, test;
    std::future<void> fut = std::async(std::launch::async, &utility::FixedQueue<int>::Push, queue, forPush);
    std::chrono::seconds span(1);
    EXPECT_TRUE(fut.wait_for(span) == std::future_status::timeout);

    queue->Pop(test);
    EXPECT_EQ(test, 15);

    // Wait for Push to return.
    std::chrono::milliseconds span_2(3);
    EXPECT_TRUE(fut.wait_for(span_2) == std::future_status::ready);
    queue->Pop(output);
    EXPECT_EQ(30, output);
    queue->Pop(output);
    EXPECT_EQ(45, output);
    queue->Pop(output);
    EXPECT_EQ(20, output);

    delete queue;
}
