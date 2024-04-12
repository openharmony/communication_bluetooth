/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_SERVICE_IPC_INTERFACE_CODE_H
#define BLUETOOTH_SERVICE_IPC_INTERFACE_CODE_H

#include "bluetooth_service_profile_interface_code.h"
#include "ipc_types.h"

/* SAID: 1130 */
namespace OHOS {
namespace Bluetooth {
enum BluetoothBleAdvertiseCallbackInterfaceCode {
    BT_BLE_ADVERTISE_CALLBACK_AUTO_STOP_EVENT = 0,
    BT_BLE_ADVERTISE_CALLBACK_START_RESULT_EVENT,
    BT_BLE_ADVERTISE_CALLBACK_ENABLE_RESULT_EVENT,
    BT_BLE_ADVERTISE_CALLBACK_DISABLE_RESULT_EVENT,
    BT_BLE_ADVERTISE_CALLBACK_STOP_RESULT_EVENT,
    BT_BLE_ADVERTISE_CALLBACK_SET_ADV_DATA,
    // The last code, if you want to add a new code, please add it before this
    BT_BLE_ADVERTISE_CALLBACK_BUTT
};

enum BluetoothBleAdvertiserInterfaceCode {
    BLE_REGISTER_BLE_ADVERTISER_CALLBACK = 0,
    BLE_DE_REGISTER_BLE_ADVERTISER_CALLBACK,
    BLE_START_ADVERTISING,
    BLE_START_ADVERTISING_WITH_RAWDATA,
    BLE_STOP_ADVERTISING,
    BLE_CLOSE,
    BLE_GET_ADVERTISER_HANDLE,
    BLE_SET_ADVERTISING_DATA,
    BLE_ENABLE_ADVERTISING,
    BLE_DISABLE_ADVERTISING,
    // The last code, if you want to add a new code, please add it before this
    BLE_ADVERTISER_BUTT
};

enum BluetoothBleCentralManagerCallbackInterfaceCode {
    BT_BLE_CENTRAL_MANAGER_CALLBACK = 0,
    BT_BLE_CENTRAL_MANAGER_BLE_BATCH_CALLBACK,
    BT_BLE_CENTRAL_MANAGER_CALLBACK_SCAN_FAILED,
    BT_BLE_LPDEVICE_CALLBACK_NOTIFY_MSG_REPORT,
    // The last code, if you want to add a new code, please add it before this
    BT_BLE_CENTRAL_MANAGER_CALLBACK_BUTT
};

enum BluetoothBleCentralManagerInterfaceCode {
    BLE_REGISTER_BLE_CENTRAL_MANAGER_CALLBACK = 0,
    BLE_DE_REGISTER_BLE_CENTRAL_MANAGER_CALLBACK,
    BLE_START_SCAN,
    BLE_START_SCAN_WITH_SETTINGS, // Deprecated
    BLE_STOP_SCAN,
    BLE_CONFIG_SCAN_FILTER, // Deprecated
    BLE_REMOVE_SCAN_FILTER,
    BLE_FREEZE_BY_RSS,
    BLE_RESET_ALL_PROXY,
    BLE_SET_LPDEVICE_ADV_PARAM,
    BLE_SET_SCAN_REPORT_CHANNEL_TO_LPDEVICE,
    BLE_ENABLE_SYNC_DATA_TO_LPDEVICE,
    BLE_DISABLE_SYNC_DATA_TO_LPDEVICE,
    BLE_SEND_PARAMS_TO_LPDEVICE,
    BLE_IS_LPDEVICE_AVAILABLE,
    BLE_SET_LPDEVICE_PARAM,
    BLE_REMOVE_LPDEVICE_PARAM,
    // The last code, if you want to add a new code, please add it before this
    BLE_CENTRAL_MANAGER_BUTT
};

enum BluetoothBlePeripheralObserverInterfaceCode {
    BLE_ON_READ_REMOTE_RSSI_EVENT = 0,
    BLE_PAIR_STATUS_CHANGED,
    BLE_ACL_STATE_CHANGED,
    // The last code, if you want to add a new code, please add it before this
    BLE_PERIPHERAL_OBSERVER_BUTT
};

enum BluetoothGattClientCallbackInterfaceCode {
    BT_GATT_CLIENT_CALLBACK_CONNECT_STATE_CHANGE = 0,
    BT_GATT_CLIENT_CALLBACK_CHARACTER_CHANGE,
    BT_GATT_CLIENT_CALLBACK_CHARACTER_READ,
    BT_GATT_CLIENT_CALLBACK_CHARACTER_WRITE,
    BT_GATT_CLIENT_CALLBACK_DESCRIPTOR_READ,
    BT_GATT_CLIENT_CALLBACK_DESCRIPTOR_WRITE,
    BT_GATT_CLIENT_CALLBACK_MTU_UPDATE,
    BT_GATT_CLIENT_CALLBACK_SERVICES_DISCOVER,
    BT_GATT_CLIENT_CALLBACK_CONNECTION_PARA_CHANGE,
    BT_GATT_CLIENT_CALLBACK_SERVICES_CHANGED,
    BT_GATT_CLIENT_CALLBACK_READ_REMOTE_RSSI_VALUE,
    // The last code, if you want to add a new code, please add it before this
    BT_GATT_CLIENT_CALLBACK_BUTT
};

enum BluetoothGattClientInterfaceCode {
    BT_GATT_CLIENT_REGISTER_APP = 0,
    BT_GATT_CLIENT_DEREGISTER_APP,
    BT_GATT_CLIENT_CONNECT,
    BT_GATT_CLIENT_DIS_CONNECT,
    BT_GATT_CLIENT_DISCOVERY_SERVICES,
    BT_GATT_CLIENT_READ_CHARACTERISTIC,
    BT_GATT_CLIENT_WRITE_CHARACTERISTIC,
    BT_GATT_CLIENT_SIGNED_WRITE_CHARACTERISTIC,
    BT_GATT_CLIENT_READ_DESCRIPTOR,
    BT_GATT_CLIENT_WRITE_DESCRIPTOR,
    BT_GATT_CLIENT_REQUEST_EXCHANGE_MTU,
    BT_GATT_CLIENT_GET_ALL_DEVICE,
    BT_GATT_CLIENT_REQUEST_CONNECTION_PRIORITY,
    BT_GATT_CLIENT_GET_SERVICES,
    BT_GATT_CLIENT_REQUEST_FASTEST_CONNECTION,
    BT_GATT_CLIENT_READ_REMOTE_RSSI_VALUE,
    BT_GATT_CLIENT_REQUEST_NOTIFICATION,
    // The last code, if you want to add a new code, please add it before this
    BT_GATT_CLIENT_BUTT
};

enum BluetoothGattServerCallbackInterfaceCode {
    GATT_SERVER_CALLBACK_CHARACTERISTIC_READREQUEST = 0,
    GATT_SERVER_CALLBACK_CONNECTIONSTATE_CHANGED,
    GATT_SERVER_CALLBACK_ADD_SERVICE,
    GATT_SERVER_CALLBACK_CHARACTERISTIC_READ_REQUEST,
    GATT_SERVER_CALLBACK_CHARACTERISTIC_WRITE_REQUEST,
    GATT_SERVER_CALLBACK_DESCRIPTOR_READ_REQUEST,
    GATT_SERVER_CALLBACK_DESCRIPTOR_WRITE_REQUEST,
    GATT_SERVER_CALLBACK_MTU_CHANGED,
    GATT_SERVER_CALLBACK_NOTIFY_CONFIRM,
    GATT_SERVER_CALLBACK_CONNECTION_PARAMETER_CHANGED,
    // The last code, if you want to add a new code, please add it before this
    GATT_SERVER_CALLBACK_BUTT
};

enum BluetoothGattServerInterfaceCode {
    GATT_SERVER_CLEAR_SERVICES = 0,
    GATT_SERVER_ADD_SERVICE,
    GATT_SERVER_REGISTER,
    GATT_SERVER_DEREGISTER,
    GATT_SERVER_CANCEL_CONNECTION,
    GATT_SERVER_NOTIFY_CLIENT,
    GATT_SERVER_REMOVE_SERVICE,
    GATT_SERVER_RESPOND_CHARACTERISTIC_READ,
    GATT_SERVER_RESPOND_CHARACTERISTIC_WRITE,
    GATT_SERVER_RESPOND_DESCRIPTOR_READ,
    GATT_SERVER_RESPOND_DESCRIPTOR_WRITE,
    GATT_SERVER_CONNECT,
    // The last code, if you want to add a new code, please add it before this
    GATT_SERVER_BUTT
};

enum class BluetoothHidHostObserverInterfaceCode {
    COMMAND_ON_CONNECTION_STATE_CHANGED = MIN_TRANSACTION_ID + 0,
    // The last code, if you want to add a new code, please add it before this
    COMMAND_HID_HOST_OBSERVER_BUTT
};

enum class BluetoothHidHostInterfaceCode {
    COMMAND_CONNECT = MIN_TRANSACTION_ID + 0,
    COMMAND_DISCONNECT,
    COMMAND_GET_DEVICE_STATE,
    COMMAND_GET_DEVICES_BY_STATES,
    COMMAND_REGISTER_OBSERVER,
    COMMAND_DEREGISTER_OBSERVER,
    COMMAND_VCUN_PLUG,
    COMMAND_SEND_DATA,
    COMMAND_SET_REPORT,
    COMMAND_GET_REPORT,
    COMMAND_SET_CONNECT_STRATEGY,
    COMMAND_GET_CONNECT_STRATEGY,
    // The last code, if you want to add a new code, please add it before this
    COMMAND_HID_HOST_BUTT
};

enum BluetoothHostObserverInterfaceCode {
    BT_HOST_OBSERVER_STATE_CHANGE = 0,
    // ON_DIS_STA_CHANGE_CODE
    BT_HOST_OBSERVER_DISCOVERY_STATE_CHANGE,
    BT_HOST_OBSERVER_DISCOVERY_RESULT,
    BT_HOST_OBSERVER_PAIR_REQUESTED,
    BT_HOST_OBSERVER_PAIR_CONFIRMED,
    BT_HOST_OBSERVER_SCAN_MODE_CHANGED,
    BT_HOST_OBSERVER_DEVICE_NAME_CHANGED,
    BT_HOST_OBSERVER_DEVICE_ADDR_CHANGED,
    // The last code, if you want to add a new code, please add it before this
    BT_HOST_OBSERVER_BUTT
};

enum BluetoothHostInterfaceCode {
    BT_REGISTER_OBSERVER = 0,
    BT_DEREGISTER_OBSERVER,
    BT_ENABLE,
    BT_DISABLE,
    BT_GETSTATE,
    BT_GETPROFILE,
    BT_GET_BLE,
    BT_FACTORY_RESET,
    BT_DISABLE_BLE,
    BT_ENABLE_BLE,
    BT_IS_BR_ENABLED,
    BT_IS_BLE_ENABLED,
    BT_GET_PROFILE_LIST,
    BT_GET_MAXNUM_CONNECTED_AUDIODEVICES,
    BT_GET_BT_STATE,
    BT_GET_BT_PROFILE_CONNSTATE,
    BT_GET_LOCAL_DEVICE_CLASS,
    BT_SET_LOCAL_DEVICE_CLASS,
    BT_GET_LOCAL_ADDRESS,
    BT_GET_LOCAL_NAME,
    BT_SET_LOCAL_NAME,
    BT_GET_BT_SCAN_MODE,
    BT_SET_BT_SCAN_MODE,
    BT_GET_BONDABLE_MODE,
    BT_SET_BONDABLE_MODE,
    BT_START_BT_DISCOVERY,
    BT_CANCEL_BT_DISCOVERY,
    BT_IS_BT_DISCOVERING,
    BT_GET_BT_DISCOVERY_END_MILLIS,
    BT_GET_PAIRED_DEVICES,
    BT_REMOVE_PAIR,
    BT_REMOVE_ALL_PAIRS,
    BT_REGISTER_REMOTE_DEVICE_OBSERVER,
    BT_DEREGISTER_REMOTE_DEVICE_OBSERVER,
    BT_GET_BLE_MAX_ADVERTISING_DATALENGTH,
    GET_DEVICE_TYPE,
    GET_PHONEBOOK_PERMISSION,
    SET_PHONEBOOK_PERMISSION,
    GET_MESSAGE_PERMISSION,
    SET_MESSAGE_PERMISSION,
    GET_POWER_MODE,
    GET_DEVICE_NAME,
    GET_DEVICE_ALIAS,
    SET_DEVICE_ALIAS,
    GET_DEVICE_BATTERY_INFO,
    GET_PAIR_STATE,
    START_PAIR,
    CANCEL_PAIRING,
    IS_BONDED_FROM_LOCAL,
    IS_ACL_CONNECTED,
    IS_ACL_ENCRYPTED,
    GET_DEVICE_CLASS,
    SET_DEVICE_PIN,
    SET_DEVICE_PAIRING_CONFIRMATION,
    SET_DEVICE_PASSKEY,
    PAIR_REQUEST_PEPLY,
    READ_REMOTE_RSSI_VALUE,
    GET_LOCAL_SUPPORTED_UUIDS,
    GET_DEVICE_UUIDS,
    BT_REGISTER_BLE_ADAPTER_OBSERVER,
    BT_DEREGISTER_BLE_ADAPTER_OBSERVER,
    BT_REGISTER_BLE_PERIPHERAL_OBSERVER,
    BT_DEREGISTER_BLE_PERIPHERAL_OBSERVER,
    GET_LOCAL_PROFILE_UUIDS,
    BT_SET_FAST_SCAN,
    GET_RANDOM_ADDRESS,
    SYNC_RANDOM_ADDRESS,
    START_CREDIBLE_PAIR,
    BT_COUNT_ENABLE_TIMES,
    CONNECT_ALLOWED_PROFILES,
    DISCONNECT_ALLOWED_PROFILES,
    GET_DEVICE_PRODUCT_ID,
    SET_CUSTOM_TYPE,
    GET_CUSTOM_TYPE,
    // The last code, if you want to add a new code, please add it before this
    BT_HOST_BUTT
};

enum BluetoothMapMceObserverInterfaceCode {
    MCE_ON_ACTION_COMPLETED = 0,
    MCE_ON_CONNECTION_STATE_CHANGED,
    MCE_ON_EVENT_REPORTED,
    MCE_ON_BMESSAGE_COMPLETED,
    MCE_ON_MESSAGE_LIST_COMPLETED,
    MCE_ON_CONVERSATION_LIST_COMPLETED,
    // The last code, if you want to add a new code, please add it before this
    MCE_OBSERVER_BUTT
};

enum BluetoothMapMceInterfaceCode {
    MCE_REGISTER_OBSERVER = 0,
    MCE_DE_REGISTER_OBSERVER,
    MCE_CONNECT,
    MCE_DISCONNECT,
    MCE_ISCONNECTED,
    MCE_GET_CONNECT_DEVICES,
    MCE_GET_DEVICES_BY_STATES,
    MCE_GET_CONNECTION_STATE,
    MCE_SET_CONNECTION_STRATEGY,
    MCE_GET_CONNECTION_STRATEGY,
    MCE_GET_UNREAD_MESSAGES,
    MCE_GET_SUPPORTED_FEATURES,
    MCE_SEND_MESSAGE,
    MCE_SET_NOTIFICATION_FILTER,
    MCE_GET_MESSAGES_LISTING,
    MCE_GET_MESSAGE,
    MCE_UPDATE_INBOX,
    MCE_GET_CONVERSATION_LISTING,
    MCE_SET_MESSAGE_STATUS,
    MCE_SET_OWNER_STATUS,
    MCE_GET_OWNER_STATUS,
    MCE_GET_MAS_INSTANCE_INFO,
    // The last code, if you want to add a new code, please add it before this
    MCE_BUTT
};

enum BluetoothMapMseObserverInterfaceCode {
    MSE_ON_CONNECTION_STATE_CHANGED = 0,
    MSE_ON_PERMISSION,
    // The last code, if you want to add a new code, please add it before this
    MSE_OBSERVER_BUTT
};

enum BluetoothMapMseInterfaceCode {
    MSE_REGISTER_OBSERVER = 0,
    MSE_DEREGISTER_OBSERVER,
    MSE_GET_DEVICE_STATE,
    MSE_DISCONNECT,
    MSE_GET_DEVICES_BY_STATES,
    MSE_SET_CONNECTION_STRATEGY,
    MSE_GET_CONNECTION_STRATEGY,
    MSE_SET_ACCESS_AUTHORIZATION,
    MSE_GET_ACCESS_AUTHORIZATION,
    // The last code, if you want to add a new code, please add it before this
    MSE_BUTT
};

enum BluetoothOppObserverInterfaceCode {
    OPP_ON_RECEIVE_INCOMING_FILE_CHANGED = MIN_TRANSACTION_ID + 0,
    OPP_ON_TRANSFER_STATE_CHANGED,
    // The last code, if you want to add a new code, please add it before this
    OPP_OBSERVER_BUTT
};

enum BluetoothOppInterfaceCode {
    COMMAND_SEND_FILE = MIN_TRANSACTION_ID + 0,
    COMMAND_SET_INCOMING_FILE_CONFIRMATION,
    COMMAND_GET_CURRENT_TRANSFER_INFORMATION,
    COMMAND_CANCEL_TRANSFER,
    COMMAND_REGISTER_OBSERVER,
    COMMAND_DEREGISTER_OBSERVER,
    COMMAND_GET_DEVICE_STATE,
    COMMAND_GET_DEVICES_BY_STATES,
    // The last code, if you want to add a new code, please add it before this
    COMMAND_OPP_BUTT
};

enum class BluetoothPanObserverInterfaceCode {
    COMMAND_ON_CONNECTION_STATE_CHANGED = MIN_TRANSACTION_ID + 0,
    // The last code, if you want to add a new code, please add it before this
    COMMAND_PAN_OBSERVER_BUTT
};

enum class BluetoothPanInterfaceCode {
    COMMAND_DISCONNECT = MIN_TRANSACTION_ID + 0,
    COMMAND_GET_DEVICE_STATE,
    COMMAND_GET_DEVICES_BY_STATES,
    COMMAND_REGISTER_OBSERVER,
    COMMAND_DEREGISTER_OBSERVER,
    COMMAND_SET_TETHERING,
    COMMAND_IS_TETHERING_ON,
    // The last code, if you want to add a new code, please add it before this
    COMMAND_PAN_BUTT
};


enum BluetoothPbapPceObserverInterfaceCode {
    PBAP_PCE_ON_SERVICE_CONNECTION_STATE_CHANGED = 0,
    PBAP_PCE_ON_SERVICE_PASSWORD_REQUIRED,
    PBAP_PCE_ON_ACTION_COMPLETED,
    // The last code, if you want to add a new code, please add it before this
    PBAP_PCE_OBSERVER_BUTT
};

enum BluetoothPbapPceInterfaceCode {
    PBAP_PCE_GET_DEVICE_STATE = 0,
    PBAP_PCE_CONNECT,
    PBAP_PCE_PULL_PHONEBOOK,
    PBAP_PCE_SET_PHONEBOOK,
    PBAP_PCE_PULL_VCARD_LISTING,
    PBAP_PCE_PULL_VCARD_ENTRY,
    PBAP_PCE_PULL_ISDOWNLOAGING,
    PBAP_PCE_ABORT_DOWNLOADING,
    PBAP_PCE_SET_DEVICE_PASSWORD,
    PBAP_PCE_DISCONNECT,
    PBAP_PCE_SET_CONNECT_STRATEGY,
    PBAP_PCE_GET_CONNECT_STRATEGY,
    PBAP_PCE_GET_DEVICES_BY_STATE,
    PBAP_PCE_REGISTER_OBSERVER,
    PBAP_PCE_DEREGISTER_OBSERVER,
    // The last code, if you want to add a new code, please add it before this
    PBAP_PCE_BUTT
};

enum BluetoothPbapPseObserverInterfaceCode {
    PBAP_PSE_ON_CONNECTION_STATE_CHANGE = 0,
    // The last code, if you want to add a new code, please add it before this
    PBAP_PSE_OBSERVER_BUTT
};

enum BluetoothPbapPseInterfaceCode {
    PBAP_PSE_GET_DEVICE_STATE = 0,
    PBAP_PSE_GET_DEVICES_BY_STATES,
    PBAP_PSE_DISCONNECT,
    PBAP_PSE_SET_CONNECTION_STRATEGY,
    PBAP_PSE_GET_CONNECTION_STRATEGY,
    PBAP_PSE_REGISTER_OBSERVER,
    PBAP_PSE_DEREGISTER_OBSERVER,
    PBAP_PSE_SET_SHARE_TYPE,
    PBAP_PSE_GET_SHARE_TYPE,
    PBAP_PSE_SET_ACCESS_AUTHORIZATION,
    PBAP_PSE_GET_ACCESS_AUTHORIZATION,
    // The last code, if you want to add a new code, please add it before this
    PBAP_PSE_BUTT
};

enum BluetoothRemoteDeviceObserverInterfaceCode {
    BT_REMOTE_DEVICE_OBSERVER_ACL_STATE = 0,
    BT_REMOTE_DEVICE_OBSERVER_PAIR_STATUS,
    BT_REMOTE_DEVICE_OBSERVER_REMOTE_UUID,
    BT_REMOTE_DEVICE_OBSERVER_REMOTE_NAME,
    BT_REMOTE_DEVICE_OBSERVER_REMOTE_ALIAS,
    BT_REMOTE_DEVICE_OBSERVER_REMOTE_COD,
    BT_REMOTE_DEVICE_OBSERVER_REMOTE_BATTERY_LEVEL,
    BT_REMOTE_DEVICE_OBSERVER_REMOTE_BATTERY_INFO_REPORT,
    // The last code, if you want to add a new code, please add it before this
    BT_REMOTE_DEVICE_OBSERVER_BUTT
};

enum BluetoothSocketInterfaceCode {
    SOCKET_CONNECT = 0,
    SOCKET_LISTEN,
    DEREGISTER_SERVER_OBSERVER,
    SOCKET_UPDATE_COC_PARAMS,
    REGISTER_CLIENT_OBSERVER,
    DEREGISTER_CLIENT_OBSERVER,
    // The last code, if you want to add a new code, please add it before this
    SOCKET_BUTT
};

enum BluetoothSocketObserverInterfaceCode {
    BT_SOCKET_OBSERVER_CONNECTION_STATE_CHANGED = 0,
    // The last code, if you want to add a new code, please add it before this
    BT_SOCKET_OBSERVER_BUTT
};

enum BluetoothAudioManagerInterfaceCode {
    WEAR_DETECTION_ENABLE = 0,
    WEAR_DETECTION_DISABLE,
    IS_WEAR_DETECTION_ENABLED,
    BT_IS_WEAR_DETECTION_SUPPORTED,
    BT_SEND_DEVICE_SELECTION,
    // The last code, if you want to add a new code, please add it before this
    WEAR_DETECTION_BUTT
};

}  // namespace Bluetooth
}  // namespace OHOS
#endif  // BLUETOOTH_SERVICE_IPC_INTERFACE_CODE_H