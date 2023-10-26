#include <libwebsockets.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include "ocpp_connect.h"
#include "ocpp_config.h"
#include "ocpp_auxiliaryTool.h"


#define OCPP_CONNECT_SEND_BUFFER 2048 // 发送缓存区大小
// 每个使用这个协议的新连接在建立连接时都会分配这么多内存，在断开连接时会释放这么多内存。指向此按连接分配的指针被传递到user参数中的回调中
typedef struct
{
	ocpp_connect_t *connect;
	size_t sendDataLen;						 // 发送数据大小
	char sendbuff[OCPP_CONNECT_SEND_BUFFER]; // 发送存储区
} ocpp_connect_session_data_t;

static ocpp_connect_session_data_t session_data;

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
int send_ping(struct lws *wsi)
{
	unsigned char buf[LWS_PRE + 2]; // 2是Ping消息的大小
	unsigned char *p = &buf[LWS_PRE];

	p[0] = 0x89; // 设置Ping帧的标志位
	p[1] = 0x00; // 设置Payload Length，通常为0，或者根据需要设置具体大小

	int ret = lws_write(wsi, p, 2, LWS_WRITE_PING);
	if (ret < 0)
	{
		lwsl_err("send ping fail, ret = %d\n", ret);
		return -1;
	}
	else
	{
		lwsl_notice("send ping success, ret = %d\n", ret);
		return 0;
	}
}
// 定义一个定时器结构体
static lws_sorted_usec_list_t sul;
struct lws *wsi = NULL;
struct lws_context *context = NULL;
struct lws_client_connect_info ccinfo;
// 定义一个定时器回调函数
static void sul_cb(lws_sorted_usec_list_t *sul)
{
	// 使用lws_client_connect_via_info函数来重新创建一个客户端连接，并且保存返回的wsi变量
	wsi = lws_client_connect_via_info(&ccinfo);
	if (wsi == NULL)
	{
		lwsl_err("Failed to reconnect to server\n");
	}
	else
	{
		lwsl_info("Reconnected to server\n");
	}
}
/**
 * @description: 回调函数
 * @param:
 * @return:
 */
static int ocpp_connect_service_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{

	switch (reason)
	{
	case LWS_CALLBACK_PROTOCOL_INIT:
		lwsl_notice("PROTOCOL_INIT\n");
		break;

	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		lwsl_err("CONNECTION ERROR: %s\n", (char *)in);
		write_data_lock();
		session_data.connect->isConnect = false;
		rwlock_unlock();
		lws_sul_schedule(context, 0, &sul, sul_cb, SERVER_RECONNECT_INTERVAL * 1000 * LWS_US_PER_MS); // 安排一个定时器，在1
		break;

	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		lwsl_notice("CONNECTION ESTABLISHED\n");
		write_data_lock();
		session_data.connect->isConnect = true;
		rwlock_unlock();
		lws_set_timer_usecs(wsi, 5000000);
		lws_callback_on_writable(wsi);
		break;

	case LWS_CALLBACK_CLIENT_CLOSED:
		lwsl_notice("CONNECTION CLOSED\n");
		write_data_lock();
		session_data.connect->isConnect = false;
		rwlock_unlock();
		lws_sul_schedule(context, 0, &sul, sul_cb, SERVER_RECONNECT_INTERVAL * 1000 * LWS_US_PER_MS); // 安排一个定时器，在1
		break;

	case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:

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

		break;

	case LWS_CALLBACK_TIMER:
		if (send_ping(wsi) == -1)
		{
			lwsl_notice("检测到与服务器断开\n");
			write_data_lock();
			session_data.connect->isConnect = false;
			rwlock_unlock();
		}
		if (session_data.connect->isConnect == false)
		{
			lws_close_reason(wsi, LWS_CLOSE_STATUS_GOINGAWAY, NULL, 0);
			return -1;
		}
		lws_set_timer_usecs(wsi, 5000000);
		break;
	case LWS_CALLBACK_CLIENT_WRITEABLE:
		read_data_lock();
		if (session_data.sendDataLen > 0)
		{
			ocpp_connect_websocket_send_back(wsi, session_data.sendbuff, session_data.sendDataLen);
			session_data.sendDataLen = 0;
		}
		rwlock_unlock();
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE:
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
	info->options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;					   // 初始化SSL库
	info->client_ssl_private_key_password = NULL;							   // 私钥
	info->client_ssl_cert_filepath = NULL;									   // 客户端的证书
	info->client_ssl_cert_mem = NULL;										   // 从内存而不是文件加载客户端证书
	info->client_ssl_cert_mem_len = 0;										   // 长度字节
	info->client_ssl_private_key_filepath = connect->ssl_private_key_filepath; // 私钥路径
	info->client_ssl_key_mem = NULL;										   // 加载客户端密钥从内存而不是文件
	info->client_ssl_key_mem_len = 0;										   //
	info->client_ssl_ca_filepath = connect->ssl_ca_filepath;				   // CA证书文件路径或NULL
	info->client_ssl_ca_mem = NULL;											   // 从内存加载CA证书，而不是文件
	info->client_ssl_ca_mem_len = 0;										   //
	info->client_ssl_cipher_list = "AES256-SHA:RSA";						   // NULL;                               //支持的加密套件,用于在会话中加密
}

/**
 * @description: 建立websock连接
 * @param:
 * @return:
 */
static void ocpp_connect_send(const char *const message, size_t len)
{
	write_data_lock();
	strncpy(session_data.sendbuff, message, OCPP_CONNECT_SEND_BUFFER);
	session_data.sendDataLen = len;
	rwlock_unlock();
	return;
}

/**
 * @description: 建立websock连接
 * @param:
 * @return:
 */
static void *ocpp_connect_websocket(ocpp_connect_t *const connect)
{
	struct lws_protocols protocol;
	struct lws_context_creation_info info;
	ocpp_connect_initialize_websocket_context(&info, &protocol, connect);
	context = lws_create_context(&info);
	if (context == NULL)
	{
		lwsl_notice("初始化上下文失败\n");
		return;
	}

	lwsl_notice("初始化上下文成功\n"); // 只创建一次上下文，之后的重连都是根据这一次的来，如果多次创建上下文会导致文件描述符的增加，即使调用lws_context_destroy(context);也不能释放，导致设备重启
									   // 只创建一次上下文，只输入一次域名，如果域名对应的IP变了，重连是否还能连接上？
									   // 定义一个整数数组，表示第几次重连时的基础延迟时间，单位是毫秒
	static const uint32_t retry_ms_table[] = {1000, 2000, 4000, 8000};

	// 定义一个lws_retry_bo_t结构体，用来设置重连的策略
	static const lws_retry_bo_t retry = {
		.retry_ms_table = retry_ms_table,						// 指向整数数组的指针
		.retry_ms_table_count = LWS_ARRAY_SIZE(retry_ms_table), // 数组中有多少个元素
		.conceal_count = 4,										// 最多可以隐藏4次重连
		.jitter_percent = 10,									// 在基础延迟时间的基础上增加10%的随机抖动
	};

	ccinfo.context = context;
	ccinfo.port = connect->port;
	ccinfo.address = connect->address;
	ccinfo.path = connect->path;
	ccinfo.ssl_connection = connect->isWss ? (LCCSCF_USE_SSL | LCCSCF_ALLOW_EXPIRED) : 0;
	ccinfo.host = connect->address;
	ccinfo.origin = connect->address;
	ccinfo.ietf_version_or_minus_one = -1;
	ccinfo.client_exts = NULL;
	ccinfo.protocol = protocol.name;
	ccinfo.retry_and_idle_policy = &retry;
	while (1)
	{
		wsi = lws_client_connect_via_info(&ccinfo);
		if (wsi != NULL)
		{
			break;
		}
		sleep(SERVER_RECONNECT_INTERVAL);
	}

	while (1)
	{

		lws_service(context, 500);
	}

	lws_context_destroy(context);

	return NULL;
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
	connect->isConnect = false; // default no connect server
	// 初始化缓存区
	memset(session_data.sendbuff, 0, OCPP_CONNECT_SEND_BUFFER);
	session_data.sendDataLen = 0;
	session_data.connect = connect;
	pthread_t ptid_connect;
	if (pthread_create(&ptid_connect, NULL, ocpp_connect_websocket, connect) != 0)
	{
		printf("cann't create connect thread %s\n", strerror(errno));
	}

	printf("connect init end\n");
}
