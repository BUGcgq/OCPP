/*
 * @Author: LIYAOHAN 1791002655@qq.com
 * @Date: 2023-04-01 13:30:21
 * @LastEditors: LIYAOHAN 1791002655@qq.com
 * @LastEditTime: 2023-04-27 17:52:21
 * @FilePath: /OCPP/ocpp_auxiliaryTool.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __OCPP_AUXILIARY_TOOL__H__
#define __OCPP_AUXILIARY_TOOL__H__

#ifdef __cplusplus
extern "C" {
#endif





char *ocpp_AuxiliaryTool_GetCurrentTime();
void ocpp_AuxiliaryTool_setSystemTime(const char * const timeStr);
char * ocpp_AuxiliaryTool_GenerateUUID();
int ocpp_AuxiliaryTool_GenerateInt();
unsigned int ocpp_AuxiliaryTool_getSystemTime_ms(void);
unsigned int ocpp_AuxiliaryTool_getDiffTime_ms(unsigned int *pu32_last_time);
int ocpp_AuxiliaryTool_charCounter(char* pString, char c);
int isWithinValidity(const char *timeStr);
// char** ocpp_AuxiliaryTool_str_split(char* a_str, const char a_delim);


#ifdef __cplusplus
}
#endif

#endif