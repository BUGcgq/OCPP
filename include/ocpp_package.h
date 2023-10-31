#ifndef __OCPP_PACKAGE__H__
#define __OCPP_PACKAGE__H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ocpp_localAuthorization.h"
#include "ocpp_chargePoint.h"
#include "ocpp_chargingProfile.h"

enum OCPP_PACKAGE{
	OCPP_PACKAGE_NONE = 0,
	OCPP_PACKAGE_AUTHORIZE,//身份认证
	OCPP_PACKAGE_BOOT_NOTIFICATION,//充电桩登录过程，用于通知中央系统充电站已启动并准备好接受充电请求。
	OCPP_PACKAGE_CANCEL_RESERVATION,//取消预约的包，用于取消之前设置的充电预约。
	OCPP_PACKAGE_CHANGE_AVAILABILITY,//改变可用性的包，用于更改充电点的可用性状态，如设置为可用或不可用。
	OCPP_PACKAGE_CHANGE_CONFIGURATION,//改变配置的包，用于更改充电站的配置参数。
	OCPP_PACKAGE_CLEAR_CACHE,//清除缓存的包，用于请求充电站清除缓存数据。
	OCPP_PACKAGE_CLEAR_CHARGINGPROFILE,//清除充电配置文件的包，用于删除充电站中特定的充电配置文件。
	OCPP_PACKAGE_DATATRANSFER,//数据传输的包，用于在充电站和中央系统之间传递自定义数据。
	OCPP_PACKAGE_DIAGNOSTICS_STATUS_NOTIFICATION,//诊断状态通知的包，用于通知充电站诊断操作的状态，例如诊断开始、进行中或完成等。
	OCPP_PACKAGE_FIRMWARE_STATUS_NOTIFICATION,//固件下载状态通知的包，用于通知固件下载操作的状态，例如下载开始、进行中或完成等。
	OCPP_PACKAGE_GET_COMPOSITESCAEDULE,//获取组合日程表的包，用于请求获取充电站中已定义的组合日程表。。
	OCPP_PACKAGE_GET_CONFIGURATION,//获取充电桩的配置信息的包，用于请求获取充电站的配置参数和设置。
	OCPP_PACKAGE_GET_DIAGNOSTICS,//获取诊断信息的包，用于请求获取充电站的诊断信息，例如日志文件、错误码等。
	OCPP_PACKAGE_GET_LOCALLISTVERSION,//请求本地存储的授权用户或车辆列表的版本号。
	OCPP_PACKAGE_HEARTBEAT,//用于指示充电站或中央系统仍处于活动状态并保持连接的心跳包。
	OCPP_PACKAGE_METERVALUES,//包含充电站当前电表值的信息，例如电流、电压、功率和能量测量。
	OCPP_PACKAGE_REMOTE_STARTTRANSACTION,//远程启动充电事务的包。
	OCPP_PACKAGE_REMOTE_STOPTRANSACTION,//远程停止充电事务的包。
	OCPP_PACKAGE_RESERVENOW,//预约当前充电点的包。
	OCPP_PACKAGE_RESET,//重置充电站的包，将其恢复到初始状态。
	OCPP_PACKAGE_SENDLOCALLIST,//将本地授权用户或车辆列表发送到充电站的包。
	OCPP_PACKAGE_SETCHARGINGPROFILE,//设置充电站的充电配置文件的包。
	OCPP_PACKAGE_STARTTRANSACTION,//启动充电事务的包。
	OCPP_PACKAGE_STATUS_NOTIFICATION,//发送充电站状态通知的包。
	OCPP_PACKAGE_STOPTRANSACTION,//停止充电事务的包。
	OCPP_PACKAGE_TRIGGER_MESSAGE,//触发通知,服务器请求充电点特定信息
	OCPP_PACKAGE_UNLOCK_CONNECTOR,//断开本地连接器的包。
	OCPP_PACKAGE_UPDATE_FIRMWARE,//更新充电站固件的包。


	OCPP_PACKAGE_CALLERROR,
	OCPP_PACKAGE_CALL,
	OCPP_PACKAGE_MAX

};

static const char * ocpp_package_text[] = {
	"None",
	"Authorize",
	"BootNotification",
	"CancelReservation",
	"ChangeAvailability",
	"ChangeConfiguration",
	"ClearCache",
	"ClearChargingProfile",
	"DataTransfer",
	"DiagnosticsStatusNotification",
	"FirmwareStatusNotification",
	"GetCompositeSchedule",
	"GetConfiguration",
	"GetDiagnostics",
	"GetLocalListVersion",
	"Heartbeat",
	"MeterValues",
	"RemoteStartTransaction",
	"RemoteStopTransaction",
	"ReserveNow",
	"Reset",
	"SendLocalList",
	"SetChargingProfile",
	"StartTransaction",
	"StatusNotification",
	"StopTransaction",
	"TriggerMessage",
	"UnlockConnector",
	"UpdateFirmware",
	"CallError",
	"Call"
};



enum OCPP_PACKAGE_MESSAGE_TYPES_E{
	OCPP_PACKAGE_CALL_MESSAGE = 2,//请求消息
	OCPP_PACKAGE_CALL_RESULT = 3,//回应消息
	OCPP_PACKAGE_CALL_ERROR = 4,//错误消息

	OCPP_PACKAGE_CALL_MAX
};



//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>Authorize.req
typedef struct{
	char idTag[OCPP_AUTHORIZATION_IDTAG_LEN];        //IdToken is case insensitive.

}ocpp_package_Authorize_req_t;



//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>BootNotification.req
enum OCPP_PACKAGE_CHARGEPOINT_MODEL{
	OCPP_PACKAGE_CHARGEPOINT_MODEL_AC_SINGLE_PHASE_CORE = 0,
	OCPP_PACKAGE_CHARGEPOINT_MODEL_AC_THREE_PHASE_CORE,
	OCPP_PACKAGE_CHARGEPOINT_MODEL_DC_CORE,
	OCPP_PACKAGE_CHARGEPOINT_MODEL_DC_EXTENDED,
	OCPP_PACKAGE_CHARGEPOINT_MODEL_DC_COMBO_CORE,
	OCPP_PACKAGE_CHARGEPOINT_MODEL_DC_UNIQUE,

	OCPP_PACKAGE_CHARGEPOINT_MODE_MAX
};


static const char * ocpp_package_chargePointModel_text[] = {
	"AC_single_phase_core",
	"AC_three_phase_core",
	"DC_core",
	"DC_extended",
	"DC_combo_core",
	"DC_unique"
};

typedef struct{
	char currentTime[32];
	int interval;
	enum OCPP_PACKAGE_REGISTRATION_STATUS_E status;

}ocpp_package_BootNotification_conf_t;

typedef struct{
	char chargeBoxSerialNumberIsUse:1;
	char chargeBoxSerialNumber[25];

	char chargePointModel[20];     

	char chargePointSerialNumberIsUse:1;
	char chargePointSerialNumber[25];

	char chargePointVendor[20];

	char firmwareVersionIsUse:1;
	char firmwareVersion[50];

	char iccidIsUse:1;
	char iccid[20];

	char imsiIsUse:1;
	char imsi[20];

	char meterSerialNumberIsUse:1;
	char meterSerialNumber[25];

	char meterTypeIsUse:1;
	char meterType[25];

}ocpp_package_BootNotification_req_t;



static const char * ocpp_package_RegistrationStatus_text[] = {
	"Accepted",
	"Pending",
	"Rejected"
};



//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>CancelReservation.req
typedef struct{
	int reservationId;

}ocpp_package_CancelReservation_req_t;


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>CancelReservation.conf
enum OCPP_PACKAGE_CANCEL_RESERVATION_STATUS_E{
	OCPP_PACKAGE_CANCEL_RESERVATION_STATUS_ACCEPTED = 0,//操作被接受
	OCPP_PACKAGE_CANCEL_RESERVATION_STATUS_REJECTED,//操作被拒绝

	OCPP_PACKAGE_CANCEL_RESERVATION_STATUS_MAX

};

static const char * ocpp_package_CancelReservationStatus_text[] = {
	"Accepted",
	"Rejected"
};

typedef struct{
	enum OCPP_PACKAGE_CANCEL_RESERVATION_STATUS_E status;

}ocpp_package_CancelReservation_conf_t;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>ChangeAvailability.req
enum OCPP_PACKAGE_AVAILABILITY_TYPE_E{

	OCPP_PACKAGE_AVAILABILITY_TYPE_INOPERATIVE = 0,//设备不可用
	OCPP_PACKAGE_AVAILABILITY_TYPE_OPERATIVE,//设备可用

	OCPP_PACKAGE_AVAILABILITY_TYPE_MAX

};

static const char * ocpp_package_AvailabilityType_text[] = {
	"Inoperative",
	"Operative"
};

typedef struct{
	int connectorId;
	enum OCPP_PACKAGE_AVAILABILITY_TYPE_E type;

}ocpp_package_ChangeAvailability_req_t;


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>ChangeAvailability.conf
enum OCPP_PACKAGE_AVAILABILITY_STATUS_E{
	OCPP_PACKAGE_AVAILABILITY_STATUS_ACCEPTED = 0,//设备可用状态已被接受
	OCPP_PACKAGE_AVAILABILITY_STATUS_REJECTED,//设备可用状态被拒绝
	OCPP_PACKAGE_AVAILABILITY_STATUS_SCHEDULED,//设备可用状态已被安排

	OCPP_PACKAGE_AVAILABILITY_STATUS_MAX

};

static const char * ocpp_package_AvailabilityStatus_text[] = {
	"Accepted",
	"Rejected",
	"Scheduled"
};

typedef struct{
	enum OCPP_PACKAGE_AVAILABILITY_STATUS_E status;

}ocpp_package_ChangeAvailability_conf_t;


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>ChangeConfiguration.req
typedef struct{
	char key[50];
	char value[500];

}ocpp_package_ChangeConfiguration_req_t;


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>ChangeConfiguration.conf
enum OCPP_PACKAGE_CONFIGURATION_STATUS_E{
	OCPP_PACKAGE_CONFIGURATION_STATUS_ACCEPTED = 0,//配置被接受。
	OCPP_PACKAGE_CONFIGURATION_STATUS_REJECTED,//配置被拒绝
	OCPP_PACKAGE_CONFIGURATION_STATUS_REBOOT_REQUIRED,//需要重新启动设备以使配置生效
	OCPP_PACKAGE_CONFIGURATION_STATUS_NOTSUPPORTED,//配置不被支持

	OCPP_PACKAGE_CONFIGURATION_STATUS_MAX
};

static const char * ocpp_package_ConfigurationStatus_text[] = {
	"Accepted",
	"Rejected",
	"RebootRequired",
	"NotSupported"
};

typedef struct{
	enum OCPP_PACKAGE_CONFIGURATION_STATUS_E status;

}ocpp_package_ChangeConfiguration_conf_t;


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>ClearCache.req
typedef struct{

}ocpp_package_ClearCache_req_t;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>ClearCache.conf
enum OCPP_PACKAGE_CLEAR_CACHE_STATUS_E{
	OCPP_PACKAGE_CLEAR_CACHE_STATUS_ACCEPTED = 0,//清除Cache被接受
	OCPP_PACKAGE_CLEAR_CACHE_STATUS_REJECTED,//清除Cache被拒绝

	OCPP_PACKAGE_CLEAR_CACHE_STATUS_MAX

};

static const char * ocpp_package_ClearCacheStatus_text[] = {
	"accepted",
	"rejected"
};


typedef struct{
	enum OCPP_PACKAGE_CLEAR_CACHE_STATUS_E status;

}ocpp_package_ClearCache_conf_t;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>ClearChargingProfile.req
enum OCPP_PACKAGE_CHARGING_PROFILE_PURPOSE_TYPE_E{
	OCPP_PACKAGE_CHARGING_PROFILE_PURPOSE_TYPE_CHARGEPOINTMAXPROFILE = 0,//充电桩的最大充电档案配置
	OCPP_PACKAGE_CHARGING_PROFILE_PURPOSE_TYPE_TXDEFAULTPROFILE,//交易的默认充电档案配置
	OCPP_PACKAGE_CHARGING_PROFILE_PURPOSE_TYPE_TXPROFILE,//交易的充电档案配置

	OCPP_PACKAGE_CHARGING_PROFILE_PURPOSE_TYPE_MAX
};

static const char * ocpp_package_ChargingProfilePurposeType_text[] = {
	"ChargePointMaxProfile",
	"TxDefaultProfile",
	"TxProfile"
};

typedef struct{
	char idIsUse:1;
	int id;

	char connectorIdIsUse:1;
	int connectorId;

	char chargingProfilePurposeIsUse:1;
	enum OCPP_PACKAGE_CHARGING_PROFILE_PURPOSE_TYPE_E chargingProfilePurpose;

	char stackLevelIsUse:1;
	int stackLevel;
}ocpp_package_ClearChargingProfile_req_t;

 //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>ClearChargingProfile.conf
enum OCPP_PACKAGE_CLEAR_CHARGING_PROFILE_STATUS_E{
	OCPP_PACKAGE_CLEAR_CHARGING_PROFILE_STATUS_ACCEPTED = 0,//清除充电档案请求被接受
	OCPP_PACKAGE_CLEAR_CHARGING_PROFILE_STATUS_UNKNOWN,//清除充电档案请求的状态未知

	OCPP_PACKAGE_CLEAR_CHARGING_PROFILE_STATUS_MAX

};

static const char * ocpp_package_ClearChargingProfileStatus_text[] = {
	"Accepted",
	"Unknown"
};

 
typedef struct{
	enum OCPP_PACKAGE_CLEAR_CHARGING_PROFILE_STATUS_E  status;

}ocpp_package_ClearChargingProfile_conf_t;


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>DataTransfer.req
typedef struct{
	char vendorId[255];

	char messageIdIsUse:1;
	char messageId[50];
	
	char dataIsUse:1;
	int dataLen;
	char data[0];

}ocpp_package_DataTransfer_req_t;


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>DataTransfer.conf
enum OCPP_PACKAGE_DATATRANSFER_STATUS_E{
	OCPP_PACKAGE_DATATRANSFER_STATUS_ACCEPTED = 0,//数据传输请求被接受
	OCPP_PACKAGE_DATATRANSFER_STATUS_REJECTED,//数据传输请求被拒绝
	OCPP_PACKAGE_DATATRANSFER_STATUS_UNKNOWN_MESSAGEID,//表示数据传输请求中的消息ID未知
	OCPP_PACKAGE_DATATRANSFER_STATUS_UNKNOWN_VENDORID,//数据传输请求中的供应商ID未知

	OCPP_PACKAGE_DATATRANSFER_STATUS_MAX

};

static const char * ocpp_package_DataTransferStatus_text[] = {
	"Accepted",
	"Rejected",
	"UnknownMessageId",
	"UnknownVendorId"
};


typedef struct{
	enum OCPP_PACKAGE_DATATRANSFER_STATUS_E status;

	char dataIsUse:1;
	int dataLen;
	char data[0];
}ocpp_package_DataTransfer_conf_t;


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>DiagnosticsStatusNotification.req
static const char * ocpp_package_DiagnosticsStatus_text[] = {
	"Idle",
	"Uploaded",
	"UploadFailed",
	"Uploading"
};

typedef struct{
	enum OCPP_PACKAGE_DIAGNOSTICS_STATUS_E status;

}ocpp_package_DiagnosticsStatusNotification_req_t;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>DiagnosticsStatusNotification.conf

static const char * ocpp_package_FirmwareStatus[] = {
	"Downloaded",
	"DownloadFailed",
	"Downloading",
	"Idle",
	"InstallationFailed",
	"Installing",
	"Installed"
};

typedef struct{
	enum OCPP_PACKAGE_FIRMWARE_STATUS_E status;

}ocpp_package_FirmwareStatusNotification_req_t;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>FirmwareStatusNotification.conf

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>GetCompositeSchedule.req
enum OCPP_PACKAGE_CHARGING_RATE_UNIT_TYPE_E{
	OCPP_PACKAGE_CHARGING_RATE_UNIT_TYPE_W = 0,
	OCPP_PACKAGE_CHARGING_RATE_UNIT_TYPE_A,

	OCPP_PACKAGE_CHARGING_RATE_UNIT_TYPE_MAX,

};

static const char * ocpp_package_ChargingRateUnitType_text[] = {
	"W",
	"A"
};

typedef struct{
	int connectorId;
	int duration;

	char chargingRateUnitIsUse:1;
	enum OCPP_PACKAGE_CHARGING_RATE_UNIT_TYPE_E chargingRateUnit;

}ocpp_package_GetCompositeSchedule_req_t;


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>GetCompositeSchedule.conf
enum OCPP_PACKAGE_GETCOMPOSITESCHEDULE_STATUS_E{
	OCPP_PACKAGE_GETCOMPOSITESCHEDULE_STATUS_ACCEPTED = 0,
	OCPP_PACKAGE_GETCOMPOSITESCHEDULE_STATUS_REJECTED,

	OCPP_PACKAGE_GETCOMPOSITESCHEDULE_STATUS_MAX,
};
static const char * ocpp_package_GetCompositeScheduleStatus_text[] = {
	"Accepted",
	"Rejected"
};


typedef struct{
	int startPeriod;
	double limit;
	int numberPhases;//
}ocpp_package_chargingSchedulePeriod_t;

typedef struct{
	int numberOfchargingSchedulePeriod; //结构体数组大小，遍历这个结构体的时候用到
	int duration;//[非必须]  配置持续时间，如果为空，最后一个chargingSchedulePeriod配置的值为最后持续时间
	int startSchedule;//[非必须] 时刻表的起始点，如果为空，起始值为一次充电的开始时间
    char chargingRateUnit[32];  // 时刻表配置项单位  W：功率配置（直流桩一般限制功率） A：电流配置（交流桩一般限制电流）
    ocpp_package_chargingSchedulePeriod_t * chargingSchedulePeriod;

}ocpp_package_chargingSchedule_t;


typedef struct{
	enum OCPP_PACKAGE_GETCOMPOSITESCHEDULE_STATUS_E status;

	char connectorIdIsUse:1;
	int connectorId;

	char scheduleStartIsUse:1;
	char scheduleStart[32];

	char chargingScheduleIsUse:1;
	ocpp_package_chargingSchedule_t chargingSchedule;

}ocpp_package_GetCompositeSchedule_conf_t;


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>GetConfiguration.req
typedef struct {
    int numberOfKey;
    char **key;
} ocpp_package_GetConfiguration_req_t;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>GetConfiguration.conf
typedef struct{
	int  numberOfKey;
	char key[50];
	int type;
	int accessibility;
	int IsUse;
	char value[500];
}ocpp_package_GetConfiguration_conf_t;




//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>GetDiagnostics.req
typedef struct{
	char location[256];

	char retriesIsUse:1;
	int retries;

	char retryIntervalIsUse:1;
	int retryInterval;

	char startTimeIsUse:1;
	char startTime[32];

	char stopTimeIsUse:1;
	char stopTime[32];

}ocpp_package_GetDiagnostics_req_t;


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>GetDiagnostics.conf
typedef struct{
	char fileNameIsUse:1;
	char fileName[256];
}ocpp_package_GetDiagnostics_conf_t;


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>GetLocalListVersion.req
typedef struct{

	char reserved;            //预留
}ocpp_package_GetLocalListVersion_req_t;
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>GetLocalListVersion.conf

typedef struct{
	int listVersion;

}ocpp_package_GetLocalListVersion_conf_t;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>Heartbeat.req
typedef struct{

	char reserved;            //预留
}ocpp_package_Heartbeat_req_t;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>Heartbeat.conf
typedef struct{

	char currentTime[32];
}ocpp_package_Heartbeat_conf_t;


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>MeterValues.req
//MS = MeterValue SampledValue
enum OCPP_PACKAGE_METERVALUES_MS_CONTEXT_E{
	OCPP_PACKAGE_METERVALUES_MS_CONTEXT_INTERRUPTION_BEGIN = 0,//采样开始前中断
	OCPP_PACKAGE_METERVALUES_MS_CONTEXT_INTERRUPTION_END,//采样结束后中断
	OCPP_PACKAGE_METERVALUES_MS_CONTEXT_OTHER,//采样值上下文为"Other"，其他未指定的情况
	OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_CLOCK,//采样值上下文为"Sample.Clock"，即时钟同步采样
	OCPP_PACKAGE_METERVALUES_MS_CONTEXT_SAMPLE_PERIODIC,//采样值上下文为"Sample.Periodic"，周期性采样
	OCPP_PACKAGE_METERVALUES_MS_CONTEXT_TRANSACTION_BEGIN,//采样值上下文为"Transaction.Begin"，即交易开始时的采样
	OCPP_PACKAGE_METERVALUES_MS_CONTEXT_TRANSACTION_END,//采样值上下文为"Transaction.End"，即交易结束时的采样
	OCPP_PACKAGE_METERVALUES_MS_CONTEXT_TRIGGER,//采样值上下文为"Trigger"，即触发时的采样

	OCPP_PACKAGE_METERVALUES_MS_CONTEXT_MAX
};
static const char * ocpp_package_MeterValues_MS_context_text[] = {
	"Interruption.Begin",
	"Interruption.End",
	"Other",
	"Sample.Clock",
	"Sample.Periodic",
	"Transaction.Begin",
	"Transaction.End",
	"Trigger"
};

enum OCPP_PACKAGE_METERVALUES_MS_FORMAT_E{
	OCPP_PACKAGE_METERVALUES_MS_FORMAT_RAW = 0,
	OCPP_PACKAGE_METERVALUES_MS_FORMAT_SIGNEDDATA,

	OCPP_PACKAGE_METERVALUES_MS_FORMAT_MAX

};
static const char * ocpp_package_MeterValues_MS_format_text[] = {
	"Raw",
	"SignedData"
};

enum OCPP_PACKAGE_METERVALUES_MS_MEASURAND_E{
	OCPP_PACKAGE_METERVALUES_MS_MEASURAND_CE = 0,//电流的出口值
	OCPP_PACKAGE_METERVALUES_MS_MEASURAND_CI,//电流的进口值
	OCPP_PACKAGE_METERVALUES_MS_MEASURAND_CO,//电流的提供值
	OCPP_PACKAGE_METERVALUES_MS_MEASURAND_EAER,//活动能量的出口值
	OCPP_PACKAGE_METERVALUES_MS_MEASURAND_EAIR,//活动能量的进口值
	OCPP_PACKAGE_METERVALUES_MS_MEASURAND_ERER,//无功能量的出口值
	OCPP_PACKAGE_METERVALUES_MS_MEASURAND_ERIR,//无功能量的进口值
	OCPP_PACKAGE_METERVALUES_MS_MEASURAND_EAEI,//活动能量的出口间隔
	OCPP_PACKAGE_METERVALUES_MS_MEASURAND_EAII,//活动能量的进口间隔
	OCPP_PACKAGE_METERVALUES_MS_MEASURAND_EREI,//无功能量的出口间隔
	OCPP_PACKAGE_METERVALUES_MS_MEASURAND_ERII,//无功能量的进口间
	OCPP_PACKAGE_METERVALUES_MS_MEASURAND_F,//频率，即"Frequency"
	OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PAE,//活动功率的出口值
	OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PAI,//活动功率的进口值
	OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PF,//功率因数
	OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PO,//提供功率
	OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PRE,//无功功率的出口值
	OCPP_PACKAGE_METERVALUES_MS_MEASURAND_PRI,//无功功率的进口值
	OCPP_PACKAGE_METERVALUES_MS_MEASURAND_R,//转速，即"RPM"
	OCPP_PACKAGE_METERVALUES_MS_MEASURAND_S,//充电状态，即"SoC"
	OCPP_PACKAGE_METERVALUES_MS_MEASURAND_T,//充电状态，即"SoC"
	OCPP_PACKAGE_METERVALUES_MS_MEASURAND_V,//电压，即"Voltage"

	OCPP_PACKAGE_METERVALUES_MS_MEASURAND_MAX

};
static const char * ocpp_package_MeterValues_MS_measurand_text[] = {
	"Current.Export",
	"Current.Import",
	"Current.Offered",
	"Energy.Active.Export.Register",
	"Energy.Active.Import.Register",
	"Energy.Reactive.Export.Register",
	"Energy.Reactive.Import.Register",
	"Energy.Active.Export.Interval",
	"Energy.Active.Import.Interval",
	"Energy.Reactive.Export.Interval",
	"Energy.Reactive.Import.Interval",
	"Frequency",
	"Power.Active.Export",
	"Power.Active.Import",
	"Power.Factor",
	"Power.Offered",
	"Power.Reactive.Export",
	"Power.Reactive.Import",
	"RPM",
	"SoC",
	"Temperature",
	"Voltage"
};

enum OCPP_PACKAGE_METERVALUES_MS_PHASE_E {
    OCPP_PACKAGE_METERVALUES_MS_PHASE_L1 = 0,    // L1相位
    OCPP_PACKAGE_METERVALUES_MS_PHASE_L2,        // L2相位
    OCPP_PACKAGE_METERVALUES_MS_PHASE_L3,        // L3相位
    OCPP_PACKAGE_METERVALUES_MS_PHASE_N,         // N相位
    OCPP_PACKAGE_METERVALUES_MS_PHASE_L1_N,      // L1-N相位
    OCPP_PACKAGE_METERVALUES_MS_PHASE_L2_N,      // L2-N相位
    OCPP_PACKAGE_METERVALUES_MS_PHASE_L3_N,      // L3-N相位
    OCPP_PACKAGE_METERVALUES_MS_PHASE_L1_L2,     // L1-L2相位
    OCPP_PACKAGE_METERVALUES_MS_PHASE_L2_L3,     // L2-L3相位
    OCPP_PACKAGE_METERVALUES_MS_PHASE_L3_L1,     // L3-L1相位
    OCPP_PACKAGE_METERVALUES_MS_PHASE_MAX        // 相位类型的数量，方便循环遍历或判断范围
};
static const char * ocpp_package_MeterValues_MS_phase_text[] = {
	"L1",
	"L2",
	"L3",
	"N",
	"L1-N",
	"L2-N",
	"L3-N",
	"L1-L2",
	"L2-L3",
	"L3-L1"

};

enum OCPP_PACKAGE_METERVALUES_MS_LOCATION_E {
    OCPP_PACKAGE_METERVALUES_MS_LOCATION_BODY = 0,           // 车身（整车测量）
    OCPP_PACKAGE_METERVALUES_MS_LOCATION_CABLE,              // 充电桩连接线（电缆）
    OCPP_PACKAGE_METERVALUES_MS_LOCATION_CHARGE_POINT,       // 充电桩本体（充电设备）
    OCPP_PACKAGE_METERVALUES_MS_LOCATION_EV,                 // 电动汽车（充电设备对应的车辆）
    OCPP_PACKAGE_METERVALUES_MS_LOCATION_INLET,              // 充电桩输入端（进线口）
    OCPP_PACKAGE_METERVALUES_MS_LOCATION_OUTLET,             // 充电桩输出端（出线口）

    OCPP_PACKAGE_METERVALUES_MS_LOCATION_MAX                 // 测量值位置的数量，方便循环遍历或判断范围
};

static const char * ocpp_package_MeterValues_MS_location_text[] = {
	"Body",
	"Cable",
	"Charge Point",
	"EV",
	"Inlet",
	"Outlet"
};

enum OCPP_PACKAGE_METERVALUES_MS_UNIT_E{
	OCPP_PACKAGE_METERVALUES_MS_UNIT_WH = 0,
	OCPP_PACKAGE_METERVALUES_MS_UNIT_KWH,
	OCPP_PACKAGE_METERVALUES_MS_UNIT_VARH,
	OCPP_PACKAGE_METERVALUES_MS_UNIT_KVARH,
	OCPP_PACKAGE_METERVALUES_MS_UNIT_W,
	OCPP_PACKAGE_METERVALUES_MS_UNIT_KW,
	OCPP_PACKAGE_METERVALUES_MS_UNIT_VA,
	OCPP_PACKAGE_METERVALUES_MS_UNIT_KVA,
	OCPP_PACKAGE_METERVALUES_MS_UNIT_VAR,
	OCPP_PACKAGE_METERVALUES_MS_UNIT_KVAR,
	OCPP_PACKAGE_METERVALUES_MS_UNIT_A,
	OCPP_PACKAGE_METERVALUES_MS_UNIT_V,
	OCPP_PACKAGE_METERVALUES_MS_UNIT_CELSIUS,
	OCPP_PACKAGE_METERVALUES_MS_UNIT_FAHRENHEIT,
	OCPP_PACKAGE_METERVALUES_MS_UNIT_K,
	OCPP_PACKAGE_METERVALUES_MS_UNIT_PERCENT,

	OCPP_PACKAGE_METERVALUES_MS_UNIT_MAX

};
static const char * ocpp_package_MeterValues_MS_unit_text[] = {
	"Wh",
	"kWh",
	"varh",
	"kvarh",
	"W",
	"kW",
	"VA",
	"kVA",
	"var",
	"kvar",
	"A",
	"V",
	"Celsius",
	"Fahrenheit",
	"K",
	"Percent",
};


typedef struct {
    int contextIsUse; // 是否使用context字段
    int formatIsUse; // 是否使用format字段
    int locationIsUse; // 是否使用location字段
    int measurandIsUse; // 是否使用measurand字段
    int phaseIsUse; // 是否使用phase字段
    int unitIsUse; // 是否使用unit字段
    
    char format[32]; // 采样值的格式，例如"Raw"
    char context[32]; // 采样值的格式，例如"Raw"
    char location[32]; // 采样值的位置，例如"Outlet"
    char measurand[32]; // 采样值的测量指标，例如"Voltage"
    char phase[32]; // 采样值的相位信息，例如"Phase.A"
    char unit[32]; // 采样值的单位，例如"V"
    char value[16]; // 采样值的实际值，例如"224.70"
} ocpp_package_MeterValues_M_SampledValue;

typedef struct {
	char transactionIdIsUse;
    int connectorId; // 充电接口的ID
    int transactionId; // 事务ID
    char timestamp[32]; // 时间戳，格式为"2023-08-09T11:47:41.000Z"
	int  sampledValueCnt;
    ocpp_package_MeterValues_M_SampledValue *sampledValue; // 采样值数组
} ocpp_package_MeterValues_req;

typedef struct{
	char timestamp[32];
	int sampledValueCnt;
	ocpp_package_MeterValues_M_SampledValue *sampledValue;
}ocpp_package_MeterValues_MeterValue_t;


typedef struct{
	int connectorId;
	char transactionIdIsUse;
	int transactionId;
	ocpp_package_MeterValues_MeterValue_t meterValue;
}ocpp_package_MeterValues_req_t;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>MeterValues.conf

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>RemoteStartTransaction.req
enum OCPP_PACKAGE_CHARGING_PROFILE_KIND_TYPE_E{
	OCPP_PACKAGE_CHARGING_PROFILE_KIND_TYPE_ABSOLUTE = 0,
	OCPP_PACKAGE_CHARGING_PROFILE_KIND_TYPE_RECURRING,
	OCPP_PACKAGE_CHARGING_PROFILE_KIND_TYPE_RELATIVE,

	OCPP_PACKAGE_CHARGING_PROFILE_KIND_TYPE_MAX

};

static const char * ocpp_package_ChargingProfileKindType_text[] = {

	"Absolute",
	"Recurring",
	"Relative"
};


enum OCPP_PACKAGE_RECURRENCY_KIND_TYPE_E{
	OCPP_PACKAGE_RECURRENCY_KIND_TYPE_DAILY = 0,
	OCPP_PACKAGE_RECURRENCY_KIND_TYPE_WEEKLY,
	OCPP_PACKAGE_RECURRENCY_KIND_TYPE_MAX

};

static const char * ocpp_package_RecurrencyKindType_text[] = {

	"Daily",
	"Weekly"
};




typedef struct{
	int vaildFrom;//[非必须]配置生效的开始时间, 如果为空，表示立即生效
	int vaildTo;//[非必须]配置失效时间,如果为空，表示永久生效。被其他配置替换可失效
	int chargingProfileId;// 唯一ID，可以理解为主键
	int transactionId;//[非必须]只有当ChargingProfilePurpose = TxProfile时才有效，针对某一次充电
    int stackLevel;//优先级别，值越大优先级越高 >=0
    char chargingProfilePurpose[20];  // 枚举类型，ChargePointMaxProfile:配置充电桩的最大功率/最大电流   TxDefaultProfile: 默认的配置  TxProfile:仅用于RemoteStartTransaction
    char chargingProfileKind[20];  // 枚举类型， Absolute： chargingSchedulePeriod的定义为固定的时间点   Recurring：  Relative
	char recurrencyKind[20];  // [非必须]枚举类型 Daily， Weekly 当chargingProfileKind为Recurring时必须
	ocpp_package_chargingSchedule_t  chargingSchedule;

}ocpp_package_chargingProfile_t;


typedef struct{
	char connectorIdIsUse:1;
	int connectorId;

	char idTag[OCPP_AUTHORIZATION_IDTAG_LEN];

	char chargingProfileIsUse:1;
	//待实现
	ocpp_package_chargingProfile_t chargingProfile;

}ocpp_package_RemoteStartTransaction_req_t;
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>RemoteStartTransaction.conf
enum OCPP_PACKAGE_REMOTE_STRATSTOP_STATUS_E{
	OCPP_PACKAGE_REMOTE_STRATSTOP_STATUS_ACCEPTED = 0,
	OCPP_PACKAGE_REMOTE_STRATSTOP_STATUS_REJECTED,

	OCPP_PACKAGE_REMOTE_STRATSTOP_STATUS_MAX
};

static const char * ocpp_package_RemoteStartStopStatus_text[] = {
	"Accepted",
	"Rejected"
};

typedef struct{
	enum OCPP_PACKAGE_REMOTE_STRATSTOP_STATUS_E status;

}ocpp_package_RemoteStartTransaction_conf_t;


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>RemoteStopTransaction.req
typedef struct{
	int transactionId;

}ocpp_package_RemoteStopTransaction_req_t;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>RemoteStopTransaction.conf
typedef struct{

	enum OCPP_PACKAGE_REMOTE_STRATSTOP_STATUS_E status;

}ocpp_package_RemoteStopTransaction_conf_t;


/////////////////////////////////////////////////////////////////////////////////

typedef struct{
	int chargingProfileId;

	char transactionIdIsUse:1;
	int transactionId;

	int stackLevel;
	enum OCPP_PACKAGE_CHARGING_PROFILE_PURPOSE_TYPE_E	chargingProfilePurpose;
	enum OCPP_PACKAGE_CHARGING_PROFILE_KIND_TYPE_E  chargingProfileKind;

	char recurrencyKindIsUse:1;
	enum OCPP_PACKAGE_RECURRENCY_KIND_TYPE_E  recurrencyKind;

	char validFromIsUse:1;
	char validFrom[32];

	
}ocpp_package_ChargingProfile_t;




//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>ReserveNow.req
typedef struct{
	int connectorId;
	char expiryDate[32];
	char idTag[20];

	char parentIdTagIsUse:1;
	char parentIdTag[20];

	int reservationId;

}ocpp_package_ReserveNow_req_t;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>ReserveNow.conf
enum OCPP_PCAKGE_RESERVATION_STATUS_E {
    OCPP_PCAKGE_RESERVATION_STATUS_ACCEPTED = 0,      // 预约已被接受
    OCPP_PCAKGE_RESERVATION_STATUS_FAULTED,          // 预约状态故障
    OCPP_PCAKGE_RESERVATION_STATUS_OCCUPIED,         // 充电桩被占用，正在充电
    OCPP_PCAKGE_RESERVATION_STATUS_REJECTED,         // 预约被拒绝
    OCPP_PCAKGE_RESERVATION_STATUS_UNAVAILABLE,      // 充电桩不可用，无法预约

    OCPP_PCAKGE_RESERVATION_STATUS_MAX               // 预约状态的数量，方便循环遍历或判断范围
};

static const char * ocpp_package_ReservationStatus_text[] = {
	"Accepted",
	"Faulted",
	"Occupied",
	"Rejected",
	"Unavailable"
};

typedef struct{
	enum OCPP_PCAKGE_RESERVATION_STATUS_E status;

}ocpp_package_ReserveNow_conf_t;


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>Reset.req
enum OCPP_PACKAGE_RESET_TYPE_E{
	OCPP_PACKAGE_RESET_TYPE_HARD = 0,
	OCPP_PACKAGE_RESET_TYPE_SOFT,

	OCPP_PACKAGE_RESET_TYPE_MAX

};

static const char * ocpp_package_ResetType_text[] = {
	"Hard",
	"Soft"
};

typedef struct{
	enum OCPP_PACKAGE_RESET_TYPE_E type;

}ocpp_package_Reset_req_t;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>Reset.conf
enum OCPP_PACKAGE_RESET_STATUS_E{
	OCPP_PACKAGE_RESET_STATUS_ACCEPTED = 0,
	OCPP_PACKAGE_RESET_STATUS_REJECTED,

	OCPP_PACKAGE_RESET_STATUS_MAX

};
static const char *ocpp_package_ResetStatus_text[] = {
	"Accepted",
	"Rejected"
};
typedef struct{

	enum OCPP_PACKAGE_RESET_STATUS_E status;
}ocpp_package_Reset_conf_t;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>SendLocalList.req

enum OCPP_PACKAGE_UPDATE_TYPE_E{
	OCPP_PACKAGE_UPDATE_TYPE_DIFFERENTIAL = 0,
	OCPP_PACKAGE_UPDATE_TYPE_FULL,
	OCPP_PACKAGE_UPDATE_TYPE_MAX


};

static const char * ocpp_package_UpdateType_text[] = {
	"Differential",
	"Full"
};


typedef struct{
	char idTag[20];
	char idTagInfoIsUse:1;
	ocpp_package_idTagInfo_t idTagInfo;

}ocpp_package_AuthorizationData_t;



typedef struct{
	int listVersion;
	char localAuthorizationListIsUse:1;
	int  numberOfAuthorizationData;
	ocpp_package_AuthorizationData_t *localAuthorizationList;

	enum OCPP_PACKAGE_UPDATE_TYPE_E UpdateType;
}ocpp_package_SendLocalList_req_t;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>SendLocalList.conf
enum OCPP_PCAKGE_UPDATE_STATUS_E{
	OCPP_PCAKGE_UPDATE_STATUS_ACCEPTED = 0,
	OCPP_PCAKGE_UPDATE_STATUS_FAILED,
	OCPP_PCAKGE_UPDATE_STATUS_NOTSUPPORTED,
	OCPP_PCAKGE_UPDATE_STATUS_VERSION_MISMATCH,

	OCPP_PACKGE_UPDATE_STATUS_MAX
};

static const char * ocpp_package_UpdateStatus_text[] = {
	"Accepted",
	"Failed",
	"NotSupported",
	"VersionMismatch"
};

typedef struct{
	enum OCPP_PCAKGE_UPDATE_STATUS_E status;

}ocpp_package_SendLocalList_conf_t;


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>SetChargingProfile.req
typedef struct{
	int connectorId;

	ocpp_package_chargingProfile_t  csChargingProfiles;

}ocpp_package_SetChargingProfile_req_t;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>SetChargingProfile.conf
enum OCPP_PACKAGE_CHARGEING_PROFILE_STATUS_E{
	OCPP_PACKAGE_CHARGEING_PROFILE_STATUS_ACCEPTED = 0,
	OCPP_PACKAGE_CHARGEING_PROFILE_STATUS_REJECTED,
	OCPP_PACKAGE_CHARGEING_PROFILE_STATUS_NOTSUPPORTED,
	OCPP_PACKAGE_CHARGEING_PROFILE_STATUS_MAX


};
static const char * ocpp_package_ChargingProfileStatus_text[] = {

	"Accepted",
	"Rejected",
	"NotSupported"
};


typedef struct{
enum OCPP_PACKAGE_CHARGEING_PROFILE_STATUS_E status;

}ocpp_package_SetChargingProfile_conf_t;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>StartTransaction.req
typedef struct{
	int connectorId;
	char idTag[OCPP_AUTHORIZATION_IDTAG_LEN];
	int meterStart;

	char reservationIdIsUse:1;
	int reservationId;
	
	char timestamp[32];

}ocpp_package_StartTransaction_req_t;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>StartTransaction.conf




static const char *ocpp_package_ChargePointErrorCode_text[] = {
	"ConnectorLockFailure",
	"EVCommunicationError",
	"GroundFailure",
	"HighTemperature",
	"InternalError",
	"LocalListConflict",
	"NoError",
	"OtherError",
	"OverCurrentFailure",
	"OverVoltage",
	"PowerMeterFailure",
	"PowerSwitchFailure",
	"ReaderFailure",
	"ResetFailure",
	"UnderVoltage",
	"WeakSignal"
};

static const char * ocpp_package_ChargePointStatus_text[] = {
	"Available",
	"Preparing",
	"Charging",
	"SuspendedEVSE",
	"SuspendedEV",
	"Finishing",
	"Reserved",
	"Unavailable",
	"Faulted"
};


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>StatusNotification.conf
typedef struct{



}ocpp_package_StatusNotification_conf_t;



static const char * ocpp_package_stop_reason_text[] = {
	"EmergencyStop",
	"EVDisconnected",
	"HardReset",
	"Local",
	"Other",
	"PowerLoss",
	"Reboot",
	"Remote",
	"SoftReset",
	"UnlockCommand",
	"DeAuthorized"
};

typedef struct{

	char idTagIsUse:1;
	char idTag[OCPP_AUTHORIZATION_IDTAG_LEN];
	int meterStop;
	char timestamp[32];
	int transactionId;

	char reasonIsUse:1;
	enum OCPP_PACKAGE_STOP_REASON_E reason;

	char transactionDataIsUse:1;
	ocpp_package_MeterValues_MeterValue_t transactionData;

}ocpp_package_StopTransaction_req_t;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>TriggerMessage.req
enum OCPP_PCAKGE_MESSAGE_TRIGGER_E {
    OCPP_PCAKGE_MESSAGE_TRIGGER_BOOTNOTIFICATION = 0,                // 启动通知
    OCPP_PCAKGE_MESSAGE_TRIGGER_DIAGNOSTICSSTATUS_NOTIFICATION,     // 诊断状态通知
    OCPP_PCAKGE_MESSAGE_TRIGGER_FIRMWARESTATUS_NOTIFICATION,        // 固件状态通知
    OCPP_PCAKGE_MESSAGE_TRIGGER_HEARTBEAT,                          // 心跳
    OCPP_PCAKGE_MESSAGE_TRIGGER_METERVALUES,                        // 电表值
    OCPP_PCAKGE_MESSAGE_TRIGGER_STATUS_NOTIFICATION,               // 状态通知

    OCPP_PACKGE_MESSAGE_TRIGGER_STATUS_MAX                          // 触发器类型的数量，方便循环遍历或判断范围
};

static const char * ocpp_package_MessageTrigger_text[] = {
	"BootNotification",
	"DiagnosticsStatusNotification",
	"FirmwareStatusNotification",
	"Heartbeat",
	"MeterValues",
	"StatusNotification"
};

typedef struct{
	enum OCPP_PCAKGE_MESSAGE_TRIGGER_E requestedMessage;

	char connectorIdIsUse:1;
	int connectorId;

}ocpp_package_TriggerMessage_req_t;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>TriggerMessage.conf
enum OCPP_PACKAGE_TRIGGER_MESSAGE_STATUS_E{
	OCPP_PACKAGE_TRIGGER_MESSAGE_STATUS_ACCEPTED = 0,
	OCPP_PACKAGE_TRIGGER_MESSAGE_STATUS_REJECTED,
	OCPP_PACKAGE_TRIGGER_MESSAGE_STATUS_NOTIMPLEMENTED,

	OCPP_PACKAGE_TRIGGER_MESSAGE_STATUS_MAX

};
static const char * ocpp_package_TriggerMessageStatus_text[] = {
	"Accepted",
	"Rejected",
	"NotImplemented"
};
typedef struct{

	enum OCPP_PACKAGE_TRIGGER_MESSAGE_STATUS_E status;
}ocpp_package_TriggerMessage_conf_t;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>UnlockConnector.req
typedef struct{
	int connectorId;

}ocpp_package_UnlockConnector_req_t;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>UnlockConnector.conf
enum OCPP_PACKAGE_UNLOCK_STATUS_E{
	OCPP_PACKAGE_UNLOCK_STATUS_UNLOCKED = 0,
	OCPP_PACKAGE_UNLOCK_STATUS_UNLOCK_FAILED,
	OCPP_PACKAGE_UNLOCK_STATUS_NOTSUPPORTED,

	OCPP_PACKAGE_UNLOCK_STATUS_MAX

};

static const char * ocpp_package_UnlockStatus_text[] = {
	"Unlocked",
	"UnlockFailed",
	"NotSupported"
};

typedef struct{
	enum OCPP_PACKAGE_UNLOCK_STATUS_E status;

}ocpp_package_UnlockConnector_conf_t;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>UpdateFirmware.req
typedef struct{
	char location[256];//url
	
	char retriesIsUse:1;
	int retries;//重试次数

	char retrieveDate[32];//重试时间

	char retryIntervalIsUse:1;
	int retryInterval;//重试时间间隔

}ocpp_package_UpdateFirmware_req_t;











//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>CallError
// NotImplemented                      Requested Action is not known by receiver
// NotSupported                        Requested Action is recognized but not supported by the receiver
// InternalError                       An internal error occurred and the receiver was not able to process the requested Action successfully
// ProtocolError                       Payload for Action is incomplete
// SecurityError                       During the processing of Action a security issue occurred preventing receiver from completing the Action successfully
// FormationViolation                  Payload for Action is syntactically incorrect or not conform the PDU structure for Action
// PropertyConstraintViolation         Payload is syntactically correct but at least one field contains an invalid value
// OccurenceConstraintViolation        Payload for Action is syntactically correct but at least one of the fields violates occurence constraints
// TypeConstraintViolation             Payload for Action is syntactically correct but at least one of the fields violates data type constraints (e.g. “somestring”: 12)
// GenericError                        Any other error not covered by the previous ones


enum OCPP_PACKAGE_CALLERROR_ERRORCODE_E {
    OCPP_PACKAGE_CALLERROR_ERRORCODE_NOT_IMPLEMENTED = 0,              // 未实现
    OCPP_PACKAGE_CALLERROR_ERRORCODE_NOT_SUPPORTED,                    // 不支持
    OCPP_PACKAGE_CALLERROR_ERRORCODE_INTERNAL_ERROR,                   // 内部错误
    OCPP_PACKAGE_CALLERROR_ERRORCODE_PROTOCOL_ERROR,                   // 协议错误
    OCPP_PACKAGE_CALLERROR_ERRORCODE_SECURITY_ERROR,                   // 安全错误
    OCPP_PACKAGE_CALLERROR_ERRORCODE_FORMATION_VIOLATION,              // 格式违规
    OCPP_PACKAGE_CALLERROR_ERRORCODE_PROPERTY_CONSTRAINT_VIOLATION,    // 属性约束违规
    OCPP_PACKAGE_CALLERROR_ERRORCODE_OCCURENCE_CONSTRAINT_VIOLATION,   // 出现约束违规
    OCPP_PACKAGE_CALLERROR_ERRORCODE_TYPE_CONSTRAINT_VIOLATION,        // 类型约束违规
    OCPP_PACKAGE_CALLERROR_ERRORCODE_GENERIC_ERROR,                    // 通用错误

    OCPP_PACKAGE_CALLERROR_ERRORCODE_MAX                              // 错误码类型的数量，方便循环遍历或判断范围
};

static const char * ocpp_package_CallError_ErrorCode_text[] = {
	"NotImplemented",
	"NotSupported",
	"InternalError",
	"ProtocolError",
	"SecurityError",
	"FormationViolation",
	"PropertyConstraintViolation",
	"OccurenceConstraintViolation",
	"TypeConstraintViolation",
	"GenericError"
};


typedef struct {
    enum OCPP_PACKAGE_CALLERROR_ERRORCODE_E errorCode;
    char errorDescriptionIsUse:1;
    char errorDescription[1024];
    int errorDetailCnt; // 错误描述的数量
    char detail[256];   // 描述
} ocpp_package_CallError_t;


void ocpp_chargePoint_Authorization_IdTag(int connector, char * idTag);//发送卡号进行认证，平台，本地列表，本地缓存，可关闭
void ocpp_package_prepare_Status_conf(const char * UniqueId, int status);
void ocpp_chargePoint_sendMeterValues_req(int connector, int transactionId);
//发送StartTransaction，StopTransaction
void ocpp_chargePoint_sendStartTransaction_req(int connector, char *idTag, int reservationId, const char * UniqueId,char *timestamp,int metervalue);
void ocpp_transaction_sendStopTransaction_req(int connector, char *idTag, int transactionId, const  char * UniqueId,int meterStop, char *timestamp,enum OCPP_PACKAGE_STOP_REASON_E reason);
//发送Heartbeat，BootNotification，StatusNotification
void ocpp_chargePoint_sendHeartbeat_req();
void ocpp_chargePoint_sendBootNotification_req();
void ocpp_chargePoint_sendStatusNotification_req(int connector);
//身份认证
void ocpp_chargePoint_sendAuthorize_req(char *idTag, const char *lastUniqueId);
void ocpp_chargePoint_sendDiagnosticsStatusNotification_req(int status);
void ocpp_chargePoint_sendGetDiagnostics_conf(const char * UniqueId, char *fileName, int status);
//远程升级
void ocpp_chargePoint_sendFirmwareStatusNotification_req(int status);
//获取配置
void ocpp_chargePoint_sendGetConfiguration_conf(const char * UniqueId, ocpp_package_GetConfiguration_conf_t *getConfiguration_conf);
void ocpp_chargePoint_sendChangeConfiguration_conf(const char * UniqueId, ocpp_package_ChangeConfiguration_req_t changeConfiguration_req);
//远程启动
void ocpp_chargePoint_sendRemoteStartTransaction_conf(const char * UniqueId, ocpp_package_RemoteStartTransaction_req_t remoteStartTransaction_req);
void ocpp_chargePoint_sendRemoteStopTransaction_conf(const char * UniqueId, ocpp_package_RemoteStopTransaction_req_t remoteStopTransaction_req);
//获取本地列表版本号，保存本地列表名单
void ocpp_chargePoint_sendGetLocalListVersion_conf(const char * UniqueId);
void ocpp_chargePoint_sendSendLocalList_conf(const char * UniqueId, ocpp_package_SendLocalList_req_t *sendLocalList_req);
//获取状态
void ocpp_chargePoint_sendTriggerMessage_conf(const char * UniqueId, ocpp_package_TriggerMessage_req_t triggerMessage_req);
//预约和取消预约
void ocpp_chargePoint_sendReserveNow_conf(const char * UniqueId, ocpp_package_ReserveNow_req_t reserveNow_req);
void ocpp_chargePoint_sendCancelReservation_conf(const char * UniqueId, ocpp_package_CancelReservation_req_t cancelReservation_req);
//清除缓存
void ocpp_chargePoint_sendClearCacheRequest_conf(const char * UniqueId, ocpp_package_ClearCache_req_t clearCache_req);
//更改可用性
void ocpp_chargePoint_sendChangeAvailability_conf(const char * UniqueId, ocpp_package_ChangeAvailability_req_t changeAvailability_req);
//
void ocpp_chargePoint_sendUnlockConnector_conf(const char * UniqueId, ocpp_package_UnlockConnector_req_t unlockConnector_req);
//
void ocpp_chargePoint_sendResetRequest_conf(const char * UniqueId, ocpp_package_Reset_req_t reset_req);
//
void ocpp_chargePoint_sendGetCompositeSchedule_conf(const char * UniqueId, ChargingProfile chargingProfile);

char * ocpp_package_prepare_CallError(const char * UniqueId, ocpp_package_CallError_t * callError);









#ifdef __cplusplus
}
#endif

#endif




















