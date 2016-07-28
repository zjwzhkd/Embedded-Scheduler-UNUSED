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
#define SCHED_USE_16BIT_TICK_EN     ( 0 )
#define SCHED_PRIOTBL_TABLE_SIZE    ( 4 )

/* 调度器功能 ----------------------------------------------------------------*/
#define SCHED_TASK_EN               ( 1 )
#define SCHED_TASK_EVENT_METHOD     ( 1 )
#define SCHED_TASK_CYCLE_EN         ( 1 )
#define SCHED_TASK_ALARM_EN         ( 1 )
#define SCHED_DAEMON_EN             ( 1 )

/* 调度器调试 ----------------------------------------------------------------*/
#define SCHED_CHECK_EN              ( 1 )
#define SCHED_ASSERT_EN             ( 1 )

/* 内存分配 ------------------------------------------------------------------*/
#define SCHED_TOTAL_HEAP_SIZE       ( 1000 )
#define SCHED_BYTE_ALIGNMENT        ( 8 )

#endif  /* __SCHED_CONFIG_H */
