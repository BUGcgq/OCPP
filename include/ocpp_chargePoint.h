#ifndef __OCPP_CHARGE_POINT__H__
#define __OCPP_CHARGE_POINT__H__

#ifdef __cplusplus
extern "C" {
#endif

#include <netdb.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <json-c/json.h>
#include <time.h>
#include <unistd.h>
#include <sqlite3.h>

#define OCPP_AUTHORIZATION_IDTAG_LEN                               21                            //卡号长度
#define OCPP_LOCAL_AUTHORIZATION_CACHE_MAX                         10                            //本地认证缓存数
//OCPP连接对象
typedef struct{
    bool isWss;                      //
    char * ssl_ca_filepath;          //如果为wss连接,需指明CA证书文件路径
    char * ssl_private_key_filepath; //私钥路径
    char * protocolName;             //websocket子协议名称 eg:ocpp1.6
    unsigned short port;             //协议端口
    char * address;                  //连接地址
    char * path;                     //服务器路径
    char * username;                 //用户名
    char * password;                 //密码
    bool  isConnect;                 //是否与服务器建立连接
    void (*send)(const char * const message, size_t len);
    void (*receive)(void * message, int len);                //需上层实现对接收的处理
}ocpp_connect_t;

enum OCPP_PACKAGE_REGISTRATION_STATUS_E{
	OCPP_PACKAGE_REGISTRATION_STATUS_ACCEPTED = 0,//操作被接受
	OCPP_PACKAGE_REGISTRATION_STATUS_PENDING,//服务器忙
	OCPP_PACKAGE_REGISTRATION_STATUS_REJECTED,//操作被拒绝

	OCPP_PACKAGE_REGISTRATION_STATUS_MAX
};


enum OCPP_PACKAGE_DIAGNOSTICS_STATUS_E{
	OCPP_PACKAGE_DIAGNOSTICS_STATUS_IDLE = 0,//诊断请求处于空闲状态
	OCPP_PACKAGE_DIAGNOSTICS_STATUS_UPLOADED,//诊断请求的结果已上传。
	OCPP_PACKAGE_DIAGNOSTICS_STATUS_UPLOAD_FAILED,//诊断请求的结果上传失败
	OCPP_PACKAGE_DIAGNOSTICS_STATUS_UPLOADING,//诊断请求的结果正在上传中

	OCPP_PACKAGE_DIAGNOSTICS_STATUS_MAX

};

enum OCPP_CHARGEPOINT_AUTHORIZE_RESULT_E {
    OCPP_CHARGEPOINT_AUTHORIZE_RESULT_FAIL = 0,    // 授权失败
    OCPP_CHARGEPOINT_AUTHORIZE_RESULT_ONGOING,      // 授权正在进行中
    OCPP_CHARGEPOINT_AUTHORIZE_RESULT_SUCCEED,      // 授权成功
    OCPP_CHARGEPOINT_AUTHORIZE_RESULT_UNKOWN       // 授权结果未知

};


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>FirmwareStatusNotification.req
enum OCPP_PACKAGE_FIRMWARE_STATUS_E{
	OCPP_PACKAGE_FIRMWARE_STATUS_DOWNLOADED = 0,//固件已下载
	OCPP_PACKAGE_FIRMWARE_STATUS_DOWNLOAD_FAILED,//固件下载失败
	OCPP_PACKAGE_FIRMWARE_STATUS_DOWNLOADING,//固件正在下载中
	OCPP_PACKAGE_FIRMWARE_STATUS_IDLE,//固件处于空闲状态
	OCPP_PACKAGE_FIRMWARE_STATUS_INSTALLATION_FAILED,//固件安装失败
	OCPP_PACKAGE_FIRMWARE_STATUS_INSTALLING,//固件正在安装中
	OCPP_PACKAGE_FIRMWARE_STATUS_INSTALLED,//固件已安装

	OCPP_PACKAGE_FIRMWARE_STATUS_MAX

};

enum OCPP_PACKAGE_CHARGEPOINT_ERRORCODE_E {
    OCPP_PACKAGE_CHARGEPOINT_ERRORCODE_CONNECTOR_LOCK_FAILURE = 0,   // 连接器锁定失败
    OCPP_PACKAGE_CHARGEPOINT_ERRORCODE_EV_COMMUNICATION_ERROR,       // 电动汽车通信错误
    OCPP_PACKAGE_CHARGEPOINT_ERRORCODE_GROUND_FAILURE,               // 地面故障
    OCPP_PACKAGE_CHARGEPOINT_ERRORCODE_HIGH_TEMPERATURE,             // 高温故障
    OCPP_PACKAGE_CHARGEPOINT_ERRORCODE_INTERNAL_ERROR,              // 内部错误
    OCPP_PACKAGE_CHARGEPOINT_ERRORCODE_LOCALLIST_CONFLICT,           // 本地列表冲突
    OCPP_PACKAGE_CHARGEPOINT_ERRORCODE_NOERROR,                      // 无错误
    OCPP_PACKAGE_CHARGEPOINT_ERRORCODE_OTHER_ERROR,                 // 其他错误
    OCPP_PACKAGE_CHARGEPOINT_ERRORCODE_OVERCURRENT_FAILURE,         // 过电流故障
    OCPP_PACKAGE_CHARGEPOINT_ERRORCODE_OVERVOLTAGE,                 // 过电压
    OCPP_PACKAGE_CHARGEPOINT_ERRORCODE_POWERMETER_FAILURE,          // 电能表故障
    OCPP_PACKAGE_CHARGEPOINT_ERRORCODE_POWERSWITCH_FAILURE,         // 电源开关故障
    OCPP_PACKAGE_CHARGEPOINT_ERRORCODE_READER_FAILURE,              // 读卡器故障
    OCPP_PACKAGE_CHARGEPOINT_ERRORCODE_RESET_FAILURE,               // 复位故障
    OCPP_PACKAGE_CHARGEPOINT_ERRORCODE_UNDERVOLTAGE,                // 欠电压
    OCPP_PACKAGE_CHARGEPOINT_ERRORCODE_WEAKSIGNAL,                  // 信号弱

    OCPP_PACKAGE_CHARGEPOINT_ERRORCODE_MAX                           // 充电桩错误代码的数量，用于迭代或范围检查
};

enum OCPP_PACKAGE_CHARGEPOINT_STATUS_E {
    OCPP_PACKAGE_CHARGEPOINT_STATUS_AVAILABLE = 0,            // 可用
    OCPP_PACKAGE_CHARGEPOINT_STATUS_PREPARING,                // 准备中
    OCPP_PACKAGE_CHARGEPOINT_STATUS_CHARGING,                 // 充电中
    OCPP_PACKAGE_CHARGEPOINT_STATUS_SUSPENDED_EVSE,           // 充电枪挂起
    OCPP_PACKAGE_CHARGEPOINT_STATUS_SUSPENDED_EV,             // 充电事务挂起
    OCPP_PACKAGE_CHARGEPOINT_STATUS_FINISHING,                // 完成中
    OCPP_PACKAGE_CHARGEPOINT_STATUS_RESERVED,                 // 已预约
    OCPP_PACKAGE_CHARGEPOINT_STATUS_UNAVAILABLE,              // 不可用
    OCPP_PACKAGE_CHARGEPOINT_STATUS_FAULTED,                  // 故障

    OCPP_PACKAGE_CHARGEPOINT_STATUS_MAX                       // 充电桩状态的数量，方便循环遍历或判断范围
};
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>StopTransaction.req
enum OCPP_PACKAGE_STOP_REASON_E {
    OCPP_PACKAGE_STOP_REASON_EMERGENCYSTOP = 0,    // 紧急停车
    OCPP_PACKAGE_STOP_REASON_EVDISCONNECTED,       // 电动车断开连接
    OCPP_PACKAGE_STOP_REASON_HARDRESET,            // 硬重置
    OCPP_PACKAGE_STOP_REASON_LOCAL,                // 本地停止
    OCPP_PACKAGE_STOP_REASON_OTHER,                // 其他原因
    OCPP_PACKAGE_STOP_REASON_POWERLOSS,            // 断电
    OCPP_PACKAGE_STOP_REASON_REBOOT,               // 重启
    OCPP_PACKAGE_STOP_REASON_REMOTE,               // 远程停止
    OCPP_PACKAGE_STOP_REASON_SOFTRESET,            // 软重置
    OCPP_PACKAGE_STOP_REASON_UNLOCKCOMMAND,        // 解锁命令
    OCPP_PACKAGE_STOP_REASON_DEAUTHORIZED,         // 未授权停车

    OCPP_PACKAGE_STOP_REASON_MAX                   // 停止充电原因的数量，方便循环遍历或判断范围
};
typedef struct{

    int reservationId;
    char expiryDate[32];
    char idTag[OCPP_AUTHORIZATION_IDTAG_LEN];
    char parentIdTag[OCPP_AUTHORIZATION_IDTAG_LEN];
}ocpp_reserve_t;

//认证标识符
typedef struct{
    bool isWaitAuthoriza;
	char idTag[OCPP_AUTHORIZATION_IDTAG_LEN];
    enum OCPP_CHARGEPOINT_AUTHORIZE_RESULT_E result;
    char lastUniqueId[40];
}ocpp_chargePoint_Authoriza_t;

enum OCPP_LOCAL_AUTHORIZATION_STATUS_E {
    OCPP_LOCAL_AUTHORIZATION_ACCEPTED = 0,            // 授权已接受，允许充电
    OCPP_LOCAL_AUTHORIZATION_BLOCKED,                 // 授权已被阻止，不允许充电
    OCPP_LOCAL_AUTHORIZATION_EXPIRED,                 // 授权已过期，不允许充电
    OCPP_LOCAL_AUTHORIZATION_INVALID,                 // 授权无效，不允许充电
    OCPP_LOCAL_AUTHORIZATION_CONCURRENTTX,            // 授权已参与另一个交易，不允许多个交易
                                                      // （仅适用于 StartTransaction.req）

    OCPP_LOCAL_AUTHORIZATION_MAX
};

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>Authorize.conf
typedef struct{
	char expiryDateIsUse:1;
	char expiryDate[32];
	char parentIdTagIsUse:1;
	char parentIdTag[OCPP_AUTHORIZATION_IDTAG_LEN];
	enum OCPP_LOCAL_AUTHORIZATION_STATUS_E AuthorizationStatus;
}ocpp_package_idTagInfo_t;

typedef struct{
	ocpp_package_idTagInfo_t idTagInfo;

}ocpp_package_Authorize_conf_t;

typedef struct{
	ocpp_package_idTagInfo_t idTagInfo;
	int transactionId;

}ocpp_package_StartTransaction_conf_t;
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>StopTransaction.conf
typedef struct{
	char idTagInfoIsUse:1;
	ocpp_package_idTagInfo_t idTagInfo;

}ocpp_package_StopTransaction_conf_t;

typedef struct{
	char IdTag[OCPP_AUTHORIZATION_IDTAG_LEN];                                        //卡号
	enum OCPP_LOCAL_AUTHORIZATION_STATUS_E status;                                   //卡状态
	char expiryDate[32];                                                           //有效期
	char parentIdTag[OCPP_AUTHORIZATION_IDTAG_LEN];                                  //父卡号

}ocpp_localAuthorization_cache_record_t;
//交易对象
typedef struct{
    bool isTransaction;                             //Is deal
    char lastUniqueId[40];                          //last send package uniqueld

    int transactionId;                             

    char reservationIdIsUse:1;
    int reservationId;

    char startIdTag[OCPP_AUTHORIZATION_IDTAG_LEN];
    bool isStart;

    char stopIdTagIsUse:1;
    char stopIdTag[OCPP_AUTHORIZATION_IDTAG_LEN];
    enum OCPP_PACKAGE_STOP_REASON_E  reason;
    bool isStop;

	int startupType; //2 刷卡启动，1远程启动
    enum OCPP_CHARGEPOINT_AUTHORIZE_RESULT_E authorizeResult;

    bool isRecStartTransaction_Conf;
    ocpp_package_StartTransaction_conf_t startTransaction;

    bool isRecStopTransaction_Conf;
    ocpp_package_StopTransaction_conf_t  stopTransaction;

    bool isRetransmission;                                //服务器应答错误,需要重新发送数据包

}ocpp_chargePoint_transaction_t;

typedef struct{
	int connectorId;
	enum OCPP_PACKAGE_CHARGEPOINT_ERRORCODE_E errorCode;

	char infoIsUse:1;
	char info[50];

	enum OCPP_PACKAGE_CHARGEPOINT_STATUS_E status;
	int  idTagStatus;

	char timestampIsUse:1;
	char timestamp[32];

	char vendorIdIsUse:1;
	char vendorId[255];

	char vendorErrorCodeIsUse:1;
	char vendorErrorCode[50];

}ocpp_package_StatusNotification_req_t;

typedef struct{
    sqlite3 *ocpp_db;


    enum OCPP_PACKAGE_REGISTRATION_STATUS_E RegistrationStatus;
    enum OCPP_PACKAGE_DIAGNOSTICS_STATUS_E  ocpp_diagnostics_lastDiagnosticsStatus;
	enum OCPP_PACKAGE_FIRMWARE_STATUS_E  ocpp_firmwareUpdate_lastUpdateState;

    int numberOfConnector;//枪的个数


    ocpp_package_StatusNotification_req_t **connector;
  

	//设置连接器状态
    void (*setConnectorStatus)(int connector,enum OCPP_PACKAGE_CHARGEPOINT_ERRORCODE_E errorCode, enum OCPP_PACKAGE_CHARGEPOINT_STATUS_E status);
    void (*setConnectorErrInfo)(int connector,const char * info,const char * vendorID,const char * vendorErrCode);
    

    //充电枪预约相关
    ocpp_reserve_t **reserveConnector;

    //充电枪认证相关
    ocpp_chargePoint_Authoriza_t ** authorizetion;

    //充电枪交易对象
    ocpp_chargePoint_transaction_t ** transaction_obj;

    ocpp_connect_t connect;

    //上报平台Boot Notification的信息回调，如果没有的字段返回false
    bool (*getChargeBoxSerialNumber)(char * const str, int len);
    bool (*getChargePointModel)(char * const str, int len);
    bool (*getChargePointSerialNumber)(char * const str, int len);
    bool (*getChargePointVendor)(char * const str, int len);
    bool (*getFirmwareVersion)(char * const str, int len);
    bool (*getIccid)(char * const str, int len);
    bool (*getImsi)(char * const str, int len);
    bool (*getMeterSerialNumber)(char * const str, int len);
    bool (*getMeterType)(char * const str, int len);

    void (*sendAuthorization)(int connector, const char * idTag);
    void (*sendStartTransaction)(int connector, const char *idTag, int reservationId, char *UniqueId,char *timestamp,int metervalue);//发送sendStartTransaction
    void (*sendStopTransaction)(int connector, const char *idTag, int transactionId, const char *UniqueId,int meterStop, char *timestamp,enum OCPP_PACKAGE_STOP_REASON_E reason);
    //发送身份认证后会触发回调返回结果，connector枪，result - 0认证不通过，1-认证通过
    void (*RecvAuthorizeResult)(int connector, int result);
    void (*RecvStartTransactionResult)(int transactionId,const char * status,const char * expiryDate,const char *parentIdTag,const char *UniqueId);
    void (*RecvStopTransactionResult)(const char * status,const char * expiryDate,const char *parentIdTag,const char *UniqueId); 
    //
    void (*getReservationStatus)(int connector, int status);
    //get MeterValues
    float (*getVoltage)(int connector);                          //电压
    float (*getTemperature)(int connector);                      //温度
    float (*getSOC)(int connector);                              //SOC
    float (*getRPM)(int connector);                              //风扇速度
    float (*getPowerReactiveImport)(int connector);              //车端瞬时输入无功功率
    float (*getPowerReactiveExport)(int connector);              //车端瞬时输出无功功率
    float (*getPowerOffered)(int connector);                     //提供给车端的最大功率
    float (*getPowerFactor)(int connector);                      //能量等级
    float (*getPowerActiveImport)(int connector);                //车端瞬时输入有功功率
    float (*getPowerActiveExport)(int connector);                //车端瞬时输出有功功率
    float (*getFrequency)(int connector);                        //能量频率
    float (*getEnergyReactiveExportInterval)(int connector);     //反向无功电量输出间隔
    float (*getEnergyReactiveImportInterval)(int connector);     //正向无功电量输入间隔
    float (*getEnergyActiveImportInterval)(int connector);       //正向有功电量输入间隔
    float (*getEnergyActiveExportInterval)(int connector);       //反向有功电量输出间隔
    float (*getEnergyReactiveImportRegister)(int connector);     //正向无功电量
    float (*getEnergyReactiveExportRegister)(int connector);     //反向无功电量
    float (*getEnergyActiveImportRegister)(int connector);       //(正向有功电量)----指已使用电量
    float (*getEnergyActiveExportRegister)(int connector);       //反向有功电量
    float (*getCurrentOffered)(int connector);                   //提供给电动汽车最大电流
    float (*getCurrentImport)(int connector);                    //输入电流(主体指车端)
    float (*getCurrentExport)(int connector);                    //输出电流(主体指车端)
    
    float (*getCurrentMeterValues)(int connector);               //获取当前电表值

    void (*startCharging)(int connector, int type, float metervalue,int transactionId,char *timestamp,char * idTag); //OCPP启动充电进行回调 ,type 0 刷卡启动，1远程启动,2离线启动服务器不回复则type为2
    void (*stopCharging)(int connector, int type,  float metervalue,int transactionId,char *timestamp,enum OCPP_PACKAGE_STOP_REASON_E  reason); //如果接收到服务器StopTransaction会进行回调服务器不回复则type为2
    void (*userPushStartButton)(char * idTag, int connector);                                             //用户点击启动充电
    void (*userPushStopButton)(char * idTag, int connector, enum OCPP_PACKAGE_STOP_REASON_E  reason);     //用户点击停止充电    

    void (*remoteStopCharging)(int connector);                                                            //远程停止充电
    void (*setChargingProfile)(int connector, int type, double limit);

    bool (*getOcppStatus)(void);                                     //获取OCPP连接状态，true-在线，false-不在线
    bool (*setOcppStatus)(bool Status);                          //设置OCPP连接姿态，true-在线，false-不在线，网络断开需要设置为不在线
}ocpp_chargePoint_t;


extern ocpp_chargePoint_t * ocpp_chargePoint;

int  ocpp_chargePoint_init(ocpp_chargePoint_t * pile);




#ifdef __cplusplus
}
#endif

#endif