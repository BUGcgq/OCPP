/*
 * @Author: LIYAOHAN liyaohan@increase.com
 * @Date: 2023-04-19 16:46:51
 * @LastEditors: LIYAOHAN 1791002655@qq.com
 * @LastEditTime: 2023-05-09 13:58:07
 * @FilePath: \OCPP\ocpp_chargePoint.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <netdb.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <json-c/json.h>
#include <time.h>

#include "ocpp_chargePoint.h"
#include "ocpp_configurationKey.h"
#include "ocpp_auxiliaryTool.h"
#include "ocpp_config.h"
#include "ocpp_localAuthorization.h"
#include "ocpp_list.h"
#include "ocpp_offline.h"
#include "ocpp_firmwareUpdata.h"
#include "ocpp_diagnostics.h"
#include "ocpp_transaction.h"

#include "ocpp_log.h"

ocpp_chargePoint_t *ocpp_chargePoint = NULL;

/**
 * @description:
 * @param {char} *str
 * @param {int} len
 * @return {*}
 */
bool ocpp_chargePoint_getDefault(char *const str, int len)
{
    return false;
}

bool ocpp_chargePoint_pushDefault(char *idTag, int connector)
{
    return false;
}

float ocpp_chargePoint_getDefaultMeterValues(int connector)
{

    return 0.0;
}

void ocpp_chargePoint_setDefault(int connector)
{

    return;
}

// 用户点击启动充电
void ocpp_chargePoint_userPushStartButton(char *idTag, int connector)
{
    ocpp_chargePoint_transaction_t *transaction = ocpp_chargePoint->transaction_obj[connector];
    memset(transaction, 0, sizeof(ocpp_chargePoint_transaction_t));
    transaction->isStart = true;
    memset(transaction->startIdTag, 0, OCPP_AUTHORIZATION_IDTAG_LEN);

    strncpy(transaction->startIdTag, idTag, OCPP_AUTHORIZATION_IDTAG_LEN);
}

// 用户点击停止充电
void ocpp_chargePoint_userPushStopButton(char *idTag, int connector, enum OCPP_PACKAGE_STOP_REASON_E reason)
{
    ocpp_chargePoint_transaction_t *transaction = ocpp_chargePoint->transaction_obj[connector];
    transaction->isStop = true;
    transaction->reason = reason;

    transaction->stopIdTagIsUse = 0;
    memset(transaction->stopIdTag, 0, OCPP_AUTHORIZATION_IDTAG_LEN);
    if (idTag != NULL)
    {
        transaction->stopIdTagIsUse = 1;
        strncpy(transaction->stopIdTag, idTag, OCPP_AUTHORIZATION_IDTAG_LEN);
        OCPP_LOG_DEBUG("stopIdTag = %s", transaction->stopIdTag);
    }
}

/**
 * @description:
 * @param {int} connector
 * @param {enum OCPP_PACKAGE_CHARGEPOINT_ERRORCODE} errorCode
 * @param {enum OCPP_PACKAGE_CHARGEPOINT_STATUS} status
 * @return {*}
 */
void setConnectorStatus(int connector, enum OCPP_PACKAGE_CHARGEPOINT_ERRORCODE_E errorCode, enum OCPP_PACKAGE_CHARGEPOINT_STATUS_E status)
{

    if (ocpp_chargePoint == NULL)
        return;
    if (ocpp_chargePoint->connector[connector] == NULL)
        return;
    if (connector > ocpp_chargePoint->numberOfConnector)
        return;

    ocpp_chargePoint->connector[connector]->errorCode = errorCode;
    ocpp_chargePoint->connector[connector]->status = status;

    ocpp_chargePoint->connector[connector]->timestampIsUse = 1;
    char *timestamp = ocpp_AuxiliaryTool_GetCurrentTime();

    strncpy(ocpp_chargePoint->connector[connector]->timestamp, timestamp, 32);
    // DEBUG_ERR("time = %s",ocpp_chargePoint->connector[connector]->timestamp);
    free(timestamp);

    return;
}

/**
 * @description:
 * @param {int} connector
 * @param {char *} info
 * @param {char *} vendorID
 * @param {char *} vendorErrCode
 * @return {*}
 */
void setConnectorErrInfo(int connector, const char *info, const char *vendorID, const char *vendorErrCode)
{
    if (ocpp_chargePoint == NULL)
        return;
    if (ocpp_chargePoint->connector[connector] == NULL)
        return;
    if (connector > ocpp_chargePoint->numberOfConnector)
        return;

    ocpp_chargePoint->connector[connector]->infoIsUse = 0;
    ocpp_chargePoint->connector[connector]->vendorIdIsUse = 0;
    ocpp_chargePoint->connector[connector]->vendorErrorCodeIsUse = 0;

    if (info != NULL)
    {
        ocpp_chargePoint->connector[connector]->infoIsUse = 1;
        strncpy(ocpp_chargePoint->connector[connector]->info, info, 50);
    }

    if (vendorID != NULL)
    {
        ocpp_chargePoint->connector[connector]->vendorIdIsUse = 1;
        strncpy(ocpp_chargePoint->connector[connector]->vendorId, vendorID, 255);
    }

    if (vendorErrCode != NULL)
    {
        ocpp_chargePoint->connector[connector]->vendorErrorCodeIsUse = 1;
        strncpy(ocpp_chargePoint->connector[connector]->vendorErrorCode, vendorErrCode, 50);
    }

    return;
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/**
 * @description: 获取连接状态
 * @return {*}
 */
bool ocpp_chargePoint_getConnectStatus()
{
    if (ocpp_chargePoint == NULL)
        return false;

    return ocpp_chargePoint->connect.isConnect;
}

/**
 * @description: 设置重连
 * @return {*}
 */
void ocpp_chargePoint_setReConnect()
{
    if (ocpp_chargePoint == NULL)
        return;
    ocpp_chargePoint->connect.interrupted = true;
}

/**
 * @description:
 * @param {char} *str
 * @return {*}
 */
void ocpp_chargePoint_send(char *unique, char *message, enum OCPP_PACKAGE action)
{
    //    OCPP_LOG_DEBUG("发送数据");

    // if online send to the server
    if (ocpp_chargePoint->connect.isConnect)
    {
        ocpp_list_SendItem_t *item = (ocpp_list_SendItem_t *)calloc(1, sizeof(ocpp_list_SendItem_t));
        item->next = NULL;
        item->package = action;
        item->Unique = unique;
        item->message = message;

        ocpp_list_insertSendItem(item);
        // DEBUG_MSG("[%s] - [action %d] = %s \t  [message] = %s",item->Unique , item->package, ocpp_package_text[item->package], item->message);
    } // if offline send to the dataBase
    else
    {

        if (action == OCPP_PACKAGE_STARTTRANSACTION || action == OCPP_PACKAGE_METERVALUES || action == OCPP_PACKAGE_STOPTRANSACTION)
        {
            ocpp_offline_offlineData_item_t item;
            item.Action = action;
            item.unique = unique;
            item.message = message;
            ocpp_offline_send(&item);
        }

        free(unique);
        free(message);
    }
}

/**
 * @description:
 * @param:
 * @return:
 */
char *ocpp_chargePoint_sendAuthorize_req(const char *const idTag)
{

    ocpp_package_Authorize_req_t authorize;
    memset(&authorize, 0, sizeof(authorize));
    strncpy(authorize.idTag, idTag, 20);

    char *unique = ocpp_AuxiliaryTool_GenerateUUID();
    char *message = ocpp_package_prepare_Authorize_Request(unique, &authorize);
    ocpp_chargePoint_send(unique, message, OCPP_PACKAGE_AUTHORIZE);
    return unique;
}

/**
 * @description:
 * @param:
 * @return:
 */
static char *ocpp_chargePoint_sendBootNotification_req()
{

    ocpp_package_BootNotification_req_t bootNotification;
    memset(&bootNotification, 0, sizeof(bootNotification));

    bootNotification.chargeBoxSerialNumberIsUse = 1;
    if (ocpp_chargePoint->getChargeBoxSerialNumber(bootNotification.chargeBoxSerialNumber, 25) == false)
        memset(bootNotification.chargeBoxSerialNumber, 0, 25);

    if (ocpp_chargePoint->getChargePointModel(bootNotification.chargePointModel, 20) == false)
        memset(bootNotification.chargePointModel, 0, 20);

    bootNotification.chargePointSerialNumberIsUse = 1;
    if (ocpp_chargePoint->getChargePointSerialNumber(bootNotification.chargePointSerialNumber, 25) == false)
        memset(bootNotification.chargePointSerialNumber, 0, 25);

    if (ocpp_chargePoint->getChargePointVendor(bootNotification.chargePointVendor, 20) == false)
        memset(bootNotification.chargePointVendor, 0, 20);

    bootNotification.firmwareVersionIsUse = 1;
    if (ocpp_chargePoint->getFirmwareVersion(bootNotification.firmwareVersion, 50) == false)
        memset(bootNotification.firmwareVersion, 0, 50);

    bootNotification.iccidIsUse = 1;
    if (ocpp_chargePoint->getIccid(bootNotification.iccid, 20) == false)
        memset(bootNotification.iccid, 0, 20);

    bootNotification.imsiIsUse = 1;
    if (ocpp_chargePoint->getImsi(bootNotification.imsi, 20) == false)
        memset(bootNotification.imsi, 0, 20);

    bootNotification.meterSerialNumberIsUse = 1;
    if (ocpp_chargePoint->getMeterSerialNumber(bootNotification.meterSerialNumber, 25) == false)
        memset(bootNotification.meterSerialNumber, 0, 25);

    bootNotification.meterTypeIsUse = 1;
    if (ocpp_chargePoint->getMeterType(bootNotification.meterType, 25) == false)
        memset(bootNotification.meterType, 0, 25);

    char *uniqueId = ocpp_AuxiliaryTool_GenerateUUID();
    char *message = ocpp_package_prepare_BootNotification_Request(uniqueId, &bootNotification);
    ocpp_chargePoint_send(uniqueId, message, OCPP_PACKAGE_BOOT_NOTIFICATION);

    return uniqueId;
}

/**
 * @description:
 * @param:
 * @return:
 */
static char *ocpp_chargePoint_sendHeartbeat_Req()
{

    ocpp_package_Heartbeat_req_t heartbeat;
    memset(&heartbeat, 0, sizeof(heartbeat));

    char *uniqueId = ocpp_AuxiliaryTool_GenerateUUID();
    char *message = ocpp_package_prepare_Heartbeat_Request(uniqueId, &heartbeat);
    ocpp_chargePoint_send(uniqueId, message, OCPP_PACKAGE_HEARTBEAT);

    return uniqueId;
}

/**
 * @description:
 * @param:
 * @return:
 */
char *ocpp_chargePoint_sendStartTransaction(int connector, const char *idTag, int reservationId)
{

    ocpp_package_StartTransaction_req_t startTransaction;
    memset(&startTransaction, 0, sizeof(startTransaction));

    char *uniqueId = ocpp_AuxiliaryTool_GenerateUUID();

    startTransaction.connectorId = connector;
    strncpy(startTransaction.idTag, idTag, 20);
    startTransaction.meterStart = (int)ocpp_chargePoint->getCurrentMeterValues(connector - 1);

    startTransaction.reservationIdIsUse = 0;

    if (reservationId > 0)
    {
        startTransaction.reservationIdIsUse = 1;
        startTransaction.reservationId = reservationId;
    }

    char *timestamp = ocpp_AuxiliaryTool_GetCurrentTime();

    strncpy(startTransaction.timestamp, timestamp, 32);

    free(timestamp);

    char *message = ocpp_package_prepare_StartTransaction_Request(uniqueId, &startTransaction);

    ocpp_chargePoint_send(uniqueId, message, OCPP_PACKAGE_STARTTRANSACTION);

    return uniqueId;
}

/**
 * @description:
 * @param {ocpp_chargePoint_t} *ocpp_chargePoint
 * @param {int} connector
 * @param {int} transactionId
 * @return {*}
 */
char *ocpp_chargePoint_sendMeterValues(int connector, int transactionId)
{

    char *uniqueId = ocpp_AuxiliaryTool_GenerateUUID();
    char *MeterValuesSampledData = (char *)calloc(1, 512);
    int MeterValuesSampledDataMaxLength = 0;

    ocpp_ConfigurationKey_getValue("MeterValuesSampledData", (void *)MeterValuesSampledData);
    ocpp_ConfigurationKey_getValue("MeterValuesSampledDataMaxLength", (void *)&MeterValuesSampledDataMaxLength);

    if (MeterValuesSampledData != NULL)
    {

        int MeterValuesSampledDataCnt = ocpp_AuxiliaryTool_charCounter(MeterValuesSampledData, ':') + 1;
        int cnt = 0;

        cnt = MeterValuesSampledDataCnt < MeterValuesSampledDataMaxLength ? MeterValuesSampledDataCnt : MeterValuesSampledDataMaxLength;

        ocpp_package_MeterValues_M_SampledValue_t **sampledValue = (ocpp_package_MeterValues_M_SampledValue_t **)calloc(cnt, sizeof(ocpp_package_MeterValues_M_SampledValue_t *));

        char **measurands;
        measurands = ocpp_AuxiliaryTool_str_split(MeterValuesSampledData, ':');
        char **freeMeasurands = measurands;

        int i = 0;
        for (i = 0; i < cnt && *measurands; i++, *(measurands++))
        {
            sampledValue[i] = calloc(1, sizeof(ocpp_package_MeterValues_M_SampledValue_t));

            sampledValue[i]->contextIsUse = 1;
            sampledValue[i]->formatIsUse = 1;
            sampledValue[i]->locationIsUse = 1;
            sampledValue[i]->measurandIsUse = 1;
            sampledValue[i]->phaseIsUse = 0;
            sampledValue[i]->unitIsUse = 1;
            memset(sampledValue[i]->value, 0, 16);

            if (strcmp(*measurands, "Voltage") == 0)
            {
                sampledValue[i]->context = OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK;
                sampledValue[i]->format = OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW;
                sampledValue[i]->location = OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET;
                sampledValue[i]->measurand = OCPP_PACKAGE_METERVALUES_MS_MEASURAND_V;
                sampledValue[i]->unit = OCPP_PACKAGE_METERVALUES_MS_UNIT_V;
                snprintf(sampledValue[i]->value, 16, "%0.2f", ocpp_chargePoint->getVoltage(connector));
            }
            else if (strcmp(*measurands, "Temperature") == 0)
            {
                sampledValue[i]->context = OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK;
                sampledValue[i]->format = OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW;
                sampledValue[i]->location = OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET;
                sampledValue[i]->measurand = OCPP_PACKAGE_METERVALUES_MS_MEASURAND_T;
                sampledValue[i]->unit = OCPP_PACKAGE_METERVALUES_MS_UNIT_CELSIUS;
                snprintf(sampledValue[i]->value, 16, "%0.2f", ocpp_chargePoint->getTemperature(connector));
            }
            else if (strcmp(*measurands, "SoC") == 0)
            {
                sampledValue[i]->context = OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK;
                sampledValue[i]->format = OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW;
                sampledValue[i]->location = OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET;
                sampledValue[i]->measurand = OCPP_PACKAGE_METERVALUES_MS_MEASURAND_S;
                sampledValue[i]->unit = OCPP_PACKAGE_METERVALUES_MS_UNIT_PERCENT;
                snprintf(sampledValue[i]->value, 16, "%0.2f", ocpp_chargePoint->getSOC(connector));
            }
            else if (strcmp(*measurands, "RPM") == 0)
            {
                sampledValue[i]->context = OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK;
                sampledValue[i]->format = OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW;
                sampledValue[i]->location = OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET;
                sampledValue[i]->measurand = OCPP_PACKAGE_METERVALUES_MS_MEASURAND_R;
                sampledValue[i]->unitIsUse = 0;
                snprintf(sampledValue[i]->value, 16, "%0.2f", ocpp_chargePoint->getRPM(connector));
            }
            else if (strcmp(*measurands, "Power.Reactive.Import") == 0)
            {
                sampledValue[i]->context = OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK;
                sampledValue[i]->format = OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW;
                sampledValue[i]->location = OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET;
                sampledValue[i]->measurand = OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PRI;
                sampledValue[i]->unit = OCPP_PACKAGE_METERVALUES_MS_UNIT_KVAR;
                snprintf(sampledValue[i]->value, 16, "%0.2f", ocpp_chargePoint->getPowerActiveImport(connector));
            }
            else if (strcmp(*measurands, "Power.Reactive.Export") == 0)
            {
                sampledValue[i]->context = OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK;
                sampledValue[i]->format = OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW;
                sampledValue[i]->location = OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET;
                sampledValue[i]->measurand = OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PRE;
                sampledValue[i]->unit = OCPP_PACKAGE_METERVALUES_MS_UNIT_KVAR;
                snprintf(sampledValue[i]->value, 16, "%0.2f", ocpp_chargePoint->getPowerActiveExport(connector));
            }
            else if (strcmp(*measurands, "Power.Offered") == 0)
            {
                sampledValue[i]->context = OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK;
                sampledValue[i]->format = OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW;
                sampledValue[i]->location = OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET;
                sampledValue[i]->measurand = OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PO;
                sampledValue[i]->unit = OCPP_PACKAGE_METERVALUES_MS_UNIT_V;
                snprintf(sampledValue[i]->value, 16, "%0.2f", ocpp_chargePoint->getPowerOffered(connector));
            }
            else if (strcmp(*measurands, "Power.Factor") == 0)
            {
                sampledValue[i]->context = OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK;
                sampledValue[i]->format = OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW;
                sampledValue[i]->location = OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET;
                sampledValue[i]->measurand = OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PF;
                sampledValue[i]->unit = OCPP_PACKAGE_METERVALUES_MS_UNIT_V;
                snprintf(sampledValue[i]->value, 16, "%0.2f", ocpp_chargePoint->getPowerFactor(connector));
            }
            else if (strcmp(*measurands, "Power.Active.Import") == 0)
            {
                sampledValue[i]->context = OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK;
                sampledValue[i]->format = OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW;
                sampledValue[i]->location = OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET;
                sampledValue[i]->measurand = OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PAI;
                sampledValue[i]->unit = OCPP_PACKAGE_METERVALUES_MS_UNIT_W;
                snprintf(sampledValue[i]->value, 16, "%0.2f", ocpp_chargePoint->getPowerActiveImport(connector));
            }
            else if (strcmp(*measurands, "Power.Active.Export") == 0)
            {
                sampledValue[i]->context = OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK;
                sampledValue[i]->format = OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW;
                sampledValue[i]->location = OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET;
                sampledValue[i]->measurand = OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PAE;
                sampledValue[i]->unit = OCPP_PACKAGE_METERVALUES_MS_UNIT_V;
                snprintf(sampledValue[i]->value, 16, "%0.2f", ocpp_chargePoint->getPowerActiveExport(connector));
            }
            else if (strcmp(*measurands, "Frequency") == 0)
            {
                sampledValue[i]->context = OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK;
                sampledValue[i]->format = OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW;
                sampledValue[i]->location = OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET;
                sampledValue[i]->measurand = OCPP_PACKAGE_METERVALUES_MS_MEASURAND_F;
                sampledValue[i]->unit = OCPP_PACKAGE_METERVALUES_MS_UNIT_V;
                snprintf(sampledValue[i]->value, 16, "%0.2f", ocpp_chargePoint->getFrequency(connector));
            }
            else if (strcmp(*measurands, "Energy.Reactive.Export.Interval") == 0)
            {
                sampledValue[i]->context = OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK;
                sampledValue[i]->format = OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW;
                sampledValue[i]->location = OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET;
                sampledValue[i]->measurand = OCPP_PACKAGE_METERVALUES_MS_MEASURAND_EREI;
                sampledValue[i]->unit = OCPP_PACKAGE_METERVALUES_MS_UNIT_V;
                snprintf(sampledValue[i]->value, 16, "%0.2f", ocpp_chargePoint->getEnergyReactiveExportInterval(connector));
            }
            else if (strcmp(*measurands, "Energy.Reactive.Import.Interval") == 0)
            {
                sampledValue[i]->context = OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK;
                sampledValue[i]->format = OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW;
                sampledValue[i]->location = OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET;
                sampledValue[i]->measurand = OCPP_PACKAGE_METERVALUES_MS_MEASURAND_ERII;
                sampledValue[i]->unit = OCPP_PACKAGE_METERVALUES_MS_UNIT_V;
                snprintf(sampledValue[i]->value, 16, "%0.2f", ocpp_chargePoint->getEnergyReactiveImportInterval(connector));
            }
            else if (strcmp(*measurands, "Energy.Active.Import.Interval") == 0)
            {
                sampledValue[i]->context = OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK;
                sampledValue[i]->format = OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW;
                sampledValue[i]->location = OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET;
                sampledValue[i]->measurand = OCPP_PACKAGE_METERVALUES_MS_MEASURAND_EAII;
                sampledValue[i]->unit = OCPP_PACKAGE_METERVALUES_MS_UNIT_V;
                snprintf(sampledValue[i]->value, 16, "%0.2f", ocpp_chargePoint->getEnergyActiveImportInterval(connector));
            }
            else if (strcmp(*measurands, "Energy.Active.Export.Interval") == 0)
            {
                sampledValue[i]->context = OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK;
                sampledValue[i]->format = OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW;
                sampledValue[i]->location = OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET;
                sampledValue[i]->measurand = OCPP_PACKAGE_METERVALUES_MS_MEASURAND_EAEI;
                sampledValue[i]->unit = OCPP_PACKAGE_METERVALUES_MS_UNIT_V;
                snprintf(sampledValue[i]->value, 16, "%0.2f", ocpp_chargePoint->getEnergyActiveExportInterval(connector));
            }
            else if (strcmp(*measurands, "Energy.Reactive.Import.Register") == 0)
            {
                sampledValue[i]->context = OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK;
                sampledValue[i]->format = OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW;
                sampledValue[i]->location = OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET;
                sampledValue[i]->measurand = OCPP_PACKAGE_METERVALUES_MS_MEASURAND_ERIR;
                sampledValue[i]->unit = OCPP_PACKAGE_METERVALUES_MS_UNIT_WH;
                snprintf(sampledValue[i]->value, 16, "%0.2f", ocpp_chargePoint->getEnergyReactiveImportRegister(connector));
            }
            else if (strcmp(*measurands, "Energy.Reactive.Export.Register") == 0)
            {
                sampledValue[i]->context = OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK;
                sampledValue[i]->format = OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW;
                sampledValue[i]->location = OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET;
                sampledValue[i]->measurand = OCPP_PACKAGE_METERVALUES_MS_MEASURAND_ERER;
                sampledValue[i]->unit = OCPP_PACKAGE_METERVALUES_MS_UNIT_V;
                snprintf(sampledValue[i]->value, 16, "%0.2f", ocpp_chargePoint->getEnergyReactiveExportRegister(connector));
            }
            else if (strcmp(*measurands, "Energy.Active.Import.Register") == 0)
            {
                sampledValue[i]->context = OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK;
                sampledValue[i]->format = OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW;
                sampledValue[i]->location = OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET;
                sampledValue[i]->measurand = OCPP_PACKAGE_METERVALUES_MS_MEASURAND_EAIR;
                sampledValue[i]->unit = OCPP_PACKAGE_METERVALUES_MS_UNIT_WH;
                snprintf(sampledValue[i]->value, 16, "%0.2f", ocpp_chargePoint->getEnergyActiveImportRegister(connector));
            }
            else if (strcmp(*measurands, "Energy.Active.Export.Register") == 0)
            {
                sampledValue[i]->context = OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK;
                sampledValue[i]->format = OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW;
                sampledValue[i]->location = OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET;
                sampledValue[i]->measurand = OCPP_PACKAGE_METERVALUES_MS_MEASURAND_EAER;
                sampledValue[i]->unit = OCPP_PACKAGE_METERVALUES_MS_UNIT_V;
                snprintf(sampledValue[i]->value, 16, "%0.2f", ocpp_chargePoint->getEnergyActiveExportRegister(connector));
            }
            else if (strcmp(*measurands, "Current.Offered") == 0)
            {
                sampledValue[i]->context = OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK;
                sampledValue[i]->format = OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW;
                sampledValue[i]->location = OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET;
                sampledValue[i]->measurand = OCPP_PACKAGE_METERVALUES_MS_MEASURAND_CO;
                sampledValue[i]->unit = OCPP_PACKAGE_METERVALUES_MS_UNIT_A;
                snprintf(sampledValue[i]->value, 16, "%0.2f", ocpp_chargePoint->getCurrentOffered(connector));
            }
            else if (strcmp(*measurands, "Current.Import") == 0)
            {
                sampledValue[i]->context = OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK;
                sampledValue[i]->format = OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW;
                sampledValue[i]->location = OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET;
                sampledValue[i]->measurand = OCPP_PACKAGE_METERVALUES_MS_MEASURAND_CI;
                sampledValue[i]->unit = OCPP_PACKAGE_METERVALUES_MS_UNIT_A;
                snprintf(sampledValue[i]->value, 16, "%0.2f", ocpp_chargePoint->getCurrentImport(connector));
            }
            else if (strcmp(*measurands, "Current.Export") == 0)
            {
                sampledValue[i]->context = OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK;
                sampledValue[i]->format = OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW;
                sampledValue[i]->location = OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET;
                sampledValue[i]->measurand = OCPP_PACKAGE_METERVALUES_MS_MEASURAND_CE;
                sampledValue[i]->unit = OCPP_PACKAGE_METERVALUES_MS_UNIT_V;
                snprintf(sampledValue[i]->value, 16, "%0.2f", ocpp_chargePoint->getCurrentExport(connector));
            }
            free(*measurands);
        }

        ocpp_package_MeterValues_req_t MeterValues;
        memset(&MeterValues, 0, sizeof(MeterValues));

        MeterValues.connectorId = connector;
        MeterValues.transactionIdIsUse = 1;
        MeterValues.transactionId = transactionId;

        char *timestamp = ocpp_AuxiliaryTool_GetCurrentTime();
        snprintf(MeterValues.meterValue.timestamp, 32, timestamp);
        free(timestamp);

        MeterValues.meterValue.sampledValueCnt = cnt;
        MeterValues.meterValue.sampledValue = sampledValue;

        char *message = ocpp_package_prepare_MeterValues_Request(uniqueId, &MeterValues);
        ocpp_chargePoint_send(uniqueId, message, OCPP_PACKAGE_METERVALUES);

        for (i = 0; i < cnt; i++)
            free(sampledValue[i]);
        free(sampledValue);
        free(freeMeasurands);
    }

    free(MeterValuesSampledData);

    return uniqueId;
}

/**
 * @description:
 * @param:
 * @return:
 */
char *ocpp_transaction_sendStopTransaction_Simpleness(int connector, const char *idTag, int transactionId, enum OCPP_PACKAGE_STOP_REASON_E reason)
{

    ocpp_package_StopTransaction_req_t stopTransaction;
    memset(&stopTransaction, 0, sizeof(stopTransaction));

    char *uniqueId = ocpp_AuxiliaryTool_GenerateUUID();

    stopTransaction.idTagIsUse = 0;
    if (idTag != NULL)
    {
        stopTransaction.idTagIsUse = 1;
        strncpy(stopTransaction.idTag, idTag, 20);
    }

    stopTransaction.meterStop = (int)ocpp_chargePoint->getCurrentMeterValues(connector);

    char *timestamp = ocpp_AuxiliaryTool_GetCurrentTime();
    strncpy(stopTransaction.timestamp, timestamp, 32);

    stopTransaction.transactionId = transactionId;

    stopTransaction.reasonIsUse = 1;
    stopTransaction.reason = reason;

    stopTransaction.transactionDataIsUse = 0;

    char *message = ocpp_package_prepare_StopTransaction_Request(uniqueId, &stopTransaction);
    ocpp_chargePoint_send(uniqueId, message, OCPP_PACKAGE_STOPTRANSACTION);

    return uniqueId;
}

/**
 * @description:
 * @param:
 * @return:
 */
char *ocpp_transaction_sendStopTransaction_Complex(int connector, const char *idTag, int transactionId, enum OCPP_PACKAGE_STOP_REASON_E reason)
{

    return NULL;
}

/**
 * @description:
 * @return {*}
 */
char *ocpp_chargePoint_sendDiagnosticsStatusNotification_Req()
{

    ocpp_package_DiagnosticsStatusNotification_req_t DiagnosticsStatusNotification;
    memset(&DiagnosticsStatusNotification, 0, sizeof(DiagnosticsStatusNotification));

    DiagnosticsStatusNotification.status = ocpp_chargePoint->ocpp_diagnostics_lastDiagnosticsStatus;

    char *uniqueId = ocpp_AuxiliaryTool_GenerateUUID();
    char *message = ocpp_package_prepare_DiagnosticsStatusNotification_Request(uniqueId, &DiagnosticsStatusNotification);

    ocpp_chargePoint_send(uniqueId, message, OCPP_PACKAGE_DIAGNOSTICS_STATUS_NOTIFICATION);

    return uniqueId;
}

/**
 * @description:
 * @return {*}
 */
char *ocpp_chargePoint_sendFirmwareStatusNotification_Req(void)
{
    ocpp_package_FirmwareStatusNotification_req_t FirmwareStatusNotification;
    memset(&FirmwareStatusNotification, 0, sizeof(FirmwareStatusNotification));

    FirmwareStatusNotification.status = ocpp_chargePoint->ocpp_firmwareUpdate_lastUpdateState;

    char *uniqueId = ocpp_AuxiliaryTool_GenerateUUID();
    char *message = ocpp_package_prepare_FirmwareStatusNotification_Request(uniqueId, &FirmwareStatusNotification);

    ocpp_chargePoint_send(uniqueId, message, OCPP_PACKAGE_FIRMWARE_STATUS_NOTIFICATION);

    return uniqueId;
}

/**
 * @description:
 * @return {*}
 */
char *ocpp_chargePoint_sendStatusNotification_Req(int connector)
{

    char *uniqueId = ocpp_AuxiliaryTool_GenerateUUID();
    char *message = ocpp_package_prepare_StatusNotification_Request(uniqueId, ocpp_chargePoint->connector[connector]);

    ocpp_chargePoint_send(uniqueId, message, OCPP_PACKAGE_STATUS_NOTIFICATION);

    return uniqueId;
}

/**
 * @description:
 * @param:
 * @return: 0:fail , 1:ongoing, 2:success, 3:unkown
 */
enum OCPP_CHARGEPOINT_AUTHORIZE_RESULT_E ocpp_chargePoint_authorizationOfIdentifier(const char *const idTag, char **uniqueId)
{
    if (idTag == NULL)
        return OCPP_CHARGEPOINT_AUTHORIZE_RESULT_FAIL;

    bool localAuthorizeOffline = false;
    bool authorizationCacheEnabled = false;
    bool localAuthListEnabled = false;
    bool localPreAuthorize = false;

    ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[OCPP_CK_LocalAuthorizeOffline], (void *)&localAuthorizeOffline);
    ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[OCPP_CK_AuthorizationCacheEnabled], (void *)&authorizationCacheEnabled);
    ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[OCPP_CK_LocalAuthListEnabled], (void *)&localAuthListEnabled);
    ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[OCPP_CK_LocalPreAuthorize], (void *)&localPreAuthorize);

    if (ocpp_chargePoint->connect.isConnect) // 服务器连接的情况下发送IdTag
    {
        *uniqueId = ocpp_chargePoint_sendAuthorize_req(idTag); // 返回uniqueId
        if (localPreAuthorize)                                 // 支持localPreAuthorize授权
        {
            if (localAuthListEnabled) // 打开localAuthListEnabled
            {
                if (ocpp_localAuthorization_List_search(idTag)) // 本地列表是否存在IdTag
                {
                    if (ocpp_localAuthorization_List_isVaild(idTag))
                    {
                        printf("列表有效\n");
                        return OCPP_CHARGEPOINT_AUTHORIZE_RESULT_SUCCEED; // 列表有效
                    }
                }
            }

            if (authorizationCacheEnabled) // 打开authorizationCacheEnabled
            {
                if (ocpp_localAuthorization_Cache_search(idTag)) // 缓存是否存在IdTag
                {
                    if (ocpp_localAuthorization_Cache_isVaild(idTag))
                    {
                        return OCPP_CHARGEPOINT_AUTHORIZE_RESULT_SUCCEED; // 缓存有效
                    }
                }
            }
        }

        return OCPP_CHARGEPOINT_AUTHORIZE_RESULT_ONGOING;
    }
    else // 服务器未连接的情况下发送IdTag
    {
        if (localAuthorizeOffline)
        {
            if (localAuthListEnabled)
            {
                if (ocpp_localAuthorization_List_search(idTag)) // 本地列表是否存在IdTag
                {
                    if (ocpp_localAuthorization_List_isVaild(idTag))
                    {
                        return OCPP_CHARGEPOINT_AUTHORIZE_RESULT_SUCCEED; // 列表有效
                    }
                }
            }

            if (authorizationCacheEnabled)
            {
                if (ocpp_localAuthorization_Cache_search(idTag)) // 缓存是否存在IdTag
                {
                    if (ocpp_localAuthorization_Cache_isVaild(idTag))
                    {
                        return OCPP_CHARGEPOINT_AUTHORIZE_RESULT_SUCCEED; // 缓存有效
                    }
                }
            }
        }
    }

    return OCPP_CHARGEPOINT_AUTHORIZE_RESULT_FAIL;
}

/**
 * @description:
 * @param {int} connector:0 indicate pile, 1 indicate connector0, 2 indicate connector1
 * @param {char *} idTag
 * @return {*}
 */
void ocpp_chargePoint_Authorization_IdTag(int connector, const char *idTag)
{

    if (connector < 0 || connector > ocpp_chargePoint->numberOfConnector)
    {
        ocpp_chargePoint->setAuthorizeResult(connector, false);
        return;
    }

    enum OCPP_CHARGEPOINT_AUTHORIZE_RESULT_E result;

    char *lastUniqueId = NULL;
    result = ocpp_chargePoint_authorizationOfIdentifier(idTag, &lastUniqueId);

    if (lastUniqueId != NULL)
    {
        strncpy(ocpp_chargePoint->authorizetion[connector]->lastUniqueId, lastUniqueId, 40);
    }

    ocpp_chargePoint->authorizetion[connector]->result = result;
    ocpp_chargePoint->authorizetion[connector]->isWaitAuthoriza = false;

    bool AllowOfflineTxForUnknownId = false;
    ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[OCPP_CK_AllowOfflineTxForUnknownId], (void *)&AllowOfflineTxForUnknownId);

    switch (result)
    {
    case OCPP_CHARGEPOINT_AUTHORIZE_RESULT_FAIL:
        ocpp_chargePoint->setAuthorizeResult(connector, false);
        break;

    case OCPP_CHARGEPOINT_AUTHORIZE_RESULT_ONGOING:
        ocpp_chargePoint->authorizetion[connector]->isWaitAuthoriza = true;
        strncpy(ocpp_chargePoint->authorizetion[connector]->idTag, idTag, OCPP_AUTHORIZATION_IDTAG_LEN);
        break;

    case OCPP_CHARGEPOINT_AUTHORIZE_RESULT_UNKOWN:
        if (AllowOfflineTxForUnknownId)
            ocpp_chargePoint->setAuthorizeResult(connector, true);
        else
            ocpp_chargePoint->setAuthorizeResult(connector, false);
        break;

    case OCPP_CHARGEPOINT_AUTHORIZE_RESULT_SUCCEED:
        ocpp_chargePoint->setAuthorizeResult(connector, true);
        break;
    }
}

/**
 * @description: boot线程
 * @param:
 * @return:
 */
static void *ocpp_chargePoint_boot(void *arg)
{

    OCPP_LOG_DEBUG("create Boot thread\n");

    int sleep_s = 0;
    while (1)
    {

        if (ocpp_chargePoint->connect.isConnect)
        {
            if (ocpp_chargePoint->RegistrationStatus != OCPP_PACKAGE_REGISTRATION_STATUS_ACCEPTED)
            {

                ocpp_chargePoint_sendBootNotification_req();
            }
            else
            {

                ocpp_chargePoint_sendHeartbeat_Req();
            }

            ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[OCPP_CK_HeartbeatInterval], (void *)&sleep_s);
        }
        else
        {

            sleep_s = 5;
        }

        if (sleep_s <= 0)
            sleep_s = 5;
        usleep(sleep_s * 1000 * 1000);

        OCPP_LOG_DEBUG("boot sleep");
    }
}

/**
 * @description:
 * @param {ocpp_chargePoint_t} *ocpp_chargePoint
 * @return {*}
 */
static void *ocpp_chargePoint_StatusNotification(void *arg)
{
    OCPP_LOG_INFO("create StatusNotification thread\n");
    enum OCPP_PACKAGE_CHARGEPOINT_STATUS_E oldStatus[ocpp_chargePoint->numberOfConnector];

    while (1)
    {
        // The minimum duration that a Charge Point or Connector status is stable before a StatusNotification.req PDU is sent to the Central System.
        bool minimumStatusDurationIsUse = ocpp_ConfigurationKey_isUse(ocpp_configurationKeyText[OCPP_CK_MinimumStatusDuration]);
        int minimumStatusDuration = 0;
        if (minimumStatusDurationIsUse)
        {
            ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[OCPP_CK_MinimumStatusDuration], (void *)&minimumStatusDuration);
        }
        if (minimumStatusDuration < 1)
            minimumStatusDuration = 1;

        if (ocpp_chargePoint->RegistrationStatus == OCPP_PACKAGE_REGISTRATION_STATUS_ACCEPTED)
        {

            // DEBUG_MSG("connector number = %d\n",ocpp_chargePoint->numberOfConnector);
            int i;
            for (i = 0; i <= ocpp_chargePoint->numberOfConnector; i++)
            {
                if (oldStatus[i] != ocpp_chargePoint->connector[i]->status)
                {

                    oldStatus[i] = ocpp_chargePoint->connector[i]->status;

                    ocpp_package_StatusNotification_req_t statusNotification;
                    memset(&statusNotification, 0, sizeof(statusNotification));

                    statusNotification.connectorId = i;
                    statusNotification.errorCode = ocpp_chargePoint->connector[i]->errorCode;

                    statusNotification.infoIsUse = ocpp_chargePoint->connector[i]->infoIsUse;
                    strncpy(statusNotification.info, ocpp_chargePoint->connector[i]->info, 50);

                    statusNotification.status = ocpp_chargePoint->connector[i]->status;

                    statusNotification.timestampIsUse = ocpp_chargePoint->connector[i]->timestampIsUse;
                    strncpy(statusNotification.timestamp, ocpp_chargePoint->connector[i]->timestamp, 32);

                    statusNotification.vendorIdIsUse = ocpp_chargePoint->connector[i]->vendorIdIsUse;
                    strncpy(statusNotification.vendorId, ocpp_chargePoint->connector[i]->vendorId, 255);

                    statusNotification.vendorErrorCodeIsUse = ocpp_chargePoint->connector[i]->vendorErrorCodeIsUse;
                    strncpy(statusNotification.vendorErrorCode, ocpp_chargePoint->connector[i]->vendorErrorCode, 50);

                    char *uniqueId = ocpp_AuxiliaryTool_GenerateUUID();
                    char *message = ocpp_package_prepare_StatusNotification_Request(uniqueId, &statusNotification);

                    OCPP_LOG_DEBUG("send StatusNotification\n");
                    ocpp_chargePoint_send(uniqueId, message, OCPP_PACKAGE_STATUS_NOTIFICATION);
                }
            }
        }
        // DEBUG_MSG("status Notification %ld ms",minimumStatusDuration * 1000);
        usleep(minimumStatusDuration * 1000 * 1000); // MinimumStatusDuration
    }
}

/**
 * @description:
 * @param {void} *arg
 * @return {*}
 */
static void *ocpp_chargePoint_Transaction_thread(void *arg)
{
    OCPP_LOG_DEBUG("create transaction thread...");
    int connector = (int)arg;
    ocpp_chargePoint_transaction_t *item = ocpp_chargePoint->transaction_obj[connector];
    item->isTransaction = true;

    enum OCPP_TRANSACTION_STATE_E
    {
        OCPP_TARNSACTION_STATE_NONE = 0,
        OCPP_TRANSACTION_STATE_STARTTRANSACTION,
        OCPP_TRANSACTION_STATE_CHARGING,
        OCPP_TRANSACTION_STATE_STOPTRANSACTION
    };

    enum OCPP_TRANSACTION_STATE_E state = OCPP_TRANSACTION_STATE_STARTTRANSACTION;

    int transactionMessageAttempts = 0;
    ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[OCPP_CK_TransactionMessageAttempt], (void *)&transactionMessageAttempts);

    int transactionMessageRetryInterval = 0;
    ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[OCPP_CK_TransactionMessageRetryInterval], (void *)&transactionMessageRetryInterval);

    int meterValueSampleInterval = 0;
    ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[OCPP_CK_MeterValueSampleInterval], (void *)&meterValueSampleInterval);

    bool isWaitResult = false;
    int attempts = 0;
    unsigned int RetriesInterval = 0;

    bool terminate = false;
    char *lastUniqueId = NULL;

    while (1)
    {

        if (terminate)
        {
            OCPP_LOG_DEBUG("terminate!");
            break;
        }

        switch (state)
        {
        case OCPP_TRANSACTION_STATE_STARTTRANSACTION:
            if (isWaitResult == false)
            {
                lastUniqueId = ocpp_chargePoint_sendStartTransaction(connector, item->startIdTag, item->reservationId);
                strncpy(item->lastUniqueId, lastUniqueId, 40);

                // 与服务器建立连接，需要等待回复
                if (ocpp_chargePoint->RegistrationStatus == OCPP_PACKAGE_REGISTRATION_STATUS_ACCEPTED)
                {
                    attempts++;
                    RetriesInterval = ocpp_AuxiliaryTool_getSystemTime_ms();
                    isWaitResult = true;
                    OCPP_LOG_DEBUG("服务器连接,等待回复");
                }
                else
                {

                    // 启动充电,返回结果给控制板
                    ocpp_chargePoint->startCharging(connector);

                    state = OCPP_TRANSACTION_STATE_CHARGING;
                    item->transactionId = ocpp_AuxiliaryTool_GenerateInt(); // 随机生成交易ID
                    OCPP_LOG_DEBUG("服务器未连接,不需要等待回复");
                }
            }
            else
            {

                if (item->isRetransmission == false)
                {
                    if (item->isRecStartTransaction_Conf)
                    {

                        isWaitResult = false;
                        attempts = 0;
                        RetriesInterval = ocpp_AuxiliaryTool_getSystemTime_ms();

                        item->transactionId = item->startTransaction.transactionId;
                        if (item->startTransaction.idTagInfo.AuthorizationStatus == OCPP_LOCAL_AUTHORIZATION_ACCEPTED)
                        {
                            OCPP_LOG_DEBUG("允许充电");

                            // 启动充电,并设置认证成功
                            ocpp_chargePoint->startCharging(connector);
                            state = OCPP_TRANSACTION_STATE_CHARGING;
                        }
                        else
                        {

                            terminate = true;
                            break;
                        }
                    }
                }
                else
                {
                    // retry
                    OCPP_LOG_DEBUG("attempts time = %lds", attempts * transactionMessageRetryInterval);
                    if (attempts < transactionMessageAttempts)
                    {
                        if (ocpp_AuxiliaryTool_getDiffTime_ms(&RetriesInterval) >= attempts * transactionMessageRetryInterval)
                        {
                            isWaitResult = false;
                            item->isRetransmission = false;
                        }
                    }
                    else
                    {
                        terminate = true;
                        break;
                    }
                }
            }
            break;

        case OCPP_TRANSACTION_STATE_CHARGING:

            if (item->isStop)
            {
                OCPP_LOG_DEBUG("stop charging ...");
                state = OCPP_TRANSACTION_STATE_STOPTRANSACTION;
            }

            // 循环发送电表数据
            if (ocpp_AuxiliaryTool_getDiffTime_ms(&RetriesInterval) >= meterValueSampleInterval * 1000)
            {

                lastUniqueId = ocpp_chargePoint_sendMeterValues(connector, item->transactionId);
                RetriesInterval = ocpp_AuxiliaryTool_getSystemTime_ms();
            }

            if (item->authorizeResult == OCPP_CHARGEPOINT_AUTHORIZE_RESULT_UNKOWN)
            {
                if (ocpp_chargePoint->RegistrationStatus == OCPP_PACKAGE_REGISTRATION_STATUS_ACCEPTED)
                {
                    if (isWaitResult == false)
                    {

                        lastUniqueId = ocpp_chargePoint_sendStartTransaction(connector, item->startIdTag, item->reservationId);
                        strncpy(item->lastUniqueId, lastUniqueId, 40);
                        attempts++;
                        RetriesInterval = ocpp_AuxiliaryTool_getSystemTime_ms();
                        isWaitResult = true;
                    }
                    else
                    {

                        if (item->isRetransmission == false)
                        {
                            if (item->isRecStartTransaction_Conf)
                            {
                                isWaitResult = false;
                                attempts = 0;
                                if (item->startTransaction.idTagInfo.AuthorizationStatus == OCPP_LOCAL_AUTHORIZATION_ACCEPTED)
                                {
                                    state = OCPP_TRANSACTION_STATE_CHARGING;
                                    item->authorizeResult = OCPP_CHARGEPOINT_AUTHORIZE_RESULT_SUCCEED;
                                    RetriesInterval = ocpp_AuxiliaryTool_getSystemTime_ms();
                                }
                                else
                                {

                                    // terminate transaction
                                    state = OCPP_TRANSACTION_STATE_STOPTRANSACTION;
                                    item->reason = OCPP_PACKAGE_STOP_REASON_DEAUTHORIZED;
                                }
                            }
                        }
                        else
                        {

                            if (attempts < transactionMessageAttempts)
                            {
                                if (ocpp_AuxiliaryTool_getDiffTime_ms(&RetriesInterval) >= attempts * transactionMessageRetryInterval)
                                {
                                    isWaitResult = false;
                                    item->isRetransmission = false;
                                }
                            }
                            else
                            {
                                // not impletement
                            }
                        }
                    }
                }
            }

            break;

        case OCPP_TRANSACTION_STATE_STOPTRANSACTION:
            if (isWaitResult == false)
            {
                OCPP_LOG_DEBUG("stopIdTag = %s", item->stopIdTag);
                lastUniqueId = ocpp_transaction_sendStopTransaction_Simpleness(connector, item->startIdTag, item->transactionId, item->reason);

                strncpy(item->lastUniqueId, lastUniqueId, 40);

                if (ocpp_chargePoint->RegistrationStatus == OCPP_PACKAGE_REGISTRATION_STATUS_ACCEPTED)
                {
                    attempts++;
                    RetriesInterval = ocpp_AuxiliaryTool_getSystemTime_ms();
                    isWaitResult = true;
                }
                else
                {

                    // stop charging and set Authorizetion success
                    terminate = true;
                    break;
                }
            }
            else
            {

                if (item->isRetransmission == false)
                {
                    if (item->isRecStopTransaction_Conf)
                    {
                        if (item->stopTransaction.idTagInfo.AuthorizationStatus == OCPP_LOCAL_AUTHORIZATION_ACCEPTED)
                        {
                            OCPP_LOG_DEBUG("permit stop charging!");

                            // stop charging and set Authorizetion success
                            terminate = true;
                            break;
                        }
                        else
                        {
                            state = OCPP_TRANSACTION_STATE_CHARGING;
                        }
                    }
                }
                else
                {
                    if (attempts < transactionMessageAttempts)
                    {

                        if (ocpp_AuxiliaryTool_getDiffTime_ms(&RetriesInterval) >= attempts * transactionMessageRetryInterval)
                        {
                            isWaitResult = false;
                            item->isRetransmission = false;
                        }
                    }
                    else
                    {
                        terminate = true;
                        break;
                    }
                }
            }

            break;

        } // switch

    } // which

    item->isTransaction = false;
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

/**
 * @description:
 * @param {char *} uniqueId
 * @param {ocpp_package_RemoteStartTransaction_req_t} remoteStartTransaction_req
 * @return {*}
 */
void ocpp_chargePoint_manageRemoteStartTransactionRequest(const char *uniqueId, ocpp_package_RemoteStartTransaction_req_t remoteStartTransaction_req)
{

    ocpp_package_RemoteStartTransaction_conf_t remoteStartTransaction_conf;
    memset(&remoteStartTransaction_conf, 0, sizeof(remoteStartTransaction_conf));

    remoteStartTransaction_conf.status = OCPP_PACKAGE_REMOTE_STRATSTOP_STATUS_REJECTED;

    bool authorizeRemoteTxRequests = false;
    ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[OCPP_CK_AuthorizeRemoteTxRequests], (void *)&authorizeRemoteTxRequests);

    if (authorizeRemoteTxRequests)
    {
        int connector = 0;
        if (remoteStartTransaction_req.connectorIdIsUse)
            connector = remoteStartTransaction_req.connectorId;

        char *uniqueId_backup = NULL;
        enum OCPP_CHARGEPOINT_AUTHORIZE_RESULT_E result = OCPP_CHARGEPOINT_AUTHORIZE_RESULT_FAIL;
        result = ocpp_chargePoint_authorizationOfIdentifier(remoteStartTransaction_req.idTag, &uniqueId_backup);

        if (result == OCPP_CHARGEPOINT_AUTHORIZE_RESULT_ONGOING)
        {
            strncpy(ocpp_chargePoint->authorizetion[connector]->idTag, remoteStartTransaction_req.idTag, OCPP_AUTHORIZATION_IDTAG_LEN);
            ocpp_chargePoint->authorizetion[connector]->isWaitAuthoriza = true;
            strncpy(ocpp_chargePoint->authorizetion[connector]->lastUniqueId, uniqueId_backup, 40);
        }
    }

    if (remoteStartTransaction_req.connectorIdIsUse)
    {

        if (ocpp_chargePoint->transaction_obj[remoteStartTransaction_req.connectorId]->isTransaction == false)
        {
            if (ocpp_chargePoint->connector[0]->status == OCPP_PACKAGE_CHARGEPOINT_STATUS_AVAILABLE)
            {
                bool isAllowTransaction = false;
                if (ocpp_chargePoint->connector[remoteStartTransaction_req.connectorId]->status == OCPP_PACKAGE_CHARGEPOINT_STATUS_RESERVED)
                {
                    if (strcmp(ocpp_chargePoint->reserveConnector[remoteStartTransaction_req.connectorId]->idTag, remoteStartTransaction_req.idTag) == 0)
                        isAllowTransaction = true;
                }
                else if (ocpp_chargePoint->connector[remoteStartTransaction_req.connectorId]->status == OCPP_PACKAGE_CHARGEPOINT_STATUS_PREPARING)
                    isAllowTransaction = true;

                if (isAllowTransaction)
                {
                    memset(ocpp_chargePoint->transaction_obj[remoteStartTransaction_req.connectorId], 0, sizeof(ocpp_chargePoint_transaction_t));
                    remoteStartTransaction_conf.status = OCPP_PACKAGE_REMOTE_STRATSTOP_STATUS_ACCEPTED;
                    strncpy(ocpp_chargePoint->transaction_obj[remoteStartTransaction_req.connectorId]->startIdTag, remoteStartTransaction_req.idTag, OCPP_AUTHORIZATION_IDTAG_LEN);

                    pthread_t tid_transaction;
                    if (pthread_create(&tid_transaction, NULL, ocpp_chargePoint_Transaction_thread, (void *)remoteStartTransaction_req.connectorId) == -1)
                    {
                        OCPP_LOG_ERROR("create transaction thread fail");
                    }
                    pthread_detach(tid_transaction);
                }
            }
        }
    }

    // response
    char *lastUniqueId = strdup(uniqueId);
    char *message = ocpp_package_prepare_RemoteStartTransaction_Response(lastUniqueId, &remoteStartTransaction_conf);
    ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALL);
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {ocpp_package_CancelReservation_req_t} cancelReservation_req
 * @return {*}
 */
void ocpp_chargePoint_manageCancelReservationRequest(const char *uniqueId, ocpp_package_CancelReservation_req_t cancelReservation_req)
{

    ocpp_package_CancelReservation_conf_t cancelReservation_conf;
    memset(&cancelReservation_conf, 0, sizeof(cancelReservation_conf));
    cancelReservation_conf.status = OCPP_PACKAGE_CANCEL_RESERVATION_STATUS_ACCEPTED;

    int connector = 0;
    for (connector = 0; connector <= ocpp_chargePoint->numberOfConnector; connector++)
    {
        if (ocpp_chargePoint->isReservation[connector])
            if (ocpp_chargePoint->reserveConnector[connector]->reservationId == cancelReservation_req.reservationId)
            {

                ocpp_chargePoint->isReservation[connector] = false;
                ocpp_reserve_removeReservation(cancelReservation_req.reservationId);
                break;
            }
    }

    if (connector > ocpp_chargePoint->numberOfConnector)
        cancelReservation_conf.status = OCPP_PACKAGE_CANCEL_RESERVATION_STATUS_REJECTED;

    char *lastUniqueId = strdup(uniqueId);
    char *message = ocpp_package_prepare_CancelReservation_Response(lastUniqueId, &cancelReservation_conf);
    ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALL);
}

/**
 * @description:
 * @param {void *} arg
 * @return {*}
 */
static void *ocpp_chargePoint_ChangeAvailability_thread(void *arg)
{
    int connector = (int)arg;

    while (1)
    {

        if (connector == 0)
        {
            int i = 0;
            for (i = 1; i <= ocpp_chargePoint->numberOfConnector; i++)
            {
                if (ocpp_chargePoint->transaction_obj[i]->isTransaction)
                    break;
            }

            if (i > ocpp_chargePoint->numberOfConnector)
            {

                for (i = 0; i < ocpp_chargePoint->numberOfConnector; i++)
                {
                    ocpp_chargePoint->setConnectorStatus(i, OCPP_PACKAGE_CHARGEPOINT_ERRORCODE_NOERROR, OCPP_PACKAGE_CHARGEPOINT_STATUS_UNAVAILABLE);
                }
                return;
            }
        }
        else
        {
            if (ocpp_chargePoint->transaction_obj[connector]->isTransaction == false)
            {
                ocpp_chargePoint->setConnectorStatus(connector, OCPP_PACKAGE_CHARGEPOINT_ERRORCODE_NOERROR, OCPP_PACKAGE_CHARGEPOINT_STATUS_UNAVAILABLE);

                return;
            }
        }

        usleep(500 * 1000); // 200ms
    }
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {ocpp_package_ChangeAvailability_req_t} changeAvailability_req
 * @return {*}
 */
void ocpp_chargePoint_manageChangeAvailabilityRequest(const char *uniqueId, ocpp_package_ChangeAvailability_req_t changeAvailability_req)
{

    ocpp_package_ChangeAvailability_conf_t changeAvailability_conf;
    memset(&changeAvailability_conf, 0, sizeof(changeAvailability_conf));

    changeAvailability_conf.status = OCPP_PACKAGE_AVAILABILITY_STATUS_ACCEPTED;

    if (changeAvailability_req.type == OCPP_PACKAGE_AVAILABILITY_TYPE_OPERATIVE)
    {
        if (ocpp_chargePoint->connector[changeAvailability_req.connectorId]->status == OCPP_PACKAGE_CHARGEPOINT_STATUS_UNAVAILABLE)
        {
            ocpp_chargePoint->setConnectorStatus(changeAvailability_req.connectorId, OCPP_PACKAGE_CHARGEPOINT_ERRORCODE_NOERROR, OCPP_PACKAGE_CHARGEPOINT_STATUS_AVAILABLE);
        }
    }
    else
    {
        // 如果有交易正在进行，则等待交易结束
        if (changeAvailability_req.connectorId == 0)
        {
            int connector = 0;
            for (connector = 1; connector <= ocpp_chargePoint->numberOfConnector; connector++)
            {
                if (ocpp_chargePoint->transaction_obj[connector]->isTransaction)
                {

                    changeAvailability_conf.status = OCPP_PACKAGE_AVAILABILITY_STATUS_SCHEDULED;
                    break;
                }
            }
        }
        else
        {
            if (ocpp_chargePoint->transaction_obj[changeAvailability_req.connectorId]->isTransaction)
                changeAvailability_conf.status = OCPP_PACKAGE_AVAILABILITY_STATUS_SCHEDULED;
        }

        // Persistent states? not implement
    }

    if (changeAvailability_conf.status == OCPP_PACKAGE_AVAILABILITY_STATUS_SCHEDULED)
    {

        pthread_t tid_changeAvailability;
        if (pthread_create(&tid_changeAvailability, NULL, ocpp_chargePoint_ChangeAvailability_thread, (void *)changeAvailability_req.connectorId) == -1)
        {
            changeAvailability_conf.status = OCPP_PACKAGE_AVAILABILITY_STATUS_REJECTED;
            OCPP_LOG_ERROR("create changeAvailability thread fail");
        }
        pthread_detach(tid_changeAvailability);
    }

    char *lastUniqueId = strdup(uniqueId);
    char *message = ocpp_package_prepare_ChangeAvailability_Response(lastUniqueId, &changeAvailability_conf);
    ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALL);
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {ocpp_package_ChangeConfiguration_req_t } changeConfiguration
 * @return {*}
 */
void ocpp_chargePoint_manageChangeConfigurationRequest(const char *uniqueId, ocpp_package_ChangeConfiguration_req_t changeConfiguration_req)
{

    ocpp_package_ChangeConfiguration_conf_t changeConfiguration_conf;

    changeConfiguration_conf.status = OCPP_PACKAGE_CONFIGURATION_STATUS_NOTSUPPORTED;

    if (ocpp_ConfigurationKey_isFound(changeConfiguration_req.key))
    {
        if (ocpp_ConfigurationKey_isUse(changeConfiguration_req.key))
        {
            if (ocpp_ConfigurationKey_getAcc(changeConfiguration_req.key) == OCPP_ACC_READONLY)
            {
                changeConfiguration_conf.status = OCPP_PACKAGE_CONFIGURATION_STATUS_REJECTED;
            }
            else
            {
                changeConfiguration_conf.status = OCPP_PACKAGE_CONFIGURATION_STATUS_ACCEPTED;
                ocpp_ConfigurationKey_Modify(changeConfiguration_req.key, changeConfiguration_req.value, 1);
            }
        }
    }

    char *responseId = strdup(uniqueId);
    char *message = ocpp_package_prepare_ChangeConfiguration_Response(responseId, &changeConfiguration_conf);
    ocpp_chargePoint_send(responseId, message, OCPP_PACKAGE_CALL);
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {ocpp_package_ClearCache_req_t} clearCache
 * @return {*}
 */
void ocpp_chargePoint_manageClearCacheRequest(const char *uniqueId, ocpp_package_ClearCache_req_t clearCache_req)
{

    ocpp_package_ClearCache_conf_t clearCache_conf;

    clearCache_conf.status = OCPP_PACKAGE_CLEAR_CACHE_STATUS_REJECTED;
    if (ocpp_localAuthorization_Cache_clearCache() == 0)
    {
        clearCache_conf.status = OCPP_PACKAGE_CLEAR_CACHE_STATUS_ACCEPTED;
    }

    char *lastUniqueId = strdup(uniqueId);
    char *message = ocpp_package_prepare_ClearCache_Response(lastUniqueId, &clearCache_conf);
    ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALL);
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {ocpp_package_ClearChargingProfile_req_t} clearChargingProfile
 * @return {*}
 */
void ocpp_chargePoint_manageClearChargingProfileRequest(const char *uniqueId, ocpp_package_ClearChargingProfile_req_t clearChargingProfile)
{
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {ocpp_package_DataTransfer_req_t} dataTransfer
 * @return {*}
 */
void ocpp_chargePoint_manageDataTransferRequest(const char *uniqueId, ocpp_package_DataTransfer_req_t dataTransfer)
{
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {ocpp_package_DataTransfer_req_t} dataTransfer
 * @return {*}
 */
void ocpp_chargePoint_manageGetCompositeScheduleRequest(const char *uniqueId, ocpp_package_GetCompositeSchedule_req_t getCompositeSchedule)
{
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {ocpp_package_GetConfiguration_req_t } getConfiguration
 * @return {*}
 */
void ocpp_chargePoint_manage_GetConfigurationRequest(const char *uniqueId, ocpp_package_GetConfiguration_req_t getConfiguration_req)
{

    ocpp_package_GetConfiguration_conf_t getConfiguration_conf;
    memset(&getConfiguration_conf, 0, sizeof(getConfiguration_conf));

    int GetConfigurationMaxKeys = 0;
    ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[OCPP_CK_GetConfigurationMaxKeys], (void *)&GetConfigurationMaxKeys);
    if (GetConfigurationMaxKeys == 0)
        GetConfigurationMaxKeys = OCPP_CK_MAXLEN;

    if (getConfiguration_req.keyIsUse)
    {

        getConfiguration_conf.configurationKeyIsUse = 1;
        getConfiguration_conf.unknownKeyIsUse = 1;

        GetConfigurationMaxKeys = (GetConfigurationMaxKeys > getConfiguration_req.numberOfKey) ? getConfiguration_req.numberOfKey : GetConfigurationMaxKeys;
        ocpp_package_KeyValue_t **keyValues = (ocpp_package_KeyValue_t **)calloc(GetConfigurationMaxKeys, sizeof(ocpp_package_KeyValue_t *));
        char **unkownKey = (char **)calloc(GetConfigurationMaxKeys, sizeof(char *));

        getConfiguration_conf.configurationKey = keyValues;
        getConfiguration_conf.unknownKey = unkownKey;

        int unkownKey_index = 0;
        int keyValues_index = 0;
        int index = 0;
        printf("GetConfigurationMaxKeys = %d\n", GetConfigurationMaxKeys);
        for (index = 0; index < GetConfigurationMaxKeys; index++)
        {
            keyValues[keyValues_index] = (ocpp_package_KeyValue_t *)calloc(1, sizeof(ocpp_package_KeyValue_t));

            if (ocpp_ConfigurationKey_isFound(getConfiguration_req.key[index]))
            {

                snprintf(keyValues[keyValues_index]->key, 50, getConfiguration_req.key[index]);

                if (ocpp_ConfigurationKey_getAcc(getConfiguration_req.key[index]) == OCPP_ACC_READONLY)
                {
                    keyValues[keyValues_index]->readonly = true;
                }
                else
                {
                    keyValues[keyValues_index]->readonly = false;
                }

                if (ocpp_ConfigurationKey_isUse(getConfiguration_req.key[index]))
                {
                    keyValues[keyValues_index]->valueIsUse = 1;

                    enum OCPP_CONFIGURATION_KEY_DATA_TYPE_E dataType = ocpp_ConfigurationKey_getType(getConfiguration_req.key[index]);

                    switch (dataType)
                    {
                    case OCPP_CK_DT_BOOLEAN:
                    {
                        bool data = false;
                        ocpp_ConfigurationKey_getValue(getConfiguration_req.key[index], (void *)&data);
                        snprintf(keyValues[keyValues_index]->value, 500, "%d", data ? 1 : 0);
                    }
                    break;

                    case OCPP_CK_DT_INTEGER:
                    {
                        int data = 0;
                        ocpp_ConfigurationKey_getValue(getConfiguration_req.key[index], (void *)&data);
                        snprintf(keyValues[keyValues_index]->value, 500, "%d", data);
                    }
                    break;

                    case OCPP_CK_DT_STRING:
                    {
                        ocpp_ConfigurationKey_getValue(getConfiguration_req.key[index], (void *)keyValues[keyValues_index]->value);
                    }
                    break;

                    default:
                        break;
                    }
                }

                keyValues_index++;
            }
            else
            {
                unkownKey[unkownKey_index] = (char *)calloc(1, 50);
                getConfiguration_conf.numberOfUnknownKey++;
                snprintf(unkownKey[unkownKey_index++], 50, "%s", getConfiguration_req.key[index]);
            }
        }

        getConfiguration_conf.numberOfConfigurationKey = keyValues_index;

        if (getConfiguration_conf.numberOfUnknownKey >= GetConfigurationMaxKeys)
            getConfiguration_conf.configurationKeyIsUse = 0;
        else if (getConfiguration_conf.numberOfUnknownKey <= 0)
            getConfiguration_conf.unknownKeyIsUse = 0;
    }
    else
    {

        getConfiguration_conf.configurationKeyIsUse = 1;

        ocpp_package_KeyValue_t **keyValue = (ocpp_package_KeyValue_t **)calloc(GetConfigurationMaxKeys, sizeof(ocpp_package_KeyValue_t *));
        int index = 0;
        for (index = 0; index < GetConfigurationMaxKeys; index++)
        {
            keyValue[index] = (ocpp_package_KeyValue_t *)calloc(1, sizeof(ocpp_package_KeyValue_t));
            getConfiguration_conf.numberOfConfigurationKey++;

            snprintf(keyValue[index]->key, 50, "%s", ocpp_configurationKeyText[index]);
            if (ocpp_ConfigurationKey_getAcc(ocpp_configurationKeyText[index]) == OCPP_ACC_READONLY)
            {
                keyValue[index]->readonly = true;
            }
            else
            {
                keyValue[index]->readonly = false;
            }

            if (ocpp_ConfigurationKey_isUse(ocpp_configurationKeyText[index]))
            {
                keyValue[index]->valueIsUse = 1;

                enum OCPP_CONFIGURATION_KEY_DATA_TYPE_E dataType;
                dataType = ocpp_ConfigurationKey_getType(ocpp_configurationKeyText[index]);

                switch (dataType)
                {
                case OCPP_CK_DT_BOOLEAN:
                {
                    bool data = false;
                    ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[index], (void *)&data);
                    snprintf(keyValue[index]->value, 500, "%d", data ? 1 : 0);
                }
                break;

                case OCPP_CK_DT_INTEGER:
                {
                    int data = 0;
                    ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[index], (void *)&data);
                    snprintf(keyValue[index]->value, 500, "%d", data);
                }
                break;

                case OCPP_CK_DT_STRING:
                {
                    ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[index], (void *)keyValue[index]->value);
                }
                break;

                default:
                    break;
                }
            }
        }

        getConfiguration_conf.configurationKey = keyValue;
    }

    // free
    for (; getConfiguration_req.numberOfKey > 0; getConfiguration_req.numberOfKey--)
    {

        free(getConfiguration_req.key[getConfiguration_req.numberOfKey - 1]);
    }

    free(getConfiguration_req.key);

    char *lastUniqueId = strdup(uniqueId);
    char *message = ocpp_package_prepare_GetConfiguration_Response(lastUniqueId, &getConfiguration_conf);
    ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALL);

    for (; getConfiguration_conf.numberOfConfigurationKey > 0; getConfiguration_conf.numberOfConfigurationKey--)
    {
        free(getConfiguration_conf.configurationKey[getConfiguration_conf.numberOfConfigurationKey - 1]);
    }
    free(getConfiguration_conf.configurationKey);

    for (; getConfiguration_conf.numberOfUnknownKey > 0; getConfiguration_conf.numberOfUnknownKey--)
    {
        free(getConfiguration_conf.configurationKey[getConfiguration_conf.numberOfUnknownKey - 1]);
    }
    free(getConfiguration_conf.unknownKey);
}

/**
 * @description:
 * @param
 * @param
 * @return
 */
void *ocpp_chargePoint_diagnostics_upload_thread(void *arg)
{
    OCPP_LOG_DEBUG("create upload thread");
    ocpp_chargePoint_diagnostics_t *diagnostics = (ocpp_chargePoint_diagnostics_t *)arg;

    int retries = 0;
    struct hostent *hp;
    char server_filepath_name[256] = {0};

    for (; true;)
    {

        if (retries >= diagnostics->retries)
        {
            ocpp_chargePoint->ocpp_diagnostics_lastDiagnosticsStatus = OCPP_PACKAGE_DIAGNOSTICS_STATUS_UPLOAD_FAILED;
            ocpp_chargePoint_sendDiagnosticsStatusNotification_Req();
            break;
        }

        // 从数据库导出日志文件
        OCPP_LOG_DEBUG("从数据库导出日志文件");
        struct tm *startTime = NULL;
        struct tm *endTime = NULL;
        if (diagnostics->isUseStartTime)
        {
            startTime = (struct tm *)calloc(1, sizeof(struct tm));
            strptime(diagnostics->startTime, "%Y-%m-%dT%H:%M:%S", startTime);
        }

        if (diagnostics->isUseStopTime)
        {
            endTime = (struct tm *)calloc(1, sizeof(struct tm));
            strptime(diagnostics->stopTime, "%Y-%m-%dT%H:%M:%S", startTime);
        }
        OCPP_LOG_DEBUG("数据库导出文件");
        if (ocpp_log_extractLogs(startTime, endTime, OCPP_LOG_ANY) == -1)
        {
            free(startTime);
            free(endTime);
            break;
        }

        free(startTime);
        free(endTime);

        // 根据域名提取IP地址
        OCPP_LOG_DEBUG("根据域名提取IP地址");
        for (; retries < diagnostics->retries; retries++)
        {
            if ((hp = gethostbyname(diagnostics->serverAddr)) != NULL)
            {
                break;
            }
            else
            {
                usleep(diagnostics->retryInterval * 1000 * 1000);
                OCPP_LOG_DEBUG("DNS error = %s", strerror(errno));
            }
        }

        // 连接FTP服务器
        OCPP_LOG_DEBUG("连接FTP服务器");
        for (; retries < diagnostics->retries; retries++)
        {
            if ((diagnostics->client.control_sock = ocpp_ftp_connect_server((char *)hp->h_addr, diagnostics->port)) >= 0)
            {
                break;
            }
            else
            {
                usleep(diagnostics->retryInterval * 1000 * 1000);
                OCPP_LOG_DEBUG("diagnostics connect FTP server fail");
            }
        }

        // 登录FTP服务器
        OCPP_LOG_DEBUG("登录FTP服务器");
        for (; retries < diagnostics->retries; retries++)
        {
            if (ocpp_ftp_login_server(diagnostics->client.control_sock, diagnostics->client.usr, diagnostics->client.passwd) >= 0)
            {
                break;
            }
            else
            {
                usleep(diagnostics->retryInterval * 1000 * 1000);
                OCPP_LOG_DEBUG("diagnostics login ftp server fail");
            }
        }

        // 传输诊断状态通知
        OCPP_LOG_DEBUG("发送诊断状态通知");
        if (retries < diagnostics->retries)
        {
            ocpp_chargePoint->ocpp_diagnostics_lastDiagnosticsStatus = OCPP_PACKAGE_DIAGNOSTICS_STATUS_UPLOADING;
            ocpp_chargePoint_sendDiagnosticsStatusNotification_Req();
            strcpy(server_filepath_name, diagnostics->client.ser_filepath);
            strcat(server_filepath_name, diagnostics->client.ser_filename);
            OCPP_LOG_DEBUG("server_filepath_name = %s", server_filepath_name);
        }

        // 上传诊断日志通知
        OCPP_LOG_DEBUG("上传诊断日志");
        for (; retries < diagnostics->retries; retries++)
        {
            if (ocpp_ftp_up_file(diagnostics->client.control_sock, server_filepath_name, OCPP_LOGS_FILEPATH, 1, 0) >= 0)
            {

                break;
            }
            else
            {
                usleep(diagnostics->retryInterval * 1000 * 1000);
                OCPP_LOG_DEBUG("diagnostics upload file fail");
            }
        }

        // 诊断日志上传成功
        OCPP_LOG_DEBUG("诊断日志上传成功");
        if (retries < diagnostics->retries)
        {
            ocpp_chargePoint->ocpp_diagnostics_lastDiagnosticsStatus = OCPP_PACKAGE_DIAGNOSTICS_STATUS_UPLOADED;
            ocpp_chargePoint_sendDiagnosticsStatusNotification_Req();
            break;
        }
    }

    ocpp_chargePoint->ocpp_diagnostics_lastDiagnosticsStatus = OCPP_PACKAGE_DIAGNOSTICS_STATUS_IDLE;
    free(diagnostics);
    return NULL;
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {ocpp_package_GetDiagnostics_req_t} GetDiagnostics
 * @return {*}
 */
void ocpp_chargePoint_manageGetDiagnosticsRequest(const char *uniqueId, ocpp_package_GetDiagnostics_req_t GetDiagnostics_req)
{

    ocpp_package_GetDiagnostics_conf_t getDiagnostics_conf;
    memset(&getDiagnostics_conf, 0, sizeof(getDiagnostics_conf));

    ocpp_chargePoint_diagnostics_t *diagnostics = (ocpp_chargePoint_diagnostics_t *)calloc(1, sizeof(ocpp_chargePoint_diagnostics_t));

    diagnostics->retries = 3;
    if (GetDiagnostics_req.retriesIsUse)
        diagnostics->retries = GetDiagnostics_req.retries;

    diagnostics->retryInterval = 60;
    if (GetDiagnostics_req.retryIntervalIsUse)
        diagnostics->retryInterval = GetDiagnostics_req.retryInterval;

    diagnostics->isUseStartTime = false;
    if (GetDiagnostics_req.startTimeIsUse)
    {
        diagnostics->isUseStartTime = true;
        strncpy(diagnostics->startTime, GetDiagnostics_req.startTime, 32);
    }

    diagnostics->isUseStopTime = false;
    if (GetDiagnostics_req.stopTimeIsUse)
    {
        diagnostics->isUseStopTime = true;
        strncpy(diagnostics->stopTime, GetDiagnostics_req.stopTime, 32);
    }

    //	"location":"ftp://0209GCpBn7:523Fw6FS@f1.iot17.com:8183/smart/2681b667733cd919e8iq55/diagnostics/1686016382951893/",
    char *index_start = NULL;
    char *index_stop = NULL;

    // extract username
    index_start = strchr(GetDiagnostics_req.location, '/') + 2;
    index_stop = strchr(index_start, ':');
    strncpy(diagnostics->client.usr, index_start, index_stop - index_start);
    diagnostics->client.usr[index_stop - index_start + 1] = '\0';
    OCPP_LOG_DEBUG("client.usr = %s", diagnostics->client.usr);

    // extract password
    index_start = index_stop + 1;
    index_stop = strchr(index_start, '@');
    strncpy(diagnostics->client.passwd, index_start, index_stop - index_start);
    diagnostics->client.passwd[index_stop - index_start + 1] = '\0';
    OCPP_LOG_DEBUG("client.passwd = %s", diagnostics->client.passwd);

    // extract server address
    index_start = index_stop + 1;
    index_stop = strchr(index_start, ':');
    strncpy(diagnostics->serverAddr, index_start, index_stop - index_start);
    diagnostics->serverAddr[index_stop - index_start + 1] = '\0';
    OCPP_LOG_DEBUG("serverAddr = %s", diagnostics->serverAddr);

    // extract port
    index_start = index_stop + 1;
    index_stop = strchr(index_start, '/');
    char port_str[10] = {0};
    strncpy(port_str, index_start, index_stop - index_start);
    port_str[index_stop - index_start + 1] = '\0';
    diagnostics->port = atoi(port_str);
    OCPP_LOG_DEBUG("port = %d", diagnostics->port);

    // extract path
    index_start = index_stop;
    strcpy(diagnostics->client.ser_filepath, index_start);
    OCPP_LOG_DEBUG("path = %s", diagnostics->client.ser_filepath);

    // extract filename
    strcpy(diagnostics->client.ser_filename, OCPP_LOGS_FILENAME);
    OCPP_LOG_DEBUG("filename = %s", diagnostics->client.ser_filename);

    // 创建诊断日志线程
    pthread_t ptid_diagnostics;
    if (pthread_create(&ptid_diagnostics, NULL, ocpp_chargePoint_diagnostics_upload_thread, (void *)diagnostics) == -1)
    {
        OCPP_LOG_ERROR("create diagnostics upload thread fail");
    }
    pthread_detach(ptid_diagnostics);

    getDiagnostics_conf.fileNameIsUse = 1;
    strncpy(getDiagnostics_conf.fileName, OCPP_LOGS_FILENAME, 256);

    char *lastUniqueId = strdup(uniqueId);
    char *message = ocpp_package_prepare_GetDiagnostics_Response(lastUniqueId, &getDiagnostics_conf);
    ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALL);
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {ocpp_package_GetLocalListVersion_req_t} GetLocalListVersion
 * @return {*}
 */
void ocpp_chargePoint_manageGetLocalListVersionRequest(const char *uniqueId, ocpp_package_GetLocalListVersion_req_t GetLocalListVersion_req)
{
    ocpp_package_GetLocalListVersion_conf_t GetLocalListVersion_conf;

    GetLocalListVersion_conf.listVersion = ocpp_localAuthorization_List_getListVersion2();

    char *lastUniqueId = strdup(uniqueId);
    char *message = ocpp_package_prepare_GetLocalListVersion_Response(lastUniqueId, &GetLocalListVersion_conf);
    ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALL);
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {ocpp_package_RemoteStopTransaction_req_t} remoteStopTransaction
 * @return {*}
 */
void ocpp_chargePoint_manageRemoteStopTransactionRequest(const char *uniqueId, ocpp_package_RemoteStopTransaction_req_t remoteStopTransaction_req)
{

    ocpp_package_RemoteStopTransaction_conf_t remoteStopTransaction_conf;
    memset(&remoteStopTransaction_conf, 0, sizeof(remoteStopTransaction_conf));

    remoteStopTransaction_conf.status = OCPP_PACKAGE_REMOTE_STRATSTOP_STATUS_REJECTED;

    int connector = 0;
    // for(connector = 0; connector < ocpp_chargePoint->numberOfConnector; connector++){
    //     if(remoteStopTransaction_req.transactionId == ocpp_transaction_getTransactionId(connector + 1)){
    //         remoteStopTransaction_conf.status = OCPP_PACKAGE_REMOTE_STRATSTOP_STATUS_ACCEPTED;
    //         ocpp_transaction_setRemoteStop(connector + 1);
    //         ocpp_transaction_setStopReason(connector + 1, OCPP_PACKAGE_STOP_REASON_REMOTE);
    //         break;

    //     }
    // }

    for (connector = 1; connector <= ocpp_chargePoint->numberOfConnector; connector++)
    {
        OCPP_LOG_DEBUG("isTransaction = %d", ocpp_chargePoint->transaction_obj[connector]->isTransaction);
        if (ocpp_chargePoint->transaction_obj[connector]->isTransaction)
            if (ocpp_chargePoint->transaction_obj[connector]->transactionId == remoteStopTransaction_req.transactionId)
            {
                ocpp_chargePoint->remoteStopCharging(connector);
                remoteStopTransaction_conf.status = OCPP_PACKAGE_REMOTE_STRATSTOP_STATUS_ACCEPTED;
                break;
            }
    }

    char *lastUniqueId = strdup(uniqueId);
    char *message = ocpp_package_prepare_RemoteStopTransaction_Response(lastUniqueId, &remoteStopTransaction_conf);
    ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALL);
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {ocpp_package_ReserveNow_req_t} reserveNow
 * @return {*}
 */
void ocpp_chargePoint_manageReserveNowRequest(const char *uniqueId, ocpp_package_ReserveNow_req_t reserveNow_req)
{

    ocpp_package_ReserveNow_conf_t reserveNow_conf;
    memset(&reserveNow_conf, 0, sizeof(&reserveNow_conf));

    reserveNow_conf.status = OCPP_PCAKGE_RESERVATION_STATUS_ACCEPTED;

    if (ocpp_chargePoint->connector[reserveNow_req.connectorId]->status == OCPP_PACKAGE_CHARGEPOINT_STATUS_UNAVAILABLE)
    {
        reserveNow_conf.status = OCPP_PCAKGE_RESERVATION_STATUS_UNAVAILABLE;
    }
    else if (ocpp_chargePoint->connector[reserveNow_req.connectorId]->status == OCPP_PACKAGE_CHARGEPOINT_STATUS_FAULTED)
    {
        reserveNow_conf.status = OCPP_PCAKGE_RESERVATION_STATUS_FAULTED;
    }
    else
    {

        if (ocpp_chargePoint->isReservation[reserveNow_req.connectorId])
        {
            if (ocpp_chargePoint->reserveConnector[reserveNow_req.connectorId]->reservationId == reserveNow_req.reservationId)
            {

                char *parentIdTag = NULL;
                if (reserveNow_req.parentIdTagIsUse)
                    parentIdTag = reserveNow_req.parentIdTag;

                ocpp_reserve_updateReservation(reserveNow_req.reservationId,
                                               reserveNow_req.connectorId, reserveNow_req.expiryDate, reserveNow_req.idTag, parentIdTag);
            }
            else
            {
                reserveNow_conf.status = OCPP_PCAKGE_RESERVATION_STATUS_OCCUPIED;
            }
        }
        else
        {

            if (reserveNow_req.connectorId == 0)
            {
                bool ReserveConnectorZeroSupported = false;
                ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[OCPP_CK_ReserveConnectorZeroSupported], (void *)&ReserveConnectorZeroSupported);
                if (ReserveConnectorZeroSupported == false)
                {

                    reserveNow_conf.status = OCPP_PCAKGE_RESERVATION_STATUS_REJECTED;
                }
            }

            if (reserveNow_conf.status == OCPP_PCAKGE_RESERVATION_STATUS_ACCEPTED)
            {

                ocpp_chargePoint->isReservation[reserveNow_req.connectorId] = true;
                ocpp_chargePoint->reserveConnector[reserveNow_req.connectorId]->reservationId = reserveNow_req.reservationId;
                strncpy(ocpp_chargePoint->reserveConnector[reserveNow_req.connectorId]->idTag, reserveNow_req.idTag, OCPP_AUTHORIZATION_IDTAG_LEN);
                memset(ocpp_chargePoint->reserveConnector[reserveNow_req.connectorId]->parentIdTag, 0, OCPP_AUTHORIZATION_IDTAG_LEN);
                if (reserveNow_req.parentIdTagIsUse)
                    strncpy(ocpp_chargePoint->reserveConnector[reserveNow_req.connectorId]->parentIdTag, reserveNow_req.parentIdTag, OCPP_AUTHORIZATION_IDTAG_LEN);

                strptime(reserveNow_req.expiryDate, "%Y-%m-%dT%H:%M:%S.", &ocpp_chargePoint->reserveConnector[reserveNow_req.connectorId]->expiryDate);

                char *parentIdTag = NULL;
                if (reserveNow_req.parentIdTagIsUse)
                    parentIdTag = reserveNow_req.parentIdTag;

                ocpp_reserve_updateReservation(reserveNow_req.reservationId,
                                               reserveNow_req.connectorId, reserveNow_req.expiryDate, reserveNow_req.idTag, parentIdTag);
            }
        }
    }

    char *lastUniqueId = strdup(uniqueId);
    char *message = ocpp_package_prepare_ReserveNow_Response(lastUniqueId, &reserveNow_conf);
    ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALL);
}

/**
 * @description:
 * @param {void *} arg
 * @return {*}
 */
void *ocpp_chargePoint_Reset_thread(void *arg)
{
    OCPP_LOG_DEBUG("create OCPP reset thread");
    enum OCPP_PACKAGE_RESET_TYPE_E type = (enum OCPP_PACKAGE_RESET_TYPE_E)arg;

    enum RESET_FLOW_E
    {
        RESET_FLOW_SENDSTOP = 0,
        RESET_FLOW_WAITSTOP,
        RESET_FLOW_RESET

    };

    enum RESET_FLOW_E flow = RESET_FLOW_SENDSTOP;

    while (1)
    {

        int connector = 0;
        switch (flow)
        {
        case RESET_FLOW_SENDSTOP:
        {
            for (connector = 1; connector <= ocpp_chargePoint->numberOfConnector; connector++)
            {

                if (ocpp_chargePoint->transaction_obj[connector]->isTransaction)
                {
                    ocpp_chargePoint->transaction_obj[connector]->isStop = true;
                    ocpp_chargePoint->transaction_obj[connector]->reason = OCPP_PACKAGE_STOP_REASON_REBOOT;
                }
            }

            flow = RESET_FLOW_WAITSTOP;
        }
        break;

        case RESET_FLOW_WAITSTOP:
        {

            for (connector = 1; connector <= ocpp_chargePoint->numberOfConnector; connector++)
            {
                if (ocpp_chargePoint->transaction_obj[connector]->isTransaction)
                    break;
            }

            if (connector > ocpp_chargePoint->numberOfConnector)
                flow = RESET_FLOW_RESET;
        }
        break;

        case RESET_FLOW_RESET:
        {
            if (type == OCPP_PACKAGE_RESET_TYPE_SOFT)
            {
                ocpp_ConfigurationKey_deinit();
                OCPP_LOG_DEBUG("software reset");
            }
            else
            {
                OCPP_LOG_DEBUG("hard reset");
                usleep(5 * 1000 * 1000);
                system("reboot");
            }

            return;
        }
        break;

        default:
            break;
        }

        usleep(500 * 1000); // 500ms
    }
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {ocpp_package_Reset_req_t} reset
 * @return {*}
 */
void ocpp_chargePoint_manageResetRequest(const char *uniqueId, ocpp_package_Reset_req_t reset_req)
{

    ocpp_package_Reset_conf_t reset_conf;

    pthread_t tid_reset;
    if (pthread_create(&tid_reset, NULL, ocpp_chargePoint_Reset_thread, (void *)reset_req.type) == -1)
    {
        OCPP_LOG_ERROR("create reset thread fail");
    }
    pthread_detach(tid_reset);

    reset_conf.status = OCPP_PACKAGE_RESET_STATUS_ACCEPTED;
    char *lastUniqueId = strdup(uniqueId);
    char *message = ocpp_package_prepare_Reset_Response(lastUniqueId, &reset_conf);
    ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALL);
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {ocpp_package_SendLocalList_req_t *} sendLocalList
 * @return {*}
 */
void ocpp_chargePoint_manageSendLocalListRequest(const char *uniqueId, ocpp_package_SendLocalList_req_t *sendLocalList_req)
{

    ocpp_package_SendLocalList_conf_t sendLocalList_conf;
    memset(&sendLocalList_conf, 0, sizeof(sendLocalList_conf));

    sendLocalList_conf.status = OCPP_PCAKGE_UPDATE_STATUS_ACCEPTED;

    bool LocalAuthListEnabled = false;
    ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[OCPP_CK_LocalAuthListEnabled], (void *)LocalAuthListEnabled);
    if (LocalAuthListEnabled == false)
    {
        sendLocalList_conf.status = OCPP_PCAKGE_UPDATE_STATUS_NOTSUPPORTED;
    }
    else
    {

        if (sendLocalList_req->listVersion <= ocpp_localAuthorization_List_getListVersion2())
        {
            sendLocalList_conf.status = OCPP_PCAKGE_UPDATE_STATUS_VERSION_MISMATCH;
        }
        else
        {
            ocpp_localAuthorization_List_setListVersion(sendLocalList_req->listVersion);

            if (sendLocalList_req->UpdateType == OCPP_PACKAGE_UPDATE_TYPE_FULL)
            {
                ocpp_localAuthorization_List_clearList();
            }

            // 更新本地列表
            if (sendLocalList_req->numberOfAuthorizationData > 0 && sendLocalList_req->localAuthorizationListIsUse)
            {

                ocpp_localAuthorization_list_entry_t *entry = calloc(1, sizeof(ocpp_localAuthorization_list_entry_t));
                entry->expiryDate = calloc(1, sizeof(struct tm));

                int i = 0;
                for (i = 0; i < sendLocalList_req->numberOfAuthorizationData; i++)
                {
                    strncpy(entry->IdTag, sendLocalList_req->localAuthorizationList[i]->idTag, OCPP_AUTHORIZATION_IDTAG_LEN);

                    if (sendLocalList_req->localAuthorizationList[i]->idTagInfoIsUse)
                    {
                        if (sendLocalList_req->localAuthorizationList[i]->idTagInfo.parentIdTagIsUse)
                            strncpy(entry->parentIdTag, sendLocalList_req->localAuthorizationList[i]->idTagInfo.parentIdTag, OCPP_AUTHORIZATION_IDTAG_LEN);
                        if (sendLocalList_req->localAuthorizationList[i]->idTagInfo.expiryDateIsUse)
                            strptime(sendLocalList_req->localAuthorizationList[i]->idTagInfo.expiryDate, "%Y-%m-%dT%H:%M:%S", entry->expiryDate);

                        entry->status = sendLocalList_req->localAuthorizationList[i]->idTagInfo.AuthorizationStatus;
                    }
                    ocpp_localAuthorization_List_write(entry);
                }

                free(entry->expiryDate);
                free(entry);
            }
        }
    }

    // 释放内存(待验证)
    if (sendLocalList_req->numberOfAuthorizationData > 0 && sendLocalList_req->localAuthorizationListIsUse)
    {
        int i = 0;
        for (i = 0; i < sendLocalList_req->numberOfAuthorizationData; i++)
        {
            free(sendLocalList_req->localAuthorizationList[i]);
        }

        free(sendLocalList_req->localAuthorizationList);
    }

    char *lastUniqueId = strdup(uniqueId);
    char *message = ocpp_package_prepare_SendLocalList_Response(lastUniqueId, &sendLocalList_conf);
    ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALL);
}

void ocpp_chargePoint_manageSetChargingProfileRequest(const char *uniqueId, ocpp_package_SetChargingProfile_req_t *setChargingProfile)
{
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {ocpp_package_TriggerMessage_req_t} triggerMessage
 * @return {*}
 */
void ocpp_chargePoint_manageTriggerMessageRequest(const char *uniqueId, ocpp_package_TriggerMessage_req_t triggerMessage_req)
{

    ocpp_package_TriggerMessage_conf_t triggerMessage_conf;
    memset(&triggerMessage_conf, 0, sizeof(triggerMessage_conf));

    triggerMessage_conf.status = OCPP_PACKAGE_TRIGGER_MESSAGE_STATUS_ACCEPTED;
    char *lastUniqueId = strdup(uniqueId);
    char *message = ocpp_package_prepare_TriggerMessage_Response(lastUniqueId, &triggerMessage_conf);

    ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALL);

    switch (triggerMessage_req.requestedMessage)
    {
    case OCPP_PCAKGE_MESSAGE_TRIGGER_BOOTNOTIFICATION:
        ocpp_chargePoint_sendBootNotification_req();
        break;

    case OCPP_PCAKGE_MESSAGE_TRIGGER_DIAGNOSTICSSTATUS_NOTIFICATION:
        ocpp_chargePoint_sendDiagnosticsStatusNotification_Req();
        break;

    case OCPP_PCAKGE_MESSAGE_TRIGGER_FIRMWARESTATUS_NOTIFICATION:
        ocpp_chargePoint_sendFirmwareStatusNotification_Req();
        break;

    case OCPP_PCAKGE_MESSAGE_TRIGGER_HEARTBEAT:
        ocpp_chargePoint_sendHeartbeat_Req();
        break;

    case OCPP_PCAKGE_MESSAGE_TRIGGER_METERVALUES:
        if (triggerMessage_req.connectorIdIsUse)
            ocpp_chargePoint_sendMeterValues(triggerMessage_req.connectorId, 0);
        else
            ocpp_chargePoint_sendMeterValues(0, 0);
        break;

    case OCPP_PCAKGE_MESSAGE_TRIGGER_STATUS_NOTIFICATION:
        if (triggerMessage_req.connectorIdIsUse)
            ocpp_chargePoint_sendStatusNotification_Req(triggerMessage_req.connectorId);
        else
            ocpp_chargePoint_sendStatusNotification_Req(0);
        break;

    default:
        break;
    }
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {ocpp_package_UnlockConnector_req_t} unlockConnector
 * @return {*}
 */
void ocpp_chargePoint_manageUnlockConnectorRequest(const char *uniqueId, ocpp_package_UnlockConnector_req_t unlockConnector_req)
{
    ocpp_package_UnlockConnector_conf_t unlockConnector_conf;

    unlockConnector_conf.status = OCPP_PACKAGE_UNLOCK_STATUS_NOTSUPPORTED;

    char *lastUniqueId = strdup(uniqueId);
    char *message = ocpp_package_prepare_UnlockConnector_Response(lastUniqueId, &unlockConnector_conf);

    ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALL);
}

/**
 * @description: 固件更新线程
 * @param
 * @param
 * @return
 */
void *ocpp_chargePoint_UpdateFirmware_thread(void *arg)
{

    ocpp_chargePoint_UpdateFirmware_t *UpdateFirmware = (ocpp_chargePoint_UpdateFirmware_t *)arg;

    int retries = 0;
    struct hostent *hp;
    char server_filepath_name[256] = {0};

    for (; true;)
    {

        if (retries >= UpdateFirmware->retries)
        {
            ocpp_chargePoint->ocpp_firmwareUpdate_lastUpdateState = OCPP_PACKAGE_FIRMWARE_STATUS_DOWNLOAD_FAILED;
            ocpp_chargePoint_sendFirmwareStatusNotification_Req();
            break;
        }

        time_t now;
        time(&now);
        struct tm retrieveDate;
        strptime(UpdateFirmware->retrieveDate, "%Y-%m-%dT%H:%M:%S", &retrieveDate);
        time_t upDateTime = mktime(&retrieveDate);

        // 等待到达更新时间
        OCPP_LOG_DEBUG("等待到达更新时间");
        for (; difftime(upDateTime, now) > 0;)
        {
            usleep(10 * 1000 * 1000);
            time(&now);
        }

        // 如果当前有交易,则结束交易
        OCPP_LOG_DEBUG("如果当前有交易,则结束交易");
        int connector = 0;
        for (connector = 1; connector <= ocpp_chargePoint->numberOfConnector; connector++)
        {
            if (ocpp_chargePoint->transaction_obj[connector]->isTransaction)
            {
                ocpp_chargePoint->transaction_obj[connector]->isStop = true;
                ocpp_chargePoint->transaction_obj[connector]->reason = OCPP_PACKAGE_STOP_REASON_SOFTRESET;
            }
        }

        // 等待交易停止
        OCPP_LOG_DEBUG("等待交易停止");
        bool isStopAllTransaction = false;
        while (!isStopAllTransaction)
        {
            for (connector = 1; connector <= ocpp_chargePoint->numberOfConnector; connector++)
            {
                if (ocpp_chargePoint->transaction_obj[connector]->isTransaction)
                {
                    break;
                }
            }

            if (connector > ocpp_chargePoint->numberOfConnector)
            {
                isStopAllTransaction = true;
            }

            usleep(5 * 1000 * 1000);
        }

        // 连接DNS获取IP地址
        OCPP_LOG_DEBUG("连接DNS获取IP地址");
        for (; retries < UpdateFirmware->retries; retries++)
        {
            if ((hp = gethostbyname(UpdateFirmware->serverAddr)) != NULL)
            {
                break;
            }
            else
            {
                usleep(UpdateFirmware->retryInterval * 1000 * 1000);
                OCPP_LOG_DEBUG("DNS error = %s", strerror(errno));
            }
        }

        // 连接FTP服务器
        OCPP_LOG_DEBUG("连接FTP服务器");
        for (; retries < UpdateFirmware->retries; retries++)
        {
            if ((UpdateFirmware->client.control_sock = ocpp_ftp_connect_server((char *)hp->h_addr, UpdateFirmware->port)) >= 0)
            {
                break;
            }
            else
            {
                usleep(UpdateFirmware->retryInterval * 1000 * 1000);
                OCPP_LOG_DEBUG("UpdateFirmware connect FTP server fail");
            }
        }

        OCPP_LOG_DEBUG("登录FTP服务器");
        for (; retries < UpdateFirmware->retries; retries++)
        {
            if (ocpp_ftp_login_server(UpdateFirmware->client.control_sock, UpdateFirmware->client.usr, UpdateFirmware->client.passwd) >= 0)
            {
                break;
            }
            else
            {
                usleep(UpdateFirmware->retryInterval * 1000 * 1000);
                OCPP_LOG_DEBUG("UpdateFirmware login ftp server fail");
            }
        }

        OCPP_LOG_DEBUG("发送固件状态通知");
        if (retries < UpdateFirmware->retries)
        {
            ocpp_chargePoint->ocpp_firmwareUpdate_lastUpdateState = OCPP_PACKAGE_FIRMWARE_STATUS_DOWNLOADING;
            ocpp_chargePoint_sendFirmwareStatusNotification_Req();
            strcpy(server_filepath_name, UpdateFirmware->client.ser_filepath);
            strcat(server_filepath_name, UpdateFirmware->client.ser_filename);
            OCPP_LOG_DEBUG("server_filepath_name = %s", server_filepath_name);
        }

        OCPP_LOG_DEBUG("下载固件软件");
        for (; retries < UpdateFirmware->retries; retries++)
        {
            if (ocpp_ftp_down_file(UpdateFirmware->client.control_sock, server_filepath_name, OCPP_FIRMWARE_UPDATA_FILEPATH, 0, 0, CMD_RETR) > 0)
            {
                break;
            }
            else
            {
                usleep(UpdateFirmware->retryInterval * 1000 * 1000);
                OCPP_LOG_DEBUG("updateFirmware download fail");
            }
        }

        // 发送固件下载完成通知
        OCPP_LOG_DEBUG("发送固件下载完成通知");
        if (retries < UpdateFirmware->retries)
        {
            ocpp_chargePoint->ocpp_firmwareUpdate_lastUpdateState = OCPP_PACKAGE_FIRMWARE_STATUS_DOWNLOADED;
            ocpp_chargePoint_sendFirmwareStatusNotification_Req();

            usleep(5 * 1000 * 1000); // 等待消息发送出去
            // 重启升级固件
            system("reboot");
        }
    }

    free(UpdateFirmware);
    return NULL;
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {ocpp_package_UpdateFirmware_req_t} updateFirmware
 * @return {*}
 */
void ocpp_chargePoint_manageUpdateFirmwareRequest(const char *uniqueId, ocpp_package_UpdateFirmware_req_t updateFirmware_req)
{

    ocpp_package_UpdateFirmware_conf_t updateFirmware_conf;
    memset(&updateFirmware_conf, 0, sizeof(updateFirmware_conf));

    ocpp_chargePoint_UpdateFirmware_t *UpdateFirmware = (ocpp_chargePoint_UpdateFirmware_t *)calloc(1, sizeof(ocpp_chargePoint_UpdateFirmware_t));

    UpdateFirmware->retries = 3;
    if (updateFirmware_req.retriesIsUse)
        UpdateFirmware->retries = updateFirmware_req.retries;

    UpdateFirmware->retryInterval = 120;
    if (updateFirmware_req.retryIntervalIsUse)
        UpdateFirmware->retryInterval = updateFirmware_req.retryInterval;

    strncpy(UpdateFirmware->retrieveDate, updateFirmware_req.retrieveDate, 32);

    //"location":"ftp://0209GCpBn7:523Fw6FS@f1.iot17.com:8183/smart/2681b667733cd919e8iq55/diagnostics/1686016382951893/",
    char *index_start = NULL;
    char *index_stop = NULL;

    // extract username
    index_start = strchr(updateFirmware_req.location, '/') + 2;
    index_stop = strchr(index_start, ':');
    strncpy(UpdateFirmware->client.usr, index_start, index_stop - index_start);
    UpdateFirmware->client.usr[index_stop - index_start + 1] = '\0';
    OCPP_LOG_DEBUG("client.usr = %s", UpdateFirmware->client.usr);

    // extract password
    index_start = index_stop + 1;
    index_stop = strchr(index_start, '@');
    strncpy(UpdateFirmware->client.passwd, index_start, index_stop - index_start);
    UpdateFirmware->client.passwd[index_stop - index_start + 1] = '\0';
    OCPP_LOG_DEBUG("client.passwd = %s", UpdateFirmware->client.passwd);

    // extract server address
    index_start = index_stop + 1;
    index_stop = strchr(index_start, ':');
    strncpy(UpdateFirmware->serverAddr, index_start, index_stop - index_start);
    UpdateFirmware->serverAddr[index_stop - index_start + 1] = '\0';
    OCPP_LOG_DEBUG("serverAddr = %s", UpdateFirmware->serverAddr);

    // extract port
    index_start = index_stop + 1;
    index_stop = strchr(index_start, '/');
    char port_str[10] = {0};
    strncpy(port_str, index_start, index_stop - index_start);
    port_str[index_stop - index_start + 1] = '\0';
    UpdateFirmware->port = atoi(port_str);
    OCPP_LOG_DEBUG("port = %d", UpdateFirmware->port);

    // extract path
    index_start = index_stop;
    strcpy(UpdateFirmware->client.ser_filepath, index_start);
    OCPP_LOG_DEBUG("path = %s", UpdateFirmware->client.ser_filepath);

    // extract filename
    strcpy(UpdateFirmware->client.ser_filename, OCPP_LOGS_FILENAME);
    OCPP_LOG_DEBUG("filename = %s", UpdateFirmware->client.ser_filename);

    pthread_t tid_firmware;
    if (pthread_create(&tid_firmware, NULL, ocpp_chargePoint_UpdateFirmware_thread, (void *)UpdateFirmware) == -1)
    {
        OCPP_LOG_ERROR("create firware thread fail");
    }
    pthread_detach(tid_firmware);

    char *lastUniqueId = strdup(uniqueId);
    char *message = ocpp_package_prepare_UpdateFirmware_Response(lastUniqueId, &updateFirmware_conf);
    ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALL);
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

/**
 * @description:
 * @param {char *} uniqueId
 * @param {json_object} *payload_obj
 * @return {*}
 */
static bool ocpp_chargePoint_checkCancelReservation_Callpackage(const char *uniqueId, json_object *payload_obj)
{
    bool isError = true;

    json_object *reservationId_obj = json_object_object_get(payload_obj, "reservationId");

    ocpp_package_CallError_t callError;
    memset(&callError, 0, sizeof(callError));
    callError.errorDescriptionIsUse = 1;
    callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_OCCURENCE_CONSTRAINT_VIOLATION;

    if (reservationId_obj == NULL)
    {
        snprintf(callError.errorDescription, 1024, "reservationId SHELL be not NULL");
    }
    else
    {
        callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_TYPE_CONSTRAINT_VIOLATION;
        if (json_object_is_type(reservationId_obj, json_type_int) == false)
        {
            snprintf(callError.errorDescription, 1024, "reservationId SHELL be integer type");
        }
        else
        {
            isError = false;
        }
    }

    if (isError)
    {
        char *lastUniqueId = strdup(uniqueId);
        char *message = ocpp_package_prepare_CallError(lastUniqueId, &callError);
        ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALLERROR);
    }

    return isError;
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {json_object} *payload_obj
 * @return {*}
 */
static bool ocpp_chargePoint_checkChangeAvailablility_Callpackage(const char *uniqueId, json_object *payload_obj)
{

    json_object *connectorId_obj = json_object_object_get(payload_obj, "connectorId");
    json_object *type_obj = json_object_object_get(payload_obj, "type");

    bool isError = true;
    ocpp_package_CallError_t callError;
    memset(&callError, 0, sizeof(callError));

    callError.errorDescriptionIsUse = 1;
    callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_OCCURENCE_CONSTRAINT_VIOLATION;
    if (connectorId_obj == NULL)
    {
        snprintf(callError.errorDescription, 1024, "%s", "connector is NULL");
    }
    else if (type_obj == NULL)
    {
        snprintf(callError.errorDescription, 1024, "%s", "type is NULL");
    }
    else
    {
        callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_TYPE_CONSTRAINT_VIOLATION;
        if (json_object_is_type(connectorId_obj, json_type_int) == false)
        {

            snprintf(callError.errorDescription, 1024, "connector SHELL be integer type");
        }
        else if (json_object_is_type(type_obj, json_type_string) == false)
        {

            snprintf(callError.errorDescription, 1024, "type SHELL be enumeration string type");
        }
        else
        {
            callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_PROPERTY_CONSTRAINT_VIOLATION;
            int connector = json_object_get_int(connectorId_obj);
            const char *type_str = json_object_get_string(type_obj);

            if (connector < 0 || connector > ocpp_chargePoint->numberOfConnector)
            {

                snprintf(callError.errorDescription, 1024, "connector SHELL be >= 0 and <= %d", ocpp_chargePoint->numberOfConnector);
            }
            else
            {
                int i = 0;
                for (i = 0; i < OCPP_PACKAGE_AVAILABILITY_STATUS_MAX; i++)
                {
                    if (strcasecmp(type_str, ocpp_package_AvailabilityType_text[i]) == 0)
                        break;
                }

                if (i >= OCPP_PACKAGE_AVAILABILITY_STATUS_MAX)
                {
                    snprintf(callError.errorDescription, 1024, "type SHELL be Inoperative or Operative");
                }
                else
                {
                    isError = false;
                }
            }
        }
    }

    if (isError)
    {
        char *lastUniqueId = strdup(uniqueId);
        char *message = ocpp_package_prepare_CallError(lastUniqueId, &callError);
        ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALLERROR);
    }

    return isError;
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {json_object} *payload_obj
 * @return {*}
 */
static bool ocpp_chargePoint_checkChangeConfiguration_Callpackage(const char *uniqueId, json_object *payload_obj)
{

    json_object *key_obj = json_object_object_get(payload_obj, "key");
    json_object *value_obj = json_object_object_get(payload_obj, "value");

    bool isError = true;

    ocpp_package_CallError_t callError;
    memset(&callError, 0, sizeof(callError));

    callError.errorDescriptionIsUse = 1;
    callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_OCCURENCE_CONSTRAINT_VIOLATION;
    if (key_obj == NULL)
    {
        snprintf(callError.errorDescription, 1024, "Key is NULL");
    }
    else if (value_obj == NULL)
    {
        snprintf(callError.errorDescription, 1024, "value is NULL");
    }
    else
    {
        callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_TYPE_CONSTRAINT_VIOLATION;
        if (json_object_is_type(key_obj, json_type_string) == false)
        {
            snprintf(callError.errorDescription, 1024, "key SHELL be integer type");
        }
        else if (json_object_is_type(value_obj, json_type_string) == false)
        {
            snprintf(callError.errorDescription, 1024, "value SHELL be integer type");
        }
        else
        {
            callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_PROPERTY_CONSTRAINT_VIOLATION;
            const char *key_str = json_object_get_string(key_obj);
            const char *value_str = json_object_get_string(value_obj);

            if (strlen(key_str) > 50)
            {
                snprintf(callError.errorDescription, 1024, "key length SHELL be CiString50Type");
            }
            else if (strlen(value_str) > 500)
            {
                snprintf(callError.errorDescription, 1024, "value length SHELL be CiString500Type");
            }
            else
            {
                isError = false;
            }
        }
    }

    if (isError)
    {
        char *lastUniqueId = strdup(uniqueId);
        char *message = ocpp_package_prepare_CallError(lastUniqueId, &callError);
        ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALLERROR);
    }

    return isError;
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {json_object *} payload_obj
 * @return {*}
 */
static bool ocpp_chargePoint_checkClearCache_Callpackage(const char *uniqueId, json_object *payload_obj)
{

    return false;
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {json_object *} payload_obj
 * @return {*}
 */
static bool ocpp_chargePoint_checkClearChargingProfile_Callpackage(const char *uniqueId, json_object *payload_obj)
{

    bool isError = true;

    json_object *id_obj = json_object_object_get(payload_obj, "id");
    json_object *connectorId_obj = json_object_object_get(payload_obj, "connectorId");
    json_object *chargingProfilePurpose_obj = json_object_object_get(payload_obj, "chargingProfilePurpose");
    json_object *stackLevel_obj = json_object_object_get(payload_obj, "stackLevel");

    ocpp_package_CallError_t callError;
    memset(&callError, 0, sizeof(callError));

    callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_TYPE_CONSTRAINT_VIOLATION;
    callError.errorDescriptionIsUse = 1;

    if (id_obj != NULL && json_object_is_type(id_obj, json_type_int) == false)
    {
        snprintf(callError.errorDescription, 1024, "id SHELL be integer type");
    }
    else if (connectorId_obj != NULL && json_object_is_type(connectorId_obj, json_type_int) == false)
    {
        snprintf(callError.errorDescription, 1024, "connectorId SHELL be integer type");
    }
    else if (chargingProfilePurpose_obj != NULL && json_object_is_type(chargingProfilePurpose_obj, json_type_string) == false)
    {
        snprintf(callError.errorDescription, 1024, "chargingProfilePurpose SHELL be enumerate string type");
    }
    else if (stackLevel_obj != NULL && json_object_is_type(stackLevel_obj, json_type_int) == false)
    {
        snprintf(callError.errorDescription, 1024, "stackLevel SHELL be integer type");
    }
    else
    {

        callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_PROPERTY_CONSTRAINT_VIOLATION;
        if (connectorId_obj != NULL)
        {
            int connectorId = json_object_get_int(connectorId_obj);
            if (connectorId > ocpp_chargePoint->numberOfConnector)
            {
                snprintf(callError.errorDescription, 1024, "connectorId SHELL less than %d", ocpp_chargePoint->numberOfConnector);
            }
            else
            {
                isError = false;
            }
        }

        if (isError == false)
        {

            if (chargingProfilePurpose_obj != NULL)
            {
                const char *chargingProfilePurpose_str = json_object_get_string(chargingProfilePurpose_obj);
                int i = 0;
                for (i = 0; i < OCPP_PACKAGE_CHARGING_PROFILE_PURPOSE_TYPE_MAX; i++)
                {
                    if (strcmp(chargingProfilePurpose_str, ocpp_package_ChargingProfilePurposeType_text[i]) == 0)
                        break;
                }

                if (i >= OCPP_PACKAGE_CHARGING_PROFILE_PURPOSE_TYPE_MAX)
                {
                    snprintf(callError.errorDescription, 1024, "chargingProfilePurpose SHELL be ChargePointMaxProfile 、TxDefaultProfile or TxProfile");
                    isError = true;
                }
            }
        }
    }

    if (isError)
    {
        char *lastUniqueId = strdup(uniqueId);
        char *message = ocpp_package_prepare_CallError(lastUniqueId, &callError);
        ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALLERROR);
    }

    return isError;
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {json_object *} payload_obj
 * @return {*}
 */
static bool ocpp_chargePoint_checkDataTransfer_Callpackage(const char *uniqueId, json_object *payload_obj)
{

    return false;
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {json_object *} payload_obj
 * @return {*}
 */
static bool ocpp_chargePoint_checkGetCompositeSchedule_Callpackage(const char *uniqueId, json_object *payload_obj)
{

    bool isError = true;

    json_object *connectorId_obj = json_object_object_get(payload_obj, "connectorId");
    json_object *duration_obj = json_object_object_get(payload_obj, "duration");
    json_object *chargingRateUnit_obj = json_object_object_get(payload_obj, "chargingRateUnit");

    ocpp_package_CallError_t callError;
    memset(&callError, 0, sizeof(callError));

    callError.errorDescriptionIsUse = 1;
    callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_OCCURENCE_CONSTRAINT_VIOLATION;

    if (connectorId_obj == NULL)
    {
        snprintf(callError.errorDescription, 1024, "connectorId SHELL be not NULL");
    }
    else if (duration_obj == NULL)
    {
        snprintf(callError.errorDescription, 1024, "duration SHELL be not NULL");
    }
    else
    {
        callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_TYPE_CONSTRAINT_VIOLATION;

        if (json_object_is_type(connectorId_obj, json_type_int) == false)
        {
            snprintf(callError.errorDescription, 1024, "connectorId SHELL be integer type");
        }
        else if (json_object_is_type(duration_obj, json_type_int) == false)
        {
            snprintf(callError.errorDescription, 1024, "duration SHELL be integer type");
        }
        else if (chargingRateUnit_obj != NULL && json_object_is_type(chargingRateUnit_obj, json_type_string) == false)
        {
            snprintf(callError.errorDescription, 1024, "chargingRateUnit SHELL be enumerate string type");
        }
        else
        {

            callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_PROPERTY_CONSTRAINT_VIOLATION;

            int connectorId = json_object_get_int(connectorId_obj);
            if (connectorId > ocpp_chargePoint->numberOfConnector)
            {
                snprintf(callError.errorDescription, 1024, "connectorId SHELL be less than %d", ocpp_chargePoint->numberOfConnector);
            }
            else if (chargingRateUnit_obj != NULL)
            {
                const char *chargingRateUnit_str = json_object_get_string(chargingRateUnit_obj);
                int i = 0;
                for (i = 0; i < OCPP_PACKAGE_CHARGING_RATE_UNIT_TYPE_MAX; i++)
                {
                    if (strcmp(chargingRateUnit_str, ocpp_package_ChargingRateUnitType_text[i]) == 0)
                        break;
                }

                if (i > OCPP_PACKAGE_CHARGING_RATE_UNIT_TYPE_MAX)
                {
                    snprintf(callError.errorDescription, 1024, "chargingRateUnit SHELL be W or A");
                }
                else
                {
                    isError = false;
                }
            }
            else
            {
                isError = false;
            }
        }
    }

    if (isError)
    {
        char *lastUniqueId = strdup(uniqueId);
        char *message = ocpp_package_prepare_CallError(lastUniqueId, &callError);
        ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALLERROR);
    }

    return isError;
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {json_object *} payload_obj
 * @return {*}
 */
static bool ocpp_chargePoint_checkGetConfiguration_Callpackage(const char *uniqueId, json_object *payload_obj)
{

    json_object *key_obj = json_object_object_get(payload_obj, "key");

    bool isError = true;

    ocpp_package_CallError_t callError;
    memset(&callError, 0, sizeof(callError));

    callError.errorDescriptionIsUse = 1;
    callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_OCCURENCE_CONSTRAINT_VIOLATION;

    if (key_obj == NULL)
    {
        isError = false;
    }
    else
    {
        callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_TYPE_CONSTRAINT_VIOLATION;
        if (json_object_is_type(key_obj, json_type_array) == false)
        {
            snprintf(callError.errorDescription, 1024, "key SHELL be array type");
        }
        else
        {
            callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_PROPERTY_CONSTRAINT_VIOLATION;

            int numberOfKey = json_object_array_length(key_obj);
            int index = 0;
            for (index = 0; index < numberOfKey; index++)
            {
                json_object *key_array = json_object_array_get_idx(key_obj, index);
                const char *key_str = json_object_get_string(key_array);

                if (strlen(key_str) > 50)
                    break;
            }

            if (index >= numberOfKey)
                isError = false;
        }
    }

    if (isError)
    {
        char *lastUniqueId = strdup(uniqueId);
        char *message = ocpp_package_prepare_CallError(lastUniqueId, &callError);
        ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALLERROR);
    }

    return isError;
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {json_object *} payload_obj
 * @return {*}
 */
static bool ocpp_chargePoint_checkGetDiagnostics_Callpackage(const char *uniqueId, json_object *payload_obj)
{

    bool isError = true;

    json_object *location_obj = json_object_object_get(payload_obj, "location");
    json_object *retries_obj = json_object_object_get(payload_obj, "retries");
    json_object *retryInterval_obj = json_object_object_get(payload_obj, "retryInterval");
    json_object *startTime_obj = json_object_object_get(payload_obj, "startTime");
    json_object *stopTime_obj = json_object_object_get(payload_obj, "stopTime");

    ocpp_package_CallError_t callError;
    memset(&callError, 0, sizeof(callError));

    callError.errorDescriptionIsUse = 1;
    callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_OCCURENCE_CONSTRAINT_VIOLATION;

    if (location_obj == NULL)
    {
        snprintf(callError.errorDescription, 1024, "location SHELL be not NULL");
    }
    else
    {
        callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_TYPE_CONSTRAINT_VIOLATION;

        if (json_object_is_type(location_obj, json_type_string) == false)
        {

            snprintf(callError.errorDescription, 1024, "location SHELL be string type");
        }
        else if (retries_obj != NULL && json_object_is_type(retries_obj, json_type_int) == false)
        {

            snprintf(callError.errorDescription, 1024, "retries SHELL be integer type");
        }
        else if (retryInterval_obj != NULL && json_object_is_type(retryInterval_obj, json_type_int) == false)
        {

            snprintf(callError.errorDescription, 1024, "retryInterval SHELL be integer type");
        }
        else if (startTime_obj != NULL && json_object_is_type(startTime_obj, json_type_string) == false)
        {

            snprintf(callError.errorDescription, 1024, "startTime SHELL be string type");
        }
        else if (stopTime_obj != NULL && json_object_is_type(stopTime_obj, json_type_string) == false)
        {

            snprintf(callError.errorDescription, 1024, "stopTime SHELL be string type");
        }
        else
        {

            callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_PROPERTY_CONSTRAINT_VIOLATION;
            isError = false;
        }
    }

    if (isError)
    {
        char *lastUniqueId = strdup(uniqueId);
        char *message = ocpp_package_prepare_CallError(lastUniqueId, &callError);
        ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALLERROR);
    }

    return isError;
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {json_object *} payload_obj
 * @return {*}
 */
static bool ocpp_chargePoint_checkGetLocalListVersion_Callpackage(const char *uniqueId, json_object *payload_obj)
{

    return false;
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {json_object} *payload_obj
 * @return {*}
 */
static bool ocpp_chargePoint_checkRemoteStartTransaction_Callpackage(const char *uniqueId, json_object *payload_obj)
{

    json_object *idTag_obj = json_object_object_get(payload_obj, "idTag");
    json_object *connectorId_obj = json_object_object_get(payload_obj, "connectorId");
    json_object *chargingProfile_obj = json_object_object_get(payload_obj, "chargingProfile");

    bool isError = false;

    ocpp_package_CallError_t callError;
    memset(&callError, 0, sizeof(callError));
    callError.errorDescriptionIsUse = 1;

    if (idTag_obj == NULL)
    {
        isError = true;
        callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_PROTOCOL_ERROR;
        snprintf(callError.errorDescription, 1024, "%s", "IdTag is NULL");
    }
    else if (connectorId_obj != NULL)
    {
        if (json_object_get_int(connectorId_obj) <= 0)
        {
            isError = true;
            callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_PROPERTY_CONSTRAINT_VIOLATION;
            snprintf(callError.errorDescription, 1024, "%s", "connectorId SHALL be > 0");
        }
    }
    else if (chargingProfile_obj != NULL)
    {
    }

    if (isError)
    {
        char *lastUniqueId = strdup(uniqueId);
        char *message = ocpp_package_prepare_CallError(lastUniqueId, &callError);
        ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALLERROR);
    }

    return isError;
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {json_object *} payload_obj
 * @return {*}
 */
static bool ocpp_chargePoint_checkRemoteStopTransaction_Callpackage(const char *uniqueId, json_object *payload_obj)
{

    bool isError = false;

    json_object *transactionId_obj = json_object_object_get(payload_obj, "transactionId");

    ocpp_package_CallError_t callError;
    memset(&callError, 0, sizeof(callError));
    callError.errorDescriptionIsUse = 1;

    if (transactionId_obj == NULL)
    {
        isError = true;
        callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_OCCURENCE_CONSTRAINT_VIOLATION;
        snprintf(callError.errorDescription, 1024, "%s", "transactionId is NULL");
    }

    if (isError)
    {
        char *lastUniqueId = strdup(uniqueId);
        char *message = ocpp_package_prepare_CallError(lastUniqueId, &callError);
        ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALLERROR);
    }

    return isError;
}

/**
 * @description:
 * @param {char} *uniqueId
 * @param {json_object *} payload_obj
 * @return {*}
 */
static bool ocpp_chargePoint_checkReserveNow_Callpackage(const char *uniqueId, json_object *payload_obj)
{
    bool isError = true;

    json_object *connectorId_obj = json_object_object_get(payload_obj, "connectorId");
    json_object *expiryDate_obj = json_object_object_get(payload_obj, "expiryDate");
    json_object *idTag_obj = json_object_object_get(payload_obj, "idTag");
    json_object *parentIdTag_obj = json_object_object_get(payload_obj, "parentIdTag");
    json_object *reservationId_obj = json_object_object_get(payload_obj, "reservationId");

    ocpp_package_CallError_t callError;
    memset(&callError, 0, sizeof(callError));

    callError.errorDescriptionIsUse = 1;
    callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_OCCURENCE_CONSTRAINT_VIOLATION;

    if (connectorId_obj == NULL)
    {
        snprintf(callError.errorDescription, 1024, "connectorId SHELL be not NULL");
    }
    else if (expiryDate_obj == NULL)
    {
        snprintf(callError.errorDescription, 1024, "expiryDate SHELL be not NULL");
    }
    else if (idTag_obj == NULL)
    {
        snprintf(callError.errorDescription, 1024, "IdTag SHELL be not NULL");
    }
    else if (reservationId_obj == NULL)
    {
        snprintf(callError.errorDescription, 1024, "reservationId SHELL be not NULL");
    }
    else
    {

        callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_TYPE_CONSTRAINT_VIOLATION;
        if (json_object_is_type(connectorId_obj, json_type_int) == false)
        {
            snprintf(callError.errorDescription, 1024, "connectorId SHELL be integer type");
        }
        else if (json_object_is_type(expiryDate_obj, json_type_string) == false)
        {
            snprintf(callError.errorDescription, 1024, "expiryDate SHELL be string type");
        }
        else if (json_object_is_type(idTag_obj, json_type_string) == false)
        {
            snprintf(callError.errorDescription, 1024, "idTag SHELL be string type");
        }
        else if (json_object_is_type(reservationId_obj, json_type_int) == false)
        {
            snprintf(callError.errorDescription, 1024, "reservationId SHELL be integer type");
        }
        else if (parentIdTag_obj != NULL && json_object_is_type(parentIdTag_obj, json_type_string) == false)
        {
            snprintf(callError.errorDescription, 1024, "parentIdTag SHELL be string type");
        }
        else
        {

            int connectorId = json_object_get_int(connectorId_obj);
            int reservationId = json_object_get_int(reservationId_obj);

            if (connectorId < 0 || connectorId > ocpp_chargePoint->numberOfConnector)
            {
                snprintf(callError.errorDescription, 1024, "connectorId SHELL be > 0 and <= %d", ocpp_chargePoint->numberOfConnector);
            }
            else if (reservationId < 0)
            {
                snprintf(callError.errorDescription, 1024, "reservationId SHELL be > 0");
            }
            else
            {
                isError = false;
            }
        }
    }

    if (isError)
    {
        char *lastUniqueId = strdup(uniqueId);
        char *message = ocpp_package_prepare_CallError(lastUniqueId, &callError);
        ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALLERROR);
    }

    return isError;
}

/**
 * @description:
 * @param {char} *uniqueId
 * @param {json_object *} payload_obj
 * @return {*}
 */
static bool ocpp_chargePoint_checkReset_Callpackage(const char *uniqueId, json_object *payload_obj)
{

    bool isError = true;

    json_object *type_obj = json_object_object_get(payload_obj, "type");

    ocpp_package_CallError_t callError;
    memset(&callError, 0, sizeof(callError));

    callError.errorDescriptionIsUse = 1;
    callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_OCCURENCE_CONSTRAINT_VIOLATION;

    if (type_obj == NULL)
    {
        snprintf(callError.errorDescription, 1024, "type SHELL be not NULL");
    }
    else
    {
        callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_TYPE_CONSTRAINT_VIOLATION;
        if (json_object_is_type(type_obj, json_type_string) == false)
        {

            snprintf(callError.errorDescription, 1024, "type SHELL be enumerate type");
        }
        else
        {
            callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_PROPERTY_CONSTRAINT_VIOLATION;
            const char *type_str = json_object_get_string(type_obj);

            int index = 0;
            for (index = 0; index < OCPP_PACKAGE_RESET_TYPE_MAX; index++)
            {

                if (strcmp(ocpp_package_ResetType_text[index], type_str) == 0)
                    break;
            }

            if (index >= OCPP_PACKAGE_RESET_TYPE_MAX)
            {
                snprintf(callError.errorDescription, 1024, "type SHELL be soft or hard");
            }
            else
            {

                isError = false;
            }
        }
    }

    if (isError)
    {
        char *lastUniqueId = strdup(uniqueId);
        char *message = ocpp_package_prepare_CallError(lastUniqueId, &callError);
        ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALLERROR);
    }

    return isError;
}

/**
 * @description:
 * @param {char} *uniqueId
 * @param {json_object *} payload_obj
 * @return {*}
 */
static bool ocpp_chargePoint_checkSendLocalList_Callpackage(const char *uniqueId, json_object *payload_obj)
{

    json_object *listVersion_obj = json_object_object_get(payload_obj, "listVersion");
    json_object *localAuthorizationList_obj = json_object_object_get(payload_obj, "localAuthorizationList");
    json_object *updateType_obj = json_object_object_get(payload_obj, "updateType");
    int numberOfList = json_object_array_length(localAuthorizationList_obj);

    bool isError = true;

    ocpp_package_CallError_t callError;
    callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_OCCURENCE_CONSTRAINT_VIOLATION;
    callError.errorDescriptionIsUse = 1;

    if (listVersion_obj == NULL)
    {
        snprintf(callError.errorDescription, 1024, "listVersion SHELL be not NULL");
    }
    else if (updateType_obj == NULL)
    {
        snprintf(callError.errorDescription, 1024, "updateType SHELL be not NULL");
    }
    else if (numberOfList > 0)
    {

        int i = 0;
        for (i = 0; i < numberOfList; i++)
        {
            json_object *localAuthorizetion_obj = json_object_array_get_idx(localAuthorizationList_obj, i);
            json_object *idTag_obj = json_object_object_get(localAuthorizetion_obj, "idTag");
            json_object *idTagInfo_obj = json_object_object_get(localAuthorizetion_obj, "idTagInfo");

            if (idTag_obj == NULL)
            {
                snprintf(callError.errorDescription, 1024, "IdTag SHELL be not NULL");
                break;
            }
            else if (idTagInfo_obj != NULL)
            {
                json_object *expiryDate_obj = json_object_object_get(idTagInfo_obj, "expiryDate");
                json_object *parentIdTag_obj = json_object_object_get(idTagInfo_obj, "parentIdTag");
                json_object *status_obj = json_object_object_get(idTagInfo_obj, "status");

                if (status_obj == NULL)
                {
                    snprintf(callError.errorDescription, 1024, "status SHELL be not NULL");
                    break;
                }
            }
        }

        if (i >= numberOfList)
        {
            callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_TYPE_CONSTRAINT_VIOLATION;

            if (json_object_is_type(listVersion_obj, json_type_int) == false)
            {
                snprintf(callError.errorDescription, 1024, "listVersion SHELL be integer type");
            }
            else if (json_object_is_type(updateType_obj, json_type_string) == false)
            {
                snprintf(callError.errorDescription, 1024, "updateType SHELL be enumration type or string type");
            }
            else
            {

                isError = false;
            }
        }
    }
    else
    {
        isError = false;
    }

    if (isError)
    {
        char *lastniqueId = strdup(uniqueId);
        char *message = ocpp_package_prepare_CallError(lastniqueId, &callError);
        ocpp_chargePoint_send(lastniqueId, message, OCPP_PACKAGE_CALLERROR);
    }

    return isError;
}

/**
 * @description:
 * @param {char} *uniqueId
 * @param {json_object *} payload_obj
 * @return {*}
 */
static bool ocpp_chargePoint_checkSetChargingProfile_Callpackage(const char *uniqueId, json_object *payload_obj)
{
    bool isError = true;
    ocpp_package_CallError_t callError;
    memset(&callError, 0, sizeof(callError));
    callError.errorDescriptionIsUse = 1;
    callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_OCCURENCE_CONSTRAINT_VIOLATION;

    json_object *connectorId_obj = json_object_object_get(payload_obj, "connectorId");
    json_object *csChargingProfiles_obj = json_object_object_get(payload_obj, "csChargingProfiles");

    if (connectorId_obj == NULL)
    {
        snprintf(callError.errorDescription, 1024, "connectorId SHELL be not NULL");
    }
    else if (csChargingProfiles_obj == NULL)
    {
        snprintf(callError.errorDescription, 1024, "csChargingProfiles SHELL be not NULL");
    }
    else
    {

        json_object *chargingProfileId_obj = json_object_object_get(csChargingProfiles_obj, "chargingProfileId");
        json_object *transactionId_obj = json_object_object_get(csChargingProfiles_obj, "transactionId");
        json_object *stackLevel_obj = json_object_object_get(csChargingProfiles_obj, "stackLevel");
        json_object *chargingProfilePurpose_obj = json_object_object_get(csChargingProfiles_obj, "chargingProfilePurpose");
        json_object *chargingProfileKind_obj = json_object_object_get(csChargingProfiles_obj, "chargingProfileKind");
        json_object *recurrencyKind_obj = json_object_object_get(csChargingProfiles_obj, "recurrencyKind");
        json_object *validFrom_obj = json_object_object_get(csChargingProfiles_obj, "validFrom");
        json_object *validTo_obj = json_object_object_get(csChargingProfiles_obj, "validTo");
        json_object *chargingSchedule_obj = json_object_object_get(csChargingProfiles_obj, "chargingSchedule");

        if (chargingProfileId_obj == NULL)
        {
            snprintf(callError.errorDescription, 1024, "chargingProfileId SHELL be not NULL");
        }
        else if (stackLevel_obj == NULL)
        {
            snprintf(callError.errorDescription, 1024, "stackLevel SHELL be not NULL");
        }
        else if (chargingProfilePurpose_obj == NULL)
        {
            snprintf(callError.errorDescription, 1024, "chargingProfilePurpose SHELL be not NULL");
        }
        else if (chargingProfileKind_obj == NULL)
        {
            snprintf(callError.errorDescription, 1024, "chargingProfileKind SHELL be not NULL");
        }
        else if (chargingSchedule_obj == NULL)
        {
            snprintf(callError.errorDescription, 1024, "chargingSchedule SHELL be not NULL");
        }
        else
        {
            json_object *duration_obj = json_object_object_get(chargingSchedule_obj, "duration");
            json_object *startSchedule_obj = json_object_object_get(chargingSchedule_obj, "startSchedule");
            json_object *chargingRateUnit_obj = json_object_object_get(chargingSchedule_obj, "chargingRateUnit");
            json_object *chargingSchedulePeriod_obj = json_object_object_get(chargingSchedule_obj, "chargingSchedulePeriod");
            json_object *minChargingRate_obj = json_object_object_get(chargingSchedule_obj, "minChargingRate");

            if (chargingRateUnit_obj == NULL)
            {
                snprintf(callError.errorDescription, 1024, "chargingRateUnit SHELL be not NULL");
            }
            else if (chargingSchedulePeriod_obj == NULL)
            {
                snprintf(callError.errorDescription, 1024, "chargingSchedulePeriod SHELL be not NULL");
            }
            else
            {
                int num = json_object_array_length(chargingSchedulePeriod_obj);
                int i = 0;
                for (i = 0; i < num; i++)
                {
                    json_object *chargingSchedulePeriod_index_obj = json_object_array_get_idx(chargingSchedulePeriod_obj, i);
                    json_object *startPeriod_obj = json_object_object_get(chargingSchedulePeriod_index_obj, "startPeriod");
                    json_object *limit_obj = json_object_object_get(chargingSchedulePeriod_index_obj, "limit");
                    json_object *numberPhases_obj = json_object_object_get(chargingSchedulePeriod_index_obj, "numberPhases");

                    if (startPeriod_obj == NULL)
                    {
                        snprintf(callError.errorDescription, 1024, "startPeriod SHELL be not NULL");
                        break;
                    }
                    else if (limit_obj == NULL)
                    {
                        snprintf(callError.errorDescription, 1024, "limit SHELL be not NULL");
                        break;
                    }
                }

                if (i >= num)
                {
                    callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_TYPE_CONSTRAINT_VIOLATION;

                    isError = false;
                }
            }
        }
    }

    if (isError)
    {
        char *lastUniqueId = strdup(uniqueId);
        char *message = ocpp_package_prepare_CallError(lastUniqueId, &callError);
        ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALLERROR);
    }

    return isError;
}

/**
 * @description:
 * @param {char} *uniqueId
 * @param {json_object *} payload_obj
 * @return {*}
 */
static bool ocpp_chargePoint_checkTriggerMessage_Callpackage(const char *uniqueId, json_object *payload_obj)
{

    bool isError = true;

    json_object *requestedMessage_obj = json_object_object_get(payload_obj, "requestedMessage");
    json_object *connectorId_obj = json_object_object_get(payload_obj, "connectorId");

    ocpp_package_CallError_t callError;
    memset(&callError, 0, sizeof(callError));

    callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_OCCURENCE_CONSTRAINT_VIOLATION;
    callError.errorDescriptionIsUse = 1;

    if (requestedMessage_obj == NULL)
    {

        snprintf(callError.errorDescription, 1024, "requestedMessage SHELL be not NULL");
    }
    else
    {

        callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_TYPE_CONSTRAINT_VIOLATION;
        if (json_object_is_type(requestedMessage_obj, json_type_string) == false)
        {

            snprintf(callError.errorDescription, 1024, "requestedMessage SHELL be enumerate string type");
        }
        else if (connectorId_obj != NULL && json_object_is_type(connectorId_obj, json_type_int) == false)
        {

            snprintf(callError.errorDescription, 1024, "connectorId SHELL be integer type");
        }
        else
        {

            callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_PROPERTY_CONSTRAINT_VIOLATION;

            const char *requestedMessage_str = json_object_get_string(requestedMessage_obj);
            int i = 0;
            for (i = 0; i < OCPP_PACKGE_MESSAGE_TRIGGER_STATUS_MAX; i++)
            {
                if (strcmp(ocpp_package_MessageTrigger_text[i], requestedMessage_str) == 0)
                    break;
            }

            if (i >= OCPP_PACKGE_MESSAGE_TRIGGER_STATUS_MAX)
            {
                snprintf(callError.errorDescription, 1024, "requestedMessage not support");
            }
            else if (connectorId_obj != NULL && json_object_get_int(connectorId_obj) > ocpp_chargePoint->numberOfConnector)
            {
                snprintf(callError.errorDescription, 1024, "connectorId not support");
            }
            else
            {
                isError = false;
            }
        }
    }

    if (isError)
    {
        char *lastUniqueId = strdup(uniqueId);
        char *message = ocpp_package_prepare_CallError(lastUniqueId, &callError);
        ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALLERROR);
    }

    return isError;
}

/**
 * @description:
 * @param {char} *uniqueId
 * @param {json_object *} payload_obj
 * @return {*}
 */
static bool ocpp_chargePoint_checkUnlockConnector_Callpackage(const char *uniqueId, json_object *payload_obj)
{

    json_object *connectorId_obj = json_object_object_get(payload_obj, "connectorId");

    bool isError = true;

    ocpp_package_CallError_t callError;
    memset(&callError, 0, sizeof(callError));

    callError.errorDescriptionIsUse = 1;
    callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_OCCURENCE_CONSTRAINT_VIOLATION;

    if (connectorId_obj == NULL)
    {
        snprintf(callError.errorDescription, 1024, "connectorId is NULL");
    }
    else
    {
        callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_TYPE_CONSTRAINT_VIOLATION;

        if (json_object_is_type(connectorId_obj, json_type_int) == false)
        {
            snprintf(callError.errorDescription, 1024, "connector SHELL be integer type");
        }
        else
        {
            callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_PROPERTY_CONSTRAINT_VIOLATION;
            int connector = json_object_get_int(connectorId_obj);

            if (connector < 0 || connector > ocpp_chargePoint->numberOfConnector)
            {
                snprintf(callError.errorDescription, 1024, "connector SHELL be > 0 and <= %d", ocpp_chargePoint->numberOfConnector);
            }
            else
            {
                isError = false;
            }
        }
    }

    if (isError)
    {
        char *lastUniqueId = strdup(uniqueId);
        char *message = ocpp_package_prepare_CallError(lastUniqueId, &callError);

        ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALLERROR);
    }

    return isError;
}

/**
 * @description:
 * @param {char} *uniqueId
 * @param {json_object *} payload_obj
 * @return {*}
 */
static bool ocpp_chargePoint_checkUpdateFirmware_Callpackage(const char *uniqueId, json_object *payload_obj)
{

    bool isError = true;

    json_object *location_obj = json_object_object_get(payload_obj, "location");
    json_object *retries_obj = json_object_object_get(payload_obj, "retries");
    json_object *retrieveDate_obj = json_object_object_get(payload_obj, "retrieveDate");
    json_object *retryInterval_obj = json_object_object_get(payload_obj, "retryInterval");

    ocpp_package_CallError_t callError;
    memset(&callError, 0, sizeof(callError));
    callError.errorDescriptionIsUse = 1;
    callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_OCCURENCE_CONSTRAINT_VIOLATION;

    if (location_obj == NULL)
    {
        snprintf(callError.errorDescription, 1024, "location SHELL be not NULL");
    }
    else if (retrieveDate_obj == NULL)
    {
        snprintf(callError.errorDescription, 1024, "retrieveDate SHELL be not NULL");
    }
    else
    {
        callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_TYPE_CONSTRAINT_VIOLATION;
        if (json_object_is_type(location_obj, json_type_string) == false)
        {
            snprintf(callError.errorDescription, 1024, "location SHELL be string type");
        }
        else if (json_object_is_type(retrieveDate_obj, json_type_string) == false)
        {
            snprintf(callError.errorDescription, 1024, "retrieveDate SHELL be string type");
        }
        else if (retries_obj != NULL && (json_object_is_type(retries_obj, json_type_int) == false))
        {
            snprintf(callError.errorDescription, 1024, "retries SHELL be integer type");
        }
        else if (retryInterval_obj != NULL && (json_object_is_type(retryInterval_obj, json_type_int) == false))
        {
            snprintf(callError.errorDescription, 1024, "retryInterval SHELL be integer type");
        }
        else
        {

            isError = false;
        }
    }

    if (isError)
    {
        char *lastUniqueId = strdup(uniqueId);
        char *message = ocpp_package_prepare_CallError(lastUniqueId, &callError);
        ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALLERROR);
    }

    return isError;
}

/**
 * @description:
 * @param {char} *uniqueId
 * @param {json_object *} payload_obj
 * @return {*}
 */
static bool ocpp_chargePoint_checkBootNotification_callResultPackage(const char *uniqueId, json_object *payload_obj)
{
    bool isError = true;

    json_object *status_obj = json_object_object_get(payload_obj, "status");
    json_object *interval_obj = json_object_object_get(payload_obj, "interval");
    json_object *currentTime_obj = json_object_object_get(payload_obj, "currentTime");

    ocpp_package_CallError_t callError;
    memset(&callError, 0, sizeof(callError));
    callError.errorDescriptionIsUse = 1;

    callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_OCCURENCE_CONSTRAINT_VIOLATION;
    if (status_obj == NULL)
    {
        snprintf(callError.errorDescription, 1024, "status SHELL be not NULL");
    }
    else if (interval_obj == NULL)
    {
        snprintf(callError.errorDescription, 1024, "interval SHELL be not NULL");
    }
    else if (currentTime_obj == NULL)
    {
        snprintf(callError.errorDescription, 1024, "currentTime SHELL be not NULL");
    }
    else
    {

        callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_TYPE_CONSTRAINT_VIOLATION;
        if (json_object_is_type(status_obj, json_type_string) == false)
        {
            snprintf(callError.errorDescription, 1024, "status SHELL be enumeration string type");
        }
        else if (json_object_is_type(interval_obj, json_type_int) == false)
        {
            snprintf(callError.errorDescription, 1024, "interval SHELL be integer type");
        }
        else if (json_object_is_type(currentTime_obj, json_type_string) == false)
        {
            snprintf(callError.errorDescription, 1024, "currentTime SHELL be enumeration string type");
        }
        else
        {
            callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_PROPERTY_CONSTRAINT_VIOLATION;

            const char *status_str = json_object_get_string(status_obj);
            int i = 0;
            for (i = 0; i < OCPP_PACKAGE_REGISTRATION_STATUS_MAX; i++)
            {
                if (strcmp(status_str, ocpp_package_RegistrationStatus_text[i]) == 0)
                    break;
            }

            if (i >= OCPP_PACKAGE_REGISTRATION_STATUS_MAX)
            {
                snprintf(callError.errorDescription, 1024, "status SHELL be Accepted、Pending or Rejected");
            }
            else
            {
                isError = false;
            }
        }
    }

    if (isError)
    {
        char *lastUniqueId = strdup(uniqueId);
        char *message = ocpp_package_prepare_CallError(lastUniqueId, &callError);

        ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALLERROR);
    }

    return isError;
}

/**
 * @description:
 * @param {char} *uniqueId
 * @param {json_object *} payload_obj
 * @return {*}
 */
static bool ocpp_chargePoint_checkAuthorize_callResultPackage(const char *uniqueId, json_object *payload_obj)
{

    bool isError = true;

    json_object *idTagInfo_obj = json_object_object_get(payload_obj, "idTagInfo");

    ocpp_package_CallError_t callError;
    memset(&callError, 0, sizeof(callError));
    callError.errorDescriptionIsUse = 1;
    callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_OCCURENCE_CONSTRAINT_VIOLATION;

    if (idTagInfo_obj == NULL)
    {
        snprintf(callError.errorDescription, 1024, "idTagInfo SHELL be not NULL");
    }
    else
    {
        json_object *status_obj = json_object_object_get(idTagInfo_obj, "status");
        json_object *expiryDate_obj = json_object_object_get(idTagInfo_obj, "expiryDate");
        json_object *parentIdTag_obj = json_object_object_get(idTagInfo_obj, "parentIdTag");

        if (status_obj == NULL)
        {
            snprintf(callError.errorDescription, 1024, "status SHELL be not NULL");
        }
        else
        {
            callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_TYPE_CONSTRAINT_VIOLATION;
            if (json_object_is_type(status_obj, json_type_string) == false)
            {
                snprintf(callError.errorDescription, 1024, "status SHELL be enumeration string type");
            }
            else if (expiryDate_obj != NULL && json_object_is_type(expiryDate_obj, json_type_string) == false)
            {
                snprintf(callError.errorDescription, 1024, "expiryDate SHELL be string type");
            }
            else if (parentIdTag_obj != NULL && json_object_is_type(parentIdTag_obj, json_type_string) == false)
            {
                snprintf(callError.errorDescription, 1024, "parentIdTag SHELL be string type");
            }
            else
            {

                callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_PROPERTY_CONSTRAINT_VIOLATION;
                const char *status_str = json_object_get_string(status_obj);
                int i = 0;
                for (i = 0; i < OCPP_LOCAL_AUTHORIZATION_MAX; i++)
                {
                    if (strcmp(status_str, ocpp_localAuthorizationStatus_Text[i]) == 0)
                        break;
                }

                if (i >= OCPP_LOCAL_AUTHORIZATION_MAX)
                {
                    snprintf(callError.errorDescription, 1024, "status SHELL be Accepted、Blocked、Expired、Invalid or ConcurrentTx");
                }
                else
                {
                    isError = false;
                }
            }
        }
    }

    if (isError)
    {

        char *lastUniqueId = strdup(uniqueId);
        char *message = ocpp_package_prepare_CallError(lastUniqueId, &callError);
        ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALLERROR);
    }

    return isError;
}

/**
 * @description:
 * @param {char} *uniqueId
 * @param {json_object *} payload_obj
 * @return {*}
 */
static bool ocpp_chargePoint_checkDataTransfer_callResultPackage(const char *uniqueId, json_object *payload_obj)
{

    return false;
}

/**
 * @description:
 * @param {char} *uniqueId
 * @param {json_object *} payload_obj
 * @return {*}
 */
static bool ocpp_chargePoint_checkDiagnosticsStatusNotification_callResultPackage(const char *uniqueId, json_object *payload_obj)
{

    return false;
}

/**
 * @description:
 * @param {char} *uniqueId
 * @param {json_object *} payload_obj
 * @return {*}
 */
static bool ocpp_chargePoint_checkFirmwareStatusNotification_callResultPackage(const char *uniqueId, json_object *payload_obj)
{

    return false;
}

/**
 * @description:
 * @param {char} *uniqueId
 * @param {json_object *} payload_obj
 * @return {*}
 */
static bool ocpp_chargePoint_checkHeartbeat_callResultPackage(const char *uniqueId, json_object *payload_obj)
{

    bool isError = true;

    json_object *currentTime_obj = json_object_object_get(payload_obj, "currentTime");

    ocpp_package_CallError_t callError;
    memset(&callError, 0, sizeof(callError));

    callError.errorDescriptionIsUse = 1;
    callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_OCCURENCE_CONSTRAINT_VIOLATION;

    if (currentTime_obj == NULL)
    {
        snprintf(callError.errorDescription, 1024, "currentTime SHELL be not NULL");
    }
    else
    {
        callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_TYPE_CONSTRAINT_VIOLATION;
        if (json_object_is_type(currentTime_obj, json_type_string) == false)
        {
            snprintf(callError.errorDescription, 1024, "currentTime SHELL be string type");
        }
        else
        {
            isError = false;
        }
    }

    if (isError)
    {
        char *lastUniqueId = strdup(uniqueId);
        char *message = ocpp_package_prepare_CallError(lastUniqueId, &callError);

        ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALLERROR);
    }

    return isError;
}

/**
 * @description:
 * @param {char} *uniqueId
 * @param {json_object *} payload_obj
 * @return {*}
 */
static bool ocpp_chargePoint_checkMeterValues_callResultPackage(const char *uniqueId, json_object *payload_obj)
{

    return false;
}

/**
 * @description:
 * @param {char} *uniqueId
 * @param {json_object *} payload_obj
 * @return {*}
 */
static bool ocpp_chargePoint_checkStartTransaction_callResultPackage(const char *uniqueId, json_object *payload_obj)
{

    bool isError = true;

    json_object *idTagInfo_obj = json_object_object_get(payload_obj, "idTagInfo");
    json_object *transactionId_obj = json_object_object_get(payload_obj, "transactionId");

    ocpp_package_CallError_t callError;
    memset(&callError, 0, sizeof(callError));

    callError.errorDescriptionIsUse = 1;
    callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_OCCURENCE_CONSTRAINT_VIOLATION;

    if (idTagInfo_obj == NULL)
    {
        snprintf(callError.errorDescription, 1024, "IdTagInfo SHELL be not NULL");
    }
    else if (transactionId_obj == NULL)
    {
        snprintf(callError.errorDescription, 1024, "transaction SHELL be not NULL");
    }
    else
    {

        json_object *status_obj = json_object_object_get(idTagInfo_obj, "status");
        json_object *expiryDate_obj = json_object_object_get(idTagInfo_obj, "expiryDate");
        json_object *parentIdTag_obj = json_object_object_get(idTagInfo_obj, "parentIdTag");

        if (status_obj == NULL)
        {
            snprintf(callError.errorDescription, 1024, "status SHELL be not NULL");
        }
        else
        {
            callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_TYPE_CONSTRAINT_VIOLATION;

            if (json_object_is_type(transactionId_obj, json_type_int) == false)
            {
                snprintf(callError.errorDescription, 1024, "transaction SHELL be integer type");
            }
            else if (json_object_is_type(status_obj, json_type_string) == false)
            {
                snprintf(callError.errorDescription, 1024, "status SHELL be enumeration string type");
            }
            else if (expiryDate_obj != NULL && json_object_is_type(expiryDate_obj, json_type_string) == false)
            {
                snprintf(callError.errorDescription, 1024, "expiryDate SHELL be string type");
            }
            else if (parentIdTag_obj != NULL && json_object_is_type(parentIdTag_obj, json_type_string) == false)
            {
                snprintf(callError.errorDescription, 1024, "parentIdTag SHELL be string type");
            }
            else
            {

                callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_PROPERTY_CONSTRAINT_VIOLATION;
                const char *status_str = json_object_get_string(status_obj);
                int i = 0;
                for (i = 0; i < OCPP_LOCAL_AUTHORIZATION_MAX; i++)
                {
                    if (strcmp(status_str, ocpp_localAuthorizationStatus_Text[i]) == 0)
                        break;
                }

                if (i >= OCPP_LOCAL_AUTHORIZATION_MAX)
                {
                    snprintf(callError.errorDescription, 1024, "status SHELL be Accepted、Blocked、Expired、Invalid or ConcurrentTx");
                }
                else
                {
                    isError = false;
                }
            }
        }
    }

    if (isError)
    {
        char *lastUniqueId = strdup(uniqueId);
        char *message = ocpp_package_prepare_CallError(lastUniqueId, &callError);
        ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALLERROR);
    }

    return isError;
}

/**
 * @description:
 * @param {char} *uniqueId
 * @param {json_object *} payload_obj
 * @return {*}
 */
static bool ocpp_chargePoint_checkStatusNotification_callResultPackage(const char *uniqueId, json_object *payload_obj)
{

    return false;
}

/**
 * @description:
 * @param {char} *uniqueId
 * @param {json_object *} payload_obj
 * @return {*}
 */
static bool ocpp_chargePoint_checkStopTransaction_callResultPackage(const char *uniqueId, json_object *payload_obj)
{

    bool isError = true;

    json_object *idTagInfo_obj = json_object_object_get(payload_obj, "idTagInfo");

    ocpp_package_CallError_t callError;
    memset(&callError, 0, sizeof(callError));
    callError.errorDescriptionIsUse = 1;
    callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_OCCURENCE_CONSTRAINT_VIOLATION;

    if (idTagInfo_obj == NULL)
    {
        snprintf(callError.errorDescription, 1024, "idTagInfo SHELL be not NULL");
    }
    else
    {
        json_object *status_obj = json_object_object_get(idTagInfo_obj, "status");
        json_object *expiryDate_obj = json_object_object_get(idTagInfo_obj, "expiryDate");
        json_object *parentIdTag_obj = json_object_object_get(idTagInfo_obj, "parentIdTag");

        if (status_obj == NULL)
        {
            snprintf(callError.errorDescription, 1024, "status SHELL be not NULL");
        }
        else
        {
            callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_TYPE_CONSTRAINT_VIOLATION;
            if (json_object_is_type(status_obj, json_type_string) == false)
            {
                snprintf(callError.errorDescription, 1024, "status SHELL be enumeration string type");
            }
            else if (expiryDate_obj != NULL && json_object_is_type(expiryDate_obj, json_type_string) == false)
            {
                snprintf(callError.errorDescription, 1024, "expiryDate SHELL be string type");
            }
            else if (parentIdTag_obj != NULL && json_object_is_type(parentIdTag_obj, json_type_string) == false)
            {
                snprintf(callError.errorDescription, 1024, "parentIdTag SHELL be string type");
            }
            else
            {

                callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_PROPERTY_CONSTRAINT_VIOLATION;
                const char *status_str = json_object_get_string(status_obj);
                int i = 0;
                for (i = 0; i < OCPP_LOCAL_AUTHORIZATION_MAX; i++)
                {
                    if (strcmp(status_str, ocpp_localAuthorizationStatus_Text[i]) == 0)
                        break;
                }

                if (i >= OCPP_LOCAL_AUTHORIZATION_MAX)
                {
                    snprintf(callError.errorDescription, 1024, "status SHELL be Accepted、Blocked、Expired、Invalid or ConcurrentTx");
                }
                else
                {
                    isError = false;
                }
            }
        }
    }

    if (isError)
    {

        char *lastUniqueId = strdup(uniqueId);
        char *message = ocpp_package_prepare_CallError(lastUniqueId, &callError);
        ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALLERROR);
    }

    return isError;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

/**
 * @description:
 * @return {*}
 */
static void ocpp_chargePoint_Call_Handler(json_object *jobj)
{
    if (jobj == NULL)
        return;

    json_object *uniqueId_obj = NULL;
    json_object *payload_obj = NULL;
    json_object *action_obj = NULL;

    uniqueId_obj = json_object_array_get_idx(jobj, 1);
    action_obj = json_object_array_get_idx(jobj, 2);
    payload_obj = json_object_array_get_idx(jobj, 3);

    if (uniqueId_obj == NULL || action_obj == NULL || payload_obj == NULL)
        return;

    const char *uniqueId_str = json_object_get_string(uniqueId_obj);

    const char *action_str = json_object_get_string(action_obj);

    if (action_str == NULL)
        return;

    int package_index = 0;

    for (package_index = 0; package_index < OCPP_PACKAGE_MAX; package_index++)
    {
        if (strcmp(action_str, ocpp_package_text[package_index]) == 0)
            break;
    }

    if (package_index >= OCPP_PACKAGE_MAX)
        return;

    switch (package_index)
    {
    case OCPP_PACKAGE_CANCEL_RESERVATION:
    {
        if (ocpp_chargePoint_checkCancelReservation_Callpackage(uniqueId_str, payload_obj))
            return;

        ocpp_package_CancelReservation_req_t cancelReservation_req;
        memset(&cancelReservation_req, 0, sizeof(cancelReservation_req));

        json_object *reservationId_obj = json_object_object_get(payload_obj, "reservationId");
        cancelReservation_req.reservationId = json_object_get_int(reservationId_obj);

        ocpp_chargePoint_manageCancelReservationRequest(uniqueId_str, cancelReservation_req);
    }
    break;

    case OCPP_PACKAGE_CHANGE_AVAILABILITY:
    {

        if (ocpp_chargePoint_checkChangeAvailablility_Callpackage(uniqueId_str, payload_obj))
            return;

        json_object *connectorId_obj = json_object_object_get(payload_obj, "connectorId");
        json_object *type_obj = json_object_object_get(payload_obj, "type");

        ocpp_package_ChangeAvailability_req_t changeAvailability_req;
        memset(&changeAvailability_req, 0, sizeof(changeAvailability_req));

        changeAvailability_req.connectorId = json_object_get_int(connectorId_obj);

        // Get Availablility Type
        const char *type_str = json_object_get_string(type_obj);
        int i = 0;
        for (i = 0; i < OCPP_PACKAGE_AVAILABILITY_TYPE_MAX; i++)
        {
            if (strcasecmp(type_str, ocpp_package_AvailabilityType_text[i]) == 0)
                break;
        }
        if (i >= OCPP_PACKAGE_AVAILABILITY_TYPE_MAX)
            return;
        changeAvailability_req.type = i;

        ocpp_chargePoint_manageChangeAvailabilityRequest(uniqueId_str, changeAvailability_req);
    }
    break;

    case OCPP_PACKAGE_CHANGE_CONFIGURATION:
    {
        if (ocpp_chargePoint_checkChangeConfiguration_Callpackage(uniqueId_str, payload_obj))
            return;

        json_object *key_obj = json_object_object_get(payload_obj, "key");
        json_object *value_obj = json_object_object_get(payload_obj, "value");

        const char *key_str = json_object_get_string(key_obj);
        const char *value_str = json_object_get_string(value_obj);

        ocpp_package_ChangeConfiguration_req_t changeConfiguration_req;
        memset(&changeConfiguration_req, 0, sizeof(changeConfiguration_req));

        strncpy(changeConfiguration_req.key, key_str, 50);
        strncpy(changeConfiguration_req.value, value_str, 500);

        ocpp_chargePoint_manageChangeConfigurationRequest(uniqueId_str, changeConfiguration_req);
    }
    break;

    case OCPP_PACKAGE_CLEAR_CACHE:
    {
        if (ocpp_chargePoint_checkClearCache_Callpackage(uniqueId_str, payload_obj))
            return;
        ocpp_package_ClearCache_req_t clearCache_req;
        memset(&clearCache_req, 0, sizeof(clearCache_req));

        ocpp_chargePoint_manageClearCacheRequest(uniqueId_str, clearCache_req);
    }
    break;

    case OCPP_PACKAGE_CLEAR_CHARGINGPROFILE:
    {
        if (ocpp_chargePoint_checkClearChargingProfile_Callpackage(uniqueId_str, payload_obj))
            return;

        ocpp_package_ClearChargingProfile_req_t clearChargingProfile_req;
        memset(&clearChargingProfile_req, 0, sizeof(clearChargingProfile_req));

        json_object *id_obj = json_object_object_get(payload_obj, "id");
        json_object *connectorId_obj = json_object_object_get(payload_obj, "connectorId");
        json_object *chargingProfilePurpose_obj = json_object_object_get(payload_obj, "chargingProfilePurpose");
        json_object *stackLevel_obj = json_object_object_get(payload_obj, "stackLevel");

        if (id_obj != NULL)
        {
            clearChargingProfile_req.idIsUse = 1;
            clearChargingProfile_req.id = json_object_get_int(id_obj);
        }

        if (connectorId_obj != NULL)
        {
            clearChargingProfile_req.connectorIdIsUse = 1;
            clearChargingProfile_req.connectorId = json_object_get_int(connectorId_obj);
        }

        if (stackLevel_obj != NULL)
        {
            clearChargingProfile_req.stackLevelIsUse = 1;
            clearChargingProfile_req.stackLevel = json_object_get_int(stackLevel_obj);
        }

        if (chargingProfilePurpose_obj != NULL)
        {
            clearChargingProfile_req.chargingProfilePurposeIsUse = 1;
            const char *chargingProfilePurpose_str = json_object_get_string(chargingProfilePurpose_obj);
            int purpose = 0;
            for (purpose = 0; purpose < OCPP_PACKAGE_CHARGING_PROFILE_PURPOSE_TYPE_MAX; purpose++)
                if (strcmp(chargingProfilePurpose_str, ocpp_package_ChargingProfilePurposeType_text[purpose]) == 0)
                    break;

            clearChargingProfile_req.chargingProfilePurpose = purpose;
        }

        ocpp_chargePoint_manageClearChargingProfileRequest(uniqueId_str, clearChargingProfile_req);
    }
    break;

    case OCPP_PACKAGE_DATATRANSFER:
    {
        ocpp_package_CallError_t callError;
        memset(&callError, 0, sizeof(callError));
        callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_NOT_SUPPORTED;
        char *lastUniqueId = strdup(uniqueId_str);
        char *message = ocpp_package_prepare_CallError(lastUniqueId, &callError);
        ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALLERROR);
    }
    break;

    case OCPP_PACKAGE_GET_COMPOSITESCAEDULE:
    {
        if (ocpp_chargePoint_checkGetCompositeSchedule_Callpackage(uniqueId_str, payload_obj))
            return;
        ocpp_package_GetCompositeSchedule_req_t getCompositeSchedule_req;
        memset(&getCompositeSchedule_req, 0, sizeof(getCompositeSchedule_req));

        json_object *connectorId_obj = json_object_object_get(payload_obj, "connectorId");
        json_object *duration_obj = json_object_object_get(payload_obj, "duration");
        json_object *chargingRateUnit_obj = json_object_object_get(payload_obj, "chargingRateUnit");

        getCompositeSchedule_req.connectorId = json_object_get_int(connectorId_obj);
        getCompositeSchedule_req.duration = json_object_get_int(duration_obj);

        if (chargingRateUnit_obj != NULL)
        {
            getCompositeSchedule_req.chargingRateUnitIsUse = 1;
            const char *chargingRateUnit_str = json_object_get_string(chargingRateUnit_obj);
            int schedule = 0;
            for (schedule = 0; schedule < OCPP_PACKAGE_CHARGING_RATE_UNIT_TYPE_MAX; schedule++)
            {
                if (strcmp(chargingRateUnit_str, ocpp_package_ChargingRateUnitType_text[schedule]) == 0)
                    break;
            }
        }

        ocpp_chargePoint_manageGetCompositeScheduleRequest(uniqueId_str, getCompositeSchedule_req);
    }
    break;

    case OCPP_PACKAGE_GET_CONFIGURATION:
    {
        if (ocpp_chargePoint_checkGetConfiguration_Callpackage(uniqueId_str, payload_obj))
            return;

        ocpp_package_GetConfiguration_req_t getConfiguration_req;
        memset(&getConfiguration_req, 0, sizeof(getConfiguration_req));

        json_object *key_obj = json_object_object_get(payload_obj, "key");
        if (key_obj != NULL)
        {
            getConfiguration_req.numberOfKey = json_object_array_length(key_obj);
            getConfiguration_req.keyIsUse = 1;
        }

        printf("numberOfKey = %d\n", getConfiguration_req.numberOfKey);
        char **key = (char **)calloc(getConfiguration_req.numberOfKey, sizeof(char *));
        int index = 0;
        for (index = 0; index < getConfiguration_req.numberOfKey; index++)
        {
            key[index] = (char *)calloc(1, 50);

            json_object *key_array = json_object_array_get_idx(key_obj, index);
            const char *key_str = json_object_get_string(key_array);

            snprintf(key[index], 50, "%s", key_str);
        }

        getConfiguration_req.key = key;

        ocpp_chargePoint_manage_GetConfigurationRequest(uniqueId_str, getConfiguration_req);
    }
    break;

    case OCPP_PACKAGE_GET_DIAGNOSTICS:
    {
        if (ocpp_chargePoint_checkGetDiagnostics_Callpackage(uniqueId_str, payload_obj))
            return;

        ocpp_package_GetDiagnostics_req_t getDiagnostics_req;
        memset(&getDiagnostics_req, 0, sizeof(getDiagnostics_req));

        json_object *location_obj = json_object_object_get(payload_obj, "location");
        json_object *retries_obj = json_object_object_get(payload_obj, "retries");
        json_object *retryInterval_obj = json_object_object_get(payload_obj, "retryInterval");
        json_object *startTime_obj = json_object_object_get(payload_obj, "startTime");
        json_object *stopTime_obj = json_object_object_get(payload_obj, "stopTime");

        const char *location_str = json_object_get_string(location_obj);
        strncpy(getDiagnostics_req.location, location_str, 256);

        getDiagnostics_req.retriesIsUse = 0;
        if (retries_obj != NULL)
        {
            getDiagnostics_req.retriesIsUse = 1;
            getDiagnostics_req.retries = json_object_get_int(retries_obj);
        }

        getDiagnostics_req.retryIntervalIsUse = 0;
        if (retryInterval_obj != NULL)
        {
            getDiagnostics_req.retryIntervalIsUse = 1;
            getDiagnostics_req.retryInterval = json_object_get_int(retryInterval_obj);
        }

        getDiagnostics_req.startTimeIsUse = 0;
        if (startTime_obj != NULL)
        {
            getDiagnostics_req.startTimeIsUse = 1;
            const char *startTime_str = json_object_get_string(startTime_obj);
            strncpy(getDiagnostics_req.startTime, startTime_str, 32);
        }

        getDiagnostics_req.stopTimeIsUse = 0;
        if (stopTime_obj != NULL)
        {
            getDiagnostics_req.stopTimeIsUse = 1;
            const char *stopTime_str = json_object_get_string(stopTime_obj);
            strncpy(getDiagnostics_req.stopTime, stopTime_str, 32);
        }

        ocpp_chargePoint_manageGetDiagnosticsRequest(uniqueId_str, getDiagnostics_req);
    }
    break;

    case OCPP_PACKAGE_GET_LOCALLISTVERSION:
    {
        if (ocpp_chargePoint_checkGetLocalListVersion_Callpackage(uniqueId_str, payload_obj))
            return;

        ocpp_package_GetLocalListVersion_req_t getLocalListVersion_req;
        memset(&getLocalListVersion_req, 0, sizeof(getLocalListVersion_req));

        ocpp_chargePoint_manageGetLocalListVersionRequest(uniqueId_str, getLocalListVersion_req);
    }
    break;

    case OCPP_PACKAGE_REMOTE_STARTTRANSACTION:
    {

        if (ocpp_chargePoint_checkRemoteStartTransaction_Callpackage(uniqueId_str, payload_obj))
            return;

        json_object *idTag_obj = json_object_object_get(payload_obj, "idTag");
        json_object *connectorId_obj = json_object_object_get(payload_obj, "connectorId");
        json_object *chargingProfile_obj = json_object_object_get(payload_obj, "chargingProfile");

        ocpp_package_RemoteStartTransaction_req_t remoteStartTransaction_req;
        memset(&remoteStartTransaction_req, 0, sizeof(remoteStartTransaction_req));

        const char *idTag_str = json_object_get_string(idTag_obj);
        OCPP_LOG_INFO("idTag_str = %s", idTag_str);
        strncpy(remoteStartTransaction_req.idTag, idTag_str, 20);

        remoteStartTransaction_req.connectorIdIsUse = 0;
        if (connectorId_obj != NULL)
        {
            remoteStartTransaction_req.connectorIdIsUse = 1;
            remoteStartTransaction_req.connectorId = json_object_get_int(connectorId_obj);
        }

        // chargingProfile not impletement
        remoteStartTransaction_req.chargingProfileIsUse = 0;
        if (chargingProfile_obj != NULL)
        {
        }

        ocpp_chargePoint_manageRemoteStartTransactionRequest(uniqueId_str, remoteStartTransaction_req);
    }
    break;

    case OCPP_PACKAGE_REMOTE_STOPTRANSACTION:
    {
        if (ocpp_chargePoint_checkRemoteStopTransaction_Callpackage(uniqueId_str, payload_obj))
            return;

        json_object *transactionId_obj = json_object_object_get(payload_obj, "transactionId");

        ocpp_package_RemoteStopTransaction_req_t remoteStopTransaction_req;
        memset(&remoteStopTransaction_req, 0, sizeof(remoteStopTransaction_req));

        remoteStopTransaction_req.transactionId = json_object_get_int(transactionId_obj);

        ocpp_chargePoint_manageRemoteStopTransactionRequest(uniqueId_str, remoteStopTransaction_req);
    }
    break;

    case OCPP_PACKAGE_RESERVENOW:
    {
        if (ocpp_chargePoint_checkReserveNow_Callpackage(uniqueId_str, payload_obj))
            return;

        ocpp_package_ReserveNow_req_t reserveNow_req;
        memset(&reserveNow_req, 0, sizeof(reserveNow_req));

        json_object *connectorId_obj = json_object_object_get(payload_obj, "connectorId");
        json_object *expiryDate_obj = json_object_object_get(payload_obj, "expiryDate");
        json_object *idTag_obj = json_object_object_get(payload_obj, "idTag");
        json_object *parentIdTag_obj = json_object_object_get(payload_obj, "parentIdTag");
        json_object *reservationId_obj = json_object_object_get(payload_obj, "reservationId");

        reserveNow_req.connectorId = json_object_get_int(connectorId_obj);
        reserveNow_req.reservationId = json_object_get_int(reservationId_obj);

        const char *expiryDate_str = json_object_get_string(expiryDate_obj);
        const char *idTag_str = json_object_get_string(idTag_obj);

        strncpy(reserveNow_req.expiryDate, expiryDate_str, 32);
        strncpy(reserveNow_req.idTag, idTag_str, OCPP_AUTHORIZATION_IDTAG_LEN);

        reserveNow_req.parentIdTagIsUse = 0;
        if (parentIdTag_obj != NULL)
        {
            reserveNow_req.parentIdTagIsUse = 1;
            const char *parentIdTag_str = json_object_get_string(parentIdTag_obj);
            strncpy(reserveNow_req.parentIdTag, parentIdTag_str, OCPP_AUTHORIZATION_IDTAG_LEN);
        }

        ocpp_chargePoint_manageReserveNowRequest(uniqueId_str, reserveNow_req);
    }
    break;

    case OCPP_PACKAGE_RESET:
    {
        if (ocpp_chargePoint_checkReset_Callpackage(uniqueId_str, payload_obj))
            return;

        json_object *type_obj = json_object_object_get(payload_obj, "type");
        const char *type_str = json_object_get_string(type_obj);

        ocpp_package_Reset_req_t reset_req;
        memset(&reset_req, 0, sizeof(reset_req));

        int index = 0;
        for (; index < OCPP_PACKAGE_RESET_TYPE_MAX; index++)
        {
            if (strcmp(ocpp_package_ResetType_text[index], type_str) == 0)
                break;
        }

        reset_req.type = index;
        ocpp_chargePoint_manageResetRequest(uniqueId_str, reset_req);
    }
    break;

    case OCPP_PACKAGE_SENDLOCALLIST:
    {
        if (ocpp_chargePoint_checkSendLocalList_Callpackage(uniqueId_str, payload_obj))
            return;

        json_object *listVersion_obj = json_object_object_get(payload_obj, "listVersion");
        json_object *updateType_obj = json_object_object_get(payload_obj, "updateType");
        json_object *localAuthorizationList_obj = json_object_object_get(payload_obj, "localAuthorizationList");
        int numberOfList = json_object_array_length(localAuthorizationList_obj);

        ocpp_package_SendLocalList_req_t sendLocalList_req;
        memset(&sendLocalList_req, 0, sizeof(sendLocalList_req));

        sendLocalList_req.listVersion = json_object_get_int(listVersion_obj);
        const char *updateType_str = json_object_get_string(updateType_obj);

        int i = 0;
        for (i = 0; i < OCPP_PACKAGE_UPDATE_TYPE_MAX; i++)
        {
            if (strcmp(ocpp_package_UpdateType_text[i], updateType_str) == 0)
                break;
            ;
        }
        sendLocalList_req.UpdateType = i;

        sendLocalList_req.localAuthorizationListIsUse = 0;
        if (numberOfList > 0)
        {
            sendLocalList_req.localAuthorizationListIsUse = 1;
            sendLocalList_req.numberOfAuthorizationData = numberOfList;
            sendLocalList_req.localAuthorizationList = (ocpp_package_AuthorizationData_t **)calloc(numberOfList, sizeof(ocpp_package_AuthorizationData_t *));

            for (i = 0; i < numberOfList; i++)
            {
                sendLocalList_req.localAuthorizationList[i] = (ocpp_package_AuthorizationData_t *)calloc(1, sizeof(ocpp_package_AuthorizationData_t));

                json_object *localAuthorizetion_index = json_object_array_get_idx(localAuthorizationList_obj, i);

                json_object *idTag_obj = json_object_object_get(localAuthorizetion_index, "idTag");
                const char *idTag_str = json_object_get_string(idTag_obj);
                strncpy(sendLocalList_req.localAuthorizationList[i]->idTag, idTag_str, OCPP_AUTHORIZATION_IDTAG_LEN);

                sendLocalList_req.localAuthorizationList[i]->idTagInfoIsUse = 0;
                json_object *idTagInfo_obj = json_object_object_get(localAuthorizetion_index, "idTagInfo");
                if (idTagInfo_obj != NULL)
                {
                    sendLocalList_req.localAuthorizationList[i]->idTagInfoIsUse = 1;
                    json_object *expiryDate_obj = json_object_object_get(idTagInfo_obj, "expiryDate");
                    json_object *parentIdTag_obj = json_object_object_get(idTagInfo_obj, "parentIdTag");
                    json_object *status_obj = json_object_object_get(idTagInfo_obj, "status");

                    const char *status_str = json_object_get_string(status_obj);
                    int j = 0;
                    for (j = 0; j < OCPP_LOCAL_AUTHORIZATION_MAX; i++)
                    {
                        if (strcmp(ocpp_localAuthorizationStatus_Text[j], status_str) == 0)
                            break;
                    }

                    sendLocalList_req.localAuthorizationList[i]->idTagInfo.AuthorizationStatus = j;

                    sendLocalList_req.localAuthorizationList[i]->idTagInfo.expiryDateIsUse = 0;
                    sendLocalList_req.localAuthorizationList[i]->idTagInfo.parentIdTagIsUse = 0;

                    if (expiryDate_obj != NULL)
                    {
                        sendLocalList_req.localAuthorizationList[i]->idTagInfo.expiryDateIsUse = 1;
                        const char *expiryDate_str = json_object_get_string(expiryDate_obj);
                        strncpy(sendLocalList_req.localAuthorizationList[i]->idTagInfo.expiryDate, expiryDate_str, 32);
                    }

                    if (parentIdTag_obj != NULL)
                    {
                        sendLocalList_req.localAuthorizationList[i]->idTagInfo.parentIdTagIsUse = 1;
                        const char *parentIdTag_str = json_object_get_string(parentIdTag_obj);
                        strncpy(sendLocalList_req.localAuthorizationList[i]->idTagInfo.parentIdTag, parentIdTag_str, OCPP_LOCAL_AUTHORIZATION_MAX);
                    }
                }
            }
        }

        ocpp_chargePoint_manageSendLocalListRequest(uniqueId_str, &sendLocalList_req);
    }
    break;

    case OCPP_PACKAGE_SETCHARGINGPROFILE:
    {
        if (ocpp_chargePoint_checkSetChargingProfile_Callpackage(uniqueId_str, payload_obj))
            return;

        ocpp_package_SetChargingProfile_req_t setChargingProfile_req;
        memset(&setChargingProfile_req, 0, sizeof(setChargingProfile_req));

        json_object *connectorId_obj = json_object_object_get(payload_obj, "connectorId");
        json_object *csChargingProfiles_obj = json_object_object_get(payload_obj, "csChargingProfiles");

        setChargingProfile_req.connectorId = json_object_get_int(connectorId_obj);

        json_object *chargingProfileId_obj = json_object_object_get(csChargingProfiles_obj, "chargingProfileId");
        json_object *transactionId_obj = json_object_object_get(csChargingProfiles_obj, "transactionId");
        json_object *stackLevel_obj = json_object_object_get(csChargingProfiles_obj, "stackLevel");
        json_object *chargingProfilePurpose_obj = json_object_object_get(csChargingProfiles_obj, "chargingProfilePurpose");
        json_object *chargingProfileKind_obj = json_object_object_get(csChargingProfiles_obj, "chargingProfileKind");
        json_object *recurrencyKind_obj = json_object_object_get(csChargingProfiles_obj, "recurrencyKind");
        json_object *validFrom_obj = json_object_object_get(csChargingProfiles_obj, "validFrom");
        json_object *validTo_obj = json_object_object_get(csChargingProfiles_obj, "validTo");
        json_object *chargingSchedule_obj = json_object_object_get(csChargingProfiles_obj, "chargingSchedule");

        setChargingProfile_req.csChargingProfiles.chargingProfileId = json_object_get_int(chargingProfileId_obj);

        if (transactionId_obj != NULL)
        {
            setChargingProfile_req.csChargingProfiles.transactionIdIsUse = 1;
            setChargingProfile_req.csChargingProfiles.transactionId = json_object_get_int(transactionId_obj);
        }

        setChargingProfile_req.csChargingProfiles.stackLevel = json_object_get_int(stackLevel_obj);

        const char *chargingProfilePurpose_str = json_object_get_string(chargingProfilePurpose_obj);
        int i = 0;
        for (i = 0; i < OCPP_PACKAGE_CHARGING_PROFILE_PURPOSE_TYPE_MAX; i++)
        {
            if (strcmp(chargingProfilePurpose_str, ocpp_package_ChargingProfilePurposeType_text[i]) == 0)
                break;
        }
        setChargingProfile_req.csChargingProfiles.chargingProfilePurpose = i;

        const char *chargingProfileKind_str = json_object_get_string(chargingProfileKind_obj);

        for (i = 0; i < OCPP_PACKAGE_CHARGING_PROFILE_KIND_TYPE_MAX; i++)
        {
            if (strcmp(chargingProfileKind_str, ocpp_package_ChargingProfileKindType_text[i]) == 0)
                break;
        }

        if (recurrencyKind_obj != NULL)
        {
            const char *recurrencyKind_str = json_object_get_string(recurrencyKind_obj);

            for (i = 0; i < OCPP_PACKAGE_RECURRENCY_KIND_TYPE_MAX; i++)
            {
                if (strcmp(recurrencyKind_str, ocpp_package_RecurrencyKindType_text[i]) == 0)
                    break;
            }
        }

        if (validFrom_obj != NULL)
        {
            setChargingProfile_req.csChargingProfiles.validFromIsUse = 1;
            const char *vailFrom_str = json_object_get_string(validFrom_obj);
            strncpy(setChargingProfile_req.csChargingProfiles.validFrom, vailFrom_str, 32);
        }

        if (validTo_obj != NULL)
        {
            setChargingProfile_req.csChargingProfiles.validToIsUse = 1;
            const char *validTo_str = json_object_get_string(validTo_obj);
            strncpy(setChargingProfile_req.csChargingProfiles.validTo, validTo_str, 32);
        }

        json_object *duration_obj = json_object_object_get(chargingSchedule_obj, "duration");
        json_object *startSchedule_obj = json_object_object_get(chargingSchedule_obj, "startSchedule");
        json_object *chargingRateUnit_obj = json_object_object_get(chargingSchedule_obj, "chargingRateUnit");
        json_object *chargingSchedulePeriod_obj = json_object_object_get(chargingSchedule_obj, "chargingSchedulePeriod");
        json_object *minChargingRate_obj = json_object_object_get(chargingSchedule_obj, "minChargingRate");

        if (duration_obj != NULL)
        {
            setChargingProfile_req.csChargingProfiles.chargingSchedule.durationIsUse = 1;
            setChargingProfile_req.csChargingProfiles.chargingSchedule.duration = json_object_get_int(duration_obj);
        }

        if (startSchedule_obj != NULL)
        {
            setChargingProfile_req.csChargingProfiles.chargingSchedule.startScheduleIsUse = 1;
            const char *startSchedule_str = json_object_get_string(startSchedule_obj);
            strncpy(setChargingProfile_req.csChargingProfiles.chargingSchedule.startSchedule, startSchedule_str, 32);
        }

        const char *chargingRateUnit_str = json_object_get_string(chargingRateUnit_obj);
        for (i = 0; i < OCPP_PACKAGE_CHARGING_RATE_UNIT_TYPE_MAX; i++)
        {
            if (strcmp(chargingRateUnit_str, ocpp_package_ChargingRateUnitType_text[i]) == 0)
                break;
        }
        setChargingProfile_req.csChargingProfiles.chargingSchedule.chargingRateUnit = i;

        if (minChargingRate_obj != NULL)
        {
            setChargingProfile_req.csChargingProfiles.chargingSchedule.minChargingRateIsUse = 1;
            setChargingProfile_req.csChargingProfiles.chargingSchedule.minChargingRate = json_object_get_double(minChargingRate_obj);
        }

        int num = json_object_array_length(chargingSchedulePeriod_obj);
        setChargingProfile_req.csChargingProfiles.chargingSchedule.numberOfchargingSchedulePeriod = num;

        setChargingProfile_req.csChargingProfiles.chargingSchedule.chargingSchedulePeriod = (ocpp_package_chargingSchedulePeriod_t **)calloc(num, sizeof(ocpp_package_chargingSchedulePeriod_t *));
        for (i = 0; i < num; i++)
        {
            json_object *chargingSchedulePeriod_index_obj = json_object_array_get_idx(chargingSchedulePeriod_obj, i);
            json_object *startPeriod_obj = json_object_object_get(chargingSchedulePeriod_index_obj, "startPeriod");
            json_object *limit_obj = json_object_object_get(chargingSchedulePeriod_index_obj, "limit");
            json_object *numberPhases_obj = json_object_object_get(chargingSchedulePeriod_index_obj, "numberPhases");

            setChargingProfile_req.csChargingProfiles.chargingSchedule.chargingSchedulePeriod[i]->startPeriod = json_object_get_int(startPeriod_obj);
            setChargingProfile_req.csChargingProfiles.chargingSchedule.chargingSchedulePeriod[i]->limit = json_object_get_double(limit_obj);

            if (numberPhases_obj != NULL)
            {
                setChargingProfile_req.csChargingProfiles.chargingSchedule.chargingSchedulePeriod[i]->numberPhasesIsUse = 1;
                setChargingProfile_req.csChargingProfiles.chargingSchedule.chargingSchedulePeriod[i]->numberPhases = json_object_get_int(numberPhases_obj);
            }
        }

        ocpp_chargePoint_manageSetChargingProfileRequest(uniqueId_str, &setChargingProfile_req);
    }
    break;

    case OCPP_PACKAGE_TRIGGER_MESSAGE:
    {
        if (ocpp_chargePoint_checkTriggerMessage_Callpackage(uniqueId_str, payload_obj))
            return;

        ocpp_package_TriggerMessage_req_t triggerMessage_req;
        memset(&triggerMessage_req, 0, sizeof(triggerMessage_req));

        json_object *requestedMessage_obj = json_object_object_get(payload_obj, "requestedMessage");
        json_object *connectorId_obj = json_object_object_get(payload_obj, "connectorId");

        const char *requestedMessage_str = json_object_get_string(requestedMessage_obj);

        int i = 0;
        for (i = 0; i < OCPP_PACKGE_MESSAGE_TRIGGER_STATUS_MAX; i++)
        {
            if (strcmp(requestedMessage_str, ocpp_package_MessageTrigger_text[i]) == 0)
                break;
        }
        triggerMessage_req.requestedMessage = i;

        triggerMessage_req.connectorIdIsUse = 0;
        if (connectorId_obj != NULL)
        {
            triggerMessage_req.connectorIdIsUse = 1;
            triggerMessage_req.connectorId = json_object_get_int(connectorId_obj);
        }

        ocpp_chargePoint_manageTriggerMessageRequest(uniqueId_str, triggerMessage_req);
    }
    break;

    case OCPP_PACKAGE_UNLOCK_CONNECTOR:
    {
        if (ocpp_chargePoint_checkUnlockConnector_Callpackage(uniqueId_str, payload_obj))
            return;

        json_object *connectorId_obj = json_object_object_get(payload_obj, "connectorId");

        ocpp_package_UnlockConnector_req_t unlockConnector_req;
        memset(&unlockConnector_req, 0, sizeof(unlockConnector_req));

        unlockConnector_req.connectorId = json_object_get_int(connectorId_obj);

        ocpp_chargePoint_manageUnlockConnectorRequest(uniqueId_str, unlockConnector_req);
    }
    break;

    case OCPP_PACKAGE_UPDATE_FIRMWARE:
    {
        if (ocpp_chargePoint_checkUpdateFirmware_Callpackage(uniqueId_str, payload_obj))
            return;

        ocpp_package_UpdateFirmware_req_t updateFirmware_req;
        memset(&updateFirmware_req, 0, sizeof(updateFirmware_req));

        json_object *location_obj = json_object_object_get(payload_obj, "location");
        json_object *retries_obj = json_object_object_get(payload_obj, "retries");
        json_object *retrieveDate_obj = json_object_object_get(payload_obj, "retrieveDate");
        json_object *retryInterval_obj = json_object_object_get(payload_obj, "retryInterval");

        const char *location_str = json_object_get_string(location_obj);
        const char *retrieveDate_str = json_object_get_string(retrieveDate_obj);
        strncpy(updateFirmware_req.location, location_str, 256);
        strncpy(updateFirmware_req.retrieveDate, retrieveDate_str, 32);

        updateFirmware_req.retriesIsUse = 0;
        if (retries_obj != NULL)
        {
            updateFirmware_req.retriesIsUse = 1;
            updateFirmware_req.retries = json_object_get_int(retries_obj);
        }

        updateFirmware_req.retryIntervalIsUse = 0;
        if (retryInterval_obj != NULL)
        {
            updateFirmware_req.retryIntervalIsUse = 1;
            updateFirmware_req.retryInterval = json_object_get_int(retryInterval_obj);
        }

        ocpp_chargePoint_manageUpdateFirmwareRequest(uniqueId_str, updateFirmware_req);
    }
    break;

    default:
    {
        ocpp_package_CallError_t callError;
        memset(&callError, 0, sizeof(callError));
        callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_NOT_IMPLEMENTED;
        char *lastUniqueId = strdup(uniqueId_str);
        char *message = ocpp_package_prepare_CallError(lastUniqueId, &callError);
        ocpp_chargePoint_send(lastUniqueId, message, OCPP_PACKAGE_CALLERROR);
    }
    break;
    }
}

// [3,"9e8d90d5-0dac-4104-9b1d-8d5cd3a11a8c",{"idTagInfo":{"expiryDate":"2024-04-01T00:00:00.000Z","parentIdTag":"","status":"Accepted"}}]

/**
 * @description:
 * @return {*}
 */
static void ocpp_chargePoint_CallResult_Handler(json_object *jobj, enum OCPP_PACKAGE package)
{
    if (jobj == NULL)
        return;

    json_object *payload_obj = NULL;
    payload_obj = json_object_array_get_idx(jobj, 2);
    if (payload_obj == NULL)
        return;

    json_object *uniqueId_obj = NULL;
    uniqueId_obj = json_object_array_get_idx(jobj, 1);
    if (uniqueId_obj == NULL)
        return;
    const char *uniqueId_str = json_object_get_string(uniqueId_obj);

    switch (package)
    {
    case OCPP_PACKAGE_BOOT_NOTIFICATION:
    {
        if (ocpp_chargePoint_checkBootNotification_callResultPackage(uniqueId_str, payload_obj))
            return;

        json_object *status_obj = json_object_object_get(payload_obj, "status");
        json_object *interval_obj = json_object_object_get(payload_obj, "interval");
        const char *status_str = json_object_get_string(status_obj);
        int interval_int = json_object_get_int(interval_obj);

        int i = 0;
        for (i = 0; i < OCPP_PACKAGE_REGISTRATION_STATUS_MAX; i++)
        {
            if (strcmp(status_str, ocpp_package_RegistrationStatus_text[i]) == 0)
                break;
        }

        ocpp_chargePoint->RegistrationStatus = i;
        // 保存心跳间隔
        char intervalStr[10] = {0};
        snprintf(intervalStr, 10, "%d", interval_int);
        ocpp_ConfigurationKey_Modify(ocpp_configurationKeyText[OCPP_CK_HeartbeatInterval], intervalStr, 1);
        OCPP_LOG_DEBUG("heartbeat intervalStr = %s", intervalStr);
    }
    break;

    case OCPP_PACKAGE_HEARTBEAT:
    {
        if (ocpp_chargePoint_checkHeartbeat_callResultPackage(uniqueId_str, payload_obj))
            return;

        json_object *currentTime_obj = json_object_object_get(payload_obj, "currentTime");
        const char *currentTime_str = json_object_get_string(currentTime_obj);
        ocpp_AuxiliaryTool_setSystemTime(currentTime_str);
        // OCPP_LOG_DEBUG("HEARTBEAT");
    }
    break;

    case OCPP_PACKAGE_STATUS_NOTIFICATION:
    {
        if (ocpp_chargePoint_checkStatusNotification_callResultPackage(uniqueId_str, payload_obj))
            return;
    }
    break;

    case OCPP_PACKAGE_DIAGNOSTICS_STATUS_NOTIFICATION:
    {
        if (ocpp_chargePoint_checkDiagnosticsStatusNotification_callResultPackage(uniqueId_str, payload_obj))
            return;
    }
    break;

    case OCPP_PACKAGE_FIRMWARE_STATUS_NOTIFICATION:
    {
        if (ocpp_chargePoint_checkFirmwareStatusNotification_callResultPackage(uniqueId_str, payload_obj))
            return;
    }
    break;

    case OCPP_PACKAGE_METERVALUES:
    {
        if (ocpp_chargePoint_checkMeterValues_callResultPackage(uniqueId_str, payload_obj))
            return;

        // 离线数据接收确认
        if (ocpp_chargePoint->offlineDate_obj.isHaveDate)
        {
            if (strcmp(ocpp_chargePoint->offlineDate_obj.lastUniqueId, uniqueId_str) == 0)
            {
                ocpp_chargePoint->offlineDate_obj.isResponse = true;
                ocpp_offline_deleteOfflineData(uniqueId_str);
            }
        }
    }
    break;

    case OCPP_PACKAGE_AUTHORIZE:
    {
        OCPP_LOG_DEBUG("Authoriza");
        if (ocpp_chargePoint_checkAuthorize_callResultPackage(uniqueId_str, payload_obj))
            return;

        json_object *idTagInfo_obj = json_object_object_get(payload_obj, "idTagInfo");
        json_object *status_obj = json_object_object_get(idTagInfo_obj, "status");
        json_object *expiryDate_obj = json_object_object_get(idTagInfo_obj, "expiryDate");
        json_object *parentIdTag_obj = json_object_object_get(idTagInfo_obj, "parentIdTag");

        ocpp_package_Authorize_conf_t authorize;
        memset(&authorize, 0, sizeof(authorize));

        // Get Authorize status
        const char *status_str = json_object_get_string(status_obj);

        int i = 0;
        for (i = 0; i < OCPP_LOCAL_AUTHORIZATION_MAX; i++)
        {
            if (strcmp(status_str, ocpp_localAuthorizationStatus_Text[i]) == 0)
                break;
        }

        authorize.idTagInfo.AuthorizationStatus = i;

        if (expiryDate_obj != NULL)
        {
            authorize.idTagInfo.expiryDateIsUse = 1;
            const char *expiryDate_str = json_object_get_string(expiryDate_obj);
            strncpy(authorize.idTagInfo.expiryDate, expiryDate_str, 32);
        }

        if (parentIdTag_obj != NULL)
        {
            authorize.idTagInfo.parentIdTagIsUse = 1;
            const char *parentIdTag_str = json_object_get_string(parentIdTag_obj);
            strncpy(authorize.idTagInfo.parentIdTag, parentIdTag_str, OCPP_AUTHORIZATION_IDTAG_LEN);
        }

        char idTag[OCPP_AUTHORIZATION_IDTAG_LEN] = {0};
        // set Authorization result
        for (i = 0; i <= ocpp_chargePoint->numberOfConnector; i++)
        {

            OCPP_LOG_DEBUG("isWaitAuthoriza = %d", ocpp_chargePoint->authorizetion[i]->isWaitAuthoriza);
            if (ocpp_chargePoint->authorizetion[i]->isWaitAuthoriza)
            {

                strncpy(idTag, ocpp_chargePoint->authorizetion[i]->idTag, OCPP_AUTHORIZATION_IDTAG_LEN);

                ocpp_chargePoint->authorizetion[i]->isWaitAuthoriza = false;

                OCPP_LOG_DEBUG("lastUniqueId = %s   uniqueId_str = %s", ocpp_chargePoint->authorizetion[i]->lastUniqueId, uniqueId_str);

                if (strcmp(ocpp_chargePoint->authorizetion[i]->lastUniqueId, uniqueId_str) == 0)
                {
                    if (authorize.idTagInfo.AuthorizationStatus == OCPP_LOCAL_AUTHORIZATION_ACCEPTED)
                    {
                        ocpp_chargePoint->setAuthorizeResult(i, true);
                    }
                    else
                    {
                        ocpp_chargePoint->setAuthorizeResult(i, false);
                    }
                }

                break;
            }
        }

        if (i > ocpp_chargePoint->numberOfConnector)
            return;

        // insert or update Authorize Cache
        ocpp_localAuthorization_cache_record_t *cache_record = (ocpp_localAuthorization_cache_record_t *)calloc(1, sizeof(ocpp_localAuthorization_cache_record_t));

        strncpy(cache_record->IdTag, idTag, OCPP_AUTHORIZATION_IDTAG_LEN);
        cache_record->status = authorize.idTagInfo.AuthorizationStatus;

        cache_record->expiryDate = NULL;
        if (authorize.idTagInfo.expiryDateIsUse)
        {
            struct tm *time = calloc(1, sizeof(struct tm));
            strptime(authorize.idTagInfo.expiryDate, "%Y-%m-%dT%H:%M:%S.", time);
            cache_record->expiryDate = time;
        }

        if (authorize.idTagInfo.parentIdTagIsUse)
        {
            strncpy(cache_record->parentIdTag, authorize.idTagInfo.parentIdTag, OCPP_AUTHORIZATION_IDTAG_LEN);
        }

        ocpp_localAuthorization_Cache_write(cache_record);

        free(cache_record->expiryDate);
        free(cache_record);
    }
    break;

    case OCPP_PACKAGE_STARTTRANSACTION:
    {
        if (ocpp_chargePoint_checkStartTransaction_callResultPackage(uniqueId_str, payload_obj))
            return;

        json_object *idTagInfo_obj = json_object_object_get(payload_obj, "idTagInfo");
        json_object *transactionId_obj = json_object_object_get(payload_obj, "transactionId");
        json_object *status_obj = json_object_object_get(idTagInfo_obj, "status");
        json_object *expiryDate_obj = json_object_object_get(idTagInfo_obj, "expiryDate");
        json_object *parentIdTag_obj = json_object_object_get(idTagInfo_obj, "parentIdTag_obj");

        ocpp_package_StartTransaction_conf_t startTransaction;
        memset(&startTransaction, 0, sizeof(startTransaction));

        // Extract data
        startTransaction.transactionId = json_object_get_int(transactionId_obj);

        const char *status_str = json_object_get_string(status_obj);
        int i = 0;
        for (i = 0; i < OCPP_LOCAL_AUTHORIZATION_MAX; i++)
        {
            if (strcmp(status_str, ocpp_localAuthorizationStatus_Text[i]) == 0)
                break;
        }
        if (i >= OCPP_LOCAL_AUTHORIZATION_MAX)
            return;
        startTransaction.idTagInfo.AuthorizationStatus = i;

        if (expiryDate_obj != NULL)
        {
            startTransaction.idTagInfo.expiryDateIsUse = 1;
            const char *expiryDate_str = json_object_get_string(expiryDate_obj);
            strncpy(startTransaction.idTagInfo.expiryDate, expiryDate_str, OCPP_AUTHORIZATION_IDTAG_LEN);
        }

        if (parentIdTag_obj != NULL)
        {
            startTransaction.idTagInfo.parentIdTagIsUse = 1;
            const char *parentIdTag_str = json_object_get_string(parentIdTag_obj);
        }

        // 找到正在交易的枪号
        for (i = 1; i <= ocpp_chargePoint->numberOfConnector; i++)
        {
            if (ocpp_chargePoint->transaction_obj[i]->isTransaction)
                if (strcmp(ocpp_chargePoint->transaction_obj[i]->lastUniqueId, uniqueId_str) == 0)
                {
                    ocpp_chargePoint->transaction_obj[i]->isRecStartTransaction_Conf = true;
                    ocpp_chargePoint->transaction_obj[i]->startTransaction = startTransaction;
                    break;
                }
        }

        if (i > ocpp_chargePoint->numberOfConnector)
            return;

        // 离线数据接收确认
        if (ocpp_chargePoint->offlineDate_obj.isHaveDate)
        {
            if (strcmp(ocpp_chargePoint->offlineDate_obj.lastUniqueId, uniqueId_str) == 0)
            {
                ocpp_chargePoint->offlineDate_obj.isResponse = true;
                ocpp_offline_deleteOfflineData(uniqueId_str);
            }
        }

        // 更新认证缓存
        ocpp_localAuthorization_cache_record_t *cache_record = (ocpp_localAuthorization_cache_record_t *)calloc(1, sizeof(ocpp_localAuthorization_cache_record_t));

        strncpy(cache_record->IdTag, ocpp_chargePoint->transaction_obj[i]->startIdTag, OCPP_AUTHORIZATION_IDTAG_LEN);
        cache_record->status = startTransaction.idTagInfo.AuthorizationStatus;

        cache_record->expiryDate = NULL;
        if (startTransaction.idTagInfo.expiryDateIsUse)
        {
            struct tm *time = calloc(1, sizeof(struct tm));
            strptime(startTransaction.idTagInfo.expiryDate, "%Y-%m-%dT%H:%M:%S.", time);
            cache_record->expiryDate = time;
        }

        if (startTransaction.idTagInfo.parentIdTagIsUse)
        {
            strncpy(cache_record->parentIdTag, startTransaction.idTagInfo.parentIdTag, OCPP_AUTHORIZATION_IDTAG_LEN);
        }

        ocpp_localAuthorization_Cache_write(cache_record);

        free(cache_record->expiryDate);
        free(cache_record);
    }
    break;

    case OCPP_PACKAGE_STOPTRANSACTION:
    {

        if (ocpp_chargePoint_checkStopTransaction_callResultPackage(uniqueId_str, payload_obj))
            return;

        json_object *idTagInfo_obj = json_object_object_get(payload_obj, "idTagInfo");

        ocpp_package_StopTransaction_conf_t stopTransaction;
        memset(&stopTransaction, 0, sizeof(stopTransaction));
        int i = 0;

        if (idTagInfo_obj != NULL)
        {
            stopTransaction.idTagInfoIsUse = 1;
            json_object *status_obj = json_object_object_get(idTagInfo_obj, "status");
            json_object *expiryDate_obj = json_object_object_get(idTagInfo_obj, "expiryDate");
            json_object *parentIdTag_obj = json_object_object_get(idTagInfo_obj, "parentIdTag_obj");

            const char *status_str = json_object_get_string(status_obj);

            for (i = 0; i < OCPP_LOCAL_AUTHORIZATION_MAX; i++)
            {
                if (strcmp(status_str, ocpp_localAuthorizationStatus_Text[i]) == 0)
                    break;
            }

            stopTransaction.idTagInfo.AuthorizationStatus = i;

            if (expiryDate_obj != NULL)
            {
                stopTransaction.idTagInfo.expiryDateIsUse = 1;
                const char *expiryDate_str = json_object_get_string(expiryDate_obj);
                strncpy(stopTransaction.idTagInfo.expiryDate, expiryDate_str, 32);
            }

            if (parentIdTag_obj != NULL)
            {
                stopTransaction.idTagInfo.parentIdTagIsUse = 1;
                const char *parentIdTag_str = json_object_get_string(parentIdTag_obj);
                strncpy(stopTransaction.idTagInfo.parentIdTag, parentIdTag_str, OCPP_AUTHORIZATION_IDTAG_LEN);
            }
        }

        // 交易数据应答确认
        for (i = 1; i <= ocpp_chargePoint->numberOfConnector; i++)
        {
            if (ocpp_chargePoint->transaction_obj[i]->isTransaction)
                if (strcmp(ocpp_chargePoint->transaction_obj[i]->lastUniqueId, uniqueId_str) == 0)
                {
                    ocpp_chargePoint->transaction_obj[i]->isRecStopTransaction_Conf = true;
                    ocpp_chargePoint->transaction_obj[i]->stopTransaction = stopTransaction;
                }
        }

        if (i > ocpp_chargePoint->numberOfConnector)
            return;

        // 离线数据接收确认
        if (ocpp_chargePoint->offlineDate_obj.isHaveDate)
        {
            if (strcmp(ocpp_chargePoint->offlineDate_obj.lastUniqueId, uniqueId_str) == 0)
            {
                ocpp_chargePoint->offlineDate_obj.isResponse = true;
                ocpp_offline_deleteOfflineData(uniqueId_str);
            }
        }

        // Insert or upDate
        ocpp_localAuthorization_cache_record_t *cache_record = (ocpp_localAuthorization_cache_record_t *)calloc(1, sizeof(ocpp_localAuthorization_cache_record_t));

        strncpy(cache_record->IdTag, ocpp_chargePoint->transaction_obj[i]->stopIdTag, OCPP_AUTHORIZATION_IDTAG_LEN);
        cache_record->status = stopTransaction.idTagInfo.AuthorizationStatus;

        cache_record->expiryDate = NULL;
        if (stopTransaction.idTagInfo.expiryDateIsUse)
        {
            struct tm *time = calloc(1, sizeof(struct tm));
            strptime(stopTransaction.idTagInfo.expiryDate, "%Y-%m-%dT%H:%M:%S.", time);
            cache_record->expiryDate = time;
        }

        if (stopTransaction.idTagInfo.parentIdTagIsUse)
        {
            strncpy(cache_record->parentIdTag, stopTransaction.idTagInfo.parentIdTag, OCPP_AUTHORIZATION_IDTAG_LEN);
        }

        ocpp_localAuthorization_Cache_write(cache_record);

        free(cache_record->expiryDate);
        free(cache_record);
    }
    break;

    default:
        OCPP_LOG_ERROR("action error");
        break;
    }
}

/**
 * @description:
 * @return {*}
 */
static void ocpp_chargePoint_CallError_Handler(json_object *jobj, enum OCPP_PACKAGE package)
{
    if (jobj == NULL)
        return;
    json_object *payload_obj = NULL;
    payload_obj = json_object_array_get_idx(jobj, 2);
    if (payload_obj == NULL)
        return;

    json_object *uniqueId_obj = NULL;
    uniqueId_obj = json_object_array_get_idx(jobj, 1);
    if (uniqueId_obj == NULL)
        return;
    const char *uniqueId_str = json_object_get_string(uniqueId_obj);

    switch (package)
    {

    case OCPP_PACKAGE_AUTHORIZE:
    {
        int i = 0;
        // find the connector when transaction
        for (i = 0; i < ocpp_chargePoint->numberOfConnector; i++)
        {
            char *lastUniqueId = ocpp_transaction_getLastUniqueId(i + 1);

            if (lastUniqueId != NULL)
            {
                if (strcmp(lastUniqueId, uniqueId_str) == 0)
                {
                    free(lastUniqueId);
                    break;
                }
                free(lastUniqueId);
            }
        }
        if (i >= ocpp_chargePoint->numberOfConnector)
            return;

        ocpp_package_Authorize_conf_t authorize_conf;
        authorize_conf.idTagInfo.AuthorizationStatus = OCPP_LOCAL_AUTHORIZATION_INVALID;
        ocpp_transaction_setAuthorize_Conf(i + 1, authorize_conf);
    }
    break;

    case OCPP_PACKAGE_BOOT_NOTIFICATION:

        break;

    case OCPP_PACKAGE_DATATRANSFER:
        break;

    case OCPP_PACKAGE_DIAGNOSTICS_STATUS_NOTIFICATION:
        break;

    case OCPP_PACKAGE_FIRMWARE_STATUS_NOTIFICATION:
        break;

    case OCPP_PACKAGE_HEARTBEAT:
        break;

    case OCPP_PACKAGE_METERVALUES:
    {

        // 离线数据接收确认
        if (ocpp_chargePoint->offlineDate_obj.isHaveDate)
        {
            if (strcmp(ocpp_chargePoint->offlineDate_obj.lastUniqueId, uniqueId_str) == 0)
            {
                ocpp_chargePoint->offlineDate_obj.isRetransmission = true;
            }
        }
    }
    break;

    case OCPP_PACKAGE_STARTTRANSACTION:
    case OCPP_PACKAGE_STOPTRANSACTION:
    {
        int i = 0;
        for (i = 1; i <= ocpp_chargePoint->numberOfConnector; i++)
        {
            if (ocpp_chargePoint->transaction_obj[i]->isTransaction)
                if (strcmp(ocpp_chargePoint->transaction_obj[i]->lastUniqueId, uniqueId_str) == 0)
                {
                    ocpp_chargePoint->transaction_obj[i]->isRetransmission = true;
                    break;
                }
        }

        // 离线数据接收错误
        if (ocpp_chargePoint->offlineDate_obj.isHaveDate)
        {
            if (strcmp(ocpp_chargePoint->offlineDate_obj.lastUniqueId, uniqueId_str) == 0)
            {
                ocpp_chargePoint->offlineDate_obj.isRetransmission = true;
            }
        }
    }
    break;

    case OCPP_PACKAGE_STATUS_NOTIFICATION:
        break;
    default:
        break;
    }
}

/**
 * @description:
 * @param {enum OCPP_PACKAGE} lastAction
 * @return {*}
 */
static void ocpp_chargePoint_Recv_timeout_Handler(enum OCPP_PACKAGE lastAction, char *lastUniqueId)
{

    switch (lastAction)
    {
    case OCPP_PACKAGE_AUTHORIZE:
    {
        int i = 0;
        for (i = 0; i < ocpp_chargePoint->numberOfConnector; i++)
        {
            if (ocpp_chargePoint->authorizetion[i]->isWaitAuthoriza)
                if (strcmp(lastUniqueId, ocpp_chargePoint->authorizetion[i]->lastUniqueId) == 0)
                    ocpp_chargePoint->setAuthorizeResult(i, false);
        }
    }
    break;

    case OCPP_PACKAGE_METERVALUES:
    case OCPP_PACKAGE_STARTTRANSACTION:
    case OCPP_PACKAGE_STOPTRANSACTION:
    {
        int connector = 1;
        for (connector = 1; connector <= ocpp_chargePoint->numberOfConnector; connector++)
        {
            if (ocpp_chargePoint->transaction_obj[connector]->isTransaction)
                if (strcmp(ocpp_chargePoint->transaction_obj[connector]->lastUniqueId, lastUniqueId) == 0)
                {
                    ocpp_chargePoint->transaction_obj[connector]->isRetransmission = true;
                    break;
                }
        }

        // 离线数据接收超时
        if (ocpp_chargePoint->offlineDate_obj.isHaveDate)
        {
            if (strcmp(ocpp_chargePoint->offlineDate_obj.lastUniqueId, lastUniqueId) == 0)
            {
                ocpp_chargePoint->offlineDate_obj.isRetransmission = true;
            }
        }
    }
    break;

    default:
        break;
    }
}

/**
 * @description:
 * @return {*}
 */
static void ocpp_chargePoint_Recv_Handler()
{

    if (ocpp_list_isRecv())
    {
        //        OCPP_LOG_DEBUG("开始解析");
        ocpp_list_RecvItem_t *recvData = ocpp_list_readAndDeleteFirstRecvItem();
        enum OCPP_PACKAGE lastAction = recvData->package;
        char *lastUniqueId = recvData->unique;
        //        OCPP_LOG_INFO("[%s] - [%s] - [RECV] = %s\n", lastUniqueId, ocpp_package_text[lastAction], recvData->message);

        // 接收超时处理，通知相应的交易线程重传交易信息
        if (strcmp(recvData->message, "ERROR") == 0)
        {

            OCPP_LOG_DEBUG("transaction recv error");
            // 接收超时处理
            ocpp_chargePoint_Recv_timeout_Handler(lastAction, lastUniqueId);
        }

        // 接收正确处理
        enum json_tokener_error err;
        json_object *jobj = json_tokener_parse_verbose((char *)recvData->message, &err);

        if (jobj != NULL)
        {

            // 获取消息类型
            json_object *messageTypeId_obj = NULL;
            messageTypeId_obj = json_object_array_get_idx(jobj, 0);

            // 获取消息ID
            json_object *uniqueId_obj = NULL;
            uniqueId_obj = json_object_array_get_idx(jobj, 1);

            if (messageTypeId_obj != NULL && uniqueId_obj != NULL)
            {

                int messageType = json_object_get_int(messageTypeId_obj);
                const char *uniqueId_str = json_object_get_string(uniqueId_obj);

                switch (messageType)
                {
                case OCPP_PACKAGE_CALL_MESSAGE:
                    ocpp_chargePoint_Call_Handler(jobj);
                    break;

                case OCPP_PACKAGE_CALL_RESULT:
                    ocpp_chargePoint_CallResult_Handler(jobj, lastAction);
                    break;

                case OCPP_PACKAGE_CALL_ERROR:
                    ocpp_chargePoint_CallError_Handler(jobj, lastAction);
                    break;
                }
            }
        }

        ocpp_list_FreeRecvItem(recvData);
    }
}

///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void *ocpp_chargePoint_offlineDate_Handler_thread(void *arg)
{

    ocpp_chargePoint->offlineDate_obj.isResponse = true;
    ocpp_chargePoint->offlineDate_obj.isRetransmission = false;
    memset(ocpp_chargePoint->offlineDate_obj.lastUniqueId, 0, 40);

#define OFFLINE_DATA_MAX 20

    int transactionMessageAttempts = 0;
    ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[OCPP_CK_TransactionMessageAttempt], (void *)&transactionMessageAttempts);

    int transactionMessageRetryInterval = 0;
    ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[OCPP_CK_TransactionMessageRetryInterval], (void *)&transactionMessageRetryInterval);

    if (transactionMessageRetryInterval <= 0)
        transactionMessageRetryInterval = 5;

    int attempts = 0;
    unsigned int RetriesInterval = 0;

    int sendCnt = 0;

    ocpp_offline_offlineData_item_t **item = NULL;
    ocpp_offline_offlineData_item_t *sendItem = NULL;
    int index = 0;

    while (1)
    {

        if (sendCnt <= 0)
        {
            free(item);

            sendCnt = ocpp_offline_OfflineDatasCnt();
            if (sendCnt > OFFLINE_DATA_MAX)
                sendCnt = OFFLINE_DATA_MAX;
            else if (sendCnt <= 0)
                return;

            item = ocpp_offline_readNData(&sendCnt);
            index = 0;
        }

        if (ocpp_chargePoint->offlineDate_obj.isResponse)
        {
            free(sendItem);

            sendCnt--;
            sendItem = item[index++];
            ocpp_chargePoint->offlineDate_obj.isResponse = false;
            strncpy(ocpp_chargePoint->offlineDate_obj.lastUniqueId, sendItem->unique, 40);
            ocpp_chargePoint_send(sendItem->unique, sendItem->message, sendItem->Action);

            attempts++;
            RetriesInterval = ocpp_AuxiliaryTool_getSystemTime_ms();
        }
        else
        {

            // need to retry
            if (ocpp_chargePoint->offlineDate_obj.isRetransmission)
            {
                ocpp_chargePoint->offlineDate_obj.isRetransmission = false;

                if (attempts < transactionMessageAttempts)
                {
                    if (ocpp_AuxiliaryTool_getDiffTime_ms(&RetriesInterval) >= attempts * transactionMessageRetryInterval)
                    {
                        attempts++;
                        RetriesInterval = ocpp_AuxiliaryTool_getSystemTime_ms();
                        ocpp_chargePoint_send(sendItem->unique, sendItem->message, sendItem->Action);
                    }
                }
            }
        }

        usleep(100 * 1000); // 100ms
    }

    ocpp_chargePoint->offlineDate_obj.isCreate = false;
    ocpp_chargePoint->offlineDate_obj.isHaveDate = false;
}

/**
 * @description:
 * @param
 * @return
 */
void *ocpp_chargePoint_client(void *arg)
{
    OCPP_LOG_INFO("create client thread\n");
    int reserveTime_ms = 0; // 检查预约是否过期时间

    // 创建boot线程
    pthread_t tid_boot;
    if (pthread_create(&tid_boot, NULL, ocpp_chargePoint_boot, NULL) != 0)
    {
        OCPP_LOG_ERROR("cann't create boot thread %s\n", strerror(errno));
    }
    pthread_detach(tid_boot);

    // 创建状态通知线程
    pthread_t tid_status;
    if (pthread_create(&tid_status, NULL, ocpp_chargePoint_StatusNotification, NULL) != 0)
    {
        OCPP_LOG_ERROR("cann't create status thread %s\n", strerror(errno));
    }
    pthread_detach(tid_status);

    while (1)
    {

        ocpp_chargePoint_Recv_Handler();

        // disconnect reset flag
        if (ocpp_chargePoint->connect.isConnect == false)
        {
            ocpp_chargePoint->RegistrationStatus = OCPP_PACKAGE_REGISTRATION_STATUS_MAX;
        }
        if (ocpp_chargePoint->RegistrationStatus == OCPP_PACKAGE_REGISTRATION_STATUS_ACCEPTED)
        {
            // //上传未结算订单
            // 判断是否有离线数据需要上传
            if (ocpp_offline_OfflineDatasCnt() > 0 && ocpp_chargePoint->offlineDate_obj.isCreate)
            {
                ocpp_chargePoint->offlineDate_obj.isCreate = true;
                ocpp_chargePoint->offlineDate_obj.isHaveDate = true;

                pthread_t tid_offline;
                if (pthread_create(&tid_offline, NULL, ocpp_chargePoint_offlineDate_Handler_thread, NULL) == -1)
                {
                    OCPP_LOG_ERROR("create offline handler thread fail");
                }
                pthread_detach(tid_offline);
            }
        }

        // 设置交易模块服务器连接状态
        //        bool isConnectorServer = false;
        //        if(ocpp_chargePoint->RegistrationStatus == OCPP_PACKAGE_REGISTRATION_STATUS_ACCEPTED){
        //            isConnectorServer = true;
        //        }
        //
        //        int num = 0;
        //        for(num = 0; num < ocpp_chargePoint->numberOfConnector; num++){
        //            ocpp_transaction_setServerStatus(num + 1, isConnectorServer);
        //
        //        }

        // 版本二
        int connector = 0;
        for (connector = 0; connector < ocpp_chargePoint->numberOfConnector; connector++)
        {
            char idTag[20] = {0};

            // pile not fail or unAvailbale
            if (ocpp_chargePoint->connector[0]->status == OCPP_PACKAGE_CHARGEPOINT_STATUS_AVAILABLE)
            {

                // user start Transaction
                if (ocpp_chargePoint->transaction_obj[connector + 1]->isStart)
                {
                    OCPP_LOG_DEBUG("user start charging...");
                    // reset isStart flag
                    ocpp_chargePoint->transaction_obj[connector + 1]->isStart = false;

                    // not transaction
                    if (ocpp_chargePoint->transaction_obj[connector + 1]->isTransaction == false)
                    {

                        OCPP_LOG_DEBUG("can transaction...");
                        OCPP_LOG_DEBUG("stop flag  %d", ocpp_chargePoint->transaction_obj[connector + 1]->isStop);
                        bool isAllowTransaction = false;
                        // put the gun
                        if (ocpp_chargePoint->connector[connector + 1]->status == OCPP_PACKAGE_CHARGEPOINT_STATUS_PREPARING)
                        {
                            isAllowTransaction = true;
                        }
                        else if (ocpp_chargePoint->connector[connector + 1]->status == OCPP_PACKAGE_CHARGEPOINT_STATUS_RESERVED)
                        {
                            if (strcmp(ocpp_chargePoint->reserveConnector[connector + 1]->idTag, ocpp_chargePoint->transaction_obj[connector + 1]->startIdTag) == 0)
                                isAllowTransaction = true;
                        }

                        if (isAllowTransaction)
                        {
                            OCPP_LOG_DEBUG("start charging...");
                            OCPP_LOG_DEBUG("stop flag  %d", ocpp_chargePoint->transaction_obj[connector + 1]->isStop);
                            pthread_t tid_transaction;
                            if (pthread_create(&tid_transaction, NULL, ocpp_chargePoint_Transaction_thread, (void *)(connector + 1)) == -1)
                            {
                                OCPP_LOG_ERROR("create transaction thread fail %s", strerror(errno));
                            }
                            pthread_detach(tid_transaction);
                        }
                    }
                }
            }

        } // for_end

        // 每5s判断预约时间是否过期
        reserveTime_ms += 100 * 1000;
        if (reserveTime_ms >= 5 * 1000 * 1000)
        {
            reserveTime_ms = 0;

            for (connector = 0; connector <= ocpp_chargePoint->numberOfConnector; connector++)
            {
                if (ocpp_chargePoint->isReservation[connector])
                {
                    //					printf("connector = %d\n", connector);
                    //					printf("years = %d\n", ocpp_chargePoint->reserveConnector[connector]->expiryDate.tm_year + 1900);
                    //					printf("month = %d\n", ocpp_chargePoint->reserveConnector[connector]->expiryDate.tm_mon);
                    //					printf("day = %d\n", ocpp_chargePoint->reserveConnector[connector]->expiryDate.tm_mday);
                    //					printf("hour = %d\n", ocpp_chargePoint->reserveConnector[connector]->expiryDate.tm_hour);
                    //					printf("min = %d\n", ocpp_chargePoint->reserveConnector[connector]->expiryDate.tm_min);
                    //					printf("sec = %d\n", ocpp_chargePoint->reserveConnector[connector]->expiryDate.tm_sec);

                    time_t now;
                    time(&now);
                    time_t reserveTime = mktime(&ocpp_chargePoint->reserveConnector[connector]->expiryDate);
                    if (difftime(reserveTime, now) <= 0.0)
                    {
                        printf("connector = %d  remove reserve = %d\n", connector, ocpp_chargePoint->reserveConnector[connector]->reservationId);
                        ocpp_chargePoint->isReservation[connector] = false;
                        ocpp_reserve_removeReservation(ocpp_chargePoint->reserveConnector[connector]->reservationId);
                    }
                }
            }
        }

        usleep(100 * 1000);
    }
}

/**
 * @description: 回调函数如果为NULL,则赋予默认值
 * @param
 * @return
 */
static void ocpp_chargePoint_defaultFunc()
{
    if (ocpp_chargePoint->getChargeBoxSerialNumber == NULL)
        ocpp_chargePoint->getChargeBoxSerialNumber = ocpp_chargePoint_getDefault;

    if (ocpp_chargePoint->getChargePointModel == NULL)
        ocpp_chargePoint->getChargePointModel = ocpp_chargePoint_getDefault;

    if (ocpp_chargePoint->getChargePointSerialNumber == NULL)
        ocpp_chargePoint->getChargePointSerialNumber = ocpp_chargePoint_getDefault;

    if (ocpp_chargePoint->getChargePointVendor == NULL)
        ocpp_chargePoint->getChargePointVendor = ocpp_chargePoint_getDefault;

    if (ocpp_chargePoint->getFirmwareVersion == NULL)
        ocpp_chargePoint->getFirmwareVersion = ocpp_chargePoint_getDefault;

    if (ocpp_chargePoint->getIccid == NULL)
        ocpp_chargePoint->getIccid = ocpp_chargePoint_getDefault;

    if (ocpp_chargePoint->getImsi == NULL)
        ocpp_chargePoint->getImsi = ocpp_chargePoint_getDefault;

    if (ocpp_chargePoint->getMeterSerialNumber == NULL)
        ocpp_chargePoint->getMeterSerialNumber = ocpp_chargePoint_getDefault;

    if (ocpp_chargePoint->getMeterType == NULL)
        ocpp_chargePoint->getMeterType = ocpp_chargePoint_getDefault;

    // 电表值相关
    if (ocpp_chargePoint->getVoltage == NULL)
        ocpp_chargePoint->getVoltage = ocpp_chargePoint_getDefaultMeterValues;

    if (ocpp_chargePoint->getTemperature == NULL)
        ocpp_chargePoint->getTemperature = ocpp_chargePoint_getDefaultMeterValues;

    if (ocpp_chargePoint->getSOC == NULL)
        ocpp_chargePoint->getSOC = ocpp_chargePoint_getDefaultMeterValues;

    if (ocpp_chargePoint->getRPM == NULL)
        ocpp_chargePoint->getRPM = ocpp_chargePoint_getDefaultMeterValues;

    if (ocpp_chargePoint->getPowerReactiveImport == NULL)
        ocpp_chargePoint->getPowerReactiveImport = ocpp_chargePoint_getDefaultMeterValues;

    if (ocpp_chargePoint->getPowerReactiveExport == NULL)
        ocpp_chargePoint->getPowerReactiveExport = ocpp_chargePoint_getDefaultMeterValues;

    if (ocpp_chargePoint->getPowerOffered == NULL)
        ocpp_chargePoint->getPowerOffered = ocpp_chargePoint_getDefaultMeterValues;

    if (ocpp_chargePoint->getPowerFactor == NULL)
        ocpp_chargePoint->getPowerFactor = ocpp_chargePoint_getDefaultMeterValues;

    if (ocpp_chargePoint->getPowerActiveImport == NULL)
        ocpp_chargePoint->getPowerActiveImport = ocpp_chargePoint_getDefaultMeterValues;

    if (ocpp_chargePoint->getPowerActiveExport == NULL)
        ocpp_chargePoint->getPowerActiveExport = ocpp_chargePoint_getDefaultMeterValues;

    if (ocpp_chargePoint->getFrequency == NULL)
        ocpp_chargePoint->getFrequency = ocpp_chargePoint_getDefaultMeterValues;

    if (ocpp_chargePoint->getEnergyReactiveExportInterval == NULL)
        ocpp_chargePoint->getEnergyReactiveExportInterval = ocpp_chargePoint_getDefaultMeterValues;

    if (ocpp_chargePoint->getEnergyReactiveImportInterval == NULL)
        ocpp_chargePoint->getEnergyReactiveImportInterval = ocpp_chargePoint_getDefaultMeterValues;

    if (ocpp_chargePoint->getEnergyActiveImportInterval == NULL)
        ocpp_chargePoint->getEnergyActiveImportInterval = ocpp_chargePoint_getDefaultMeterValues;

    if (ocpp_chargePoint->getEnergyActiveExportInterval == NULL)
        ocpp_chargePoint->getEnergyActiveExportInterval = ocpp_chargePoint_getDefaultMeterValues;

    if (ocpp_chargePoint->getEnergyReactiveImportRegister == NULL)
        ocpp_chargePoint->getEnergyReactiveImportRegister = ocpp_chargePoint_getDefaultMeterValues;

    if (ocpp_chargePoint->getEnergyReactiveExportRegister == NULL)
        ocpp_chargePoint->getEnergyReactiveExportRegister = ocpp_chargePoint_getDefaultMeterValues;

    if (ocpp_chargePoint->getEnergyActiveImportRegister == NULL)
        ocpp_chargePoint->getEnergyActiveImportRegister = ocpp_chargePoint_getDefaultMeterValues;

    if (ocpp_chargePoint->getEnergyActiveExportRegister == NULL)
        ocpp_chargePoint->getEnergyActiveExportRegister = ocpp_chargePoint_getDefaultMeterValues;

    if (ocpp_chargePoint->getCurrentOffered == NULL)
        ocpp_chargePoint->getCurrentOffered = ocpp_chargePoint_getDefaultMeterValues;

    if (ocpp_chargePoint->getCurrentImport == NULL)
        ocpp_chargePoint->getCurrentImport = ocpp_chargePoint_getDefaultMeterValues;

    if (ocpp_chargePoint->getCurrentExport == NULL)
        ocpp_chargePoint->getCurrentExport = ocpp_chargePoint_getDefaultMeterValues;

    //
    // if(ocpp_chargePoint->userPushStartButton == NULL)
    //     ocpp_chargePoint->userPushStartButton = ocpp_chargePoint_pushDefault;

    // if(ocpp_chargePoint->userPushStopButton == NULL)
    //     ocpp_chargePoint->userPushStopButton = ocpp_chargePoint_pushDefault;

    if (ocpp_chargePoint->userPushStartButton == NULL)
        ocpp_chargePoint->userPushStartButton = ocpp_chargePoint_userPushStartButton;

    if (ocpp_chargePoint->userPushStopButton == NULL)
        ocpp_chargePoint->userPushStopButton = ocpp_chargePoint_userPushStopButton;

    if (ocpp_chargePoint->startCharging == NULL)
        ocpp_chargePoint->startCharging = ocpp_chargePoint_setDefault;

    // if(ocpp_chargePoint->stopCharging == NULL)
    //     ocpp_chargePoint->stopCharging = ocpp_chargePoint_setDefault;

    if (ocpp_chargePoint->remoteStopCharging == NULL)
        ocpp_chargePoint->remoteStopCharging = ocpp_chargePoint_setDefault;
}

/**
 * @description:
 * @param {ocpp_chargePoint_t} *ocpp_chargePoint
 * @return {*}
 */
void ocpp_chargePoint_init(ocpp_chargePoint_t *pile)
{

    // 先创建日志线程，确保保存所有日志

    pthread_t ptid_log;
    if (pthread_create(&ptid_log, NULL, ocpp_log_record_thread, NULL) != 0)
    {
        printf("create log thread fail\n");
    }
    pthread_detach(ptid_log);
    usleep(5 * 1000 * 1000);
    ocpp_chargePoint = pile;

    /////////////////-test_debug-////////////////////
    OCPP_LOG_DEBUG("pile = %p  point = %p", pile, ocpp_chargePoint);
    OCPP_LOG_DEBUG("isWss = %d\n", ocpp_chargePoint->connect.isWss);
    OCPP_LOG_DEBUG("ssl_ca_filepath = %s", ocpp_chargePoint->connect.ssl_ca_filepath);
    OCPP_LOG_DEBUG("protocolName = %s", ocpp_chargePoint->connect.protocolName);
    OCPP_LOG_DEBUG("username = %s", ocpp_chargePoint->connect.username);
    OCPP_LOG_DEBUG("password = %s", ocpp_chargePoint->connect.password);
    OCPP_LOG_DEBUG("address = %s", ocpp_chargePoint->connect.address);
    OCPP_LOG_DEBUG("port = %d", ocpp_chargePoint->connect.port);
    OCPP_LOG_DEBUG("path = %s", ocpp_chargePoint->connect.path);

    ////////////////////////////////////////////////

    ocpp_chargePoint_defaultFunc();
    ocpp_chargePoint->userPushStartButton = ocpp_chargePoint_userPushStartButton;
    ocpp_chargePoint->userPushStopButton = ocpp_chargePoint_userPushStopButton;

    ocpp_chargePoint->RegistrationStatus = OCPP_PACKAGE_REGISTRATION_STATUS_REJECTED;
    ocpp_chargePoint->setConnectorStatus = setConnectorStatus;
    ocpp_chargePoint->setConnectorErrInfo = setConnectorErrInfo;

    // 打开数据库
    if (sqlite3_open(OCPP_DATABASE_FILE_NAME, &(ocpp_chargePoint->ocpp_db)) != SQLITE_OK)
    {
        printf("打开OCPP数据库失败\n");
    }

    // 初始化OCPP配置
    if (ocpp_ConfigurationKey_init(ocpp_chargePoint->ocpp_db) == -1)
        OCPP_LOG_ERROR("ocpp ConfigurationKey init fail\n");

    // 初始化OCPP本地认证
    ocpp_localAuthorization_init(ocpp_chargePoint->ocpp_db);

    // 初始化 offline
    ocpp_offline_init(ocpp_chargePoint->ocpp_db);

    // 初始化 Reserve
    ocpp_reserve_init(ocpp_chargePoint->ocpp_db);
    ocpp_chargePoint->reserveConnector = (ocpp_reserve_t **)calloc(ocpp_chargePoint->numberOfConnector + 1, sizeof(ocpp_reserve_t *));
    ocpp_chargePoint->isReservation = (char *)calloc(ocpp_chargePoint->numberOfConnector + 1, sizeof(char));
    int i = 0;
    for (i = 0; i <= ocpp_chargePoint->numberOfConnector; i++)
    {
        ocpp_chargePoint->reserveConnector[i] = (ocpp_reserve_t *)calloc(1, sizeof(ocpp_reserve_t));
    }

    // read Reserve
    for (i = 0; i <= ocpp_chargePoint->numberOfConnector; i++)
    {
        if (ocpp_reserve_readReservation(i, ocpp_chargePoint->reserveConnector[i]) == 0)
            ocpp_chargePoint->isReservation[i] = true;
    }

    // 初始化认证状态
    ocpp_chargePoint->authorizetion = (ocpp_chargePoint_Authoriza_t **)calloc(ocpp_chargePoint->numberOfConnector + 1, sizeof(ocpp_chargePoint_Authoriza_t *));
    for (i = 0; i <= ocpp_chargePoint->numberOfConnector; i++)
    {
        ocpp_chargePoint->authorizetion[i] = (ocpp_chargePoint_Authoriza_t *)calloc(1, sizeof(ocpp_chargePoint_Authoriza_t));
    }

    // 初始化交易对象
    ocpp_chargePoint->transaction_obj = (ocpp_chargePoint_transaction_t **)calloc(ocpp_chargePoint->numberOfConnector + 1, sizeof(ocpp_chargePoint_transaction_t *));
    for (i = 0; i <= ocpp_chargePoint->numberOfConnector; i++)
    {
        ocpp_chargePoint->transaction_obj[i] = (ocpp_chargePoint_transaction_t *)calloc(1, sizeof(ocpp_chargePoint_transaction_t));
    }

    // base number of connector create object , connector = 0 indicate pile not is connector
    OCPP_LOG_DEBUG("numberOfConnector = %d\n", ocpp_chargePoint->numberOfConnector);
    ocpp_chargePoint->connector = (ocpp_package_StatusNotification_req_t **)calloc(ocpp_chargePoint->numberOfConnector + 1, sizeof(ocpp_package_StatusNotification_req_t *));
    for (i = 0; i <= ocpp_chargePoint->numberOfConnector; i++)
    {
        ocpp_chargePoint->connector[i] = (ocpp_package_StatusNotification_req_t *)calloc(1, sizeof(ocpp_package_StatusNotification_req_t));
        ocpp_chargePoint->connector[i]->errorCode = OCPP_PACKAGE_CHARGEPOINT_ERRORCODE_NOERROR;
        ocpp_chargePoint->connector[i]->status = OCPP_PACKAGE_CHARGEPOINT_STATUS_AVAILABLE;
    }

    //////////////////////////////////////
    ocpp_transaction_init(ocpp_chargePoint->numberOfConnector);

    //////////////////////////////////////
    ocpp_connect_init(&(ocpp_chargePoint->connect));
    ocpp_list_init(&(ocpp_chargePoint->connect));

    // 创建客户端线程
    pthread_t tid_client;
    if (pthread_create(&tid_client, NULL, ocpp_chargePoint_client, NULL) != 0)
    {
        OCPP_LOG_ERROR("cann't create client thread %s\n", strerror(errno));
    }
    pthread_detach(tid_client);

    OCPP_LOG_INFO("initial end");
}

//////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////-TEST-/////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
static bool getChargePointVendor(char *const str, int len)
{

    snprintf(str, len, "%s", "increase");

    return true;
}

static bool getChargePointModel(char *const str, int len)
{
    snprintf(str, len, "%s", "DC Charing");

    return true;
}

int main()
{
    printf("in main\n");

    ocpp_chargePoint_t client;
    memset(&client, 0, sizeof(ocpp_chargePoint_t));
    client.connect.isWss = true;
    client.connect.ssl_ca_filepath = "/app/core/ssl/ca/cacert.pem";
    client.connect.protocolName = "ocpp1.6";
    client.connect.username = "h5wa015ajqA2Eg8D";
    client.connect.password = "LDhf7qM2Pw5P8p92";

    // client.connect.address = "ocpp.glcharge.com";
    // client.connect.port = 80;
    // client.connect.path = "/v16/DCCharger123";

    client.connect.address = "evcs.hetivert.com";
    client.connect.port = 443;
    client.connect.path = "/inc-server/inc-chargecloud-websocket/inc-websocket/2023040310001001";

    // 测试回调函数
    client.getChargePointVendor = getChargePointVendor;
    client.getChargePointModel = getChargePointModel;

    ocpp_chargePoint_init(&client);

    while (1)
    {
    }
}
