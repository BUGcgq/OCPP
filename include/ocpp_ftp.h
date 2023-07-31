/*
 * @Author: LIYAOHAN 1791002655@qq.com
 * @Date: 2023-05-05 14:36:43
 * @LastEditors: LIYAOHAN 1791002655@qq.com
 * @LastEditTime: 2023-05-06 10:21:59
 * @FilePath: /OCPP/ocpp_ftp.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __OCPP_FTP__H__
#define __OCPP_FTP__H__

#ifdef __cplusplus
extern "C" {
#endif

 
#define OCPP_FTP_SERVER_PORT 21

enum OCPP_FTP_CMD_TYPE_E{
    CMD_USER =0,       //用户名
    CMD_PASS,          //密码
    CMD_PASV,          //让服务器进入被动模式
    CMD_CWD,           //切换工作目录
    CMD_SIZE_FTP,      //获取文件大小
    CMD_RETR,          //下载文件
    CMD_REST,          //指定下载文件的偏移量
    CMD_QUIT,          //退出命令 
    CMD_LIST,          //列表
    CMD_STOR,          //上传文件    
    CMD_PORT_FTP,      //发送客户端端口给服务器
    CMD_MLSD,          //列表
};


typedef struct{
     char usr[32];
     char passwd[32];
     char ser_filepath[512];
     char ser_filename[64];
     char new_filename[64];
     int control_sock;
 
}ocpp_ftp_client_t; 


int ocpp_ftp_connect_server(const char* server_ip,const int port);
int ocpp_ftp_login_server(int ctrl_sock,const char *user_name, const char * passwd);
int ocpp_ftp_down_file(int ctrl_sock, char *server_filepath_name,const  char *newfilename,int connect_mode,int offset,enum OCPP_FTP_CMD_TYPE_E type);
int ocpp_ftp_up_file(int ctrl_sock, char *server_filepath_name,const  char *srcfilename,int connect_mode,int offset);
int ocpp_ftp_get_fsize(int ctrl_sock, char *server_filepath_name);
int ocpp_ftp_quit_server(int ctrl_sock);
 
 
 





#ifdef __cplusplus
}
#endif

#endif