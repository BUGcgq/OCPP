#include <libwebsockets.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include "ocpp_connect.h"

int current_reconnect_attempt = 0;
#define OCPP_CONNECT_SEND_BUFFER 2048 // 发送缓存区大小
#define PING_COUNTER_OVER_MAX 200	  // 设置为200以实现每10秒发送一次Ping帧
// 每个使用这个协议的新连接在建立连接时都会分配这么多内存，在断开连接时会释放这么多内存。指向此按连接分配的指针被传递到user参数中的回调中
typedef struct
{
	int fd;
	struct lws_context *context;
	ocpp_connect_t *connect;
	struct lws_protocols *protocol;

	size_t sendDataLen;						 // 发送数据大小
	char sendbuff[OCPP_CONNECT_SEND_BUFFER]; // 发送存储区
	pthread_mutex_t buffLock;				 // 缓存区锁
} ocpp_connect_session_data_t;

static ocpp_connect_session_data_t session_data;

/**
 * @description: 建立连接
 * @param:
 * @return:
 */
static int ocpp_connect_establish()
{

	struct lws_client_connect_info connectInfo;
	memset(&connectInfo, 0, sizeof(struct lws_client_connect_info));
	connectInfo.port = session_data.connect->port;
	connectInfo.address = session_data.connect->address;
	connectInfo.path = session_data.connect->path;
	connectInfo.context = session_data.context;
	if (session_data.connect->isWss)
		connectInfo.ssl_connection = LCCSCF_USE_SSL | LCCSCF_ALLOW_EXPIRED;
	else
		connectInfo.ssl_connection = 0;

	connectInfo.host = connectInfo.address;
	connectInfo.origin = connectInfo.address;
	connectInfo.ietf_version_or_minus_one = -1;
	connectInfo.client_exts = NULL;
	connectInfo.protocol = session_data.protocol->name;

	lwsl_notice("ocpp connect address = %s  port = %d\n",connectInfo.address, connectInfo.port);
	lwsl_notice("ocpp connect path = %s isWss = %d protocolName = %s\n",connectInfo.path, session_data.connect->isWss, connectInfo.protocol);

	if (connectInfo.context == NULL)
		lwsl_notice("ocpp connect context NULL\n");
	if (!lws_client_connect_via_info(&connectInfo))
		return 1;

	return 0;
}

/**
 * @description:
 * @param:
 * @return:
 */
static int ocpp_connect_websocket_send_back(struct lws *wsi_in, char *str, int str_size_in)
{

	if (!wsi_in || !str || str_size_in <= 0)
	{
		fprintf(stderr, "Invalid input parameters or message\n");
		return -1; // 返回错误码表示失败
	}

	int len = (str_size_in < 1) ? strlen(str) : str_size_in;
	char *out = (char *)calloc(1, sizeof(char) * (LWS_SEND_BUFFER_PRE_PADDING + len + LWS_SEND_BUFFER_POST_PADDING));

	if (!out)
	{
		fprintf(stderr, "Memory allocation failed\n");
		return -1; // 返回错误码表示失败
	}

	memcpy(out + LWS_SEND_BUFFER_PRE_PADDING, str, len);
	int n = lws_write(wsi_in, out + LWS_SEND_BUFFER_PRE_PADDING, len, LWS_WRITE_TEXT);

	if (n < 0)
	{
		fprintf(stderr, "Failed to send message\n");
	}

	if (out)
	{
		free(out);
	}

	return n; // 返回发送结果
}

/**
 * @description: 发送ping
 * @param:
 * @return:
 */
static int send_ping(struct lws *wsi)
{
	unsigned char ping_payload[LWS_PRE + 125];
	memset(ping_payload, 0, sizeof(ping_payload));

	int len = lws_write(wsi, &ping_payload[LWS_PRE], 0, LWS_WRITE_PING);
	if (len < 0)
	{
		lwsl_notice("Ping sent ERROR!\n"); // 在发送成功时打印消息
		return -1;						   // 发送失败
	}
	else
	{
		lwsl_notice("Ping sent successfully!\n"); // 在发送成功时打印消息
		return 0;								  // 发送成功
	}
}

/**
 * @description: 回调函数
 * @param:
 * @return:
 */
static int ocpp_connect_service_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{

	// printf("       reason = %d\n",reason);
	static int pingCounter = 0;
	switch (reason)
	{
	case LWS_CALLBACK_PROTOCOL_INIT:
		lwsl_notice("protocol init\n");
		ocpp_connect_establish();
		lwsl_notice("protocol init end\n");

		break;

	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		lwsl_err("CONNECTION ERROR: %s\n", (char *)in);
		session_data.connect->interrupted = true;
		break;

	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		lwsl_notice("CONNECTION ESTABLISHED\n");
		session_data.connect->isConnect = true;
		current_reconnect_attempt = 0;
		lws_callback_on_writable(wsi);

		break;

	case LWS_CALLBACK_CLIENT_CLOSED:
		lwsl_notice("CONNECTION CLOSED\n");
		session_data.connect->interrupted = true;
		break;

	case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
	{
		lwsl_notice("start Authorization\n");
		char *username = session_data.connect->username;
		char *password = session_data.connect->password;
		char *authorization = (char *)calloc(1, 256);
		char *user_pwd = (char *)calloc(1, 256);

		sprintf(user_pwd, "%s:%s", username, password);
		lws_b64_encode_string(user_pwd, strlen(user_pwd), authorization, 256);
		char *basic = (char *)calloc(1, 256);
		sprintf(basic, "Basic %s", authorization);
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

		pingCounter++;

		if (pingCounter >= PING_COUNTER_OVER_MAX)
		{
			pingCounter = 0;

			// 发送ping帧
			if (send_ping(wsi) < 0)
			{
				// ping失败，执行重新连接
				lwsl_notice("ping失败,执行重新连接\n");
				session_data.connect->interrupted = true;
			}
		}

		lws_callback_on_writable(wsi);

		break;

	case LWS_CALLBACK_CLIENT_WRITEABLE:
		pthread_mutex_lock(&session_data.buffLock);
		if (session_data.sendDataLen > 0)
		{
			if (ocpp_connect_websocket_send_back(wsi, session_data.sendbuff, session_data.sendDataLen) < session_data.sendDataLen)
				return -1;
			session_data.sendDataLen = 0;
			session_data.connect->isSendFinish = true;
		}
		pthread_mutex_unlock(&session_data.buffLock);
		lws_set_timer_usecs(wsi, 50 * 1000); // 50ms
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE:
		//			printf("[RECV] = %s",(char *)in);
		session_data.connect->receive(in, len);

		break;

	case LWS_CALLBACK_RECEIVE_PONG:
		lwsl_notice("Received PONG frame\n"); // 在接收到pong帧时打印消息
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
static void ocpp_connect_initialize_websocket_context(struct lws_context_creation_info *const info, struct lws_protocols *const protocol, ocpp_connect_t *const connect)
{
	memset(info, 0, sizeof(struct lws_context_creation_info));
	memset(protocol, 0, sizeof(struct lws_protocols));

	protocol->name = connect->protocolName;
	protocol->callback = &ocpp_connect_service_callback;
	protocol->per_session_data_size = 0;
	protocol->rx_buffer_size = 0;
	protocol->id = 0;
	info->port = CONTEXT_PORT_NO_LISTEN; // 创建客户端,不监听端口
	info->iface = NULL;
	info->protocols = protocol;
	info->gid = -1;
	info->uid = -1;
	// info->keepalive_ping_interval = 5;     // 设置 ping 间隔为 5 秒
	//  info->keepalive_timeout = 10; // 设置 ping 超时时间为 10 秒

	info->options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;					   // 初始化SSL库
	info->client_ssl_private_key_password = NULL;							   // 私钥
	info->client_ssl_cert_filepath = NULL;									   // 客户端的证书
	info->client_ssl_cert_mem = NULL;										   // 从内存而不是文件加载客户端证书
	info->client_ssl_cert_mem_len = 0;										   // 长度字节
	info->client_ssl_private_key_filepath = connect->ssl_private_key_filepath; // 私钥路径
	info->client_ssl_key_mem = NULL;										   // 加载客户端密钥从内存而不是文件
	info->client_ssl_key_mem_len = 0;										   //
	info->client_ssl_ca_filepath = connect->ssl_ca_filepath;				   // CA证书文件路径或NULL
	info->client_ssl_ca_mem = NULL;					 // 从内存加载CA证书，而不是文件
	info->client_ssl_ca_mem_len = 0;				 //
	info->client_ssl_cipher_list = "AES256-SHA:RSA"; // NULL;                               //支持的加密套件,用于在会话中加密
}

/**
 * @description: 建立websock连接
 * @param:
 * @return:
 */
static void ocpp_connect_send(const char *const message, size_t len)
{

	pthread_mutex_lock(&session_data.buffLock);
	strncpy(session_data.sendbuff, message, OCPP_CONNECT_SEND_BUFFER);
	session_data.sendDataLen = len;
	pthread_mutex_unlock(&session_data.buffLock);

	return;
}

#define MAX_RECONNECT_ATTEMPTS 3
#define MAX_WAIT_TIME_SECONDS 3
/**
 * @description: 建立websock连接
 * @param:
 * @return:
 */
static void *ocpp_connect_websocket(ocpp_connect_t *const connect)
{
	struct lws_context *context = NULL;
	struct lws_protocols protocol;
	struct lws_context_creation_info info;
	while (1)
	{
		lwsl_notice("连接服务器\n");
		while (1)
		{
			ocpp_connect_initialize_websocket_context(&info, &protocol, connect);
			context = lws_create_context(&info);
			if (context == NULL)
			{
				lwsl_notice("初始化连接失败:等待5秒后重试\n");
				usleep(5 * 1000 * 1000); // 等待一段时间后重试
			}
			else
			{
				lwsl_notice("初始化连接成功\n");
				break; // 成功创建 context，跳出重试循环
			}
		}

		session_data.context = context;
		session_data.connect = connect;
		session_data.protocol = &protocol;

		int n = 0;
		while (n >= 0 && connect->interrupted == false)
		{
			n = lws_service(context, 0);
			if (n < 0)
			{
				lwsl_notice("lws_service returned error: %d\n", n);
				break;
			}
		}

		lwsl_notice("exiting service , interrupted = %d\n", connect->interrupted);

		lws_context_destroy(context);
		connect->interrupted = false;
		connect->isConnect = false;

		usleep(10 * 1000 * 1000); // 等待一段时间后进行下一次连接尝试

		current_reconnect_attempt++;

		lwsl_notice("第%d次尝试重连\n", current_reconnect_attempt);

		if (current_reconnect_attempt >= MAX_RECONNECT_ATTEMPTS)
		{
			lwsl_notice("达到最大重连次数%d,等待%d分钟后再重试\n", MAX_RECONNECT_ATTEMPTS, MAX_WAIT_TIME_SECONDS);
			sleep(MAX_WAIT_TIME_SECONDS * 60); // 等待一段时间后重试
			current_reconnect_attempt = 0;	   // 重置重连计数
		}
	}

	return NULL;
}

/**
 * @description:
 * @return {*}
 */
bool ocpp_connect_isSendFinish()
{
	if (session_data.connect == NULL)
		return false;

	return session_data.connect->isSendFinish;
}

/**
 * @description: 连接初始化
 * @param:
 * @return:
 */
void ocpp_connect_init(ocpp_connect_t *const connect)
{
	printf("连接初始化");
	connect->send = ocpp_connect_send;
	connect->isConnect = false;	  // default no connect server
	connect->interrupted = false; // default no interrupted
	connect->isSendFinish = true;
	pthread_mutex_init(&session_data.buffLock, NULL);
	// 初始化缓存区
	memset(session_data.sendbuff, 0, OCPP_CONNECT_SEND_BUFFER);
	session_data.sendDataLen = 0;
	pthread_t ptid_connect;
	if (pthread_create(&ptid_connect, NULL, ocpp_connect_websocket, connect) != 0)
	{
		printf("cann't create connect thread %s\n", strerror(errno));
	}

	printf("connect init end\n");
}
