//==============================================================================
//
// Title:		ConfigPanel.c
// Purpose:		仪器的连接及配置
//
// Created on:	2022/7/6 at 9:43:43 by dongpeng liu.
// Copyright:	. All Rights Reserved.
//
//==============================================================================

//==============================================================================
// Include files
#include "toolbox.h"
#include <ansi_c.h>
#include <visa.h>

#include "NS-Multimeter.h"

#include "NsSqlite.h"
#include "NsuMultimeter.h"
#include "NsuElement.h"
#include "ConfigPanel.h"
#include "SaveConfig.h"

//==============================================================================
// Global variables
/* 仪器连接成功标志（0未连接， 1已连接） */
volatile int connectState;

//==============================================================================
// Static functions
/* 设置y轴量程 */
static void SetYRange()
{
	double maxRange, maxRange2;
	int count;
	double *rangeArr = NULL;
	double range; // 量程

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
		for (size_t i = 0; i < count; ++i)
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

/* 把仪器资源加载到ring控件供用户选择则 */
static int InsertSrcAddrToRing(int connType)
{
	ViUInt32 viDevNum = 0; 	// 匹配到的设备数量
	ViFindList viDevList = 0;	// 设备列表标识符

	// 清除下拉列表的内容
	DeleteListItem(configPanelHandle, PANEL_CONF_RING_SRC_ADDR, 0, -1);
	switch (connType)
	{
		case RS232:
			// 搜索仪器资源
			viFindRsrc(defaultRM, "ASRL[0-9]*::?*INSTR", &viDevList, &viDevNum, viDevName);
			break;

		case GPIB:
			// 搜索仪器资源
			viFindRsrc(defaultRM, "GPIB[0-9]*::?*INSTR", &viDevList, &viDevNum, viDevName);
			break;

		case USB:
			// 搜索仪器资源
			viFindRsrc(defaultRM, "USB?*INSTR", &viDevList, &viDevNum, viDevName);
			break;

		default:
			break;
	}

	// 把搜索到的所有资源添加到配置面板上的资源地址下拉列表供用户选择
	if (viDevNum > 0)
	{
		InsertListItem(configPanelHandle, PANEL_CONF_RING_SRC_ADDR, -1, viDevName, viDevName);
	}

	for (size_t i = 1; i < viDevNum; i++)
	{
		viFindNext(viDevList, viDevName);
		InsertListItem(configPanelHandle, PANEL_CONF_RING_SRC_ADDR, -1, viDevName, viDevName);
	}
	return 0;
}

// 加载连接失败面板
void LoadConnectFaildPanel()
{
	if (connectFaildPanelHandleFlag == 0)
	{
		connectFaildPanelHandleFlag = 1;
		connectFaildPanelHandle = LoadPanel(panelHandle, "NS-Multimeter.uir", PANEL_CERR);
	}
	// 设置面板位置
	SetPanelPos(connectFaildPanelHandle, 205, 422);
	// 显示连接失败面板
	DisplayPanel(connectFaildPanelHandle);
}

//==============================================================================
// Global functions
/* 连接类型下拉列表 */
int CVICALLBACK RingConnectTypeCB(int panel, int control, int event,
								  void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// 0、连接方式：RS-232 			资源地址、波特率
			// 1、连接方式：GPIB 				资源地址
			// 2、连接方式：LAN			 	IP地址、端口
			// 3、连接方式：USB			 	资源地址

			// 获取当前连接类型
			GetCtrlVal(panel, control, &connectType);

			switch (connectType)
			{
				case RS232: // 连接方式为RS-232
					// 把扫描到的所有资源添加到 资源地址下拉列表中
					InsertSrcAddrToRing(RS232);

					// 更新 资源地址 textmsg
					SetCtrlVal(panel, PANEL_CONF_TEXTMSG_SRC_ADDR, "资源地址");
					// 设置 资源地址下拉列表 可见
					SetCtrlAttribute(panel, PANEL_CONF_RING_SRC_ADDR, ATTR_VISIBLE, 1);

					// 更新 波特率 textmsg
					SetCtrlVal(panel, PANEL_CONF_TEXTMSG_BAUDRATE, "波特率");
					// 设置 波特率下拉列表 可见
					SetCtrlAttribute(panel, PANEL_CONF_RING_BAUDRATE, ATTR_VISIBLE, 1);
					// 设置 波特率下面的图片 可见
					SetCtrlAttribute(panel, PANEL_CONF_PICTURE_113_BAUDRATE, ATTR_VISIBLE, 1);

					// 设置 IP string 不可见
					SetCtrlAttribute(panel, PANEL_CONF_STRING_IP, ATTR_VISIBLE, 0);
					break;

				case GPIB: // 连接方式为GPIB
					// 把扫描到的所有资源添加到 资源地址下拉列表中
					InsertSrcAddrToRing(GPIB);

					// 更新 资源地址 textmsg
					SetCtrlVal(panel, PANEL_CONF_TEXTMSG_SRC_ADDR, "资源地址");
					// 设置 资源地址下拉列表 可见
					SetCtrlAttribute(panel, PANEL_CONF_RING_SRC_ADDR, ATTR_VISIBLE, 1);

					// 隐藏 端口号 标签
					SetCtrlVal(panel, PANEL_CONF_TEXTMSG_BAUDRATE, "");
					// 设置 波特率下拉列表 不可见
					SetCtrlAttribute(panel, PANEL_CONF_RING_BAUDRATE, ATTR_VISIBLE, 0);
					// 设置 波特率下面的图片不可见
					SetCtrlAttribute(panel, PANEL_CONF_PICTURE_113_BAUDRATE, ATTR_VISIBLE, 0);
					break;

				case LAN: // 连接方式为LAN
					// 更新 IP地址 textmsg
					SetCtrlVal(panel, PANEL_CONF_TEXTMSG_SRC_ADDR, "IP地址");
					// 设置 IP地址 string 可见
					SetCtrlAttribute(panel, PANEL_CONF_STRING_IP, ATTR_VISIBLE, 1);

					// 更新 端口号 textmsg
					SetCtrlVal(panel, PANEL_CONF_TEXTMSG_BAUDRATE, "");

					// 设置 波特率下面的图片 不可见
					SetCtrlAttribute(panel, PANEL_CONF_PICTURE_113_BAUDRATE, ATTR_VISIBLE, 0);
					// 设置 波特率下拉列表 不可见
					SetCtrlAttribute(panel, PANEL_CONF_RING_BAUDRATE, ATTR_VISIBLE, 0);

					// 设置 资源地址下拉列表 不可见
					SetCtrlAttribute(panel, PANEL_CONF_RING_SRC_ADDR, ATTR_VISIBLE, 0);
					break;

				case USB: // 连接方式为USB
					// 把扫描到的所有资源添加到 资源地址下拉列表中
					InsertSrcAddrToRing(USB);

					// 更新 资源地址 textmsg
					SetCtrlVal(panel, PANEL_CONF_TEXTMSG_SRC_ADDR, "资源地址");
					// 设置 资源地址下拉列表 可见
					SetCtrlAttribute(panel, PANEL_CONF_RING_SRC_ADDR, ATTR_VISIBLE, 1);

					// 设置 IP地址 string 不可见
					SetCtrlAttribute(panel, PANEL_CONF_STRING_IP, ATTR_VISIBLE, 0);

					// 隐藏 端口号 textmsg
					SetCtrlVal(panel, PANEL_CONF_TEXTMSG_BAUDRATE, "");
					// 设置 波特率下面的图片 不可见
					SetCtrlAttribute(panel, PANEL_CONF_PICTURE_113_BAUDRATE, ATTR_VISIBLE, 0);
					// 设置 波特率下拉列表 不可见
					SetCtrlAttribute(panel, PANEL_CONF_RING_BAUDRATE, ATTR_VISIBLE, 0);
					break;

				default:
					break;
			}

			break;
	}
	return 0;
}

/* 资源地址下拉列表 */
int CVICALLBACK RingSrcAddrCB(int panel, int control, int event,
							  void *callbackData, int eventData1, int eventData2)
{
	int num = 0; // 下拉列中项目数量

	switch (event)
	{
		case EVENT_COMMIT:
			// 如果量程下拉列表中没有项目，直接退出
			GetNumListItems(configPanelHandle, PANEL_CONF_RING_SRC_ADDR, &num);
			if (num <= 0)
			{
				return 0;
			}

			// 获取仪器资源地址
			GetCtrlVal(configPanelHandle, PANEL_CONF_RING_SRC_ADDR, viDevName);
			break;

		default:
			break;
	}
	return 0;
}

/* 波特率下拉列表 */
int CVICALLBACK RingBaudRateCB(int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// 获取用户选择的波特率
			GetCtrlVal(configPanelHandle, PANEL_CONF_RING_BAUDRATE, &baudRate);

			// 设置串口波特率
			if (connectType == RS232)
			{
				viSetAttribute(viDev, VI_ATTR_ASRL_BAUD, baudRate);
			}
			break;
	}
	return 0;
}

/* 连接仪器的图片按钮 */
int CVICALLBACK PBtnConnectCB(int panel, int control, int event,
							  void *callbackData, int eventData1, int eventData2)
{
	int ret = 0;
	char ip[32] = {0};
	char tmpPath[1024] = {0}; 	// 临时路径
	InstrConfig config = {0}; 	// 上次仪器配置
	char mader[1024] = {0};	  	// 厂家
	char type[1024] = {0};		// 型号
	char code[1024] = {0};		// 序列号
	int len = 0; 				// mader_type长度
	char buf[128] = {0};
	int lineCount = 0;
	char lineBuf[1024] = {0};

	switch (event)
	{
		case EVENT_COMMIT:
			// 清空仪器相关信息
			memset(mader_type, 0, sizeof(char) * 1024);

			// 检查仪器连接状态
			if (connectState == 0) // 仪器未连接
			{
				// 判断连接类型
				if (connectType == LAN) // LAN
				{
					// 获取用户输入的IP
					GetCtrlVal(configPanelHandle, PANEL_CONF_STRING_IP, ip);

					sprintf(viDevName, "TCPIP0::%s::inst0::INSTR", ip);
					// 把资源地址加入到下拉列表
					InsertListItem(configPanelHandle, PANEL_CONF_RING_SRC_ADDR, -1, viDevName, viDevName);
				}

				// 获取默认（下拉列表第一个）仪器资源地址
				GetCtrlVal(configPanelHandle, PANEL_CONF_RING_SRC_ADDR, viDevName);

				// 连接设备
				ret = viOpen(defaultRM, viDevName, VI_NULL, VI_NULL, &viDev);
				if (ret != 0) // 连接失败
				{
					viDev = 0;

					// 更新文字说明
					SetCtrlVal(panel, PANEL_CONF_TEXTMSG_CONN_STATE, "连接失败");
					// 计算红色图片的路径，更新红色图片
					strcpy(tmpPath, projectDir);
					strcat(tmpPath, "\\Icon\\connectFaild.png");
					DisplayImageFile(panel, PANEL_CONF_PICTURE_CONNECT, tmpPath);

					// 加载连接失败面板
					LoadConnectFaildPanel();
					// 直接退出
					return -1;
				}

				char buffer[256] = {0};
				viWrite(viDev, (ViBuf)"*IDN?\n", strlen("*IDN?\n"), VI_NULL);
				viRead(viDev, (ViBuf)buffer, sizeof(buffer), VI_NULL);
				if (strcmp(buffer, "") == 0)
				{
					return -1;
				}
				// 如果是串口，则设置结束符
				if (connectType == RS232)
				{
					viSetAttribute(viDev, VI_ATTR_TERMCHAR_EN, VI_TRUE);
					viSetAttribute(viDev, VI_ATTR_TERMCHAR, buffer[strlen(buffer) - 1]);
				}

				// 获取仪器的生产厂商，仪器型号，仪器序列号
				GetMulSeries(viDev, mader, type, code);

				// 仪器连接失败 或 仪器出问题
				if (!strcmp(mader, "") || !strcmp(type, "") || !strcmp(code, ""))
				{
					// 关闭仪器的连接
					viClose(viDev);
					viDev = 0;

					// 更新文字说明
					SetCtrlVal(panel, PANEL_CONF_TEXTMSG_CONN_STATE, "连接失败");
					// 计算红色图片的路径，更新红色图片
					strcpy(tmpPath, projectDir);
					strcat(tmpPath, "\\Icon\\connectFaild.png");
					DisplayImageFile(panel, PANEL_CONF_PICTURE_CONNECT, tmpPath);

					// 弹窗警告
					MessagePopup("警告！", "仪器连接失败或仪器出问题，请断开连接稍后再试或者重启仪器！");
					return -1;
				}

				// 拼接仪器厂家和型号
				if (strcmp(mader, "Keithley") == 0 || strcmp(mader, "KEITHLEY") == 0)
				{
					// 拆分 MODEL 2000
					char* tar = strstr(type, " ");
					if (tar == NULL)
					{
						return -1;
					}
					mader[1] = 0;
					strcpy(mader_type, mader);
					strcat(mader_type, "_");
					strcat(mader_type, tar + 1);
				}
				else
				{
					mader[1] = 0;
					strcpy(mader_type, mader);
					strcat(mader_type, "_");
					strcat(mader_type, type);
				}
				// 如果有小写字母，就全转成大写
				len = strlen(mader_type);
				for (size_t i = 0; i < len; ++i)
				{
					if (mader_type[i] >= 'a' && mader_type[i] <= 'z')
					{
						mader_type[i] -= ('a' - 'A');
					}
				}

				// 更新仪器连接状态
				connectState = 1;

				// 如果是串口，则要设置波特率
				if (connectType == RS232)
				{
					// 设置波特率
					viSetAttribute(viDev, VI_ATTR_ASRL_BAUD, baudRate);
					// 清空输入队列
					viFlush(viDev, VI_ASRL_IN_BUF);
				}

				//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 仪器连接成功 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
				// 加载配置
				LoadConfigFromIniFile(configPanelHandle, &config);

				strcpy(dirPath, config.savePath);
				ResetTextBox(panel, PANEL_CONF_TEXTBOX_DATA_PATH, config.savePath); // 设置保存路径

				// 设置波特率
				int baudrateIndex = 0;
				baudRate = config.baudRate;
				GetIndexFromValue(panel, PANEL_CONF_RING_BAUDRATE, &baudrateIndex, baudRate);
				if (baudrateIndex < 0)
					baudrateIndex = 0;
				SetCtrlIndex(panel, PANEL_CONF_RING_BAUDRATE, baudrateIndex);
				CallCtrlCallback(panel, PANEL_CONF_RING_BAUDRATE, EVENT_COMMIT, 0, 0, 0);

				// 设置测试项目
				int testItemIndex = 0;
				testItem = config.testItem;
				GetIndexFromValue(panel, PANEL_CONF_RING_TEST_ITEM, &testItemIndex, testItem);
				if (testItemIndex < 0)
					testItemIndex = 0;
				SetCtrlIndex(panel, PANEL_CONF_RING_TEST_ITEM, testItemIndex);
				CallCtrlCallback(panel, PANEL_CONF_RING_TEST_ITEM, EVENT_COMMIT, 0, 0, 0);

				// 设置测试量程
				int rangeIndex = 0;
				range = config.range;
				GetIndexFromValue(panel, PANEL_CONF_RING_TEST_RANGE, &rangeIndex, range);
				if (rangeIndex < 0)
					rangeIndex = 0;
				SetCtrlIndex(panel, PANEL_CONF_RING_TEST_RANGE, rangeIndex);
				CallCtrlCallback(panel, PANEL_CONF_RING_TEST_RANGE, EVENT_COMMIT, 0, 0, 0);

				// 测试间隔
				samplingSpace = config.testSapce;
				sprintf(buf, "%.3lf", config.testSapce);
				SetCtrlVal(panel, PANEL_CONF_STRING_TIME_SPACE, buf); // 设置测试时间

				// 勾选了启用测试时长
				useFlag = config.testTimeUseFlag;
				if (config.testTimeUseFlag)
				{
					sprintf(buf, "%.3lf", config.testTime);
					SetCtrlVal(panel, PANEL_CONF_STRING_TEST_TIME, buf); // 设置测试时间
					SetCtrlAttribute(panel, PANEL_CONF_STRING_TEST_TIME, ATTR_DIMMED, 0); // 可用

					// 更换图片
					strcpy(tmpPath, projectDir);
					strcat(tmpPath, "\\Icon\\PTBtnSelect.png");
					SetCtrlAttribute(panel, PANEL_CONF_PTBTN_USE_TESTTIME, ATTR_IMAGE_FILE, tmpPath);
					// 设置值
					SetCtrlVal(panel, PANEL_CONF_PTBTN_USE_TESTTIME, 1);
				}

				// 勾选了启用自动存储
				autoSaveFlag = config.autoSaveFlag;
				if (config.autoSaveFlag)
				{
					strcpy(tmpPath, projectDir);
					strcat(tmpPath, "\\Icon\\PTBtnSelect.png");
					SetCtrlAttribute(panel, PANEL_CONF_PTBTN_AUTOSAVE, ATTR_IMAGE_FILE, tmpPath);
					SetCtrlVal(panel, PANEL_CONF_PTBTN_AUTOSAVE, 1);
				}
				//================================================= 加载配置完毕 =================================================//

				// 更新文字说明
				SetCtrlVal(panel, PANEL_CONF_TEXTMSG_CONN_STATE, "连接成功");
				// 计算绿色图片的路径，更新绿色图片
				strcpy(tmpPath, projectDir);
				strcat(tmpPath, "\\Icon\\connectOk.png");
				DisplayImageFile(panel, PANEL_CONF_PICTURE_CONNECT, tmpPath);

				// 更换连接按钮图片为断开连接
				strcpy(tmpPath, projectDir);
				strcat(tmpPath, "\\Icon\\connect_0.png");
				SetCtrlAttribute(panel, PANEL_CONF_PBTN_CONNECT, ATTR_IMAGE_FILE, tmpPath);
			}
			else // 仪器已连接
			{
				// 关闭仪器
				viClose(viDev);
				viDev = 0;

				// 更新仪器连接状态
				connectState = 0;

				//============================================== 保存配置数据 ==============================================//
				strcpy(config.mader_type, mader_type); // 厂家_型号

				config.connectType = connectType; // 连接类型
				strcpy(config.srcAddr, viDevName); // 资源地址

				GetCtrlVal(configPanelHandle, PANEL_CONF_RING_BAUDRATE, &baudRate);
				config.baudRate = baudRate; // 波特率

				GetCtrlVal(configPanelHandle, PANEL_CONF_RING_TEST_ITEM, &testItem);
				config.testItem = testItem; // 测试项目

				GetCtrlVal(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, &range);
				config.range = range; // 测试量程

				//GetCtrlVal(configPanelHandle, PANEL_CONF_RING_DATA_WIDTH, &digit);
				//config.digit = digit; //数据位数

				GetCtrlVal(configPanelHandle, PANEL_CONF_STRING_TIME_SPACE, buf);
				// 字符串转double
				samplingSpace = atof(buf);
				config.testSapce = samplingSpace; // 采样间隔

				GetCtrlVal(configPanelHandle, PANEL_CONF_STRING_TEST_TIME, buf);
				// 字符串转double
				testTime = atof(buf);
				config.testTime = testTime; // 测试时长

				GetCtrlVal(configPanelHandle, PANEL_CONF_PTBTN_USE_TESTTIME, &(config.testTimeUseFlag)); // 判断用户是否勾选启用
				GetCtrlVal(configPanelHandle, PANEL_CONF_PTBTN_AUTOSAVE, &(config.autoSaveFlag)); // 自动存储标志置

				// 获取文本行数
				GetNumTextBoxLines(configPanelHandle, PANEL_CONF_TEXTBOX_DATA_PATH, &lineCount);
				// 拼接路径
				memset(dirPath, 0, sizeof(char) * 1024);
				for (size_t i = 0; i < lineCount; ++i)
				{
					GetTextBoxLine(configPanelHandle, PANEL_CONF_TEXTBOX_DATA_PATH, i, lineBuf);
					strcat(dirPath, lineBuf);
				}
				strcpy(config.savePath, dirPath); // 测试结果文件保存路径
				SaveConfigToIniFile(&config);

				// 更新文字说明
				SetCtrlVal(panel, PANEL_CONF_TEXTMSG_CONN_STATE, "未连接");
				// 计算灰色默认图片的路径，并更新图片
				strcpy(tmpPath, projectDir);
				strcat(tmpPath, "\\Icon\\connectDefault.png");
				DisplayImageFile(panel, PANEL_CONF_PICTURE_CONNECT, tmpPath);

				// 更换断开连接图片为连接
				strcpy(tmpPath, projectDir);
				strcat(tmpPath, "\\Icon\\connect_1.png");
				SetCtrlAttribute(panel, PANEL_CONF_PBTN_CONNECT, ATTR_IMAGE_FILE, tmpPath);
			}
			break;
	}
	return 0;
}

/* 仪器连接失败面板上的我知道了按钮 */
int CVICALLBACK PBtnIKnowCB(int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// 注销连接失败面板
			DiscardPanel(connectFaildPanelHandle);
			connectFaildPanelHandle = 0;
			connectFaildPanelHandleFlag = 0;
			break;
	}
	return 0;
}

/* 测试项目下拉列表 */
int CVICALLBACK RingTestItemCB(int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	char ***data = NULL;  // 查询到的数据
	int num = -1;		  // 数据行数
	char sql[1024] = {0}; // SQL语句

	switch (event)
	{
		case EVENT_COMMIT:
			// 清空量程下拉列表
			DeleteListItem(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, 0, -1);
			
			// 获取当前测试项目
			GetCtrlVal(configPanelHandle, PANEL_CONF_RING_TEST_ITEM, &testItem);
			switch (testItem)
			{
				case MM_DCI: // 直流电流
					// 更新波形y轴标签
					SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_GRAPH, ATTR_YNAME, "电流(A)");

					/*-------------------- 在数据库中查找直流电流量程 --------------------*/
					// 查询直流电流的SQL语句
					sprintf(sql, "SELECT label, val FROM dca_t WHERE type = '%s'", mader_type);

					// 查询
					SqliteQuery(sql, &num, &data, 1024);

					// 把每个量程都添加到下拉列表
					for (size_t i = 0; i < num; ++i)
					{
						InsertListItem(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, -1, data[i][0], atof(data[i][1]));
					}

					// 给仪器发送指令
					MulFunctionSet(viDev, MM_DCI);
					break;

				case MM_ACI: // 交流电流
					// 更新波形y轴标签
					SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_GRAPH, ATTR_YNAME, "电流(A)");

					/*-------------------- 在数据库中查找交流电流量程 --------------------*/
					// 查询直流电流的SQL语句
					sprintf(sql, "SELECT label, val FROM aca_t WHERE type = '%s'", mader_type);

					// 查询
					SqliteQuery(sql, &num, &data, 1024);

					// 把每个量程都添加到下拉列表
					for (size_t i = 0; i < num; ++i)
					{
						InsertListItem(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, -1, data[i][0], atof(data[i][1]));
					}

					// 给仪器发送指令
					MulFunctionSet(viDev, MM_ACI);
					break;

				case MM_DCV: // 直流电压
					// 更新波形y轴标签
					SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_GRAPH, ATTR_YNAME, "电压(V)");

					/*-------------------- 在数据库中查找直流电压量程 --------------------*/
					// 查询直流电流的SQL语句
					sprintf(sql, "SELECT label, val FROM dcv_t WHERE type = '%s'", mader_type);

					// 查询
					SqliteQuery(sql, &num, &data, 1024);

					// 把每个量程都添加到下拉列表
					for (size_t i = 0; i < num; ++i)
					{
						InsertListItem(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, -1, data[i][0], atof(data[i][1]));
					}
					
					// 给仪器发送指令
					MulFunctionSet(viDev, MM_DCV);
					break;

				case MM_ACV: // 交流电压
					// 更新波形y轴标签
					SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_GRAPH, ATTR_YNAME, "电压(V)");

					/*-------------------- 在数据库中查找交流电压量程 --------------------*/
					// 查询直流电流的SQL语句
					sprintf(sql, "SELECT label, val FROM acv_t WHERE type = '%s'", mader_type);

					// 查询
					SqliteQuery(sql, &num, &data, 1024);

					// 把每个量程都添加到下拉列表
					for (size_t i = 0; i < num; ++i)
					{
						InsertListItem(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, -1, data[i][0], atof(data[i][1]));
					}
					
					// 给仪器发送指令
					MulFunctionSet(viDev, MM_ACV);
					break;

				case MM_RES: // 二线电阻
					// 更新波形y轴标签
					SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_GRAPH, ATTR_YNAME, "电阻(Ω)");

					/*-------------------- 在数据库中查找二线电阻量程 --------------------*/
					// 查询二线电阻的SQL语句
					sprintf(sql, "SELECT label, val FROM r2_t WHERE type = '%s'", mader_type);

					// 查询
					SqliteQuery(sql, &num, &data, 1024);

					// 把每个量程都添加到下拉列表
					for (size_t i = 0; i < num; ++i)
					{
						InsertListItem(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, -1, data[i][0], atof(data[i][1]));
					}

					// 给仪器发送指令
					MulFunctionSet(viDev, MM_RES);
					break;

				case MM_FRES: // 四线电阻
					// 更新波形y轴标签
					SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_GRAPH, ATTR_YNAME, "电阻(Ω)");

					// 更新量程下拉列表
					/*-------------------- 在数据库中查找四线电阻量程 --------------------*/
					// 查询直流电路的SQL语句
					sprintf(sql, "SELECT label, val FROM r4_t WHERE type = '%s'", mader_type);

					// 查询
					SqliteQuery(sql, &num, &data, 1024);

					// 把每个量程都添加到下拉列表
					for (size_t i = 0; i < num; ++i)
					{
						InsertListItem(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, -1, data[i][0], atof(data[i][1]));
					}

					// 给仪器发送指令
					MulFunctionSet(viDev, MM_FRES);
					break;

				default:
					break;
			}
			// 清空数据库查询语句开辟的内存
			FreeSqlData();
			data = NULL;

			// 设置y轴
			SetYRange();
			break;
	}
	return 0;
}

/* 量程下拉列表 */
int CVICALLBACK RingTestRangeCB(int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{

	int num; // 下拉列表中的项目数量
	
	switch (event)
	{
		case EVENT_COMMIT:
			// 如果量程下拉列表中没有项目，直接退出
			GetNumListItems(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, &num);
			if (num <= 0)
			{
				return 0;
			}

			// 获取量程
			GetCtrlVal(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, &range);

			// 获取当前测试项目
			GetCtrlVal(configPanelHandle, PANEL_CONF_RING_TEST_ITEM, &testItem);
			switch (testItem)
			{
				case MM_DCI:		   // 直流电流
					// 给仪器发送指令
					MulRangeSet(viDev, MM_DCI, range);
					break;

				case MM_ACI:		   // 交流电流
					// 给仪器发送指令
					MulRangeSet(viDev, MM_ACI, range);
					break;

				case MM_DCV:		   // 直流电压
					// 给仪器发送指令
					MulRangeSet(viDev, MM_DCV, range);
					break;

				case MM_ACV:		   // 交流电压
					// 给仪器发送指令
					MulRangeSet(viDev, MM_ACV, range);
					break;

				case MM_RES:		   // 二线电阻
					// 给仪器发送指令
					MulRangeSet(viDev, MM_RES, range);
					break;

				case MM_FRES:		   // 四线电阻
					// 给仪器发送指令
					MulRangeSet(viDev, MM_FRES, range);
					break;

				default:
					break;
			}
			
			// 设置Y轴量程
			SetYRange();
			break;
	}
	return 0;
}

/* 位数（分辨率/测量速率）下拉列表 */
int CVICALLBACK RingDataWidthCB(int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	char cmd[1024] = {0};
	char rateLabel[32] = {0};
	char rateVal[32] = {0};

	switch (event)
	{
		case EVENT_COMMIT:
			// 获取当前速率
			GetCtrlVal(configPanelHandle, PANEL_CONF_RING_DATA_WIDTH, rateLabel);
			
			if (!strcmp(rateLabel, "fast")) // 快
			{
				strcpy(rateVal, "MIN");
			}
			else if (!strcmp(rateLabel, "med")) // 中
			{
				strcpy(rateVal, "DEF");
			}
			else // 慢
			{
				strcpy(rateVal, "MAX");
			}
			
			switch (testItem)
			{
				case MM_DCI:
					if (!strcmp(mader_type, "R_DM3054") || !strcmp(mader_type, "R_DM3058")) // 普源DM3000系列
					{
						sprintf(cmd, ":RESO:CURR:DC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "K_2000") || !strcmp(mader_type, "K_2002") || !strcmp(mader_type, "K_2010") || 
							 !strcmp(mader_type, "K_2100") || !strcmp(mader_type, "K_2700") || !strcmp(mader_type, "K_2750")) // 吉时利2000系列
					{
						sprintf(cmd, "CURR:NPLC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "K_6500") || !strcmp(mader_type, "K_7510")) // 吉时利6500 7510
					{
						sprintf(cmd, "CURR:NPLC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "A_U3606A")) // 安捷伦U3606A
					{
						sprintf(cmd, "CURR:DC:RES %s\n", rateVal);
					}
					else
					{
						//==============================================
					}
					break;

				case MM_DCV:
					if (!strcmp(mader_type, "R_DM3054") || !strcmp(mader_type, "R_DM3058")) // 普源
					{
						sprintf(cmd, ":RESO:VOLT:DC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "K_2000") || !strcmp(mader_type, "K_2002") || !strcmp(mader_type, "K_2010") || 
							 !strcmp(mader_type, "K_2100") || !strcmp(mader_type, "K_2700") || !strcmp(mader_type, "K_2750")) // 吉时利2000系列
					{
						sprintf(cmd, "VOLT:NPLC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "K_6500") || !strcmp(mader_type, "K_7510")) // 吉时利6500 7510
					{
						sprintf(cmd, "VOLT:NPLC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "A_U3606A")) // 安捷伦U3606A
					{
						sprintf(cmd, "VOLT:DC:RES %s\n", rateVal);
					}
					else
					{
						//==============================================
					}
					break;

				case MM_ACI:
					if (!strcmp(mader_type, "R_DM3054") || !strcmp(mader_type, "R_DM3058")) // 普源
					{
						sprintf(cmd, ":RESO:CURR:AC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "K_2000") || !strcmp(mader_type, "K_2002") || !strcmp(mader_type, "K_2010") || 
							 !strcmp(mader_type, "K_2100") || !strcmp(mader_type, "K_2700") || !strcmp(mader_type, "K_2750")) // 吉时利
					{
						sprintf(cmd, "CURR:AC:NPLC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "K_6500") || !strcmp(mader_type, "K_7510")) // 吉时利
					{
						sprintf(cmd, "CURR:AC:NPLC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "A_U3606A")) // 安捷伦U3606A
					{
						sprintf(cmd, "CURR:AC:RES %s\n", rateVal);
					}
					else
					{
						//==============================================
					}
					break;

				case MM_ACV:
					if (!strcmp(mader_type, "R_DM3054") || !strcmp(mader_type, "R_DM3058")) // 普源
					{
						sprintf(cmd, ":RESO:VOLT:AC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "K_2000") || !strcmp(mader_type, "K_2002") || !strcmp(mader_type, "K_2010") || 
							 !strcmp(mader_type, "K_2100") || !strcmp(mader_type, "K_2700") || !strcmp(mader_type, "K_2750")) // 吉时利2000系列
					{
						sprintf(cmd, "VOLT:AC:NPLC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "K_6500") || !strcmp(mader_type, "K_7510")) // 吉时利6500
					{
						sprintf(cmd, "VOLT:AC:NPLC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "A_U3606A")) // 安捷伦U3606A
					{
						sprintf(cmd, "VOLT:AC:RES %s\n", rateVal);
					}
					else
					{
						//==============================================
					}
					break;

				case MM_RES:
					if (!strcmp(mader_type, "R_DM3054") || !strcmp(mader_type, "R_DM3058")) // 普源
					{
						sprintf(cmd, ":RESO:RES %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "K_2000") || !strcmp(mader_type, "K_2002") || !strcmp(mader_type, "K_2010") || 
							 !strcmp(mader_type, "K_2100") || !strcmp(mader_type, "K_2700") || !strcmp(mader_type, "K_2750")) // 吉时利2000系列
					{
						sprintf(cmd, "RES:NPLC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "K_6500") || !strcmp(mader_type, "K_7510")) // 吉时利6500
					{
						sprintf(cmd, "RES:NPLC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "A_U3606A")) // 安捷伦U3606A
					{
						sprintf(cmd, "RES:RES %s\n", rateVal);
					}
					else
					{
						//==============================================
					}
					break;

				case MM_FRES:
					if (!strcmp(mader_type, "R_DM3054") || !strcmp(mader_type, "R_DM3058")) // 普源
					{
						sprintf(cmd, ":RESO:FRES %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "K_2000") || !strcmp(mader_type, "K_2002") || !strcmp(mader_type, "K_2010") || 
							 !strcmp(mader_type, "K_2100") || !strcmp(mader_type, "K_2700") || !strcmp(mader_type, "K_2750")) // 吉时利2000系列
					{
						sprintf(cmd, "FRES:NPLC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "K_6500") || !strcmp(mader_type, "K_7510")) // 吉时利6500
					{
						sprintf(cmd, "FRES:NPLC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "A_U3606A")) // 安捷伦U3606A
					{
						sprintf(cmd, "LRES:RES %s\n", rateVal);
					}
					else
					{
						//==============================================
					}
					break;

				default:
					break;
			}
			// 写指令
			viWrite(viDev, (ViBuf)cmd, strlen(cmd), VI_NULL);
			break;
	}
	return 0;
}

/* 数据存储路径按钮 */
int CVICALLBACK PBtnDataSavePathCB(int panel, int control, int event,
								   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// 打开 文件资源管理器
			if (DirSelectPopup(projectDir, "选择文件夹", 1, 1, dirPath) == 0) // 没有选择文件夹
			{
				return 0;
			}

			// 路径前 根盘符字母大写
			if (dirPath[0] >= 'a' && dirPath[0] <= 'z')
			{
				dirPath[0] -= ('a' - 'A');
			}

			// 显示用户选择的路径
			ResetTextBox(configPanelHandle, PANEL_CONF_TEXTBOX_DATA_PATH, dirPath);
			break;
	}
	return 0;
}

// 启用测测试时长按钮
int CVICALLBACK PTBtnUseTestTimeCB (int panel, int control, int event,
									void *callbackData, int eventData1, int eventData2)
{
	char tmpPath[1024] = {0};

	switch (event)
	{
		case EVENT_COMMIT:
			// 判断用户是否选启用
			GetCtrlVal(panel, PANEL_CONF_PTBTN_USE_TESTTIME, &useFlag);

			if (useFlag == 0) // 没选
			{
				// 测试时长文本框不可用
				SetCtrlAttribute(configPanelHandle, PANEL_CONF_STRING_TEST_TIME, ATTR_DIMMED, 1);

				// 更换图片
				strcpy(tmpPath, projectDir);
				strcat(tmpPath, "\\Icon\\PTBtnUnSelect.png");
				SetCtrlAttribute(panel, PANEL_CONF_PTBTN_USE_TESTTIME, ATTR_IMAGE_FILE, tmpPath);
			}
			else // 选
			{
				// 测试时长文本框可用
				SetCtrlAttribute(configPanelHandle, PANEL_CONF_STRING_TEST_TIME, ATTR_DIMMED, 0);

				// 更换图片
				strcpy(tmpPath, projectDir);
				strcat(tmpPath, "\\Icon\\PTBtnSelect.png");
				SetCtrlAttribute(panel, PANEL_CONF_PTBTN_USE_TESTTIME, ATTR_IMAGE_FILE, tmpPath);
			}
			break;
	}
	return 0;
}

// 启用自动存储按钮
int CVICALLBACK PTBtnAutoSaveCB (int panel, int control, int event,
								 void *callbackData, int eventData1, int eventData2)
{
	char tmpPath[1024] = {0};

	switch (event)
	{
		case EVENT_COMMIT:
			// 自动存储标志置
			GetCtrlVal(panel, control, &autoSaveFlag);

			// 更换图片
			if (autoSaveFlag)
			{
				strcpy(tmpPath, projectDir);
				strcat(tmpPath, "\\Icon\\PTBtnSelect.png");
				SetCtrlAttribute(panel, PANEL_CONF_PTBTN_AUTOSAVE, ATTR_IMAGE_FILE, tmpPath);
			}
			else
			{
				strcpy(tmpPath, projectDir);
				strcat(tmpPath, "\\Icon\\PTBtnUnSelect.png");
				SetCtrlAttribute(panel, PANEL_CONF_PTBTN_AUTOSAVE, ATTR_IMAGE_FILE, tmpPath);
			}
			break;
	}
	return 0;
}

