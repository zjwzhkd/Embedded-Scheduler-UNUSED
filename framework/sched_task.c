/*******************************************************************************
* 文 件 名: sched_task.c
* 创 建 者: Keda Huang
* 版    本: V1.0
* 创建日期: 2016-07-29
* 文件说明: 实现事件驱动调度器的核心框架 - 任务管理
*******************************************************************************/

#include "sched_framework.h"

#if SCHED_TASK_EN
/*******************************************************************************

                                    全局变量

*******************************************************************************/
/*任务优先级管理*/
static SchedTask_t * taskPrioGroup[SCHED_LOWEST_PRIORITY+1];
static SchedPrioTable_t taskReadyTable;

static SchedTask_t * prvGetHighestPriorityReadyTask(void);
/*******************************************************************************

                                    操作函数

*******************************************************************************/
/*任务管理环境初始化*/
void framework_TaskEnvirInit(void)
{
uint8_t i;

    for (i=0;i<=SCHED_LOWEST_PRIORITY;i++)
    {
        taskPrioGroup[i] = NULL;
    }
    internal_PriotblInit(&taskReadyTable);
}

/**
 * 说明: 1.创建一个新任务
 *       2.仅允许在调度器启动前创建新任务
 *
 * 参数: 1.prio     - 任务优先级
 *       2.queueLen - 消息队列长度
 *       3.initial  - 初始伪状态函数
 *
 * 返回: 1.NULL   表示任务控制块分配失败
 *       2.非NULL 表示任务控制块指针
 */
SchedTask_t *framework_TaskCreate(uint8_t prio, EvtPos_t queueLen, SchedStateFunction_t initial)
{
SchedTask_t *pTask = NULL;

    /*参数检验*/
    SCHED_ASSERT(prio<=SCHED_LOWEST_PRIORITY,errSCHED_TASK_PRIO_OVER_LOWEST);
    SCHED_ASSERT(SCHED_CORE_STOP == framework_CoreStatus,errSCHED_TASK_NOT_CREATED_BEFORE_CORE_RUNNING);
    /*分配任务控制块*/
    pTask = (SchedTask_t *)sched_PortMalloc(sizeof(SchedTask_t));
    if (NULL != pTask)
    {
        /*构建FSM*/
        framework_FSM_Ctor(&pTask->fsm, initial);
        /*设置优先级*/
        pTask->prio = prio;
        SCHED_ASSERT(NULL == taskPrioGroup[prio],errSCHED_TASK_PRIO_IS_ALLOCATED);
        taskPrioGroup[prio] = pTask;
        /*初始化周期循环信号*/
        #if SCHED_TASK_CYCLE_EN
        {
            pTask->cycleFlag   = 0;
            pTask->cyclePeriod = 0;
            pTask->cycleTick   = 0;
            internal_ListInit(&pTask->cycleListItem, SCHED_LIST_CYCLE);
        }
        #endif
        /*初始化消息队列或事件表*/
        #if SCHED_TASK_EVENT_METHOD == 0
        {
            internal_PriotblInit(&pTask->sigtbl);
        }
        #else
        {
        SchedEvent_t *pEvents = NULL;

            /*分配事件块数组*/
            if (queueLen>0)
            {
                pEvents = (SchedEvent_t *)sched_PortMalloc((size_t)queueLen*sizeof(SchedEvent_t));
            }
            if (NULL == pEvents)
            {
                queueLen = 0;
            }
            /*初始化消息队列*/
            internal_QueueInit(&pTask->queue,pEvents,queueLen);
        }
        #endif
    }
    return (pTask);
}

#if SCHED_TASK_CYCLE_EN
/**
 * 说明: 1.设置任务周期循环信号产生的周期,
 *       2.周期为0表示不产生周期循环信号,
 *       3.复位周期循环信号节拍计数,
 *       4.可以选择是否立即产生信号(节拍0对应的信号)
 *
 * 参数: 1.task      - 任务控制块指针
 *       2.period    - 周期循环信号产生的周期
 *       3.immedTRIG - 是否立即触发信号(SCHED_TRUE/SCHED_FALSE)
 *
 * 返回: 无返回
 */
void framework_TaskSetCyclePeriod(SchedTask_t *task, SchedTick_t period, SchedBool_t immedTRIG)
{
SchedCPU_t cpu_sr;

    SCHED_ASSERT(NULL != task,errSCHED_PARAM_PTR_IS_NULL);
    SCHED_ASSERT((SCHED_FALSE == immedTRIG)||(SCHED_TRUE == immedTRIG),errSCHED_PARAM_NOT_ALLOWED);

    cpu_sr = SCHED_EnterCritical(); /*进入临界区*/
    {
        internal_ListRemove(&task->cycleListItem);
        task->cycleFlag   = 0;
        task->cycleTick   = 0;
        task->cyclePeriod = period;
        if (period > 0)
        {
            if (immedTRIG)
            {
                task->cycleFlag = 1;
                __framework_TaskRecordReadyTask(task);
            }
            __framework_CoreTimeManagerAddDelay(&task->cycleListItem, period);
        }
        __framework_CoreTimeManagerUpdate();
    }
    SCHED_ExitCritical(cpu_sr);     /*退出临界区*/
}

/**
 * 说明: 获取周期循环信号节拍计数
 *
 * 参数: 1.task - 任务控制块指针
 *
 * 返回: 周期循环信号节拍计数
 */
SchedTick_t framework_TaskGetCycleTick(SchedTask_t *task)
{
SchedCPU_t cpu_sr;
SchedTick_t tick;

    SCHED_ASSERT(NULL != task,errSCHED_PARAM_PTR_IS_NULL);
    cpu_sr = SCHED_EnterCritical(); /*进入临界区*/
    {
        tick = task->cycleTick;
    }
    SCHED_ExitCritical(cpu_sr);     /*退出临界区*/
    return (tick);
}
#endif  /* SCHED_TASK_CYCLE_EN */

/*初始化所有任务*/
void framework_TaskInitialiseAll(void)
{
SchedTask_t *pTask;
uint8_t i;

    for (i=0;i<=SCHED_LOWEST_PRIORITY;i++)
    {
        pTask = taskPrioGroup[i];
        if (NULL != pTask)
        {
            framework_FSM_Init(&pTask->fsm);
        }
    }
}

/**
 * 说明: 任务调度函数
 *
 * 参数: 无参数
 *
 * 返回: 1.SCHED_TRUE  表示完成一次任务调度,
 *       2.SCHED_FALSE 表示没有就绪任务,进行了一次空操作
 */
SchedBool_t framework_TaskExecute(void)
{
SchedBool_t     ret;
SchedCPU_t      cpu_sr;
SchedTask_t    *pTask;
SchedEvent_t    event;

    cpu_sr = SCHED_EnterCritical(); /*进入临界区*/
    {
        pTask = prvGetHighestPriorityReadyTask();
        if (NULL != pTask)
        {
            /*获取未处理事件*/
            #if SCHED_TASK_CYCLE_EN
            if (pTask->cycleFlag)
            {
                pTask->cycleFlag = 0;
                sched_PortEventCopy(&event, &internal_event[SCHED_SIG_CYCLE]);
                event.msg = (EvtMsg_t)(pTask->cycleTick);
                ret = SCHED_TRUE;
            } else
            #endif
            if (SCHED_SUCCESS == __framework_EventReceive(pTask,&event))
            {
                ret = SCHED_TRUE;
            }
            else
            {
                ret = SCHED_FALSE;
            }
            /*判断是否剩余事件未处理*/
            if (SCHED_EVENT_RECEIVE_FAILED == __framework_EventTryReceive(pTask))
            {
                __framework_TaskResetReadyTask(pTask);
            }
        }
        else
        {
            ret = SCHED_FALSE;
        }
    }
    SCHED_ExitCritical(cpu_sr);     /*退出临界区*/

    /*状态机处理事件*/
    if (ret)
    {
        framework_FSM_Dispatch(&pTask->fsm,&event);
    }

    return (ret);
}

/*******************************************************************************

                                    内部函数

*******************************************************************************/
/*记录就绪任务*/
void __framework_TaskRecordReadyTask(SchedTask_t const *task)
{
uint8_t prio = task->prio;

    SCHED_ASSERT(prio<=SCHED_LOWEST_PRIORITY,errSCHED_TASK_PRIO_OVER_LOWEST);
    internal_PriotblRecordPrio(&taskReadyTable,prio);
}

/*清除就绪任务*/
void __framework_TaskResetReadyTask(SchedTask_t const *task)
{
uint8_t prio = task->prio;

    SCHED_ASSERT(prio<=SCHED_LOWEST_PRIORITY,errSCHED_TASK_PRIO_OVER_LOWEST);
    internal_PriotblResetPrio(&taskReadyTable,prio);
}

#if SCHED_TASK_CYCLE_EN
/**
 * 说明: 1.时间管理器的延时对象到时回调函数,
 *       2.返回0表示时间管理器无进一步动作,
 *       3.返回非0表示时间管理器将当前对象重新加入延时链表,返回值是延时时间
 *
 * 参数: 1.pArrivalListItem - 结束延时的对象的链表项指针
 *
 * 返回: 1.0  表示不将对象重新加入延时链表
 *       2.>0 表示将对象重新加入延时链表, 延时时间是返回值
 */
SchedTick_t __framework_TaskTimeArrivalHandler(SchedList_t *pArrivalListItem)
{
SchedTask_t *pTask;

    pTask = internal_ListEntry(pArrivalListItem,SchedTask_t,cycleListItem);
    if (pTask->cyclePeriod > 0)
    {
        pTask->cycleFlag = 1;
        pTask->cycleTick++;
        __framework_TaskRecordReadyTask(pTask);
    }
    return (pTask->cyclePeriod);
}
#endif  /* SCHED_TASK_CYCLE_EN */

/*******************************************************************************

                                    私有函数

*******************************************************************************/
/*获取最高优先级任务*/
static SchedTask_t * prvGetHighestPriorityReadyTask(void)
{
SchedTask_t *pTask;
uint8_t highestPrio;

    if (SCHED_FALSE == internal_PriotblIsEmpty(&taskReadyTable))
    {
        highestPrio = internal_PriotblGetHighestPrio(&taskReadyTable);
        SCHED_ASSERT(highestPrio<=SCHED_LOWEST_PRIORITY,errSCHED_TASK_PRIO_OVER_LOWEST);
        pTask = taskPrioGroup[highestPrio];
        SCHED_ASSERT(NULL != pTask,errSCHED_TASK_NOT_EXISTED);
    }
    else
    {
        pTask = NULL;
    }
    return (pTask);
}

#endif  /* SCHED_TASK_EN */
