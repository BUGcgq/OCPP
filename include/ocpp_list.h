/*
 * @Author: LIYAOHAN 1791002655@qq.com
 * @Date: 2023-04-09 17:34:12
 * @LastEditors: LIYAOHAN 1791002655@qq.com
 * @LastEditTime: 2023-04-21 10:19:14
 * @FilePath: /OCPP/ocpp_list.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __OCPP_LIST__H__
#define __OCPP_LIST__H__

#ifdef __cplusplus
extern "C" {
#endif

#include<stdbool.h>
#include<pthread.h>

#include "ocpp_package.h"
#include "ocpp_connect.h"

#define RED_COLOR "\033[31m"
#define GREEN_COLOR "\033[32m"
#define YELLOW_COLOR "\033[33m"
#define BLUE_COLOR "\033[34m"
#define MAGENTA_COLOR "\033[35m"
#define CYAN_COLOR "\033[36m"
#define WHITE_COLOR "\033[37m"

#define RESET_COLOR "\033[0m"
//////////////////////////////////////-发送-/////////////////////////////////////
#define MESSAGE_UNIQUE_LEN 38 //Unique长度
#define MESSAGE_CONTENT_LEN 2048//message长度
#define MAX_TIMEOUT_SECONDS 4//回复超时时间

typedef struct Message 
{
    char Unique[MESSAGE_UNIQUE_LEN];
    char message[MESSAGE_CONTENT_LEN];
    int  action;
    int status;//0:未发送，1：已发送，2超时
    struct Message *next;
} SendMessage;

typedef struct SendMessageQueue 
{
    SendMessage *head;
    SendMessage *tail;
    int  lastaction;
    int nodeCount; // 节点数量
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} SendMessageQueue;

void initSendMessageQueue();// 初始化消息队列
void enqueueSendMessage(const char *Unique, const char *message, int action);// 添加消息到队列
bool dequeueSendMessage(char *Unique, char *message, int *action, int *status);
void updateSendMessageStatus(const char *Unique, int status);// 更新消息状态
int getActionByUnique(const char *Unique);
/////////////////接收/////////////////////////////
typedef struct RecvMessage {
    char Unique[MESSAGE_UNIQUE_LEN];
    char message[MESSAGE_CONTENT_LEN];
    int messageTypeId; // 消息类型标识
    int action;
    struct RecvMessage *next;
} RecvMessage;

typedef struct RecvMessageQueue {
    RecvMessage *head;
    RecvMessage *tail;
    pthread_mutex_t mutex;
    int nodeCount; // 节点数量
} RecvMessageQueue;
void initRecvMessageQueue();//// 初始化消息队列
void enqueueRecvMessage(const char *Unique, const char *message, int messageTypeId, int action);//添加消息到队列
bool dequeueRecvMessage(char *Unique, char *message, int *messageTypeId, int *action);// 从队列中获取消息
bool IsRecvMessage();//是否有要发送的信息
void *sendMessageToServer(void *arg);// 发送消息给服务器的线程函数

void ocpp_message_rec(void *message, int len);
void ocpp_list_init(ocpp_connect_t *connect);
#ifdef __cplusplus
}
#endif

#endif