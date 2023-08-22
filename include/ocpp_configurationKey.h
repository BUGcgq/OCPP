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

enum OCPP_CONFIGURATION_KEY_E {
    // Core Profile
    OCPP_CK_AllowOfflineTxForUnknownId = 0,               // 是否允许未知用户离线充电
    OCPP_CK_AuthorizationCacheEnabled,                    // 是否启用授权缓存
    OCPP_CK_AuthorizeRemoteTxRequests,                    // 是否允许远程授权充电请求
    OCPP_CK_BlinkRepeat,                                  // 充电指示灯闪烁次数
    OCPP_CK_ClockAlignedDataInterval,                     // 对齐数据间隔
    OCPP_CK_ConnectionTimeOut,                            // 连接超时时间
    OCPP_CK_GetConfigurationMaxKeys,                      // 获取配置的最大键数
    OCPP_CK_HeartbeatInterval,                            // 心跳间隔
    OCPP_CK_LightIntensity,                               // 灯光强度
    OCPP_CK_LocalAuthorizeOffline,                        // 离线本地授权
    OCPP_CK_LocalPreAuthorize,                            // 本地预授权
    OCPP_CK_MaxEnergyOnInvalidId,                         // 无效 ID 时的最大能量
    OCPP_CK_MeterValuesAlignedData,                       // 对齐的电表值数据
    OCPP_CK_MeterValuesAlignedDataMaxLength,              // 对齐的电表值数据的最大长度
    OCPP_CK_MeterValuesSampledData,                       // 采样的电表值数据
    OCPP_CK_MeterValuesSampledDataMaxLength,              // 采样的电表值数据的最大长度
    OCPP_CK_MeterValueSampleInterval,                     // 电表值采样间隔
    OCPP_CK_MinimumStatusDuration,                        // 最小状态持续时间
    OCPP_CK_NumberOfConnectors,                           // 连接器数量
    OCPP_CK_ResetRetries,                                 // 重置尝试次数
    OCPP_CK_ConnectorPhaseRotation,                       // 连接器相位旋转
    OCPP_CK_ConnectorPhaseRotationMaxLength,              // 连接器相位旋转的最大长度
    OCPP_CK_StopTransactionOnEVSideDisconnect,            // 在电动汽车侧断开连接时停止充电事务
    OCPP_CK_StopTransactionOnInvalidId,                   // 在无效 ID 时停止充电事务
    OCPP_CK_StopTxnAlignedData,                           // 停止充电事务的对齐数据
    OCPP_CK_StopTxnAlignedDataMaxLength,                  // 停止充电事务的对齐数据的最大长度
    OCPP_CK_StopTxnSampledData,                           // 停止充电事务的采样数据
    OCPP_CK_StopTxnSampledDataMaxLength,                  // 停止充电事务的采样数据的最大长度
    OCPP_CK_SupportedFeatureProfiles,                     // 支持的功能配置文件
    OCPP_CK_SupportedFeatureProfilesMaxLength,            // 支持的功能配置文件的最大长度
    OCPP_CK_TransactionMessageAttempt,                    // 事务消息尝试次数
    OCPP_CK_TransactionMessageRetryInterval,              // 事务消息重试间隔
    OCPP_CK_UnlockConnectorOnEVSideDisconnect,            // 在电动汽车侧断开连接时解锁连接器
    OCPP_CK_WebSocketPingInterval,                        // WebSocket 心跳间隔
    
    // Local Auth List Management Profile
    OCPP_CK_LocalAuthListEnabled,                         // 是否启用本地身份验证列表
    OCPP_CK_LocalAuthListMaxLength,                       // 本地身份验证列表的最大长度
    OCPP_CK_SendLocalListMaxLength,                       // 发送本地列表的最大长度
    
    // Reservation Profile
    OCPP_CK_ReserveConnectorZeroSupported,                // 是否支持预留编号为零的连接器
    
    // Smart Charging Profile
    OCPP_CK_ChargeProfileMaxStackLevel,                   // 充电配置文件最大堆叠层数
    OCPP_CK_ChargingScheduleAllowedChargingRateUnit,      // 充电计划允许的充电速率单位
    OCPP_CK_ChargingScheduleMaxPeriods,                   // 充电计划允许的最大充电时段数
    OCPP_CK_ConnectorSwitch3to1PhaseSupported,            // 是否支持将连接器从三相切换为单相
    OCPP_CK_MaxChargingProfilesInstalled,                 // 支持的最大充电配置文件安装数量
    
    OCPP_CK_MAXLEN                                        // 配置项的数量
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