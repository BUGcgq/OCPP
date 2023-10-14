#ifndef __OCPP_OFFLINE__H__
#define __OCPP_OFFLINE__H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sqlite3.h>
#include "ocpp_package.h"
#include <pthread.h>

void ocpp_order_init(sqlite3 *ocpp_db);

#ifdef __cplusplus
}
#endif

#endif