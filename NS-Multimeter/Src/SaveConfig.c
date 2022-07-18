//==============================================================================
//
// Title:		SaveConfig.c
// Purpose:		A short description of the implementation.
//
// Created on:	2022/7/3 at 18:46:16 by dongpeng liu.
// Copyright:	. All Rights Reserved.
//
//==============================================================================

//==============================================================================
// Include files

#include <inifile.h>
#include <visa.h>

#include "SaveConfig.h"
#include "NS-Multimeter.h"

//==============================================================================
// Global functions
// 保存配置到ini文件
int SaveConfigToIniFile(const InstrConfig* config)
{
	char iniPath[1024] = {0};
	IniText iniHandle = 0;

	//获取工程文件路径
	GetProjectDir (iniPath);
	strcat(iniPath, "\\config.ini");

	iniHandle = Ini_New(0);

	// 写入配置
	Ini_PutString(iniHandle, "Mader", "mader_type", config->mader_type); // 厂家_型号
	
	Ini_PutInt(iniHandle, "Connect", "connectType", config->connectType); // 连接方式
	Ini_PutString(iniHandle, "Connect", "srcAddr", config->srcAddr); // 资源地址
	Ini_PutInt(iniHandle, "Connect", "baudRate", config->baudRate); // 波特率

	Ini_PutInt(iniHandle, "Test", "testItem", config->testItem); // 测试项目
	Ini_PutDouble(iniHandle, "Test", "testRange", config->range); // 量程


	Ini_PutInt(iniHandle, "Test", "dataWidth", config->digit); // 位数
	Ini_PutDouble(iniHandle, "Test", "testSapce", config->testSapce); // 测试间隔
	Ini_PutDouble(iniHandle, "Test", "testTime", config->testTime); // 测试时长
	Ini_PutInt(iniHandle, "Test", "testTimeUseFlag", config->testTimeUseFlag); // 启用测试时长

	Ini_PutInt(iniHandle, "SaveData", "autoSaveFlag", config->autoSaveFlag); // 启用自动存储
	Ini_PutString(iniHandle, "SaveData", "savePath", config->savePath); // 存储路径

	//写入ini文件
	Ini_WriteToFile(iniHandle, iniPath);
	Ini_Dispose(iniHandle);

	return 0;
}

// 从ini文件加载配置
int LoadConfigFromIniFile(int panel, InstrConfig* config)
{
	char iniPath[1024] = {0};
	IniText iniHandle = 0;
	char* buf = NULL;
	
	//获取工程文件路径
	GetProjectDir (iniPath);
	strcat(iniPath, "\\config.ini");

	iniHandle = Ini_New(0);
	//把数据拿到内存区
	if (Ini_ReadFromFile(iniHandle, iniPath) != 0)
	{
		return -1;
	}
	
	Ini_GetInt(iniHandle, "Connect", "connectType", &config->connectType); // 连接方式

	Ini_GetStringCopy(iniHandle, "Connect", "srcAddr", &buf); // 资源地址
	if (buf != NULL)
	{
		strcpy(config->srcAddr, buf);
		free(buf);
		buf = NULL;
	}
	
	Ini_GetStringCopy(iniHandle, "Mader", "mader_type", &buf); // 厂家_型号
	if (buf != NULL)
	{
		strcpy(config->mader_type, buf);
		free(buf);
		buf = NULL;
	}

	Ini_GetInt(iniHandle, "Connect", "baudRate", &config->baudRate); // 波特率

	Ini_GetInt(iniHandle, "Test", "testItem", &config->testItem); // 测试项目

	Ini_GetDouble(iniHandle, "Test", "testRange", &config->range); // 量程

	Ini_GetInt(iniHandle, "Test", "dataWidth", &config->digit); // 位数

	Ini_GetDouble(iniHandle, "Test", "testSapce", &config->testSapce); // 测试间隔

	Ini_GetDouble(iniHandle, "Test", "testTime", &config->testTime); // 测试时长

	Ini_GetInt(iniHandle, "Test", "testTimeUseFlag", &config->testTimeUseFlag); // 启用测试时长

	Ini_GetInt(iniHandle, "SaveData", "autoSaveFlag", &config->autoSaveFlag); // 启用自动存储

	Ini_GetStringCopy(iniHandle, "SaveData", "savePath", &buf); // 存储路径
	if (buf != NULL)
	{
		strcpy(config->savePath, buf);
		free(buf);
		buf = NULL;
	}

	Ini_Dispose(iniHandle);
	return 0;
}
