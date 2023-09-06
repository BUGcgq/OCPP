

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "ocpp_order.h"

static sqlite3 *ocpp_OL;

// 函数用于创建StartTransaction表格
static int ocpp_StartTransaction_create_table(sqlite3 *db)
{
	if (db == -1)
	{
		return -1;
	}

	char *errMsg = 0;
	const char *sql = "CREATE TABLE IF NOT EXISTS StartTransaction ("
					  "ID INTEGER PRIMARY KEY,"
					  "TransactionID INTEGER NOT NULL,"
					  "ConnectorID INTEGER NOT NULL,"
					  "IdTag TEXT NOT NULL,"
					  "MeterStart INTEGER NOT NULL,"
					  "ReservationID INTEGER NOT NULL,"
					  "Timestamp TIMESTAMP NOT NULL,"
					  "IsCompleted INTEGER NOT NULL,"
					  "Offline INTEGER NOT NULL,"
					  "UniqueID TEXT NOT NULL,"
					  "UNIQUE(TransactionID) ON CONFLICT REPLACE"
					  ");";
	int rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", errMsg);
		sqlite3_free(errMsg);
		return -1; // 创建失败
	}
	return 0; // 创建成功
}

// 函数用于创建MeterValues表格
static int ocpp_MeterValues_create_table(sqlite3 *db)
{
	if (db == -1)
	{
		return -1;
	}
	char *errMsg = 0;
	const char *sql = "CREATE TABLE IF NOT EXISTS MeterValues ("
					  "ID INTEGER PRIMARY KEY,"
					  "TransactionID INTEGER NOT NULL,"
					  "ConnectorID INTEGER NOT NULL,"
					  "MeterValue TEXT NOT NULL,"
					  "IsSent INTEGER NOT NULL,"
					  "UniqueID TEXT NOT NULL"
					  ");";

	int rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", errMsg);
		sqlite3_free(errMsg);
		return -1; // 创建失败
	}
	return 0; // 创建成功
}

static int ocpp_StopTransaction_create_table(sqlite3 *db)
{
	if (db == -1)
	{
		return -1;
	}
	char *errMsg = 0;
	const char *sql = "CREATE TABLE IF NOT EXISTS StopTransaction ("
					  "ID INTEGER PRIMARY KEY,"
					  "TransactionID INTEGER NOT NULL,"
					  "IdTag TEXT NOT NULL,"
					  "Timestamp TIMESTAMP NOT NULL,"
					  "MeterStop INTEGER NOT NULL,"
					  "Reason TEXT NOT NULL,"
					  "TransactionData TEXT,"
					  "UniqueID TEXT NOT NULL,"
					  "UNIQUE(TransactionID) ON CONFLICT REPLACE"
					  ");";
	int rc = sqlite3_exec(ocpp_OL, sql, 0, 0, &errMsg);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", errMsg);
		sqlite3_free(errMsg);
		return -1; // 创建失败
	}
	return 0; // 创建成功
}
/**
 * @description: 向 StartTransaction 表格中插入新的交易记录
 * @param transactionId: 交易ID
 * @param connectorId: 充电枪ID
 * @param idTag: 用户标识
 * @param meterStart: 电表起始值
 * @param reservationId: 预约ID
 * @param timestamp: 时间戳
 * @param isCompleted: 交易是否已完成 (0: 未完成, 1: 已完成)
 * @param uniqueId: 唯一标识符
 * @param Offline: 
 * @return: 成功返回0，失败返回-1
 */
int ocpp_StartTransaction_insert(int transactionId, int connectorId, const char *idTag, int meterStart, int reservationId, const char *timestamp, int isCompleted, const char *uniqueId, int Offline)
{
	if (ocpp_OL == -1)
	{
		return -1;
	}

	// 查询是否已存在相同的 UniqueID
	const char *checkSQL = "SELECT COUNT(*) FROM StartTransaction WHERE UniqueID = ?;";
	sqlite3_stmt *checkStmt;
	int rc = sqlite3_prepare_v2(ocpp_OL, checkSQL, -1, &checkStmt, 0);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		return -1;
	}

	sqlite3_bind_text(checkStmt, 1, uniqueId, -1, SQLITE_STATIC);

	int count = 0;
	if (sqlite3_step(checkStmt) == SQLITE_ROW)
	{
		count = sqlite3_column_int(checkStmt, 0);
	}

	sqlite3_finalize(checkStmt);

	if (count > 0)
	{
		// 已存在相同的 UniqueID，返回错误
		return -1;
	}

	// 查询当前数据数量
	const char *countSQL = "SELECT COUNT(*) FROM StartTransaction;";
	sqlite3_stmt *stmt;
	rc = sqlite3_prepare_v2(ocpp_OL, countSQL, -1, &stmt, 0);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		return -1;
	}

	int rowCount = 0;
	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		rowCount = sqlite3_column_int(stmt, 0);
	}

	sqlite3_finalize(stmt);

	// 如果数据数量超过上限，删除最旧的数据
	if (rowCount >= START_STOP_TRANSACTION_MAX_ROWS)
	{
		const char *deleteSQL = "DELETE FROM StartTransaction WHERE TransactionID IN (SELECT TransactionID FROM StartTransaction ORDER BY TransactionID LIMIT 1);";
		char *errMsg = 0;
		rc = sqlite3_exec(ocpp_OL, deleteSQL, 0, 0, &errMsg);
		if (rc != SQLITE_OK)
		{
			fprintf(stderr, "SQL error: %s\n", errMsg);
			sqlite3_free(errMsg);
			return -1;
		}
		rowCount--;
	}
	// 插入新数据，包括手动插入的事务ID、UniqueID和Offline
	const char *insertSQL = "INSERT INTO StartTransaction "
							"(TransactionID, ConnectorID, IdTag, MeterStart, ReservationID, Timestamp, IsCompleted, UniqueID, Offline) "
							"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";
	rc = sqlite3_prepare_v2(ocpp_OL, insertSQL, -1, &stmt, 0);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		return -1;
	}
	sqlite3_bind_int(stmt, 1, transactionId); // 手动插入事务ID
	sqlite3_bind_int(stmt, 2, connectorId);
	sqlite3_bind_text(stmt, 3, idTag, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 4, meterStart);
	sqlite3_bind_int(stmt, 5, reservationId);
	sqlite3_bind_text(stmt, 6, timestamp, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 7, isCompleted);
	sqlite3_bind_text(stmt, 8, uniqueId, -1, SQLITE_STATIC); // 插入UniqueID
	sqlite3_bind_int(stmt, 9, Offline);						 // 插入IsSent
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		sqlite3_finalize(stmt);
		return -1;
	}
	sqlite3_finalize(stmt);

	rowCount++;

	return 0;
}
/**
 * @description: 向 ocpp_MeterValues_insert 表格中插入新的交易记录
 * @param transactionId: 交易ID
 * @param connectorId: 充电枪ID
 * @param meterValue:  发送的数据
 * @param isSent: 订单是否已发送 (0: 未发送, 1: 已发送)
 * @return: 成功返回0，失败返回-1
 */
int ocpp_MeterValues_insert(int transactionId, int connectorId, const char *meterValue, int isSent, const char *uniqueId)
{
	if (ocpp_OL == -1)
	{
		return -1;
	}

	// 查询当前数据数量
	const char *countSQL = "SELECT COUNT(*) FROM MeterValues;";
	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(ocpp_OL, countSQL, -1, &stmt, 0);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		return -1;
	}
	int rowCount = 0;
	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		rowCount = sqlite3_column_int(stmt, 0);
	}
	sqlite3_finalize(stmt);

	// 如果数据数量超过上限，删除最旧的数据
	if (rowCount >= METER_VALUES_MAX_ROWS)
	{
		const char *deleteSQL = "DELETE FROM MeterValues WHERE ID IN (SELECT ID FROM MeterValues ORDER BY ID LIMIT 1);";
		char *errMsg = 0;
		rc = sqlite3_exec(ocpp_OL, deleteSQL, 0, 0, &errMsg);
		if (rc != SQLITE_OK)
		{
			fprintf(stderr, "SQL error: %s\n", errMsg);
			sqlite3_free(errMsg);
			return -1;
		}
		rowCount--;
	}

	// 插入新数据，包括手动插入的 UniqueID
	const char *insertSQL = "INSERT INTO MeterValues "
							"(TransactionID, ConnectorID, MeterValue, IsSent, UniqueID) "
							"VALUES (?, ?, ?, ?, ?);";
	rc = sqlite3_prepare_v2(ocpp_OL, insertSQL, -1, &stmt, 0);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		return -1;
	}
	sqlite3_bind_int(stmt, 1, transactionId);
	sqlite3_bind_int(stmt, 2, connectorId);
	sqlite3_bind_text(stmt, 3, meterValue, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 4, isSent);
	sqlite3_bind_text(stmt, 5, uniqueId, -1, SQLITE_STATIC); // 插入 UniqueID
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		sqlite3_finalize(stmt);
		return -1;
	}
	sqlite3_finalize(stmt);

	rowCount++;

	return 0;
}

/**
 * @description: 向 StartTransaction 表格中插入新的交易记录
 * @param transactionId: 交易ID
 * @param connectorId: 充电枪ID
 * @param idTag: 用户标识
 * @param meterStop: 电表结算值
 * @param reservationId: 预约ID
 * @param timestamp: 时间戳
 * @param reason: 停止原因
 * @param transactionData: 电表采集数据
 * @param uniqueId: uniqueId
 * @return: 成功返回0，失败返回-1
 */
int ocpp_StopTransaction_insert(int transactionId, const char *idTag, const char *timestamp, int meterStop, const char *reason, const char *transactionData, const char *uniqueId)
{
	if (ocpp_OL == -1)
	{
		return -1;
	}

	// 查询是否已存在相同的 UniqueID
	const char *checkSQL = "SELECT COUNT(*) FROM StopTransaction WHERE UniqueID = ?;";
	sqlite3_stmt *checkStmt;
	int rc = sqlite3_prepare_v2(ocpp_OL, checkSQL, -1, &checkStmt, 0);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		return -1;
	}

	sqlite3_bind_text(checkStmt, 1, uniqueId, -1, SQLITE_STATIC);

	int count = 0;
	if (sqlite3_step(checkStmt) == SQLITE_ROW)
	{
		count = sqlite3_column_int(checkStmt, 0);
	}

	sqlite3_finalize(checkStmt);

	if (count > 0)
	{
		// 已存在相同的 UniqueID，返回错误
		return -1;
	}

	// 查询当前数据数量
	const char *countSQL = "SELECT COUNT(*) FROM StopTransaction;";
	sqlite3_stmt *stmt;
	rc = sqlite3_prepare_v2(ocpp_OL, countSQL, -1, &stmt, 0);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		return -1;
	}

	int rowCount = 0;
	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		rowCount = sqlite3_column_int(stmt, 0);
	}

	sqlite3_finalize(stmt);

	// 如果数据数量超过上限，删除最旧的数据
	if (rowCount >= START_STOP_TRANSACTION_MAX_ROWS)
	{
		const char *deleteSQL = "DELETE FROM StopTransaction WHERE TransactionID IN (SELECT TransactionID FROM StopTransaction ORDER BY TransactionID LIMIT 1);";
		char *errMsg = 0;
		rc = sqlite3_exec(ocpp_OL, deleteSQL, 0, 0, &errMsg);
		if (rc != SQLITE_OK)
		{
			fprintf(stderr, "SQL error: %s\n", errMsg);
			sqlite3_free(errMsg);
			return -1;
		}
		rowCount--;
	}

	// 插入新数据
	const char *insertSQL = "INSERT INTO StopTransaction "
							"(TransactionID, IdTag, Timestamp, MeterStop, Reason, TransactionData, UniqueID) "
							"VALUES (?, ?, ?, ?, ?, ?, ?);";
	rc = sqlite3_prepare_v2(ocpp_OL, insertSQL, -1, &stmt, 0);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		return -1;
	}
	sqlite3_bind_int(stmt, 1, transactionId);
	sqlite3_bind_text(stmt, 2, idTag, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 3, timestamp, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 4, meterStop);
	sqlite3_bind_text(stmt, 5, reason, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 6, transactionData, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 7, uniqueId, -1, SQLITE_STATIC); // 插入UniqueID
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		sqlite3_finalize(stmt);
		return -1;
	}
	sqlite3_finalize(stmt);

	rowCount++;

	return 0;
}
/**
 * @description: 输入uniqueId，更新StartTransaction表中isCompleted的值
 * @param uniqueId: uniqueId
 * @param isCompleted: 订单是否完成 (0: 完成, 1: 未完成)
 * @return: 成功返回0，失败返回-1
 */

int updateStartTransactionIsCompletedByUniqueID(const char *uniqueId, int isCompleted)
{
	if (ocpp_OL == -1)
	{
		return -1;
	}

	sqlite3_stmt *stmt;
	int rc;

	// 1. 查询与 UniqueID 关联的事务ID
	const char *selectSQL = "SELECT TransactionID FROM StopTransaction WHERE UniqueID = ?;";

	// 2. 使用 SQLite 准备语句执行查询
	rc = sqlite3_prepare_v2(ocpp_OL, selectSQL, -1, &stmt, 0);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		return -1;
	}

	sqlite3_bind_text(stmt, 1, uniqueId, -1, SQLITE_STATIC);

	int transactionId = -1;
	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		transactionId = sqlite3_column_int(stmt, 0);
	}

	sqlite3_finalize(stmt);

	// 3. 如果找到事务ID，使用它来更新 StartTransaction 表格中的 IsCompleted 字段
	if (transactionId != -1)
	{
		const char *updateSQL = "UPDATE StartTransaction SET IsCompleted = ? WHERE TransactionID = ?;";

		rc = sqlite3_prepare_v2(ocpp_OL, updateSQL, -1, &stmt, 0);
		if (rc != SQLITE_OK)
		{
			fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
			return -1;
		}

		sqlite3_bind_int(stmt, 1, isCompleted);
		sqlite3_bind_int(stmt, 2, transactionId);

		rc = sqlite3_step(stmt);
		if (rc != SQLITE_DONE)
		{
			fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
			sqlite3_finalize(stmt);
			return -1;
		}

		sqlite3_finalize(stmt);

		return 0; // 成功更新
	}

	return -1; // 没找到匹配的 UniqueID
}
/**
 * @description: 输入uniqueId，更新所有TransactionId
 * @param uniqueId: uniqueId
 * @param newTransactionId: TransactionId
 * @return: 成功返回0，失败返回-1
 */

int updateTransactionIDsByUniqueID(const char *uniqueId, int newTransactionId)
{
	if (ocpp_OL == -1)
	{
		return -1;
	}

	sqlite3_stmt *stmt;
	int rc;

	// 1. 查询 StartTransaction 表格中的事务ID
	const char *selectSQLStart = "SELECT TransactionID FROM StartTransaction WHERE UniqueID = ?;";

	rc = sqlite3_prepare_v2(ocpp_OL, selectSQLStart, -1, &stmt, 0);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		return -1;
	}

	sqlite3_bind_text(stmt, 1, uniqueId, -1, SQLITE_STATIC);

	int startTransactionId = -1;
	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		startTransactionId = sqlite3_column_int(stmt, 0);
	}

	sqlite3_finalize(stmt);

	// 2. 如果找到 StartTransaction 表格中的事务ID，使用它来更新相关的表格中的记录
	if (startTransactionId != -1)
	{
		// 更新 StartTransaction 表格中的事务ID
		const char *updateSQLStart = "UPDATE StartTransaction SET TransactionID = ? WHERE UniqueID = ?;";

		rc = sqlite3_prepare_v2(ocpp_OL, updateSQLStart, -1, &stmt, 0);
		if (rc != SQLITE_OK)
		{
			fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
			return -1;
		}

		sqlite3_bind_int(stmt, 1, newTransactionId);
		sqlite3_bind_text(stmt, 2, uniqueId, -1, SQLITE_STATIC);

		rc = sqlite3_step(stmt);
		if (rc != SQLITE_DONE)
		{
			fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
			sqlite3_finalize(stmt);
			return -1;
		}

		sqlite3_finalize(stmt);

		// 尝试更新 MeterValues 表格中的相关记录
		const char *updateSQLMeterValues = "UPDATE MeterValues SET TransactionID = ? WHERE TransactionID = ?;";

		rc = sqlite3_prepare_v2(ocpp_OL, updateSQLMeterValues, -1, &stmt, 0);
		if (rc == SQLITE_OK)
		{
			sqlite3_bind_int(stmt, 1, newTransactionId);
			sqlite3_bind_int(stmt, 2, startTransactionId);

			rc = sqlite3_step(stmt);
			if (rc != SQLITE_DONE)
			{
				fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
			}

			sqlite3_finalize(stmt);
		}

		// 尝试更新 StopTransaction 表格中的相关记录
		const char *updateSQLStopTransaction = "UPDATE StopTransaction SET TransactionID = ? WHERE TransactionID = ?;";

		rc = sqlite3_prepare_v2(ocpp_OL, updateSQLStopTransaction, -1, &stmt, 0);
		if (rc == SQLITE_OK)
		{
			sqlite3_bind_int(stmt, 1, newTransactionId);
			sqlite3_bind_int(stmt, 2, startTransactionId);

			rc = sqlite3_step(stmt);
			if (rc != SQLITE_DONE)
			{
				fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
			}

			sqlite3_finalize(stmt);
		}

		return 0; // 成功更新
	}

	return -1; // 没找到匹配的 UniqueID
}
int updateTransactionIDs(int oldTransactionId, int newTransactionId)
{
	if (ocpp_OL == -1)
	{
		return -1;
	}

	sqlite3_stmt *stmt;
	int rc;

	// 1. 更新 StartTransaction 表格中的事务ID
	const char *updateSQLStart = "UPDATE StartTransaction SET TransactionID = ? WHERE TransactionID = ?;";

	rc = sqlite3_prepare_v2(ocpp_OL, updateSQLStart, -1, &stmt, 0);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		return -1;
	}

	sqlite3_bind_int(stmt, 1, newTransactionId);
	sqlite3_bind_int(stmt, 2, oldTransactionId);

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		sqlite3_finalize(stmt);
		return -1;
	}

	sqlite3_finalize(stmt);

	// 2. 更新 MeterValues 表格中的事务ID
	const char *updateSQLMeterValues = "UPDATE MeterValues SET TransactionID = ? WHERE TransactionID = ?;";

	rc = sqlite3_prepare_v2(ocpp_OL, updateSQLMeterValues, -1, &stmt, 0);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		return -1;
	}

	sqlite3_bind_int(stmt, 1, newTransactionId);
	sqlite3_bind_int(stmt, 2, oldTransactionId);

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		sqlite3_finalize(stmt);
		return -1;
	}

	sqlite3_finalize(stmt);

	// 3. 更新 StopTransaction 表格中的事务ID
	const char *updateSQLStopTransaction = "UPDATE StopTransaction SET TransactionID = ? WHERE TransactionID = ?;";

	rc = sqlite3_prepare_v2(ocpp_OL, updateSQLStopTransaction, -1, &stmt, 0);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		return -1;
	}

	sqlite3_bind_int(stmt, 1, newTransactionId);
	sqlite3_bind_int(stmt, 2, oldTransactionId);

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		sqlite3_finalize(stmt);
		return -1;
	}

	sqlite3_finalize(stmt);

	return 0; // 成功更新
}

/**
 * @description: 根据 UniqueID 更新 IsSent 的值
 * @param uniqueId: 唯一标识符
 * @param isSent: 新的 IsSent 值 (0 或 1)
 * @return: 成功返回0，失败返回-1
 */
int updateStartTransactionIsSentByUniqueId(const char *uniqueId, int isSent)
{
	if (ocpp_OL == -1)
	{
		return -1;
	}

	sqlite3_stmt *stmt;
	int rc;

	// 1. 使用 UniqueID 查询匹配的记录
	const char *selectSQL = "SELECT TransactionID FROM StartTransaction WHERE UniqueID = ?;";

	// 2. 使用 SQLite 准备语句执行查询
	rc = sqlite3_prepare_v2(ocpp_OL, selectSQL, -1, &stmt, 0);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		return -1;
	}

	sqlite3_bind_text(stmt, 1, uniqueId, -1, SQLITE_STATIC);

	int transactionId = -1;
	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		transactionId = sqlite3_column_int(stmt, 0);
	}

	sqlite3_finalize(stmt);

	// 3. 如果找到匹配的记录，使用它来更新 StartTransaction 表格中的 IsSent 字段
	if (transactionId != -1)
	{
		const char *updateSQL = "UPDATE StartTransaction SET IsSent = ? WHERE TransactionID = ?;";

		rc = sqlite3_prepare_v2(ocpp_OL, updateSQL, -1, &stmt, 0);
		if (rc != SQLITE_OK)
		{
			fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
			return -1;
		}

		sqlite3_bind_int(stmt, 1, isSent);
		sqlite3_bind_int(stmt, 2, transactionId);

		rc = sqlite3_step(stmt);
		if (rc != SQLITE_DONE)
		{
			fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
			sqlite3_finalize(stmt);
			return -1;
		}

		sqlite3_finalize(stmt);

		return 0; // 成功更新
	}

	return -1; // 没找到匹配的 UniqueID
}

void ocpp_order_init(sqlite3 *ocpp_db)
{

	ocpp_OL = ocpp_db;
	if (ocpp_StartTransaction_create_table(ocpp_OL) == -1)
	{
		printf("create offline table fail\n");
	}
	if (ocpp_MeterValues_create_table(ocpp_OL) == -1)
	{
		printf("create offline table fail\n");
	}
	if (ocpp_StopTransaction_create_table(ocpp_OL) == -1)
	{
		printf("create offline table fail\n");
	}
}
