/*******************************************************************************
* 文 件 名: sched_queue.c
* 创 建 者: Keda Huang
* 版    本: V1.0
* 创建日期: 2016-07-25
* 文件说明: 实现事件驱动调度器的内部数据结构 - 消息队列
*******************************************************************************/

#include "sched_internal.h"

/*******************************************************************************

                                    操作函数

*******************************************************************************/

/**
 * 队列初始化
 *
 * @param queue: 待初始化队列的指针
 *
 * @param evtQueue: 队列Buffer数组的首元素指针
 *
 * @param len: 队列长度, 长度若为0, 参数evtQueue传入为NULL
 */
void internal_QueueInit(SchedQueue_t *queue, SchedEvent_t *evtQueue, EvtPos_t len)
{
    queue->evtQueue = evtQueue;
    queue->end      = len;
    queue->head     = 0;
    queue->tail     = 0;
    queue->nUsed    = 0;
    queue->nMaxUsed = 0;
}

/**
 * 向队列尾部插入一个事件块
 *
 * @param queue: 目标队列指针
 *
 * @param evt: 待复制的事件块指针, 事件块内容复制到队列Buffer
 *
 * @return: 布尔值(SCHED_TRUE/SCHED_FALSE),
 *          SCHED_TRUE  表示事件块插入成功
 *          SCHED_FALSE 表示队列已满
 */
SchedBool_t internal_QueueSend(SchedQueue_t *queue, SchedEvent_t const *evt)
{
SchedBool_t ret;

    if (queue->nUsed == queue->end)
    {
        /*队列已满*/
        ret = SCHED_FALSE;
    }
    else
    {
        /*插入事件块*/
        sched_PortEventCopy(&queue->evtQueue[queue->tail++], evt);
        if (queue->tail == queue->end)
        {
            queue->tail = 0;
        }
        queue->nUsed++;
        if (queue->nUsed > queue->nMaxUsed)
        {
            queue->nMaxUsed = queue->nUsed;
        }
        ret = SCHED_TRUE;
    }

    return (ret);
}

/**
 * 向队列头部插入一个事件块
 *
 * @param queue: 目标队列指针
 *
 * @param evt: 待复制的事件块指针, 事件块内容复制到队列Buffer
 *
 * @return: 布尔值(SCHED_TRUE/SCHED_FALSE),
 *          SCHED_TRUE  表示事件块插入成功
 *          SCHED_FALSE 表示队列已满
 */
SchedBool_t internal_QueueSendFront(SchedQueue_t *queue, SchedEvent_t const *evt)
{
SchedBool_t ret;

    if (queue->nUsed == queue->end)
    {
        /*队列已满*/
        ret = SCHED_FALSE;
    }
    else
    {
        if (0 == queue->head)
        {
            queue->head = queue->end - 1;
        }
        else
        {
            queue->head--;
        }
        sched_PortEventCopy(&queue->evtQueue[queue->head], evt);
        queue->nUsed++;
        if (queue->nUsed > queue->nMaxUsed)
        {
            queue->nMaxUsed = queue->nUsed;
        }
        ret = SCHED_TRUE;
    }
    return (ret);
}

/**
 * 从队列头部取出一个事件块
 *
 * @param queue: 目标队列指针
 *
 * @param evt: 保存结果的事件块指针, 事件块内容从队列Buffer中复制
 *
 * @return: 布尔值(SCHED_TRUE/SCHED_FALSE),
 *          SCHED_TRUE  表示事件块取出成功
 *          SCHED_FALSE 表示队列已空
 */
SchedBool_t internal_QueueReceive(SchedQueue_t *queue, SchedEvent_t *evt)
{
SchedBool_t ret;

    if (0 == queue->nUsed)
    {
        ret = SCHED_FALSE;
    }
    else
    {
        sched_PortEventCopy(evt, &queue->evtQueue[queue->head++]);
        if (queue->head == queue->end)
        {
            queue->head = 0;
        }
        queue->nUsed--;
        ret = SCHED_TRUE;
    }
    return (ret);
}

/**
 * 判断队列是否为空
 *
 * @param queue: 目标队列指针
 *
 * @return: 布尔值(SCHED_TRUE/SCHED_FALSE),
 *          SCHED_TRUE  表示队列为空
 *          SCHED_FALSE 表示队列不空
 */
SchedBool_t internal_QueueIsEmpty(SchedQueue_t *queue)
{
SchedBool_t ret;

    if (0 == queue->nUsed)
    {
        ret = SCHED_TRUE;
    }
    else
    {
        ret = SCHED_FALSE;
    }
    return (ret);
}

/**
 * 判断队列是否已满
 *
 * @param queue: 目标队列指针
 *
 * @return: 布尔值(SCHED_TRUE/SCHED_FALSE),
 *          SCHED_TRUE  表示队列已满
 *          SCHED_FALSE 表示队列未满
 */
SchedBool_t internal_QueueIsFull(SchedQueue_t *queue)
{
SchedBool_t ret;

    if (queue->end == queue->nUsed)
    {
        ret = SCHED_TRUE;
    }
    else
    {
        ret = SCHED_FALSE;
    }
    return (ret);
}

/**
 * 获取队列最大使用量
 *
 * @param queue: 目标队列指针
 *
 * @return: 队列最大使用量
 */
EvtPos_t internal_QueueGetMaxUsed(SchedQueue_t *queue)
{
    return (queue->nMaxUsed);
}

/**
 * 获取队列长度
 *
 * @param queue: 目标队列指针
 *
 * @return: 队列长度
 */
EvtPos_t internal_QueueGetLength(SchedQueue_t *queue)
{
    return (queue->end);
}
