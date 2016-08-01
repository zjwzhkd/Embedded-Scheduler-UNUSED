/*******************************************************************************
* 文 件 名: sched_event_table.c
* 创 建 者: Keda Huang
* 版    本: V1.0
* 创建日期: 2016-08-01
* 文件说明: 实现事件驱动调度器的核心框架 - 事件管理(记录表)
*******************************************************************************/

#include "sched_framework.h"

#if SCHED_TASK_EN && (SCHED_TASK_EVENT_METHOD == 0)
/*******************************************************************************

                                    操作函数

*******************************************************************************/

/**
 * 说明: 1.向指定任务传递一个事件
 *       2.使用记录表存储事件
 *
 * 参数: 1.task - 目标任务控制块指针
 *       2.evt  - 待传递的事件块指针
 *
 * 返回: 1.SCHED_SUCCESS           表示发送成功
 *       2.SCHED_EVENT_SEND_FAILED 表示发送失败
 */
SchedStatus_t framework_EventSend(SchedTask_t *task, SchedEvent_t const *evt)
{
SchedCPU_t cpu_sr;

    SCHED_ASSERT(NULL != task,errSCHED_PARAM_PTR_IS_NULL);
    SCHED_ASSERT(evt->sig >= SCHED_SIG_USER,errSCHED_EVENT_SEND_NOT_USER_SIGNAL);
    SCHED_ASSERT(SCHED_CORE_RUNNING == framework_CoreStatus,errSCHED_EVENT_SEND_BEFORE_CORE_RUNNING);

    cpu_sr = SCHED_EnterCritical(); /*进入临界区*/
    {
        __framework_TaskRecordReadyTask(task);
        internal_PriotblRecordPrio(&task->sigtbl, (uint8_t)(evt->sig - SCHED_SIG_USER));
    }
    SCHED_ExitCritical(cpu_sr);     /*退出临界区*/

    return (SCHED_SUCCESS);
}

/**
 * 说明: 1.向指定任务传递一个紧急事件
 *       2.使用记录表存储事件
 *
 * 参数: 1.task - 目标任务控制块指针
 *       2.evt  - 待传递的事件块指针
 *
 * 返回: 1.SCHED_SUCCESS           表示发送成功
 *       2.SCHED_EVENT_SEND_FAILED 表示发送失败
 */
SchedStatus_t framework_EventSendFront(SchedTask_t *task, SchedEvent_t const *evt)
{
    return framework_EventSend(task, evt);
}

/**
 * 说明: 1.在中断函数中向指定任务传递一个事件
 *       2.使用记录表存储事件
 *
 * 参数: 1.task - 目标任务控制块指针
 *       2.evt  - 待传递的事件块指针
 *
 * 返回: 1.SCHED_SUCCESS           表示发送成功
 *       2.SCHED_EVENT_SEND_FAILED 表示发送失败
 */
SchedStatus_t framework_EventSendFromISR(SchedTask_t *task, SchedEvent_t const *evt)
{
SchedStatus_t ret;
SchedCPU_t cpu_sr;

    if (SCHED_CORE_RUNNING == framework_CoreStatus)
    {
        SCHED_ASSERT(NULL != task,errSCHED_PARAM_PTR_IS_NULL);
        SCHED_ASSERT(evt->sig >= SCHED_SIG_USER,errSCHED_EVENT_SEND_NOT_USER_SIGNAL);

        cpu_sr = SCHED_EnterCriticalFromISR();  /*进入临界区*/
        {
            __framework_TaskRecordReadyTask(task);
            internal_PriotblRecordPrio(&task->sigtbl, (uint8_t)(evt->sig - SCHED_SIG_USER));
        }
        SCHED_ExitCriticalFromISR(cpu_sr);      /*退出临界区*/
        ret = SCHED_SUCCESS;
    }
    else
    {
        ret = SCHED_EVENT_SEND_FAILED;
    }

    return (ret);
}

/**
 * 说明: 1.在中断函数中向指定任务传递一个紧急事件
 *       2.使用记录表存储事件
 *
 * 参数: 1.task - 目标任务控制块指针
 *       2.evt  - 待传递的事件块指针
 *
 * 返回: 1.SCHED_SUCCESS           表示发送成功
 *       2.SCHED_EVENT_SEND_FAILED 表示发送失败
 */
SchedStatus_t framework_EventSendFrontFromISR(SchedTask_t *task, SchedEvent_t const *evt)
{
    return framework_EventSendFromISR(task, evt);
}

/*******************************************************************************

                                    内部函数

*******************************************************************************/

/**
 * 说明: 1.尝试接收指定任务的事件块(实际上没有接收)
 *       2.使用记录表存储事件
 *
 * 参数: task - 指定的任务控制块指针
 *
 * 返回: 1.SCHED_SUCCESS              表示接收成功
 *       2.SCHED_EVENT_RECEIVE_FAILED 表示接收失败
 */
SchedStatus_t __framework_EventTryReceive(SchedTask_t *task)
{
SchedStatus_t ret;

    if (SCHED_FALSE != internal_PriotblIsEmpty(&task->sigtbl))
    {
        ret = SCHED_EVENT_RECEIVE_FAILED;
    }
    else
    {
        ret = SCHED_SUCCESS;
    }
    return (ret);
}

/**
 * 说明: 1.从指定任务中接收一个事件
 *       2.使用记录表存储事件
 *
 * 参数: 1.task - 指定的任务控制块指针
 *       2.evt  - 保存事件的事件块指针
 *
 * 返回: 1.SCHED_SUCCESS              表示接收成功
 *       2.SCHED_EVENT_RECEIVE_FAILED 表示接收失败
 */
SchedStatus_t __framework_EventReceive(SchedTask_t *task, SchedEvent_t *evt)
{
SchedStatus_t ret;
uint8_t sig;

    if (SCHED_FALSE != internal_PriotblIsEmpty(&task->sigtbl))
    {
        ret = SCHED_EVENT_RECEIVE_FAILED;
    }
    else
    {
        sig = internal_PriotblGetHighestPrio(&task->sigtbl);
        internal_PriotblResetPrio(&task->sigtbl,sig);
        evt->sig = (EvtSig_t)sig + SCHED_SIG_USER;
        evt->msg = 0;
        ret = SCHED_SUCCESS;
    }
    return (ret);
}

#endif  /* #if SCHED_TASK_EN && (SCHED_TASK_EVENT_METHOD == 0) */
