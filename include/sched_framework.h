/*******************************************************************************
* 文 件 名: sched_framework.h
* 创 建 者: Keda Huang
* 版    本: V1.0
* 创建日期: 2016-07-26
* 文件说明: 定义事件驱动调度器的核心框架
*******************************************************************************/

#ifndef __SCHED_FRAMEWORK_H
#define __SCHED_FRAMEWORK_H

#include "sched_port.h"
#include "sched_internal.h"

/*******************************************************************************

                                    调度核心

*******************************************************************************/
/* 全局变量 ------------------------------------------------------------------*/
/*调度器核心当前状态*/
extern SchedStatus_t framework_CoreStatus;

/* 操作函数 ------------------------------------------------------------------*/
/*调度器初始化*/
void framework_CoreInit(void);
/*启动调度器*/
void framework_CoreStart(void);
/*调度器节拍中断*/
void framework_CoreTickHandler(void);

/* 内部函数 ------------------------------------------------------------------*/
/*
    向时间管理器添加延时对象
    确保添加的延时对象链表项是孤立的!
    若延时时间为0,则不执行任何操作
*/
void __framework_CoreTimeManagerAddDelay(SchedList_t *pListItem, SchedTick_t delay);
/*
    更新时间管理器,用于优化节拍中断执行效率
    当对象链表项可能从延时链表中删除时,建议调用本函数
*/
void __framework_CoreTimeManagerUpdate(void);

#if SCHED_TASK_EN
/*******************************************************************************

                                     状态机

*******************************************************************************/
/* 全局变量 ------------------------------------------------------------------*/
/*状态机内部事件*/
extern const SchedEvent_t internal_event[4];

/* 数据结构 ------------------------------------------------------------------*/
typedef struct sched_fsm SchedFSM_t;
struct sched_fsm
{
    SchedStateFunction_t    state;  /*有限状态机当前状态*/
};

/* 操作函数 ------------------------------------------------------------------*/
/*构造状态机*/
void framework_FSM_Ctor(SchedFSM_t *fsm, SchedStateFunction_t initial);
/*初始化状态机*/
void framework_FSM_Init(SchedFSM_t *fsm);
/*状态机处理事件*/
void framework_FSM_Dispatch(SchedFSM_t *fsm, SchedEvent_t const *e);

/*******************************************************************************

                                    任务管理

*******************************************************************************/
/* 数据结构 ------------------------------------------------------------------*/
typedef struct sched_task SchedTask_t;
struct sched_task
{
    SchedFSM_t              fsm;            /*有限状态机,偏移量必须为0  */
#if SCHED_TASK_EVENT_METHOD == 0            /*使用优先级记录表记录事件  */
    SchedPrioTable_t        sigtbl;         /*记录事件的优先级记录表    */
#else                                       /*使用消息队列记录事件      */
    SchedQueue_t            queue;          /*记录事件的消息队列        */
#endif

    uint8_t                 prio;           /*任务优先级,0为最高优先级  */

#if SCHED_TASK_CYCLE_EN
    uint8_t     volatile    cycleFlag;      /*周期循环信号触发标志      */
    SchedTick_t             cyclePeriod;    /*周期循环信号产生的周期    */
    SchedTick_t volatile    cycleTick;      /*周期循环信号节拍计数      */
    SchedList_t             cycleListItem;  /*周期循环信号对象管理链表项*/
#endif
};

/* 操作函数 ------------------------------------------------------------------*/
/*任务管理环境初始化*/
void framework_TaskEnvirInit(void);
/*创建新任务*/
SchedTask_t *framework_TaskCreate(uint8_t prio, EvtPos_t queueLen, SchedStateFunction_t initial);

#if SCHED_TASK_CYCLE_EN
/*
    设置任务周期循环信号产生的周期,
    周期为0表示不产生周期循环信号,
    复位周期循环信号节拍计数,
    可以选择是否立即产生信号(节拍0对应的信号)
*/
void framework_TaskSetCyclePeriod(SchedTask_t *task, SchedTick_t period, SchedBool_t immedTRIG);
/*获取周期循环信号节拍计数*/
SchedTick_t framework_TaskGetCycleTick(SchedTask_t *task);
#endif

/*初始化所有任务*/
void framework_TaskInitialiseAll(void);
/*
    任务调度函数,
    返回SCHED_TRUE表示完成一次任务调度,
    返回SCHED_FALSE表示没有就绪任务,进行了一次空操作
*/
SchedBool_t framework_TaskExecute(void);

/* 内部函数 ------------------------------------------------------------------*/
/*记录就绪任务*/
void __framework_TaskRecordReadyTask(SchedTask_t const *task);
/*清除就绪任务*/
void __framework_TaskResetReadyTask(SchedTask_t const *task);

#if SCHED_TASK_CYCLE_EN
/*
    时间管理器的延时对象到时回调函数,
    返回0表示时间管理器无进一步动作,
    返回非0表示时间管理器将当前对象重新加入延时链表,返回值是延时时间
*/
SchedTick_t __framework_TaskTimeArrivalHandler(SchedList_t *pArrivalListItem);
#endif

/*******************************************************************************

                                    事件管理

*******************************************************************************/
/* 操作函数 ------------------------------------------------------------------*/
/*向指定任务发送事件块*/
SchedStatus_t framework_EventSend(SchedTask_t *task, SchedEvent_t const *evt);
/*向指定任务发送紧急事件块*/
SchedStatus_t framework_EventSendFront(SchedTask_t *task, SchedEvent_t const *evt);

/*在中断函数中向指定任务发送事件块*/
SchedStatus_t framework_EventSendFromISR(SchedTask_t *task, SchedEvent_t const *evt);
/*在中断函数中向指定任务发送紧急事件块*/
SchedStatus_t framework_EventSendFrontFromISR(SchedTask_t *task, SchedEvent_t const *evt);

/* 内部函数 ------------------------------------------------------------------*/
/*尝试接收指定任务的事件块(实际上没有接收)*/
SchedStatus_t __framework_EventTryReceive(SchedTask_t *task);
/*接收指定任务的事件块*/
SchedStatus_t __framework_EventReceive(SchedTask_t *task, SchedEvent_t *evt);

#if SCHED_TASK_ALARM_EN
/*******************************************************************************

                                    闹钟管理

*******************************************************************************/
/* 数据结构 ------------------------------------------------------------------*/
typedef struct sched_alarm SchedAlarm_t;
struct sched_alarm
{
    SchedTask_t            *task;           /*闹钟所属任务指针  */
    SchedEvent_t            event;          /*闹钟到时事件      */
    SchedList_t             alarmListItem;  /*闹钟对象管理链表项*/
    uint8_t      volatile   flag;           /*闹钟到时标志      */
};

/* 操作函数 ------------------------------------------------------------------*/
/*闹钟管理环境初始化*/
void framework_AlarmEnvirInit(void);
/*创建新闹钟*/
SchedAlarm_t *framework_AlarmCreate(SchedTask_t *task, SchedEvent_t const *evt);

/*设置闹钟事件*/
void framework_AlarmSetEvent(SchedAlarm_t *alarm, SchedEvent_t const *evt);
/*取消闹钟响应*/
void framework_AlarmCancel(SchedAlarm_t *alarm);
/*设置并重启闹钟*/
void framework_AlarmSet(SchedAlarm_t *alarm, SchedTick_t period);
/*获取闹钟状态*/
SchedStatus_t framework_AlarmGetStatus(SchedAlarm_t *alarm);

/* 内部函数 ------------------------------------------------------------------*/
/*
    时间管理器的延时对象到时回调函数,
    返回0表示时间管理器无进一步动作,
    返回非0表示时间管理器将当前对象重新加入延时链表,返回值是延时时间
*/
SchedTick_t __framework_AlarmTimeArrivalHandler(SchedList_t *pArrivalListItem);

#endif  /* SCHED_TASK_ALARM_EN */

#endif  /* SCHED_TASK_EN */

#if SCHED_DAEMON_EN
/*******************************************************************************

                                  守护任务管理

*******************************************************************************/
/* 全局变量 ------------------------------------------------------------------*/
/*当前运行的守护任务*/
extern struct sched_daemon *framework_daemonCurrentRunning;

/* 数据结构 ------------------------------------------------------------------*/
typedef struct sched_daemon SchedDaemon_t;
struct sched_daemon
{
    SchedDaemonFunction_t   daemonFunc;     /*守护任务处理函数*/
    SchedEvent_t            event;          /*守护任务响应事件*/
    SchedList_t             daemonListItem; /*对象管理链表项  */
    SchedStatus_t           listPlace;      /*对象链表项位置  */
    uint8_t                 prio;           /*守护任务优先级  */
};

/* 常量定义 ------------------------------------------------------------------*/
/*守护任务最低优先级*/
#define SCHED_DAEMON_LOWEST_PRIO        ( (uint8_t)0xFF )

/* 操作函数 ------------------------------------------------------------------*/
/*守护任务管理环境初始化*/
void framework_DaemonEnvirInit(void);
/*创建新守护任务*/
SchedDaemon_t *framework_DaemonCreate(uint8_t prio, SchedDaemonFunction_t daemonFunc);

/*唤醒守护任务并执行给定的事件*/
SchedStatus_t framework_DaemonCall(SchedDaemon_t *daemon, SchedEvent_t const *evt, SchedTick_t delay);
/*终止指定的守护任务*/
SchedStatus_t framework_DaemonAbort(SchedDaemon_t *daemon);
/*获取指定守护任务的状态*/
SchedStatus_t framework_DaemonGetStatus(SchedDaemon_t *daemon);

/*在中断函数中唤醒守护任务并执行给定的事件*/
SchedStatus_t framework_DaemonDelayCall(SchedDaemon_t *daemon, SchedEvent_t const *evt, SchedTick_t delay);
/*在中断函数中终止指定的守护任务*/
SchedStatus_t framework_DaemonAbortFromISR(SchedDaemon_t *daemon);
/*在中断函数中获取指定守护任务的状态*/
SchedStatus_t framework_DaemonGetStatusFromISR(SchedDaemon_t *daemon);

/*
    守护任务调度函数,
    返回SCHED_TRUE表示完成一次任务调度,
    返回SCHED_FALSE表示没有就绪任务,进行了一次空操作
*/
SchedBool_t framework_DaemonExecute(void);

/* 内部函数 ------------------------------------------------------------------*/
/*将守护任务添加到事件等待链表,确保链表项是孤立的*/
void __framework_DaemonPlaceOnEventList(SchedList_t *pEventList, SchedDaemon_t *daemon, SchedEvent_t const *evt);
/*将指定的任务从事件等待链表中移除,并添加至就绪链表*/
void __framework_DaemonRemoveFromEventList(SchedDaemon_t *daemon);
/*移除指定事件链表的所有链表项*/
void __framework_DaemonResetEventList(SchedDaemon_t *pEventList);
/*
    时间管理器的延时对象到时回调函数,
    返回0表示时间管理器无进一步动作,
    返回非0表示时间管理器将当前对象重新加入延时链表,返回值是延时时间
*/
SchedTick_t __framework_DaemonTimeArrivalHandler(SchedList_t *pArrivalListItem);

#endif  /* SCHED_DAEMON_EN */

#endif  /* __SCHED_FRAMEWORK_H */
