#include <stdio.h>
#include "ocpp_diagnostics.h"
#include "ocpp_chargePoint.h"

extern ocpp_chargePoint_t *ocpp_chargePoint;
/**
 * @description:
 * @param
 * @param
 * @return
 */
void *ocpp_chargePoint_diagnostics_upload_thread(void *arg)
{
    printf("create upload thread");
    ocpp_package_GetDiagnostics_req_t *diagnostics = (ocpp_package_GetDiagnostics_req_t *)arg;

    int retries = 0;

    while (1)
    {
        if (retries >= diagnostics->retries)
        {
            ocpp_chargePoint->ocpp_diagnostics_lastDiagnosticsStatus = OCPP_PACKAGE_DIAGNOSTICS_STATUS_UPLOAD_FAILED;
            ocpp_chargePoint_sendDiagnosticsStatusNotification_Req(OCPP_PACKAGE_DIAGNOSTICS_STATUS_UPLOAD_FAILED);
            break;
        }
        ocpp_chargePoint_sendDiagnosticsStatusNotification_Req(OCPP_PACKAGE_DIAGNOSTICS_STATUS_UPLOADING);
        if (upload_file(diagnostics->location, OCPP_DIAGNOSTICS_UPDATA_FILEPATH) == 0)
        {
            ocpp_chargePoint->ocpp_diagnostics_lastDiagnosticsStatus = OCPP_PACKAGE_DIAGNOSTICS_STATUS_UPLOADED;
            ocpp_chargePoint_sendDiagnosticsStatusNotification_Req(OCPP_PACKAGE_DIAGNOSTICS_STATUS_UPLOADED);
            break;
        }
        ocpp_chargePoint->ocpp_diagnostics_lastDiagnosticsStatus = OCPP_PACKAGE_DIAGNOSTICS_STATUS_UPLOAD_FAILED;
        ocpp_chargePoint_sendDiagnosticsStatusNotification_Req(OCPP_PACKAGE_DIAGNOSTICS_STATUS_UPLOAD_FAILED);
        retries++;
        sleep(diagnostics->retryInterval);
    }
    ocpp_chargePoint->ocpp_diagnostics_lastDiagnosticsStatus = OCPP_PACKAGE_DIAGNOSTICS_STATUS_IDLE;
    free(diagnostics);
    return NULL;
}
