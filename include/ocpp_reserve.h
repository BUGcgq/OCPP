/*
 * @Author: LIYAOHAN 1791002655@qq.com
 * @Date: 2023-04-23 16:42:31
 * @LastEditors: LIYAOHAN 1791002655@qq.com
 * @LastEditTime: 2023-05-07 10:52:15
 * @FilePath: /OCPP/ocpp_reserve.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __OCPP_RESERVE__H__
#define __OCPP_RESERVE__H__

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include <sqlite3.h>
#include "ocpp_config.h"

typedef struct{

    int reservationId;
    struct tm expiryDate;
    char idTag[OCPP_AUTHORIZATION_IDTAG_LEN];
    char parentIdTag[OCPP_AUTHORIZATION_IDTAG_LEN];
    
}ocpp_reserve_t;



void ocpp_reserve_updateReservation(int reservationId, int connector, char *expiryDate, char *Idtag, char *parentIdTag);
void ocpp_reserve_removeReservation(int reservationId);
int ocpp_reserve_readReservation(int connectorId, ocpp_reserve_t * reserve);
void ocpp_reserve_init(sqlite3 *ocpp_db);



#ifdef __cplusplus
}
#endif

#endif