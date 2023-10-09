#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "ocpp_order.h"

static sqlite3 *ocpp_OL;

// // 函数用于创建StartTransaction表格
// static int ocpp_Order_create_table(sqlite3 *db)
// {
// 	if (db == NULL)
// 	{
// 		return -1;
// 	}

// 	char *errMsg = 0;

// 	const char *createTableSQL =
// 		"CREATE TABLE Transactions ("
// 		"ID INTEGER PRIMARY KEY AUTOINCREMENT,"
// 		"TransactionID INT NOT NULL,"
// 		"ConnectorID INT NOT NULL,"
// 		"IdTag NOT TEXT,"
// 		"MeterStart INT,"
// 		"MeterStop INT,"
// 		"StartTimes TEXT,"
// 		"StopTimes TEXT,"
// 		"StartUniqueID NOT TEXT,"
// 		"StopUniqueID NOT TEXT,"
// 		"Reason TEXT,"
// 		"MeterValue TEXT,"
// 		"IsCompleted INT NOT NULL"
// 		");";

// 	int result = sqlite3_exec(db, createTableSQL, 0, 0, &errMsg);

// 	if (result != SQLITE_OK)
// 	{
// 		fprintf(stderr, "SQL error: %s\n", errMsg);
// 		sqlite3_free(errMsg);
// 		return result;
// 	}
// 	else
// 	{
// 		printf("Transactions table created successfully.\n");
// 		return SQLITE_OK;
// 	}
// }

// int ocpp_StartTransaction_insert(int TransactionID, int ConnectorID, const char *IdTag, int MeterStart, const char *StartTimes, const char *StartUniqueID, int IsCompleted, const char *Reason)
// {
// 	if (ocpp_OL == NULL)
// 	{
// 		return -1; // 检查数据库连接是否有效
// 	}

// 	int recordCount = 0;
// 	sqlite3_stmt *countStmt;

// 	// 查询当前记录数
// 	const char *countSql = "SELECT COUNT(*) FROM Transactions";
// 	if (sqlite3_prepare_v2(ocpp_OL, countSql, -1, &countStmt, NULL) == SQLITE_OK)
// 	{
// 		if (sqlite3_step(countStmt) == SQLITE_ROW)
// 		{
// 			recordCount = sqlite3_column_int(countStmt, 0);
// 		}

// 		sqlite3_finalize(countStmt);
// 	}

// 	// 如果记录数超过限制，先删除最旧的记录
// 	if (recordCount >= MAX_TRANSACTION_RECORDS)
// 	{
// 		int deleteCount = recordCount - MAX_TRANSACTION_RECORDS + 1; // 要删除的记录数
// 		char deleteSql[128];
// 		snprintf(deleteSql, sizeof(deleteSql), "DELETE FROM Transactions WHERE ID IN (SELECT ID FROM Transactions ORDER BY ID LIMIT %d);", deleteCount);

// 		sqlite3_exec(ocpp_OL, deleteSql, 0, 0, 0);
// 	}

// 	// 插入新记录
// 	char sql[512];
// 	sqlite3_stmt *stmt;

// 	snprintf(sql, sizeof(sql), "INSERT INTO Transactions (TransactionID, ConnectorID, IdTag, MeterStart, StartTimes, StartUniqueID, IsCompleted, Reason) VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
// 	if (sqlite3_prepare_v2(ocpp_OL, sql, -1, &stmt, NULL) == SQLITE_OK)
// 	{
// 		sqlite3_bind_int(stmt, 1, TransactionID);
// 		sqlite3_bind_int(stmt, 2, ConnectorID);
// 		sqlite3_bind_text(stmt, 3, IdTag, -1, SQLITE_STATIC);
// 		sqlite3_bind_int(stmt, 4, MeterStart);
// 		sqlite3_bind_text(stmt, 5, StartTimes, -1, SQLITE_STATIC);
// 		sqlite3_bind_text(stmt, 6, StartUniqueID, -1, SQLITE_STATIC);
// 		sqlite3_bind_int(stmt, 7, IsCompleted);
// 		sqlite3_bind_text(stmt, 8, Reason, -1, SQLITE_STATIC);

// 		if (sqlite3_step(stmt) == SQLITE_DONE)
// 		{
// 			sqlite3_finalize(stmt);
// 			return 0; // 插入成功
// 		}

// 		sqlite3_finalize(stmt);
// 	}

// 	return -1; // 操作失败
// }

// int ocpp_StopTransaction_update(int TransactionID, int MeterStop, const char *StopTimes, const char *StopUniqueID, const char *Reason, const char *MeterValue)
// {
// 	if (ocpp_OL == NULL)
// 	{
// 		return -1;
// 	}
// 	char sql[512];
// 	sqlite3_stmt *stmt;

// 	snprintf(sql, sizeof(sql), "UPDATE Transactions SET MeterStop = ?, StopTimes = ?, StopUniqueID = ?, Reason = ?, MeterValue = ? WHERE TransactionID = ?");
// 	if (sqlite3_prepare_v2(ocpp_OL, sql, -1, &stmt, NULL) == SQLITE_OK)
// 	{
// 		sqlite3_bind_int(stmt, 1, MeterStop);
// 		sqlite3_bind_text(stmt, 2, StopTimes, -1, SQLITE_STATIC);
// 		sqlite3_bind_text(stmt, 3, StopUniqueID, -1, SQLITE_STATIC);
// 		sqlite3_bind_text(stmt, 4, Reason, -1, SQLITE_STATIC);
// 		sqlite3_bind_text(stmt, 5, MeterValue, -1, SQLITE_STATIC);
// 		sqlite3_bind_int(stmt, 6, TransactionID);

// 		if (sqlite3_step(stmt) == SQLITE_DONE)
// 		{
// 			sqlite3_finalize(stmt);
// 			return 0; // 更新成功
// 		}

// 		sqlite3_finalize(stmt);
// 	}

// 	return -1; // 更新失败
// }

// int ocpp_Transaction_updateTransactionID(const char *StartUniqueID, int NewTransactionID)
// {
// 	if (ocpp_OL == NULL)
// 	{
// 		return -1; // 检查数据库连接是否有效
// 	}

// 	char sql[256];
// 	sqlite3_stmt *stmt;

// 	// 尝试执行更新操作
// 	snprintf(sql, sizeof(sql), "UPDATE Transactions SET TransactionID = ? WHERE StartUniqueID = ?");
// 	if (sqlite3_prepare_v2(ocpp_OL, sql, -1, &stmt, NULL) == SQLITE_OK)
// 	{
// 		sqlite3_bind_int(stmt, 1, NewTransactionID);
// 		sqlite3_bind_text(stmt, 2, StartUniqueID, -1, SQLITE_STATIC);

// 		if (sqlite3_step(stmt) == SQLITE_DONE)
// 		{
// 			sqlite3_finalize(stmt);
// 			return 0; // 更新成功
// 		}

// 		sqlite3_finalize(stmt);
// 	}

// 	return -1; // 操作失败
// }

// int ocpp_Transaction_updateIsCompleted(const char *StopUniqueID, int NewIsCompleted)
// {
// 	if (ocpp_OL == NULL)
// 	{
// 		return -1; // 检查数据库连接是否有效
// 	}

// 	char sql[256];
// 	sqlite3_stmt *stmt;

// 	// 尝试执行更新操作
// 	snprintf(sql, sizeof(sql), "UPDATE Transactions SET IsCompleted = ? WHERE StopUniqueID = ?");
// 	if (sqlite3_prepare_v2(ocpp_OL, sql, -1, &stmt, NULL) == SQLITE_OK)
// 	{
// 		sqlite3_bind_int(stmt, 1, NewIsCompleted);
// 		sqlite3_bind_text(stmt, 2, StopUniqueID, -1, SQLITE_STATIC);

// 		if (sqlite3_step(stmt) == SQLITE_DONE)
// 		{
// 			sqlite3_finalize(stmt);
// 			return 0; // 更新成功
// 		}

// 		sqlite3_finalize(stmt);
// 	}

// 	return -1; // 操作失败
// }

void ocpp_order_init(sqlite3 *ocpp_db)
{
	ocpp_OL = ocpp_db;
	// if (ocpp_Order_create_table(ocpp_OL) == -1)
	// {
	// 	printf("create offline table fail\n");
	// }
}
