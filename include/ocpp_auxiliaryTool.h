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



#ifdef __cplusplus
}
#endif

#endif