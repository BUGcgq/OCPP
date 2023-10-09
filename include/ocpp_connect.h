#ifndef __OCPP_CONNECT__H__
#define __OCPP_CONNECT__H__

#ifdef __cplusplus
extern "C" {
#endif

#include<stdbool.h>


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
    bool  interrupted;               //连接是否中断
    bool  isConnect;                 //是否与服务器建立连接
    bool  isSendFinish;              //
    void (*send)(const char * const message, size_t len);
    void (*receive)(void * message, int len);                //需上层实现对接收的处理

}ocpp_connect_t;



void ocpp_connect_init(ocpp_connect_t * const connect);
bool ocpp_connect_isSendFinish();

#ifdef __cplusplus
}
#endif

#endif