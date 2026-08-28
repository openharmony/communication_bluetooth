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

    virtual BtStackStatus Init(Callbacks *callbacks, int maxHfClients,
                             bool inbandRingingEnabled) = 0;
    virtual void Cleanup() = 0;
    virtual BtStackStatus Connect(RawAddress *bdAddr) = 0;
    virtual BtStackStatus Disconnect(RawAddress *bdAddr) = 0;
    virtual BtStackStatus ConnectAudio(RawAddress *bdAddr) = 0;
    virtual BtStackStatus DisconnectAudio(RawAddress *bdAddr) = 0;
    virtual BtStackStatus StartVoiceRecognition(RawAddress *bdAddr) = 0;
    virtual BtStackStatus StopVoiceRecognition(RawAddress *bdAddr) = 0;
    virtual BtStackStatus VolumeControl(BthfVolumeType type, int volume,
                                      RawAddress *bdAddr) = 0;
    virtual BtStackStatus CopsResponse(const char *cops, RawAddress *bdAddr) = 0;
    virtual BtStackStatus CindResponse(int svc, int num_active, int num_held,
                                     BthfCallState call_setup, int signal,
                                     int roam, int batt_chg,
                                     RawAddress *bdAddr) = 0;
    virtual BtStackStatus FormattedAtResponse(const char *rsp,
                                            RawAddress *bdAddr) = 0;
    virtual BtStackStatus AtResponse(BthfAtResponse response_code,
                                   int error_code, RawAddress *bdAddr) = 0;
    virtual BtStackStatus ClccResponse(int index, BthfCallDirection dir,
                                     BthfCallState state,
                                     BthfCallMode mode,
                                     BthfCallMptyType mpty,
                                     const char *number,
                                     BthfCallAddrtype type,
                                     RawAddress *bdAddr) = 0;
    virtual BtStackStatus PhoneStateChange(int num_active, int num_held,
                                         BthfCallState call_state,
                                         const char *number,
                                         BthfCallAddrtype type,
                                         const char *name,
                                         RawAddress *bdAddr) = 0;
    virtual void SetActiveDevice(RawAddress *bdAddr) = 0;
    virtual BtStackStatus SendBsir(int value, RawAddress *bdAddr) = 0;
    virtual BtStackStatus DeviceStatusNotification(
        BthfNetworkState ntk_state, BthfServiceType svc_type,
        int signal, int batt_chg, RawAddress *bdAddr) = 0;
    virtual BtStackStatus isVoiceRecognitionSupported(RawAddress *bdAddr) = 0;
};

}  // namespace headset
}  // namespace bluetooth

#endif  // BLUETOOTH_HEADSET_INTERFACE_H
