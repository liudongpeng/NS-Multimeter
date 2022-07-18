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
// �������õ�ini�ļ�
int SaveConfigToIniFile(const InstrConfig* config)
{
	char iniPath[1024] = {0};
	IniText iniHandle = 0;

	//��ȡ�����ļ�·��
	GetProjectDir (iniPath);
	strcat(iniPath, "\\config.ini");

	iniHandle = Ini_New(0);

	// д������
	Ini_PutString(iniHandle, "Mader", "mader_type", config->mader_type); // ����_�ͺ�
	
	Ini_PutInt(iniHandle, "Connect", "connectType", config->connectType); // ���ӷ�ʽ
	Ini_PutString(iniHandle, "Connect", "srcAddr", config->srcAddr); // ��Դ��ַ
	Ini_PutInt(iniHandle, "Connect", "baudRate", config->baudRate); // ������

	Ini_PutInt(iniHandle, "Test", "testItem", config->testItem); // ������Ŀ
	Ini_PutDouble(iniHandle, "Test", "testRange", config->range); // ����


	Ini_PutInt(iniHandle, "Test", "dataWidth", config->digit); // λ��
	Ini_PutDouble(iniHandle, "Test", "testSapce", config->testSapce); // ���Լ��
	Ini_PutDouble(iniHandle, "Test", "testTime", config->testTime); // ����ʱ��
	Ini_PutInt(iniHandle, "Test", "testTimeUseFlag", config->testTimeUseFlag); // ���ò���ʱ��

	Ini_PutInt(iniHandle, "SaveData", "autoSaveFlag", config->autoSaveFlag); // �����Զ��洢
	Ini_PutString(iniHandle, "SaveData", "savePath", config->savePath); // �洢·��

	//д��ini�ļ�
	Ini_WriteToFile(iniHandle, iniPath);
	Ini_Dispose(iniHandle);

	return 0;
}

// ��ini�ļ���������
int LoadConfigFromIniFile(int panel, InstrConfig* config)
{
	char iniPath[1024] = {0};
	IniText iniHandle = 0;
	char* buf = NULL;
	
	//��ȡ�����ļ�·��
	GetProjectDir (iniPath);
	strcat(iniPath, "\\config.ini");

	iniHandle = Ini_New(0);
	//�������õ��ڴ���
	if (Ini_ReadFromFile(iniHandle, iniPath) != 0)
	{
		return -1;
	}
	
	Ini_GetInt(iniHandle, "Connect", "connectType", &config->connectType); // ���ӷ�ʽ

	Ini_GetStringCopy(iniHandle, "Connect", "srcAddr", &buf); // ��Դ��ַ
	if (buf != NULL)
	{
		strcpy(config->srcAddr, buf);
		free(buf);
		buf = NULL;
	}
	
	Ini_GetStringCopy(iniHandle, "Mader", "mader_type", &buf); // ����_�ͺ�
	if (buf != NULL)
	{
		strcpy(config->mader_type, buf);
		free(buf);
		buf = NULL;
	}

	Ini_GetInt(iniHandle, "Connect", "baudRate", &config->baudRate); // ������

	Ini_GetInt(iniHandle, "Test", "testItem", &config->testItem); // ������Ŀ

	Ini_GetDouble(iniHandle, "Test", "testRange", &config->range); // ����

	Ini_GetInt(iniHandle, "Test", "dataWidth", &config->digit); // λ��

	Ini_GetDouble(iniHandle, "Test", "testSapce", &config->testSapce); // ���Լ��

	Ini_GetDouble(iniHandle, "Test", "testTime", &config->testTime); // ����ʱ��

	Ini_GetInt(iniHandle, "Test", "testTimeUseFlag", &config->testTimeUseFlag); // ���ò���ʱ��

	Ini_GetInt(iniHandle, "SaveData", "autoSaveFlag", &config->autoSaveFlag); // �����Զ��洢

	Ini_GetStringCopy(iniHandle, "SaveData", "savePath", &buf); // �洢·��
	if (buf != NULL)
	{
		strcpy(config->savePath, buf);
		free(buf);
		buf = NULL;
	}

	Ini_Dispose(iniHandle);
	return 0;
}