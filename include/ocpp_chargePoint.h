/*
 * @Author: LIYAOHAN 1791002655@qq.com
 * @Date: 2023-04-15 10:00:41
 * @LastEditors: LIYAOHAN 1791002655@qq.com
 * @LastEditTime: 2023-05-09 09:38:00
 * @FilePath: /OCPP/ocpp_chargePoint.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __OCPP_CHARGE_POINT__H__
#define __OCPP_CHARGE_POINT__H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ocpp_connect.h"
#include "ocpp_package.h"
#include "ocpp_reserve.h"
#include "ocpp_ftp.h"



enum OCPP_CHARGEPOINT_AUTHORIZE_RESULT_E {
    OCPP_CHARGEPOINT_AUTHORIZE_RESULT_FAIL = 0,    // 授权失败
    OCPP_CHARGEPOINT_AUTHORIZE_RESULT_ONGOING,      // 授权正在进行中
    OCPP_CHARGEPOINT_AUTHORIZE_RESULT_SUCCEED,      // 授权成功
    OCPP_CHARGEPOINT_AUTHORIZE_RESULT_UNKOWN       // 授权结果未知

};


//认证标识符
typedef struct{
    bool isWaitAuthoriza;
	char idTag[OCPP_AUTHORIZATION_IDTAG_LEN];
    enum OCPP_CHARGEPOINT_AUTHORIZE_RESULT_E result;
    char lastUniqueId[40];
}ocpp_chargePoint_Authoriza_t;




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


    enum OCPP_CHARGEPOINT_AUTHORIZE_RESULT_E authorizeResult;

    bool isRecStartTransaction_Conf;
    ocpp_package_StartTransaction_conf_t startTransaction;

    bool isRecStopTransaction_Conf;
    ocpp_package_StopTransaction_conf_t  stopTransaction;

    bool isRetransmission;                                //服务器应答错误,需要重新发送数据包

}ocpp_chargePoint_transaction_t;



//离线数据对象
typedef struct{
	bool isCreate;                            //是否已经创建线程
	char lastUniqueId[40];
	bool isHaveDate;
	bool isResponse;
	bool isRetransmission;
	
}ocpp_chargePoint_offlineDate_t;




//诊断日志对象
typedef struct{
	ocpp_ftp_client_t client;
	int port;                                          //ftp连接端口
	char serverAddr[32];                               //服务器地址
	
	int retries;                                       //重传次数
	int retryInterval;                                 //间隔

	bool isUseStartTime;
	char startTime[32];

	bool isUseStopTime;
	char stopTime[32];

}ocpp_chargePoint_diagnostics_t;


//固件更新对象
typedef struct{
	ocpp_ftp_client_t client;
	int port;                                          //ftp连接端口
	char serverAddr[32];                               //服务器地址
	int retries;                                       //重传次数
	int retryInterval;                                 //间隔
	char retrieveDate[32];                             //更新时间

}ocpp_chargePoint_UpdateFirmware_t;



typedef struct{
    sqlite3 *ocpp_db;


    enum OCPP_PACKAGE_REGISTRATION_STATUS_E RegistrationStatus;
    enum OCPP_PACKAGE_DIAGNOSTICS_STATUS_E  ocpp_diagnostics_lastDiagnosticsStatus;
	enum OCPP_PACKAGE_FIRMWARE_STATUS_E  ocpp_firmwareUpdate_lastUpdateState;

    int numberOfConnector;


    ocpp_package_StatusNotification_req_t **connector;
  

	//设置连接器状态
    void (*setConnectorStatus)(int connector,enum OCPP_PACKAGE_CHARGEPOINT_ERRORCODE_E errorCode, enum OCPP_PACKAGE_CHARGEPOINT_STATUS_E status);
    void (*setConnectorErrInfo)(int connector,const char * info,const char * vendorID,const char * vendorErrCode);
    

    //充电枪预约相关
    ocpp_reserve_t **reserveConnector;
    char *  isReservation;                   //bool 型二维数组

    //充电枪认证相关
    ocpp_chargePoint_Authoriza_t ** authorizetion;

    //充电枪交易对象
    ocpp_chargePoint_transaction_t ** transaction_obj;

	//离线对象
	ocpp_chargePoint_offlineDate_t offlineDate_obj;


    ocpp_connect_t connect;


    //have return true,not have return false
    bool (*getChargeBoxSerialNumber)(char * const str, int len);
    bool (*getChargePointModel)(char * const str, int len);
    bool (*getChargePointSerialNumber)(char * const str, int len);
    bool (*getChargePointVendor)(char * const str, int len);
    bool (*getFirmwareVersion)(char * const str, int len);
    bool (*getIccid)(char * const str, int len);
    bool (*getImsi)(char * const str, int len);
    bool (*getMeterSerialNumber)(char * const str, int len);
    bool (*getMeterType)(char * const str, int len);


    //Authoriza Reasult
    void (*setAuthorizeResult)(int connector, int result);


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

    void (*startCharging)(int connector);                        //启动充电
    // void (*stopCharging)(int connector);                         //停止充电
    
    // bool (*userPushStartButton)(char * idTag, int connector);    //用户点击启动充电
    // bool (*userPushStopButton)(char * idTag, int connector);     //用户点击停止充电

    void (*userPushStartButton)(char * idTag, int connector);                                             //用户点击启动充电
    void (*userPushStopButton)(char * idTag, int connector, enum OCPP_PACKAGE_STOP_REASON_E  reason);     //用户点击停止充电    

    void (*remoteStopCharging)(int connector);                                                            //远程停止充电

}ocpp_chargePoint_t;





extern ocpp_chargePoint_t * ocpp_chargePoint;

bool ocpp_chargePoint_getConnectStatus();
void ocpp_chargePoint_setReConnect();
void ocpp_chargePoint_init(ocpp_chargePoint_t * pile);

enum OCPP_CHARGEPOINT_AUTHORIZE_RESULT_E ocpp_chargePoint_authorizationOfIdentifier(const char * const idTag, char * uniqueId);
void ocpp_chargePoint_Authorization_IdTag(int connector, const char * idTag);




#ifdef __cplusplus
}
#endif

#endif