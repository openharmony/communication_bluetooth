  /*
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
#include "bluetooth_log.h"
#include "bluetooth_opp_transfer_information.h"

namespace OHOS {
namespace Bluetooth {
BluetoothIOppTransferInformation::BluetoothIOppTransferInformation(const bluetooth::IOppTransferInformation &other)
{
    SetId(other.GetId());
    SetFileName(other.GetFileName());
    SetFilePath(other.GetFilePath());
    SetFileType(other.GetFileType());
    SetDeviceName(other.GetDeviceName());
    SetDeviceAddress(other.GetDeviceAddress());
    SetDirection(other.GetDirection());
    SetStatus(other.GetStatus());
    SetResult(other.GetResult());
    SetTimeStamp(other.GetTimeStamp());
    SetCurrentBytes(other.GetCurrentBytes());
    SetTotalBytes(other.GetTotalBytes());
    SetCurrentCount(other.GetCurrentCount());
    SetTotalCount(other.GetTotalCount());
}
bool BluetoothIOppTransferInformation::WriteToParcel(Parcel &parcel) const
{
    if (!parcel.WriteInt32(GetId())) {
        return false;
    }
    if (!parcel.WriteString(GetFileName())) {
        return false;
    }
    if (!parcel.WriteString(GetFilePath())) {
        return false;
    }
    if (!parcel.WriteString(GetFileType())) {
        return false;
    }
    if (!parcel.WriteString(GetDeviceName())) {
        return false;
    }
    if (!parcel.WriteString(GetDeviceAddress())) {
        return false;
    }
    if (!parcel.WriteInt32(GetDirection())) {
        return false;
    }
    if (!parcel.WriteInt32(GetStatus())) {
        return false;
    }
    if (!parcel.WriteInt32(GetResult())) {
        return false;
    }
    if (!parcel.WriteInt64(GetTimeStamp())) {
        return false;
    }
    if (!parcel.WriteInt64(GetCurrentBytes())) {
        return false;
    }
    if (!parcel.WriteInt64(GetTotalBytes())) {
        return false;
    }
    if (!parcel.WriteInt32(GetCurrentCount())) {
        return false;
    }
    if (!parcel.WriteInt32(GetTotalCount())) {
        return false;
    }
    return true;
}

bool BluetoothIOppTransferInformation::ReadFromParcel(Parcel &parcel)
{
    std::string str;
    int64_t val64 = 0;
    int val = 0;
    if (!parcel.ReadInt32(val)) {
        return false;
    }
    SetId(val);
    if (!parcel.ReadString(str)) {
        return false;
    }
    SetFileName(str);
    if (!parcel.ReadString(str)) {
        return false;
    }
    SetFilePath(str);
    if (!parcel.ReadString(str)) {
        return false;
    }
    SetFileType(str);
    if (!parcel.ReadString(str)) {
        return false;
    }
    SetDeviceName(str);
    if (!parcel.ReadString(str)) {
        return false;
    }
    SetDeviceAddress(str);
    if (!parcel.ReadInt32(val)) {
        return false;
    }
    SetDirection(val);
    if (!parcel.ReadInt32(val)) {
        return false;
    }
    SetStatus(val);
    if (!parcel.ReadInt32(val)) {
        return false;
    }
    SetResult(val);
    if (!parcel.ReadInt64(val64)) {
        return false;
    }
    SetTimeStamp(val64);
    if (!parcel.ReadInt64(val64)) {
        return false;
    }
    SetCurrentBytes(val64);

    if (!parcel.ReadInt64(val64)) {
        return false;
    }
    SetTotalBytes(val64);
    if (!parcel.ReadInt32(val)) {
        return false;
    }
    SetCurrentCount(val);
    if (!parcel.ReadInt32(val)) {
        return false;
    }
    SetTotalCount(val);

    return true;
}

bool BluetoothIOppTransferInformation::Marshalling(Parcel &parcel) const
{
    return WriteToParcel(parcel);
}

BluetoothIOppTransferInformation *BluetoothIOppTransferInformation::Unmarshalling(Parcel &parcel)
{
    BluetoothIOppTransferInformation *oppTransferInformation = new BluetoothIOppTransferInformation();
    if (oppTransferInformation != nullptr && !oppTransferInformation->ReadFromParcel(parcel)) {
        delete oppTransferInformation;
        oppTransferInformation = nullptr;
    }
    return oppTransferInformation;
}

BluetoothIOppTransferFileHolder::BluetoothIOppTransferFileHolder(const std::string &filePath,
    const int64_t &fileSize, const int32_t &fileFd)
{
    SetFilePath(filePath);
    SetFileSize(fileSize);
    SetFileFd(fileFd);
}

BluetoothIOppTransferFileHolder::BluetoothIOppTransferFileHolder(const bluetooth::IOppTransferFileHolder &other)
{
    SetFilePath(other.GetFilePath());
    SetFileSize(other.GetFileSize());
    SetFileFd(other.GetFileFd());
}

bool BluetoothIOppTransferFileHolder::WriteToParcel(Parcel &parcel) const
{
    if (!parcel.WriteString(GetFilePath())) {
        return false;
    }
    if (!parcel.WriteInt64(GetFileSize())) {
        return false;
    }
    if (!parcel.WriteInt32(GetFileFd())) {
        return false;
    }
    return true;
}

bool BluetoothIOppTransferFileHolder::ReadFromParcel(Parcel &parcel)
{
    std::string str;
    int64_t val64 = 0;
    int32_t val32 = 0;
    if (!parcel.ReadString(str)) {
        return false;
    }
    SetFilePath(str);
    if (!parcel.ReadInt64(val64)) {
        return false;
    }
    SetFileSize(val64);
    if (!parcel.ReadInt32(val32)) {
        return false;
    }
    SetFileFd(val32);
    return true;
}

bool BluetoothIOppTransferFileHolder::Marshalling(Parcel &parcel) const
{
    return WriteToParcel(parcel);
}

BluetoothIOppTransferFileHolder *BluetoothIOppTransferFileHolder::Unmarshalling(Parcel &parcel)
{
    BluetoothIOppTransferFileHolder *oppTransferFileHolder = new BluetoothIOppTransferFileHolder();
    if (oppTransferFileHolder != nullptr && !oppTransferFileHolder->ReadFromParcel(parcel)) {
        delete oppTransferFileHolder;
        oppTransferFileHolder = nullptr;
        return nullptr;
    }
    return oppTransferFileHolder;
}

}  // namespace Bluetooth
}  // namespace OHOS