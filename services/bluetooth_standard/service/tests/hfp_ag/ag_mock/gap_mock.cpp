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

#include "ag_mock.h"

using namespace bluetooth;
AgGapMock *gGapMocker = nullptr;

void registerGapMocker(AgGapMock *mocker)
{
    gGapMocker = mocker;
}

AgGapMock& GetGapMocker()
{
    return *gGapMocker;
}

int GAPIF_RegisterServiceSecurity(const BtAddr *addr, const GapServiceSecurityInfo *serviceInfo, uint16_t securityMode)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gGapMocker->GAPIF_RegisterServiceSecurity();
}

int GAPIF_DeregisterServiceSecurity(const BtAddr *addr, const GapServiceSecurityInfo *serviceInfo)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gGapMocker->GAPIF_DeregisterServiceSecurity();
}

int GAP_RequestSecurity(const BtAddr *addr, const GapRequestSecurityParam *param)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gGapMocker->GAP_RequestSecurity();
}