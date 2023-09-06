
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <json-c/json.h>
#include <unistd.h>
#include "ocpp_list.h"

#define OCPP_LIST_TIMER_TIME_S 3 // 定时器时间

SendMessageQueue sendQueue; // 发送队列
RecvMessageQueue recvQueue; // 接收队列
/**
 * @description: 发送信息
 * @param message需要发送的信息
 * @param len发送信息的长度
 */
void ocpp_message_rec(void *message, int len)
{
    if (message == NULL)
    {
        return;
    }
    enum json_tokener_error err;
    json_object *jobj = NULL;

    char *data = (char *)message;
    printf(GREEN_COLOR "[RECV] = %s\n\n" RESET_COLOR, (char *)data);
    jobj = json_tokener_parse_verbose((char *)data, &err);
    if (jobj == NULL) // 如果jobj为NULL，就直接返回
    {
        return;
    }

    json_object *MessageTypeId_obj = NULL;
    json_object *UniqueId_obj = NULL;
    MessageTypeId_obj = json_object_array_get_idx(jobj, 0);
    UniqueId_obj = json_object_array_get_idx(jobj, 1);

    if (MessageTypeId_obj == NULL || UniqueId_obj == NULL)
    {
        return;
    }

    int messageTypeId_int = json_object_get_int(MessageTypeId_obj);
    const char *uniqueId_str = json_object_get_string(UniqueId_obj);

    int Action = 0;
    if (messageTypeId_int == OCPP_PACKAGE_CALL_RESULT || messageTypeId_int == OCPP_PACKAGE_CALL_ERROR)
    {
        Action = getActionByUnique(uniqueId_str);
        if (Action != -1)
        {
            enqueueRecvMessage(uniqueId_str, message, messageTypeId_int, Action); // 信息插入到信息列表
            updateSendMessageStatus(uniqueId_str, 1);                             // 改变列表信息状态为已回复
        }
        else
        {
            enqueueRecvMessage(uniqueId_str, message, messageTypeId_int, sendQueue.lastaction); // 信息插入到信息列表
        }
    }
    else
    {
        enqueueRecvMessage(uniqueId_str, message, messageTypeId_int, Action); // 信息插入到信息列表
    }

    json_object_put(jobj); // 释放jobj指向的堆内存
}

/**
 * @description: 判断是否超时
 * @param startTime判断的时间
 */
static bool isTimeout(time_t startTime)
{
    time_t currentTime = time(NULL);
    return (currentTime - startTime >= MAX_TIMEOUT_SECONDS);
}
/**
 * @description: 初始化发送信息的队列
 */
void initSendMessageQueue()
{
    sendQueue.head = NULL;
    sendQueue.tail = NULL;
    sendQueue.nodeCount = 0;
    pthread_mutex_init(&sendQueue.mutex, NULL);
    pthread_cond_init(&sendQueue.cond, NULL);
}
/**
 * @description: 插入信息到队列
 * @param Unique 信息唯一标识符
 * @param message 发送信息
 * @param action 信息类型
 */
void enqueueSendMessage(const char *Unique, const char *message, int action)
{
    if (Unique == NULL || message == NULL || sendQueue.nodeCount > 10)
    {
        fprintf(stderr, "Invalid input: Unique or message is NULL\n");
        return; // 参数为NULL，返回避免继续执行
    }

    SendMessage *newMessage = (SendMessage *)malloc(sizeof(SendMessage));
    if (!newMessage)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return; // 内存分配失败，返回避免继续执行
    }

    strncpy(newMessage->Unique, Unique, sizeof(newMessage->Unique) - 1);
    strncpy(newMessage->message, message, sizeof(newMessage->message) - 1);
    newMessage->action = action;
    newMessage->status = 0; // 0 表示未回复
    newMessage->next = NULL;

    pthread_mutex_lock(&sendQueue.mutex);

    if (sendQueue.tail == NULL)
    {
        sendQueue.head = newMessage;
        sendQueue.tail = newMessage;
    }
    else
    {
        sendQueue.tail->next = newMessage;
        sendQueue.tail = newMessage;
    }
    sendQueue.nodeCount ++;
    // 唤醒等待的线程
    pthread_cond_signal(&sendQueue.cond);

    pthread_mutex_unlock(&sendQueue.mutex);
}
/**
 * @description: 读取队列最新的信息
 * @param Unique  读取到的Unique
 * @param message 读取到的message
 * @param action  读取到的action
 * @param status  读取到的姿态0-未发送，1-已发送，2-已超时
 */
bool dequeueSendMessage(char *Unique, char *message, int *action, int *status)
{
    if (Unique == NULL || message == NULL || status == NULL) 
    {
        fprintf(stderr, "Invalid input: Unique, message, status, or startTime is NULL\n");
        return false; // 不处理无效输入
    }
    pthread_mutex_lock(&sendQueue.mutex);

    while (sendQueue.head == NULL)
    {
        pthread_cond_wait(&sendQueue.cond, &sendQueue.mutex);
    }

    SendMessage *messageNode = sendQueue.head;
    strncpy(Unique, messageNode->Unique, sizeof(messageNode->Unique) - 1);
    strncpy(message, messageNode->message, sizeof(messageNode->message) - 1);
    *status = messageNode->status;
    *action = messageNode->action;

    pthread_mutex_unlock(&sendQueue.mutex);
    return true;
}
/**
 * @description: 根据Unique返回他的类型
 * @param Unique  Unique
 *  @return 失败 -1 ，成功返回 信息类型action
 */
int getActionByUnique(const char *Unique)
{
    if (Unique == NULL)
    {
        return -1; // 或者您可以选择其他适当的错误码
    }

    pthread_mutex_lock(&sendQueue.mutex);

    SendMessage *current = sendQueue.head;

    while (current != NULL)
    {
        if (strcmp(current->Unique, Unique) == 0)
        {
            int action = current->action;
            pthread_mutex_unlock(&sendQueue.mutex);
            return action;
        }

        current = current->next;
    }

    pthread_mutex_unlock(&sendQueue.mutex);
    return -1; // 未找到对应的 Unique
}
/**
 * @description: 根据Unique获得信息状态
 * @param status 读取到的姿态0-未发送，1-已发送，2-已超时
 */
void updateSendMessageStatus(const char *Unique, int status)
{
    if (Unique == NULL)
    {
        return;
    }

    pthread_mutex_lock(&sendQueue.mutex);

    SendMessage *current = sendQueue.head;

    while (current != NULL)
    {
        if (strcmp(current->Unique, Unique) == 0)
        {
            current->status = status;
            break;
        }

        current = current->next;
    }

    pthread_mutex_unlock(&sendQueue.mutex);
}
/**
 * @description: 根据Unique删除信息
 * @param Unique Unique
 */
static void deleteSendMessage(const char *Unique)
{
    if (Unique == NULL)
    {
        return;
    }
    pthread_mutex_lock(&sendQueue.mutex);

    SendMessage *current = sendQueue.head;
    SendMessage *prev = NULL;

    while (current != NULL)
    {
        if (strcmp(current->Unique, Unique) == 0)
        {
            // 找到匹配的消息节点
            if (prev != NULL)
            {
                prev->next = current->next;
            }
            else
            {
                sendQueue.head = current->next;
            }

            if (current == sendQueue.tail)
            {
                sendQueue.tail = prev;
            }
            sendQueue.nodeCount --;
            free(current);
            break;
        }

        prev = current;
        current = current->next;
    }
    
    // 检查是否还有其他消息
    if (sendQueue.head != NULL)
    {
        // 发出条件变量信号通知其他线程
        pthread_cond_signal(&sendQueue.cond);
    }

    pthread_mutex_unlock(&sendQueue.mutex);
}
/**
 * @description: 信息发送控制线程
 */
void *sendMessageToServer(void *arg)
{
    ocpp_connect_t *connect = (ocpp_connect_t *)arg;
    char Unique[MESSAGE_UNIQUE_LEN];
    char message[MESSAGE_CONTENT_LEN];
    int status;
    int action;
    time_t sendTime;
    bool messageSent = false; // 标志变量，追踪消息是否已发送
    while (1)
    {
        if (dequeueSendMessage(Unique, message, &action, &status))
        {
            // 发送消息只有在消息未发送过或状态发生变化时执行
            if (!messageSent && message != NULL)
            {
                sendQueue.lastaction = action;
                printf(YELLOW_COLOR "[SEND] = %s\n\n" RESET_COLOR, (char *)message);
                connect->send(message, strlen(message));
                messageSent = true;
                // 获取消息发送前的时间
                sendTime = time(NULL);
            }

            if (isTimeout(sendTime) && messageSent)
            {
                printf(RED_COLOR "Timeout for Unique: %s\n\n" RESET_COLOR, Unique);
                if (status != 1)
                {
                    // 只有当状态不是已回复时才更新为超时
                    status = 2; // 2 表示超时
                }
            }

            if (status == 1 || status == 2 || !connect->isConnect)
            {
                deleteSendMessage(Unique);
                messageSent = false;
                status = 0;
            }

            memset(message, 0, MESSAGE_CONTENT_LEN);
            memset(Unique, 0, MESSAGE_UNIQUE_LEN);
        }
        sleep(1); // 等待一段时间后重试
    }

    return NULL;
}
/**
 * @description: 初始化发送信息的队列
 */
void initRecvMessageQueue()
{
    recvQueue.head = NULL;
    recvQueue.tail = NULL;
    pthread_mutex_init(&recvQueue.mutex, NULL);
    recvQueue.nodeCount = 0;
}
/**
 * @description: 插入信息到接收队列
 * @param Unique 信息唯一标识符
 * @param message 发送信息
 * @param action 信息类型
 */
void enqueueRecvMessage(const char *Unique, const char *message, int messageTypeId, int action)
{
    if (Unique == NULL || message == NULL)
    {
        return;
    }
    RecvMessage *newMessage = (RecvMessage *)malloc(sizeof(RecvMessage));
    if (newMessage == NULL)
    {
        perror("Failed to allocate memory for new message");
        return;
    }

    strncpy(newMessage->Unique, Unique, MESSAGE_UNIQUE_LEN);
    strncpy(newMessage->message, message, MESSAGE_CONTENT_LEN);
    newMessage->messageTypeId = messageTypeId;
    newMessage->action = action; // 添加 action 字段
    newMessage->next = NULL;

    pthread_mutex_lock(&recvQueue.mutex);
    if (recvQueue.tail == NULL)
    {
        recvQueue.head = newMessage;
        recvQueue.tail = newMessage;
    }
    else
    {
        recvQueue.tail->next = newMessage;
        recvQueue.tail = newMessage;
    }
    recvQueue.nodeCount++;

    pthread_mutex_unlock(&recvQueue.mutex);
}
/**
 * @description: 读取队列最新的信息
 * @param Unique  读取到的Unique
 * @param message 读取到的message
 * @param action  读取到的action
 * @ return  成功返回0，失败返回-1
 */
bool dequeueRecvMessage(char *Unique, char *message, int *messageTypeId, int *action)
{
    if (Unique == NULL || message == NULL || messageTypeId == NULL || action == NULL)
    {
        return false;
    }
    pthread_mutex_lock(&recvQueue.mutex);
    if (recvQueue.nodeCount == 0)
    {
        pthread_mutex_unlock(&recvQueue.mutex);
        return false;
    }

    RecvMessage *oldestMessage = recvQueue.head;
    recvQueue.head = oldestMessage->next;
    recvQueue.nodeCount--;

    if (recvQueue.nodeCount == 0)
    {
        recvQueue.tail = NULL;
    }

    pthread_mutex_unlock(&recvQueue.mutex);

    strncpy(Unique, oldestMessage->Unique, MESSAGE_UNIQUE_LEN);
    strncpy(message, oldestMessage->message, MESSAGE_CONTENT_LEN);
    *messageTypeId = oldestMessage->messageTypeId;
    *action = oldestMessage->action; // 获取 action 字段
    free(oldestMessage);

    return true;
}
/**
 * @description: 判断是否有未读取信息
 */
bool IsRecvMessage()
{
    int cnt = 0;
    pthread_mutex_lock(&recvQueue.mutex);
    cnt = recvQueue.nodeCount;
    pthread_mutex_unlock(&recvQueue.mutex);

    if (cnt > 0)
        return true;
    else
        return false;
}

/**
 * @description:
 * @param:
 * @return:
 */
void ocpp_list_init(ocpp_connect_t *connect)
{
    connect->receive = ocpp_message_rec;
    initSendMessageQueue();
    initRecvMessageQueue();
    // 创建线程用于发送消息给服务器
    pthread_t serverThread;
    if (pthread_create(&serverThread, NULL, sendMessageToServer, (void *)connect) != 0)
    {
        printf("cann't create list thread %s\n", strerror(errno));
    }
}