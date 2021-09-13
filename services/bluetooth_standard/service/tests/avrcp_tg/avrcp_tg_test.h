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

#ifndef AVRCP_TG_TEST_H
#define AVRCP_TG_TEST_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "common_mock_all.h"
#include "stub/media_service.h"
#include "avrcp_tg_gap.h"
#include "avrcp_tg_sdp.h"
#include "avrcp_tg_state_machine.h"
#include "avrcp_tg_internal.h"
#include "avrcp_tg_pass_through.h"
#include "avrcp_tg_connection.h"
#include "avrcp_tg_sub_unit_info.h"
#include "avrcp_tg_vendor.h"
#include "avrcp_tg_profile.h"
#include "avrcp_tg_service.h"
#include "avrcp_ct_pass_through.h"
#include "avrcp_ct_unit_info.h"
#include "avrcp_ct_sub_unit_info.h"
#include "avrcp_ct_vendor.h"
#include "avrcp_ct_browse.h"
#include "avrcp_ct_notification.h"
#include "avrcp_ct_vendor_player_application_settings.h"
#include "avrcp_ct_vendor_continuation.h"
#include "interface_profile_avrcp_tg.h"

extern "C" {
#include "avctp.h"
#include "log.h"
#include "btstack.h"
#include "message.h"
#include <assert.h>
#include "sdp.h"
#include "gap_if.h"
}

#define AVRC_TG_TEST_TIMER_NORMAL (50000)  // us
#define AVRC_TG_TEST_TIMER_LONG (500000)   // us
#define AVRC_TG_TEST_TIMEOUT (3000000)     // us

#endif /*AVRCP_TG_TEST_H*/
