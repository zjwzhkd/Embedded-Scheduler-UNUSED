/*******************************************************************************
* 文 件 名: sched_internal.h
* 创 建 者: Keda Huang
* 版    本: V1.0
* 创建日期: 2016-07-25
* 文件说明: 事件驱动调度器的内部数据结构定义
*******************************************************************************/

#ifndef __SCHED_INTERNAL_H
#define __SCHED_INTERNAL_H

#include "sched_port.h"

/*******************************************************************************

                                   结构体相关

*******************************************************************************/
/*获取结构体成员变量的偏移量*/
#ifndef offsetof
    #define offsetof(type,member)   ( (size_t)(&((type *)0)->member) )
#endif

/*通过结构体成员指针获得结构体指针*/
#ifndef container_of
    #define container_of(ptr, type, member) \
        ( (type *)((char *)(ptr) - offsetof(type, member)) )
#endif

/*******************************************************************************

                                  对象管理链表

*******************************************************************************/
/* 数据结构 ------------------------------------------------------------------*/
typedef struct sched_list SchedList_t;
struct sched_list
{
    SchedList_t        *next;   /*指向链表后一项*/
    SchedList_t        *prev;   /*指向链表前一项*/
    SchedTick_t         value;  /*链表排序数值  */
    SchedBase_t         type;   /*链表所属类型  */
};

/* 常量定义 ------------------------------------------------------------------*/
enum {
    SCHED_LIST_HEAD = 0,    /*链表头类型      */
    SCHED_LIST_CYCLE,       /*循环信号对象类型*/
    SCHED_LIST_ALARM,       /*闹钟对象类型    */
    SCHED_LIST_DAEMON,      /*守护任务对象类型*/
};

/* 操作宏 --------------------------------------------------------------------*/
/*
 * 获取包含链表项的结构体指针
 * ptr:    链表项指针
 * type:   包含链表项的结构体类型
 * member: 链表项在结构体中的成员变量名称
 * return: 结构体指针
 */
#define internal_ListEntry(ptr, type, member)   container_of(ptr, type, member)
/*
 * 设置链表排序值
 * pList:  链表指针
 * xValue: 设置的排序值
 */
#define internal_ListSetValue(pList, xValue)    ( (pList)->value = (xValue) )
/*
 * 获取链表排序值
 * pList:  链表指针
 * return: 链表排序值
 */
#define internal_ListGetValue(pList)            ( (pList)->value )
/*
 * 获取链表后一项
 * pList:  链表指针
 * return: 链表后一项指针
 */
#define internal_ListNext(pList)                ( (pList)->next )

/* 操作函数 ------------------------------------------------------------------*/
/*初始化链表或者链表项*/
void internal_ListInit(SchedList_t *pList, SchedBase_t type);
/*按照排序值从小到大的顺序插入链表项*/
void internal_ListInsert(SchedList_t *pList, SchedList_t *pListItem);
/*向链表尾部插入链表项*/
void internal_ListInsertEnd(SchedList_t *pList, SchedList_t *pListItem);
/*移除链表项*/
void internal_ListRemove(SchedList_t *pListItem);
/*判断链表是否为空或者链表项是否为孤立链表项*/
SchedBool_t internal_ListIsEmpty(SchedList_t *pList);

/*******************************************************************************

                                  优先级记录表

*******************************************************************************/
/* 数据结构 ------------------------------------------------------------------*/
typedef struct sched_priotable SchedPrioTable_t;
struct sched_priotable
{
    uint8_t     tbl[SCHED_PRIOTBL_TABLE_SIZE];
    uint8_t     grp;
};

/* 常量定义 ------------------------------------------------------------------*/
/*优先级记录表允许最低优先级*/
#define SCHED_PRIOTBL_LOWEST_PRIO   ( 8*SCHED_PRIOTBL_TABLE_SIZE-1 )

/* 操作函数 ------------------------------------------------------------------*/
/*初始化优先级记录表*/
void internal_PriotblInit(SchedPrioTable_t *tbl);
/*在优先级记录表中记录一个优先级*/
void internal_PriotblRecordPrio(SchedPrioTable_t *tbl, uint8_t prio);
/*在优先级记录表中清除一个优先级*/
void internal_PriotblResetPrio(SchedPrioTable_t *tbl, uint8_t prio);
/*判断优先级记录表是否为空*/
SchedBool_t internal_PriotblIsEmpty(SchedPrioTable_t const *tbl);
/*获取优先级记录表中的最高优先级*/
uint8_t internal_PriotblGetHighestPrio(SchedPrioTable_t const *tbl);

/*******************************************************************************

                                    消息队列

*******************************************************************************/
/* 数据结构 ------------------------------------------------------------------*/
typedef struct sched_queue SchedQueue_t;
struct sched_queue
{
    SchedEvent_t       *evtQueue;   /*队列环形Buffer*/
    EvtPos_t            end;        /*队列总长度    */
    EvtPos_t            head;       /*队列头部(读)  */
    EvtPos_t            tail;       /*队列尾部(写)  */
    EvtPos_t            nUsed;      /*队列已使用量  */
    EvtPos_t            nMaxUsed;   /*队列最大使用量*/
};

/* 操作函数 ------------------------------------------------------------------*/
/*队列初始化*/
void internal_QueueInit(SchedQueue_t *queue, SchedEvent_t *evtQueue, EvtPos_t len);
/*向队列尾部插入一个事件块*/
SchedBool_t internal_QueueSend(SchedQueue_t *queue, SchedEvent_t const *evt);
/*向队列头部插入一个事件块*/
SchedBool_t internal_QueueSendFront(SchedQueue_t *queue, SchedEvent_t const *evt);
/*从队列头部取出一个事件块*/
SchedBool_t internal_QueueReceive(SchedQueue_t *queue, SchedEvent_t *evt);
/*判断队列是否为空*/
SchedBool_t internal_QueueIsEmpty(SchedQueue_t *queue);
/*判断队列是否已满*/
SchedBool_t internal_QueueIsFull(SchedQueue_t *queue);
/*获取队列最大使用量*/
EvtPos_t internal_QueueGetMaxUsed(SchedQueue_t *queue);
/*获取队列长度*/
EvtPos_t internal_QueueGetLength(SchedQueue_t *queue);

#endif  /* __SCHED_INTERNAL_H */
