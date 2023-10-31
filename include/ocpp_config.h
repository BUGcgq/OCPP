#ifndef __OCPP_CONFIG__H__
#define __OCPP_CONFIG__H__

#ifdef __cplusplus
extern "C" {
#endif

#include<sqlite3.h>

#define  OCPP_DATABASE_FILE_NAME                                 "/app/etc/ocpp.db"
#define  OCPP_FIRMWARE_UPDATA_FILEPATH                           "/app/update/EVCM-SD10.tar.gz"               //下载文件路径
#define  OCPP_DIAGNOSTICS_UPDATA_FILEPATH                        "/app/core/run_log.txt"                      //上传文件路径
#define  OCPP_DIAGNOSTICS_UPDATA_FILENAME                        "run_log.txt"                      //文件名

#define OCPP_LOCAL_AUTHORIZATION_LIST_MAX                          1000                          //本地认证列表长度
#define OCPP_CONNECT_SEND_BUFFER                                   2048                          // 发送缓存区大小
#define OCPP_CONFIGURATION_KEY_SIZE                                75                             //数据库最大key数
#define OCPP_CONFIGURATION_VALUES_STRING_MAX                       512                            //插入key数据库的key最大长度
#define MESSAGE_UNIQUE_LEN                                         38                             //发送Unique长度
#define MESSAGE_CONTENT_LEN                                        2048                           //发送message长度
#define MAX_TIMEOUT_SECONDS                                        4                              //判断回复信息超时的时间
#define SERVER_RECONNECT_INTERVAL                                  5                              // 断线重连间隔，单位为秒
#define MAX_TIMEOUT_COUNT                                          4                              //最大允许超时次数
#define CURL_FTP_mode                                              1                             //FTP上传和下载的模式，0是主动，1是被动
#ifdef __cplusplus
}
#endif

#endif