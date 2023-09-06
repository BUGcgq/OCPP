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
#include "ocpp_package.h"
#include "ocpp_chargePoint.h"


static enum OCPP_PACKAGE_FIRMWARE_STATUS_E ocpp_firmwareUpdate_lastUpdateState;

int ftpRecvResponse(int sock, char *buf)
{
	int i;

	if (recv(sock, buf, 5120, 0) == -1)
	{ // receive the data
		perror("recv");
		return 0;
		;
	}

	for (i = (strlen(buf) - 1); i > 0; i--)
	{
		if (buf[i] == '.' || buf[i] == '\r')
		{
			buf[i + 1] = '\0';
			break;
		}
	}
	return 1;
}

// Envia un mensaje FTP a un socket
int ftpNewCmd(int sock, char *buf, char *cmd, char *param)
{
	strcpy(buf, cmd);
	if (param != NULL && strlen(param) > 0)
	{
		strcat(buf, " ");
		strcat(buf, param);
	}
	strcat(buf, "\r\n");
	printf("*%s", buf); // print the cmd to the screen
	if (send(sock, buf, strlen(buf), 0) == -1)
	{
		perror("send");
		return 0;
	}
	// clear the buffer
	return 1;
}

// Convierte lo recibido del servidor en una IP y un puerto, para saber donde nos indica
// que enviemos realmente el fichero. En hostname y port se escribiran estos datos
//...a partir de lo que tenemos en buf
// 将从服务器收到的内容转换为IP和端口，以了解它指示我们的位置

// 让我们真正发送文件。这些数据将写入主机名和端口

// …从我们在布夫的东西中

int ftpConvertAddy(char *buf, char *hostname, int *port)
{
	unsigned int i, t = 0;
	int flag = 0, decCtr = 0, tport1, tport2;
	char tmpPort[6];

	for (i = 0; i < strlen(buf); i++)
	{
		if (buf[i] == '(')
		{
			flag = 1;
			i++;
		}
		if (buf[i] == ')')
		{
			hostname[t] = '\0';
			break;
		}

		if (flag == 1)
		{
			if (buf[i] != ',')
			{
				hostname[t] = buf[i];
				t++;
			}
			else
			{
				hostname[t] = '.';
				t++;
			}
		}
	}
	t = 0;
	for (i = 0; i < strlen(hostname); i++)
	{
		if (hostname[i] == '.')
			decCtr++;
		if (decCtr == 4 && hostname[i] != '.')
		{
			tmpPort[t] = hostname[i];
			t++;
			if (hostname[i + 1] == '.')
			{
				tmpPort[t] = '\0';
				tport1 = atoi(tmpPort);
				t = 0;
			}
		}

		if (decCtr == 5 && hostname[i] != '.')
		{
			tmpPort[t] = hostname[i];
			t++;
			if (hostname[i + 1] == '\0')
			{
				tmpPort[t] = '\0';
				tport2 = atoi(tmpPort);
				t = 0;
			}
		}
	}

	*port = tport1 * 256 + tport2;
	decCtr = 0;
	for (i = 0; i < strlen(hostname); i++)
	{
		if (hostname[i] == '.')
		{
			decCtr++;
		}
		if (decCtr > 3)
			hostname[i] = '\0';
	}
	return 0;
}

int ftpGetFile(char *filename, int sd)
{
	// Cogido de https://www.linuxquestions.org/questions/programming-9/tcp-file-transfer-in-c-with-socket-server-client-on-linux-help-with-code-4175413995/
	char revbuf[512];
	FILE *pf;
	unsigned long fsize;
	pf = fopen(filename, "wb");
	if (pf == NULL)
	{
		printf("File not found!\n");
		return 1;
	}
	else
	{
		bzero(revbuf, 512);
		int block_sz = 0;
		while ((block_sz = recv(sd, revbuf, 512, 0)) >= 0)
		{
			int write_sz = fwrite(revbuf, sizeof(char), block_sz, pf);
			fflush(pf);
			if (write_sz < block_sz)
			{
				perror("File write failed.\n");
			}

			bzero(revbuf, 512);

			if (block_sz == 0 || block_sz != 512)
			{
				break;
			}
		}

		if (block_sz < 0)
		{
			if (errno == EAGAIN)
			{
				printf("recv() timed out.\n");
			}
			else
			{
				fprintf(stderr, "recv() failed due to errno = %d\n", errno);
			}
		}

		printf("Ok received from server!\n");

		fclose(pf);
	}

	close(sd); // close the socket
	return 0;
}

// Funcion que se descarga un fichero del hostname y puerto indicados, con el usuario y password indicados
// 函数从指定的主机名和端口下载文件，并指定用户和密码
// ftpGetProcess(params->location, 21, params->username, params->password, params->originalfilename,params->destinationfilename);
int ocpp_firmwareUpdata_ftpGetProcess(char *hostname, int port, char *username, char *password)
{
	int sd;
	struct sockaddr_in pin;
	struct hostent *hp;
	char tmpHost[100];
	int tmpPort;
	char buf[5120];

	int filesize; // 升级包大小
	// DATA
	int dataSock;
	struct sockaddr_in dataServAddr; /* data address*/
	unsigned short dataServPort;
	int numberOfConn;

	///////////////////////////////////////////////////////////////
	//"ftp://192.168.1.31/inc/SD10"  EVCM-SD10.tar.gz
	printf("URL = %s", hostname);
	char *stringPointStart;
	char *stringPointEnd;
	stringPointStart = strchr(hostname, '/');
	stringPointStart++;
	stringPointStart++;								////stringPointStart = '1'
													//   DEBUG_MSG("stringPointStart = #%c#",*stringPointStart);
	stringPointEnd = strchr(stringPointStart, '/'); ////stringPointEnd = 'IP地址结尾'
													//   DEBUG_MSG("stringPointEnd = #%c#",*stringPointEnd);
	char ipAddress[20];
	strncpy(ipAddress, stringPointStart, (stringPointEnd - stringPointStart));
	ipAddress[stringPointEnd - stringPointStart + 1] = '\0';
	printf("ipAddress = #%s#", ipAddress);

	char ftpPath[256];
	char ftpUpdateFileName[128];
	ftpPath[0] = '.';
	stringPointStart = strchr(stringPointStart, '/'); // stringPointStart = 'IP地址结尾的 / '
	stringPointEnd = strrchr(stringPointStart, '/');  // stringPointEnd = 'URL最后一个 /'
	//   DEBUG_MSG("stringPointStart  = #%c#",*stringPointStart);
	//   DEBUG_MSG("stringPointEnd = #%c#",*stringPointEnd);
	strncpy(ftpPath + 1, stringPointStart, (stringPointEnd - stringPointStart) + 1);
	ftpPath[1 + (stringPointEnd - stringPointStart) + 1] = '\0';
	printf("ftpPath = #%s#\n", ftpPath);

	strncpy(ftpUpdateFileName, stringPointEnd + 1, strlen(stringPointEnd) + 1);
	printf("ftpUpdataName = #%s#\n", ftpUpdateFileName);

	// IP地址、文件路径、文件名为空
	if (ipAddress == NULL || ftpPath == NULL || ftpUpdateFileName == NULL)
	{
		return -1;
	}

	// 文件名不正确
	if (strcmp(ftpUpdateFileName, "EVCM-SD10.tar.gz") != 0)
	{
		return -1;
	}
	//////////////////////////////////////////////////////////////

	if ((hp = gethostbyname(ipAddress)) == 0)
	{
		perror("gethostbyname");
		return -1;
	}

	memset(&pin, 0, sizeof(pin));
	pin.sin_family = AF_INET;
	pin.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
	pin.sin_port = htons(port);

	// Socket for port 21
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket 21");
		return -1;
	}

	if (connect(sd, (struct sockaddr *)&pin, sizeof(pin)) == -1)
	{
		perror("connect");
		return -1;
	}
	/*
	   ////////////////////////
	   //Socket for port 20
	   if ((dataSock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		  perror("socket 20");
		  return -1;
	   }

	   dataServPort = 20;

	   memset(&dataServAddr, 0, sizeof(dataServAddr));
	   dataServAddr.sin_family         = AF_INET;
	   dataServAddr.sin_addr.s_addr    = htonl(INADDR_ANY);
	   dataServAddr.sin_port           = htons(dataServPort);


	   bind(dataSock,(struct sockaddr *) &dataServAddr, sizeof(dataServAddr));


	   listen(dataSock, numberOfConn);
	*/
	system("rm /app/update/EVCM-SD10.tar.gz");
	if (ftpRecvResponse(sd, buf) == 1)
	{
		printf("wait for ftp server to start talking");

		if (strncmp(buf, "220", 3) == 0)
		{
			printf("make sure it ends with a 220");

			if (ftpNewCmd(sd, buf, "USER", username) == 1)
			{
				printf("issue the command to login");

				if (ftpRecvResponse(sd, buf) == 1)
				{
					printf("wait for response");

					if (strncmp(buf, "331", 3) == 0)
					{
						printf("make sure response is a 331");

						if (ftpNewCmd(sd, buf, "PASS", password) == 1)
						{
							printf("input password");

							if (ftpRecvResponse(sd, buf) == 1)
							{
								printf("wait for response");

								if (strncmp(buf, "230", 3) == 0)
								{
									printf("make sure its a 230");

									if (ftpNewCmd(sd, buf, "TYPE", "I") == 1)
									{
										printf("file type");
										//										DEBUG_MSG("notify server we want to pass a file");

										if (ftpRecvResponse(sd, buf) == 1)
										{
											printf("wait for response");

											if (strncmp(buf, "200", 3) == 0)
											{
												printf("mke sure it starts with a 200");
												//												DEBUG_MSG("make sure it starts with a 229");

												// 请求服务器等待数据连接
												if (ftpNewCmd(sd, buf, "PASV", NULL) == 1)
												{

													printf("The request server is waiting for a data connection");
													if (ftpRecvResponse(sd, buf) == 1)
													{

														printf("wait for response");
														if (strncmp(buf, "227", 3) == 0)
														{

															printf("make sure it start with 227");
															ftpConvertAddy(buf, tmpHost, &tmpPort); // then convert the return to usable data
															printf("I will back %s\n\n", buf);
															printf("HOST: %s, PORT:%d", tmpHost, tmpPort);
															////////////////////////////////////////////////
															// NUEVA CONEXION DE DATOS
															int sd2;
															struct sockaddr_in pin;
															struct hostent *hp;
															//        	              									char *buf;
															char buffer[256];

															if ((hp = gethostbyname(tmpHost)) == 0)
															{
														
																perror("gethostbyname");
																return -1;
															}

															memset(&pin, 0, sizeof(pin));
															pin.sin_family = AF_INET;
															pin.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
															pin.sin_port = htons(tmpPort);

															if ((sd2 = socket(AF_INET, SOCK_STREAM, 0)) == -1)
															{
														
																perror("socket port 20");
																return -1;
															}

															if (connect(sd2, (struct sockaddr *)&pin, sizeof(pin)) == -1)
															{
																perror("Error while connecting.");
																return -1;
															}

															////////////////////////////////////////////////
													
															if (ftpNewCmd(sd, buf, "CWD", ftpPath) == 1)
															{ /////////////FTP服务器路径
																printf("change dir");

																if (ftpRecvResponse(sd, buf) == 1)
																{
																	printf("wait a response");

																	if (strncmp(buf, "250", 3) == 0)
																	{
																		printf("make sure it start with 250");

																		if (ftpNewCmd(sd, buf, "SIZE", ftpUpdateFileName) == 1)
																		{ /////////////////升级包文件名
																			printf("Get file size");

																			if (ftpRecvResponse(sd, buf) == 1)
																			{
																				printf("wait for a response");

																				if (strncmp(buf, "213", 3) == 0)
																				{
																					filesize = atoi((char *)buf + 4);
																					printf("file size %d", filesize);

																					if (ftpNewCmd(sd, buf, "RETR", ftpUpdateFileName) == 1)
																					{ /////////////////////从服务器下载文件
																						printf("set the file name AND");

																						if (ftpRecvResponse(sd, buf) == 1)
																						{
																							printf("wait for a response");

																							if (strncmp(buf, "150", 3) == 0 || strncmp(buf, "120", 3) == 0)
																							{
																								printf("make sure its a 150 so we know the server got it all");
																								printf("下载数据包");
																								ftpGetFile("/app/update/EVCM-SD10.tar.gz" /*file*/, sd2); ///////////////////本地路径

																								if (ftpRecvResponse(sd, buf) == 1)
																								{
																									printf("wait for a response");

																									if (strncmp(buf, "226", 3) == 0)
																									{
																										printf("make sure it start with 226");
																										if (ftpNewCmd(sd, buf, "QUIT", "") == 1)
																										{ // set the file name AND
																											printf("Quit");
																											return 1;
																										}
																									}
																								}
																							}
																						}
																					}
																				}
																			}
																		}
																	}
																}
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return 0;
}

/**
 * @description:
 * @param {void *} arg
 * @return {*}
 */
void *ocpp_firmwareUpdata_thread(void *arg)
{
	ocpp_firmwareUpdata_data_t *data = (ocpp_firmwareUpdata_data_t *)arg;

	char *username = strdup("inc01");
	char *password = strdup("inc01123");

	struct tm *startTime;
	startTime = calloc(1, sizeof(struct tm));
	strptime(data->retrieveDate, "%Y-%m-%dT%H:%M:%S.%Z", startTime);
	time_t fecha = mktime(startTime);
	free(startTime);

	time_t now;
	struct tm *now_tm;
	time(&now);
	now_tm = localtime(&now);
	double diffSecs = difftime(fecha, now);

	enum FIRMWARE_UPDATE_FLOW
	{
		WAIT_DOWNLOAD = 0,
		DOWNLOADING,
		INSTALL,
		FINISH
	};

	enum FIRMWARE_UPDATE_FLOW flow = WAIT_DOWNLOAD;

	while (1)
	{

		switch (flow)
		{
		case WAIT_DOWNLOAD:
		{

			while (diffSecs > 0.0)
			{
				usleep(10 * 1000 * 1000);
				time(&now);
				diffSecs = difftime(fecha, now);
			}
			ocpp_firmwareUpdate_lastUpdateState = OCPP_PACKAGE_FIRMWARE_STATUS_DOWNLOADED;

			flow = DOWNLOADING;
		}
		break;

		case DOWNLOADING:
		{
			int result = ocpp_firmwareUpdata_ftpGetProcess(data->anyURI, 21, username, password);

			if (result != 1)
			{
				if (data->retryInterval > 0)
					usleep(1000 * 1000 * data->retryInterval);
				else
					usleep(30 * 1000 * 1000);

				int i = 0;
				for (i = 0; i < data->retries; i++)
				{
					result = ocpp_firmwareUpdata_ftpGetProcess(data->anyURI, 21, username, password);
					if (result != 1)
					{
						if (data->retryInterval > 0)
							usleep(1000 * 1000 * data->retryInterval);
						else
							usleep(30 * 1000 * 1000);
					}
					else
					{
						flow = INSTALL;
						ocpp_firmwareUpdate_lastUpdateState = OCPP_PACKAGE_FIRMWARE_STATUS_INSTALLING;
						break;
					}
				}

				// 获取升级包失败
				if (i >= data->retries)
				{
					flow = FINISH;
					ocpp_firmwareUpdate_lastUpdateState = OCPP_PACKAGE_FIRMWARE_STATUS_DOWNLOAD_FAILED;
				}
			}
			else
			{
				flow = INSTALL;
				ocpp_firmwareUpdate_lastUpdateState = OCPP_PACKAGE_FIRMWARE_STATUS_INSTALLING;
			}
		}
		break;

		case INSTALL:
		{
			int connector = 0;
			for (connector = 0; connector < ocpp_chargePoint->numberOfConnector; connector++)
			{
				if (ocpp_transaction_isTransaction(connector + 1))
				{
					ocpp_transaction_setRemoteStop(connector + 1);
					ocpp_transaction_setStopReason(connector + 1, OCPP_PACKAGE_STOP_REASON_SOFTRESET);
				}
			}
		}
		break;

		case FINISH:
		{

			int connector = 0;
			for (connector = 0; connector < ocpp_chargePoint->numberOfConnector; connector++)
			{
				if (ocpp_transaction_isTransaction(connector + 1))
					break;
			}

			if (connector >= ocpp_chargePoint->numberOfConnector)
			{
				free(username);
				free(password);
				free(data);
				reboot();
				return;
			}
		}
		break;
		}
	}
}

enum OCPP_PACKAGE_FIRMWARE_STATUS_E ocpp_firmwareUpdate_getUpdataStatus()
{

	return ocpp_firmwareUpdate_lastUpdateState;
}

void ocpp_firmwareUpdate_init()
{

	ocpp_firmwareUpdate_lastUpdateState = OCPP_PACKAGE_FIRMWARE_STATUS_IDLE;
}
