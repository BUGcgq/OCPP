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



typedef struct{

    char anyURI[256];
    int retries;
    char retrieveDate[32];
    int retryInterval;

    char usr[32];
    char passwd[32];
    char ser_filepath[512];
    char ser_filename[64];
    char new_filename[64];
    int control_sock;
 
}ocpp_firmwareUpdata_data_t;


void * ocpp_firmwareUpdata_thread(void * arg);

enum OCPP_PACKAGE_FIRMWARE_STATUS_E ocpp_firmwareUpdate_getUpdataStatus();
void ocpp_firmwareUpdate_init();




#ifdef __cplusplus
}
#endif

#endif