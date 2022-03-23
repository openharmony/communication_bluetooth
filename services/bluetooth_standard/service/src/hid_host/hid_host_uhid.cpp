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

#include "hid_host_uhid.h"
#include "hid_host_service.h"

namespace bluetooth {
HidHostUhid::HidHostUhid(std::string address)
{
    address_ = address;
    pollThreadId_ = -1;
    fd_ = -1;
    keepPolling_ = false;
    readyForData_ = false;
    task_id_ = 0;
}

HidHostUhid::~HidHostUhid()
{
    if (keepPolling_) {
        ClosePollThread();
    }
}

int HidHostUhid::Open()
{
    pthread_t threadId;

    if (fd_ < 0) {
        LOG_DEBUG("[UHID]%{public}s():fd is null,creat fd", __FUNCTION__);
        fd_ = open(UHID_DEVICE_PATH, O_RDWR);
        if (fd_ < 0) {
            LOG_ERROR("[UHID]%{public}s():open %{public}s failed", __FUNCTION__, UHID_DEVICE_PATH);
            return E_ERR_OPEN;
        } else {
            LOG_DEBUG("[UHID]%{public}s():uhid fd = %{public}d", __FUNCTION__, fd_);
        }

        LOG_DEBUG("[UHID]%{public}s():recreate thread", __FUNCTION__);
        threadId = CreateThread(PollEventThread, this);

        if(threadId == static_cast<pthread_t>(E_ERR_CREATE_PTHREAD)) {
            LOG_DEBUG("[UHID]%{public}s():create thread failed", __FUNCTION__);
            return E_ERR_CREATE_THREAD;
        } else {
            pollThreadId_ = threadId;
            keepPolling_ = true;
            return E_NO_ERR;
        }
    }

    LOG_DEBUG("[UHID]%{public}s():uhid fd is not null,fd = %{public}d", __FUNCTION__, fd_);
    return E_NO_ERR;
}

int HidHostUhid::Close()
{
    ClosePollThread();
    return E_NO_ERR;
}

int HidHostUhid::SendData(uint8_t* pRpt, uint16_t len)
{
    LOG_DEBUG("[UHID]%{public}s():", __FUNCTION__);

    if (fd_ >= 0) {
        uint32_t polling_attempts = 0;
        while (!readyForData_ && polling_attempts++ < MAX_POLLING_ATTEMPTS) {
            usleep(POLLING_SLEEP_DURATION_US);
        }
    }
    // Send the HID data to the kernel.
    if ((fd_ >= 0) && readyForData_) {
        WritePackUhid(fd_, pRpt, len);
    } else {
        LOG_WARN("[UHID]%{public}s(): Error: fd = %{public}d, ready %{public}d, len = %{public}d",
            __FUNCTION__, fd_, readyForData_, len);
        return E_ERR_WRITE;
    }
    return E_NO_ERR;
}

int HidHostUhid::SendControlData(uint8_t* pRpt, uint16_t len)
{
    LOG_DEBUG("[UHID]%{public}s():", __FUNCTION__);

    if (fd_ >= 0) {
        uint32_t polling_attempts = 0;
        while (!readyForData_ && polling_attempts++ < MAX_POLLING_ATTEMPTS) {
            usleep(POLLING_SLEEP_DURATION_US);
        }
    }
    // Send the HID control data to the kernel.
    if ((fd_ >= 0) && readyForData_) {
        if (task_type_ == HID_HOST_DATA_TYPE_GET_REPORT) {
            SendGetReportReplyUhid(fd_, task_id_, E_NO_ERR, pRpt, len);
        } else if (task_type_ == HID_HOST_DATA_TYPE_SET_REPORT) {
            SendSetReportReplyUhid(fd_, task_id_, E_NO_ERR);
        } else {
            LOG_ERROR("[UHID]%{public}s():Unknow task!", __FUNCTION__);
        }
        task_id_ = 0;
        task_type_ = -1;
    } else {
        LOG_WARN("[UHID]%{public}s(): Error: fd = %{public}d, ready %{public}d, len = %{public}d",
            __FUNCTION__, fd_, readyForData_, len);
        return E_ERR_WRITE;
    }
    return E_NO_ERR;
}

int HidHostUhid::SendHandshake(uint16_t err)
{
    LOG_DEBUG("[UHID]%{public}s():", __FUNCTION__);

    if (fd_ >= 0) {
        uint32_t polling_attempts = 0;
        while (!readyForData_ && polling_attempts++ < MAX_POLLING_ATTEMPTS) {
            usleep(POLLING_SLEEP_DURATION_US);
        }
    }
    // Send the HID handshake to the kernel.
    if ((fd_ >= 0) && readyForData_) {
        if (task_type_ == HID_HOST_DATA_TYPE_GET_REPORT) {
            SendGetReportReplyUhid(fd_, task_id_, err, nullptr, 0);
        } else if (task_type_ == HID_HOST_DATA_TYPE_SET_REPORT) {
            SendSetReportReplyUhid(fd_, task_id_, err);
        } else {
            LOG_ERROR("[UHID]%{public}s():Unknow task!", __FUNCTION__);
        }
        task_id_ = 0;
        task_type_ = -1;
    } else {
        LOG_WARN("[UHID]%{public}s(): Error: fd = %{public}d, ready %{public}d",
            __FUNCTION__, fd_, readyForData_);
        return E_ERR_WRITE;
    }
    return E_NO_ERR;
}

int HidHostUhid::SendGetReportReplyUhid(int fd, int id, uint16_t err, uint8_t* rpt, uint16_t len)
{
    struct uhid_event ev;
    memset_s(&ev, sizeof(ev), 0, sizeof(ev));
    ev.type = UHID_FEATURE_ANSWER;
    ev.u.feature_answer.id = id;
    ev.u.feature_answer.err = err;
    ev.u.feature_answer.size = len;
    if (len > sizeof(ev.u.feature_answer.data)) {
        LOG_WARN("[UHID]%{public}s(): Report size greater than allowed size", __FUNCTION__);
        return E_LEN_VAL;
    }
    if (memcpy_s(ev.u.feature_answer.data, sizeof(ev.u.feature_answer.data), rpt, len) != EOK) {
        LOG_WARN("[UHID]%{public}s(): memcpy_s fail", __FUNCTION__);
        return E_ERR_NULL;
    }
    return WriteUhid(fd, &ev);
}

int HidHostUhid::SendSetReportReplyUhid(int fd, int id, uint16_t err)
{
    struct uhid_event ev;
    memset_s(&ev, sizeof(ev), 0, sizeof(ev));
    ev.type = UHID_SET_REPORT_REPLY;
    ev.u.set_report_reply.id = id;
    ev.u.set_report_reply.err = err;
    return WriteUhid(fd, &ev);
}

int HidHostUhid::SendHidInfo(const char* devName, PnpInformation pnpInf, HidInformation hidInf)
{
    struct uhid_event ev;
    int ret ;

    if (fd_ < 0) {
        LOG_WARN("[UHID]%{public}s(): Error: fd = %{public}d, dscp_len = %{public}d", __FUNCTION__, fd_,
            hidInf.descLength);
       return  E_ERR_OPEN;
    }
    // Create and send hid descriptor to kernel
    memset_s(&ev, sizeof(ev), 0, sizeof(ev));
    ev.type = UHID_CREATE;
    BtAddr btAddr;
    RawAddress(address_).ConvertToUint8(btAddr.addr);
    if (strncpy_s((char*)ev.u.create.name, sizeof(ev.u.create.name), devName, strlen(devName)) != EOK) {
        LOG_ERROR("[UHID]%{public}s(): strncpy_s name fail", __FUNCTION__);
        return E_ERR_NULL;
    }
    int uniqLength = snprintf_s((char*)ev.u.create.uniq, sizeof(ev.u.create.uniq), sizeof(btAddr.addr),
        "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X", btAddr.addr[HID_HOST_NUMBER_5], btAddr.addr[HID_HOST_NUMBER_4],
        btAddr.addr[HID_HOST_NUMBER_3], btAddr.addr[HID_HOST_NUMBER_2], btAddr.addr[HID_HOST_NUMBER_1],
        btAddr.addr[HID_HOST_NUMBER_0]);
    if (uniqLength < 0) {
        LOG_ERROR("[UHID]%{public}s(): snprintf length error, uniqLength = %{public}d", __FUNCTION__, uniqLength);
    }
    ev.u.create.rd_size = hidInf.descLength;
    ev.u.create.rd_data = hidInf.descInfo;
    ev.u.create.bus = BUS_BLUETOOTH;
    ev.u.create.vendor = pnpInf.vendorId;
    ev.u.create.product = pnpInf.productId;
    ev.u.create.version = pnpInf.version;
    ev.u.create.country = hidInf.ctryCode;
    ret = WriteUhid(fd_, &ev);

    LOG_DEBUG(
        "[UHID]%{public}s(): wrote descriptor to fd = %{public}d, dscp_len = %{public}d,result = %{public}d",
        __FUNCTION__, fd_, hidInf.descLength, ret);

    if (ret) {
        LOG_ERROR("[UHID]%{public}s(): Error: failed to send DSCP, result = %{public}d", __FUNCTION__, ret);
        /* The HID report descriptor is corrupted. Close the driver. */
        close(fd_);
        fd_ = -1;
        return E_ERR_WRITE;
    }

    return E_NO_ERR;
}

int HidHostUhid::Destroy()
{
    if (fd_ >= 0) {
        struct uhid_event ev;
        memset_s(&ev, sizeof(ev), 0, sizeof(ev));
        ev.type = UHID_DESTROY;

        WriteUhid(fd_, &ev);
        LOG_DEBUG("[UHID]%{public}s(): Closing fd=%{public}d", __FUNCTION__, fd_);
        close(fd_);
        fd_ = -1;
    }
    return E_NO_ERR;
}

int HidHostUhid::WriteUhid(int fd, const struct uhid_event* ev)
{
    ssize_t ret;
    do {
    } while ((ret = write(fd, ev, sizeof(*ev))) == -1 && errno == EINTR);
    if (ret < 0) {
        int rtn = errno;
        LOG_ERROR("[UHID]%{public}s(): Cannot write to uhid:%{public}s", __FUNCTION__, strerror(errno));
        return rtn;
    } else if (ret != static_cast<ssize_t>(sizeof(*ev))) {
        LOG_ERROR("[UHID]%{public}s(): Wrong size written to uhid: %{public}zd != %{public}zu", __FUNCTION__,
            ret, sizeof(*ev));
        return EFAULT;
    }
    return E_NO_ERR;
}

int HidHostUhid::WritePackUhid(int fd, uint8_t* rpt, uint16_t len)
{
    struct uhid_event ev;
    memset_s(&ev, sizeof(ev), 0, sizeof(ev));
    ev.type = UHID_INPUT;
    ev.u.input.size = len;
    if (len > sizeof(ev.u.input.data)) {
        LOG_WARN("[UHID]%{public}s(): Report size greater than allowed size", __FUNCTION__);
        return E_LEN_VAL;
    }
    if (memcpy_s(ev.u.input.data, sizeof(ev.u.input.data), rpt, len) != EOK) {
        LOG_WARN("[UHID]%{public}s(): memcpy_s fail", __FUNCTION__);
        return E_LEN_VAL;
    }
    return WriteUhid(fd, &ev);
}

int HidHostUhid::ClosePollThread()
{
    LOG_DEBUG("[UHID]%{public}s():", __FUNCTION__);
    keepPolling_ = false;
    readyForData_ = false;
    if (pollThreadId_ > 0) {
        pthread_join(pollThreadId_, nullptr);
    }
    pollThreadId_ = -1;
    return E_NO_ERR;
}

pthread_t HidHostUhid::CreateThread(void* (*startRoutine)(void*), void* arg)
{
    LOG_DEBUG("[UHID]%{public}s():create_thread: entered", __FUNCTION__);
    pthread_attr_t threadAttr;

    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);
    pthread_t threadId = static_cast<pthread_t>(-1);
    if (pthread_create(&threadId, &threadAttr, startRoutine, arg) != 0) {
        LOG_ERROR("[UHID]%{public}s():pthread_create : %{public}s", __FUNCTION__, strerror(errno));
        return static_cast<pthread_t>(E_ERR_CREATE_PTHREAD);
    }
    LOG_DEBUG("[UHID]%{public}s():%{public}lu: threadcreated successfully", __FUNCTION__, threadId);
    return threadId;
}

void* HidHostUhid::PollEventThread(void* arg)
{
    HidHostUhid *uhid = (HidHostUhid*)arg;
    if (uhid != nullptr) {
        uhid->PollEventThread_();
    } else {
        LOG_ERROR("[UHID]%{public}s():Thread creat fail,uhid is null", __FUNCTION__);
    }
    return nullptr;
}

void HidHostUhid::PollEventThread_()
{
    LOG_DEBUG("[UHID]%{public}s(): Thread%d  fd%{public}d execute", __FUNCTION__, (int)pollThreadId_, fd_);
    struct pollfd pfds[1];

    pfds[0].fd = fd_;
    pfds[0].events = POLLIN;

    // Set the uhid fd as non-blocking to ensure we never block the BTU thread
    SetUhidNonBlocking(fd_);

    while (keepPolling_) {
        int ret;
        do {
        } while ((ret = poll(pfds, 1, POLL_TIMEOUT)) == -1 && errno == EINTR);
        if (ret < 0) {
            LOG_ERROR("[UHID]%{public}s(): Cannot poll for fds: %{public}s", __FUNCTION__,
                strerror(errno));
            break;
        }
        if (pfds[0].revents & POLLIN) {
            LOG_DEBUG("[UHID]%{public}s(): POLLIN", __FUNCTION__);
            ret = ReadUhidEvent();
            if (ret != 0) {
                break;
            }
        }
    }
    LOG_DEBUG("[UHID]%{public}s(): exit", __FUNCTION__);

    pollThreadId_ = -1;
}

void HidHostUhid::SetUhidNonBlocking(int fd)
{
    int opts = fcntl(fd, F_GETFL);
    if (opts < 0)
        LOG_DEBUG("[UHID]%{public}s(): Getting flags failed (%{public}s)", __FUNCTION__,
            strerror(errno));

    if (fcntl(fd, F_SETFL, opts | O_NONBLOCK) < 0)
        LOG_DEBUG("[UHID]%{public}s(): Setting non-blocking flag failed (%{public}s)", __FUNCTION__,
            strerror(errno));
}

int HidHostUhid::ReadUhidEvent()
{
    struct uhid_event ev;
    ssize_t ret;

    memset_s(&ev, sizeof(ev), 0, sizeof(ev));

    do {
    } while ((ret = read(fd_, &ev, sizeof(ev))) == -1 && errno == EINTR);

    if (ret <= 0) {
        LOG_ERROR("[UHID]%{public}s():read err", __FUNCTION__);
        return -E_RD_INVDAT;
    }
    LOG_DEBUG("[UHID]%{public}s():ev.type:%{public}d", __FUNCTION__, ev.type);

    switch (ev.type) {
        case UHID_START:
            LOG_DEBUG("[UHID]%{public}s():UHID_START from uhid-dev", __FUNCTION__);
            readyForData_ = true;
            break;
        case UHID_STOP:
            LOG_DEBUG("[UHID]%{public}s():UHID_STOP from uhid-dev", __FUNCTION__);
            readyForData_ = false;
            break;
        case UHID_OPEN:
            LOG_DEBUG("[UHID]%{public}s():UHID_OPEN from uhid-dev", __FUNCTION__);
            readyForData_ = true;
            break;
        case UHID_CLOSE:
            LOG_DEBUG("[UHID]%{public}s():UHID_CLOSE from uhid-dev", __FUNCTION__);
            readyForData_ = false;
            break;
        case UHID_OUTPUT:
            ReadUhidOutPut(ev);
            break;
        case UHID_OUTPUT_EV:
            LOG_DEBUG("[UHID]%{public}s():UHID_OUTPUT_EV from uhid-dev", __FUNCTION__);
            break;
        case UHID_FEATURE:
            ReadUhidFeature(ev);
            break;
        case UHID_SET_REPORT:
            ReadUhidSetReport(ev);
            break;
        default:
            LOG_DEBUG("[UHID]%{public}s():Invalid event from uhid-dev: %{public}u", __FUNCTION__, ev.type);
    }

    return E_NO_ERR;
}

void HidHostUhid::ReadUhidOutPut(uhid_event ev)
{
    if (ev.u.output.rtype == UHID_FEATURE_REPORT) {
        HidHostService::GetService()->HidHostSetReport(address_, HID_HOST_FEATURE_REPORT, ev.u.output.size,
            ev.u.output.data);
    } else if (ev.u.output.rtype == UHID_OUTPUT_REPORT) {
        HidHostService::GetService()->HidHostSetReport(address_, HID_HOST_OUTPUT_REPORT, ev.u.output.size,
            ev.u.output.data);
    } else {
        HidHostService::GetService()->HidHostSetReport(address_, HID_HOST_INPUT_REPORT, ev.u.output.size,
            ev.u.output.data);
    }
}

void HidHostUhid::ReadUhidFeature(uhid_event ev)
{
    LOG_DEBUG("[UHID]%{public}s():UHID_FEATURE from uhid-dev id=%{public}d,rnum=%{public}d,rtype=%{public}d",
        __FUNCTION__, ev.u.feature.id, ev.u.feature.rnum, ev.u.feature.rtype);
    if (ev.u.feature.rtype == UHID_FEATURE_REPORT) {
        HidHostService::GetService()->HidHostGetReport(address_, ev.u.feature.rnum, 0,
            HID_HOST_FEATURE_REPORT);
    } else if (ev.u.feature.rtype == UHID_OUTPUT_REPORT) {
        HidHostService::GetService()->HidHostGetReport(address_, ev.u.feature.rnum, 0,
            HID_HOST_OUTPUT_REPORT);
    } else {
        HidHostService::GetService()->HidHostGetReport(address_, ev.u.feature.rnum, 0,
            HID_HOST_INPUT_REPORT);
    }
    task_id_ = ev.u.feature.id;
    task_type_ = HID_HOST_DATA_TYPE_GET_REPORT;
}

void HidHostUhid::ReadUhidSetReport(uhid_event ev)
{
    LOG_DEBUG("[UHID]%{public}s():UHID_SET_REPORT from uhid-dev id=%{public}d,rnum=%{public}d,rtype=%{public}d,size=%{public}d",
        __FUNCTION__, ev.u.set_report.id, ev.u.set_report.rnum, ev.u.set_report.rtype, ev.u.set_report.size);
    if (ev.u.set_report.rtype == UHID_FEATURE_REPORT) {
        HidHostService::GetService()->HidHostSetReport(address_, HID_HOST_FEATURE_REPORT, ev.u.set_report.size,
            ev.u.set_report.data);
    } else if (ev.u.set_report.rtype == UHID_OUTPUT_REPORT) {
        HidHostService::GetService()->HidHostSetReport(address_, HID_HOST_OUTPUT_REPORT, ev.u.set_report.size,
            ev.u.set_report.data);
    } else {
        HidHostService::GetService()->HidHostSetReport(address_, HID_HOST_INPUT_REPORT, ev.u.set_report.size,
            ev.u.set_report.data);
    }
    task_id_ = ev.u.feature.id;
    task_type_ = HID_HOST_DATA_TYPE_SET_REPORT;
}

}  // namespace bluetooth
