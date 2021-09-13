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
        sleep(1);
        while (true) {
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
            sleep(1);
        }
        return result;
    }
};
#endif