/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
#ifndef NAPI_BLUETOOTH_UTILS_H
#define NAPI_BLUETOOTH_UTILS_H

#include "bluetooth_gatt_client.h"
#include "bluetooth_gatt_descriptor.h"
#include "bluetooth_gatt_server.h"
#include "bluetooth_gatt_service.h"
#include "bluetooth_log.h"
#include "bluetooth_opp.h"
#include "bluetooth_remote_device.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <cstdint>
#include <string>
#include <vector>

#include "uv.h"

#include "bluetooth_socket.h"

namespace OHOS {
namespace Bluetooth {
constexpr size_t CALLBACK_SIZE = 1;
constexpr size_t ARGS_SIZE_ZERO = 0;
constexpr size_t ARGS_SIZE_ONE = 1;
constexpr size_t ARGS_SIZE_TWO = 2;
constexpr size_t ARGS_SIZE_THREE = 3;
constexpr size_t ARGS_SIZE_FOUR = 4;
constexpr int32_t DEFAULT_INT32 = 0;
constexpr int32_t PARAM0 = 0;
constexpr int32_t PARAM1 = 1;
constexpr int32_t PARAM2 = 2;
constexpr int32_t CODE_SUCCESS = 0;
constexpr int32_t CODE_FAILED = -1;
constexpr int ASYNC_IDLE = 0;
constexpr int ASYNC_START = 1;
constexpr int ASYNC_DONE = 2;
constexpr int32_t THREAD_WAIT_TIMEOUT = 5;
constexpr int32_t BLUETOOTH_DEVICE_FIND_TYPE = 1;
constexpr int32_t STATE_CHANGE_TYPE = 2;
constexpr int32_t PIN_REQUEST_TYPE = 3;
constexpr int32_t BOND_STATE_CHANGE_TYPE = 4;
constexpr int32_t BLE_DEVICE_FIND_TYPE = 5;

constexpr uint32_t INVALID_REF_COUNT = 0xFF;

struct ServerResponse {
    std::string deviceId = "";
    int transId = 0;
    int status = 0;
    int offset = 0;
    uint8_t *value = nullptr;
    int length = 0;
    void SetValue(uint8_t *values, size_t len)
    {
        HILOGI("GattCharacteristic::SetValue starts");
        if (value != nullptr) {
            value = nullptr;
        }

        length = static_cast<int>(len);
        value = values;
    }
};

struct SppOption {
    std::string uuid_ = "";
    bool secure_ = false;
    BtSocketType type_;
};

const char * const REGISTER_STATE_CHANGE_TYPE = "stateChange";

const char * const INVALID_DEVICE_ID = "00:00:00:00:00:00";

bool ParseString(napi_env env, std::string &param, napi_value args);
bool ParseInt32(napi_env env, int32_t &param, napi_value args);
bool ParseBool(napi_env env, bool &param, napi_value args);
bool ParseArrayBuffer(napi_env env, uint8_t **data, size_t &size, napi_value args);
napi_value GetCallbackErrorValue(napi_env env, int errCode);

napi_status ConvertStringVectorToJS(napi_env env, napi_value result, std::vector<std::string> &stringVector);
void ConvertStateChangeParamToJS(napi_env env, napi_value result, const std::string &device, int state, int cause);
void ConvertScoStateChangeParamToJS(napi_env env, napi_value result, const std::string &device, int state);
void ConvertUuidsVectorToJS(napi_env env, napi_value result, const std::vector<std::string> &uuids);
napi_status ConvertOppTransferInformationToJS(napi_env env,
    napi_value result, const BluetoothOppTransferInformation& transferInformation);

std::shared_ptr<SppOption> GetSppOptionFromJS(napi_env env, napi_value object);

void SetNamedPropertyByInteger(napi_env env, napi_value dstObj, int32_t objName, const char *propName);
void SetNamedPropertyByString(napi_env env, napi_value dstObj, const std::string &strValue, const char *propName);
napi_value NapiGetNull(napi_env env);
napi_value NapiGetBooleanFalse(napi_env env);
napi_value NapiGetBooleanTrue(napi_env env);
napi_value NapiGetBooleanRet(napi_env env, bool ret);
napi_value NapiGetUndefinedRet(napi_env env);
napi_value NapiGetInt32Ret(napi_env env, int32_t res);

int GetProfileConnectionState(int state);
int GetScoConnectionState(int state);
uint32_t GetProfileId(int profile);

struct AsyncCallbackInfo {
    napi_env env_;
    napi_async_work asyncWork_;
    napi_deferred deferred_;
    napi_ref callback_ = 0;
    int errorCode_ = 0;
};

struct BluetoothCallbackInfo {
    napi_env env_;
    napi_ref callback_ = 0;
    int state_;
    std::string deviceId_;
    int info_;
};

struct PairConfirmedCallBackInfo {
    int number;
    int pinType;
    std::string deviceAddr;

    PairConfirmedCallBackInfo(int number, int pinType, std::string deviceAddr)
    {
        this->number = number;
        this->pinType = pinType;
        this->deviceAddr = deviceAddr;
    }
};

struct TransforInformationCallbackInfo : public BluetoothCallbackInfo {
    std::shared_ptr<BluetoothOppTransferInformation> information_;
};

namespace {
using sysBLEMap = std::map<std::string, std::array<std::shared_ptr<BluetoothCallbackInfo>, ARGS_SIZE_THREE>>;
sysBLEMap g_sysBLEObserver;
std::mutex g_sysBLEObserverMutex;
std::map<std::string, std::shared_ptr<BluetoothCallbackInfo>> g_Observer;
std::mutex g_observerMutex;
}  // namespace
std::map<std::string, std::shared_ptr<BluetoothCallbackInfo>> GetObserver();
const sysBLEMap &GetSysBLEObserver();

void RegisterSysBLEObserver(const std::shared_ptr<BluetoothCallbackInfo> &, int32_t, const std::string &);
void UnregisterSysBLEObserver(const std::string &);
std::shared_ptr<BluetoothCallbackInfo> GetCallbackInfoByType(const std::string &type);

struct ScanFilter {
    std::string deviceId;     // The name of a BLE peripheral device
    std::string name;         // The name of a BLE peripheral device
    UUID serviceUuid;  // The service UUID of a BLE peripheral device
    UUID serviceUuidMask;
    UUID serviceSolicitationUuid;
    UUID serviceSolicitationUuidMask;

    std::vector<uint8_t> serviceData;
    std::vector<uint8_t> serviceDataMask;

    uint16_t manufacturerId = 0;
    std::vector<uint8_t> manufactureData;
    std::vector<uint8_t> manufactureDataMask;
};

enum class MatchMode {
    MATCH_MODE_AGGRESSIVE = 1,  //  aggressive mode
    MATCH_MODE_STICKY = 2       // sticky mode
};

enum class SensitivityMode {
    SENSITIVITY_MODE_HIGH = 1,  //  high sensitivity mode
    SENSITIVITY_MODE_LOW = 2    //  low sensitivity mode
};

enum class ScanDuty {
    SCAN_MODE_LOW_POWER = 0,   // low power mode */
    SCAN_MODE_BALANCED = 1,    // balanced power mode
    SCAN_MODE_LOW_LATENCY = 2  // Scan using highest duty cycle
};

enum class PhyType {
    PHY_LE_1M = 1,              // phy 1M
    PHY_LE_2M = 2,              // phy 2M
    PHY_LE_CODED = 3,           // phy coded
    PHY_LE_ALL_SUPPORTED = 255  // phy coded
};

enum class AdvertisingState {
    STARTED = 1,  // advertiser started
    ENABLED = 2,  // advertiser temporarily enabled
    DISABLED = 3, // advertiser temporarily disabled
    STOPPED = 4  // advertiser stopped
};

enum class ScanReportType {
    ON_FOUND = 1, // the found of advertisement packet
    ON_LOST = 2 // the lost of advertisement packet
};

// Report mode used during scan.
enum class ScanReportMode {
    NORMAL = 1,
    FENCE_SENSITIVITY_LOW = 10,
    FENCE_SENSITIVITY_HIGH = 11,
};

struct ScanOptions {
    int32_t interval = 0;                                   // Time of delay for reporting the scan result
    ScanDuty dutyMode = ScanDuty::SCAN_MODE_LOW_POWER;       // Bluetooth LE scan mode
    MatchMode matchMode = MatchMode::MATCH_MODE_AGGRESSIVE;  // Match mode for Bluetooth LE scan filters hardware match
    SensitivityMode sensitivityMode = SensitivityMode::SENSITIVITY_MODE_HIGH; // Sensitivity Mode for scan
    PhyType phyType = PhyType::PHY_LE_1M;                    // Phy for Bluetooth LE scan
    ScanReportMode reportMode = ScanReportMode::NORMAL;      // Scan report mode
};

struct ScanResult {
    std::string deviceId;       // Address of the scanned device
    int32_t rssi;               // RSSI of the remote device
    std::vector<uint8_t> data;  // The raw data of broadcast packet
};

struct ScanReport {
    ScanReportType reportType;
    std::vector<ScanResult> scanResult;
};

struct NapiAdvManufactureData {
    uint16_t id = 0;
    std::string value {};
};

struct NapiAdvServiceData {
    std::string uuid {};
    std::vector<uint8_t> value {};
};

struct NapiNotifyCharacteristic {
    UUID serviceUuid;
    UUID characterUuid;
    std::vector<uint8_t> characterValue {};
    bool confirm;
};

struct NapiGattsServerResponse {
    std::string deviceId {};
    int transId;
    int status;
    int offset;
    std::vector<uint8_t> value;
};

enum ProfileConnectionState {
    STATE_DISCONNECTED = 0,  // the current profile is disconnected
    STATE_CONNECTING = 1,    // the current profile is being connected
    STATE_CONNECTED = 2,     // the current profile is connected
    STATE_DISCONNECTING = 3  // the current profile is being disconnected
};

enum ScoState {
    SCO_DISCONNECTED,
    SCO_CONNECTING,
    SCO_DISCONNECTING,
    SCO_CONNECTED
};

enum ConnectionStrategy {
    CONNECTION_UNKNOWN = 0,
    CONNECTION_ALLOWED = 1,
    CONNECTION_FORBIDDEN = 2,
};

enum MajorClass {
    MAJOR_MISC = 0x0000,
    MAJOR_COMPUTER = 0x0100,
    MAJOR_PHONE = 0x0200,
    MAJOR_NETWORKING = 0x0300,
    MAJOR_AUDIO_VIDEO = 0x0400,
    MAJOR_PERIPHERAL = 0x0500,
    MAJOR_IMAGING = 0x0600,
    MAJOR_WEARABLE = 0x0700,
    MAJOR_TOY = 0x0800,
    MAJOR_HEALTH = 0x0900,
    MAJOR_UNCATEGORIZED = 0x1F00
};

enum MajorMinorClass {
    // The Minor Device Class field
    // Computer Major Class
    COMPUTER_UNCATEGORIZED = 0x0100,
    COMPUTER_DESKTOP = 0x0104,
    COMPUTER_SERVER = 0x0108,
    COMPUTER_LAPTOP = 0x010C,
    COMPUTER_HANDHELD_PC_PDA = 0x0110,
    COMPUTER_PALM_SIZE_PC_PDA = 0x0114,
    COMPUTER_WEARABLE = 0x0118,
    COMPUTER_TABLET = 0x011C,

    // Phone Major Class
    PHONE_UNCATEGORIZED = 0x0200,
    PHONE_CELLULAR = 0x0204,
    PHONE_CORDLESS = 0x0208,
    PHONE_SMART = 0x020C,
    PHONE_MODEM_OR_GATEWAY = 0x0210,
    PHONE_ISDN = 0x0214,

    // LAN/Network Access Point Major Class
    NETWORK_FULLY_AVAILABLE = 0x0300,
    NETWORK_1_TO_17_UTILIZED = 0x0320,
    NETWORK_17_TO_33_UTILIZED = 0x0340,
    NETWORK_33_TO_50_UTILIZED = 0x0360,
    NETWORK_60_TO_67_UTILIZED = 0x0380,
    NETWORK_67_TO_83_UTILIZED = 0x03A0,
    NETWORK_83_TO_99_UTILIZED = 0x03C0,
    NETWORK_NO_SERVICE = 0x03E0,

    // Audio/Video Major Class
    AUDIO_VIDEO_UNCATEGORIZED = 0x0400,
    AUDIO_VIDEO_WEARABLE_HEADSET = 0x0404,
    AUDIO_VIDEO_HANDSFREE = 0x0408,
    AUDIO_VIDEO_MICROPHONE = 0x0410,
    AUDIO_VIDEO_LOUDSPEAKER = 0x0414,
    AUDIO_VIDEO_HEADPHONES = 0x0418,
    AUDIO_VIDEO_PORTABLE_AUDIO = 0x041C,
    AUDIO_VIDEO_CAR_AUDIO = 0x0420,
    AUDIO_VIDEO_SET_TOP_BOX = 0x0424,
    AUDIO_VIDEO_HIFI_AUDIO = 0x0428,
    AUDIO_VIDEO_VCR = 0x042C,
    AUDIO_VIDEO_VIDEO_CAMERA = 0x0430,
    AUDIO_VIDEO_CAMCORDER = 0x0434,
    AUDIO_VIDEO_VIDEO_MONITOR = 0x0438,
    AUDIO_VIDEO_VIDEO_DISPLAY_AND_LOUDSPEAKER = 0x043C,
    AUDIO_VIDEO_VIDEO_CONFERENCING = 0x0440,
    AUDIO_VIDEO_VIDEO_GAMING_TOY = 0x0448,

    // Peripheral Major Class
    PERIPHERAL_NON_KEYBOARD_NON_POINTING = 0x0500,
    PERIPHERAL_KEYBOARD = 0x0540,
    PERIPHERAL_POINTING_DEVICE = 0x0580,
    PERIPHERAL_KEYBOARD_POINTING = 0x05C0,
    PERIPHERAL_UNCATEGORIZED = 0x0500,
    PERIPHERAL_JOYSTICK = 0x0504,
    PERIPHERAL_GAMEPAD = 0x0508,
    PERIPHERAL_REMOTE_CONTROL = 0x05C0,
    PERIPHERAL_SENSING_DEVICE = 0x0510,
    PERIPHERAL_DIGITIZER_TABLET = 0x0514,
    PERIPHERAL_CARD_READER = 0x0518,
    PERIPHERAL_DIGITAL_PEN = 0x051C,
    PERIPHERAL_SCANNER_RFID = 0x0520,
    PERIPHERAL_GESTURAL_INPUT = 0x0522,

    // Imaging Major Class
    IMAGING_UNCATEGORIZED = 0x0600,
    IMAGING_DISPLAY = 0x0610,
    IMAGING_CAMERA = 0x0620,
    IMAGING_SCANNER = 0x0640,
    IMAGING_PRINTER = 0x0680,

    // Wearable Major Class
    WEARABLE_UNCATEGORIZED = 0x0700,
    WEARABLE_WRIST_WATCH = 0x0704,
    WEARABLE_PAGER = 0x0708,
    WEARABLE_JACKET = 0x070C,
    WEARABLE_HELMET = 0x0710,
    WEARABLE_GLASSES = 0x0714,

    // Minor Device Class field - Toy Major Class
    TOY_UNCATEGORIZED = 0x0800,
    TOY_ROBOT = 0x0804,
    TOY_VEHICLE = 0x0808,
    TOY_DOLL_ACTION_FIGURE = 0x080C,
    TOY_CONTROLLER = 0x0810,
    TOY_GAME = 0x0814,

    // Minor Device Class field - Health
    HEALTH_UNCATEGORIZED = 0x0900,
    HEALTH_BLOOD_PRESSURE = 0x0904,
    HEALTH_THERMOMETER = 0x0908,
    HEALTH_WEIGHING = 0x090C,
    HEALTH_GLUCOSE = 0x0910,
    HEALTH_PULSE_OXIMETER = 0x0914,
    HEALTH_PULSE_RATE = 0x0918,
    HEALTH_DATA_DISPLAY = 0x091C,
    HEALTH_STEP_COUNTER = 0x0920,
    HEALTH_BODY_COMPOSITION_ANALYZER = 0x0924,
    HEALTH_PEAK_FLOW_MOITOR = 0x0928, // HEALTH_PEAK_FLOW_MOITOR is compatible with api8.
    HEALTH_PEAK_FLOW_MONITOR = 0x0928,
    HEALTH_MEDICATION_MONITOR = 0x092C,
    HEALTH_KNEE_PROSTHESIS = 0x0930,
    HEALTH_ANKLE_PROSTHESIS = 0x0934,
    HEALTH_GENERIC_HEALTH_MANAGER = 0x0938,
    HEALTH_PERSONAL_MOBILITY_DEVICE = 0x093C,
};

enum SppType {
    /** RFCOMM */
    SPP_RFCOMM = 0
};

enum PlayingState {
    STATE_NOT_PLAYING = 0,
    STATE_PLAYING = 1
};

enum ProfileId {
    PROFILE_A2DP_SINK = 0,
    PROFILE_A2DP_SOURCE = 1,
    PROFILE_AVRCP_CT = 2,
    PROFILE_AVRCP_TG = 3,
    PROFILE_HANDS_FREE_AUDIO_GATEWAY = 4,
    PROFILE_HANDS_FREE_UNIT = 5,
    PROFILE_HID_HOST = 6,
    PROFILE_PAN_NETWORK = 7,
    PROFILE_PBAP_CLIENT = 8,
    PROFILE_PBAP_SERVER = 9,
    PROFILE_OPP = 10
};

enum AccessAuthorization {
    UNKNOWN = 0,
    ALLOWED = 1,
    REJECTED = 2,
};

template<typename T1, typename T2, typename T3>
struct AfterWorkCallbackData {
    T1* object;
    T2 function;
    napi_env env;
    napi_ref callback;
    T3 data;
};

template<typename T>
void AfterWorkCallback(uv_work_t *work, int status)
{
    if (work == nullptr) {
        return;
    }
    T data = static_cast<T>(work->data);
    if (data == nullptr) {
        return;
    }
    (data->object->*(data->function))(work, data->data);
    if (work->data != nullptr) {
        delete data;
        work->data = nullptr;
    }
    delete work;
    work = nullptr;
}

#define NAPI_BT_CALL_RETURN(func)                                          \
    do {                                                                   \
        napi_status ret = (func);                                          \
        if (ret != napi_ok) {                                              \
            HILOGE("napi call function failed. ret:%{public}d", ret);      \
            return ret;                                                    \
        }                                                                  \
    } while (0)

#define NAPI_BT_RETURN_IF(condition, msg, ret)              \
    do {                                                    \
        if ((condition)) {                                  \
            HILOGE(msg);                                    \
            return (ret);                                   \
        }                                                   \
    } while (0)

int DoInJsMainThread(napi_env env, std::function<void(void)> func);

bool IsValidAddress(std::string bdaddr);
bool IsValidTransport(int transport);
bool IsValidConnectStrategy(int strategy);
napi_status NapiIsBoolean(napi_env env, napi_value value);
napi_status NapiIsNumber(napi_env env, napi_value value);
napi_status NapiIsString(napi_env env, napi_value value);
napi_status NapiIsFunction(napi_env env, napi_value value);
napi_status NapiIsArray(napi_env env, napi_value value);
napi_status NapiIsArrayBuffer(napi_env env, napi_value value);
napi_status NapiIsObject(napi_env env, napi_value value);
napi_status ParseNumberParams(napi_env env, napi_value object, const char *name, bool &outExist, napi_value &outParam);
napi_status ParseInt32Params(napi_env env, napi_value object, const char *name, bool &outExist, int32_t &outParam);
napi_status ParseUint32Params(napi_env env, napi_value object, const char *name, bool &outExist, uint32_t &outParam);
napi_status ParseBooleanParams(napi_env env, napi_value object, const char *name, bool &outExist, bool &outParam);
napi_status ParseStringParams(napi_env env, napi_value object, const char *name, bool &outExist,
    std::string &outParam);
napi_status ParseArrayBufferParams(napi_env env, napi_value object, const char *name, bool &outExist,
    std::vector<uint8_t> &outParam);
napi_status ParseUuidParams(napi_env env, napi_value object, const char *name, bool &outExist, UUID &outUuid);

bool CheckDeivceIdParam(napi_env env, napi_callback_info info, std::string &addr);
bool CheckProfileIdParam(napi_env env, napi_callback_info info, int &profileId);
bool CheckProfileIdParamEx(napi_env env, napi_callback_info info, int &profileId, size_t &argc);
bool CheckSetDevicePairingConfirmationParam(napi_env env, napi_callback_info info, std::string &addr, bool &accept);
bool CheckLocalNameParam(napi_env env, napi_callback_info info, std::string &name);
bool CheckSetBluetoothScanModeParam(napi_env env, napi_callback_info info, int32_t &mode, int32_t &duration);
napi_status CheckEmptyParam(napi_env env, napi_callback_info info);
napi_status NapiCheckObjectPropertiesName(napi_env env, napi_value object, const std::vector<std::string> &names);
napi_status CheckSetConnectStrategyParam(napi_env env, napi_callback_info info, std::string &addr, int32_t &strategy);
napi_status CheckDeviceAddressParam(napi_env env, napi_callback_info info, std::string &addr);
napi_status CheckAccessAuthorizationParam(napi_env env, napi_callback_info info, std::string &addr,
    int32_t &accessAuthorization);
napi_status NapiGetOnOffCallbackName(napi_env env, napi_callback_info info, std::string &name);
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // NAPI_BLUETOOTH_UTILS_H
