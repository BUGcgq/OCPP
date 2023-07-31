#include <libwebsockets.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include "ocpp_log.h"
#include "ocpp_connect.h"


// lws_service(context, 0);           https://libwebsockets.org/lws-api-doc-main/html/group__service.html#gaf95bd0c663d6516a0c80047d9b1167a8
// lws_write()                        https://libwebsockets.org/lws-api-doc-main/html/lws-write_8h.html#afd5fdd285a0e25ba7e3e1051deec1001
// struct lws_client_connect_info     https://libwebsockets.org/lws-api-doc-main/html/group__client.html#structlws__client__connect__info
// struct lws_protocols               https://libwebsockets.org/lws-api-doc-main/html/group__Protocols-and-Plugins.html#structlws__protocols
// struct lws_context_creation_info   https://libwebsockets.org/lws-api-doc-main/html/structlws__context__creation__info.html



#define OCPP_CONNECT_SEND_BUFFER                2048                       //发送缓存区大小

// 每个使用这个协议的新连接在建立连接时都会分配这么多内存，在断开连接时会释放这么多内存。指向此按连接分配的指针被传递到user参数中的回调中
typedef struct {
    int fd;
	struct lws_context * context;
	ocpp_connect_t * connect;
	struct lws_protocols * protocol;

	size_t  sendDataLen;                                      //发送数据大小
	char sendbuff[OCPP_CONNECT_SEND_BUFFER];                  //发送存储区
	pthread_mutex_t buffLock;                                 //缓存区锁
}ocpp_connect_session_data_t;


static ocpp_connect_session_data_t session_data;

/**
 * @description: 建立连接
 * @param:
 * @return: 
 */
static int ocpp_connect_establish(){
	
	struct lws_client_connect_info connectInfo;                         
    memset(&connectInfo, 0, sizeof(struct lws_client_connect_info));
	connectInfo.port = session_data.connect->port;
	connectInfo.address = session_data.connect->address;
	connectInfo.path = session_data.connect->path;
	connectInfo.context = session_data.context;
	if(session_data.connect->isWss)
		connectInfo.ssl_connection = LCCSCF_USE_SSL | LCCSCF_ALLOW_EXPIRED;
	else
        connectInfo.ssl_connection = 0;
	
	connectInfo.host = connectInfo.address;
	connectInfo.origin = connectInfo.address;
	connectInfo.ietf_version_or_minus_one = -1;
	connectInfo.client_exts = NULL;	
	connectInfo.protocol = session_data.protocol->name;

	lwsl_notice("%s ocpp connect address = %s  port = %d\n",__func__,connectInfo.address,connectInfo.port);
	lwsl_notice("%s ocpp connect path = %s isWss = %d protocolName = %s\n",__func__,connectInfo.path,session_data.connect->isWss,connectInfo.protocol);

	if(connectInfo.context == NULL)lwsl_notice("ocpp connect context NULL\n");
	if (!lws_client_connect_via_info(&connectInfo)) return 1;

	return 0;

}

/**
 * @description: 
 * @param:
 * @return: 
 */
static int ocpp_connect_websocket_send_back(struct lws *wsi_in, char *str, int str_size_in){
	printf(YELLOW"[SEND] = %s\n\n"RESET,str);
//	OCPP_LOG_INFO("[SEND] = %s", str);
    int n = 0;
    int len = 0;
    char *out = NULL;

    if (str_size_in < 1)
        len = strlen(str);
    else
        len = str_size_in;

    //为加密的字符串保留内存
    out = (char *)calloc(1, sizeof(char)*(LWS_SEND_BUFFER_PRE_PADDING + len + LWS_SEND_BUFFER_POST_PADDING));

    memcpy (out + LWS_SEND_BUFFER_PRE_PADDING, str, len );
    n = lws_write(wsi_in, out + LWS_SEND_BUFFER_PRE_PADDING, len, LWS_WRITE_TEXT);

    if (out) free(out);
    return n;
}


/**
 * @description: 回调函数
 * @param:
 * @return: 
 */
static int ocpp_connect_service_callback(struct lws *wsi,enum lws_callback_reasons reason, void *user,void *in, size_t len){

	// printf("       reason = %d\n",reason);
	switch(reason){
		case LWS_CALLBACK_PROTOCOL_INIT:
			lwsl_notice("protocol init\n");
			ocpp_connect_establish();
			lwsl_notice("protocol init end\n");
		
		break;

		case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
			lwsl_err("CLIENT_CONNECTION_ERROR: %s\n", in ? (char *)in :"(null)");
			session_data.connect->interrupted = true;
		break;

		case LWS_CALLBACK_CLIENT_ESTABLISHED:
			lwsl_notice("%s: established connection, wsi = %p\n",__func__, wsi);
			session_data.connect->isConnect = true;
			lws_callback_on_writable(wsi);
			
		break;

		case LWS_CALLBACK_CLIENT_CLOSED:
			lwsl_notice("%s: CLOSED: %s\n", __func__,in ? (char *)in :"(null)");
			session_data.connect->interrupted = true;
		break;

		case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
			{
				lwsl_notice("start Authorization\n");
				char * username = session_data.connect->username;
				char * password = session_data.connect->password;
				char * authorization = (char *)calloc(1,256);
				char * user_pwd = (char *)calloc(1,256);
				
				sprintf(user_pwd,"%s:%s",username,password);
				lws_b64_encode_string(user_pwd,strlen(user_pwd),authorization,256);
				char * basic = (char *)calloc(1,256);
				sprintf(basic,"Basic %s",authorization);
				int value_len = strlen(basic);
				unsigned char **p, *end;
				p = (unsigned char **)in;
				end = (*p) + len;
				lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_AUTHORIZATION, basic, value_len, p, end);

				free(authorization);
				free(user_pwd);
				free(basic);
				lwsl_notice("start Authorization end\n");
			}
			break;

		case LWS_CALLBACK_TIMER:
			// lwsl_notice("%s: timer >>>>>>>>>>>>>>>>>>>\n",__func__);
			// Let the main loop know we want to send another message to the server
			lws_callback_on_writable(wsi);

		break;

		case LWS_CALLBACK_CLIENT_WRITEABLE:
			// lwsl_notice("%s: Write >>>>>>>>>>>>>>>>>>>\n",__func__);
			lws_set_timer_usecs(wsi, 50*1000);          //50ms

			pthread_mutex_lock(&session_data.buffLock);
			if(session_data.sendDataLen > 0){

				if(ocpp_connect_websocket_send_back(wsi, session_data.sendbuff, session_data.sendDataLen) < session_data.sendDataLen)  return -1;
				session_data.sendDataLen = 0;
				session_data.connect->isSendFinish = true;
				
			}
			pthread_mutex_unlock(&session_data.buffLock);
			


		break;

		case LWS_CALLBACK_CLIENT_RECEIVE:
			printf(RED"[RECV] = %s\n\n"RESET,(char *)in);
//			OCPP_LOG_INFO("[RECV] = %s",(char *)in);
			session_data.connect->receive((char *)in, len);
			
		break;

		default:
			break;
	}
	// DEBUG_MSG("out");
	return lws_callback_http_dummy(wsi, reason, user, in, len);
}






/**
 * @description: 初始化上下文环境
 * @param:
 * @return: 
 */
static void ocpp_connect_init_lws_context_creation_info(struct lws_context_creation_info * const info, struct lws_protocols * const protocol, ocpp_connect_t * const connect){
    memset(info, 0, sizeof(struct lws_context_creation_info));
	info->port = CONTEXT_PORT_NO_LISTEN;                              //创建客户端,不监听端口
    info->iface = NULL;
	info->protocols = protocol;
    info->gid = -1;
	info->uid = -1;
//	info->keepalive_ping_interval = 5;     // 设置 ping 间隔为 5 秒
    info->keepalive_timeout = 10;          // 设置 ping 超时时间为 10 秒

	
	info->options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;              //初始化SSL库

	info->client_ssl_private_key_password = NULL;                      //私钥
	info->client_ssl_cert_filepath = NULL;                             //客户端的证书
	info->client_ssl_cert_mem = NULL;                                  //从内存而不是文件加载客户端证书
	info->client_ssl_cert_mem_len = 0;                                 //长度字节
	info->client_ssl_private_key_filepath = connect->ssl_private_key_filepath;                      //私钥路径
	info->client_ssl_key_mem = NULL;                                   //加载客户端密钥从内存而不是文件
	info->client_ssl_key_mem_len = 0;                                  //
	info->client_ssl_ca_filepath = connect->ssl_ca_filepath;           //CA证书文件路径或NULL
	OCPP_LOG_DEBUG("info->client_ssl_ca_filepath = %s",info->client_ssl_ca_filepath);
	OCPP_LOG_DEBUG("info->ssl_private_key_filepath = %s",info->client_ssl_private_key_filepath);
	info->client_ssl_ca_mem = NULL;                                    //从内存加载CA证书，而不是文件
	info->client_ssl_ca_mem_len = 0;                                   //
	info->client_ssl_cipher_list = "AES256-SHA:RSA";//NULL;                               //支持的加密套件,用于在会话中加密
}



/**
 * @description: 
 * @param:
 * @return: 
 */
static void ocpp_connect_init_lws_protocols(struct lws_protocols * protocol,ocpp_connect_t * const connect){

    protocol->name = connect->protocolName;
	OCPP_LOG_DEBUG("protocol->name = %s",protocol->name);
	protocol->callback = &ocpp_connect_service_callback;                                      
	protocol->per_session_data_size = 0;
	protocol->rx_buffer_size = 0;
	protocol->id = 0;

}


/**
 * @description: 建立websock连接
 * @param:
 * @return: 
 */
static void ocpp_connect_send(const char * const message, size_t len){
	
	pthread_mutex_lock(&session_data.buffLock);
	strncpy(session_data.sendbuff, message, OCPP_CONNECT_SEND_BUFFER);
	session_data.sendDataLen = len;
	pthread_mutex_unlock(&session_data.buffLock);

	return;
}





/**
 * @description: 建立websock连接
 * @param:
 * @return: 
 */
static void * ocpp_connect_websocket(ocpp_connect_t * const connect){
	OCPP_LOG_DEBUG("创建websock连接线程");
	// printf("%s = %p\n",__func__,arg);
	// ocpp_connect_t * connect = (ocpp_connect_t *)arg;
	// printf("%s = %p\n",__func__,connect);

	struct lws_context *context = NULL;                           
    struct lws_protocols protocol;                                  
	struct lws_context_creation_info info;                           



	//初始化缓存区
	memset(session_data.sendbuff, 0, OCPP_CONNECT_SEND_BUFFER);
	session_data.sendDataLen = 0;
	pthread_mutex_init(&session_data.buffLock, NULL);

	ocpp_connect_init_lws_protocols(&protocol,connect);
    ocpp_connect_init_lws_context_creation_info(&info,&protocol,connect);
	// printf("%s protocol = %p\n",__func__,&protocol);
Reconnection: 
	OCPP_LOG_DEBUG("context start\n");
    context = lws_create_context(&info);

    if(context == NULL){
        printf("conntext NULL,connect fail\n");
		goto RETRY;
    }


	session_data.context = context;
	session_data.connect = connect;
	session_data.protocol = &protocol;
	OCPP_LOG_DEBUG("start server callback");
	int n = 0;
	while (n >=0 && connect->interrupted == false)
		n = lws_service(context, 0);

	lwsl_notice("%s: exiting service , interrupted = %d\n",__func__, connect->interrupted);

	lws_context_destroy(context);
	connect->interrupted = false;
	connect->isConnect = false;
RETRY:
	usleep(5 * 1000 * 1000);

goto Reconnection;

}


/**
 * @description: 
 * @return {*}
 */
bool ocpp_connect_isSendFinish(){
	if(session_data.connect == NULL) return false;

	return session_data.connect->isSendFinish;
}


/**
 * @description: 连接初始化
 * @param:
 * @return: 
 */
void ocpp_connect_init(ocpp_connect_t * const connect){
	OCPP_LOG_INFO("连接初始化");
	connect->send = ocpp_connect_send;
	connect->isConnect = false;            //default no connect server
	connect->interrupted = false;          //default no interrupted
	connect->isSendFinish = true;

	pthread_t ptid_connect;
	if(pthread_create(&ptid_connect, NULL, ocpp_connect_websocket, connect) != 0){
		printf("cann't create connect thread %s\n",strerror(errno));
	}

	printf("connect init end\n");
}








/**
 * @description: 建立websock连接
 * @param:
 * @return: 
 */
// int main(void){
// 	printf("in main\n");

// 	ocpp_connect_t connect;

// 	connect.isWss = true;
// 	connect.ssl_ca_filepath = "/app/core/ssl/ca/cacert.pem";
// 	connect.protocolName = "ocpp1.6";
// 	connect.address = "baal.tuyacn.com";
// 	connect.port = 9000;
// 	connect.path = "/h5wa015ajqA2Eg8D";
// 	connect.username = "h5wa015ajqA2Eg8D";
// 	connect.password = "LDhf7qM2Pw5P8p92";
// 	connect.interrupted = true;

// 	ocpp_connect_init(&connect);

// 	while(1){}
// }




















