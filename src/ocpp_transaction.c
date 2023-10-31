#include <time.h>
#include "ocpp_chargePoint.h"
#include "ocpp_transaction.h"
#include "ocpp_configurationKey.h"
#include "ocpp_chargingProfile.h"
#include "ocpp_auxiliaryTool.h"
#include "ocpp_package.h"

static void ocpp_setChargingProfile(int connector)
{
    time_t currentTime;
    struct tm *timeInfo;
    time(&currentTime);
    timeInfo = localtime(&currentTime);
    int secondsSinceMidnight = timeInfo->tm_hour * 3600 + timeInfo->tm_min * 60 + timeInfo->tm_sec;
    ChargingProfile chargingProfiles;
    memset(&chargingProfiles, 0, sizeof(chargingProfiles));
    if (ocpp_ChargingProfile_read(connector, 1, &chargingProfiles) == 0)
    {
        // 遍历所有时间段，查找最小正差值
        int minDifference = INT_MAX; // 初始化为最大整数
        int selectedPeriod = -1;     // 初始化为无效值
        int i;
        int type = 1;
        if (chargingProfiles.chargingSchedule.chargingSchedulePeriods != NULL)
        {
            for (i = 0; i < chargingProfiles.chargingSchedule.numPeriods; i++)
            {
                int startPeriod = chargingProfiles.chargingSchedule.chargingSchedulePeriods[i].startPeriod;
                int difference = secondsSinceMidnight - startPeriod;
                if (difference >= 0 && difference < minDifference)
                {
                    minDifference = difference;
                    selectedPeriod = i;
                }
            }
            if (selectedPeriod >= 0)
            {
                if (strcmp(chargingProfiles.chargingSchedule.chargingRateUnit, "W") != 0)
                {
                    type = 2;
                }
                ocpp_chargePoint->setChargingProfile(connector, type, chargingProfiles.chargingSchedule.chargingSchedulePeriods[selectedPeriod].limit);
                ocpp_ChargingProfile_deleteByPurpose("TxProfile");
            }
            else
            {
                ocpp_chargePoint->setChargingProfile(0, 0, 0);
            }

            free(chargingProfiles.chargingSchedule.chargingSchedulePeriods);
        }
    }
    else
    {
        ocpp_chargePoint->setChargingProfile(0, 0, 0);
    }
}

void *ocpp_chargePoint_Transaction_thread(void *arg)
{
    int connector = (int)arg;
    ocpp_chargePoint_transaction_t *item = ocpp_chargePoint->transaction_obj[connector];
    write_data_lock();
    item->isTransaction = true;
    rwlock_unlock();
    int meterValueSampleInterval = 0;
    ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[OCPP_CK_MeterValueSampleInterval], (void *)&meterValueSampleInterval);
    if (meterValueSampleInterval < 10)
    {
        meterValueSampleInterval = 10;
    }
    char temptimestamp[32] = {0};
    char *StartuniqueId = ocpp_AuxiliaryTool_GenerateUUID();
    char *Starttimestamp = ocpp_AuxiliaryTool_GetCurrentTime();
    float Startmetervalue = ocpp_chargePoint->getCurrentMeterValues(connector);
    ocpp_chargePoint->sendStartTransaction(connector, item->startIdTag, item->reservationId, StartuniqueId, Starttimestamp, Startmetervalue);
    memcpy(item->lastUniqueId, StartuniqueId, sizeof(item->lastUniqueId));
    memcpy(temptimestamp, Starttimestamp, sizeof(temptimestamp));
    free(StartuniqueId);
    free(Starttimestamp);
    unsigned int RetriesInterval = ocpp_AuxiliaryTool_getSystemTime_ms();
    unsigned int MeterValInterval = ocpp_AuxiliaryTool_getSystemTime_ms();
    unsigned int sendStopTransaction = ocpp_AuxiliaryTool_getSystemTime_ms();
    enum OCPP_TRANSACTION_STATE_E state = OCPP_TARNSACTION_STATE_NONE;
    bool Startoffline = false;
    char *StopuniqueId = NULL;
    char *Stoptimestamp = NULL;
    float Stopmetervalue = 0;
    int sendCnt = 0;
    bool terminate = false;
    while (1)
    {
        if (terminate)
        {
            printf("充电事物线程退出\n");
            break;
        }
        switch (state)
        {
        case OCPP_TARNSACTION_STATE_NONE:
            if (ocpp_chargePoint->connect.isConnect)
            {
                if (ocpp_AuxiliaryTool_getDiffTime_ms(&RetriesInterval) >= 20000)
                {
                    item->reason = OCPP_PACKAGE_STOP_REASON_OTHER;
                    state = OCPP_TRANSACTION_STATE_STOPTRANSACTION;
                }
                else if (item->isRecStartTransaction_Conf)
                {
                    item->transactionId = item->startTransaction.transactionId;
                    if (item->startTransaction.idTagInfo.AuthorizationStatus == OCPP_LOCAL_AUTHORIZATION_ACCEPTED)
                    {
                        printf("允许充电\n");
                        ocpp_setChargingProfile(connector);
                        ocpp_chargePoint->startCharging(connector, item->startupType, Startmetervalue, item->transactionId, temptimestamp, item->startIdTag);
                        state = OCPP_TRANSACTION_STATE_CHARGING;
                    }
                    else
                    {
                        printf("不允许充电\n");
                        state = OCPP_TRANSACTION_STATE_STOPTRANSACTION;
                    }
                }
            }
            else
            {
                item->transactionId = ocpp_AuxiliaryTool_GenerateInt(); // 随机生成交易ID
                ocpp_setChargingProfile(connector);
                ocpp_chargePoint->startCharging(connector, 2, Startmetervalue, item->transactionId, temptimestamp, item->startIdTag);
                Startoffline = true;
                state = OCPP_TRANSACTION_STATE_CHARGING;
            }
            break;
        case OCPP_TRANSACTION_STATE_CHARGING:
            if (item->isStop)
            {
                state = OCPP_TRANSACTION_STATE_STOPTRANSACTION;
            }
            if (ocpp_AuxiliaryTool_getDiffTime_ms(&MeterValInterval) >= meterValueSampleInterval * 1000)
            {
                if (!Startoffline )
                {
                 ocpp_chargePoint_sendMeterValues(connector, item->transactionId);
                }
                MeterValInterval = ocpp_AuxiliaryTool_getSystemTime_ms();
            }
            break;
        case OCPP_TRANSACTION_STATE_STOPTRANSACTION:
            if (ocpp_AuxiliaryTool_getDiffTime_ms(&sendStopTransaction) >= 5 * 1000 && !Startoffline)
            {
                StopuniqueId = ocpp_AuxiliaryTool_GenerateUUID();
                Stoptimestamp = ocpp_AuxiliaryTool_GetCurrentTime();
                Stopmetervalue = ocpp_chargePoint->getCurrentMeterValues(connector);
                memcpy(item->lastUniqueId, StopuniqueId, sizeof(item->lastUniqueId));
                memset(temptimestamp, 0, sizeof(temptimestamp));
                memcpy(temptimestamp, Stoptimestamp, sizeof(temptimestamp));
                ocpp_chargePoint->sendStopTransaction(connector, item->startIdTag, item->transactionId, StopuniqueId, Stopmetervalue, Stoptimestamp, item->reason);
                free(StopuniqueId);
                free(Stoptimestamp);
                sendStopTransaction = ocpp_AuxiliaryTool_getSystemTime_ms();
                sendCnt++;
            }
            if (Startoffline || sendCnt >= 3 || !ocpp_chargePoint->connect.isConnect) // 离线姿态或者发三次sendStopTransaction超时
            {
                ocpp_chargePoint->stopCharging(connector, 2, Stopmetervalue, item->transactionId, temptimestamp, item->reason);
                terminate = true;
            }
            if (item->isRecStopTransaction_Conf && item->stopTransaction.idTagInfo.AuthorizationStatus == 0)
            {
                ocpp_chargePoint->stopCharging(connector, item->startupType, Stopmetervalue, item->transactionId, temptimestamp, item->reason);
                terminate = true;
            }
            break;
        default:
            break;
        }
        usleep(1 * 1000 * 1000); // 等待一段时间后重试
    }
    write_data_lock();
    memset(ocpp_chargePoint->transaction_obj[connector], 0, sizeof(ocpp_chargePoint_transaction_t));
    rwlock_unlock();
    item = NULL;
    return NULL;
}