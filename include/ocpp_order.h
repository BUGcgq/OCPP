#ifndef __OCPP_OFFLINE__H__
#define __OCPP_OFFLINE__H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sqlite3.h>
#include "ocpp_package.h"
#include <pthread.h>

typedef struct {
    int ID;
    int TransactionID;
    int ConnectorID;
    char idTag[22];
    int MeterStart;
    int MeterStop;
    char StartTimes[32];
    char StopTimes[32];
    char StartUniqueID[32];
    char StopUniqueID[32];
    int Status;
    int Reason;
    int ReservationId;
    int Completed;
} TransactionRecord;
// 定义数据记录上限和要删除的最旧记录数
#define DATA_LIMIT 1000
#define DELETE_LIMIT 100

int ocpp_Transaction_insert(int TransactionID, int ConnectorID, const char *IdTag, int MeterStart, const char *StartTimes, const char *StartUniqueID, int Status, int Reason, int ReservationId);

int ocpp_Transaction_update(int TransactionID, int MeterStop, const char *StopTimes, const char *StopUniqueID, int Status, int Reason);

int ocpp_MeterValues_insert(int TransactionID, const char *meterValue);

int ocpp_UpdateTransactionIDByUniqueID(const char *UniqueID, int NewTransactionID);

int ocpp_UpdateStatusByUniqueID(const char *UniqueID, int NewStatus);

int ocpp_UpdateCompletedByUniqueID(const char *UniqueID, int NewCompleted);

int ocpp_ReadSingleIncompleteTransaction(TransactionRecord *transaction);

void ocpp_order_init(sqlite3 *ocpp_db);

#ifdef __cplusplus
}
#endif

#endif