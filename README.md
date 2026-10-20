<div align="center">

# PubTimers

**通用嵌入式时间管理模块 · Generic Embedded Time Management Module**

[English](#english) | [中文](#中文)

`阻塞延时` · `非阻塞定时` · `周期任务调度` · `自动处理计数器溢出`  
`Blocking Delay` · `Non-blocking Timer` · `Periodic Scheduling` · `Overflow-safe`

**适用平台 / Targets:** 8051 · STM32 · 通用 MCU / Generic MCU

</div>

---

## 中文

### 特性

- 同时支持 **阻塞** 与 **非阻塞** 两种时间控制方式
- 推荐使用 **指针版本** 接口，自动更新时间戳，杜绝漏更新
- 支持 **毫秒 / 秒级** 两种时间粒度
- 基于无符号减法比较，**天然兼容计数器溢出**
- 通过宏开关逐功能 **裁剪代码**，减小 Flash 占用
- 仅依赖一个 1 ms 时基，移植成本极低（3 个文件，零外部依赖）

### 文件结构

```
public_timers/
├── pub_timers.c         // 模块实现 / Implementation
├── pub_timers.h         // 对外接口 / Public API
├── pub_timers_config.h  // 功能配置 / Compile-time configuration
└── README.md
```

### 快速开始

**三步接入 / Three steps:**

1. **配置时基 / Configure the time base** — 在 `pub_timers_config.h` 中按需打开功能宏
2. **挂载 1 ms 任务 / Hook the 1 ms tick** — 在定时器中断中每 1 ms 调用一次 `PubTime_1ms_Task()`
3. **主循环调度 / Schedule in the main loop** — 使用 `PubTimerPtr*Expired()` 判断周期到达

```c
/* ① 定时器中断，每 1ms 调用一次 / Timer ISR, called every 1ms */
void Timer_ISR(void) interrupt 1   /* interrupt 1 仅适用于 Keil C51 */
{
    PubTime_1ms_Task();
}
```

```c
/* ② 主循环 / Main loop */
#include "pub_timers.h"

void main(void)
{
    unsigned long led_timer    = 0;
    unsigned long print_timer = 0;

    while (1)
    {
        /* 每 500ms 闪烁一次 / Toggle LED every 500ms */
        if (PubTimerPtrMsExpired(&led_timer, 500)) {
            LED_Toggle();
        }

        /* 每 2 秒执行一次 / Run every 2 seconds */
        if (PubTimerPtrSecExpired(&print_timer, 2)) {
            // TODO: your periodic task
        }
    }
}
```

### 功能开关配置（`pub_timers_config.h`）

#### 1. 总开关

| 宏                | 说明 / Description                     | 默认 / Default |
| :--------------- | :----------------------------------- | :----------: |
| `OPEN_ALL_FUNCS` | 打开全部功能 / Enable all features at once |      `0`     |

#### 2. 单功能开关

| 宏                      | 功能 / Feature              | 对应接口 / API                |  默认 |
| :--------------------- | :------------------------ | :------------------------ | :-: |
| `OPEN_DELAY_US`        | 微秒级延时 / Microsecond delay | `PubDelayUs()`            | `0` |
| `OPEN_DELAY_MS`        | 毫秒级延时 / Millisecond delay | `PubDelayMs()`            | `1` |
| `OPEN_FORMAT_TIME`     | 秒 → 时分秒 / Seconds → H:M:S | `TimeFromSeconds()`       | `0` |
| `OPEN_TIME_DISP`       | 转显示数组 / To display digits | `TimeToDisplayArray()`    | `0` |
| `OPEN_EXPIRED_MS`      | 毫秒超时检测（值传递）               | `PubTimerMsExpired()`     | `0` |
| `OPEN_EXPIRED_SEC`     | 秒超时检测（值传递）                | `PubTimerSecExpired()`    | `0` |
| `OPEN_EXPIRED_PTR_MS`  | 毫秒超时（指针，推荐）               | `PubTimerPtrMsExpired()`  | `1` |
| `OPEN_EXPIRED_PTR_SEC` | 秒超时（指针，推荐）                | `PubTimerPtrSecExpired()` | `1` |
| `OPEN_REMAIN_SEC`      | 剩余时间计算 / Remaining time   | `PubTimerSecRemain()`     | `1` |

> 置 `OPEN_ALL_FUNCS` 为 `1` 可一次性打开全部功能，随后再逐个关闭不需要的项。

#### 3. 参数配置

| 宏                | 说明 / Description                   | 默认 / Default |
| :--------------- | :--------------------------------- | :----------: |
| `TIMER_DELAY_MS` | 延时循环系数，**与主频强相关，需实测标定**            | `12`（12 MHz） |
| `TIMEOUT_YES`    | 超时返回值 / Returned on timeout        |      `1`     |
| `TIMEOUT_NO`     | 未超时返回值 / Returned when not expired |      `0`     |

### API 参考

#### 时间基准（必须实现 / Required）

```c
void PubTime_1ms_Task(void);
```

**必须在定时器中断中严格每 1 ms 调用一次。**  
Must be called from a timer interrupt at a **strict 1 ms** interval.

#### 推荐接口：指针版本（自动更新时间）

```c
unsigned char PubTimerPtrMsExpired (unsigned long *lastTime, unsigned long offset_ms);
unsigned char PubTimerPtrSecExpired(unsigned long *lastTime, unsigned long offset_sec);
```

超时后自动将 `*lastTime` 更新为当前计数值，返回 `TIMEOUT_YES`；否则返回 `TIMEOUT_NO`。  
Updates `*lastTime` automatically when expired. Returns `TIMEOUT_YES` / `TIMEOUT_NO`.

#### 普通接口：值传递（需手动更新）

```c
unsigned char PubTimerMsExpired (unsigned long lastTime, unsigned long offset_ms);
unsigned char PubTimerSecExpired(unsigned long lastTime, unsigned long offset_sec);
```

> ⚠️ **不推荐**：需调用方自行更新 `lastTime`，极易漏写。  
> Not recommended — the caller must update `lastTime` manually, which is error-prone.

```c
/* 反例 / Anti-pattern */
if (PubTimerMsExpired(t, 100)) {
    t = PubTimerMs_GetCount();   /* 忘了这行 → 逻辑永久失效 */
}
```

#### 读取当前计数值

```c
unsigned long PubTimerMs_GetCount(void);   /* 当前毫秒计数 / current ms count  */
unsigned long PubTimerSec_GetCount(void);  /* 当前秒计数   / current second count */
```

#### 剩余时间计算

```c
unsigned long PubTimerSecRemain(unsigned long lastTime, unsigned long offset_sec);
```

已超时返回 `0`，否则返回剩余秒数。用于倒计时显示与 UI 进度控制。  
Returns `0` if already expired, otherwise the remaining seconds.

#### 阻塞延时（慎用 / Use with care）

```c
void PubDelayUs(unsigned int us);
void PubDelayMs(unsigned int ms);
```

> ⚠️ 纯软件空转，**占用 CPU 且精度依赖主频与编译器优化等级**。  
> 不可用于中断上下文或实时任务，精确延时请改用硬件定时器。  
> Software busy-loop. Occupies the CPU; **do not** call from ISRs or real-time tasks.

#### 时间格式化

```c
void TimeFromSeconds(unsigned long total_seconds,
                     unsigned int  *hour,     /* [out] 时 / hours   */
                     unsigned char *minute,   /* [out] 分 / minutes */
                     unsigned char *second);  /* [out] 秒 / seconds */

void TimeToDisplayArray(unsigned long total_seconds, unsigned char *time_array);
```

`TimeToDisplayArray()` 输出 6 位数字，便于直接送显示驱动：

```c
unsigned char buf[6];
TimeToDisplayArray(3661, buf);   /* 01:01:01 -> {0,1,0,1,0,1} */
```

### 应用场景

| 场景 / Scenario                | 推荐接口 / Recommended API |
| :--------------------------- | :--------------------- |
| 按键消抖 / Key debouncing        | 毫秒超时（指针）               |
| LED 闪烁 / LED blinking        | 毫秒 / 秒超时（指针）           |
| 状态机节拍 / State machine tick   | 毫秒超时（指针）               |
| 通信超时 / Communication timeout | 毫秒超时（指针）               |
| 倒计时显示 / Countdown display    | `PubTimerSecRemain()`  |

### 移植说明

1. 将三个文件加入工程（Keil / IAR / GCC 均可）
2. 在 `pub_timers_config.h` 中选择所需功能
3. 依据实际主频**标定 `TIMER_DELAY_MS`**（见下方注意事项 3）
4. 在 1 ms 定时器中断中调用 `PubTime_1ms_Task()`

---

### 注意事项 / Notes

**1. 时基必须稳定 — 最高优先级**

`PubTime_1ms_Task()` 必须严格 1 ms 调用一次。中断被长时间屏蔽、主循环中有长延时、或中断优先级配置不当，都会导致计时漂移与超时误判。  
`PubTime_1ms_Task()` must be called at a strict 1 ms interval. Long interrupt blocking or delays in the main loop will cause drift and false timeout decisions.

**2. 优先使用指针版本**

指针版自动更新时间戳，是本模块的**推荐用法**；值传递版本需调用方手动更新，容易出错。

**3. `TIMER_DELAY_MS` 必须实测标定**

默认值 `12` 仅适用于 **12 MHz** 主频。延时精度受主频、编译器优化等级、指令集共同影响。更换主频或使用 `-O2` 后，请用示波器/逻辑分析仪实测并重新标定该值，否则阻塞延时误差可能达数十个百分点。

**4. 计数器溢出已自动处理**

内部统一使用无符号减法比较：

```c
(current - lastTime) >= offset
```

`unsigned long` 的回绕（wrap-around）被自然吸收，无需调用方额外干预。但前提是 **`offset` 必须远小于计数器满量程**（32 位下约为 49.7 天），否则单次判断的语义会失真。

**5. 非原子访问**

`PubTimesVar` 为 32 位变量，在 8 位 MCU（8051）上由中断更新、主循环读取时，存在**撕裂读**（torn read）风险。仅读取秒/毫秒计数值时请注意；超时判断接口内部已通过减法比较规避该问题。

**6. 已知代码问题 / Known code issues**

- **重复声明**：当前 `pub_timers.h` 中 `PubTimerMs_GetCount()` 被声明了两次，且返回类型不一致（`unsigned long` 与 `unsigned int`）。同时开启 `OPEN_EXPIRED_MS` 时会触发编译错误，请将第 105 行的重复声明删除。
- **全局变量未导出**：`PubTimesVar` 与 `TimesVarialbes` 仅定义在 `pub_timers.c` 中，头文件未提供 `extern` 声明，因此**外部无法访问**。请统一通过 `PubTimerMs_GetCount()` / `PubTimerSec_GetCount()` 获取计数值。
- **`TimeFromSeconds()` 的 NULL 语义**：实现为「三个指针中任一为 `NULL` 即整体返回」，而非「各自独立判空」。请务必传入三个有效指针。

---

## English

### Features

- Supports both **blocking** and **non-blocking** time control
- Pointer-based API **updates the timestamp automatically** — no missed updates
- Works at **millisecond** and **second** granularity
- **Overflow-safe** by construction (unsigned subtraction comparison)
- **Per-feature compile-time switches** to trim Flash footprint
- Needs only a 1 ms tick — 3 files, zero external dependencies

### File Structure

```
public_timers/
├── pub_timers.c         // Implementation
├── pub_timers.h         // Public API
├── pub_timers_config.h  // Compile-time configuration
└── README.md
```

### Quick Start

**Three steps:**

1. **Configure** — enable the feature macros you need in `pub_timers_config.h`
2. **Hook the 1 ms tick** — call `PubTime_1ms_Task()` from a timer interrupt every 1 ms
3. **Schedule** — use `PubTimerPtr*Expired()` in your main loop

```c
/* ① Timer ISR, called every 1ms — "interrupt 1" is Keil C51 syntax */
void Timer_ISR(void) interrupt 1
{
    PubTime_1ms_Task();
}
```

```c
/* ② Main loop */
#include "pub_timers.h"

void main(void)
{
    unsigned long led_timer    = 0;
    unsigned long print_timer = 0;

    while (1)
    {
        if (PubTimerPtrMsExpired(&led_timer, 500)) {
            LED_Toggle();          /* every 500ms */
        }

        if (PubTimerPtrSecExpired(&print_timer, 2)) {
            // TODO: your periodic task
        }
    }
}
```

### Compile-time Configuration (`pub_timers_config.h`)

#### 1. Master switch

| Macro            | Description                 | Default |
| :--------------- | :-------------------------- | :-----: |
| `OPEN_ALL_FUNCS` | Enable all features at once |   `0`   |

#### 2. Per-feature switches

| Macro                  | Feature                                  | API                       | Default |
| :--------------------- | :--------------------------------------- | :------------------------ | :-----: |
| `OPEN_DELAY_US`        | Microsecond blocking delay               | `PubDelayUs()`            |   `0`   |
| `OPEN_DELAY_MS`        | Millisecond blocking delay               | `PubDelayMs()`            |   `1`   |
| `OPEN_FORMAT_TIME`     | Seconds → H:M:S                          | `TimeFromSeconds()`       |   `0`   |
| `OPEN_TIME_DISP`       | Convert to display digits                | `TimeToDisplayArray()`    |   `0`   |
| `OPEN_EXPIRED_MS`      | ms timeout (by value)                    | `PubTimerMsExpired()`     |   `0`   |
| `OPEN_EXPIRED_SEC`     | s timeout (by value)                     | `PubTimerSecExpired()`    |   `0`   |
| `OPEN_EXPIRED_PTR_MS`  | ms timeout (by pointer, **recommended**) | `PubTimerPtrMsExpired()`  |   `1`   |
| `OPEN_EXPIRED_PTR_SEC` | s timeout (by pointer, **recommended**)  | `PubTimerPtrSecExpired()` |   `1`   |
| `OPEN_REMAIN_SEC`      | Remaining-time calculation               | `PubTimerSecRemain()`     |   `1`   |

#### 3. Parameters

| Macro            | Description                                                          |    Default    |
| :--------------- | :------------------------------------------------------------------- | :-----------: |
| `TIMER_DELAY_MS` | Delay loop coefficient — **frequency dependent, must be calibrated** | `12` (12 MHz) |
| `TIMEOUT_YES`    | Returned when expired                                                |      `1`      |
| `TIMEOUT_NO`     | Returned when not expired                                            |      `0`      |

### API Reference

#### Time Base (required)

```c
void PubTime_1ms_Task(void);
```

Must be called from a timer interrupt at a **strict 1 ms** interval.

#### Recommended: pointer-based (auto-updating)

```c
unsigned char PubTimerPtrMsExpired (unsigned long *lastTime, unsigned long offset_ms);
unsigned char PubTimerPtrSecExpired(unsigned long *lastTime, unsigned long offset_sec);
```

On expiry, `*lastTime` is set to the current counter and `TIMEOUT_YES` is returned; otherwise `TIMEOUT_NO`.

#### By-value variants (manual update required)

```c
unsigned char PubTimerMsExpired (unsigned long lastTime, unsigned long offset_ms);
unsigned char PubTimerSecExpired(unsigned long lastTime, unsigned long offset_sec);
```

> ⚠️ **Not recommended** — forgetting to update `lastTime` silently breaks the logic forever.


```c
/* Anti-pattern */
if (PubTimerMsExpired(t, 100)) {
    t = PubTimerMs_GetCount();   /* omit this line and the task never fires again */
}
```

#### Reading the Current Counters

```c
unsigned long PubTimerMs_GetCount(void);   /* current millisecond count */
unsigned long PubTimerSec_GetCount(void);  /* current second count     */
```

#### Remaining Time

```c
unsigned long PubTimerSecRemain(unsigned long lastTime, unsigned long offset_sec);
```

Returns `0` if already expired, otherwise the number of seconds left. Useful for countdown displays and UI progress.

#### Blocking Delays (use with care)

```c
void PubDelayUs(unsigned int us);
void PubDelayMs(unsigned int ms);
```

> ⚠️ Pure software busy-loop. Occupies the CPU; accuracy depends on clock frequency and compiler optimization level. **Never** call from ISR context or real-time tasks — use a hardware timer for precise delays.

#### Time Formatting

```c
void TimeFromSeconds(unsigned long total_seconds,
                     unsigned int  *hour,     /* [out] hours   */
                     unsigned char *minute,   /* [out] minutes */
                     unsigned char *second);  /* [out] seconds */

void TimeToDisplayArray(unsigned long total_seconds, unsigned char *time_array);
```

`TimeToDisplayArray()` emits 6 digit values ready to hand to a display driver:

```c
unsigned char buf[6];
TimeToDisplayArray(3661, buf);   /* 01:01:01 -> {0,1,0,1,0,1} */
```

### Typical Use Cases

| Scenario              | Recommended API          |
| :-------------------- | :----------------------- |
| Key debouncing        | ms timeout (pointer)     |
| LED blinking          | ms / s timeout (pointer) |
| State machine tick    | ms timeout (pointer)     |
| Communication timeout | ms timeout (pointer)     |
| Countdown display     | `PubTimerSecRemain()`    |

### Porting

1. Add the three files to your project (Keil / IAR / GCC)
2. Select the required features in `pub_timers_config.h`
3. **Calibrate `TIMER_DELAY_MS`** for your actual clock frequency (see Note 3)
4. Call `PubTime_1ms_Task()` from your 1 ms timer interrupt

---

### Notes

**1. The time base must be stable — highest priority**

`PubTime_1ms_Task()` must run at a strict 1 ms interval. Long interrupt blocking, long delays in the main loop, or a misconfigured interrupt priority will cause drift and false timeout decisions.

**2. Prefer the pointer-based API**

The pointer variants update the timestamp for you. That is the intended usage of this module; the by-value variants rely on the caller and are easy to get wrong.

**3. `TIMER_DELAY_MS` must be calibrated on your hardware**

The default `12` targets a **12 MHz** clock only. Delay accuracy depends on clock frequency, optimization level (`-O2` changes the generated loop) and instruction set. Re-measure with an oscilloscope or logic analyzer after changing clock or compiler settings — otherwise the error can reach tens of percent.

**4. Counter overflow is handled for you**

All comparisons use unsigned subtraction:

```c
(current - lastTime) >= offset
```

Wrap-around of the `unsigned long` counter is absorbed naturally, with no special handling required. The one precondition: **`offset` must be much smaller than the counter's full range** (~49.7 days at 32 bits), otherwise the comparison semantics break down.

**5. Non-atomic access**

`PubTimesVar` is a 32-bit variable. On 8-bit MCUs (8051) it is updated in an ISR and read in the main loop, which carries a risk of **torn reads**. Be aware of this when reading the raw counters. The timeout APIs avoid the issue internally by using subtraction comparison.

**6. Known code issues**

- **Duplicate declaration**: `pub_timers.h` declares `PubTimerMs_GetCount()` twice with mismatched return types (`unsigned long` vs `unsigned int`). Enabling `OPEN_EXPIRED_MS` triggers a compile error — remove the redundant declaration on line 105.
- **Globals are not exported**: `PubTimesVar` and `TimesVarialbes` are defined only in `pub_timers.c` with no `extern` declaration in the header, so they are **not accessible** from outside the module. Use `PubTimerMs_GetCount()` / `PubTimerSec_GetCount()` instead.
- **`TimeFromSeconds()` NULL semantics**: the implementation bails out if **any** of the three pointers is `NULL` — it does not handle them independently. Always pass three valid pointers.

---

## 许可证 / License

自由使用、修改，无限制 / Free to use and modify, no restrictions.

## 总结 / Summary

本模块的核心就一句话：

> **基于 1 ms 时基的软件定时器系统。**

The whole idea in one line:

> **A software timer system built on a 1 ms time base.**

三步走 / Three steps:

1. 定时器中断 → 每 1 ms 调用 `PubTime_1ms_Task()`
2. 主循环 → 用 `PtrExpired` 做任务调度
3. 避免滥用阻塞延时
