#ifndef __OCPP_DIAGNOSTICS__H__
#define __OCPP_DIAGNOSTICS__H__

#ifdef __cplusplus
extern "C" {
#endif



#include "ocpp_package.h"


void *ocpp_chargePoint_diagnostics_upload_thread(void *arg);


#ifdef __cplusplus
}
#endif

#endif