/*******************************************************************************
* 文 件 名: sched.h
* 创 建 者: Keda Huang
* 版    本: V1.0
* 创建日期: 2016-08-03
* 文件说明: 事件驱动调度器的API函数定义
*******************************************************************************/

#ifndef __SCHED_H
#define __SCHED_H

#include "sched_port.h"

/*******************************************************************************

                                  调度核心管理

*******************************************************************************/
/**
 * 调度器初始化
 *
 * @note: 在执行任何API前必须先调用本函数
 */
void sched_Init(void);

/**
 * 启动调度器
 *
 * @note: 调用本函数后永远不会返回
 */
void sched_Start(void);

#if SCHED_TASK_EN
/*******************************************************************************

                                    任务管理

*******************************************************************************/
/**
 * 创建一个任务并返回任务句柄, 仅允许在调用sched_Start()启动调度器前创建新任务
 *
 * @param prio: 任务优先级(0 - SCHED_LOWEST_PRIORITY)
 *
 * @param queueLen: 配置SCHED_TASK_EVENT_METHOD>=1时, 表示消息队列的长度;
 *                  配置SCHED_TASK_EVENT_METHOD =0时, 参数queueLen无效
 *
 * @param initial: 状态机初始伪状态函数
 *
 * @return: 返回任务句柄, 若为NULL则表示创建失败
 */
SchedTaskHandle_t sched_TaskCreate(uint8_t prio, EvtPos_t queueLen, SchedStateFunction_t initial);

#if SCHED_TASK_CYCLE_EN
/**
 * 设置任务的周期循环信号触发周期, 并复位周期信号节拍计数
 *
 * @param task: 指定任务的任务句柄
 *
 * @param period: 设置周期循环信号触发的周期, 0表示不产生周期循环信号
 *
 * @param immedTRIG: 布尔值(SCHED_TRUE/SCHED_FALSE),
 *                   SCHED_TRUE  表示立即触发信号,
 *                   SCHED_FALSE 表示第一个周期结束时触发第一个信号
 */
void sched_TaskSetCyclePeriod(SchedTaskHandle_t task, SchedTick_t period, SchedBool_t immedTRIG);

/**
 * 获取任务的周期循环信号节拍计数
 *
 * @param task: 指定任务的任务句柄
 *
 * @return: 返回指定任务的周期循环信号节拍计数
 */
SchedTick_t sched_TaskGetCycleTick(SchedTaskHandle_t task);
#endif  /* SCHED_TASK_CYCLE_EN */

/*******************************************************************************

                                    事件管理

*******************************************************************************/
/**
 * 向指定任务传递一个事件
 *
 * @param task: 目标任务的任务句柄
 *
 * @param evtSig: 待传递的事件信号
 *
 * @param evtMsg: 待传递的事件消息, 若配置SCHED_TASK_EVENT_METHOD=0, 参数无效
 *
 * @return: SCHED_SUCCESS           表示发送成功
 *          SCHED_EVENT_SEND_FAILED 表示发送失败
 */
SchedStatus_t sched_EventSend(SchedTaskHandle_t task, EvtSig_t evtSig, EvtMsg_t evtMsg);

/**
 * 向指定任务传递一个紧急事件
 *
 * @param task: 目标任务的任务句柄
 *
 * @param evtSig: 待传递的事件信号
 *
 * @param evtMsg: 待传递的事件消息, 若配置SCHED_TASK_EVENT_METHOD=0, 参数无效
 *
 * @return: SCHED_SUCCESS           表示发送成功
 *          SCHED_EVENT_SEND_FAILED 表示发送失败
 */
SchedStatus_t sched_EventSendFront(SchedTaskHandle_t task, EvtSig_t evtSig, EvtMsg_t evtMsg);

/**
 * 在中断函数中向指定任务传递一个事件
 *
 * @param task: 目标任务的任务句柄
 *
 * @param evtSig: 待传递的事件信号
 *
 * @param evtMsg: 待传递的事件消息, 若配置SCHED_TASK_EVENT_METHOD=0, 参数无效
 *
 * @return: SCHED_SUCCESS           表示发送成功
 *          SCHED_EVENT_SEND_FAILED 表示发送失败
 */
SchedStatus_t sched_EventSendFromISR(SchedTaskHandle_t task, EvtSig_t evtSig, EvtMsg_t evtMsg);

/**
 * 在中断函数中向指定任务传递一个紧急事件
 *
 * @param task: 目标任务的任务句柄
 *
 * @param evtSig: 待传递的事件信号
 *
 * @param evtMsg: 待传递的事件消息, 若配置SCHED_TASK_EVENT_METHOD=0, 参数无效
 *
 * @return: SCHED_SUCCESS           表示发送成功
 *          SCHED_EVENT_SEND_FAILED 表示发送失败
 */
SchedStatus_t sched_EventSendFrontFromISR(SchedTaskHandle_t task, EvtSig_t evtSig, EvtMsg_t evtMsg);

#if SCHED_TASK_ALARM_EN
/*******************************************************************************

                                    闹钟管理

*******************************************************************************/
/**
 * 创建新闹钟, 仅允许在调用sched_Start()启动调度器前创建闹钟
 *
 * @param task: 闹钟目标任务的任务句柄
 *
 * @param evtSig: 闹钟到时触发的事件信号
 *
 * @param evtMsg: 闹钟到时触发的事件消息, 若配置SCHED_TASK_EVENT_METHOD=0, 参数无效
 *
 * @return: 返回闹钟句柄, 若返回NULL表示创建失败
 */
SchedAlarmHandle_t sched_AlarmCreate(SchedTaskHandle_t task, EvtSig_t evtSig, EvtMsg_t evtMsg);

/**
 * 设置闹钟事件
 *
 * @param alarm: 待操作的闹钟句柄
 *
 * @param evtSig: 闹钟到时触发的事件信号
 *
 * @param evtMsg: 闹钟到时触发的事件消息, 若配置SCHED_TASK_EVENT_METHOD=0, 参数无效
 */
void sched_AlarmSetEvent(SchedAlarmHandle_t alarm, EvtSig_t evtSig, EvtMsg_t evtMsg);

/**
 * 取消闹钟响应, 必须在调度器启动后调用
 *
 * @param alarm: 待操作的闹钟句柄
 */
void sched_AlarmCancel(SchedAlarmHandle_t alarm);

/**
 * 设置并重启闹钟, 必须在调度器启动后调用
 *
 * @param alarm: 待操作的闹钟句柄
 *
 * @param period: 闹钟到时周期, 若为0则立即触发闹钟事件
 */
void sched_AlarmSet(SchedAlarmHandle_t alarm, SchedTick_t period);

/**
 * 获取闹钟状态
 *
 * @param alarm: 待操作的闹钟句柄
 *
 * @return: SCHED_ALARM_STOP    表示闹钟停止(闹钟已取消)
 *          SCHED_ALARM_RUNNING 表示闹钟正在运行
 *          SCHED_ALARM_ARRIVED 表示闹钟已到时(此时闹钟停止)
 */
SchedStatus_t sched_AlarmGetStatus(SchedAlarmHandle_t alarm);
#endif  /* SCHED_TASK_ALARM_EN */

#endif  /* SCHED_TASK_EN */

#if SCHED_DAEMON_EN
/*******************************************************************************

                                  守护任务管理

*******************************************************************************/
/**
 * 创建新的守护任务, 仅允许在调用sched_Start()启动调度器前创建守护任务
 *
 * @param daemonFunc: 守护任务函数指针
 *
 * @return: 返回守护任务句柄, 若返回NULL表示创建失败
 */
SchedDaemonHandle_t sched_DaemonCreate(SchedDaemonFunction_t daemonFunc);

/**
 * 终止指定的守护任务, 使得指定的守护任务进入休眠状态
 *
 * @param daemon: 守护任务句柄
 */
void sched_DaemonAbort(SchedDaemonHandle_t daemon);

/**
 * 唤醒守护任务并执行给定的事件
 *
 * @note: 当守护任务处于休眠状态(SCHED_DAEMON_DORMANT)或者
 *        运行状态(SCHED_DAEMON_RUNNING)时,允许唤醒守护任务
 *
 * @param daemon: 待唤醒的守护任务句柄
 *
 * @param evtSig: 守护任务唤醒的事件信号
 *
 * @param evtMsg: 守护任务唤醒的事件消息
 *
 * @param delay: 守护任务执行延时, 延时为0表示守护任务立即就绪
 *
 * @return: SCHED_SUCCESS            表示守护任务唤醒成功
 *          SCHED_DAEMON_CALL_FAILED 表示守护任务唤醒失败
 */
SchedStatus_t sched_DaemonCall(SchedDaemonHandle_t daemon, EvtSig_t evtSig, EvtMsg_t evtMsg, SchedTick_t delay);

/**
 * 获取指定守护任务的状态
 *
 * @param daemon: 守护任务句柄
 *
 * @return: SCHED_DAEMON_RUNNING 表示守护任务正在运行
 *          SCHED_DAEMON_ACTIVE  表示守护任务已被唤醒
 *          SCHED_DAEMON_DORMANT 表示守护任务处于休眠
 */
SchedStatus_t sched_DaemonGetStatus(SchedDaemonHandle_t daemon);

/**
 * 在中断函数中唤醒守护任务并执行给定的事件
 *
 * @note: 仅当守护任务处于休眠状态(SCHED_DAEMON_DORMANT), 允许唤醒守护任务
 *
 * @param daemon: 待唤醒的守护任务句柄
 *
 * @param evtSig: 守护任务唤醒的事件信号
 *
 * @param evtMsg: 守护任务唤醒的事件消息
 *
 * @param delay: 守护任务执行延时, 延时为0表示守护任务立即就绪
 *
 * @return: SCHED_SUCCESS            表示守护任务唤醒成功
 *          SCHED_DAEMON_CALL_FAILED 表示守护任务唤醒失败
 */
SchedStatus_t sched_DaemonCallFromISR(SchedDaemonHandle_t daemon, EvtSig_t evtSig, EvtMsg_t evtMsg, SchedTick_t delay);

/**
 * 在中断函数中获取指定守护任务的状态
 *
 * @param daemon: 守护任务句柄
 *
 * @return: SCHED_DAEMON_RUNNING 表示守护任务正在运行
 *          SCHED_DAEMON_ACTIVE  表示守护任务已被唤醒
 *          SCHED_DAEMON_DORMANT 表示守护任务处于休眠
 */
SchedStatus_t sched_DaemonGetStatusFromISR(SchedDaemonHandle_t daemon);
#endif  /* SCHED_DAEMON_EN */

#endif  /* __SCHED_H */
