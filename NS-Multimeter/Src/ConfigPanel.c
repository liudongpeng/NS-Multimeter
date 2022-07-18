//==============================================================================
//
// Title:		ConfigPanel.c
// Purpose:		���������Ӽ�����
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
/* �������ӳɹ���־��0δ���ӣ� 1�����ӣ� */
volatile int connectState;

//==============================================================================
// Static functions
/* ����y������ */
static void SetYRange()
{
	double maxRange, maxRange2;
	int count;
	double *rangeArr = NULL;
	double range; // ����

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
		for (size_t i = 0; i < count; ++i)
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

/* ��������Դ���ص�ring�ؼ����û�ѡ���� */
static int InsertSrcAddrToRing(int connType)
{
	ViUInt32 viDevNum = 0; 	// ƥ�䵽���豸����
	ViFindList viDevList = 0;	// �豸�б���ʶ��

	// ��������б�������
	DeleteListItem(configPanelHandle, PANEL_CONF_RING_SRC_ADDR, 0, -1);
	switch (connType)
	{
		case RS232:
			// ����������Դ
			viFindRsrc(defaultRM, "ASRL[0-9]*::?*INSTR", &viDevList, &viDevNum, viDevName);
			break;

		case GPIB:
			// ����������Դ
			viFindRsrc(defaultRM, "GPIB[0-9]*::?*INSTR", &viDevList, &viDevNum, viDevName);
			break;

		case USB:
			// ����������Դ
			viFindRsrc(defaultRM, "USB?*INSTR", &viDevList, &viDevNum, viDevName);
			break;

		default:
			break;
	}

	// ����������������Դ���ӵ���������ϵ���Դ��ַ�����б����û�ѡ��
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

// ��������ʧ�����
void LoadConnectFaildPanel()
{
	if (connectFaildPanelHandleFlag == 0)
	{
		connectFaildPanelHandleFlag = 1;
		connectFaildPanelHandle = LoadPanel(panelHandle, "NS-Multimeter.uir", PANEL_CERR);
	}
	// �������λ��
	SetPanelPos(connectFaildPanelHandle, 205, 422);
	// ��ʾ����ʧ�����
	DisplayPanel(connectFaildPanelHandle);
}

//==============================================================================
// Global functions
/* �������������б� */
int CVICALLBACK RingConnectTypeCB(int panel, int control, int event,
								  void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// 0�����ӷ�ʽ��RS-232 			��Դ��ַ��������
			// 1�����ӷ�ʽ��GPIB 				��Դ��ַ
			// 2�����ӷ�ʽ��LAN			 	IP��ַ���˿�
			// 3�����ӷ�ʽ��USB			 	��Դ��ַ

			// ��ȡ��ǰ��������
			GetCtrlVal(panel, control, &connectType);

			switch (connectType)
			{
				case RS232: // ���ӷ�ʽΪRS-232
					// ��ɨ�赽��������Դ���ӵ� ��Դ��ַ�����б���
					InsertSrcAddrToRing(RS232);

					// ���� ��Դ��ַ textmsg
					SetCtrlVal(panel, PANEL_CONF_TEXTMSG_SRC_ADDR, "��Դ��ַ");
					// ���� ��Դ��ַ�����б� �ɼ�
					SetCtrlAttribute(panel, PANEL_CONF_RING_SRC_ADDR, ATTR_VISIBLE, 1);

					// ���� ������ textmsg
					SetCtrlVal(panel, PANEL_CONF_TEXTMSG_BAUDRATE, "������");
					// ���� �����������б� �ɼ�
					SetCtrlAttribute(panel, PANEL_CONF_RING_BAUDRATE, ATTR_VISIBLE, 1);
					// ���� �����������ͼƬ �ɼ�
					SetCtrlAttribute(panel, PANEL_CONF_PICTURE_113_BAUDRATE, ATTR_VISIBLE, 1);

					// ���� IP string ���ɼ�
					SetCtrlAttribute(panel, PANEL_CONF_STRING_IP, ATTR_VISIBLE, 0);
					break;

				case GPIB: // ���ӷ�ʽΪGPIB
					// ��ɨ�赽��������Դ���ӵ� ��Դ��ַ�����б���
					InsertSrcAddrToRing(GPIB);

					// ���� ��Դ��ַ textmsg
					SetCtrlVal(panel, PANEL_CONF_TEXTMSG_SRC_ADDR, "��Դ��ַ");
					// ���� ��Դ��ַ�����б� �ɼ�
					SetCtrlAttribute(panel, PANEL_CONF_RING_SRC_ADDR, ATTR_VISIBLE, 1);

					// ���� �˿ں� ��ǩ
					SetCtrlVal(panel, PANEL_CONF_TEXTMSG_BAUDRATE, "");
					// ���� �����������б� ���ɼ�
					SetCtrlAttribute(panel, PANEL_CONF_RING_BAUDRATE, ATTR_VISIBLE, 0);
					// ���� �����������ͼƬ���ɼ�
					SetCtrlAttribute(panel, PANEL_CONF_PICTURE_113_BAUDRATE, ATTR_VISIBLE, 0);
					break;

				case LAN: // ���ӷ�ʽΪLAN
					// ���� IP��ַ textmsg
					SetCtrlVal(panel, PANEL_CONF_TEXTMSG_SRC_ADDR, "IP��ַ");
					// ���� IP��ַ string �ɼ�
					SetCtrlAttribute(panel, PANEL_CONF_STRING_IP, ATTR_VISIBLE, 1);

					// ���� �˿ں� textmsg
					SetCtrlVal(panel, PANEL_CONF_TEXTMSG_BAUDRATE, "");

					// ���� �����������ͼƬ ���ɼ�
					SetCtrlAttribute(panel, PANEL_CONF_PICTURE_113_BAUDRATE, ATTR_VISIBLE, 0);
					// ���� �����������б� ���ɼ�
					SetCtrlAttribute(panel, PANEL_CONF_RING_BAUDRATE, ATTR_VISIBLE, 0);

					// ���� ��Դ��ַ�����б� ���ɼ�
					SetCtrlAttribute(panel, PANEL_CONF_RING_SRC_ADDR, ATTR_VISIBLE, 0);
					break;

				case USB: // ���ӷ�ʽΪUSB
					// ��ɨ�赽��������Դ���ӵ� ��Դ��ַ�����б���
					InsertSrcAddrToRing(USB);

					// ���� ��Դ��ַ textmsg
					SetCtrlVal(panel, PANEL_CONF_TEXTMSG_SRC_ADDR, "��Դ��ַ");
					// ���� ��Դ��ַ�����б� �ɼ�
					SetCtrlAttribute(panel, PANEL_CONF_RING_SRC_ADDR, ATTR_VISIBLE, 1);

					// ���� IP��ַ string ���ɼ�
					SetCtrlAttribute(panel, PANEL_CONF_STRING_IP, ATTR_VISIBLE, 0);

					// ���� �˿ں� textmsg
					SetCtrlVal(panel, PANEL_CONF_TEXTMSG_BAUDRATE, "");
					// ���� �����������ͼƬ ���ɼ�
					SetCtrlAttribute(panel, PANEL_CONF_PICTURE_113_BAUDRATE, ATTR_VISIBLE, 0);
					// ���� �����������б� ���ɼ�
					SetCtrlAttribute(panel, PANEL_CONF_RING_BAUDRATE, ATTR_VISIBLE, 0);
					break;

				default:
					break;
			}

			break;
	}
	return 0;
}

/* ��Դ��ַ�����б� */
int CVICALLBACK RingSrcAddrCB(int panel, int control, int event,
							  void *callbackData, int eventData1, int eventData2)
{
	int num = 0; // ����������Ŀ����

	switch (event)
	{
		case EVENT_COMMIT:
			// ������������б���û����Ŀ��ֱ���˳�
			GetNumListItems(configPanelHandle, PANEL_CONF_RING_SRC_ADDR, &num);
			if (num <= 0)
			{
				return 0;
			}

			// ��ȡ������Դ��ַ
			GetCtrlVal(configPanelHandle, PANEL_CONF_RING_SRC_ADDR, viDevName);
			break;

		default:
			break;
	}
	return 0;
}

/* �����������б� */
int CVICALLBACK RingBaudRateCB(int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// ��ȡ�û�ѡ��Ĳ�����
			GetCtrlVal(configPanelHandle, PANEL_CONF_RING_BAUDRATE, &baudRate);

			// ���ô��ڲ�����
			if (connectType == RS232)
			{
				viSetAttribute(viDev, VI_ATTR_ASRL_BAUD, baudRate);
			}
			break;
	}
	return 0;
}

/* ����������ͼƬ��ť */
int CVICALLBACK PBtnConnectCB(int panel, int control, int event,
							  void *callbackData, int eventData1, int eventData2)
{
	int ret = 0;
	char ip[32] = {0};
	char tmpPath[1024] = {0}; 	// ��ʱ·��
	InstrConfig config = {0}; 	// �ϴ���������
	char mader[1024] = {0};	  	// ����
	char type[1024] = {0};		// �ͺ�
	char code[1024] = {0};		// ���к�
	int len = 0; 				// mader_type����
	char buf[128] = {0};
	int lineCount = 0;
	char lineBuf[1024] = {0};

	switch (event)
	{
		case EVENT_COMMIT:
			// ������������Ϣ
			memset(mader_type, 0, sizeof(char) * 1024);

			// �����������״̬
			if (connectState == 0) // ����δ����
			{
				// �ж���������
				if (connectType == LAN) // LAN
				{
					// ��ȡ�û������IP
					GetCtrlVal(configPanelHandle, PANEL_CONF_STRING_IP, ip);

					sprintf(viDevName, "TCPIP0::%s::inst0::INSTR", ip);
					// ����Դ��ַ���뵽�����б�
					InsertListItem(configPanelHandle, PANEL_CONF_RING_SRC_ADDR, -1, viDevName, viDevName);
				}

				// ��ȡĬ�ϣ������б���һ����������Դ��ַ
				GetCtrlVal(configPanelHandle, PANEL_CONF_RING_SRC_ADDR, viDevName);

				// �����豸
				ret = viOpen(defaultRM, viDevName, VI_NULL, VI_NULL, &viDev);
				if (ret != 0) // ����ʧ��
				{
					viDev = 0;

					// ��������˵��
					SetCtrlVal(panel, PANEL_CONF_TEXTMSG_CONN_STATE, "����ʧ��");
					// �����ɫͼƬ��·�������º�ɫͼƬ
					strcpy(tmpPath, projectDir);
					strcat(tmpPath, "\\Icon\\connectFaild.png");
					DisplayImageFile(panel, PANEL_CONF_PICTURE_CONNECT, tmpPath);

					// ��������ʧ�����
					LoadConnectFaildPanel();
					// ֱ���˳�
					return -1;
				}

				char buffer[256] = {0};
				viWrite(viDev, (ViBuf)"*IDN?\n", strlen("*IDN?\n"), VI_NULL);
				viRead(viDev, (ViBuf)buffer, sizeof(buffer), VI_NULL);
				if (strcmp(buffer, "") == 0)
				{
					return -1;
				}
				// ����Ǵ��ڣ������ý�����
				if (connectType == RS232)
				{
					viSetAttribute(viDev, VI_ATTR_TERMCHAR_EN, VI_TRUE);
					viSetAttribute(viDev, VI_ATTR_TERMCHAR, buffer[strlen(buffer) - 1]);
				}

				// ��ȡ�������������̣������ͺţ��������к�
				GetMulSeries(viDev, mader, type, code);

				// ��������ʧ�� �� ����������
				if (!strcmp(mader, "") || !strcmp(type, "") || !strcmp(code, ""))
				{
					// �ر�����������
					viClose(viDev);
					viDev = 0;

					// ��������˵��
					SetCtrlVal(panel, PANEL_CONF_TEXTMSG_CONN_STATE, "����ʧ��");
					// �����ɫͼƬ��·�������º�ɫͼƬ
					strcpy(tmpPath, projectDir);
					strcat(tmpPath, "\\Icon\\connectFaild.png");
					DisplayImageFile(panel, PANEL_CONF_PICTURE_CONNECT, tmpPath);

					// ��������
					MessagePopup("���棡", "��������ʧ�ܻ����������⣬��Ͽ������Ժ����Ի�������������");
					return -1;
				}

				// ƴ���������Һ��ͺ�
				if (strcmp(mader, "Keithley") == 0 || strcmp(mader, "KEITHLEY") == 0)
				{
					// ��� MODEL 2000
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
				// �����Сд��ĸ����ȫת�ɴ�д
				len = strlen(mader_type);
				for (size_t i = 0; i < len; ++i)
				{
					if (mader_type[i] >= 'a' && mader_type[i] <= 'z')
					{
						mader_type[i] -= ('a' - 'A');
					}
				}

				// ������������״̬
				connectState = 1;

				// ����Ǵ��ڣ���Ҫ���ò�����
				if (connectType == RS232)
				{
					// ���ò�����
					viSetAttribute(viDev, VI_ATTR_ASRL_BAUD, baudRate);
					// ����������
					viFlush(viDev, VI_ASRL_IN_BUF);
				}

				//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! �������ӳɹ� !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
				// ��������
				LoadConfigFromIniFile(configPanelHandle, &config);

				strcpy(dirPath, config.savePath);
				ResetTextBox(panel, PANEL_CONF_TEXTBOX_DATA_PATH, config.savePath); // ���ñ���·��

				// ���ò�����
				int baudrateIndex = 0;
				baudRate = config.baudRate;
				GetIndexFromValue(panel, PANEL_CONF_RING_BAUDRATE, &baudrateIndex, baudRate);
				if (baudrateIndex < 0)
					baudrateIndex = 0;
				SetCtrlIndex(panel, PANEL_CONF_RING_BAUDRATE, baudrateIndex);
				CallCtrlCallback(panel, PANEL_CONF_RING_BAUDRATE, EVENT_COMMIT, 0, 0, 0);

				// ���ò�����Ŀ
				int testItemIndex = 0;
				testItem = config.testItem;
				GetIndexFromValue(panel, PANEL_CONF_RING_TEST_ITEM, &testItemIndex, testItem);
				if (testItemIndex < 0)
					testItemIndex = 0;
				SetCtrlIndex(panel, PANEL_CONF_RING_TEST_ITEM, testItemIndex);
				CallCtrlCallback(panel, PANEL_CONF_RING_TEST_ITEM, EVENT_COMMIT, 0, 0, 0);

				// ���ò�������
				int rangeIndex = 0;
				range = config.range;
				GetIndexFromValue(panel, PANEL_CONF_RING_TEST_RANGE, &rangeIndex, range);
				if (rangeIndex < 0)
					rangeIndex = 0;
				SetCtrlIndex(panel, PANEL_CONF_RING_TEST_RANGE, rangeIndex);
				CallCtrlCallback(panel, PANEL_CONF_RING_TEST_RANGE, EVENT_COMMIT, 0, 0, 0);

				// ���Լ��
				samplingSpace = config.testSapce;
				sprintf(buf, "%.3lf", config.testSapce);
				SetCtrlVal(panel, PANEL_CONF_STRING_TIME_SPACE, buf); // ���ò���ʱ��

				// ��ѡ�����ò���ʱ��
				useFlag = config.testTimeUseFlag;
				if (config.testTimeUseFlag)
				{
					sprintf(buf, "%.3lf", config.testTime);
					SetCtrlVal(panel, PANEL_CONF_STRING_TEST_TIME, buf); // ���ò���ʱ��
					SetCtrlAttribute(panel, PANEL_CONF_STRING_TEST_TIME, ATTR_DIMMED, 0); // ����

					// ����ͼƬ
					strcpy(tmpPath, projectDir);
					strcat(tmpPath, "\\Icon\\PTBtnSelect.png");
					SetCtrlAttribute(panel, PANEL_CONF_PTBTN_USE_TESTTIME, ATTR_IMAGE_FILE, tmpPath);
					// ����ֵ
					SetCtrlVal(panel, PANEL_CONF_PTBTN_USE_TESTTIME, 1);
				}

				// ��ѡ�������Զ��洢
				autoSaveFlag = config.autoSaveFlag;
				if (config.autoSaveFlag)
				{
					strcpy(tmpPath, projectDir);
					strcat(tmpPath, "\\Icon\\PTBtnSelect.png");
					SetCtrlAttribute(panel, PANEL_CONF_PTBTN_AUTOSAVE, ATTR_IMAGE_FILE, tmpPath);
					SetCtrlVal(panel, PANEL_CONF_PTBTN_AUTOSAVE, 1);
				}
				//================================================= ����������� =================================================//

				// ��������˵��
				SetCtrlVal(panel, PANEL_CONF_TEXTMSG_CONN_STATE, "���ӳɹ�");
				// ������ɫͼƬ��·����������ɫͼƬ
				strcpy(tmpPath, projectDir);
				strcat(tmpPath, "\\Icon\\connectOk.png");
				DisplayImageFile(panel, PANEL_CONF_PICTURE_CONNECT, tmpPath);

				// �������Ӱ�ťͼƬΪ�Ͽ�����
				strcpy(tmpPath, projectDir);
				strcat(tmpPath, "\\Icon\\connect_0.png");
				SetCtrlAttribute(panel, PANEL_CONF_PBTN_CONNECT, ATTR_IMAGE_FILE, tmpPath);
			}
			else // ����������
			{
				// �ر�����
				viClose(viDev);
				viDev = 0;

				// ������������״̬
				connectState = 0;

				//============================================== ������������ ==============================================//
				strcpy(config.mader_type, mader_type); // ����_�ͺ�

				config.connectType = connectType; // ��������
				strcpy(config.srcAddr, viDevName); // ��Դ��ַ

				GetCtrlVal(configPanelHandle, PANEL_CONF_RING_BAUDRATE, &baudRate);
				config.baudRate = baudRate; // ������

				GetCtrlVal(configPanelHandle, PANEL_CONF_RING_TEST_ITEM, &testItem);
				config.testItem = testItem; // ������Ŀ

				GetCtrlVal(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, &range);
				config.range = range; // ��������

				//GetCtrlVal(configPanelHandle, PANEL_CONF_RING_DATA_WIDTH, &digit);
				//config.digit = digit; //����λ��

				GetCtrlVal(configPanelHandle, PANEL_CONF_STRING_TIME_SPACE, buf);
				// �ַ���תdouble
				samplingSpace = atof(buf);
				config.testSapce = samplingSpace; // �������

				GetCtrlVal(configPanelHandle, PANEL_CONF_STRING_TEST_TIME, buf);
				// �ַ���תdouble
				testTime = atof(buf);
				config.testTime = testTime; // ����ʱ��

				GetCtrlVal(configPanelHandle, PANEL_CONF_PTBTN_USE_TESTTIME, &(config.testTimeUseFlag)); // �ж��û��Ƿ�ѡ����
				GetCtrlVal(configPanelHandle, PANEL_CONF_PTBTN_AUTOSAVE, &(config.autoSaveFlag)); // �Զ��洢��־��

				// ��ȡ�ı�����
				GetNumTextBoxLines(configPanelHandle, PANEL_CONF_TEXTBOX_DATA_PATH, &lineCount);
				// ƴ��·��
				memset(dirPath, 0, sizeof(char) * 1024);
				for (size_t i = 0; i < lineCount; ++i)
				{
					GetTextBoxLine(configPanelHandle, PANEL_CONF_TEXTBOX_DATA_PATH, i, lineBuf);
					strcat(dirPath, lineBuf);
				}
				strcpy(config.savePath, dirPath); // ���Խ���ļ�����·��
				SaveConfigToIniFile(&config);

				// ��������˵��
				SetCtrlVal(panel, PANEL_CONF_TEXTMSG_CONN_STATE, "δ����");
				// �����ɫĬ��ͼƬ��·����������ͼƬ
				strcpy(tmpPath, projectDir);
				strcat(tmpPath, "\\Icon\\connectDefault.png");
				DisplayImageFile(panel, PANEL_CONF_PICTURE_CONNECT, tmpPath);

				// �����Ͽ�����ͼƬΪ����
				strcpy(tmpPath, projectDir);
				strcat(tmpPath, "\\Icon\\connect_1.png");
				SetCtrlAttribute(panel, PANEL_CONF_PBTN_CONNECT, ATTR_IMAGE_FILE, tmpPath);
			}
			break;
	}
	return 0;
}

/* ��������ʧ������ϵ���֪���˰�ť */
int CVICALLBACK PBtnIKnowCB(int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// ע������ʧ�����
			DiscardPanel(connectFaildPanelHandle);
			connectFaildPanelHandle = 0;
			connectFaildPanelHandleFlag = 0;
			break;
	}
	return 0;
}

/* ������Ŀ�����б� */
int CVICALLBACK RingTestItemCB(int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	char ***data = NULL;  // ��ѯ��������
	int num = -1;		  // ��������
	char sql[1024] = {0}; // SQL���

	switch (event)
	{
		case EVENT_COMMIT:
			// ������������б�
			DeleteListItem(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, 0, -1);
			
			// ��ȡ��ǰ������Ŀ
			GetCtrlVal(configPanelHandle, PANEL_CONF_RING_TEST_ITEM, &testItem);
			switch (testItem)
			{
				case MM_DCI: // ֱ������
					// ���²���y���ǩ
					SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_GRAPH, ATTR_YNAME, "����(A)");

					/*-------------------- �����ݿ��в���ֱ���������� --------------------*/
					// ��ѯֱ��������SQL���
					sprintf(sql, "SELECT label, val FROM dca_t WHERE type = '%s'", mader_type);

					// ��ѯ
					SqliteQuery(sql, &num, &data, 1024);

					// ��ÿ�����̶����ӵ������б�
					for (size_t i = 0; i < num; ++i)
					{
						InsertListItem(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, -1, data[i][0], atof(data[i][1]));
					}

					// ����������ָ��
					MulFunctionSet(viDev, MM_DCI);
					break;

				case MM_ACI: // ��������
					// ���²���y���ǩ
					SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_GRAPH, ATTR_YNAME, "����(A)");

					/*-------------------- �����ݿ��в��ҽ����������� --------------------*/
					// ��ѯֱ��������SQL���
					sprintf(sql, "SELECT label, val FROM aca_t WHERE type = '%s'", mader_type);

					// ��ѯ
					SqliteQuery(sql, &num, &data, 1024);

					// ��ÿ�����̶����ӵ������б�
					for (size_t i = 0; i < num; ++i)
					{
						InsertListItem(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, -1, data[i][0], atof(data[i][1]));
					}

					// ����������ָ��
					MulFunctionSet(viDev, MM_ACI);
					break;

				case MM_DCV: // ֱ����ѹ
					// ���²���y���ǩ
					SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_GRAPH, ATTR_YNAME, "��ѹ(V)");

					/*-------------------- �����ݿ��в���ֱ����ѹ���� --------------------*/
					// ��ѯֱ��������SQL���
					sprintf(sql, "SELECT label, val FROM dcv_t WHERE type = '%s'", mader_type);

					// ��ѯ
					SqliteQuery(sql, &num, &data, 1024);

					// ��ÿ�����̶����ӵ������б�
					for (size_t i = 0; i < num; ++i)
					{
						InsertListItem(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, -1, data[i][0], atof(data[i][1]));
					}
					
					// ����������ָ��
					MulFunctionSet(viDev, MM_DCV);
					break;

				case MM_ACV: // ������ѹ
					// ���²���y���ǩ
					SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_GRAPH, ATTR_YNAME, "��ѹ(V)");

					/*-------------------- �����ݿ��в��ҽ�����ѹ���� --------------------*/
					// ��ѯֱ��������SQL���
					sprintf(sql, "SELECT label, val FROM acv_t WHERE type = '%s'", mader_type);

					// ��ѯ
					SqliteQuery(sql, &num, &data, 1024);

					// ��ÿ�����̶����ӵ������б�
					for (size_t i = 0; i < num; ++i)
					{
						InsertListItem(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, -1, data[i][0], atof(data[i][1]));
					}
					
					// ����������ָ��
					MulFunctionSet(viDev, MM_ACV);
					break;

				case MM_RES: // ���ߵ���
					// ���²���y���ǩ
					SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_GRAPH, ATTR_YNAME, "����(��)");

					/*-------------------- �����ݿ��в��Ҷ��ߵ������� --------------------*/
					// ��ѯ���ߵ����SQL���
					sprintf(sql, "SELECT label, val FROM r2_t WHERE type = '%s'", mader_type);

					// ��ѯ
					SqliteQuery(sql, &num, &data, 1024);

					// ��ÿ�����̶����ӵ������б�
					for (size_t i = 0; i < num; ++i)
					{
						InsertListItem(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, -1, data[i][0], atof(data[i][1]));
					}

					// ����������ָ��
					MulFunctionSet(viDev, MM_RES);
					break;

				case MM_FRES: // ���ߵ���
					// ���²���y���ǩ
					SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_GRAPH, ATTR_YNAME, "����(��)");

					// �������������б�
					/*-------------------- �����ݿ��в������ߵ������� --------------------*/
					// ��ѯֱ����·��SQL���
					sprintf(sql, "SELECT label, val FROM r4_t WHERE type = '%s'", mader_type);

					// ��ѯ
					SqliteQuery(sql, &num, &data, 1024);

					// ��ÿ�����̶����ӵ������б�
					for (size_t i = 0; i < num; ++i)
					{
						InsertListItem(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, -1, data[i][0], atof(data[i][1]));
					}

					// ����������ָ��
					MulFunctionSet(viDev, MM_FRES);
					break;

				default:
					break;
			}
			// ������ݿ��ѯ��俪�ٵ��ڴ�
			FreeSqlData();
			data = NULL;

			// ����y��
			SetYRange();
			break;
	}
	return 0;
}

/* ���������б� */
int CVICALLBACK RingTestRangeCB(int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{

	int num; // �����б��е���Ŀ����
	
	switch (event)
	{
		case EVENT_COMMIT:
			// ������������б���û����Ŀ��ֱ���˳�
			GetNumListItems(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, &num);
			if (num <= 0)
			{
				return 0;
			}

			// ��ȡ����
			GetCtrlVal(configPanelHandle, PANEL_CONF_RING_TEST_RANGE, &range);

			// ��ȡ��ǰ������Ŀ
			GetCtrlVal(configPanelHandle, PANEL_CONF_RING_TEST_ITEM, &testItem);
			switch (testItem)
			{
				case MM_DCI:		   // ֱ������
					// ����������ָ��
					MulRangeSet(viDev, MM_DCI, range);
					break;

				case MM_ACI:		   // ��������
					// ����������ָ��
					MulRangeSet(viDev, MM_ACI, range);
					break;

				case MM_DCV:		   // ֱ����ѹ
					// ����������ָ��
					MulRangeSet(viDev, MM_DCV, range);
					break;

				case MM_ACV:		   // ������ѹ
					// ����������ָ��
					MulRangeSet(viDev, MM_ACV, range);
					break;

				case MM_RES:		   // ���ߵ���
					// ����������ָ��
					MulRangeSet(viDev, MM_RES, range);
					break;

				case MM_FRES:		   // ���ߵ���
					// ����������ָ��
					MulRangeSet(viDev, MM_FRES, range);
					break;

				default:
					break;
			}
			
			// ����Y������
			SetYRange();
			break;
	}
	return 0;
}

/* λ�����ֱ���/�������ʣ������б� */
int CVICALLBACK RingDataWidthCB(int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	char cmd[1024] = {0};
	char rateLabel[32] = {0};
	char rateVal[32] = {0};

	switch (event)
	{
		case EVENT_COMMIT:
			// ��ȡ��ǰ����
			GetCtrlVal(configPanelHandle, PANEL_CONF_RING_DATA_WIDTH, rateLabel);
			
			if (!strcmp(rateLabel, "fast")) // ��
			{
				strcpy(rateVal, "MIN");
			}
			else if (!strcmp(rateLabel, "med")) // ��
			{
				strcpy(rateVal, "DEF");
			}
			else // ��
			{
				strcpy(rateVal, "MAX");
			}
			
			switch (testItem)
			{
				case MM_DCI:
					if (!strcmp(mader_type, "R_DM3054") || !strcmp(mader_type, "R_DM3058")) // ��ԴDM3000ϵ��
					{
						sprintf(cmd, ":RESO:CURR:DC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "K_2000") || !strcmp(mader_type, "K_2002") || !strcmp(mader_type, "K_2010") || 
							 !strcmp(mader_type, "K_2100") || !strcmp(mader_type, "K_2700") || !strcmp(mader_type, "K_2750")) // ��ʱ��2000ϵ��
					{
						sprintf(cmd, "CURR:NPLC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "K_6500") || !strcmp(mader_type, "K_7510")) // ��ʱ��6500 7510
					{
						sprintf(cmd, "CURR:NPLC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "A_U3606A")) // ������U3606A
					{
						sprintf(cmd, "CURR:DC:RES %s\n", rateVal);
					}
					else
					{
						//==============================================
					}
					break;

				case MM_DCV:
					if (!strcmp(mader_type, "R_DM3054") || !strcmp(mader_type, "R_DM3058")) // ��Դ
					{
						sprintf(cmd, ":RESO:VOLT:DC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "K_2000") || !strcmp(mader_type, "K_2002") || !strcmp(mader_type, "K_2010") || 
							 !strcmp(mader_type, "K_2100") || !strcmp(mader_type, "K_2700") || !strcmp(mader_type, "K_2750")) // ��ʱ��2000ϵ��
					{
						sprintf(cmd, "VOLT:NPLC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "K_6500") || !strcmp(mader_type, "K_7510")) // ��ʱ��6500 7510
					{
						sprintf(cmd, "VOLT:NPLC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "A_U3606A")) // ������U3606A
					{
						sprintf(cmd, "VOLT:DC:RES %s\n", rateVal);
					}
					else
					{
						//==============================================
					}
					break;

				case MM_ACI:
					if (!strcmp(mader_type, "R_DM3054") || !strcmp(mader_type, "R_DM3058")) // ��Դ
					{
						sprintf(cmd, ":RESO:CURR:AC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "K_2000") || !strcmp(mader_type, "K_2002") || !strcmp(mader_type, "K_2010") || 
							 !strcmp(mader_type, "K_2100") || !strcmp(mader_type, "K_2700") || !strcmp(mader_type, "K_2750")) // ��ʱ��
					{
						sprintf(cmd, "CURR:AC:NPLC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "K_6500") || !strcmp(mader_type, "K_7510")) // ��ʱ��
					{
						sprintf(cmd, "CURR:AC:NPLC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "A_U3606A")) // ������U3606A
					{
						sprintf(cmd, "CURR:AC:RES %s\n", rateVal);
					}
					else
					{
						//==============================================
					}
					break;

				case MM_ACV:
					if (!strcmp(mader_type, "R_DM3054") || !strcmp(mader_type, "R_DM3058")) // ��Դ
					{
						sprintf(cmd, ":RESO:VOLT:AC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "K_2000") || !strcmp(mader_type, "K_2002") || !strcmp(mader_type, "K_2010") || 
							 !strcmp(mader_type, "K_2100") || !strcmp(mader_type, "K_2700") || !strcmp(mader_type, "K_2750")) // ��ʱ��2000ϵ��
					{
						sprintf(cmd, "VOLT:AC:NPLC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "K_6500") || !strcmp(mader_type, "K_7510")) // ��ʱ��6500
					{
						sprintf(cmd, "VOLT:AC:NPLC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "A_U3606A")) // ������U3606A
					{
						sprintf(cmd, "VOLT:AC:RES %s\n", rateVal);
					}
					else
					{
						//==============================================
					}
					break;

				case MM_RES:
					if (!strcmp(mader_type, "R_DM3054") || !strcmp(mader_type, "R_DM3058")) // ��Դ
					{
						sprintf(cmd, ":RESO:RES %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "K_2000") || !strcmp(mader_type, "K_2002") || !strcmp(mader_type, "K_2010") || 
							 !strcmp(mader_type, "K_2100") || !strcmp(mader_type, "K_2700") || !strcmp(mader_type, "K_2750")) // ��ʱ��2000ϵ��
					{
						sprintf(cmd, "RES:NPLC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "K_6500") || !strcmp(mader_type, "K_7510")) // ��ʱ��6500
					{
						sprintf(cmd, "RES:NPLC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "A_U3606A")) // ������U3606A
					{
						sprintf(cmd, "RES:RES %s\n", rateVal);
					}
					else
					{
						//==============================================
					}
					break;

				case MM_FRES:
					if (!strcmp(mader_type, "R_DM3054") || !strcmp(mader_type, "R_DM3058")) // ��Դ
					{
						sprintf(cmd, ":RESO:FRES %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "K_2000") || !strcmp(mader_type, "K_2002") || !strcmp(mader_type, "K_2010") || 
							 !strcmp(mader_type, "K_2100") || !strcmp(mader_type, "K_2700") || !strcmp(mader_type, "K_2750")) // ��ʱ��2000ϵ��
					{
						sprintf(cmd, "FRES:NPLC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "K_6500") || !strcmp(mader_type, "K_7510")) // ��ʱ��6500
					{
						sprintf(cmd, "FRES:NPLC %s\n", rateVal);
					}
					else if (!strcmp(mader_type, "A_U3606A")) // ������U3606A
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
			// дָ��
			viWrite(viDev, (ViBuf)cmd, strlen(cmd), VI_NULL);
			break;
	}
	return 0;
}

/* ���ݴ洢·����ť */
int CVICALLBACK PBtnDataSavePathCB(int panel, int control, int event,
								   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// �� �ļ���Դ������
			if (DirSelectPopup(projectDir, "ѡ���ļ���", 1, 1, dirPath) == 0) // û��ѡ���ļ���
			{
				return 0;
			}

			// ·��ǰ ���̷���ĸ��д
			if (dirPath[0] >= 'a' && dirPath[0] <= 'z')
			{
				dirPath[0] -= ('a' - 'A');
			}

			// ��ʾ�û�ѡ���·��
			ResetTextBox(configPanelHandle, PANEL_CONF_TEXTBOX_DATA_PATH, dirPath);
			break;
	}
	return 0;
}

// ���ò����ʱ����ť
int CVICALLBACK PTBtnUseTestTimeCB (int panel, int control, int event,
									void *callbackData, int eventData1, int eventData2)
{
	char tmpPath[1024] = {0};

	switch (event)
	{
		case EVENT_COMMIT:
			// �ж��û��Ƿ�ѡ����
			GetCtrlVal(panel, PANEL_CONF_PTBTN_USE_TESTTIME, &useFlag);

			if (useFlag == 0) // ûѡ
			{
				// ����ʱ���ı��򲻿���
				SetCtrlAttribute(configPanelHandle, PANEL_CONF_STRING_TEST_TIME, ATTR_DIMMED, 1);

				// ����ͼƬ
				strcpy(tmpPath, projectDir);
				strcat(tmpPath, "\\Icon\\PTBtnUnSelect.png");
				SetCtrlAttribute(panel, PANEL_CONF_PTBTN_USE_TESTTIME, ATTR_IMAGE_FILE, tmpPath);
			}
			else // ѡ
			{
				// ����ʱ���ı������
				SetCtrlAttribute(configPanelHandle, PANEL_CONF_STRING_TEST_TIME, ATTR_DIMMED, 0);

				// ����ͼƬ
				strcpy(tmpPath, projectDir);
				strcat(tmpPath, "\\Icon\\PTBtnSelect.png");
				SetCtrlAttribute(panel, PANEL_CONF_PTBTN_USE_TESTTIME, ATTR_IMAGE_FILE, tmpPath);
			}
			break;
	}
	return 0;
}

// �����Զ��洢��ť
int CVICALLBACK PTBtnAutoSaveCB (int panel, int control, int event,
								 void *callbackData, int eventData1, int eventData2)
{
	char tmpPath[1024] = {0};

	switch (event)
	{
		case EVENT_COMMIT:
			// �Զ��洢��־��
			GetCtrlVal(panel, control, &autoSaveFlag);

			// ����ͼƬ
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
