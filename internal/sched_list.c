/*******************************************************************************
* 文 件 名: sched_list.c
* 创 建 者: Keda Huang
* 版    本: V1.0
* 创建日期: 2016-07-25
* 文件说明: 实现事件驱动调度器的内部数据结构 - 对象管理链表
*******************************************************************************/

#include "sched_internal.h"
/*******************************************************************************

                                    操作函数

*******************************************************************************/
/**
 * 说明: 初始化链表或者链表项
 *
 * 参数: 1.pList - 待初始化的链表或链表项
 *       2.type  - 初始化对象类型
 *
 * 返回: 无返回
 */
void internal_ListInit(SchedList_t *pList, SchedBase_t type)
{
    pList->next  = pList;
    pList->prev  = pList;
    pList->type  = type;
    pList->value = SCHED_MAX_TICK;
}

/**
 * 说明: 按照排序值从小到大的顺序插入链表项
 *
 * 参数: 1.pList     - 链表指针
 *       2.pListItem - 待插入的链表项指针
 *
 * 返回: 无返回
 */
void internal_ListInsert(SchedList_t *pList, SchedList_t *pListItem)
{
SchedList_t *pInterator;
SchedTick_t const insertValue = pListItem->value;

    SCHED_ASSERT(internal_ListIsEmpty(pListItem),errSCHED_LIST_ERROR);
    SCHED_ASSERT(SCHED_LIST_HEAD != pListItem->type,errSCHED_LIST_ERROR);
    SCHED_ASSERT(SCHED_LIST_HEAD == pList->type,errSCHED_LIST_ERROR);
    SCHED_ASSERT(SCHED_MAX_TICK == pList->value,errSCHED_LIST_ERROR);

    if (SCHED_MAX_TICK == insertValue)
    {
        pInterator = pList->prev;
    }
    else
    {
        /*
            1. if pList->next->value > insetValue  ==> pInterator = pList
            2. if pList->next->value <= insetValue ==> pInterator->value <= inserValue and pInterator->next->value > insetValue
        */
        for (pInterator = pList;pInterator->next->value <= insertValue;pInterator = pInterator->next)
        {
        }
    }
    pListItem->next       = pInterator->next;
    pListItem->prev       = pInterator;
    pListItem->next->prev = pListItem;
    pInterator->next      = pListItem;
}

/**
 * 说明: 向链表尾部插入链表项
 *
 * 参数: 1.pList     - 链表指针
 *       2.pListItem - 待插入的链表项指针
 *
 * 返回: 无返回
 */
void internal_ListInsertEnd(SchedList_t *pList, SchedList_t *pListItem)
{
    SCHED_ASSERT(internal_ListIsEmpty(pListItem),errSCHED_LIST_ERROR);
    SCHED_ASSERT(SCHED_LIST_HEAD != pListItem->type,errSCHED_LIST_ERROR);
    SCHED_ASSERT(SCHED_LIST_HEAD == pList->type,errSCHED_LIST_ERROR);
    SCHED_ASSERT(SCHED_MAX_TICK == pList->value,errSCHED_LIST_ERROR);

    pListItem->next       = pList;
    pListItem->prev       = pList->prev;
    pListItem->prev->next = pListItem;
    pList->prev           = pListItem;
}

/**
 * 说明: 移除链表项
 *
 * 参数: 1.pListItem - 待移除的链表项
 *
 * 返回: 无返回
 */
void internal_ListRemove(SchedList_t *pListItem)
{
    SCHED_ASSERT(SCHED_LIST_HEAD != pListItem->type,errSCHED_LIST_ERROR);

    pListItem->prev->next = pListItem->next;
    pListItem->next->prev = pListItem->prev;
    pListItem->next       = pListItem;
    pListItem->prev       = pListItem;
}

/**
 * 说明: 判断链表是否为空或者链表项是否为孤立链表项
 *
 * 参数: 1.pList - 链表指针
 *
 * 返回: 布尔值,
 *       1.SCHED_FALSE 表示链表非空或者链表项非孤立
 *       2.SCHED_TRUE  表示链表为空或者链表项为孤立
 */
SchedBool_t internal_ListIsEmpty(SchedList_t *pList)
{
SchedBool_t ret;

    if (pList->next == pList)
    {
        SCHED_ASSERT(pList->prev == pList,errSCHED_LIST_ERROR);
        ret = SCHED_TRUE;
    }
    else
    {
        SCHED_ASSERT(pList->prev != pList,errSCHED_LIST_ERROR);
        ret = SCHED_FALSE;
    }
    return (ret);
}
