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

#include "ocpp_firmwareUpdata.h"
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

// 通用文件上传函数，支持HTTP和FTP
int upload_file(const char *upload_url, const char *local_file_path)
{
	if (local_file_path == NULL || upload_url == NULL)
	{
		fprintf(stderr, "错误:本地文件路径和上传URL不能为空。\n");
		return -1; // 返回错误代码，表示参数无效
	}

	CURL *curl;
	FILE *file;

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();

	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, upload_url);

		// 设置FTP上传选项
		if (strstr(upload_url, "ftp://"))
		{
			curl_easy_setopt(curl, CURLOPT_UPLOAD, -1L);
			curl_easy_setopt(curl, CURLOPT_READDATA, fopen(local_file_path, "rb"));
		}

		CURLcode res = curl_easy_perform(curl);

		if (res == CURLE_OK)
		{
			printf("文件上传完成.\n");
		}
		else
		{
			fprintf(stderr, "上传失败: %s\n", curl_easy_strerror(res));
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

//test
// int main()
// {
// 	const char *url = "http://10.100.70.120/EVCM-SD10.tar.gz"; // HTTP URL
// 	const char *save_path = "/app/core/EVCM-SD10.tar.gz";	   // 本地保存路径

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
