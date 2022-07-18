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
	int connectType; // ���ӷ�ʽ
	char srcAddr[128]; // ��Դ��ַ
	int baudRate; // ������
	char mader_type[128]; // ����_�ͺ�
	int testItem; // ������Ŀ
	double range; // ���� V A ��
	int digit; // λ��
	double testSapce; // ���Լ��
	double testTime; // ����ʱ��
	int testTimeUseFlag; // ���ò���ʱ��
	int autoSaveFlag; // �����Զ��洢
	
	char savePath[1024]; // �洢·��
} InstrConfig;

//==============================================================================
// External variables
extern char projectDir[1024];

//==============================================================================
// Global functions
// �������õ�ini�ļ�
int SaveConfigToIniFile(const InstrConfig* config);

// ��ini�ļ���������
int LoadConfigFromIniFile(int panel, InstrConfig* config);

#ifdef __cplusplus
    }
#endif

#endif  /* ndef __SaveConfig_H__ */