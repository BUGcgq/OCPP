/*
 * @Author: LIYAOHAN 1791002655@qq.com
 * @Date: 2023-04-01 15:34:31
 * @LastEditors: LIYAOHAN 1791002655@qq.com
 * @LastEditTime: 2023-05-07 10:50:17
 * @FilePath: /OCPP/ocpp_localAuthorization.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __OCPP_LOCAL_AUTHORIZATION__H__
#define __OCPP_LOCAL_AUTHORIZATION__H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <sqlite3.h>
#include "ocpp_config.h"





//7.2. AuthorizationStatus

enum OCPP_LOCAL_AUTHORIZATION_STATUS_E {
    OCPP_LOCAL_AUTHORIZATION_ACCEPTED = 0,            // 授权已接受，允许充电
    OCPP_LOCAL_AUTHORIZATION_BLOCKED,                 // 授权已被阻止，不允许充电
    OCPP_LOCAL_AUTHORIZATION_EXPIRED,                 // 授权已过期，不允许充电
    OCPP_LOCAL_AUTHORIZATION_INVALID,                 // 授权无效，不允许充电
    OCPP_LOCAL_AUTHORIZATION_CONCURRENTTX,            // 授权已参与另一个交易，不允许多个交易
                                                      // （仅适用于 StartTransaction.req）

    OCPP_LOCAL_AUTHORIZATION_MAX
};


static const char * ocpp_localAuthorizationStatus_Text[] = {
    "Accepted",
    "Blocked",
    "Expired",
    "Invalid",
    "ConcurrentTx"
};


typedef struct{
	char IdTag[OCPP_AUTHORIZATION_IDTAG_LEN];                                        //卡号
	enum OCPP_LOCAL_AUTHORIZATION_STATUS_E status;                                   //卡状态
	struct tm *expiryDate;                                                           //有效期
	char parentIdTag[OCPP_AUTHORIZATION_IDTAG_LEN];                                  //父卡号

}ocpp_localAuthorization_cache_record_t;



typedef struct{
    char IdTag[OCPP_AUTHORIZATION_IDTAG_LEN];                                       //卡号
    enum OCPP_LOCAL_AUTHORIZATION_STATUS_E status;                                    //卡状态
    struct tm *expiryDate;                                                          //有效期
    char parentIdTag[OCPP_AUTHORIZATION_IDTAG_LEN];                                 //父卡号

}ocpp_localAuthorization_list_entry_t;


int ocpp_localAuthorization_Cache_clearCache(void);
int ocpp_localAuthorization_Cache_write(ocpp_localAuthorization_cache_record_t * cache_record);
bool ocpp_localAuthorization_Cache_isVaild(const char * idTag);
// char **ocpp_localAuthorization_Cache_getAllInvaild(int * inVaildCnt);
bool ocpp_localAuthorization_Cache_search(const char * idTag);



int ocpp_localAuthorization_List_clearList(void);
int ocpp_localAuthorization_List_write(ocpp_localAuthorization_list_entry_t * list_entry);
bool ocpp_localAuthorization_List_isVaild(const char * idTag);
int ocpp_localAuthorization_List_getListVersion2(void);
int ocpp_localAuthorization_List_setListVersion(int version);
bool ocpp_localAuthorization_List_search(const char * idTag);



void ocpp_localAuthorization_init(sqlite3 *ocpp_db);


#ifdef __cplusplus
}
#endif

#endif