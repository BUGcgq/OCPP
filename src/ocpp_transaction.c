#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "ocpp_transaction.h"
#include "ocpp_chargePoint.h"
#include "ocpp_configurationKey.h"
#include "ocpp_auxiliaryTool.h"
#include "ocpp_log.h"


static ocpp_transaction_item_t  **transaction;
static sqlite3 *ocpp_HO;


/**
 * @description: 创建未结算记录
 * @param:
 * @return: 0-success, -1-fail
 */
static int ocpp_transaction_create_UnsettledRecord_table(sqlite3 *p_db){
	char *zErrMsg = 0;
	char sql[1024] = {0};

    snprintf(sql, sizeof(sql), "CREATE TABLE IF NOT EXISTS UnsettledOfOrder("\
          "NO                integer                 PRIMARY KEY,"\
          "Connector         integer                 NOT NULL,"\
          "TransactionId     integer                 NOT NULL,"\
          "IdTag             TEXT                    NOT NULL,"\
          "MeterStart        real                    NOT NULL,"\
          "MeterStop         real                    default 0,"\
          "StartTime         TEXT                    NOT NULL,"\
          "StopTime          TEXT                    default NULL,"\
          "StopReason        TEXT                    default NULL);");
    

    if(sqlite3_exec(p_db, sql, NULL, 0, &zErrMsg) != SQLITE_OK){
    	OCPP_LOG_ERROR("create UnsettledOfOrder fail %s\n",zErrMsg); 
        sqlite3_free(zErrMsg);
        return -1;
    }

    return 0;
}



/**
 * @description: 插入未结算记录
 * @param:
 * @return: 0-success, -1-fail
 */
static int ocpp_transaction_insertAUnsetlledRecord(int connector, int transactionId, const char * idTag, float meterStart){
    
    if(idTag == NULL) return -1;
    if(connector <= 0) return -1;
    
    char sql[512] = {0};
    char *ErrMsg;
    
    snprintf(sql, 512, "INSERT INTO UnsettledOfOrder(Connector,TransactionId,IdTag,MeterStart,MeterStop,StartTime,StopTime,StopReason)" \
                                            "VALUES(%d,%d,'%s',%0.2f,%0.2f,datetime('now'),datetime('now'),'Reboot')",
                                            connector, transactionId, idTag, meterStart, meterStart);

    
    if(sqlite3_exec(ocpp_HO,sql,NULL, NULL,&ErrMsg) != SQLITE_OK){
        OCPP_LOG_ERROR("ERROR = %s\n",ErrMsg);
        sqlite3_free(ErrMsg);
        return -1;
    }

    return 0;
}


/**
 * @description: 更新未结算记录
 * @param:
 * @return: 0-success, -1-fail
 */
static int ocpp_transaction_upDateAUnsetlledRecord(int transactionId, float meterStop, char *reason){
        
    char sql[128] = {0};
    char *ErrMsg;

    snprintf(sql, 128,"UPDATE UnsettledOfOrder set MeterStop = %0.2f, StopReason = %s where transactionId = %d",
                                                      meterStop, reason, transactionId);

    if(sqlite3_exec(ocpp_HO,sql,NULL, NULL,&ErrMsg) != SQLITE_OK){
        printf("ERROR = %s\n",ErrMsg);
        sqlite3_free(ErrMsg);
        return -1;
    }

    return 0;
}

/**
 * @description: 删除未结算记录
 * @param:
 * @return: 0-success, -1-fail
 */
static int ocpp_transaction_deleteAUnsetlledRecord(int transaction){

    char sql[128] = {0};
    char *ErrMsg;

    snprintf(sql,128,"delete from UnsettledOfOrder where TransactionId = %d",transaction);
    
    if(sqlite3_exec(ocpp_HO,sql,NULL, NULL,&ErrMsg) != SQLITE_OK){
        printf("ERROR = %s\n",ErrMsg);
        sqlite3_free(ErrMsg);
        return -1;
    }

    return 0; 
}



/**
 * @description: 获取未结算记录数目
 * @param:
 * @return: -1:获取失败, 0 > 获取数目
 */
static int ocpp_transaction_getUnsetlledRecordCnt(){

 	char sql[128] = {0};
	snprintf(sql,128,"SELECT count(*) FROM UnsettledOfOrder;");
	sqlite3_stmt *res;
	
	int find = -1;
	
	if (sqlite3_prepare_v2(ocpp_HO, sql, -1, &res,NULL) != SQLITE_OK){
		OCPP_LOG_ERROR("查询失败 'SELECT count(*) FROM UnsettledOfOrder;'");
		
	}

	if(sqlite3_step(res) == SQLITE_ROW){
		find = sqlite3_column_int(res,0);
	}
	
	sqlite3_finalize(res);
	return find;	

}







/**
 * @description: 创建历史订单记录
 * @param:
 * @return: -1 = fail, 0 = success
 */
static int ocpp_transaction_create_HistoryOfOrder_table(sqlite3 *p_db){
	char *zErrMsg = 0;
	char sql[512];

	//CREATE TABLE IF NOT EXISTS
    snprintf(sql, sizeof(sql), "CREATE TABLE IF NOT EXISTS HistoryOfOrder("  \
          "No              integer PRIMARY KEY," \
          "TransactionID    TEXT    NOT NULL," \
          "StartTime        TEXT    NOT NULL," \
          "EndTime          TEXT    NOT NULL," \
          "Settlement       TEXT    NOT NULL," \
          "IdTag            TEXT    ,"         \
          "ChargeTime       REAL    NOT NULL," \
          "ChargePrice      REAL    NOT NULL," \
		  "Energy           REAL    NOT NULL," \
          "EndReason        TEXT    NOT NULL);");
    

    if(sqlite3_exec(p_db, sql, NULL, 0, &zErrMsg) != SQLITE_OK){
		
    	OCPP_LOG_DEBUG("创建表失败 %s",zErrMsg);        
        sqlite3_free(zErrMsg);
        return -1;
    }
    return 0;
}









/**
 * @description: 初始化交易数据库
 * @param:
 * @return: 
 */
//void ocpp_transaction_init(sqlite3 *ocpp_db){
//
//
//
//
//}





///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


unsigned int test = 0;
bool isStop(char * idTag, int connector){
    strncpy(idTag, "0ABCFC49", 20);
    if(ocpp_AuxiliaryTool_getDiffTime_ms(&test) >= 30 * 1000) return true;
    return false;
}


/**
 * @description: 接收到Authorize回复是设置
 * @param:
 * @return: 
 */
void ocpp_transaction_setAuthorize_Conf(int connector, ocpp_package_Authorize_conf_t authorize){
    if(transaction[connector -1] == NULL) return;

    pthread_mutex_lock(&transaction[connector - 1]->lock);

    transaction[connector -1]->isRecAuthorize_Conf = true;
    transaction[connector -1]->authorize.idTagInfo.AuthorizationStatus = authorize.idTagInfo.AuthorizationStatus;

    pthread_mutex_unlock(&transaction[connector - 1]->lock);
}


/**
 * @description: 接收到StartTransaction回复是设置
 * @param:
 * @return: 
 */
void ocpp_transaction_setStartTransaction_Conf(int connector, ocpp_package_StartTransaction_conf_t startTransaction){
    if(transaction[connector -1] == NULL) return;

    pthread_mutex_lock(&transaction[connector - 1]->lock);

    transaction[connector - 1]->isRecStartTransaction_Conf = true;
    transaction[connector - 1]->startTransaction.idTagInfo.AuthorizationStatus = startTransaction.idTagInfo.AuthorizationStatus;
    transaction[connector - 1]->startTransaction.transactionId = startTransaction.transactionId;

    pthread_mutex_unlock(&transaction[connector - 1]->lock);

}


/**
 * @description: 接收到StopTransaction回复是设置
 * @param:
 * @return: 
 */

void ocpp_transaction_setStopTransaction_Conf(int connector, ocpp_package_StopTransaction_conf_t stopTransaction){
    if(transaction[connector -1] == NULL) return;
    pthread_mutex_lock(&transaction[connector - 1]->lock);

    transaction[connector - 1]->isRecStopTransaction_Conf = true;
    if(stopTransaction.idTagInfoIsUse){
        transaction[connector -1]->stopTransaction.idTagInfo.AuthorizationStatus = stopTransaction.idTagInfo.AuthorizationStatus;

    }else{

        transaction[connector -1]->stopTransaction.idTagInfo.AuthorizationStatus = OCPP_LOCAL_AUTHORIZATION_INVALID;
    }
    

    pthread_mutex_unlock(&transaction[connector - 1]->lock);
}


/**
 * @description: 
 * @param:
 * @return: 
 */
void ocpp_transaction_setStopReason(int connector, enum OCPP_PACKAGE_STOP_REASON_E  reason){
    if(transaction[connector -1] == NULL) return;
    pthread_mutex_lock(&transaction[connector - 1]->lock); 

    transaction[connector -1]->reason = reason;

    pthread_mutex_unlock(&transaction[connector - 1]->lock);
}


/**
 * @description: 设置服务器连接状态
 * @param:
 * @return: 
 */
void ocpp_transaction_setServerStatus(int connector, bool isConnectorServer){
    if(transaction[connector -1] == NULL) return;
    pthread_mutex_lock(&transaction[connector - 1]->lock); 

    transaction[connector -1]->isConnectorServer = isConnectorServer;

    pthread_mutex_unlock(&transaction[connector - 1]->lock);
}

/**
 * @description: 设置交易信息需要重传
 * @param:
 * @return: 
 */
void ocpp_transaction_setRetransmission(int connector){
    if(transaction[connector -1] == NULL) return;
    pthread_mutex_lock(&transaction[connector - 1]->lock); 

    transaction[connector -1]->isRetransmission = true;

    pthread_mutex_unlock(&transaction[connector - 1]->lock);
}


/**
 * @description: 设置远程停止充电
 * @param:
 * @return: 
 */
void ocpp_transaction_setRemoteStop(int connector){
    if(transaction[connector -1] == NULL) return;
    pthread_mutex_lock(&transaction[connector - 1]->lock); 

    transaction[connector -1]->isRemoteStop = true;

    pthread_mutex_unlock(&transaction[connector - 1]->lock);
}

/**
 * @description: 判断connector是否在交易
 * @param:
 * @return: 
 */
bool ocpp_transaction_isTransaction(int connector){
    // DEBUG_MSG("connector = %d", connector);
    // if(transaction[0] == NULL)DEBUG_MSG("0T = NULL");
    // else DEBUG_MSG("0T != NULL");
    // if(transaction[1] == NULL)DEBUG_MSG("1T = NULL");
    // else DEBUG_MSG("1T != NULL");

    if(transaction[connector -1] == NULL) return false;

    return true;

}

/**
 * @description: 获取交易上一次发送的uniqueId
 * @param:
 * @return: 
 */
char * ocpp_transaction_getLastUniqueId(int connector){

    if(transaction[connector -1] == NULL) return NULL;
    pthread_mutex_lock(&transaction[connector - 1]->lock);   

    char * lastUniqueId = strdup(transaction[connector - 1]->lastUniqueId);
    pthread_mutex_unlock(&transaction[connector - 1]->lock);

    return lastUniqueId;
}



/**
 * @description: 获取交易transactionId
 * @param {int} connector
 * @return {*}
 */
int ocpp_transaction_getTransactionId(int connector){

    if(transaction[connector -1] == NULL) return 0;
    pthread_mutex_lock(&transaction[connector - 1]->lock);   

    int transactionId = transaction[connector - 1]->startTransaction.transactionId;
    pthread_mutex_unlock(&transaction[connector - 1]->lock);


    return transactionId;
}




/**
 * @description: 交易线程
 * @param:
 * @return: 
 */
void * ocpp_transaction_transaction(void * arg){
    test = ocpp_AuxiliaryTool_getSystemTime_ms();
    OCPP_LOG_DEBUG("create transaction thread");
    
    ocpp_transaction_data_t * tdata = (ocpp_transaction_data_t *)arg;
    if(tdata == NULL) return;

    int connector = tdata->connector;
    OCPP_LOG_DEBUG("connector = %d",connector);
    if(transaction[connector -1] != NULL) return;

    transaction[connector - 1] = (ocpp_transaction_item_t *)calloc(1, sizeof(ocpp_transaction_item_t));
    ocpp_transaction_item_t * item = transaction[connector - 1];
    pthread_mutex_init(&item->lock, NULL);


    pthread_mutex_lock(&item->lock);
    item->tdata = tdata;
    if(item->tdata->isStop == NULL) item->tdata->isStop = isStop;

    item->authorizeResult = OCPP_CHARGEPOINT_AUTHORIZE_RESULT_FAIL;
    item->isRecAuthorize_Conf = false;
    item->isRecStartTransaction_Conf = false;
    item->isRecStopTransaction_Conf = false;
    item->reason = OCPP_PACKAGE_STOP_REASON_MAX;
    
    item->status = OCPP_TARNSACTION_STATE_NONE;
    item->isConnectorServer = false;
    item->isRemoteStop = false;
    memset(item->lastUniqueId, 0, 40);
    pthread_mutex_unlock(&item->lock);


    bool authorizeRemoteTxRequests = false;
    ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[OCPP_CK_AuthorizeRemoteTxRequests], (void*)&authorizeRemoteTxRequests);

    int transactionMessageAttempts = 0;
    ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[OCPP_CK_TransactionMessageAttempt], (void*)&transactionMessageAttempts);

    int transactionMessageRetryInterval = 0;
    ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[OCPP_CK_TransactionMessageRetryInterval], (void*)&transactionMessageRetryInterval);

    int meterValueSampleInterval = 0;
    ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[OCPP_CK_MeterValueSampleInterval], (void*)&meterValueSampleInterval);

    bool allowOfflineTxForUnknowIdIsUse = ocpp_ConfigurationKey_isUse(ocpp_configurationKeyText[OCPP_CK_AllowOfflineTxForUnknownId]);

    bool allowOfflineTxForUnknowId = false;
    ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[OCPP_CK_AllowOfflineTxForUnknownId], (void *)&allowOfflineTxForUnknowId);




    bool isWaitResult = false;
    int attempts = 0;
    unsigned int RetriesInterval = 0;

    bool terminate = false;
    char stopIdTag[20] = {0};
    char * lastUniqueId = NULL;

    while(1){
        if(terminate) { OCPP_LOG_DEBUG("terminate!");  break; }
        // DEBUG_MSG("status = %d",item->status);
        pthread_mutex_lock(&item->lock);
        switch(item->status){
            case OCPP_TARNSACTION_STATE_NONE:
                    item->status = OCPP_TRANSACTION_STATE_AUTHORIZATION;
                break;

            case OCPP_TRANSACTION_STATE_AUTHORIZATION:
                    if(item->tdata->startModel == OCPP_TRANSACTION_START_MODEL_REMOTE)
                        if(authorizeRemoteTxRequests == false){ 
                            item->status = OCPP_TRANSACTION_STATE_STARTTRANSACTION;
                            break;
                        }
                    
                    if(isWaitResult == false){

                        item->authorizeResult = ocpp_chargePoint_authorizationOfIdentifier(item->tdata->idTag, &lastUniqueId);
                        OCPP_LOG_DEBUG("last = %s",lastUniqueId);
                        printf("授权结果 = %d\n",item->authorizeResult);
                        if(lastUniqueId != NULL) strncpy(item->lastUniqueId, lastUniqueId, 40);
                        OCPP_LOG_DEBUG("Authorize Result = %d",item->authorizeResult);
                        switch(item->authorizeResult){
                            case OCPP_CHARGEPOINT_AUTHORIZE_RESULT_SUCCEED:

                                item->status = OCPP_TRANSACTION_STATE_STARTTRANSACTION;

                                break;

                            case OCPP_CHARGEPOINT_AUTHORIZE_RESULT_ONGOING:

                                isWaitResult = true;
                                break;
                            
                            case OCPP_CHARGEPOINT_AUTHORIZE_RESULT_FAIL:
                                terminate = true;
                                break;

                            case OCPP_CHARGEPOINT_AUTHORIZE_RESULT_UNKOWN:

                                terminate = true;
                                if(allowOfflineTxForUnknowIdIsUse){
                                    if(allowOfflineTxForUnknowId){
                                        item->status = OCPP_TRANSACTION_STATE_STARTTRANSACTION;
                                        terminate = false;
                                    }

                                }                                
                                break;

                            default:
                                break;

                        }
                             
                    }else{
                        if(item->isRecAuthorize_Conf){
                            if(item->authorize.idTagInfo.AuthorizationStatus == OCPP_PACKAGE_AVAILABILITY_STATUS_ACCEPTED){
                                OCPP_LOG_DEBUG("Accepted");
                                item->status = OCPP_TRANSACTION_STATE_STARTTRANSACTION;
                                isWaitResult = false;
                            }
                            else{
                                terminate = true;
                                break;
                            }
                                

                        }
                    }
                break;

            case OCPP_TRANSACTION_STATE_STARTTRANSACTION:
                if(isWaitResult == false){
                    lastUniqueId = ocpp_chargePoint_sendStartTransaction(item->tdata->connector, item->tdata->idTag, item->tdata->reservationId);
                    strncpy(item->lastUniqueId, lastUniqueId, 40);
                    if(item->isConnectorServer){
                        attempts++;
                        RetriesInterval = ocpp_AuxiliaryTool_getSystemTime_ms();
                        isWaitResult = true;
                        printf("服务器连接,等待回复\n");
                        OCPP_LOG_DEBUG("服务器连接,等待回复");
                    }else{

                        //启动充电,并设置认证成功
                        ocpp_chargePoint->startCharging(item->tdata->connector);
                        ocpp_chargePoint->setAuthorizeResult(item->tdata->connector, true);

                        item->status = OCPP_TRANSACTION_STATE_CHARGING;
                        item->startTransaction.transactionId = ocpp_AuxiliaryTool_GenerateInt();
                        printf("服务器未连接,不需要等待回复\n");
                        OCPP_LOG_DEBUG("服务器未连接,不需要等待回复");
                    }

                }else{

                    if(item->isRetransmission == false){                   
                        if(item->isRecStartTransaction_Conf){

                            if(item->startTransaction.idTagInfo.AuthorizationStatus == OCPP_LOCAL_AUTHORIZATION_ACCEPTED){
                                OCPP_LOG_DEBUG("允许充电");
                                printf("允许充电\n");
                                ////启动充电,并设置认证成功
                                ocpp_chargePoint->startCharging(item->tdata->connector);
                                item->status = OCPP_TRANSACTION_STATE_CHARGING;
                                isWaitResult = false;
                                attempts = 0;
                                RetriesInterval = ocpp_AuxiliaryTool_getSystemTime_ms();
                            }else{
                                terminate = true;
                                break;

                            }
                        }

                    }else{
                        //retry
                        OCPP_LOG_DEBUG("attempts time = %lds", attempts * transactionMessageRetryInterval);
                        if(attempts < transactionMessageAttempts){
                            if(ocpp_AuxiliaryTool_getDiffTime_ms(&RetriesInterval) >= attempts * transactionMessageRetryInterval){
                                isWaitResult = false;
                                item->isRetransmission = false;
                            }
                        }else{
                            ocpp_chargePoint->setAuthorizeResult(item->tdata->connector, false);
                            terminate = true;
                            break;
                        }

                    }

                }
                break;

            case OCPP_TRANSACTION_STATE_CHARGING:
                if(item->tdata->isStop(stopIdTag, item->tdata->connector - 1)){
                    if(strcmp(item->tdata->idTag, stopIdTag) != 0){

                        item->isRecAuthorize_Conf = false;
                        item->status = OCPP_TRANSACTION_START_STOPAUTHORIZATION;

                    }else{
                        item->status = OCPP_TRANSACTION_STATE_STOPTRANSACTION;
                    }
                    
                }

                if(item->isRemoteStop){
                    strncpy(stopIdTag, item->tdata->idTag, 20);
                    item->status = OCPP_TRANSACTION_STATE_STOPTRANSACTION;

                }


                if(ocpp_AuxiliaryTool_getDiffTime_ms(&RetriesInterval) >= meterValueSampleInterval * 1000){

                    lastUniqueId = ocpp_chargePoint_sendMeterValues(item->tdata->connector,item->startTransaction.transactionId);
                    strncpy(item->lastUniqueId, lastUniqueId, 40);
                    RetriesInterval = ocpp_AuxiliaryTool_getSystemTime_ms();

                }

                if(item->authorizeResult == OCPP_CHARGEPOINT_AUTHORIZE_RESULT_UNKOWN){
                    if(item->isConnectorServer){
                        if(isWaitResult == false){
                            lastUniqueId = ocpp_chargePoint_sendStartTransaction(item->tdata->connector, item->tdata->idTag, item->tdata->reservationId);    
                            strncpy(item->lastUniqueId, lastUniqueId, 40);    
                            attempts++;
                            RetriesInterval = ocpp_AuxiliaryTool_getSystemTime_ms();
                            isWaitResult = true;

                        }else{

                            if(item->isRetransmission == false){                   
                                if(item->isRecStartTransaction_Conf){
                                    isWaitResult = false;
                                    attempts = 0;
                                    if(item->startTransaction.idTagInfo.AuthorizationStatus == OCPP_LOCAL_AUTHORIZATION_ACCEPTED){
                                        item->status = OCPP_TRANSACTION_STATE_CHARGING;

                                        RetriesInterval = ocpp_AuxiliaryTool_getSystemTime_ms();

                                    }else{

                                        //terminate transaction
                                        item->status = OCPP_TRANSACTION_STATE_STOPTRANSACTION;
                                        item->reason = OCPP_PACKAGE_STOP_REASON_DEAUTHORIZED;
                                        strncpy(stopIdTag, item->tdata->idTag, 20);
                                    }
                                }
                            }else{

                                if(attempts < transactionMessageAttempts){
                                    if(ocpp_AuxiliaryTool_getDiffTime_ms(&RetriesInterval) >= attempts * transactionMessageRetryInterval){
                                        isWaitResult = false;
                                        item->isRetransmission = false;
                                    }

                                }else{
                                    //not impletement

                                }
                            }                             
                        }
                    }
                }
                break;

            case OCPP_TRANSACTION_START_STOPAUTHORIZATION:
                if(isWaitResult == false){
                    item->authorizeResult = ocpp_chargePoint_authorizationOfIdentifier(stopIdTag, &lastUniqueId);
                    strncpy(item->lastUniqueId, lastUniqueId, 40);

                    switch(item->authorizeResult){
                        case OCPP_CHARGEPOINT_AUTHORIZE_RESULT_ONGOING:
                            isWaitResult = true;
                            break;
                        
                        case OCPP_CHARGEPOINT_AUTHORIZE_RESULT_FAIL:
                            isWaitResult = false;
                            item->status = OCPP_TRANSACTION_STATE_CHARGING;
                            break;
                        
                        case OCPP_CHARGEPOINT_AUTHORIZE_RESULT_SUCCEED:
                            isWaitResult = false;
                            item->status = OCPP_TRANSACTION_START_STOPAUTHORIZATION;
                            break;
                        
                        case OCPP_CHARGEPOINT_AUTHORIZE_RESULT_UNKOWN:
                            isWaitResult = false;
                            item->status = OCPP_TRANSACTION_STATE_CHARGING;
                            
                            if(allowOfflineTxForUnknowIdIsUse)
                                if(allowOfflineTxForUnknowId){
                                    isWaitResult = false;
                                    item->status = OCPP_TRANSACTION_START_STOPAUTHORIZATION;
                                }
                            break;
                        default:
                            break;

                    }

                }else{
                    if(item->isRecAuthorize_Conf){
                        if(item->authorize.idTagInfo.AuthorizationStatus == OCPP_LOCAL_AUTHORIZATION_ACCEPTED){
                            item->status = OCPP_TRANSACTION_STATE_STOPTRANSACTION;

                        }else{
                            item->status = OCPP_TRANSACTION_STATE_CHARGING;
                        }

                        isWaitResult = false;
                    }

                }
                break;

            case OCPP_TRANSACTION_STATE_STOPTRANSACTION:
                if(isWaitResult == false){
                    lastUniqueId = ocpp_transaction_sendStopTransaction_Simpleness(item->tdata->connector, stopIdTag, 
                                                                    item->startTransaction.transactionId, item->reason);

                    strncpy(item->lastUniqueId, lastUniqueId, 40);

                    if(item->isConnectorServer){
                        attempts++;
                        RetriesInterval = ocpp_AuxiliaryTool_getSystemTime_ms();
                        isWaitResult = true;
                    }else{

                        //stop charging and set Authorizetion success
                        // ocpp_chargePoint->stopCharging(item->tdata->connector);
                        ocpp_chargePoint->setAuthorizeResult(item->tdata->connector, true);
                        terminate = true;
                        break;

                    }


                }else{

                    if(item->isRetransmission == false){
                        if(item->isRecStopTransaction_Conf){
                            if(item->stopTransaction.idTagInfo.AuthorizationStatus == OCPP_LOCAL_AUTHORIZATION_ACCEPTED){
                                OCPP_LOG_DEBUG("permit stop charging!");

                                //stop charging and set Authorizetion success
                                // ocpp_chargePoint->stopCharging(item->tdata->connector);
                                ocpp_chargePoint->setAuthorizeResult(item->tdata->connector, true);
                                terminate = true;
                                break;
                                
                            }else{
                                
                                //set Authorizetion fail
                                ocpp_chargePoint->setAuthorizeResult(item->tdata->connector, false);
                                item->status = OCPP_TRANSACTION_STATE_CHARGING;

                            }

                        }

                    }else{
                        if(attempts < transactionMessageAttempts){

                            if(ocpp_AuxiliaryTool_getDiffTime_ms(&RetriesInterval) >= attempts * transactionMessageRetryInterval){
                                isWaitResult = false;
                                item->isRetransmission = false;

                            }

                        }else{
                            terminate = true;
                            break;
                        }
                    }
                }
                break;
        }
        pthread_mutex_unlock(&item->lock);
        
        usleep(100 * 1000);    //100ms
    }


    pthread_mutex_destroy(&item->lock);

    free(item->tdata);
    free(transaction[connector - 1]);
    transaction[connector - 1] = NULL;

}








/**
 * @description: 
 * @param:
 * @return: 
 */
void ocpp_transaction_init(int numberOfConnector){

    transaction = (ocpp_transaction_item_t **)calloc(numberOfConnector, sizeof(ocpp_transaction_item_t *));

    int i = 0;
    for(i = 0; i < numberOfConnector; i++){
        transaction[i] = NULL;

    }
}


































