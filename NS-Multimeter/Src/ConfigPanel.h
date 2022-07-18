//==============================================================================
//
// Title:		ConfigPanel.h
// Purpose:		A short description of the interface.
//
// Created on:	2022/7/6 at 9:43:43 by dongpeng liu.
// Copyright:	. All Rights Reserved.
//
//==============================================================================

#ifndef __ConfigPanel_H__
#define __ConfigPanel_H__

#ifdef __cplusplus
    extern "C" {
#endif

//==============================================================================
// Include files

#include "cvidef.h"

//==============================================================================
// Types
/* 连接类型枚举 */
enum ConnectType_en
{
	RS232 = 0,
	GPIB,
	LAN,
	USB
};

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

/* 连接失败面板 */
extern int connectFaildPanelHandle;
/* 连接失败面板加载标志 */
extern volatile int connectFaildPanelHandleFlag;

/* VISA资源管器 */
extern ViSession defaultRM;
/* 设备句柄 */
extern ViSession viDev;
/* 当前连接类型 */
extern int connectType;
/* 设备名称 */
extern char viDevName[1024];
/* 波特率 */
extern ViUInt32 baudRate;

/* 厂家_型号 */
extern char mader_type[1204];

/* 工程路径 */
extern char projectDir[1024];
/* 测试结果保存路径 */
extern char dirPath[1024];

/* 自动存储标志 */
extern volatile int autoSaveFlag;
/* 启用测试时长标志 */
extern volatile int useFlag;
/* 当前测试项目 */
extern int testItem;
/* 量程 */
extern double range;
/* 位数索引 */
extern int digit;
/* 采样间隔（s） */
extern double samplingSpace;
/* 测试时长（s） */
extern double testTime;

#ifdef __cplusplus
    }
#endif

#endif  /* ndef __ConfigPanel_H__ */
