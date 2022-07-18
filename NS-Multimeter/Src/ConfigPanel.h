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
/* ��������ö�� */
enum ConnectType_en
{
	RS232 = 0,
	GPIB,
	LAN,
	USB
};

//==============================================================================
// External variables
/* ������ӡ�������� */
extern int configPanelHandle;
/* ������ӡ����������ر�־ */
extern volatile int configPanelHandleFlag;

/* �Ҳನ����� */
extern int wavePanelHandle;
/* �Ҳನ�������ر�־ */
extern volatile int wavePanelHandleFlag;

/* ����� */
extern int panelHandle;

/* ����ʧ����� */
extern int connectFaildPanelHandle;
/* ����ʧ�������ر�־ */
extern volatile int connectFaildPanelHandleFlag;

/* VISA��Դ���� */
extern ViSession defaultRM;
/* �豸��� */
extern ViSession viDev;
/* ��ǰ�������� */
extern int connectType;
/* �豸���� */
extern char viDevName[1024];
/* ������ */
extern ViUInt32 baudRate;

/* ����_�ͺ� */
extern char mader_type[1204];

/* ����·�� */
extern char projectDir[1024];
/* ���Խ������·�� */
extern char dirPath[1024];

/* �Զ��洢��־ */
extern volatile int autoSaveFlag;
/* ���ò���ʱ����־ */
extern volatile int useFlag;
/* ��ǰ������Ŀ */
extern int testItem;
/* ���� */
extern double range;
/* λ������ */
extern int digit;
/* ���������s�� */
extern double samplingSpace;
/* ����ʱ����s�� */
extern double testTime;

#ifdef __cplusplus
    }
#endif

#endif  /* ndef __ConfigPanel_H__ */