#ifndef __OCPP_DIAGNOSTICS__H__
#define __OCPP_DIAGNOSTICS__H__

#ifdef __cplusplus
extern "C" {
#endif



#include "ocpp_package.h"






enum OCPP_PACKAGE_DIAGNOSTICS_STATUS_E  ocpp_diagnostics_getDiagnosticsStatus();
void ocpp_diagnostics_init(void);


#ifdef __cplusplus
}
#endif

#endif