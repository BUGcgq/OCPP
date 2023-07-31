/*
 * @Author: LIYAOHAN 1791002655@qq.com
 * @Date: 2023-04-21 09:41:02
 * @LastEditors: LIYAOHAN 1791002655@qq.com
 * @LastEditTime: 2023-05-08 14:53:46
 * @FilePath: /OCPP/ocpp_transaction.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AEin
 */
#ifndef __OCPP_TRANSACTION__H__
#define __OCPP_TRANSACTION__H__

#ifdef __cplusplus
extern "C" {
#endif


//自定义文件
#include "ocpp_chargePoint.h"
#include "ocpp_package.h"


//内核文件
#include <string.h>
#include <stdbool.h>
#include <pthread.h>


//第三方库
#include <sqlite3.h>









enum OCPP_TRANSACTION_STATE_E{
    OCPP_TARNSACTION_STATE_NONE = 0,
    OCPP_TRANSACTION_STATE_AUTHORIZATION,
    OCPP_TRANSACTION_STATE_STARTTRANSACTION,
    OCPP_TRANSACTION_STATE_CHARGING,
    OCPP_TRANSACTION_START_STOPAUTHORIZATION,
    OCPP_TRANSACTION_STATE_STOPTRANSACTION
};


enum OCPP_TRANSACTION_START_MODEL_E{
    OCPP_TRANSACTION_START_MODEL_LOCAL = 0,
    OCPP_TRANSACTION_START_MODEL_REMOTE
};


typedef struct{

    int connector;
    char idTag[20];
    int reservationId;
    enum OCPP_TRANSACTION_START_MODEL_E startModel;

    bool (*isStop)(char * idTag, int connector);
}ocpp_transaction_data_t;


typedef struct{

    pthread_mutex_t lock;

    ocpp_transaction_data_t * tdata;

    bool isRecAuthorize_Conf;
    ocpp_package_Authorize_conf_t authorize;

    bool isRecStartTransaction_Conf;
    ocpp_package_StartTransaction_conf_t startTransaction;

    bool isRecStopTransaction_Conf;
    ocpp_package_StopTransaction_conf_t  stopTransaction;
    

    enum OCPP_PACKAGE_STOP_REASON_E  reason;

    enum OCPP_TRANSACTION_STATE_E status;
    enum OCPP_CHARGEPOINT_AUTHORIZE_RESULT_E authorizeResult;

    
    bool isConnectorServer;
    bool isRetransmission;
    bool isRemoteStop;


    char lastUniqueId[40];
}ocpp_transaction_item_t;







void ocpp_transaction_setAuthorize_Conf(int connector, ocpp_package_Authorize_conf_t authorize);
void ocpp_transaction_setStartTransaction_Conf(int connector, ocpp_package_StartTransaction_conf_t startTransaction);
void ocpp_transaction_setStopTransaction_Conf(int connector, ocpp_package_StopTransaction_conf_t stopTransaction);
void ocpp_transaction_setStopReason(int connector, enum OCPP_PACKAGE_STOP_REASON_E  reason);
void ocpp_transaction_setServerStatus(int connector, bool isConnectorServer);
void ocpp_transaction_setRetransmission(int connector);
void ocpp_transaction_setRemoteStop(int connector);
bool ocpp_transaction_isTransaction(int connector);
char * ocpp_transaction_getLastUniqueId(int connector);
int ocpp_transaction_getTransactionId(int connector);

void * ocpp_transaction_transaction(void * arg);
void ocpp_transaction_init(int numberOfConnector);



/*
1.用户点击启动充电按钮
2.OCPP发送Authorization认证卡号
3.界面等待认证结果

4.1--认证失败：返回空闲界面
4.2--认证成功：OCPP发送StartTransaction

5.1--StartTransaction允许充电--发送MeterValues
5.2--StartTransaction禁止充电--发送StopTransion--reason：DeAuthorized


6.1--用户点击停止充电按钮--OCPP判断是否和StartTransaction属于同一卡号,如果为同一卡号,认证成功,否则OCPP发送Authorization等待认证成功,认证成功后发送StopTransaction
6.2--遇到其它情况停止，直接发送StopTransaction



接收到RemoteStartTransaction
1.发送StartTransaction






*/




#ifdef __cplusplus
}
#endif

#endif