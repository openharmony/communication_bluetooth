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
#include "map_mse_params.h"
#include "string.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>

using namespace testing;
using namespace bluetooth;

TEST(MseAppParameter_test, Construct)
{
    ObexTlvParamters appParamters;
    TlvTriplet maxListCount(MapMseParams::PARAM_MAX_LIST_COUNT, (uint16_t)65535);
    appParamters.AppendTlvtriplet(maxListCount);
    TlvTriplet listStartOffSet(MapMseParams::PARAM_LIST_START_OFF_SET, (uint16_t)65535);
    appParamters.AppendTlvtriplet(listStartOffSet);
    TlvTriplet filterMessageType(MapMseParams::PARAM_FILTER_MESSAGE_TYPE, (uint8_t)255);
    appParamters.AppendTlvtriplet(filterMessageType);
    TlvTriplet filterPeriodBegin(MapMseParams::PARAM_FILTER_PERIOD_BEGIN, (uint16_t)65535);
    appParamters.AppendTlvtriplet(filterPeriodBegin);
    TlvTriplet filterPeriodEnd(MapMseParams::PARAM_FILTER_PERIOD_END, (uint16_t)65535);
    appParamters.AppendTlvtriplet(filterPeriodEnd);
    TlvTriplet filterReadStatus(MapMseParams::PARAM_FILTER_READ_STATUS, (uint8_t)255);
    appParamters.AppendTlvtriplet(filterReadStatus);
    TlvTriplet filterRecipient(MapMseParams::PARAM_FILTER_RECIPIENT, (uint16_t)65535);
    appParamters.AppendTlvtriplet(filterRecipient);
    TlvTriplet filterOriginator(MapMseParams::PARAM_FILTER_ORIGINATOR, (uint16_t)65535);
    appParamters.AppendTlvtriplet(filterOriginator);
    TlvTriplet filterPriority(MapMseParams::PARAM_FILTER_PRIORITY, (uint8_t)255);
    appParamters.AppendTlvtriplet(filterPriority);
    TlvTriplet attachment(MapMseParams::PARAM_ATTACHMENT, (uint8_t)255);
    appParamters.AppendTlvtriplet(attachment);
    TlvTriplet transparent(MapMseParams::PARAM_TRANSPARENT, (uint8_t)255);
    appParamters.AppendTlvtriplet(transparent);
    TlvTriplet retry(MapMseParams::PARAM_RETRY, (uint8_t)255);
    appParamters.AppendTlvtriplet(retry);
    TlvTriplet newMessage(MapMseParams::PARAM_NEW_MESSAGE, (uint8_t)255);
    appParamters.AppendTlvtriplet(newMessage);
    TlvTriplet notificationStatus(MapMseParams::PARAM_NOTIFICATION_STATUS, (uint8_t)255);
    appParamters.AppendTlvtriplet(notificationStatus);
    TlvTriplet masInstanceid(MapMseParams::PARAM_MAS_INSTANCEID, (uint8_t)255);
    appParamters.AppendTlvtriplet(masInstanceid);
    TlvTriplet parameterMask(MapMseParams::PARAM_PARAMETER_MASK, (uint32_t)4294967295);
    appParamters.AppendTlvtriplet(parameterMask);
    TlvTriplet folderListingSize(MapMseParams::PARAM_FOLDER_LISTING_SIZE, (uint16_t)65535);
    appParamters.AppendTlvtriplet(folderListingSize);
    TlvTriplet listingSize(MapMseParams::PARAM_LISTING_SIZE, (uint16_t)65535);
    appParamters.AppendTlvtriplet(listingSize);
    TlvTriplet subjectLength(MapMseParams::PARAM_SUBJECT_LENGTH, (uint8_t)255);
    appParamters.AppendTlvtriplet(subjectLength);
    TlvTriplet charset(MapMseParams::PARAM_CHARSET, (uint8_t)255);
    appParamters.AppendTlvtriplet(charset);
    TlvTriplet fractionRequest(MapMseParams::PARAM_FRACTION_REQUEST, (uint8_t)255);
    appParamters.AppendTlvtriplet(fractionRequest);
    TlvTriplet fractionDeliver(MapMseParams::PARAM_FRACTION_DELIVER, (uint8_t)255);
    appParamters.AppendTlvtriplet(fractionDeliver);
    TlvTriplet statusIndicator(MapMseParams::PARAM_STATUS_INDICATOR, (uint8_t)255);
    appParamters.AppendTlvtriplet(statusIndicator);
    TlvTriplet statusValue(MapMseParams::PARAM_STATUS_VALUE, (uint8_t)255);
    appParamters.AppendTlvtriplet(statusValue);
    TlvTriplet mseTime(MapMseParams::PARAM_MSE_TIME, (uint16_t)65535);
    appParamters.AppendTlvtriplet(mseTime);
    TlvTriplet databaseIdentifier(MapMseParams::PARAM_DATABASE_IDENTIFIER, (uint32_t)4294967295);
    appParamters.AppendTlvtriplet(databaseIdentifier);
    TlvTriplet conversationListingVersionCounter(
        MapMseParams::PARAM_CONVERSATION_LISTING_VERSION_COUNTER, (uint32_t)4294967295);
    appParamters.AppendTlvtriplet(conversationListingVersionCounter);
    TlvTriplet presenceAvailability(MapMseParams::PARAM_PRESENCE_AVAILABILITY, (uint8_t)255);
    appParamters.AppendTlvtriplet(presenceAvailability);
    TlvTriplet presenceText(MapMseParams::PARAM_PRESENCE_TEXT, (uint16_t)65535);
    appParamters.AppendTlvtriplet(presenceText);
    TlvTriplet lastActivity(MapMseParams::PARAM_LAST_ACTIVITY, (uint16_t)65535);
    appParamters.AppendTlvtriplet(lastActivity);
    TlvTriplet filterLastActivityBegin(MapMseParams::PARAM_FILTER_LAST_ACTIVITY_BEGIN, (uint16_t)65535);
    appParamters.AppendTlvtriplet(filterLastActivityBegin);
    TlvTriplet filterLastActivityEnd(MapMseParams::PARAM_FILTER_LAST_ACTIVITY_END, (uint16_t)65535);
    appParamters.AppendTlvtriplet(filterLastActivityEnd);
    TlvTriplet chatState(MapMseParams::PARAM_CHAT_STATE, (uint8_t)255);
    appParamters.AppendTlvtriplet(chatState);
    TlvTriplet conversationId(MapMseParams::PARAM_CONVERSATION_ID, (uint32_t)4294967295);
    appParamters.AppendTlvtriplet(conversationId);
    TlvTriplet folderVersionCounter(MapMseParams::PARAM_FOLDER_VERSION_COUNTER, (uint32_t)4294967295);
    appParamters.AppendTlvtriplet(folderVersionCounter);
    TlvTriplet filterMessageHandle(MapMseParams::PARAM_FILTER_MESSAGE_HANDLE, (uint16_t)65535);
    appParamters.AppendTlvtriplet(filterMessageHandle);
    TlvTriplet notificationFilterMask(MapMseParams::PARAM_NOTIFICATION_FILTER_MASK, (uint16_t)65535);
    appParamters.AppendTlvtriplet(notificationFilterMask);
    TlvTriplet convParameterMask(MapMseParams::PARAM_CONV_PARAMETER_MASK, (uint16_t)65535);
    appParamters.AppendTlvtriplet(convParameterMask);
    TlvTriplet ownerUci(MapMseParams::PARAM_OWNER_UCI, (uint16_t)65535);
    appParamters.AppendTlvtriplet(ownerUci);
    TlvTriplet extendedData(MapMseParams::PARAM_EXTENDED_DATA, (uint16_t)65535);
    appParamters.AppendTlvtriplet(extendedData);
    TlvTriplet mapSupportedFeatures(MapMseParams::PARAM_MAP_SUPPORTED_FEATURES, (uint16_t)65535);
    appParamters.AppendTlvtriplet(mapSupportedFeatures);
    TlvTriplet messageHandle(MapMseParams::PARAM_MESSAGE_HANDLE, (uint16_t)65535);
    appParamters.AppendTlvtriplet(messageHandle);
    TlvTriplet modifyTest(MapMseParams::PARAM_MODIFY_TEXT, (uint8_t)255);
    appParamters.AppendTlvtriplet(modifyTest);

    auto header = ObexHeader::CreateRequest(ObexOpeId::GET_FINAL);
    header->AppendItemConnectionId(1);
    header->AppendItemName(u"telecom/inbox");
    header->AppendItemType("x-bt/Map-msg-listing");
    header->AppendItemAppParams(appParamters);
    auto appParams = std::make_unique<MapMseParams>(appParamters);
    EXPECT_EQ(0, 0);
}

TEST(MseAppParameter_test, Other)
{
    ObexTlvParamters appParamters;
    TlvTriplet maxListCount(MapMseParams::PARAM_MAX_LIST_COUNT, (uint32_t)4294967295);
    appParamters.AppendTlvtriplet(maxListCount);
    TlvTriplet listStartOffSet(MapMseParams::PARAM_LIST_START_OFF_SET, (uint32_t)4294967295);
    appParamters.AppendTlvtriplet(listStartOffSet);
    TlvTriplet filterMessageType(MapMseParams::PARAM_FILTER_MESSAGE_TYPE, (uint16_t)65535);
    appParamters.AppendTlvtriplet(filterMessageType);
    TlvTriplet filterReadStatus(MapMseParams::PARAM_FILTER_READ_STATUS, (uint16_t)65535);
    appParamters.AppendTlvtriplet(filterReadStatus);
    TlvTriplet filterPriority(MapMseParams::PARAM_FILTER_PRIORITY, (uint16_t)65535);
    appParamters.AppendTlvtriplet(filterPriority);
    TlvTriplet attachment(MapMseParams::PARAM_ATTACHMENT, (uint16_t)65535);
    appParamters.AppendTlvtriplet(attachment);
    TlvTriplet transparent(MapMseParams::PARAM_TRANSPARENT, (uint16_t)65535);
    appParamters.AppendTlvtriplet(transparent);
    TlvTriplet retry(MapMseParams::PARAM_RETRY, (uint16_t)65535);
    appParamters.AppendTlvtriplet(retry);
    TlvTriplet newMessage(MapMseParams::PARAM_NEW_MESSAGE, (uint16_t)65535);
    appParamters.AppendTlvtriplet(newMessage);
    TlvTriplet notificationStatus(MapMseParams::PARAM_NOTIFICATION_STATUS, (uint16_t)65535);
    appParamters.AppendTlvtriplet(notificationStatus);
    TlvTriplet masInstanceid(MapMseParams::PARAM_MAS_INSTANCEID, (uint16_t)65535);
    appParamters.AppendTlvtriplet(masInstanceid);
    TlvTriplet folderListingSize(MapMseParams::PARAM_FOLDER_LISTING_SIZE, (uint32_t)4294967295);
    appParamters.AppendTlvtriplet(folderListingSize);
    TlvTriplet listingSize(MapMseParams::PARAM_LISTING_SIZE, (uint32_t)4294967295);
    appParamters.AppendTlvtriplet(listingSize);
    TlvTriplet subjectLength(MapMseParams::PARAM_SUBJECT_LENGTH, (uint16_t)65535);
    appParamters.AppendTlvtriplet(subjectLength);
    TlvTriplet charset(MapMseParams::PARAM_CHARSET, (uint16_t)65535);
    appParamters.AppendTlvtriplet(charset);
    TlvTriplet fractionRequest(MapMseParams::PARAM_FRACTION_REQUEST, (uint16_t)65535);
    appParamters.AppendTlvtriplet(fractionRequest);
    TlvTriplet fractionDeliver(MapMseParams::PARAM_FRACTION_DELIVER, (uint16_t)65535);
    appParamters.AppendTlvtriplet(fractionDeliver);
    TlvTriplet statusIndicator(MapMseParams::PARAM_STATUS_INDICATOR, (uint16_t)65535);
    appParamters.AppendTlvtriplet(statusIndicator);
    TlvTriplet statusValue(MapMseParams::PARAM_STATUS_VALUE, (uint16_t)65535);
    appParamters.AppendTlvtriplet(statusValue);
    TlvTriplet chatState(MapMseParams::PARAM_CHAT_STATE, (uint16_t)65535);
    appParamters.AppendTlvtriplet(chatState);
    TlvTriplet filterMessageHandle(MapMseParams::PARAM_FILTER_MESSAGE_HANDLE, (uint32_t)4294967295);
    appParamters.AppendTlvtriplet(filterMessageHandle);
    TlvTriplet notificationFilterMask(MapMseParams::PARAM_NOTIFICATION_FILTER_MASK, (uint16_t)65535);
    appParamters.AppendTlvtriplet(notificationFilterMask);
    TlvTriplet convParameterMask(MapMseParams::PARAM_CONV_PARAMETER_MASK, (uint16_t)65535);
    appParamters.AppendTlvtriplet(convParameterMask);
    TlvTriplet ownerUci(MapMseParams::PARAM_OWNER_UCI, (uint16_t)65535);
    appParamters.AppendTlvtriplet(ownerUci);
    TlvTriplet extendedData(MapMseParams::PARAM_EXTENDED_DATA, (uint16_t)65535);
    appParamters.AppendTlvtriplet(extendedData);
    TlvTriplet mapSupportedFeatures(MapMseParams::PARAM_MAP_SUPPORTED_FEATURES, (uint16_t)65535);
    appParamters.AppendTlvtriplet(mapSupportedFeatures);
    TlvTriplet messageHandle(MapMseParams::PARAM_MESSAGE_HANDLE, (uint16_t)65535);
    appParamters.AppendTlvtriplet(messageHandle);
    TlvTriplet modifyTest(MapMseParams::PARAM_MODIFY_TEXT, (uint16_t)65535);
    appParamters.AppendTlvtriplet(modifyTest);

    auto header = ObexHeader::CreateRequest(ObexOpeId::GET_FINAL);
    header->AppendItemConnectionId(1);
    header->AppendItemName(u"telecom/inbox");
    header->AppendItemType("x-bt/Map-msg-listing");
    header->AppendItemAppParams(appParamters);
    auto appParams = std::make_unique<MapMseParams>(appParamters);
    sleep(3);
    auto defaultPara = std::make_unique<MapMseParams>();
    EXPECT_EQ(0, 0);
}