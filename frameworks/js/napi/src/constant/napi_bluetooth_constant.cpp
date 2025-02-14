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
#ifndef LOG_TAG
#define LOG_TAG "bt_napi_constant"
#endif

#include "napi_bluetooth_constant.h"

#include "bluetooth_log.h"
#include "napi_bluetooth_utils.h"
#include "hitrace_meter.h"

namespace {
const std::string PROFILE_UUID_BASE_UUID = "00000000-0000-1000-8000-00805F9B34FB";
const std::string PROFILE_UUID_HFP_AG = "0000111F-0000-1000-8000-00805F9B34FB";
const std::string PROFILE_UUID_HFP_HF = "0000111E-0000-1000-8000-00805F9B34FB";
const std::string PROFILE_UUID_HSP_AG = "00001112-0000-1000-8000-00805F9B34FB";
const std::string PROFILE_UUID_HSP_HS = "00001108-0000-1000-8000-00805F9B34FB";
const std::string PROFILE_UUID_A2DP_SRC = "0000110A-0000-1000-8000-00805F9B34FB";
const std::string PROFILE_UUID_A2DP_SINK = "0000110B-0000-1000-8000-00805F9B34FB";
const std::string PROFILE_UUID_AVRCP_CT = "0000110E-0000-1000-8000-00805F9B34FB";
const std::string PROFILE_UUID_AVRCP_TG = "0000110C-0000-1000-8000-00805F9B34FB";
const std::string PROFILE_UUID_HID = "00001124-0000-1000-8000-00805F9B34FB";
const std::string PROFILE_UUID_HOGP = "00001812-0000-1000-8000-00805F9B34FB";
const std::string PROFILE_UUID_PBAP_PSE = "0000112f-0000-1000-8000-00805F9B34FB";
}  // namespace

namespace OHOS {
namespace Bluetooth {
napi_value NapiConstant::DefineJSConstant(napi_env env, napi_value exports)
{
    HILOGD("enter");
    ConstantPropertyValueInit(env, exports);
    napi_property_descriptor desc[] = {};
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}

napi_value NapiConstant::ConstantPropertyValueInit(napi_env env, napi_value exports)
{
    HILOGD("enter");
    napi_value profileIdObj = ProfileIdInit(env);
    napi_value profileUuidsObj = ProfileUuidsInit(env);
    napi_value profileStateObj = ProfileStateInit(env);
    napi_value majorClassObj = MajorClassOfDeviceInit(env);
    napi_value majorMinorClassObj = MajorMinorClassOfDeviceInit(env);
    napi_value accessAuthorizationObj = AccessAuthorizationInit(env);
    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("ProfileId", profileIdObj),
        DECLARE_NAPI_PROPERTY("ProfileUuids", profileUuidsObj),
        DECLARE_NAPI_PROPERTY("ProfileConnectionState", profileStateObj),
        DECLARE_NAPI_PROPERTY("MajorClass", majorClassObj),
        DECLARE_NAPI_PROPERTY("MajorMinorClass", majorMinorClassObj),
        DECLARE_NAPI_PROPERTY("AccessAuthorization", accessAuthorizationObj),
    };
    HITRACE_METER_NAME(HITRACE_TAG_OHOS, "constant:napi_define_properties");
    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);
    return exports;
}

napi_value NapiConstant::ProfileIdInit(napi_env env)
{
    HILOGD("enter");
    napi_value profileId = nullptr;
    napi_create_object(env, &profileId);
    SetNamedPropertyByInteger(env, profileId, ProfileId::PROFILE_A2DP_SINK, "PROFILE_A2DP_SINK");
    SetNamedPropertyByInteger(env, profileId, ProfileId::PROFILE_A2DP_SOURCE, "PROFILE_A2DP_SOURCE");
    SetNamedPropertyByInteger(env, profileId, ProfileId::PROFILE_AVRCP_CT, "PROFILE_AVRCP_CT");
    SetNamedPropertyByInteger(env, profileId, ProfileId::PROFILE_AVRCP_TG, "PROFILE_AVRCP_TG");
#ifdef BLUETOOTH_API_SINCE_10
    SetNamedPropertyByInteger(
        env, profileId, ProfileId::PROFILE_HANDS_FREE_AUDIO_GATEWAY, "PROFILE_HANDSFREE_AUDIO_GATEWAY");
#else
    SetNamedPropertyByInteger(
        env, profileId, ProfileId::PROFILE_HANDS_FREE_AUDIO_GATEWAY, "PROFILE_HANDS_FREE_AUDIO_GATEWAY");
#endif
    SetNamedPropertyByInteger(env, profileId, ProfileId::PROFILE_HANDS_FREE_UNIT, "PROFILE_HANDS_FREE_UNIT");
    SetNamedPropertyByInteger(env, profileId, ProfileId::PROFILE_HID_HOST, "PROFILE_HID_HOST");
    SetNamedPropertyByInteger(env, profileId, ProfileId::PROFILE_PAN_NETWORK, "PROFILE_PAN_NETWORK");
    SetNamedPropertyByInteger(env, profileId, ProfileId::PROFILE_PBAP_CLIENT, "PROFILE_PBAP_PCE");
    SetNamedPropertyByInteger(env, profileId, ProfileId::PROFILE_PBAP_SERVER, "PROFILE_PBAP_PSE");
    SetNamedPropertyByInteger(env, profileId, ProfileId::PROFILE_OPP, "PROFILE_OPP_SERVER");
    return profileId;
}

napi_value NapiConstant::ProfileUuidsInit(napi_env env)
{
    HILOGD("enter");
    napi_value profileUuidsObj = nullptr;
    napi_create_object(env, &profileUuidsObj);
    SetNamedPropertyByString(env, profileUuidsObj, PROFILE_UUID_HFP_AG, "PROFILE_UUID_HFP_AG");
    SetNamedPropertyByString(env, profileUuidsObj, PROFILE_UUID_HFP_HF, "PROFILE_UUID_HFP_HF");
    SetNamedPropertyByString(env, profileUuidsObj, PROFILE_UUID_HSP_AG, "PROFILE_UUID_HSP_AG");
    SetNamedPropertyByString(env, profileUuidsObj, PROFILE_UUID_HSP_HS, "PROFILE_UUID_HSP_HS");
    SetNamedPropertyByString(env, profileUuidsObj, PROFILE_UUID_A2DP_SRC, "PROFILE_UUID_A2DP_SRC");
    SetNamedPropertyByString(env, profileUuidsObj, PROFILE_UUID_A2DP_SINK, "PROFILE_UUID_A2DP_SINK");
    SetNamedPropertyByString(env, profileUuidsObj, PROFILE_UUID_AVRCP_CT, "PROFILE_UUID_AVRCP_CT");
    SetNamedPropertyByString(env, profileUuidsObj, PROFILE_UUID_AVRCP_TG, "PROFILE_UUID_AVRCP_TG");
    SetNamedPropertyByString(env, profileUuidsObj, PROFILE_UUID_HID, "PROFILE_UUID_HID");
    SetNamedPropertyByString(env, profileUuidsObj, PROFILE_UUID_HOGP, "PROFILE_UUID_HOGP");
    SetNamedPropertyByString(env, profileUuidsObj, PROFILE_UUID_PBAP_PSE, "PROFILE_UUID_PBAP_PSE");
    return profileUuidsObj;
}

napi_value NapiConstant::ProfileStateInit(napi_env env)
{
    HILOGD("enter");
    napi_value profileState = nullptr;
    napi_create_object(env, &profileState);
    SetNamedPropertyByInteger(env, profileState, ProfileConnectionState::STATE_DISCONNECTED, "STATE_DISCONNECTED");
    SetNamedPropertyByInteger(env, profileState, ProfileConnectionState::STATE_CONNECTING, "STATE_CONNECTING");
    SetNamedPropertyByInteger(env, profileState, ProfileConnectionState::STATE_CONNECTED, "STATE_CONNECTED");
    SetNamedPropertyByInteger(env, profileState, ProfileConnectionState::STATE_DISCONNECTING, "STATE_DISCONNECTING");
    return profileState;
}

napi_value NapiConstant::MajorClassOfDeviceInit(napi_env env)
{
    HILOGD("enter");
    napi_value majorClass = nullptr;
    napi_create_object(env, &majorClass);
    // MajorClass
    SetNamedPropertyByInteger(env, majorClass, MajorClass::MAJOR_MISC, "MAJOR_MISC");
    SetNamedPropertyByInteger(env, majorClass, MajorClass::MAJOR_COMPUTER, "MAJOR_COMPUTER");
    SetNamedPropertyByInteger(env, majorClass, MajorClass::MAJOR_PHONE, "MAJOR_PHONE");
    SetNamedPropertyByInteger(env, majorClass, MajorClass::MAJOR_NETWORKING, "MAJOR_NETWORKING");
    SetNamedPropertyByInteger(env, majorClass, MajorClass::MAJOR_AUDIO_VIDEO, "MAJOR_AUDIO_VIDEO");
    SetNamedPropertyByInteger(env, majorClass, MajorClass::MAJOR_PERIPHERAL, "MAJOR_PERIPHERAL");
    SetNamedPropertyByInteger(env, majorClass, MajorClass::MAJOR_IMAGING, "MAJOR_IMAGING");
    SetNamedPropertyByInteger(env, majorClass, MajorClass::MAJOR_WEARABLE, "MAJOR_WEARABLE");
    SetNamedPropertyByInteger(env, majorClass, MajorClass::MAJOR_TOY, "MAJOR_TOY");
    SetNamedPropertyByInteger(env, majorClass, MajorClass::MAJOR_HEALTH, "MAJOR_HEALTH");
    SetNamedPropertyByInteger(env, majorClass, MajorClass::MAJOR_UNCATEGORIZED, "MAJOR_UNCATEGORIZED");
    return majorClass;
}

napi_value NapiConstant::MajorMinorClassOfDeviceInit(napi_env env)
{
    HILOGD("enter");
    napi_value majorMinorClass = nullptr;
    napi_create_object(env, &majorMinorClass);
    // MajorMinorClass
    // Computer Major Class
    ComputerMajorClassOfDeviceInit(env, majorMinorClass);
    // Phone Major Class
    PhoneMajorClassOfDeviceInit(env, majorMinorClass);
    // LAN/Network Access Point Major Class
    NetworkMajorClassOfDeviceInit(env, majorMinorClass);
    // Audio/Video Major Class
    AudioVideoMajorClassOfDeviceInit(env, majorMinorClass);
    // Peripheral Major Class
    PeripheralMajorClassOfDeviceInit(env, majorMinorClass);
    // Imaging Major Class
    ImagingMajorClassOfDeviceInit(env, majorMinorClass);
    // Wearable Major Class
    WearableMajorClassOfDeviceInit(env, majorMinorClass);
    // Minor Device Class field - Toy Major Class
    ToyMajorClassOfDeviceInit(env, majorMinorClass);
    // Minor Device Class field - Health
    HealthMajorClassOfDeviceInit(env, majorMinorClass);
    return majorMinorClass;
}

napi_value NapiConstant::ComputerMajorClassOfDeviceInit(napi_env env, napi_value majorMinorClass)
{
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::COMPUTER_UNCATEGORIZED), "COMPUTER_UNCATEGORIZED");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::COMPUTER_DESKTOP), "COMPUTER_DESKTOP");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::COMPUTER_SERVER), "COMPUTER_SERVER");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::COMPUTER_LAPTOP), "COMPUTER_LAPTOP");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::COMPUTER_HANDHELD_PC_PDA), "COMPUTER_HANDHELD_PC_PDA");
    SetNamedPropertyByInteger(env,
        majorMinorClass,
        static_cast<int>(MajorMinorClass::COMPUTER_PALM_SIZE_PC_PDA),
        "COMPUTER_PALM_SIZE_PC_PDA");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::COMPUTER_WEARABLE), "COMPUTER_WEARABLE");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::COMPUTER_TABLET), "COMPUTER_TABLET");
    return majorMinorClass;
}

napi_value NapiConstant::PhoneMajorClassOfDeviceInit(napi_env env, napi_value majorMinorClass)
{
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::PHONE_UNCATEGORIZED), "PHONE_UNCATEGORIZED");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::PHONE_CELLULAR), "PHONE_CELLULAR");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::PHONE_CORDLESS), "PHONE_CORDLESS");
    SetNamedPropertyByInteger(env, majorMinorClass, static_cast<int>(MajorMinorClass::PHONE_SMART), "PHONE_SMART");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::PHONE_MODEM_OR_GATEWAY), "PHONE_MODEM_OR_GATEWAY");
    SetNamedPropertyByInteger(env, majorMinorClass, static_cast<int>(MajorMinorClass::PHONE_ISDN), "PHONE_ISDN");
    return majorMinorClass;
}

napi_value NapiConstant::NetworkMajorClassOfDeviceInit(napi_env env, napi_value majorMinorClass)
{
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::NETWORK_FULLY_AVAILABLE), "NETWORK_FULLY_AVAILABLE");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::NETWORK_1_TO_17_UTILIZED), "NETWORK_1_TO_17_UTILIZED");
    SetNamedPropertyByInteger(env,
        majorMinorClass,
        static_cast<int>(MajorMinorClass::NETWORK_17_TO_33_UTILIZED),
        "NETWORK_17_TO_33_UTILIZED");
    SetNamedPropertyByInteger(env,
        majorMinorClass,
        static_cast<int>(MajorMinorClass::NETWORK_33_TO_50_UTILIZED),
        "NETWORK_33_TO_50_UTILIZED");
    SetNamedPropertyByInteger(env,
        majorMinorClass,
        static_cast<int>(MajorMinorClass::NETWORK_60_TO_67_UTILIZED),
        "NETWORK_60_TO_67_UTILIZED");
    SetNamedPropertyByInteger(env,
        majorMinorClass,
        static_cast<int>(MajorMinorClass::NETWORK_67_TO_83_UTILIZED),
        "NETWORK_67_TO_83_UTILIZED");
    SetNamedPropertyByInteger(env,
        majorMinorClass,
        static_cast<int>(MajorMinorClass::NETWORK_83_TO_99_UTILIZED),
        "NETWORK_83_TO_99_UTILIZED");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::NETWORK_NO_SERVICE), "NETWORK_NO_SERVICE");
    return majorMinorClass;
}

napi_value NapiConstant::AudioVideoMajorClassOfDeviceInit(napi_env env, napi_value majorMinorClass)
{
    SetNamedPropertyByInteger(env, majorMinorClass,
        static_cast<int>(MajorMinorClass::AUDIO_VIDEO_UNCATEGORIZED), "AUDIO_VIDEO_UNCATEGORIZED");
    SetNamedPropertyByInteger(env, majorMinorClass,
        static_cast<int>(MajorMinorClass::AUDIO_VIDEO_WEARABLE_HEADSET), "AUDIO_VIDEO_WEARABLE_HEADSET");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::AUDIO_VIDEO_HANDSFREE), "AUDIO_VIDEO_HANDSFREE");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::AUDIO_VIDEO_MICROPHONE), "AUDIO_VIDEO_MICROPHONE");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::AUDIO_VIDEO_LOUDSPEAKER), "AUDIO_VIDEO_LOUDSPEAKER");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::AUDIO_VIDEO_HEADPHONES), "AUDIO_VIDEO_HEADPHONES");
    SetNamedPropertyByInteger(env, majorMinorClass,
        static_cast<int>(MajorMinorClass::AUDIO_VIDEO_PORTABLE_AUDIO), "AUDIO_VIDEO_PORTABLE_AUDIO");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::AUDIO_VIDEO_CAR_AUDIO), "AUDIO_VIDEO_CAR_AUDIO");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::AUDIO_VIDEO_SET_TOP_BOX), "AUDIO_VIDEO_SET_TOP_BOX");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::AUDIO_VIDEO_HIFI_AUDIO), "AUDIO_VIDEO_HIFI_AUDIO");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::AUDIO_VIDEO_VCR), "AUDIO_VIDEO_VCR");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::AUDIO_VIDEO_VIDEO_CAMERA), "AUDIO_VIDEO_VIDEO_CAMERA");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::AUDIO_VIDEO_CAMCORDER), "AUDIO_VIDEO_CAMCORDER");
    SetNamedPropertyByInteger(env, majorMinorClass,
        static_cast<int>(MajorMinorClass::AUDIO_VIDEO_VIDEO_MONITOR), "AUDIO_VIDEO_VIDEO_MONITOR");
    SetNamedPropertyByInteger(env, majorMinorClass,
        static_cast<int>(MajorMinorClass::AUDIO_VIDEO_VIDEO_DISPLAY_AND_LOUDSPEAKER),
        "AUDIO_VIDEO_VIDEO_DISPLAY_AND_LOUDSPEAKER");
    SetNamedPropertyByInteger(env, majorMinorClass,
        static_cast<int>(MajorMinorClass::AUDIO_VIDEO_VIDEO_CONFERENCING), "AUDIO_VIDEO_VIDEO_CONFERENCING");
    SetNamedPropertyByInteger(env, majorMinorClass,
        static_cast<int>(MajorMinorClass::AUDIO_VIDEO_VIDEO_GAMING_TOY), "AUDIO_VIDEO_VIDEO_GAMING_TOY");
    return majorMinorClass;
}

napi_value NapiConstant::PeripheralMajorClassOfDeviceInit(napi_env env, napi_value majorMinorClass)
{
    SetNamedPropertyByInteger(env,
        majorMinorClass,
        static_cast<int>(MajorMinorClass::PERIPHERAL_NON_KEYBOARD_NON_POINTING),
        "PERIPHERAL_NON_KEYBOARD_NON_POINTING");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::PERIPHERAL_KEYBOARD), "PERIPHERAL_KEYBOARD");
    SetNamedPropertyByInteger(env,
        majorMinorClass,
        static_cast<int>(MajorMinorClass::PERIPHERAL_POINTING_DEVICE),
        "PERIPHERAL_POINTING_DEVICE");
    SetNamedPropertyByInteger(env,
        majorMinorClass,
        static_cast<int>(MajorMinorClass::PERIPHERAL_KEYBOARD_POINTING),
        "PERIPHERAL_KEYBOARD_POINTING");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::PERIPHERAL_UNCATEGORIZED), "PERIPHERAL_UNCATEGORIZED");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::PERIPHERAL_JOYSTICK), "PERIPHERAL_JOYSTICK");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::PERIPHERAL_GAMEPAD), "PERIPHERAL_GAMEPAD");
    SetNamedPropertyByInteger(env,
        majorMinorClass,
        static_cast<int>(MajorMinorClass::PERIPHERAL_REMOTE_CONTROL),
        "PERIPHERAL_REMOTE_CONTROL");
    SetNamedPropertyByInteger(env,
        majorMinorClass,
        static_cast<int>(MajorMinorClass::PERIPHERAL_SENSING_DEVICE),
        "PERIPHERAL_SENSING_DEVICE");
    SetNamedPropertyByInteger(env,
        majorMinorClass,
        static_cast<int>(MajorMinorClass::PERIPHERAL_DIGITIZER_TABLET),
        "PERIPHERAL_DIGITIZER_TABLET");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::PERIPHERAL_CARD_READER), "PERIPHERAL_CARD_READER");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::PERIPHERAL_DIGITAL_PEN), "PERIPHERAL_DIGITAL_PEN");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::PERIPHERAL_SCANNER_RFID), "PERIPHERAL_SCANNER_RFID");
    SetNamedPropertyByInteger(env,
        majorMinorClass,
        static_cast<int>(MajorMinorClass::PERIPHERAL_GESTURAL_INPUT),
        "PERIPHERAL_GESTURAL_INPUT");
    return majorMinorClass;
}

napi_value NapiConstant::ImagingMajorClassOfDeviceInit(napi_env env, napi_value majorMinorClass)
{
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::IMAGING_UNCATEGORIZED), "IMAGING_UNCATEGORIZED");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::IMAGING_DISPLAY), "IMAGING_DISPLAY");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::IMAGING_CAMERA), "IMAGING_CAMERA");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::IMAGING_SCANNER), "IMAGING_SCANNER");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::IMAGING_PRINTER), "IMAGING_PRINTER");
    return majorMinorClass;
}

napi_value NapiConstant::WearableMajorClassOfDeviceInit(napi_env env, napi_value majorMinorClass)
{
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::WEARABLE_UNCATEGORIZED), "WEARABLE_UNCATEGORIZED");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::WEARABLE_WRIST_WATCH), "WEARABLE_WRIST_WATCH");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::WEARABLE_PAGER), "WEARABLE_PAGER");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::WEARABLE_JACKET), "WEARABLE_JACKET");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::WEARABLE_HELMET), "WEARABLE_HELMET");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::WEARABLE_GLASSES), "WEARABLE_GLASSES");
    return majorMinorClass;
}

napi_value NapiConstant::ToyMajorClassOfDeviceInit(napi_env env, napi_value majorMinorClass)
{
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::TOY_UNCATEGORIZED), "TOY_UNCATEGORIZED");
    SetNamedPropertyByInteger(env, majorMinorClass, static_cast<int>(MajorMinorClass::TOY_ROBOT), "TOY_ROBOT");
    SetNamedPropertyByInteger(env, majorMinorClass, static_cast<int>(MajorMinorClass::TOY_VEHICLE), "TOY_VEHICLE");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::TOY_DOLL_ACTION_FIGURE), "TOY_DOLL_ACTION_FIGURE");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::TOY_CONTROLLER), "TOY_CONTROLLER");
    SetNamedPropertyByInteger(env, majorMinorClass, static_cast<int>(MajorMinorClass::TOY_GAME), "TOY_GAME");
    return majorMinorClass;
}

napi_value NapiConstant::HealthMajorClassOfDeviceInit(napi_env env, napi_value majorMinorClass)
{
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::HEALTH_UNCATEGORIZED), "HEALTH_UNCATEGORIZED");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::HEALTH_BLOOD_PRESSURE), "HEALTH_BLOOD_PRESSURE");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::HEALTH_THERMOMETER), "HEALTH_THERMOMETER");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::HEALTH_WEIGHING), "HEALTH_WEIGHING");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::HEALTH_GLUCOSE), "HEALTH_GLUCOSE");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::HEALTH_PULSE_OXIMETER), "HEALTH_PULSE_OXIMETER");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::HEALTH_PULSE_RATE), "HEALTH_PULSE_RATE");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::HEALTH_DATA_DISPLAY), "HEALTH_DATA_DISPLAY");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::HEALTH_STEP_COUNTER), "HEALTH_STEP_COUNTER");
    SetNamedPropertyByInteger(env,
        majorMinorClass,
        static_cast<int>(MajorMinorClass::HEALTH_BODY_COMPOSITION_ANALYZER),
        "HEALTH_BODY_COMPOSITION_ANALYZER");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::HEALTH_PEAK_FLOW_MOITOR), "HEALTH_PEAK_FLOW_MOITOR");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::HEALTH_PEAK_FLOW_MONITOR), "HEALTH_PEAK_FLOW_MONITOR");
    SetNamedPropertyByInteger(env,
        majorMinorClass,
        static_cast<int>(MajorMinorClass::HEALTH_MEDICATION_MONITOR),
        "HEALTH_MEDICATION_MONITOR");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::HEALTH_KNEE_PROSTHESIS), "HEALTH_KNEE_PROSTHESIS");
    SetNamedPropertyByInteger(
        env, majorMinorClass, static_cast<int>(MajorMinorClass::HEALTH_ANKLE_PROSTHESIS), "HEALTH_ANKLE_PROSTHESIS");
    SetNamedPropertyByInteger(env,
        majorMinorClass,
        static_cast<int>(MajorMinorClass::HEALTH_GENERIC_HEALTH_MANAGER),
        "HEALTH_GENERIC_HEALTH_MANAGER");
    SetNamedPropertyByInteger(env,
        majorMinorClass,
        static_cast<int>(MajorMinorClass::HEALTH_PERSONAL_MOBILITY_DEVICE),
        "HEALTH_PERSONAL_MOBILITY_DEVICE");
    return majorMinorClass;
}

napi_value NapiConstant::AccessAuthorizationInit(napi_env env)
{
    HILOGD("enter");
    napi_value accessAuthorization = nullptr;
    napi_create_object(env, &accessAuthorization);
    SetNamedPropertyByInteger(env, accessAuthorization, AccessAuthorization::UNKNOWN, "UNKNOWN");
    SetNamedPropertyByInteger(env, accessAuthorization, AccessAuthorization::ALLOWED, "ALLOWED");
    SetNamedPropertyByInteger(env, accessAuthorization, AccessAuthorization::REJECTED, "REJECTED");
    return accessAuthorization;
}
}  // namespace Bluetooth
}  // namespace OHOS
