/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include "../mce_mock/mce_mock.h"
#include "log.h"
#include "obex_mp_client.h"
#include "map_mce_types.h"
#include "map_mce_data_analyse.h"
#include "map_mce_instance_request.h"
#include "map_mce_service.h"
#include "map_mce_instance_stm.h"
#include "../obex/obex_mp_client.h"
#include "map_mce_instance_client.h"
#include "map_mce_device_ctrl.h"
#include "map_mce_service.h"
#include "map_mce_mns_server.h"

std::unique_ptr<bluetooth::ObexHeader> header0 = nullptr;
std::unique_ptr<bluetooth::ObexHeader> header1 = nullptr;

namespace bluetooth {
const char *str = "teststring";
const char *str2 = "teststring2";
const char *str3 = "teststring3";
const char *str4 = "teststring4";
const char *str5 = "teststring5";
const char *str6 = "teststring6";
const char *body =
    "<?xml version='1.0' encoding='utf-8' standalone='yes' ?>"
    "\n<MAP-msg-listing version=\"1.0\">"
    "\n    <msg handle=\"04000000000002D7\" subject=\"序号01的验证码605473，您...\" datetime=\"20201103T135114\" "
    "sender_name=\"95533\" sender_addressing=\"95533\" recipient_name=\"\" recipient_addressing=\"+8618388888888\" "
    "type=\"SMS_GSM\" size=\"63\" text=\"yes\" reception_status=\"complete\" attachment_size=\"0\" priority=\"no\" "
    "read=\"no\" sent=\"no\" protected=\"no\" />"
    "\n    <msg handle=\"04000000000002D6\" subject=\"尊敬的客户:感谢您参加中国移动&quot;...\" "
    "datetime=\"20201102T173851\" sender_name=\"10086\" sender_addressing=\"10086\" recipient_name=\"\" "
    "recipient_addressing=\"+8618388888888\" type=\"SMS_GSM\" size=\"149\" text=\"yes\" reception_status=\"complete\" "
    "attachment_size=\"0\" priority=\"no\" read=\"yes\" sent=\"no\" protected=\"no\" />"
    "\n</MAP-msg-listing>";
const char *endbody =
    "<?xml version='1.0' encoding='utf-8' standalone='yes' ?>"
    "\n<MAP-msg-listing version=\"1.0\">"
    "\n    <msg handle=\"04000000000002D7\" subject=\"序号01的验证码605473，您...\" datetime=\"20201103T135114\" "
    "sender_name=\"95533\" sender_addressing=\"95533\" recipient_name=\"\" recipient_addressing=\"+8618388888888\" "
    "type=\"SMS_GSM\" size=\"63\" text=\"yes\" reception_status=\"complete\" attachment_size=\"0\" priority=\"no\" "
    "read=\"no\" sent=\"no\" protected=\"no\" />"
    "\n    <msg handle=\"04000000000002D6\" subject=\"尊敬的客户:感谢您参加中国移动&quot;...\" "
    "datetime=\"20201102T173851\" sender_name=\"10086\" sender_addressing=\"10086\" recipient_name=\"\" "
    "recipient_addressing=\"+8618388888888\" type=\"SMS_GSM\" size=\"149\" text=\"yes\" reception_status=\"complete\" "
    "attachment_size=\"0\" priority=\"no\" read=\"yes\" sent=\"no\" protected=\"no\" />"
    "\n</MAP-msg-listing>";

ObexMpClient::ObexMpClient(
    const ObexClientConfig &config, ObexClientObserver &observer, utility::Dispatcher &dispatcher)
    : ObexClient(config, observer, dispatcher)
{
    header1 = ObexHeader::CreateResponse(ObexRspCode::NON_AUTH);
    header0 = ObexHeader::CreateResponse(ObexRspCode::SUCCESS);
    header0->AppendItemEndBody((uint8_t *)endbody, strlen(endbody));
    header0->AppendItemBody((uint8_t *)body, 4);

    // make writer object
    std::shared_ptr<ObexBodyObject> read = std::make_shared<ObexArrayBodyObject>();
    read->Write((const uint8_t *)endbody, sizeof(endbody));
    header0->SetExtendBodyObject(read);

    // Application Parameters:
    ObexTlvParamters params;

    TlvTriplet owneruci(MCE_OWNER_UCI, 10, (uint8_t *)str);
    params.AppendTlvtriplet(owneruci);

    TlvTriplet fractionDeliver(MCE_FRACTION_DELIVER, (uint8_t)1);
    params.AppendTlvtriplet(fractionDeliver);

    TlvTriplet presencevalability(MCE_PRESENCE_AVAILABILITY, (uint8_t)1);
    params.AppendTlvtriplet(presencevalability);

    TlvTriplet presencetext(MCE_PRESENCE_TEXT, 11, (uint8_t *)str2);
    params.AppendTlvtriplet(presencetext);

    TlvTriplet lastactivity(MCE_LAST_ACTIVITY, 11, (uint8_t *)str3);
    params.AppendTlvtriplet(lastactivity);

    TlvTriplet chatstate(MCE_CHAT_STATE, (uint8_t)1);
    params.AppendTlvtriplet(chatstate);

    TlvTriplet newmessage(MCE_NEW_MESSAGE, (uint8_t)1);
    params.AppendTlvtriplet(newmessage);

    TlvTriplet msetime(MCE_MSE_TIME, 11, (uint8_t *)str4);
    params.AppendTlvtriplet(msetime);

    TlvTriplet listingsize(MCE_LISTING_SIZE, (uint16_t)12);
    params.AppendTlvtriplet(listingsize);

    TlvTriplet databaseidentifier(MCE_DATABASE_IDENTIFIER, 11, (uint8_t *)str5);
    params.AppendTlvtriplet(databaseidentifier);

    TlvTriplet folserversioncounter(MCE_FOLDER_VERSION_COUNTER, 11, (uint8_t *)str6);
    params.AppendTlvtriplet(folserversioncounter);

    header0->AppendItemAppParams(params);
}

int ObexMpClient::Put(const ObexHeader &req)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);

    switch (obexPutGetCtrl) {
        case MCE_TEST_CTRL_ACTION_COMPLETE_OK:
            clientObserver_.OnActionCompleted(*this, *header0);
            break;
        case MCE_TEST_CTRL_ACTION_COMPLETE_NG:
            clientObserver_.OnActionCompleted(*this, *header1);
            break;
        default:
            break;
    }
    return BT_NO_ERROR;
}

int ObexMpClient::Get(const ObexHeader &req)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    switch (obexPutGetCtrl) {
        case MCE_TEST_CTRL_ACTION_COMPLETE_OK:
            clientObserver_.OnActionCompleted(*this, *header0);
            break;
        case MCE_TEST_CTRL_ACTION_COMPLETE_NG:
            clientObserver_.OnActionCompleted(*this, *header1);
            break;
        default:
            break;
    }
    return BT_NO_ERROR;
}

int ObexMpClient::Get(const ObexHeader &req, std::shared_ptr<ObexBodyObject> writer, int srmpCount)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    switch (obexPutGetCtrl) {
        case MCE_TEST_CTRL_ACTION_COMPLETE_OK:
            clientObserver_.OnActionCompleted(*this, *header0);
            break;
        case MCE_TEST_CTRL_ACTION_COMPLETE_NG:
            clientObserver_.OnActionCompleted(*this, *header1);
            break;
        default:
            break;
    }
    return BT_NO_ERROR;
}

int ObexMpClient::Put(const ObexHeader &req, std::shared_ptr<ObexBodyObject> reader)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    switch (obexPutGetCtrl) {
        case MCE_TEST_CTRL_ACTION_COMPLETE_OK:
            clientObserver_.OnActionCompleted(*this, *header0);
            break;
        case MCE_TEST_CTRL_ACTION_COMPLETE_NG:
            clientObserver_.OnActionCompleted(*this, *header1);
            break;
        default:
            break;
    }
    return BT_NO_ERROR;
}

int ObexMpClient::SetPath(uint8_t flag, const std::u16string &path)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    switch (obexPutGetCtrl) {
        case MCE_TEST_CTRL_ACTION_COMPLETE_OK:
            clientObserver_.OnActionCompleted(*this, *header0);
            break;
        case MCE_TEST_CTRL_ACTION_COMPLETE_NG:
            clientObserver_.OnActionCompleted(*this, *header1);
            break;
        default:
            break;
    }
    return BT_NO_ERROR;
}

int ObexMpClient::SetPath(const std::vector<std::u16string> &paths)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    switch (obexPutGetCtrl) {
        case MCE_TEST_CTRL_ACTION_COMPLETE_OK:
            clientObserver_.OnActionCompleted(*this, *header0);
            break;
        case MCE_TEST_CTRL_ACTION_COMPLETE_NG:
            clientObserver_.OnActionCompleted(*this, *header1);
            break;
        default:
            break;
    }
    return BT_NO_ERROR;
}

void ObexMpClient::PutDataAvailable(const ObexHeader &resp)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

void ObexMpClient::GetDataAvailable(const ObexHeader &resp)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

void ObexMpClient::SetPathDataAvailable(const ObexHeader &resp)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
}

void ObexMpClient::HandleTransportDataBusy(uint8_t isBusy)
{
    LOG_INFO("Call %s, isBusy %d", __PRETTY_FUNCTION__, isBusy);
}

void ObexMpClient::AbortDataAvailable(const ObexHeader &resp)
{
    LOG_INFO("Call %s", __PRETTY_FUNCTION__);
    isWaitingSendAbort_ = false;
    isAbortSended_ = false;
    this->clientSession_->FreeReceivedObject();
    this->clientSession_->FreeSendObject();
    ObexClient::AbortDataAvailable(resp);
}
}  // namespace bluetooth