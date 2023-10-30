#ifndef __OCPP_TRADSACTION__H__
#define __OCPP_TRADSACTION__H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

enum OCPP_TRANSACTION_STATE_E
{
    OCPP_TARNSACTION_STATE_NONE = 0,
    OCPP_TRANSACTION_STATE_CHARGING,
    OCPP_TRANSACTION_STATE_STOPTRANSACTION
};


void *ocpp_chargePoint_Transaction_thread(void *arg);

#ifdef __cplusplus
}
#endif

#endif