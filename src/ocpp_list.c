
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <json-c/json.h>

#include "ocpp_log.h"
#include "ocpp_list.h"

#define OCPP_LIST_TIMER_TIME_S 3 // 定时器时间

static ocpp_list_root_SendItem_t rootSend; // 发送队列
static ocpp_list_root_RecvItem_t rootRecv; // 接收队列
static pthread_rwlock_t ocpp_list_rwlock;  // 读写锁
static timer_t timerid;

static void show()
{
    ocpp_list_SendItem_t *send = rootSend.index;
    ocpp_list_RecvItem_t *recv = rootRecv.index;

    printf("\n\n>>>>>>>>>>>>>>>>>>SEND\n");
    printf("\t numberOfItem = %d\n", rootSend.numberOfItem);
    printf("\t isTimeout = %d\n", rootSend.isTimeout);
    printf("\t isRec = %d\n", rootSend.isRec);
    printf("\t lastUnique = %s\n", rootSend.lastUnique);
    if (rootSend.index != NULL)
        printf("\t nextUnique = %s\n", rootSend.index->Unique);

    printf("\nRECV<<<<<<<<<<<<<<<<<<\n");
    printf("\t numberOfItem = %d\n", rootRecv.numberOfItem);
    printf("\t isOther = %d\n", rootRecv.isOther);
    printf("\t index = %p\n", rootRecv.index);
    printf("\t unique = %s\n", rootRecv.unique);
    if (rootRecv.index != NULL)
        printf("\t nextUnique = %s\n", rootRecv.index->unique);
    printf("\n\n");

    int i = 0;
    while (send != NULL)
    {
        printf("send_%d = %p  send_%d = %p\n", i, send, i, send->next);
        send = send->next;
        i++;
    }

    printf("\n\n");
    i = 0;
    while (recv != NULL)
    {
        printf("recv_%d = %p  recv_%d = %p\n", i, recv, i, recv->next);
        recv = recv->next;
    }
    printf("\n\n");
}

/**
 * @description: 初始化队列
 * @param:
 * @return:
 */
static void ocpp_list_initSendRecv_Queue(void)
{

    pthread_rwlock_wrlock(&ocpp_list_rwlock);

    rootSend.numberOfItem = 0;
    rootSend.isTimeout = false;
    rootSend.isRec = true;
    rootSend.lastUnique = (char *)calloc(1, 40);
    rootSend.package = OCPP_PACKAGE_NONE;
    rootSend.index = NULL;

    rootRecv.numberOfItem = 0;
    rootRecv.isOther = false;
    rootRecv.index = NULL;
    rootRecv.package = OCPP_PACKAGE_NONE;
    rootRecv.unique = (char *)calloc(1, 40);

    pthread_rwlock_unlock(&ocpp_list_rwlock);
}

/**
 * @description: 插入发送节点
 * @param:
 * @return:
 */
void ocpp_list_insertSendItem(ocpp_list_SendItem_t *item)
{
    // DEBUG_MSG("start");
    pthread_rwlock_wrlock(&ocpp_list_rwlock);
    ocpp_list_SendItem_t *temp = rootSend.index;
    item->next = NULL;

    if (rootSend.index == NULL)
    {
        rootSend.index = item;
        // rootSend.package = item->package;
    }
    else
    {

        while (temp->next != NULL)
        {
            temp = temp->next;
        }
        temp->next = item;
    }
    rootSend.numberOfItem++;
    pthread_rwlock_unlock(&ocpp_list_rwlock);
    // DEBUG_MSG("END");
}

/**
 * @description: 插入接收节点
 * @param:
 * @return:
 */
static void ocpp_list_insertRecvItem(ocpp_list_RecvItem_t *item)
{

    pthread_rwlock_wrlock(&ocpp_list_rwlock);
    ocpp_list_RecvItem_t *temp = rootRecv.index;
    item->next = NULL;

    if (rootRecv.index == NULL)
    {
        rootRecv.index = item;
        rootRecv.package = item->package;
        rootRecv.isOther = item->isOther;
        strncpy(rootRecv.unique, item->unique, strlen(item->unique));
    }
    else
    {
        while (temp->next != NULL)
        {
            temp = temp->next;
        }
        temp->next = item;
    }
    rootRecv.numberOfItem++;
    pthread_rwlock_unlock(&ocpp_list_rwlock);
}

/**
 * @description: 从头节点获取第一个数据并从链表中删除
 * @param:
 * @return:
 */
static ocpp_list_SendItem_t *ocpp_list_readAndDeleteFirstSendItem()
{

    ocpp_list_SendItem_t *temp = NULL;

    if (rootSend.numberOfItem != 0)
    {

        temp = rootSend.index;
        rootSend.index = rootSend.index->next;

        strncpy(rootSend.lastUnique, temp->Unique, strlen(temp->Unique));
        rootSend.package = temp->package;

        rootSend.numberOfItem--;
    }

    return temp;
}

/**
 * @description: 从头节点获取第一个数据并从链表中删除
 * @param:
 * @return:
 */
ocpp_list_RecvItem_t *ocpp_list_readAndDeleteFirstRecvItem()
{

    ocpp_list_RecvItem_t *temp = NULL;
    pthread_rwlock_wrlock(&ocpp_list_rwlock);
    if (rootRecv.numberOfItem != 0)
    {
        temp = rootRecv.index;
        rootRecv.index = rootRecv.index->next;

        if (rootRecv.index != NULL)
        {
            strncpy(rootRecv.unique, rootRecv.index->unique, strlen(rootRecv.index->unique));
            rootRecv.package = rootRecv.index->package;
            rootRecv.isOther = rootRecv.index->isOther;
        }
        else
        {
            memset(rootRecv.unique, 0, 40);
            rootRecv.package = OCPP_PACKAGE_NONE;
            rootRecv.isOther = false;
        }
        rootRecv.numberOfItem--;
    }
    pthread_rwlock_unlock(&ocpp_list_rwlock);

    return temp;
}

/**
 * @description: 判断是否接收到数据
 * @param:
 * @return:
 */
bool ocpp_list_isRecv()
{
    int cnt = 0;
    pthread_rwlock_rdlock(&ocpp_list_rwlock);
    cnt = rootRecv.numberOfItem;
    pthread_rwlock_unlock(&ocpp_list_rwlock);

    if (cnt > 0)
        return true;
    else
        return false;
}

/**
 * @description: 释放节点内存
 * @param:
 * @return:
 */
static void ocpp_list_FreeSendItem(ocpp_list_SendItem_t *item)
{
    if (item == NULL)
        return;
    free(item->message);
    free(item->Unique);
    free(item);
}
void ocpp_list_FreeRecvItem(ocpp_list_RecvItem_t *item)
{
    if (item == NULL)
        return;
    free(item->message);
    free(item->unique);
    free(item);
}

/**
 * @description: 启动定时器
 * @param:
 * @return:
 */
static void ocpp_list_start_timer()
{

    struct itimerspec ts;
    // 第一次调用后，每次调用间隔时间，若不需要周期性启动定时器可将下面两个参数设置值为0
    ts.it_interval.tv_sec = OCPP_LIST_TIMER_TIME_S;
    ts.it_interval.tv_nsec = 0;

    // 第一次调用时间，即程序跑到这多久后启动定时器
    ts.it_value.tv_sec = OCPP_LIST_TIMER_TIME_S;
    ts.it_value.tv_nsec = 0;

    if (timer_settime(timerid, 0, &ts, NULL) == -1)
        printf("启动定时器失败\n");
}

/**
 * @description: 停止定时器
 * @param:
 * @return:
 */
static void ocpp_list_stop_timer()
{
    struct itimerspec ts;
    // 第一次调用后，每次调用间隔时间，若不需要周期性启动定时器可将下面两个参数设置值为0
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 0;

    // 第一次调用时间，即程序跑到这多久后启动定时器
    ts.it_value.tv_sec = 0;
    ts.it_value.tv_nsec = 0;

    if (timer_settime(timerid, 0, &ts, NULL) == -1)
        printf("停止定时器失败\n");
}

/**
 * @description: 删除定时器
 * @param:
 * @return:
 */
static void ocpp_list_delete_timer()
{

    timer_delete(timerid);
}

/**
 * @description: 定时器超时回调函数
 * @param:
 * @return:
 */
static void ocpp_list_timer_CallBack(union sigval value)
{
    printf("timer timeout\n");
    pthread_rwlock_wrlock(&ocpp_list_rwlock);

    // 插入一个错误节点
    ocpp_list_RecvItem_t *item = calloc(1, sizeof(ocpp_list_RecvItem_t));
    item->package = rootSend.package;
    item->unique = strndup(rootSend.lastUnique, strlen(rootSend.lastUnique));
    item->isOther = false;
    item->message = strdup("ERROR");
    item->next = NULL;
    OCPP_LOG_DEBUG("[UniqueId] = %s \t [action] = %s \t [message] = %s", item->unique, ocpp_package_text[item->package], item->message);
    ocpp_list_stop_timer();

    rootSend.isTimeout = true;

    pthread_rwlock_unlock(&ocpp_list_rwlock);
    ocpp_list_insertRecvItem(item);
}

/**
 * @description: 定时器初始化
 * @param:
 * @return:
 */
static void ocpp_list_init_timer()
{
    struct sigevent evp;
    evp.sigev_notify = SIGEV_THREAD;
    evp.sigev_notify_function = ocpp_list_timer_CallBack;
    evp.sigev_notify_attributes = NULL;

    if (timer_create(CLOCK_MONOTONIC, &evp, &timerid) == -1)
    {
        printf("创建定时器失败 %s\n", strerror(errno));
        return;
    }
}

/**
 * @description: 接收 (根据线程ID将数据分发给每个线程)
 * @param:
 * @return:
 */
static void ocpp_list_rec(const char *const message, int len)
{

    enum json_tokener_error err;
    json_object *jobj = NULL;

    char *data = (char *)message;
    jobj = json_tokener_parse_verbose((char *)data, &err);
    if (jobj == NULL)
        return;

    json_object *MessageTypeId_obj = NULL;
    json_object *UniqueId_obj = NULL;

    MessageTypeId_obj = json_object_array_get_idx(jobj, 0);
    UniqueId_obj = json_object_array_get_idx(jobj, 1);

    if (MessageTypeId_obj == NULL || UniqueId_obj == NULL)
        return;

    int messageTypeId_int = json_object_get_int(MessageTypeId_obj);
    const char *uniqueId_str = json_object_get_string(UniqueId_obj);

    ocpp_list_RecvItem_t *item = (ocpp_list_RecvItem_t *)calloc(1, sizeof(ocpp_list_RecvItem_t));
    item->next = NULL;
    item->unique = strdup(uniqueId_str);
    item->message = strndup(message, len);
    item->package = OCPP_PACKAGE_NONE;
    item->isOther = true;

    // 获取上一个发送的消息ID 、线程ID
    char lastUnqueId[40] = {0};
    enum OCPP_PACKAGE lastAction = OCPP_PACKAGE_NONE;
    pthread_rwlock_rdlock(&ocpp_list_rwlock);
    strncpy(lastUnqueId, rootSend.lastUnique, 40);
    lastAction = rootSend.package;

    // DEBUG_INFO("上一个发送消息的 UID = %s",lastUnqueId);

    // CALLResult or CALLError
    if (messageTypeId_int == OCPP_PACKAGE_CALL_RESULT || messageTypeId_int == OCPP_PACKAGE_CALL_ERROR)
    {

        //        OCPP_LOG_INFO("RECV_UID = %s       SEND_UID = %s\n",uniqueId_str,lastUnqueId);
        if (strcmp(uniqueId_str, lastUnqueId) == 0)
        {
            //            printf("相等\n");
            item->package = lastAction;
            item->isOther = false;
            rootSend.isRec = true;
            ocpp_list_stop_timer(); // 接收到服务器应答消息,停止计时
        }
        else
        {
            //            printf("不相等\n");
        }
    }

    pthread_rwlock_unlock(&ocpp_list_rwlock);

    //    OCPP_LOG_DEBUG("将接收到的插入队列");
    //    OCPP_LOG_DEBUG("[UniqueId] = %s \t [action] = %s \t [message] = %s", lastUnqueId,ocpp_package_text[item->package], item->message);
    ocpp_list_insertRecvItem(item);
    // show();
}

/**
 * @description: 发送
 * @param:
 * @return:
 */
static void ocpp_list_send(ocpp_connect_t *connect)
{

    // 连接断开时该如何处理(连接断开时,会每5s丢弃一条数据)--超时时间为5s

    pthread_rwlock_wrlock(&ocpp_list_rwlock);
    if (rootSend.numberOfItem > 0)
    {
        if (rootSend.isTimeout || rootSend.isRec)
        {
            //           OCPP_LOG_DEBUG("已接收到回复或超时，发送新数据 isTimeout = %d  isRec = %d",rootSend.isTimeout, rootSend.isRec);
            ocpp_list_SendItem_t *item = ocpp_list_readAndDeleteFirstSendItem();

            // DEBUG_MSG("len = %d",strlen(item->message));
            // DEBUG_MSG("message = %s", item->message);
            connect->send(item->message, strlen(item->message));
            ocpp_list_FreeSendItem(item);
            // 开始接收计时
            ocpp_list_start_timer();
            rootSend.isRec = false;
            rootSend.isTimeout = false;
        }
    }

    // CallError or Call
    if (rootSend.package == OCPP_PACKAGE_CALLERROR || rootSend.package == OCPP_PACKAGE_CALL)
    {
        if (ocpp_connect_isSendFinish())
        {
            ocpp_list_stop_timer();
            rootSend.isRec = true;
        }
    }

    pthread_rwlock_unlock(&ocpp_list_rwlock);
}

/**
 * @description: 链表控制线程
 * @param:
 * @return:
 */
static void *pthread_ocpp_list_control(void *arg)
{
    OCPP_LOG_DEBUG("创建链表控制线程");
    ocpp_connect_t *connect = (ocpp_connect_t *)arg;

    while (1)
    {

        // show();
        ocpp_list_send(connect);

        usleep(100 * 1000); // 100ms
    }
}

/**
 * @description:
 * @param:
 * @return:
 */
void ocpp_list_init(ocpp_connect_t *connect)
{
    OCPP_LOG_DEBUG("初始化LIST");

    pthread_rwlock_init(&ocpp_list_rwlock, NULL);
    ocpp_list_initSendRecv_Queue();
    ocpp_list_init_timer();
    connect->receive = ocpp_list_rec;

    pthread_t ptid_control;
    if (pthread_create(&ptid_control, NULL, pthread_ocpp_list_control, (void *)connect) != 0)
    {
        printf("cann't create list thread %s\n", strerror(errno));
    }
}

///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
// void * test_send_pthread(void * arg){

// }

// int main(void){

//     printf("in main\n");
//     ocpp_connect_t connect;
//     ocpp_list_init(&connect);

//     ocpp_list_init_timer();
//     ocpp_list_start_timer();
//     printf(">>>启动定时器\n");
//     usleep(3 * 1000 * 1000);
//     printf(">>>>>>>>>>>>>停止定时器\n");
//     ocpp_list_stop_timer();
//     usleep(5 * 1000 * 1000);
//     printf(">>>>>>>>>>>>>启动定时器\n");
//     ocpp_list_start_timer();
//     int i = 0;
//     while(1){
// i++;
// if(i == 10)ocpp_list_stop_timer();
// usleep(1 * 1000 * 1000);
// }
// printf("init quene\n");
// ocpp_list_SendItem_t *send0 = (ocpp_list_SendItem_t *)calloc(1, sizeof(ocpp_list_SendItem_t));
// ocpp_list_SendItem_t *send1 = (ocpp_list_SendItem_t *)calloc(1, sizeof(ocpp_list_SendItem_t));
// ocpp_list_SendItem_t *send2 = (ocpp_list_SendItem_t *)calloc(1, sizeof(ocpp_list_SendItem_t));
// ocpp_list_SendItem_t *send3 = (ocpp_list_SendItem_t *)calloc(1, sizeof(ocpp_list_SendItem_t));
// ocpp_list_SendItem_t *send4 = (ocpp_list_SendItem_t *)calloc(1, sizeof(ocpp_list_SendItem_t));

// printf("1\n");
// ocpp_list_insertSendItem(send0);
// printf("2\n");
// ocpp_list_insertSendItem(send1);
// ocpp_list_insertSendItem(send2);
// ocpp_list_insertSendItem(send3);
// ocpp_list_insertSendItem(send4);
// printf("\n\n>>>>>>>>show list\n");
// show();

// printf("delete one item\n");
// ocpp_list_readAndDeleteFirstSendItem();
// ocpp_list_readAndDeleteFirstSendItem();
// ocpp_list_readAndDeleteFirstSendItem();
// show();

// ocpp_list_RecvItem_t recv0;
// ocpp_list_RecvItem_t recv1;
// ocpp_list_RecvItem_t recv2;
// ocpp_list_RecvItem_t recv3;
// ocpp_list_RecvItem_t recv4;

// ocpp_list_insertRecvItem(&recv0);
// ocpp_list_insertRecvItem(&recv1);
// ocpp_list_insertRecvItem(&recv2);
// ocpp_list_insertRecvItem(&recv3);
// ocpp_list_insertRecvItem(&recv4);
// show();
// ocpp_list_readAndDeleteFirstRecvItem();
// ocpp_list_readAndDeleteFirstRecvItem();
// show();

//     while (1){}

// }
