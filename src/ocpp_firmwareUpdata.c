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
#include <time.h>
#include "ocpp_package.h"
#include "ocpp_firmwareUpdata.h"
#include "ocpp_chargePoint.h"
#include "ocpp_auxiliaryTool.h"
extern ocpp_chargePoint_t *ocpp_chargePoint;
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
		write_data_lock();
		ocpp_chargePoint->ocpp_firmwareUpdate_lastUpdateState = OCPP_PACKAGE_FIRMWARE_STATUS_DOWNLOADING;
		rwlock_unlock();
		ocpp_chargePoint_sendFirmwareStatusNotification_Req(OCPP_PACKAGE_FIRMWARE_STATUS_DOWNLOADING);
		usleep(MAX_TIMEOUT_SECONDS * 1000 * 1000);
		if (ocpp_download_file(UpdateFirmware->location, OCPP_FIRMWARE_UPDATA_FILEPATH, CURL_FTP_mode) == 0)
		{
			downloadstatus = true;
			write_data_lock();
			ocpp_chargePoint->ocpp_firmwareUpdate_lastUpdateState = OCPP_PACKAGE_FIRMWARE_STATUS_DOWNLOADED;
			rwlock_unlock();
			ocpp_chargePoint_sendFirmwareStatusNotification_Req(OCPP_PACKAGE_FIRMWARE_STATUS_DOWNLOADED);
			usleep(MAX_TIMEOUT_SECONDS * 1000 * 1000);
			break;
		}
		else
		{
			write_data_lock();
			ocpp_chargePoint->ocpp_firmwareUpdate_lastUpdateState = OCPP_PACKAGE_FIRMWARE_STATUS_DOWNLOAD_FAILED;
			rwlock_unlock();
			ocpp_chargePoint_sendFirmwareStatusNotification_Req(OCPP_PACKAGE_FIRMWARE_STATUS_DOWNLOAD_FAILED);
			usleep(MAX_TIMEOUT_SECONDS * 1000 * 1000);
			retries++;
			continue;
		}
		sleep(UpdateFirmware->retryInterval);
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
			read_data_lock();
			for (int connector = 1; connector <= ocpp_chargePoint->numberOfConnector; connector++)
			{
				if (ocpp_chargePoint->transaction_obj[connector]->isTransaction)
				{

					allChargersStopped = 0; // 有枪还在充电，将标志设置为 0
				}
			}
			rwlock_unlock();
			if (!allChargersStopped)
			{
				sleep(5); // 休眠 5 秒（根据需求调整等待时间）
			}
		}
		ocpp_chargePoint_sendFirmwareStatusNotification_Req(OCPP_PACKAGE_FIRMWARE_STATUS_INSTALLING);
		usleep(MAX_TIMEOUT_SECONDS * 1000 * 1000);
		system("reboot");
	}

	if (UpdateFirmware)
	{
		free(UpdateFirmware);
	}
	write_data_lock();
	ocpp_chargePoint->ocpp_firmwareUpdate_lastUpdateState = OCPP_PACKAGE_FIRMWARE_STATUS_IDLE;
	rwlock_unlock();
	return NULL;
}
