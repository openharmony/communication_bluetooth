/**
 * Copyright (c) 2022 Shenzhen Kaihong Digital Industry Development Co., Ltd.
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

/**
 * Gatt Page Of Bluetooth test
 */
import Router from '@system.router';
import prompt from '@system.prompt';

const PAGE_URI_GET_GATT_CLIENT_MANAGER = 'pages/subManualApiTestManager/subGattTestManager/gattClientManager';
const PAGE_URI_GET_GATT_SERVER_MANAGER = 'pages/subManualApiTestManager/subGattTestManager/gattServerManager';

export function testGattClientManager(): String {
  try {
    Router.push( { uri : PAGE_URI_GET_GATT_CLIENT_MANAGER } )
    let message = "跳转成功"
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testGattClientManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testGattClientManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}

export function testGattServerManager(): String {
  try {
    Router.push( { uri : PAGE_URI_GET_GATT_SERVER_MANAGER } )
    let message = "跳转成功"
    prompt.showToast( { message : message } )
    return message
  } catch( e ) {
    console.log( `Failed to testGattServerManager. message: ${ e.message }, code: ${ e.code }` )
    let message = `Failed to testGattServerManager. message: ${ e.message }, code: ${ e.code }`
    prompt.showToast( { message : message } )
    return message
  }
}


