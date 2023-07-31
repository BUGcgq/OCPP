/*
 * @Author: LIYAOHAN 1791002655@qq.com
 * @Date: 2023-04-02 10:20:14
 * @LastEditors: LIYAOHAN 1791002655@qq.com
 * @LastEditTime: 2023-05-07 10:26:53
 * @FilePath: /OCPP/ocpp_config.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __OCPP_CONFIG__H__
#define __OCPP_CONFIG__H__

#ifdef __cplusplus
extern "C" {
#endif

#include<sqlite3.h>

#define OCPP_DATABASE_FILE_NAME                    "/app/etc/ocpp.db"
#define OCPP_OFFLINE_FILE_NAME                     "/app/etc/offline.txt"



#define  OCPP_LOGS_MAX_CNT                20000                             //最大存储的日志数目
#define  OCPP_LOGS_MAX_OVERFLOW_CNT       1000                              //允许溢出的日志数目
#define  OCPP_LOGS_QUEUE_NODE_MAX         200                               //日志队列节点最大个数,防止处理过慢,导致内存占用过多
#define  OCPP_LOGS_FILEPATH               "/app/etc/ocpp_logs.log"          //导出日志存储路径
#define  OCPP_LOGS_FILENAME               "ocpp_logs.log"                   //日志文件名
#define  OCPP_LOGS_DATABASE_PATH          "/app/etc/ocpp_logs.db"           //日志数据库路径
#define  OCPP_FIRMWARE_UPDATA_FILEPATH    "/app/update/EVCM-SD10.tar.gz"    //固件更新文件路径
#define  OCPP_FIRMWARE_UPDATA_FILENAME    "EVCM-SD10.tar.gz"                //更新固件名


#define OCPP_OFFOLINE_FILE_MAX_SIZE       (10 * 1024 * 1024 * 1024)          //10M



#define OCPP_AUTHORIZATION_IDTAG_LEN                               21                            //卡号长度
#define OCPP_LOCAL_AUTHORIZATION_CACHE_MAX                         100                            //本地认证缓存长度
#define OCPP_LOCAL_AUTHORIZATION_LIST_MAX                          1000                          //本地认证列表长度

#ifdef __cplusplus
}
#endif

#endif