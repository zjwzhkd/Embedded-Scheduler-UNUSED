/*******************************************************************************
* 文 件 名: sched_api.c
* 创 建 者: Keda Huang
* 版    本: V1.0
* 创建日期: 2016-08-03
* 文件说明: 实现事件驱动调度器的API函数
*******************************************************************************/

#include "sched.h"
#include "sched_framework.h"

/*******************************************************************************

                                  调度核心管理

*******************************************************************************/
void sched_Init(void)
{
    framework_CoreInit();
}

void sched_Start(void)
{
    framework_CoreStart();
}

#if SCHED_TASK_EN
/*******************************************************************************

                                    任务管理

*******************************************************************************/
SchedTaskHandle_t sched_TaskCreate(uint8_t prio, EvtPos_t queueLen, SchedStateFunction_t initial)
{
    return ((SchedTaskHandle_t)framework_TaskCreate(prio, queueLen, initial));
}

#if SCHED_TASK_CYCLE_EN
void sched_TaskSetCyclePeriod(SchedTaskHandle_t task, SchedTick_t period, SchedBool_t immedTRIG)
{
    framework_TaskSetCyclePeriod((SchedTask_t *)task, period, immedTRIG);
}

SchedTick_t sched_TaskGetCycleTick(SchedTaskHandle_t task)
{
    return framework_TaskGetCycleTick((SchedTask_t *)task);
}
#endif  /* SCHED_TASK_CYCLE_EN */

/*******************************************************************************

                                    事件管理

*******************************************************************************/
SchedStatus_t sched_EventSend(SchedTaskHandle_t task, EvtSig_t evtSig, EvtMsg_t evtMsg)
{
SchedEvent_t event;

    event.sig = evtSig;
    event.msg = evtMsg;
    return framework_EventSend((SchedTask_t *)task, &event);
}

SchedStatus_t sched_EventSendFront(SchedTaskHandle_t task, EvtSig_t evtSig, EvtMsg_t evtMsg)
{
SchedEvent_t event;

    event.sig = evtSig;
    event.msg = evtMsg;
    return framework_EventSendFront((SchedTask_t *)task, &event);
}

SchedStatus_t sched_EventSendFromISR(SchedTaskHandle_t task, EvtSig_t evtSig, EvtMsg_t evtMsg)
{
SchedEvent_t event;

    event.sig = evtSig;
    event.msg = evtMsg;
    return framework_EventSendFromISR((SchedTask_t *)task, &event);
}

SchedStatus_t sched_EventSendFrontFromISR(SchedTaskHandle_t task, EvtSig_t evtSig, EvtMsg_t evtMsg)
{
SchedEvent_t event;

    event.sig = evtSig;
    event.msg = evtMsg;
    return framework_EventSendFrontFromISR((SchedTask_t *)task, &event);
}

#if SCHED_TASK_ALARM_EN
/*******************************************************************************

                                    闹钟管理

*******************************************************************************/
SchedAlarmHandle_t sched_AlarmCreate(SchedTaskHandle_t task, EvtSig_t evtSig, EvtMsg_t evtMsg)
{
SchedEvent_t event;

    event.sig = evtSig;
    event.msg = evtMsg;
    return ((SchedAlarmHandle_t)framework_AlarmCreate((SchedTask_t *)task, &event));
}

void sched_AlarmSetEvent(SchedAlarmHandle_t alarm, EvtSig_t evtSig, EvtMsg_t evtMsg)
{
SchedEvent_t event;

    event.sig = evtSig;
    event.msg = evtMsg;
    framework_AlarmSetEvent((SchedAlarm_t *)alarm, &event);
}

void sched_AlarmCancel(SchedAlarmHandle_t alarm)
{
    framework_AlarmCancel((SchedAlarm_t *)alarm);
}

void sched_AlarmSet(SchedAlarmHandle_t alarm, SchedTick_t period)
{
    framework_AlarmSet((SchedAlarm_t *)alarm, period);
}

SchedStatus_t sched_AlarmGetStatus(SchedAlarmHandle_t alarm)
{
    return framework_AlarmGetStatus((SchedAlarm_t *)alarm);
}
#endif  /* SCHED_TASK_ALARM_EN */

#endif  /* SCHED_TASK_EN */

#if SCHED_DAEMON_EN
/*******************************************************************************

                                  守护任务管理

*******************************************************************************/
SchedDaemonHandle_t sched_DaemonCreate(SchedDaemonFunction_t daemonFunc)
{
    return ((SchedDaemonHandle_t)framework_DaemonCreate(daemonFunc));
}

void sched_DaemonAbort(SchedDaemonHandle_t daemon)
{
    framework_DaemonAbort((SchedDaemon_t *)daemon);
}

SchedStatus_t sched_DaemonCall(SchedDaemonHandle_t daemon, EvtSig_t evtSig, EvtMsg_t evtMsg, SchedTick_t delay)
{
SchedEvent_t event;

    event.sig = evtSig;
    event.msg = evtMsg;
    return framework_DaemonCall((SchedDaemon_t *)daemon, &event, delay);
}

SchedStatus_t sched_DaemonGetStatus(SchedDaemonHandle_t daemon)
{
    return framework_DaemonGetStatus((SchedDaemon_t *)daemon);
}

SchedStatus_t sched_DaemonCallFromISR(SchedDaemonHandle_t daemon, EvtSig_t evtSig, EvtMsg_t evtMsg, SchedTick_t delay)
{
SchedEvent_t event;

    event.sig = evtSig;
    event.msg = evtMsg;
    return framework_DaemonCallFromISR((SchedDaemon_t *)daemon, &event, delay);
}

SchedStatus_t sched_DaemonGetStatusFromISR(SchedDaemonHandle_t daemon)
{
    return framework_DaemonGetStatusFromISR((SchedDaemon_t *)daemon);
}

#endif  /* SCHED_DAEMON_EN */
