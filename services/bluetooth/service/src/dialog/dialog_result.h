/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef DIALOG_RESULT_H
#define DIALOG_RESULT_H

#include <thread>
#include <mutex>
#include <condition_variable>

namespace OHOS {
namespace bluetooth {
class DialogResult {
public:
    DialogResult();
    ~DialogResult();

    /**
     * @brief Handle bluetooth switch dialog result.
     *
     * @param isAsync Indicate Async function flag.
     * @return Returns <b>BT_ERR_DIALOG_FOR_USER_CONFIRM</b> if sync func or the dialog result is accepted;
     *         returns <b>BT_ERR_DIALOG_FOR_USER_NOT_RESPOND</b> if the user does not respond;
               returns <b>BT_ERR_DIALOG_FOR_USER_REFUSE</b> if the user refuse the action.
     * @since 20
     */
    int32_t HandleBluetoothSwitchDialogResult(bool isAsync);

    /**
     * @brief Notify bluetooth switch dialog result.
     *
     * @param dialogType The type of bluetooth dialog.
     * @param dialogResult The result of bluetooth dailog.
     * @since 20
     */
    void NotifyBluetoothSwitchDialogResult(uint32_t dialogType, bool dialogResult);

    /**
     * @brief Wait for bluetooth switch dialog result.
     *
     * @since 20
     */
    void WaitAdapterManagerDialogResultComplete(void);

    bool GetIsDialogResultReceived(void);
    void IsDialogResultReceived(bool isDialogResult);
    bool GetDialogResult(void);
    void SetDialogResult(bool dialogResult);
    
private:
    std::mutex getDialogResultMutex_ {};
    std::condition_variable getDialogResultConditionVariable_ {};
    bool isDialogResult_ = false;
    bool dialogResult_ = false;
    static constexpr int32_t DIALOG_RESULT_DELAY_MS = 10000;
};
} // namespace bluetooth
} // namespace OHOS
#endif // DIALOG_RESULT_GET_H