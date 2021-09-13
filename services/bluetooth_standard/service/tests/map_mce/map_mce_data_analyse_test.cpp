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
#include "interface_profile_map_mce.h"
#include "mce_mock/mce_mock.h"
#include "map_mce_types.h"
#include "map_mce_data_analyse.h"
#include "map_mce_instance_request.h"
#include "map_mce_service.h"
#include "map_mce_instance_stm.h"
#include "map_mce_instance_client.h"
#include "map_mce_device_ctrl.h"
#include "map_mce_service.h"
#include "map_mce_mns_server.h"

using namespace testing;
using namespace bluetooth;

const char *tempcharV1_0 =
    "<?xml version='1.0' encoding='utf-8' standalone='yes' ?>"
    "\n<MAP-msg-listing version = \"1.0\">"
    "\n<msg handle = \"20000100001\" subject = \"Hello\" datetime = \"20071213T130510\" sender_name = \"Jamie\" "
    "sender_addressing = \"+1-987-6543210\" recipient_addressing = \"+1-0123-456789\" type = \"SMS_GSM\" "
    "size = \"256\" attachment_size = \"0\" priority = \"no\" read = \"yes\" sent = \"no\" protected = \"no\"/>"
    "\n<msg handle = \"20000100002\" subject = \"Guten Tag\" datetime = \"20071214T092200\" sender_name = \"Dmitri\" "
    "sender_addressing = \"8765432109\" recipient_addressing = \"+49-9012-345678\" type = \"SMS_GSM\" "
    "size = \"512\" attachment_size = \"3000\" priority = \"no\" read = \"no\" sent = \"yes\" protected = \"no\"/>"
    "\n<msg handle = \"20000100003\" subject = \"Ohayougozaimasu\" datetime = \"20071215T134326\" "
    "sender_name = \"Andy\" sender_addressing = \"+49-7654-321098\" recipient_addressing = \"+49-89-01234567\" "
    "type = \"SMS_GSM\" size = \"256\" attachment_size = \"0\" priority = \"no\" read = \"yes\" sent = \"no\" "
    "protected =\"no\"/>"
    "\n<msg handle = \"20000100000\" subject = \"Bonjour\" datetime = \"20071215T171204\" sender_name = \"Marc\" "
    "sender_addressing = \"marc@carworkinggroup.bluetooth\" recipient_addressing =\"burch@carworkinggroup.bluetooth\" "
    "type = \"EMAIL\" size = \"1032\" attachment_size = \"0\" priority = \"yes\" "
    "read = \"yes\" sent = \"no\" protected = \"yes\"/>"
    "\n<msg handle = \"50000900011\" subject = \"Bonjour\" datetime = \"20071215T171204\" sender_name = \"Marc\" "
    "sender_addressing = \"marc@carworkinggroup.bluetooth\" recipient_addressing =\"burch@carworkinggroup.bluetooth\" "
    "type = \"SMS_CDMA\" size = \"1032\" attachment_size = \"0\" priority = \"yes\" "
    "read = \"yes\" sent = \"no\" protected = \"yes\" receptionStatus = \"complete\"/>"
    "\n<msg handle = \"50000900012\" subject = \"Bonjour\" datetime = \"20071215T171204\" sender_name = \"Marc\" "
    "sender_addressing = \"marc@carworkinggroup.bluetooth\" recipient_addressing =\"burch@carworkinggroup.bluetooth\" "
    "type = \"MMS\" size = \"1032\" attachment_size = \"0\" priority = \"yes\" "
    "read = \"yes\" sent = \"no\" protected = \"yes\" receptionStatus = \"fractioned\"/>"
    "\n<msg handle = \"50000900013\" subject = \"Bonjour\" datetime = \"20071215T171204\" sender_name = \"Marc\" "
    "sender_addressing = \"marc@carworkinggroup.bluetooth\" recipient_addressing =\"burch@carworkinggroup.bluetooth\" "
    "type = \"EMAIL\" size = \"1032\" attachment_size = \"0\" priority = \"yes\" "
    "read = \"yes\" sent = \"no\" protected = \"yes\" receptionStatus = \"notification\" text = \"no\"/>"
    "\n<msg handle = \"50000900014\" subject = \"Bonjour\" datetime = \"20071215T171204\" sender_name = \"Marc\" "
    "sender_addressing = \"marc@carworkinggroup.bluetooth\" recipient_addressing =\"burch@carworkinggroup.bluetooth\" "
    "type = \"EMAIL\" size = \"1032\" attachment_size = \"0\" priority = \"yes\" "
    "read = \"yes\" sent = \"no\" protected = \"yes\" text = \"yes\" />"
    "\n</MAP-msg-listing>";

TEST(MceTypesMessagesListing, BuildObjectData_V1_0)
{
    std::string msgListTestData(tempcharV1_0);
    IProfileMessagesListingParamStruct testParamIn;
    IProfileMessagesListingParamStruct testParamOut;
    std::vector<IProfileMessageOutline> testMsgOutlineList;
    std::string testMsgObject;

    MceTypesMessagesListing *listObjectPtr_ = new MceTypesMessagesListing();
    listObjectPtr_->BuildObjectData(testParamIn, msgListTestData);

    testMsgOutlineList = listObjectPtr_->GetList();
    testParamOut = listObjectPtr_->GetParam();
    testMsgObject = listObjectPtr_->GetStringObject();

    EXPECT_THAT("1.0", testParamOut.Version);
    EXPECT_THAT(msgListTestData, testMsgObject);
    EXPECT_EQ(8, (int)testMsgOutlineList.size());
    IProfileMessageOutline msgOutline;

    for (auto it = testMsgOutlineList.begin(); it != testMsgOutlineList.end(); it++) {
        msgOutline = *it;

        if (msgOutline.handle == "20000100001") {
            EXPECT_THAT("20000100001", msgOutline.handle);
            EXPECT_THAT("Hello", msgOutline.subject);
            EXPECT_THAT("20071213T130510", msgOutline.datetime);
            EXPECT_THAT("Jamie", msgOutline.sender_name);
            EXPECT_THAT("+1-987-6543210", msgOutline.sender_addressing);
            EXPECT_THAT("", msgOutline.replyto_addressing);
            EXPECT_THAT("", msgOutline.recipient_name);
            EXPECT_THAT("+1-0123-456789", msgOutline.recipient_addressing);
            EXPECT_EQ(MapMessageType::SMS_GSM, msgOutline.type);
            EXPECT_EQ(MapMsgReceptionStatus::INVALID, msgOutline.receptionStatus);
            EXPECT_EQ(256, msgOutline.size);
            EXPECT_EQ(0, msgOutline.attachment_size);
            EXPECT_EQ(MapBoolType::INVALID, msgOutline.text);
            EXPECT_EQ(MapMessageStatus::READ, msgOutline.read);
            EXPECT_EQ(MapBoolType::NO, msgOutline.sent);
            EXPECT_EQ(MapBoolType::NO, msgOutline.protected_);
            EXPECT_EQ(MapBoolType::NO, msgOutline.priority);
            // v1.1
            EXPECT_EQ(MapMsgDeliveryStatus::INVALID, msgOutline.delivery_status);
            EXPECT_THAT("", msgOutline.conversation_id);
            EXPECT_THAT("", msgOutline.conversation_name);
            EXPECT_EQ(MapMsgDirection::INVALID, msgOutline.direction);
            EXPECT_THAT("", msgOutline.attachment_mime_types);

        } else if (msgOutline.handle == "20000100002") {
            EXPECT_THAT("20000100002", msgOutline.handle);
            EXPECT_THAT("Guten Tag", msgOutline.subject);
            EXPECT_THAT("20071214T092200", msgOutline.datetime);
            EXPECT_THAT("Dmitri", msgOutline.sender_name);
            EXPECT_THAT("8765432109", msgOutline.sender_addressing);
            EXPECT_THAT("", msgOutline.replyto_addressing);
            EXPECT_THAT("", msgOutline.recipient_name);
            EXPECT_THAT("+49-9012-345678", msgOutline.recipient_addressing);
            EXPECT_EQ(MapMessageType::SMS_GSM, msgOutline.type);
            EXPECT_EQ(MapMsgReceptionStatus::INVALID, msgOutline.receptionStatus);
            EXPECT_EQ(512, msgOutline.size);
            EXPECT_EQ(3000, msgOutline.attachment_size);
            EXPECT_EQ(MapBoolType::INVALID, msgOutline.text);
            EXPECT_EQ(MapMessageStatus::UNREAD, msgOutline.read);
            EXPECT_EQ(MapBoolType::YES, msgOutline.sent);
            EXPECT_EQ(MapBoolType::NO, msgOutline.protected_);
            EXPECT_EQ(MapBoolType::NO, msgOutline.priority);
            // v1.1
            EXPECT_EQ(MapMsgDeliveryStatus::INVALID, msgOutline.delivery_status);
            EXPECT_THAT("", msgOutline.conversation_id);
            EXPECT_THAT("", msgOutline.conversation_name);
            EXPECT_EQ(MapMsgDirection::INVALID, msgOutline.direction);
            EXPECT_THAT("", msgOutline.attachment_mime_types);

        } else if (msgOutline.handle == "20000100003") {
            EXPECT_THAT("20000100003", msgOutline.handle);
            EXPECT_THAT("Ohayougozaimasu", msgOutline.subject);
            EXPECT_THAT("20071215T134326", msgOutline.datetime);
            EXPECT_THAT("Andy", msgOutline.sender_name);
            EXPECT_THAT("+49-7654-321098", msgOutline.sender_addressing);
            EXPECT_THAT("", msgOutline.replyto_addressing);
            EXPECT_THAT("", msgOutline.recipient_name);
            EXPECT_THAT("+49-89-01234567", msgOutline.recipient_addressing);
            EXPECT_EQ(MapMessageType::SMS_GSM, msgOutline.type);
            EXPECT_EQ(MapMsgReceptionStatus::INVALID, msgOutline.receptionStatus);
            EXPECT_EQ(256, msgOutline.size);
            EXPECT_EQ(0, msgOutline.attachment_size);
            EXPECT_EQ(MapBoolType::INVALID, msgOutline.text);
            EXPECT_EQ(MapMessageStatus::READ, msgOutline.read);
            EXPECT_EQ(MapBoolType::NO, msgOutline.sent);
            EXPECT_EQ(MapBoolType::NO, msgOutline.protected_);
            EXPECT_EQ(MapBoolType::NO, msgOutline.priority);
            // v1.1
            EXPECT_EQ(MapMsgDeliveryStatus::INVALID, msgOutline.delivery_status);
            EXPECT_THAT("", msgOutline.conversation_id);
            EXPECT_THAT("", msgOutline.conversation_name);
            EXPECT_EQ(MapMsgDirection::INVALID, msgOutline.direction);
            EXPECT_THAT("", msgOutline.attachment_mime_types);

        } else if (msgOutline.handle == "20000100000") {
            EXPECT_THAT("20000100000", msgOutline.handle);
            EXPECT_THAT("Bonjour", msgOutline.subject);
            EXPECT_THAT("20071215T171204", msgOutline.datetime);
            EXPECT_THAT("Marc", msgOutline.sender_name);
            EXPECT_THAT("marc@carworkinggroup.bluetooth", msgOutline.sender_addressing);
            EXPECT_THAT("", msgOutline.replyto_addressing);
            EXPECT_THAT("", msgOutline.recipient_name);
            EXPECT_THAT("burch@carworkinggroup.bluetooth", msgOutline.recipient_addressing);
            EXPECT_EQ(MapMessageType::EMAIL, msgOutline.type);
            EXPECT_EQ(MapMsgReceptionStatus::INVALID, msgOutline.receptionStatus);
            EXPECT_EQ(1032, msgOutline.size);
            EXPECT_EQ(0, msgOutline.attachment_size);
            EXPECT_EQ(MapBoolType::INVALID, msgOutline.text);
            EXPECT_EQ(MapMessageStatus::READ, msgOutline.read);
            EXPECT_EQ(MapBoolType::NO, msgOutline.sent);
            EXPECT_EQ(MapBoolType::YES, msgOutline.protected_);
            EXPECT_EQ(MapBoolType::YES, msgOutline.priority);
            // v1.1
            EXPECT_EQ(MapMsgDeliveryStatus::INVALID, msgOutline.delivery_status);
            EXPECT_THAT("", msgOutline.conversation_id);
            EXPECT_THAT("", msgOutline.conversation_name);
            EXPECT_EQ(MapMsgDirection::INVALID, msgOutline.direction);
            EXPECT_THAT("", msgOutline.attachment_mime_types);
        } else if (msgOutline.handle == "50000900011") {
            EXPECT_THAT("50000900011", msgOutline.handle);
            EXPECT_THAT("Bonjour", msgOutline.subject);
            EXPECT_THAT("20071215T171204", msgOutline.datetime);
            EXPECT_THAT("Marc", msgOutline.sender_name);
            EXPECT_THAT("marc@carworkinggroup.bluetooth", msgOutline.sender_addressing);
            EXPECT_THAT("", msgOutline.replyto_addressing);
            EXPECT_THAT("", msgOutline.recipient_name);
            EXPECT_THAT("burch@carworkinggroup.bluetooth", msgOutline.recipient_addressing);
            EXPECT_EQ(MapMessageType::SMS_CDMA, msgOutline.type);
            EXPECT_EQ(MapMsgReceptionStatus::COMPLETE, msgOutline.receptionStatus);
            EXPECT_EQ(1032, msgOutline.size);
            EXPECT_EQ(0, msgOutline.attachment_size);
            EXPECT_EQ(MapBoolType::INVALID, msgOutline.text);
            EXPECT_EQ(MapMessageStatus::READ, msgOutline.read);
            EXPECT_EQ(MapBoolType::NO, msgOutline.sent);
            EXPECT_EQ(MapBoolType::YES, msgOutline.protected_);
            EXPECT_EQ(MapBoolType::YES, msgOutline.priority);
            // v1.1
            EXPECT_EQ(MapMsgDeliveryStatus::INVALID, msgOutline.delivery_status);
            EXPECT_THAT("", msgOutline.conversation_id);
            EXPECT_THAT("", msgOutline.conversation_name);
            EXPECT_EQ(MapMsgDirection::INVALID, msgOutline.direction);
            EXPECT_THAT("", msgOutline.attachment_mime_types);
        } else if (msgOutline.handle == "50000900012") {
            EXPECT_THAT("50000900012", msgOutline.handle);
            EXPECT_THAT("Bonjour", msgOutline.subject);
            EXPECT_THAT("20071215T171204", msgOutline.datetime);
            EXPECT_THAT("Marc", msgOutline.sender_name);
            EXPECT_THAT("marc@carworkinggroup.bluetooth", msgOutline.sender_addressing);
            EXPECT_THAT("", msgOutline.replyto_addressing);
            EXPECT_THAT("", msgOutline.recipient_name);
            EXPECT_THAT("burch@carworkinggroup.bluetooth", msgOutline.recipient_addressing);
            EXPECT_EQ(MapMessageType::MMS, msgOutline.type);
            EXPECT_EQ(MapMsgReceptionStatus::FRACTIONED, msgOutline.receptionStatus);
            EXPECT_EQ(1032, msgOutline.size);
            EXPECT_EQ(0, msgOutline.attachment_size);
            EXPECT_EQ(MapBoolType::INVALID, msgOutline.text);
            EXPECT_EQ(MapMessageStatus::READ, msgOutline.read);
            EXPECT_EQ(MapBoolType::NO, msgOutline.sent);
            EXPECT_EQ(MapBoolType::YES, msgOutline.protected_);
            EXPECT_EQ(MapBoolType::YES, msgOutline.priority);
            // v1.1
            EXPECT_EQ(MapMsgDeliveryStatus::INVALID, msgOutline.delivery_status);
            EXPECT_THAT("", msgOutline.conversation_id);
            EXPECT_THAT("", msgOutline.conversation_name);
            EXPECT_EQ(MapMsgDirection::INVALID, msgOutline.direction);
            EXPECT_THAT("", msgOutline.attachment_mime_types);
        } else if (msgOutline.handle == "50000900013") {
            EXPECT_THAT("50000900013", msgOutline.handle);
            EXPECT_THAT("Bonjour", msgOutline.subject);
            EXPECT_THAT("20071215T171204", msgOutline.datetime);
            EXPECT_THAT("Marc", msgOutline.sender_name);
            EXPECT_THAT("marc@carworkinggroup.bluetooth", msgOutline.sender_addressing);
            EXPECT_THAT("", msgOutline.replyto_addressing);
            EXPECT_THAT("", msgOutline.recipient_name);
            EXPECT_THAT("burch@carworkinggroup.bluetooth", msgOutline.recipient_addressing);
            EXPECT_EQ(MapMessageType::EMAIL, msgOutline.type);
            EXPECT_EQ(MapMsgReceptionStatus::NOTIFICATION, msgOutline.receptionStatus);
            EXPECT_EQ(1032, msgOutline.size);
            EXPECT_EQ(0, msgOutline.attachment_size);
            EXPECT_EQ(MapBoolType::NO, msgOutline.text);
            EXPECT_EQ(MapMessageStatus::READ, msgOutline.read);
            EXPECT_EQ(MapBoolType::NO, msgOutline.sent);
            EXPECT_EQ(MapBoolType::YES, msgOutline.protected_);
            EXPECT_EQ(MapBoolType::YES, msgOutline.priority);
            // v1.1
            EXPECT_EQ(MapMsgDeliveryStatus::INVALID, msgOutline.delivery_status);
            EXPECT_THAT("", msgOutline.conversation_id);
            EXPECT_THAT("", msgOutline.conversation_name);
            EXPECT_EQ(MapMsgDirection::INVALID, msgOutline.direction);
            EXPECT_THAT("", msgOutline.attachment_mime_types);
        } else if (msgOutline.handle == "50000900014") {
            EXPECT_THAT("50000900014", msgOutline.handle);
            EXPECT_THAT("Bonjour", msgOutline.subject);
            EXPECT_THAT("20071215T171204", msgOutline.datetime);
            EXPECT_THAT("Marc", msgOutline.sender_name);
            EXPECT_THAT("marc@carworkinggroup.bluetooth", msgOutline.sender_addressing);
            EXPECT_THAT("", msgOutline.replyto_addressing);
            EXPECT_THAT("", msgOutline.recipient_name);
            EXPECT_THAT("burch@carworkinggroup.bluetooth", msgOutline.recipient_addressing);
            EXPECT_EQ(MapMessageType::EMAIL, msgOutline.type);
            EXPECT_EQ(MapMsgReceptionStatus::INVALID, msgOutline.receptionStatus);
            EXPECT_EQ(1032, msgOutline.size);
            EXPECT_EQ(0, msgOutline.attachment_size);
            EXPECT_EQ(MapBoolType::YES, msgOutline.text);
            EXPECT_EQ(MapMessageStatus::READ, msgOutline.read);
            EXPECT_EQ(MapBoolType::NO, msgOutline.sent);
            EXPECT_EQ(MapBoolType::YES, msgOutline.protected_);
            EXPECT_EQ(MapBoolType::YES, msgOutline.priority);
            // v1.1
            EXPECT_EQ(MapMsgDeliveryStatus::INVALID, msgOutline.delivery_status);
            EXPECT_THAT("", msgOutline.conversation_id);
            EXPECT_THAT("", msgOutline.conversation_name);
            EXPECT_EQ(MapMsgDirection::INVALID, msgOutline.direction);
            EXPECT_THAT("", msgOutline.attachment_mime_types);
        } else {
            // error
        }
    }
    delete listObjectPtr_;
    EXPECT_EQ(0, 0);
}

const char *tempcharV1_1 =
    "<?xml version='1.0' encoding='utf-8' standalone='yes' ?>"
    "\n<MAP-msg-listing version=\"1.1\">"
    "\n<msg handle=\"20000100001\" subject=\"Welcome Clara Nicole\" datetime=\"20140706T095000-0400\" "
    "sender_name=\"Max\" "
    "sender_addressing=\"4924689753@s.whateverapp.net\" recipient_addressing=\"\" type=\"IM\" size=\"256\" "
    "attachment_size=\"0\" priority=\"no\" read=\"no\" sent=\"no\" protected=\"no\" "
    "conversation_id=\"E1E2E3E4F1F2F3F4A1A2A3A4B1B2B3B4\" direction=\"incoming\"/> "
    "\n<msg handle=\"20000100002\" subject= \"What’s the progress Max?\" datetime=\"20140705T092200+0100\" "
    "sender_name=\"Jonas\" sender_addressing=\"4913579864@s.whateverapp.net\" recipient_addressing = \"\" "
    "type=\"IM\" "
    "size=\"512\" attachment_size=\"8671724\" priority=\"no\" read=\"yes\" sent=\"yes\" protected=\"no\" "
    "conversation_id=\"E1E2E3E4F1F2F3F4A1A2A3A4B1B2B3B5\" direction=\"outgoingdraft\" "
    "attachment_mime_types=\"video/mpeg\"/>"
    "\n<msg handle=\"60000801001\" subject= \"What’s the progress Max?\" datetime=\"20140705T092200+0100\" "
    "sender_name=\"Jonas\" sender_addressing=\"4913579864@s.whateverapp.net\" recipient_addressing = \"\" "
    "type=\"IM\" "
    "size=\"512\" attachment_size=\"8671724\" priority=\"no\" read=\"yes\" sent=\"yes\" protected=\"no\" "
    "conversation_id=\"E1E2E3E4F1F2F3F4A1A2A3A4B1B2B3B5\" direction=\"outgoing\" "
    "attachment_mime_types=\"video/mpeg\" delivery_status=\"delivered\" />"
    "\n<msg handle=\"60000801002\" subject= \"What’s the progress Max?\" datetime=\"20140705T092200+0100\" "
    "sender_name=\"Jonas\" sender_addressing=\"4913579864@s.whateverapp.net\" recipient_addressing = \"\" "
    "type=\"IM\" "
    "size=\"512\" attachment_size=\"8671724\" priority=\"no\" read=\"yes\" sent=\"yes\" protected=\"no\" "
    "conversation_id=\"E1E2E3E4F1F2F3F4A1A2A3A4B1B2B3B5\" direction=\"outgoingpending\" "
    "attachment_mime_types=\"video/mpeg\" delivery_status=\"sent\" />"
    "\n<msg handle=\"60000801003\" subject= \"What’s the progress Max?\" datetime=\"20140705T092200+0100\" "
    "sender_name=\"Jonas\" sender_addressing=\"4913579864@s.whateverapp.net\" recipient_addressing = \"\" "
    "type=\"IM\" "
    "size=\"512\" attachment_size=\"8671724\" priority=\"no\" read=\"yes\" sent=\"yes\" protected=\"no\" "
    "conversation_id=\"E1E2E3E4F1F2F3F4A1A2A3A4B1B2B3B5\" direction=\"outgoingdraft\" "
    "attachment_mime_types=\"video/mpeg\" delivery_status=\"unknown\" />"
    "\n</MAP-msg-listing>";

TEST(MceTypesMessagesListing, BuildObjectData_V1_1)
{
    std::string msgListTestData(tempcharV1_1);
    IProfileMessagesListingParamStruct testParamIn;
    IProfileMessagesListingParamStruct testParamOut;
    std::vector<IProfileMessageOutline> testMsgOutlineList;
    std::string testMsgObject;

    MceTypesMessagesListing *listObjectPtr_ = new MceTypesMessagesListing();
    listObjectPtr_->BuildObjectData(testParamIn, msgListTestData);

    testMsgOutlineList = listObjectPtr_->GetList();
    testParamOut = listObjectPtr_->GetParam();
    testMsgObject = listObjectPtr_->GetStringObject();

    EXPECT_THAT("1.1", testParamOut.Version);
    EXPECT_THAT(msgListTestData, testMsgObject);
    EXPECT_EQ(5, (int)testMsgOutlineList.size());
    IProfileMessageOutline msgOutline;
    for (auto it = testMsgOutlineList.begin(); it != testMsgOutlineList.end(); it++) {
        msgOutline = *it;
        if (msgOutline.handle == "20000100001") {
            EXPECT_THAT("20000100001", msgOutline.handle);
            EXPECT_THAT("Welcome Clara Nicole", msgOutline.subject);
            EXPECT_THAT("20140706T095000-0400", msgOutline.datetime);
            EXPECT_THAT("Max", msgOutline.sender_name);
            EXPECT_THAT("4924689753@s.whateverapp.net", msgOutline.sender_addressing);
            EXPECT_THAT("", msgOutline.replyto_addressing);
            EXPECT_THAT("", msgOutline.recipient_name);
            EXPECT_THAT("", msgOutline.recipient_addressing);
            EXPECT_EQ(MapMessageType::IM, msgOutline.type);
            EXPECT_EQ(MapMsgReceptionStatus::INVALID, msgOutline.receptionStatus);
            EXPECT_EQ(256, msgOutline.size);
            EXPECT_EQ(0, msgOutline.attachment_size);
            EXPECT_EQ(MapBoolType::INVALID, msgOutline.text);
            EXPECT_EQ(MapMessageStatus::UNREAD, msgOutline.read);
            EXPECT_EQ(MapBoolType::NO, msgOutline.sent);
            EXPECT_EQ(MapBoolType::NO, msgOutline.protected_);
            EXPECT_EQ(MapBoolType::NO, msgOutline.priority);
            // v1.1
            EXPECT_EQ(MapMsgDeliveryStatus::INVALID, msgOutline.delivery_status);
            EXPECT_THAT("E1E2E3E4F1F2F3F4A1A2A3A4B1B2B3B4", msgOutline.conversation_id);
            EXPECT_THAT("", msgOutline.conversation_name);
            EXPECT_EQ(MapMsgDirection::INCOMING, msgOutline.direction);
            EXPECT_THAT("", msgOutline.attachment_mime_types);
        } else if (msgOutline.handle == "20000100002") {
            EXPECT_THAT("20000100002", msgOutline.handle);
            EXPECT_THAT("What’s the progress Max?", msgOutline.subject);
            EXPECT_THAT("20140705T092200+0100", msgOutline.datetime);
            EXPECT_THAT("Jonas", msgOutline.sender_name);
            EXPECT_THAT("4913579864@s.whateverapp.net", msgOutline.sender_addressing);
            EXPECT_THAT("", msgOutline.replyto_addressing);
            EXPECT_THAT("", msgOutline.recipient_name);
            EXPECT_THAT("", msgOutline.recipient_addressing);
            EXPECT_EQ(MapMessageType::IM, msgOutline.type);
            EXPECT_EQ(MapMsgReceptionStatus::INVALID, msgOutline.receptionStatus);
            EXPECT_EQ(512, msgOutline.size);
            EXPECT_EQ(8671724, msgOutline.attachment_size);
            EXPECT_EQ(MapBoolType::INVALID, msgOutline.text);
            EXPECT_EQ(MapMessageStatus::READ, msgOutline.read);
            EXPECT_EQ(MapBoolType::YES, msgOutline.sent);
            EXPECT_EQ(MapBoolType::NO, msgOutline.protected_);
            EXPECT_EQ(MapBoolType::NO, msgOutline.priority);
            // v1.1
            EXPECT_EQ(MapMsgDeliveryStatus::INVALID, msgOutline.delivery_status);
            EXPECT_THAT("E1E2E3E4F1F2F3F4A1A2A3A4B1B2B3B5", msgOutline.conversation_id);
            EXPECT_THAT("", msgOutline.conversation_name);
            EXPECT_EQ(MapMsgDirection::OUTGOINGDRAFT, msgOutline.direction);
            EXPECT_THAT("video/mpeg", msgOutline.attachment_mime_types);
        } else if (msgOutline.handle == "60000801001") {
            EXPECT_THAT("60000801001", msgOutline.handle);
            EXPECT_THAT("What’s the progress Max?", msgOutline.subject);
            EXPECT_THAT("20140705T092200+0100", msgOutline.datetime);
            EXPECT_THAT("Jonas", msgOutline.sender_name);
            EXPECT_THAT("4913579864@s.whateverapp.net", msgOutline.sender_addressing);
            EXPECT_THAT("", msgOutline.replyto_addressing);
            EXPECT_THAT("", msgOutline.recipient_name);
            EXPECT_THAT("", msgOutline.recipient_addressing);
            EXPECT_EQ(MapMessageType::IM, msgOutline.type);
            EXPECT_EQ(MapMsgReceptionStatus::INVALID, msgOutline.receptionStatus);
            EXPECT_EQ(512, msgOutline.size);
            EXPECT_EQ(8671724, msgOutline.attachment_size);
            EXPECT_EQ(MapBoolType::INVALID, msgOutline.text);
            EXPECT_EQ(MapMessageStatus::READ, msgOutline.read);
            EXPECT_EQ(MapBoolType::YES, msgOutline.sent);
            EXPECT_EQ(MapBoolType::NO, msgOutline.protected_);
            EXPECT_EQ(MapBoolType::NO, msgOutline.priority);
            // v1.1
            EXPECT_EQ(MapMsgDeliveryStatus::DELIVERED, msgOutline.delivery_status);
            EXPECT_THAT("E1E2E3E4F1F2F3F4A1A2A3A4B1B2B3B5", msgOutline.conversation_id);
            EXPECT_THAT("", msgOutline.conversation_name);
            EXPECT_EQ(MapMsgDirection::OUTGOING, msgOutline.direction);
            EXPECT_THAT("video/mpeg", msgOutline.attachment_mime_types);
        } else if (msgOutline.handle == "60000801002") {
            EXPECT_THAT("60000801002", msgOutline.handle);
            EXPECT_THAT("What’s the progress Max?", msgOutline.subject);
            EXPECT_THAT("20140705T092200+0100", msgOutline.datetime);
            EXPECT_THAT("Jonas", msgOutline.sender_name);
            EXPECT_THAT("4913579864@s.whateverapp.net", msgOutline.sender_addressing);
            EXPECT_THAT("", msgOutline.replyto_addressing);
            EXPECT_THAT("", msgOutline.recipient_name);
            EXPECT_THAT("", msgOutline.recipient_addressing);
            EXPECT_EQ(MapMessageType::IM, msgOutline.type);
            EXPECT_EQ(MapMsgReceptionStatus::INVALID, msgOutline.receptionStatus);
            EXPECT_EQ(512, msgOutline.size);
            EXPECT_EQ(8671724, msgOutline.attachment_size);
            EXPECT_EQ(MapBoolType::INVALID, msgOutline.text);
            EXPECT_EQ(MapMessageStatus::READ, msgOutline.read);
            EXPECT_EQ(MapBoolType::YES, msgOutline.sent);
            EXPECT_EQ(MapBoolType::NO, msgOutline.protected_);
            EXPECT_EQ(MapBoolType::NO, msgOutline.priority);
            // v1.1
            EXPECT_EQ(MapMsgDeliveryStatus::SENT, msgOutline.delivery_status);
            EXPECT_THAT("E1E2E3E4F1F2F3F4A1A2A3A4B1B2B3B5", msgOutline.conversation_id);
            EXPECT_THAT("", msgOutline.conversation_name);
            EXPECT_EQ(MapMsgDirection::OUTGOINGPENDING, msgOutline.direction);
            EXPECT_THAT("video/mpeg", msgOutline.attachment_mime_types);
        } else if (msgOutline.handle == "60000801003") {
            EXPECT_THAT("60000801003", msgOutline.handle);
            EXPECT_THAT("What’s the progress Max?", msgOutline.subject);
            EXPECT_THAT("20140705T092200+0100", msgOutline.datetime);
            EXPECT_THAT("Jonas", msgOutline.sender_name);
            EXPECT_THAT("4913579864@s.whateverapp.net", msgOutline.sender_addressing);
            EXPECT_THAT("", msgOutline.replyto_addressing);
            EXPECT_THAT("", msgOutline.recipient_name);
            EXPECT_THAT("", msgOutline.recipient_addressing);
            EXPECT_EQ(MapMessageType::IM, msgOutline.type);
            EXPECT_EQ(MapMsgReceptionStatus::INVALID, msgOutline.receptionStatus);
            EXPECT_EQ(512, msgOutline.size);
            EXPECT_EQ(8671724, msgOutline.attachment_size);
            EXPECT_EQ(MapBoolType::INVALID, msgOutline.text);
            EXPECT_EQ(MapMessageStatus::READ, msgOutline.read);
            EXPECT_EQ(MapBoolType::YES, msgOutline.sent);
            EXPECT_EQ(MapBoolType::NO, msgOutline.protected_);
            EXPECT_EQ(MapBoolType::NO, msgOutline.priority);
            // v1.1
            EXPECT_EQ(MapMsgDeliveryStatus::UNKNOWN, msgOutline.delivery_status);
            EXPECT_THAT("E1E2E3E4F1F2F3F4A1A2A3A4B1B2B3B5", msgOutline.conversation_id);
            EXPECT_THAT("", msgOutline.conversation_name);
            EXPECT_EQ(MapMsgDirection::OUTGOINGDRAFT, msgOutline.direction);
            EXPECT_THAT("video/mpeg", msgOutline.attachment_mime_types);
        } else {
            // error
        }
    }
    delete listObjectPtr_;
    EXPECT_EQ(0, 0);
}

const char *realPhoneData =
    "<?xml version='1.0' encoding='utf-8' standalone='yes' ?>"
    "\n<MAP-msg-listing version=\"1.0\">"
    "\n    <msg handle=\"04000000000002D7\" subject=\"序号01的验证码605473，您...\" datetime=\"20201103T135114\" "
    "sender_name=\"95533\" sender_addressing=\"95533\" recipient_name=\"\" recipient_addressing=\"+8618388888888\" "
    "type=\"SMS_GSM\" size=\"63\" text=\"yes\" reception_status=\"complete\" attachment_size=\"0\" priority=\"no\" "
    "read=\"no\" sent=\"no\" protected=\"no\" />"
    "\n    <msg handle=\"04000000000002D6\" subject=\"尊敬的客户:感谢您参加中国移动&quot;...\" "
    "datetime=\"20201102T173851\" sender_name=\"10086\" sender_addressing=\"10086\" recipient_name=\"\" "
    "recipient_addressing=\"+8618388888888\" type=\"SMS_GSM\" size=\"149\" text=\"yes\" reception_status=\"complete\" "
    "attachment_size=\"0\" priority=\"no\" read=\"yes\" sent=\"no\" protected=\"no\" />"
    "\n</MAP-msg-listing>";

TEST(MceTypesMessagesListing, BuildObjectData_V1_0_fromRealPhone)
{
    std::string msgListTestData(realPhoneData);
    IProfileMessagesListingParamStruct testParamIn;
    IProfileMessagesListingParamStruct testParamOut;
    std::vector<IProfileMessageOutline> testMsgOutlineList;
    std::string testMsgObject;

    MceTypesMessagesListing *listObjectPtr_ = new MceTypesMessagesListing();
    listObjectPtr_->BuildObjectData(testParamIn, msgListTestData);

    testMsgOutlineList = listObjectPtr_->GetList();
    testParamOut = listObjectPtr_->GetParam();
    testMsgObject = listObjectPtr_->GetStringObject();

    EXPECT_THAT("1.0", testParamOut.Version);
    EXPECT_THAT(msgListTestData, testMsgObject);
    EXPECT_EQ(2, (int)testMsgOutlineList.size());
    IProfileMessageOutline msgOutline;

    for (auto it = testMsgOutlineList.begin(); it != testMsgOutlineList.end(); it++) {
        msgOutline = *it;

        if (msgOutline.handle == "04000000000002D7") {
            EXPECT_THAT("04000000000002D7", msgOutline.handle);
            EXPECT_THAT("序号01的验证码605473，您...", msgOutline.subject);
            EXPECT_THAT("20201103T135114", msgOutline.datetime);
            EXPECT_THAT("95533", msgOutline.sender_name);
            EXPECT_THAT("95533", msgOutline.sender_addressing);
            EXPECT_THAT("", msgOutline.replyto_addressing);
            EXPECT_THAT("", msgOutline.recipient_name);
            EXPECT_THAT("+8618388888888", msgOutline.recipient_addressing);
            EXPECT_EQ(MapMessageType::SMS_GSM, msgOutline.type);
            EXPECT_EQ(MapMsgReceptionStatus::INVALID, msgOutline.receptionStatus);
            EXPECT_EQ(63, msgOutline.size);
            EXPECT_EQ(0, msgOutline.attachment_size);
            EXPECT_EQ(MapBoolType::YES, msgOutline.text);
            EXPECT_EQ(MapMessageStatus::UNREAD, msgOutline.read);
            EXPECT_EQ(MapBoolType::NO, msgOutline.sent);
            EXPECT_EQ(MapBoolType::NO, msgOutline.protected_);
            EXPECT_EQ(MapBoolType::NO, msgOutline.priority);
            // v1.1
            EXPECT_EQ(MapMsgDeliveryStatus::INVALID, msgOutline.delivery_status);
            EXPECT_THAT("", msgOutline.conversation_id);
            EXPECT_THAT("", msgOutline.conversation_name);
            EXPECT_EQ(MapMsgDirection::INVALID, msgOutline.direction);
            EXPECT_THAT("", msgOutline.attachment_mime_types);

        } else if (msgOutline.handle == "04000000000002D6") {

            EXPECT_THAT("04000000000002D6", msgOutline.handle);
            EXPECT_THAT("尊敬的客户:感谢您参加中国移动\"...", msgOutline.subject);  // html: [&quot;] == c string：["]
            EXPECT_THAT("20201102T173851", msgOutline.datetime);
            EXPECT_THAT("10086", msgOutline.sender_name);
            EXPECT_THAT("10086", msgOutline.sender_addressing);
            EXPECT_THAT("", msgOutline.replyto_addressing);
            EXPECT_THAT("", msgOutline.recipient_name);
            EXPECT_THAT("+8618388888888", msgOutline.recipient_addressing);
            EXPECT_EQ(MapMessageType::SMS_GSM, msgOutline.type);
            EXPECT_EQ(MapMsgReceptionStatus::INVALID, msgOutline.receptionStatus);
            EXPECT_EQ(149, msgOutline.size);
            EXPECT_EQ(0, msgOutline.attachment_size);
            EXPECT_EQ(MapBoolType::YES, msgOutline.text);
            EXPECT_EQ(MapMessageStatus::READ, msgOutline.read);
            EXPECT_EQ(MapBoolType::NO, msgOutline.sent);
            EXPECT_EQ(MapBoolType::NO, msgOutline.protected_);
            EXPECT_EQ(MapBoolType::NO, msgOutline.priority);
            // v1.1
            EXPECT_EQ(MapMsgDeliveryStatus::INVALID, msgOutline.delivery_status);
            EXPECT_THAT("", msgOutline.conversation_id);
            EXPECT_THAT("", msgOutline.conversation_name);
            EXPECT_EQ(MapMsgDirection::INVALID, msgOutline.direction);
            EXPECT_THAT("", msgOutline.attachment_mime_types);
        } else {
            // error
        }
    }
    delete listObjectPtr_;
    EXPECT_EQ(0, 0);
}

const char *charFilter = "\r\n";

const char *profileBmessageV1_0 = "BEGIN:BMSG"
                                  "\nVERSION:1.0"
                                  "\nSTATUS:UNREAD"
                                  "\nTYPE:EMAIL\n"
                                  "\nFOLDER:TELECOM/MSG/INBOX"
                                  "\n\nBEGIN:VCARD"
                                  "\nVERSION:2.1"
                                  "\nN:Mat"
                                  "\nEMAIL:ma@abc.edu"
                                  "\nEND:VCARD"
                                  "\nBEGIN:BENV"
                                  "\nBEGIN:VCARD"
                                  "\nVERSION:2.1"
                                  "\nN:Tanaka"
                                  "\nEMAIL:tanaka@def.edu"
                                  "\nEND:VCARD"
                                  "\nBEGIN:BENV"
                                  "\nBEGIN:VCARD"
                                  "\nVERSION:2.1"
                                  "\nN:Laurent"
                                  "\nEMAIL:laurent@ghi.edu"
                                  "\nEND:VCARD"
                                  "\nBEGIN:BBODY"
                                  "\nENCODING:8BIT"
                                  "\nLENGTH:125"
                                  "\nBEGIN:MSG"
                                  "\nDate: 20 Jun 96"
                                  "\nSubject: Fish"
                                  "\nFrom: tanaka@def.edu"
                                  "\nTo: laurent@ghi.edu"
                                  "\nLet's go fishing!"
                                  "\nBR, Mat"
                                  "\nEND:MSG"
                                  "\nEND:BBODY"
                                  "\nEND:BENV"
                                  "\nEND:BENV"
                                  "\nEND:BMSG";

TEST(MceSplitString, SplitProfileBmessageV1_0)
{
    std::string filter(charFilter);
    MceSplitString tester(profileBmessageV1_0, filter);
    EXPECT_THAT("BEGIN:BMSG", tester.NextWord());
    EXPECT_THAT("VERSION:1.0", tester.NextWord());
    EXPECT_THAT("STATUS:UNREAD", tester.NextWord());
    EXPECT_THAT("TYPE:EMAIL", tester.NextWord());
    EXPECT_THAT("FOLDER:TELECOM/MSG/INBOX", tester.NextWord());
    EXPECT_THAT("BEGIN:VCARD", tester.NextWord());
    EXPECT_THAT("VERSION:2.1", tester.NextWord());
    EXPECT_THAT("N:Mat", tester.NextWord());
    EXPECT_THAT("EMAIL:ma@abc.edu", tester.NextWord());
    EXPECT_THAT("END:VCARD", tester.NextWord());
    EXPECT_THAT("BEGIN:BENV", tester.NextWord());
    EXPECT_THAT("BEGIN:VCARD", tester.NextWord());
    EXPECT_THAT("VERSION:2.1", tester.NextWord());
    EXPECT_THAT("N:Tanaka", tester.NextWord());
    EXPECT_THAT("EMAIL:tanaka@def.edu", tester.NextWord());
    EXPECT_THAT("END:VCARD", tester.NextWord());
    EXPECT_THAT("BEGIN:BENV", tester.NextWord());
    EXPECT_THAT("BEGIN:VCARD", tester.NextWord());
    EXPECT_THAT("VERSION:2.1", tester.NextWord());
    EXPECT_THAT("N:Laurent", tester.NextWord());
    EXPECT_THAT("EMAIL:laurent@ghi.edu", tester.NextWord());
    EXPECT_THAT("END:VCARD", tester.NextWord());
    EXPECT_THAT("BEGIN:BBODY", tester.NextWord());
    EXPECT_THAT("ENCODING:8BIT", tester.NextWord());
    EXPECT_THAT("LENGTH:125", tester.NextWord());
    EXPECT_THAT("BEGIN:MSG", tester.NextWord());
    EXPECT_THAT("Date: 20 Jun 96", tester.NextWord());
    EXPECT_THAT("Subject: Fish", tester.NextWord());
    EXPECT_THAT("From: tanaka@def.edu", tester.NextWord());
    EXPECT_THAT("To: laurent@ghi.edu", tester.NextWord());
    EXPECT_THAT("Let's go fishing!", tester.NextWord());
    EXPECT_THAT("BR, Mat", tester.NextWord());
    EXPECT_THAT("END:MSG", tester.NextWord());
    EXPECT_THAT("END:BBODY", tester.NextWord());
    EXPECT_THAT("END:BENV", tester.NextWord());
    EXPECT_THAT("END:BENV", tester.NextWord());
    EXPECT_THAT("END:BMSG", tester.NextWord());
    EXPECT_THAT("", tester.NextWord());
    EXPECT_EQ(0, 0);
}

TEST(MceBmessageParamAnalyser, BMessageNodeCheck_ProfileBmessageV1_0)
{
    MceBmessageParamAnalyser tester(profileBmessageV1_0);
    MceBMessageNode rootNode = tester.GetFirstChildNode();
    MceBMessageNode nodeLevel1;
    MceBMessageNode nodeLevel1_2;
    MceBMessageNode nodeLevel2;
    MceBMessageNode nodeLevel2_2;
    MceBMessageNode nodeLevel3;
    std::string tempStr;

    EXPECT_THAT("Date: 20 Jun 96\nSubject: Fish\nFrom: tanaka@def.edu\nTo: laurent@ghi.edu\nLet's go fishing!\nBR, Mat",
        tester.GetMsgText());
    EXPECT_THAT("1.0", rootNode.GetParamValue("VERSION:"));
    EXPECT_THAT("UNREAD", rootNode.GetParamValue("STATUS:"));
    EXPECT_THAT("EMAIL", rootNode.GetParamValue("TYPE:"));
    EXPECT_THAT("TELECOM/MSG/INBOX", rootNode.GetParamValue("FOLDER:"));
    EXPECT_THAT("UNREAD", rootNode.GetParamValue("STATUS:"));
    EXPECT_THAT("TELECOM/MSG/INBOX", rootNode.GetParamValue("FOLDER:"));
    EXPECT_THAT("TELECOM/MSG/INBOX", rootNode.GetParamValue("FOLDER:"));
    EXPECT_THAT("1.0", rootNode.GetParamValue("VERSION:"));
    EXPECT_THAT("1.0", rootNode.GetParamValue("VERSION:"));
    EXPECT_THAT("EMAIL", rootNode.GetParamValue("TYPE:"));
    nodeLevel1 = rootNode.GetNextChild();
    EXPECT_NE(0, (int)nodeLevel1.GetSize());
    EXPECT_THAT("VCARD", nodeLevel1.GetNodeName());
    EXPECT_THAT("2.1", nodeLevel1.GetParamValue("VERSION:"));
    EXPECT_THAT("Mat", nodeLevel1.GetParamValue("N:"));
    tempStr = *(nodeLevel1.GetParamValueList("EMAIL:").begin());
    EXPECT_THAT("ma@abc.edu", tempStr);  // "EMAIL:"
    nodeLevel1 = rootNode.GetNextChild();
    EXPECT_NE(0, (int)nodeLevel1.GetSize());
    nodeLevel1_2 = rootNode.GetNextChild();
    EXPECT_EQ(0, (int)nodeLevel1_2.GetSize());  // level1 node finish
    nodeLevel2 = nodeLevel1.GetNextChild();
    EXPECT_NE(0, (int)nodeLevel2.GetSize());
    EXPECT_THAT("VCARD", nodeLevel2.GetNodeName());
    EXPECT_THAT("2.1", nodeLevel2.GetParamValue("VERSION:"));
    EXPECT_THAT("Tanaka", nodeLevel2.GetParamValue("N:"));
    tempStr = *(nodeLevel2.GetParamValueList("EMAIL:").begin());
    EXPECT_THAT("tanaka@def.edu", tempStr);  // "EMAIL:"
    nodeLevel2 = nodeLevel1.GetNextChild();
    EXPECT_NE(0, (int)nodeLevel2.GetSize());
    nodeLevel2_2 = nodeLevel1.GetNextChild();
    EXPECT_EQ(0, (int)nodeLevel2_2.GetSize());
    nodeLevel3 = nodeLevel2.GetNextChild();
    EXPECT_NE(0, (int)nodeLevel3.GetSize());
    EXPECT_THAT("VCARD", nodeLevel3.GetNodeName());
    EXPECT_THAT("2.1", nodeLevel3.GetParamValue("VERSION:"));
    EXPECT_THAT("Laurent", nodeLevel3.GetParamValue("N:"));
    tempStr = *(nodeLevel3.GetParamValueList("EMAIL:").begin());
    EXPECT_THAT("laurent@ghi.edu", tempStr);  // "EMAIL:"
    EXPECT_THAT("VCARD", nodeLevel3.GetNodeName());
    tempStr = *(nodeLevel3.GetParamValueList("EMAIL:").begin());
    EXPECT_THAT("laurent@ghi.edu", tempStr);  // "EMAIL:"
    nodeLevel3 = nodeLevel2.GetNextChild();
    EXPECT_NE(0, (int)nodeLevel3.GetSize());
    EXPECT_THAT("BBODY", nodeLevel3.GetNodeName());
    EXPECT_THAT("8BIT", nodeLevel3.GetParamValue("ENCODING:"));
    EXPECT_THAT("125", nodeLevel3.GetParamValue("LENGTH:"));
    EXPECT_THAT("8BIT", nodeLevel3.GetParamValue("ENCODING:"));
    EXPECT_THAT("125", nodeLevel3.GetParamValue("LENGTH:"));
    EXPECT_THAT("BBODY", nodeLevel3.GetNodeName());
    EXPECT_THAT("Date: 20 Jun 96\nSubject: Fish\nFrom: tanaka@def.edu\nTo: laurent@ghi.edu\nLet's go fishing!\nBR, Mat",
        tester.GetMsgText());
    EXPECT_EQ(0, 0);
}

TEST(MceCombineNode, CombineStringProfileBmessageV1_0)
{
    MceCombineNode node;
    node.NodeBegin("BMSG");
    node.AddParamStr("VERSION:", "1.0");
    node.AddParamStr("STATUS:", "UNREAD");
    node.AddParamStr("TYPE:", "EMAIL");
    node.AddParamStr("FOLDER:", "TELECOM/MSG/INBOX");
    node.NodeBegin("VCARD");
    node.AddParamStr("VERSION:", "2.1");
    node.AddParamStr("N:", "Mat");
    node.AddParamStr("EMAIL:", "ma@abc.edu");
    node.NodeEnd();  // VCARD
    node.NodeBegin("BENV");
    node.NodeBegin("VCARD");
    node.AddParamStr("VERSION:", "2.1");
    node.AddParamStr("N:", "Tanaka");
    node.AddParamStr("EMAIL:", "tanaka@def.edu");
    node.NodeEnd();  // VCARD
    node.NodeBegin("BENV");
    node.NodeBegin("VCARD");
    node.AddParamStr("VERSION:", "2.1");
    node.AddParamStr("N:", "Laurent");
    node.AddParamStr("EMAIL:", "laurent@ghi.edu");
    node.NodeEnd();  // VCARD
    node.NodeBegin("BBODY");
    node.AddParamStr("ENCODING:", "8BIT");
    node.AddParamStr("LENGTH:", "125");
    node.NodeBegin("MSG");
    node.AddText(
        "Date: 20 Jun 96\nSubject: Fish\nFrom: tanaka@def.edu\nTo: laurent@ghi.edu\nLet's go fishing!\nBR, Mat");
    node.NodeEnd();  // MSG
    node.NodeEnd();  // BBODY
    node.NodeEnd();  // BENV
    node.NodeEnd();  // BENV
    node.NodeEnd();  // BMSG
    std::string msgString = node.GetCombineString();

    //================================== test the Split Word ===================================
    std::string filter(charFilter);
    MceSplitString tester(msgString, filter);
    EXPECT_THAT("BEGIN:BMSG", tester.NextWord());
    EXPECT_THAT("VERSION:1.0", tester.NextWord());
    EXPECT_THAT("STATUS:UNREAD", tester.NextWord());
    EXPECT_THAT("TYPE:EMAIL", tester.NextWord());
    EXPECT_THAT("FOLDER:TELECOM/MSG/INBOX", tester.NextWord());
    EXPECT_THAT("BEGIN:VCARD", tester.NextWord());
    EXPECT_THAT("VERSION:2.1", tester.NextWord());
    EXPECT_THAT("N:Mat", tester.NextWord());
    EXPECT_THAT("EMAIL:ma@abc.edu", tester.NextWord());
    EXPECT_THAT("END:VCARD", tester.NextWord());
    EXPECT_THAT("BEGIN:BENV", tester.NextWord());
    EXPECT_THAT("BEGIN:VCARD", tester.NextWord());
    EXPECT_THAT("VERSION:2.1", tester.NextWord());
    EXPECT_THAT("N:Tanaka", tester.NextWord());
    EXPECT_THAT("EMAIL:tanaka@def.edu", tester.NextWord());
    EXPECT_THAT("END:VCARD", tester.NextWord());
    EXPECT_THAT("BEGIN:BENV", tester.NextWord());
    EXPECT_THAT("BEGIN:VCARD", tester.NextWord());
    EXPECT_THAT("VERSION:2.1", tester.NextWord());
    EXPECT_THAT("N:Laurent", tester.NextWord());
    EXPECT_THAT("EMAIL:laurent@ghi.edu", tester.NextWord());
    EXPECT_THAT("END:VCARD", tester.NextWord());
    EXPECT_THAT("BEGIN:BBODY", tester.NextWord());
    EXPECT_THAT("ENCODING:8BIT", tester.NextWord());
    EXPECT_THAT("LENGTH:125", tester.NextWord());
    EXPECT_THAT("BEGIN:MSG", tester.NextWord());
    EXPECT_THAT("Date: 20 Jun 96", tester.NextWord());
    EXPECT_THAT("Subject: Fish", tester.NextWord());
    EXPECT_THAT("From: tanaka@def.edu", tester.NextWord());
    EXPECT_THAT("To: laurent@ghi.edu", tester.NextWord());
    EXPECT_THAT("Let's go fishing!", tester.NextWord());
    EXPECT_THAT("BR, Mat", tester.NextWord());
    EXPECT_THAT("END:MSG", tester.NextWord());
    EXPECT_THAT("END:BBODY", tester.NextWord());
    EXPECT_THAT("END:BENV", tester.NextWord());
    EXPECT_THAT("END:BENV", tester.NextWord());
    EXPECT_THAT("END:BMSG", tester.NextWord());
    EXPECT_THAT("", tester.NextWord());
    EXPECT_EQ(0, 0);

    //================================ test the BMessage Analyser  ================================
    MceBmessageParamAnalyser tester2(msgString);
    MceBMessageNode rootNode = tester2.GetFirstChildNode();
    MceBMessageNode nodeLevel1;
    MceBMessageNode nodeLevel1_2;
    MceBMessageNode nodeLevel2;
    MceBMessageNode nodeLevel2_2;
    MceBMessageNode nodeLevel3;
    std::string tempStr;
    EXPECT_THAT("Date: 20 Jun 96\nSubject: Fish\nFrom: tanaka@def.edu\nTo: laurent@ghi.edu\nLet's go fishing!\nBR, Mat",
        tester2.GetMsgText());
    EXPECT_THAT("1.0", rootNode.GetParamValue("VERSION:"));
    EXPECT_THAT("UNREAD", rootNode.GetParamValue("STATUS:"));
    EXPECT_THAT("EMAIL", rootNode.GetParamValue("TYPE:"));
    EXPECT_THAT("TELECOM/MSG/INBOX", rootNode.GetParamValue("FOLDER:"));
    EXPECT_THAT("UNREAD", rootNode.GetParamValue("STATUS:"));
    EXPECT_THAT("TELECOM/MSG/INBOX", rootNode.GetParamValue("FOLDER:"));
    EXPECT_THAT("TELECOM/MSG/INBOX", rootNode.GetParamValue("FOLDER:"));
    EXPECT_THAT("1.0", rootNode.GetParamValue("VERSION:"));
    EXPECT_THAT("1.0", rootNode.GetParamValue("VERSION:"));
    EXPECT_THAT("EMAIL", rootNode.GetParamValue("TYPE:"));
    nodeLevel1 = rootNode.GetNextChild();
    EXPECT_NE(0, (int)nodeLevel1.GetSize());
    EXPECT_THAT("VCARD", nodeLevel1.GetNodeName());
    EXPECT_THAT("2.1", nodeLevel1.GetParamValue("VERSION:"));
    EXPECT_THAT("Mat", nodeLevel1.GetParamValue("N:"));
    tempStr = *(nodeLevel1.GetParamValueList("EMAIL:").begin());
    EXPECT_THAT("ma@abc.edu", tempStr);  // "EMAIL:"
    nodeLevel1 = rootNode.GetNextChild();
    EXPECT_NE(0, (int)nodeLevel1.GetSize());
    nodeLevel1_2 = rootNode.GetNextChild();
    EXPECT_EQ(0, (int)nodeLevel1_2.GetSize());  // level1 node finish
    nodeLevel2 = nodeLevel1.GetNextChild();
    EXPECT_NE(0, (int)nodeLevel2.GetSize());
    EXPECT_THAT("VCARD", nodeLevel2.GetNodeName());
    EXPECT_THAT("2.1", nodeLevel2.GetParamValue("VERSION:"));
    EXPECT_THAT("Tanaka", nodeLevel2.GetParamValue("N:"));
    tempStr = *(nodeLevel2.GetParamValueList("EMAIL:").begin());
    EXPECT_THAT("tanaka@def.edu", tempStr);  // "EMAIL:"
    nodeLevel2 = nodeLevel1.GetNextChild();
    EXPECT_NE(0, (int)nodeLevel2.GetSize());
    nodeLevel2_2 = nodeLevel1.GetNextChild();
    EXPECT_EQ(0, (int)nodeLevel2_2.GetSize());
    nodeLevel3 = nodeLevel2.GetNextChild();
    EXPECT_NE(0, (int)nodeLevel3.GetSize());
    EXPECT_THAT("VCARD", nodeLevel3.GetNodeName());
    EXPECT_THAT("2.1", nodeLevel3.GetParamValue("VERSION:"));
    EXPECT_THAT("Laurent", nodeLevel3.GetParamValue("N:"));
    tempStr = *(nodeLevel3.GetParamValueList("EMAIL:").begin());
    EXPECT_THAT("laurent@ghi.edu", tempStr);  // "EMAIL:"
    EXPECT_THAT("VCARD", nodeLevel3.GetNodeName());
    tempStr = *(nodeLevel3.GetParamValueList("EMAIL:").begin());
    EXPECT_THAT("laurent@ghi.edu", tempStr);  // "EMAIL:"
    nodeLevel3 = nodeLevel2.GetNextChild();
    EXPECT_NE(0, (int)nodeLevel3.GetSize());
    EXPECT_THAT("BBODY", nodeLevel3.GetNodeName());
    EXPECT_THAT("8BIT", nodeLevel3.GetParamValue("ENCODING:"));
    EXPECT_THAT("125", nodeLevel3.GetParamValue("LENGTH:"));
    EXPECT_THAT("8BIT", nodeLevel3.GetParamValue("ENCODING:"));
    EXPECT_THAT("125", nodeLevel3.GetParamValue("LENGTH:"));
    EXPECT_THAT("BBODY", nodeLevel3.GetNodeName());
    EXPECT_THAT("Date: 20 Jun 96\nSubject: Fish\nFrom: tanaka@def.edu\nTo: laurent@ghi.edu\nLet's go fishing!\nBR, Mat",
        tester2.GetMsgText());
    EXPECT_EQ(0, 0);
}

TEST(MceBmessageParamAnalyser, IProfileBMessageStruct_ProfileBmessageV1_0)
{
    MceBmessageParamAnalyser tester(profileBmessageV1_0);
    tester.StartAnalyse();
    IProfileBMessageStruct testStruct = tester.GetMsgStruct();
    IProfileMapVcard vcard;
    EXPECT_THAT("1.0", testStruct.version_property);
    EXPECT_EQ(MapMessageStatus::UNREAD, testStruct.readstatus_property);
    EXPECT_EQ(MapMessageType::EMAIL, testStruct.type_property);
    EXPECT_THAT(u"TELECOM/MSG/INBOX", testStruct.folder_property);
    EXPECT_THAT("", testStruct.extendeddata_property);
    EXPECT_EQ(1, (int)testStruct.originator_.size());
    if (testStruct.originator_.size() != 0) {
        auto it = testStruct.originator_.begin();
        vcard = *it;
        EXPECT_THAT("2.1", vcard.VERSION);
        EXPECT_THAT("Mat", vcard.N);
        EXPECT_THAT("ma@abc.edu", *(vcard.EMAIL.begin()));
        EXPECT_EQ(0, (int)vcard.X_BT_UCI.size());
        EXPECT_EQ(0, (int)vcard.X_BT_UID.size());
        EXPECT_THAT("", vcard.FN);
        EXPECT_EQ(0, (int)vcard.TEL.size());
    }
    EXPECT_EQ(1, (int)testStruct.envelope_.recipientLevel1_.size());
    if ((int)testStruct.envelope_.recipientLevel1_.size() != 0) {
        auto it2 = testStruct.envelope_.recipientLevel1_.begin();
        vcard = *it2;
        EXPECT_THAT("2.1", vcard.VERSION);
        EXPECT_THAT("Tanaka", vcard.N);
        EXPECT_THAT("tanaka@def.edu", *(vcard.EMAIL.begin()));
        EXPECT_EQ(0, (int)vcard.X_BT_UCI.size());
        EXPECT_EQ(0, (int)vcard.X_BT_UID.size());
        EXPECT_THAT("", vcard.FN);
        EXPECT_EQ(0, (int)vcard.TEL.size());
    }
    EXPECT_EQ(1, (int)testStruct.envelope_.recipientLevel2_.size());
    if ((int)testStruct.envelope_.recipientLevel2_.size() != 0) {
        auto it3 = testStruct.envelope_.recipientLevel2_.begin();
        vcard = *it3;
        EXPECT_THAT("2.1", vcard.VERSION);
        EXPECT_THAT("Laurent", vcard.N);
        EXPECT_THAT("laurent@ghi.edu", *(vcard.EMAIL.begin()));
        EXPECT_EQ(0, (int)vcard.X_BT_UCI.size());
        EXPECT_EQ(0, (int)vcard.X_BT_UID.size());
        EXPECT_THAT("", vcard.FN);
        EXPECT_EQ(0, (int)vcard.TEL.size());
    }
    EXPECT_EQ(0, (int)testStruct.envelope_.recipientLevel3_.size());
    EXPECT_EQ(2, testStruct.envelope_.maxLevelOfEnvelope_);
    EXPECT_THAT("8BIT", testStruct.envelope_.msgBody_.body_encoding);
    EXPECT_EQ(125, testStruct.envelope_.msgBody_.body_content_length);
    EXPECT_THAT("Date: 20 Jun 96\nSubject: Fish\nFrom: tanaka@def.edu\nTo: laurent@ghi.edu\nLet's go fishing!\nBR, Mat",
        testStruct.envelope_.msgBody_.body_content);
    EXPECT_EQ(0, 0);
}

TEST(MceBmessageParamMakeStringObject, BMessageMakeStringObject_ProfileBmessageV1_0)
{
    // buide struct
    MceBmessageParamAnalyser testerString(profileBmessageV1_0);
    testerString.StartAnalyse();
    IProfileBMessageStruct testMsgStruct = testerString.GetMsgStruct();

    // Make string object
    MceBmessageParamMakeStringObject makeObject(testMsgStruct);
    std::string newStringObject = makeObject.GetStringObject();

    // check struct with new string object
    MceBmessageParamAnalyser tester(newStringObject);
    tester.StartAnalyse();
    IProfileBMessageStruct testStruct = tester.GetMsgStruct();
    IProfileMapVcard vcard;
    EXPECT_THAT("1.0", testStruct.version_property);
    EXPECT_EQ(MapMessageStatus::UNREAD, testStruct.readstatus_property);
    EXPECT_EQ(MapMessageType::EMAIL, testStruct.type_property);
    EXPECT_THAT(u"TELECOM/MSG/INBOX", testStruct.folder_property);
    EXPECT_THAT("", testStruct.extendeddata_property);
    EXPECT_EQ(1, (int)testStruct.originator_.size());
    if (testStruct.originator_.size() != 0) {
        auto it = testStruct.originator_.begin();
        vcard = *it;
        EXPECT_THAT("2.1", vcard.VERSION);
        EXPECT_THAT("Mat", vcard.N);
        EXPECT_THAT("ma@abc.edu", *(vcard.EMAIL.begin()));
        EXPECT_EQ(0, (int)vcard.X_BT_UCI.size());
        EXPECT_EQ(0, (int)vcard.X_BT_UID.size());
        EXPECT_THAT("", vcard.FN);
        EXPECT_EQ(0, (int)vcard.TEL.size());
    }
    EXPECT_EQ(1, (int)testStruct.envelope_.recipientLevel1_.size());
    if ((int)testStruct.envelope_.recipientLevel1_.size() != 0) {
        auto it2 = testStruct.envelope_.recipientLevel1_.begin();
        vcard = *it2;
        EXPECT_THAT("2.1", vcard.VERSION);
        EXPECT_THAT("Tanaka", vcard.N);
        EXPECT_THAT("tanaka@def.edu", *(vcard.EMAIL.begin()));
        EXPECT_EQ(0, (int)vcard.X_BT_UCI.size());
        EXPECT_EQ(0, (int)vcard.X_BT_UID.size());
        EXPECT_THAT("", vcard.FN);
        EXPECT_EQ(0, (int)vcard.TEL.size());
    }
    EXPECT_EQ(1, (int)testStruct.envelope_.recipientLevel2_.size());
    if ((int)testStruct.envelope_.recipientLevel2_.size() != 0) {
        auto it3 = testStruct.envelope_.recipientLevel2_.begin();
        vcard = *it3;
        EXPECT_THAT("2.1", vcard.VERSION);
        EXPECT_THAT("Laurent", vcard.N);
        EXPECT_THAT("laurent@ghi.edu", *(vcard.EMAIL.begin()));
        EXPECT_EQ(0, (int)vcard.X_BT_UCI.size());
        EXPECT_EQ(0, (int)vcard.X_BT_UID.size());
        EXPECT_THAT("", vcard.FN);
        EXPECT_EQ(0, (int)vcard.TEL.size());
    }
    EXPECT_EQ(0, (int)testStruct.envelope_.recipientLevel3_.size());
    EXPECT_EQ(2, testStruct.envelope_.maxLevelOfEnvelope_);
    EXPECT_THAT("8BIT", testStruct.envelope_.msgBody_.body_encoding);
    EXPECT_EQ(125, testStruct.envelope_.msgBody_.body_content_length);
    EXPECT_THAT("Date: 20 Jun 96\nSubject: Fish\nFrom: tanaka@def.edu\nTo: laurent@ghi.edu\nLet's go fishing!\nBR, Mat",
        testStruct.envelope_.msgBody_.body_content);
    EXPECT_EQ(0, 0);
}

const char *profileBmessageV1_1 = "BEGIN:BMSG"
                                  "\nVERSION:1.1"
                                  "\nSTATUS:UNREAD"
                                  "\nTYPE:IM"
                                  "\nFOLDER:TELECOM/MSG/INBOX"
                                  "\nEXTENDEDDATA:0:3;"
                                  "\nBEGIN:VCARD"
                                  "\nVERSION:2.1"
                                  "\nN:L;Marry"
                                  "\nX-BT-UCI:whateverapp:497654321@s.whateverapp.net"
                                  "\nEND:VCARD"
                                  "\nBEGIN:BENV"
                                  "\nBEGIN:VCARD"
                                  "\nVERSION:2.1"
                                  "\nN:S.;Alois"
                                  "\nX-BT-UCI:whateverapp:4912345@s.whateverapp.net"
                                  "\nEND:VCARD"
                                  "\nBEGIN:BBODY"
                                  "\nENCODING:8BIT"
                                  "\nLENGTH:"
                                  "\nBEGIN:MSG"
                                  "\nDate: Fri, 1 Aug 2014 01:29:12 +01:00"
                                  "\nFrom: whateverapp:497654321@s.whateverapp.net"
                                  "\nTo: whateverapp:4912345@s.whateverapp.net"
                                  "\nContent-Type: text/plain;"
                                  "\nHappy birthday"
                                  "\nEND:MSG"
                                  "\nEND:BBODY"
                                  "\nEND:BENV"
                                  "\nEND:BMSG";

TEST(MceBmessageParamAnalyser, IProfileBMessageStruct_ProfileBmessageV1_1)
{
    MceBmessageParamAnalyser tester(profileBmessageV1_1);
    tester.StartAnalyse();
    IProfileBMessageStruct testStruct = tester.GetMsgStruct();
    IProfileMapVcard vcard;

    EXPECT_THAT("1.1", testStruct.version_property);
    EXPECT_EQ(MapMessageStatus::UNREAD, testStruct.readstatus_property);
    EXPECT_EQ(MapMessageType::IM, testStruct.type_property);
    EXPECT_THAT(u"TELECOM/MSG/INBOX", testStruct.folder_property);
    EXPECT_THAT("0:3;", testStruct.extendeddata_property);
    EXPECT_EQ(1, (int)testStruct.originator_.size());
    if (testStruct.originator_.size() != 0) {
        auto it = testStruct.originator_.begin();
        vcard = *it;
        EXPECT_THAT("2.1", vcard.VERSION);
        EXPECT_THAT("L;Marry", vcard.N);
        EXPECT_THAT("whateverapp:497654321@s.whateverapp.net", *(vcard.X_BT_UCI.begin()));
    }
    EXPECT_EQ(1, (int)testStruct.envelope_.recipientLevel1_.size());
    if ((int)testStruct.envelope_.recipientLevel1_.size() != 0) {
        auto it2 = testStruct.envelope_.recipientLevel1_.begin();
        vcard = *it2;
        EXPECT_THAT("2.1", vcard.VERSION);
        EXPECT_THAT("S.;Alois", vcard.N);
        EXPECT_THAT("whateverapp:4912345@s.whateverapp.net", *(vcard.X_BT_UCI.begin()));
    }
    EXPECT_EQ(0, (int)testStruct.envelope_.recipientLevel2_.size());
    EXPECT_EQ(0, (int)testStruct.envelope_.recipientLevel3_.size());
    EXPECT_EQ(1, testStruct.envelope_.maxLevelOfEnvelope_);
    EXPECT_THAT("8BIT", testStruct.envelope_.msgBody_.body_encoding);
    EXPECT_THAT(0, testStruct.envelope_.msgBody_.body_content_length);
    EXPECT_THAT("Date: Fri, 1 Aug 2014 01:29:12 +01:00\nFrom: whateverapp:497654321@s.whateverapp.net"
                "\nTo: whateverapp:4912345@s.whateverapp.net\nContent-Type: text/plain;\nHappy birthday",
        testStruct.envelope_.msgBody_.body_content);
    EXPECT_EQ(0, 0);
}

TEST(MceBmessageParamMakeStringObject, BMessageMakeStringObject_ProfileBmessageV1_1)
{
    // buide struct
    MceBmessageParamAnalyser testerString(profileBmessageV1_1);
    testerString.StartAnalyse();
    IProfileBMessageStruct structParam = testerString.GetMsgStruct();

    // Make string object
    MceBmessageParamMakeStringObject makeObject(structParam);
    std::string newStringObject = makeObject.GetStringObject();

    // buide struct again
    MceBmessageParamAnalyser tester(newStringObject);
    tester.StartAnalyse();
    IProfileBMessageStruct testStruct = tester.GetMsgStruct();
    IProfileMapVcard vcard;

    EXPECT_THAT("1.1", testStruct.version_property);
    EXPECT_EQ(MapMessageStatus::UNREAD, testStruct.readstatus_property);
    EXPECT_EQ(MapMessageType::IM, testStruct.type_property);
    EXPECT_THAT(u"TELECOM/MSG/INBOX", testStruct.folder_property);
    EXPECT_THAT("0:3;", testStruct.extendeddata_property);
    EXPECT_EQ(1, (int)testStruct.originator_.size());
    if (testStruct.originator_.size() != 0) {
        auto it = testStruct.originator_.begin();
        vcard = *it;
        EXPECT_THAT("2.1", vcard.VERSION);
        EXPECT_THAT("L;Marry", vcard.N);
        EXPECT_THAT("whateverapp:497654321@s.whateverapp.net", *(vcard.X_BT_UCI.begin()));
    }
    EXPECT_EQ(1, (int)testStruct.envelope_.recipientLevel1_.size());
    if ((int)testStruct.envelope_.recipientLevel1_.size() != 0) {
        auto it2 = testStruct.envelope_.recipientLevel1_.begin();
        vcard = *it2;
        EXPECT_THAT("2.1", vcard.VERSION);
        EXPECT_THAT("S.;Alois", vcard.N);
        EXPECT_THAT("whateverapp:4912345@s.whateverapp.net", *(vcard.X_BT_UCI.begin()));
    }
    EXPECT_EQ(0, (int)testStruct.envelope_.recipientLevel2_.size());
    EXPECT_EQ(0, (int)testStruct.envelope_.recipientLevel3_.size());
    EXPECT_EQ(1, testStruct.envelope_.maxLevelOfEnvelope_);
    EXPECT_THAT("8BIT", testStruct.envelope_.msgBody_.body_encoding);
    EXPECT_THAT(0, testStruct.envelope_.msgBody_.body_content_length);
    EXPECT_THAT("Date: Fri, 1 Aug 2014 01:29:12 +01:00\nFrom: whateverapp:497654321@s.whateverapp.net"
                "\nTo: whateverapp:4912345@s.whateverapp.net\nContent-Type: text/plain;\nHappy birthday",
        testStruct.envelope_.msgBody_.body_content);
    EXPECT_EQ(0, 0);
}

const char *realphoneBmessage1 = "BEGIN:BMSG"
                                 "\nVERSION:1.0"
                                 "\nSTATUS:READ"
                                 "\nTYPE:SMS_GSM"
                                 "\nFOLDER:telecom/msg/inbox"
                                 "\nBEGIN:VCARD"
                                 "\nVERSION:3.0"
                                 "\nFN:"
                                 "\nN:"
                                 "\nTEL:95533"
                                 "\nEND:VCARD"
                                 "\nBEGIN:BENV"
                                 "\nBEGIN:BBODY"
                                 "\nCHARSET:UTF-8"
                                 "\nLENGTH:169"
                                 "\nBEGIN:MSG"
                                 "\n序号01的验证码605888，您向刘某某尾号1288账户转账50000.0元。"
                                 "任何索要验证码的都是骗子，千万别给！[建设银行]"
                                 "\nEND:MSG"
                                 "\nEND:BBODY"
                                 "\nEND:BENV"
                                 "\nEND:BMSG";

TEST(MceBmessageParamAnalyser, IProfileBMessageStruct_realphoneBmessage1)
{
    MceBmessageParamAnalyser tester(realphoneBmessage1);
    tester.StartAnalyse();
    IProfileBMessageStruct testStruct = tester.GetMsgStruct();
    IProfileMapVcard vcard;
    EXPECT_THAT("1.0", testStruct.version_property);
    EXPECT_EQ(MapMessageStatus::READ, testStruct.readstatus_property);
    EXPECT_EQ(MapMessageType::SMS_GSM, testStruct.type_property);
    EXPECT_THAT(u"telecom/msg/inbox", testStruct.folder_property);
    EXPECT_THAT("", testStruct.extendeddata_property);
    EXPECT_EQ(1, (int)testStruct.originator_.size());
    if (testStruct.originator_.size() != 0) {
        auto it = testStruct.originator_.begin();
        vcard = *it;
        EXPECT_THAT("3.0", vcard.VERSION);
        EXPECT_THAT("", vcard.N);
        EXPECT_THAT("", vcard.FN);
        EXPECT_THAT("95533", *(vcard.TEL.begin()));
        EXPECT_EQ(0, (int)vcard.X_BT_UCI.size());
        EXPECT_EQ(0, (int)vcard.X_BT_UID.size());
    }
    EXPECT_EQ(0, (int)testStruct.envelope_.recipientLevel1_.size());
    EXPECT_EQ(0, (int)testStruct.envelope_.recipientLevel2_.size());
    EXPECT_EQ(0, (int)testStruct.envelope_.recipientLevel3_.size());
    EXPECT_EQ(1, testStruct.envelope_.maxLevelOfEnvelope_);
    EXPECT_THAT("", testStruct.envelope_.msgBody_.body_encoding);
    EXPECT_THAT("UTF-8", testStruct.envelope_.msgBody_.body_charset);
    EXPECT_THAT(169, testStruct.envelope_.msgBody_.body_content_length);
    EXPECT_THAT(
        "序号01的验证码605888，您向刘某某尾号1288账户转账50000.0元。任何索要验证码的都是骗子，千万别给！[建设银行]",
        testStruct.envelope_.msgBody_.body_content);
    EXPECT_EQ(0, 0);
}

TEST(MceBmessageParamMakeStringObject, BMessageMakeStringObject_realphoneBmessage1)
{
    // buide struct
    MceBmessageParamAnalyser testerString(realphoneBmessage1);
    testerString.StartAnalyse();
    IProfileBMessageStruct structParam = testerString.GetMsgStruct();

    // Make string object
    MceBmessageParamMakeStringObject makeObject(structParam);
    std::string newStringObject = makeObject.GetStringObject();

    // check new string object agian
    MceBmessageParamAnalyser tester(newStringObject);
    tester.StartAnalyse();
    IProfileBMessageStruct testStruct = tester.GetMsgStruct();
    IProfileMapVcard vcard;
    EXPECT_THAT("1.0", testStruct.version_property);
    EXPECT_EQ(MapMessageStatus::READ, testStruct.readstatus_property);
    EXPECT_EQ(MapMessageType::SMS_GSM, testStruct.type_property);
    EXPECT_THAT(u"telecom/msg/inbox", testStruct.folder_property);
    EXPECT_THAT("", testStruct.extendeddata_property);
    EXPECT_EQ(1, (int)testStruct.originator_.size());
    if (testStruct.originator_.size() != 0) {
        auto it = testStruct.originator_.begin();
        vcard = *it;
        EXPECT_THAT("3.0", vcard.VERSION);
        EXPECT_THAT("", vcard.N);
        EXPECT_THAT("", vcard.FN);
        EXPECT_THAT("95533", *(vcard.TEL.begin()));
        EXPECT_THAT(0, (int)vcard.X_BT_UCI.size());
        EXPECT_THAT(0, (int)vcard.X_BT_UID.size());
    }
    EXPECT_EQ(0, (int)testStruct.envelope_.recipientLevel1_.size());
    EXPECT_EQ(0, (int)testStruct.envelope_.recipientLevel2_.size());
    EXPECT_EQ(0, (int)testStruct.envelope_.recipientLevel3_.size());
    EXPECT_EQ(1, testStruct.envelope_.maxLevelOfEnvelope_);
    EXPECT_THAT("", testStruct.envelope_.msgBody_.body_encoding);
    EXPECT_THAT("UTF-8", testStruct.envelope_.msgBody_.body_charset);
    EXPECT_THAT(169, testStruct.envelope_.msgBody_.body_content_length);
    EXPECT_THAT(
        "序号01的验证码605888，您向刘某某尾号1288账户转账50000.0元。任何索要验证码的都是骗子，千万别给！[建设银行]",
        testStruct.envelope_.msgBody_.body_content);
    EXPECT_EQ(0, 0);
}

const char *realphoneBmessage2 = "BEGIN:BMSG"
                                 "\nVERSION:1.0"
                                 "\nSTATUS:READ"
                                 "\nTYPE:SMS_GSM"
                                 "\nFOLDER:telecom/msg/inbox"
                                 "\nBEGIN:VCARD"
                                 "\nVERSION:3.0"
                                 "\nFN:"
                                 "\nN:"
                                 "\nTEL:106581226300"
                                 "\nEND:VCARD"
                                 "\nBEGIN:BENV"
                                 "\nBEGIN:BBODY"
                                 "\nCHARSET:UTF-8"
                                 "\nLENGTH:83"
                                 "\nBEGIN:MSG"
                                 "\n尊敬的中国移动用户，您的短信验证码为:108800"
                                 "\nEND:MSG"
                                 "\nEND:BBODY"
                                 "\nEND:BENV"
                                 "\nEND:BMSG";

TEST(MceSplitString, SplitRealphoneBmessage2)
{
    std::string filter(charFilter);
    MceSplitString tester(realphoneBmessage2, filter);
    EXPECT_THAT("BEGIN:BMSG", tester.NextWord());
    EXPECT_THAT("VERSION:1.0", tester.NextWord());
    EXPECT_THAT("STATUS:READ", tester.NextWord());
    EXPECT_THAT("TYPE:SMS_GSM", tester.NextWord());
    EXPECT_THAT("FOLDER:telecom/msg/inbox", tester.NextWord());
    EXPECT_THAT("BEGIN:VCARD", tester.NextWord());
    EXPECT_THAT("VERSION:3.0", tester.NextWord());
    EXPECT_THAT("FN:", tester.NextWord());
    EXPECT_THAT("N:", tester.NextWord());
    EXPECT_THAT("TEL:106581226300", tester.NextWord());
    EXPECT_THAT("END:VCARD", tester.NextWord());
    EXPECT_THAT("BEGIN:BENV", tester.NextWord());
    EXPECT_THAT("BEGIN:BBODY", tester.NextWord());
    EXPECT_THAT("CHARSET:UTF-8", tester.NextWord());
    EXPECT_THAT("LENGTH:83", tester.NextWord());
    EXPECT_THAT("BEGIN:MSG", tester.NextWord());
    EXPECT_THAT("尊敬的中国移动用户，您的短信验证码为:108800", tester.NextWord());
    EXPECT_THAT("END:MSG", tester.NextWord());
    EXPECT_THAT("END:BBODY", tester.NextWord());
    EXPECT_THAT("END:BENV", tester.NextWord());
    EXPECT_THAT("END:BMSG", tester.NextWord());
    EXPECT_THAT("", tester.NextWord());
    EXPECT_EQ(0, 0);
}

const char *profileBmessageV1_1_testData = "BEGIN:BMSG"
                                           "\nVERSION:1.1"
                                           "\nSTATUS:UNREAD"
                                           "\nTYPE:SMS_CDMA"
                                           "\nFOLDER:TELECOM/MSG/INBOX"
                                           "\nEXTENDEDDATA:0:3;"
                                           "\nBEGIN:VCARD"
                                           "\nVERSION:3.0"
                                           "\nN:JoachimRootvcard"
                                           "\nFN:LMarry"
                                           "\nX-BT-UCI:uci245root"
                                           "\nX-BT-UID:WWW.sina.net"
                                           "\nTEL:00498912345678"
                                           "\nEMAIL:111@neusoft.com"
                                           "\nEND:VCARD"
                                           "\nBEGIN:BENV"
                                           "\nBEGIN:VCARD"
                                           "\nVERSION:3.0"
                                           "\nN:level1vcard"
                                           "\nFN:LMarryyyy"
                                           "\nX-BT-UCI:whateverappXXXXX11111"
                                           "\nX-BT-UID:WWW.sina.net99999"
                                           "\nEMAIL:222@neusoft.com"
                                           "\nTEL:00498912345678675674567"
                                           "\nEND:VCARD"
                                           "\nBEGIN:BENV"
                                           "\nBEGIN:VCARD"
                                           "\nVERSION:3.0"
                                           "\nN:JoachimLevel2_1"
                                           "\nFN:LMarry"
                                           "\nX-BT-UCI:whateverapp:497654321@s.whateverapp.net2222221111"
                                           "\nX-BT-UID:WWW.sina.net"
                                           "\nTEL:004989123456782222"
                                           "\nEMAIL:333@neusoft.com"
                                           "\nEND:VCARD"
                                           "\nBEGIN:VCARD"
                                           "\nVERSION:3.0"
                                           "\nN:JoachimLevel2_2"
                                           "\nFN:LMarry"
                                           "\nX-BT-UCI:whateverapp:497654321@s.whateverapp.net2222"
                                           "\nX-BT-UID:WWW.sina.net222"
                                           "\nTEL:004989123456782222"
                                           "\nEMAIL:444@neusoft.com"
                                           "\nEND:VCARD"
                                           "\nBEGIN:BENV"
                                           "\nBEGIN:VCARD"
                                           "\nVERSION:3.0"
                                           "\nN:JoachimLevel3"
                                           "\nFN:LMarry3"
                                           "\nX-BT-UCI:whateverapp:497654321@s.whateverapp.net333"
                                           "\nX-BT-UID:WWW.sina.net333"
                                           "\nTEL:0049891234567833333"
                                           "\nEND:VCARD"
                                           "\nBEGIN:BBODY"
                                           "\nENCODING:G-7BIT"
                                           "\nLENGTH:96"
                                           "\nCHARSET:16-BIT"
                                           "\nLANGUAGE:ENGLISTH"
                                           "\nPARTID:partID"
                                           "\nBEGIN:MSG"
                                           "\n0191000E9100949821436587000011303231"
                                           "\n12928211CC32FD34079DDF20737A8E4EBBCF21"
                                           "\nEND:MSG"
                                           "\nEND:BBODY"
                                           "\nEND:BENV"
                                           "\nEND:BENV"
                                           "\nEND:BENV"
                                           "\nEND:BMSG";

TEST(MceBmessageParamAnalyser, IProfileBMessageStruct_profileBmessageV1_1_testData)
{
    MceBmessageParamAnalyser tester(profileBmessageV1_1_testData);
    tester.StartAnalyse();
    IProfileBMessageStruct testStruct = tester.GetMsgStruct();
    IProfileMapVcard vcard;
    EXPECT_THAT("1.1", testStruct.version_property);
    EXPECT_EQ(MapMessageStatus::UNREAD, testStruct.readstatus_property);
    EXPECT_EQ(MapMessageType::SMS_CDMA, testStruct.type_property);
    EXPECT_THAT(u"TELECOM/MSG/INBOX", testStruct.folder_property);
    EXPECT_THAT("0:3;", testStruct.extendeddata_property);
    EXPECT_EQ(1, (int)testStruct.originator_.size());
    if (testStruct.originator_.size() != 0) {
        auto it = testStruct.originator_.begin();
        vcard = *it;
        EXPECT_THAT("3.0", vcard.VERSION);
        EXPECT_THAT("JoachimRootvcard", vcard.N);
        EXPECT_THAT("LMarry", vcard.FN);
        EXPECT_THAT("00498912345678", *(vcard.TEL.begin()));
        EXPECT_THAT("uci245root", *(vcard.X_BT_UCI.begin()));
        EXPECT_THAT("WWW.sina.net", *(vcard.X_BT_UID.begin()));
    }
    EXPECT_EQ(1, (int)testStruct.envelope_.recipientLevel1_.size());
    if ((int)testStruct.envelope_.recipientLevel1_.size() != 0) {
        auto it2 = testStruct.envelope_.recipientLevel1_.begin();
        vcard = *it2;
        EXPECT_THAT("3.0", vcard.VERSION);
        EXPECT_THAT("level1vcard", vcard.N);
        EXPECT_THAT("LMarryyyy", vcard.FN);
        EXPECT_THAT("00498912345678675674567", *(vcard.TEL.begin()));
        EXPECT_THAT("whateverappXXXXX11111", *(vcard.X_BT_UCI.begin()));
        EXPECT_THAT("WWW.sina.net99999", *(vcard.X_BT_UID.begin()));
    }
    EXPECT_EQ(2, (int)testStruct.envelope_.recipientLevel2_.size());
    if ((int)testStruct.envelope_.recipientLevel2_.size() != 0) {
        auto it2 = testStruct.envelope_.recipientLevel2_.begin();
        vcard = *it2;
        EXPECT_THAT("3.0", vcard.VERSION);
        EXPECT_THAT("JoachimLevel2_1", vcard.N);
        EXPECT_THAT("LMarry", vcard.FN);
        EXPECT_THAT("004989123456782222", *(vcard.TEL.begin()));
        EXPECT_THAT("whateverapp:497654321@s.whateverapp.net2222221111", *(vcard.X_BT_UCI.begin()));
        EXPECT_THAT("WWW.sina.net", *(vcard.X_BT_UID.begin()));
    }
    if ((int)testStruct.envelope_.recipientLevel2_.size() != 0) {
        auto it3 = testStruct.envelope_.recipientLevel2_.begin();
        it3++;
        if (it3 != testStruct.envelope_.recipientLevel2_.end()) {
            vcard = *it3;
            EXPECT_THAT("3.0", vcard.VERSION);
            EXPECT_THAT("JoachimLevel2_2", vcard.N);
            EXPECT_THAT("LMarry", vcard.FN);
            EXPECT_THAT("004989123456782222", *(vcard.TEL.begin()));
            EXPECT_THAT("whateverapp:497654321@s.whateverapp.net2222", *(vcard.X_BT_UCI.begin()));
            EXPECT_THAT("WWW.sina.net222", *(vcard.X_BT_UID.begin()));
        }
    }
    EXPECT_EQ(1, (int)testStruct.envelope_.recipientLevel3_.size());
    if ((int)testStruct.envelope_.recipientLevel3_.size() != 0) {
        auto it3 = testStruct.envelope_.recipientLevel3_.begin();
        vcard = *it3;
        EXPECT_THAT("3.0", vcard.VERSION);
        EXPECT_THAT("JoachimLevel3", vcard.N);
        EXPECT_THAT("LMarry3", vcard.FN);
        EXPECT_THAT("0049891234567833333", *(vcard.TEL.begin()));
        EXPECT_THAT("whateverapp:497654321@s.whateverapp.net333", *(vcard.X_BT_UCI.begin()));
        EXPECT_THAT("WWW.sina.net333", *(vcard.X_BT_UID.begin()));
    }
    EXPECT_EQ(3, testStruct.envelope_.maxLevelOfEnvelope_);
    EXPECT_THAT("G-7BIT", testStruct.envelope_.msgBody_.body_encoding);
    EXPECT_THAT(96, testStruct.envelope_.msgBody_.body_content_length);
    EXPECT_THAT("ENGLISTH", testStruct.envelope_.msgBody_.body_language);
    EXPECT_THAT("16-BIT", testStruct.envelope_.msgBody_.body_charset);
    EXPECT_THAT("0191000E9100949821436587000011303231\n12928211CC32FD34079DDF20737A8E4EBBCF21",
        testStruct.envelope_.msgBody_.body_content);
    EXPECT_EQ(0, 0);
}

// multi vard members
const char *profileBmessageV1_1_testData2 = "BEGIN:BMSG"
                                            "\nVERSION:1.1"
                                            "\nSTATUS:UNREAD"
                                            "\nTYPE:SMS_CDMA"
                                            "\nFOLDER:TELECOM/MSG/INBOX"
                                            "\nEXTENDEDDATA:0:3;"
                                            "\nBEGIN:VCARD"
                                            "\nVERSION:3.0"
                                            "\nN:JoachimRootvcard"
                                            "\nFN:LMarry"
                                            "\nX-BT-UCI:uci245root"
                                            "\nX-BT-UID:WWW.sina.net"
                                            "\nTEL:00498912345678"
                                            "\nTEL:00498912345678_2"  // multi vcard members
                                            "\nTEL:00498912345678_3"
                                            "\nEMAIL:111@neusoft.com"
                                            "\nEMAIL:111@neusoft.com2"  // multi vcard members
                                            "\nEMAIL:111@neusoft.com3"
                                            "\nEMAIL:111@neusoft.com4"
                                            "\nEMAIL:111@neusoft.com5"
                                            "\nEMAIL:111@neusoft.com6"
                                            "\nEND:VCARD"
                                            "\nBEGIN:BENV"
                                            "\nBEGIN:VCARD"
                                            "\nVERSION:3.0"
                                            "\nN:JoachimLevel3"
                                            "\nFN:LMarry3"
                                            "\nX-BT-UCI:whateverapp:497654321@s.whateverapp.net333"
                                            "\nX-BT-UCI:whateverapp:497654321@s.whateverapp.net333_2"
                                            "\nX-BT-UCI:whateverapp:497654321@s.whateverapp.net333_3"
                                            "\nX-BT-UCI:whateverapp:497654321@s.whateverapp.net333_4"
                                            "\nX-BT-UID:WWW.sina.net333"
                                            "\nX-BT-UID:WWW.sina.net333_2"
                                            "\nX-BT-UID:WWW.sina.net333_3"
                                            "\nTEL:0049891234567833333"
                                            "\nEND:VCARD"
                                            "\nBEGIN:BBODY"
                                            "\nENCODING:G-7BIT"
                                            "\nLENGTH:96"
                                            "\nCHARSET:16-BIT"
                                            "\nLANGUAGE:ENGLISTH"
                                            "\nPARTID:partID"
                                            "\nBEGIN:MSG"
                                            "\n0191000E9100949821436587000011303231"
                                            "\n12928211CC32FD34079DDF20737A8E4EBBCF21"
                                            "\nEND:MSG"
                                            "\nEND:BBODY"
                                            "\nEND:BENV"
                                            "\nEND:BMSG";

TEST(MceBmessageParamAnalyser, IProfileBMessageStruct_profileBmessageV1_1_testData2)
{
    MceBmessageParamAnalyser tester(profileBmessageV1_1_testData2);
    tester.StartAnalyse();
    IProfileBMessageStruct testStruct = tester.GetMsgStruct();
    IProfileMapVcard vcard;
    EXPECT_THAT("1.1", testStruct.version_property);
    EXPECT_EQ(MapMessageStatus::UNREAD, testStruct.readstatus_property);
    EXPECT_EQ(MapMessageType::SMS_CDMA, testStruct.type_property);
    EXPECT_THAT(u"TELECOM/MSG/INBOX", testStruct.folder_property);
    EXPECT_THAT("0:3;", testStruct.extendeddata_property);
    EXPECT_EQ(1, (int)testStruct.originator_.size());
    std::vector<std::string>::iterator itString;
    if (testStruct.originator_.size() != 0) {
        auto it = testStruct.originator_.begin();
        vcard = *it;
        EXPECT_THAT("3.0", vcard.VERSION);
        EXPECT_THAT("JoachimRootvcard", vcard.N);
        EXPECT_THAT("LMarry", vcard.FN);
        if (vcard.TEL.size() != 3) {
            EXPECT_EQ(0, 1);
        } else {
            itString = vcard.TEL.begin();
            EXPECT_THAT("00498912345678", *itString++);
            EXPECT_THAT("00498912345678_2", *itString++);
            EXPECT_THAT("00498912345678_3", *itString);
        }
        if (vcard.X_BT_UCI.size() == 0) {
            EXPECT_EQ(0, 1);
        } else {
            EXPECT_THAT("uci245root", *(vcard.X_BT_UCI.begin()));
        }
        if (vcard.X_BT_UID.size() == 0) {
            EXPECT_EQ(0, 1);
        } else {
            EXPECT_THAT("WWW.sina.net", *(vcard.X_BT_UID.begin()));
        }
        if (vcard.EMAIL.size() != 6) {
            EXPECT_EQ(0, 1);
        } else {
            itString = vcard.EMAIL.begin();
            EXPECT_THAT("111@neusoft.com", *itString++);
            EXPECT_THAT("111@neusoft.com2", *itString++);
            EXPECT_THAT("111@neusoft.com3", *itString++);
            EXPECT_THAT("111@neusoft.com4", *itString++);
            EXPECT_THAT("111@neusoft.com5", *itString++);
            EXPECT_THAT("111@neusoft.com6", *itString++);
        }
    }
    EXPECT_EQ(1, (int)testStruct.envelope_.recipientLevel1_.size());
    if ((int)testStruct.envelope_.recipientLevel1_.size() != 0) {
        auto it3 = testStruct.envelope_.recipientLevel1_.begin();
        vcard = *it3;
        EXPECT_THAT("3.0", vcard.VERSION);
        EXPECT_THAT("JoachimLevel3", vcard.N);
        EXPECT_THAT("LMarry3", vcard.FN);
        if (vcard.TEL.size() == 0) {
            EXPECT_EQ(0, 1);
        } else {
            EXPECT_THAT("0049891234567833333", *(vcard.TEL.begin()));
        }
        if (vcard.X_BT_UCI.size() != 4) {
            EXPECT_EQ(0, 1);
        } else {
            itString = vcard.X_BT_UCI.begin();
            EXPECT_THAT("whateverapp:497654321@s.whateverapp.net333", *itString++);
            EXPECT_THAT("whateverapp:497654321@s.whateverapp.net333_2", *itString++);
            EXPECT_THAT("whateverapp:497654321@s.whateverapp.net333_3", *itString++);
            EXPECT_THAT("whateverapp:497654321@s.whateverapp.net333_4", *itString++);
        }
        if (vcard.X_BT_UID.size() != 3) {
            EXPECT_EQ(0, 1);
        } else {
            itString = vcard.X_BT_UID.begin();
            EXPECT_THAT("WWW.sina.net333", *itString++);
            EXPECT_THAT("WWW.sina.net333_2", *itString++);
            EXPECT_THAT("WWW.sina.net333_3", *itString++);
        }
    }
    EXPECT_EQ(1, testStruct.envelope_.maxLevelOfEnvelope_);
    EXPECT_THAT("G-7BIT", testStruct.envelope_.msgBody_.body_encoding);
    EXPECT_THAT(96, testStruct.envelope_.msgBody_.body_content_length);
    EXPECT_THAT("ENGLISTH", testStruct.envelope_.msgBody_.body_language);
    EXPECT_THAT("16-BIT", testStruct.envelope_.msgBody_.body_charset);
    EXPECT_THAT("0191000E9100949821436587000011303231\n12928211CC32FD34079DDF20737A8E4EBBCF21",
        testStruct.envelope_.msgBody_.body_content);
    EXPECT_EQ(0, 0);
}

TEST(MceBmessageParamMakeStringObject, BMessageMakeStringObject_profileBmessageV1_1_testData)
{
    // buide struct
    MceBmessageParamAnalyser testerString(profileBmessageV1_1_testData);
    testerString.StartAnalyse();
    IProfileBMessageStruct structParam = testerString.GetMsgStruct();

    // Make string object
    MceBmessageParamMakeStringObject makeObject(structParam);
    std::string newStringObject = makeObject.GetStringObject();

    // check new string object agian
    MceBmessageParamAnalyser tester(newStringObject);
    tester.StartAnalyse();
    IProfileBMessageStruct testStruct = tester.GetMsgStruct();
    IProfileMapVcard vcard;
    EXPECT_THAT("1.1", testStruct.version_property);
    EXPECT_EQ(MapMessageStatus::UNREAD, testStruct.readstatus_property);
    EXPECT_EQ(MapMessageType::SMS_CDMA, testStruct.type_property);
    EXPECT_THAT(u"TELECOM/MSG/INBOX", testStruct.folder_property);
    EXPECT_THAT("0:3;", testStruct.extendeddata_property);
    EXPECT_EQ(1, (int)testStruct.originator_.size());
    if (testStruct.originator_.size() != 0) {
        auto it = testStruct.originator_.begin();
        vcard = *it;
        EXPECT_THAT("3.0", vcard.VERSION);
        EXPECT_THAT("JoachimRootvcard", vcard.N);
        EXPECT_THAT("LMarry", vcard.FN);
        EXPECT_THAT("00498912345678", *(vcard.TEL.begin()));
        EXPECT_THAT("uci245root", *(vcard.X_BT_UCI.begin()));
        EXPECT_THAT("WWW.sina.net", *(vcard.X_BT_UID.begin()));
    }
    EXPECT_EQ(1, (int)testStruct.envelope_.recipientLevel1_.size());
    if ((int)testStruct.envelope_.recipientLevel1_.size() != 0) {
        auto it2 = testStruct.envelope_.recipientLevel1_.begin();
        vcard = *it2;
        EXPECT_THAT("3.0", vcard.VERSION);
        EXPECT_THAT("level1vcard", vcard.N);
        EXPECT_THAT("LMarryyyy", vcard.FN);
        EXPECT_THAT("00498912345678675674567", *(vcard.TEL.begin()));
        EXPECT_THAT("whateverappXXXXX11111", *(vcard.X_BT_UCI.begin()));
        EXPECT_THAT("WWW.sina.net99999", *(vcard.X_BT_UID.begin()));
    }
    EXPECT_EQ(2, (int)testStruct.envelope_.recipientLevel2_.size());
    if ((int)testStruct.envelope_.recipientLevel2_.size() != 0) {
        auto it2 = testStruct.envelope_.recipientLevel2_.begin();
        vcard = *it2;
        EXPECT_THAT("3.0", vcard.VERSION);
        EXPECT_THAT("JoachimLevel2_1", vcard.N);
        EXPECT_THAT("LMarry", vcard.FN);
        EXPECT_THAT("004989123456782222", *(vcard.TEL.begin()));
        EXPECT_THAT("whateverapp:497654321@s.whateverapp.net2222221111", *(vcard.X_BT_UCI.begin()));
        EXPECT_THAT("WWW.sina.net", *(vcard.X_BT_UID.begin()));
    }
    if ((int)testStruct.envelope_.recipientLevel2_.size() != 0) {
        auto it3 = testStruct.envelope_.recipientLevel2_.begin();
        it3++;
        if (it3 != testStruct.envelope_.recipientLevel2_.end()) {
            vcard = *it3;
            EXPECT_THAT("3.0", vcard.VERSION);
            EXPECT_THAT("JoachimLevel2_2", vcard.N);
            EXPECT_THAT("LMarry", vcard.FN);
            EXPECT_THAT("004989123456782222", *(vcard.TEL.begin()));
            EXPECT_THAT("whateverapp:497654321@s.whateverapp.net2222", *(vcard.X_BT_UCI.begin()));
            EXPECT_THAT("WWW.sina.net222", *(vcard.X_BT_UID.begin()));
        }
    }
    EXPECT_EQ(1, (int)testStruct.envelope_.recipientLevel3_.size());
    if ((int)testStruct.envelope_.recipientLevel3_.size() != 0) {
        auto it3 = testStruct.envelope_.recipientLevel3_.begin();
        vcard = *it3;
        EXPECT_THAT("3.0", vcard.VERSION);
        EXPECT_THAT("JoachimLevel3", vcard.N);
        EXPECT_THAT("LMarry3", vcard.FN);
        EXPECT_THAT("0049891234567833333", *(vcard.TEL.begin()));
        EXPECT_THAT("whateverapp:497654321@s.whateverapp.net333", *(vcard.X_BT_UCI.begin()));
        EXPECT_THAT("WWW.sina.net333", *(vcard.X_BT_UID.begin()));
    }
    EXPECT_EQ(3, testStruct.envelope_.maxLevelOfEnvelope_);
    EXPECT_THAT("G-7BIT", testStruct.envelope_.msgBody_.body_encoding);
    EXPECT_THAT(96, testStruct.envelope_.msgBody_.body_content_length);
    EXPECT_THAT("ENGLISTH", testStruct.envelope_.msgBody_.body_language);
    EXPECT_THAT("16-BIT", testStruct.envelope_.msgBody_.body_charset);
    EXPECT_THAT("0191000E9100949821436587000011303231\n12928211CC32FD34079DDF20737A8E4EBBCF21",
        testStruct.envelope_.msgBody_.body_content);
    EXPECT_EQ(0, 0);
}

const char *profileConvoListing_v1_0 =
    "<MAP-convo-listing version = \"1.0\">"
    "\n<conversation id=\"E1E2E3E4F1F2F3F4A1A2A3A4B1B2B3B4\" name=\"BeergardenConnection\" "
    "last_activity=\"20140612T105430+0100\" read_status=\"no\" version_counter=\"A1A1B2B2C3C3D4D5E5E6F6F7A7A8B8B\">"
    "\n<participant uci=\"4986925814@s.whateverapp.net\" display_name=\"Tien\" chat_state=\"3\" "
    "last_activity=\"20140612T105430+0100\"/>"
    "\n<participant uci=\"4912345678@s.whateverapp.net\" display_name=\"Jonas\" chat_state=\"5\" "
    "last_activity=\"20140610T115130+0100\"/>"
    "\n<participant uci=\"4913579864@s.whateverapp.net\" display_name=\"Max\" chat_state=\"2\" "
    "last_activity=\"20140608T225435+0100\"/>"
    "\n<participant uci=\"4924689753@s.whateverapp.net\" display_name=\"Nils\" chat_state=\"0\" "
    "last_activity=\"20140612T092320+0100\"/>"
    "\n<participant uci=\"4923568910@s.whateverapp.net\" display_name=\"Alex\" chat_state=\"4\" "
    "last_activity=\"20140612T104110+0100\"/>"
    "\n</conversation>"
    "\n<conversation id=\"C1C2C3C4D1D2D3D4E1E2E3E4F1F2F3F4\" name=\"\" last_activity=\"20140801T012900+0100\" "
    "read_status=\"yes\" version_counter=\"0A0A1B1B2C2C3D3D4E4E5F5F6A6A7B7B\">"
    "\n<participant uci=\"malo@email.de\" display_name=\"Mari\" chat_state=\"2\" "
    "last_activity=\"20140801T012900+0100\" x_bt_uid=\"A1A2A3A4B1B2C1C2D1D2E1E2E3E4F1F2\"/>"
    "\n</conversation>"
    "\n<conversation id=\"F1F2F3F4E1E2E3E4D1D2D3D4C1C2C3C4\" name=\"family\" last_activity=\"20140925T133700+0100\" "
    "read_status=\"yes\" version_counter=\"1A1A2B2B3C3C4D4D5E5E6F6F7A7A8B8B\">"
    "\n<participant uci=\"malo@email.de\" display_name=\"Mari\" chat_state=\"2\" "
    "last_activity=\"20140801T012900+0100\" x_bt_uid=\"A1A2A3A4B1B2C1C2D1D2E1E2E3E4F1F2\" name=\"Mareike Loh\" "
    "presence_availability=\"2\" presence_text=\"Wow it’s hot today\" priority=\"100\"/>"
    "\n<participant uci=\"alois.s@august.de\" display_name=\"Lil Al\" chat_state=\"1\" "
    "last_activity=\"20140801T012800+0100\" x_bt_uid=\"A1A2A3A4B1B2C1C2D1D2E1E2E3E4F1F2\" name=\"Alois S.\" "
    "presence_availability=\"0\" presence_text=\"On my way\" priority=\"100\"/>"
    "\n</conversation>"
    "\n</MAP-convo-listing>";

TEST(MceTypesConversationListing, profileConvoListing_v1_0)
{
    std::string testString(profileConvoListing_v1_0);
    IProfileConversationListingParamStruct initStruct;
    MceTypesConversationListing tester;
    tester.BuildObjectData(initStruct, testString);

    IProfileConversationListingParamStruct paramStruct = tester.GetParam();
    std::vector<IProfileConversation> conversationList = tester.GetList();
    std::string sstringObject = tester.GetStringObject();
    IProfileConversation conversation;
    IProfileParticipant participant;

    EXPECT_THAT("1.0", paramStruct.Version);
    int rootMask = 0;
    EXPECT_EQ(3, (int)conversationList.size());
    for (auto it = conversationList.begin(); it != conversationList.end(); it++) {
        conversation = *it;
        if (conversation.id == "E1E2E3E4F1F2F3F4A1A2A3A4B1B2B3B4") {
            rootMask |= 0b001;

            EXPECT_THAT("E1E2E3E4F1F2F3F4A1A2A3A4B1B2B3B4", conversation.id);
            EXPECT_THAT("BeergardenConnection", conversation.name);
            EXPECT_THAT("20140612T105430+0100", conversation.last_activity);
            EXPECT_THAT("no", conversation.read_status);
            EXPECT_THAT("A1A1B2B2C3C3D4D5E5E6F6F7A7A8B8B", conversation.version_counter);
            EXPECT_THAT("", conversation.summary);
            EXPECT_EQ(5, (int)conversation.participantList_.size());

            // loop control
            int mask = 0;
            for (auto it2 = conversation.participantList_.begin(); it2 != conversation.participantList_.end(); it2++) {
                participant = *it2;
                if (participant.uci == "4986925814@s.whateverapp.net") {
                    mask |= 0b00001;
                    EXPECT_THAT("4986925814@s.whateverapp.net", participant.uci);
                    EXPECT_THAT("Tien", participant.display_name);
                    EXPECT_THAT("3", participant.chat_state);
                    EXPECT_THAT("20140612T105430+0100", participant.last_activity);  ///
                    EXPECT_THAT("", participant.x_bt_uid);
                    EXPECT_THAT("", participant.name);
                    EXPECT_THAT("", participant.presence_availability);
                    EXPECT_THAT("", participant.presence_text);
                    EXPECT_THAT("", participant.priority);
                } else if (participant.uci == "4912345678@s.whateverapp.net") {
                    mask |= 0b00010;
                    EXPECT_THAT("4912345678@s.whateverapp.net", participant.uci);
                    EXPECT_THAT("Jonas", participant.display_name);
                    EXPECT_THAT("5", participant.chat_state);
                    EXPECT_THAT("20140610T115130+0100", participant.last_activity);  ////
                    EXPECT_THAT("", participant.x_bt_uid);
                    EXPECT_THAT("", participant.name);
                    EXPECT_THAT("", participant.presence_availability);
                    EXPECT_THAT("", participant.presence_text);
                    EXPECT_THAT("", participant.priority);
                } else if (participant.uci == "4913579864@s.whateverapp.net") {
                    mask |= 0b00100;
                    EXPECT_THAT("4913579864@s.whateverapp.net", participant.uci);
                    EXPECT_THAT("Max", participant.display_name);
                    EXPECT_THAT("2", participant.chat_state);  ///
                    EXPECT_THAT("20140608T225435+0100", participant.last_activity);
                    EXPECT_THAT("", participant.x_bt_uid);
                    EXPECT_THAT("", participant.name);
                    EXPECT_THAT("", participant.presence_availability);
                    EXPECT_THAT("", participant.presence_text);
                    EXPECT_THAT("", participant.priority);
                } else if (participant.uci == "4924689753@s.whateverapp.net") {
                    mask |= 0b01000;
                    EXPECT_THAT("4924689753@s.whateverapp.net", participant.uci);
                    EXPECT_THAT("Nils", participant.display_name);
                    EXPECT_THAT("0", participant.chat_state);  ///
                    EXPECT_THAT("20140612T092320+0100", participant.last_activity);
                    EXPECT_THAT("", participant.x_bt_uid);
                    EXPECT_THAT("", participant.name);
                    EXPECT_THAT("", participant.presence_availability);
                    EXPECT_THAT("", participant.presence_text);
                    EXPECT_THAT("", participant.priority);
                } else if (participant.uci == "4923568910@s.whateverapp.net") {
                    mask |= 0b10000;
                    EXPECT_THAT("4923568910@s.whateverapp.net", participant.uci);
                    EXPECT_THAT("Alex", participant.display_name);
                    EXPECT_THAT("4", participant.chat_state);  //
                    EXPECT_THAT("20140612T104110+0100", participant.last_activity);
                    EXPECT_THAT("", participant.x_bt_uid);
                    EXPECT_THAT("", participant.name);
                    EXPECT_THAT("", participant.presence_availability);
                    EXPECT_THAT("", participant.presence_text);
                    EXPECT_THAT("", participant.priority);
                } else {
                }
            }
            // all case in forloop is excuted
            EXPECT_EQ(0b11111, mask);

        } else if (conversation.id == "C1C2C3C4D1D2D3D4E1E2E3E4F1F2F3F4") {
            rootMask |= 0b010;

            EXPECT_THAT("C1C2C3C4D1D2D3D4E1E2E3E4F1F2F3F4", conversation.id);
            EXPECT_THAT("", conversation.name);
            EXPECT_THAT("20140801T012900+0100", conversation.last_activity);
            EXPECT_THAT("yes", conversation.read_status);
            EXPECT_THAT("0A0A1B1B2C2C3D3D4E4E5F5F6A6A7B7B", conversation.version_counter);
            EXPECT_THAT("", conversation.summary);

            auto it3 = conversation.participantList_.begin();
            if (it3 != conversation.participantList_.end()) {
                participant = *it3;
                EXPECT_THAT("malo@email.de", participant.uci);  ///
                EXPECT_THAT("Mari", participant.display_name);
                EXPECT_THAT("2", participant.chat_state);
                EXPECT_THAT("20140801T012900+0100", participant.last_activity);
                EXPECT_THAT("A1A2A3A4B1B2C1C2D1D2E1E2E3E4F1F2", participant.x_bt_uid);
                EXPECT_THAT("", participant.name);
                EXPECT_THAT("", participant.presence_availability);
                EXPECT_THAT("", participant.presence_text);
                EXPECT_THAT("", participant.priority);
            } else {
                EXPECT_EQ(0, 1);
            }

        } else if (conversation.id == "F1F2F3F4E1E2E3E4D1D2D3D4C1C2C3C4") {
            rootMask |= 0b100;

            EXPECT_THAT("F1F2F3F4E1E2E3E4D1D2D3D4C1C2C3C4", conversation.id);
            EXPECT_THAT("family", conversation.name);
            EXPECT_THAT("20140925T133700+0100", conversation.last_activity);
            EXPECT_THAT("yes", conversation.read_status);
            EXPECT_THAT("1A1A2B2B3C3C4D4D5E5E6F6F7A7A8B8B", conversation.version_counter);
            EXPECT_THAT("", conversation.summary);

            int mask2 = 0;
            for (auto it4 = conversation.participantList_.begin(); it4 != conversation.participantList_.end(); it4++) {
                participant = *it4;
                if (participant.uci == "malo@email.de") {
                    mask2 |= 0b01;
                    EXPECT_THAT("malo@email.de", participant.uci);
                    EXPECT_THAT("Mari", participant.display_name);
                    EXPECT_THAT("2", participant.chat_state);
                    EXPECT_THAT("20140801T012900+0100", participant.last_activity);
                    EXPECT_THAT("A1A2A3A4B1B2C1C2D1D2E1E2E3E4F1F2", participant.x_bt_uid);
                    EXPECT_THAT("Mareike Loh", participant.name);
                    EXPECT_THAT("2", participant.presence_availability);
                    EXPECT_THAT("Wow it’s hot today", participant.presence_text);
                    EXPECT_THAT("100", participant.priority);
                } else if (participant.uci == "alois.s@august.de") {
                    mask2 |= 0b10;
                    EXPECT_THAT("alois.s@august.de", participant.uci);
                    EXPECT_THAT("Lil Al", participant.display_name);
                    EXPECT_THAT("1", participant.chat_state);
                    EXPECT_THAT("20140801T012800+0100", participant.last_activity);
                    EXPECT_THAT("A1A2A3A4B1B2C1C2D1D2E1E2E3E4F1F2", participant.x_bt_uid);
                    EXPECT_THAT("Alois S.", participant.name);
                    EXPECT_THAT("0", participant.presence_availability);
                    EXPECT_THAT("On my way", participant.presence_text);
                    EXPECT_THAT("100", participant.priority);
                } else {
                }
            }
            EXPECT_EQ(0b11, mask2);
        } else {
        }
    }
    EXPECT_EQ(0b111, rootMask);
    EXPECT_EQ(0, 0);
}

const char *profileEventReport1_0 =
    "<MAP-event-report version = \"1.0\">"
    "\n<event type = \"NewMessage\" handle = \"12345678\" folder = \"TELECOM/MSG/INBOX\" msg_type = \"SMS_CDMA\" />"
    "\n</MAP-event-report>";

TEST(IProfileMapEventReportClass, profileConvoListing_v1_0)
{
    std::string testString(profileEventReport1_0);
    uint8_t isntanceId = 0;
    MceTypesEventReport tester;
    tester.BuildObjectData(isntanceId, testString);
    IProfileMapEventReport param = tester.GetParam();

    EXPECT_THAT("NewMessage", param.type);
    EXPECT_THAT("12345678", param.handle);
    EXPECT_THAT(u"TELECOM/MSG/INBOX", param.folder);
    EXPECT_THAT(u"", param.old_folder);
    EXPECT_EQ(MapMessageType::SMS_CDMA, param.msg_type);
    EXPECT_THAT("", param.datetime);
    EXPECT_THAT("", param.subject);
    EXPECT_THAT("", param.sender_name);
    EXPECT_EQ(MapBoolType::INVALID, param.priority);
    EXPECT_THAT("", param.conversation_name);
    EXPECT_THAT("", param.conversation_id);
    EXPECT_THAT("", param.presence_availability);
    EXPECT_THAT("", param.presence_text);
    EXPECT_THAT("", param.last_activity);
    EXPECT_THAT("", param.chat_state);
    EXPECT_EQ(MapMessageStatus::INVALID, param.read_status);
    EXPECT_THAT("", param.extended_data);
    EXPECT_THAT("", param.participant_uci);
    EXPECT_THAT("", param.contact_uid);
    EXPECT_THAT("1.0", param.version);
    EXPECT_EQ(0, param.masInstanceId_);
    EXPECT_THAT(testString, param.eventReportStringObject_);
    EXPECT_THAT(testString, tester.GetStringObject());
    EXPECT_EQ(0, 0);
}

const char *profileEventReport1_1 =
    "<MAP-event-report version = \"1.1\">"
    "\n<event type = \"NewMessage\" handle = \"12345678\" folder =\"TELECOM/MSG/INBOX\" msg_type = \"SMS_CDMA\" "
    "subject = \"Hello\" datetime = \"20110221T130510\" sender_name = \"Jamie\" priority = \"yes\" />"
    "\n</MAP-event-report>";

TEST(IProfileMapEventReportClass, profileEventReport1_1)
{
    std::string testString(profileEventReport1_1);
    uint8_t isntanceId = 255;
    MceTypesEventReport tester;
    tester.BuildObjectData(isntanceId, testString);
    IProfileMapEventReport param = tester.GetParam();
    EXPECT_THAT("NewMessage", param.type);
    EXPECT_THAT("12345678", param.handle);
    EXPECT_THAT(u"TELECOM/MSG/INBOX", param.folder);
    EXPECT_THAT(u"", param.old_folder);
    EXPECT_EQ(MapMessageType::SMS_CDMA, param.msg_type);
    EXPECT_THAT("20110221T130510", param.datetime);
    EXPECT_THAT("Hello", param.subject);
    EXPECT_THAT("Jamie", param.sender_name);
    EXPECT_EQ(MapBoolType::YES, param.priority);
    EXPECT_THAT("", param.conversation_name);
    EXPECT_THAT("", param.conversation_id);
    EXPECT_THAT("", param.presence_availability);
    EXPECT_THAT("", param.presence_text);
    EXPECT_THAT("", param.last_activity);
    EXPECT_THAT("", param.chat_state);
    EXPECT_EQ(MapMessageStatus::INVALID, param.read_status);
    EXPECT_THAT("", param.extended_data);
    EXPECT_THAT("", param.participant_uci);
    EXPECT_THAT("", param.contact_uid);
    EXPECT_THAT("1.1", param.version);
    EXPECT_EQ(255, param.masInstanceId_);
    EXPECT_THAT(testString, param.eventReportStringObject_);
    EXPECT_THAT(testString, tester.GetStringObject());
    EXPECT_EQ(0, 0);
}

const char *profileEventReport1_2 =
    "<MAP-event-report version = \"1.2\">"
    "\n<event type = \"ConversationChanged\" msg_type=\"IM\" sender_name=\"Mads\" conversation_name=\"friends\" "
    "conversation_id=\"A1A2A3A4B1B2B3B4C1C2C3C4D1D2D3D4\" participant_uci=\"4916579864@s.whateverapp.net\"/>"
    "\n</MAP-event-report>";

TEST(IProfileMapEventReportClass, profileEventReport1_2)
{
    std::string testString(profileEventReport1_2);
    uint8_t isntanceId = 82;
    MceTypesEventReport tester;
    tester.BuildObjectData(isntanceId, testString);
    IProfileMapEventReport param = tester.GetParam();

    EXPECT_THAT("ConversationChanged", param.type);
    EXPECT_THAT("", param.handle);
    EXPECT_THAT(u"", param.folder);
    EXPECT_THAT(u"", param.old_folder);
    EXPECT_EQ(MapMessageType::IM, param.msg_type);
    EXPECT_THAT("", param.datetime);
    EXPECT_THAT("", param.subject);
    EXPECT_THAT("Mads", param.sender_name);
    EXPECT_EQ(MapBoolType::INVALID, param.priority);
    EXPECT_THAT("friends", param.conversation_name);
    EXPECT_THAT("A1A2A3A4B1B2B3B4C1C2C3C4D1D2D3D4", param.conversation_id);
    EXPECT_THAT("", param.presence_availability);
    EXPECT_THAT("", param.presence_text);
    EXPECT_THAT("", param.last_activity);
    EXPECT_THAT("", param.chat_state);
    EXPECT_EQ(MapMessageStatus::INVALID, param.read_status);
    EXPECT_THAT("", param.extended_data);
    EXPECT_THAT("4916579864@s.whateverapp.net", param.participant_uci);
    EXPECT_THAT("", param.contact_uid);
    EXPECT_THAT("1.2", param.version);
    EXPECT_EQ(82, param.masInstanceId_);
    EXPECT_THAT(testString, param.eventReportStringObject_);
    EXPECT_THAT(testString, tester.GetStringObject());
    EXPECT_EQ(0, 0);
}

const char *eventReport1_2_alltest =
    "<MAP-event-report version = \"1.2\">"
    "\n<event type = \"ConversationChanged\" msg_type=\"IM\" sender_name=\"Mads\" conversation_name=\"friends\" "
    "handle = \"1234567890\" folder = \"MAILBOX\" old_folder = \"INBOX\" datetime = \"201008081200\" subject = \"I am "
    "a bt\" "
    "presence_availability = \"presence availability\" presence_text = \"presence text\" last_activity = \"last "
    "activity\" "
    "extended_data = \"extended data\" contact_uid = \"contact uid\" chat_state = \"chat state\" read_status = \"yes\" "
    "priority = \"no\" "
    "conversation_id=\"A1A2A3A4B1B2B3B4C1C2C3C4D1D2D3D4\" participant_uci=\"4916579864@s.whateverapp.net\"/>"
    "\n</MAP-event-report>";

TEST(IProfileMapEventReportClass, eventReport1_2_alltest)
{
    std::string testString(eventReport1_2_alltest);
    uint8_t isntanceId = 200;
    MceTypesEventReport tester;
    tester.BuildObjectData(isntanceId, testString);
    IProfileMapEventReport param = tester.GetParam();
    EXPECT_THAT("ConversationChanged", param.type);
    EXPECT_THAT("1234567890", param.handle);
    EXPECT_THAT(u"MAILBOX", param.folder);
    EXPECT_THAT(u"INBOX", param.old_folder);
    EXPECT_EQ(MapMessageType::IM, param.msg_type);
    EXPECT_THAT("201008081200", param.datetime);
    EXPECT_THAT("I am a bt", param.subject);
    EXPECT_THAT("Mads", param.sender_name);
    EXPECT_EQ(MapBoolType::NO, param.priority);
    EXPECT_THAT("friends", param.conversation_name);
    EXPECT_THAT("A1A2A3A4B1B2B3B4C1C2C3C4D1D2D3D4", param.conversation_id);
    EXPECT_THAT("presence availability", param.presence_availability);
    EXPECT_THAT("presence text", param.presence_text);
    EXPECT_THAT("last activity", param.last_activity);
    EXPECT_THAT("chat state", param.chat_state);
    EXPECT_EQ(MapMessageStatus::READ, param.read_status);
    EXPECT_THAT("extended data", param.extended_data);
    EXPECT_THAT("4916579864@s.whateverapp.net", param.participant_uci);
    EXPECT_THAT("contact uid", param.contact_uid);
    EXPECT_THAT("1.2", param.version);
    EXPECT_EQ(200, param.masInstanceId_);
    EXPECT_THAT(testString, param.eventReportStringObject_);
    EXPECT_THAT(testString, tester.GetStringObject());
    EXPECT_EQ(0, 0);
}

const char *folderListObexSample =
    "<?xml version=\"1.0\"?>"
    "\n<!DOCTYPE folder-listing SYSTEM \"obex-folder-listing.dtd\">"
    "\n<folder-listing version=\"1.0\">"
    "\n<parent-folder />"
    "\n<folder name=\"System\"/>"
    "\n<folder name=\"IR Inbox\"/>"
    "\n<file name=\"Jumar.txt\" size=\"6672\">Jumar Handling Guide</file>"
    "\n<file name =\"Obex.doc\" type = \"application/msword\">OBEX Specification v1.0</file>"
    "\n</folder-listing>";

TEST(MceTypesFolderListing, folderListObexSample)
{
    std::string testString(folderListObexSample);
    MceTypesFolderListing tester;
    tester.BuildObjectData(testString);
    std::vector<std::string> nameList = tester.GetList();
    std::string version = tester.GetVersion();
    std::string folserName;
    EXPECT_THAT("1.0", version);

    auto it = nameList.begin();
    if (nameList.size() == 2) {
        folserName = *it;
        EXPECT_THAT("System", folserName);
        it++;
        folserName = *it;
        EXPECT_THAT("IR Inbox", folserName);
    }

    EXPECT_EQ(0, 0);
}
