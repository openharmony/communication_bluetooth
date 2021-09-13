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

#include "state_machine.h"
#include "gtest/gtest.h"

std::string state0 = "StateZero";
std::string state1A = "StateOne_A";
std::string state1B = "StateOne_B";
std::string state1C = "StateOne_C";
std::string state2A = "StateTwo_A";
std::string state2B = "StateTwo_B";
std::string state2C = "StateTwo_C";

// StateMachine Implementation.
class StateMachineImpl : public utility::StateMachine {
public:    
    class State_Test : public utility::StateMachine::State {
    public:
        State_Test(const std::string &name, StateMachine &sm, State &parent) : State(name, sm, parent)
        {}
        State_Test(const std::string &name, StateMachine &sm) : State(name, sm)
        {}
        void Entry() override
        {
            entryCount_++;
            std::cout << "Entering " << this->Name() << std::endl;
        }
        void Exit() override
        {
            exitCount_++;
            std::cout << "Exiting " << this->Name() << std::endl;
        }

        int GetEntryCount() const
        {
            return entryCount_;
        }

        int GetExitCount() const
        {
            return exitCount_;
        }

    private:
        int entryCount_{0};
        int exitCount_{0};
    };

    class State_Zero : public State_Test {
    public:
        State_Zero(StateMachine &sm) : State_Test(state0, sm)
        {}
        ~State_Zero(){};
        bool Dispatch(const utility::Message &msg)
        {
            std::cout << "Dispatching " << Name() << std::endl;
            switch (msg.what_) {
                case 0:
                    Transition(state0);
                    return true;
                case 1:
                    Transition(state1A);
                    return true;
                case 2:
                    Transition(state1B);
                    return true;
                case 3:
                    Transition(state1C);
                    return true;
                case 4:
                    Transition(state2A);
                    return true;
                default:
                    return false;
            }
        }
    };

    class State_One_A : public State_Test {
    public:
        State_One_A(StateMachine &sm, State &parent) : State_Test(state1A, sm, parent)
        {}
        ~State_One_A(){};
        bool Dispatch(const utility::Message &msg)
        {
            std::cout << "Dispatching " << Name() << std::endl;
            switch (msg.what_) {
                case 0:
                    Transition(state2A);
                    return true;
                case 1:
                    Transition(state2B);
                    return true;
                case 2:
                    Transition(state2C);
                    return true;
                case -1:
                    Transition(state0);
                    return true;
                default:
                    return false;
            }
        }
    };

    class State_One_B : public State_Test {
    public:
        State_One_B(StateMachine &sm, State &parent) : State_Test(state1B, sm, parent)
        {}
        ~State_One_B(){};
        bool Dispatch(const utility::Message &msg)
        {
            std::cout << "Dispatching " << Name() << std::endl;
            return true;
        }
    };

    class State_One_C : public State_Test {
    public:
        State_One_C(StateMachine &sm, State &parent) : State_Test(state1C, sm, parent)
        {}
        ~State_One_C(){};
        bool Dispatch(const utility::Message &msg)
        {
            std::cout << "Dispatching " << Name() << std::endl;
            return true;
        }
    };

    class State_Two_A : public State_Test {
    public:
        State_Two_A(StateMachine &sm, State &parent) : State_Test(state2A, sm, parent)
        {}
        ~State_Two_A(){};
        bool Dispatch(const utility::Message &msg)
        {
            std::cout << "Dispatching " << Name() << std::endl;
            switch (msg.what_) {
                case 0:
                    Transition(state0);
                    return true;
                case 1:
                    Transition(state1A);
                    return true;
                case 2:
                    Transition(state1B);
                    return true;
                case 3:
                    Transition(state1C);
                    return true;
                case 4:
                    Transition(state2A);
                    return true;
                case 5:
                    Transition(state2B);
                    return true;
                case 6:
                    Transition(state2C);
                    return true;
                default:
                    return false;
            }
        }
    };

    class State_Two_B : public State_Test {
    public:
        State_Two_B(StateMachine &sm, State &parent) : State_Test(state2B, sm, parent)
        {}
        ~State_Two_B(){};
        bool Dispatch(const utility::Message &msg)
        {
            std::cout << "Dispatching " << Name() << std::endl;
            return true;
        }
    };

    class State_Two_C : public State_Test {
    public:
        State_Two_C(StateMachine &sm, State &parent) : State_Test(state2C, sm, parent)
        {}
        ~State_Two_C(){};
        bool Dispatch(const utility::Message &msg)
        {
            std::cout << "Dispatching " << Name() << std::endl;
            return true;
        }
    };

public:
    StateMachineImpl() : StateMachine()
    {
        std::unique_ptr<utility::StateMachine::State> state_zero_ = std::make_unique<State_Zero>(*this);
        std::unique_ptr<utility::StateMachine::State> state_one_A_ = std::make_unique<State_One_A>(*this, *state_zero_);
        std::unique_ptr<utility::StateMachine::State> state_one_B_ = std::make_unique<State_One_B>(*this, *state_zero_);
        std::unique_ptr<utility::StateMachine::State> state_one_C_ = std::make_unique<State_One_C>(*this, *state_zero_);
        std::unique_ptr<utility::StateMachine::State> state_two_A_ = std::make_unique<State_Two_A>(*this, *state_one_A_);
        std::unique_ptr<utility::StateMachine::State> state_two_B_ = std::make_unique<State_Two_B>(*this, *state_one_B_);
        std::unique_ptr<utility::StateMachine::State> state_two_C_ = std::make_unique<State_Two_C>(*this, *state_one_C_);

        Move(state_zero_);
        Move(state_one_A_);
        Move(state_one_B_);
        Move(state_one_C_);
        Move(state_two_A_);
        Move(state_two_B_);
        Move(state_two_C_);
    }
};

class StateMachineTest : public testing::Test {
public:
    virtual void SetUp()
    {}
    virtual void TearDown()
    {}
};

// Tests the initial state and check if it is the one we expected.
TEST_F(StateMachineTest, StateMachine_Initial)
{
    StateMachineImpl sm_;
    sm_.InitState(state0);
    EXPECT_EQ(state0, sm_.GetState()->Name());
}

// Tests simple transition and check the result.
TEST_F(StateMachineTest, StateMachine_Transition_1)
{
    StateMachineImpl sm_;
    sm_.InitState(state0);
    int test = 155;
    utility::Message message(1, 20, &test);
    sm_.ProcessMessage(message);
    EXPECT_EQ(state1A, sm_.GetState()->Name());
}

// Tests transition which requires exiting and check the result.
TEST_F(StateMachineTest, StateMachine_Transition_2)
{
    StateMachineImpl sm_;
    sm_.InitState(state0);
    int test = 155;
    utility::Message message(4, 20, &test);
    sm_.ProcessMessage(message);
    EXPECT_EQ(state2A, sm_.GetState()->Name());
    utility::Message message_(0, 20, &test);
    sm_.ProcessMessage(message_);
    EXPECT_EQ(state0, sm_.GetState()->Name());
}

// Tests transition which requires exiting and entering another layer then check the result.
TEST_F(StateMachineTest, StateMachine_Transition_3)
{
    StateMachineImpl sm_;
    sm_.InitState(state0);
    int test = 277;
    utility::Message message(4, 20, &test);
    sm_.ProcessMessage(message);
    EXPECT_EQ(state2A, sm_.GetState()->Name());
    utility::Message message_(6, 20, &test);
    sm_.ProcessMessage(message_);
    EXPECT_EQ(state2C, sm_.GetState()->Name());
}

TEST_F(StateMachineTest, StateMachine_Transition_4)
{
    StateMachineImpl sm_;
    sm_.InitState(state1A);
    EXPECT_EQ(state1A, sm_.GetState()->Name());
    int test = -1;
    utility::Message message(0, 20, &test);
    sm_.ProcessMessage(message);
    EXPECT_EQ(state2A, sm_.GetState()->Name());
    utility::Message message_(0, 20, &test);
    sm_.ProcessMessage(message_);
    EXPECT_EQ(state0, sm_.GetState()->Name());
}

TEST_F(StateMachineTest, StateMachine_Transition_5)
{
    StateMachineImpl sm_;
    sm_.InitState(state2A);
    EXPECT_EQ(state2A, sm_.GetState()->Name());
    int test = -100;
    utility::Message message(1, 20, &test);
    sm_.ProcessMessage(message);
    EXPECT_EQ(state1A, sm_.GetState()->Name());
    utility::Message message_(2, 20, &test);
    sm_.ProcessMessage(message_);
    EXPECT_EQ(state2C, sm_.GetState()->Name());
}

// Tests Message that need parent to handle and check statemachine's state doesn't change.
TEST_F(StateMachineTest, StateMachine_ProcessMessage_acrosslayer)
{
    StateMachineImpl sm_;
    sm_.InitState(state0);
    int test = 277;
    utility::Message message(1, 20, &test);
    sm_.ProcessMessage(message);
    EXPECT_EQ(state1A, sm_.GetState()->Name());
    utility::Message message_(2, 20, &test);
    sm_.ProcessMessage(message_);
    EXPECT_EQ(state2C, sm_.GetState()->Name());
}

// Tests Message that need parent to handle and check statemachine's state doesn't change.
TEST_F(StateMachineTest, StateMachine_ProcessMessage_fatherTochild_across)
{
    StateMachineImpl sm_;
    sm_.InitState(state0);
    int test = 277;
    utility::Message message(1, 20, &test);
    sm_.ProcessMessage(message);
    EXPECT_EQ(state1A, sm_.GetState()->Name());
    EXPECT_EQ(1, ((const StateMachineImpl::State_Test*)sm_.GetState())->GetEntryCount());
    EXPECT_EQ(0, ((const StateMachineImpl::State_Test*)sm_.GetState())->GetExitCount());
    utility::Message message_1(-1, 20, &test);
    sm_.ProcessMessage(message_1);
    EXPECT_EQ(state0, sm_.GetState()->Name());
    utility::Message message_2(1, 20, &test);
    sm_.ProcessMessage(message_2);
    EXPECT_EQ(state1A, sm_.GetState()->Name());
    EXPECT_EQ(2, ((const StateMachineImpl::State_Test*)sm_.GetState())->GetEntryCount());
    EXPECT_EQ(1, ((const StateMachineImpl::State_Test*)sm_.GetState())->GetExitCount());
}

// Tests Message that no state could handle and expect a false return value.
TEST_F(StateMachineTest, StateMachine_ProcessMessage_Fail1)
{
    StateMachineImpl sm_;
    sm_.InitState(state0);
    int test = 277;
    utility::Message message(4, 20, &test);
    sm_.ProcessMessage(message);
    EXPECT_EQ(state2A, sm_.GetState()->Name());
    utility::Message message_(13, 20, &test);
    EXPECT_FALSE(sm_.ProcessMessage(message_));
    EXPECT_EQ(state2A, sm_.GetState()->Name());
}

// current state is null
TEST_F(StateMachineTest, StateMachine_ProcessMessage_Fail2)
{
    StateMachineImpl sm_;
    int test = 277;
    utility::Message message(4, 20, &test);
    EXPECT_FALSE(sm_.ProcessMessage(message));
    EXPECT_EQ(nullptr, sm_.GetState());
}
