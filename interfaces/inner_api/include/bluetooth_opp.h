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

/**
 * @addtogroup Bluetooth
 * @{
 *
 * @brief Defines a bluetooth system that provides basic bluetooth connection and profile functions,
 *        including A2DP, AVRCP, BLE, GATT, HFP, MAP, PBAP, and SPP, etc.
 *
 */

/**
 * @file bluetooth_opp.h
 *
 * @brief Declares OPP role framework functions, including basic and observer functions.
 *
 */
#ifndef BLUETOOTH_OPP_H
#define BLUETOOTH_OPP_H

#include <string>
#include <vector>
#include <memory>

#include "bluetooth_def.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_types.h"
#include "bluetooth_no_destructor.h"
namespace OHOS {
namespace Bluetooth {
/**
 * @brief Bluetooth Opp Transfer Information.
 *
 * @since 9
 */
class BLUETOOTH_API BluetoothOppTransferInformation {
public:
    /**
     * @brief A constructor used to create a <b>BluetoothOppTransferInformation</b> instance.
     *
     * @since 15
     */
    BluetoothOppTransferInformation();

    /**
     * @brief A destructor used to delete the <b>BluetoothOppTransferInformation</b> instance.
     *
     * @since 15
     */
    ~BluetoothOppTransferInformation();

    /**
     * @brief Get Id.
     *
     * @return Returns Id.
     * @since 15
     */
    int GetId() const;

    /**
     * @brief Get File Name.
     *
     * @return File Name.
     * @since 15
     */
    std::string GetFileName() const;

    /**
     * @brief Get File Path.
     *
     * @return Returns File Path.
     * @since 15
     */
    std::string GetFilePath() const;

    /**
     * @brief Get Mime Type.
     *
     * @since 15
     */
    std::string GetMimeType() const;

    /**
     * @brief Get Device Name.
     *
     * @return Returns Device Name.
     * @since 15
     */
    std::string GetDeviceName() const;

    /**
     * @brief Get Device Address.
     *
     * @return Returns Device Address.
     * @since 15
     */
    std::string GetDeviceAddress() const;

    /**
     * @brief Get Direction.
     *
     * @return Returns Direction.
     * @since 15
     */
    int GetDirection() const;

    /**
     * @brief Get Status.
     *
     * @return Returns Status.
     * @since 15
     */
    int GetStatus() const;

    /**
     * @brief Get Result.
     *
     * @return Returns Result.
     * @since 15
     */
    int GetResult() const;

    /**
     * @brief Get Time Stamp.
     *
     * @return TimeStamp.
     * @since 15
     */
    uint64_t GetTimeStamp() const;

    /**
     * @brief Get Current Bytes.
     *
     * @return Returns Current Bytes.
     * @since 15
     */
    uint64_t GetCurrentBytes() const;

    /**
     * @brief Get Total Bytes.
     *
     * @return Returns TotalBytes.
     * @since 15
     */
    uint64_t GetTotalBytes() const;

    /**
     * @brief Get CurrentCount.
     *
     * @return Returns CurrentCount.
     * @since 15
     */
    int GetCurrentCount() const;

    /**
     * @brief Get TotalCount.
     *
     * @return Returns TotalCount.
     * @since 15
     */
    int GetTotalCount() const;

    /**
     * @brief Set Id.
     *
     * @param interval Id.
     * @since 15
     */
    void SetId(int id);

    /**
     * @brief Set File Name.
     *
     * @param interval File Name.
     * @since 15
     */
    void SetFileName(std::string fileName);

    /**
     * @brief Set File Path.
     *
     * @param interval File Path.
     * @since 15
     */
    void SetFilePath(std::string filePath);

    /**
     * @brief Set Mime Type.
     *
     * @param interval Mime Type.
     * @since 15
     */
    void SetMimeType(std::string mimeType);

    /**
     * @brief Set Device Name.
     *
     * @param interval Device Name.
     * @since 15
     */
    void SetDeviceName(std::string deviceName);

    /**
     * @brief Set Device Address.
     *
     * @param interval Device Address.
     * @since 15
     */
    void SetDeviceAddress(std::string deviceAddress);

    /**
     * @brief Set Direction.
     *
     * @param interval Direction.
     * @since 15
     */
    void SetDirection(int direction);

    /**
     * @brief Set Status.
     *
     * @param interval Status.
     * @since 15
     */
    void SetStatus(int status);

    /**
     * @brief Set Result.
     *
     * @param interval Result.
     * @since 15
     */
    void SetResult(int result);

    /**
     * @brief Set Time Stamp.
     *
     * @param interval Time Stamp.
     * @since 15
     */
    void SetTimeStamp(uint64_t timeStamp);

    /**
     * @brief Set Current Bytes.
     *
     * @param interval Current Bytes.
     * @since 15
     */
    void SetCurrentBytes(uint64_t currentBytes);

    /**
     * @brief Set Total Bytes.
     *
     * @param interval Total Bytes.
     * @since 15
     */
    void SetTotalBytes(uint64_t totalBytes);

    /**
     * @brief Set CurrentCount.
     *
     * @param interval CurrentCount.
     * @since 15
     */
    void SetCurrentCount(int currentCount);

    /**
     * @brief Set TotalCount.
     *
     * @param interval TotalCount.
     * @since 15
     */
    void SetTotalCount(int totalCount);

private:
    int id_ = -1;
    std::string fileName_;
    std::string filePath_;
    std::string mimeType_;
    std::string deviceName_;
    std::string deviceAddress_;
    int direction_ = 0;
    int status_ = 0;
    int result_ = 0;
    uint64_t timeStamp_ = 0;
    uint64_t currentBytes_ = 0;
    uint64_t totalBytes_ = 0;
    int currentCount_ = 0;
    int totalCount_ = 0;
};

/**
 * @brief Bluetooth Opp FileHolder.
 *
 * @since 15
 */
class BLUETOOTH_API BluetoothOppFileHolder {
public:
    /**
     * @brief A destructor used to create the <b>BluetoothOppFileHolder</b> instance.
     *
     * @since 15
     */
    BluetoothOppFileHolder(const std::string &filePath, const int64_t &fileSize, const int32_t &fileFd);

    /**
     * @brief A destructor used to create the <b>BluetoothOppFileHolder</b> instance.
     *
     * @since 15
     */
    BluetoothOppFileHolder();

    /**
     * @brief A destructor used to delete the <b>BluetoothOppFileHolder</b> instance.
     *
     * @since 15
     */
    ~BluetoothOppFileHolder();

    /**
     * @brief Get File Path.
     *
     * @return filePath.
     * @since 15
     */
    std::string GetFilePath() const;

    /**
     * @brief Get File Size.
     *
     * @return fileSize.
     * @since 15
     */
    int64_t GetFileSize() const;

    /**
     * @brief Get File Fd.
     *
     * @return fileFd.
     * @since 15
     */
    int32_t GetFileFd() const;

    /**
     * @brief Set File Path.
     *
     * @return filePath.
     * @since 15
     */
    void SetFilePath(const std::string &filePath);

    /**
     * @brief Set File Fd.
     *
     * @return fileFd.
     * @since 15
     */
    void SetFileFd(const int32_t &fileFd);

    /**
     * @brief Set File Size.
     *
     * @return fileSize.
     * @since 15
     */
    void SetFileSize(const int64_t &fileSize);

private:
    std::string filePath_;
    int64_t fileSize_;
    int32_t fileFd_;
};

/**
 * @brief Class for Opp Host observer functions.
 *
 */
class OppObserver {
public:
    /**
     * @brief The observer function to notify receive incoming file changed.
     *
     * @param transferInformation transfer Information.
     */
    virtual void OnReceiveIncomingFileChanged(const BluetoothOppTransferInformation &transferInformation)
    {}

   /**
     * @brief The observer function to notify transfer state changed.
     *
     * @param transferInformation transfer Information.
     */
    virtual void OnTransferStateChanged(const BluetoothOppTransferInformation &transferInformation)
    {}

    /**
     * @brief Destroy the OppObserver object.
     *
     */
    virtual ~OppObserver()
    {}
};

/**
 * @brief Class for Opp API.
 *
 */
class BLUETOOTH_API Opp {
public:
    /**
     * @brief Get the instance of Opp object.
     *
     * @return Returns the pointer to the Opp instance.
     */
    static Opp *GetProfile();

    /**
     * @brief Send File.
     *
     * @return Return operate result to confirm whether the file is successfully sent.
     */
    int32_t SendFile(std::string device, std::vector<BluetoothOppFileHolder> fileHolders, bool& result);

    /**
     * @brief Set Incoming File Confirmation.
     *
     * @return Return operate result to confirm whether the setting is successful.
     */
    int32_t SetIncomingFileConfirmation(bool accept, int32_t fileFd);

    /**
     * @brief Get Current Transfer Information.
     *
     * @return Return operate result to getCurrentTransferInformation.
     */
    int32_t GetCurrentTransferInformation(BluetoothOppTransferInformation &transferInformation);

     /**
     * @brief Cancel Transfer.
     *
     * @return Return operate result to confirm whether the cancellation is set successfully.
     */
    int32_t CancelTransfer(bool& result);

    /**
     * @brief Set LastReceived FileUri.
     * @param Set LastReceived FileUri.
     *
     * @return Return operate result to SetLastReceivedFileUri.
     */
    int32_t SetLastReceivedFileUri(const std::string &uri);

    /**
     * @brief Get remote opp device list which are in the specified states.
     *
     * @param states List of remote device states.
     * @param result the list of devices
     * @return Returns operate result.
     */
    int32_t GetDevicesByStates(const std::vector<int32_t> &states, std::vector<BluetoothRemoteDevice> &result) const;

    /**
     * @brief Get the connection state of the specified remote opp device.
     *
     * @param device Remote device object.
     * @param result the connection state of the remote device
     * @return Returns operate result.
     */
    int32_t GetDeviceState(const BluetoothRemoteDevice &device, int32_t &result) const;

    /**
     * @brief Register Opp observer instance.
     *
     * @param observer Opp observer instance.
     */
    void RegisterObserver(std::shared_ptr<OppObserver> observer);

    /**
     * @brief Deregister Opp observer instance.
     *
     * @param observer Opp observer instance.
     */
    void DeregisterObserver(std::shared_ptr<OppObserver> observer);

    /**
     * @brief The external process calls the Opp profile interface before the Bluetooth process starts. At this
     * time, it needs to monitor the start of the Bluetooth process, and then call this interface to initialize the
     * Opp proflie.
     */
    void Init();

    /**
     * @brief Static Opp observer instance.
     *
     */
    static OppObserver *instance_;

private:
    Opp();
    ~Opp();
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(Opp);
    BLUETOOTH_DECLARE_IMPL();

#ifdef DTFUZZ_TEST
    friend class BluetoothNoDestructor<Opp>;
#endif
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // BLUETOOTH_OPP_H