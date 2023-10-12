#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "ocpp_order.h"

static sqlite3 *ocpp_OL;

// 函数用于创建StartTransaction表格
static int ocpp_Order_create_table(sqlite3 *db)
{
	if (db == NULL)
	{
		return -1;
	}

	char *errMsg = 0;

	// 创建Transactions表格
	const char *createTransactionsTableSQL =
		"CREATE TABLE Transactions ("
		"ID INTEGER PRIMARY KEY AUTOINCREMENT,"
		"TransactionID INT NOT NULL UNIQUE,"
		"ConnectorID INT NOT NULL,"
		"IdTag TEXT NOT NULL,"
		"MeterStart INT,"
		"MeterStop INT,"
		"StartTimes TEXT,"
		"StopTimes TEXT,"
		"StartUniqueID TEXT,"
		"StopUniqueID TEXT,"
		"Status INT,"
		"Reason INT,"
		"ReservationId INT,"
		"Completed INT"
		");";

	int result = sqlite3_exec(db, createTransactionsTableSQL, 0, 0, &errMsg);

	if (result != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
		return result;
	}

	// 创建MeterValues表格
	const char *createMeterValuesTableSQL =
		"CREATE TABLE MeterValues ("
		"ID INTEGER PRIMARY KEY AUTOINCREMENT,"
		"TransactionID INT,"
		"meterValue TEXT"
		");";

	result = sqlite3_exec(db, createMeterValuesTableSQL, 0, 0, &errMsg);

	if (result != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
		return result;
	}

	return 0;
}
// Status 0 订单在离线充电，1 订单在线充电，2 已经结束充电
int ocpp_Transaction_insert(int TransactionID, int ConnectorID, const char *IdTag, int MeterStart, const char *StartTimes, const char *StartUniqueID, int Status, int Reason, int ReservationId)
{
	if (ocpp_OL == NULL)
	{
		fprintf(stderr, "Database connection is not available.\n");
		return -1; // 数据库指针为空，操作失败
	}

	char *errMsg = 0;

	// 插入数据
	const char *insertSQL = "INSERT INTO Transactions (TransactionID, ConnectorID, IdTag, MeterStart, StartTimes, StartUniqueID, Status, Reason, ReservationId, Completed) "
							"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, 0);";

	sqlite3_stmt *stmt = NULL;

	if (sqlite3_prepare_v2(ocpp_OL, insertSQL, -1, &stmt, NULL) != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		return -1; // SQL 准备失败，操作失败
	}

	sqlite3_bind_int(stmt, 1, TransactionID);
	sqlite3_bind_int(stmt, 2, ConnectorID);
	sqlite3_bind_text(stmt, 3, IdTag, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 4, MeterStart);
	sqlite3_bind_text(stmt, 5, StartTimes, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 6, StartUniqueID, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 7, Status);
	sqlite3_bind_int(stmt, 8, Reason);
	sqlite3_bind_int(stmt, 9, ReservationId);

	if (sqlite3_step(stmt) != SQLITE_DONE)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		sqlite3_finalize(stmt);
		return -1; // 插入操作失败
	}

	sqlite3_finalize(stmt);

	// 检查数据行数，如果超过 DATA_LIMIT，删除最老的 DELETE_LIMIT 行
	const char *countSQL = "SELECT COUNT(*) FROM Transactions;";
	int rowCount = 0;

	sqlite3_stmt *countStmt = NULL;

	if (sqlite3_prepare_v2(ocpp_OL, countSQL, -1, &countStmt, NULL) != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		return -1; // SQL 准备失败，操作失败
	}

	if (sqlite3_step(countStmt) == SQLITE_ROW)
	{
		rowCount = sqlite3_column_int(countStmt, 0);
	}

	sqlite3_finalize(countStmt);

	if (rowCount > DATA_LIMIT)
	{
		int deleteCount = rowCount - DATA_LIMIT;

		// 删除最老的 DELETE_LIMIT 行
		const char *deleteSQL = "DELETE FROM Transactions WHERE ID IN (SELECT ID FROM Transactions ORDER BY ID LIMIT ?);";
		sqlite3_stmt *deleteStmt = NULL;

		if (sqlite3_prepare_v2(ocpp_OL, deleteSQL, -1, &deleteStmt, NULL) != SQLITE_OK)
		{
			fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
			return -1; // SQL 准备失败，操作失败
		}

		sqlite3_bind_int(deleteStmt, 1, deleteCount);

		if (sqlite3_step(deleteStmt) != SQLITE_DONE)
		{
			fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
			sqlite3_finalize(deleteStmt);
			return -1; // 删除操作失败
		}

		sqlite3_finalize(deleteStmt);
	}

	return 0; // 成功
}

int ocpp_Transaction_update(int TransactionID, int MeterStop, const char *StopTimes, const char *StopUniqueID, int Status, int Reason)
{
	if (ocpp_OL == NULL)
	{
		fprintf(stderr, "Database connection is not available.\n");
		return -1; // 数据库指针为空，操作失败
	}

	char *errMsg = 0;

	const char *updateSQL = "UPDATE Transactions "
							"SET MeterStop = ?, StopTimes = ?, StopUniqueID = ?, Status = ?, Reason = ?, Completed = ? "
							"WHERE TransactionID = ?;";

	sqlite3_stmt *stmt = NULL;

	if (sqlite3_prepare_v2(ocpp_OL, updateSQL, -1, &stmt, NULL) != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		return -1; // SQL 准备失败，操作失败
	}

	sqlite3_bind_int(stmt, 1, MeterStop);
	sqlite3_bind_text(stmt, 2, StopTimes, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 3, StopUniqueID, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 4, Status);
	sqlite3_bind_int(stmt, 5, Reason);
	sqlite3_bind_int(stmt, 6, 0);
	sqlite3_bind_int(stmt, 7, TransactionID);

	if (sqlite3_step(stmt) != SQLITE_DONE)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		sqlite3_finalize(stmt);
		return -1; // 更新操作失败
	}

	sqlite3_finalize(stmt);

	return 0; // 成功
}

int ocpp_MeterValues_insert(int TransactionID, const char *meterValue)
{
	if (ocpp_OL == NULL)
	{
		fprintf(stderr, "Database connection is not available.\n");
		return -1; // 数据库指针为空，操作失败
	}

	char *errMsg = 0;

	const char *insertSQL = "INSERT INTO MeterValues (TransactionID, meterValue) VALUES (?, ?);";

	sqlite3_stmt *stmt = NULL;

	if (sqlite3_prepare_v2(ocpp_OL, insertSQL, -1, &stmt, NULL) != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		return -1; // SQL 准备失败，操作失败
	}

	sqlite3_bind_int(stmt, 1, TransactionID);
	sqlite3_bind_text(stmt, 2, meterValue, -1, SQLITE_STATIC);

	if (sqlite3_step(stmt) != SQLITE_DONE)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		sqlite3_finalize(stmt);
		return -1; // 插入操作失败
	}

	sqlite3_finalize(stmt);

	return 0; // 成功
}

int ocpp_UpdateTransactionIDByUniqueID(const char *UniqueID, int NewTransactionID)
{
	if (ocpp_OL == NULL)
	{
		fprintf(stderr, "Database connection is not available.\n");
		return -1; // 数据库指针为空，操作失败
	}

	char *errMsg = 0;

	// 开始数据库事务
	if (sqlite3_exec(ocpp_OL, "BEGIN TRANSACTION;", 0, 0, 0) != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		return -1; // 开始事务失败
	}

	int rowsUpdated = 0;

	// 尝试在MeterValues表中更新TransactionID
	const char *updateMeterValuesSQL = "UPDATE MeterValues SET TransactionID = ? WHERE TransactionID IN (SELECT TransactionID FROM Transactions WHERE StartUniqueID = ? OR StopUniqueID = ?);";

	sqlite3_stmt *stmtMeterValues = NULL;

	if (sqlite3_prepare_v2(ocpp_OL, updateMeterValuesSQL, -1, &stmtMeterValues, NULL) != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		sqlite3_exec(ocpp_OL, "ROLLBACK;", 0, 0, 0); // 回滚事务
		return -1;									 // SQL 准备失败，操作失败
	}

	sqlite3_bind_int(stmtMeterValues, 1, NewTransactionID);
	sqlite3_bind_text(stmtMeterValues, 2, UniqueID, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmtMeterValues, 3, UniqueID, -1, SQLITE_STATIC);

	if (sqlite3_step(stmtMeterValues) == SQLITE_DONE)
	{
		rowsUpdated = sqlite3_changes(ocpp_OL);
	}

	sqlite3_finalize(stmtMeterValues);

	// 如果在MeterValues表中没有找到匹配的TransactionID，尝试在Transactions表中更新
	if (rowsUpdated == 0)
	{
		const char *updateTransactionsSQL = "UPDATE Transactions SET TransactionID = ? WHERE StartUniqueID = ? OR StopUniqueID = ?;";

		sqlite3_stmt *stmtTransactions = NULL;

		if (sqlite3_prepare_v2(ocpp_OL, updateTransactionsSQL, -1, &stmtTransactions, NULL) != SQLITE_OK)
		{
			fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
			sqlite3_exec(ocpp_OL, "ROLLBACK;", 0, 0, 0); // 回滚事务
			return -1;									 // SQL 准备失败，操作失败
		}

		sqlite3_bind_int(stmtTransactions, 1, NewTransactionID);
		sqlite3_bind_text(stmtTransactions, 2, UniqueID, -1, SQLITE_STATIC);
		sqlite3_bind_text(stmtTransactions, 3, UniqueID, -1, SQLITE_STATIC);

		if (sqlite3_step(stmtTransactions) != SQLITE_DONE)
		{
			fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
			sqlite3_finalize(stmtTransactions);
			sqlite3_exec(ocpp_OL, "ROLLBACK;", 0, 0, 0); // 回滚事务
			return -1;									 // 更新操作失败
		}

		sqlite3_finalize(stmtTransactions);
	}

	// 提交数据库事务
	if (sqlite3_exec(ocpp_OL, "COMMIT;", 0, 0, 0) != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		sqlite3_exec(ocpp_OL, "ROLLBACK;", 0, 0, 0); // 回滚事务
		return -1;									 // 提交事务失败
	}

	return 0; // 成功
}

int ocpp_UpdateStatusByUniqueID(const char *UniqueID, int NewStatus)
{
	if (ocpp_OL == NULL)
	{
		fprintf(stderr, "Database connection is not available.\n");
		return -1; // 数据库指针为空，操作失败
	}

	char *errMsg = 0;

	// 更新Status字段
	const char *updateStatusSQL = "UPDATE Transactions SET Status = ? WHERE StartUniqueID = ? OR StopUniqueID = ?;";

	sqlite3_stmt *stmt = NULL;

	if (sqlite3_prepare_v2(ocpp_OL, updateStatusSQL, -1, &stmt, NULL) != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		return -1; // SQL 准备失败，操作失败
	}

	sqlite3_bind_int(stmt, 1, NewStatus);
	sqlite3_bind_text(stmt, 2, UniqueID, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 3, UniqueID, -1, SQLITE_STATIC);

	if (sqlite3_step(stmt) != SQLITE_DONE)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		sqlite3_finalize(stmt);
		return -1; // 更新操作失败
	}

	sqlite3_finalize(stmt);

	return 0; // 成功
}
int ocpp_UpdateCompletedByUniqueID(const char *UniqueID, int NewCompleted)
{
	if (ocpp_OL == NULL)
	{
		fprintf(stderr, "Database connection is not available.\n");
		return -1; // 数据库指针为空，操作失败
	}

	char *errMsg = 0;

	// 更新Completed字段
	const char *updateCompletedSQL = "UPDATE Transactions SET Completed = ? WHERE StartUniqueID = ? OR StopUniqueID = ?;";

	sqlite3_stmt *stmt = NULL;

	if (sqlite3_prepare_v2(ocpp_OL, updateCompletedSQL, -1, &stmt, NULL) != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		return -1; // SQL 准备失败，操作失败
	}

	sqlite3_bind_int(stmt, 1, NewCompleted);
	sqlite3_bind_text(stmt, 2, UniqueID, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 3, UniqueID, -1, SQLITE_STATIC);

	if (sqlite3_step(stmt) != SQLITE_DONE)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		sqlite3_finalize(stmt);
		return -1; // 更新操作失败
	}

	sqlite3_finalize(stmt);

	return 0; // 成功
}

/**
 * @description: 读取事务数据库中，优先读取Reason = 5，然后读取Completed != 1并且Status不能是0，和1，然后按ID从小到大来读取，每次只读取一个，成功返回0，失败返回-1
 * @param
 * @return:
 * */
int ocpp_ReadSingleIncompleteTransaction(TransactionRecord *transaction)
{
	if (ocpp_OL == NULL)
	{
		fprintf(stderr, "Database connection is not available.\n");
		return -1; // 数据库指针为空，操作失败
	}

	const char *selectSQL = "SELECT * FROM Transactions WHERE (Reason = 5 OR (Completed != 1 AND Status != 0 AND Status != 1)) ORDER BY Reason = 5 DESC, ID ASC LIMIT 1;";
	sqlite3_stmt *stmt = NULL;

	if (sqlite3_prepare_v2(ocpp_OL, selectSQL, -1, &stmt, NULL) != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_OL));
		return -1; // SQL preparation failed
	}

	int result = -1;

	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		transaction->ID = sqlite3_column_int(stmt, 0);
		transaction->TransactionID = sqlite3_column_int(stmt, 1);
		transaction->ConnectorID = sqlite3_column_int(stmt, 2);
		memcpy(transaction->idTag, (const char *)sqlite3_column_text(stmt, 3), sizeof(transaction->idTag));
		transaction->MeterStart = sqlite3_column_int(stmt, 4);
		transaction->MeterStop = sqlite3_column_int(stmt, 5);
		memcpy(transaction->StartTimes, (const char *)sqlite3_column_text(stmt, 6), sizeof(transaction->StartTimes));
		memcpy(transaction->StopTimes, (const char *)sqlite3_column_text(stmt, 7), sizeof(transaction->StopTimes));
		memcpy(transaction->StartUniqueID, (const char *)sqlite3_column_text(stmt, 8), sizeof(transaction->StartUniqueID));
		memcpy(transaction->StopUniqueID, (const char *)sqlite3_column_text(stmt, 9), sizeof(transaction->StopUniqueID));
		transaction->Status = sqlite3_column_int(stmt, 10);
		transaction->Reason = sqlite3_column_int(stmt, 11);
		transaction->ReservationId = sqlite3_column_int(stmt, 12);
		transaction->Completed = sqlite3_column_int(stmt, 13);
		result = 0; 
	}

	sqlite3_finalize(stmt);

	return result; 
}

void ocpp_order_init(sqlite3 *ocpp_db)
{
	ocpp_OL = ocpp_db;
	if (ocpp_Order_create_table(ocpp_OL) == -1)
	{
		printf("create order table fail\n");
	}
}
