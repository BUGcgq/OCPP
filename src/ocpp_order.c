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

	// 创建Transactions表格，仅当不存在时创建
	const char *createTransactionsTableSQL =
		"CREATE TABLE IF NOT EXISTS Transactions ("
		"ID INTEGER PRIMARY KEY AUTOINCREMENT,"
		"TransactionID INT NOT NULL UNIQUE,"
		"ConnectorID INT NOT NULL,"
		"IdTag TEXT NOT NULL,"
		"MeterStart INT,"
		"StartTimes TEXT,"
		"ReservationId INT,"
		"StartUniqueID TEXT,"
		"MeterStop INT,"
		"StopTimes TEXT,"
		"StopUniqueID TEXT,"
		"Status INT,"
		"Reason INT,"
		"Completed INT"
		");";

	int result = sqlite3_exec(db, createTransactionsTableSQL, 0, 0, &errMsg);

	if (result != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
		return result;
	}

	// 创建MeterValues表格，仅当不存在时创建
	const char *createMeterValuesTableSQL =
		"CREATE TABLE IF NOT EXISTS MeterValues ("
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


void ocpp_order_init(sqlite3 *ocpp_db)
{
	ocpp_OL = ocpp_db;
	if (ocpp_Order_create_table(ocpp_OL) == -1)
	{
		printf("create order table fail\n");
	}
}
