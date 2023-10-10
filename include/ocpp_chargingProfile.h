#ifndef __OCPP_CHARGING_PROFILE__H__
#define __OCPP_CHARGING_PROFILE__H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <sqlite3.h>


// 定义 ChargingSchedulePeriod 结构体
typedef struct
{
    int startPeriod;
    double limit; 
    int numberPhases;
} ChargingSchedulePeriod;

// 定义 ChargingSchedule 结构体
typedef struct
{
    int duration;
    char startSchedule[32];
    char chargingRateUnit[2];
    ChargingSchedulePeriod *chargingSchedulePeriods;
    int numPeriods;
} ChargingSchedule;

// 定义 ChargingProfile 结构体
typedef struct
{
    int connectorId;
    int chargingProfileId;
    int transactionId;
    int stackLevel;
    char chargingProfilePurpose[50];
    char chargingProfileKind[20];
    char recurrencyKind[10];
    char validFrom[32];
    char validTo[32];
    ChargingSchedule chargingSchedule;
} ChargingProfile;

void ocpp_ChargingProfile_init(sqlite3 *ocpp_db);
int ocpp_ChargingProfile_read(int connectorId, int transactionId, ChargingProfile *chargingProfile);
int ocpp_ChargingProfile_find(int connectorId, const char *chargingRateUnit, int duration, ChargingProfile *chargingProfile);
int ocpp_ChargingProfile_insert(int connectorId, const ChargingProfile *chargingProfile);
void printChargingProfile(const ChargingProfile *chargingProfile);
int ocpp_ChargingProfile_delete(int connectorId, int stackLevel, int chargingProfileId);
int ocpp_ChargingProfile_deleteByPurpose(const char *chargingProfilePurpose);
#ifdef __cplusplus
}
#endif

#endif