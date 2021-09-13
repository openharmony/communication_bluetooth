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

#include "hfp_ag_service.h"
#include "hfp_ag_command_processor.h"
#include "common_mock_all.h"
#include "ag_mock.h"

using namespace testing;
using namespace bluetooth;
class HfpAgProcessorTest : public testing::Test {
public:
    HfpAgDataConnection dataConn{};

    void SetUp()
    {
        // Mock AdapterManager::GetState()
        registerNewMockAdapterManager(&amMock_);
        EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(AtLeast(0))
            .WillRepeatedly(Return(STATE_TURN_ON));

        // Mock ProfileServiceManager
        registerNewMockProfileServiceManager(&psmInstanceMock_);
        EXPECT_CALL(psmInstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&psmMock_));
        agService.GetContext()->Initialize();
        EXPECT_CALL(psmMock_, GetProfileService(_)).Times(AtLeast(0)).WillRepeatedly(Return(&agService));

        // Mock Rfcomm
        registerRfcommMocker(&rfcommMock_);
        initRfcommMock();
    }

    void TearDown()
    {
        agService.GetContext()->Uninitialize();
    }

    void initRfcommMock()
    {
        EXPECT_CALL(GetRfcommMocker(), RFCOMM_Read(_, _)).Times(AnyNumber()).WillRepeatedly(Return(1));
        EXPECT_CALL(GetRfcommMocker(), RFCOMM_Write()).Times(AnyNumber()).WillRepeatedly(Return(0));
    }

private:
    StrictMock<AdapterManagerMock> amMock_;
    StrictMock<ProfileServiceManagerInstanceMock> psmInstanceMock_;
    StrictMock<ProfileServiceManagerMock> psmMock_;
    StrictMock<AgRfcommMock> rfcommMock_;
    HfpAgService agService{};
};

TEST_F(HfpAgProcessorTest, AtEmptyFn)
{
    HfpAgCommandProcessor::GetInstance().AtEmptyFn(dataConn, "");
}

TEST_F(HfpAgProcessorTest, CcwaSetter)
{
    HfpAgCommandProcessor::GetInstance().CcwaSetter(dataConn, "");
}

TEST_F(HfpAgProcessorTest, ChldTester)
{
    HfpAgCommandProcessor::GetInstance().ChldTester(dataConn, "");
}

TEST_F(HfpAgProcessorTest, CindTester)
{
    HfpAgCommandProcessor::GetInstance().CindTester(dataConn, "");
}

TEST_F(HfpAgProcessorTest, ClipSetter)
{
    HfpAgCommandProcessor::GetInstance().ClipSetter(dataConn, "");
}

TEST_F(HfpAgProcessorTest, CmerSetter)
{
    HfpAgCommandProcessor::GetInstance().CmerSetter(dataConn, "");
}

TEST_F(HfpAgProcessorTest, BldnExecuter)
{
    HfpAgCommandProcessor::GetInstance().BldnExecuter(dataConn, "");
}

TEST_F(HfpAgProcessorTest, BrsfSetter)
{
    HfpAgCommandProcessor::GetInstance().BrsfSetter(dataConn, "");
}

TEST_F(HfpAgProcessorTest, CopsSetter_1)
{
    HfpAgCommandProcessor::GetInstance().CopsSetter(dataConn, "3,0");
}

TEST_F(HfpAgProcessorTest, CopsSetter_2)
{
    HfpAgCommandProcessor::GetInstance().CopsSetter(dataConn, "test");
}

TEST_F(HfpAgProcessorTest, CmeeSetter)
{
    HfpAgCommandProcessor::GetInstance().CmeeSetter(dataConn, "");
}

TEST_F(HfpAgProcessorTest, BcsSetter)
{
    HfpAgCommandProcessor::GetInstance().BcsSetter(dataConn, "");
}

TEST_F(HfpAgProcessorTest, BindGetter)
{
    HfpAgCommandProcessor::GetInstance().BindGetter(dataConn, "");
}

TEST_F(HfpAgProcessorTest, BindTester)
{
    HfpAgCommandProcessor::GetInstance().BindTester(dataConn, "");
}

TEST_F(HfpAgProcessorTest, BacSetter)
{
    HfpAgCommandProcessor::GetInstance().BacSetter(dataConn, "1");
}

TEST_F(HfpAgProcessorTest, BtrhGetter)
{
    HfpAgCommandProcessor::GetInstance().BtrhGetter(dataConn, "");
}

TEST_F(HfpAgProcessorTest, BtrhSetter)
{
    HfpAgCommandProcessor::GetInstance().BtrhSetter(dataConn, "");
}