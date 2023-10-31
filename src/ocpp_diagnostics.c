#include <stdio.h>
#include "ocpp_diagnostics.h"
#include "ocpp_chargePoint.h"
#include "ocpp_auxiliaryTool.h"
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
    char url[512];  // 这里假设目标字符串足够大
    snprintf(url, sizeof(url), "%s%s", diagnostics->location, OCPP_DIAGNOSTICS_UPDATA_FILENAME);

    while (1)
    {
        if (retries >= diagnostics->retries)
        {
            write_data_lock();
            ocpp_chargePoint->ocpp_diagnostics_lastDiagnosticsStatus = OCPP_PACKAGE_DIAGNOSTICS_STATUS_UPLOAD_FAILED;
            rwlock_unlock();
            ocpp_chargePoint_sendDiagnosticsStatusNotification_req(OCPP_PACKAGE_DIAGNOSTICS_STATUS_UPLOAD_FAILED);
            break;
        }
        ocpp_chargePoint_sendDiagnosticsStatusNotification_req(OCPP_PACKAGE_DIAGNOSTICS_STATUS_UPLOADING);
        if (ocpp_upload_file(url, OCPP_DIAGNOSTICS_UPDATA_FILEPATH,CURL_FTP_mode) == 0)
        {
            write_data_lock();
            ocpp_chargePoint->ocpp_diagnostics_lastDiagnosticsStatus = OCPP_PACKAGE_DIAGNOSTICS_STATUS_UPLOADED;
            rwlock_unlock();
            ocpp_chargePoint_sendDiagnosticsStatusNotification_req(OCPP_PACKAGE_DIAGNOSTICS_STATUS_UPLOADED);
            break;
        }
        write_data_lock();
        ocpp_chargePoint->ocpp_diagnostics_lastDiagnosticsStatus = OCPP_PACKAGE_DIAGNOSTICS_STATUS_UPLOAD_FAILED;
        rwlock_unlock();
        ocpp_chargePoint_sendDiagnosticsStatusNotification_req(OCPP_PACKAGE_DIAGNOSTICS_STATUS_UPLOAD_FAILED);
        retries++;
        sleep(diagnostics->retryInterval);
    }
    write_data_lock();
    ocpp_chargePoint->ocpp_diagnostics_lastDiagnosticsStatus = OCPP_PACKAGE_DIAGNOSTICS_STATUS_IDLE;
    rwlock_unlock();
    free(diagnostics);
    return NULL;
}
