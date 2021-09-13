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

#ifndef AVRCP_CT_TEST_H
#define AVRCP_CT_TEST_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "common_mock_all.h"
#include "avrcp_ct_state_machine.h"
#include "avrcp_ct_pass_through.h"
#include "avrcp_ct_sdp.h"
#include "avrcp_ct_gap.h"
#include "avrcp_ct_profile.h"
#include "avrcp_ct_service.h"
#include "avrcp_ct_connection.h"
#include "avrcp_ct_internal.h"
#include "avrcp_ct_vendor.h"
#include "avrcp_ct_notification.h"
#include "interface_profile_avrcp_ct.h"
#include "avrcp_tg_browse.h"
#include "avrcp_tg_vendor.h"
#include "avrcp_tg_pass_through.h"
#include "avrcp_tg_unit_info.h"
#include "avrcp_tg_sub_unit_info.h"
#include "avrcp_tg_notification.h"
#include "avrcp_tg_vendor_continuation.h"
#include "avrcp_tg_vendor_player_application_settings.h"

extern "C" {
#include <memory.h>
#include "avctp.h"
#include "log.h"
#include "btstack.h"
#include "message.h"
#include <assert.h>
#include "sdp.h"
#include "gap_if.h"
}

#define AVRC_CT_TEST_TIMER_NORMAL (50000)  // us
#define AVRC_CT_TEST_TIMER_LONG (500000)   // us
#define AVRC_CT_TEST_TIMEOUT (3000000)     // us
#define AVRC_CT_TEST_SIZE_OF_QUEUE (AVRC_CT_DEFAULT_SIZE_OF_QUEUE - 18)
#define AVRC_CT_TEST_FRAGMENTS (AVRC_CT_DEFAULT_MAX_FRAGMENTS + 10)

#endif /* AVRCP_CT_TEST_H */