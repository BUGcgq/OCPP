/*
 * @Author: LIYAOHAN liyaohan@increase.com
 * @Date: 2023-04-08 10:37:48
 * @LastEditors: LIYAOHAN 1791002655@qq.com
 * @LastEditTime: 2023-05-05 19:06:40
 * @FilePath: \功能模块\OCPP\ocpp_package.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "ocpp_log.h"
#include "ocpp_package.h"
#include <json-c/json.h>

/**
 * @description: 创建Authorize.req消息
 * @param:
 * @return:
 */
char *ocpp_package_prepare_Authorize_Request(const char *UniqueId, ocpp_package_Authorize_req_t *Authorize)
{
    if (UniqueId == NULL || Authorize == NULL)
    {
        return NULL;
    }

    struct json_object *root_object = json_object_new_array();

    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_MESSAGE));
    json_object_array_add(root_object, json_object_new_string(UniqueId));
    json_object_array_add(root_object, json_object_new_string("Authorize"));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "idTag", json_object_new_string(Authorize->idTag));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);
    char *message = strdup(json_string);

    json_object_put(root_object);

    return message;
}

/**
 * @description: 创建BootNotification.req消息
 * @param:
 * @return:
 */
char *ocpp_package_prepare_BootNotification_Request(const char *UniqueId, ocpp_package_BootNotification_req_t *BootNotification)
{
    if (UniqueId == NULL || BootNotification == NULL)
    {
        return NULL;
    }

    struct json_object *root_object = json_object_new_array();

    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_MESSAGE));
    json_object_array_add(root_object, json_object_new_string(UniqueId));
    json_object_array_add(root_object, json_object_new_string("BootNotification"));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "chargePointModel", json_object_new_string(BootNotification->chargePointModel));

    if (BootNotification->chargeBoxSerialNumberIsUse)
    {
        json_object_object_add(payload_object, "chargeBoxSerialNumber", json_object_new_string(BootNotification->chargeBoxSerialNumber));
    }

    if (BootNotification->chargePointSerialNumberIsUse)
    {
        json_object_object_add(payload_object, "chargePointSerialNumber", json_object_new_string(BootNotification->chargePointSerialNumber));
    }

    json_object_object_add(payload_object, "chargePointVendor", json_object_new_string(BootNotification->chargePointVendor));

    if (BootNotification->firmwareVersionIsUse)
    {
        json_object_object_add(payload_object, "firmwareVersion", json_object_new_string(BootNotification->firmwareVersion));
    }

    if (BootNotification->iccidIsUse)
    {
        json_object_object_add(payload_object, "iccid", json_object_new_string(BootNotification->iccid));
    }

    if (BootNotification->imsiIsUse)
    {
        json_object_object_add(payload_object, "imsi", json_object_new_string(BootNotification->imsi));
    }

    if (BootNotification->meterSerialNumberIsUse)
    {
        json_object_object_add(payload_object, "meterSerialNumber", json_object_new_string(BootNotification->meterSerialNumber));
    }

    if (BootNotification->meterTypeIsUse)
    {
        json_object_object_add(payload_object, "meterType", json_object_new_string(BootNotification->meterType));
    }

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);
    char *message = strdup(json_string);

    json_object_put(root_object);

    return message;
}

/**
 * @description: 创建DataTransfer.req消息
 * @param:
 * @return:
 */
char *ocpp_package_prepare_DataTransfer_Request(const char *UniqueId, ocpp_package_DataTransfer_req_t *DataTransfer)
{
    if (UniqueId == NULL || DataTransfer == NULL)
    {
        return NULL;
    }
    struct json_object *root_object = json_object_new_array();

    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_MESSAGE));
    json_object_array_add(root_object, json_object_new_string(UniqueId));
    json_object_array_add(root_object, json_object_new_string("DataTransfer"));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "vendorId", json_object_new_string(DataTransfer->vendorId));

    if (DataTransfer->messageIdIsUse)
    {
        json_object_object_add(payload_object, "messageId", json_object_new_string(DataTransfer->messageId));
    }

    if (DataTransfer->dataIsUse && DataTransfer->dataLen > 0)
    {
        json_object_object_add(payload_object, "data", json_object_new_string_len(DataTransfer->data, DataTransfer->dataLen));
    }

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);
    char *message = strdup(json_string);

    json_object_put(root_object);

    return message;
}

/**
 * @description: 创建DiagnosticsStatusNotification.req消息
 * @param:
 * @return:
 */
char *ocpp_package_prepare_DiagnosticsStatusNotification_Request(const char *UniqueId, ocpp_package_DiagnosticsStatusNotification_req_t *DiagnosticsStatusNotification)
{
    if (UniqueId == NULL || DiagnosticsStatusNotification == NULL)
    {
        return NULL;
    }
    struct json_object *root_object = json_object_new_array();

    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_MESSAGE));
    json_object_array_add(root_object, json_object_new_string(UniqueId));
    json_object_array_add(root_object, json_object_new_string("DiagnosticsStatusNotification"));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_DiagnosticsStatus_text[DiagnosticsStatusNotification->status]));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);
    char *message = strdup(json_string);

    json_object_put(root_object);

    return message;
}

/**
 * @description: 创建FirmwareStatusNotification.req消息
 * @param:
 * @return:
 */
char *ocpp_package_prepare_FirmwareStatusNotification_Request(const char *UniqueId, ocpp_package_FirmwareStatusNotification_req_t *FirmwareStatusNotification)
{
    if (UniqueId == NULL || FirmwareStatusNotification == NULL)
    {
        return NULL;
    }
    struct json_object *root_object = json_object_new_array();

    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_MESSAGE)); // OCPP_PACKAGE_CALL_MESSAGE
    json_object_array_add(root_object, json_object_new_string(UniqueId));
    json_object_array_add(root_object, json_object_new_string("DiagnosticsStatusNotification"));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_FirmwareStatus[FirmwareStatusNotification->status]));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);
    char *message = strdup(json_string);

    json_object_put(root_object);

    return message;
}

/**
 * @description: 创建 Heartbeat.req消息
 * @param:
 * @return:
 */
char *ocpp_package_prepare_Heartbeat_Request(const char *UniqueId, ocpp_package_Heartbeat_req_t *Heartbeat)
{
    if (UniqueId == NULL || Heartbeat == NULL)
    {
        return NULL;
    }
    struct json_object *root_object = json_object_new_array();

    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_MESSAGE));
    json_object_array_add(root_object, json_object_new_string(UniqueId));
    json_object_array_add(root_object, json_object_new_string("Heartbeat"));

    // 添加空对象{}，即使没有附带额外数据
    struct json_object *empty_object = json_object_new_object();
    json_object_array_add(root_object, empty_object);

    const char *json_string = json_object_to_json_string(root_object);
    char *message = strdup(json_string);

    json_object_put(root_object);
    return message;
}

/**
 * @description: 创建 MeterValues.req消息
 * @param:
 * @return:
 */
char *ocpp_package_prepare_MeterValues_Request(const char *UniqueId, ocpp_package_MeterValues_req_t *MeterValues)
{
    if (UniqueId == NULL || MeterValues == NULL)
    {
        return NULL;
    }
    struct json_object *root_object = json_object_new_array();

    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_MESSAGE));
    json_object_array_add(root_object, json_object_new_string(UniqueId));
    json_object_array_add(root_object, json_object_new_string("MeterValues"));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "connectorId", json_object_new_int(MeterValues->connectorId));

    if (MeterValues->transactionIdIsUse)
        json_object_object_add(payload_object, "transactionId", json_object_new_int(MeterValues->transactionId));

    struct json_object *meterValue_array = json_object_new_array();
    struct json_object *meterValue_object = json_object_new_object();

    json_object_object_add(meterValue_object, "timestamp", json_object_new_string(MeterValues->meterValue.timestamp));

    struct json_object *sampledValue_array = json_object_new_array();
    int i;
    for (i = 0; i < MeterValues->meterValue.sampledValueCnt; i++)
    {
        struct json_object *sampledValue_object = json_object_new_object();
        json_object_object_add(sampledValue_object, "value", json_object_new_string(MeterValues->meterValue.sampledValue[i]->value));

        if (MeterValues->meterValue.sampledValue[i]->contextIsUse)
            json_object_object_add(sampledValue_object, "context", json_object_new_string(ocpp_package_MeterValues_MS_context_text[MeterValues->meterValue.sampledValue[i]->context]));

        if (MeterValues->meterValue.sampledValue[i]->formatIsUse)
            json_object_object_add(sampledValue_object, "format", json_object_new_string(ocpp_package_MeterValues_MS_format_text[MeterValues->meterValue.sampledValue[i]->format]));

        if (MeterValues->meterValue.sampledValue[i]->measurandIsUse)
            json_object_object_add(sampledValue_object, "measurand", json_object_new_string(ocpp_package_MeterValues_MS_measurand_text[MeterValues->meterValue.sampledValue[i]->measurand]));

        if (MeterValues->meterValue.sampledValue[i]->phaseIsUse)
            json_object_object_add(sampledValue_object, "phase", json_object_new_string(ocpp_package_MeterValues_MS_phase_text[MeterValues->meterValue.sampledValue[i]->phase]));

        if (MeterValues->meterValue.sampledValue[i]->locationIsUse)
            json_object_object_add(sampledValue_object, "location", json_object_new_string(ocpp_package_MeterValues_MS_location_text[MeterValues->meterValue.sampledValue[i]->location]));

        if (MeterValues->meterValue.sampledValue[i]->unitIsUse)
            json_object_object_add(sampledValue_object, "unit", json_object_new_string(ocpp_package_MeterValues_MS_unit_text[MeterValues->meterValue.sampledValue[i]->unit]));

        json_object_array_add(sampledValue_array, sampledValue_object);
    }

    json_object_object_add(meterValue_object, "sampledValue", sampledValue_array);
    json_object_array_add(meterValue_array, meterValue_object);
    json_object_object_add(payload_object, "meterValue", meterValue_array);

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);
    char *message = strdup(json_string);

    json_object_put(root_object);

    return message;
}

/**
 * @description: 创建 StartTransaction.req消息
 * @param:
 * @return:
 */
char *ocpp_package_prepare_StartTransaction_Request(const char *UniqueId, ocpp_package_StartTransaction_req_t *StartTransaction)
{
    if (UniqueId == NULL || StartTransaction == NULL)
    {
        return NULL;
    }
    struct json_object *root_object = json_object_new_array();

    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_MESSAGE));
    json_object_array_add(root_object, json_object_new_string(UniqueId));
    json_object_array_add(root_object, json_object_new_string("StartTransaction"));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "connectorId", json_object_new_int(StartTransaction->connectorId));
    json_object_object_add(payload_object, "idTag", json_object_new_string(StartTransaction->idTag));
    json_object_object_add(payload_object, "meterStart", json_object_new_int(StartTransaction->meterStart));

    if (StartTransaction->reservationIdIsUse)
    {
        json_object_object_add(payload_object, "reservationId", json_object_new_int(StartTransaction->reservationId));
    }

    json_object_object_add(payload_object, "timestamp", json_object_new_string(StartTransaction->timestamp));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);
    char *message = strdup(json_string);

    json_object_put(root_object);

    return message;
}

/**
 * @description: 创建StatusNotification.req消息
 * @param:
 * @return:
 */
char *ocpp_package_prepare_StatusNotification_Request(const char *UniqueId, ocpp_package_StatusNotification_req_t *StatusNotification)
{
    if (UniqueId == NULL || StatusNotification == NULL)
    {
        return NULL;
    }
    struct json_object *root_object = json_object_new_array();

    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_MESSAGE));
    json_object_array_add(root_object, json_object_new_string(UniqueId));
    json_object_array_add(root_object, json_object_new_string("StatusNotification"));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "connectorId", json_object_new_int(StatusNotification->connectorId));
    json_object_object_add(payload_object, "errorCode", json_object_new_string(ocpp_package_ChargePointErrorCode_text[StatusNotification->errorCode]));
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_ChargePointStatus_text[StatusNotification->status]));

    if (StatusNotification->timestampIsUse)
        json_object_object_add(payload_object, "timestamp", json_object_new_string(StatusNotification->timestamp));

    if (StatusNotification->infoIsUse)
        json_object_object_add(payload_object, "info", json_object_new_string(StatusNotification->info));

    if (StatusNotification->vendorIdIsUse)
        json_object_object_add(payload_object, "vendorId", json_object_new_string(StatusNotification->vendorId));

    if (StatusNotification->vendorErrorCodeIsUse)
        json_object_object_add(payload_object, "vendorErrorCode", json_object_new_string(StatusNotification->vendorErrorCode));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);
    char *message = strdup(json_string);

    json_object_put(root_object);

    return message;
}

/**
 * @description: 创建StopTransaction.req消息
 * @param:
 * @return:
 */
char *ocpp_package_prepare_StopTransaction_Request(const char *UniqueId, ocpp_package_StopTransaction_req_t *StopTransaction)
{
    if (UniqueId == NULL || StopTransaction == NULL)
    {
        return NULL;
    }
    struct json_object *root_object = json_object_new_array();

    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_MESSAGE));
    json_object_array_add(root_object, json_object_new_string(UniqueId));
    json_object_array_add(root_object, json_object_new_string("StopTransaction"));

    struct json_object *payload_object = json_object_new_object();

    json_object_object_add(payload_object, "transactionId", json_object_new_int(StopTransaction->transactionId));
    json_object_object_add(payload_object, "idTag", json_object_new_string(StopTransaction->idTag));
    json_object_object_add(payload_object, "timestamp", json_object_new_string(StopTransaction->timestamp));
    json_object_object_add(payload_object, "meterStop", json_object_new_int(StopTransaction->meterStop));

    if (StopTransaction->reasonIsUse)
        json_object_object_add(payload_object, "reason", json_object_new_string(ocpp_package_stop_reason_text[StopTransaction->reasonIsUse]));

    if (StopTransaction->transactionDataIsUse)
    {
        struct json_object *transactionData_array = json_object_new_array();
        struct json_object *transactionData_object = json_object_new_object();

        json_object_object_add(transactionData_object, "timestamp", json_object_new_string(StopTransaction->transactionData.timestamp));

        struct json_object *values_array = json_object_new_array();
        int i;
        for (i = 0; i < StopTransaction->transactionData.sampledValueCnt; i++)
        {
            struct json_object *value_object = json_object_new_object();
            json_object_object_add(value_object, "measurand", json_object_new_string(ocpp_package_MeterValues_MS_measurand_text[StopTransaction->transactionData.sampledValue[i]->measurand]));
            json_object_object_add(value_object, "unit", json_object_new_string(ocpp_package_MeterValues_MS_unit_text[StopTransaction->transactionData.sampledValue[i]->unit]));
            json_object_object_add(value_object, "value", json_object_new_string(StopTransaction->transactionData.sampledValue[i]->value));
            json_object_array_add(values_array, value_object);
        }

        json_object_object_add(transactionData_object, "values", values_array);
        json_object_array_add(transactionData_array, transactionData_object);
        json_object_object_add(payload_object, "transactionData", transactionData_array);
    }

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);
    char *message = strdup(json_string);

    json_object_put(root_object);

    return message;
}

/**
 * @description: 创建CancelReservation.conf消息
 * @param:
 * @return:
 */
char *ocpp_package_prepare_CancelReservation_Response(const char *UniqueId, ocpp_package_CancelReservation_conf_t *CancelReservation)
{
    if (UniqueId == NULL || CancelReservation == NULL)
    {
        return NULL;
    }
    struct json_object *root_object = json_object_new_array();
    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_object, json_object_new_string(UniqueId));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_CancelReservationStatus_text[CancelReservation->status]));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);
    char *message = strdup(json_string);

    json_object_put(payload_object);
    json_object_put(root_object);

    return message;
}

/**
 * @description: 创建ChangeAvailability.conf消息
 * @param:
 * @return:
 */
char *ocpp_package_prepare_ChangeAvailability_Response(const char *UniqueId, ocpp_package_ChangeAvailability_conf_t *ChangeAvailability)
{
    if (UniqueId == NULL || ChangeAvailability == NULL)
    {
        return NULL;
    }
    struct json_object *root_object = json_object_new_array();
    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_object, json_object_new_string(UniqueId));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_AvailabilityStatus_text[ChangeAvailability->status]));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);
    char *message = strdup(json_string);

    json_object_put(payload_object);
    json_object_put(root_object);

    return message;
}

/**
 * @description: 创建ChangeConfiguration.conf消息
 * @param:
 * @return:
 */
char *ocpp_package_prepare_ChangeConfiguration_Response(const char *UniqueId, ocpp_package_ChangeConfiguration_conf_t *ChangeConfiguration)
{
    if (UniqueId == NULL || ChangeConfiguration == NULL)
    {
        return NULL;
    }
    struct json_object *root_object = json_object_new_array();
    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_object, json_object_new_string(UniqueId));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_ConfigurationStatus_text[ChangeConfiguration->status]));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);
    char *message = strdup(json_string);

    json_object_put(payload_object);
    json_object_put(root_object);

    return message;
}

/**
 * @description: 创建ClearCache.conf消息
 * @param:
 * @return:
 */
char *ocpp_package_prepare_ClearCache_Response(const char *UniqueId, ocpp_package_ClearCache_conf_t *ClearCache)
{
    if (UniqueId == NULL || ClearCache == NULL)
    {
        return NULL;
    }
    struct json_object *root_object = json_object_new_array();
    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_object, json_object_new_string(UniqueId));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_ClearCacheStatus_text[ClearCache->status]));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);
    char *message = strdup(json_string);

    json_object_put(payload_object);
    json_object_put(root_object);

    return message;
}

/**
 * @description: 创建ClearChargingProfile.conf消息
 * @param:
 * @return:
 */
char *ocpp_package_prepare_ClearChargingProfile_Response(const char *UniqueId, ocpp_package_ClearChargingProfile_conf_t *ClearChargingProfile)
{
    if (UniqueId == NULL || ClearChargingProfile == NULL)
    {
        return NULL;
    }
    struct json_object *root_object = json_object_new_array();
    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_object, json_object_new_string(UniqueId));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_ClearChargingProfileStatus_text[ClearChargingProfile->status]));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);
    char *message = strdup(json_string);

    json_object_put(payload_object);
    json_object_put(root_object);

    return message;
}

/**
 * @description: 创建DataTransfer.conf消息
 * @param:
 * @return:
 */
char *ocpp_package_prepare_DataTransfer_Response(const char *UniqueId, ocpp_package_DataTransfer_conf_t *DataTransfer)
{
    if (UniqueId == NULL || DataTransfer == NULL)
    {
        return NULL;
    }
    struct json_object *root_object = json_object_new_array();

    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_object, json_object_new_string(UniqueId));
    struct json_object *payload_object = json_object_new_object();

    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_DataTransferStatus_text[DataTransfer->status]));

    if (DataTransfer->dataIsUse && DataTransfer->dataLen > 0)
    {
        json_object_object_add(payload_object, "data", json_object_new_string(DataTransfer->data));
    }

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);
    char *message = strdup(json_string);

    json_object_put(root_object);

    return message;
}
/**
 * @description: 创建GetLocalListVersion.conf消息
 * @param:
 * @return:
 */
char *ocpp_package_prepare_GetLocalListVersion_Response(const char *UniqueId, ocpp_package_GetLocalListVersion_conf_t *GetLocalListVersion)
{
    if (UniqueId == NULL || GetLocalListVersion == NULL)
    {
        return NULL;
    }
    struct json_object *root_object = json_object_new_array();
    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_object, json_object_new_string(UniqueId));

    struct json_object *payload_object = json_object_new_object();

    json_object_object_add(payload_object, "listVersion", json_object_new_int(GetLocalListVersion->listVersion));
    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);
    char *message = strdup(json_string);

    json_object_put(root_object);

    return message;
}

/**
 * @description: 创建 GetCompositeSchedule.conf 消息
 * @param:
 * @return:
 */
char *ocpp_package_prepare_GetCompositeSchedule_Response(const char *UniqueId, ocpp_package_GetCompositeSchedule_conf_t *GetCompositeSchedule)
{
    if (UniqueId == NULL || GetCompositeSchedule == NULL)
    {
        return NULL;
    }
    struct json_object *root_object = json_object_new_array();
    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_object, json_object_new_string(UniqueId));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_GetCompositeScheduleStatus_text[GetCompositeSchedule->status]));

    if (GetCompositeSchedule->connectorIdIsUse)
    {
        json_object_object_add(payload_object, "connectorId", json_object_new_int(GetCompositeSchedule->connectorId));
    }

    if (GetCompositeSchedule->scheduleStartIsUse)
    {
        json_object_object_add(payload_object, "scheduleStart", json_object_new_string(GetCompositeSchedule->scheduleStart));
    }

    if (GetCompositeSchedule->chargingScheduleIsUse)
    {
        struct json_object *chargingSchedule_object = json_object_new_object();
        json_object_object_add(chargingSchedule_object, "chargingRateUnit", json_object_new_string(ocpp_package_ChargingRateUnitType_text[GetCompositeSchedule->chargingSchedule.chargingRateUnit]));

        if (GetCompositeSchedule->chargingSchedule.durationIsUse)
        {
            json_object_object_add(chargingSchedule_object, "duration", json_object_new_int(GetCompositeSchedule->chargingSchedule.duration));
        }

        if (GetCompositeSchedule->chargingSchedule.startScheduleIsUse)
        {
            json_object_object_add(chargingSchedule_object, "startSchedule", json_object_new_string(GetCompositeSchedule->chargingSchedule.startSchedule));
        }

        if (GetCompositeSchedule->chargingSchedule.minChargingRateIsUse)
        {
            json_object_object_add(chargingSchedule_object, "minChargingRate", json_object_new_double(GetCompositeSchedule->chargingSchedule.minChargingRate));
        }

        if (GetCompositeSchedule->chargingSchedule.numberOfchargingSchedulePeriod > 0)
        {
            struct json_object *chargingSchedulePeriod_array = json_object_new_array();
            int i;
            for (i = 0; i < GetCompositeSchedule->chargingSchedule.numberOfchargingSchedulePeriod; i++)
            {
                struct json_object *chargingSchedulePeriod_object = json_object_new_object();
                json_object_object_add(chargingSchedulePeriod_object, "startPeriod", json_object_new_int(GetCompositeSchedule->chargingSchedule.chargingSchedulePeriod[i]->startPeriod));
                json_object_object_add(chargingSchedulePeriod_object, "limit", json_object_new_double(GetCompositeSchedule->chargingSchedule.chargingSchedulePeriod[i]->limit));

                if (GetCompositeSchedule->chargingSchedule.chargingSchedulePeriod[i]->numberPhasesIsUse)
                {
                    json_object_object_add(chargingSchedulePeriod_object, "numberPhases", json_object_new_int(GetCompositeSchedule->chargingSchedule.chargingSchedulePeriod[i]->numberPhases));
                }

                json_object_array_add(chargingSchedulePeriod_array, chargingSchedulePeriod_object);
            }

            json_object_object_add(chargingSchedule_object, "chargingSchedulePeriod", chargingSchedulePeriod_array);
        }

        json_object_object_add(payload_object, "chargingSchedule", chargingSchedule_object);
    }

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);
    char *message = strdup(json_string);

    json_object_put(root_object);

    return message;
}

/**
 * @description: 创建 GetConfiguration.conf 消息
 * @param:
 * @return:
 */
char *ocpp_package_prepare_GetConfiguration_Response(const char *UniqueId, ocpp_package_GetConfiguration_conf_t *GetConfiguration)
{
    if (UniqueId == NULL || GetConfiguration == NULL)
    {
        return NULL;
    }
    struct json_object *root_object = json_object_new_array();
    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_object, json_object_new_string(UniqueId));

    struct json_object *payload_object = json_object_new_object();

    if (GetConfiguration->configurationKeyIsUse)
    {
        struct json_object *config_keys_array = json_object_new_array();

        int i = 0;
        for (i = 0; i < GetConfiguration->numberOfConfigurationKey; i++)
        {
            struct json_object *config_key_object = json_object_new_object();
            json_object_object_add(config_key_object, "key", json_object_new_string(GetConfiguration->configurationKey[i]->key));
            json_object_object_add(config_key_object, "readonly", json_object_new_boolean(GetConfiguration->configurationKey[i]->readonly));

            if (GetConfiguration->configurationKey[i]->valueIsUse)
            {
                json_object_object_add(config_key_object, "value", json_object_new_string(GetConfiguration->configurationKey[i]->value));
            }

            json_object_array_add(config_keys_array, config_key_object);
        }

        json_object_object_add(payload_object, "configurationKey", config_keys_array);
    }

    if (GetConfiguration->unknownKeyIsUse)
    {
        struct json_object *unknown_keys_array = json_object_new_array();

        int i = 0;
        for (i = 0; i < GetConfiguration->numberOfUnknownKey; i++)
        {
            json_object_array_add(unknown_keys_array, json_object_new_string(GetConfiguration->unknownKey[i]));
        }

        json_object_object_add(payload_object, "unknownKey", unknown_keys_array);
    }

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);
    char *message = strdup(json_string);

    json_object_put(root_object);

    return message;
}

/**
 * @description: 创建 GetDiagnostics.conf 消息
 * @param:
 * @return:
 */
char *ocpp_package_prepare_GetDiagnostics_Response(const char *UniqueId, ocpp_package_GetDiagnostics_conf_t *GetDiagnostics)
{
    if (UniqueId == NULL || GetDiagnostics == NULL)
    {
        return NULL;
    }
    struct json_object *root_object = json_object_new_array();
    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_object, json_object_new_string(UniqueId));

    struct json_object *payload_object = json_object_new_object();
    if (GetDiagnostics->fileNameIsUse)
    {
        json_object_object_add(payload_object, "fileName", json_object_new_string(GetDiagnostics->fileName));
    }

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);
    char *message = strdup(json_string);

    json_object_put(root_object);

    return message;
}

/**
 * @description: 创建 RemoteStartTransaction.conf 消息
 * @param:
 * @return:
 */
char *ocpp_package_prepare_RemoteStartTransaction_Response(const char *UniqueId, ocpp_package_RemoteStartTransaction_conf_t *RemoteStartTransaction)
{
    if (UniqueId == NULL || RemoteStartTransaction == NULL)
    {
        return NULL;
    }
    struct json_object *root_object = json_object_new_array();
    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_object, json_object_new_string(UniqueId));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_RemoteStartStopStatus_text[RemoteStartTransaction->status]));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);
    char *message = strdup(json_string);

    json_object_put(root_object);

    return message;
}

/**
 * @description: 创建 RemoteStopTransaction.conf 消息
 * @param:
 * @return:
 */
char *ocpp_package_prepare_RemoteStopTransaction_Response(const char *UniqueId, ocpp_package_RemoteStopTransaction_conf_t *RemoteStopTransaction)
{
    if (UniqueId == NULL || RemoteStopTransaction == NULL)
    {
        return NULL;
    }
    struct json_object *root_object = json_object_new_array();
    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_object, json_object_new_string(UniqueId));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_RemoteStartStopStatus_text[RemoteStopTransaction->status]));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);
    char *message = strdup(json_string);

    json_object_put(root_object);

    return message;
}

/**
 * @description: 创建 ReserveNow.conf 消息
 * @param:
 * @return:
 */
char *ocpp_package_prepare_ReserveNow_Response(const char *UniqueId, ocpp_package_ReserveNow_conf_t *ReserveNow)
{
    if (UniqueId == NULL || ReserveNow == NULL)
    {
        return NULL;
    }
    struct json_object *root_object = json_object_new_array();
    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_object, json_object_new_string(UniqueId));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_ReservationStatus_text[ReserveNow->status]));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);
    char *message = strdup(json_string);

    json_object_put(root_object);

    return message;
}

/**
 * @description: 创建 Reset.conf消息
 * @param:
 * @return:
 */
char *ocpp_package_prepare_Reset_Response(const char *UniqueId, ocpp_package_Reset_conf_t *Reset)
{
    if (UniqueId == NULL || Reset == NULL)
    {
        return NULL;
    }
    struct json_object *root_object = json_object_new_array();
    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_object, json_object_new_string(UniqueId));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_ResetStatus_text[Reset->status]));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);
    char *message = strdup(json_string);

    json_object_put(root_object);

    return message;
}

/**
 * @description: 创建SendLocalList.conf消息
 * @param:
 * @return:
 */
char *ocpp_package_prepare_SendLocalList_Response(const char *UniqueId, ocpp_package_SendLocalList_conf_t *SendLocalList)
{
    if (UniqueId == NULL || SendLocalList == NULL)
    {
        return NULL;
    }
    struct json_object *root_object = json_object_new_array();
    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_object, json_object_new_string(UniqueId));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_UpdateStatus_text[SendLocalList->status]));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);
    char *message = strdup(json_string);

    json_object_put(root_object);

    return message;
}

/**
 * @description: 创建SetChargingProfile.conf消息
 * @param:
 * @return:
 */
char *ocpp_package_prepare_SetChargingProfile_Response(const char *UniqueId, ocpp_package_SetChargingProfile_conf_t *SetChargingProfile)
{
    if (UniqueId == NULL || SetChargingProfile == NULL)
    {
        return NULL;
    }
    struct json_object *root_object = json_object_new_array();
    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_object, json_object_new_string(UniqueId));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_ChargingProfileStatus_text[SetChargingProfile->status]));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);
    char *message = strdup(json_string);

    json_object_put(root_object);

    return message;
}

/**
 * @description: 创建TriggerMessage.conf消息
 * @param:
 * @return:
 */
char *ocpp_package_prepare_TriggerMessage_Response(const char *UniqueId, ocpp_package_TriggerMessage_conf_t *TriggerMessage)
{
    if (UniqueId == NULL || TriggerMessage == NULL)
    {
        return NULL;
    }
    struct json_object *root_object = json_object_new_array();
    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_object, json_object_new_string(UniqueId));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_TriggerMessageStatus_text[TriggerMessage->status]));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);
    char *message = strdup(json_string);

    json_object_put(root_object);

    return message;
}

/**
 * @description: 创建UnlockConnector.conf消息
 * @param:
 * @return:
 */
char *ocpp_package_prepare_UnlockConnector_Response(const char *UniqueId, ocpp_package_UnlockConnector_conf_t *UnlockConnector)
{
    if (UniqueId == NULL || UnlockConnector == NULL)
    {
        return NULL;
    }
    struct json_object *root_object = json_object_new_array();
    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_object, json_object_new_string(UniqueId));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_UnlockStatus_text[UnlockConnector->status]));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);
    char *message = strdup(json_string);

    json_object_put(root_object);

    return message;
}

/**
 * @description: 创建UpdateFirmware.conf消息
 * @param:
 * @return:
 */
char *ocpp_package_prepare_UpdateFirmware_Response(const char *UniqueId, ocpp_package_UpdateFirmware_conf_t *UpdateFirmware)
{
    if (UniqueId == NULL || UpdateFirmware == NULL)
    {
        return NULL;
    }
    struct json_object *root_array = json_object_new_array();
    json_object_array_add(root_array, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_array, json_object_new_string(UniqueId));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string("Accepted"));

    json_object_array_add(root_array, payload_object);

    const char *json_string = json_object_to_json_string(root_array);
    char *message = strdup(json_string);

    json_object_put(root_array);

    return message;
}

/**
 * @description:
 * @param:
 * @return:
 */
char *ocpp_package_prepare_CallError(const char *UniqueId, ocpp_package_CallError_t *callError)
{
    if (UniqueId == NULL || callError == NULL)
    {
        return NULL;
    }
    struct json_object *root_array = json_object_new_array();
    json_object_array_add(root_array, json_object_new_int(OCPP_PACKAGE_CALL_ERROR));
    json_object_array_add(root_array, json_object_new_string(UniqueId));

    struct json_object *error_object = json_object_new_object();
    json_object_object_add(error_object, "errorCode", json_object_new_string(ocpp_package_CallError_ErrorCode_text[callError->errorCode]));

    if (callError->errorDescriptionIsUse)
        json_object_object_add(error_object, "errorDescription", json_object_new_string(callError->errorDescription));
    else
        json_object_object_add(error_object, "errorDescription", json_object_new_string(""));

    struct json_object *detail_object = json_object_new_object();

    if (callError->errorDetailCnt > 0)
    {
        int i = 0;
        for (i = 0; i < callError->errorDetailCnt; i++)
        {
            char key[20];
            snprintf(key, sizeof(key), "errorDetail%d", i + 1);
            json_object_object_add(detail_object, key, json_object_new_string(callError->detail));
        }
    }

    json_object_object_add(error_object, "errorDetails", detail_object);
    json_object_array_add(root_array, error_object);

    const char *json_string = json_object_to_json_string(root_array);
    char *message = strdup(json_string);

    json_object_put(root_array);

    return message;
}
