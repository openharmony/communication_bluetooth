/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import resourceManager from '@ohos.resourceManager';
import commonEventManager from '@ohos.commonEventManager';
import type { BusinessError } from '@ohos.base';

const TAG: string = 'CommonUtils: ';
const BT_TRANSACTION_EVENT: string = 'usual.event.bluetooth.REPORT.TRANSACTION';

const RESULT_SUCCESS: number = 0;
const RESULT_ERROR_UNSUPPORTED_TYPE: number = 1;
const RESULT_ERROR_BAD_REQUEST: number = 2;
const RESULT_ERROR_NOT_ACCEPTABLE: number = 3;
const RESULT_ERROR_CANCELED: number = 4;
const RESULT_ERROR_CONNECTION_FAILED: number = 5;
const RESULT_ERROR_TRANSFER_FAILED: number = 6;

const TRANSACTION_RESULT_TOTAL: number = 1;
const TRANSACTION_RESULT_SUCCESS: number = 2;
const TRANSACTION_RESULT_FAIL: number = 3;

const SCENECODE_NA: number = 0;

export const btTransactionStatisticsType = {
    TRANSACTION_TYPE_OPP_SEND: 1,
    TRANSACTION_TYPE_OPP_RECEIVE: 2,
    TRANSACTION_TYPE_OPP_RECEIVE_MEDIA_TYPE: 7,
};

export const btTransactionStatisticsResult = {
    TRANSACTION_RESULT_TOTAL: 1,
    TRANSACTION_RESULT_SUCCESS: 2,
    TRANSACTION_RESULT_FAIL: 3,
};

export const btTransactionStatisticsSceneCode = {
    TRANSACTION_SCENECODE_NA: 0,
    TRANSACTION_SCENECODE_1: 1,
    TRANSACTION_SCENECODE_2: 2,
    TRANSACTION_SCENECODE_3: 3,
    TRANSACTION_SCENECODE_4: 4,
    TRANSACTION_SCENECODE_5: 5,
    TRANSACTION_SCENECODE_6: 6,
    TRANSACTION_SCENECODE_7: 7,
    TRANSACTION_SCENECODE_8: 8,
    TRANSACTION_SCENECODE_9: 9,
    TRANSACTION_SCENECODE_10: 10,
    TRANSACTION_SCENECODE_11: 11,
    TRANSACTION_SCENECODE_12: 12,
    TRANSACTION_SCENECODE_13: 13,
    TRANSACTION_SCENECODE_14: 14,
    TRANSACTION_SCENECODE_15: 15,
    TRANSACTION_SCENECODE_16: 16,
};

export class CommonUtils {

    /**
    * 上报蓝牙事务统计
    *
    * @param transactionType 事务类型
    * @param result 结果类型
    * @param resultCount 结果数量
    * @param sceneCode 场景码
    * @param sceneCodeCount 场景码数量
    */
    reportBtTransactionChr(transactionType: number, result: number, resultCount: number,
        sceneCode: number, sceneCodeCount: number) {
        const options: commonEventManager.CommonEventPublishData = {
            code: 0,
            data: 'message',
            subscriberPermissions: ['ohos.permission.MANAGE_BLUETOOTH'],
            isOrdered: true,
            isSticky: false,
            parameters: { 'transactionType': transactionType, 'result': result,
                'resultCount': resultCount, 'sceneCode': sceneCode, 'sceneCodeCount': sceneCodeCount}
        }
        commonEventManager.publish(BT_TRANSACTION_EVENT, options, (err) => {
            if (err) {
                console.info(TAG, 'get bt transaction event publish failed.' + JSON.stringify(err));
            } else {
                console.info(TAG, 'get bt transaction event publish success.');
            }
        })
    }

    /**
    * 根据发送传输结果上报蓝牙事务统计数据
    *
    * @param dataResult 传输结果
    * @param totalCount 总数量
    * @param successCount 成功数量
    */
    reportBtSendChrDataByResult(dataResult: number, totalCount: number, successCount: number) {
        let transactionType = btTransactionStatisticsType.TRANSACTION_TYPE_OPP_SEND;
        if (totalCount - successCount > totalCount) {
            return;
        }
        if (dataResult == RESULT_SUCCESS) {
            this.reportBtTransactionChr(transactionType, TRANSACTION_RESULT_SUCCESS,
                1, SCENECODE_NA, 0);
        } else if ((dataResult == RESULT_ERROR_UNSUPPORTED_TYPE ||
            dataResult == RESULT_ERROR_BAD_REQUEST) && totalCount > 0) {
            this.reportBtTransactionChr(transactionType, TRANSACTION_RESULT_FAIL,
                totalCount, dataResult, totalCount);
            this.reportBtTransactionChr(transactionType, TRANSACTION_RESULT_TOTAL,
                totalCount - 1, SCENECODE_NA, 0);
        } else if (dataResult == RESULT_ERROR_CANCELED ||
            dataResult == RESULT_ERROR_TRANSFER_FAILED) {
            this.reportBtTransactionChr(transactionType, TRANSACTION_RESULT_FAIL, 1, dataResult, 1);
        } else {
            this.reportBtTransactionChr(transactionType, TRANSACTION_RESULT_FAIL, 1, dataResult, 1);
        }
        this.reportBtTransactionChr(transactionType, TRANSACTION_RESULT_TOTAL,
            1, SCENECODE_NA, 0);
    }

    /**
    * 根据接收传输结果上报蓝牙事务统计数据
    *
    * @param dataResult 传输结果
    * @param totalCount 总数量
    * @param successCount 成功数量
    */
    reportBtReceiveChrDataByResult(dataResult: number, totalCount: number, successCount: number) {
        let transactionType = btTransactionStatisticsType.TRANSACTION_TYPE_OPP_RECEIVE;
        if (dataResult == RESULT_SUCCESS) {
            this.reportBtTransactionChr(transactionType, TRANSACTION_RESULT_SUCCESS,
                1, SCENECODE_NA, 0);
        } else if ((dataResult == RESULT_ERROR_NOT_ACCEPTABLE ||
            dataResult == RESULT_ERROR_BAD_REQUEST) && totalCount > 0) {
            this.reportBtTransactionChr(transactionType, TRANSACTION_RESULT_FAIL,
                totalCount, dataResult, totalCount);
            this.reportBtTransactionChr(transactionType, TRANSACTION_RESULT_TOTAL,
                totalCount - 1, SCENECODE_NA, 0);
        } else if ((dataResult == RESULT_ERROR_CANCELED ||
            dataResult == RESULT_ERROR_TRANSFER_FAILED) &&
            totalCount - successCount >= 1) {
            this.reportBtTransactionChr(transactionType, TRANSACTION_RESULT_FAIL,
                totalCount - successCount, dataResult, totalCount - successCount);
            this.reportBtTransactionChr(transactionType, TRANSACTION_RESULT_TOTAL,
                totalCount - successCount - 1, SCENECODE_NA, 0);
        } else if (dataResult == RESULT_ERROR_CONNECTION_FAILED) {
            this.reportBtTransactionChr(transactionType, TRANSACTION_RESULT_FAIL, 1, dataResult, 1);
        }
        this.reportBtTransactionChr(transactionType, TRANSACTION_RESULT_TOTAL,
            1, SCENECODE_NA, 0);
    }

    /**
    * 以同步方式获取string资源值
    *
    * @param resource Resource资源类
    */
    ResourceManagerGetStringSync(context: Context, resource: Resource | undefined): string {
        let resourceString: string = '';
        if (!resource) {
            return resourceString;
        }
        if (typeof resource === 'string') {
            return resource;
        }
        let resourceManager = context.resourceManager;
        if (!resourceManager) {
            console.error(TAG, `resourceManager is null`);
            return resourceString;
        }
        try {
            resourceString = resourceManager.getStringSync(resource.id);
        } catch (error) {
            console.error(TAG, `getStringSync failed, cause: ${(error as BusinessError).message}`);
        }
        return resourceString;
    }
}

let mCommonUtils = new CommonUtils();

export default mCommonUtils as CommonUtils;
