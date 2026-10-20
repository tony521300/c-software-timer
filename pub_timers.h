/**
 * @file    pub_timers.h
 * @brief   通用延时与非阻塞超时检测接口头文件
 * @author  Cnn
 * @date    2025-08-21
 * @date    2026-05-30  update  PubTime_1ms_Task()
 * @note
 * - 需要在项目中维护全局计数变量：
 * // 全局 Timers 计数器
 *    PubTime_1ms_Task(); 定时器里 每1ms调用
 * - 提供阻塞延时函数（微秒/毫秒）及非阻塞超时检测接口
 */

#ifndef _PUB_TIMES_H_
#define _PUB_TIMES_H_

#include "pub_timers_config.h"

// 定时器里 每1ms调用
void PubTime_1ms_Task(void);

#if (OPEN_ALL_FUNCS || OPEN_FORMAT_TIME || OPEN_TIME_DISP)
/*================== 格式化时间接口（时:分:秒）==================*/
/**
 * @brief   将总秒数转换为小时、分钟和秒
 *
 * @param   total_seconds  [in]  总秒数
 * @param   hour           [out] 存放小时的指针，不能为 NULL
 * @param   minute         [out] 存放分钟的指针，不能为 NULL
 * @param   second         [out] 存放秒的指针，不能为 NULL
 *
 * @note
 * 1. 函数内部会检查指针是否为 NULL，若任一指针为 NULL，则函数直接返回。
 * 2. 小时可以大于 24，函数不做限制。
 * 3. 可用于倒计时、定时器或时间显示等功能。
 */
void TimeFromSeconds(unsigned long total_seconds,
                     unsigned int *hour,
                     unsigned char *minute,
                     unsigned char *second);
#endif

#if (OPEN_ALL_FUNCS || OPEN_TIME_DISP)
/**
 * @brief   将时间转换为6位数组 [HH:MM:SS] → [h十,h个,m十,m个,s十,s个]
 */
void TimeToDisplayArray(unsigned long total_seconds, unsigned char *time_array);
#endif

/*================== 阻塞延时接口 ==================*/
#if OPEN_ALL_FUNCS || OPEN_DELAY_US || OPEN_DELAY_MS
/**
 * @brief 阻塞微秒延时
 * @param us 延时微秒数
 * @note 阻塞 CPU，精度依赖 MCU 主频
 */
void PubDelayUs(unsigned int us);
#endif

#if OPEN_ALL_FUNCS || OPEN_DELAY_MS
/**
 * @brief 阻塞毫秒延时
 * @param ms 延时毫秒数
 * @note 阻塞 CPU，精度依赖 MCU 主频
 */
void PubDelayMs(unsigned int ms);
#endif

/*================== 秒计数器 ==================*/
#if OPEN_ALL_FUNCS || OPEN_EXPIRED_MS || OPEN_EXPIRED_PTR_MS
/**
 * @brief 秒计数器
 * @return 秒
 */
unsigned long PubTimerSec_GetCount(void);
#endif

/*================== 毫秒计数器 ==================*/
#if OPEN_ALL_FUNCS || OPEN_EXPIRED_MS || OPEN_EXPIRED_PTR_MS
/**
 * @brief 毫秒计数器
 * @return 毫秒
 */
unsigned long PubTimerMs_GetCount(void);
#endif

/*================== 非阻塞超时检测接口 ==================*/
#if OPEN_ALL_FUNCS || OPEN_EXPIRED_SEC

/**
 * @brief 秒级超时检测
 * @param last_time 上次记录秒计数
 * @param offset 检测间隔秒数
 * @return TIMEOUT_YES 已超时 / TIMEOUT_NO 未超时
 */
unsigned char PubTimerSecExpired(unsigned long last_time, unsigned long offset);
#endif

#if OPEN_ALL_FUNCS || OPEN_EXPIRED_MS

/**
 * @brief 微秒计数器
 * @return 微秒
 */
unsigned int PubTimerMs_GetCount(void);

/**
 * @brief 毫秒级超时检测
 * @param last_time 上次记录毫秒计数
 * @param offset 检测间隔毫秒数
 * @return TIMEOUT_YES 已超时 / TIMEOUT_NO 未超时
 */
unsigned char PubTimerMsExpired(unsigned long last_time, unsigned long offset);
#endif

#if OPEN_ALL_FUNCS || OPEN_EXPIRED_PTR_MS
/**
 * @brief 毫秒级超时检测 (指针)
 * @param last_time 上次记录毫秒计数
 * @param offset 检测间隔毫秒数
 * @return TIMEOUT_YES 已超时 / TIMEOUT_NO 未超时
 */
unsigned char PubTimerPtrMsExpired(unsigned long *lastTime, unsigned long offset);
#endif

#if OPEN_ALL_FUNCS || OPEN_EXPIRED_PTR_SEC
/**
 * @brief 秒级超时检测 (指针)
 * @param last_time 上次记录秒计数
 * @param offset 检测间隔秒数
 * @return TIMEOUT_YES 已超时 / TIMEOUT_NO 未超时
 */
unsigned char PubTimerPtrSecExpired(unsigned long *lastTime, unsigned long offset);
#endif

#if OPEN_ALL_FUNCS || OPEN_REMAIN_SEC
/**
 * @brief   计算还剩多少秒（未超时返回剩余秒，超时返回0）
 * @param   lastTime  上次记录秒计数
 * @param   offset    超时时间间隔(秒)
 * @return  剩余秒数，超时返回0
 */

unsigned long PubTimerSecRemain(unsigned long lastTime, unsigned long offset);
#endif

#endif /* _PUB_TIMES_H_ */
