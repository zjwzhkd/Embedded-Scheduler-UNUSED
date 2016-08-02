/*******************************************************************************
* 文 件 名: sched_port.c
* 创 建 者: Keda Huang
* 版    本: V1.0
* 创建日期: 2016-07-25
* 文件说明: 事件驱动调度器的底层接口
*******************************************************************************/

#include "sched_port.h"
/*******************************************************************************

                                    底层接口

*******************************************************************************/
/*调度器底层初始化*/
void sched_PortInit(void)
{
    /*环境检查*/
    SCHED_CHECK(sizeof(EvtMsg_t)>=sizeof(void*),chkSCHED_TYPE_CONVERSION_FAILED);
    #if SCHED_TASK_CYCLE_EN
    {
        SCHED_CHECK(sizeof(EvtMsg_t)>=sizeof(SchedTick_t),chkSCHED_TYPE_CONVERSION_FAILED);
    }
    #endif
    /*初始化内存管理*/
    sched_PortHeapInit();
}

/**
 * 说明: 事件块复制
 *
 * 参数: 1.dest - 目标事件块地址
 *       2.src  - 源事件块地址
 *
 * 返回: 无返回
 */
void sched_PortEventCopy(SchedEvent_t *dest, SchedEvent_t const *src)
{
    /*参数校验*/
    SCHED_ASSERT(NULL != dest, errSCHED_PARAM_PTR_IS_NULL);
    /*复制事件块*/
    dest->sig = src->sig;
    dest->msg = src->msg;
}

/*调度器错误处理函数*/
__weak void sched_PortErrorHandler(SchedStatus_t errCode)
{
    ((void)errCode);
    debug_assert(0);
}

/*调度器空闲处理函数*/
__weak void sched_PortIdleHandler(void)
{
}
