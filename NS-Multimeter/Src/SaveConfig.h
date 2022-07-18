//==============================================================================
//
// Title:		SaveConfig.h
// Purpose:		A short description of the interface.
//
// Created on:	2022/7/3 at 18:46:16 by dongpeng liu.
// Copyright:	. All Rights Reserved.
//
//==============================================================================

#ifndef __SaveConfig_H__
#define __SaveConfig_H__

#ifdef __cplusplus
    extern "C" {
#endif

//==============================================================================
// Include files

#include "cvidef.h"

//==============================================================================
// Types
typedef struct config
{
	int connectType; // 连接方式
	char srcAddr[128]; // 资源地址
	int baudRate; // 波特率
	char mader_type[128]; // 厂家_型号
	int testItem; // 测试项目
	double range; // 量程 V A Ω
	int digit; // 位数
	double testSapce; // 测试间隔
	double testTime; // 测试时长
	int testTimeUseFlag; // 启用测试时长
	int autoSaveFlag; // 启用自动存储
	
	char savePath[1024]; // 存储路径
} InstrConfig;

//==============================================================================
// External variables
extern char projectDir[1024];

//==============================================================================
// Global functions
// 保存配置到ini文件
int SaveConfigToIniFile(const InstrConfig* config);

// 从ini文件加载配置
int LoadConfigFromIniFile(int panel, InstrConfig* config);

#ifdef __cplusplus
    }
#endif

#endif  /* ndef __SaveConfig_H__ */
