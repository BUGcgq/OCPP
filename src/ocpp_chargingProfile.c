#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "ocpp_chargingProfile.h"
#include "ocpp_configurationKey.h"
static sqlite3 *ocpp_cf;

static int ocpp_ChargingProfile_create_tables(sqlite3 *db)
{
    if (db == NULL)
    {
        return -1;
    }

    char *errMsg = 0;

    // 创建 ChargingProfiles 表
    const char *chargingProfilesTableSQL =
        "CREATE TABLE IF NOT EXISTS ChargingProfiles ("
        "chargingProfileId INT PRIMARY KEY NOT NULL,"
        "connectorId INT NOT NULL,"
        "transactionId INT,"
        "stackLevel INT NOT NULL,"
        "chargingProfilePurpose TEXT NOT NULL,"
        "chargingProfileKind TEXT NOT NULL,"
        "recurrencyKind TEXT,"
        "validFrom TEXT,"
        "validTo TEXT"
        ");";

    if (sqlite3_exec(db, chargingProfilesTableSQL, 0, 0, &errMsg) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return -1;
    }

    // 创建 ChargingSchedule 表
    const char *chargingScheduleTableSQL =
        "CREATE TABLE IF NOT EXISTS ChargingSchedule ("
        "chargingScheduleId INTEGER PRIMARY KEY AUTOINCREMENT,"
        "chargingProfileId INT NOT NULL,"
        "duration INT,"
        "startSchedule TEXT,"
        "chargingRateUnit TEXT NOT NULL,"
        "numPeriods INT NOT NULL,"
        "FOREIGN KEY (chargingProfileId) REFERENCES ChargingProfiles(chargingProfileId)"
        ");";

    if (sqlite3_exec(db, chargingScheduleTableSQL, 0, 0, &errMsg) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return -1;
    }

    // 创建 ChargingSchedulePeriod 表
    const char *chargingSchedulePeriodTableSQL =
        "CREATE TABLE IF NOT EXISTS ChargingSchedulePeriod ("
        "chargingSchedulePeriodId INTEGER PRIMARY KEY AUTOINCREMENT,"
        "chargingProfileId INT NOT NULL,"
        "startPeriod INT NOT NULL,"
        "restrict REAL NOT NULL,"
        "numberPhases INT,"
        "FOREIGN KEY (chargingProfileId) REFERENCES ChargingProfiles(chargingProfileId)"
        ");";

    if (sqlite3_exec(db, chargingSchedulePeriodTableSQL, 0, 0, &errMsg) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return -1;
    }

    return 0;
}

int ocpp_ChargingProfile_insert(int connectorId, const ChargingProfile *chargingProfile)
{
    if (chargingProfile == NULL && ocpp_cf == NULL)
    {
        return -1;
    }
     
    int ChargeProfileMaxStackLevel = 0;
    int ChargingScheduleMaxPeriods = 0;
    int MaxChargingProfilesInstalled = 0;

    ocpp_ConfigurationKey_getValue("ChargeProfileMaxStackLevel",&ChargeProfileMaxStackLevel);
    ocpp_ConfigurationKey_getValue("ChargingScheduleMaxPeriods",&ChargingScheduleMaxPeriods);
    ocpp_ConfigurationKey_getValue("MaxChargingProfilesInstalled",&MaxChargingProfilesInstalled);

    if (chargingProfile->stackLevel > ChargeProfileMaxStackLevel || chargingProfile->chargingSchedule.numPeriods > ChargingScheduleMaxPeriods)
    {
        return -1;
    }
    

    // 查询 ChargingProfiles 表中当前的记录条数
    const char *countQuery = "SELECT COUNT(*) FROM ChargingProfiles;";
    sqlite3_stmt *countStmt = NULL;

    if (sqlite3_prepare_v2(ocpp_cf, countQuery, -1, &countStmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));
        return -1; // SQL 准备失败，操作失败
    }

    if (sqlite3_step(countStmt) == SQLITE_ROW)
    {
        int currentCount = sqlite3_column_int(countStmt, 0);
        if (currentCount >= MaxChargingProfilesInstalled)
        {
            fprintf(stderr, "Exceeded the maximum number of records (20) for the ChargingProfiles table.\n");
            sqlite3_finalize(countStmt);
            return -1; // 超过记录条数限制，操作失败
        }
    }
    else
    {
        fprintf(stderr, "Failed to retrieve record count.\n");
        sqlite3_finalize(countStmt);
        return -1; // 无法获取记录数，操作失败
    }

    sqlite3_finalize(countStmt); // 释放查询语句对象

    char *errMsg = 0;
    sqlite3_stmt *stmt = NULL;

    // 插入到 ChargingProfiles 表
    const char *chargingProfilesInsertSQL =
        "INSERT INTO ChargingProfiles (chargingProfileId, connectorId, transactionId, stackLevel, chargingProfilePurpose, chargingProfileKind, recurrencyKind, validFrom, validTo) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";

    if (sqlite3_prepare_v2(ocpp_cf, chargingProfilesInsertSQL, -1, &stmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));
        return -1;
    }

    sqlite3_bind_int(stmt, 1, chargingProfile->chargingProfileId);
    sqlite3_bind_int(stmt, 2, connectorId);
    sqlite3_bind_int(stmt, 3, chargingProfile->transactionId);
    sqlite3_bind_int(stmt, 4, chargingProfile->stackLevel);
    sqlite3_bind_text(stmt, 5, chargingProfile->chargingProfilePurpose, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, chargingProfile->chargingProfileKind, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, chargingProfile->recurrencyKind, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 8, chargingProfile->validFrom, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 9, chargingProfile->validTo, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        fprintf(stderr, "Insertion into ChargingProfiles failed: %s\n", sqlite3_errmsg(ocpp_cf));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_reset(stmt); // 重置语句以进行下一次使用

    // 插入到 ChargingSchedule 表
    const char *chargingScheduleInsertSQL =
        "INSERT INTO ChargingSchedule (chargingProfileId, duration, startSchedule, chargingRateUnit, numPeriods) VALUES (?, ?, ?, ?, ?);";

    if (sqlite3_prepare_v2(ocpp_cf, chargingScheduleInsertSQL, -1, &stmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_bind_int(stmt, 1, chargingProfile->chargingProfileId);
    sqlite3_bind_int(stmt, 2, chargingProfile->chargingSchedule.duration);
    sqlite3_bind_text(stmt, 3, chargingProfile->chargingSchedule.startSchedule, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, chargingProfile->chargingSchedule.chargingRateUnit, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, chargingProfile->chargingSchedule.numPeriods);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        fprintf(stderr, "Insertion into ChargingSchedule failed: %s\n", sqlite3_errmsg(ocpp_cf));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_reset(stmt); // 重置语句以进行下一次使用

    // 插入到 ChargingSchedulePeriod 表
    const char *chargingSchedulePeriodInsertSQL =
        "INSERT INTO ChargingSchedulePeriod (chargingProfileId, startPeriod, restrict, numberPhases) VALUES (?, ?, ?, ?);";
    int i;
    for (i = 0; i < chargingProfile->chargingSchedule.numPeriods; i++)
    {
        if (sqlite3_prepare_v2(ocpp_cf, chargingSchedulePeriodInsertSQL, -1, &stmt, NULL) != SQLITE_OK)
        {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));
            sqlite3_finalize(stmt);
            return -1;
        }

        sqlite3_bind_int(stmt, 1, chargingProfile->chargingProfileId);
        sqlite3_bind_int(stmt, 2, chargingProfile->chargingSchedule.chargingSchedulePeriods[i].startPeriod);
        sqlite3_bind_double(stmt, 3, chargingProfile->chargingSchedule.chargingSchedulePeriods[i].limit);
        sqlite3_bind_int(stmt, 4, chargingProfile->chargingSchedule.chargingSchedulePeriods[i].numberPhases);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            fprintf(stderr, "Insertion into ChargingSchedulePeriod failed: %s\n", sqlite3_errmsg(ocpp_cf));
            sqlite3_finalize(stmt);
            return -1;
        }

        sqlite3_reset(stmt); // 重置语句以进行下一次使用
    }

    sqlite3_finalize(stmt);
    return 0;
}

// 删除数据函数，根据 int connectorId, int stackLevel, int chargingProfileId 删除相关数据
int ocpp_ChargingProfile_delete(int connectorId, int stackLevel, int chargingProfileId)
{
    if (ocpp_cf == NULL)
    {
        fprintf(stderr, "Database connection is not available.\n");
        return -1; // 数据库指针为空，操作失败
    }

    sqlite3_stmt *selectProfileIdStmt = NULL; // 定义 selectProfileIdStmt

    // 开始事务
    if (sqlite3_exec(ocpp_cf, "BEGIN TRANSACTION;", 0, 0, 0) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));
        return -1; // 开始事务失败
    }

    // 查询匹配的 chargingProfileId
    const char *selectProfileIdSQL = "SELECT chargingProfileId FROM ChargingProfiles WHERE connectorId = ? AND stackLevel = ? AND chargingProfileId = ?;";

    if (sqlite3_prepare_v2(ocpp_cf, selectProfileIdSQL, -1, &selectProfileIdStmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));

        // 回滚事务
        sqlite3_exec(ocpp_cf, "ROLLBACK;", 0, 0, 0);
        return -1; // SQL 准备失败，操作失败
    }

    // 绑定参数
    sqlite3_bind_int(selectProfileIdStmt, 1, connectorId);
    sqlite3_bind_int(selectProfileIdStmt, 2, stackLevel);
    sqlite3_bind_int(selectProfileIdStmt, 3, chargingProfileId);

    // 循环处理查询结果
    while (sqlite3_step(selectProfileIdStmt) == SQLITE_ROW)
    {
        int foundChargingProfileId = sqlite3_column_int(selectProfileIdStmt, 0);

        // 删除第二个表（ChargingSchedule）中的记录
        const char *deleteScheduleSQL = "DELETE FROM ChargingSchedule WHERE chargingProfileId = ?;";
        sqlite3_stmt *deleteScheduleStmt = NULL;

        if (sqlite3_prepare_v2(ocpp_cf, deleteScheduleSQL, -1, &deleteScheduleStmt, NULL) != SQLITE_OK)
        {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));

            // 回滚事务
            sqlite3_exec(ocpp_cf, "ROLLBACK;", 0, 0, 0);
            sqlite3_finalize(selectProfileIdStmt);
            return -1; // SQL 准备失败，操作失败
        }

        // 绑定参数 chargingProfileId
        sqlite3_bind_int(deleteScheduleStmt, 1, foundChargingProfileId);

        // 执行删除操作
        if (sqlite3_step(deleteScheduleStmt) != SQLITE_DONE)
        {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));

            // 回滚事务
            sqlite3_exec(ocpp_cf, "ROLLBACK;", 0, 0, 0);
            sqlite3_finalize(deleteScheduleStmt);
            sqlite3_finalize(selectProfileIdStmt);
            return -1; // 删除操作失败
        }

        sqlite3_finalize(deleteScheduleStmt); // 释放语句对象

        // 删除第三个表（ChargingSchedulePeriod）中的记录
        const char *deletePeriodSQL = "DELETE FROM ChargingSchedulePeriod WHERE chargingProfileId = ?;";
        sqlite3_stmt *deletePeriodStmt = NULL;

        if (sqlite3_prepare_v2(ocpp_cf, deletePeriodSQL, -1, &deletePeriodStmt, NULL) != SQLITE_OK)
        {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));

            // 回滚事务
            sqlite3_exec(ocpp_cf, "ROLLBACK;", 0, 0, 0);
            sqlite3_finalize(selectProfileIdStmt);
            return -1; // SQL 准备失败，操作失败
        }

        // 绑定参数 chargingProfileId
        sqlite3_bind_int(deletePeriodStmt, 1, foundChargingProfileId);

        // 执行删除操作
        if (sqlite3_step(deletePeriodStmt) != SQLITE_DONE)
        {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));

            // 回滚事务
            sqlite3_exec(ocpp_cf, "ROLLBACK;", 0, 0, 0);
            sqlite3_finalize(deletePeriodStmt);
            sqlite3_finalize(selectProfileIdStmt);
            return -1; // 删除操作失败
        }

        sqlite3_finalize(deletePeriodStmt); // 释放语句对象

        // 删除第一个表（ChargingProfiles）中的记录
        const char *deleteProfileSQL = "DELETE FROM ChargingProfiles WHERE chargingProfileId = ?;";
        sqlite3_stmt *deleteProfileStmt = NULL;

        if (sqlite3_prepare_v2(ocpp_cf, deleteProfileSQL, -1, &deleteProfileStmt, NULL) != SQLITE_OK)
        {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));

            // 回滚事务
            sqlite3_exec(ocpp_cf, "ROLLBACK;", 0, 0, 0);
            sqlite3_finalize(selectProfileIdStmt);
            return -1; // SQL 准备失败，操作失败
        }

        // 绑定参数 chargingProfileId
        sqlite3_bind_int(deleteProfileStmt, 1, foundChargingProfileId);

        // 执行删除操作
        if (sqlite3_step(deleteProfileStmt) != SQLITE_DONE)
        {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));

            // 回滚事务
            sqlite3_exec(ocpp_cf, "ROLLBACK;", 0, 0, 0);
            sqlite3_finalize(deleteProfileStmt);
            sqlite3_finalize(selectProfileIdStmt);
            return -1; // 删除操作失败
        }

        sqlite3_finalize(deleteProfileStmt); // 释放语句对象
    }

    // 提交事务
    if (sqlite3_exec(ocpp_cf, "COMMIT;", 0, 0, 0) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));
        sqlite3_finalize(selectProfileIdStmt);
        return -1; // 提交事务失败
    }

    sqlite3_finalize(selectProfileIdStmt);
    return 0; // 成功
}

// 删除数据函数，根据 chargingProfilePurpose 删除相关数据
int ocpp_ChargingProfile_deleteByPurpose(const char *chargingProfilePurpose)
{
    if (ocpp_cf == NULL)
    {
        fprintf(stderr, "Database connection is not available.\n");
        return -1; // 数据库指针为空，操作失败
    }

    sqlite3_stmt *selectProfileIdStmt = NULL; // 定义 selectProfileIdStmt

    // 开始事务
    if (sqlite3_exec(ocpp_cf, "BEGIN TRANSACTION;", 0, 0, 0) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));
        return -1; // 开始事务失败
    }

    // 查询匹配的 chargingProfileId
    const char *selectProfileIdSQL = "SELECT chargingProfileId FROM ChargingProfiles WHERE chargingProfilePurpose = ?;";

    if (sqlite3_prepare_v2(ocpp_cf, selectProfileIdSQL, -1, &selectProfileIdStmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));

        // 回滚事务
        sqlite3_exec(ocpp_cf, "ROLLBACK;", 0, 0, 0);
        return -1; // SQL 准备失败，操作失败
    }

    // 绑定参数
    sqlite3_bind_text(selectProfileIdStmt, 1, chargingProfilePurpose, -1, SQLITE_STATIC);

    // 循环处理查询结果
    while (sqlite3_step(selectProfileIdStmt) == SQLITE_ROW)
    {
        int foundChargingProfileId = sqlite3_column_int(selectProfileIdStmt, 0);

        // 删除第二个表（ChargingSchedule）中的记录
        const char *deleteScheduleSQL = "DELETE FROM ChargingSchedule WHERE chargingProfileId = ?;";
        sqlite3_stmt *deleteScheduleStmt = NULL;

        if (sqlite3_prepare_v2(ocpp_cf, deleteScheduleSQL, -1, &deleteScheduleStmt, NULL) != SQLITE_OK)
        {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));

            // 回滚事务
            sqlite3_exec(ocpp_cf, "ROLLBACK;", 0, 0, 0);
            sqlite3_finalize(selectProfileIdStmt);
            return -1; // SQL 准备失败，操作失败
        }

        // 绑定参数 chargingProfileId
        sqlite3_bind_int(deleteScheduleStmt, 1, foundChargingProfileId);

        // 执行删除操作
        if (sqlite3_step(deleteScheduleStmt) != SQLITE_DONE)
        {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));

            // 回滚事务
            sqlite3_exec(ocpp_cf, "ROLLBACK;", 0, 0, 0);
            sqlite3_finalize(deleteScheduleStmt);
            sqlite3_finalize(selectProfileIdStmt);
            return -1; // 删除操作失败
        }

        sqlite3_finalize(deleteScheduleStmt); // 释放语句对象

        // 删除第三个表（ChargingSchedulePeriod）中的记录
        const char *deletePeriodSQL = "DELETE FROM ChargingSchedulePeriod WHERE chargingProfileId = ?;";
        sqlite3_stmt *deletePeriodStmt = NULL;

        if (sqlite3_prepare_v2(ocpp_cf, deletePeriodSQL, -1, &deletePeriodStmt, NULL) != SQLITE_OK)
        {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));

            // 回滚事务
            sqlite3_exec(ocpp_cf, "ROLLBACK;", 0, 0, 0);
            sqlite3_finalize(selectProfileIdStmt);
            return -1; // SQL 准备失败，操作失败
        }

        // 绑定参数 chargingProfileId
        sqlite3_bind_int(deletePeriodStmt, 1, foundChargingProfileId);

        // 执行删除操作
        if (sqlite3_step(deletePeriodStmt) != SQLITE_DONE)
        {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));

            // 回滚事务
            sqlite3_exec(ocpp_cf, "ROLLBACK;", 0, 0, 0);
            sqlite3_finalize(deletePeriodStmt);
            sqlite3_finalize(selectProfileIdStmt);
            return -1; // 删除操作失败
        }

        sqlite3_finalize(deletePeriodStmt); // 释放语句对象

        // 删除第一个表（ChargingProfiles）中的记录
        const char *deleteProfileSQL = "DELETE FROM ChargingProfiles WHERE chargingProfileId = ?;";
        sqlite3_stmt *deleteProfileStmt = NULL;

        if (sqlite3_prepare_v2(ocpp_cf, deleteProfileSQL, -1, &deleteProfileStmt, NULL) != SQLITE_OK)
        {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));

            // 回滚事务
            sqlite3_exec(ocpp_cf, "ROLLBACK;", 0, 0, 0);
            sqlite3_finalize(selectProfileIdStmt);
            return -1; // SQL 准备失败，操作失败
        }

        // 绑定参数 chargingProfileId
        sqlite3_bind_int(deleteProfileStmt, 1, foundChargingProfileId);

        // 执行删除操作
        if (sqlite3_step(deleteProfileStmt) != SQLITE_DONE)
        {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));

            // 回滚事务
            sqlite3_exec(ocpp_cf, "ROLLBACK;", 0, 0, 0);
            sqlite3_finalize(deleteProfileStmt);
            sqlite3_finalize(selectProfileIdStmt);
            return -1; // 删除操作失败
        }

        sqlite3_finalize(deleteProfileStmt); // 释放语句对象
    }

    // 提交事务
    if (sqlite3_exec(ocpp_cf, "COMMIT;", 0, 0, 0) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));
        sqlite3_finalize(selectProfileIdStmt);
        return -1; // 提交事务失败
    }

    sqlite3_finalize(selectProfileIdStmt);
    return 0; // 成功
}

int ocpp_ChargingProfile_read(int connectorId, int transactionId, ChargingProfile *chargingProfile)
{
    if (chargingProfile == NULL || ocpp_cf == NULL)
    {
        return -1;
    }

    int chargingProfileId = -1;

    // 首先按照 transactionId 进行查询
    const char *transactionQuery = "SELECT * FROM ChargingProfiles WHERE connectorId = ? AND transactionId = ? ORDER BY stackLevel DESC LIMIT 1;";

    sqlite3_stmt *transactionStmt = NULL;
    if (sqlite3_prepare_v2(ocpp_cf, transactionQuery, -1, &transactionStmt, NULL) == SQLITE_OK)
    {
        // 绑定参数
        sqlite3_bind_int(transactionStmt, 1, connectorId);
        sqlite3_bind_int(transactionStmt, 2, transactionId);

        if (sqlite3_step(transactionStmt) == SQLITE_ROW)
        {
            // 找到匹配的 chargingProfileId
            chargingProfileId = sqlite3_column_int(transactionStmt, 0);
        }

        // 完成查询
        sqlite3_finalize(transactionStmt);
    }

    // 如果根据 transactionId 找不到匹配项，按照 stackLevel 降序和 chargingProfileId 降序排序，返回优先级最高的匹配项
    if (chargingProfileId == -1)
    {
        const char *priorityProfileQuery = "SELECT * FROM ChargingProfiles WHERE connectorId = ? ORDER BY stackLevel DESC, chargingProfileId DESC LIMIT 1;";
        sqlite3_stmt *priorityProfileStmt = NULL;
        if (sqlite3_prepare_v2(ocpp_cf, priorityProfileQuery, -1, &priorityProfileStmt, NULL) != SQLITE_OK)
        {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));
            return -1;
        }

        // 绑定参数
        sqlite3_bind_int(priorityProfileStmt, 1, connectorId);

        if (sqlite3_step(priorityProfileStmt) == SQLITE_ROW)
        {
            // 从查询结果中填充 ChargingProfile 结构体
            chargingProfileId = sqlite3_column_int(priorityProfileStmt, 0);
        }

        // 完成查询
        sqlite3_finalize(priorityProfileStmt);
    }

    // 如果仍然找不到匹配项，则返回错误
    if (chargingProfileId == -1)
    {
        return -1;
    }

    // 查询 ChargingProfiles 表
    const char *profileQuery = "SELECT * FROM ChargingProfiles WHERE chargingProfileId = ?;";
    sqlite3_stmt *profileStmt = NULL;
    if (sqlite3_prepare_v2(ocpp_cf, profileQuery, -1, &profileStmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));
        return -1;
    }

    // 绑定参数
    sqlite3_bind_int(profileStmt, 1, chargingProfileId);

    if (sqlite3_step(profileStmt) == SQLITE_ROW)
    {
        // 从查询结果中填充 ChargingProfile 结构体
        chargingProfile->chargingProfileId = chargingProfileId;
        chargingProfile->transactionId = sqlite3_column_int(profileStmt, 1);
        chargingProfile->stackLevel = sqlite3_column_int(profileStmt, 2);
        strncpy(chargingProfile->chargingProfilePurpose, (const char *)sqlite3_column_text(profileStmt, 3), sizeof(chargingProfile->chargingProfilePurpose));
        strncpy(chargingProfile->chargingProfileKind, (const char *)sqlite3_column_text(profileStmt, 4), sizeof(chargingProfile->chargingProfileKind));
        strncpy(chargingProfile->recurrencyKind, (const char *)sqlite3_column_text(profileStmt, 5), sizeof(chargingProfile->recurrencyKind));
        strncpy(chargingProfile->validFrom, (const char *)sqlite3_column_text(profileStmt, 6), sizeof(chargingProfile->validFrom));
        strncpy(chargingProfile->validTo, (const char *)sqlite3_column_text(profileStmt, 7), sizeof(chargingProfile->validTo));

        // 查询 ChargingSchedule 表
        const char *scheduleQuery = "SELECT * FROM ChargingSchedule WHERE chargingProfileId = ?;";
        sqlite3_stmt *scheduleStmt = NULL;
        if (sqlite3_prepare_v2(ocpp_cf, scheduleQuery, -1, &scheduleStmt, NULL) != SQLITE_OK)
        {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));
            sqlite3_finalize(profileStmt);
            return -1;
        }

        // 绑定参数
        sqlite3_bind_int(scheduleStmt, 1, chargingProfileId);

        if (sqlite3_step(scheduleStmt) == SQLITE_ROW)
        {
            // 从查询结果中填充 ChargingSchedule 结构体
            chargingProfile->chargingSchedule.duration = sqlite3_column_int(scheduleStmt, 2);
            strncpy(chargingProfile->chargingSchedule.startSchedule, (const char *)sqlite3_column_text(scheduleStmt, 3), sizeof(chargingProfile->chargingSchedule.startSchedule));
            strncpy(chargingProfile->chargingSchedule.chargingRateUnit, (const char *)sqlite3_column_text(scheduleStmt, 4), sizeof(chargingProfile->chargingSchedule.chargingRateUnit));
            chargingProfile->chargingSchedule.numPeriods = sqlite3_column_int(scheduleStmt, 5);

            // 分配内存以存储 ChargingSchedulePeriod 数组
            chargingProfile->chargingSchedule.chargingSchedulePeriods = (ChargingSchedulePeriod *)malloc(chargingProfile->chargingSchedule.numPeriods * sizeof(ChargingSchedulePeriod));

            if (chargingProfile->chargingSchedule.chargingSchedulePeriods == NULL)
            {
                fprintf(stderr, "Memory allocation failed\n");
                sqlite3_finalize(scheduleStmt);
                sqlite3_finalize(profileStmt);
                return -1;
            }

            // 查询 ChargingSchedulePeriod 表
            const char *periodQuery = "SELECT * FROM ChargingSchedulePeriod WHERE chargingProfileId = ? ORDER BY chargingSchedulePeriodId ASC;";
            sqlite3_stmt *periodStmt = NULL;
            if (sqlite3_prepare_v2(ocpp_cf, periodQuery, -1, &periodStmt, NULL) != SQLITE_OK)
            {
                fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));
                sqlite3_finalize(scheduleStmt);
                sqlite3_finalize(profileStmt);
                return -1;
            }

            // 绑定参数
            sqlite3_bind_int(periodStmt, 1, chargingProfileId);
            int i;
            for (i = 0; i < chargingProfile->chargingSchedule.numPeriods && sqlite3_step(periodStmt) == SQLITE_ROW; i++)
            {
                chargingProfile->chargingSchedule.chargingSchedulePeriods[i].startPeriod = sqlite3_column_int(periodStmt, 2);
                chargingProfile->chargingSchedule.chargingSchedulePeriods[i].limit = sqlite3_column_double(periodStmt, 3);
                chargingProfile->chargingSchedule.chargingSchedulePeriods[i].numberPhases = sqlite3_column_int(periodStmt, 4);
            }

            // 完成查询
            sqlite3_finalize(periodStmt);
        }
        else
        {
            // 没有匹配的记录
            sqlite3_finalize(scheduleStmt);
            sqlite3_finalize(profileStmt);
            return -1;
        }

        // 完成查询
        sqlite3_finalize(scheduleStmt);
        sqlite3_finalize(profileStmt);
        return 0;
    }
    else
    {
        sqlite3_finalize(profileStmt);
        return -1; // 未找到匹配的记录
    }
}

int ocpp_ChargingProfile_find(int connectorId, const char *chargingRateUnit, int duration, ChargingProfile *chargingProfile)
{
    if (chargingProfile == NULL && ocpp_cf == NULL)
    {
        return -1;
    }

    // 查询 ChargingProfiles 表，包括 connectorId 的筛选条件
    const char *profileQuery = "SELECT * FROM ChargingProfiles WHERE connectorId = ?;";

    sqlite3_stmt *profileStmt = NULL;
    if (sqlite3_prepare_v2(ocpp_cf, profileQuery, -1, &profileStmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));
        return -1;
    }
    int chargingProfileId;
    // 绑定参数
    sqlite3_bind_int(profileStmt, 1, connectorId);

    if (sqlite3_step(profileStmt) == SQLITE_ROW)
    {
        // 获取chargingProfileId
        chargingProfileId = sqlite3_column_int(profileStmt, 0);

        // 查询第二个表 ChargingSchedule，包括 duration 和 chargingRateUnit 的条件筛选
        const char *scheduleQuery = "SELECT * FROM ChargingSchedule WHERE chargingProfileId = ? AND duration = ? AND chargingRateUnit = ?;";

        sqlite3_stmt *scheduleStmt = NULL;
        if (sqlite3_prepare_v2(ocpp_cf, scheduleQuery, -1, &scheduleStmt, NULL) != SQLITE_OK)
        {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));
            sqlite3_finalize(profileStmt);
            return -1;
        }

        // 绑定参数
        sqlite3_bind_int(scheduleStmt, 1, chargingProfileId);
        sqlite3_bind_int(scheduleStmt, 2, duration);
        sqlite3_bind_text(scheduleStmt, 3, chargingRateUnit, -1, SQLITE_STATIC); // 使用 SQLITE_STATIC 表示不释放参数

        if (sqlite3_step(scheduleStmt) == SQLITE_ROW)
        {
            // 从查询结果中填充 ChargingProfile 结构体
            chargingProfile->chargingProfileId = chargingProfileId;
            chargingProfile->transactionId = sqlite3_column_int(profileStmt, 1);
            chargingProfile->stackLevel = sqlite3_column_int(profileStmt, 2);
            strncpy(chargingProfile->chargingProfilePurpose, (const char *)sqlite3_column_text(profileStmt, 3), sizeof(chargingProfile->chargingProfilePurpose));
            strncpy(chargingProfile->chargingProfileKind, (const char *)sqlite3_column_text(profileStmt, 4), sizeof(chargingProfile->chargingProfileKind));
            strncpy(chargingProfile->recurrencyKind, (const char *)sqlite3_column_text(profileStmt, 5), sizeof(chargingProfile->recurrencyKind));
            strncpy(chargingProfile->validFrom, (const char *)sqlite3_column_text(profileStmt, 6), sizeof(chargingProfile->validFrom));
            strncpy(chargingProfile->validTo, (const char *)sqlite3_column_text(profileStmt, 7), sizeof(chargingProfile->validTo));

            // 填充 ChargingSchedule 结构体
            chargingProfile->chargingSchedule.duration = sqlite3_column_int(scheduleStmt, 2);
            strncpy(chargingProfile->chargingSchedule.startSchedule, (const char *)sqlite3_column_text(scheduleStmt, 3), sizeof(chargingProfile->chargingSchedule.startSchedule));
            strncpy(chargingProfile->chargingSchedule.chargingRateUnit, (const char *)sqlite3_column_text(scheduleStmt, 4), sizeof(chargingProfile->chargingSchedule.chargingRateUnit));
            chargingProfile->chargingSchedule.numPeriods = sqlite3_column_int(scheduleStmt, 5);

            // 分配内存以存储 ChargingSchedulePeriod 数组
            chargingProfile->chargingSchedule.chargingSchedulePeriods = (ChargingSchedulePeriod *)malloc(chargingProfile->chargingSchedule.numPeriods * sizeof(ChargingSchedulePeriod));

            if (chargingProfile->chargingSchedule.chargingSchedulePeriods == NULL)
            {
                fprintf(stderr, "Memory allocation failed\n");
                sqlite3_finalize(scheduleStmt);
                sqlite3_finalize(profileStmt);
                return -1;
            }

            // 查询 ChargingSchedulePeriod 表
            const char *periodQuery = "SELECT * FROM ChargingSchedulePeriod WHERE chargingProfileId = ? ORDER BY chargingSchedulePeriodId ASC;";

            sqlite3_stmt *periodStmt = NULL;
            if (sqlite3_prepare_v2(ocpp_cf, periodQuery, -1, &periodStmt, NULL) != SQLITE_OK)
            {
                fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));
                sqlite3_finalize(scheduleStmt);
                sqlite3_finalize(profileStmt);
                return -1;
            }

            // 绑定参数
            sqlite3_bind_int(periodStmt, 1, chargingProfileId);
            int i;
            for (i = 0; i < chargingProfile->chargingSchedule.numPeriods && sqlite3_step(periodStmt) == SQLITE_ROW; i++)
            {
                chargingProfile->chargingSchedule.chargingSchedulePeriods[i].startPeriod = sqlite3_column_int(periodStmt, 2);
                chargingProfile->chargingSchedule.chargingSchedulePeriods[i].limit = sqlite3_column_double(periodStmt, 3);
                chargingProfile->chargingSchedule.chargingSchedulePeriods[i].numberPhases = sqlite3_column_int(periodStmt, 4);
            }

            // 完成查询
            sqlite3_finalize(periodStmt);
        }
        else
        {
            // 没有匹配的记录
            sqlite3_finalize(scheduleStmt);
            sqlite3_finalize(profileStmt);
            return -1;
        }

        // 完成查询
        sqlite3_finalize(scheduleStmt);
        sqlite3_finalize(profileStmt);
        return 0;
    }
    else
    {
        sqlite3_finalize(profileStmt);
        return -1; // 未找到匹配的记录
    }
}

void printChargingProfile(const ChargingProfile *chargingProfile)
{
    printf("chargingProfileId: %d\n", chargingProfile->chargingProfileId);
    printf("TransactionId: %d\n", chargingProfile->transactionId);
    printf("StackLevel: %d\n", chargingProfile->stackLevel);
    printf("ChargingProfilePurpose: %s\n", chargingProfile->chargingProfilePurpose);
    printf("ChargingProfileKind: %s\n", chargingProfile->chargingProfileKind);
    printf("RecurrencyKind: %s\n", chargingProfile->recurrencyKind);
    printf("ValidFrom: %s\n", chargingProfile->validFrom);
    printf("ValidTo: %s\n", chargingProfile->validTo);
    printf("ChargingSchedule:\n");
    printf("  Duration: %d\n", chargingProfile->chargingSchedule.duration);
    printf("  StartSchedule: %s\n", chargingProfile->chargingSchedule.startSchedule);
    printf("  ChargingRateUnit: %s\n", chargingProfile->chargingSchedule.chargingRateUnit);
    printf("  NumPeriods: %d\n", chargingProfile->chargingSchedule.numPeriods);

    printf("  ChargingSchedulePeriods:\n");
    int i;
    for (i = 0; i < chargingProfile->chargingSchedule.numPeriods; i++)
    {
        printf("    Period %d:\n", i + 1);
        printf("      StartPeriod: %d\n", chargingProfile->chargingSchedule.chargingSchedulePeriods[i].startPeriod);
        printf("      Limit: %f\n", chargingProfile->chargingSchedule.chargingSchedulePeriods[i].limit);
        printf("      NumberPhases: %d\n", chargingProfile->chargingSchedule.chargingSchedulePeriods[i].numberPhases);
    }
    printf("\n");
}

void ocpp_ChargingProfile_init(sqlite3 *ocpp_db)
{
    ocpp_cf = ocpp_db;
    if (ocpp_ChargingProfile_create_tables(ocpp_cf) == -1)
    {
        printf("create ChargingProfile table fail\n");
        return;
    }

    return 0;
}
