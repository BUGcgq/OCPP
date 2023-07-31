

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "ocpp_offline.h"
#include "ocpp_config.h"
#include "ocpp_log.h"


static sqlite3 *ocpp_OL;


/**
 * @description: 创建离线存储数据表
 * @param:
 * @return: 0-成功,-1-失败
 */
static int ocpp_offline_create_table(sqlite3 *p_db){

	char *zErrMsg = 0;
	char sql[512] = {0};

    snprintf(sql, sizeof(sql), "CREATE TABLE IF NOT EXISTS OfflineDatas("  \
          "NO               integer    PRIMARY KEY," \
          "MessageAction    integer    default 0,"    \
          "UniqueId         TEXT       NOT NULL,"    \
          "Payload          TEXT       NOT NULL);");
    

    if(sqlite3_exec(p_db, sql, NULL, 0, &zErrMsg) != SQLITE_OK){
    	OCPP_LOG_ERROR("create OfflineDatas %s\n",zErrMsg);        
        sqlite3_free(zErrMsg);
        return -1;
    }
    return 0;
}


/**
 * @description:将离线数据插入数据库
 * @param:
 * @return：
 */
static void ocpp_offline_insertOfflineData(enum OCPP_PACKAGE messageAction, const char * uniqueId, const char * payload){
	if (ocpp_OL == NULL) return;

	int rc = 0;
	char *ErrMsg;
	char *sql=(char *)calloc(1,2048);
	
	snprintf(sql, 2048, "insert INTO OfflineDatas(MessageAction,UniqueId,Payload) "  \
        "VALUES (%d, '%s', '%s');", messageAction, uniqueId, payload);
	
	rc = sqlite3_exec(ocpp_OL, sql, NULL, NULL, &ErrMsg);

	if (rc != SQLITE_OK){
		OCPP_LOG_ERROR("ERROR = %s\n",ErrMsg);
		sqlite3_free(ErrMsg);		
	}
	free(sql);
}



/**
 * @description:根据uniqueId,将数据从OfflineData删除
 * @param:
 * @return：
 */
void ocpp_offline_deleteOfflineData(const char * uniqueId){
	if (ocpp_OL == NULL) return;

	char *ErrMsg;
	char *sql=(char *)calloc(1,128);
	
	snprintf(sql, 128, "DELETE FROM OfflineDatas where UniqueId = '%s'",uniqueId);
	
	if (sqlite3_exec(ocpp_OL, sql, NULL, NULL, &ErrMsg) != SQLITE_OK)
	{
		OCPP_LOG_ERROR("ERROR = %s\n",ErrMsg);
		sqlite3_free(ErrMsg);		
	}
	free(sql);
	
}


/**
 * @description:从OfflineData获取最旧的第一条数据
 * @param:
 * @return：
 */
ocpp_offline_offlineData_item_t * ocpp_offline_readAData(){

	if (ocpp_OL == NULL) return NULL;
	ocpp_offline_offlineData_item_t * item = (ocpp_offline_offlineData_item_t *)calloc(1, sizeof(ocpp_offline_offlineData_item_t));
	
	sqlite3_stmt *res;

	int rc = sqlite3_prepare_v2(ocpp_OL,"SELECT * FROM OfflineDatas LIMIT 1 OFFSET 0",strlen("SELECT * FROM OfflineDatas LIMIT 1 OFFSET 0"), &res,NULL);
	
	if ( rc!= SQLITE_OK){

		OCPP_LOG_ERROR("查询失败 'SELECT * FROM OfflineDatas LIMIT 1 OFFSET 0'\n");
		return NULL;
	}

	if(sqlite3_step(res) == SQLITE_ROW){
		
		item->Action = sqlite3_column_int(res,1);
		item->unique = strdup(sqlite3_column_text(res,2));
		item->message = strdup(sqlite3_column_text(res,3));

		OCPP_LOG_DEBUG("离线数据 messageAction = %d", item->Action);
		OCPP_LOG_DEBUG("离线数据 uniqueId = %s", item->unique);
		OCPP_LOG_DEBUG("离线数据 payload = %s", item->message);

		rc = sqlite3_step(res);
	}
	
	sqlite3_finalize(res);

	return item;

}


/**
 * @description: 
 * @param {ocpp_offline_offlineData_item_t **} item
 * @param {int} number
 * @return {*}
 */
void ocpp_offline_freeItem(ocpp_offline_offlineData_item_t ** item, int number){
	if(item == NULL || number <= 0) return;

	int i = 0;
	for(i = 0; i < number; i++){
		if(item[i] != NULL) free(item[i]);
	}

	return;
}

/**
 * @description:从OfflineData获取最旧的n条数据
 * @param:
 * @return：
 */
ocpp_offline_offlineData_item_t ** ocpp_offline_readNData(int * number){
	if((*number) <= 0 || ocpp_OL == NULL){
		*number = 0; 
		return NULL;
	} 

	ocpp_offline_offlineData_item_t ** item = (ocpp_offline_offlineData_item_t **)calloc((*number), sizeof(ocpp_offline_offlineData_item_t *));
	int i = 0;
	for(i = 0; i < (*number); i++)
		item[i] = (ocpp_offline_offlineData_item_t *)calloc(1, sizeof(ocpp_offline_offlineData_item_t));


	sqlite3_stmt *res;
	char sql[128] = {0};
	snprintf(sql, 256, "SELECT * FROM OfflineDatas LIMIT %d OFFSET 0", *number);

	
	if (sqlite3_prepare_v2(ocpp_OL,sql,strlen(sql), &res,NULL) != SQLITE_OK){

		ocpp_offline_freeItem(item, *number);
		OCPP_LOG_ERROR("查询失败 'SELECT * FROM OfflineDatas LIMIT 1 OFFSET 0'\n");
		return NULL;
	}	


	i = 0;
	while(sqlite3_step(res) == SQLITE_ROW){
		
		item[i]->Action = sqlite3_column_int(res,1);
		item[i]->unique = strdup(sqlite3_column_text(res,2));
		item[i]->message = strdup(sqlite3_column_text(res,3));
		i++;
	}
	
	sqlite3_finalize(res);

	*number = i;
	return item;

}


/**
 * @description: 从OfflineData删除最旧的n条数据
 * @param {int} number
 * @return {*}
 */
void ocpp_offline_deleteNData(int number){

	if (ocpp_OL == NULL) return;

	char *ErrMsg;
	char *sql=(char *)calloc(1,128);
	
	snprintf(sql, 128, "DELETE FROM OfflineDatas LIMIT %d OFFSET 0",number);
	
	if (sqlite3_exec(ocpp_OL, sql, NULL, NULL, &ErrMsg) != SQLITE_OK){
		OCPP_LOG_ERROR("ERROR = %s\n",ErrMsg);
		sqlite3_free(ErrMsg);		
	}
	free(sql);

}




/**
 * @description:获取离线数据的数目
 * @param:
 * @return：
 */
int ocpp_offline_OfflineDatasCnt(){
	
	if (ocpp_OL == NULL) return 0;
	int cnt = -1;
	sqlite3_stmt *res;

	int rc=sqlite3_prepare_v2(ocpp_OL,"SELECT count(*) FROM OfflineDatas",strlen("SELECT count(*) FROM OfflineDatas"), &res,NULL);
	
	if ( rc!= SQLITE_OK){
		OCPP_LOG_ERROR("查询失败 'SELECT count(*) FROM OfflineDatas'\n");
		return 0;
	}

	if(sqlite3_step(res) == SQLITE_ROW){
		cnt = sqlite3_column_int(res,0);
	}
	
	sqlite3_finalize(res);
	return cnt;
}




/**
 * @description: 
 * @param {ocpp_offline_offlineData_item_t *} item
 * @return {*}
 */
void ocpp_offline_send(ocpp_offline_offlineData_item_t * item){
	if(item == NULL) return;

	ocpp_offline_insertOfflineData(item->Action, item->unique, item->message);

	return;
}





/**
 * @description: 
 * @param {sqlite3} *ocpp_db
 * @return {*}
 */
void ocpp_offline_init(sqlite3 *ocpp_db){

	ocpp_OL = ocpp_db;
	if(ocpp_offline_create_table(ocpp_OL) == -1){
		OCPP_LOG_ERROR("create offline table fail\n");

	}



}










































