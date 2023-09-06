#include "ocpp_diagnostics.h"

static enum OCPP_PACKAGE_DIAGNOSTICS_STATUS_E ocpp_diagnostics_lastDiagnosticsStatus;

/**
 * @description:
 * @return {*}
 */
enum OCPP_PACKAGE_DIAGNOSTICS_STATUS_E ocpp_diagnostics_getDiagnosticsStatus()
{

    return ocpp_diagnostics_lastDiagnosticsStatus;
}

/**
 * @description:
 * @return {*}
 */
void ocpp_diagnostics_init(void)
{
    ocpp_diagnostics_lastDiagnosticsStatus = OCPP_PACKAGE_DIAGNOSTICS_STATUS_IDLE;
}
