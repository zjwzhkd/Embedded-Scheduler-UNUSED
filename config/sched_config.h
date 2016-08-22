/*******************************************************************************
* 文 件 名: sched_config.h
* 创 建 者: Keda Huang
* 版    本: V1.0
* 创建日期: 2016-07-25
* 文件说明: 事件驱动调度器配置
*******************************************************************************/

#ifndef __SCHED_CONFIG_H
#define __SCHED_CONFIG_H

/* 调度器参数 ----------------------------------------------------------------*/
#define SCHED_TICK_HZ               ( (uint32_t) 1000 ) /* 调度器节拍频率(Hz) */
#define SCHED_LOWEST_PRIORITY       ( 3 )               /* 调度器最低优先级   */

/* 调度器配置 ----------------------------------------------------------------*/
#define SCHED_USE_16BIT_TICK_EN     ( 0 )   /* 0-使用32位节拍, 1-使用16位节拍 */
#define SCHED_PRIOTBL_TABLE_SIZE    ( 4 )   /* 优先级记录表大小               */

/* 调度器功能 ----------------------------------------------------------------*/
#define SCHED_TASK_EN               ( 1 )   /* 任务使能控制(0/1)              */
#define SCHED_TASK_EVENT_METHOD     ( 1 )   /* 0-使用记录表, >=1-使用消息队列 */
#define SCHED_TASK_CYCLE_EN         ( 1 )   /* 周期信号使能控制(0/1)          */
#define SCHED_TASK_ALARM_EN         ( 1 )   /* 闹钟使能控制(0/1)              */
#define SCHED_DAEMON_EN             ( 1 )   /* 守护任务使能控制(0/1)          */

/* 调度器调试 ----------------------------------------------------------------*/
#define SCHED_CHECK_EN              ( 1 )   /* 调度器运行监测使能(0/1)        */
#define SCHED_ASSERT_EN             ( 1 )   /* 调度器断言使能(0/1)            */

/* 内存分配 ------------------------------------------------------------------*/
#define SCHED_TOTAL_HEAP_SIZE       ( 1000 )        /* 调度器内存分配总大小   */
#define SCHED_BYTE_ALIGNMENT        ( 8 )           /* 调度器内存分配字节对齐 */
#define SCHED_BYTE_ALIGNMENT_MASK   ( SCHED_BYTE_ALIGNMENT-1 )

#endif  /* __SCHED_CONFIG_H */
