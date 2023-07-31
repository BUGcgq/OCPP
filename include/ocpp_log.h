/*
 * @Author: LIYAOHAN 1791002655@qq.com
 * @Date: 2023-05-04 08:56:06
 * @LastEditors: LIYAOHAN 1791002655@qq.com
 * @LastEditTime: 2023-05-06 10:12:48
 * @FilePath: /OCPP/ocpp_log.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __OCPP_LOG_H_
#define __OCPP_LOG_H_

#ifdef __cplusplus
    extern "C"{
#endif

#include <stdio.h>

#define   __SAVE_LOG__     0
#define   __DEBUG_LOG__    1



#define BLACK     	"\033[0;32;30m"
#define RED       	"\033[0;32;31m"
#define GREEN     	"\033[0;32;32m"
#define YELLOW    	"\033[0;32;33m"
#define BLUE      	"\033[0;32;34m"
#define PURPLE    	"\033[0;32;35m"
#define DEEP_GREEN  "\033[0;32;36m"
#define WHITE       "\033[0;32;37m"
#define RESET     	"\033[0m"


enum OCPP_LOG_LEVELS_E{
	OCPP_LOG_NO = 0,
	OCPP_LOG_DEBUG,
	OCPP_LOG_INFO,
	OCPP_LOG_WARNING,
	OCPP_LOG_ERROR,
	OCPP_LOG_ANY
};


static const char * ocpp_log_Level_text[] = {
	"NONE",
	"DEBUG",
	"INFO",
	"WARNING",
	"ERROR",
	"ANY"
};


#if __SAVE_LOG__

#define OCPP_LOG_DEBUG(x,...)		  ocpp_log_add(OCPP_LOG_DEBUG,""x"", ##__VA_ARGS__)
#define OCPP_LOG_INFO(x,...)  	      ocpp_log_add(OCPP_LOG_INFO, ""x"", ##__VA_ARGS__)
#define OCPP_LOG_WARNING(x,...)       ocpp_log_add(OCPP_LOG_WARNING, ""x"", ##__VA_ARGS__)
#define OCPP_LOG_ERROR(x,...)         ocpp_log_add(OCPP_LOG_ERROR, ""x"", ##__VA_ARGS__)

#elif __DEBUG_LOG__

#define OCPP_LOG_DEBUG(x,...)         printf("[DEBUG]>>[%s]---[%s]---[%d]"x"\r\n",/*__FILE__*/"OCPP",__FUNCTION__,__LINE__,##__VA_ARGS__)
#define OCPP_LOG_INFO(x,...)          printf(DEEP_GREEN"[INFO]>>[%s]---[%s]---[%d]:"x"\r\n"RESET,/*__FILE__*/"OCPP",__FUNCTION__,__LINE__,##__VA_ARGS__)     
#define OCPP_LOG_WARNING(x,...)       printf(YELLOW"[WARNING]>>[%s]---[%s]---[%d]:"x"\r\n"RESET,/*__FILE__*/"OCPP",__FUNCTION__,__LINE__,##__VA_ARGS__) 
#define OCPP_LOG_ERROR(x,...)         printf(RED"[ERR]>>[%s]---[%s]---[%d]:"x"\r\n"RESET,/*__FILE__*/"OCPP",__FUNCTION__,__LINE__,##__VA_ARGS__) 

#else

#define OCPP_LOG_DEBUG(x...)
#define OCPP_LOG_INFO(x...)
#define OCPP_LOG_WARNING(x...)
#define OCPP_LOG_ERROR(x...)

#endif






void ocpp_log_add(enum OCPP_LOG_LEVELS_E warning_level,char *format,...);
void ocpp_log_deleteAllLog(void);
int ocpp_log_extractLogs(struct tm* startTime,struct tm * endTime,enum OCPP_LOG_LEVELS_E level);
void * ocpp_log_record_thread(void * arg);






#ifdef __cplusplus
        }
#endif


#endif

















