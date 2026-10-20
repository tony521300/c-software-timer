/**
 * @file    pub_timers_config.h
 * @brief   定时器相关接口配置头文件
 *
 * - 需要在项目中维护全局计数变量：
 * - PubTime_1ms_Task(); 定时器里 每1ms调用
 * - 提供阻塞延时函数（微秒/毫秒）及非阻塞超时检测接口

*/

#ifndef _PUB_TIMES_CONFIG_H_
#define _PUB_TIMES_CONFIG_H_

#ifdef __C51__ // 8051 编译器
#define FLASH_CODE  code
#define FLASH_XDATA xdata
#else // ARM / STM32
#define FLASH_CODE
#define FLASH_XDATA
#endif

/*================== 函数启用配置开关 ==================*/

// 启用全部函数
#define OPEN_ALL_FUNCS 0

/** 格式化时间 : 0-禁用, 1-启用 时-分-秒*/
#define OPEN_FORMAT_TIME 0 // TimeFromSeconds(total, &h, &m, &s)

/** 格式化时间成数组 : 0-禁用, 1-启用 时-分-秒*/
#define OPEN_TIME_DISP 0 // TimeToDisplayArray(total, &time_array)

/** 微秒级延时开关: 0-禁用, 1-启用 */
#define OPEN_DELAY_US 0 // PubDelayUs(us)

/** 毫秒级延时开关: 0-禁用, 1-启用 */
#define OPEN_DELAY_MS 1 // PubDelayMs(ms)

/** 毫秒级超时检测: 0-禁用, 1-启用 */
#define OPEN_EXPIRED_MS 0 // PubTimerMsExpired(lastTime, offset)

/** 秒级超时检测: 0-禁用, 1-启用 */
#define OPEN_EXPIRED_SEC 0 // PubTimerSecExpired(lastTime, offset)

/** 指针 毫秒级超时检测: 0-禁用, 1-启用 */
#define OPEN_EXPIRED_PTR_MS 1 // PubTimerPtrMsExpired(&lastTime, offset)

/** 指针 秒级超时检测: 0-禁用, 1-启用 */
#define OPEN_EXPIRED_PTR_SEC 1 // PubTimerPtrSecExpired(&lastTime, offset)

/**
 * @brief   计算还剩多少秒（未超时返回剩余秒，超时返回0） 0-禁用, 1-启用
 * @param   lastTime  上次记录秒计数
 * @param   offset    超时时间间隔(秒)
 * @return  剩余秒数，超时返回0
 */
#define OPEN_REMAIN_SEC 1 //  unsigned int PubTimerSecRemain(lastTime, offset)

/*================== 参数配置 ==================*/

/** MCU 12MHz 时的毫秒延时计数 */
#define TIMER_DELAY_MS 12

/** 超时状态 */
#define TIMEOUT_YES 1

#define TIMEOUT_NO  0

#endif