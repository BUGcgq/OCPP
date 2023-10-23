#ifndef __OCPP_AUXILIARY_TOOL__H__
#define __OCPP_AUXILIARY_TOOL__H__

#ifdef __cplusplus
extern "C" {
#endif


#include "curl/curl.h"


char *ocpp_AuxiliaryTool_GetCurrentTime();
void ocpp_AuxiliaryTool_setSystemTime(const char * const timeStr);
char * ocpp_AuxiliaryTool_GenerateUUID();
int ocpp_AuxiliaryTool_GenerateInt();
unsigned int ocpp_AuxiliaryTool_getSystemTime_ms(void);
unsigned int ocpp_AuxiliaryTool_getDiffTime_ms(unsigned int *pu32_last_time);
int ocpp_AuxiliaryTool_charCounter(char* pString, char c);

void initialize_rwlock();
void read_data_lock();
void write_data_lock();
void rwlock_unlock();
void destroy_rwlock();

int ocpp_download_file(const char *url, const char *local_file_path, int mode);
int ocpp_upload_file(const char *url, const char *local_file_path, int mode);

#ifdef __cplusplus
}
#endif

#endif