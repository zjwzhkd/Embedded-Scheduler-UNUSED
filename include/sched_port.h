/*******************************************************************************
* 文 件 名: sched_port.h
* 创 建 者: Keda Huang
* 版    本: V1.0
* 创建日期: 2016-07-25
* 文件说明: 事件驱动调度器的底层接口
*******************************************************************************/

#ifndef __SCHED_PORT_H
#define __SCHED_PORT_H

/* 头文件 --------------------------------------------------------------------*/
#include "cpu.h"
#include "sched_config.h"
#include <stdint.h>
#include <stddef.h>

/* 底层移植接口 --------------------------------------------------------------*/
/*基本类型*/
typedef base_t  SchedBase_t;
/*体系类型*/
typedef cpu_t   SchedCPU_t;
/*临界区管理*/
#define SCHED_EnterCritical()           CPU_EnterCritical()
#define SCHED_ExitCritical(x)           CPU_ExitCritical(x)
#define SCHED_EnterCriticalFromISR()    CPU_EnterCriticalFromISR()
#define SCHED_ExitCriticalFromISR(x)    CPU_ExitCriticalFromISR(x)
/*中断检测*/
/*
    如果微控制器不支持中断环境判断,
    则定义:
        #define SCHED_IN_INTERRUPT()    ( -1 )
*/
#ifdef CPU_InHandlerMode
    #define SCHED_IN_INTERRUPT()        CPU_InHandlerMode()
#else
    #define SCHED_IN_INTERRUPT()        ( -1 )
#endif

/* 调度器数据类型 ------------------------------------------------------------*/
/*节拍类型*/
#if SCHED_USE_16BIT_TICK_EN
    typedef uint16_t SchedTick_t;
    #define SCHED_MAX_TICK  ( (SchedTick_t)0xFFFF )
#else
    typedef uint32_t SchedTick_t;
    #define SCHED_MAX_TICK  ( (SchedTick_t)0xFFFFFFFF )
#endif

/*布尔类型*/
typedef enum {SCHED_FALSE = 0, SCHED_TRUE = 1}  SchedBool_t;

/*调度器状态类型*/
typedef enum sched_status SchedStatus_t;

/*事件块类型*/
typedef uint8_t     EvtPos_t;   /*消息队列偏移量类型*/
typedef uint16_t    EvtSig_t;   /*事件块信号数据类型*/
typedef uint32_t    EvtMsg_t;   /*事件块消息数据类型*/
typedef struct sched_event SchedEvent_t;
struct sched_event
{
    EvtSig_t    sig;    /*信号*/
    EvtMsg_t    msg;    /*消息*/
};

/*任务句柄*/
typedef void *  SchedTaskHandle_t;

/*闹钟句柄*/
typedef void *  SchedAlarmHandle_t;

/*守护任务句柄*/
typedef void *  SchedDaemonHandle_t;

/*状态函数*/
typedef SchedBase_t (*SchedStateFunction_t)(SchedTaskHandle_t me, SchedEvent_t const *e);

/*守护任务函数*/
typedef void (*SchedDaemonFunction_t)(SchedDaemonHandle_t me, SchedEvent_t const *e);

/* 调度器常量 ----------------------------------------------------------------*/
/*状态函数返回常量*/
#define SCHED_RET_HANDLED   ( (SchedBase_t) 0 )
#define SCHED_RET_IGNORED   ( (SchedBase_t) 1 )
#define SCHED_RET_TRAN      ( (SchedBase_t) 2 )

/*内部信号常量*/
enum {
    SCHED_SIG_EMPTY = 0,    /*初始化空信号*/
    SCHED_SIG_ENTRY,        /*状态进入信号*/
    SCHED_SIG_EXIT,         /*状态退出信号*/
    SCHED_SIG_CYCLE,        /*周期循环信号*/
    SCHED_SIG_USER,         /*自定义信号  */
};

/*调度器状态值*/
enum sched_status
{
    SCHED_SUCCESS = 0,
    SCHED_CORE_UNKNOWN,
    SCHED_CORE_STOP,
    SCHED_CORE_RUNNING,
    SCHED_EVENT_SEND_FAILED,
    SCHED_EVENT_RECEIVE_FAILED,
    SCHED_ALARM_STOP,
    SCHED_ALARM_RUNNING,
    SCHED_ALARM_ARRIVED,
    SCHED_DAEMON_LIST_PLACE_NONE,
    SCHED_DAEMON_LIST_PLACE_READY,
    SCHED_DAEMON_LIST_PLACE_DELAY,
    SCHED_DAEMON_LIST_PLACE_EVENT,
    SCHED_DAEMON_IDLE,
    SCHED_DAEMON_BLOCKED,
    SCHED_DAEMON_READY,
    SCHED_DAEMON_RUNNING,

    errSCHED_PARAM_PTR_IS_NULL = 64,
    errSCHED_PARAM_NOT_ALLOWED,
    errSCHED_LIST_ERROR,
    errSCHED_PRIOTBL_ERROR,
    errSCHED_NOT_IN_INTERRUPT,
    errSCHED_FSM_INITIAL_NOT_TRAN,
    errSCHED_FSM_ENTRY_TRAN,
    errSCHED_FSM_EXIT_TRAN,
    errSCHED_CORE_START_BEFORE_INIT,
    errSCHED_TASK_PRIO_OVER_LOWEST,
    errSCHED_TASK_PRIO_IS_ALLOCATED,
    errSCHED_TASK_NOT_CREATED_BEFORE_CORE_RUNNING,
    errSCHED_TASK_NOT_EXISTED,
    errSCHED_EVENT_SEND_NOT_USER_SIGNAL,
    errSCHED_EVENT_SEND_BEFORE_CORE_RUNNING,

    chkSCHED_MALLOC_FAILED = 128,
    chkSCHED_TYPE_CONVERSION_FAILED,
    chkSCHED_EVENT_SEND_FAILED,
};

/* 调度器宏定义 --------------------------------------------------------------*/
/*状态函数返回宏*/
#define SCHED_HANDLED()         ( SCHED_RET_HANDLED )
#define SCHED_IGNORED()         ( SCHED_RET_IGNORED )
#define SCHED_TRAN(target)      ( *((SchedStateFunction_t *)me) = (target), SCHED_RET_TRAN )

/*状态函数宏函数*/
#define SCHED_THIS_TASK()       ( (SchedTaskHandle_t)me )
#define SCHED_THIS_STATE()      ( *(SchedStateFunction_t *)me )
#define SCHED_MS_TO_TICK(nms)   ( (SchedTick_t)((uint32_t)(nms)*SCHED_TICK_HZ/1000) )
#define SCHED_HZ_TO_TICK(nhz)   ( (SchedTick_t)(SCHED_TICK_HZ/(nhz)) )

/* 内部宏定义 ----------------------------------------------------------------*/
#if SCHED_ASSERT_EN
    #define SCHED_ASSERT(expr, errCode) \
        if (!(expr)) {sched_PortErrorHandler(errCode);}
    #define SCHED_ASSERT_IN_INTERRUPT() \
        SCHED_ASSERT(SCHED_IN_INTERRUPT(),errSCHED_NOT_IN_INTERRUPT)
#else
    #define SCHED_ASSERT(expr, errCode) ((void)0)
    #define SCHED_ASSERT_IN_INTERRUPT() ((void)0)
#endif

#if SCHED_CHECK_EN
    #define SCHED_CHECK(expr, chkCode) \
        if (!(expr)) {sched_PortErrorHandler(chkCode);}
#else
    #define SCHED_CHECK(expr, chkCode) ((void)0)
#endif

/* 底层函数 ------------------------------------------------------------------*/
/*调度器底层初始化*/
void sched_PortInit(void);
/*事件块复制*/
void sched_PortEventCopy(SchedEvent_t *dest, SchedEvent_t const *src);
/*动态内存分配*/
void *sched_PortMalloc(size_t size);
/*动态内存释放*/
void sched_PortFree(void *pv);
/*调度器错误处理函数*/
void sched_PortErrorHandler(SchedStatus_t errCode);
/*调度器空闲处理函数*/
void sched_PortIdleHandler(void);

#endif  /* __SCHED_PORT_H */
