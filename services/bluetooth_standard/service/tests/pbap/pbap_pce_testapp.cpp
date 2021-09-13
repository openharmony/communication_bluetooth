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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include "adapter_manager.h"
#include "pbap_pce_def.h"
#include "pbap_pce_service.h"

using namespace bluetooth;

int TestAppMainClient(int argc, char *argv[])
{
    std::cout << "TestAppMainClient" << std::endl;
    PbapPceService pbapServcie;
    pbapServcie.Enable();

    std::string s("00:00:01:00:00:7F");
    RawAddress rawAddr(s);

    std::cout << "Connect" << std::endl;
    pbapServcie.Connect(rawAddr);
    getchar();

    IPbapPullPhoneBookParam phoneBookSizeParam;
    phoneBookSizeParam.SetName(u"telecom/pb.vcf");
    phoneBookSizeParam.SetPropertySelector(0x124);
    phoneBookSizeParam.SetFormat(0x01);
    phoneBookSizeParam.SetMaxListCount(0);
    phoneBookSizeParam.SetListStartOffset(0);
    phoneBookSizeParam.SetResetNewMissedCalls(0);
    phoneBookSizeParam.SetvCardSelector(0x120);
    phoneBookSizeParam.SetvCardSelectorOp(0x01);
    std::cout << "PhoneBookSize" << std::endl;
    pbapServcie.PullPhoneBook(rawAddr, phoneBookSizeParam);
    getchar();

    IPbapPullPhoneBookParam pullPhoneBookParam;
    pullPhoneBookParam.SetName(u"telecom/pb.vcf");
    pullPhoneBookParam.SetPropertySelector(0x124);
    pullPhoneBookParam.SetFormat(0x01);
    pullPhoneBookParam.SetMaxListCount(3);
    pullPhoneBookParam.SetListStartOffset(0);
    pullPhoneBookParam.SetResetNewMissedCalls(0);
    pullPhoneBookParam.SetvCardSelector(0x120);
    pullPhoneBookParam.SetvCardSelectorOp(0x01);
    std::cout << "PullPhoneBook" << std::endl;
    pbapServcie.PullPhoneBook(rawAddr, pullPhoneBookParam);
    getchar();

    std::cout << "SetPhoneBook" << std::endl;
    pbapServcie.SetPhoneBook(rawAddr, u"telecom/pb", 0);
    getchar();

    std::cout << "PullvCardListing" << std::endl;
    IPbapPullvCardListingParam pullvCardListingParam;
    pullvCardListingParam.SetName(u"");
    pullvCardListingParam.SetOrder(0x01);
    pullvCardListingParam.SetSearchValue("34");
    pullvCardListingParam.SetSearchProperty(0x01);
    pullvCardListingParam.SetMaxListCount(20);
    pullvCardListingParam.SetListStartOffset(0x0);
    pullvCardListingParam.SetResetNewMissedCalls(0x0);
    pullvCardListingParam.SetvCardSelector(0x0);
    pullvCardListingParam.SetvCardSelectorOp(0x0);
    pbapServcie.PullvCardListing(rawAddr, pullvCardListingParam);
    getchar();

    IPbapPullvCardEntryParam pullvCardEntryParam;
    pullvCardEntryParam.SetName(u"X-BT-UID:A1A2A3A4B1B2C1C2D1D2E1E2E3E4E5E6");
    pullvCardEntryParam.SetPropertySelector(0);
    pullvCardEntryParam.SetFormat(0);
    std::cout << "PullvCardEntry" << std::endl;
    pbapServcie.PullvCardEntry(rawAddr, pullvCardEntryParam);
    getchar();

    std::cout << "Disconnect" << std::endl;
    pbapServcie.Disconnect(rawAddr);
    getchar();

    std::cout << "Reconnect" << std::endl;
    pbapServcie.Connect(rawAddr);
    getchar();
    return 0;
}