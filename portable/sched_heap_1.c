/*******************************************************************************
* 文 件 名: sched_heap_1.c
* 创 建 者: Keda Huang
* 版    本: V1.0
* 创建日期: 2016-08-22
* 文件说明: 事件驱动调度器的动态内存分配 - 仅实现分配函数
*******************************************************************************/

#include "sched_port.h"
/*******************************************************************************

                                    全局变量

*******************************************************************************/
#define HEAP_TOTAL_SIZE     ( SCHED_TOTAL_HEAP_SIZE - SCHED_BYTE_ALIGNMENT )
static uint8_t  heapMemory[SCHED_TOTAL_HEAP_SIZE];
static uint8_t *heapAlignedStart;
static size_t   heapNextFreeByte;

/*******************************************************************************

                                    内存分配

*******************************************************************************/
/*内存管理初始化*/
void sched_PortHeapInit(void)
{
    heapNextFreeByte = 0;
    if ( (((size_t)heapMemory)&SCHED_BYTE_ALIGNMENT_MASK) != 0 )
    {
        heapAlignedStart = heapMemory + ( SCHED_BYTE_ALIGNMENT - (((size_t)heapMemory)&SCHED_BYTE_ALIGNMENT_MASK) );
    }
    else
    {
        heapAlignedStart = heapMemory;
    }
}

/*动态内存分配*/
void *sched_PortMalloc(size_t size)
{
void *ret = NULL;

    if ( (size&SCHED_BYTE_ALIGNMENT_MASK) != 0 )
    {
        size += ( SCHED_BYTE_ALIGNMENT - (size&SCHED_BYTE_ALIGNMENT_MASK) );
    }

    if (((size + heapNextFreeByte) < HEAP_TOTAL_SIZE) &&
        ((size + heapNextFreeByte) > heapNextFreeByte))
    {
        ret = heapAlignedStart + heapNextFreeByte;
        heapNextFreeByte += size;
    }

    SCHED_CHECK(NULL != ret,chkSCHED_MALLOC_FAILED);
    return (ret);
}

/*动态内存释放*/
void sched_PortFree(void *pv)
{
    SCHED_CHECK(NULL == pv,chkSCHED_MALLOC_FAILED);
}
