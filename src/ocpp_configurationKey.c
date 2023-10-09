#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "ocpp_configurationKey.h"
#include "ocpp_config.h"

static sqlite3 *ocpp_CK;

static bool ocpp_ConfigurationKey_TableExists(sqlite3 *db)
{
	int rc;
	sqlite3_stmt *stmt = NULL;
	const char *sql = "SELECT name FROM sqlite_master WHERE type='table' AND name='ConfigurationKeys';";

	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		// 处理SQL查询准备失败的情况
		return false;
	}

	rc = sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	return (rc == SQLITE_ROW); // 如果查询返回行，则表存在；否则，表不存在
}

static void ocpp_ConfiguartionKey_data_init(OCPP_ConfigurationKey_t *OCPP_ConfigurationKey)
{
	// 9.1.1. AllowOfflineTxForUnknownId
	OCPP_ConfigurationKey[OCPP_CK_AllowOfflineTxForUnknownId].isUsed = false;
	OCPP_ConfigurationKey[OCPP_CK_AllowOfflineTxForUnknownId].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_AllowOfflineTxForUnknownId].dataType = OCPP_CK_DT_BOOLEAN;
	OCPP_ConfigurationKey[OCPP_CK_AllowOfflineTxForUnknownId].type.boolData = false;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_AllowOfflineTxForUnknownId].key,
			ocpp_configurationKeyText[OCPP_CK_AllowOfflineTxForUnknownId], 64);

	// 9.1.2. AuthorizationCacheEnable
	OCPP_ConfigurationKey[OCPP_CK_AuthorizationCacheEnabled].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_AuthorizationCacheEnabled].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_AuthorizationCacheEnabled].dataType = OCPP_CK_DT_BOOLEAN;
	OCPP_ConfigurationKey[OCPP_CK_AuthorizationCacheEnabled].type.boolData = true;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_AuthorizationCacheEnabled].key,
			ocpp_configurationKeyText[OCPP_CK_AuthorizationCacheEnabled], 64);

	// 9.1.3. AuthorizeRemoteTxRequests
	//
	OCPP_ConfigurationKey[OCPP_CK_AuthorizeRemoteTxRequests].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_AuthorizeRemoteTxRequests].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_AuthorizeRemoteTxRequests].dataType = OCPP_CK_DT_BOOLEAN;
	OCPP_ConfigurationKey[OCPP_CK_AuthorizeRemoteTxRequests].type.boolData = true;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_AuthorizeRemoteTxRequests].key,
			ocpp_configurationKeyText[OCPP_CK_AuthorizeRemoteTxRequests], 64);

	// 9.1.4. BlinkRepeat
	OCPP_ConfigurationKey[OCPP_CK_BlinkRepeat].isUsed = false;
	OCPP_ConfigurationKey[OCPP_CK_BlinkRepeat].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_BlinkRepeat].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_BlinkRepeat].type.intData = 0;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_BlinkRepeat].key,
			ocpp_configurationKeyText[OCPP_CK_BlinkRepeat], 64);

	// 9.1.5. ClockAlignedDataInterval
	OCPP_ConfigurationKey[OCPP_CK_ClockAlignedDataInterval].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_ClockAlignedDataInterval].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_ClockAlignedDataInterval].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_ClockAlignedDataInterval].type.intData = 0;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_ClockAlignedDataInterval].key,
			ocpp_configurationKeyText[OCPP_CK_ClockAlignedDataInterval], 64);

	// 9.1.6. ConnectionTimeOut
	OCPP_ConfigurationKey[OCPP_CK_ConnectionTimeOut].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_ConnectionTimeOut].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_ConnectionTimeOut].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_ConnectionTimeOut].type.intData = 0;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_ConnectionTimeOut].key,
			ocpp_configurationKeyText[OCPP_CK_ConnectionTimeOut], 64);

	// 9.1.7. GetConfigurationMaxKeys
	OCPP_ConfigurationKey[OCPP_CK_GetConfigurationMaxKeys].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_GetConfigurationMaxKeys].accessibility = OCPP_ACC_READONLY;
	OCPP_ConfigurationKey[OCPP_CK_GetConfigurationMaxKeys].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_GetConfigurationMaxKeys].type.intData = 5;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_GetConfigurationMaxKeys].key,
			ocpp_configurationKeyText[OCPP_CK_GetConfigurationMaxKeys], 64);

	// 9.1.8. HeartbeatInterva
	OCPP_ConfigurationKey[OCPP_CK_HeartbeatInterval].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_HeartbeatInterval].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_HeartbeatInterval].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_HeartbeatInterval].type.intData = 30;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_HeartbeatInterval].key,
			ocpp_configurationKeyText[OCPP_CK_HeartbeatInterval], 64);

	// 9.1.9. LightIntensity
	OCPP_ConfigurationKey[OCPP_CK_LightIntensity].isUsed = false;
	OCPP_ConfigurationKey[OCPP_CK_LightIntensity].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_LightIntensity].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_LightIntensity].type.intData = 0;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_LightIntensity].key,
			ocpp_configurationKeyText[OCPP_CK_LightIntensity], 64);

	// 9.1.10. LocalAuthorizeOffline
	OCPP_ConfigurationKey[OCPP_CK_LocalAuthorizeOffline].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_LocalAuthorizeOffline].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_LocalAuthorizeOffline].dataType = OCPP_CK_DT_BOOLEAN;
	OCPP_ConfigurationKey[OCPP_CK_LocalAuthorizeOffline].type.boolData = true;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_LocalAuthorizeOffline].key,
			ocpp_configurationKeyText[OCPP_CK_LocalAuthorizeOffline], 64);

	// 9.1.11. LocalPreAuthorize
	OCPP_ConfigurationKey[OCPP_CK_LocalPreAuthorize].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_LocalPreAuthorize].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_LocalPreAuthorize].dataType = OCPP_CK_DT_BOOLEAN;
	OCPP_ConfigurationKey[OCPP_CK_LocalPreAuthorize].type.boolData = false;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_LocalPreAuthorize].key,
			ocpp_configurationKeyText[OCPP_CK_LocalPreAuthorize], 64);

	// 9.1.12. MaxEnergyOnInvalidId
	OCPP_ConfigurationKey[OCPP_CK_MaxEnergyOnInvalidId].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_MaxEnergyOnInvalidId].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_MaxEnergyOnInvalidId].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_MaxEnergyOnInvalidId].type.intData = 0;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_MaxEnergyOnInvalidId].key,
			ocpp_configurationKeyText[OCPP_CK_MaxEnergyOnInvalidId], 64);

	// 9.1.13. MeterValuesAlignedData
	OCPP_ConfigurationKey[OCPP_CK_MeterValuesAlignedData].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_MeterValuesAlignedData].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_MeterValuesAlignedData].dataType = OCPP_CK_DT_STRING;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_MeterValuesAlignedData].type.stringData,
			"Voltage,Current.Import,Energy.Active.Import.Register,SoC,Current.Offered,Power.Active.Import,Energy.Reactive.Import.Register", 512);
	strncpy(OCPP_ConfigurationKey[OCPP_CK_MeterValuesAlignedData].key,
			ocpp_configurationKeyText[OCPP_CK_MeterValuesAlignedData], 64);

	// 9.1.14. MeterValuesAlignedDataMaxLength
	OCPP_ConfigurationKey[OCPP_CK_MeterValuesAlignedDataMaxLength].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_MeterValuesAlignedDataMaxLength].accessibility = OCPP_ACC_READONLY;
	OCPP_ConfigurationKey[OCPP_CK_MeterValuesAlignedDataMaxLength].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_MeterValuesAlignedDataMaxLength].type.intData = 10;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_MeterValuesAlignedDataMaxLength].key,
			ocpp_configurationKeyText[OCPP_CK_MeterValuesAlignedDataMaxLength], 64);

	// 9.1.15. MeterValuesSampledData
	OCPP_ConfigurationKey[OCPP_CK_MeterValuesSampledData].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_MeterValuesSampledData].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_MeterValuesSampledData].dataType = OCPP_CK_DT_STRING;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_MeterValuesSampledData].key,
			ocpp_configurationKeyText[OCPP_CK_MeterValuesSampledData], 64);
	strncpy(OCPP_ConfigurationKey[OCPP_CK_MeterValuesSampledData].type.stringData,
			"Voltage,Current.Import,Energy.Active.Import.Register,SoC,Current.Offered,Power.Active.Import,Energy.Reactive.Import.Register", 512);

	// 9.1.16. MeterValuesSampledDataMaxLen
	OCPP_ConfigurationKey[OCPP_CK_MeterValuesSampledDataMaxLength].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_MeterValuesSampledDataMaxLength].accessibility = OCPP_ACC_READONLY;
	OCPP_ConfigurationKey[OCPP_CK_MeterValuesSampledDataMaxLength].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_MeterValuesSampledDataMaxLength].type.intData = 10;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_MeterValuesSampledDataMaxLength].key,
			ocpp_configurationKeyText[OCPP_CK_MeterValuesSampledDataMaxLength], 64);

	// 9.1.17. MeterValueSampleInterval
	OCPP_ConfigurationKey[OCPP_CK_MeterValueSampleInterval].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_MeterValueSampleInterval].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_MeterValueSampleInterval].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_MeterValueSampleInterval].type.intData = 10;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_MeterValueSampleInterval].key,
			ocpp_configurationKeyText[OCPP_CK_MeterValueSampleInterval], 64);

	// 9.1.18. MinimumStatusDuration
	OCPP_ConfigurationKey[OCPP_CK_MinimumStatusDuration].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_MinimumStatusDuration].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_MinimumStatusDuration].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_MinimumStatusDuration].type.intData = 600;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_MinimumStatusDuration].key,
			ocpp_configurationKeyText[OCPP_CK_MinimumStatusDuration], 64);
	// 9.1.19. NumberOfConnectors
	OCPP_ConfigurationKey[OCPP_CK_NumberOfConnectors].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_NumberOfConnectors].accessibility = OCPP_ACC_READONLY;
	OCPP_ConfigurationKey[OCPP_CK_NumberOfConnectors].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_NumberOfConnectors].type.intData = 2;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_NumberOfConnectors].key,
			ocpp_configurationKeyText[OCPP_CK_NumberOfConnectors], 64);
	// 9.1.20. ResetRetries
	OCPP_ConfigurationKey[OCPP_CK_ResetRetries].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_ResetRetries].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_ResetRetries].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_ResetRetries].type.intData = 3;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_ResetRetries].key,
			ocpp_configurationKeyText[OCPP_CK_ResetRetries], 64);
	// 9.1.21. ConnectorPhaseRotation
	OCPP_ConfigurationKey[OCPP_CK_ConnectorPhaseRotation].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_ConnectorPhaseRotation].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_ConnectorPhaseRotation].dataType = OCPP_CK_DT_STRING;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_ConnectorPhaseRotation].key,
			ocpp_configurationKeyText[OCPP_CK_ConnectorPhaseRotation], 64);
	strncpy(OCPP_ConfigurationKey[OCPP_CK_ConnectorPhaseRotation].type.stringData, "NotApplicable", 512);

	// 9.1.22. ConnectorPhaseRotationMaxLength
	OCPP_ConfigurationKey[OCPP_CK_ConnectorPhaseRotationMaxLength].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_ConnectorPhaseRotationMaxLength].accessibility = OCPP_ACC_READONLY;
	OCPP_ConfigurationKey[OCPP_CK_ConnectorPhaseRotationMaxLength].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_ConnectorPhaseRotationMaxLength].type.intData = 0;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_ConnectorPhaseRotationMaxLength].key,
			ocpp_configurationKeyText[OCPP_CK_ConnectorPhaseRotationMaxLength], 64);

	// 9.1.23. StopTransactionOnEVSideDisconnect
	OCPP_ConfigurationKey[OCPP_CK_StopTransactionOnEVSideDisconnect].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_StopTransactionOnEVSideDisconnect].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_StopTransactionOnEVSideDisconnect].dataType = OCPP_CK_DT_BOOLEAN;
	OCPP_ConfigurationKey[OCPP_CK_StopTransactionOnEVSideDisconnect].type.boolData = true;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_StopTransactionOnEVSideDisconnect].key,
			ocpp_configurationKeyText[OCPP_CK_StopTransactionOnEVSideDisconnect], 64);

	// 9.1.24. StopTransactionOnInvalidId
	OCPP_ConfigurationKey[OCPP_CK_StopTransactionOnInvalidId].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_StopTransactionOnInvalidId].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_StopTransactionOnInvalidId].dataType = OCPP_CK_DT_BOOLEAN;
	OCPP_ConfigurationKey[OCPP_CK_StopTransactionOnInvalidId].type.boolData = true;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_StopTransactionOnInvalidId].key,
			ocpp_configurationKeyText[OCPP_CK_StopTransactionOnInvalidId], 64);

	// 9.1.25. StopTxnAlignedData
	OCPP_ConfigurationKey[OCPP_CK_StopTxnAlignedData].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_StopTxnAlignedData].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_StopTxnAlignedData].dataType = OCPP_CK_DT_STRING;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_StopTxnAlignedData].key,
			ocpp_configurationKeyText[OCPP_CK_StopTxnAlignedData], 64);
	strncpy(OCPP_ConfigurationKey[OCPP_CK_StopTxnAlignedData].type.stringData,
			"Voltage,Current.Import,Energy.Active.Import.Register,SoC,Current.Offered,Power.Active.Import,Energy.Reactive.Import.Register", 512);

	// 9.1.26. StopTxnAlignedDataMaxLength
	OCPP_ConfigurationKey[OCPP_CK_StopTxnAlignedDataMaxLength].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_StopTxnAlignedDataMaxLength].accessibility = OCPP_ACC_READONLY;
	OCPP_ConfigurationKey[OCPP_CK_StopTxnAlignedDataMaxLength].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_StopTxnAlignedDataMaxLength].type.intData = 10;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_StopTxnAlignedDataMaxLength].key,
			ocpp_configurationKeyText[OCPP_CK_StopTxnAlignedDataMaxLength], 64);
	// 9.1.27. StopTxnSampledData
	OCPP_ConfigurationKey[OCPP_CK_StopTxnSampledData].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_StopTxnSampledData].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_StopTxnSampledData].dataType = OCPP_CK_DT_STRING;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_StopTxnSampledData].key,
			ocpp_configurationKeyText[OCPP_CK_StopTxnSampledData], 64);
	strncpy(OCPP_ConfigurationKey[OCPP_CK_StopTxnSampledData].type.stringData,
			"Voltage,Current.Import,Energy.Active.Import.Register,SoC", 512);

	// 9.1.28. StopTxnSampledDataMaxLength
	OCPP_ConfigurationKey[OCPP_CK_StopTxnSampledDataMaxLength].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_StopTxnSampledDataMaxLength].accessibility = OCPP_ACC_READONLY;
	OCPP_ConfigurationKey[OCPP_CK_StopTxnSampledDataMaxLength].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_StopTxnSampledDataMaxLength].type.intData = 10;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_StopTxnSampledDataMaxLength].key,
			ocpp_configurationKeyText[OCPP_CK_StopTxnSampledDataMaxLength], 64);
	// 9.1.29. SupportedFeatureProfiles
	OCPP_ConfigurationKey[OCPP_CK_SupportedFeatureProfiles].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_SupportedFeatureProfiles].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_SupportedFeatureProfiles].dataType = OCPP_CK_DT_STRING;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_SupportedFeatureProfiles].key,
			ocpp_configurationKeyText[OCPP_CK_SupportedFeatureProfiles], 64);
	strncpy(OCPP_ConfigurationKey[OCPP_CK_SupportedFeatureProfiles].type.stringData,
			"Core,FirmwareManagement,LocalAuthListManagement,Reservation,SmartCharging,RemoteTrigger", 512);

	// 9.1.30. SupportedFeatureProfilesMaxLength
	OCPP_ConfigurationKey[OCPP_CK_SupportedFeatureProfilesMaxLength].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_SupportedFeatureProfilesMaxLength].accessibility = OCPP_ACC_READONLY;
	OCPP_ConfigurationKey[OCPP_CK_SupportedFeatureProfilesMaxLength].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_SupportedFeatureProfilesMaxLength].type.intData = 6;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_SupportedFeatureProfilesMaxLength].key,
			ocpp_configurationKeyText[OCPP_CK_SupportedFeatureProfilesMaxLength], 64);
	// 9.1.31. TransactionMessageAttempts
	OCPP_ConfigurationKey[OCPP_CK_TransactionMessageAttempt].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_TransactionMessageAttempt].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_TransactionMessageAttempt].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_TransactionMessageAttempt].type.intData = 3;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_TransactionMessageAttempt].key,
			ocpp_configurationKeyText[OCPP_CK_TransactionMessageAttempt], 64);
	// 9.1.32. TransactionMessageRetryInterva
	OCPP_ConfigurationKey[OCPP_CK_TransactionMessageRetryInterval].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_TransactionMessageRetryInterval].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_TransactionMessageRetryInterval].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_TransactionMessageRetryInterval].type.intData = 15;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_TransactionMessageRetryInterval].key,
			ocpp_configurationKeyText[OCPP_CK_TransactionMessageRetryInterval], 64);
	// 9.1.33. UnlockConnectorOnEVSideDisconnect
	OCPP_ConfigurationKey[OCPP_CK_UnlockConnectorOnEVSideDisconnect].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_UnlockConnectorOnEVSideDisconnect].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_UnlockConnectorOnEVSideDisconnect].dataType = OCPP_CK_DT_BOOLEAN;
	OCPP_ConfigurationKey[OCPP_CK_UnlockConnectorOnEVSideDisconnect].type.boolData = true;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_UnlockConnectorOnEVSideDisconnect].key,
			ocpp_configurationKeyText[OCPP_CK_UnlockConnectorOnEVSideDisconnect], 64);
	// 9.1.34. WebSocketPingInterva
	OCPP_ConfigurationKey[OCPP_CK_WebSocketPingInterval].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_WebSocketPingInterval].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_WebSocketPingInterval].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_WebSocketPingInterval].type.intData = 10;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_WebSocketPingInterval].key,
			ocpp_configurationKeyText[OCPP_CK_WebSocketPingInterval], 64);
	// 9.2.1. LocalAuthListEnabled
	OCPP_ConfigurationKey[OCPP_CK_LocalAuthListEnabled].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_LocalAuthListEnabled].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_LocalAuthListEnabled].dataType = OCPP_CK_DT_BOOLEAN;
	OCPP_ConfigurationKey[OCPP_CK_LocalAuthListEnabled].type.boolData = true;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_LocalAuthListEnabled].key,
			ocpp_configurationKeyText[OCPP_CK_LocalAuthListEnabled], 64);
	// 9.2.2. LocalAuthListMaxLength
	OCPP_ConfigurationKey[OCPP_CK_LocalAuthListMaxLength].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_LocalAuthListMaxLength].accessibility = OCPP_ACC_READONLY;
	OCPP_ConfigurationKey[OCPP_CK_LocalAuthListMaxLength].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_LocalAuthListMaxLength].type.intData = 50;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_LocalAuthListMaxLength].key,
			ocpp_configurationKeyText[OCPP_CK_LocalAuthListMaxLength], 64);
	// 9.2.3. SendLocalListMaxLength
	OCPP_ConfigurationKey[OCPP_CK_SendLocalListMaxLength].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_SendLocalListMaxLength].accessibility = OCPP_ACC_READONLY;
	OCPP_ConfigurationKey[OCPP_CK_SendLocalListMaxLength].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_SendLocalListMaxLength].type.boolData = 10;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_SendLocalListMaxLength].key,
			ocpp_configurationKeyText[OCPP_CK_SendLocalListMaxLength], 64);
	// 9.3.1. ReserveConnectorZeroSupported
	OCPP_ConfigurationKey[OCPP_CK_ReserveConnectorZeroSupported].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_ReserveConnectorZeroSupported].accessibility = OCPP_ACC_READONLY;
	OCPP_ConfigurationKey[OCPP_CK_ReserveConnectorZeroSupported].dataType = OCPP_CK_DT_BOOLEAN;
	OCPP_ConfigurationKey[OCPP_CK_ReserveConnectorZeroSupported].type.boolData = true;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_ReserveConnectorZeroSupported].key,
			ocpp_configurationKeyText[OCPP_CK_ReserveConnectorZeroSupported], 64);
	// 9.4.1. ChargeProfileMaxStackLevel
	OCPP_ConfigurationKey[OCPP_CK_ChargeProfileMaxStackLevel].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_ChargeProfileMaxStackLevel].accessibility = OCPP_ACC_READONLY;
	OCPP_ConfigurationKey[OCPP_CK_ChargeProfileMaxStackLevel].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_ChargeProfileMaxStackLevel].type.intData = 3;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_ChargeProfileMaxStackLevel].key,
			ocpp_configurationKeyText[OCPP_CK_ChargeProfileMaxStackLevel], 64);
	// 9.4.2. ChargingScheduleAllowedChargingRateUnit
	OCPP_ConfigurationKey[OCPP_CK_ChargingScheduleAllowedChargingRateUnit].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_ChargingScheduleAllowedChargingRateUnit].accessibility = OCPP_ACC_READONLY;
	OCPP_ConfigurationKey[OCPP_CK_ChargingScheduleAllowedChargingRateUnit].dataType = OCPP_CK_DT_STRING;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_ChargingScheduleAllowedChargingRateUnit].key,
			ocpp_configurationKeyText[OCPP_CK_ChargingScheduleAllowedChargingRateUnit], 64);
	strncpy(OCPP_ConfigurationKey[OCPP_CK_ChargingScheduleAllowedChargingRateUnit].type.stringData, "Current,Power", 512);

	// 9.4.3. ChargingScheduleMaxPeriods
	OCPP_ConfigurationKey[OCPP_CK_ChargingScheduleMaxPeriods].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_ChargingScheduleMaxPeriods].accessibility = OCPP_ACC_READONLY;
	OCPP_ConfigurationKey[OCPP_CK_ChargingScheduleMaxPeriods].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_ChargingScheduleMaxPeriods].type.intData = 10;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_ChargingScheduleMaxPeriods].key,
			ocpp_configurationKeyText[OCPP_CK_ChargingScheduleMaxPeriods], 64);

	// 9.4.4. ConnectorSwitch3to1PhaseSupported
	OCPP_ConfigurationKey[OCPP_CK_ConnectorSwitch3to1PhaseSupported].isUsed = false;
	OCPP_ConfigurationKey[OCPP_CK_ConnectorSwitch3to1PhaseSupported].accessibility = OCPP_ACC_READONLY;
	OCPP_ConfigurationKey[OCPP_CK_ConnectorSwitch3to1PhaseSupported].dataType = OCPP_CK_DT_BOOLEAN;
	OCPP_ConfigurationKey[OCPP_CK_ConnectorSwitch3to1PhaseSupported].type.boolData = true;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_ConnectorSwitch3to1PhaseSupported].key,
			ocpp_configurationKeyText[OCPP_CK_ConnectorSwitch3to1PhaseSupported], 64);

	// 9.4.5. MaxChargingProfilesInstalled
	OCPP_ConfigurationKey[OCPP_CK_MaxChargingProfilesInstalled].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_MaxChargingProfilesInstalled].accessibility = OCPP_ACC_READONLY;
	OCPP_ConfigurationKey[OCPP_CK_MaxChargingProfilesInstalled].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_MaxChargingProfilesInstalled].type.intData = 20;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_MaxChargingProfilesInstalled].key,
			ocpp_configurationKeyText[OCPP_CK_MaxChargingProfilesInstalled], 64);
}

static int ocpp_ConfigurationKey_default_init(OCPP_ConfigurationKey_t *OCPP_ConfigurationKey, int size)
{
	if (ocpp_CK == NULL || OCPP_ConfigurationKey == NULL || size <= 0)
	{
		return -1; // 输入参数错误
	}

	int rc;
	char sql[512];
	// 查询数据库中是否有数据
	char checkQuery[512];
	snprintf(checkQuery, sizeof(checkQuery), "SELECT COUNT(*) FROM ConfigurationKeys");

	sqlite3_stmt *checkStmt;
	rc = sqlite3_prepare_v2(ocpp_CK, checkQuery, -1, &checkStmt, NULL);
	if (rc != SQLITE_OK)
	{
		printf("Error preparing SQL: %s\n", sqlite3_errmsg(ocpp_CK));
		return -1; // 处理数据库查询准备失败的情况
	}

	rc = sqlite3_step(checkStmt);
	int rowCount = sqlite3_column_int(checkStmt, 0);
	sqlite3_finalize(checkStmt);

	// 如果数据库中有数据，清空表格
	if (rowCount > 0)
	{
		char clearTableQuery[] = "DELETE FROM ConfigurationKeys";
		rc = sqlite3_exec(ocpp_CK, clearTableQuery, NULL, NULL, NULL);
		if (rc != SQLITE_OK)
		{
			return -1; // 处理表格清空失败的情况
		}
	}

	for (int i = 0; i < size; i++)
	{
		// 根据联合体类型将值转换为文本
		char valueText[512];
		switch (OCPP_ConfigurationKey[i].dataType)
		{
		case 0:
			snprintf(valueText, sizeof(valueText), "%d", OCPP_ConfigurationKey[i].type.boolData);
			break;
		case 1:
			snprintf(valueText, sizeof(valueText), "%d", OCPP_ConfigurationKey[i].type.intData);
			break;
		case 2:
			snprintf(valueText, sizeof(valueText), "%s", OCPP_ConfigurationKey[i].type.stringData);
			break;
		default:
			// 处理未知数据类型的情况
			snprintf(valueText, sizeof(valueText), "unknown");
			break;
		}

		// 构建插入 SQL 语句，将 Value 设置为转换后的文本
		snprintf(sql, 512, "INSERT INTO ConfigurationKeys (Key,Value,Type,Accessibility,IsUse) VALUES ('%s', '%s', %d, %d, %d)",
				 OCPP_ConfigurationKey[i].key, valueText, OCPP_ConfigurationKey[i].dataType, OCPP_ConfigurationKey[i].accessibility, OCPP_ConfigurationKey[i].isUsed);

		rc = sqlite3_exec(ocpp_CK, sql, NULL, NULL, NULL);
		if (rc != SQLITE_OK)
		{
			return -1; // 插入失败
		}
	}

	return 0; // 成功插入默认数据
}
/**
 * @description:
 * @param
 * @return
 */
int ocpp_ConfigurationKey_deinit(void)
{
	if (ocpp_CK == NULL)
	{
		return 0; // 处理错误情况，例如数据库未初始化或传入了无效的键
	}
	char *sql = "DELETE FROM ConfigurationKeys;";
	int rc = sqlite3_exec(ocpp_CK, sql, NULL, 0, NULL);
	if (rc != SQLITE_OK)
	{
		printf("清空ConfigurationKeys表失败\n");
		return -1;
	}
	OCPP_ConfigurationKey_t OCPP_ConfigurationKey[OCPP_CONFIGURATION_KEY_SIZE];
	// 初始化默认配置数据
	ocpp_ConfiguartionKey_data_init(OCPP_ConfigurationKey);
	ocpp_ConfigurationKey_default_init(OCPP_ConfigurationKey, OCPP_CK_MAXLEN);
}
/**
 * @description: 判断这个key值在数据库是否存在
 * @param {char *} key
 * @return {*} 1 = 存在,0 = 不存在
 */

bool ocpp_ConfigurationKey_isFound(const char *key)
{
	if (ocpp_CK == NULL || key == NULL)
	{
		return 0; // 处理错误情况，例如数据库未初始化或传入了无效的键
	}

	// 查询是否存在指定的键
	char *sql = sqlite3_mprintf("SELECT Accessibility FROM ConfigurationKeys WHERE Key='%q'", key);
	if (sql == NULL)
	{
		return -1; // 或者其他适当的错误值
	}
	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(ocpp_CK, sql, -1, &stmt, NULL);
	sqlite3_free(sql);
	if (rc != SQLITE_OK)
	{
		printf("Error preparing SQL: %s\n", sqlite3_errmsg(ocpp_CK));
		return 0; // 处理数据库查询准备失败的情况
	}

	rc = sqlite3_step(stmt);
	if (rc == SQLITE_ROW)
	{
		int count = sqlite3_column_int(stmt, 0);
		sqlite3_finalize(stmt);
		return (count > 0) ? 1 : 0; // 如果存在返回1，否则返回0
	}

	sqlite3_finalize(stmt);
	return 0; // 出现错误或未找到匹配的记录
}

/**
 * @description: 获取配置key的数据
 * @param {char *} key
 * @param {void *} value
 * @return {*} 返回value的数据类型,-1 = Error
 */

int ocpp_ConfigurationKey_getValue(const char *key, void *value)
{
	if (ocpp_CK == NULL || key == NULL || value == NULL)
	{
		return -1; // 处理错误情况，例如数据库未初始化、传入了无效的键或无效的值指针
	}

	int type = -1;
	char dbValue[500];

	char *sql = sqlite3_mprintf("SELECT Value, Type FROM ConfigurationKeys WHERE Key COLLATE NOCASE = '%q'", key);
	if (sql == NULL)
	{
		return -1; // 或者其他适当的错误值
	}
	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(ocpp_CK, sql, -1, &stmt, NULL);
	sqlite3_free(sql);
	if (rc != SQLITE_OK)
	{
		printf("Error preparing SQL: %s\n", sqlite3_errmsg(ocpp_CK));
		return -1; // 处理数据库查询准备失败的情况
	}

	rc = sqlite3_step(stmt);
	if (rc == SQLITE_ROW)
	{
		snprintf(dbValue, 500, "%s", (char *)sqlite3_column_text(stmt, 0));
		type = sqlite3_column_int(stmt, 1);
	}

	sqlite3_finalize(stmt);

	if (type == -1 || dbValue == NULL)
	{
		return -1; // 没有找到匹配的记录或值为空
	}
	// 根据Type将数据库中的值转换为对应的类型
	switch (type)
	{
	case 0: // 布尔类型
	case 1: // 整数类型
		*((int *)value) = atoi(dbValue);
		break;
	case 2: // 字符串类型
		snprintf((char *)value, 500, "%s", dbValue);
		break;
	default:
		return -1; // 未知类型
	}

	return type; // 返回Type的值
}

/**
 * @description: 判断配置是否使用
 * @param {char *} key = "key名"
 * @return {*} true = 使用, false = 未使用, -1 = Error
 */
int ocpp_ConfigurationKey_getIsUse(const char *key)
{
	if (ocpp_CK == NULL || key == NULL)
	{
		return -1; // 处理错误情况，例如数据库未初始化或传入了无效的键
	}

	int isUse = 0;

	char *sql = sqlite3_mprintf("SELECT IsUse FROM ConfigurationKeys WHERE Key='%q' COLLATE NOCASE", key);
	if (sql == NULL)
	{
		return -1; // 或者其他适当的错误值
	}
	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(ocpp_CK, sql, -1, &stmt, NULL);
	sqlite3_free(sql);
	if (rc != SQLITE_OK)
	{
		printf("Error preparing SQL: %s\n", sqlite3_errmsg(ocpp_CK));
		return -1; // 处理数据库查询准备失败的情况
	}

	rc = sqlite3_step(stmt);
	if (rc == SQLITE_ROW)
	{
		isUse = sqlite3_column_int(stmt, 0);
	}

	sqlite3_finalize(stmt);
	return isUse;
}

/**
 * @description: 获取配置的读写权限
 * @param {char *} key = "key名"
 * @return {*}
 *             0 = 只读,
 *             1 = 只写,
 *             2 = 读写
 *            -1 = Error
 */
int ocpp_ConfigurationKey_getAcc(const char *key)
{
	if (ocpp_CK == NULL || key == NULL)
	{
		return -1; // 或者其他适当的错误值
	}

	char *sql = sqlite3_mprintf("SELECT Accessibility FROM ConfigurationKeys WHERE Key='%q' COLLATE NOCASE", key);
	if (sql == NULL)
	{
		return -1; // 或者其他适当的错误值
	}

	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(ocpp_CK, sql, -1, &stmt, NULL);
	sqlite3_free(sql);
	if (rc != SQLITE_OK)
	{
		printf("SQL error: %s\n", sqlite3_errmsg(ocpp_CK));
		return -1; // 或者其他适当的错误值
	}

	int access = 0;
	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		access = sqlite3_column_int(stmt, 0);
	}

	sqlite3_finalize(stmt);
	return access;
}

/**
 * @description: 获取配置key的数据类型
 * @param {char *} key = "key名"
 * @return {*}0 = bool
 *            1 = int
 *            2  = string
 *            -1 = Error
 */
int ocpp_ConfigurationKey_getType(const char *key)
{
	if (ocpp_CK == NULL || key == NULL)
	{
		// 处理错误情况，例如数据库未初始化或传入了无效的键
		return -1;
	}

	int dataType = -1;

	char *sql = sqlite3_mprintf("SELECT Type FROM ConfigurationKeys WHERE Key='%q' COLLATE NOCASE", key);
	if (sql == NULL)
	{
		return -1; // 或者其他适当的错误值
	}

	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(ocpp_CK, sql, -1, &stmt, NULL);
	sqlite3_free(sql);
	if (rc != SQLITE_OK)
	{
		printf("Error preparing SQL: %s\n", sqlite3_errmsg(ocpp_CK));
		sqlite3_finalize(stmt); // 释放资源
		return -1;				// 处理数据库查询准备失败的情况
	}

	rc = sqlite3_step(stmt);
	if (rc == SQLITE_ROW)
	{
		dataType = sqlite3_column_int(stmt, 0);
	}
	else if (rc != SQLITE_DONE)
	{
		printf("Error executing SQL: %s\n", sqlite3_errmsg(ocpp_CK));
		// 在这里处理执行 SQL 失败的情况
	}

	sqlite3_finalize(stmt);

	return dataType;
}

/**
 * @description: 将Configurationkey插入数据库
 * @param:
 * @return: 0 = success, -1 = fail
 */
int ocpp_ConfigurationKey_InsertSQLite(const char *key, const char *value, char type, char accessibility, int isUse)
{
    if (ocpp_CK == NULL)
        return -1;
    if (key == NULL)
        return -1;
    int rc;
    char sql[512];

    snprintf(sql, 512, "INSERT INTO ConfigurationKeys (Key,Value,Type,Accessibility,IsUse) VALUES ('%s','%s','%c','%c',%d)",
             key, value, type, accessibility, isUse);

    rc = sqlite3_exec(ocpp_CK, sql, NULL, NULL, NULL);
    if (rc != SQLITE_OK)
    {
        printf("插入数据失败\n");
        return -1;
    }

    return 0;
}

static int ocpp_ConfigurationKey_Create_Table(sqlite3 *p_db)
{
    int rc = 0;
    char sql[512];

    snprintf(sql, sizeof(sql), "CREATE TABLE IF NOT EXISTS ConfigurationKeys("
                               "ID             integer  PRIMARY KEY,"
                               "Key            TEXT     NOT NULL,"
                               "Value          TEXT     default NULL,"
                               "Type           integer     NOT NULL,"
                               "Accessibility  integer     NOT NULL,"
                               "IsUse          integer  NOT NULL);");

    rc = sqlite3_exec(p_db, sql, NULL, 0, NULL);
    if (rc != SQLITE_OK)
    {
        printf("创建 ConfigurationKeys 表失败\n");
        return -1;
    }

    return 0;
}


/**
 * @description:更新配置的值和可用性
 * @param {char *} key
 * @param {char *} value
 * @param {int} isUse
 * @return {*} 0 = success,1 = fail
 */
int ocpp_ConfigurationKey_Modify(const char *key, const char *value, int isUse)
{
	if (ocpp_CK == NULL || key == NULL || value == NULL)
	{
		return -1; // 输入参数错误
	}

	int rc;
	char sql[512];
	memset(sql, 0, sizeof(sql));

	snprintf(sql, 512, "UPDATE ConfigurationKeys SET Value='%s', IsUse=%d WHERE Key='%s'", value, isUse, key);

	rc = sqlite3_exec(ocpp_CK, sql, NULL, NULL, NULL);
	if (rc != SQLITE_OK)
	{
		return -1; // 更新失败
	}

	return 0; // 成功更新值和IsUse
}

int ocpp_ConfigurationKey_init(sqlite3 *ocpp_db)
{

	ocpp_CK = ocpp_db;

	// 检查配置表是否存在
	if (ocpp_ConfigurationKey_TableExists(ocpp_CK))
	{
		return 0; // 表已存在，无需进一步初始化
	}

	// 创建配置表
	if (ocpp_ConfigurationKey_Create_Table(ocpp_CK) == -1)
	{
		return -1; // 创建表失败
	}

	ocpp_ConfigurationKey_deinit();

	return 0; // 初始化成功
}