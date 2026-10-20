# PubTimers 时间管理模块 (VER 2.1)

## 模块简介

`PubTimers` 是一个通用的 **嵌入式时间管理模块**，用于实现：

- 阻塞延时（ms）
- 非阻塞定时（ms / s）
- 周期任务调度基础
- 自动处理计数器溢出

适用于 **8051 / STM32 / 通用 MCU 项目**

---

## 模块特点

- 支持 **阻塞 + 非阻塞** 两种时间控制方式
- 推荐使用 **指针方式自动更新时间**
- 支持 **毫秒 / 秒级任务调度**
- 自动兼容 **计数器溢出**
- 通过宏控制 **代码裁剪**

---

## 文件结构

```
PubTimers/
├── pub_timers.c         // 模块实现
├── pub_timers.h         // 对外接口
└── pub_timers_config.h  // 功能配置
```

---

## 功能开关配置（pub_timers_config.h）

### 1 全部功能开关

| 宏               | 说明         | 默认 |
| ---------------- | ------------ | ---- |
| `OPEN_ALL_FUNCS` | 打开全部功能 | 0    |

---

### 2 单功能开关

| 宏                     | 功能           | 默认 |
| ---------------------- | -------------- | ---- |
| `OPEN_FORMAT_TIME`     | 秒 → 时分秒    | 0    |
| `OPEN_TIME_DISP`       | 转显示数组     | 0    |
| `OPEN_DELAY_MS`        | 毫秒延时       | 1    |
| `OPEN_EXPIRED_MS`      | ms超时检测     | 0    |
| `OPEN_EXPIRED_SEC`     | s超时检测      | 0    |
| `OPEN_EXPIRED_PTR_MS`  | ms超时（指针） | 1    |
| `OPEN_EXPIRED_PTR_SEC` | s超时（指针）  | 1    |
| `OPEN_REMAIN_SEC`      | 剩余时间计算   | 1    |

---

## 参数配置

| 宏               | 说明                       | 默认 |
| ---------------- | -------------------------- | ---- |
| `TIMER_DELAY_MS` | 延时循环系数（与主频相关） | 12   |
| `TIMEOUT_YES`    | 超时返回值                 | 1    |
| `TIMEOUT_NO`     | 未超时返回值               | 0    |

---

## 核心数据结构

```c
typedef struct {
    unsigned long ms;     // 毫秒计数
    unsigned long second; // 秒计数
    unsigned long minute; // 分钟计数
} TimesVarialbes;

extern TimesVarialbes PubTimesVar;
```

---

## 时间基准（必须实现）

### 1ms 定时任务（核心）

```c
void PubTime_1ms_Task(void);
```

必须在 **定时器中断中每 1ms 调用一次**

---

## 功能接口说明

---

### 1. 时间格式化

```c
void TimeFromSeconds(unsigned long total,
                     unsigned int *h,
                     unsigned char *m,
                     unsigned char *s);
```

功能：

- 秒 → 时 / 分 / 秒

---

### 2. 时间转显示数组

```c
void TimeToDisplayArray(unsigned long total,
                        unsigned char *buf);
```

输出格式：

```
HH:MM:SS → [h十, h个, m十, m个, s十, s个]
```

---

### 3. 阻塞延时（慎用）

```c
void PubDelayMs(unsigned int ms);
```

特点：

- 占用 CPU
- 不适用于实时任务或中断环境

---

### 4. 非阻塞超时检测（核心）

---

#### 推荐：指针版本（自动更新时间）

```c
unsigned char PubTimerPtrMsExpired(unsigned long *last, unsigned long ms);
unsigned char PubTimerPtrSecExpired(unsigned long *last, unsigned long sec);
```

优点：

- 自动更新时间
- 使用简单
- 不易出错

---

#### 普通版本（需手动更新）

```c
unsigned char PubTimerMsExpired(unsigned long last, unsigned long ms);
unsigned char PubTimerSecExpired(unsigned long last, unsigned long sec);
```

---

### 5. 剩余时间计算

```c
unsigned long PubTimerSecRemain(unsigned long last, unsigned long sec);
```

用途：

- 倒计时显示
- UI 进度控制

---

## 使用示例

---

### 示例：1ms 定时器

```c
void Timer_ISR(void) interrupt 1
{
    PubTime_1ms_Task();
}
```

---

### 示例：主循环任务调度

```c
void main(void)
{
    unsigned long led_timer = 0;
    unsigned long print_timer = 0;

    while (1)
    {
        // 每500ms执行
        if (PubTimerPtrMsExpired(&led_timer, 500)) {
            LED_Toggle(); // 闪烁
        }

        // 每2秒执行
        if (PubTimerPtrSecExpired(&print_timer, 2)) {

        }
    }
}
```

---

## 应用场景

| 场景       | 推荐接口 |
| ---------- | -------- |
| 按键消抖   | ms超时   |
| LED闪烁    | ms / s   |
| 状态机节拍 | ms       |
| 通信超时   | ms       |
| 倒计时显示 | 剩余时间 |

---

## 使用注意事项

### 1 时间基准必须稳定

`PubTime_1ms_Task()` 必须 **严格 1ms 调用**

否则会导致：

- 时间不准
- 超时判断错误

---

### 2 推荐使用指针版本

```c
// 推荐
PubTimerPtrMsExpired(&t, 100);

// 不推荐
if (PubTimerMsExpired(t,100)) t = PubTimesVar.ms;
```

---

### 3 关于计数器溢出

内部使用：

```c
(now - last) >= offset
```

自动支持溢出，无需额外处理

---

## 许可证

- 可自由使用 / 修改
- 无限制

---

## 总结

该模块核心就是：

> **“基于 1ms 时基的软件定时器系统”**

使用三步：

1. 定时器中断 → 每 1ms 调用 `PubTime_1ms_Task`
2. 主循环 → 使用 `PtrExpired` 做调度
3. 避免滥用阻塞延时

---
