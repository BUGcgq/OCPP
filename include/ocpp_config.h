#ifndef __OCPP_CONFIG__H__
#define __OCPP_CONFIG__H__

#ifdef __cplusplus
extern "C" {
#endif

#include<sqlite3.h>

#define OCPP_DATABASE_FILE_NAME                    "/app/etc/ocpp.db"
#define  OCPP_FIRMWARE_UPDATA_FILEPATH    "/app/update/EVCM-SD10.tar.gz"    //固件更新文件路径
#define  OCPP_DIAGNOSTICS_UPDATA_FILEPATH    "/app/core/run_log.txt"                //更新固件名

#define OCPP_AUTHORIZATION_IDTAG_LEN                               21                            //卡号长度
#define OCPP_LOCAL_AUTHORIZATION_CACHE_MAX                         10                          //本地认证缓存长度
#define OCPP_LOCAL_AUTHORIZATION_LIST_MAX                          1000                          //本地认证列表长度

#ifdef __cplusplus
}
#endif

#endif