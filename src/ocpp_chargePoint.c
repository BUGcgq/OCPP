#include "ocpp_chargePoint.h"
#include "ocpp_configurationKey.h"
#include "ocpp_config.h"
#include "ocpp_localAuthorization.h"
#include "ocpp_list.h"
#include "ocpp_firmwareUpdata.h"
#include "ocpp_diagnostics.h"
#include "ocpp_chargingProfile.h"
#include "ocpp_auxiliaryTool.h"
#include "ocpp_bootNotification.h"
#include "ocpp_transaction.h"
#include <time.h>

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
void ocpp_chargePoint_Chargingint(int connector, int type)
{

    return;
}
// 设置充电桩状态
void ocpp_chargePoint_setChangerStatus(int connector, int status)
{
}

// 用户点击启动充电
void ocpp_chargePoint_userPushStartButton(char *idTag, int connector)
{

    if (connector < 0 || connector > ocpp_chargePoint->numberOfConnector)
    {

        return;
    }
    write_data_lock();
    ocpp_chargePoint_transaction_t *transaction = ocpp_chargePoint->transaction_obj[connector];
    memset(transaction, 0, sizeof(ocpp_chargePoint_transaction_t));
    if (!transaction->isStart && !transaction->isTransaction)
    {
        strncpy(transaction->startIdTag, idTag, OCPP_AUTHORIZATION_IDTAG_LEN);
        transaction->isStart = true;
    }
    rwlock_unlock();
}

// 用户点击停止充电
void ocpp_chargePoint_userPushStopButton(char *idTag, int connector, enum OCPP_PACKAGE_STOP_REASON_E reason)
{

    if (connector < 0 || connector > ocpp_chargePoint->numberOfConnector)
    {

        return;
    }
    write_data_lock();
    ocpp_chargePoint_transaction_t *transaction = ocpp_chargePoint->transaction_obj[connector];
    transaction->isStop = true;
    transaction->reason = reason;

    transaction->stopIdTagIsUse = 0;
    memset(transaction->stopIdTag, 0, OCPP_AUTHORIZATION_IDTAG_LEN);
    if (idTag != NULL)
    {
        transaction->stopIdTagIsUse = 1;
        strncpy(transaction->stopIdTag, idTag, OCPP_AUTHORIZATION_IDTAG_LEN);
        printf("stopIdTag = %s\n", transaction->stopIdTag);
    }
    rwlock_unlock();
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

    if (ocpp_chargePoint == NULL || ocpp_chargePoint->connector[connector] == NULL || connector < 0 || connector > ocpp_chargePoint->numberOfConnector)
        return;

    write_data_lock();
    ocpp_chargePoint->connector[connector]->errorCode = errorCode;
    ocpp_chargePoint->connector[connector]->status = status;

    ocpp_chargePoint->connector[connector]->timestampIsUse = 1;
    char *timestamp = ocpp_AuxiliaryTool_GetCurrentTime();

    strncpy(ocpp_chargePoint->connector[connector]->timestamp, timestamp, 32);
    if (timestamp)
    {
        free(timestamp);
    }
    rwlock_unlock();
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
    if (connector < 0 || connector > ocpp_chargePoint->numberOfConnector)
    {
        return;
    }
    write_data_lock();
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
    rwlock_unlock();
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
    read_data_lock();
    bool Connect = ocpp_chargePoint->connect.isConnect;
    rwlock_unlock();
    return Connect;
} /**
   * @description: 获取连接状态
   * @return {*}
   */
bool ocpp_chargePoint_setConnectStatus(bool Status)
{
    if (ocpp_chargePoint == NULL)
        return false;
    write_data_lock();
    ocpp_chargePoint->connect.isConnect = Status;
    rwlock_unlock();
    return true;
}

/**
 * @description:
 * @param:
 * @return: 0:fail , 1:ongoing, 2:success, 3:unkown
 */
enum OCPP_CHARGEPOINT_AUTHORIZE_RESULT_E ocpp_chargePoint_authorizationOfIdentifier(const char *const idTag, char *uniqueId)
{
    if (idTag == NULL)
        return OCPP_CHARGEPOINT_AUTHORIZE_RESULT_FAIL;

    int localAuthorizeOffline = 0;
    int authorizationCacheEnabled = 0;
    int localAuthListEnabled = 0;
    int localPreAuthorize = 0;

    ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[OCPP_CK_LocalAuthorizeOffline], (void *)&localAuthorizeOffline);
    ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[OCPP_CK_AuthorizationCacheEnabled], (void *)&authorizationCacheEnabled);
    ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[OCPP_CK_LocalAuthListEnabled], (void *)&localAuthListEnabled);
    ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[OCPP_CK_LocalPreAuthorize], (void *)&localPreAuthorize);
    if (ocpp_chargePoint->connect.isConnect) // 服务器连接的情况下发送IdTag
    {
        ocpp_chargePoint_sendAuthorize_req(idTag, uniqueId);
        if (localPreAuthorize) // 支持localPreAuthorize授权
        {
            if (localAuthListEnabled) // 打开localAuthListEnabled
            {
                if (ocpp_localAuthorization_List_search(idTag)) // 本地列表是否存在IdTag
                {
                    if (ocpp_localAuthorization_List_isValid(idTag))
                    {
                        return OCPP_CHARGEPOINT_AUTHORIZE_RESULT_SUCCEED; // 列表有效
                    }
                }
            }

            if (authorizationCacheEnabled) // 打开authorizationCacheEnabled
            {
                if (ocpp_localAuthorization_Cache_search(idTag)) // 缓存是否存在IdTag
                {
                    if (ocpp_localAuthorization_Cache_isValid(idTag))
                    {
                        return OCPP_CHARGEPOINT_AUTHORIZE_RESULT_SUCCEED; // 缓存有效
                    }
                }
            }
        }

        return OCPP_CHARGEPOINT_AUTHORIZE_RESULT_ONGOING;
    }
    else
    {
        if (localAuthorizeOffline)
        {
            if (localAuthListEnabled)
            {
                if (ocpp_localAuthorization_List_search(idTag)) // 本地列表是否存在IdTag
                {
                    if (ocpp_localAuthorization_List_isValid(idTag))
                    {
                        return OCPP_CHARGEPOINT_AUTHORIZE_RESULT_SUCCEED; // 列表有效
                    }
                }
            }

            if (authorizationCacheEnabled)
            {
                if (ocpp_localAuthorization_Cache_search(idTag)) // 缓存是否存在IdTag
                {
                    if (ocpp_localAuthorization_Cache_isValid(idTag))
                    {
                        return OCPP_CHARGEPOINT_AUTHORIZE_RESULT_SUCCEED; // 缓存有效
                    }
                }
            }
        }
    }

    return OCPP_CHARGEPOINT_AUTHORIZE_RESULT_FAIL;
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

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
 * @param {ocpp_package_GetConfiguration_req_t } getConfiguration
 * @return {*}
 */
void ocpp_chargePoint_manage_GetConfigurationRequest(const char *uniqueId, ocpp_package_GetConfiguration_req_t getConfiguration_req)
{
    if (uniqueId == NULL)
    {
        return;
    }
    // 定义一个结构体数组，根据 getConfiguration_req.numberOfKey 分配空间
    ocpp_package_GetConfiguration_conf_t *getConfiguration_conf = (ocpp_package_GetConfiguration_conf_t *)malloc(getConfiguration_req.numberOfKey * sizeof(ocpp_package_GetConfiguration_conf_t));
    if (getConfiguration_conf == NULL)
    {
        return;
    }
    getConfiguration_conf->numberOfKey = getConfiguration_req.numberOfKey;
    int type;
    int i;
    int data = 0;
    int accessibility;
    for (i = 0; i < getConfiguration_req.numberOfKey; i++)
    {
        snprintf(getConfiguration_conf[i].key, sizeof(getConfiguration_conf[i].key), "%s", getConfiguration_req.key[i]);
        type = ocpp_ConfigurationKey_getType(getConfiguration_conf[i].key);
        accessibility = ocpp_ConfigurationKey_getAcc(getConfiguration_conf[i].key);
        if (accessibility == 0)
        {
            getConfiguration_conf[i].accessibility = 1;
        }
        else
        {
            getConfiguration_conf[i].accessibility = 0;
        }
        switch (type)
        {
        case 0:
            getConfiguration_conf[i].type = type;
            if (ocpp_ConfigurationKey_getValue(getConfiguration_conf[i].key, (void *)&data) == -1)
            {
                snprintf(getConfiguration_conf[i].value, sizeof(getConfiguration_conf[i].value), "%s", "ERROR");
            }
            else
            {
                if (data == 1)
                {
                    snprintf(getConfiguration_conf[i].value, sizeof(getConfiguration_conf[i].value), "%s", "true");
                }
                else
                {
                    snprintf(getConfiguration_conf[i].value, sizeof(getConfiguration_conf[i].value), "%s", "false");
                }
            }
            break;
        case 1:
            getConfiguration_conf[i].type = type;
            if (ocpp_ConfigurationKey_getValue(getConfiguration_conf[i].key, (void *)&data) == -1)
            {
                snprintf(getConfiguration_conf[i].value, sizeof(getConfiguration_conf[i].value), "%s", "ERROR");
            }
            else
            {
                snprintf(getConfiguration_conf[i].value, sizeof(getConfiguration_conf[i].value), "%d", data);
            }
            break;
        case 2:
            getConfiguration_conf[i].type = type;
            if (ocpp_ConfigurationKey_getValue(getConfiguration_conf[i].key, getConfiguration_conf[i].value) == -1)
            {
                snprintf(getConfiguration_conf[i].value, sizeof(getConfiguration_conf[i].value), "%s", "ERROR");
            }
            break;
        default:
            getConfiguration_conf[i].type = -1;
            break;
        }
    }

    ocpp_package_prepare_GetConfiguration_Response(uniqueId, getConfiguration_conf);

    free(getConfiguration_conf);
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {ocpp_package_GetDiagnostics_req_t} GetDiagnostics
 * @return {*}
 */
void ocpp_chargePoint_manageGetDiagnosticsRequest(ocpp_package_GetDiagnostics_req_t GetDiagnostics_req)
{
    ocpp_package_GetDiagnostics_req_t *GetDiagnostics_conf = malloc(sizeof(ocpp_package_GetDiagnostics_req_t));
    memcpy(GetDiagnostics_conf, &GetDiagnostics_req, sizeof(ocpp_package_GetDiagnostics_req_t));

    if (!GetDiagnostics_conf->retriesIsUse || GetDiagnostics_conf->retries == 0)
    {
        GetDiagnostics_conf->retries = 1;
    }
    else
    {
        GetDiagnostics_conf->retries = GetDiagnostics_conf->retries;
    }
    if (!GetDiagnostics_conf->retryIntervalIsUse || GetDiagnostics_conf->retryInterval == 0)
    {
        GetDiagnostics_conf->retryInterval = 120;
    }

    pthread_t ptid_diagnostics;
    if (pthread_create(&ptid_diagnostics, NULL, ocpp_chargePoint_diagnostics_upload_thread, (void *)GetDiagnostics_conf) == -1)
    {
        write_data_lock();
        ocpp_chargePoint->ocpp_diagnostics_lastDiagnosticsStatus = OCPP_PACKAGE_DIAGNOSTICS_STATUS_IDLE;
        rwlock_unlock();
        ocpp_chargePoint_sendDiagnosticsStatusNotification_Req(OCPP_PACKAGE_DIAGNOSTICS_STATUS_UPLOAD_FAILED);
        printf("create firmware thread fail\n");
    }
    else
    {

        pthread_detach(ptid_diagnostics);
    }
}

/**
 * @description:
 * @param {char *} uniqueId
 * @param {ocpp_package_UpdateFirmware_req_t} updateFirmware
 * @return {*}
 */
void ocpp_chargePoint_manageUpdateFirmwareRequest(ocpp_package_UpdateFirmware_req_t updateFirmware_req)
{
    ocpp_package_UpdateFirmware_req_t *updateFirmware_conf = malloc(sizeof(ocpp_package_UpdateFirmware_req_t));
    memcpy(updateFirmware_conf, &updateFirmware_req, sizeof(ocpp_package_UpdateFirmware_req_t));

    if (!updateFirmware_conf->retriesIsUse || updateFirmware_conf->retries == 0)
    {
        updateFirmware_conf->retries = 1;
    }
    else
    {
        updateFirmware_conf->retries = updateFirmware_conf->retries;
    }
    if (!updateFirmware_conf->retryIntervalIsUse || updateFirmware_conf->retryInterval == 0)
    {
        updateFirmware_conf->retryInterval = 120;
    }
    printf("开始更新\n");

    pthread_t tid_firmware;
    if (pthread_create(&tid_firmware, NULL, ocpp_chargePoint_UpdateFirmware_thread, (void *)updateFirmware_conf) == -1)
    {
        write_data_lock();
        ocpp_chargePoint->ocpp_firmwareUpdate_lastUpdateState = OCPP_PACKAGE_FIRMWARE_STATUS_IDLE;
        rwlock_unlock();
        ocpp_chargePoint_sendFirmwareStatusNotification_Req(OCPP_PACKAGE_FIRMWARE_STATUS_DOWNLOAD_FAILED);
        printf("create firmware thread fail\n");
    }
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

        char *message = ocpp_package_prepare_CallError(uniqueId, &callError);
        enqueueSendMessage(uniqueId, message, OCPP_PACKAGE_CALLERROR);
        if (message)
        {
            free(message);
        }
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

        char *message = ocpp_package_prepare_CallError(uniqueId, &callError);
        enqueueSendMessage(uniqueId, message, OCPP_PACKAGE_CALLERROR);
        if (message)
        {
            free(message);
        }
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

        char *message = ocpp_package_prepare_CallError(uniqueId, &callError);
        enqueueSendMessage(uniqueId, message, OCPP_PACKAGE_CALLERROR);
        if (message)
        {
            free(message);
        }
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
            if (connectorId < 0 || connectorId > ocpp_chargePoint->numberOfConnector)
            {

                snprintf(callError.errorDescription, 1024, "connector SHELL be >= 0 and <= %d", ocpp_chargePoint->numberOfConnector);
            }
            else
            {
                isError = false;
            }
        }
    }

    if (isError)
    {

        char *message = ocpp_package_prepare_CallError(uniqueId, &callError);
        enqueueSendMessage(uniqueId, message, OCPP_PACKAGE_CALLERROR);
        if (message)
        {
            free(message);
        }
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
            if (connectorId < 0 || connectorId > ocpp_chargePoint->numberOfConnector)
            {

                snprintf(callError.errorDescription, 1024, "connector SHELL be >= 0 and <= %d", ocpp_chargePoint->numberOfConnector);
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

        char *message = ocpp_package_prepare_CallError(uniqueId, &callError);
        enqueueSendMessage(uniqueId, message, OCPP_PACKAGE_CALLERROR);
        if (message)
        {
            free(message);
        }
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
        snprintf(callError.errorDescription, 1024, "The GetConfiguration request is missing key information.");
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

        char *message = ocpp_package_prepare_CallError(uniqueId, &callError);
        enqueueSendMessage(uniqueId, message, OCPP_PACKAGE_CALLERROR);
        if (message)
        {
            free(message);
        }
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

        char *message = ocpp_package_prepare_CallError(uniqueId, &callError);
        enqueueSendMessage(uniqueId, message, OCPP_PACKAGE_CALLERROR);
        if (message)
        {
            free(message);
        }
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

    if (connectorId_obj == NULL)
    {
        snprintf(callError.errorDescription, 1024, "connectorId SHELL be not NULL");
    }
    else if (idTag_obj == NULL)
    {
        snprintf(callError.errorDescription, 1024, "idTag SHELL be not NULL");
    }
    else
    {
        int connectorId = json_object_get_int(connectorId_obj);
        if (connectorId < 0 || connectorId > ocpp_chargePoint->numberOfConnector)
        {
            snprintf(callError.errorDescription, 1024, "connectorId SHELL be > 0 and <= %d", ocpp_chargePoint->numberOfConnector);
        }
        else
        {
            isError = false;
        }
    }

    if (isError)
    {

        char *message = ocpp_package_prepare_CallError(uniqueId, &callError);
        enqueueSendMessage(uniqueId, message, OCPP_PACKAGE_CALLERROR);
        if (message)
        {
            free(message);
        }
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

        char *message = ocpp_package_prepare_CallError(uniqueId, &callError);
        enqueueSendMessage(uniqueId, message, OCPP_PACKAGE_CALLERROR);
        if (message)
        {
            free(message);
        }
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

        char *message = ocpp_package_prepare_CallError(uniqueId, &callError);
        enqueueSendMessage(uniqueId, message, OCPP_PACKAGE_CALLERROR);
        if (message)
        {
            free(message);
        }
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

        char *message = ocpp_package_prepare_CallError(uniqueId, &callError);
        enqueueSendMessage(uniqueId, message, OCPP_PACKAGE_CALLERROR);
        if (message)
        {
            free(message);
        }
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
        char *message = ocpp_package_prepare_CallError(uniqueId, &callError);
        enqueueSendMessage(uniqueId, message, OCPP_PACKAGE_CALLERROR);
        if (message)
        {
            free(message);
        }
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
        int connectorId = json_object_get_int(connectorId_obj);
        if (connectorId < 0 || connectorId > ocpp_chargePoint->numberOfConnector)
        {
            snprintf(callError.errorDescription, 1024, "connectorId SHELL be > 0 and <= %d", ocpp_chargePoint->numberOfConnector);
        }
        else
        {
            isError = false;
        }
    }

    if (isError)
    {

        char *message = ocpp_package_prepare_CallError(uniqueId, &callError);
        enqueueSendMessage(uniqueId, message, OCPP_PACKAGE_CALLERROR);
        if (message)
        {
            free(message);
        }
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
        if (json_object_get_int(connectorId_obj) > ocpp_chargePoint->numberOfConnector)
        {
            isError = true;
            callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_PROPERTY_CONSTRAINT_VIOLATION;
            snprintf(callError.errorDescription, 1024, "%s", "connectorId  > MaxNumber");
        }
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

        char *message = ocpp_package_prepare_CallError(uniqueId, &callError);
        enqueueSendMessage(uniqueId, message, OCPP_PACKAGE_CALLERROR);
        if (message)
        {
            free(message);
        }
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

        char *message = ocpp_package_prepare_CallError(uniqueId, &callError);
        enqueueSendMessage(uniqueId, message, OCPP_PACKAGE_CALLERROR);
        if (message)
        {
            free(message);
        }
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

        char *message = ocpp_package_prepare_CallError(uniqueId, &callError);
        enqueueSendMessage(uniqueId, message, OCPP_PACKAGE_CALLERROR);
        if (message)
        {
            free(message);
        }
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

        char *message = ocpp_package_prepare_CallError(uniqueId, &callError);
        enqueueSendMessage(uniqueId, message, OCPP_PACKAGE_CALLERROR);
        if (message)
        {
            free(message);
        }
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

        char *message = ocpp_package_prepare_CallError(uniqueId, &callError);
        enqueueSendMessage(uniqueId, message, OCPP_PACKAGE_CALLERROR);
        if (message)
        {
            free(message);
        }
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

        char *message = ocpp_package_prepare_CallError(uniqueId, &callError);
        enqueueSendMessage(uniqueId, message, OCPP_PACKAGE_CALLERROR);
        if (message)
        {
            free(message);
        }
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

        char *message = ocpp_package_prepare_CallError(uniqueId, &callError);
        enqueueSendMessage(uniqueId, message, OCPP_PACKAGE_CALLERROR);
        if (message)
        {
            free(message);
        }
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

        char *message = ocpp_package_prepare_CallError(uniqueId, &callError);
        enqueueSendMessage(uniqueId, message, OCPP_PACKAGE_CALLERROR);
        if (message)
        {
            free(message);
        }
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
        ocpp_package_prepare_Status_Req(uniqueId_str, 3);
        // ocpp_chargePoint_manageCancelReservationRequest(uniqueId_str, cancelReservation_req);
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

        json_object *id_obj = json_object_object_get(payload_obj, "id");
        json_object *connectorId_obj = json_object_object_get(payload_obj, "connectorId");
        json_object *chargingProfilePurpose_obj = json_object_object_get(payload_obj, "chargingProfilePurpose");
        json_object *stackLevel_obj = json_object_object_get(payload_obj, "stackLevel");
        int id = -1;
        int connectorId = -1;
        int stackLevel = -1;
        if (connectorId_obj != NULL)
        {
            id = json_object_get_int(id_obj);
        }
        if (connectorId_obj != NULL)
        {
            connectorId = json_object_get_int(connectorId_obj);
        }
        if (stackLevel_obj != NULL)
        {
            stackLevel = json_object_get_int(stackLevel_obj);
        }
        if (ocpp_ChargingProfile_delete(connectorId, stackLevel, id) == 0)
        {
            ocpp_package_prepare_Status_Req(uniqueId_str, 0);
        }
        else
        {
            ocpp_package_prepare_Status_Req(uniqueId_str, 1);
        }
    }
    break;

    case OCPP_PACKAGE_DATATRANSFER:
    {
        ocpp_package_prepare_Status_Req(uniqueId_str, 0);
    }
    break;

    case OCPP_PACKAGE_GET_COMPOSITESCAEDULE:
    {
        if (ocpp_chargePoint_checkGetCompositeSchedule_Callpackage(uniqueId_str, payload_obj))
            return;

        ChargingProfile chargingProfiles;
        memset(&chargingProfiles, 0, sizeof(chargingProfiles));
        json_object *connectorId_obj = json_object_object_get(payload_obj, "connectorId");
        json_object *duration_obj = json_object_object_get(payload_obj, "duration");
        json_object *chargingRateUnit_obj = json_object_object_get(payload_obj, "chargingRateUnit");
        int duration = -1;
        char chargingRateUnit[2];
        if (chargingRateUnit_obj)
        {
            strncpy(chargingRateUnit, json_object_get_string(chargingRateUnit_obj), sizeof(chargingRateUnit));
        }
        if (duration_obj)
        {
            duration = json_object_get_int(duration_obj);
        }
        if (connectorId_obj)
        {
            chargingProfiles.connectorId = json_object_get_int(connectorId_obj);
        }
        if (ocpp_ChargingProfile_find(chargingProfiles.connectorId, chargingRateUnit, duration, &chargingProfiles) == 0)
        {
            ocpp_chargePoint_manageGetCompositeScheduleRequest(uniqueId_str, chargingProfiles);
        }
        else
        {
            ocpp_package_prepare_Status_Req(uniqueId_str, 1);
        }
        if (chargingProfiles.chargingSchedule.chargingSchedulePeriods)
        {
            free(chargingProfiles.chargingSchedule.chargingSchedulePeriods);
        }
    }
    break;

    case OCPP_PACKAGE_GET_CONFIGURATION:
    {
        if (ocpp_chargePoint_checkGetConfiguration_Callpackage(uniqueId_str, payload_obj))
            return;

        int MaxnumberOfKey = 0;
        int i;
        ocpp_ConfigurationKey_getValue("GetConfigurationMaxKeys", &MaxnumberOfKey);

        ocpp_package_GetConfiguration_req_t getConfiguration_req;
        memset(&getConfiguration_req, 0, sizeof(getConfiguration_req));

        json_object *key_obj = json_object_object_get(payload_obj, "key");

        // 解析 JSON 获取 "key" 数组
        if (key_obj != NULL && json_object_is_type(key_obj, json_type_array))
        {
            int numberOfKeys = json_object_array_length(key_obj);
            if (numberOfKeys > MaxnumberOfKey && MaxnumberOfKey != 0)
            {
                numberOfKeys = MaxnumberOfKey;
            }

            getConfiguration_req.numberOfKey = numberOfKeys;
            getConfiguration_req.key = (char **)malloc(getConfiguration_req.numberOfKey * sizeof(char *));

            for (i = 0; i < getConfiguration_req.numberOfKey; i++)
            {
                struct json_object *keyElement = json_object_array_get_idx(key_obj, i);
                if (keyElement != NULL && json_object_is_type(keyElement, json_type_string))
                {
                    const char *keyStr = json_object_get_string(keyElement);
                    getConfiguration_req.key[i] = (char *)malloc(strlen(keyStr) + 1); // 使用malloc和strcpy函数
                    strcpy(getConfiguration_req.key[i], keyStr);
                }
            }
        }
        ocpp_chargePoint_manage_GetConfigurationRequest(uniqueId_str, getConfiguration_req);
        // 在释放内存前检查是否已分配内存
        if (getConfiguration_req.key)
        {
            for (i = 0; i < getConfiguration_req.numberOfKey; i++)
            {
                if (getConfiguration_req.key[i]) // 检查指针是否为NULL
                {
                    free(getConfiguration_req.key[i]);
                }
            }
            free(getConfiguration_req.key);
            getConfiguration_req.key = NULL; // 设置为 NULL 避免悬空指针
        }
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
        read_data_lock();
        enum OCPP_PACKAGE_DIAGNOSTICS_STATUS_E DiagnosticsStatus = ocpp_chargePoint->ocpp_diagnostics_lastDiagnosticsStatus;
        rwlock_unlock();
        if (DiagnosticsStatus == OCPP_PACKAGE_DIAGNOSTICS_STATUS_IDLE)
        {
            ocpp_chargePoint_manageGetDiagnosticsRequest(getDiagnostics_req);
            ocpp_chargePoint_sendGetDiagnostics_req(uniqueId_str,OCPP_DIAGNOSTICS_UPDATA_FILENAME,0);
        }
        else
        {
            ocpp_chargePoint_sendGetDiagnostics_req(uniqueId_str, NULL,1);
        }
    }
    break;

    case OCPP_PACKAGE_GET_LOCALLISTVERSION:
    {
        if (ocpp_chargePoint_checkGetLocalListVersion_Callpackage(uniqueId_str, payload_obj))
            return;

        ocpp_chargePoint_GetLocalListVersion_Req(uniqueId_str);
    }
    break;

    case OCPP_PACKAGE_REMOTE_STARTTRANSACTION:
    {

        if (ocpp_chargePoint_checkRemoteStartTransaction_Callpackage(uniqueId_str, payload_obj))
            return;

        json_object *idTag_obj = NULL;
        json_object *connectorId_obj = NULL;
        json_object *chargingProfile_obj = NULL;

        if (json_object_object_get_ex(payload_obj, "idTag", &idTag_obj) &&
            json_object_object_get_ex(payload_obj, "connectorId", &connectorId_obj))
        {
            ocpp_package_RemoteStartTransaction_req_t remoteStartTransaction_req;
            memset(&remoteStartTransaction_req, 0, sizeof(remoteStartTransaction_req));

            const char *idTag_str = json_object_get_string(idTag_obj);
            if (idTag_str != NULL)
            {
                strncpy(remoteStartTransaction_req.idTag, idTag_str, sizeof(remoteStartTransaction_req.idTag));
            }

            remoteStartTransaction_req.connectorIdIsUse = 0;
            if (json_object_get_type(connectorId_obj) == json_type_int)
            {
                remoteStartTransaction_req.connectorIdIsUse = 1;
                remoteStartTransaction_req.connectorId = json_object_get_int(connectorId_obj);
            }

            remoteStartTransaction_req.chargingProfileIsUse = 0;
            if (json_object_object_get_ex(payload_obj, "chargingProfile", &chargingProfile_obj))
            {
                ChargingProfile chargingProfiles;

                json_object *chargeProfileId_obj = json_object_object_get(chargingProfile_obj, "chargingProfileId");
                if (json_object_get_type(chargeProfileId_obj) == json_type_int)
                {
                    chargingProfiles.chargingProfileId = json_object_get_int(chargeProfileId_obj);
                }
                // 解析 chargingProfileKind 字段
                json_object *chargingProfileKind_obj = json_object_object_get(chargingProfile_obj, "chargingProfileKind");
                const char *chargingProfileKind_str = json_object_get_string(chargingProfileKind_obj);
                if (chargingProfileKind_str != NULL)
                {
                    strncpy(chargingProfiles.chargingProfileKind, chargingProfileKind_str, sizeof(chargingProfiles.chargingProfileKind));
                }
                // 解析 chargingProfilePurpose 字段
                json_object *chargingProfilePurpose_obj = json_object_object_get(chargingProfile_obj, "chargingProfilePurpose");
                const char *chargingProfilePurpose_str = json_object_get_string(chargingProfilePurpose_obj);
                if (chargingProfilePurpose_str != NULL)
                {
                    strncpy(chargingProfiles.chargingProfilePurpose, chargingProfilePurpose_str, sizeof(chargingProfiles.chargingProfilePurpose) - 1);
                    if (strcmp(chargingProfiles.chargingProfilePurpose, "TxProfile") == 0)
                    {
                        chargingProfiles.transactionId = 1;
                    }
                }
                json_object *stackLevel_obj = json_object_object_get(chargingProfile_obj, "stackLevel");
                if (json_object_get_type(stackLevel_obj) == json_type_int)
                {
                    chargingProfiles.stackLevel = json_object_get_int(stackLevel_obj);
                }
                // 解析 chargingSchedule 对象
                json_object *chargingSchedule_obj = json_object_object_get(chargingProfile_obj, "chargingSchedule");
                if (chargingSchedule_obj != NULL)
                {
                    // 解析 duration 字段
                    json_object *duration_obj = json_object_object_get(chargingSchedule_obj, "duration");
                    if (json_object_get_type(duration_obj) == json_type_int)
                    {
                        chargingProfiles.chargingSchedule.duration = json_object_get_int(duration_obj);
                    }
                    // 解析 startSchedule 字段
                    json_object *startSchedule_obj = json_object_object_get(chargingSchedule_obj, "startSchedule");
                    const char *startSchedule_str = json_object_get_string(startSchedule_obj);
                    if (startSchedule_str != NULL)
                    {
                        strncpy(chargingProfiles.chargingSchedule.startSchedule, startSchedule_str, sizeof(chargingProfiles.chargingSchedule.startSchedule));
                    }
                    // 解析 chargingRateUnit 字段
                    json_object *chargingRateUnit_obj = json_object_object_get(chargingSchedule_obj, "chargingRateUnit");
                    const char *chargingRateUnit_str = json_object_get_string(chargingRateUnit_obj);
                    if (chargingRateUnit_str != NULL)
                    {
                        strncpy(chargingProfiles.chargingSchedule.chargingRateUnit, chargingRateUnit_str, sizeof(chargingProfiles.chargingSchedule.chargingRateUnit));
                    }
                    // 解析 chargingSchedulePeriod 数组
                    json_object *chargingSchedulePeriod_obj = json_object_object_get(chargingSchedule_obj, "chargingSchedulePeriod");
                    if (chargingSchedulePeriod_obj != NULL)
                    {
                        chargingProfiles.chargingSchedule.numPeriods = json_object_array_length(chargingSchedulePeriod_obj);
                        chargingProfiles.chargingSchedule.chargingSchedulePeriods = (ChargingSchedulePeriod *)malloc(chargingProfiles.chargingSchedule.numPeriods * sizeof(ChargingSchedulePeriod));
                        if (chargingProfiles.chargingSchedule.chargingSchedulePeriods != NULL)
                        {
                            int i;
                            // 在这里可以遍历 chargingSchedulePeriod 数组并解析每个元素
                            for (i = 0; i < chargingProfiles.chargingSchedule.numPeriods; i++)
                            {
                                json_object *period_obj = json_object_array_get_idx(chargingSchedulePeriod_obj, i);

                                // 解析 chargingSchedulePeriod 的字段，例如 startPeriod、limit、numberPhases
                                json_object *startPeriod_obj = json_object_object_get(period_obj, "startPeriod");
                                if (json_object_get_type(startPeriod_obj) == json_type_int)
                                {
                                    chargingProfiles.chargingSchedule.chargingSchedulePeriods[i].startPeriod = json_object_get_int(startPeriod_obj);
                                }

                                json_object *limit_obj = json_object_object_get(period_obj, "limit");
                                if (json_object_get_type(limit_obj) == json_type_double)
                                {
                                    chargingProfiles.chargingSchedule.chargingSchedulePeriods[i].limit = json_object_get_double(limit_obj);
                                }

                                json_object *numberPhases_obj = json_object_object_get(period_obj, "numberPhases");
                                if (json_object_get_type(numberPhases_obj) == json_type_int)
                                {
                                    chargingProfiles.chargingSchedule.chargingSchedulePeriods[i].numberPhases = json_object_get_int(numberPhases_obj);
                                }
                            }
                            ocpp_ChargingProfile_insert(remoteStartTransaction_req.connectorId, &chargingProfiles);
                            free(chargingProfiles.chargingSchedule.chargingSchedulePeriods);
                        }
                    }
                }
            }
            printf("接受到远程启动充电\n");
            ocpp_chargePoint_manageRemoteStartTransaction_Req(uniqueId_str, remoteStartTransaction_req);
        }
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
        ocpp_package_prepare_Status_Req(uniqueId_str, 3);
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
        struct json_object *listVersion_obj = json_object_object_get(payload_obj, "listVersion");
        struct json_object *updateType_obj = json_object_object_get(payload_obj, "updateType");
        struct json_object *localAuthorizationList_obj = json_object_object_get(payload_obj, "localAuthorizationList");
        int numberOfList = json_object_array_length(localAuthorizationList_obj);

        ocpp_package_SendLocalList_req_t sendLocalList_req;
        memset(&sendLocalList_req, 0, sizeof(sendLocalList_req));

        sendLocalList_req.listVersion = json_object_get_int(listVersion_obj);
        const char *updateType_str = json_object_get_string(updateType_obj);

        int i = 0;
        for (i = 0; i < OCPP_PACKAGE_UPDATE_TYPE_MAX; i++)
        {
            if (strcmp(ocpp_package_UpdateType_text[i], updateType_str) == 0)
            {
                break;
            }
        }
        sendLocalList_req.UpdateType = i;

        sendLocalList_req.localAuthorizationListIsUse = 0;
        if (numberOfList > 0)
        {
            sendLocalList_req.localAuthorizationListIsUse = 1;
            sendLocalList_req.numberOfAuthorizationData = numberOfList;
            sendLocalList_req.localAuthorizationList = (ocpp_package_AuthorizationData_t *)calloc(numberOfList, sizeof(ocpp_package_AuthorizationData_t));
            if (sendLocalList_req.localAuthorizationList == NULL)
            {
                printf("Memory allocation error.\n");
                break;
            }

            for (i = 0; i < numberOfList; i++)
            {
                struct json_object *localAuthorizetion_index = json_object_array_get_idx(localAuthorizationList_obj, i);

                struct json_object *idTag_obj = json_object_object_get(localAuthorizetion_index, "idTag");
                const char *idTag_str = json_object_get_string(idTag_obj);
                strncpy(sendLocalList_req.localAuthorizationList[i].idTag, idTag_str, OCPP_AUTHORIZATION_IDTAG_LEN);

                sendLocalList_req.localAuthorizationList[i].idTagInfoIsUse = 0;
                struct json_object *idTagInfo_obj = json_object_object_get(localAuthorizetion_index, "idTagInfo");
                if (idTagInfo_obj != NULL)
                {
                    sendLocalList_req.localAuthorizationList[i].idTagInfoIsUse = 1;
                    struct json_object *expiryDate_obj = json_object_object_get(idTagInfo_obj, "expiryDate");
                    struct json_object *parentIdTag_obj = json_object_object_get(idTagInfo_obj, "parentIdTag");
                    struct json_object *status_obj = json_object_object_get(idTagInfo_obj, "status");

                    const char *status_str = json_object_get_string(status_obj);
                    int j = 0;
                    for (j = 0; j < OCPP_LOCAL_AUTHORIZATION_MAX; j++)
                    {
                        if (strcmp(ocpp_localAuthorizationStatus_Text[j], status_str) == 0)
                        {
                            break;
                        }
                    }

                    sendLocalList_req.localAuthorizationList[i].idTagInfo.AuthorizationStatus = j;

                    sendLocalList_req.localAuthorizationList[i].idTagInfo.expiryDateIsUse = 0;
                    sendLocalList_req.localAuthorizationList[i].idTagInfo.parentIdTagIsUse = 0;

                    if (expiryDate_obj != NULL)
                    {
                        sendLocalList_req.localAuthorizationList[i].idTagInfo.expiryDateIsUse = 1;
                        const char *expiryDate_str = json_object_get_string(expiryDate_obj);
                        strncpy(sendLocalList_req.localAuthorizationList[i].idTagInfo.expiryDate, expiryDate_str, 32);
                    }

                    if (parentIdTag_obj != NULL)
                    {
                        sendLocalList_req.localAuthorizationList[i].idTagInfo.parentIdTagIsUse = 1;
                        const char *parentIdTag_str = json_object_get_string(parentIdTag_obj);
                        strncpy(sendLocalList_req.localAuthorizationList[i].idTagInfo.parentIdTag, parentIdTag_str, OCPP_LOCAL_AUTHORIZATION_MAX);
                    }
                }
            }
        }

        ocpp_chargePoint_manageSendLocalList_Req(uniqueId_str, &sendLocalList_req);
        if (sendLocalList_req.localAuthorizationList)
        {
            free(sendLocalList_req.localAuthorizationList);
        }
    }
    break;

    case OCPP_PACKAGE_SETCHARGINGPROFILE:
    {
        if (ocpp_chargePoint_checkSetChargingProfile_Callpackage(uniqueId_str, payload_obj))
            return;

        ChargingProfile chargingProfiles;
        memset(&chargingProfiles, 0, sizeof(chargingProfiles));
        // 解析 payload_obj 并填充结构体
        struct json_object *connectorIdObj = NULL;
        if (json_object_object_get_ex(payload_obj, "connectorId", &connectorIdObj))
        {
            chargingProfiles.connectorId = json_object_get_int(connectorIdObj);
        }

        struct json_object *csChargingProfilesObj = NULL;
        if (json_object_object_get_ex(payload_obj, "csChargingProfiles", &csChargingProfilesObj))
        {
            struct json_object *chargingProfileIdObj = NULL;
            if (json_object_object_get_ex(csChargingProfilesObj, "chargingProfileId", &chargingProfileIdObj))
            {
                chargingProfiles.chargingProfileId = json_object_get_int(chargingProfileIdObj);
            }

            struct json_object *transactionIdObj = NULL;
            if (json_object_object_get_ex(csChargingProfilesObj, "transactionId", &transactionIdObj))
            {
                chargingProfiles.transactionId = json_object_get_int(transactionIdObj);
            }

            struct json_object *stackLevelObj = NULL;
            if (json_object_object_get_ex(csChargingProfilesObj, "stackLevel", &stackLevelObj))
            {
                chargingProfiles.stackLevel = json_object_get_int(stackLevelObj);
            }
            struct json_object *vaildFromObj = NULL;
            if (json_object_object_get_ex(csChargingProfilesObj, "validFrom", &vaildFromObj))
            {
                strncpy(chargingProfiles.validFrom, json_object_get_string(vaildFromObj), sizeof(chargingProfiles.validFrom));
            }
            struct json_object *vaildToObj = NULL;
            if (json_object_object_get_ex(csChargingProfilesObj, "validTo", &vaildToObj))
            {
                strncpy(chargingProfiles.validTo, json_object_get_string(vaildToObj), sizeof(chargingProfiles.validTo));
            }
            struct json_object *chargingProfilePurposeObj = NULL;
            if (json_object_object_get_ex(csChargingProfilesObj, "chargingProfilePurpose", &chargingProfilePurposeObj))
            {
                strncpy(chargingProfiles.chargingProfilePurpose, json_object_get_string(chargingProfilePurposeObj), sizeof(chargingProfiles.chargingProfilePurpose));
            }

            struct json_object *chargingProfileKindObj = NULL;
            if (json_object_object_get_ex(csChargingProfilesObj, "chargingProfileKind", &chargingProfileKindObj))
            {
                strncpy(chargingProfiles.chargingProfileKind, json_object_get_string(chargingProfileKindObj), sizeof(chargingProfiles.chargingProfileKind));
            }

            struct json_object *recurrencyKindObj = NULL;
            if (json_object_object_get_ex(csChargingProfilesObj, "recurrencyKind", &recurrencyKindObj))
            {
                strncpy(chargingProfiles.recurrencyKind, json_object_get_string(recurrencyKindObj), sizeof(chargingProfiles.recurrencyKind));
            }

            struct json_object *chargingScheduleObj = NULL;
            if (json_object_object_get_ex(csChargingProfilesObj, "chargingSchedule", &chargingScheduleObj))
            {
                struct json_object *chargingRateUnitObj = NULL;
                if (json_object_object_get_ex(chargingScheduleObj, "chargingRateUnit", &chargingRateUnitObj))
                {
                    strncpy(chargingProfiles.chargingSchedule.chargingRateUnit, json_object_get_string(chargingRateUnitObj), sizeof(chargingProfiles.chargingSchedule.chargingRateUnit));
                }
                struct json_object *startScheduleObj = NULL;
                if (json_object_object_get_ex(csChargingProfilesObj, "startSchedule", &startScheduleObj))
                {
                    strncpy(chargingProfiles.chargingSchedule.startSchedule, json_object_get_string(startScheduleObj), sizeof(chargingProfiles.chargingSchedule.startSchedule));
                }
                struct json_object *durationObj = NULL;
                if (json_object_object_get_ex(csChargingProfilesObj, "duration", &durationObj))
                {
                    chargingProfiles.chargingSchedule.duration = json_object_get_int(durationObj);
                }
                struct json_object *chargingSchedulePeriodArray = NULL;
                if (json_object_object_get_ex(chargingScheduleObj, "chargingSchedulePeriod", &chargingSchedulePeriodArray))
                {
                    int arrayLength = json_object_array_length(chargingSchedulePeriodArray);
                    // 分配内存以保存chargingSchedulePeriod数组;
                    chargingProfiles.chargingSchedule.numPeriods = arrayLength;
                    chargingProfiles.chargingSchedule.chargingSchedulePeriods = (ChargingSchedulePeriod *)malloc(arrayLength * sizeof(ChargingSchedulePeriod));
                    int i;
                    for (i = 0; i < arrayLength; i++)
                    {
                        struct json_object *chargingSchedulePeriodObj = json_object_array_get_idx(chargingSchedulePeriodArray, i);
                        if (chargingSchedulePeriodObj != NULL)
                        {
                            struct json_object *startPeriodObj = NULL;
                            if (json_object_object_get_ex(chargingSchedulePeriodObj, "startPeriod", &startPeriodObj))
                            {
                                chargingProfiles.chargingSchedule.chargingSchedulePeriods[i].startPeriod = json_object_get_int(startPeriodObj);
                            }

                            struct json_object *limitObj = NULL;
                            if (json_object_object_get_ex(chargingSchedulePeriodObj, "limit", &limitObj))
                            {
                                chargingProfiles.chargingSchedule.chargingSchedulePeriods[i].limit = json_object_get_double(limitObj);
                            }
                            struct json_object *numberPhasesObj = NULL;
                            if (json_object_object_get_ex(chargingSchedulePeriodObj, "numberPhases", &numberPhasesObj))
                            {
                                chargingProfiles.chargingSchedule.chargingSchedulePeriods[i].numberPhases = json_object_get_int(numberPhasesObj);
                            }
                        }
                    }
                }
            }
        }

        if (ocpp_ChargingProfile_insert(chargingProfiles.connectorId, &chargingProfiles) == 0)
        {
            ocpp_package_prepare_Status_Req(uniqueId_str, 0);
        }
        else
        {
            ocpp_package_prepare_Status_Req(uniqueId_str, 1);
        }

        printChargingProfile(&chargingProfiles);
        if (chargingProfiles.chargingSchedule.chargingSchedulePeriods)
        {
            free(chargingProfiles.chargingSchedule.chargingSchedulePeriods);
        }
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
        ocpp_package_prepare_Status_Req(uniqueId_str, 0);
        // ocpp_chargePoint_manageUnlockConnectorRequest(uniqueId_str, unlockConnector_req);
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

        if (retries_obj != NULL)
        {
            updateFirmware_req.retriesIsUse = 1;
            updateFirmware_req.retries = json_object_get_int(retries_obj);
        }

        if (retryInterval_obj != NULL)
        {
            updateFirmware_req.retryIntervalIsUse = 1;
            updateFirmware_req.retryInterval = json_object_get_int(retryInterval_obj);
        }
        // 在需要创建线程的地方
        read_data_lock();
        enum OCPP_PACKAGE_FIRMWARE_STATUS_E UpdateState = ocpp_chargePoint->ocpp_firmwareUpdate_lastUpdateState;
        rwlock_unlock();
        if (UpdateState == OCPP_PACKAGE_FIRMWARE_STATUS_IDLE)
        {
            ocpp_chargePoint_manageUpdateFirmwareRequest(updateFirmware_req);
            ocpp_package_prepare_Status_Req(uniqueId_str, OCPP_PACKAGE_CONFIGURATION_STATUS_ACCEPTED);
        }
        else
        {
            ocpp_package_prepare_Status_Req(uniqueId_str, OCPP_PACKAGE_CONFIGURATION_STATUS_REJECTED);
        }
        break;
    }
    default:
    {
        ocpp_package_CallError_t callError;
        memset(&callError, 0, sizeof(callError));
        callError.errorCode = OCPP_PACKAGE_CALLERROR_ERRORCODE_NOT_IMPLEMENTED;
        char *message = ocpp_package_prepare_CallError(uniqueId_str, &callError);
        enqueueSendMessage(uniqueId_str, message, OCPP_PACKAGE_CALLERROR_ERRORCODE_NOT_IMPLEMENTED);
        if (message)
        {
            free(message);
        }
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
        printf("heartbeat intervalStr = %s\n", intervalStr);
    }
    break;

    case OCPP_PACKAGE_HEARTBEAT:
    {
        if (ocpp_chargePoint_checkHeartbeat_callResultPackage(uniqueId_str, payload_obj))
            return;

        json_object *currentTime_obj = json_object_object_get(payload_obj, "currentTime");
        const char *currentTime_str = json_object_get_string(currentTime_obj);
        ocpp_AuxiliaryTool_setSystemTime(currentTime_str);
        // printf("HEARTBEAT");
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
        ocpp_package_prepare_Status_Req(uniqueId_str, 0);
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
    }
    break;

    case OCPP_PACKAGE_AUTHORIZE:
    {
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
            memcpy(authorize.idTagInfo.expiryDate, expiryDate_str, sizeof(authorize.idTagInfo.expiryDate));
        }

        if (parentIdTag_obj != NULL)
        {
            authorize.idTagInfo.parentIdTagIsUse = 1;
            const char *parentIdTag_str = json_object_get_string(parentIdTag_obj);
            memcpy(authorize.idTagInfo.parentIdTag, parentIdTag_str, OCPP_AUTHORIZATION_IDTAG_LEN);
        }

        char idTag[OCPP_AUTHORIZATION_IDTAG_LEN] = {0};
        // set Authorization result
        for (i = 0; i <= ocpp_chargePoint->numberOfConnector; i++)
        {
            if (strcmp(ocpp_chargePoint->authorizetion[i]->lastUniqueId, uniqueId_str) == 0)
            {
                if (authorize.idTagInfo.AuthorizationStatus == OCPP_LOCAL_AUTHORIZATION_ACCEPTED)
                {
                    memcpy(idTag, ocpp_chargePoint->authorizetion[i]->idTag, OCPP_AUTHORIZATION_IDTAG_LEN);
                    ocpp_chargePoint->RecvAuthorizeResult(i, true);
                }
                else
                {
                    ocpp_chargePoint->RecvAuthorizeResult(i, false);
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

        if (authorize.idTagInfo.expiryDateIsUse)
        {
            memcpy(cache_record->expiryDate, authorize.idTagInfo.expiryDate, sizeof(cache_record->expiryDate));
        }

        if (authorize.idTagInfo.parentIdTagIsUse)
        {
            memcpy(cache_record->parentIdTag, authorize.idTagInfo.parentIdTag, OCPP_AUTHORIZATION_IDTAG_LEN);
        }

        ocpp_localAuthorization_Cache_write(cache_record);

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
        const char *expiryDate_str = NULL;
        const char *parentIdTag_str = NULL;
        int i;
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
            expiryDate_str = json_object_get_string(expiryDate_obj);
            memcpy(startTransaction.idTagInfo.expiryDate, expiryDate_str, 32);
        }

        if (parentIdTag_obj != NULL)
        {
            startTransaction.idTagInfo.parentIdTagIsUse = 1;
            parentIdTag_str = json_object_get_string(parentIdTag_obj);
        }
        ocpp_chargePoint->RecvStartTransactionResult(startTransaction.transactionId, status_str, expiryDate_str, parentIdTag_str,uniqueId_str);
        // 找到正在交易的枪号
        for (i = 1; i <= ocpp_chargePoint->numberOfConnector; i++)
        {
            if (strcmp(ocpp_chargePoint->transaction_obj[i]->lastUniqueId, uniqueId_str) == 0 && ocpp_chargePoint->transaction_obj[i]->isTransaction)
            {
                write_data_lock();
                ocpp_chargePoint->transaction_obj[i]->isRecStartTransaction_Conf = true;
                memcpy(&ocpp_chargePoint->transaction_obj[i]->startTransaction, &startTransaction, sizeof(ocpp_package_StartTransaction_conf_t));
                rwlock_unlock();
                break;
            }
        }

        if (i > ocpp_chargePoint->numberOfConnector)
        {
            return;
        }
        // 更新认证缓存
        ocpp_localAuthorization_cache_record_t *cache_record = (ocpp_localAuthorization_cache_record_t *)calloc(1, sizeof(ocpp_localAuthorization_cache_record_t));

        memcpy(cache_record->IdTag, ocpp_chargePoint->transaction_obj[i]->startIdTag, OCPP_AUTHORIZATION_IDTAG_LEN);
        cache_record->status = startTransaction.idTagInfo.AuthorizationStatus;

        if (startTransaction.idTagInfo.expiryDateIsUse)
        {
            memcpy(cache_record->expiryDate, startTransaction.idTagInfo.expiryDate, 32);
        }

        if (startTransaction.idTagInfo.parentIdTagIsUse)
        {
            memcpy(cache_record->parentIdTag, startTransaction.idTagInfo.parentIdTag, OCPP_AUTHORIZATION_IDTAG_LEN);
        }

        ocpp_localAuthorization_Cache_write(cache_record);

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
        const char *status_str = NULL;
        const char *expiryDate_str = NULL;
        const char *parentIdTag_str = NULL;
        int i = 0;
        if (idTagInfo_obj != NULL)
        {
            stopTransaction.idTagInfoIsUse = 1;
            json_object *status_obj = json_object_object_get(idTagInfo_obj, "status");
            json_object *expiryDate_obj = json_object_object_get(idTagInfo_obj, "expiryDate");
            json_object *parentIdTag_obj = json_object_object_get(idTagInfo_obj, "parentIdTag_obj");

            status_str = json_object_get_string(status_obj);
            for (i = 0; i < OCPP_LOCAL_AUTHORIZATION_MAX; i++)
            {
                if (strcmp(status_str, ocpp_localAuthorizationStatus_Text[i]) == 0)
                    break;
            }

            stopTransaction.idTagInfo.AuthorizationStatus = i;

            if (expiryDate_obj != NULL)
            {
                stopTransaction.idTagInfo.expiryDateIsUse = 1;
                expiryDate_str = json_object_get_string(expiryDate_obj);
                memcpy(stopTransaction.idTagInfo.expiryDate, expiryDate_str, 32);
            }

            if (parentIdTag_obj != NULL)
            {
                stopTransaction.idTagInfo.parentIdTagIsUse = 1;
                parentIdTag_str = json_object_get_string(parentIdTag_obj);
                memcpy(stopTransaction.idTagInfo.parentIdTag, parentIdTag_str, OCPP_AUTHORIZATION_IDTAG_LEN);
            }
        }
        ocpp_chargePoint->RecvStopTransactionResult(status_str, expiryDate_str, parentIdTag_str,uniqueId_str);
        // 找到正在交易的枪号
        for (i = 1; i <= ocpp_chargePoint->numberOfConnector; i++)
        {
            if (strcmp(ocpp_chargePoint->transaction_obj[i]->lastUniqueId, uniqueId_str) == 0 && ocpp_chargePoint->transaction_obj[i]->isTransaction)
            {
                write_data_lock();
                ocpp_chargePoint->transaction_obj[i]->isRecStopTransaction_Conf = true;
                memcpy(&ocpp_chargePoint->transaction_obj[i]->stopTransaction, &stopTransaction, sizeof(ocpp_package_StopTransaction_conf_t));
                rwlock_unlock();
                break;
            }
        }

        if (i > ocpp_chargePoint->numberOfConnector)
        {
            return;
        }
        ocpp_localAuthorization_cache_record_t *cache_record = (ocpp_localAuthorization_cache_record_t *)calloc(1, sizeof(ocpp_localAuthorization_cache_record_t));

        memcpy(cache_record->IdTag, ocpp_chargePoint->transaction_obj[i]->stopIdTag, OCPP_AUTHORIZATION_IDTAG_LEN);
        cache_record->status = stopTransaction.idTagInfo.AuthorizationStatus;

        if (stopTransaction.idTagInfo.expiryDateIsUse)
        {
            memcpy(cache_record->expiryDate, stopTransaction.idTagInfo.expiryDate, 32);
        }

        if (stopTransaction.idTagInfo.parentIdTagIsUse)
        {
            memcpy(cache_record->parentIdTag, stopTransaction.idTagInfo.parentIdTag, OCPP_AUTHORIZATION_IDTAG_LEN);
        }

        ocpp_localAuthorization_Cache_write(cache_record);

        free(cache_record);
    }
    break;

    default:
        printf("action error\n");
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
        break;

    case OCPP_PACKAGE_STARTTRANSACTION:
    case OCPP_PACKAGE_STOPTRANSACTION:
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
        break;
    case OCPP_PACKAGE_METERVALUES:
        break;
    case OCPP_PACKAGE_STARTTRANSACTION:
        break;
    case OCPP_PACKAGE_STOPTRANSACTION:
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

    if (IsRecvMessage())
    {
        char Unique[MESSAGE_UNIQUE_LEN];
        char message[MESSAGE_CONTENT_LEN];
        int messageTypeId;
        int action;
        if (dequeueRecvMessage(Unique, message, &messageTypeId, &action))
        {
            enum json_tokener_error err;
            json_object *jobj = json_tokener_parse_verbose(message, &err);

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
                        ocpp_chargePoint_CallResult_Handler(jobj, action);
                        break;

                    case OCPP_PACKAGE_CALL_ERROR:
                        ocpp_chargePoint_CallError_Handler(jobj, action);
                        break;
                    }
                }
                json_object_put(jobj);
            }
        }
    }
}
// 函数用于检查日期和时间是否已经过期
int isExpired(const char *expiryDate)
{
    // 获取当前时间
    time_t currentTime;
    time(&currentTime);

    // 将日期和时间字符串转换为tm结构体
    struct tm expiryTime;
    memset(&expiryTime, 0, sizeof(struct tm));

    if (strptime(expiryDate, "%Y-%m-%dT%H:%M:%S", &expiryTime) == NULL)
    {
        // 日期时间解析失败
        return 1; // 假设未过期
    }

    // 将tm结构体转换为时间戳
    time_t expiryTimestamp = mktime(&expiryTime);

    // 比较当前时间和过期时间
    return (expiryTimestamp <= currentTime);
}
/**
 * @description:
 * @param
 * @return
 */
void *ocpp_chargePoint_client(void *arg)
{
    printf("create client thread\n");
    int reserveTime_ms = 0; // 检查预约是否过期时间

    // 创建boot线程
    pthread_t tid_boot;
    if (pthread_create(&tid_boot, NULL, ocpp_chargePoint_boot, NULL) != 0)
    {
        printf("cann't create boot thread %s\n", strerror(errno));
    }

    pthread_detach(tid_boot);
    int connector = 0;
    while (1)
    {

        ocpp_chargePoint_Recv_Handler();

        // disconnect reset flag
        if (ocpp_chargePoint->connect.isConnect == false)
        {
            ocpp_chargePoint->RegistrationStatus = OCPP_PACKAGE_REGISTRATION_STATUS_MAX;
        }
        // 版本二
        read_data_lock();
        for (connector = 0; connector < ocpp_chargePoint->numberOfConnector; connector++)
        {
            if (ocpp_chargePoint->connector[0]->status == OCPP_PACKAGE_CHARGEPOINT_STATUS_AVAILABLE)
            {
                if (!ocpp_chargePoint->transaction_obj[connector + 1]->isTransaction && ocpp_chargePoint->transaction_obj[connector + 1]->isStart)
                {
                    bool isAllowTransaction = false;
                    if (ocpp_chargePoint->connector[connector + 1]->status == OCPP_PACKAGE_CHARGEPOINT_STATUS_PREPARING)
                    {
                        isAllowTransaction = true;
                    }
                    if (isAllowTransaction)
                    {
                        pthread_t tid_transaction;
                        if (pthread_create(&tid_transaction, NULL, ocpp_chargePoint_Transaction_thread, (void *)(connector + 1)) == -1)
                        {
                            printf("create transaction thread fail\n");
                        }
                        pthread_detach(tid_transaction);
                    }
                }
            }
        }
        rwlock_unlock();
        usleep(1000 * 1000);
    } // for_end
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

    if (ocpp_chargePoint->userPushStartButton == NULL)
        ocpp_chargePoint->userPushStartButton = ocpp_chargePoint_userPushStartButton;

    if (ocpp_chargePoint->userPushStopButton == NULL)
        ocpp_chargePoint->userPushStopButton = ocpp_chargePoint_userPushStopButton;

    if (ocpp_chargePoint->startCharging == NULL)
        ocpp_chargePoint->startCharging = ocpp_chargePoint_Chargingint;

    if (ocpp_chargePoint->sendAuthorization == NULL)
        ocpp_chargePoint->sendAuthorization = ocpp_chargePoint_Authorization_IdTag;
        
    if (ocpp_chargePoint->remoteStopCharging == NULL)
        ocpp_chargePoint->remoteStopCharging = ocpp_chargePoint_setDefault;

    if (ocpp_chargePoint->sendStartTransaction == NULL)
        ocpp_chargePoint->sendStartTransaction = ocpp_chargePoint_sendStartTransaction;

    if (ocpp_chargePoint->sendStopTransaction == NULL)
        ocpp_chargePoint->sendStopTransaction = ocpp_transaction_sendStopTransaction;

    if (ocpp_chargePoint->getOcppStatus == NULL)
        ocpp_chargePoint->getOcppStatus = ocpp_chargePoint_getConnectStatus;

    if (ocpp_chargePoint->setOcppStatus == NULL)
        ocpp_chargePoint->setOcppStatus = ocpp_chargePoint_setConnectStatus;

}

/**
 * @description:
 * @param {ocpp_chargePoint_t} *ocpp_chargePoint
 * @return {*}
 */
int ocpp_chargePoint_init(ocpp_chargePoint_t *pile)
{
    if (pile == NULL)
    {
        return -1;
    }
    ocpp_chargePoint = pile;

    /////////////////-test_debug-////////////////////
    printf("pile = %p  point = %p\n", pile, ocpp_chargePoint);
    printf("isWss = %d\n", ocpp_chargePoint->connect.isWss);
    printf("ssl_ca_filepath = %s\n", ocpp_chargePoint->connect.ssl_ca_filepath);
    printf("protocolName = %s\n", ocpp_chargePoint->connect.protocolName);
    printf("username = %s\n", ocpp_chargePoint->connect.username);
    printf("password = %s\n", ocpp_chargePoint->connect.password);
    printf("address = %s\n", ocpp_chargePoint->connect.address);
    printf("port = %d\n", ocpp_chargePoint->connect.port);
    printf("path = %s\n", ocpp_chargePoint->connect.path);

    ////////////////////////////////////////////////
    initialize_rwlock();
    ocpp_chargePoint_defaultFunc();
    ocpp_chargePoint->userPushStartButton = ocpp_chargePoint_userPushStartButton;
    ocpp_chargePoint->userPushStopButton = ocpp_chargePoint_userPushStopButton;

    ocpp_chargePoint->RegistrationStatus = OCPP_PACKAGE_REGISTRATION_STATUS_REJECTED;
    ocpp_chargePoint->setConnectorStatus = setConnectorStatus;
    ocpp_chargePoint->setConnectorErrInfo = setConnectorErrInfo;

    int rc;
    // 打开数据库文件或创建并打开
    rc = sqlite3_open_v2(OCPP_DATABASE_FILE_NAME, &(ocpp_chargePoint->ocpp_db), SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);
    if (rc != SQLITE_OK)
    {
        printf("无法创建或打开OCPP数据库\n");
        return -1;
    }

    // 初始化OCPP配置
    if (ocpp_ConfigurationKey_init(ocpp_chargePoint->ocpp_db) == -1)
    {
        printf("ocpp ConfigurationKey init fail\n");
        return -1;
    }

    // 初始化OCPP本地认证
    ocpp_localAuthorization_init(ocpp_chargePoint->ocpp_db);
    ocpp_ChargingProfile_init(ocpp_chargePoint->ocpp_db);

    // 初始化 Reserve
    ocpp_chargePoint->reserveConnector = (ocpp_reserve_t **)calloc(ocpp_chargePoint->numberOfConnector + 1, sizeof(ocpp_reserve_t *));
    int i = 0;
    for (i = 0; i <= ocpp_chargePoint->numberOfConnector; i++)
    {
        ocpp_chargePoint->reserveConnector[i] = (ocpp_reserve_t *)calloc(1, sizeof(ocpp_reserve_t));
    }

    // 初始化认证状态
    ocpp_chargePoint->authorizetion = (ocpp_chargePoint_Authoriza_t **)calloc(ocpp_chargePoint->numberOfConnector + 1, sizeof(ocpp_chargePoint_Authoriza_t *));
    for (i = 0; i <= ocpp_chargePoint->numberOfConnector; i++)
    {
        ocpp_chargePoint->authorizetion[i] = (ocpp_chargePoint_Authoriza_t *)calloc(1, sizeof(ocpp_chargePoint_Authoriza_t));
        memset(ocpp_chargePoint->authorizetion[i], 0, sizeof(ocpp_chargePoint_Authoriza_t));
    }

    // 初始化交易对象
    ocpp_chargePoint->transaction_obj = (ocpp_chargePoint_transaction_t **)calloc(ocpp_chargePoint->numberOfConnector + 1, sizeof(ocpp_chargePoint_transaction_t *));
    for (i = 0; i <= ocpp_chargePoint->numberOfConnector; i++)
    {
        ocpp_chargePoint->transaction_obj[i] = (ocpp_chargePoint_transaction_t *)calloc(1, sizeof(ocpp_chargePoint_transaction_t));
        memset(ocpp_chargePoint->transaction_obj[i], 0, sizeof(ocpp_chargePoint_transaction_t));
    }

    // base number of connector create object , connector = 0 indicate pile not is connector
    printf("numberOfConnector = %d\n", ocpp_chargePoint->numberOfConnector);
    ocpp_chargePoint->connector = (ocpp_package_StatusNotification_req_t **)calloc(ocpp_chargePoint->numberOfConnector + 1, sizeof(ocpp_package_StatusNotification_req_t *));
    for (i = 0; i <= ocpp_chargePoint->numberOfConnector; i++)
    {
        ocpp_chargePoint->connector[i] = (ocpp_package_StatusNotification_req_t *)calloc(1, sizeof(ocpp_package_StatusNotification_req_t));
        ocpp_chargePoint->connector[i]->errorCode = OCPP_PACKAGE_CHARGEPOINT_ERRORCODE_NOERROR;
        ocpp_chargePoint->connector[i]->status = OCPP_PACKAGE_CHARGEPOINT_STATUS_AVAILABLE;
    }

    //////////////////////////////////////
    ocpp_connect_init(&(ocpp_chargePoint->connect));
    ocpp_list_init(&(ocpp_chargePoint->connect));
    ocpp_chargePoint->ocpp_firmwareUpdate_lastUpdateState = OCPP_PACKAGE_FIRMWARE_STATUS_IDLE;
    ocpp_chargePoint->ocpp_diagnostics_lastDiagnosticsStatus = OCPP_PACKAGE_DIAGNOSTICS_STATUS_IDLE;
    // 创建客户端线程
    pthread_t tid_client;
    if (pthread_create(&tid_client, NULL, ocpp_chargePoint_client, NULL) != 0)
    {
        printf("cann't create client thread %s\n", strerror(errno));
        return -1;
    }
    pthread_detach(tid_client);

    printf("initial end\n");

    return 0;
}
