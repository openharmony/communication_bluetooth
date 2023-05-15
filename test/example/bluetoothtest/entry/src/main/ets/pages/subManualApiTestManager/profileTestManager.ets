/**
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

// @ts-nocheck

import router from '@system.router';
import { TestData } from '../../MainAbility/model/testData'
import { initProfileManagerApiData } from '../../MainAbility/model/testDataModelsManager'
import { PageTitle } from '../../Component/pageTitle';
import { TestImageDisplay } from '../../Component/testImageDisplay';
import { ContentTable } from '../../Component/contentTable';
import btmProfileModel from '../../MainAbility/model/btmProfileModel';

/**
 * Profile Test Page Of Bluetooth test
 */

@Entry
@Component
struct ProfileTestManager {
  private deviceId = router.getParams().deviceId;
  private testItem: TestData = router.getParams().testId;

  aboutToAppear() {
    btmProfileModel.initProfile()
  }

  build() {
    Column() {
      Stack( { alignContent : Alignment.TopStart } ) {
        TestImageDisplay( { testItem : this.testItem } )
        PageTitle( { testItem : this.testItem } )
      }

      ContentTable( { testItem : this.testItem , apiItems : initProfileManagerApiData() } )
    }
    .backgroundColor( $r( "sys.color.ohos_id_color_sub_background" ) )
    .height( '100%' )
    .alignItems( VerticalAlign.Top )
  }
}