/*
 * @Author: LIYAOHAN 1791002655@qq.com
 * @Date: 2023-04-23 15:24:03
 * @LastEditors: LIYAOHAN 1791002655@qq.com
 * @LastEditTime: 2023-05-05 17:32:11
 * @FilePath: /OCPP/ocpp_firmwareUpdata.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __OCPP_FIRMWARE_UPDATA__H__
#define __OCPP_FIRMWARE_UPDATA__H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "curl/curl.h"


int upload_file(const char *upload_url,const char *local_file_path);
int download_file(const char *url, const char *save_path);


#ifdef __cplusplus
}
#endif

#endif