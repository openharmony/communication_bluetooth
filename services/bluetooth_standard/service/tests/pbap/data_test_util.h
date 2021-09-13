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

#ifndef DATA_TEST_UTIL_H
#define DATA_TEST_UTIL_H
#include <codecvt>
#include <locale>
#include <ctime>
#include <exception>
#include <functional>
#include <iostream>
#include <unistd.h>
class DataTestUtil {
public:
    static bool WaitFor(const std::function<bool()> &condition, int maxWaitSecs = 10)
    {
        bool result = false;
        time_t startTime = time(0);
        while (true) {
            sleep(1);
            if (condition()) {
                result = true;
                break;
            }
            time_t nowTime = time(0);
            int costTime = nowTime - startTime;
            if (costTime > maxWaitSecs) {
                std::cout << "wait time out " << maxWaitSecs << "" << std::endl;
                break;
            }
        }
        return result;
    }
};

#define WAIT_FOR(assert_exp, close_server_lambda)                 \
    if (!DataTestUtil::WaitFor([&]() { return (assert_exp); })) { \
        close_server_lambda();                                    \
        GTEST_FATAL_FAILURE_("time out");                         \
        return;                                                   \
    }

#ifndef DEBUG_PRINT
#define DEBUG_PRINT(fmt, ...)                                           \
    std::cout << std::endl;                                             \
    printf("%s(%d):[%s]" fmt,                                           \
        strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__, \
        __LINE__,                                                       \
        __func__,                                                       \
        ##__VA_ARGS__);                                                 \
    std::cout << std::endl;
#endif  // DEBUG_PRINT

#ifndef U16_2_STR
#define U16_2_STR(u16str) std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(u16str)
#endif  // U16_2_STR

#ifndef STR_2_U16
#define STR_2_U16(str) std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(str)
#endif  // STR_2_U16

#endif  // DATA_TEST_UTIL_H