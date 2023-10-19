#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <stdbool.h>
#include "ocpp_package.h"
#include "ocpp_firmwareUpdata.h"
#include "ocpp_chargePoint.h"
#include "ocpp_auxiliaryTool.h"
extern ocpp_chargePoint_t *ocpp_chargePoint;
// 通用文件下载函数，支持HTTP和FTP
int download_file(const char *url, const char *local_file_path)
{
	if (url == NULL || local_file_path == NULL)
	{
		fprintf(stderr, "错误:URL和本地文件路径不能为空。\n");
		return -1; // 返回错误代码，表示参数无效
	}

	CURL *curl;
	FILE *file;

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();

	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, url);

		file = fopen(local_file_path, "wb");
		if (file)
		{
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
			CURLcode res = curl_easy_perform(curl);
			fclose(file);

			if (res == CURLE_OK)
			{
				printf("文件下载完成.\n");
			}
			else
			{
				fprintf(stderr, "下载失败: %s\n", curl_easy_strerror(res));
				return -1;
			}
		}
		else
		{
			fprintf(stderr, "无法打开文件进行写入\n");
			return -1;
		}

		curl_easy_cleanup(curl);
	}
	else
	{
		fprintf(stderr, "初始化libcurl失败\n");
		return -1;
	}

	curl_global_cleanup();
	return 0;
}

int upload_file(const char *url, const char *local_file_path)
{
	if (url == NULL || local_file_path == NULL)
	{
		fprintf(stderr, "错误:URL和路径不能为空。\n");
		return -1;
	}

	CURL *curl = curl_easy_init();
	if (curl == NULL)
	{
		fprintf(stderr, "错误:初始化libcurl失败。\n");
		return -1;
	}

	curl_easy_setopt(curl, CURLOPT_URL, url);

	FILE *file = fopen(local_file_path, "rb");
	if (file == NULL)
	{
		fprintf(stderr, "错误：打开本地文件失败。\n");
		curl_easy_cleanup(curl);
		return -1;
	}

	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	long size = 0; // 文件大小

	// 根据URL协议选择上传方式
	if (strncmp(url, "ftp://", 6) == 0) // FTP上传
	{
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L); // 启用上传模式
		curl_easy_setopt(curl, CURLOPT_READDATA, file);
	}
	else if (strncmp(url, "http://", 7) == 0 || strncmp(url, "https://", 8) == 0) // HTTP(S)上传
	{
		fseek(file, 0, SEEK_END);
		size = ftell(file);
		fseek(file, 0, SEEK_SET);

		// 设置HTTP请求方法为POST
		curl_easy_setopt(curl, CURLOPT_POST, 1L);

		// 设置HTTP请求体为本地文件内容
		curl_easy_setopt(curl, CURLOPT_READDATA, file);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)size);
	}
	else
	{
		fprintf(stderr, "错误：不支持的协议。\n");
		fclose(file);
		curl_easy_cleanup(curl);
		return -1;
	}

	CURLcode res = curl_easy_perform(curl);

	curl_slist_free_all(headers);
	fclose(file);
	curl_easy_cleanup(curl);

	if (res == CURLE_OK)
	{
		printf("文件上传完成。\n");
		return 0;
	}
	else
	{
		fprintf(stderr, "上传失败: %s\n", curl_easy_strerror(res));
		return -1;
	}
}
/**
 * @description: 固件更新线程
 * @param
 * @param
 * @return
 */
void *ocpp_chargePoint_UpdateFirmware_thread(void *arg)
{
	ocpp_package_UpdateFirmware_req_t *UpdateFirmware = (ocpp_package_UpdateFirmware_req_t *)arg;
	printf("url = %s\n", UpdateFirmware->location);
	int retries = 0;
	bool isStopAllTransaction = false;
	bool downloadstatus = false;
	int connector = 0;
	struct tm retrieveDate;
	memset(&retrieveDate, 0, sizeof(struct tm));
	strptime(UpdateFirmware->retrieveDate, "%Y-%m-%dT%H:%M:%S.000Z", &retrieveDate);
	// 计算目标时间的时间戳
	time_t firmwareTime = mktime(&retrieveDate);
	int secondsToWait = 0;
	time_t currentTime = 0;
	int allChargersStopped = 0; // 标志变量，0 表示不是所有枪都已停止充电
	while (1)
	{
		if (retries >= UpdateFirmware->retries)
		{
			break;
		}
		ocpp_chargePoint->ocpp_firmwareUpdate_lastUpdateState = OCPP_PACKAGE_FIRMWARE_STATUS_DOWNLOADING;
		ocpp_chargePoint_sendFirmwareStatusNotification_Req(OCPP_PACKAGE_FIRMWARE_STATUS_DOWNLOADING);
		if (download_file(UpdateFirmware->location, OCPP_FIRMWARE_UPDATA_FILEPATH) == 0)
		{
			downloadstatus = true;

			ocpp_chargePoint->ocpp_firmwareUpdate_lastUpdateState = OCPP_PACKAGE_FIRMWARE_STATUS_DOWNLOADED;

			ocpp_chargePoint_sendFirmwareStatusNotification_Req(OCPP_PACKAGE_FIRMWARE_STATUS_DOWNLOADED);
			break;
		}
		else
		{

			ocpp_chargePoint->ocpp_firmwareUpdate_lastUpdateState = OCPP_PACKAGE_FIRMWARE_STATUS_DOWNLOAD_FAILED;
			ocpp_chargePoint_sendFirmwareStatusNotification_Req(OCPP_PACKAGE_FIRMWARE_STATUS_DOWNLOAD_FAILED);
		}
		retries++;
		usleep(UpdateFirmware->retryInterval * 1000 * 1000);
	}

	if (downloadstatus)
	{
		// 获取当前时间的时间戳
		currentTime = time(NULL);
		// 计算等待时间
		secondsToWait = firmwareTime - currentTime;
		if (secondsToWait > 0)
		{
			sleep(secondsToWait); // 等待到达升级时间
		}
		while (!allChargersStopped)
		{
			allChargersStopped = 1; // 假设所有枪都已停止充电

			for (int connector = 1; connector <= ocpp_chargePoint->numberOfConnector; connector++)
			{
				if (ocpp_chargePoint->transaction_obj[connector]->isTransaction)
				{
					// 查询充电状态
					if (isChargingStopped(ocpp_chargePoint->transaction_obj[connector]))
					{
						ocpp_chargePoint->transaction_obj[connector]->isTransaction = 0; // 设置为充电已停止
					}
					else
					{
						allChargersStopped = 0; // 有枪还在充电，将标志设置为 0
					}
				}
			}

			if (!allChargersStopped)
			{
				sleep(5); // 休眠 5 秒（你可以根据需求调整等待时间）
			}
		}
		ocpp_chargePoint_sendFirmwareStatusNotification_Req(OCPP_PACKAGE_FIRMWARE_STATUS_INSTALLING);
		system("reboot");
	}

	if (UpdateFirmware)
	{
		free(UpdateFirmware);
	}

	ocpp_chargePoint->ocpp_firmwareUpdate_lastUpdateState == OCPP_PACKAGE_FIRMWARE_STATUS_IDLE;

	return NULL;
}

// test
//  int main()
//  {
//  	const char *url = "http://10.100.70.120/EVCM-SD10.tar.gz"; // HTTP URL
//  	const char *save_path = "/app/core/EVCM-SD10.tar.gz";	   // 本地保存路径

// 	int result = download_file(url, save_path);

// 	if (result == 0)
// 	{
// 		printf("文件下载成功！\n");
// 	}
// 	else
// 	{
// 		printf("文件下载失败。\n");
// 	}

// 	return 0;
// }
