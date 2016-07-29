/*******************************************************************************
* 文 件 名: sched_fsm.c
* 创 建 者: Keda Huang
* 版    本: V1.0
* 创建日期: 2016-07-29
* 文件说明: 实现事件驱动调度器的核心框架 - 有限状态机
*******************************************************************************/

#include "sched_framework.h"

#if SCHED_TASK_EN
/*******************************************************************************

                                    全局变量

*******************************************************************************/
/*状态机内部事件*/
const SchedEvent_t internal_event[4] =
{
    {SCHED_SIG_EMPTY,   0},
    {SCHED_SIG_ENTRY,   0},
    {SCHED_SIG_EXIT,    0},
    {SCHED_SIG_CYCLE,   0},
};

/*******************************************************************************

                                    操作函数

*******************************************************************************/

/**
 * 说明: 构造状态机
 *
 * 参数: 1.fsm     - 状态机指针
 *       2.initial - 状态机初始伪状态
 *
 * 返回: 无返回
 */
void framework_FSM_Ctor(SchedFSM_t *fsm, SchedStateFunction_t initial)
{
    fsm->state = initial;
}

/**
 * 说明: 初始化状态机,执行初始化状态转移
 *
 * 参数: 1.fsm - 状态机指针
 *
 * 返回: 无返回
 */
void framework_FSM_Init(SchedFSM_t *fsm)
{
SchedBase_t ret;

    /*执行初始化状态转移*/
    ret = (fsm->state)(fsm, &internal_event[SCHED_SIG_EMPTY]);
    SCHED_ASSERT(SCHED_RET_TRAN == ret,errSCHED_FSM_INITIAL_NOT_TRAN);
    /*执行新状态进入动作*/
    ret = (fsm->state)(fsm, &internal_event[SCHED_SIG_ENTRY]);
    SCHED_ASSERT(SCHED_RET_TRAN != ret,errSCHED_FSM_ENTRY_TRAN);
    /*消除编译器警告*/
    ((void) ret);
}

/**
 * 说明: 状态机处理事件
 *
 * 参数: 1.fsm - 状态机指针
 *       2.e   - 状态机待处理事件指针
 *
 * 返回: 无返回
 */
void framework_FSM_Dispatch(SchedFSM_t *fsm, SchedEvent_t const *e)
{
SchedStateFunction_t    tmp;
SchedBase_t             ret;

    /*状态机处理事件*/
    tmp = fsm->state;
    ret = (fsm->state)(fsm, e);
    /*发生状态转移*/
    if (SCHED_RET_TRAN == ret)
    {
        /*执行原状态退出动作*/
        ret = (tmp)(fsm, &internal_event[SCHED_SIG_EXIT]);
        SCHED_ASSERT(SCHED_RET_TRAN != ret,errSCHED_FSM_EXIT_TRAN);
        /*执行新状态进入动作*/
        ret = (fsm->state)(fsm, &internal_event[SCHED_SIG_ENTRY]);
        SCHED_ASSERT(SCHED_RET_TRAN != ret,errSCHED_FSM_ENTRY_TRAN);
        /*消除编译器警告*/
        ((void) ret);
    }
}

#endif  /* SCHED_TASK_EN */
