#ifndef __OCPP_LOCAL_AUTHORIZATION__H__
#define __OCPP_LOCAL_AUTHORIZATION__H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <sqlite3.h>
#include "ocpp_config.h"
#include "ocpp_chargePoint.h"




//7.2. AuthorizationStatus



static const char * ocpp_localAuthorizationStatus_Text[] = {
    "Accepted",
    "Blocked",
    "Expired",
    "Invalid",
    "ConcurrentTx"
};



typedef struct{
    char IdTag[OCPP_AUTHORIZATION_IDTAG_LEN];                                       //卡号
    enum OCPP_LOCAL_AUTHORIZATION_STATUS_E status;                                    //卡状态
    char  expiryDate[32];                                                          //有效期
    char parentIdTag[OCPP_AUTHORIZATION_IDTAG_LEN];                                 //父卡号

}ocpp_localAuthorization_list_entry_t;


int ocpp_localAuthorization_Cache_clearCache(void);
int ocpp_localAuthorization_Cache_write(ocpp_localAuthorization_cache_record_t * cache_record);
bool ocpp_localAuthorization_Cache_isValid(const char *idTag);
bool ocpp_localAuthorization_Cache_search(const char * idTag);



int ocpp_localAuthorization_List_clearList(void);
int ocpp_localAuthorization_List_write(ocpp_localAuthorization_list_entry_t * list_entry);
int ocpp_localAuthorization_Version_setVersion(const char *tableName, int version);
int ocpp_localAuthorization_List_getListVersion(const char *tableName);
int ocpp_localAuthorization_List_clearList(void);
bool ocpp_localAuthorization_List_search(const char * idTag);
int ocpp_localAuthorization_List_RecordCount(void);
bool ocpp_localAuthorization_List_isValid(const char *idTag);

void ocpp_localAuthorization_init(sqlite3 *ocpp_db);


#ifdef __cplusplus
}
#endif

#endif