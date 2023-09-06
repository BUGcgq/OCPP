#ifndef __OCPP_OFFLINE__H__
#define __OCPP_OFFLINE__H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sqlite3.h>
#include "ocpp_package.h"
#include <pthread.h>

#define START_STOP_TRANSACTION_MAX_ROWS 100
#define METER_VALUES_MAX_ROWS 1000


int ocpp_StartTransaction_insert(int transactionId, int connectorId, const char *idTag, int meterStart, int reservationId, const char *timestamp, int isCompleted, const char *uniqueId, int Offline);
int ocpp_MeterValues_insert(int transactionId, int connectorId, const char *meterValue, int isSent, const char *uniqueId);
int ocpp_StopTransaction_insert(int transactionId, const char *idTag, const char *timestamp, int meterStop, const char *reason, const char *transactionData, const char *uniqueId);
int updateTransactionIDsByUniqueID(const char *uniqueId, int newTransactionId);
int updateTransactionIDs(int oldTransactionId, int newTransactionId);
int updateStartTransactionIsCompletedByUniqueID(const char *uniqueId, int isCompleted);
int updateStartTransactionIsSentByUniqueId(const char *uniqueId, int isSent);
void ocpp_order_init(sqlite3 *ocpp_db);

#ifdef __cplusplus
}
#endif

#endif