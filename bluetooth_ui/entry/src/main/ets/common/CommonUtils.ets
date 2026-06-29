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
import type { BusinessError } from '@ohos.base';

const TAG: string = 'CommonUtils: ';

export class CommonUtils {

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
