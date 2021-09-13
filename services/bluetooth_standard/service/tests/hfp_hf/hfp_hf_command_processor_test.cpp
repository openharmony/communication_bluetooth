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
#include "hfp_hf_command_processor.h"
#include "common_mock_all.h"
#include "hf_mock.h"

using namespace testing;
using namespace bluetooth;
class HfpHfProcessorTest : public testing::Test {
public:
    HfpHfDataConnection dataConn{};
    HfpHfCommandProcessor commandProcessor{};

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

        // Mock Rfcomm
        registerRfcommMocker(&rfcommMock_);
        initRfcommMock();

        dataConn.SetSlcConnectState(true);
        commandProcessor.Init(hfAddr);
    }

    void TearDown()
    {
        hfService.GetContext()->Uninitialize();

        dataConn.SetSlcConnectState(false);
    }

    void initRfcommMock()
    {
        EXPECT_CALL(GetRfcommMocker(), RFCOMM_Read(_, _)).Times(AnyNumber()).WillRepeatedly(Return(1));
        EXPECT_CALL(GetRfcommMocker(), RFCOMM_Write()).Times(AnyNumber()).WillRepeatedly(Return(0));
    }

    bool FindAtCmdFunc(std::string cmd, std::string arg)
    {
        bool ret = false;
        for (auto &it : atCmdMap) {
            if (!strncmp(it.first.c_str(), cmd.c_str(), it.first.length())) {
                commandProcessor.ProcessCommand(dataConn, it.first, arg);
                ret = true;
                break;
            }
        }
        return ret;
    }

private:
    StrictMock<AdapterManagerMock> amMock_;
    StrictMock<ProfileServiceManagerInstanceMock> psmInstanceMock_;
    StrictMock<ProfileServiceManagerMock> psmMock_;
    StrictMock<HfRfcommMock> rfcommMock_;
    HfpHfService hfService{};
    std::unordered_map<std::string, HfpHfCommandProcessor::HfpHfAtHandler> &atCmdMap{
        HfpHfCommandProcessor::GetAtCmdMap()};
};

TEST_F(HfpHfProcessorTest, ProcessOK)
{
    bool ret = FindAtCmdFunc("OK", "");
    EXPECT_TRUE(ret);
}

TEST_F(HfpHfProcessorTest, ProcessError)
{
    bool ret = FindAtCmdFunc("ERROR", "");
    EXPECT_TRUE(ret);
}

TEST_F(HfpHfProcessorTest, ProcessCmeError)
{
    bool ret = FindAtCmdFunc("+CME ERROR: ", "");
    EXPECT_TRUE(ret);
}

TEST_F(HfpHfProcessorTest, ProcessBrsf)
{
    bool ret = FindAtCmdFunc("+BRSF: ", "1");
    EXPECT_TRUE(ret);
}

TEST_F(HfpHfProcessorTest, ProcessCindGetter)
{
    bool ret = FindAtCmdFunc("+CIND: ", "1,0");
    EXPECT_TRUE(ret);
}

TEST_F(HfpHfProcessorTest, ProcessChld)
{
    bool ret = FindAtCmdFunc("+CHLD: ", "0,1,1x,2,2x,3,4");
    EXPECT_TRUE(ret);
}

TEST_F(HfpHfProcessorTest, ProcessBind_1)
{
    bool ret = FindAtCmdFunc("+BIND: ", "(1,1,1,1,1,1,1)");
    EXPECT_TRUE(ret);
}

TEST_F(HfpHfProcessorTest, ProcessBind_2)
{
    bool ret = FindAtCmdFunc("+BIND: ", "1,1");
    EXPECT_TRUE(ret);
}

TEST_F(HfpHfProcessorTest, ProcessOK_AT_BIA_SETTER)
{
    commandProcessor.SendAtCommand(dataConn, "", HfpHfCommandProcessor::AT_BIA_SETTER);
    bool ret = FindAtCmdFunc("OK", "");
    EXPECT_TRUE(ret);
}

TEST_F(HfpHfProcessorTest, ProcessOK_AT_BCS_SETTER)
{
    commandProcessor.SendAtCommand(dataConn, "", HfpHfCommandProcessor::AT_BCS_SETTER);
    bool ret = FindAtCmdFunc("OK", "");
    EXPECT_TRUE(ret);
}

TEST_F(HfpHfProcessorTest, ProcessOK_AT_CLIP_SETTER)
{
    commandProcessor.SendAtCommand(dataConn, "", HfpHfCommandProcessor::AT_CLIP_SETTER);
    bool ret = FindAtCmdFunc("OK", "");
    EXPECT_TRUE(ret);
}

TEST_F(HfpHfProcessorTest, ProcessOK_AT_CLCC_EXECUTER)
{
    commandProcessor.SendAtCommand(dataConn, "", HfpHfCommandProcessor::AT_CLCC_EXECUTER);
    bool ret = FindAtCmdFunc("OK", "");
    EXPECT_TRUE(ret);
}

TEST_F(HfpHfProcessorTest, ProcessOK_AT_BVRA_1_SETTER)
{
    commandProcessor.SendAtCommand(dataConn, "", HfpHfCommandProcessor::AT_BVRA_1_SETTER);
    bool ret = FindAtCmdFunc("OK", "");
    EXPECT_TRUE(ret);
}

TEST_F(HfpHfProcessorTest, ProcessOK_AT_BVRA_0_SETTER)
{
    commandProcessor.SendAtCommand(dataConn, "", HfpHfCommandProcessor::AT_BVRA_0_SETTER);
    bool ret = FindAtCmdFunc("OK", "");
    EXPECT_TRUE(ret);
}

TEST_F(HfpHfProcessorTest, ProcessError_AT_BIA_SETTER)
{
    commandProcessor.SendAtCommand(dataConn, "", HfpHfCommandProcessor::AT_BIA_SETTER);
    bool ret = FindAtCmdFunc("ERROR", "");
    EXPECT_TRUE(ret);
}

TEST_F(HfpHfProcessorTest, ProcessError_AT_BCC_EXECUTER)
{
    commandProcessor.SendAtCommand(dataConn, "", HfpHfCommandProcessor::AT_BCC_EXECUTER);
    bool ret = FindAtCmdFunc("ERROR", "");
    EXPECT_TRUE(ret);
}

TEST_F(HfpHfProcessorTest, ProcessError_AT_CLIP_SETTER)
{
    commandProcessor.SendAtCommand(dataConn, "", HfpHfCommandProcessor::AT_CLIP_SETTER);
    bool ret = FindAtCmdFunc("ERROR", "");
    EXPECT_TRUE(ret);
}

TEST_F(HfpHfProcessorTest, ConnectSlc_AT_BRSF_SETTER)
{
    dataConn.SetSlcConnectState(false);
    commandProcessor.SendAtCommand(dataConn, "", HfpHfCommandProcessor::AT_BRSF_SETTER);
    commandProcessor.ConnectSlc(dataConn);
}

TEST_F(HfpHfProcessorTest, ConnectSlc_AT_BAC_SETTER)
{
    dataConn.SetSlcConnectState(false);
    commandProcessor.SendAtCommand(dataConn, "", HfpHfCommandProcessor::AT_BAC_SETTER);
    commandProcessor.ConnectSlc(dataConn);
}

TEST_F(HfpHfProcessorTest, ConnectSlc_AT_CIND_TESTER)
{
    dataConn.SetSlcConnectState(false);
    commandProcessor.SendAtCommand(dataConn, "", HfpHfCommandProcessor::AT_CIND_TESTER);
    commandProcessor.ConnectSlc(dataConn);
}

TEST_F(HfpHfProcessorTest, ConnectSlc_AT_CIND_GETTER)
{
    dataConn.SetSlcConnectState(false);
    commandProcessor.SendAtCommand(dataConn, "", HfpHfCommandProcessor::AT_CIND_GETTER);
    commandProcessor.ConnectSlc(dataConn);
}

TEST_F(HfpHfProcessorTest, ConnectSlc_AT_CMER_EXECUTER)
{
    dataConn.SetSlcConnectState(false);
    commandProcessor.SendAtCommand(dataConn, "", HfpHfCommandProcessor::AT_CMER_EXECUTER);
    commandProcessor.ConnectSlc(dataConn);
}

TEST_F(HfpHfProcessorTest, ConnectSlc_AT_CHLD_TESTER)
{
    dataConn.SetSlcConnectState(false);
    commandProcessor.SendAtCommand(dataConn, "", HfpHfCommandProcessor::AT_CHLD_TESTER);
    commandProcessor.ConnectSlc(dataConn);
}

TEST_F(HfpHfProcessorTest, ConnectSlc_AT_BIND_SETTER)
{
    dataConn.SetSlcConnectState(false);
    commandProcessor.SendAtCommand(dataConn, "", HfpHfCommandProcessor::AT_BIND_SETTER);
    commandProcessor.ConnectSlc(dataConn);
}

TEST_F(HfpHfProcessorTest, ConnectSlc_AT_BIND_TESTER)
{
    dataConn.SetSlcConnectState(false);
    commandProcessor.SendAtCommand(dataConn, "", HfpHfCommandProcessor::AT_BIND_TESTER);
    commandProcessor.ConnectSlc(dataConn);
}

TEST_F(HfpHfProcessorTest, ConnectSlc_AT_BIND_GETTER)
{
    dataConn.SetSlcConnectState(false);
    commandProcessor.SendAtCommand(dataConn, "", HfpHfCommandProcessor::AT_BIND_GETTER);
    commandProcessor.ConnectSlc(dataConn);
}