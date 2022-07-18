//==============================================================================
//
// Title:		PlotPanel.c
// Purpose:		采集数据和画波形
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
#define DEFAULT_ARR_SIZE 100000UL // 默认数组元素个数(10w)

//==============================================================================
// Static global variables
static size_t cap;				// 容器的容量

static size_t samplingCount; 	// 采样点数计数

static double curVal;	   		// 当前采样的值
static double *pCurValArr; 		// 当前采样的值素数组指针

static double curTime;	   		// 当前采样的时间
static double *pCurTimeArr; 	// 当前采样的时间数组指针

static double maxVal;	   		// 最大值
static double *pMaxValArr;	 	// 最大值数组指针

static double minVal;	   		// 最小值
static double *pMinValArr; 		// 最小值数组指针

static double sum;		   		// 采样点数数值总和
static double avgVal;	   		// 平均值
static double *pAvgValArr; 		// 平均值数组指针

/* 测试进行标志 */
static volatile int testRunFlag;

/* 采样线程id */
CmtThreadFunctionID samplingThreadId;

/* 画图线程id */
CmtThreadFunctionID PlotThreadId;

//==============================================================================
// Static functions
/* 自动存储 */
void AutoSave()
{
	char path[1024] = {0};			  // 用户输入或者选择的路径
	char fileName[1024] = {0};		  // 最终的文件名
	char lineBuf[1024] = {0};		  // 文本框中每行文本
	int lineCount;					  // 文本框中文本的行数
	FILE *fp = NULL;				  // 读写文件的指针
	char defaultFileType[1024] = {0}; // 默认文件类型
	char testItemName[1024] = {0};	  // 当前测试项目名称
	int idx;						  // 当前选中的下拉列表索引
	char time[1024] = {0};			  // 存放当前时间
	char symb[32] = {0}; // 测试项目的符号A V Ω
	char pathTip[1024] = "测试数据文件存放在";

	// 获取文本行数
	GetNumTextBoxLines(configPanelHandle, PANEL_CONF_TEXTBOX_DATA_PATH, &lineCount);

	// 用户清空了路径编辑框
	if (lineCount == 0)
	{
		// 获取工程路径
		GetProjectDir(projectDir);

		// 跳到上一级路径
		strcat(projectDir, "\\..");

		// 进入工程目录
		SetDir(projectDir);

		// 创建存放数据的文件夹（若文件夹已存在，则创建失败）
		MakeDir("TestData");

		// 保存测试数据到TestData文件夹下
		strcat(projectDir, "\\TestData");

		// 把文件保存到工程目录下的一个文件夹
		strcpy(path, projectDir);
	}

	// 拼接路径
	for (size_t i = 0; i < lineCount; ++i)
	{
		GetTextBoxLine(configPanelHandle, PANEL_CONF_TEXTBOX_DATA_PATH, i, lineBuf);
		strcat(path, lineBuf);
	}

	// 切换路径
	SetDir(path);

	// 获取当前时间
	strcpy(time, DateStr());
	strcat(time, "-");
	strcat(time, TimeStr());
	*strstr(time, ":") = '.';
	*strstr(time, ":") = '.';

	// 获取测试项目
	GetCtrlIndex(configPanelHandle, PANEL_CONF_RING_TEST_ITEM, &idx);
	GetLabelFromIndex(configPanelHandle, PANEL_CONF_RING_TEST_ITEM, idx, testItemName);
	sprintf(defaultFileType, "%s测试数据%s.csv", testItemName, time);

	// 文件名
	strcpy(fileName, defaultFileType);

	// 文件名前 根盘符字母大写，如果是小写，则转换成大写
	if (fileName[0] >= 'a' && fileName[0] <= 'z')
	{
		fileName[0] -= ('a' - 'A');
	}

	// 打开文件，若文件不存在则创建新文件
	if ((fp = fopen(fileName, "w+")) == NULL)
	{
		return;
	}

	// 测试项目的符号
	if (strstr(testItemName, "电压") != NULL) // 电压
	{
		strcpy(symb, "V");
	}
	else if (strstr(testItemName, "电流") != NULL) // 电流
	{
		strcpy(symb, "A");
	}
	else // 电阻
	{
		strcpy(symb, "Ω");
	}

	// 写入数据
	fprintf(fp, "%s(s),%s(%s)\n", "时间", testItemName, symb);
	fprintf(fp, "采样点数：%d,平均值：%lf，最大值：%lf，最小值：%lf\n", samplingCount, avgVal, maxVal, minVal);
	for (size_t i = 0; i < samplingCount; ++i)
	{
		fprintf(fp, "%lf,%lf\n", pCurTimeArr[i], pCurValArr[i]);
	}

	// 关闭文件流
	fclose(fp);

	// 释放内存
	FreeMemory();
	
	// 用户清空了路径编辑框
	if (lineCount == 0)
	{
		strcat(pathTip, path);
		// 给用户提示测试结果存放路径
		MessagePopup("提示", pathTip);
	}
}

/* 设置y轴量程 */
void SetYRange()
{
	double maxRange = 0, maxRange2 = 0;
	int count = 0;
	double *rangeArr = NULL;

	// 获取量程个数
	GetNumListItems(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, &count);
	if (count <= 0)
	{
		return;
	}

	// 获取当前量程
	GetCtrlVal(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, &range);

	if (range <= 0) // 自动量程
	{
		// 开辟空间用来存放所有量程
		rangeArr = (double *)malloc(sizeof(double) * count);
		if (rangeArr == NULL) // 开辟空间失败，则选择
		{
			// 最后一个索引对应的量程
			GetValueFromIndex(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, count - 1, &maxRange);

			// 第一个索引对应的量程
			GetValueFromIndex(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, 0, &maxRange2);

			maxRange = maxRange > maxRange2 ? maxRange : maxRange2;
		}

		// 获取所有量程
		for (size_t i = 0; i < count; i++)
		{
			GetValueFromIndex(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, i, rangeArr + i);
		}

		// 找最大量程
		maxRange = rangeArr[0];
		for (size_t i = 1; i < count; ++i)
		{
			maxRange = maxRange > rangeArr[i] ? maxRange : rangeArr[i];
		}

		if (rangeArr != NULL)
		{
			free(rangeArr);
		}

		// 设置y轴量程
		SetAxisScalingMode(wavePanelHandle, PANEL_WAVE_GRAPH, VAL_LEFT_YAXIS, VAL_MANUAL, -maxRange, maxRange);
		return;
	}

	// 设置y轴量程
	SetAxisScalingMode(wavePanelHandle, PANEL_WAVE_GRAPH, VAL_LEFT_YAXIS, VAL_MANUAL, -range, range);
}

// 释放内存
void FreeMemory()
{
	// 释放内存
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

/* 采样线程 */
int CVICALLBACK SamplingThread(void *functionData)
{
	long long startTest = 0, endTest = 0, end = 0, spaceTimeMs = 0, remainTimeMs = 0;
	double *tmp = NULL;
	char tmpPath[1024] = {0};
	int flag = 1;
	
	// 获取当前测试项目
	GetCtrlVal(configPanelHandle, PANEL_CONF_RING_TEST_ITEM, &testItem);
	
	// 不用在循环内每次运算一遍
	spaceTimeMs = samplingSpace * 1000;
	
	while (testRunFlag) // 只要测试进行，就持续采集数据
	{
		// 采集数据开始
		startTest = GetTickCount();
		
		if (MulDataMeasure(viDev, testItem, &curVal) != 0)
		{
			// 采集数据失败，可能是仪器断开了
			viClose(viDev);
			// 清空仪器连接信息
			viDev = 0;
			// 停止测试
			testRunFlag = 0;
			// 仪器断开连接
			connectState = 0;
			// 清空测试时长
			testTime = 0;
			// 更新文字说明
			SetCtrlVal(configPanelHandle, PANEL_CONF_TEXTMSG_CONN_STATE, "连接断开！");
			
			// 获取工程路径
			GetProjectDir(projectDir);
			// 计算红色图片的路径，更新红色图片
			strcpy(tmpPath, projectDir);
			strcat(tmpPath, "\\Icon\\connectFaild.png");
			DisplayImageFile(configPanelHandle, PANEL_CONF_PICTURE_CONNECT, tmpPath);

			// 更换停止测试按钮图片为开始测试
			strcpy(tmpPath, projectDir);
			strcat(tmpPath, "\\Icon\\startTest.png");
			SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_START_TEST, ATTR_IMAGE_FILE, tmpPath);
			
			// 更换断开连接按钮为连接
			strcpy(tmpPath, projectDir);
			strcat(tmpPath, "\\Icon\\connect_1.png");
			SetCtrlAttribute(configPanelHandle, PANEL_CONF_PBTN_CONNECT, ATTR_IMAGE_FILE, tmpPath);

			// 弹窗警告
			MessagePopup("警告！", "仪器断开了，请重新连接！");
			return -1;
		}
	
		//----------------------- 采样点数+1 -----------------------//
		++samplingCount;
		
		// 判断需不需要扩容
		if (samplingCount >= cap)
		{
			cap *= 1.5;

			// 当前值
			tmp = (double *)malloc(sizeof(double) * cap);
			memmove(tmp, pCurValArr, sizeof(double) * samplingCount);
			free(pCurValArr);
			pCurValArr = tmp;

			// 最大值
			tmp = (double *)malloc(sizeof(double) * cap);
			memmove(tmp, pMaxValArr, sizeof(double) * samplingCount);
			free(pMaxValArr);
			pMaxValArr = tmp;

			// 最小值
			tmp = (double *)malloc(sizeof(double) * cap);
			memmove(tmp, pMinValArr, sizeof(double) * samplingCount);
			free(pMinValArr);
			pMinValArr = tmp;

			// 平均值
			tmp = (double *)malloc(sizeof(double) * cap);
			memmove(tmp, pAvgValArr, sizeof(double) * samplingCount);
			free(pAvgValArr);
			pAvgValArr = tmp;

			// 时间
			tmp = (double *)malloc(sizeof(double) * cap);
			memmove(tmp, pCurTimeArr, sizeof(double) * samplingCount);
			free(pCurTimeArr);
			pCurTimeArr = tmp;
		}
		
		// 当前值
		pCurValArr[samplingCount - 1] = curVal;

		// 最大值
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

		// 最小值
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

		// 平均值
		sum += curVal;
		avgVal = sum / samplingCount;
		for (size_t i = 0; i < samplingCount; ++i)
		{
			pAvgValArr[i] = avgVal;
		}
		
		// 采集结束
		endTest = GetTickCount();
		// 延时 间隔时间 - 测试用的时间
		remainTimeMs = spaceTimeMs - (endTest - startTest);
		if (remainTimeMs > 0)
		{
			Sleep((DWORD)remainTimeMs); // 毫秒	
		}

		// 结束
		end = GetTickCount();
		
		// 当前时间
		curTime += ((end - startTest) / 1000.0); // 秒
		pCurTimeArr[samplingCount] = curTime;
		
		// 采一个点后开始画图
		if (flag == 1)
		{
			CmtScheduleThreadPoolFunction(DEFAULT_THREAD_POOL_HANDLE, PlotThread, NULL, &PlotThreadId);
			flag = 0;
		}
		
		// 判断是否需要倒计时
		if (useFlag == 1) // 用户勾选启用测试时长
		{
			testTime -= ((end - startTest) / 1000.0); // 测试时长 - 采一个点画的时间
			if (testTime < 0.0) // 测试结束
			{
				testRunFlag = 0;
				
				// 获取工程路径
				GetProjectDir(projectDir);
				// 更换按钮图片为开始测试
				strcpy(tmpPath, projectDir);
				strcat(tmpPath, "\\Icon\\startTest.png");
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_START_TEST, ATTR_IMAGE_FILE, tmpPath);

				// 数据存储按钮设置为可用
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_SAVE_DATA, ATTR_DIMMED, 0);
				// 更换数据存储图片
				strcpy(tmpPath, projectDir);
				strcat(tmpPath, "\\Icon\\saveData_1.png");
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_SAVE_DATA, ATTR_IMAGE_FILE, tmpPath);

				// 获取自动存储标志
				GetCtrlVal(configPanelHandle, PANEL_CONF_PTBTN_AUTOSAVE, &autoSaveFlag);
				// 用户选择了自动存储
				if (autoSaveFlag)
				{
					AutoSave();
				}

				// 释放内存
				FreeMemory();
				return 0;
			}
		}
	}
	// while 结束
	return 0;
}

/* 画图线程 */
int CVICALLBACK PlotThread(void *functionData)
{
	long long startTest = 0, endTest = 0, spaceTimeMs = 0, remainTimeMs = 0;
	char buf[64] = {0};
	
	// 不用在循环内每次运算一遍
	spaceTimeMs = samplingSpace * 1000;
	
	while (testRunFlag)
	{
		// 开始
		startTest = GetTickCount();
		
		// 删除之前的波形
		if (testRunFlag)
		{
			DeleteGraphPlot(wavePanelHandle, PANEL_WAVE_GRAPH, -1, VAL_IMMEDIATE_DRAW);
		}
		
		// 当前值波形
		PlotXY(wavePanelHandle, PANEL_WAVE_GRAPH, pCurTimeArr, pCurValArr, samplingCount, VAL_DOUBLE, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_GREEN);
		
		// 最大值波形
		PlotXY(wavePanelHandle, PANEL_WAVE_GRAPH, pCurTimeArr, pMaxValArr, samplingCount, VAL_DOUBLE, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_BLUE);
		// 更新标签位置
		sprintf(buf, "最大值：%.3lf", maxVal);
		PlotText(wavePanelHandle, PANEL_WAVE_GRAPH, pCurTimeArr[samplingCount - 1], maxVal, buf, VAL_APP_META_FONT, VAL_BLUE, VAL_TRANSPARENT);
		
		// 最小值波形
		PlotXY(wavePanelHandle, PANEL_WAVE_GRAPH, pCurTimeArr, pMinValArr, samplingCount, VAL_DOUBLE, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_BLUE);
		// 更新标签位置
		sprintf(buf, "最小值：%.3lf", minVal);
		PlotText(wavePanelHandle, PANEL_WAVE_GRAPH, pCurTimeArr[samplingCount - 1], minVal, buf, VAL_APP_META_FONT, VAL_BLUE, VAL_TRANSPARENT);
		
		// 平均值波形
		PlotXY(wavePanelHandle, PANEL_WAVE_GRAPH, pCurTimeArr, pAvgValArr, samplingCount, VAL_DOUBLE, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_DK_YELLOW);
		// 更新标签位置
		sprintf(buf, "平均值：%.3lf", avgVal);
		PlotText(wavePanelHandle, PANEL_WAVE_GRAPH, pCurTimeArr[samplingCount - 1], avgVal, buf, VAL_APP_META_FONT, VAL_DK_YELLOW, VAL_TRANSPARENT);
		
		// 结束
		endTest = GetTickCount();
		
		// 延时 间隔时间 - 测试用的时间
		remainTimeMs = spaceTimeMs - (endTest - startTest);
		if (remainTimeMs > 0)
		{
			Sleep((DWORD)remainTimeMs); // 毫秒	
		}
	}
	return 0;
}

//==============================================================================
// Global functions
/* 开始测试的图片按钮 */
int CVICALLBACK PBtnStartTest(int panel, int control, int event,
							  void *callbackData, int eventData1, int eventData2)
{
	char buf[32] = {0};
	char tmpPath[1024] = {0};
	
	switch (event)
	{
		case EVENT_COMMIT:
			// 获取工程路径
			GetProjectDir(projectDir);
			// 判断是否正在进行测试
			if (testRunFlag == 0) // 没有进行测试，点击按钮则开始测试
			{
				// 释放内存
				FreeMemory();

				// 创建当前值，最大值，最小值，平均值，时间 的容器
				pCurValArr = malloc(sizeof(double) * DEFAULT_ARR_SIZE);
				pMaxValArr = malloc(sizeof(double) * DEFAULT_ARR_SIZE);
				pMinValArr = malloc(sizeof(double) * DEFAULT_ARR_SIZE);
				pAvgValArr = malloc(sizeof(double) * DEFAULT_ARR_SIZE);
				pCurTimeArr = malloc(sizeof(double) * DEFAULT_ARR_SIZE);
				// 记录容量
				cap = DEFAULT_ARR_SIZE;

				// 数据采集初始化
				samplingCount = curVal = maxVal = minVal = avgVal = sum = curTime = 0;
				memset(pCurValArr, 0, sizeof(double) * cap);
				memset(pMaxValArr, 0, sizeof(double) * cap);
				memset(pMinValArr, 0, sizeof(double) * cap);
				memset(pAvgValArr, 0, sizeof(double) * cap);
				memset(pCurTimeArr, 0, sizeof(double) * cap);
				
				// 清除之前的波形
				DeleteGraphPlot(wavePanelHandle, PANEL_WAVE_GRAPH, -1, VAL_IMMEDIATE_DRAW);
				
				// 读取采样间隔
				GetCtrlVal(configPanelHandle, PANEL_CONF_STRING_TIME_SPACE, buf);
				// 字符串转double
				samplingSpace = atof(buf);
				if (samplingSpace < 0) // 采样间隔不能太小
				{
					MessagePopup("警告！", "测量间隔时间必须大于0秒");
					FreeMemory();
					return -1;
				}
				
				// 判断用户是否勾选启用
				GetCtrlVal(configPanelHandle, PANEL_CONF_PTBTN_USE_TESTTIME, &useFlag);
				// 读取测试时长
				GetCtrlVal(configPanelHandle, PANEL_CONF_STRING_TEST_TIME, buf);
				// 字符串转double
				testTime = atof(buf);
				
				// 测试进行标志置1
				testRunFlag = 1;
				
				// 采样线程
				CmtScheduleThreadPoolFunction(DEFAULT_THREAD_POOL_HANDLE, SamplingThread, NULL, &samplingThreadId);

				// 开启框选放大局部图像功能
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_GRAPH, ATTR_ZOOM_STYLE, VAL_ZOOM_TO_RECT);
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_GRAPH, ATTR_ENABLE_ZOOM_AND_PAN, 1);
				
				// 更换按钮图片为停止测试
				strcpy(tmpPath, projectDir);
				strcat(tmpPath, "\\Icon\\stopTest.png");
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_START_TEST, ATTR_IMAGE_FILE, tmpPath);

				// 用户提示 可见
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PICTURE_VECTOR_0, ATTR_VISIBLE, 1);
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_TEXTMSG_WAVE_TIP, ATTR_VISIBLE, 1);
				// 恢复图像按钮 可见
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_RECOVER_IMG, ATTR_VISIBLE, 1);

				// 数据存储按钮设置为不可用
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_SAVE_DATA, ATTR_DIMMED, 1);
				// 更换数据存储按钮图片为灰色
				strcpy(tmpPath, projectDir);
				strcat(tmpPath, "\\Icon\\saveData_0.png");
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_SAVE_DATA, ATTR_IMAGE_FILE, tmpPath);
			}
			else // 正在测试中，点击按钮则停止测试
			{
				// 测试进行标志置0
				testRunFlag = 0;
				
				// 清空测试时间
				testTime = 0;
				
				// 更换按钮图片为开始测试
				strcpy(tmpPath, projectDir);
				strcat(tmpPath, "\\Icon\\startTest.png");
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_START_TEST, ATTR_IMAGE_FILE, tmpPath);

				// 数据存储按钮设置为可用
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_SAVE_DATA, ATTR_DIMMED, 0);
				// 更换数据存储图片
				strcpy(tmpPath, projectDir);
				strcat(tmpPath, "\\Icon\\saveData_1.png");
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_SAVE_DATA, ATTR_IMAGE_FILE, tmpPath);

				// 释放内存
				FreeMemory();
			}
			break;
	}
	return 0;
}

/* 保存数据的图按钮 */
int CVICALLBACK PBtnSaveData(int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	char path[1024] = {0};			  // 用户输入或者选择的路径
	char fileName[1024] = {0};		  // 最终的文件名
	char lineBuf[1024] = {0};		  // 文本框中每行文本
	int lineCount;					  // 文本框中文本的行数
	FILE *fp = NULL;				  // 读写文件的指针
	char defaultFileType[1024] = {0}; // 默认文件类型
	char testItemName[64] = {0};	  // 测试项目名称
	int idx;						  // 当前下拉列表的索引
	char symb[32] = {0};		      // 测试项目的符号A V Ω
	char projectDir[1024] = {0};

	switch (event)
	{
		case EVENT_COMMIT:
			// 获取文本行数
			GetNumTextBoxLines(configPanelHandle, PANEL_CONF_TEXTBOX_DATA_PATH, &lineCount);

			// 用户清空了路径编辑框
			if (lineCount == 0)
			{
				// 获取工程路径
				GetProjectDir(projectDir);

				// 进入工程目录
				SetDir(projectDir);

				// 创建存放数据的文件夹（若文件夹已存在，则创建失败）
				MakeDir("TestData");

				// 保存测试数据到TestData文件夹下
				strcat(projectDir, "\\TestData");

				// 把文件保存到工程目录下的一个文件夹
				strcpy(path, projectDir);
			}

			// 拼接路径
			for (size_t i = 0; i < lineCount; i++)
			{
				GetTextBoxLine(configPanelHandle, PANEL_CONF_TEXTBOX_DATA_PATH, i, lineBuf);
				strcat(path, lineBuf);
			}

			GetCtrlIndex(configPanelHandle, PANEL_CONF_RING_TEST_ITEM, &idx);
			GetLabelFromIndex(configPanelHandle, PANEL_CONF_RING_TEST_ITEM, idx, testItemName);
			sprintf(defaultFileType, "%s测试数据.csv", testItemName);

			// 弹出 选择文件 界面
			if (FileSelectPopup(path, defaultFileType, "", "选择文件", VAL_SAVE_BUTTON, 0, 0, 1, 1, fileName) == 0) // 没有新建或者选择文件
			{
				return 0; // 退出
			}

			// 文件名前 根盘符字母大写
			if (fileName[0] >= 'a' && fileName[0] <= 'z')
			{
				fileName[0] -= ('a' - 'A');
			}

			// 打开文件，若文件不存在则创建新文件
			if ((fp = fopen(fileName, "w+")) == NULL)
			{
				// 出错
				return -1;
			}

			// 测试项目的符号
			if (strstr(testItemName, "电压") != NULL) // 电压
			{
				strcpy(symb, "V");
			}
			else if (strstr(testItemName, "电流") != NULL) // 电流
			{
				strcpy(symb, "A");
			}
			else // 电阻
			{
				strcpy(symb, "Ω");
			}

			// 写入数据
			fprintf(fp, "%s(s),%s(%s)\n", "时间", testItemName, symb);
			fprintf(fp, "采样点数：%d,平均值：%lf，最大值：%lf，最小值：%lf\n", samplingCount, avgVal, maxVal, minVal);
			for (size_t i = 0; i < samplingCount; ++i)
			{
				fprintf(fp, "%lf,%lf\n", pCurTimeArr[i], pCurValArr[i]);
			}
			
			// 关闭文件流
			fclose(fp);
			break;
	}
	return 0;
}

/* 恢复图像按钮 */
int CVICALLBACK PBtnRecoverImage(int panel, int control, int event,
								 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// 获取当前量程
			GetCtrlVal(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, &range);

			// 设置y轴量程
			SetAxisScalingMode(wavePanelHandle, PANEL_WAVE_GRAPH, VAL_LEFT_YAXIS, VAL_MANUAL, -range, range);
			break;
	}
	return 0;
}

/* 波形面板回调 */
int CVICALLBACK WavePanelCB(int panel, int event, void *callbackData,
							int eventData1, int eventData2)
{
	int btnX, btnY; // 按钮当前的位置
	int x, y;		// 鼠标 当前相对 取消授权按钮 的 位置
	int state;		// 取消授权按钮 当前的 状态（是否被隐藏）

	switch (event)
	{
		case EVENT_LEFT_CLICK: // 单击面板 取消授权按钮 以外的地方，隐藏 取消授权按钮
			// 获取 取消授权按钮 当前的 状态（是否被隐藏）
			GetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_CANCEL_AUTHOR, ATTR_VISIBLE, &state);
			if (state) // 取消授权按钮 当前没有被隐藏
			{
				// 获取 鼠标 当前相对 取消授权按钮 的 位置
				GetRelativeMouseState(wavePanelHandle, PANEL_WAVE_PBTN_CANCEL_AUTHOR, &x, &y, NULL, NULL, NULL);

				// 获取按钮当前位置
				GetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_CANCEL_AUTHOR, ATTR_LEFT, &btnX);
				GetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_CANCEL_AUTHOR, ATTR_TOP, &btnY);

				// 鼠标 在 取消授权按钮 以外的地方
				if (x < btnX || x > 107 + btnX || y < btnY || y > 42 + btnY)
				{
					// 隐藏 取消授权按钮
					SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_CANCEL_AUTHOR, ATTR_VISIBLE, 0);
				}
			}
			break;
	}
	return 0;
}

/* graph控件回调 */
int CVICALLBACK GraphCB (int panel, int control, int event,
						 void *callbackData, int eventData1, int eventData2)
{
	double curX, curY; // 游标当前的坐标
	char val[32] = {0};

	switch (event)
	{
		case EVENT_LEFT_CLICK:
			// 获取鼠标位置
			SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_TEXTMSG_CURSORS, ATTR_TOP, eventData1);
			SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_TEXTMSG_CURSORS, ATTR_LEFT, eventData2);
			
		case EVENT_COMMIT:
			// 获取数值值
			GetGraphCursor(wavePanelHandle, PANEL_WAVE_GRAPH, 1, &curX, &curY);
			sprintf(val, "%.6lf", curY);
			SetCtrlVal(wavePanelHandle, PANEL_WAVE_TEXTMSG_CURSORS, val);
			break;
	}
	return 0;
}