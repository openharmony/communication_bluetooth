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

#include "hfp_hf_service.h"
#include "hfp_hf_call_manager.h"
#include "common_mock_all.h"
#include "hf_mock.h"

using namespace testing;
using namespace bluetooth;
class HfpHfCallManagerTest : public testing::Test {
public:
    std::unique_ptr<HfpHfCallManager> hfCallManager{nullptr};

    void SetUp()
    {
        // Mock AdapterManager::GetState()
        registerNewMockAdapterManager(&amMock_);
        EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(AtLeast(0))
            .WillRepeatedly(Return(STATE_TURN_ON));

        // Mock ProfileServiceManager
        registerNewMockProfileServiceManager(&psmInstanceMock_);
        EXPECT_CALL(psmInstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&psmMock_));
        hfService.GetContext()->Initialize();
        EXPECT_CALL(psmMock_, GetProfileService(_)).Times(AtLeast(0)).WillRepeatedly(Return(&hfService));

        hfCallManager = std::make_unique<HfpHfCallManager>(hfAddr);
    }

    void TearDown()
    {
        hfService.GetContext()->Uninitialize();
    }

private:
    StrictMock<AdapterManagerMock> amMock_;
    StrictMock<ProfileServiceManagerInstanceMock> psmInstanceMock_;
    StrictMock<ProfileServiceManagerMock> psmMock_;
    HfpHfService hfService{};
};

TEST_F(HfpHfCallManagerTest, GetCallById)
{
    auto call = hfCallManager->GetCallById(1);
    EXPECT_EQ(nullptr, call);
}

TEST_F(HfpHfCallManagerTest, GetCallNumberByState)
{
    int number = hfCallManager->GetCallNumberByState(HFP_HF_CALL_STATE_ACTIVE);
    EXPECT_EQ(0, number);
}

TEST_F(HfpHfCallManagerTest, UpdateCallDone)
{
    HandsFreeUnitCalls call(hfAddr, 1, HFP_HF_CALL_STATE_INCOMING, "999", false, false, false);
    hfCallManager->UpdateCall(call);
    hfCallManager->UpdateCallDone();
    EXPECT_EQ(HFP_HF_CALL_STATE_INCOMING, hfCallManager->GetCallById(1)->GetState());

    call.SetState(HFP_HF_CALL_STATE_ACTIVE);
    hfCallManager->UpdateCall(call);
    hfCallManager->UpdateCallDone();
    EXPECT_EQ(HFP_HF_CALL_STATE_ACTIVE, hfCallManager->GetCallById(1)->GetState());

    hfCallManager->UpdateCallDone();
    EXPECT_EQ(nullptr, hfCallManager->GetCallById(1));
}