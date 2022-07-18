#include <Windows.h>
#include <formatio.h>
#include <utility.h>
#include <ansi_c.h>
#include <cvirte.h>
#include <userint.h>
#include <visa.h>
#include "toolbox.h"

#include "NS-Multimeter.h"

#include "license.h"
#include "ConfigPanel.h"
#include "PlotPanel.h"
#include "NsSqlite.h"
#include "NsuElement.h"
#include "SaveConfig.h"

//==============================================================================
// Static global variables
/* ��������Ƿ����� */
static volatile int unfoldFlag;
/* ������������� */
static int aboutUsPanelHandle;
/* �������������ر�־ */
static volatile int loadAboutUsPanelFlag;
/* ��Ȩ��� */
static int authorizePanelHandle;
/* ��Ȩ�����ر�־ */
static volatile int loadAuthorizePanelFlag;
/* ��Ȩ�ɹ���� */
static volatile int authorizeOkPanelHandle;
/* ��Ȩ�ɹ������ر�־ */
static volatile int loadAuthorizeOkPanelFlag;
/* ȡ����Ȩ��Ȩȷ����� */
static int cancelAuthorizePanelHandle;
/* ȡ����Ȩ��Ȩȷ�������ر�־ */
static volatile int loadCancelAuthorizePanelFlag;
/* ����ҳ */
static int startPagePanelHandle;
/* �˳�ʱ�Ķ���ȷ�Ͻ��� */
static int quitConfirmPanelHandle;
static volatile int quitConfirmPanelHandleFlag;
/* �Ƿ���Ȩ */
static volatile int authorizeFlag;

//==============================================================================
// Global variables
/* ����� */
int panelHandle;
/* ������ӡ�������� */
int configPanelHandle;
/* ������ӡ����������ر�־ */
volatile int configPanelHandleFlag;
/* �Ҳನ����� */
int wavePanelHandle;
/* �Ҳನ�������ر�־ */
volatile int wavePanelHandleFlag;
/* ����ʧ����� */
int connectFaildPanelHandle;
/* ����ʧ�������ر�־ */
volatile int connectFaildPanelHandleFlag;
/* ���������s�� */
double samplingSpace;
/* ����ʱ����s�� */
double testTime;
/* �Զ��洢��־ */
volatile int autoSaveFlag;
/* ���ò���ʱ����־ */
volatile int useFlag;
/* ����·�� */
char projectDir[1024];
/* ���Խ������·�� */
char dirPath[1024];
/* ��ǰ������Ŀ */
int testItem;
/* ���� */
double range;
/* λ������ */
int digit;
/* ����_�ͺ� */
char mader_type[1204];
/* VISA��Դ���� */
ViSession defaultRM;
/* �豸��� */
ViSession viDev;
/* ��ǰ�������� */
int connectType;
/* �豸���� */
char viDevName[1024];
/* ������ */
ViUInt32 baudRate;

/*------------------------------------------------ �������� ------------------------------------------------*/
void LoadCancelAuthorPanel();		   // ������Ȩȡ������
void LoadAuthorOkPanel();			   // ������Ȩ�ɹ����
void LoadAboutUsPanel();			   // ���ع����������
void LoadQuitConfirmPanel();		   // �����˳�ʱ����ȷ�Ͻ���
void LoadStartPage();				   // ������ʼҳ��
void LoadConfigPanel();				   // ���ز���ʾ�������
void LoadWavePanel();				   // ���ز���ʾ�������
void LoadAuthorPanel();				   // ������Ȩ���
int IsAuthorize();					   // �ж��û��Ƿ��ѻ����Ȩ
void InitSys();						   // ϵͳ��ʼ��
void LoadQuitConfirmPanel();		   // �˳�����ʱ�Ķ���ȷ�Ͻ���

//========================================== main ==========================================//
int main(int argc, char *argv[])
{
	if (InitCVIRTE(0, argv, 0) == 0)
		return -1; /* out of memory */

	// ��������ҳ
	LoadStartPage();

	if ((panelHandle = LoadPanel(0, "NS-Multimeter.uir", PANEL)) < 0)
		return -1;
	DisplayPanel(panelHandle);

	// ��ʼ��
	InitSys();

	// ����û��Ƿ�����Ȩ
	IsAuthorize();

	RunUserInterface();
	DiscardPanel(panelHandle);
	return 0;
}

// ������Ȩȡ������
void LoadCancelAuthorPanel()
{
	if (loadCancelAuthorizePanelFlag == 0)
	{
		// ȡ����Ȩȷ�������ر�־��1
		loadCancelAuthorizePanelFlag = 1;
		// ������ť������ ȡ����Ȩȷ�����
		cancelAuthorizePanelHandle = LoadPanel(panelHandle, "NS-Multimeter.uir", PANEL_CNAU);
	}
	// �������λ��
	SetPanelPos(cancelAuthorizePanelHandle, 205, 422);
	// ��ʾ ȡ����Ȩȷ��������
	DisplayPanel(cancelAuthorizePanelHandle);
}

// ������Ȩ�ɹ����
void LoadAuthorOkPanel()
{
	if (loadAuthorizeOkPanelFlag == 0)
	{
		loadAuthorizeOkPanelFlag = 1;
		// ������Ȩ�ɹ����
		authorizeOkPanelHandle = LoadPanel(panelHandle, "NS-Multimeter.uir", PANEL_AUOK);
	}
	// �������λ��
	SetPanelPos(authorizeOkPanelHandle, 205, 422);
	// ��ʾ��Ȩ�ɹ����
	DisplayPanel(authorizeOkPanelHandle);
}

/* ���ز���ʾ������� */
void LoadWavePanel()
{
	if (wavePanelHandleFlag == 0)
	{
		wavePanelHandleFlag = 1;
		// �����Ҳನ�����
		wavePanelHandle = LoadPanel(panelHandle, "NS-Multimeter.uir", PANEL_WAVE);
	}

	// �������λ��
	SetPanelPos(wavePanelHandle, 52, 252);
	// ��ʾ��� �������
	DisplayPanel(wavePanelHandle);
}

/* ���ز���ʾ������� */
void LoadConfigPanel()
{
	if (configPanelHandleFlag == 0)
	{
		configPanelHandleFlag = 1;

		// ������� �������
		configPanelHandle = LoadPanel(panelHandle, "NS-Multimeter.uir", PANEL_CONF);
	}
	// �������λ��
	SetPanelPos(configPanelHandle, 41, 0);
	// ��ʾ��� �������
	DisplayPanel(configPanelHandle);
}

// ���ع����������
void LoadAboutUsPanel()
{
	// ���δ���ع����
	if (loadAboutUsPanelFlag == 0)
	{
		// ��־��1
		loadAboutUsPanelFlag = 1;
		// �������
		aboutUsPanelHandle = LoadPanel(panelHandle, "NS-Multimeter.uir", PANEL_ABUS);
	}
	// ��ʾ���
	DisplayPanel(aboutUsPanelHandle);
}

/* �����˳�ʱ����ȷ�Ͻ��� */
void LoadQuitConfirmPanel()
{
	if (quitConfirmPanelHandleFlag == 0)
	{
		quitConfirmPanelHandleFlag = 1;
		quitConfirmPanelHandle = LoadPanel(panelHandle, "NS-Multimeter.uir", PANEL_QUCF);
	}
	DisplayPanel(quitConfirmPanelHandle);
}

// ����ҳ���
void LoadStartPage()
{
	startPagePanelHandle = LoadPanel(0, "NS-Multimeter.uir", PANEL_SPAG);
	DisplayPanel(startPagePanelHandle);
	Delay(3);
	DiscardPanel(startPagePanelHandle);
}

/* �ж��û��Ƿ��ѻ����Ȩ 1�����Ȩ 0δ�����Ȩ */
int IsAuthorize()
{
	char message[1024] = {0}; 		// ��Ϣ
	int surplus;			  		// ʣ������
	int ret;
	char signatureCode[1024] = {0}; // ����������
	char remainDay[128] = {0};		// ��Ȩʣ��ʱ��

	// ����û���license�Ƿ����
	ret = CheckLicense(message, &surplus);
	if (ret != 0) // �û���license����ʹ��
	{
		// �ж��ܷ������Ȩ���
		if (loadAuthorizePanelFlag == 0)
		{
			// ������Ȩ���
			authorizePanelHandle = LoadPanel(panelHandle, "NS-Multimeter.uir", PANEL_AUTH);

			// ��Ȩ�����ر�־��1
			loadAuthorizePanelFlag = 1;
		}

		// �������λ��
		SetPanelPos(authorizePanelHandle, 167, 368);
		// ��ʾ���
		DisplayPanel(authorizePanelHandle);
		
		// ��ȡ����������
		GetSignature(signatureCode);
		// ��ʾ�����뵽 ��Ȩ����ϵ�String�ؼ�
		SetCtrlVal(authorizePanelHandle, PANEL_AUTH_STRING_PC_SIGNATURE, signatureCode);

		// ��ʼ���԰�ť�����ã��û������Ȩ�����
		SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_START_TEST, ATTR_DIMMED, 1);
		// �������Ӱ�ť�����ã��û���Ȩ�����
		SetCtrlAttribute(configPanelHandle, PANEL_CONF_PBTN_CONNECT, ATTR_DIMMED, 1);

		// �û���Ȩδ��Ȩ
		authorizeFlag = 0;
	}
	else // ret == 0�� �û���license����ʹ�ã��鿴��Ȩʣ������
	{
		if (surplus == -1) // ������Ȩ
		{
			// ��ʾ������Ȩ��ť
			SetCtrlAttribute(panelHandle, PANEL_PBTN_FOREVER_AUTHOR, ATTR_VISIBLE, 1);

			// ������δ��Ȩ��ť
			SetCtrlAttribute(panelHandle, PANEL_PBTN_NOT_AUTHOR, ATTR_VISIBLE, 0);

			// ������ʱ��ȨͼƬ��ť
			SetCtrlAttribute(panelHandle, PANEL_PBTN_LIMIT_AUTHOR, ATTR_VISIBLE, 0);
		}
		else // ������Ȩ
		{
			// ����������Ȩ��ť
			SetCtrlAttribute(panelHandle, PANEL_PBTN_FOREVER_AUTHOR, ATTR_VISIBLE, 0);

			// ������δ��Ȩ��ť
			SetCtrlAttribute(panelHandle, PANEL_PBTN_NOT_AUTHOR, ATTR_VISIBLE, 0);

			// ��ȡ��Ȩʣ��ʱ��
			sprintf(remainDay, "��Ч��%d��", surplus);
			// ������ʱ��Ȩ��ť�ı�ǩ
			SetCtrlAttribute(panelHandle, PANEL_PBTN_LIMIT_AUTHOR, ATTR_LABEL_TEXT, remainDay);
			// ��ʾ��ʱ��ȨͼƬ��ť
			SetCtrlAttribute(panelHandle, PANEL_PBTN_LIMIT_AUTHOR, ATTR_VISIBLE, 1);
		}
		// ��Ȩ��־��1
		authorizeFlag = 1;

		// �û������Ȩ����ʼ���԰�ť����
		SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_START_TEST, ATTR_DIMMED, 0);
		// �û������Ȩ���������Ӱ�ť����
		SetCtrlAttribute(configPanelHandle, PANEL_CONF_PBTN_CONNECT, ATTR_DIMMED, 0);
	}
	return 0;
}

/* ϵͳ��ʼ�� */
void InitSys()
{
	InstrConfig config = {0}; 	// �ϴ���������
	
	// ��VISA��Դ������
	viOpenDefaultRM(&defaultRM);

	// ���ز���ʾ�������
	LoadConfigPanel();

	// ���ز���ʾ�������
	LoadWavePanel();

	// �����ݿ�
	OpenSqliteDB("ns.db");

	// ��ȡ����·��
	GetDir(projectDir);
	
	// ��������
	LoadConfigFromIniFile(configPanelHandle, &config);
	
	// �������ӷ�ʽ
	int connectTypeIndex = 0;
	connectType = config.connectType;
	GetIndexFromValue(configPanelHandle, PANEL_CONF_RING_CONN_TYPE, &connectTypeIndex, connectType);
	if (connectTypeIndex < 0)
		connectTypeIndex = 0;
	SetCtrlIndex(configPanelHandle, PANEL_CONF_RING_CONN_TYPE, connectTypeIndex);
	CallCtrlCallback(configPanelHandle, PANEL_CONF_RING_CONN_TYPE, EVENT_COMMIT, 0, 0, 0);
	
	// ������Դ��ַ
	int srcAddrIndex = 0;
	strcpy(viDevName, config.srcAddr);
	GetIndexFromValue(configPanelHandle, PANEL_CONF_RING_SRC_ADDR, &srcAddrIndex, viDevName);
	if (srcAddrIndex < 0)
		srcAddrIndex = 0;
	SetCtrlIndex(configPanelHandle, PANEL_CONF_RING_SRC_ADDR, srcAddrIndex);
	CallCtrlCallback(configPanelHandle, PANEL_CONF_RING_SRC_ADDR, EVENT_COMMIT, 0, 0, 0);
}

/* ���ز���ʾ��Ȩ���� */
void LoadAuthorPanel()
{
	/* ���������� */
	char signatureCode[1024] = {0};

	// �ж��ܷ�������
	if (loadAuthorizePanelFlag == 0)
	{
		// ��Ȩ�����ر�־��1
		loadAuthorizePanelFlag = 1;
		// ������Ȩ���
		authorizePanelHandle = LoadPanel(panelHandle, "NS-Multimeter.uir", PANEL_AUTH);
	}

	// ��ȡ����������
	GetSignature(signatureCode);
	// ��ʾ�����뵽 ��Ȩ����ϵ�String�ؼ�
	SetCtrlVal(authorizePanelHandle, PANEL_AUTH_STRING_PC_SIGNATURE, signatureCode);
	// �������λ��
	SetPanelPos(authorizePanelHandle, 167, 368);
	// ��ʾ���
	DisplayPanel(authorizePanelHandle);
}

//====================================== �ؼ��ص� ======================================//

/* ���Ͻǵ��˳���ť */
int CVICALLBACK PBtnQuitCallback(int panel, int control, int event,
								 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// �����˳�����ȷ�Ͻ���
			LoadQuitConfirmPanel();
			break;
	}
	return 0;
}

/* ���Ͻǵ���С����ť */
int CVICALLBACK PBtnMiniMize(int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// ��С������
			SetPanelAttribute(panel, ATTR_WINDOW_ZOOM, VAL_MINIMIZE);
			break;
	}
	return 0;
}

/* �û����涥������Ļ���ص� */
int CVICALLBACK CanvasMoveCB(int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{

	switch (event)
	{
		case EVENT_LEFT_CLICK:
			// �϶�����ƶ�
			DragPanelMove(panel, control);
			break;

		default:
			break;
	}
	return 0;
}

/* ���ϽǵĹ������ǰ�ť */
int CVICALLBACK PBtnAboutUsCB(int panel, int control, int event,
							  void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_LEFT_CLICK:
			// ���ع����������
			LoadAboutUsPanel();
			break;

		default:
			break;
	}
	return 0;
}

/* ��������������ϽǵĹرհ�ť */
int CVICALLBACK PBtnAboutUsQuitCB(int panel, int control, int event,
								  void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_LEFT_CLICK:
			// ע�����
			DiscardPanel(aboutUsPanelHandle);

			// ��־��0
			loadAboutUsPanelFlag = 0;
			break;
	}
	return 0;
}

/* ���Ͻǵ��û�������ť */
int CVICALLBACK PBtnUserHelpCB(int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_LEFT_CLICK:
			// ��굥��
			/*=====================================================================================================================*/
			break;
	}
	return 0;
}

/* ������Ȩ��ť */
int CVICALLBACK PBtnForeverAuthorCB(int panel, int control, int event,
									void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_RIGHT_CLICK: // �û�����һ�
			// �Ҽ� ��ʾȡ����Ȩ��ť
			SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_CANCEL_AUTHOR, ATTR_VISIBLE, 1);
			break;
	}
	return 0;
}

/* �û����涥������δ��Ȩ��ť */
int CVICALLBACK PBtnAuthorizeCB(int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// ���ز���ʾ��Ȩ����
			LoadAuthorPanel();
			break;
	}
	return 0;
}

/* ��Ȩҳ��� �ر�/�˳� ��ť */
int CVICALLBACK PBtnAuthorizeQuitCB(int panel, int control, int event,
									void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// ע�����
			DiscardPanel(authorizePanelHandle);
			// ��Ȩ�����ر�־��0
			loadAuthorizePanelFlag = 0;
			break;
	}
	return 0;
}

/* ��Ȩҳ��� ȷ�ϰ�ť */
int CVICALLBACK PBtnAuthorConfirmCB(int panel, int control, int event,
									void *callbackData, int eventData1, int eventData2)
{
	/* �û������license */
	char license[1024] = {0};

	/* ��Ϣ */
	char message[1024] = {0};

	/* ��Ȩ����֤���������� 0:ͨ�� 1:license��Ч 2:����ע��ɹ��� */
	int result = -1;

	switch (event)
	{
		case EVENT_COMMIT:
			// ��ȡ�û��������Ȩ��
			GetCtrlVal(authorizePanelHandle, PANEL_AUTH_STRING_AUTHOR_CODE, license);

			// ��֤��Ȩ��
			result = CheckRegister(license, message);
			if (result == 0 || result == 2) // ע��ɹ�
			{
				// ��Ȩ��־��1
				authorizeFlag = 1;

				// �ر���Ȩ���
				DiscardPanel(authorizePanelHandle);
				// ��Ȩ�����ر�־��0
				loadAuthorizePanelFlag = 0;

				// �û������Ȩ����ʼ���԰�ť����
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_START_TEST, ATTR_DIMMED, 0);

				// �û������Ȩ���������Ӱ�ť����
				SetCtrlAttribute(configPanelHandle, PANEL_CONF_PBTN_CONNECT, ATTR_DIMMED, 0);

				// ������δ��Ȩ��ť
				SetCtrlAttribute(panelHandle, PANEL_PBTN_NOT_AUTHOR, ATTR_VISIBLE, 0);
				// �ж� ������Ȩ ���� ������Ȩ
				if (result == 0) // ����
				{
					// ��ʾ������Ȩ��ť
					SetCtrlAttribute(panelHandle, PANEL_PBTN_FOREVER_AUTHOR, ATTR_VISIBLE, 1);

					// ������ʱ��Ȩ��ť
					SetCtrlAttribute(panelHandle, PANEL_PBTN_LIMIT_AUTHOR, ATTR_VISIBLE, 0);
				}
				else // ����
				{
					// ����������Ȩ��ť
					SetCtrlAttribute(panelHandle, PANEL_PBTN_FOREVER_AUTHOR, ATTR_VISIBLE, 0);

					// ��ʾ��ʱ��Ȩ��ť
					SetCtrlAttribute(panelHandle, PANEL_PBTN_LIMIT_AUTHOR, ATTR_VISIBLE, 1);

					// ��ʾ��ʾʱ��
					IsAuthorize();
				}

				// ������Ȩ�ɹ����
				LoadAuthorOkPanel();
			}
			else // license��Ч
			{
				// ��ʾ��Ȩ�����
				SetCtrlAttribute(authorizePanelHandle, PANEL_AUTH_TEXTMSG_LIC_ERR, ATTR_TEXT_COLOR, 0xEB4747); // ����������ɫ
				SetCtrlVal(authorizePanelHandle, PANEL_AUTH_TEXTMSG_LIC_ERR, "��Ȩ�����");
			}
			break;
	}
	return 0;
}

/* ��Ȩ�ɹ��󣬼�����Ȩ�ɹ���壬������п�ʼ���԰�ť */
int CVICALLBACK PBtnStartTestAfterAuthorCB(int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// �����ʼ���԰�ť��ע����Ȩ�ɹ����
			DiscardPanel(authorizeOkPanelHandle);
			authorizeOkPanelHandle = 0;

			// ��Ȩ�ɹ������ر�־��0
			loadAuthorizeOkPanelFlag = 0;
			break;
	}
	return 0;
}

/* ȡ����Ȩ��ť */
int CVICALLBACK PBtnCancelAuthorCB(int panel, int control, int event,
								   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_LEFT_CLICK:
			// ����ȡ����Ȩȷ��������
			LoadCancelAuthorPanel();
			// ���� ȡ����Ȩ��ť
			SetCtrlAttribute(panel, PANEL_WAVE_PBTN_CANCEL_AUTHOR, ATTR_VISIBLE, 0);
			break;
	}
	return 0;
}

/* ȡ����Ȩ��� �ϵ� ȡ����ť */
int CVICALLBACK PBtnCancelAuthorCancelCB(int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_LEFT_CLICK:
			// ע�� ȡ����Ȩ���
			DiscardPanel(cancelAuthorizePanelHandle);

			// ȡ����Ȩȷ�������ر�־��1
			loadCancelAuthorizePanelFlag = 0;
			break;
	}
	return 0;
}

/* ȡ����Ȩ��� �ϵ� ȷ�ϰ�ť */
int CVICALLBACK PBtnCancelAuthorOkCB(int panel, int control, int event,
									 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// ȡ����Ȩ
			CancelLicense();

			// ��Ȩ��־��0
			authorizeFlag = 0;

			// ע�� ȡ����Ȩ���
			DiscardPanel(cancelAuthorizePanelHandle);

			// ȡ����Ȩȷ�������ر�־��1
			loadCancelAuthorizePanelFlag = 0;

			// ����������Ȩ��ť
			SetCtrlAttribute(panelHandle, PANEL_PBTN_FOREVER_AUTHOR, ATTR_VISIBLE, 0);
			// ������ʱ��Ȩ��ť
			SetCtrlAttribute(panelHandle, PANEL_PBTN_LIMIT_AUTHOR, ATTR_VISIBLE, 0);
			// ��ʾ��δ��Ȩ��ť
			SetCtrlAttribute(panelHandle, PANEL_PBTN_NOT_AUTHOR, ATTR_VISIBLE, 1);
			
			// ��ʼ���ԡ��������Ӻ����ݴ洢��ť������
			SetCtrlAttribute(configPanelHandle, PANEL_CONF_PBTN_CONNECT, ATTR_DIMMED, 1);
			SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_START_TEST, ATTR_DIMMED, 1);
			SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_SAVE_DATA, ATTR_DIMMED, 1);
			break;
	}
	return 0;
}

/* �����ص� */
int CVICALLBACK PanelCB(int panel, int event, void *callbackData,
						int eventData1, int eventData2)
{
	int btnX, btnY; // ��ť��ǰ��λ��
	int x, y;		// ��� ��ǰ��� ȡ����Ȩ��ť �� λ��
	int state;		// ȡ����Ȩ��ť ��ǰ�� ״̬���Ƿ����أ�

	switch (event)
	{
		case EVENT_LEFT_CLICK: // ������� ȡ����Ȩ��ť ����ĵط������� ȡ����Ȩ��ť
			// ��ȡ ȡ����Ȩ��ť ��ǰ�� ״̬���Ƿ����أ�
			GetCtrlAttribute(panel, PANEL_WAVE_PBTN_CANCEL_AUTHOR, ATTR_VISIBLE, &state);
			if (state) // ȡ����Ȩ��ť ��ǰû�б�����
			{
				// ��ȡ ��� ��ǰ��� ȡ����Ȩ��ť �� λ��
				GetRelativeMouseState(panel, PANEL_WAVE_PBTN_CANCEL_AUTHOR, &x, &y, NULL, NULL, NULL);

				// ��ȡ��ť��ǰλ��
				GetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_CANCEL_AUTHOR, ATTR_LEFT, &btnX);
				GetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_CANCEL_AUTHOR, ATTR_TOP, &btnY);

				// ��� �� ȡ����Ȩ��ť ����ĵط�
				if (x < btnX || x > 123 + btnX || y < btnY || y > 58 + btnY)
				{
					// ���� ȡ����Ȩ��ť
					SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_CANCEL_AUTHOR, ATTR_VISIBLE, 0);
				}
			}
			break;
	}
	return 0;
}

/* չ������ť */
int CVICALLBACK PBtnUnfoldCB(int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	char tmpPath[1024] = {0};

	switch (event)
	{
		case EVENT_COMMIT:
			// �ж��������״̬
			if (unfoldFlag) // ������崦������״̬
			{
				// ��ʾ �������
				DisplayPanel(configPanelHandle);

				// ����չ������ť��ͼƬ
				strcpy(tmpPath, projectDir);
				strcat(tmpPath, "\\Icon\\unfold_0.png");
				SetCtrlAttribute(panelHandle, PANEL_PBTN_UNFOLD, ATTR_IMAGE_FILE, tmpPath);
				
				// �ƶ�չ������ť ��λ��
				SetCtrlAttribute(panel, PANEL_PBTN_UNFOLD, ATTR_LEFT, 226);

				// �ָ���������С
				SetPanelSize(wavePanelHandle, 619, 1018);

				// �ָ��������λ��
				SetPanelPos(wavePanelHandle, 51, 252);

				// �ָ�������ʾ�ؼ���С
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_GRAPH, ATTR_WIDTH, 1008);

				// �ָ� ����������Ͻǵ� ��ʼ���� �� ���ݴ洢 ��ťλ��
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_START_TEST, ATTR_LEFT, 800);
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_SAVE_DATA, ATTR_LEFT, 910);

				// �������չ��
				unfoldFlag = 0;
			}
			else // ������崦����ʾ״̬
			{
				// ���� �������
				HidePanel(configPanelHandle);
				
				// ����չ������ť��ͼƬ
				strcpy(tmpPath, projectDir);
				strcat(tmpPath, "\\Icon\\unfold_1.png");
				SetCtrlAttribute(panelHandle, PANEL_PBTN_UNFOLD, ATTR_IMAGE_FILE, tmpPath);

				// �ƶ�չ������ť ��λ��
				SetCtrlAttribute(panel, PANEL_PBTN_UNFOLD, ATTR_LEFT, 0);

				// �Ŵ������
				SetPanelSize(wavePanelHandle, 619, 1243);

				// ���ò������λ��
				SetPanelPos(wavePanelHandle, 51, 27);

				// �Ŵ�����ʾ�ؼ�
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_GRAPH, ATTR_WIDTH, 1230);

				// ���� ����������Ͻǵ� ��ʼ���� �� ���ݴ洢 ��ťλ��
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_START_TEST, ATTR_LEFT, 1025);
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_SAVE_DATA, ATTR_LEFT, 1135);

				// �����������
				unfoldFlag = 1;
			}
			break;
	}
	return 0;
}

// �˳�ʱ����ȷ��ȡ����ť
int CVICALLBACK PBtnQuitConfirmNoCB (int panel, int control, int event,
									 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// ע���˳�ʱ����ȷ�Ͻ���
			quitConfirmPanelHandleFlag = 0;
			DiscardPanel(quitConfirmPanelHandle);
			break;
	}
	return 0;
}

// �˳�ʱ����ȷ��ȷ�ϰ�ť
int CVICALLBACK PBtnQuitConfirmYesCB (int panel, int control, int event,
									  void *callbackData, int eventData1, int eventData2)
{
	InstrConfig config = {0};
	int lineCount = 0;
	char lineBuf[1024] = {0};
	char buf[64] = {0};
	//int digit = 0;
	
	switch (event)
	{
		case EVENT_COMMIT:
			// ����������û�رգ���ر�
			if (viDev)
				viClose(viDev);

			// �ر�VISA��Դ������
			viClose(defaultRM);
			
			// �ر����ݿ�
			CloseSqliteDB();
			
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
			
			// �����������ݵ�ini�ļ�
			SaveConfigToIniFile(&config);

			// ע���˳�ʱ����ȷ�Ͻ���
			if (quitConfirmPanelHandleFlag)
				DiscardPanel(quitConfirmPanelHandle);
			
			// ע����Ȩ���
			if (authorizePanelHandle)
				DiscardPanel(authorizePanelHandle);
			
			// ע����� �������
			if (configPanelHandleFlag)
				DiscardPanel(configPanelHandle);

			// ע���Ҳ� �������
			if (wavePanelHandleFlag)
				DiscardPanel(wavePanelHandle);

			// ע������ʧ�����
			if (connectFaildPanelHandleFlag)
				DiscardPanel(connectFaildPanelHandle);

			// �������
			QuitUserInterface(0);
			break;
	}
	return 0;
}