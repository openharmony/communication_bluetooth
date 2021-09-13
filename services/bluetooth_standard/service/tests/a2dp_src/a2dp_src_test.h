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

#ifndef AVDT_TEST_H
#define AVDT_TEST_H

#include "common_mock_all.h"
#include "stub/a2dp_data_service.h"
#include "gavdp/a2dp_avdtp.h"
#include "gavdp/a2dp_def.h"
#include "gavdp/a2dp_state_machine.h"
#include "a2dp_src_service.h"
#include "a2dp_snk_service.h"
#include "adapter_config.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include "gavdp/a2dp_profile.h"
#include "gavdp/a2dp_source.h"
#include "gavdp/a2dp_codec_thread.h"
#include "gavdp/a2dp_codec/include/a2dp_aac_param_ctrl.h"
#include "a2dp_mock.h"
#include "log.h"

#define A2DP_SRC_TEST_TIMER_NORMAL (50000)  // us
#define A2DP_TEST_SIGNALLING_TIMEOUT (9)    // s

#endif