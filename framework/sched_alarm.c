/*******************************************************************************
* 文 件 名: sched_alarm.c
* 创 建 者: Keda Huang
* 版    本: V1.0
* 创建日期: 2016-08-02
* 文件说明: 实现事件驱动调度器的核心框架 - 闹钟管理
*******************************************************************************/

#include "sched_framework.h"

#if SCHED_TASK_EN && SCHED_TASK_ALARM_EN
/*******************************************************************************

                                    操作函数

*******************************************************************************/

/**
 * 创建新闹钟, 仅允许在调度器启动前创建闹钟
 *
 * @param task: 闹钟目标任务控制块指针
 *
 * @param evt: 闹钟到时触发的事件
 *
 * @return: 若创建成功, 返回闹钟控制块指针
 *          若创建失败, 返回NULL
 */
SchedAlarm_t *framework_AlarmCreate(SchedTask_t *task, SchedEvent_t const *evt)
{
SchedAlarm_t *pAlarm = NULL;

    /*参数检验*/
    SCHED_ASSERT(NULL != task,errSCHED_PARAM_PTR_IS_NULL);
    SCHED_ASSERT(evt->sig >= SCHED_SIG_USER,errSCHED_ALARM_EVENT_NOT_USER_SIGNAL);
    SCHED_ASSERT(SCHED_CORE_STOP == framework_CoreStatus,errSCHED_ALARM_NOT_CREATED_BEFORE_CORE_RUNNING);
    /*分配闹钟控制块*/
    pAlarm = (SchedAlarm_t *)sched_PortMalloc(sizeof(SchedAlarm_t));
    if (NULL != pAlarm)
    {
        pAlarm->task = task;
        pAlarm->flag = 0;
        sched_PortEventCopy(&pAlarm->event,evt);
        internal_ListInit(&pAlarm->alarmListItem, SCHED_LIST_ALARM);
    }
    return (pAlarm);
}

/**
 * 设置闹钟事件
 *
 * @param alarm: 闹钟控制块指针
 *
 * @param evt: 闹钟到时触发的事件
 */
void framework_AlarmSetEvent(SchedAlarm_t *alarm, SchedEvent_t const *evt)
{
SchedCPU_t cpu_sr;

    SCHED_ASSERT(NULL != alarm,errSCHED_PARAM_PTR_IS_NULL);
    SCHED_ASSERT(evt->sig >= SCHED_SIG_USER,errSCHED_ALARM_EVENT_NOT_USER_SIGNAL);

    cpu_sr = SCHED_EnterCritical(); /*进入临界区*/
    {
        sched_PortEventCopy(&alarm->event,evt);
    }
    SCHED_ExitCritical(cpu_sr);     /*退出临界区*/
}

/**
 * 取消闹钟响应, 必须在调度器启动后调用
 *
 * @param alarm: 闹钟控制块指针
 */
void framework_AlarmCancel(SchedAlarm_t *alarm)
{
SchedCPU_t cpu_sr;

    SCHED_ASSERT(NULL != alarm,errSCHED_PARAM_PTR_IS_NULL);
    SCHED_ASSERT(SCHED_CORE_RUNNING == framework_CoreStatus,errSCHED_ALARM_OPERATED_BEFORE_CORE_RUNNING);

    cpu_sr = SCHED_EnterCritical(); /*进入临界区*/
    {
        alarm->flag = 0;
        internal_ListRemove(&alarm->alarmListItem);
        __framework_CoreTimeManagerUpdate();
    }
    SCHED_ExitCritical(cpu_sr);     /*退出临界区*/
}

/**
 * 设置并重启闹钟, 必须在调度器启动后调用
 *
 * @param alarm: 闹钟控制块指针
 *
 * @param period: 闹钟到时周期, 若为0则立即触发闹钟事件
 */
void framework_AlarmSet(SchedAlarm_t *alarm, SchedTick_t period)
{
SchedCPU_t cpu_sr;

    SCHED_ASSERT(NULL != alarm,errSCHED_PARAM_PTR_IS_NULL);
    SCHED_ASSERT(SCHED_CORE_RUNNING == framework_CoreStatus,errSCHED_ALARM_OPERATED_BEFORE_CORE_RUNNING);

    cpu_sr = SCHED_EnterCritical(); /*进入临界区*/
    {
        alarm->flag = 0;
        internal_ListRemove(&alarm->alarmListItem);
        if (period > 0)
        {
            __framework_CoreTimeManagerAddDelay(&alarm->alarmListItem, period);
        }
        else
        {
            framework_EventSend(alarm->task,&alarm->event);
        }
        __framework_CoreTimeManagerUpdate();
    }
    SCHED_ExitCritical(cpu_sr);     /*退出临界区*/
}

/**
 * 获取闹钟状态
 *
 * @param alarm: 闹钟控制块指针
 *
 * @return: SCHED_ALARM_STOP    表示闹钟停止(闹钟已取消)
 *          SCHED_ALARM_RUNNING 表示闹钟正在运行
 *          SCHED_ALARM_ARRIVED 表示闹钟已到时(此时闹钟停止)
 */
SchedStatus_t framework_AlarmGetStatus(SchedAlarm_t *alarm)
{
SchedCPU_t      cpu_sr;
SchedStatus_t   ret;

    SCHED_ASSERT(NULL != alarm,errSCHED_PARAM_PTR_IS_NULL);

    cpu_sr = SCHED_EnterCritical(); /*进入临界区*/
    {
        if (SCHED_FALSE == internal_ListIsEmpty(&alarm->alarmListItem))
        {
            ret = SCHED_ALARM_RUNNING;
        }
        else if (0 == alarm->flag)
        {
            ret = SCHED_ALARM_STOP;
        }
        else
        {
            ret = SCHED_ALARM_ARRIVED;
        }
    }
    SCHED_ExitCritical(cpu_sr);     /*退出临界区*/

    return (ret);
}

/*******************************************************************************

                                    内部函数

*******************************************************************************/

/**
 * 时间管理器的对象延时到时回调函数
 *
 * @param pArrivalListItem: 结束延时的对象的链表项指针
 *
 * @return: 返回0表示时间管理器无进一步动作,
 *          返回非零值表示时间管理器将当前对象重新加入延时链表,返回值是延时时间
 */
SchedTick_t __framework_AlarmTimeArrivalHandler(SchedList_t *pArrivalListItem)
{
SchedAlarm_t *pAlarm;

    pAlarm = internal_ListEntry(pArrivalListItem,SchedAlarm_t,alarmListItem);
    framework_EventSendFromISR(pAlarm->task, &pAlarm->event);
    return (0);
}

#endif  /* SCHED_TASK_EN && SCHED_TASK_ALARM_EN */
