/*
 * @Author: LIYAOHAN 1791002655@qq.com
 * @Date: 2023-04-01 15:46:34
 * @LastEditors: LIYAOHAN 1791002655@qq.com
 * @LastEditTime: 2023-05-05 19:05:24
 * @FilePath: /OCPP/ocpp_configuration.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */


#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

#include "ocpp_configurationKey.h"
#include "ocpp_config.h"
#include "ocpp_log.h"

static pthread_rwlock_t ocpp_ConfigurationKey_rwlock;
static sqlite3 *ocpp_CK;

/**
 * @description: 
 * @return {*}
 */
static void ocpp_ConfiguartionKey_default_init(void){
    int stringLen = 0;

    //9.1.1. AllowOfflineTxForUnknownId
    OCPP_ConfigurationKey[OCPP_CK_AllowOfflineTxForUnknownId].isUsed = false;
    OCPP_ConfigurationKey[OCPP_CK_AllowOfflineTxForUnknownId].accessibility = OCPP_ACC_READWRITE;
    OCPP_ConfigurationKey[OCPP_CK_AllowOfflineTxForUnknownId].dataType = OCPP_CK_DT_BOOLEAN;
    OCPP_ConfigurationKey[OCPP_CK_AllowOfflineTxForUnknownId].type.boolData = false;

    //9.1.2. AuthorizationCacheEnable
    OCPP_ConfigurationKey[OCPP_CK_AuthorizationCacheEnabled].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_AuthorizationCacheEnabled].accessibility = OCPP_ACC_READWRITE;
    OCPP_ConfigurationKey[OCPP_CK_AuthorizationCacheEnabled].dataType = OCPP_CK_DT_BOOLEAN;
    OCPP_ConfigurationKey[OCPP_CK_AuthorizationCacheEnabled].type.boolData = true;

    //9.1.3. AuthorizeRemoteTxRequests
    //
    OCPP_ConfigurationKey[OCPP_CK_AuthorizeRemoteTxRequests].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_AuthorizeRemoteTxRequests].accessibility = OCPP_ACC_READWRITE;
    OCPP_ConfigurationKey[OCPP_CK_AuthorizeRemoteTxRequests].dataType = OCPP_CK_DT_BOOLEAN;
    OCPP_ConfigurationKey[OCPP_CK_AuthorizeRemoteTxRequests].type.boolData = true;

    //9.1.4. BlinkRepeat
    OCPP_ConfigurationKey[OCPP_CK_BlinkRepeat].isUsed = false;
    OCPP_ConfigurationKey[OCPP_CK_BlinkRepeat].accessibility = OCPP_ACC_READWRITE;
    OCPP_ConfigurationKey[OCPP_CK_BlinkRepeat].dataType = OCPP_CK_DT_INTEGER;
    OCPP_ConfigurationKey[OCPP_CK_BlinkRepeat].type.intData = 0;

    //9.1.5. ClockAlignedDataInterval
    OCPP_ConfigurationKey[OCPP_CK_ClockAlignedDataInterval].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_ClockAlignedDataInterval].accessibility = OCPP_ACC_READWRITE;
    OCPP_ConfigurationKey[OCPP_CK_ClockAlignedDataInterval].dataType = OCPP_CK_DT_INTEGER;
    OCPP_ConfigurationKey[OCPP_CK_ClockAlignedDataInterval].type.intData = 0;

    //9.1.6. ConnectionTimeOut
    OCPP_ConfigurationKey[OCPP_CK_ConnectionTimeOut].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_ConnectionTimeOut].accessibility = OCPP_ACC_READWRITE;
    OCPP_ConfigurationKey[OCPP_CK_ConnectionTimeOut].dataType = OCPP_CK_DT_INTEGER;
    OCPP_ConfigurationKey[OCPP_CK_ConnectionTimeOut].type.intData = 0;

    //9.1.7. GetConfigurationMaxKeys
    OCPP_ConfigurationKey[OCPP_CK_GetConfigurationMaxKeys].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_GetConfigurationMaxKeys].accessibility = OCPP_ACC_READONLY;
    OCPP_ConfigurationKey[OCPP_CK_GetConfigurationMaxKeys].dataType = OCPP_CK_DT_INTEGER;
    OCPP_ConfigurationKey[OCPP_CK_GetConfigurationMaxKeys].type.intData = 5;

    //9.1.8. HeartbeatInterva
    OCPP_ConfigurationKey[OCPP_CK_HeartbeatInterval].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_HeartbeatInterval].accessibility = OCPP_ACC_READWRITE;
    OCPP_ConfigurationKey[OCPP_CK_HeartbeatInterval].dataType = OCPP_CK_DT_INTEGER;
    OCPP_ConfigurationKey[OCPP_CK_HeartbeatInterval].type.intData = 30;

    //9.1.9. LightIntensity
    OCPP_ConfigurationKey[OCPP_CK_LightIntensity].isUsed = false;
    OCPP_ConfigurationKey[OCPP_CK_LightIntensity].accessibility = OCPP_ACC_READWRITE;
    OCPP_ConfigurationKey[OCPP_CK_LightIntensity].dataType = OCPP_CK_DT_INTEGER;
    OCPP_ConfigurationKey[OCPP_CK_LightIntensity].type.intData = 0;

    //9.1.10. LocalAuthorizeOffline
    OCPP_ConfigurationKey[OCPP_CK_LocalAuthorizeOffline].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_LocalAuthorizeOffline].accessibility = OCPP_ACC_READWRITE;
    OCPP_ConfigurationKey[OCPP_CK_LocalAuthorizeOffline].dataType = OCPP_CK_DT_BOOLEAN;
    OCPP_ConfigurationKey[OCPP_CK_LocalAuthorizeOffline].type.boolData = true;

    //9.1.11. LocalPreAuthorize
    OCPP_ConfigurationKey[OCPP_CK_LocalPreAuthorize].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_LocalPreAuthorize].accessibility = OCPP_ACC_READWRITE;
    OCPP_ConfigurationKey[OCPP_CK_LocalPreAuthorize].dataType = OCPP_CK_DT_BOOLEAN;
    OCPP_ConfigurationKey[OCPP_CK_LocalPreAuthorize].type.boolData = false;

    //9.1.12. MaxEnergyOnInvalidId
    OCPP_ConfigurationKey[OCPP_CK_MaxEnergyOnInvalidId].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_MaxEnergyOnInvalidId].accessibility = OCPP_ACC_READWRITE;
    OCPP_ConfigurationKey[OCPP_CK_MaxEnergyOnInvalidId].dataType = OCPP_CK_DT_INTEGER;
    OCPP_ConfigurationKey[OCPP_CK_MaxEnergyOnInvalidId].type.intData = 0;

    //9.1.13. MeterValuesAlignedData
    OCPP_ConfigurationKey[OCPP_CK_MeterValuesAlignedData].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_MeterValuesAlignedData].accessibility = OCPP_ACC_READWRITE;
    OCPP_ConfigurationKey[OCPP_CK_MeterValuesAlignedData].dataType = OCPP_CK_DT_STRING;
    OCPP_ConfigurationKey[OCPP_CK_MeterValuesAlignedData].type.stringData = (char *)calloc(1, OCPP_CONFIGURATION_VALUES_STRING_MAX);
    stringLen = strlen("Voltage:Current.Import:Energy.Active.Import.Register:SoC:Current.Offered:Power.Active.Import:Energy.Reactive.Import.Register");
    strncpy(OCPP_ConfigurationKey[OCPP_CK_MeterValuesAlignedData].type.stringData,
            "Voltage:Current.Import:Energy.Active.Import.Register:SoC:Current.Offered:Power.Active.Import:Energy.Reactive.Import.Register",
            (stringLen > OCPP_CONFIGURATION_VALUES_STRING_MAX) ? OCPP_CONFIGURATION_VALUES_STRING_MAX : stringLen);
  
    //9.1.14. MeterValuesAlignedDataMaxLength
    OCPP_ConfigurationKey[OCPP_CK_MeterValuesAlignedDataMaxLength].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_MeterValuesAlignedDataMaxLength].accessibility = OCPP_ACC_READONLY;
    OCPP_ConfigurationKey[OCPP_CK_MeterValuesAlignedDataMaxLength].dataType = OCPP_CK_DT_INTEGER;
    OCPP_ConfigurationKey[OCPP_CK_MeterValuesAlignedDataMaxLength].type.intData = 10;

    //9.1.15. MeterValuesSampledData
    OCPP_ConfigurationKey[OCPP_CK_MeterValuesSampledData].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_MeterValuesSampledData].accessibility = OCPP_ACC_READWRITE;
    OCPP_ConfigurationKey[OCPP_CK_MeterValuesSampledData].dataType = OCPP_CK_DT_STRING;
    OCPP_ConfigurationKey[OCPP_CK_MeterValuesSampledData].type.stringData = (char *)calloc(1, OCPP_CONFIGURATION_VALUES_STRING_MAX);
    stringLen = strlen("Voltage:Current.Import:Energy.Active.Import.Register:SoC:Current.Offered:Power.Active.Import:Energy.Reactive.Import.Register");
    strncpy(OCPP_ConfigurationKey[OCPP_CK_MeterValuesSampledData].type.stringData,
            "Voltage:Current.Import:Energy.Active.Import.Register:SoC:Current.Offered:Power.Active.Import:Energy.Reactive.Import.Register",
            (stringLen > OCPP_CONFIGURATION_VALUES_STRING_MAX) ? OCPP_CONFIGURATION_VALUES_STRING_MAX : stringLen);

    //9.1.16. MeterValuesSampledDataMaxLen
    OCPP_ConfigurationKey[OCPP_CK_MeterValuesSampledDataMaxLength].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_MeterValuesSampledDataMaxLength].accessibility = OCPP_ACC_READONLY;
    OCPP_ConfigurationKey[OCPP_CK_MeterValuesSampledDataMaxLength].dataType = OCPP_CK_DT_INTEGER;
    OCPP_ConfigurationKey[OCPP_CK_MeterValuesSampledDataMaxLength].type.intData = 10;

    //9.1.17. MeterValueSampleInterval
    OCPP_ConfigurationKey[OCPP_CK_MeterValueSampleInterval].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_MeterValueSampleInterval].accessibility = OCPP_ACC_READWRITE;
    OCPP_ConfigurationKey[OCPP_CK_MeterValueSampleInterval].dataType = OCPP_CK_DT_INTEGER;
    OCPP_ConfigurationKey[OCPP_CK_MeterValueSampleInterval].type.intData = 5;

    //9.1.18. MinimumStatusDuration
    OCPP_ConfigurationKey[OCPP_CK_MinimumStatusDuration].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_MinimumStatusDuration].accessibility = OCPP_ACC_READWRITE;
    OCPP_ConfigurationKey[OCPP_CK_MinimumStatusDuration].dataType = OCPP_CK_DT_INTEGER;
    OCPP_ConfigurationKey[OCPP_CK_MinimumStatusDuration].type.intData = 10;

    //9.1.19. NumberOfConnectors
    OCPP_ConfigurationKey[OCPP_CK_NumberOfConnectors].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_NumberOfConnectors].accessibility = OCPP_ACC_READONLY;
    OCPP_ConfigurationKey[OCPP_CK_NumberOfConnectors].dataType = OCPP_CK_DT_INTEGER;
    OCPP_ConfigurationKey[OCPP_CK_NumberOfConnectors].type.intData = 2;

    //9.1.20. ResetRetries
    OCPP_ConfigurationKey[OCPP_CK_ResetRetries].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_ResetRetries].accessibility = OCPP_ACC_READWRITE;
    OCPP_ConfigurationKey[OCPP_CK_ResetRetries].dataType = OCPP_CK_DT_INTEGER;
    OCPP_ConfigurationKey[OCPP_CK_ResetRetries].type.intData = 3;

    //9.1.21. ConnectorPhaseRotation
    OCPP_ConfigurationKey[OCPP_CK_ConnectorPhaseRotation].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_ConnectorPhaseRotation].accessibility = OCPP_ACC_READWRITE;
    OCPP_ConfigurationKey[OCPP_CK_ConnectorPhaseRotation].dataType = OCPP_CK_DT_STRING;
    OCPP_ConfigurationKey[OCPP_CK_ConnectorPhaseRotation].type.stringData = (char *)calloc(1, OCPP_CONFIGURATION_VALUES_STRING_MAX);
    stringLen = strlen("NotApplicable");
    strncpy(OCPP_ConfigurationKey[OCPP_CK_ConnectorPhaseRotation].type.stringData,
            "NotApplicable",
            (stringLen > OCPP_CONFIGURATION_VALUES_STRING_MAX) ? OCPP_CONFIGURATION_VALUES_STRING_MAX : stringLen);

    //9.1.22. ConnectorPhaseRotationMaxLength
    OCPP_ConfigurationKey[OCPP_CK_ConnectorPhaseRotationMaxLength].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_ConnectorPhaseRotationMaxLength].accessibility = OCPP_ACC_READONLY;
    OCPP_ConfigurationKey[OCPP_CK_ConnectorPhaseRotationMaxLength].dataType = OCPP_CK_DT_INTEGER;
    OCPP_ConfigurationKey[OCPP_CK_ConnectorPhaseRotationMaxLength].type.intData = 0;

    //9.1.23. StopTransactionOnEVSideDisconnect
    OCPP_ConfigurationKey[OCPP_CK_StopTransactionOnEVSideDisconnect].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_StopTransactionOnEVSideDisconnect].accessibility = OCPP_ACC_READWRITE;
    OCPP_ConfigurationKey[OCPP_CK_StopTransactionOnEVSideDisconnect].dataType = OCPP_CK_DT_BOOLEAN;
    OCPP_ConfigurationKey[OCPP_CK_StopTransactionOnEVSideDisconnect].type.boolData = true;

    //9.1.24. StopTransactionOnInvalidId
    OCPP_ConfigurationKey[OCPP_CK_StopTransactionOnInvalidId].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_StopTransactionOnInvalidId].accessibility = OCPP_ACC_READWRITE;
    OCPP_ConfigurationKey[OCPP_CK_StopTransactionOnInvalidId].dataType = OCPP_CK_DT_BOOLEAN;
    OCPP_ConfigurationKey[OCPP_CK_StopTransactionOnInvalidId].type.boolData = true;

    //9.1.25. StopTxnAlignedData
    OCPP_ConfigurationKey[OCPP_CK_StopTxnAlignedData].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_StopTxnAlignedData].accessibility = OCPP_ACC_READWRITE;
    OCPP_ConfigurationKey[OCPP_CK_StopTxnAlignedData].dataType = OCPP_CK_DT_STRING;
    OCPP_ConfigurationKey[OCPP_CK_StopTxnAlignedData].type.stringData = (char *)calloc(1, OCPP_CONFIGURATION_VALUES_STRING_MAX);
    stringLen = strlen("Voltage:Current.Import:Energy.Active.Import.Register:SoC:Current.Offered:Power.Active.Import:Energy.Reactive.Import.Register");
    strncpy(OCPP_ConfigurationKey[OCPP_CK_StopTxnAlignedData].type.stringData,
            "Voltage:Current.Import:Energy.Active.Import.Register:SoC:Current.Offered:Power.Active.Import:Energy.Reactive.Import.Register",
            (stringLen > OCPP_CONFIGURATION_VALUES_STRING_MAX) ? OCPP_CONFIGURATION_VALUES_STRING_MAX : stringLen);

    //9.1.26. StopTxnAlignedDataMaxLength
    OCPP_ConfigurationKey[OCPP_CK_StopTxnAlignedDataMaxLength].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_StopTxnAlignedDataMaxLength].accessibility = OCPP_ACC_READONLY;
    OCPP_ConfigurationKey[OCPP_CK_StopTxnAlignedDataMaxLength].dataType = OCPP_CK_DT_INTEGER;
    OCPP_ConfigurationKey[OCPP_CK_StopTxnAlignedDataMaxLength].type.intData = 10;

    //9.1.27. StopTxnSampledData
    OCPP_ConfigurationKey[OCPP_CK_StopTxnSampledData].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_StopTxnSampledData].accessibility = OCPP_ACC_READWRITE;
    OCPP_ConfigurationKey[OCPP_CK_StopTxnSampledData].dataType = OCPP_CK_DT_STRING;
    OCPP_ConfigurationKey[OCPP_CK_StopTxnSampledData].type.stringData = (char *)calloc(1, OCPP_CONFIGURATION_VALUES_STRING_MAX);
    stringLen = strlen("Voltage:Current.Import:Energy.Active.Import.Register:SoC:Current.Offered:Power.Active.Import:Energy.Reactive.Import.Register");
    strncpy(OCPP_ConfigurationKey[OCPP_CK_StopTxnSampledData].type.stringData,
            "Voltage:Current.Import:Energy.Active.Import.Register:SoC:Current.Offered:Power.Active.Import:Energy.Reactive.Import.Register",
            (stringLen > OCPP_CONFIGURATION_VALUES_STRING_MAX) ? OCPP_CONFIGURATION_VALUES_STRING_MAX : stringLen);

    //9.1.28. StopTxnSampledDataMaxLength
    OCPP_ConfigurationKey[OCPP_CK_StopTxnSampledDataMaxLength].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_StopTxnSampledDataMaxLength].accessibility = OCPP_ACC_READONLY;
    OCPP_ConfigurationKey[OCPP_CK_StopTxnSampledDataMaxLength].dataType = OCPP_CK_DT_INTEGER;
    OCPP_ConfigurationKey[OCPP_CK_StopTxnSampledDataMaxLength].type.intData = 10;

    //9.1.29. SupportedFeatureProfiles
    OCPP_ConfigurationKey[OCPP_CK_SupportedFeatureProfiles].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_SupportedFeatureProfiles].accessibility = OCPP_ACC_READWRITE;
    OCPP_ConfigurationKey[OCPP_CK_SupportedFeatureProfiles].dataType = OCPP_CK_DT_STRING;
    OCPP_ConfigurationKey[OCPP_CK_SupportedFeatureProfiles].type.stringData = (char *)calloc(1, OCPP_CONFIGURATION_VALUES_STRING_MAX);
    stringLen = strlen("Core,FirmwareManagement,LocalAuthListManagement,Reservation,SmartCharging,RemoteTrigger");
    strncpy(OCPP_ConfigurationKey[OCPP_CK_SupportedFeatureProfiles].type.stringData,
            "Core,FirmwareManagement,LocalAuthListManagement,Reservation,SmartCharging,RemoteTrigger",
            (stringLen > OCPP_CONFIGURATION_VALUES_STRING_MAX) ? OCPP_CONFIGURATION_VALUES_STRING_MAX : stringLen);

    //9.1.30. SupportedFeatureProfilesMaxLength
    OCPP_ConfigurationKey[OCPP_CK_SupportedFeatureProfilesMaxLength].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_SupportedFeatureProfilesMaxLength].accessibility = OCPP_ACC_READONLY;
    OCPP_ConfigurationKey[OCPP_CK_SupportedFeatureProfilesMaxLength].dataType = OCPP_CK_DT_INTEGER;
    OCPP_ConfigurationKey[OCPP_CK_SupportedFeatureProfilesMaxLength].type.intData = 6;

    //9.1.31. TransactionMessageAttempts
    OCPP_ConfigurationKey[OCPP_CK_TransactionMessageAttempt].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_TransactionMessageAttempt].accessibility = OCPP_ACC_READWRITE;
    OCPP_ConfigurationKey[OCPP_CK_TransactionMessageAttempt].dataType = OCPP_CK_DT_INTEGER;
    OCPP_ConfigurationKey[OCPP_CK_TransactionMessageAttempt].type.intData = 3;

    //9.1.32. TransactionMessageRetryInterva
    OCPP_ConfigurationKey[OCPP_CK_TransactionMessageRetryInterval].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_TransactionMessageRetryInterval].accessibility = OCPP_ACC_READWRITE;
    OCPP_ConfigurationKey[OCPP_CK_TransactionMessageRetryInterval].dataType = OCPP_CK_DT_INTEGER;
    OCPP_ConfigurationKey[OCPP_CK_TransactionMessageRetryInterval].type.intData = 15;

    //9.1.33. UnlockConnectorOnEVSideDisconnect
    OCPP_ConfigurationKey[OCPP_CK_UnlockConnectorOnEVSideDisconnect].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_UnlockConnectorOnEVSideDisconnect].accessibility = OCPP_ACC_READWRITE;
    OCPP_ConfigurationKey[OCPP_CK_UnlockConnectorOnEVSideDisconnect].dataType = OCPP_CK_DT_BOOLEAN;
    OCPP_ConfigurationKey[OCPP_CK_UnlockConnectorOnEVSideDisconnect].type.boolData = true;

    //9.1.34. WebSocketPingInterva
    OCPP_ConfigurationKey[OCPP_CK_WebSocketPingInterval].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_WebSocketPingInterval].accessibility = OCPP_ACC_READWRITE;
    OCPP_ConfigurationKey[OCPP_CK_WebSocketPingInterval].dataType = OCPP_CK_DT_INTEGER;
    OCPP_ConfigurationKey[OCPP_CK_WebSocketPingInterval].type.intData = 10;

    //9.2.1. LocalAuthListEnabled
    OCPP_ConfigurationKey[OCPP_CK_LocalAuthListEnabled].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_LocalAuthListEnabled].accessibility = OCPP_ACC_READWRITE;
    OCPP_ConfigurationKey[OCPP_CK_LocalAuthListEnabled].dataType = OCPP_CK_DT_BOOLEAN;
    OCPP_ConfigurationKey[OCPP_CK_LocalAuthListEnabled].type.boolData = true;

    //9.2.2. LocalAuthListMaxLength
    OCPP_ConfigurationKey[OCPP_CK_LocalAuthListMaxLength].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_LocalAuthListMaxLength].accessibility = OCPP_ACC_READONLY;
    OCPP_ConfigurationKey[OCPP_CK_LocalAuthListMaxLength].dataType = OCPP_CK_DT_INTEGER;
    OCPP_ConfigurationKey[OCPP_CK_LocalAuthListMaxLength].type.intData = 50;

    //9.2.3. SendLocalListMaxLength
    OCPP_ConfigurationKey[OCPP_CK_SendLocalListMaxLength].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_SendLocalListMaxLength].accessibility = OCPP_ACC_READONLY;
    OCPP_ConfigurationKey[OCPP_CK_SendLocalListMaxLength].dataType = OCPP_CK_DT_INTEGER;
    OCPP_ConfigurationKey[OCPP_CK_SendLocalListMaxLength].type.boolData = 10;

    //9.3.1. ReserveConnectorZeroSupported
    OCPP_ConfigurationKey[OCPP_CK_ReserveConnectorZeroSupported].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_ReserveConnectorZeroSupported].accessibility = OCPP_ACC_READONLY;
    OCPP_ConfigurationKey[OCPP_CK_ReserveConnectorZeroSupported].dataType = OCPP_CK_DT_BOOLEAN;
    OCPP_ConfigurationKey[OCPP_CK_ReserveConnectorZeroSupported].type.boolData = true;

    //9.4.1. ChargeProfileMaxStackLevel
    OCPP_ConfigurationKey[OCPP_CK_ChargeProfileMaxStackLevel].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_ChargeProfileMaxStackLevel].accessibility = OCPP_ACC_READONLY;
    OCPP_ConfigurationKey[OCPP_CK_ChargeProfileMaxStackLevel].dataType = OCPP_CK_DT_INTEGER;
    OCPP_ConfigurationKey[OCPP_CK_ChargeProfileMaxStackLevel].type.intData = 3;

    //9.4.2. ChargingScheduleAllowedChargingRateUnit
    OCPP_ConfigurationKey[OCPP_CK_ChargingScheduleAllowedChargingRateUnit].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_ChargingScheduleAllowedChargingRateUnit].accessibility = OCPP_ACC_READONLY;
    OCPP_ConfigurationKey[OCPP_CK_ChargingScheduleAllowedChargingRateUnit].dataType = OCPP_CK_DT_STRING;
    OCPP_ConfigurationKey[OCPP_CK_ChargingScheduleAllowedChargingRateUnit].type.stringData = (char *)calloc(1, OCPP_CONFIGURATION_VALUES_STRING_MAX);
    stringLen = strlen("Current,Power");
    strncpy(OCPP_ConfigurationKey[OCPP_CK_ChargingScheduleAllowedChargingRateUnit].type.stringData,
            "Current,Power",
            (stringLen > OCPP_CONFIGURATION_VALUES_STRING_MAX) ? OCPP_CONFIGURATION_VALUES_STRING_MAX : stringLen);

    //9.4.3. ChargingScheduleMaxPeriods
    OCPP_ConfigurationKey[OCPP_CK_ChargingScheduleMaxPeriods].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_ChargingScheduleMaxPeriods].accessibility = OCPP_ACC_READONLY;
    OCPP_ConfigurationKey[OCPP_CK_ChargingScheduleMaxPeriods].dataType = OCPP_CK_DT_INTEGER;
    OCPP_ConfigurationKey[OCPP_CK_ChargingScheduleMaxPeriods].type.intData = 10;

    //9.4.4. ConnectorSwitch3to1PhaseSupported
    OCPP_ConfigurationKey[OCPP_CK_ConnectorSwitch3to1PhaseSupported].isUsed = false;
    OCPP_ConfigurationKey[OCPP_CK_ConnectorSwitch3to1PhaseSupported].accessibility = OCPP_ACC_READONLY;
    OCPP_ConfigurationKey[OCPP_CK_ConnectorSwitch3to1PhaseSupported].dataType = OCPP_CK_DT_BOOLEAN;
    OCPP_ConfigurationKey[OCPP_CK_ConnectorSwitch3to1PhaseSupported].type.boolData = true;

    //9.4.5. MaxChargingProfilesInstalled
    OCPP_ConfigurationKey[OCPP_CK_MaxChargingProfilesInstalled].isUsed = true;
    OCPP_ConfigurationKey[OCPP_CK_MaxChargingProfilesInstalled].accessibility = OCPP_ACC_READONLY;
    OCPP_ConfigurationKey[OCPP_CK_MaxChargingProfilesInstalled].dataType = OCPP_CK_DT_INTEGER;
    OCPP_ConfigurationKey[OCPP_CK_MaxChargingProfilesInstalled].type.intData = 10;


}



/**
 * @description: 
 * @param {char *} key
 * @return {*}
 */
bool ocpp_ConfigurationKey_isFound(const char * key){
   if(key == NULL) return false;

   int index = 0;
   while(index < OCPP_CK_MAXLEN){
      if(strcmp(key,ocpp_configurationKeyText[index]) == 0){
         break;
      }
      index++;  
   }   

   if(index >= OCPP_CK_MAXLEN) return false;

   return true;
}


/**
 * @description: 判断配置是否使用
 * @param {char *} key = "key名"
 * @return {*} true = 使用, false = 未使用
 */
bool ocpp_ConfigurationKey_isUse(const char * key){

   if(key == NULL) return false;

   int index = 0;
   while(index < OCPP_CK_MAXLEN){
      if(strcmp(key,ocpp_configurationKeyText[index]) == 0){
         break;
      }
      index++;  
   }

   if(index >= OCPP_CK_MAXLEN) return false;
        
   bool isUse = false;
   pthread_rwlock_rdlock(&ocpp_ConfigurationKey_rwlock);
   isUse = OCPP_ConfigurationKey[index].isUsed;
   pthread_rwlock_unlock(&ocpp_ConfigurationKey_rwlock);
   return isUse;
}


/**
 * @description: 获取配置的读写权限
 * @param {char *} key = "key名"
 * @return {*} OCPP_ACC_NONE = 无权限,
 *             OCPP_ACC_READONLY = 只读,
 *             OCPP_ACC_WRITEONLY = 只写,
 *             OCPP_ACC_READWRITE = 读写
 */
enum OCPP_ACCESSIBILITY_TYPE_E ocpp_ConfigurationKey_getAcc(const char * key){
   if(key == NULL) return OCPP_ACC_NONE;

   int index = 0;
   while(index < OCPP_CK_MAXLEN){
      if(strcmp(key,ocpp_configurationKeyText[index]) == 0){
         break;
      }
      index++;  
   }

   if(index >= OCPP_CK_MAXLEN) return OCPP_ACC_NONE;
   
   enum OCPP_ACCESSIBILITY_TYPE_E accessibility = OCPP_ACC_NONE;

   pthread_rwlock_rdlock(&ocpp_ConfigurationKey_rwlock);
   accessibility = OCPP_ConfigurationKey[index].accessibility;
   pthread_rwlock_unlock(&ocpp_ConfigurationKey_rwlock);
   return accessibility; 
} 

/**
 * @description: 获取配置key的数据类型
 * @param {char *} key = "key名"
 * @return {*}OCPP_CK_DT_BOOLEAN = bool
 *            OCPP_CK_DT_INTEGER = int
 *            OCPP_CK_DT_STRING  = string
 */
enum OCPP_CONFIGURATION_KEY_DATA_TYPE_E ocpp_ConfigurationKey_getType(const char * key){
   if(key == NULL) return OCPP_CK_DT_NONE;

   int index = 0;
   while(index < OCPP_CK_MAXLEN){
      if(strcmp(key,ocpp_configurationKeyText[index]) == 0){
         break;
      }
      index++;  
   }

   if(index >= OCPP_CK_MAXLEN) return OCPP_CK_DT_NONE;

   int type = OCPP_CK_DT_NONE;
   pthread_rwlock_rdlock(&ocpp_ConfigurationKey_rwlock);
   type = OCPP_ConfigurationKey[index].dataType;
   pthread_rwlock_unlock(&ocpp_ConfigurationKey_rwlock);
   return type;
        
}

/**
 * @description: 获取配置key的数据
 * @param {char *} key
 * @param {void *} value
 * @return {*} 返回value的数据类型
 */
enum OCPP_CONFIGURATION_KEY_DATA_TYPE_E ocpp_ConfigurationKey_getValue(const char * key,void * value){
   if(key == NULL) return -1;

   int index = 0;
   while(index < OCPP_CK_MAXLEN){
      if(strcmp(key,ocpp_configurationKeyText[index]) == 0){
         break;
      }
      index++;  
   }

   if(index >= OCPP_CK_MAXLEN) return -1;

   int dataType = -1;
   pthread_rwlock_rdlock(&ocpp_ConfigurationKey_rwlock);
   if(OCPP_ConfigurationKey[index].dataType == OCPP_CK_DT_BOOLEAN){
        *((bool *)value) = OCPP_ConfigurationKey[index].type.boolData;
        dataType =  OCPP_CK_DT_BOOLEAN;

   }else if(OCPP_ConfigurationKey[index].dataType == OCPP_CK_DT_INTEGER){
        *((int *)value) = OCPP_ConfigurationKey[index].type.intData;
        dataType = OCPP_CK_DT_INTEGER;

   }else if(OCPP_ConfigurationKey[index].dataType == OCPP_CK_DT_STRING){
        strncpy((char *)value,OCPP_ConfigurationKey[index].type.stringData,strlen(OCPP_ConfigurationKey[index].type.stringData));
        dataType = OCPP_CK_DT_STRING;
  
   }
   pthread_rwlock_unlock(&ocpp_ConfigurationKey_rwlock);
   return dataType;
}


/**
 * @description: 创建configurationKeys数据表
 * @param:
 * @return: 0-成功,-1-失败
 */
static int ocpp_ConfigurationKey_Create_Table(sqlite3 *p_db){

   int rc = 0;
   char *zErrMsg = 0;
   char sql[512];

   snprintf(sql, sizeof(sql), "CREATE TABLE IF NOT EXISTS ConfigurationKeys("\
          "ID             integer  PRIMARY KEY,"\
          "Key            TEXT     NOT NULL,"\
          "Value          TEXT     default NULL,"\
          "Type           TEXT     NOT NULL,"\
          "Accessibility  TEXT     NOT NULL,"\
          "IsUse          integer  NOT NULL);");
    
   rc = sqlite3_exec(p_db, sql, NULL, 0, &zErrMsg);
   if(rc != SQLITE_OK){   
      printf("error create %s\n",zErrMsg);
      sqlite3_free(zErrMsg);
      
      return -1;
   }

   return 0;
}


/**
 * @description: 将Configurationkey插入数据库
 * @param:
 * @return: 0 = success, -1 = fail
 */
static int ocpp_ConfigurationKey_InsertSQLite(const char *key, const char *value, const char * type, const char * acessibility, int isUse){
   if(ocpp_CK == NULL) return -1;
   if(key == NULL) return -1;
   int rc;
   char sql[512];
   char *ErrMsg;

   snprintf(sql,512,"INSERT INTO ConfigurationKeys (Key,Value,Type,Accessibility,IsUse) VALUES ('%s','%s','%s','%s',%d)",
                key,value,type,acessibility,isUse);

   rc=sqlite3_exec(ocpp_CK,sql,NULL, NULL,&ErrMsg);
   if (rc!=SQLITE_OK){
      printf("ERROR = %s\n",ErrMsg);
      sqlite3_free(ErrMsg);
      return -1;
   }

   return 0;
}

/**
 * @description: 从数据库中读取键、值,并修改内存中的键、值
 * @param:
 * @return： 0 = success, -1 = fail
 */
static int ocpp_ConfigurationKey_ReadSQLite(const char *key){

   if(ocpp_CK == NULL) return -1;
   if(key == NULL) return -1;

   int index = 0;
   while(index < OCPP_CK_MAXLEN){
      if(strcmp(key,ocpp_configurationKeyText[index]) == 0){
         break;
      }
      index++;
   }

   if(index >= OCPP_CK_MAXLEN) return -1;

   sqlite3_stmt * findstmt = NULL;
   sqlite3_stmt * readstmt = NULL;
   char sql[256];
   int find = 0;

   snprintf(sql,256,"SELECT count(*) FROM ConfigurationKeys where Key = '%s'",key);
   int rc=sqlite3_prepare_v2(ocpp_CK,sql,strlen(sql), &findstmt,NULL);
   
   if(rc == SQLITE_OK){
      rc=sqlite3_step(findstmt);
      find=sqlite3_column_int(findstmt,0);     

      //find data
      if(find != 0){
        memset(sql,0,256);
        snprintf(sql,256,"SELECT * FROM ConfigurationKeys where Key = '%s'",key);
        int rc=sqlite3_prepare_v2(ocpp_CK,sql,-1, &readstmt,NULL);

        if (rc == SQLITE_OK){
            rc=sqlite3_step(readstmt);
            //accessibility
            if(strstr(sqlite3_column_text(readstmt,4),"READONLY") != NULL)
                OCPP_ConfigurationKey[index].accessibility = OCPP_ACC_READONLY;

            else if(strstr(sqlite3_column_text(readstmt,4),"WRITEONLY") != NULL)
                OCPP_ConfigurationKey[index].accessibility = OCPP_ACC_WRITEONLY;

            else if(strstr(sqlite3_column_text(readstmt,4),"READWRITE") != NULL)
                OCPP_ConfigurationKey[index].accessibility = OCPP_ACC_READWRITE;

            else
                OCPP_ConfigurationKey[index].accessibility = OCPP_ACC_NONE;

           //type
           const char * value = sqlite3_column_text(readstmt,2);
           const char * type = sqlite3_column_text(readstmt,3);                        

           if(value != NULL){
             if(strstr(type,"boolean") != NULL){
                OCPP_ConfigurationKey[index].type.boolData = atoi(value);

             }else if(strstr(type,"integer") != NULL){
                OCPP_ConfigurationKey[index].type.intData = atoi(value);

             }else if(strstr(type,"string") != NULL){
            
                strncpy(OCPP_ConfigurationKey[index].type.stringData,value,strlen(value));
             }else
                printf("type error\n");           
         }

         //isUse
         OCPP_ConfigurationKey[index].isUsed = sqlite3_column_int(readstmt,5);  

        }else{
            printf("查询失败 'SELECT KEY,VALUE FROM ConfigurationKeys'");
        }
        
      }
      //not find data
      else{
         char stringvalue[10] = {0};


         if(OCPP_ConfigurationKey[index].dataType == OCPP_CK_DT_BOOLEAN){

                snprintf(stringvalue,10,"%d",OCPP_ConfigurationKey[index].type.boolData);
                ocpp_ConfigurationKey_InsertSQLite(key,
                                                  stringvalue,
                                                  "boolean",
                                                  ocpp_AccessibilityText[OCPP_ConfigurationKey[index].accessibility],
                                                  OCPP_ConfigurationKey[index].isUsed);

         }else if(OCPP_ConfigurationKey[index].dataType == OCPP_CK_DT_INTEGER){

                snprintf(stringvalue,10,"%d",OCPP_ConfigurationKey[index].type.intData);
                ocpp_ConfigurationKey_InsertSQLite(key,
                                                  stringvalue,
                                                  "integer",
                                                  ocpp_AccessibilityText[OCPP_ConfigurationKey[index].accessibility],
                                                  OCPP_ConfigurationKey[index].isUsed);

         }else if(OCPP_ConfigurationKey[index].dataType == OCPP_CK_DT_STRING){

                ocpp_ConfigurationKey_InsertSQLite(key,
                                                  OCPP_ConfigurationKey[index].type.stringData,
                                                  "string",
                                                  ocpp_AccessibilityText[OCPP_ConfigurationKey[index].accessibility],
                                                  OCPP_ConfigurationKey[index].isUsed);

                
         }else{
                printf("%s dataType error",ocpp_configurationKeyText[index]);

         }
         
         


      }



   }else{
        printf("read Error\n");

   }


   sqlite3_finalize(findstmt);
   sqlite3_finalize(readstmt);

   return 0;
}


/**
 * @description: 更新数据库的key、value、isUse
 * @param:
 * @return：
 */
static int ocpp_ConfigurationKey_UpdateSQLite(const char *key, char *value,int isUse){

   if (ocpp_CK == NULL) return -1;
   if(key == NULL) return -1;

   sqlite3_stmt *res;
   char *ErrMsg;
   char sql[256];
   int find = 0;

   snprintf(sql,256,"SELECT count(*) FROM ConfigurationKeys where Key = '%s'",key);
   int rc=sqlite3_prepare_v2(ocpp_CK,sql,strlen(sql), &res,NULL);

   if (rc == SQLITE_OK){
      
      rc=sqlite3_step(res);
      find=sqlite3_column_int(res,0);

      if(find > 0){
         memset(sql, 0, 256);
         snprintf(sql,256,"UPDATE ConfigurationKeys set Value = '%s',isUse = %d where Key = '%s'",value,isUse,key);
	 rc=sqlite3_exec(ocpp_CK,sql,NULL, NULL,&ErrMsg);

	 if(rc!=SQLITE_OK){
	    printf("ERROR = %s",ErrMsg);
	    sqlite3_free(ErrMsg);
         }
      }
     
   }else{

        printf("SELECT count(*) FROM ConfigurationKeys-->ERROR\n");
   }

   sqlite3_finalize(res);
   return 0;

}


/**
 * @description: 
 * @param {char *} key
 * @param {char *} value
 * @param {int} isUse
 * @return {*} 0 = success,1 = fail
 */
int ocpp_ConfigurationKey_Modify(const char * key,char * value,int isUse){

   if(key == NULL) return -1;
   
   int index = 0;
   while(index < OCPP_CK_MAXLEN){
      if(strcmp(key,ocpp_configurationKeyText[index]) == 0){
         break;
      }
      index++;  
   }

   if(index >= OCPP_CK_MAXLEN) return -1;

   pthread_rwlock_wrlock(&ocpp_ConfigurationKey_rwlock);
   OCPP_ConfigurationKey[index].isUsed = isUse;
   if(OCPP_ConfigurationKey[index].dataType == OCPP_CK_DT_BOOLEAN){
        OCPP_ConfigurationKey[index].type.boolData = atoi(value);
        

   }else if(OCPP_ConfigurationKey[index].dataType == OCPP_CK_DT_INTEGER){
        OCPP_ConfigurationKey[index].type.intData = atoi(value);

   }else if(OCPP_ConfigurationKey[index].dataType == OCPP_CK_DT_STRING){
        strncpy(OCPP_ConfigurationKey[index].type.stringData,value,strlen(value));

   }else{
        printf("type error\n");
   }
   pthread_rwlock_unlock(&ocpp_ConfigurationKey_rwlock);
   ocpp_ConfigurationKey_UpdateSQLite(key,value,isUse);
   
   return 0;
}



/**
 * @description: 
 * @return {*}
 */
void ocpp_ConfigurationKey_deinit(void){
   pthread_rwlock_wrlock(&ocpp_ConfigurationKey_rwlock);
   ocpp_ConfiguartionKey_default_init();
   pthread_rwlock_unlock(&ocpp_ConfigurationKey_rwlock);
}



/**
 * @description: 
 * @return {*} 0 = success, 1 = fail
 */
int ocpp_ConfigurationKey_init(sqlite3 *ocpp_db){

   ocpp_CK = ocpp_db;

   pthread_rwlock_init(&ocpp_ConfigurationKey_rwlock,NULL);
   
   if(ocpp_ConfigurationKey_Create_Table(ocpp_CK) == -1) return -1;

   pthread_rwlock_wrlock(&ocpp_ConfigurationKey_rwlock);
   ocpp_ConfiguartionKey_default_init();

   int n = 0;
   while(n < OCPP_CK_MAXLEN){
        ocpp_ConfigurationKey_ReadSQLite(ocpp_configurationKeyText[n]);
        n++;
   }

   pthread_rwlock_unlock(&ocpp_ConfigurationKey_rwlock);

}

/**
 * @description: 
 * @return {*}
 */
// void main(void){
//    printf("OCPP_ConfigurationKey Test\n");
//    sqlite3_open("ocpp.db",&db);
//    printf("init = %d\n",ocpp_ConfigurationKey_init());
   
//    int n = 0;
//    bool boolData = false;
//    int intData = -1;
//    char stringData[512];
//    memset(stringData,0,512);
//    while(n < OCPP_CK_MAXLEN){
//        printf("name = %s\n",ocpp_configurationKeyText[n]);
//        printf("isUse = %d\n",ocpp_ConfigurationKey_isUse(ocpp_configurationKeyText[n]));
//        printf("acc = %s\n",ocpp_AccessibilityText[ocpp_ConfigurationKey_getAcc(ocpp_configurationKeyText[n])]);

//        if(ocpp_ConfigurationKey_getType(ocpp_configurationKeyText[n]) == OCPP_CK_DT_BOOLEAN){
//                 ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[n],&boolData);
//                 printf("value_boolean = %d\n",boolData);

//        }else if(ocpp_ConfigurationKey_getType(ocpp_configurationKeyText[n]) == OCPP_CK_DT_INTEGER){
//                 ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[n],&intData);
//                 printf("value_int = %d\n",intData);

//        }else if(ocpp_ConfigurationKey_getType(ocpp_configurationKeyText[n]) == OCPP_CK_DT_STRING){
//                 ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[n],stringData);
//                 printf("value_string = %s\n",stringData);

//        }else{

//                 printf("value error\n");
//        }
//        printf("\n\n");
//        n++;
//    } 

//    ocpp_ConfigurationKey_Modify("ConnectionTimeOut","100",1);
//    return;
// }


























