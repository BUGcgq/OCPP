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
     OCPP_CK_AllowOfflineTxForUnknownId = 0,//如果此 key 存在，则充电点支持 Unknown Offline Authorization。如果此项报告的值为 true，则启用Unknown Offline Authorization
     OCPP_CK_AuthorizationCacheEnabled,//如果此 key 存在，则充电点支持 Authorization Cache。如果此项报告的值为 true，则启用 Authorization Cache。
     OCPP_CK_AuthorizeRemoteTxRequests,//是否应事先授权以 RemoteStartTransaction.req 消息的形式启动交易的远程请求，就像本地操作启动交易一样。
     OCPP_CK_BlinkRepeat,//发出信号时充电点照明闪烁的次数
     OCPP_CK_ClockAlignedDataInterval,//时钟对齐数据间隔的大小（以秒为单位）
     OCPP_CK_ConnectionTimeOut,//将充电枪插入车辆，否则自动取消交易的超时时间
     OCPP_CK_GetConfigurationMaxKeys,//ConnectorPhaseRotation 配置键中的最大项目数
     OCPP_CK_HeartbeatInterval,//与中央系统处于非活动状态（无 OCPP 数据交换）的时间间隔，在此时间间隔之后，充电点应发送 Heartbeat.req PDU
     OCPP_CK_LightIntensity,//充电点照明的亮度
     OCPP_CK_LocalAuthorizeOffline,//充电点在 offline（离线） 时是否会启动本地授权标识符的交易
     OCPP_CK_LocalPreAuthorize,//充电点在 online（联机）时是否会启动本地授权标识符的交易，而无需等待来自中央系统的 Authorize.conf 授权。
     OCPP_CK_MaxEnergyOnInvalidId,//当标识符在交易开始后，中央系统失效时，以Wh 为单位的最大能量。
     OCPP_CK_MeterValuesAlignedData,//当标识符在交易开始后，中央系统失效时，以Wh 为单位的最大能量。
     OCPP_CK_MeterValuesAlignedDataMaxLength,//MeterValuesAlignedData 配置键中的最大项目数。
     OCPP_CK_MeterValuesSampledData,//默认值 ："Energy.Active.Import.Register"
     OCPP_CK_MeterValuesSampledDataMaxLength,//
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
        "MeterValueSampleInterval",
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
    char key[64];
    bool isUsed;                                                
    enum OCPP_ACCESSIBILITY_TYPE_E accessibility; 
    enum OCPP_CONFIGURATION_KEY_DATA_TYPE_E dataType;
    union{
        bool boolData;
        int  intData;
        char stringData[OCPP_CONFIGURATION_VALUES_STRING_MAX];;
    }type;
    
}OCPP_ConfigurationKey_t;

bool ocpp_ConfigurationKey_isFound(const char *key);
int ocpp_ConfigurationKey_getValue(const char *key, void *value);
int ocpp_ConfigurationKey_getIsUse(const char *key);
int ocpp_ConfigurationKey_getAcc(const char *key);
int ocpp_ConfigurationKey_getType(const char *key);
int ocpp_ConfigurationKey_Modify(const char *key, const char *value, int isUse);
int ocpp_ConfigurationKey_init(sqlite3 *ocpp_db);
void ocpp_ConfigurationKey_deinit(void);


#ifdef __cplusplus
}
#endif

#endif