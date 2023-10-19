
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <sys/time.h>
#include "ocpp_auxiliaryTool.h"
#include <math.h>
#include <stdio.h>
#include <semaphore.h>


/**
 * @description:
 * @param {char} *timeStr
 * @return {*}
 */
void ocpp_AuxiliaryTool_setSystemTime(const char *timeStr)
{

    struct tm *time = calloc(1, sizeof(struct tm));
    strptime(timeStr, "%Y-%m-%dT%H:%M:%S", time);

    time_t time_s = mktime(time);

    if (time_s == -1)
    {
        printf("setime fail1 %s\n", strerror(errno));
        free(time); // 释放分配的内存
        return;
    }

    if (stime(&time_s) == -1)
    {
        printf("setTime fail2 %s\n", strerror(errno));
    }

    free(time); // 在不再需要时释放内存
}

/**
 * @description: 随机生成一个UUID
 * @param:
 * @return:
 */
char *ocpp_AuxiliaryTool_GenerateUUID()
{

    char *uuid = (char *)calloc(1, sizeof(char) * (36 + 1));
    int fd = -1;
    memset(uuid, 0, 37);
    if ((fd = open("/proc/sys/kernel/random/uuid", O_RDONLY)) > 0)
    {

        if (read(fd, uuid, 36) < 0)
            printf("读UUID出错\n");
    }
    uuid[36] = '\0';
    close(fd);
    return uuid;
}

/**
 * @description: 随机生成一个字符串
 * @param:
 * @return:
 */
char *ocpp_AuxiliaryTool_GenerateString(int stringLen)
{

    int n = 0;
    char *string = (char *)calloc(1, sizeof(char) * (stringLen + 1));

    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    if (string)
    {
        for (n = 0; n < stringLen; n++)
        {
            int key = rand() % (int)(sizeof(charset) - 1);
            string[n] = charset[key];
        }
        string[stringLen + 1] = '\0';
    }
    return string;
}

/**
 * @description: 随机生成一个 integer
 * @param:
 * @return:
 */
int ocpp_AuxiliaryTool_GenerateInt()
{

    // 设置随机数种子为当前时间
    srand(time(0));

    return rand();
}

/**
 * @description: 获取当前时间字符串 格式为: %Y-%m-%dT%H:%M:%S.000Z
 * @param:
 * @return:
 */
char *ocpp_AuxiliaryTool_GetCurrentTime()
{

    time_t now;
    time(&now);
    struct tm *now_tm;
    now_tm = localtime(&now);
    char *currentTime = (char *)calloc(1, sizeof(char) * 32);
    strftime(currentTime, 32, "%Y-%m-%dT%H:%M:%S.000Z", now_tm);
    currentTime[31] = '\0';
    return currentTime;
}

/**
 * @description: 获取系统上电运行已来的毫秒时间
 * @param:
 * @return:
 */
unsigned int ocpp_AuxiliaryTool_getSystemTime_ms(void)
{
    struct timeval tv;
    unsigned int u32_time_ms;

    gettimeofday(&tv, NULL);
    u32_time_ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    return u32_time_ms;
}
/**
 * @description: 计算时间差
 * @param:
 * @return:
 */
unsigned int ocpp_AuxiliaryTool_getDiffTime_ms(unsigned int *pu32_last_time)
{
    unsigned int u32_curr_time, u32_diff_time;

    u32_curr_time = ocpp_AuxiliaryTool_getSystemTime_ms();
    if (u32_curr_time >= (*pu32_last_time))
    {
        u32_diff_time = u32_curr_time - (*pu32_last_time);
    }
    else
    {
        u32_diff_time = ~(*pu32_last_time) + u32_curr_time;
    }
    return u32_diff_time;
}

/**
 * @description: 计算字符串pString中c字符的数量
 * @param:
 * @return：
 */
int ocpp_AuxiliaryTool_charCounter(char *pString, char c)
{
    int count = 0;
    char *pTemp = pString;

    while (pTemp != NULL)
    {
        pTemp = strchr(pTemp, c);
        if (pTemp)
        {
            pTemp++;
            count++;
        }
    }

    return count;
}

/**
 * @description: 将base乘于exp 次，exp中每一个bit为一次
 * @param:
 * @return:
 */
int ocpp_AuxiliaryTool_Ipow(int base, int exp)
{
    int result = 1;
    while (exp)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        base *= base;
    }

    return result;
}

/**
 * @description: 字符串倒转
 * @param:  str-字符串指针,len-字符串长度
 * @return：
 */
void ocpp_AuxiliaryTool_Reverse(char *str, int len)
{
    int i = 0, j = len - 1, temp;
    while (i < j)
    {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}

/**
 * @description: 整形转换为字符串
 * @param:  x = 需要转化的整数,str = 存储转化后的字符，d = 转化的字符串位数
 * eg：x = 123, d = 5, str = '00123'
 * @return：
 */
int ocpp_AuxiliaryTool_intToStr(int x, char str[], int d)
{
    int i = 0;
    if (x != 0)
    {
        while (x)
        {
            str[i++] = (x % 10) + '0';
            x = x / 10;
        }
    }
    else
    {
        str[i++] = '0';
    }

    while (i < d)
        str[i++] = '0';

    ocpp_AuxiliaryTool_Reverse(str, i);
    str[i] = '\0';
    return i;
}

/**
 * @description: 将浮点数转换为字符串。
 * @param:
 * @return：
 */
void ocpp_AuxiliaryTool_ftoa(float n, char *res, int afterpoint)
{

    int ipart = (int)n;                                 // 提取整数部分
    float fpart = n - (float)ipart;                     // 提取浮动部分
    int i = ocpp_AuxiliaryTool_intToStr(ipart, res, 0); // 将整数部分转换为字符串

    // 检查点后的显示选项
    if (afterpoint != 0)
    {
        res[i] = '.';

        // 获取分数部分的值，直至给定编号。点后的点数。需要第三个参数,处理像233.007这样的案件
        fpart = fpart * ocpp_AuxiliaryTool_Ipow(10, afterpoint);

        ocpp_AuxiliaryTool_intToStr((int)fpart, res + i + 1, afterpoint);
    }
}
