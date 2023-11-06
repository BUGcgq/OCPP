#ifndef __OCPP_CONNECT__H__
#define __OCPP_CONNECT__H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "ocpp_chargePoint.h"


void ocpp_connect_init(ocpp_connect_t * const connect);

#ifdef __cplusplus
}
#endif

#endif