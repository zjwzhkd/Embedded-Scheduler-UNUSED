/*******************************************************************************
* 文 件 名: sched_daemon.c
* 创 建 者: Keda Huang
* 版    本: V1.0
* 创建日期: 2016-08-02
* 文件说明: 实现事件驱动调度器的核心框架 - 守护任务管理
*******************************************************************************/

#include "sched_framework.h"

#if SCHED_DAEMON_EN
/*******************************************************************************

                                    全局变量

*******************************************************************************/
static SchedList_t daemonReadyList;
static SchedDaemon_t *currentDaemon;

/*******************************************************************************

                                    操作函数

*******************************************************************************/

/*守护任务管理环境初始化*/
void framework_DaemonEnvirInit(void)
{
    internal_ListInit(&daemonReadyList, SCHED_LIST_HEAD);
    currentDaemon = NULL;
}

/**
 * 说明: 1.创建新的守护任务
 *       2.仅允许在调度器启动前创建守护任务
 *
 * 参数: 1.daemonFunc - 守护任务函数
 *
 * 返回: 1.NULL   表示守护任务创建失败
 *       2.非NULL 表示守护任务控制块指针
 */
SchedDaemon_t *framework_DaemonCreate(SchedDaemonFunction_t daemonFunc)
{
SchedDaemon_t *pDaemon = NULL;

    SCHED_ASSERT(SCHED_CORE_STOP == framework_CoreStatus,errSCHED_DAEMON_NOT_CREATED_BEFORE_CORE_RUNNING);
    pDaemon = (SchedDaemon_t *)sched_PortMalloc(sizeof(SchedDaemon_t));
    if (NULL != pDaemon)
    {
        pDaemon->daemonFunc = daemonFunc;
        internal_ListInit(&pDaemon->daemonListItem, SCHED_LIST_DAEMON);
    }
    return (pDaemon);
}

/**
 * 说明: 1.唤醒守护任务并执行给定的事件,
 *       2.当守护任务处于休眠状态(SCHED_DAEMON_DORMANT)或者
 *         运行状态(SCHED_DAEMON_RUNNING)时,允许唤醒守护任务
 *
 * 参数: 1.daemon - 守护任务控制块指针
 *       2.evt    - 守护任务执行的事件块
 *       3.delay  - 守护任务执行延时(0表示直接就绪)
 *
 * 返回: 1.SCHED_SUCCESS            表示守护任务唤醒成功
 *       2.SCHED_DAEMON_CALL_FAILED 表示守护任务唤醒失败
 */
SchedStatus_t framework_DaemonCall(SchedDaemon_t *daemon, SchedEvent_t const *evt, SchedTick_t delay)
{
SchedStatus_t   ret;
SchedCPU_t      cpu_sr;

    SCHED_ASSERT(NULL != daemon,errSCHED_PARAM_PTR_IS_NULL);
    cpu_sr = SCHED_EnterCritical(); /*进入临界区*/
    {
        /*守护任务处于休眠状态或者运行状态*/
        if (SCHED_FALSE != internal_ListIsEmpty(&daemon->daemonListItem))
        {
            sched_PortEventCopy(&daemon->event,evt);
            if (delay > 0)
            {
                __framework_CoreTimeManagerAddDelay(&daemon->daemonListItem,delay);
            }
            else
            {
                internal_ListInsertEnd(&daemonReadyList,&daemon->daemonListItem);
            }
            ret = SCHED_SUCCESS;
        }
        else
        {
            ret = SCHED_DAEMON_CALL_FAILED;
        }
    }
    SCHED_ExitCritical(cpu_sr);     /*退出临界区*/

    return (ret);
}

/**
 * 说明: 终止指定的守护任务
 *
 * 参数: 1.daemon - 守护任务控制块指针
 *
 * 返回: 无返回
 */
void framework_DaemonAbort(SchedDaemon_t *daemon)
{
SchedCPU_t cpu_sr;

    SCHED_ASSERT(NULL != daemon,errSCHED_PARAM_PTR_IS_NULL);
    cpu_sr = SCHED_EnterCritical(); /*进入临界区*/
    {
        internal_ListRemove(&daemon->daemonListItem);
        __framework_CoreTimeManagerUpdate();
    }
    SCHED_ExitCritical(cpu_sr);     /*退出临界区*/
}

/**
 * 说明: 获取指定守护任务的状态
 *
 * 参数: 1.daemon - 守护任务控制块指针
 *
 * 返回: 1.SCHED_DAEMON_RUNNING 表示守护任务正在运行
 *       2.SCHED_DAEMON_ACTIVE  表示守护任务已被唤醒
 *       3.SCHED_DAEMON_DORMANT 表示守护任务处于休眠
 */
SchedStatus_t framework_DaemonGetStatus(SchedDaemon_t *daemon)
{
SchedStatus_t   ret;
SchedCPU_t      cpu_sr;

    SCHED_ASSERT(NULL != daemon,errSCHED_PARAM_PTR_IS_NULL);
    cpu_sr = SCHED_EnterCritical(); /*进入临界区*/
    {
        if (daemon == currentDaemon)
        {
            ret = SCHED_DAEMON_RUNNING;
        }
        else if (SCHED_FALSE == internal_ListIsEmpty(&daemon->daemonListItem))
        {
            ret = SCHED_DAEMON_ACTIVE;
        }
        else
        {
            ret = SCHED_DAEMON_DORMANT;
        }
    }
    SCHED_ExitCritical(cpu_sr);     /*退出临界区*/

    return (ret);
}

/**
 * 说明: 1.在中断函数中唤醒守护任务并执行给定的事件,
 *       2.仅当守护任务处于休眠状态(SCHED_DAEMON_DORMANT), 允许在中断中唤醒守护任务
 *
 * 参数: 1.daemon - 守护任务控制块指针
 *       2.evt    - 守护任务执行的事件块
 *       3.delay  - 守护任务执行延时(0表示直接就绪)
 *
 * 返回: 1.SCHED_SUCCESS            表示守护任务唤醒成功
 *       2.SCHED_DAEMON_CALL_FAILED 表示守护任务唤醒失败
 */
SchedStatus_t framework_DaemonCallFromISR(SchedDaemon_t *daemon, SchedEvent_t const *evt, SchedTick_t delay)
{
SchedStatus_t   ret;
SchedCPU_t      cpu_sr;

    SCHED_ASSERT(NULL != daemon,errSCHED_PARAM_PTR_IS_NULL);
    cpu_sr = SCHED_EnterCriticalFromISR();  /*进入临界区*/
    {
        /*守护任务处于休眠状态*/
        if ((SCHED_FALSE != internal_ListIsEmpty(&daemon->daemonListItem)) && (daemon != currentDaemon))
        {
            sched_PortEventCopy(&daemon->event,evt);
            if (delay > 0)
            {
                __framework_CoreTimeManagerAddDelay(&daemon->daemonListItem,delay);
            }
            else
            {
                internal_ListInsertEnd(&daemonReadyList,&daemon->daemonListItem);
            }
            ret = SCHED_SUCCESS;
        }
        else
        {
            ret = SCHED_DAEMON_CALL_FAILED;
        }
    }
    SCHED_ExitCriticalFromISR(cpu_sr);      /*退出临界区*/

    return (ret);
}

/**
 * 说明: 在中断函数中获取指定守护任务的状态
 *
 * 参数: 1.daemon - 守护任务控制块指针
 *
 * 返回: 1.SCHED_DAEMON_RUNNING 表示守护任务正在运行
 *       2.SCHED_DAEMON_ACTIVE  表示守护任务已被唤醒
 *       3.SCHED_DAEMON_DORMANT 表示守护任务处于休眠
 */
SchedStatus_t framework_DaemonGetStatusFromISR(SchedDaemon_t *daemon)
{
SchedStatus_t   ret;
SchedCPU_t      cpu_sr;

    SCHED_ASSERT(NULL != daemon,errSCHED_PARAM_PTR_IS_NULL);
    cpu_sr = SCHED_EnterCriticalFromISR();  /*进入临界区*/
    {
        if (daemon == currentDaemon)
        {
            ret = SCHED_DAEMON_RUNNING;
        }
        else if (SCHED_FALSE == internal_ListIsEmpty(&daemon->daemonListItem))
        {
            ret = SCHED_DAEMON_ACTIVE;
        }
        else
        {
            ret = SCHED_DAEMON_DORMANT;
        }
    }
    SCHED_ExitCriticalFromISR(cpu_sr);      /*退出临界区*/

    return (ret);
}

/**
 * 说明: 守护任务调度函数
 *
 * 参数: 无参数
 *
 * 返回: 1.SCHED_TRUE表示完成一次任务调度,
 *       2.SCHED_FALSE表示没有就绪任务,进行了一次空操作
 */
SchedBool_t framework_DaemonExecute(void)
{
SchedBool_t     ret;
SchedEvent_t    event;
SchedList_t    *pListItem;
SchedCPU_t      cpu_sr;

    cpu_sr = SCHED_EnterCritical(); /*进入临界区*/
    {
        /*获取就绪守护任务*/
        if (SCHED_FALSE == internal_ListIsEmpty(&daemonReadyList))
        {
            pListItem = internal_ListNext(&daemonReadyList);
            internal_ListRemove(pListItem);
            currentDaemon = internal_ListEntry(pListItem,SchedDaemon_t,daemonListItem);
            sched_PortEventCopy(&event, &currentDaemon->event);
            ret = SCHED_TRUE;
        }
        else
        {
            currentDaemon = NULL;
            ret = SCHED_FALSE;
        }
    }
    SCHED_ExitCritical(cpu_sr);     /*退出临界区*/

    /*守护任务处理事件*/
    if (ret)
    {
        (currentDaemon->daemonFunc)(currentDaemon, &event);
    }

    cpu_sr = SCHED_EnterCritical(); /*进入临界区*/
    {
        currentDaemon = NULL;
    }
    SCHED_ExitCritical(cpu_sr);     /*退出临界区*/

    return (ret);
}

/*******************************************************************************

                                    内部函数

*******************************************************************************/
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
SchedTick_t __framework_DaemonTimeArrivalHandler(SchedList_t *pArrivalListItem)
{
    internal_ListInsertEnd(&daemonReadyList,pArrivalListItem);
    return (0);
}

#endif  /* SCHED_DAEMON_EN */
