#ifndef __OCPP_FIRMWARE_UPDATA__H__
#define __OCPP_FIRMWARE_UPDATA__H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "curl/curl.h"


int upload_file(const char *url,const char *local_file_path);
int download_file(const char *url, const char *local_file_path);
void *ocpp_chargePoint_UpdateFirmware_thread(void *arg);

#ifdef __cplusplus
}
#endif

#endif