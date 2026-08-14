/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_HEADSET_INTERFACE_H
#define BLUETOOTH_HEADSET_INTERFACE_H

#include "hardware/bt_hf.h"
#include "types/raw_address.h"

namespace bluetooth {
namespace headset {

class Callbacks;

/* Interface exposed by the HFP AG stack. */
class Interface {
public:
    virtual ~Interface() = default;

    virtual bt_status_t Init(Callbacks *callbacks, int maxHfClients,
                             bool inbandRingingEnabled) = 0;
    virtual void Cleanup() = 0;
    virtual bt_status_t Connect(RawAddress *bdAddr) = 0;
    virtual bt_status_t Disconnect(RawAddress *bdAddr) = 0;
    virtual bt_status_t ConnectAudio(RawAddress *bdAddr) = 0;
    virtual bt_status_t DisconnectAudio(RawAddress *bdAddr) = 0;
    virtual bt_status_t StartVoiceRecognition(RawAddress *bdAddr) = 0;
    virtual bt_status_t StopVoiceRecognition(RawAddress *bdAddr) = 0;
    virtual bt_status_t VolumeControl(bthf_volume_type_t type, int volume,
                                      RawAddress *bdAddr) = 0;
    virtual bt_status_t CopsResponse(const char *cops, RawAddress *bdAddr) = 0;
    virtual bt_status_t CindResponse(int svc, int num_active, int num_held,
                                     bthf_call_state_t call_setup, int signal,
                                     int roam, int batt_chg,
                                     RawAddress *bdAddr) = 0;
    virtual bt_status_t FormattedAtResponse(const char *rsp,
                                            RawAddress *bdAddr) = 0;
    virtual bt_status_t AtResponse(bthf_at_response_t response_code,
                                   int error_code, RawAddress *bdAddr) = 0;
    virtual bt_status_t ClccResponse(int index, bthf_call_direction_t dir,
                                     bthf_call_state_t state,
                                     bthf_call_mode_t mode,
                                     bthf_call_mpty_type_t mpty,
                                     const char *number,
                                     bthf_call_addrtype_t type,
                                     RawAddress *bdAddr) = 0;
    virtual bt_status_t PhoneStateChange(int num_active, int num_held,
                                         bthf_call_state_t call_state,
                                         const char *number,
                                         bthf_call_addrtype_t type,
                                         const char *name,
                                         RawAddress *bdAddr) = 0;
    virtual void SetActiveDevice(RawAddress *bdAddr) = 0;
    virtual bt_status_t SendBsir(int value, RawAddress *bdAddr) = 0;
    virtual bt_status_t DeviceStatusNotification(
        bthf_network_state_t ntk_state, bthf_service_type_t svc_type,
        int signal, int batt_chg, RawAddress *bdAddr) = 0;
    virtual bt_status_t isVoiceRecognitionSupported(RawAddress *bdAddr) = 0;
};

}  // namespace headset
}  // namespace bluetooth

#endif  // BLUETOOTH_HEADSET_INTERFACE_H
