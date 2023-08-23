/*
 * @Author: LIYAOHAN 1791002655@qq.com
 * @Date: 2023-04-23 17:31:12
 * @LastEditors: LIYAOHAN 1791002655@qq.com
 * @LastEditTime: 2023-05-05 08:41:02
 * @FilePath: /OCPP/ocpp_diagnostics.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

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
