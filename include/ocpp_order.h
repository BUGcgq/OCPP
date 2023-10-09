#ifndef __OCPP_OFFLINE__H__
#define __OCPP_OFFLINE__H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sqlite3.h>
#include "ocpp_package.h"
#include <pthread.h>

// #define MAX_TRANSACTION_RECORDS 1000 // 定义最大记录数限制
// #define MAX_DELETE_RECORDS 100         // 定义一次删除的最大记录数

// int ocpp_StartTransaction_insert(int TransactionID, int ConnectorID, const char *IdTag, int MeterStart, const char *StartTimes, const char *StartUniqueID, int IsCompleted, const char *Reason);

// int ocpp_StopTransaction_update(int TransactionID, int MeterStop, const char *StopTimes, const char *StopUniqueID, const char *Reason, const char *MeterValue);

// int ocpp_Transaction_updateTransactionID(const char *StartUniqueID, int NewTransactionID);

// int ocpp_Transaction_updateIsCompleted(const char *StopUniqueID, int NewIsCompleted);
void ocpp_order_init(sqlite3 *ocpp_db);

#ifdef __cplusplus
}
#endif

#endif