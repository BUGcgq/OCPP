/*
 * @Author: LIYAOHAN 1791002655@qq.com
 * @Date: 2023-04-01 15:34:43
 * @LastEditors: LIYAOHAN 1791002655@qq.com
 * @LastEditTime: 2023-04-27 13:50:25
 * @FilePath: /OCPP/ocpp_localAuthorization.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

// 外部头文件

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
// 内部头文件
#include "ocpp_localAuthorization.h"
#include "ocpp_config.h"

static int ocpp_localAuthorization_listVersion = 0;
static sqlite3 *ocpp_AL;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////-本地认证缓存-////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @description: 创建authorizationCache数据表
 * @param:
 * @return: 0-成功,-1-失败
 */
static int ocpp_localAuthorization_Cache_create_table(sqlite3 *p_db)
{
	int rc = 0;
	char *zErrMsg = 0;
	char sql[512] = {0};

	snprintf(sql, sizeof(sql), "CREATE TABLE IF NOT EXISTS AuthorizationCache("
							   "NO                integer PRIMARY KEY,"
							   "IdTag             TEXT    NOT NULL UNIQUE,"
							   "Status            TEXT    NOT NULL,"
							   "GetTime           TEXT    NOT NULL,"
							   "ExpiryDate        TEXT    default NULL,"
							   "ParentIdTag       TEXT    default NULL);");

	rc = sqlite3_exec(p_db, sql, NULL, 0, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		printf("创建AuthorizationCache表失败 %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}

	return 0;
}

/**
 * @description: 清空authorizationCache数据表
 * @param:
 * @return: 0-成功,-1-失败
 */
int ocpp_localAuthorization_Cache_clearCache(void)
{

	if (ocpp_AL == NULL)
		return -1;
	char sql[64] = {0};
	char *ErrMsg;
	snprintf(sql, 64, "delete from AuthorizationCache;");

	if (sqlite3_exec(ocpp_AL, sql, NULL, NULL, &ErrMsg) != SQLITE_OK)
	{
		printf("ERROR = %s\n", ErrMsg);
		sqlite3_free(ErrMsg);
	}
	return 0;
}

/**
 * @description: 查看的 idTagInfo 是否存在缓存中
 * @param:
 * @return: true = 存在,fail = 不存在
 */
bool ocpp_localAuthorization_Cache_search(const char *idTag)
{

	if (ocpp_AL == NULL || idTag == NULL)
		return false;

	sqlite3_stmt *res = NULL;
	char *ErrMsg;
	char sql[128] = {0};
	snprintf(sql, 256, "SELECT count(*) FROM AuthorizationCache where IdTag = '%s';", idTag);
	bool found = false;

	if (sqlite3_prepare_v2(ocpp_AL, sql, -1, &res, NULL) != SQLITE_OK)
	{
		printf("查询失败 'SELECT count(*) FROM AuthorizationCache;'\n");
	}

	if (sqlite3_step(res) == SQLITE_ROW)
	{
		found = sqlite3_column_int(res, 0);
	}
	sqlite3_finalize(res);

	return found > 0 ? true : false;
}

/**
 * @description: 将一条缓存记录插入AuthorizationCache
 * @param {ocpp_localAuthorization_cache_record_t} cache_record
 * @return {*} 0 = success, -1 = fail
 */
static int ocpp_localAuthorization_Cache_insert(ocpp_localAuthorization_cache_record_t *cache_record)
{

	if (ocpp_AL == NULL || cache_record == NULL)
		return -1;

	char sql[256] = {0};

	char expriyDate[32] = {0};
	if (cache_record->expiryDate)
	{
		strftime(expriyDate, 32, "%Y-%m-%d %H:%M:%S", cache_record->expiryDate);
	}
	else
	{
		snprintf(expriyDate, 32, "NULL");
	}

	char status[16];
	if (cache_record->status == OCPP_LOCAL_AUTHORIZATION_ACCEPTED || cache_record->status == OCPP_LOCAL_AUTHORIZATION_CONCURRENTTX)
	{
		snprintf(status, 16, "valid");
	}
	else
	{
		snprintf(status, 16, "invalid");
	}

	snprintf(sql, 256, "INSERT INTO AuthorizationCache(IdTag,Status,GetTime,ExpiryDate,ParentIdTag) \
	                  VALUES('%s','%s',datetime('now'),'%s','%s');",
			 cache_record->IdTag, status,
			 expriyDate, cache_record->parentIdTag);

	char *ErrMsg;
	if (sqlite3_exec(ocpp_AL, sql, NULL, NULL, &ErrMsg) != SQLITE_OK)
	{
		printf("ERROR = %s\n", ErrMsg);
		sqlite3_free(ErrMsg);
		return false;
	}

	return true;
}

/**
 * @description: 更新缓存中的 IdTagInfo
 * @return {*} 0 = success, -1 = fail
 */
static int ocpp_localAuthorization_Cache_UpdateIdTagInfo(ocpp_localAuthorization_cache_record_t *cache_record)
{

	if (ocpp_AL == NULL || cache_record == NULL)
		return -1;

	char expriyDate[32] = {0};
	if (cache_record->expiryDate)
	{
		strftime(expriyDate, 32, "%Y-%m-%d %H:%M:%S", cache_record->expiryDate);
	}
	else
	{
		snprintf(expriyDate, 32, "NULL");
	}

	char status[16];
	if (cache_record->status == OCPP_LOCAL_AUTHORIZATION_ACCEPTED || cache_record->status == OCPP_LOCAL_AUTHORIZATION_CONCURRENTTX)
	{
		snprintf(status, 16, "valid");
	}
	else
	{
		snprintf(status, 16, "invalid");
	}

	char sql[128] = {0};
	snprintf(sql, 128, "UPDATE AuthorizationCache set Status = '%s',GetTime = datetime('now'),ExpiryDate = '%s',ParentIdTag = '%s';",
			 status, expriyDate, cache_record->parentIdTag);

	char *ErrMsg;
	if (sqlite3_exec(ocpp_AL, sql, NULL, NULL, &ErrMsg) != SQLITE_OK)
	{
		printf("ERROR = %s\n", ErrMsg);
		sqlite3_free(ErrMsg);
		return -1;
	}

	return 0;
}

/**
 * @description: 移除所有无效的缓存(invalid、过期)
 * @return {*} 0 = success, -1 = fail
 */
static int ocpp_localAuthorization_Cache_RmoveAllInvail(void)
{
	if (ocpp_AL == NULL)
		return -1;

	char sql[128] = {0};
	snprintf(sql, 128, "delete from AuthorizationCache where Status = 'invalid' and ExpiryDate <= datetime('now);");

	char *ErrMsg;
	if (sqlite3_exec(ocpp_AL, sql, NULL, NULL, &ErrMsg) != SQLITE_OK)
	{
		printf("ERROR = %s\n", ErrMsg);
		sqlite3_free(ErrMsg);
		return -1;
	}

	return 0;
}

/**
 * @description: 移除一条最老的缓存
 * @return {*} 0 = success, -1 = fail
 */
static int ocpp_localAuthorization_Cache_RemoveOldest(void)
{
	if (ocpp_AL == NULL)
		return -1;

	char sql[128] = {0};

	snprintf(sql, 256, "delete from AuthorizationCache where GetTime in (select GetTime from AuthorizationCache order by GetTime limit 0,1);");

	char *ErrMsg;
	if (sqlite3_exec(ocpp_AL, sql, NULL, NULL, &ErrMsg) != SQLITE_OK)
	{
		printf("ERROR = %s\n", ErrMsg);
		sqlite3_free(ErrMsg);
		return -1;
	}
	return 0;
}

/**
 * @description: 获取缓存的数量
 * @return {*}
 */
static int ocpp_localAuthorization_Cache_RecordCount(void)
{
	if (ocpp_AL == NULL)
		return -1;

	sqlite3_stmt *res = NULL;
	char sql[128] = {0};
	snprintf(sql, 256, "SELECT count(*) FROM AuthorizationCache;");

	if (sqlite3_prepare_v2(ocpp_AL, sql, -1, &res, NULL) != SQLITE_OK)
	{
		printf("查询失败 'SELECT count(*) FROM WhiteListCard;'\n");
	}

	int count = 0;
	if (sqlite3_step(res) == SQLITE_ROW)
	{
		count = sqlite3_column_int(res, 0);
	}
	sqlite3_finalize(res);

	return count;
}

/**
 * @description: 将接收到的 idTagInfo 写入缓存
 * @param:
 * @return: 0-成功,-1-失败
 */
int ocpp_localAuthorization_Cache_write(ocpp_localAuthorization_cache_record_t *cache_record)
{

	if (ocpp_AL == NULL || cache_record == NULL)
	{
		return -1;
	}

	if (ocpp_localAuthorization_Cache_search(cache_record->IdTag) == true) // 如果缓存存在该卡号
	{
		ocpp_localAuthorization_Cache_UpdateIdTagInfo(cache_record); // 更新它的消息
	}
	else // 如果缓存不存在该卡号
	{
		if (ocpp_localAuthorization_Cache_RecordCount() >= OCPP_LOCAL_AUTHORIZATION_CACHE_MAX)
		{
			ocpp_localAuthorization_Cache_RmoveAllInvail(); // 移除所有无效的缓存(invalid、过期)
		}

		if (ocpp_localAuthorization_Cache_RecordCount() >= OCPP_LOCAL_AUTHORIZATION_CACHE_MAX)
		{
			ocpp_localAuthorization_Cache_RemoveOldest(); // 移除一条最老的缓存
		}

		ocpp_localAuthorization_Cache_insert(cache_record);
	}

	ocpp_localAuthorization_Cache_RmoveAllInvail(); // 移除所有无效的缓存(invalid、过期)
}

/**
 * @description: 判断卡号是否有效
 * @param:
 * @return: true = 有效,false = 无效
 */
bool ocpp_localAuthorization_Cache_isVaild(const char *idTag)
{

	if (ocpp_AL == NULL)
		return false;
	char sql[128] = {0};
	snprintf(sql, 128, "select Status from AuthorizationCache where IdTag = '%s';", idTag);

	sqlite3_stmt *res = NULL;
	bool result = false;
	if (sqlite3_prepare_v2(ocpp_AL, sql, -1, &res, NULL) != SQLITE_OK)
	{
		printf("查询失败 'select Status from AuthorizationCache where IdTag'\n");
	}

	char status[16] = {0};
	if (sqlite3_step(res) == SQLITE_ROW)
	{
		strncpy(status, sqlite3_column_text(res, 0), 16);
	}
	sqlite3_finalize(res);

	printf("status = %s\n", status);

	if (strcmp(status, "valid") == 0)
		result = true;

	printf("result = %d\n", result);
	return result;
}

/**
 * @description: 获取缓存中所有无效条目
 * @param:
 * @return: 无效缓存的数量
 */
// char ** ocpp_localAuthorization_Cache_getAllInvaild(int * inVaildCnt){

//     if(ocpp_AL == NULL || inVaildCnt == NULL) return NULL;

//     char sql[128] = {0};
//     snprintf(sql,128,"SELECT COUNT(*) from AuthorizationCache where Status = 'invalid' and ExpiryDate <= datetime('now');");

//     sqlite3_stmt *res = NULL;
//     if (sqlite3_prepare_v2(ocpp_AL,sql,-1, &res,NULL) != SQLITE_OK){
// 		printf("查询失败 'SELECT COUNT(*) from AuthorizationCache where Status = 'invalid' and ExpiryDate <= datetime('now')'\n");
// 	}

//     *inVaildCnt = 0;
//     if(sqlite3_step(res) == SQLITE_ROW){
//         *inVaildCnt = sqlite3_column_int(res,0);
// 	}
//     sqlite3_finalize(res);

//     if(*inVaildCnt <= 0) return NULL;

// 	char ** idTags = (char **)calloc(*inVaildCnt , sizeof(char *));

//     snprintf(sql,128,"SELECT IdTag from AuthorizationCache where Status = 'invalid' and ExpiryDate <= datetime('now');");
//     if (sqlite3_prepare_v2(ocpp_AL,sql,-1, &res,NULL) != SQLITE_OK){
// 		printf("查询失败 'SELECT IdTag from AuthorizationCache where Status = 'invalid' and ExpiryDate <= datetime('now')'\n");
// 	}

//     int index = 0;
//     while(sqlite3_step(res) == SQLITE_ROW){

// 		idTags[index++] = strndup(sqlite3_column_text(res,0), OCPP_AUTHORIZATION_IDTAG_LEN + 1);
// 		if(index >= (*inVaildCnt)) break;

// 	}
//     sqlite3_finalize(res);

//     return idTags;

// }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////-本地认证列表-////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @description: 创建authorizationList数据表
 * @param:
 * @return: 0-成功,-1-失败
 */
static int ocpp_localAuthorization_List_create_table(sqlite3 *p_db)
{
	int rc = 0;
	char *zErrMsg = 0;
	char sql[512];

	snprintf(sql, sizeof(sql), "CREATE TABLE IF NOT EXISTS AuthorizationList_%d("
							   "NO                integer PRIMARY KEY,"
							   "IdTag             TEXT    NOT NULL UNIQUE,"
							   "status            TEXT    NOT NULL,"
							   "expiryDate        TEXT,"
							   "parentIdTag       TEXT    );",
			 ocpp_localAuthorization_listVersion);

	rc = sqlite3_exec(p_db, sql, NULL, 0, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		printf("创建AuthorizationList表失败 %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}

	return 0;
}

/**
 * @description: 清空List
 * @param:
 * @return: 0-成功,-1-失败
 */
int ocpp_localAuthorization_List_clearList(void)
{

	if (ocpp_AL == NULL)
		return -1;
	char sql[64] = {0};
	char *ErrMsg;
	snprintf(sql, 64, "delete from AuthorizationList_%d;", ocpp_localAuthorization_listVersion);

	if (sqlite3_exec(ocpp_AL, sql, NULL, NULL, &ErrMsg) != SQLITE_OK)
	{
		printf("ERROR = %s\n", ErrMsg);
		sqlite3_free(ErrMsg);
	}
	return 0;
}

/**
 * @description: 查看的 idTagInfo 是否存在中
 * @param:
 * @return: true = 存在,fail = 不存在
 */
bool ocpp_localAuthorization_List_search(const char *idTag)
{

	if (ocpp_AL == NULL || idTag == NULL)
		return false;

	sqlite3_stmt *res = NULL;
	char *ErrMsg;
	char sql[128] = {0};
	snprintf(sql, 256, "SELECT count(*) FROM AuthorizationList_%d where IdTag = '%s';", ocpp_localAuthorization_listVersion, idTag);
	bool found = false;

	if (sqlite3_prepare_v2(ocpp_AL, sql, -1, &res, NULL) != SQLITE_OK)
	{
		printf("查询失败 'SELECT count(*) FROM AuthorizationList;'\n");
	}

	if (sqlite3_step(res) == SQLITE_ROW)
	{
		found = sqlite3_column_int(res, 0);
	}
	sqlite3_finalize(res);

	return found > 0 ? true : false;
}

/**
 * @description: AuthorizationList
 * @param {ocpp_localAuthorization_list_entry_t} list_entry
 * @return {*} 0 = success, -1 = fail
 */
static int ocpp_localAuthorization_List_insert(ocpp_localAuthorization_list_entry_t *list_entry)
{

	if (ocpp_AL == NULL || list_entry == NULL)
		return -1;

	char sql[256] = {0};

	char expriyDate[32] = {0};
	if (list_entry->expiryDate)
	{
		strftime(expriyDate, 32, "%Y-%m-%d %H:%M:%S", list_entry->expiryDate);
	}
	else
	{
		snprintf(expriyDate, 32, "NULL");
	}

	char status[16];
	if (list_entry->status == OCPP_LOCAL_AUTHORIZATION_ACCEPTED || list_entry->status == OCPP_LOCAL_AUTHORIZATION_CONCURRENTTX)
	{
		snprintf(status, 16, "valid");
	}
	else if (list_entry->status == OCPP_LOCAL_AUTHORIZATION_EXPIRED)
	{
		snprintf(status, 16, "expired");
	}
	else if (list_entry->status == OCPP_LOCAL_AUTHORIZATION_BLOCKED)
	{
		snprintf(status, 16, "blocked");
	}
	else if (list_entry->status == OCPP_LOCAL_AUTHORIZATION_INVALID)
	{
		snprintf(status, 16, "blocklisted");
	}
	else
	{
		snprintf(status, 16, "blocklisted");
	}

	snprintf(sql, 256, "INSERT INTO AuthorizationList_%d(IdTag,Status,ExpiryDate,ParentIdTag) \
	                  VALUES('%s','%s','%s','%s');",
			 ocpp_localAuthorization_listVersion, list_entry->IdTag, status,
			 expriyDate, list_entry->parentIdTag);

	char *ErrMsg;
	if (sqlite3_exec(ocpp_AL, sql, NULL, NULL, &ErrMsg) != SQLITE_OK)
	{
		printf("ERROR = %s\n", ErrMsg);
		sqlite3_free(ErrMsg);
		return false;
	}

	return true;
}

/**
 * @description: 更新列表中的 IdTagInfo
 * @return {*} 0 = success, -1 = fail
 */
static int ocpp_localAuthorization_List_UpdateIdTagInfo(ocpp_localAuthorization_list_entry_t *list_entry)
{

	if (ocpp_AL == NULL || list_entry == NULL)
		return -1;

	char expriyDate[32] = {0};
	if (list_entry->expiryDate)
	{
		strftime(expriyDate, 32, "%Y-%m-%d %H:%M:%S", list_entry->expiryDate);
	}
	else
	{
		snprintf(expriyDate, 32, "NULL");
	}

	char status[16];
	if (list_entry->status == OCPP_LOCAL_AUTHORIZATION_ACCEPTED || list_entry->status == OCPP_LOCAL_AUTHORIZATION_CONCURRENTTX)
	{
		snprintf(status, 16, "valid");
	}
	else if (list_entry->status == OCPP_LOCAL_AUTHORIZATION_EXPIRED)
	{
		snprintf(status, 16, "expired");
	}
	else if (list_entry->status == OCPP_LOCAL_AUTHORIZATION_BLOCKED)
	{
		snprintf(status, 16, "blocked");
	}
	else if (list_entry->status == OCPP_LOCAL_AUTHORIZATION_INVALID)
	{
		snprintf(status, 16, "blocklisted");
	}
	else
	{
		snprintf(status, 16, "blocklisted");
	}

	char sql[128] = {0};
	snprintf(sql, 128, "UPDATE AuthorizationList_%d set Status = '%s',ExpiryDate = '%s',ParentIdTag = '%s';",
			 ocpp_localAuthorization_listVersion, status, expriyDate, list_entry->parentIdTag);

	char *ErrMsg;
	if (sqlite3_exec(ocpp_AL, sql, NULL, NULL, &ErrMsg) != SQLITE_OK)
	{
		printf("ERROR = %s\n", ErrMsg);
		sqlite3_free(ErrMsg);
		return -1;
	}

	return 0;
}

/**
 * @description: 获取缓存的数量
 * @return {*}
 */
static int ocpp_localAuthorization_List_RecordCount(void)
{
	if (ocpp_AL == NULL)
		return -1;

	sqlite3_stmt *res = NULL;
	char sql[128] = {0};
	snprintf(sql, 256, "SELECT count(*) FROM AuthorizationList_%d;", ocpp_localAuthorization_listVersion);

	if (sqlite3_prepare_v2(ocpp_AL, sql, -1, &res, NULL) != SQLITE_OK)
	{
		printf("查询失败 'SELECT count(*) FROM AuthorizationList;'\n");
	}

	int count = 0;
	if (sqlite3_step(res) == SQLITE_ROW)
	{
		count = sqlite3_column_int(res, 0);
	}
	sqlite3_finalize(res);

	return count;
}

/**
 * @description: 将接收到的 idTagInfo 写入缓存
 * @param:
 * @return: 0-成功,-1-失败
 */
int ocpp_localAuthorization_List_write(ocpp_localAuthorization_list_entry_t *list_entry)
{

	if (ocpp_AL == NULL || list_entry == NULL)
		return -1;

	if (ocpp_localAuthorization_List_search(list_entry->IdTag) == true)
	{
		ocpp_localAuthorization_List_UpdateIdTagInfo(list_entry);
	}
	else
	{

		ocpp_localAuthorization_List_insert(list_entry);
	}
}

/**
 * @description: 判断卡号是否有效
 * @param:
 * @return: true = 有效,false = 无效
 */
bool ocpp_localAuthorization_List_isVaild(const char *idTag)
{

	if (ocpp_AL == NULL)
		return false;
	char sql[128] = {0};
	snprintf(sql, 128, "select status from AuthorizationList_%d where IdTag = '%s';", ocpp_localAuthorization_listVersion, idTag);

	sqlite3_stmt *res = NULL;
	bool result = false;
	if (sqlite3_prepare_v2(ocpp_AL, sql, -1, &res, NULL) != SQLITE_OK)
	{
		printf("查询失败 'select status from AuthorizationList where IdTag'\n");
	}

	char status[16] = {0};
	if (sqlite3_step(res) == SQLITE_ROW)
	{
		strncpy(status, sqlite3_column_text(res, 2), 16);
	}
	sqlite3_finalize(res);

	printf("status = %s\n", status);

	if (strcmp(status, "valid") == 0)
		result = true;

	return result;
}

/**
 * @description: 获取本地认证列表的版本号
 * @param:
 * @return: >= 0--版本号 ,-1-失败
 */
static int ocpp_localAuthorization_List_getListVersion(void)
{

	if (ocpp_AL == NULL)
		return -1;
	char sql[128] = {0};
	snprintf(sql, 128, "select name from sqlite_master where name like 'AuthorizationList%%';");

	sqlite3_stmt *res = NULL;
	if (sqlite3_prepare_v2(ocpp_AL, sql, -1, &res, NULL) != SQLITE_OK)
	{
		printf("查询失败 'select name from sqlite_master where name like 'AuthorizationList;'\n");
	}

	if (sqlite3_step(res) == SQLITE_ROW)
	{
		const char *name = sqlite3_column_text(res, 0);
		strchr(name, '_');
		ocpp_localAuthorization_listVersion = atoi(name + 1);
	}
	else
	{
		ocpp_localAuthorization_listVersion = 0;
	}

	sqlite3_finalize(res);
	return ocpp_localAuthorization_listVersion;
}

/**
 * @description: 获取本地认证列表的版本号(外部接口)
 * @param:
 * @return:
 */
int ocpp_localAuthorization_List_getListVersion2(void)
{

	return ocpp_localAuthorization_listVersion;
}

/**
 * @description: 获取本地认证列表的版本号
 * @param:
 * @return: 0 = success ,-1 = fail
 */
int ocpp_localAuthorization_List_setListVersion(int version)
{
	if (version < 0)
		return -1;

	char sql[128] = {0};

	snprintf(sql, 128, "ALTER TABLE AuthorizationList_%d TO AuthorizationList_%d;", ocpp_localAuthorization_listVersion, version);

	char *ErrMsg;
	if (sqlite3_exec(ocpp_AL, sql, NULL, NULL, &ErrMsg) != SQLITE_OK)
	{
		printf("ERROR = %s\n", ErrMsg);
		sqlite3_free(ErrMsg);
		return -1;
	}

	ocpp_localAuthorization_listVersion = version;
	return 0;
}

/**
 * @description: 初始化本地认证
 * @param:
 * @return:
 */
void ocpp_localAuthorization_init(sqlite3 *ocpp_db)
{
	ocpp_AL = ocpp_db;
	if (ocpp_localAuthorization_Cache_create_table(ocpp_AL) == -1)
		printf("create localAuthorization_Cache fail\n");
	if (ocpp_localAuthorization_List_getListVersion() == -1)
		printf("get localAuthorization_List list version fail\n");
	if (ocpp_localAuthorization_List_create_table(ocpp_AL) == -1)
		printf("create localAuthorization_List fail\n");
}

/**
 * @description: main
 * @param:
 * @return: 0-成功,-1-失败
 */
// int main(void){
//     printf("main in\n");
//     sqlite3_open("ocpp.db",&db);
//     ocpp_localAuthorization_init();

//     ocpp_localAuthorization_cache_record_t cache;
//     cache.expiryDate = NULL;
//     strcpy(cache.IdTag,"123456789");
//     strcpy(cache.parentIdTag,"54321");
//     cache.status = OCPP_LOCAL_AUTHORIZATION_ACCEPTED;
//     ocpp_localAuthorization_Cache_insert(&cache);

// }