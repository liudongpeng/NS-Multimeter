//==============================================================================
//
// Title:		PlotPanel.h
// Purpose:		引用外部变量
//
// Created on:	2022/7/6 at 9:48:08 by dongpeng liu.
// Copyright:	. All Rights Reserved.
//
//==============================================================================

#ifndef __PlotPanel_H__
#define __PlotPanel_H__

#ifdef __cplusplus
    extern "C" {
#endif

//==============================================================================
// Include files

#include "cvidef.h"

//==============================================================================
// External variables
/* 左侧连接、配置面板 */
extern int configPanelHandle;
/* 左侧连接、配置面板加载标志 */
extern volatile int configPanelHandleFlag;

/* 右侧波形面板 */
extern int wavePanelHandle;
/* 右侧波形面板加载标志 */
extern volatile int wavePanelHandleFlag;

/* 主面板 */
extern int panelHandle;

/* 设备句柄 */
extern ViSession viDev;

/* 工程路径 */
extern char projectDir[1024];

/* 启用测试时长标志 */
extern volatile int useFlag;
/* 采样间隔（s） */
extern double samplingSpace;
/* 测试时长（s） */
extern double testTime;
/* 当前测试项目 */
extern int testItem;
/* 量程 */
double range;
/* 自动存储标志 */
extern volatile int autoSaveFlag;
/* 仪器连接成功标志（0未连接， 1已连接） */
extern volatile int connectState;


//==============================================================================
// Global functions
/* 自动存储 */
void AutoSave();

/* 设置y轴量程 */
void SetYRange();

// 释放内存
void FreeMemory();

/* 采样线程 */
int CVICALLBACK SamplingThread(void *functionData);

/* 画图线程 */
int CVICALLBACK PlotThread(void *functionData);

#ifdef __cplusplus
    }
#endif

#endif  /* ndef __PlotPanel_H__ */
