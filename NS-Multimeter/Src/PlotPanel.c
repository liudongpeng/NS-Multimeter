//==============================================================================
//
// Title:		PlotPanel.c
// Purpose:		�ɼ����ݺͻ�����
//
// Created on:	2022/7/6 at 9:48:08 by dongpeng liu.
// Copyright:	. All Rights Reserved.
//
//==============================================================================

//==============================================================================
// Include files
#include <Windows.h>
#include <visa.h>
#include <ansi_c.h>
#include <utility.h>

#include "NS-Multimeter.h"

#include "NsuMultimeter.h"
#include "PlotPanel.h"

//==============================================================================
// Constants
#define DEFAULT_ARR_SIZE 100000UL // Ĭ������Ԫ�ظ���(10w)

//==============================================================================
// Static global variables
static size_t cap;				// ����������

static size_t samplingCount; 	// ������������

static double curVal;	   		// ��ǰ������ֵ
static double *pCurValArr; 		// ��ǰ������ֵ������ָ��

static double curTime;	   		// ��ǰ������ʱ��
static double *pCurTimeArr; 	// ��ǰ������ʱ������ָ��

static double maxVal;	   		// ���ֵ
static double *pMaxValArr;	 	// ���ֵ����ָ��

static double minVal;	   		// ��Сֵ
static double *pMinValArr; 		// ��Сֵ����ָ��

static double sum;		   		// ����������ֵ�ܺ�
static double avgVal;	   		// ƽ��ֵ
static double *pAvgValArr; 		// ƽ��ֵ����ָ��

/* ���Խ��б�־ */
static volatile int testRunFlag;

/* �����߳�id */
CmtThreadFunctionID samplingThreadId;

/* ��ͼ�߳�id */
CmtThreadFunctionID PlotThreadId;

//==============================================================================
// Static functions
/* �Զ��洢 */
void AutoSave()
{
	char path[1024] = {0};			  // �û��������ѡ���·��
	char fileName[1024] = {0};		  // ���յ��ļ���
	char lineBuf[1024] = {0};		  // �ı�����ÿ���ı�
	int lineCount;					  // �ı������ı�������
	FILE *fp = NULL;				  // ��д�ļ���ָ��
	char defaultFileType[1024] = {0}; // Ĭ���ļ�����
	char testItemName[1024] = {0};	  // ��ǰ������Ŀ����
	int idx;						  // ��ǰѡ�е������б�����
	char time[1024] = {0};			  // ��ŵ�ǰʱ��
	char symb[32] = {0}; // ������Ŀ�ķ���A V ��
	char pathTip[1024] = "���������ļ������";

	// ��ȡ�ı�����
	GetNumTextBoxLines(configPanelHandle, PANEL_CONF_TEXTBOX_DATA_PATH, &lineCount);

	// �û������·���༭��
	if (lineCount == 0)
	{
		// ��ȡ����·��
		GetProjectDir(projectDir);

		// ������һ��·��
		strcat(projectDir, "\\..");

		// ���빤��Ŀ¼
		SetDir(projectDir);

		// ����������ݵ��ļ��У����ļ����Ѵ��ڣ��򴴽�ʧ�ܣ�
		MakeDir("TestData");

		// ����������ݵ�TestData�ļ�����
		strcat(projectDir, "\\TestData");

		// ���ļ����浽����Ŀ¼�µ�һ���ļ���
		strcpy(path, projectDir);
	}

	// ƴ��·��
	for (size_t i = 0; i < lineCount; ++i)
	{
		GetTextBoxLine(configPanelHandle, PANEL_CONF_TEXTBOX_DATA_PATH, i, lineBuf);
		strcat(path, lineBuf);
	}

	// �л�·��
	SetDir(path);

	// ��ȡ��ǰʱ��
	strcpy(time, DateStr());
	strcat(time, "-");
	strcat(time, TimeStr());
	*strstr(time, ":") = '.';
	*strstr(time, ":") = '.';

	// ��ȡ������Ŀ
	GetCtrlIndex(configPanelHandle, PANEL_CONF_RING_TEST_ITEM, &idx);
	GetLabelFromIndex(configPanelHandle, PANEL_CONF_RING_TEST_ITEM, idx, testItemName);
	sprintf(defaultFileType, "%s��������%s.csv", testItemName, time);

	// �ļ���
	strcpy(fileName, defaultFileType);

	// �ļ���ǰ ���̷���ĸ��д�������Сд����ת���ɴ�д
	if (fileName[0] >= 'a' && fileName[0] <= 'z')
	{
		fileName[0] -= ('a' - 'A');
	}

	// ���ļ������ļ��������򴴽����ļ�
	if ((fp = fopen(fileName, "w+")) == NULL)
	{
		return;
	}

	// ������Ŀ�ķ���
	if (strstr(testItemName, "��ѹ") != NULL) // ��ѹ
	{
		strcpy(symb, "V");
	}
	else if (strstr(testItemName, "����") != NULL) // ����
	{
		strcpy(symb, "A");
	}
	else // ����
	{
		strcpy(symb, "��");
	}

	// д������
	fprintf(fp, "%s(s),%s(%s)\n", "ʱ��", testItemName, symb);
	fprintf(fp, "����������%d,ƽ��ֵ��%lf�����ֵ��%lf����Сֵ��%lf\n", samplingCount, avgVal, maxVal, minVal);
	for (size_t i = 0; i < samplingCount; ++i)
	{
		fprintf(fp, "%lf,%lf\n", pCurTimeArr[i], pCurValArr[i]);
	}

	// �ر��ļ���
	fclose(fp);

	// �ͷ��ڴ�
	FreeMemory();
	
	// �û������·���༭��
	if (lineCount == 0)
	{
		strcat(pathTip, path);
		// ���û���ʾ���Խ�����·��
		MessagePopup("��ʾ", pathTip);
	}
}

/* ����y������ */
void SetYRange()
{
	double maxRange = 0, maxRange2 = 0;
	int count = 0;
	double *rangeArr = NULL;

	// ��ȡ���̸���
	GetNumListItems(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, &count);
	if (count <= 0)
	{
		return;
	}

	// ��ȡ��ǰ����
	GetCtrlVal(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, &range);

	if (range <= 0) // �Զ�����
	{
		// ���ٿռ����������������
		rangeArr = (double *)malloc(sizeof(double) * count);
		if (rangeArr == NULL) // ���ٿռ�ʧ�ܣ���ѡ��
		{
			// ���һ��������Ӧ������
			GetValueFromIndex(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, count - 1, &maxRange);

			// ��һ��������Ӧ������
			GetValueFromIndex(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, 0, &maxRange2);

			maxRange = maxRange > maxRange2 ? maxRange : maxRange2;
		}

		// ��ȡ��������
		for (size_t i = 0; i < count; i++)
		{
			GetValueFromIndex(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, i, rangeArr + i);
		}

		// ���������
		maxRange = rangeArr[0];
		for (size_t i = 1; i < count; ++i)
		{
			maxRange = maxRange > rangeArr[i] ? maxRange : rangeArr[i];
		}

		if (rangeArr != NULL)
		{
			free(rangeArr);
		}

		// ����y������
		SetAxisScalingMode(wavePanelHandle, PANEL_WAVE_GRAPH, VAL_LEFT_YAXIS, VAL_MANUAL, -maxRange, maxRange);
		return;
	}

	// ����y������
	SetAxisScalingMode(wavePanelHandle, PANEL_WAVE_GRAPH, VAL_LEFT_YAXIS, VAL_MANUAL, -range, range);
}

// �ͷ��ڴ�
void FreeMemory()
{
	// �ͷ��ڴ�
	if (!pCurValArr)
	{
		free(pCurValArr);
		pCurValArr = NULL;
	}

	if (!pMaxValArr)
	{
		free(pMaxValArr);
		pMaxValArr = NULL;
	}

	if (!pMinValArr)
	{
		free(pMinValArr);
		pMinValArr = NULL;
	}

	if (!pAvgValArr)
	{
		free(pAvgValArr);
		pAvgValArr = NULL;
	}

	if (!pCurTimeArr)
	{
		free(pCurTimeArr);
		pCurTimeArr = NULL;
	}
}

/* �����߳� */
int CVICALLBACK SamplingThread(void *functionData)
{
	long long startTest = 0, endTest = 0, end = 0, spaceTimeMs = 0, remainTimeMs = 0;
	double *tmp = NULL;
	char tmpPath[1024] = {0};
	int flag = 1;
	
	// ��ȡ��ǰ������Ŀ
	GetCtrlVal(configPanelHandle, PANEL_CONF_RING_TEST_ITEM, &testItem);
	
	// ������ѭ����ÿ������һ��
	spaceTimeMs = samplingSpace * 1000;
	
	while (testRunFlag) // ֻҪ���Խ��У��ͳ����ɼ�����
	{
		// �ɼ����ݿ�ʼ
		startTest = GetTickCount();
		
		if (MulDataMeasure(viDev, testItem, &curVal) != 0)
		{
			// �ɼ�����ʧ�ܣ������������Ͽ���
			viClose(viDev);
			// �������������Ϣ
			viDev = 0;
			// ֹͣ����
			testRunFlag = 0;
			// �����Ͽ�����
			connectState = 0;
			// ��ղ���ʱ��
			testTime = 0;
			// ��������˵��
			SetCtrlVal(configPanelHandle, PANEL_CONF_TEXTMSG_CONN_STATE, "���ӶϿ���");
			
			// ��ȡ����·��
			GetProjectDir(projectDir);
			// �����ɫͼƬ��·�������º�ɫͼƬ
			strcpy(tmpPath, projectDir);
			strcat(tmpPath, "\\Icon\\connectFaild.png");
			DisplayImageFile(configPanelHandle, PANEL_CONF_PICTURE_CONNECT, tmpPath);

			// ����ֹͣ���԰�ťͼƬΪ��ʼ����
			strcpy(tmpPath, projectDir);
			strcat(tmpPath, "\\Icon\\startTest.png");
			SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_START_TEST, ATTR_IMAGE_FILE, tmpPath);
			
			// �����Ͽ����Ӱ�ťΪ����
			strcpy(tmpPath, projectDir);
			strcat(tmpPath, "\\Icon\\connect_1.png");
			SetCtrlAttribute(configPanelHandle, PANEL_CONF_PBTN_CONNECT, ATTR_IMAGE_FILE, tmpPath);

			// ��������
			MessagePopup("���棡", "�����Ͽ��ˣ����������ӣ�");
			return -1;
		}
	
		//----------------------- ��������+1 -----------------------//
		++samplingCount;
		
		// �ж��費��Ҫ����
		if (samplingCount >= cap)
		{
			cap *= 1.5;

			// ��ǰֵ
			tmp = (double *)malloc(sizeof(double) * cap);
			memmove(tmp, pCurValArr, sizeof(double) * samplingCount);
			free(pCurValArr);
			pCurValArr = tmp;

			// ���ֵ
			tmp = (double *)malloc(sizeof(double) * cap);
			memmove(tmp, pMaxValArr, sizeof(double) * samplingCount);
			free(pMaxValArr);
			pMaxValArr = tmp;

			// ��Сֵ
			tmp = (double *)malloc(sizeof(double) * cap);
			memmove(tmp, pMinValArr, sizeof(double) * samplingCount);
			free(pMinValArr);
			pMinValArr = tmp;

			// ƽ��ֵ
			tmp = (double *)malloc(sizeof(double) * cap);
			memmove(tmp, pAvgValArr, sizeof(double) * samplingCount);
			free(pAvgValArr);
			pAvgValArr = tmp;

			// ʱ��
			tmp = (double *)malloc(sizeof(double) * cap);
			memmove(tmp, pCurTimeArr, sizeof(double) * samplingCount);
			free(pCurTimeArr);
			pCurTimeArr = tmp;
		}
		
		// ��ǰֵ
		pCurValArr[samplingCount - 1] = curVal;

		// ���ֵ
		if (samplingCount <= 1)
		{
			maxVal = curVal;
		}
		else
		{
			maxVal = maxVal > curVal ? maxVal : curVal;
		}
		for (size_t i = 0; i < samplingCount; ++i)
		{
			pMaxValArr[i] = maxVal;
		}

		// ��Сֵ
		if (samplingCount <= 1)
		{
			minVal = curVal;
		}
		else
		{
			minVal = minVal < curVal ? minVal : curVal;
		}
		for (size_t i = 0; i < samplingCount; ++i)
		{
			pMinValArr[i] = minVal;
		}

		// ƽ��ֵ
		sum += curVal;
		avgVal = sum / samplingCount;
		for (size_t i = 0; i < samplingCount; ++i)
		{
			pAvgValArr[i] = avgVal;
		}
		
		// �ɼ�����
		endTest = GetTickCount();
		// ��ʱ ���ʱ�� - �����õ�ʱ��
		remainTimeMs = spaceTimeMs - (endTest - startTest);
		if (remainTimeMs > 0)
		{
			Sleep((DWORD)remainTimeMs); // ����	
		}

		// ����
		end = GetTickCount();
		
		// ��ǰʱ��
		curTime += ((end - startTest) / 1000.0); // ��
		pCurTimeArr[samplingCount] = curTime;
		
		// ��һ�����ʼ��ͼ
		if (flag == 1)
		{
			CmtScheduleThreadPoolFunction(DEFAULT_THREAD_POOL_HANDLE, PlotThread, NULL, &PlotThreadId);
			flag = 0;
		}
		
		// �ж��Ƿ���Ҫ����ʱ
		if (useFlag == 1) // �û���ѡ���ò���ʱ��
		{
			testTime -= ((end - startTest) / 1000.0); // ����ʱ�� - ��һ���㻭��ʱ��
			if (testTime < 0.0) // ���Խ���
			{
				testRunFlag = 0;
				
				// ��ȡ����·��
				GetProjectDir(projectDir);
				// ������ťͼƬΪ��ʼ����
				strcpy(tmpPath, projectDir);
				strcat(tmpPath, "\\Icon\\startTest.png");
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_START_TEST, ATTR_IMAGE_FILE, tmpPath);

				// ���ݴ洢��ť����Ϊ����
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_SAVE_DATA, ATTR_DIMMED, 0);
				// �������ݴ洢ͼƬ
				strcpy(tmpPath, projectDir);
				strcat(tmpPath, "\\Icon\\saveData_1.png");
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_SAVE_DATA, ATTR_IMAGE_FILE, tmpPath);

				// ��ȡ�Զ��洢��־
				GetCtrlVal(configPanelHandle, PANEL_CONF_PTBTN_AUTOSAVE, &autoSaveFlag);
				// �û�ѡ�����Զ��洢
				if (autoSaveFlag)
				{
					AutoSave();
				}

				// �ͷ��ڴ�
				FreeMemory();
				return 0;
			}
		}
	}
	// while ����
	return 0;
}

/* ��ͼ�߳� */
int CVICALLBACK PlotThread(void *functionData)
{
	long long startTest = 0, endTest = 0, spaceTimeMs = 0, remainTimeMs = 0;
	char buf[64] = {0};
	
	// ������ѭ����ÿ������һ��
	spaceTimeMs = samplingSpace * 1000;
	
	while (testRunFlag)
	{
		// ��ʼ
		startTest = GetTickCount();
		
		// ɾ��֮ǰ�Ĳ���
		if (testRunFlag)
		{
			DeleteGraphPlot(wavePanelHandle, PANEL_WAVE_GRAPH, -1, VAL_IMMEDIATE_DRAW);
		}
		
		// ��ǰֵ����
		PlotXY(wavePanelHandle, PANEL_WAVE_GRAPH, pCurTimeArr, pCurValArr, samplingCount, VAL_DOUBLE, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_GREEN);
		
		// ���ֵ����
		PlotXY(wavePanelHandle, PANEL_WAVE_GRAPH, pCurTimeArr, pMaxValArr, samplingCount, VAL_DOUBLE, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_BLUE);
		// ���±�ǩλ��
		sprintf(buf, "���ֵ��%.3lf", maxVal);
		PlotText(wavePanelHandle, PANEL_WAVE_GRAPH, pCurTimeArr[samplingCount - 1], maxVal, buf, VAL_APP_META_FONT, VAL_BLUE, VAL_TRANSPARENT);
		
		// ��Сֵ����
		PlotXY(wavePanelHandle, PANEL_WAVE_GRAPH, pCurTimeArr, pMinValArr, samplingCount, VAL_DOUBLE, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_BLUE);
		// ���±�ǩλ��
		sprintf(buf, "��Сֵ��%.3lf", minVal);
		PlotText(wavePanelHandle, PANEL_WAVE_GRAPH, pCurTimeArr[samplingCount - 1], minVal, buf, VAL_APP_META_FONT, VAL_BLUE, VAL_TRANSPARENT);
		
		// ƽ��ֵ����
		PlotXY(wavePanelHandle, PANEL_WAVE_GRAPH, pCurTimeArr, pAvgValArr, samplingCount, VAL_DOUBLE, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_DK_YELLOW);
		// ���±�ǩλ��
		sprintf(buf, "ƽ��ֵ��%.3lf", avgVal);
		PlotText(wavePanelHandle, PANEL_WAVE_GRAPH, pCurTimeArr[samplingCount - 1], avgVal, buf, VAL_APP_META_FONT, VAL_DK_YELLOW, VAL_TRANSPARENT);
		
		// ����
		endTest = GetTickCount();
		
		// ��ʱ ���ʱ�� - �����õ�ʱ��
		remainTimeMs = spaceTimeMs - (endTest - startTest);
		if (remainTimeMs > 0)
		{
			Sleep((DWORD)remainTimeMs); // ����	
		}
	}
	return 0;
}

//==============================================================================
// Global functions
/* ��ʼ���Ե�ͼƬ��ť */
int CVICALLBACK PBtnStartTest(int panel, int control, int event,
							  void *callbackData, int eventData1, int eventData2)
{
	char buf[32] = {0};
	char tmpPath[1024] = {0};
	
	switch (event)
	{
		case EVENT_COMMIT:
			// ��ȡ����·��
			GetProjectDir(projectDir);
			// �ж��Ƿ����ڽ��в���
			if (testRunFlag == 0) // û�н��в��ԣ������ť��ʼ����
			{
				// �ͷ��ڴ�
				FreeMemory();

				// ������ǰֵ�����ֵ����Сֵ��ƽ��ֵ��ʱ�� ������
				pCurValArr = malloc(sizeof(double) * DEFAULT_ARR_SIZE);
				pMaxValArr = malloc(sizeof(double) * DEFAULT_ARR_SIZE);
				pMinValArr = malloc(sizeof(double) * DEFAULT_ARR_SIZE);
				pAvgValArr = malloc(sizeof(double) * DEFAULT_ARR_SIZE);
				pCurTimeArr = malloc(sizeof(double) * DEFAULT_ARR_SIZE);
				// ��¼����
				cap = DEFAULT_ARR_SIZE;

				// ���ݲɼ���ʼ��
				samplingCount = curVal = maxVal = minVal = avgVal = sum = curTime = 0;
				memset(pCurValArr, 0, sizeof(double) * cap);
				memset(pMaxValArr, 0, sizeof(double) * cap);
				memset(pMinValArr, 0, sizeof(double) * cap);
				memset(pAvgValArr, 0, sizeof(double) * cap);
				memset(pCurTimeArr, 0, sizeof(double) * cap);
				
				// ���֮ǰ�Ĳ���
				DeleteGraphPlot(wavePanelHandle, PANEL_WAVE_GRAPH, -1, VAL_IMMEDIATE_DRAW);
				
				// ��ȡ�������
				GetCtrlVal(configPanelHandle, PANEL_CONF_STRING_TIME_SPACE, buf);
				// �ַ���תdouble
				samplingSpace = atof(buf);
				if (samplingSpace < 0) // �����������̫С
				{
					MessagePopup("���棡", "�������ʱ��������0��");
					FreeMemory();
					return -1;
				}
				
				// �ж��û��Ƿ�ѡ����
				GetCtrlVal(configPanelHandle, PANEL_CONF_PTBTN_USE_TESTTIME, &useFlag);
				// ��ȡ����ʱ��
				GetCtrlVal(configPanelHandle, PANEL_CONF_STRING_TEST_TIME, buf);
				// �ַ���תdouble
				testTime = atof(buf);
				
				// ���Խ��б�־��1
				testRunFlag = 1;
				
				// �����߳�
				CmtScheduleThreadPoolFunction(DEFAULT_THREAD_POOL_HANDLE, SamplingThread, NULL, &samplingThreadId);

				// ������ѡ�Ŵ�ֲ�ͼ����
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_GRAPH, ATTR_ZOOM_STYLE, VAL_ZOOM_TO_RECT);
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_GRAPH, ATTR_ENABLE_ZOOM_AND_PAN, 1);
				
				// ������ťͼƬΪֹͣ����
				strcpy(tmpPath, projectDir);
				strcat(tmpPath, "\\Icon\\stopTest.png");
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_START_TEST, ATTR_IMAGE_FILE, tmpPath);

				// �û���ʾ �ɼ�
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PICTURE_VECTOR_0, ATTR_VISIBLE, 1);
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_TEXTMSG_WAVE_TIP, ATTR_VISIBLE, 1);
				// �ָ�ͼ��ť �ɼ�
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_RECOVER_IMG, ATTR_VISIBLE, 1);

				// ���ݴ洢��ť����Ϊ������
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_SAVE_DATA, ATTR_DIMMED, 1);
				// �������ݴ洢��ťͼƬΪ��ɫ
				strcpy(tmpPath, projectDir);
				strcat(tmpPath, "\\Icon\\saveData_0.png");
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_SAVE_DATA, ATTR_IMAGE_FILE, tmpPath);
			}
			else // ���ڲ����У������ť��ֹͣ����
			{
				// ���Խ��б�־��0
				testRunFlag = 0;
				
				// ��ղ���ʱ��
				testTime = 0;
				
				// ������ťͼƬΪ��ʼ����
				strcpy(tmpPath, projectDir);
				strcat(tmpPath, "\\Icon\\startTest.png");
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_START_TEST, ATTR_IMAGE_FILE, tmpPath);

				// ���ݴ洢��ť����Ϊ����
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_SAVE_DATA, ATTR_DIMMED, 0);
				// �������ݴ洢ͼƬ
				strcpy(tmpPath, projectDir);
				strcat(tmpPath, "\\Icon\\saveData_1.png");
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_SAVE_DATA, ATTR_IMAGE_FILE, tmpPath);

				// �ͷ��ڴ�
				FreeMemory();
			}
			break;
	}
	return 0;
}

/* �������ݵ�ͼ��ť */
int CVICALLBACK PBtnSaveData(int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	char path[1024] = {0};			  // �û��������ѡ���·��
	char fileName[1024] = {0};		  // ���յ��ļ���
	char lineBuf[1024] = {0};		  // �ı�����ÿ���ı�
	int lineCount;					  // �ı������ı�������
	FILE *fp = NULL;				  // ��д�ļ���ָ��
	char defaultFileType[1024] = {0}; // Ĭ���ļ�����
	char testItemName[64] = {0};	  // ������Ŀ����
	int idx;						  // ��ǰ�����б�������
	char symb[32] = {0};		      // ������Ŀ�ķ���A V ��
	char projectDir[1024] = {0};

	switch (event)
	{
		case EVENT_COMMIT:
			// ��ȡ�ı�����
			GetNumTextBoxLines(configPanelHandle, PANEL_CONF_TEXTBOX_DATA_PATH, &lineCount);

			// �û������·���༭��
			if (lineCount == 0)
			{
				// ��ȡ����·��
				GetProjectDir(projectDir);

				// ���빤��Ŀ¼
				SetDir(projectDir);

				// ����������ݵ��ļ��У����ļ����Ѵ��ڣ��򴴽�ʧ�ܣ�
				MakeDir("TestData");

				// ����������ݵ�TestData�ļ�����
				strcat(projectDir, "\\TestData");

				// ���ļ����浽����Ŀ¼�µ�һ���ļ���
				strcpy(path, projectDir);
			}

			// ƴ��·��
			for (size_t i = 0; i < lineCount; i++)
			{
				GetTextBoxLine(configPanelHandle, PANEL_CONF_TEXTBOX_DATA_PATH, i, lineBuf);
				strcat(path, lineBuf);
			}

			GetCtrlIndex(configPanelHandle, PANEL_CONF_RING_TEST_ITEM, &idx);
			GetLabelFromIndex(configPanelHandle, PANEL_CONF_RING_TEST_ITEM, idx, testItemName);
			sprintf(defaultFileType, "%s��������.csv", testItemName);

			// ���� ѡ���ļ� ����
			if (FileSelectPopup(path, defaultFileType, "", "ѡ���ļ�", VAL_SAVE_BUTTON, 0, 0, 1, 1, fileName) == 0) // û���½�����ѡ���ļ�
			{
				return 0; // �˳�
			}

			// �ļ���ǰ ���̷���ĸ��д
			if (fileName[0] >= 'a' && fileName[0] <= 'z')
			{
				fileName[0] -= ('a' - 'A');
			}

			// ���ļ������ļ��������򴴽����ļ�
			if ((fp = fopen(fileName, "w+")) == NULL)
			{
				// ����
				return -1;
			}

			// ������Ŀ�ķ���
			if (strstr(testItemName, "��ѹ") != NULL) // ��ѹ
			{
				strcpy(symb, "V");
			}
			else if (strstr(testItemName, "����") != NULL) // ����
			{
				strcpy(symb, "A");
			}
			else // ����
			{
				strcpy(symb, "��");
			}

			// д������
			fprintf(fp, "%s(s),%s(%s)\n", "ʱ��", testItemName, symb);
			fprintf(fp, "����������%d,ƽ��ֵ��%lf�����ֵ��%lf����Сֵ��%lf\n", samplingCount, avgVal, maxVal, minVal);
			for (size_t i = 0; i < samplingCount; ++i)
			{
				fprintf(fp, "%lf,%lf\n", pCurTimeArr[i], pCurValArr[i]);
			}
			
			// �ر��ļ���
			fclose(fp);
			break;
	}
	return 0;
}

/* �ָ�ͼ��ť */
int CVICALLBACK PBtnRecoverImage(int panel, int control, int event,
								 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// ��ȡ��ǰ����
			GetCtrlVal(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, &range);

			// ����y������
			SetAxisScalingMode(wavePanelHandle, PANEL_WAVE_GRAPH, VAL_LEFT_YAXIS, VAL_MANUAL, -range, range);
			break;
	}
	return 0;
}

/* �������ص� */
int CVICALLBACK WavePanelCB(int panel, int event, void *callbackData,
							int eventData1, int eventData2)
{
	int btnX, btnY; // ��ť��ǰ��λ��
	int x, y;		// ��� ��ǰ��� ȡ����Ȩ��ť �� λ��
	int state;		// ȡ����Ȩ��ť ��ǰ�� ״̬���Ƿ����أ�

	switch (event)
	{
		case EVENT_LEFT_CLICK: // ������� ȡ����Ȩ��ť ����ĵط������� ȡ����Ȩ��ť
			// ��ȡ ȡ����Ȩ��ť ��ǰ�� ״̬���Ƿ����أ�
			GetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_CANCEL_AUTHOR, ATTR_VISIBLE, &state);
			if (state) // ȡ����Ȩ��ť ��ǰû�б�����
			{
				// ��ȡ ��� ��ǰ��� ȡ����Ȩ��ť �� λ��
				GetRelativeMouseState(wavePanelHandle, PANEL_WAVE_PBTN_CANCEL_AUTHOR, &x, &y, NULL, NULL, NULL);

				// ��ȡ��ť��ǰλ��
				GetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_CANCEL_AUTHOR, ATTR_LEFT, &btnX);
				GetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_CANCEL_AUTHOR, ATTR_TOP, &btnY);

				// ��� �� ȡ����Ȩ��ť ����ĵط�
				if (x < btnX || x > 107 + btnX || y < btnY || y > 42 + btnY)
				{
					// ���� ȡ����Ȩ��ť
					SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_CANCEL_AUTHOR, ATTR_VISIBLE, 0);
				}
			}
			break;
	}
	return 0;
}

/* graph�ؼ��ص� */
int CVICALLBACK GraphCB (int panel, int control, int event,
						 void *callbackData, int eventData1, int eventData2)
{
	double curX, curY; // �α굱ǰ������
	char val[32] = {0};

	switch (event)
	{
		case EVENT_LEFT_CLICK:
			// ��ȡ���λ��
			SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_TEXTMSG_CURSORS, ATTR_TOP, eventData1);
			SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_TEXTMSG_CURSORS, ATTR_LEFT, eventData2);
			
		case EVENT_COMMIT:
			// ��ȡ��ֵֵ
			GetGraphCursor(wavePanelHandle, PANEL_WAVE_GRAPH, 1, &curX, &curY);
			sprintf(val, "%.6lf", curY);
			SetCtrlVal(wavePanelHandle, PANEL_WAVE_TEXTMSG_CURSORS, val);
			break;
	}
	return 0;
}