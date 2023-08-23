#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "ocpp_log.h"
#include "ocpp_config.h"

#define OCPP_LOGS_LEVEL_LEN 20
#define OCPP_LOGS_MSG_LEN 1024

// 日志队列节点
struct OCPP_LOG_QUEUE_NODE_S
{
	char level[OCPP_LOGS_LEVEL_LEN];	// 日志等级
	char message[OCPP_LOGS_MSG_LEN];	// 日志内容
	struct OCPP_LOG_QUEUE_NODE_S *next; // 下一个队列节点
};

struct OCPP_LOG_S
{
	char *level;   // 日志等级
	char *message; // 日志内容
};

static int ocpp_log_QueueMutex = 0;							// 日志队列互斥锁
static struct OCPP_LOG_QUEUE_NODE_S *ocpp_log_queue = NULL; // 指向日志队列头节点
static unsigned int ocpp_log_queue_node_cnt = 0;			// 当前日志队列节点个数

static sqlite3 *ocpp_logsdb = NULL;
static sqlite3_stmt *ocpp_log_stmt_logsCount = NULL;  // 日志数目语句
static sqlite3_stmt *ocpp_log_stmt_logsInsert = NULL; // 日志插入语句

/**
 * @description: 创建一个空的日志队列节点
 * @param:
 * @return:指向新建日志节点指针
 */
static struct OCPP_LOG_QUEUE_NODE_S *ocpp_log_create_empty_queue_node()
{

	struct OCPP_LOG_QUEUE_NODE_S *empty = (struct OCPP_LOG_QUEUE_NODE_S *)calloc(1, sizeof(struct OCPP_LOG_QUEUE_NODE_S));

	return empty;
}

/**
 * @description: 显示队列所有数据
 * @param:
 * @return:
 */
static void ocpp_log_show_queue(void)
{
	printf("Display queue\n");
	struct OCPP_LOG_QUEUE_NODE_S *p = ocpp_log_queue;
	while (p)
	{
		printf("level = %s\n", p->level);
		printf("message = %s\n", p->message);
		p = p->next;
	}
}

/**
 * @description: 将节点加入队列尾部
 * @param:
 * @return:
 */
static void ocpp_log_Enqueue_queue(struct OCPP_LOG_QUEUE_NODE_S *node)
{

	// 队列满了，丢弃日志
	if (ocpp_log_queue_node_cnt > OCPP_LOGS_QUEUE_NODE_MAX)
	{
		printf("Queue overflow\n");
		free(node);
		return;
	}

	ocpp_log_queue_node_cnt++;

	while (ocpp_log_QueueMutex)
		usleep(50);
	ocpp_log_QueueMutex = 1;

	if (ocpp_log_queue == NULL)
	{

		ocpp_log_queue = node;
		ocpp_log_queue->next = NULL;
	}
	else
	{
		struct OCPP_LOG_QUEUE_NODE_S *p = ocpp_log_queue;
		while (p->next)
			p = p->next;
		p->next = node;
		p->next->next = NULL;
	}
	ocpp_log_QueueMutex = 0;

	return;
}

/**
 * @description: 从头部删除一个节点
 * @param:
 * @return:
 */
static void ocpp_log_Dequeue_queue()
{

	while (ocpp_log_QueueMutex)
		usleep(50);
	ocpp_log_QueueMutex = 1;
	if (ocpp_log_queue_node_cnt > 0)
		ocpp_log_queue_node_cnt--;

	struct OCPP_LOG_QUEUE_NODE_S *p = ocpp_log_queue;
	ocpp_log_queue = p->next;
	if (p != NULL)
	{
		free(p);
	}
	ocpp_log_QueueMutex = 0;

	return;
}

/**
 * @description: 繁忙处理程序
 * @param:
 * @return: 非0:则再次尝试访问数据库,并重复此循环.0:不访问数据库
 */
static int ocpp_log_busy_handler_callBack(void *argm, int count)
{

	printf("database %s Busy processing count = %d\n", (char *)argm, count);
	if (count >= 5)
	{
		usleep(5 * 1000);
		return 1;
	}
	else
	{
		return 0;
	}
}

/**
 * @description: 数据库跟踪处理
 * @param:
 * @return:
 */
static void ocpp_log_trace_callBack(void *argm, const char *string)
{

	//	DEBUG_INFO("数据库 %s 跟踪 %s",(char *)argm,string);
}

/**
 * @description: 创建Logs数据表
 * @param:
 * @return: 0-成功,-1-失败
 */
static int ocpp_log_create_table()
{
	int rc = 0;
	char *zErrMsg = 0;
	char sql[512];

	if (ocpp_logsdb == NULL)
		return -1;

	// insert
	snprintf(sql, sizeof(sql), "CREATE TABLE IF NOT EXISTS Logs("
							   "ID               integer PRIMARY KEY,"
							   "TimeStamp        TEXT    NOT NULL,"
							   "WarningLevel     TEXT    NOT NULL,"
							   "Message          TEXT    NOT NULL);");

	rc = sqlite3_exec(ocpp_logsdb, sql, NULL, 0, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		printf("create Logs table failed %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}

	return 0;
}

/**
 * @description: 获取Logs中日志的数目
 * @param:
 * @return: 其它:日志数目,-1:失败
 */
static int ocpp_log_getLogCnt(sqlite3_stmt *stmt)
{

	if (ocpp_logsdb == NULL)
		return -1;
	char *ErrMsg;
	int find = -1;

	int rc = sqlite3_step(stmt);
	if (rc == SQLITE_ROW)
	{
		find = sqlite3_column_int(stmt, 0);
	}

	return find;
}

/**
 * @description: 删除Logs中前n条最旧的日志
 * @param:删除的日志数目:count
 * @return:
 */
static int ocpp_log_removeLogs(int count)
{

	if (ocpp_logsdb == NULL || count < 0)
		return -1;

	char *sql = (char *)calloc(1, 256);
	char *ErrMsg;

	snprintf(sql, 256, "delete from Logs where ID in (select ID from Logs order by ID limit 0,%d);", count);

	if (sqlite3_exec(ocpp_logsdb, sql, NULL, NULL, &ErrMsg) != SQLITE_OK)
	{
		printf("ERROR = %s\n", ErrMsg);
		sqlite3_free(ErrMsg);
	}

	free(sql);
	return 0;
}

/**
 * @description: 删除Logs中的所有日志
 * @param:
 * @return: 0:删除成功,-1:失败
 */
void ocpp_log_deleteAllLog(void)
{

	if (ocpp_logsdb == NULL)
		return;

	char *sql = (char *)calloc(1, 64);
	char *ErrMsg;

	snprintf(sql, 64, "delete from Logs;");

	if (sqlite3_exec(ocpp_logsdb, sql, NULL, NULL, &ErrMsg) != SQLITE_OK)
	{
		printf("ERROR = %s\n", ErrMsg);
		sqlite3_free(ErrMsg);
	}

	free(sql);
	return;
}

/**
 * @description: 读取Logs中的第n条记录
 * @param:
 * @return: 0:读取成功，-1:读取失败
 */
// select * from Logs limit 1 offset 3;
static int ocpp_log_readXLog(sqlite3_stmt *stmt, int n, struct OCPP_LOG_S *log)
{

	if (ocpp_logsdb == NULL || n <= 0)
		return -1;
	if (!stmt)
		return -1;

	if (ocpp_log_getLogCnt(stmt) < n)
		return -1;

	sqlite3_stmt *res;
	char *ErrMsg;
	char *sql = (char *)calloc(1, 128);

	snprintf(sql, 128, "select * from Logs limit 1 offset %d;", n - 1);

	int rc = sqlite3_prepare_v2(ocpp_logsdb, sql, strlen(sql), &res, NULL);

	if (rc != SQLITE_OK)
	{
		printf("Query failed 'select * from Logs limit 1 offset n'\n");
	}

	rc = sqlite3_step(res);

	if (rc == SQLITE_ROW)
	{

		const char *encontrado = (char *)calloc(1, 150); // encontrado:发现

		// 获取日志等级
		encontrado = sqlite3_column_text(res, 2);
		log->level = calloc(1, OCPP_LOGS_LEVEL_LEN);
		strncpy(log->level, encontrado, strlen(encontrado));

		// 获取日志内容
		encontrado = sqlite3_column_text(res, 3);
		log->message = calloc(1, OCPP_LOGS_MSG_LEN);
		strncpy(log->message, encontrado, strlen(encontrado));
		strcat(log->message, "\n");
		free((void *)encontrado);
	}

	sqlite3_finalize(res);

	printf("\nlog_level = %s\n log_message = %s\n", log->level, log->message);
	free(sql);
	return 0;
}

/**
 * @description: 提取startTime ~ endTime等级不大于level的所有日志并写入文件中
 * @param:
 * @return: 0:提取成功，-1:提取失败
 */
// int extractLogsFromLogs(struct tm* startTime,struct tm * endTime,char *level,char *file){
//
//
//	time_t sTime = mktime(startTime);
//	time_t eTime = mktime(endTime);
//	time_t rTime = 0;
//
//	struct tm * recordTime = (struct tm *)calloc(1, sizeof(struct tm));
//	struct LOG_S * log = calloc(1,sizeof(struct LOG_S));
//	if(!recordTime || !log){
//		if(recordTime) free(recordTime);
//		if(log) free(log);
//		return -1;
//	}
//
//
//	int fd = -1;
//	if((fd = open(LOGS_FILEPATH,O_WRONLY | O_APPEND | O_CREAT | O_SYNC)) < 0){
//		free(recordTime);
//		free(log);
//		return -1;
//	}
//
//
//	int logsCnt = getLogCntFromLogs();
//
//	while(logsCnt > 0){
//		if(readNLogFromLogs(logsCnt, log) == 0){
//			DEBUG_MSG();
//			//判读日志等级是否符合
//			if(strcmp(log->level,level) == 0){
//				//判断日志输出模块是否符合
////				if()
//
//				//判读时间是否符合
//				strptime(log->recordTime, "%Y-%m-%dT%H:%M:%S", recordTime);
//				rTime = mktime(recordTime);
//				if(difftime(eTime,rTime) >= 0 && difftime(sTime,rTime) <= 0){
//					if(write(fd,log->message,strlen(log->message)) == strlen(log->message)){
//
//					}
//
//				}
//
//			}
//
//			free(log->level);
//			free(log->message);
//		}
//		logsCnt--;
//	}
//	close(fd);
//	free(recordTime);
//	return 0;
//}

/**
 * @description:
 * @param:日志信息:Message,告警等级:warning_level,模块:file
 * @return：
 * @notice:此函数用logMutex在addLog中加锁,不用数据库互斥锁
 */
static int ocpp_log_insertIntoLog(sqlite3_stmt *stmt, char *Message, char *warning_level)
{

	if (ocpp_logsdb == NULL)
		return -1;
	if (!Message)
		return -1;
	if (!stmt)
		return -1;

	sqlite3_reset(stmt);
	sqlite3_bind_text(stmt, sqlite3_bind_parameter_index(stmt, ":WarningLevel"),
					  warning_level, strlen(warning_level), SQLITE_TRANSIENT);

	sqlite3_bind_text(stmt, sqlite3_bind_parameter_index(stmt, ":Message"),
					  Message, strlen(Message), SQLITE_TRANSIENT);

	sqlite3_step(stmt);

	return 0;
}

/**
 * @description:
 * @param:格式话参数format,告警等级warning_level
 * @return:
 */
void ocpp_log_add(enum OCPP_LOG_LEVELS_E warning_level, char *format, ...)
{
	if (!format)
		return;

	// 如果最大日志级别是错误的，并且我们收到信息或调试（次要），我们将退出,告警等级
	if (warning_level > OCPP_LOG_ERROR)
		return;

	// 提取日志的内容
	char *message = (char *)calloc(1, OCPP_LOGS_MSG_LEN); // 写入数据库字符串
	if (message == NULL)
		return;

	va_list ap;
	va_start(ap, format);
	vsnprintf(message, OCPP_LOGS_MSG_LEN, format, ap);
	va_end(ap);

	if (warning_level == OCPP_LOG_DEBUG)
	{
		printf("[%s] = %s\r\n", ocpp_log_Level_text[warning_level], message);
		free(message);
		return;
	}

	// 将日志加入日志队列
	struct OCPP_LOG_QUEUE_NODE_S *new_log;
	new_log = ocpp_log_create_empty_queue_node();
	strncpy(new_log->level, ocpp_log_Level_text[warning_level], OCPP_LOGS_LEVEL_LEN);
	strncpy(new_log->message, message, OCPP_LOGS_MSG_LEN);
	new_log->next = NULL;

	ocpp_log_Enqueue_queue(new_log);

	if (message)
		free(message);

	return;
}

/**
 * @description: 提取日志
 * @param:
 * @return: -1:失败,0:成功
 * @note:单线程导出2000条记录用时1秒
 */
int ocpp_log_extractLogs(struct tm *startTime, struct tm *endTime, enum OCPP_LOG_LEVELS_E level)
{

	char *sTime = (char *)calloc(1, 32);
	char *eTime = (char *)calloc(1, 32);

	if (!sTime || !eTime)
	{
		free(sTime);
		free(eTime);
		return -1;
	}

	// 数据库不存在
	if (access(OCPP_LOGS_DATABASE_PATH, F_OK) != 0)
		return -1;

	if (startTime != NULL)
		strftime(sTime, 32, "%Y-%m-%d %H:%M:%S", startTime);

	if (endTime != NULL)
		strftime(eTime, 32, "%Y-%m-%d %H:%M:%S", endTime);
	printf("sTime = %s\n", sTime);
	printf("eTime = %s\n", eTime);

	// 导出日志
	printf("export log\n");
	char command[256] = {0};

	if (level == OCPP_LOG_ANY)
	{

		if (startTime == NULL && endTime == NULL)
		{
			sprintf(command, "sqlite3 -header -csv %s \"select * from %s;\" > %s",
					OCPP_LOGS_DATABASE_PATH, "Logs", OCPP_LOGS_FILEPATH);
		}
		else if (startTime == NULL)
		{
			sprintf(command, "sqlite3 -header -csv %s \"select * from %s where TimeStamp <= '%s';\" > %s",
					OCPP_LOGS_DATABASE_PATH, "Logs", eTime, OCPP_LOGS_FILEPATH);
		}
		else if (endTime == NULL)
		{
			sprintf(command, "sqlite3 -header -csv %s \"select * from %s where TimeStamp >= '%s';\" > %s",
					OCPP_LOGS_DATABASE_PATH, "Logs", sTime, OCPP_LOGS_FILEPATH);
		}
		else
		{
			sprintf(command, "sqlite3 -header -csv %s \"select * from %s where TimeStamp >= '%s'   \
																	   and TimeStamp <= '%s';\" > %s",
					OCPP_LOGS_DATABASE_PATH, "Logs", sTime, eTime, OCPP_LOGS_FILEPATH);
		}
	}
	else
	{
		sprintf(command, "sqlite3 -header -csv %s \"select * from %s where TimeStamp >= '%s'   \
		                                                                   and TimeStamp <= '%s'   \
		                                                                   and WarningLevel = '%s';\" > %s",
				OCPP_LOGS_DATABASE_PATH, "Logs", sTime, eTime, ocpp_log_Level_text[level], OCPP_LOGS_FILEPATH);
	}
	system(command);

	return 0;
}

/**
 * @description: 日志线程
 * @param:
 * @return: -1:失败,0:成功
 */
void *ocpp_log_record_thread(void *arg)
{
	printf("Create Logging Thread\n");

	////////////////////-测试-/////////////////////////
	//	struct tm * start = calloc(1,sizeof(struct tm));
	//	struct tm * end = calloc(1,sizeof(struct tm));
	//	strptime("2023-02-21 15:03:01", "%Y-%m-%d %H:%M:%S", start);
	//	strptime("2023-02-23 00:00:00", "%Y-%m-%d %H:%M:%S", end);
	//
	//	time_t now;
	//	time(&now);
	//	extractLogsFromLogs(start,end,"ANY","ANY");
	//	time_t after;
	//	time(&after);
	//	DEBUG_ERR("diffsec = %f",difftime(after,now));

	////////////////////////////////////////////

	int logs_retry_connect = 0;

LOGS_DATABASE_OPEN:
	logs_retry_connect = 0;
	sqlite3_finalize(ocpp_log_stmt_logsCount);
	sqlite3_finalize(ocpp_log_stmt_logsInsert);
	sqlite3_close(ocpp_logsdb);

	if (sqlite3_open(OCPP_LOGS_DATABASE_PATH, &ocpp_logsdb) == SQLITE_OK)
	{
		printf("Successfully opened the Logs database\n");

		if (ocpp_log_create_table(ocpp_logsdb) == 0)
		{
			printf("Successfully created logs table\n");
		}
		else
		{
			printf("Failed to create logs table\n");
			logs_retry_connect = 1;
		}
	}
	else
	{
		printf("Failed to open database\n");
		logs_retry_connect = 1;
	}

	// 注册繁忙处理
	sqlite3_busy_handler(ocpp_logsdb, ocpp_log_busy_handler_callBack, "ocpp_logsdb");

	// 注册跟踪函数(调试用)
	sqlite3_trace(ocpp_logsdb, ocpp_log_trace_callBack, "ocpp_logsdb");

	// 创建获取日志数量语句
	if (SQLITE_OK != sqlite3_prepare_v2(ocpp_logsdb, "SELECT count(*) FROM Logs;", -1, &ocpp_log_stmt_logsCount, NULL))
	{
		printf("%s\n", sqlite3_errmsg(ocpp_logsdb));
		logs_retry_connect = 1;
	}

	// 创建插入语句
	char *sql_insert = "INSERT INTO Logs (TimeStamp,WarningLevel,Message)"
					   "VALUES(datetime('now'),:WarningLevel,:Message);";

	if (SQLITE_OK != sqlite3_prepare_v2(ocpp_logsdb, sql_insert, -1, &ocpp_log_stmt_logsInsert, NULL))
	{
		printf("%s\n", sqlite3_errmsg(ocpp_logsdb));
		logs_retry_connect = 1;
	}

	if (logs_retry_connect)
		goto LOGS_DATABASE_OPEN;

	while (1)
	{

		//		show_Log_queue();
		// 日志溢出时删除最旧的5000条日志
		if (ocpp_log_getLogCnt(ocpp_log_stmt_logsCount) > OCPP_LOGS_MAX_CNT + OCPP_LOGS_MAX_OVERFLOW_CNT)
		{
			ocpp_log_removeLogs(5000);
		}

		// 将日志存进数据库
		while (ocpp_log_queue != NULL)
		{
			//			printf("插入日志\n");
			if (ocpp_log_insertIntoLog(ocpp_log_stmt_logsInsert, ocpp_log_queue->message, ocpp_log_queue->level) == 0)
			{
				ocpp_log_Dequeue_queue();
			}
		}

		if (ocpp_log_queue == NULL)
		{
			//			printf("队列为空\n");
		}

		usleep(2 * 1000 * 1000);
	}

	sqlite3_finalize(ocpp_log_stmt_logsCount);
	sqlite3_finalize(ocpp_log_stmt_logsInsert);
	sqlite3_close(ocpp_logsdb);
}
