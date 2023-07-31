/*
 * @Author: LIYAOHAN 1791002655@qq.com
 * @Date: 2023-04-23 16:42:42
 * @LastEditors: LIYAOHAN 1791002655@qq.com
 * @LastEditTime: 2023-05-08 16:57:43
 * @FilePath: /OCPP/ocpp_reserve.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "ocpp_reserve.h"
#include "ocpp_log.h"

#include <string.h>



static sqlite3 *ocpp_RE;



/**
 * @description: 创建Reservations数据表
 * @param:
 * @return: 0-成功,-1-失败
 */
static int ocpp_reserve_create_table(sqlite3 *p_db){
	char *zErrMsg = 0;
	char sql[512];

    //insert
    snprintf(sql, sizeof(sql), "CREATE TABLE IF NOT EXISTS Reservations("  \
          "ID          integer PRIMARY KEY," \
          "ReservationId      integer    NOT NULL,"  \
          "ConnectorId        integer    NOT NULL,"  \
		  "ExpiryDate         TEXT       NOT NULL,"  \
          "ParentIdTag        TEXT    DEFAULT NULL," \
          "IdTag              TEXT    NOT NULL);");
    
    if(sqlite3_exec(p_db, sql, NULL, 0, &zErrMsg) != SQLITE_OK){
    	OCPP_LOG_DEBUG("创建Reservations表失败 %s",zErrMsg); 
        sqlite3_free(zErrMsg);
        return -1;
    }

    return 0;
}





/**
 * @description:Reservations如果没有相关ReservationId,则插入ReservationId相关信息,如果存在,不进行任何操作
 * @param:
 * @return：
 */
static void ocpp_reserve_insert(int reservationId, int connector, char *expiryDate, char *Idtag, char * parentIdTag){

	sqlite3_stmt *res;
	char *ErrMsg;
	char sql[256] = {0};
	int found = 0;

	snprintf(sql, 256, "SELECT count(*) FROM Reservations where ReservationId=%d;",reservationId);

	int rc = sqlite3_prepare_v2(ocpp_RE,sql,strlen(sql), &res,NULL);

	if (rc != SQLITE_OK){
		OCPP_LOG_ERROR("SELECT count(*) FROM Reservations where ReservationId = %d Fail", reservationId);
		return;
	}

	rc = sqlite3_step(res);
	found = sqlite3_column_int(res,0);
	sqlite3_finalize(res);

	if (found <= 0){
		
		memset(sql,0,256);
		snprintf(sql, 256, "INSERT INTO Reservations (ReservationId, ConnectorId, ExpiryDate, ParentIdTag, IdTag) VALUES("\
		"%d,%d,'%s','%s','%s');",reservationId,connector,expiryDate,parentIdTag,Idtag);

		rc = sqlite3_exec(ocpp_RE, sql, NULL, NULL, &ErrMsg);

		if (rc != SQLITE_OK){
			OCPP_LOG_ERROR("fail insert reserve %d %s", reservationId, ErrMsg);
			sqlite3_free(ErrMsg);
		}
	}
}


/**
 * @description:设置Reservations的ExpiryDate为当前时间
 * @param:
 * @return：
 */
static void ocpp_reserve_closeReservation(int reservationId){
	
	sqlite3_stmt *res;
	char *ErrMsg;
	char sql[256] = {0};

	snprintf(sql, 256, "UPDATE Reservations set ExpiryDate=datetime('now') where reservationId=%d",reservationId);


	if (sqlite3_exec(ocpp_RE, sql, NULL, NULL, &ErrMsg) != SQLITE_OK){
		OCPP_LOG_ERROR("ERROR = %s",ErrMsg);
		sqlite3_free(ErrMsg);
	}
}


/**
 * @description:Reservations如果没有相关ReservationId,则插入ReservationId相关信息,如果存在,则更新
 * @param:
 * @return：
 */
void ocpp_reserve_updateReservation(int reservationId, int connector, char *expiryDate, char *Idtag, char *parentIdTag){

	sqlite3_stmt *res;
	char *ErrMsg;
	char sql[256] = {0};
	int found = 0;

	snprintf(sql, 256, "SELECT count(*) FROM Reservations where ReservationId=%d", reservationId);

	int rc = sqlite3_prepare_v2(ocpp_RE, sql, strlen(sql), &res, NULL);

	if (rc != SQLITE_OK){

		OCPP_LOG_ERROR("Fail SELECT count(*) FROM Reservations where ReservationId=%d", reservationId);
		return;
	}

	rc = sqlite3_step(res);
	found = sqlite3_column_int(res,0);
	sqlite3_finalize(res);

	if (found <= 0){
		OCPP_LOG_DEBUG("insert Reservation");
		ocpp_reserve_insert(reservationId, connector, expiryDate, Idtag, parentIdTag);
	}
	else{
		OCPP_LOG_DEBUG("upDate Reservation");
		snprintf(sql, 256, "UPDATE Reservations set ConnectorId=%d,ExpiryDate='%s',IdTag='%s',ParentIdTag='%s' where ReservationId=%d;",
		connector,expiryDate,Idtag,parentIdTag,reservationId);

		rc = sqlite3_exec(ocpp_RE, sql, NULL, NULL, &ErrMsg);

		if (rc != SQLITE_OK){
			OCPP_LOG_ERROR("Fail update reserve %d %s",reservationId, ErrMsg);
			sqlite3_free(ErrMsg);
		}
	}
}


/**
 * @description:删除Reservations相关ReservationId
 * @param:
 * @return：
 */
void ocpp_reserve_removeReservation(int reservationId){
	
	sqlite3_stmt *res;
	char *ErrMsg;
	char sql[256] = {0};


	snprintf(sql, 256, "DELETE FROM Reservations where ReservationId=%d;",reservationId);

	if (sqlite3_exec(ocpp_RE, sql, NULL, NULL, &ErrMsg) != SQLITE_OK){
	
		OCPP_LOG_ERROR("ERROR = %s", ErrMsg);
	}
}



/**
 * @description:读预约,断电时从数据库中读取有效的预约
 * @param:
 * @return： 0 = success, -1 = fail
 */
int ocpp_reserve_readReservation(int connectorId, ocpp_reserve_t * reserve){
	
	if(reserve == NULL) return -1;
	sqlite3_stmt *res;
	char sql[256] = {0};
	snprintf(sql, 256, "SELECT ReservationId,ExpiryDate,IdTag,ParentIdTag FROM Reservations where ConnectorId = %d;", connectorId);

	int rc = sqlite3_prepare_v2(ocpp_RE, sql, strlen(sql), &res, 0);
	
	if (rc != SQLITE_OK){
		OCPP_LOG_ERROR("查询失败 SELECT ReservationId,ExpiryDate,IdTag FROM Reservations;");
		return -1;
	}

	rc = sqlite3_step(res);

	int result = -1;
	if(rc == SQLITE_ROW){

		reserve->reservationId = sqlite3_column_int(res,0);
		const char *IdTag = sqlite3_column_text(res,2);
		const char *parentIdTag = sqlite3_column_text(res,3);
		strncpy(reserve->idTag, IdTag, OCPP_AUTHORIZATION_IDTAG_LEN);
		strncpy(reserve->parentIdTag, parentIdTag, OCPP_AUTHORIZATION_IDTAG_LEN);

		const char *ExpiryDate = sqlite3_column_text(res,1);
		strptime(ExpiryDate, "%Y-%m-%dT%H:%M:%S", &reserve->expiryDate);
		result = 0;

	}

	sqlite3_finalize(res);

	return result;

}




/**
 * @description: 
 * @return {*}
 */
void ocpp_reserve_init(sqlite3 *ocpp_db){

	ocpp_RE = ocpp_db;
	if(ocpp_reserve_create_table(ocpp_RE) == -1){
		OCPP_LOG_ERROR("create reserve table fail");

	}
}


