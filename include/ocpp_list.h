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
//////////////////////////////////////-发送-/////////////////////////////////////
//链表节点
typedef struct OCPP_LIST_SEND_ITEM_S{
    struct OCPP_LIST_SEND_ITEM_S * next;               //指向下一个节点
    char *                         Unique;             //发送的消息的唯一ID   
    char *                         message;            //指向代发送的消息
    enum OCPP_PACKAGE              package;            //发送数据的Action
}ocpp_list_SendItem_t;



//链表头节点
typedef struct{

    unsigned int           numberOfItem;          //发送节点数目
    bool                   isTimeout;             //发送消息计时器(指示多久没有接收到回复后超时)
    bool                   isRec;                 //是否接收到回复
    char *                 lastUnique;            //发送的消息的唯一ID
    enum OCPP_PACKAGE      package;               //发送数据的Action
    ocpp_list_SendItem_t * index;                 //链表节点索引指针

}ocpp_list_root_SendItem_t;



//////////////////////////////////////-接收-/////////////////////////////////////
typedef struct OCPP_LIST_RECV_ITEM_S{

    char *                         unique;         //接收到的报文ID
    bool                           isOther;        //接收到消息ID不匹配小消息,eg:ocpp call消息
    char *                         message;        //接收到的数据
    enum OCPP_PACKAGE              package;        //接收数据的Action
    struct OCPP_LIST_RECV_ITEM_S * next;           //指向下一个节点

}ocpp_list_RecvItem_t;




typedef struct{
    unsigned int                   numberOfItem;         //接收到的报文数目
    bool                           isOther;              //接收到消息ID不匹配小消息,eg:ocpp call消息
    char *                         unique;               //接收到的报文ID 
    enum OCPP_PACKAGE              package;              //接收到回复数据的Action
    ocpp_list_RecvItem_t *         index;                //指向接收到的数据               

}ocpp_list_root_RecvItem_t;



void ocpp_list_insertSendItem(ocpp_list_SendItem_t * item);
ocpp_list_RecvItem_t * ocpp_list_readAndDeleteFirstRecvItem();
bool ocpp_list_isRecv();
void ocpp_list_init(ocpp_connect_t * connect);
void ocpp_list_FreeRecvItem(ocpp_list_RecvItem_t * item);



#ifdef __cplusplus
}
#endif

#endif