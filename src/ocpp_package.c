#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ocpp_package.h"
#include <json-c/json.h>
#include "ocpp_chargePoint.h"
#include "ocpp_configurationKey.h"
#include "ocpp_auxiliaryTool.h"
#include "ocpp_list.h"
extern ocpp_chargePoint_t *ocpp_chargePoint;
/**
 * @description: 创建Authorize.req消息
 * @param:
 * @return:
 */
int ocpp_chargePoint_sendAuthorize_req(const char *const idTag, char *lastUniqueId)
{
    struct json_object *root_object = json_object_new_array();
    if (root_object == NULL || lastUniqueId == NULL)
    {
        return -1;
    }
    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_MESSAGE));
    json_object_array_add(root_object, json_object_new_string(lastUniqueId));
    json_object_array_add(root_object, json_object_new_string("Authorize"));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "idTag", json_object_new_string(idTag));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);

    enqueueSendMessage(lastUniqueId, json_string, OCPP_PACKAGE_AUTHORIZE);

    json_object_put(root_object);

    return 0;
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

    char *unique = ocpp_AuxiliaryTool_GenerateUUID();
    result = ocpp_chargePoint_authorizationOfIdentifier(idTag, unique);

    if (unique != NULL)
    {
        strncpy(ocpp_chargePoint->authorizetion[connector]->lastUniqueId, unique, 40);
        free(unique);
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
        printf("授权成功\n");
        ocpp_chargePoint->setAuthorizeResult(connector, true);
        break;
    }
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
    if (root_object == NULL || uniqueId == NULL)
    {
        return -1;
    }
    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_MESSAGE));
    json_object_array_add(root_object, json_object_new_string(uniqueId));
    json_object_array_add(root_object, json_object_new_string("BootNotification"));

    struct json_object *payload_object = json_object_new_object();

    ocpp_package_BootNotification_req_t bootNotification;
    memset(&bootNotification, 0, sizeof(bootNotification));

    ocpp_chargePoint->getChargePointModel(bootNotification.chargePointModel, 20);
    ocpp_chargePoint->getChargePointVendor(bootNotification.chargePointVendor, 20);

    if (ocpp_chargePoint->getChargeBoxSerialNumber(bootNotification.chargeBoxSerialNumber, 25) == true)
    {
        bootNotification.chargeBoxSerialNumberIsUse = 1;
    }

    if (ocpp_chargePoint->getChargePointSerialNumber(bootNotification.chargePointSerialNumber, 25) == true)
    {
        bootNotification.chargePointSerialNumberIsUse = 1;
    }

    if (ocpp_chargePoint->getFirmwareVersion(bootNotification.firmwareVersion, 50) == true)
    {
        bootNotification.firmwareVersionIsUse = 1;
    }

    if (ocpp_chargePoint->getIccid(bootNotification.iccid, 20) == true)
    {
        bootNotification.iccidIsUse = 1;
    }

    if (ocpp_chargePoint->getImsi(bootNotification.imsi, 20) == true)
    {
        bootNotification.imsiIsUse = 1;
    }

    if (ocpp_chargePoint->getMeterSerialNumber(bootNotification.meterSerialNumber, 25) == true)
    {
        bootNotification.meterSerialNumberIsUse = 1;
    }

    if (ocpp_chargePoint->getMeterType(bootNotification.meterType, 25) == true)
    {
        bootNotification.meterTypeIsUse = 1;
    }

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

    json_object_put(root_object);

    free(uniqueId);

    return 0;
}

/**
 * @description: 创建DiagnosticsStatusNotification.req消息
 * @param:
 * @return:
 */
int ocpp_chargePoint_sendDiagnosticsStatusNotification_Req(int status)
{

    char *uniqueId = ocpp_AuxiliaryTool_GenerateUUID();
    struct json_object *root_object = json_object_new_array();
    if (root_object == NULL || uniqueId == NULL)
    {
        return -1;
    }

    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_MESSAGE));
    json_object_array_add(root_object, json_object_new_string(uniqueId));
    json_object_array_add(root_object, json_object_new_string("DiagnosticsStatusNotification"));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_DiagnosticsStatus_text[status]));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);

    enqueueSendMessage(uniqueId, json_string, OCPP_PACKAGE_DIAGNOSTICS_STATUS_NOTIFICATION);

    json_object_put(root_object);

    free(uniqueId);

    return 0;
}

/**
 * @description: 创建FirmwareStatusNotification.req消息
 * @param:
 * @return:
 */
int ocpp_chargePoint_sendFirmwareStatusNotification_Req(int status)
{

    char *uniqueId = ocpp_AuxiliaryTool_GenerateUUID();
    struct json_object *root_object = json_object_new_array();
    if (root_object == NULL || uniqueId == NULL)
    {
        return -1;
    }

    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_MESSAGE)); // OCPP_PACKAGE_CALL_MESSAGE
    json_object_array_add(root_object, json_object_new_string(uniqueId));
    json_object_array_add(root_object, json_object_new_string("FirmwareStatusNotification"));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_FirmwareStatus[status]));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);

    enqueueSendMessage(uniqueId, json_string, OCPP_PACKAGE_FIRMWARE_STATUS_NOTIFICATION);

    json_object_put(root_object);

    free(uniqueId);

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
    if (root_object == NULL || uniqueId == NULL)
    {
        return -1;
    }
    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_MESSAGE));
    json_object_array_add(root_object, json_object_new_string(uniqueId));
    json_object_array_add(root_object, json_object_new_string("Heartbeat"));

    // 添加空对象{}，即使没有附带额外数据
    struct json_object *empty_object = json_object_new_object();
    json_object_array_add(root_object, empty_object);
    const char *json_string = json_object_to_json_string(root_object);
    enqueueSendMessage(uniqueId, json_string, OCPP_PACKAGE_HEARTBEAT);

    free(uniqueId);

    json_object_put(root_object);

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
        MeterValuesSampledDataCnt = ocpp_AuxiliaryTool_charCounter(MeterValuesSampledData, ',') + 1;
        ocpp_package_MeterValues_M_SampledValue sampledValue[MeterValuesSampledDataCnt];
        char *token = strtok(MeterValuesSampledData, ",");
        char *timestamp = ocpp_AuxiliaryTool_GetCurrentTime();
        struct json_object *root_object = json_object_new_array();
        if (root_object == NULL || uniqueId == NULL || timestamp == NULL)
        {
            return -1;
        }
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
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PF]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_V]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getPowerFactor(connector));
            }
            else if (strcmp(token, "Power.Active.Import") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PAI]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_WH]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getPowerActiveImport(connector));
            }
            else if (strcmp(token, "Power.Active.Export") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PAE]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_WH]);
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
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_CI]);
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
            token = strtok(NULL, ",");
            i++;
        }
        json_object_object_add(meterValue_object, "sampledValue", sampledValue_array);
        json_object_array_add(meterValue_array, meterValue_object);
        json_object_object_add(payload_object, "meterValue", meterValue_array);
        json_object_array_add(root_object, payload_object);
        const char *json_string = json_object_to_json_string(root_object);

        enqueueSendMessage(uniqueId, json_string, OCPP_PACKAGE_METERVALUES);

        free(uniqueId);

        free(timestamp);

        json_object_put(root_object);
    }

    return 0;
}

/**
 * @description:
 * @param:
 * @return:
 */
int ocpp_chargePoint_sendStartTransaction(int connector, const char *idTag, int reservationId, char *UniqueId, char *timestamp, int metervalue)
{
    struct json_object *root_object = json_object_new_array();
    if (root_object == NULL || UniqueId == NULL || timestamp == NULL)
    {
        return -1;
    }

    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_MESSAGE));
    json_object_array_add(root_object, json_object_new_string(UniqueId));
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
    enqueueSendMessage(UniqueId, json_string, OCPP_PACKAGE_STARTTRANSACTION);

    free(root_object);

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
    if (root_object == NULL || uniqueId == NULL)
    {
        return -1;
    }
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

    json_object_put(root_object);

    free(uniqueId);

    return 0;
}
/**
 * @description:
 * @param:
 * @return:
 */
int ocpp_transaction_sendStopTransaction(int connector, const char *idTag, int transactionId, const char *UniqueId, int meterStop, char *timestamp, enum OCPP_PACKAGE_STOP_REASON_E reason)
{

    const char *transactionDataStr = NULL;
    struct json_object *root_object = json_object_new_array();
    if (root_object == NULL || UniqueId == NULL || timestamp == NULL)
    {
        return -1;
    }
    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_MESSAGE));
    json_object_array_add(root_object, json_object_new_string(UniqueId));
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
        MeterValuesSampledDataCnt = ocpp_AuxiliaryTool_charCounter(StopTxnSampledData, ',') + 1;
        ocpp_package_MeterValues_M_SampledValue sampledValue[MeterValuesSampledDataCnt];
        char *token = strtok(StopTxnSampledData, ",");
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
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PF]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_V]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getPowerFactor(connector));
            }
            else if (strcmp(token, "Power.Active.Import") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PAI]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_WH]);
                snprintf(sampledValue[i].value, 16, "%0.2f", ocpp_chargePoint->getPowerActiveImport(connector));
            }
            else if (strcmp(token, "Power.Active.Export") == 0)
            {
                snprintf(sampledValue[i].context, 32, "%s", ocpp_package_MeterValues_MS_context_text[OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK]);
                snprintf(sampledValue[i].format, 32, "%s", ocpp_package_MeterValues_MS_format_text[OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW]);
                snprintf(sampledValue[i].location, 32, "%s", ocpp_package_MeterValues_MS_location_text[OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET]);
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PAE]);
                snprintf(sampledValue[i].unit, 32, "%s", ocpp_package_MeterValues_MS_unit_text[OCPP_PACKAGE_METERVALUES_MS_UNIT_WH]);
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
                snprintf(sampledValue[i].measurand, 32, "%s", ocpp_package_MeterValues_MS_measurand_text[OCPP_PACKAGE_METERVALUES_MS_MEASURAND_CI]);
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
            token = strtok(NULL, ",");
            i++;
        }
        json_object_object_add(transactionData_obj, "sampledValue", values_array);
        transactionDataStr = json_object_to_json_string(transactionData_obj);
        json_object_array_add(transactionData_array, transactionData_obj);
        json_object_object_add(payload_object, "transactionData", transactionData_array);
    }

    json_object_array_add(root_object, payload_object);
    const char *json_string = json_object_to_json_string(root_object);
    enqueueSendMessage(UniqueId, json_string, OCPP_PACKAGE_STOPTRANSACTION);
    json_object_put(root_object);

    return 0;
}
/**
 * @description:
 * @param {char *} uniqueId
 * @param {ocpp_package_CancelReservation_req_t} cancelReservation_req
 * @return {*}
 */
void ocpp_chargePoint_manageCancelReservationRequest(const char *uniqueId, ocpp_package_CancelReservation_req_t cancelReservation_req)
{
    struct json_object *root_object = json_object_new_array();
    if (root_object == NULL || uniqueId == NULL)
    {
        return;
    }
    ocpp_package_CancelReservation_conf_t cancelReservation_conf;
    memset(&cancelReservation_conf, 0, sizeof(cancelReservation_conf));
    cancelReservation_conf.status = OCPP_PACKAGE_CANCEL_RESERVATION_STATUS_REJECTED;

    int connector = 0;
    for (connector = 0; connector <= ocpp_chargePoint->numberOfConnector; connector++)
    {
        if (ocpp_chargePoint->reserveConnector[connector]->reservationId == cancelReservation_req.reservationId)
        {
            memset(ocpp_chargePoint->reserveConnector[connector], 0, sizeof(ocpp_chargePoint->reserveConnector[connector]));
            ocpp_chargePoint->setConnectorStatus(connector, OCPP_PACKAGE_CHARGEPOINT_ERRORCODE_NOERROR, OCPP_PACKAGE_CHARGEPOINT_STATUS_AVAILABLE);
            ocpp_chargePoint->getReservationStatus(connector, 0);
            cancelReservation_conf.status = OCPP_PACKAGE_CANCEL_RESERVATION_STATUS_ACCEPTED;
            break;
        }
    }

    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_object, json_object_new_string(uniqueId));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_CancelReservationStatus_text[cancelReservation_conf.status]));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);

    enqueueSendMessage(uniqueId, json_string, OCPP_PACKAGE_CALL);
    json_object_put(root_object);
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
                return NULL;
            }
        }
        else
        {
            if (ocpp_chargePoint->transaction_obj[connector]->isTransaction == false)
            {
                ocpp_chargePoint->setConnectorStatus(connector, OCPP_PACKAGE_CHARGEPOINT_ERRORCODE_NOERROR, OCPP_PACKAGE_CHARGEPOINT_STATUS_UNAVAILABLE);

                return NULL;
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

    struct json_object *root_object = json_object_new_array();
    if (uniqueId == NULL || root_object == NULL)
    {
        return;
    }

    ocpp_package_ChangeAvailability_conf_t changeAvailability_conf;
    memset(&changeAvailability_conf, 0, sizeof(changeAvailability_conf));

    changeAvailability_conf.status = OCPP_PACKAGE_AVAILABILITY_STATUS_ACCEPTED;

    if (changeAvailability_req.type == OCPP_PACKAGE_AVAILABILITY_TYPE_OPERATIVE) // 如果设备是可用的情况，直接改
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

                    changeAvailability_conf.status = OCPP_PACKAGE_AVAILABILITY_STATUS_SCHEDULED; // 把状态改成'Scheduled'，以指示 Scheduled（计划） 在交易完成后发生。
                    break;
                }
            }
        }
        else
        {
            if (ocpp_chargePoint->transaction_obj[changeAvailability_req.connectorId]->isTransaction) // 把状态改成'Scheduled'，以指示 Scheduled（计划） 在交易完成后发生。
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
            printf("create changeAvailability thread fail\n");
        }
        pthread_detach(tid_changeAvailability);
    }
    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_object, json_object_new_string(uniqueId));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_AvailabilityStatus_text[changeAvailability_conf.status]));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);

    enqueueSendMessage(uniqueId, json_string, OCPP_PACKAGE_CALL);

    json_object_put(root_object);
}
/**
 * @description:
 * @param {char *} uniqueId
 * @param {ocpp_package_ChangeConfiguration_req_t } changeConfiguration
 * @return {*}
 */
void ocpp_chargePoint_manageChangeConfigurationRequest(const char *uniqueId, ocpp_package_ChangeConfiguration_req_t changeConfiguration_req)
{
    struct json_object *root_object = json_object_new_array();
    if (uniqueId == NULL || root_object == NULL)
    {
        return;
    }

    ocpp_package_ChangeConfiguration_conf_t changeConfiguration_conf;

    changeConfiguration_conf.status = OCPP_PACKAGE_CONFIGURATION_STATUS_NOTSUPPORTED;

    if (ocpp_ConfigurationKey_isFound(changeConfiguration_req.key))
    {
        if (ocpp_ConfigurationKey_getIsUse(changeConfiguration_req.key))
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
    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_object, json_object_new_string(uniqueId));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_ConfigurationStatus_text[changeConfiguration_conf.status]));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);
    enqueueSendMessage(uniqueId, json_string, OCPP_PACKAGE_CALL);
    json_object_put(root_object);
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {ocpp_package_ClearCache_req_t} clearCache
 * @return {*}
 */
void ocpp_chargePoint_manageClearCacheRequest(const char *uniqueId, ocpp_package_ClearCache_req_t clearCache_req)
{
    struct json_object *root_object = json_object_new_array();
    if (uniqueId == NULL || root_object == NULL)
    {
        return;
    }

    ocpp_package_ClearCache_conf_t clearCache_conf;

    clearCache_conf.status = OCPP_PACKAGE_CLEAR_CACHE_STATUS_REJECTED;
    if (ocpp_localAuthorization_Cache_clearCache() == 0)
    {
        clearCache_conf.status = OCPP_PACKAGE_CLEAR_CACHE_STATUS_ACCEPTED;
    }

    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_object, json_object_new_string(uniqueId));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_ClearCacheStatus_text[clearCache_conf.status]));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);

    enqueueSendMessage(uniqueId, json_string, OCPP_PACKAGE_CALL);
    json_object_put(root_object);
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
 * @description: 创建 GetConfiguration.conf 消息
 * @param:
 * @return:
 */
void ocpp_package_prepare_GetConfiguration_Response(const char *UniqueId, ocpp_package_GetConfiguration_conf_t *getConfiguration_conf)
{

    // 创建一个 JSON 对象用于存储 GetConfiguration.conf 响应
    struct json_object *response_obj = json_object_new_array();
    if (UniqueId == NULL || getConfiguration_conf == NULL || response_obj == NULL)
    {
        return;
    }

    json_object_array_add(response_obj, json_object_new_int(OCPP_PACKAGE_CALL_RESULT)); // 请求序列号
    json_object_array_add(response_obj, json_object_new_string(UniqueId));              // 唯一标识符
    json_object_array_add(response_obj, json_object_new_string("GetConfiguration"));    // 操作类型

    // 创建一个 JSON 对象用于存储配置信息
    struct json_object *config_obj = json_object_new_object();

    // 创建 "configurationKey" 数组和 "unknownKey" 数组
    struct json_object *configuration_key_array = json_object_new_array();
    struct json_object *unknown_key_array = json_object_new_array();
    // 遍历 GetConfiguration.conf 结构体数组
    int i;
    for (i = 0; i < getConfiguration_conf->numberOfKey; i++)
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
 * @description:
 * @param {char *} uniqueId
 * @param {ocpp_package_RemoteStartTransaction_req_t} remoteStartTransaction_req
 * @return {*}
 */
void ocpp_chargePoint_manageRemoteStartTransaction_Req(const char *uniqueId, ocpp_package_RemoteStartTransaction_req_t remoteStartTransaction_req)
{
    struct json_object *root_object = json_object_new_array();
    if (root_object == NULL || uniqueId == NULL)
    {
        return;
    }
    ocpp_package_RemoteStartTransaction_conf_t remoteStartTransaction_conf;
    memset(&remoteStartTransaction_conf, 0, sizeof(remoteStartTransaction_conf));
    ocpp_chargePoint_transaction_t *transaction = ocpp_chargePoint->transaction_obj[remoteStartTransaction_req.connectorId];
    remoteStartTransaction_conf.status = OCPP_PACKAGE_REMOTE_STRATSTOP_STATUS_REJECTED;
    if ((transaction->isStart == false && transaction->isTransaction == false && ocpp_chargePoint->connector[remoteStartTransaction_req.connectorId]->status == OCPP_PACKAGE_CHARGEPOINT_STATUS_PREPARING))
    {
        write_data_lock();
        memset(transaction, 0, sizeof(ocpp_chargePoint_transaction_t));
        strncpy(transaction->startIdTag, remoteStartTransaction_req.idTag, OCPP_AUTHORIZATION_IDTAG_LEN);
        transaction->isStart = true;
        transaction->startupType = 1;
        rwlock_unlock();
        remoteStartTransaction_conf.status = OCPP_PACKAGE_REMOTE_STRATSTOP_STATUS_ACCEPTED;
    }
    else
    {
        remoteStartTransaction_conf.status = OCPP_PACKAGE_REMOTE_STRATSTOP_STATUS_REJECTED;
    }

    // response
    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_object, json_object_new_string(uniqueId));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_RemoteStartStopStatus_text[remoteStartTransaction_conf.status]));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);

    enqueueSendMessage(uniqueId, json_string, OCPP_PACKAGE_CALL);

    json_object_put(root_object);
}

void ocpp_chargePoint_manageRemoteStopTransactionRequest(const char *uniqueId, ocpp_package_RemoteStopTransaction_req_t remoteStopTransaction_req)
{
    struct json_object *root_object = json_object_new_array();
    if (uniqueId == NULL || root_object == NULL)
    {
        return;
    }
    int status = OCPP_PACKAGE_REMOTE_STRATSTOP_STATUS_REJECTED;

    int connector = 0;
    for (connector = 1; connector <= ocpp_chargePoint->numberOfConnector; connector++)
    {
        printf("isTransaction = %d\n", ocpp_chargePoint->transaction_obj[connector]->isTransaction);
        if (ocpp_chargePoint->transaction_obj[connector]->isTransaction)
            if (ocpp_chargePoint->transaction_obj[connector]->transactionId == remoteStopTransaction_req.transactionId)
            {
                ocpp_chargePoint->remoteStopCharging(connector);
                status = OCPP_PACKAGE_REMOTE_STRATSTOP_STATUS_ACCEPTED;
                break;
            }
    }

    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_object, json_object_new_string(uniqueId));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_RemoteStartStopStatus_text[status]));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);

    enqueueSendMessage(uniqueId, json_string, OCPP_PACKAGE_CALL);
    json_object_put(root_object);
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {ocpp_package_ReserveNow_req_t} reserveNow
 * @return {*}
 */
void ocpp_chargePoint_manageReserveNowRequest(const char *uniqueId, ocpp_package_ReserveNow_req_t reserveNow_req)
{
    struct json_object *root_object = json_object_new_array();
    if (root_object == NULL || uniqueId == NULL || reserveNow_req.connectorId < 0 || reserveNow_req.connectorId > ocpp_chargePoint->numberOfConnector)
    {
        return;
    }
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
        memset(ocpp_chargePoint->reserveConnector[reserveNow_req.connectorId], 0, sizeof(ocpp_chargePoint->reserveConnector[reserveNow_req.connectorId]));
        snprintf(ocpp_chargePoint->reserveConnector[reserveNow_req.connectorId]->idTag, OCPP_AUTHORIZATION_IDTAG_LEN, "%s", reserveNow_req.idTag);
        snprintf(ocpp_chargePoint->reserveConnector[reserveNow_req.connectorId]->expiryDate, 32, "%s", reserveNow_req.idTag);
        if (reserveNow_req.parentIdTagIsUse)
        {
            snprintf(ocpp_chargePoint->reserveConnector[reserveNow_req.connectorId]->parentIdTag, OCPP_AUTHORIZATION_IDTAG_LEN, "%s", reserveNow_req.parentIdTag);
        }
        ocpp_chargePoint->reserveConnector[reserveNow_req.connectorId]->reservationId = reserveNow_req.reservationId;
        ocpp_chargePoint->setConnectorStatus(reserveNow_req.connectorId, OCPP_PACKAGE_CHARGEPOINT_ERRORCODE_NOERROR, OCPP_PACKAGE_CHARGEPOINT_STATUS_RESERVED);
        ocpp_chargePoint->getReservationStatus(reserveNow_req.connectorId, 6);
    }

    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_object, json_object_new_string(uniqueId));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_ReservationStatus_text[reserveNow_conf.status]));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);

    enqueueSendMessage(uniqueId, json_string, OCPP_PACKAGE_CALL);
    json_object_put(root_object);
}
/**
 * @description:
 * @param {void *} arg
 * @return {*}
 */
void *ocpp_chargePoint_Reset_thread(void *arg)
{
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
                usleep(5 * 1000 * 1000);
                system("reboot");
                printf("software reset\n");
            }
            else
            {
                printf("hard reset\n");
                usleep(5 * 1000 * 1000);
                system("reboot");
            }

            return NULL;
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
    struct json_object *root_object = json_object_new_array();
    if (uniqueId == NULL || NULL == root_object)
    {
        return;
    }
    ocpp_package_Reset_conf_t reset_conf;
    pthread_t tid_reset;
    if (pthread_create(&tid_reset, NULL, ocpp_chargePoint_Reset_thread, (void *)reset_req.type) == -1)
    {
        printf("create reset thread fail\n");
    }
    pthread_detach(tid_reset);

    reset_conf.status = OCPP_PACKAGE_RESET_STATUS_ACCEPTED;
    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_object, json_object_new_string(uniqueId));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_ResetStatus_text[reset_conf.status]));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);

    enqueueSendMessage(uniqueId, json_string, OCPP_PACKAGE_CALL);
    json_object_put(root_object);
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
    if (sendLocalList_req->UpdateType == OCPP_PACKAGE_UPDATE_TYPE_FULL)
    {
        ocpp_localAuthorization_List_clearList();
    }
    int LocalAuthListEnabled = 0;
    int LocalAuthListMaxLength;
    int Length = ocpp_localAuthorization_List_RecordCount() + sendLocalList_req->numberOfAuthorizationData;
    ocpp_ConfigurationKey_getValue("LocalAuthListEnabled", &LocalAuthListEnabled);
    ocpp_ConfigurationKey_getValue("LocalAuthListMaxLength", &LocalAuthListMaxLength);

    if (LocalAuthListEnabled == 0)
    {
        sendLocalList_conf.status = OCPP_PCAKGE_UPDATE_STATUS_NOTSUPPORTED;
    }
    else if (Length > LocalAuthListMaxLength)
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
        // 更新本地列表
        if (sendLocalList_req->numberOfAuthorizationData > 0 && sendLocalList_req->localAuthorizationListIsUse)
        {
            ocpp_localAuthorization_list_entry_t entry;
            memset(&entry, 0, sizeof(ocpp_localAuthorization_list_entry_t));
            int i;
            for (i = 0; i < sendLocalList_req->numberOfAuthorizationData; i++)
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
 * @description:
 * @param {char *} uniqueId
 * @param {ocpp_package_TriggerMessage_req_t} triggerMessage
 * @return {*}
 */
void ocpp_chargePoint_manageTriggerMessageRequest(const char *uniqueId, ocpp_package_TriggerMessage_req_t triggerMessage_req)
{
    ocpp_package_prepare_Status_Req((char *)uniqueId, 0);

    switch (triggerMessage_req.requestedMessage)
    {
    case OCPP_PCAKGE_MESSAGE_TRIGGER_BOOTNOTIFICATION:
        ocpp_chargePoint_sendBootNotification_req();
        break;

    case OCPP_PCAKGE_MESSAGE_TRIGGER_DIAGNOSTICSSTATUS_NOTIFICATION:
        ocpp_chargePoint_sendDiagnosticsStatusNotification_Req(ocpp_chargePoint->ocpp_diagnostics_lastDiagnosticsStatus);
        break;

    case OCPP_PCAKGE_MESSAGE_TRIGGER_FIRMWARESTATUS_NOTIFICATION:
        ocpp_chargePoint_sendFirmwareStatusNotification_Req(ocpp_chargePoint->ocpp_firmwareUpdate_lastUpdateState);
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
    struct json_object *root_object = json_object_new_array();
    if (uniqueId == NULL || root_object == NULL)
    {
        return;
    }
    ocpp_package_UnlockConnector_conf_t unlockConnector_conf;

    unlockConnector_conf.status = OCPP_PACKAGE_UNLOCK_STATUS_NOTSUPPORTED;
    json_object_array_add(root_object, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_object, json_object_new_string(uniqueId));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_UnlockStatus_text[unlockConnector_conf.status]));

    json_object_array_add(root_object, payload_object);

    const char *json_string = json_object_to_json_string(root_object);
    enqueueSendMessage(uniqueId, json_string, OCPP_PACKAGE_CALL);

    json_object_put(root_object);
}

void ocpp_package_prepare_Status_Req(char *UniqueId, int status)
{

    struct json_object *root_array = json_object_new_array();
    if (root_array == NULL)
    {
        return;
    }

    json_object_array_add(root_array, json_object_new_int(OCPP_PACKAGE_CALL_RESULT));
    json_object_array_add(root_array, json_object_new_string(UniqueId));

    struct json_object *payload_object = json_object_new_object();
    json_object_object_add(payload_object, "status", json_object_new_string(ocpp_package_ConfigurationStatus_text[status]));

    json_object_array_add(root_array, payload_object);

    const char *json_string = json_object_to_json_string(root_array);
    enqueueSendMessage(UniqueId, json_string, OCPP_PACKAGE_CALL);
    json_object_put(root_array);
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
