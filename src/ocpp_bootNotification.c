#include <time.h>
#include "ocpp_chargePoint.h"
#include "ocpp_package.h"
#include "ocpp_bootNotification.h"
#include "ocpp_configurationKey.h"
#include "ocpp_chargingProfile.h"

/**
 * @description: boot线程
 * @param:
 * @return:
 */
void *ocpp_chargePoint_boot(void *arg)
{

    printf("create Boot thread\n");

    int sleep_s = 0;
    int needBootNotification = 1; // 初始时需要发送BootNotification
    enum OCPP_PACKAGE_CHARGEPOINT_STATUS_E oldStatus[ocpp_chargePoint->numberOfConnector];
    enum OCPP_PACKAGE_REGISTRATION_STATUS_E previousStatus;
    unsigned int sendHeartbeatonTime = ocpp_AuxiliaryTool_getSystemTime_ms();
    unsigned int BootNotificationTime = ocpp_AuxiliaryTool_getSystemTime_ms();
    unsigned int StatusNotificationTime = ocpp_AuxiliaryTool_getSystemTime_ms();
    unsigned int minimumStatusDurationTime = ocpp_AuxiliaryTool_getSystemTime_ms();
    bool minimumStatusDurationIsUse;
    unsigned int minimumStatusDuration = 1;
    int i;
    while (1)
    {

        if (ocpp_chargePoint->connect.isConnect)
        {
            if (ocpp_chargePoint->RegistrationStatus != OCPP_PACKAGE_REGISTRATION_STATUS_ACCEPTED)
            {
                if (needBootNotification) // 只有在需要发送BootNotification时才发送
                {
                    ocpp_chargePoint_sendBootNotification_req();
                    BootNotificationTime = ocpp_AuxiliaryTool_getSystemTime_ms();
                    needBootNotification = 0; // 发送后重置标志
                    for (i = 0; i <= ocpp_chargePoint->numberOfConnector; i++)
                    {
                        oldStatus[i] = ocpp_chargePoint->connector[i]->status;
                        ocpp_chargePoint_sendStatusNotification_Req(i);
                    }
                }
                if (ocpp_AuxiliaryTool_getDiffTime_ms(&BootNotificationTime) > 30 * 1000)
                {
                    needBootNotification = 1;
                }
            }
            else
            {
                minimumStatusDurationIsUse = ocpp_ConfigurationKey_getIsUse(ocpp_configurationKeyText[OCPP_CK_MinimumStatusDuration]);
                if (minimumStatusDurationIsUse)
                {
                    ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[OCPP_CK_MinimumStatusDuration], (void *)&minimumStatusDuration);
                    if (minimumStatusDuration < 60)
                    {
                        minimumStatusDuration = 60;
                    }
                    if (ocpp_AuxiliaryTool_getDiffTime_ms(&minimumStatusDurationTime) > minimumStatusDuration * 1000 && ocpp_chargePoint->RegistrationStatus == OCPP_PACKAGE_REGISTRATION_STATUS_ACCEPTED)
                    {
                        for (i = 0; i <= ocpp_chargePoint->numberOfConnector; i++)
                        {
                            ocpp_chargePoint_sendStatusNotification_Req(i);
                        }
                        minimumStatusDurationTime = ocpp_AuxiliaryTool_getSystemTime_ms();
                    }
                }
                ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[OCPP_CK_HeartbeatInterval], (void *)&sleep_s);
                if (sleep_s < 5)
                {
                    sleep_s = 5;
                }
                if (ocpp_AuxiliaryTool_getDiffTime_ms(&sendHeartbeatonTime) > sleep_s * 1000)
                {
                    ocpp_chargePoint_sendHeartbeat_Req();
                    sendHeartbeatonTime = ocpp_AuxiliaryTool_getSystemTime_ms();
                }
                if (ocpp_AuxiliaryTool_getDiffTime_ms(&StatusNotificationTime) > 2000 && ocpp_chargePoint->RegistrationStatus == OCPP_PACKAGE_REGISTRATION_STATUS_ACCEPTED)
                {
                    for (i = 0; i <= ocpp_chargePoint->numberOfConnector; i++)
                    {
                        if (oldStatus[i] != ocpp_chargePoint->connector[i]->status)
                        {

                            oldStatus[i] = ocpp_chargePoint->connector[i]->status;
                            ocpp_chargePoint_sendStatusNotification_Req(i);
                        }
                    }
                    StatusNotificationTime = ocpp_AuxiliaryTool_getSystemTime_ms();
                }
            }
        }

        else
        {
            sleep_s = 5;
            needBootNotification = 1; // 连接断开时重置标志，以便下次重新连接时发送BootNotification
            ocpp_chargePoint->RegistrationStatus = OCPP_PACKAGE_REGISTRATION_STATUS_MAX;
        }

        usleep(1000 * 1000);
    }
}