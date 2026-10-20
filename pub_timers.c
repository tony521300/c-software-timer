/**
 * @file    pub_timers.c
 * @brief   通用延时与超时检测模块（非阻塞优先）
 * @author  Cnn
 * @date    2025-08-21
 * @date    2026-05-30  update  PubTime_1ms_Task()
 * @note   定时器里 每1ms调用 PubTime_1ms_Task() 支持微秒、毫秒、秒级延时与超时溢出
 */

#include "pub_timers.h"

#include "pub_timers_config.h"

/*================== 全局计数器 ==================*/

/*
 * 全局计数器结构体 由PubTime_1ms_Task()更新
 * TimesVarialbes PubTimesVar = {0};
 */
typedef struct
{
    unsigned long ms;     // 毫秒计数
    unsigned long second; // 秒计数
    unsigned long minute; // 分钟计数
} TimesVarialbes;

// 全局 Timers 计数器
TimesVarialbes PubTimesVar = {0};

// 定时器里 每1ms调用
void PubTime_1ms_Task(void)
{
    static unsigned int timer_1ms_counter = 0;
    static unsigned char timer_1s_counter = 0;

    PubTimesVar.ms++;
    timer_1ms_counter++;
    if (timer_1ms_counter >= 1000) {
        timer_1ms_counter = 0;

        PubTimesVar.second++;
        timer_1s_counter++;
        if (timer_1s_counter >= 60) {
            timer_1s_counter = 0;
            PubTimesVar.minute++;
        }
    }
}

#if (OPEN_ALL_FUNCS || OPEN_FORMAT_TIME || OPEN_TIME_DISP)

/**
 * @brief   将总秒数转换为 时:分:秒
 */
void TimeFromSeconds(unsigned long total_seconds,
                     unsigned int *hour,
                     unsigned char *minute,
                     unsigned char *second)
{
    if (!hour || !minute || !second) return;

    *hour   = total_seconds / 3600;
    *minute = (total_seconds % 3600) / 60;
    *second = total_seconds % 60;
}
#endif

#if (OPEN_ALL_FUNCS || OPEN_TIME_DISP)
/**
 * @brief   转换为6位数组 [HH:MM:SS] → [h十,h个,m十,m个,s十,s个]
 */
void TimeToDisplayArray(unsigned long total_seconds, unsigned char *time_array)
{
    unsigned int h;
    unsigned char m, s;

    TimeFromSeconds(total_seconds, &h, &m, &s);

    time_array[0] = h / 10;
    time_array[1] = h % 10;

    time_array[2] = m / 10;
    time_array[3] = m % 10;

    time_array[4] = s / 10;
    time_array[5] = s % 10;
}

#endif

/*================== 延时函数 ==================*/
#if OPEN_ALL_FUNCS || OPEN_DELAY_US || OPEN_DELAY_MS
/**
 * @brief 阻塞式微秒延时
 * @param us 延时微秒数
 * @note 精度依赖 MCU 主频，阻塞 CPU
 */
void PubDelayUs(unsigned int us)
{
    volatile unsigned int i;
    volatile unsigned char j;
    while (us--) {
        for (i = 0; i < TIMER_DELAY_MS; i++) {
            j++;
        }
    }
}
#endif

#if OPEN_ALL_FUNCS || OPEN_DELAY_MS
/**
 * @brief 阻塞式毫秒延时
 * @param ms 延时毫秒数
 * @note 精度依赖 MCU 主频，阻塞 CPU
 */
void PubDelayMs(unsigned int ms)
{
    while (ms--) {
        PubDelayUs(1000);
    }
}
#endif

/*================== 通用超时检测 ==================*/
#if OPEN_ALL_FUNCS || OPEN_EXPIRED_SEC || OPEN_EXPIRED_MS || OPEN_EXPIRED_PTR_MS || OPEN_EXPIRED_PTR_SEC
/**
 * @brief 通用32位超时检测
 * @param current 当前计数值
 * @param lastTime 上次记录计数
 * @param offset 超时时间间隔
 * @return TIMEOUT_YES 已超时 / TIMEOUT_NO 未超时
 * @note 自动处理 32 位计数溢出
 */
static unsigned char ExpiredTimer(unsigned long current, unsigned long lastTime, unsigned long offset)
{
    unsigned long diff = (unsigned long)(current - lastTime);
    if (diff >= offset) {
        return TIMEOUT_YES;
    } else {
        return TIMEOUT_NO;
    }
}
#endif

/*================== 秒计数器 ==================*/
#if OPEN_ALL_FUNCS || OPEN_EXPIRED_SEC || OPEN_EXPIRED_PTR_SEC
/**
 * @brief 秒计数器
 * @return 秒
 */
unsigned long PubTimerSec_GetCount(void)
{
    return PubTimesVar.second;
}
#endif

/*================== 毫秒计数器 ==================*/
#if OPEN_ALL_FUNCS || OPEN_EXPIRED_MS || OPEN_EXPIRED_PTR_MS
/**
 * @brief 毫秒计数器
 * @return 毫秒
 */
unsigned long PubTimerMs_GetCount(void)
{
    return PubTimesVar.ms;
}
#endif

/*================== 秒级超时 ==================*/
#if OPEN_ALL_FUNCS || OPEN_EXPIRED_SEC

/**
 * @brief 秒级超时检测
 * @param lastTime 上次记录秒计数
 * @param offset 检测秒数间隔
 * @return TIMEOUT_YES 已超时 / TIMEOUT_NO 未超时
 */
unsigned char PubTimerSecExpired(unsigned long lastTime, unsigned long offset)
{
    return ExpiredTimer(PubTimesVar.second, lastTime, offset);
}
#endif

/*================== 毫秒级超时 ==================*/
#if OPEN_ALL_FUNCS || OPEN_EXPIRED_MS

/**
 * @brief 毫秒级超时检测
 * @param lastTime 上次记录毫秒计数
 * @param offset 检测毫秒数间隔
 * @return TIMEOUT_YES 已超时 / TIMEOUT_NO 未超时
 */
unsigned char PubTimerMsExpired(unsigned long lastTime, unsigned long offset)
{
    return ExpiredTimer(PubTimesVar.ms, lastTime, offset);
}
#endif

#if OPEN_ALL_FUNCS || OPEN_EXPIRED_PTR_MS
/**
 * @brief 毫秒级超时检测 (指针)
 * @param last_time 上次记录毫秒计数
 * @param offset 检测间隔毫秒数
 * @return TIMEOUT_YES 已超时 / TIMEOUT_NO 未超时
 */
unsigned char PubTimerPtrMsExpired(unsigned long *lastTime, unsigned long offset)
{
    if (ExpiredTimer(PubTimesVar.ms, *lastTime, offset)) {
        *lastTime = PubTimesVar.ms;
        return TIMEOUT_YES;
    }
    return TIMEOUT_NO;
}
#endif

#if OPEN_ALL_FUNCS || OPEN_EXPIRED_PTR_SEC
/**
 * @brief 秒级超时检测 (指针)
 * @param last_time 上次记录秒计数
 * @param offset 检测间隔秒数
 * @return TIMEOUT_YES 已超时 / TIMEOUT_NO 未超时
 */
unsigned char PubTimerPtrSecExpired(unsigned long *lastTime, unsigned long offset)
{
    if (ExpiredTimer(PubTimesVar.second, *lastTime, offset)) {
        *lastTime = PubTimesVar.second;
        return TIMEOUT_YES;
    }
    return TIMEOUT_NO;
}
#endif

#if OPEN_ALL_FUNCS || OPEN_REMAIN_SEC
/**
 * @brief   计算还剩多少秒
 * @param   lastTime  上次记录秒计数
 * @param   offset    超时时间间隔(秒)
 * @return  剩余秒数，超时返回0
 */
unsigned long PubTimerSecRemain(unsigned long lastTime, unsigned long offset)
{
    unsigned long diff = (unsigned long)(PubTimesVar.second - lastTime);

    return (diff >= offset) ? 0 : (offset - diff);
}
#endif
