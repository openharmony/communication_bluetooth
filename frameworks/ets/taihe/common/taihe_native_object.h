/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef TAIHE_NATIVE_OBJECT_H
#define TAIHE_NATIVE_OBJECT_H

#include "taihe_bluetooth_utils.h"

#include "bluetooth_gatt_characteristic.h"

namespace OHOS {
namespace Bluetooth {
class TaiheNativeObject {
public:
    virtual ~TaiheNativeObject() = default;
    virtual ani_ref ToTaiheValue(ani_env *env) const = 0;
};

class TaiheNativeEmpty : public TaiheNativeObject {
public:
    ~TaiheNativeEmpty() override = default;

    ani_ref ToTaiheValue(ani_env *env) const override
    {
        return TaiheGetNull(env);
    }
};

class TaiheNativeUndefined : public TaiheNativeObject {
public:
    ~TaiheNativeUndefined() override = default;

    ani_ref ToTaiheValue(ani_env *env) const override
    {
        return TaiheGetUndefined(env);
    }
};

class TaiheNativeInt : public TaiheNativeObject {
public:
    explicit TaiheNativeInt(int value) : value_(value) {}
    ~TaiheNativeInt() override = default;

    ani_ref ToTaiheValue(ani_env *env) const override;
private:
    int value_;
};

class TaiheNativeBool : public TaiheNativeObject {
public:
    explicit TaiheNativeBool(bool value) : value_(value) {}
    ~TaiheNativeBool() override = default;

    ani_ref ToTaiheValue(ani_env *env) const override;
private:
    bool value_;
};

class TaiheNativeBleCharacteristic : public TaiheNativeObject {
public:
    TaiheNativeBleCharacteristic(const GattCharacteristic &character) : character_(character) {}
    ~TaiheNativeBleCharacteristic() override = default;

    ani_ref ToTaiheValue(ani_env *env) const override;
private:
    GattCharacteristic character_;
};

class TaiheNativeBleDescriptor : public TaiheNativeObject {
public:
    TaiheNativeBleDescriptor(const GattDescriptor &descriptor) : descriptor_(descriptor) {}
    ~TaiheNativeBleDescriptor() override = default;

    ani_ref ToTaiheValue(ani_env *env) const override;
private:
    GattDescriptor descriptor_;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // TAIHE_NATIVE_OBJECT_H