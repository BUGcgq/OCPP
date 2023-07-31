/*
 * @Author: LIYAOHAN 1791002655@qq.com
 * @Date: 2023-04-01 15:46:16
 * @LastEditors: LIYAOHAN 1791002655@qq.com
 * @LastEditTime: 2023-04-25 21:53:22
 * @FilePath: /OCPP/ocpp_configuration.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __OCPP_CONFIGURATION_KEY__H__
#define __OCPP_CONFIGURATION_KEY__H__

#ifdef __cplusplus
extern "C" {
#endif
#include<stdbool.h>
#include<sqlite3.h>

#define OCPP_CONFIGURATION_KEY_SIZE                75
#define OCPP_CONFIGURATION_VALUES_STRING_MAX       512




enum OCPP_ACCESSIBILITY_TYPE_E{
    OCPP_ACC_READONLY = 0,
    OCPP_ACC_WRITEONLY,
    OCPP_ACC_READWRITE,
    OCPP_ACC_NONE,
    OCPP_ACC_MAX
};

static const char * ocpp_AccessibilityText[OCPP_ACC_MAX] = {
    "READONLY",
    "WRITEONLY",
    "READWRITE",
    "NONE"
};

enum OCPP_CONFIGURATION_KEY_DATA_TYPE_E{
    OCPP_CK_DT_BOOLEAN,
    OCPP_CK_DT_INTEGER,
    OCPP_CK_DT_STRING,
    OCPP_CK_DT_NONE
};

enum OCPP_CONFIGURATION_KEY_E{
    //Core Profile
     OCPP_CK_AllowOfflineTxForUnknownId = 0,
     OCPP_CK_AuthorizationCacheEnabled,
     OCPP_CK_AuthorizeRemoteTxRequests,
     OCPP_CK_BlinkRepeat,
     OCPP_CK_ClockAlignedDataInterval,
     OCPP_CK_ConnectionTimeOut,
     OCPP_CK_GetConfigurationMaxKeys,
     OCPP_CK_HeartbeatInterval,
     OCPP_CK_LightIntensity,
     OCPP_CK_LocalAuthorizeOffline,
     OCPP_CK_LocalPreAuthorize,
     OCPP_CK_MaxEnergyOnInvalidId,
     OCPP_CK_MeterValuesAlignedData,
     OCPP_CK_MeterValuesAlignedDataMaxLength,
     OCPP_CK_MeterValuesSampledData,
     OCPP_CK_MeterValuesSampledDataMaxLength,
     OCPP_CK_MeterValueSampleInterval,
     OCPP_CK_MinimumStatusDuration,
     OCPP_CK_NumberOfConnectors,
     OCPP_CK_ResetRetries,
     OCPP_CK_ConnectorPhaseRotation,
     OCPP_CK_ConnectorPhaseRotationMaxLength,
     OCPP_CK_StopTransactionOnEVSideDisconnect,
     OCPP_CK_StopTransactionOnInvalidId,
     OCPP_CK_StopTxnAlignedData,
     OCPP_CK_StopTxnAlignedDataMaxLength,
     OCPP_CK_StopTxnSampledData,
     OCPP_CK_StopTxnSampledDataMaxLength,
     OCPP_CK_SupportedFeatureProfiles,
     OCPP_CK_SupportedFeatureProfilesMaxLength,
     OCPP_CK_TransactionMessageAttempt,
     OCPP_CK_TransactionMessageRetryInterval,
     OCPP_CK_UnlockConnectorOnEVSideDisconnect,
     OCPP_CK_WebSocketPingInterval,
     //Local Auth List Management Profile
     OCPP_CK_LocalAuthListEnabled,
     OCPP_CK_LocalAuthListMaxLength,
     OCPP_CK_SendLocalListMaxLength,
     //Reservation Profile
     OCPP_CK_ReserveConnectorZeroSupported,
     // Smart Charging Profil
     OCPP_CK_ChargeProfileMaxStackLevel,
     OCPP_CK_ChargingScheduleAllowedChargingRateUnit,
     OCPP_CK_ChargingScheduleMaxPeriods,
     OCPP_CK_ConnectorSwitch3to1PhaseSupported,
     OCPP_CK_MaxChargingProfilesInstalled,
     OCPP_CK_MAXLEN
};

static const char *ocpp_configurationKeyText[OCPP_CONFIGURATION_KEY_SIZE]={
		//9.1 CORE PROFILE
		"AllowOfflineTxForUnknownId",
        "AuthorizationCacheEnabled",
        "AuthorizeRemoteTxRequests",
        "BlinkRepeat",
        "ClockAlignedDataInterval",
		"ConnectionTimeOut",
        "GetConfigurationMaxKeys",
        "HeartbeatInterval",
        "LightIntensity",
        "LocalAuthorizeOffline",
        "LocalPreAuthorize",
        "MaxEnergyOnInvalidId",
		//电表值相关
		"MeterValuesAlignedData", 
        "MeterValuesAlignedDataMaxLength", 
        "MeterValuesSampledData", 
        "MeterValuesSampledDataMaxLength", 
        "MeterValuesSampleInterval",

		"MinimumStatusDuration",
		"NumberOfConnectors",
        "ResetRetries",
        "ConnectorPhaseRotation",
        "ConnectorPhaseRotationMaxLength",
        "StopTransactionOnEVSideDisconnect",
        "StopTransactionOnInvalidId",
		"StopTxnAlignedData",
        "StopTxnAlignedDataMaxLength",
        "StopTxnSampledData",
        "StopTxnSampledDataMaxLength",
        "SupportedFeatureProfiles",
        "SupportedFeatureProfilesMaxLength",
		"TransactionMessageAttempts",
        "TransactionMessageRetryInterval",
        "UnlockConnectorOnEVSideDisconnect",
        "WebSocketPingInterval",
		//9.2 LOCAL AUTH LIST MANAGEMENT PROFILE //本地身份验证列表管理配置文件
		"LocalAuthListEnabled",
        "LocalAuthListMaxLength",
        "SendLocalListMaxLength",
		//9.3 RESERVATION PROFILE //9.3预订简介
		"ReserveConnectorZeroSupported",
		//9.4 SMART CHARGING PROFILE//9.4智能充电模式
		"ChargeProfileMaxStackLevel",
        "ChargingScheduleAllowedChargingRateUnit",
        "ChargingScheduleMaxPeriods",
        "ConnectorSwitch3to1PhaseSupported",
        "MaxChargingProfilesInstalled"
};

typedef struct{
    bool isUsed;                                                
    enum OCPP_ACCESSIBILITY_TYPE_E accessibility; 
    enum OCPP_CONFIGURATION_KEY_DATA_TYPE_E dataType;
    union{
        bool boolData;
        int  intData;
        char * stringData;
    }type;
    
}OCPP_ConfigurationKey_t;



static OCPP_ConfigurationKey_t OCPP_ConfigurationKey[OCPP_CONFIGURATION_KEY_SIZE];


bool ocpp_ConfigurationKey_isFound(const char * key);
bool ocpp_ConfigurationKey_isUse(const char * key);
enum OCPP_ACCESSIBILITY_TYPE_E ocpp_ConfigurationKey_getAcc(const char * key);
enum OCPP_CONFIGURATION_KEY_DATA_TYPE_E ocpp_ConfigurationKey_getType(const char * key);
enum OCPP_CONFIGURATION_KEY_DATA_TYPE_E ocpp_ConfigurationKey_getValue(const char * key,void * value);
int ocpp_ConfigurationKey_Modify(const char * key,char * value,int isUse);
int ocpp_ConfigurationKey_init(sqlite3 *ocpp_db);
void ocpp_ConfigurationKey_deinit(void);



#ifdef __cplusplus
}
#endif

#endif