/*******************************************************************************
* 文 件 名: sched_priotbl.c
* 创 建 者: Keda Huang
* 版    本: V1.0
* 创建日期: 2016-07-25
* 文件说明: 实现事件驱动调度器的内部数据结构 - 优先级记录表
*******************************************************************************/

#include "sched_internal.h"
/*******************************************************************************

                                    全局数组

*******************************************************************************/
static uint8_t const FLASH_DATA priotbl_unmap[] =
{
    0u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x00 to 0x0F */
    4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x10 to 0x1F */
    5u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x20 to 0x2F */
    4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x30 to 0x3F */
    6u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x40 to 0x4F */
    4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x50 to 0x5F */
    5u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x60 to 0x6F */
    4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x70 to 0x7F */
    7u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x80 to 0x8F */
    4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0x90 to 0x9F */
    5u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0xA0 to 0xAF */
    4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0xB0 to 0xBF */
    6u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0xC0 to 0xCF */
    4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0xD0 to 0xDF */
    5u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, /* 0xE0 to 0xEF */
    4u, 0u, 1u, 0u, 2u, 0u, 1u, 0u, 3u, 0u, 1u, 0u, 2u, 0u, 1u, 0u  /* 0xF0 to 0xFF */
};

/*******************************************************************************

                                    操作函数

*******************************************************************************/
/**
 * 说明: 初始化优先级记录表
 *
 * 参数: 1.tbl - 待初始化的优先级记录表指针
 *
 * 返回: 无返回
 */
void internal_PriotblInit(SchedPrioTable_t *tbl)
{
uint8_t i;

    for (i=0;i<SCHED_PRIOTBL_TABLE_SIZE;i++)
    {
        tbl->tbl[i] = 0;
    }
    tbl->grp = 0;
}

/**
 * 说明: 在优先级记录表中记录一个优先级
 *
 * 参数: 1.tbl  - 待操作的优先级记录表指针
 *       2.prio - 记录的优先级, 有效范围是0 - SCHED_PRIOTBL_LOWEST_PRIO
 *
 * 返回: 无返回
 */
void internal_PriotblRecordPrio(SchedPrioTable_t *tbl, uint8_t prio)
{
    SCHED_ASSERT(prio<=SCHED_PRIOTBL_LOWEST_PRIO,errSCHED_PRIOTBL_ERROR);
    if (prio <= SCHED_PRIOTBL_LOWEST_PRIO)
    {
    uint8_t x = prio&0x7;
    uint8_t y = prio>>3;

        tbl->tbl[y] |= (uint8_t)1<<x;
        tbl->grp    |= (uint8_t)1<<y;
    }
}

/**
 * 说明: 在优先级记录表中清除一个优先级
 *
 * 参数: 1.tbl  - 待操作的优先级记录表指针
 *       2.prio - 清除的优先级, 有效范围是0 - SCHED_PRIOTBL_LOWEST_PRIO
 *
 * 返回: 无返回
 */
void internal_PriotblResetPrio(SchedPrioTable_t *tbl, uint8_t prio)
{
    SCHED_ASSERT(prio<=SCHED_PRIOTBL_LOWEST_PRIO,errSCHED_PRIOTBL_ERROR);
    if (prio <= SCHED_PRIOTBL_LOWEST_PRIO)
    {
    uint8_t x = prio&0x7;
    uint8_t y = prio>>3;

        tbl->tbl[y]  &= ~((uint8_t)1<<x);
        if (0 == tbl->tbl[y])
        {
            tbl->grp &= ~((uint8_t)1<<y);
        }
    }
}

/**
 * 说明: 判断优先级记录表是否为空
 *
 * 参数: tbl - 优先级记录表指针
 *
 * 返回: 布尔值,
 *       1.SCHED_TRUE   表示优先级记录表为空
 *       2.SCHED_FALSE  表示优先级记录表不空
 */
SchedBool_t internal_PriotblIsEmpty(SchedPrioTable_t const *tbl)
{
    if (0 == tbl->grp)
    {
        return (SCHED_TRUE);
    }
    else
    {
        return (SCHED_FALSE);
    }
}

/**
 * 说明: 获取优先级记录表中的最高优先级
 *
 * 参数: tbl - 优先级记录表指针
 *
 * 返回: 1.如果优先级记录表非空, 返回记录的最高优先级
 *       2.如果优先级记录表为空, 返回0
 */
uint8_t internal_PriotblGetHighestPrio(SchedPrioTable_t const *tbl)
{
uint8_t prio;
uint8_t x,y;

    y = priotbl_unmap[tbl->grp];
    SCHED_ASSERT(y<SCHED_PRIOTBL_TABLE_SIZE,errSCHED_PRIOTBL_ERROR);
    x = priotbl_unmap[tbl->tbl[y]];
    prio = x + (y<<3);
    return (prio);
}
