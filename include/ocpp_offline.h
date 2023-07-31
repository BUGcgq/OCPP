#ifndef __OCPP_OFFLINE__H__
#define __OCPP_OFFLINE__H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sqlite3.h>
#include "ocpp_package.h"

typedef struct{
    enum OCPP_PACKAGE Action;
    char *unique;
    char *message;

}ocpp_offline_offlineData_item_t;









void ocpp_offline_deleteOfflineData(const char * uniqueId);
ocpp_offline_offlineData_item_t * ocpp_offline_readAData();
void ocpp_offline_freeItem(ocpp_offline_offlineData_item_t ** item, int number);
ocpp_offline_offlineData_item_t ** ocpp_offline_readNData(int * number);
void ocpp_offline_deleteNData(int number);

int ocpp_offline_OfflineDatasCnt();
void ocpp_offline_send(ocpp_offline_offlineData_item_t * item);
void ocpp_offline_init(sqlite3 *ocpp_db);


#ifdef __cplusplus
}
#endif

#endif