/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

/*
 * Stub of the removed stack layer btif gatt glue (btif_gatt.h).
 * The scanner/advertiser interfaces accept any argument list so that every
 * service layer call site compiles; the real stack is dlopened at runtime
 * and the instances stay null in the stub world.
 */

#ifndef BTIF_GATT_H
#define BTIF_GATT_H

#include "bt_types.h"
#include "bt_gatt.h"

class BleScannerInterface {
public:
    virtual ~BleScannerInterface() = default;
    template <typename... Args>
    void Scan(Args &&...args)
    {
    }
    template <typename... Args>
    void ScanFilterEnable(Args &&...args)
    {
    }
    template <typename... Args>
    void ScanFilterParamSetup(Args &&...args)
    {
    }
    template <typename... Args>
    void ScanFilterAdd(Args &&...args)
    {
    }
    template <typename... Args>
    void BatchscanEnable(Args &&...args)
    {
    }
    template <typename... Args>
    void BatchscanDisable(Args &&...args)
    {
    }
    template <typename... Args>
    void BatchscanConfigStorage(Args &&...args)
    {
    }
    template <typename... Args>
    void BatchscanReadReports(Args &&...args)
    {
    }
    template <typename... Args>
    void SetScanParameters(Args &&...args)
    {
    }
};

class BleAdvertiserInterface {
public:
    virtual ~BleAdvertiserInterface() = default;
    template <typename... Args>
    void RegisterAdvertiser(Args &&...args)
    {
    }
    template <typename... Args>
    void Unregister(Args &&...args)
    {
    }
    template <typename... Args>
    void SetData(Args &&...args)
    {
    }
    template <typename... Args>
    void SetParameters(Args &&...args)
    {
    }
    template <typename... Args>
    void StartAdvertising(Args &&...args)
    {
    }
    template <typename... Args>
    void Enable(Args &&...args)
    {
    }
};

inline BleScannerInterface *get_ble_scanner_instance()
{
    return nullptr;
}

inline BleAdvertiserInterface *get_ble_advertiser_instance()
{
    return nullptr;
}

#endif  // BTIF_GATT_H
