//==============================================================================
//
// Title:		PlotPanel.h
// Purpose:		�����ⲿ����
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

/* �豸��� */
extern ViSession viDev;

/* ����·�� */
extern char projectDir[1024];

/* ���ò���ʱ����־ */
extern volatile int useFlag;
/* ���������s�� */
extern double samplingSpace;
/* ����ʱ����s�� */
extern double testTime;
/* ��ǰ������Ŀ */
extern int testItem;
/* ���� */
double range;
/* �Զ��洢��־ */
extern volatile int autoSaveFlag;
/* �������ӳɹ���־��0δ���ӣ� 1�����ӣ� */
extern volatile int connectState;


//==============================================================================
// Global functions
/* �Զ��洢 */
void AutoSave();

/* ����y������ */
void SetYRange();

// �ͷ��ڴ�
void FreeMemory();

/* �����߳� */
int CVICALLBACK SamplingThread(void *functionData);

/* ��ͼ�߳� */
int CVICALLBACK PlotThread(void *functionData);

#ifdef __cplusplus
    }
#endif

#endif  /* ndef __PlotPanel_H__ */