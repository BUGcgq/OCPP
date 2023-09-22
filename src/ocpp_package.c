#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "ocpp_package.h"
#include <json-c/json.h>
#include "ocpp_order.h"
#include "ocpp_chargePoint.h"
extern ocpp_chargePoint_t *ocpp_chargePoint;
/**
 * @description: 创建Authorize.req消息
 * @param:
 * @return:
 */
int ocpp_chargePoint_sendAuthorize_req(const char *const idTag, char *lastUniqueId)
{
    char *unique = ocpp_AuxiliaryTool_GenerateUUID();
    strncpy(lastUniqueId, unique, 37);
    struct json_object *root_object = json_object_new_array();

    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_MESSAGE));
    json_object_array_add(root_object, json_object_new_string(unique));
    json_object_array_add(root_object, json_object_new_string("Authorize"));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "idTag", json_object_new_string(idTag));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);

    enqueueSendMessage(unique, json_string, OCPP_PACKAGE_AUTHORIZE);
    if (unique)
    {
        free(unique);
    }
    if (root_object)
    {
        json_object_put(root_object);
    }

    return 0;
}

/**
 * @description:
 * @param:
 * @return:
 */
int ocpp_chargePoint_sendBootNotification_req()
{
    char *uniqueId = ocpp_AuxiliaryTool_GenerateUUID();
    struct json_object *root_object = json_object_new_array();

    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_MESSAGE));
    json_object_array_add(root_object, json_object_new_string(uniqueId));
    json_object_array_add(root_object, json_object_new_string("BootNotification"));

    struct json_object *payload_object = json_object_new_object();

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

    json_object_object_add(payload_object, "chargePointModel", json_object_new_string(bootNotification.chargePointModel));
    if (bootNotification.chargeBoxSerialNumberIsUse)
    {
        json_object_object_add(payload_object, "chargeBoxSerialNumber", json_object_new_string(bootNotification.chargeBoxSerialNumber));
    }

    if (bootNotification.chargePointSerialNumberIsUse)
    {
        json_object_object_add(payload_object, "chargePointSerialNumber", json_object_new_string(bootNotification.chargePointSerialNumber));
    }

    json_object_object_add(payload_object, "chargePointVendor", json_object_new_string(bootNotification.chargePointVendor));

    if (bootNotification.firmwareVersionIsUse)
    {
        json_object_object_add(payload_object, "firmwareVersion", json_object_new_string(bootNotification.firmwareVersion));
    }

    if (bootNotification.iccidIsUse)
    {
        json_object_object_add(payload_object, "iccid", json_object_new_string(bootNotification.iccid));
    }

    if (bootNotification.imsiIsUse)
    {
        json_object_object_add(payload_object, "imsi", json_object_new_string(bootNotification.imsi));
    }

    if (bootNotification.meterSerialNumberIsUse)
    {
        json_object_object_add(payload_object, "meterSerialNumber", json_object_new_string(bootNotification.meterSerialNumber));
    }

    if (bootNotification.meterTypeIsUse)
    {
        json_object_object_add(payload_object, "meterType", json_object_new_string(bootNotification.meterType));
    }

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);

    enqueueSendMessage(uniqueId, json_string, OCPP_PACKAGE_BOOT_NOTIFICATION);

    if (root_object)
    {
        json_object_put(root_object);
    }
    if (uniqueId)
    {
        free(uniqueId);
    }

    return 0;
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
int ocpp_chargePoint_sendDiagnosticsStatusNotification_Req()
{

    char *uniqueId = ocpp_AuxiliaryTool_GenerateUUID();
    struct json_object *root_object = json_object_new_array();

    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_MESSAGE));
    json_object_array_add(root_object, json_object_new_string(uniqueId));
    json_object_array_add(root_object, json_object_new_string("DiagnosticsStatusNotification"));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_DiagnosticsStatus_text[ocpp_chargePoint->ocpp_diagnostics_lastDiagnosticsStatus]));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);

    json_object_put(root_object);

    enqueueSendMessage(uniqueId, json_string, OCPP_PACKAGE_DIAGNOSTICS_STATUS_NOTIFICATION);
    if (root_object)
    {
        json_object_put(root_object);
    }
    if (uniqueId)
    {
        free(uniqueId);
    }
    return NULL;
}

/**
 * @description: 创建FirmwareStatusNotification.req消息
 * @param:
 * @return:
 */
int ocpp_chargePoint_sendFirmwareStatusNotification_Req()
{

    char *uniqueId = ocpp_AuxiliaryTool_GenerateUUID();
    struct json_object *root_object = json_object_new_array();

    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_MESSAGE)); // OCPP_PACKAGE_CALL_MESSAGE
    json_object_array_add(root_object, json_object_new_string(uniqueId));
    json_object_array_add(root_object, json_object_new_string("FirmwareStatusNotification"));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_FirmwareStatus[ocpp_chargePoint->ocpp_firmwareUpdate_lastUpdateState]));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);

    json_object_put(root_object);
    enqueueSendMessage(uniqueId, json_string, OCPP_PACKAGE_FIRMWARE_STATUS_NOTIFICATION);
    if (root_object)
    {
        json_object_put(root_object);
    }
    if (uniqueId)
    {
        free(uniqueId);
    }

    return 0;
}

/**
 * @description:
 * @param:
 * @return:
 */
int ocpp_chargePoint_sendHeartbeat_Req()
{
    char *uniqueId = ocpp_AuxiliaryTool_GenerateUUID();
    struct json_object *root_object = json_object_new_array();

    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_MESSAGE));
    json_object_array_add(root_object, json_object_new_string(uniqueId));
    json_object_array_add(root_object, json_object_new_string("Heartbeat"));

    // 添加空对象{}，即使没有附带额外数据
    struct json_object *empty_object = json_object_new_object();
    json_object_array_add(root_object, empty_object);
    const char *json_string = json_object_to_json_string(root_object);
    enqueueSendMessage(uniqueId, json_string, OCPP_PACKAGE_HEARTBEAT);

    if (uniqueId)
    {
        free(uniqueId);
    }
    if (root_object)
    {
        json_object_put(root_object);
    }

    return 0;
}

/**
 * @description:
 * @param {ocpp_chargePoint_t} *ocpp_chargePoint
 * @param {int} connector
 * @param {int} transactionId
 * @return {*}
 */
int ocpp_chargePoint_sendMeterValues(int connector, int transactionId)
{

    char MeterValuesSampledData[512];
    int MeterValuesSampledDataCnt = 0;
    ocpp_ConfigurationKey_getValue("MeterValuesSampledData", (void *)MeterValuesSampledData);
    int i = 0;
    if (MeterValuesSampledData != NULL)
    {
        char *uniqueId = ocpp_AuxiliaryTool_GenerateUUID();
        MeterValuesSampledDataCnt = ocpp_AuxiliaryTool_charCounter(MeterValuesSampledData, ':') + 1;
        ocpp_package_MeterValues_M_SampledValue sampledValue[MeterValuesSampledDataCnt];
        char *token = strtok(MeterValuesSampledData, ":");
        char *timestamp = ocpp_AuxiliaryTool_GetCurrentTime();
        struct json_object *root_object = json_object_new_array();

        json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_MESSAGE));
        json_object_array_add(root_object, json_object_new_string(uniqueId));
        json_object_array_add(root_object, json_object_new_string("MeterValues"));

        struct json_object *payload_object = json_object_new_object();
        json_object_object_add(payload_object, "connectorId", json_object_new_int(connector));

        json_object_object_add(payload_object, "transactionId", json_object_new_int(transactionId));

        struct json_object *meterValue_array = json_object_new_array();
        struct json_object *meterValue_object = json_object_new_object();

        json_object_object_add(meterValue_object, "timestamp", json_object_new_string(timestamp));

        struct json_object *sampledValue_array = json_object_new_array();
        while (token != NULL)
        {
            sampledValue[i].contextIsUse = 1;
            sampledValue[i].formatIsUse = 1;
            sampledValue[i].locationIsUse = 1;
            sampledValue[i].measurandIsUse = 1;
            sampledValue[i].phaseIsUse = 0;
            sampledValue[i].unitIsUse = 1;
            if (strcmp(token, "Voltage") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_V]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_V]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getVoltage(connector));
            }
            else if (strcmp(token, "Temperature") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_T]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_CELSIUS]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getTemperature(connector));
            }
            else if (strcmp(token, "SoC") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_S]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_PERCENT]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getSOC(connector));
            }
            else if (strcmp(token, "RPM") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_R]);
                sampledValue[i].unitIsUse = 0;
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getRPM(connector));
            }
            else if (strcmp(token, "Power.Reactive.Import") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PRI]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_KVAR]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getPowerActiveImport(connector));
            }
            else if (strcmp(token, "Power.Reactive.Export") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PRE]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_KVAR]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getPowerActiveExport(connector));
            }
            else if (strcmp(token, "Power.Offered") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PO]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_V]);
                snprintf(sampledValue[i].value, 16, "%s", ocpp_chargePoint->getPowerOffered(connector));
            }
            else if (strcmp(token, "Power.Factor") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PAI]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PO]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getPowerOffered(connector));
            }
            else if (strcmp(token, "Power.Active.Import") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PAI]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_V]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getPowerFactor(connector));
            }
            else if (strcmp(token, "Power.Active.Export") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PAE]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_V]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getPowerActiveExport(connector));
            }

            else if (strcmp(token, "Frequency") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_F]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_V]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getFrequency(connector));
            }
            else if (strcmp(token, "Energy.Reactive.Export.Interval") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_EREI]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_V]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getEnergyReactiveExportInterval(connector));
            }
            else if (strcmp(token, "Energy.Reactive.Import.Interval") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_ERII]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_V]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getEnergyReactiveImportInterval(connector));
            }
            else if (strcmp(token, "Energy.Active.Import.Interval") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_EAII]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_V]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getEnergyActiveImportInterval(connector));
            }
            else if (strcmp(token, "Energy.Active.Export.Interval") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_EAEI]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_V]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getEnergyActiveExportInterval(connector));
            }
            else if (strcmp(token, "Energy.Reactive.Import.Register") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_ERIR]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_WH]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getEnergyReactiveImportRegister(connector));
            }
            else if (strcmp(token, "Energy.Reactive.Export.Register") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_ERER]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_V]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getEnergyReactiveExportRegister(connector));
            }
            else if (strcmp(token, "Energy.Active.Import.Register") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_EAIR]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_WH]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getEnergyActiveImportRegister(connector));
            }
            else if (strcmp(token, "Energy.Active.Export.Register") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_EAER]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_V]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getEnergyActiveExportRegister(connector));
            }
            else if (strcmp(token, "Current.Offered") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_CO]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_A]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getCurrentOffered(connector));
            }
            else if (strcmp(token, "Current.Import") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_CO]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_A]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getCurrentImport(connector));
            }
            else if (strcmp(token, "Current.Export") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_CE]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_V]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getCurrentExport(connector));
            }
            struct json_object *sampledValue_object = json_object_new_object();
            json_object_object_add(sampledValue_object, "value", json_object_new_string(sampledValue[i].value));
            if (sampledValue[i].contextIsUse)
                json_object_object_add(sampledValue_object, "context", json_object_new_string(sampledValue[i].context));

            if (sampledValue[i].formatIsUse)
                json_object_object_add(sampledValue_object, "format", json_object_new_string(sampledValue[i].format));

            if (sampledValue[i].measurandIsUse)
                json_object_object_add(sampledValue_object, "measurand", json_object_new_string(sampledValue[i].measurand));

            if (sampledValue[i].phaseIsUse)
                json_object_object_add(sampledValue_object, "phase", json_object_new_string(sampledValue[i].phase));

            if (sampledValue[i].locationIsUse)
                json_object_object_add(sampledValue_object, "location", json_object_new_string(sampledValue[i].location));

            if (sampledValue[i].unitIsUse)
                json_object_object_add(sampledValue_object, "unit", json_object_new_string(sampledValue[i].unit));

            json_object_array_add(sampledValue_array, sampledValue_object);
            token = strtok(NULL, ":");
            i++;
        }
        json_object_object_add(meterValue_object, "sampledValue", sampledValue_array);
        json_object_array_add(meterValue_array, meterValue_object);
        json_object_object_add(payload_object, "meterValue", meterValue_array);
        json_object_array_add(root_object, payload_object);
        const char *json_string = json_object_to_json_string(root_object);

        enqueueSendMessage(uniqueId, json_string, OCPP_PACKAGE_METERVALUES);
        if (uniqueId)
        {
            free(uniqueId);
        }
        if (timestamp)
        {
            free(timestamp);
        }
        if (root_object)
        {
            json_object_put(root_object);
        }
    }

    return 0;
}

/**
 * @description:
 * @param:
 * @return:
 */
int ocpp_chargePoint_sendStartTransaction(int connector, const char *idTag, int reservationId, char *lastUniqueId)
{
    char *uniqueId = ocpp_AuxiliaryTool_GenerateUUID();
    if (uniqueId != NULL)
    {
        strncpy(lastUniqueId, uniqueId, 40);
    }

    char *timestamp = ocpp_AuxiliaryTool_GetCurrentTime();
    int metervalue = (int)ocpp_chargePoint->getCurrentMeterValues(connector);
    struct json_object *root_object = json_object_new_array();

    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_MESSAGE));
    json_object_array_add(root_object, json_object_new_string(uniqueId));
    json_object_array_add(root_object, json_object_new_string("StartTransaction"));
    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "connectorId", json_object_new_int(connector));
    json_object_object_add(payload_object, "idTag", json_object_new_string(idTag));
    json_object_object_add(payload_object, "meterStart", json_object_new_int((metervalue)));
    if (reservationId > 0)
    {
        json_object_object_add(payload_object, "reservationId", json_object_new_int(reservationId));
    }
    json_object_object_add(payload_object, "timestamp", json_object_new_string(timestamp));
    json_object_array_add(root_object, payload_object);
    const char *json_string = json_object_to_json_string(root_object);
    enqueueSendMessage(uniqueId, json_string, OCPP_PACKAGE_STARTTRANSACTION);
    if (uniqueId)
    {
        free(uniqueId);
    }
    if (timestamp)
    {
        free(timestamp);
    }
    if (root_object)
    {
        free(root_object);
    }

    return 0;
}

/**
 * @description: 创建StatusNotification.req消息
 * @param:
 * @return:
 */
int ocpp_chargePoint_sendStatusNotification_Req(int connector)
{

    char *uniqueId = ocpp_AuxiliaryTool_GenerateUUID();
    struct json_object *root_object = json_object_new_array();

    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_MESSAGE));
    json_object_array_add(root_object, json_object_new_string(uniqueId));
    json_object_array_add(root_object, json_object_new_string("StatusNotification"));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "connectorId", json_object_new_int(connector));
    json_object_object_add(payload_object, "errorCode", json_object_new_string(ocpp_package_ChargePointErrorCode_text[ocpp_chargePoint->connector[connector]->errorCode]));
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_ChargePointStatus_text[ocpp_chargePoint->connector[connector]->status]));

    if (ocpp_chargePoint->connector[connector]->timestampIsUse)
        json_object_object_add(payload_object, "timestamp", json_object_new_string(ocpp_chargePoint->connector[connector]->timestamp));

    if (ocpp_chargePoint->connector[connector]->infoIsUse)
        json_object_object_add(payload_object, "info", json_object_new_string(ocpp_chargePoint->connector[connector]->info));

    if (ocpp_chargePoint->connector[connector]->vendorIdIsUse)
        json_object_object_add(payload_object, "vendorId", json_object_new_string(ocpp_chargePoint->connector[connector]->vendorId));

    if (ocpp_chargePoint->connector[connector]->vendorErrorCodeIsUse)
        json_object_object_add(payload_object, "vendorErrorCode", json_object_new_string(ocpp_chargePoint->connector[connector]->vendorErrorCode));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);

    enqueueSendMessage(uniqueId, json_string, OCPP_PACKAGE_STATUS_NOTIFICATION);

    if (root_object)
    {
        json_object_put(root_object);
    }
    if (uniqueId)
    {
        free(uniqueId);
    }
    return 0;
}
int ocpp_chargePoint_sendFinishing_Req(int connector)
{
    char *uniqueId = ocpp_AuxiliaryTool_GenerateUUID();
    struct json_object *root_object = json_object_new_array();

    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_MESSAGE));
    json_object_array_add(root_object, json_object_new_string(uniqueId));
    json_object_array_add(root_object, json_object_new_string("StatusNotification"));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "connectorId", json_object_new_int(connector));
    json_object_object_add(payload_object, "errorCode", json_object_new_string(ocpp_package_ChargePointErrorCode_text[ocpp_chargePoint->connector[connector]->errorCode]));
    json_object_object_add(payload_object, "status", json_object_new_string("Finishing"));

    if (ocpp_chargePoint->connector[connector]->timestampIsUse)
        json_object_object_add(payload_object, "timestamp", json_object_new_string(ocpp_chargePoint->connector[connector]->timestamp));

    if (ocpp_chargePoint->connector[connector]->infoIsUse)
        json_object_object_add(payload_object, "info", json_object_new_string(ocpp_chargePoint->connector[connector]->info));

    if (ocpp_chargePoint->connector[connector]->vendorIdIsUse)
        json_object_object_add(payload_object, "vendorId", json_object_new_string(ocpp_chargePoint->connector[connector]->vendorId));

    if (ocpp_chargePoint->connector[connector]->vendorErrorCodeIsUse)
        json_object_object_add(payload_object, "vendorErrorCode", json_object_new_string(ocpp_chargePoint->connector[connector]->vendorErrorCode));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);

    enqueueSendMessage(uniqueId, json_string, OCPP_PACKAGE_STATUS_NOTIFICATION);

    if (root_object)
    {
        json_object_put(root_object);
    }
    if (uniqueId)
    {
        free(uniqueId);
    }
    return 0;
}
/**
 * @description:
 * @param:
 * @return:
 */
int ocpp_transaction_sendStopTransaction_Simpleness(int connector, const char *idTag, int transactionId, const char *lastUniqueId, enum OCPP_PACKAGE_STOP_REASON_E reason)
{

    char *uniqueId = ocpp_AuxiliaryTool_GenerateUUID();
    if (lastUniqueId != NULL)
    {
        strncpy(lastUniqueId, uniqueId, 40);
    }
    const char *transactionDataStr = NULL;
    int meterStop = (int)ocpp_chargePoint->getCurrentMeterValues(connector);
    char *timestamp = ocpp_AuxiliaryTool_GetCurrentTime();
    struct json_object *root_object = json_object_new_array();

    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_MESSAGE));
    json_object_array_add(root_object, json_object_new_string(uniqueId));
    json_object_array_add(root_object, json_object_new_string("StopTransaction"));

    struct json_object *payload_object = json_object_new_object();

    json_object_object_add(payload_object, "transactionId", json_object_new_int(transactionId));
    json_object_object_add(payload_object, "idTag", json_object_new_string(idTag));
    json_object_object_add(payload_object, "timestamp", json_object_new_string(timestamp));
    json_object_object_add(payload_object, "meterStop", json_object_new_int(meterStop));
    json_object_object_add(payload_object, "reason", json_object_new_string(ocpp_package_stop_reason_text[reason]));
    int ret = ocpp_ConfigurationKey_getIsUse("StopTxnSampledData");
    if (ret == 1)
    {
        json_object *transactionData_array = json_object_new_array();
        json_object *transactionData_obj = json_object_new_object();
        json_object_object_add(transactionData_obj, "timestamp", json_object_new_string(timestamp));
        // 创建嵌套的 JSON 数组
        json_object *values_array = json_object_new_array();

        char StopTxnSampledData[512];
        int MeterValuesSampledDataCnt = 0;
        int i = 0;
        ocpp_ConfigurationKey_getValue("StopTxnSampledData", (void *)StopTxnSampledData);
        MeterValuesSampledDataCnt = ocpp_AuxiliaryTool_charCounter(StopTxnSampledData, ':') + 1;
        ocpp_package_MeterValues_M_SampledValue sampledValue[MeterValuesSampledDataCnt];
        char *token = strtok(StopTxnSampledData, ":");
        while (token != NULL)
        {
            struct json_object *sampledValue_object = json_object_new_object();
            sampledValue[i].contextIsUse = 1;
            sampledValue[i].formatIsUse = 1;
            sampledValue[i].locationIsUse = 1;
            sampledValue[i].measurandIsUse = 1;
            sampledValue[i].phaseIsUse = 0;
            sampledValue[i].unitIsUse = 1;
            if (strcmp(token, "Voltage") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_V]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_V]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getVoltage(connector));
            }
            else if (strcmp(token, "Temperature") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_T]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_CELSIUS]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getTemperature(connector));
            }
            else if (strcmp(token, "SoC") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_S]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_PERCENT]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getSOC(connector));
            }
            else if (strcmp(token, "RPM") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_R]);
                sampledValue[i].unitIsUse = 0;
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getRPM(connector));
            }
            else if (strcmp(token, "Power.Reactive.Import") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PRI]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_KVAR]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getPowerActiveImport(connector));
            }
            else if (strcmp(token, "Power.Reactive.Export") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PRE]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_KVAR]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getPowerActiveExport(connector));
            }
            else if (strcmp(token, "Power.Offered") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PO]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_V]);
                snprintf(sampledValue[i].value, 16, "%s", ocpp_chargePoint->getPowerOffered(connector));
            }
            else if (strcmp(token, "Power.Factor") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PAI]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PO]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getPowerOffered(connector));
            }
            else if (strcmp(token, "Power.Active.Import") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PAI]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_V]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getPowerFactor(connector));
            }
            else if (strcmp(token, "Power.Active.Export") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PAE]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_V]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getPowerActiveExport(connector));
            }

            else if (strcmp(token, "Frequency") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_F]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_V]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getFrequency(connector));
            }
            else if (strcmp(token, "Energy.Reactive.Export.Interval") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_EREI]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_V]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getEnergyReactiveExportInterval(connector));
            }
            else if (strcmp(token, "Energy.Reactive.Import.Interval") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_ERII]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_V]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getEnergyReactiveImportInterval(connector));
            }
            else if (strcmp(token, "Energy.Active.Import.Interval") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_EAII]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_V]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getEnergyActiveImportInterval(connector));
            }
            else if (strcmp(token, "Energy.Active.Export.Interval") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_EAEI]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_V]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getEnergyActiveExportInterval(connector));
            }
            else if (strcmp(token, "Energy.Reactive.Import.Register") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_ERIR]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_WH]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getEnergyReactiveImportRegister(connector));
            }
            else if (strcmp(token, "Energy.Reactive.Export.Register") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_ERER]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_V]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getEnergyReactiveExportRegister(connector));
            }
            else if (strcmp(token, "Energy.Active.Import.Register") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_EAIR]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_WH]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getEnergyActiveImportRegister(connector));
            }
            else if (strcmp(token, "Energy.Active.Export.Register") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_EAER]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_V]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getEnergyActiveExportRegister(connector));
            }
            else if (strcmp(token, "Current.Offered") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_CO]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_A]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getCurrentOffered(connector));
            }
            else if (strcmp(token, "Current.Import") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_CO]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_A]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getCurrentImport(connector));
            }
            else if (strcmp(token, "Current.Export") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_CE]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_V]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getCurrentExport(connector));
            }
            json_object_object_add(sampledValue_object, "value", json_object_new_string(sampledValue[i].value));
            if (sampledValue[i].contextIsUse)
                json_object_object_add(sampledValue_object, "context", json_object_new_string(sampledValue[i].context));
            if (sampledValue[i].formatIsUse)
                json_object_object_add(sampledValue_object, "format", json_object_new_string(sampledValue[i].format));
            if (sampledValue[i].measurandIsUse)
                json_object_object_add(sampledValue_object, "measurand", json_object_new_string(sampledValue[i].measurand));
            if (sampledValue[i].phaseIsUse)
                json_object_object_add(sampledValue_object, "phase", json_object_new_string(sampledValue[i].phase));
            if (sampledValue[i].locationIsUse)
                json_object_object_add(sampledValue_object, "location", json_object_new_string(sampledValue[i].location));
            if (sampledValue[i].unitIsUse)
                json_object_object_add(sampledValue_object, "unit", json_object_new_string(sampledValue[i].unit));

            json_object_array_add(values_array, sampledValue_object);
            token = strtok(NULL, ":");
            i++;
        }
        json_object_object_add(transactionData_obj, "sampledValue", values_array);
        transactionDataStr = json_object_to_json_string(transactionData_obj);
        json_object_array_add(transactionData_array, transactionData_obj);
        json_object_object_add(payload_object, "transactionData", transactionData_array);
    }

    json_object_array_add(root_object, payload_object);
    const char *json_string = json_object_to_json_string(root_object);
    enqueueSendMessage(uniqueId, json_string, OCPP_PACKAGE_STOPTRANSACTION);
    ocpp_StopTransaction_insert(transactionId, idTag, timestamp,
                                meterStop, ocpp_package_stop_reason_text[reason], transactionDataStr, uniqueId);
    if (root_object)
    {
        json_object_put(root_object);
    }
    if (uniqueId)
    {
        free(uniqueId);
    }
    if (timestamp)
    {
        free(timestamp);
    }
    return 0;
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
 * @description:
 * @param {char *} uniqueId
 * @param {ocpp_package_GetLocalListVersion_req_t} GetLocalListVersion
 * @return {*}
 */
void ocpp_chargePoint_GetLocalListVersion_Req(const char *uniqueId)
{
    struct json_object *root_object = json_object_new_array();
    if (root_object == NULL)
    {
        return;
    }
    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_object, json_object_new_string(uniqueId));

    struct json_object *payload_object = json_object_new_object();

    json_object_object_add(payload_object, "listVersion", json_object_new_int(ocpp_localAuthorization_List_getListVersion("Version")));
    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);

    enqueueSendMessage(uniqueId, json_string, OCPP_PACKAGE_CALL);

    json_object_put(root_object);
}
/**
 * @description: 创建 GetCompositeSchedule.conf 消息
 * @param:
 * @return:
 */
char *ocpp_package_prepare_GetCompositeSchedule_Response(const char *UniqueId, ocpp_package_GetCompositeSchedule_conf_t *GetCompositeSchedule)
{
    // if (UniqueId == NULL || GetCompositeSchedule == NULL)
    // {
    //     return NULL;
    // }
    // struct json_object *root_object = json_object_new_array();
    // json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    // json_object_array_add(root_object, json_object_new_string(UniqueId));

    // struct json_object *payload_object = json_object_new_object();
    // json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_GetCompositeScheduleStatus_text[GetCompositeSchedule->status]));

    // if (GetCompositeSchedule->connectorIdIsUse)
    // {
    //     json_object_object_add(payload_object, "connectorId", json_object_new_int(GetCompositeSchedule->connectorId));
    // }

    // if (GetCompositeSchedule->scheduleStartIsUse)
    // {
    //     json_object_object_add(payload_object, "scheduleStart", json_object_new_string(GetCompositeSchedule->scheduleStart));
    // }

    // if (GetCompositeSchedule->chargingScheduleIsUse)
    // {
    //     struct json_object *chargingSchedule_object = json_object_new_object();
    //     json_object_object_add(chargingSchedule_object, "chargingRateUnit", json_object_new_string(ocpp_package_ChargingRateUnitType_text[GetCompositeSchedule->chargingSchedule.chargingRateUnit]));

    //     if (GetCompositeSchedule->chargingSchedule.durationIsUse)
    //     {
    //         json_object_object_add(chargingSchedule_object, "duration", json_object_new_int(GetCompositeSchedule->chargingSchedule.duration));
    //     }

    //     if (GetCompositeSchedule->chargingSchedule.startScheduleIsUse)
    //     {
    //         json_object_object_add(chargingSchedule_object, "startSchedule", json_object_new_string(GetCompositeSchedule->chargingSchedule.startSchedule));
    //     }

    //     if (GetCompositeSchedule->chargingSchedule.minChargingRateIsUse)
    //     {
    //         json_object_object_add(chargingSchedule_object, "minChargingRate", json_object_new_double(GetCompositeSchedule->chargingSchedule.minChargingRate));
    //     }

    //     if (GetCompositeSchedule->chargingSchedule.numberOfchargingSchedulePeriod > 0)
    //     {
    //         struct json_object *chargingSchedulePeriod_array = json_object_new_array();
    //         int i;
    //         for (i = 0; i < GetCompositeSchedule->chargingSchedule.numberOfchargingSchedulePeriod; i++)
    //         {
    //             struct json_object *chargingSchedulePeriod_object = json_object_new_object();
    //             json_object_object_add(chargingSchedulePeriod_object, "startPeriod", json_object_new_int(GetCompositeSchedule->chargingSchedule.chargingSchedulePeriod[i]->startPeriod));
    //             json_object_object_add(chargingSchedulePeriod_object, "limit", json_object_new_double(GetCompositeSchedule->chargingSchedule.chargingSchedulePeriod[i]->limit));

    //             if (GetCompositeSchedule->chargingSchedule.chargingSchedulePeriod[i]->numberPhasesIsUse)
    //             {
    //                 json_object_object_add(chargingSchedulePeriod_object, "numberPhases", json_object_new_int(GetCompositeSchedule->chargingSchedule.chargingSchedulePeriod[i]->numberPhases));
    //             }

    //             json_object_array_add(chargingSchedulePeriod_array, chargingSchedulePeriod_object);
    //         }

    //         json_object_object_add(chargingSchedule_object, "chargingSchedulePeriod", chargingSchedulePeriod_array);
    //     }

    //     json_object_object_add(payload_object, "chargingSchedule", chargingSchedule_object);
    // }

    // json_object_array_add(root_object, payload_object);

    // const char *json_string = json_object_to_json_string(root_object);
    // char *message = strdup(json_string);

    // json_object_put(root_object);

    // return message;
}

/**
 * @description: 创建 GetConfiguration.conf 消息
 * @param:
 * @return:
 */
void ocpp_package_prepare_GetConfiguration_Response(const char *UniqueId, ocpp_package_GetConfiguration_conf_t *getConfiguration_conf)
{
    if (UniqueId == NULL || getConfiguration_conf == NULL)
    {
        return;
    }

    // 创建一个 JSON 对象用于存储 GetConfiguration.conf 响应
    struct json_object *response_obj = json_object_new_array();
    json_object_array_add(response_obj, json_object_new_int(OCPP_PACKAGE_CALL_RESULT)); // 请求序列号
    json_object_array_add(response_obj, json_object_new_string(UniqueId));              // 唯一标识符
    json_object_array_add(response_obj, json_object_new_string("GetConfiguration"));    // 操作类型

    // 创建一个 JSON 对象用于存储配置信息
    struct json_object *config_obj = json_object_new_object();

    // 创建 "configurationKey" 数组和 "unknownKey" 数组
    struct json_object *configuration_key_array = json_object_new_array();
    struct json_object *unknown_key_array = json_object_new_array();
    // 遍历 GetConfiguration.conf 结构体数组
    for (int i = 0; i < getConfiguration_conf->numberOfKey; i++)
    {
        // 根据类型将配置项添加到相应的数组中
        if (getConfiguration_conf[i].type == -1)
        {
            json_object_array_add(unknown_key_array, json_object_new_string(getConfiguration_conf[i].key));
        }
        else
        {
            struct json_object *config_key_obj = json_object_new_object();
            json_object_object_add(config_key_obj, "key", json_object_new_string(getConfiguration_conf[i].key));
            json_object_object_add(config_key_obj, "readonly", json_object_new_boolean(getConfiguration_conf[i].accessibility));
            json_object_object_add(config_key_obj, "value", json_object_new_string(getConfiguration_conf[i].value));
            json_object_array_add(configuration_key_array, config_key_obj);
        }
    }

    // 添加 "configurationKey" 数组和 "unknownKey" 数组到配置信息对象
    json_object_object_add(config_obj, "configurationKey", configuration_key_array);
    json_object_object_add(config_obj, "unknownKey", unknown_key_array);

    // 将配置信息添加到响应对象中
    json_object_array_add(response_obj, config_obj);

    // 打印或返回 JSON 响应对象
    const char *json_string = json_object_to_json_string(response_obj);

    enqueueSendMessage(UniqueId, json_string, 0);

    // 释放 JSON 对象
    json_object_put(response_obj);
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
 * @description:
 * @param {char *} uniqueId
 * @param {ocpp_package_RemoteStartTransaction_req_t} remoteStartTransaction_req
 * @return {*}
 */
void ocpp_chargePoint_manageRemoteStartTransaction_Req(const char *uniqueId, ocpp_package_RemoteStartTransaction_req_t remoteStartTransaction_req)
{

    ocpp_package_RemoteStartTransaction_conf_t remoteStartTransaction_conf;
    memset(&remoteStartTransaction_conf, 0, sizeof(remoteStartTransaction_conf));
    ocpp_chargePoint_transaction_t *transaction = ocpp_chargePoint->transaction_obj[remoteStartTransaction_req.connectorId];
    remoteStartTransaction_conf.status = OCPP_PACKAGE_REMOTE_STRATSTOP_STATUS_REJECTED;
    if (transaction->isStart == false && transaction->isTransaction == false)
    {
        memset(transaction, 0, sizeof(ocpp_chargePoint_transaction_t));
        strncpy(transaction->startIdTag, remoteStartTransaction_req.idTag, OCPP_AUTHORIZATION_IDTAG_LEN);
        transaction->isStart = true;
        remoteStartTransaction_conf.status = OCPP_PACKAGE_REMOTE_STRATSTOP_STATUS_ACCEPTED;
    }
    // response
    struct json_object *root_object = json_object_new_array();
    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_object, json_object_new_string(uniqueId));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_RemoteStartStopStatus_text[remoteStartTransaction_conf.status]));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);

    enqueueSendMessage(uniqueId, json_string, OCPP_PACKAGE_CALL);
    if (root_object)
    {
        json_object_put(root_object);
    }
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
 * @description:
 * @param {char *} uniqueId
 * @param {ocpp_package_SendLocalList_req_t *} sendLocalList
 * @return {*}
 */
void ocpp_chargePoint_manageSendLocalList_Req(const char *uniqueId, ocpp_package_SendLocalList_req_t *sendLocalList_req)
{
    ocpp_package_SendLocalList_conf_t sendLocalList_conf;
    memset(&sendLocalList_conf, 0, sizeof(sendLocalList_conf));

    sendLocalList_conf.status = OCPP_PCAKGE_UPDATE_STATUS_ACCEPTED;

    int LocalAuthListEnabled = 0;
    int LocalAuthListMaxLength;
    ocpp_ConfigurationKey_getValue("LocalAuthListEnabled", &LocalAuthListEnabled);
    ocpp_ConfigurationKey_getValue("LocalAuthListMaxLength", &LocalAuthListMaxLength);

    printf("LocalAuthListEnabled = %d\n", LocalAuthListEnabled);
    if (LocalAuthListEnabled == 0)
    {
        sendLocalList_conf.status = OCPP_PCAKGE_UPDATE_STATUS_NOTSUPPORTED;
    }
    else if (sendLocalList_req->numberOfAuthorizationData > LocalAuthListMaxLength)
    {
        sendLocalList_conf.status = OCPP_PCAKGE_UPDATE_STATUS_FAILED;
    }
    else if (sendLocalList_req->listVersion <= ocpp_localAuthorization_List_getListVersion("Version"))
    {
        sendLocalList_conf.status = OCPP_PCAKGE_UPDATE_STATUS_VERSION_MISMATCH;
    }

    else
    {
        ocpp_localAuthorization_Version_setVersion("Version", sendLocalList_req->listVersion);

        if (sendLocalList_req->UpdateType == OCPP_PACKAGE_UPDATE_TYPE_FULL)
        {
            ocpp_localAuthorization_List_clearList();
        }

        // 更新本地列表
        if (sendLocalList_req->numberOfAuthorizationData > 0 && sendLocalList_req->localAuthorizationListIsUse)
        {
            ocpp_localAuthorization_list_entry_t entry;
            memset(&entry, 0, sizeof(ocpp_localAuthorization_list_entry_t));

            for (int i = 0; i < sendLocalList_req->numberOfAuthorizationData; i++)
            {
                strncpy(entry.IdTag, sendLocalList_req->localAuthorizationList[i].idTag, OCPP_AUTHORIZATION_IDTAG_LEN);

                if (sendLocalList_req->localAuthorizationList[i].idTagInfoIsUse)
                {
                    if (sendLocalList_req->localAuthorizationList[i].idTagInfo.parentIdTagIsUse)
                        strncpy(entry.parentIdTag, sendLocalList_req->localAuthorizationList[i].idTagInfo.parentIdTag, OCPP_AUTHORIZATION_IDTAG_LEN);
                    if (sendLocalList_req->localAuthorizationList[i].idTagInfo.expiryDateIsUse)
                    {
                        strncpy(entry.expiryDate, sendLocalList_req->localAuthorizationList[i].idTagInfo.expiryDate, 32);
                    }

                    entry.status = sendLocalList_req->localAuthorizationList[i].idTagInfo.AuthorizationStatus;
                }
                printf("expiryDate = %s\n", sendLocalList_req->localAuthorizationList[i].idTagInfo.expiryDate);
                ocpp_localAuthorization_List_write(&entry);

                if (entry.expiryDate)
                    free(entry.expiryDate); // 释放已分配的内存
            }
        }
    }

    struct json_object *root_object = json_object_new_array();
    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_object, json_object_new_string(uniqueId));
    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_UpdateStatus_text[sendLocalList_conf.status]));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);

    enqueueSendMessage(uniqueId, json_string, OCPP_PACKAGE_CALL);

    if (root_object)
    {
        json_object_put(root_object);
    }
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
    json_object_object_add(payload_object, "status", json_object_new_string("Accepted"));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);
    char *message = strdup(json_string);

    json_object_put(root_object);

    return message;
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {ocpp_package_TriggerMessage_req_t} triggerMessage
 * @return {*}
 */
void ocpp_chargePoint_manageTriggerMessageRequest(const char *uniqueId, ocpp_package_TriggerMessage_req_t triggerMessage_req)
{
    switch (triggerMessage_req.requestedMessage)
    {
    case OCPP_PCAKGE_MESSAGE_TRIGGER_BOOTNOTIFICATION:
       ocpp_package_prepare_ConfigurationStatus_Req(uniqueId, 0);
        ocpp_chargePoint_sendBootNotification_req();
        break;

    case OCPP_PCAKGE_MESSAGE_TRIGGER_DIAGNOSTICSSTATUS_NOTIFICATION:
        // ocpp_chargePoint_sendDiagnosticsStatusNotification_Req();
        ocpp_package_prepare_ConfigurationStatus_Req(uniqueId, 3);
        break;

    case OCPP_PCAKGE_MESSAGE_TRIGGER_FIRMWARESTATUS_NOTIFICATION:
        ocpp_package_prepare_ConfigurationStatus_Req(uniqueId, 3);
        // ocpp_chargePoint_sendFirmwareStatusNotification_Req();
        break;

    case OCPP_PCAKGE_MESSAGE_TRIGGER_HEARTBEAT:
        ocpp_package_prepare_ConfigurationStatus_Req(uniqueId, 0);
        ocpp_chargePoint_sendHeartbeat_Req();
        break;

    case OCPP_PCAKGE_MESSAGE_TRIGGER_METERVALUES:
        ocpp_package_prepare_ConfigurationStatus_Req(uniqueId, 0);
        if (triggerMessage_req.connectorIdIsUse)
            ocpp_chargePoint_sendMeterValues(triggerMessage_req.connectorId, 0);
        else
            ocpp_chargePoint_sendMeterValues(0, 0);
        break;

    case OCPP_PCAKGE_MESSAGE_TRIGGER_STATUS_NOTIFICATION:
        ocpp_package_prepare_ConfigurationStatus_Req(uniqueId, 0);
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
void ocpp_package_prepare_ConfigurationStatus_Req(char *UniqueId, enum OCPP_PACKAGE_CONFIGURATION_STATUS_E Status)
{

    struct json_object *root_array = json_object_new_array();
    json_object_array_add(root_array, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_array, json_object_new_string(UniqueId));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_ConfigurationStatus_text[Status]));

    json_object_array_add(root_array, payload_object);

    const char *json_string = json_object_to_json_string(root_array);
    enqueueSendMessage(UniqueId, json_string, OCPP_PACKAGE_CALL);
    if (root_array)
    {
        json_object_put(root_array);
    }
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
