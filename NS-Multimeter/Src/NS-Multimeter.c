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
/* 配置面板是否收起 */
static volatile int unfoldFlag;
/* 关于我们面板句柄 */
static int aboutUsPanelHandle;
/* 关于我们面板加载标志 */
static volatile int loadAboutUsPanelFlag;
/* 授权面板 */
static int authorizePanelHandle;
/* 授权面板加载标志 */
static volatile int loadAuthorizePanelFlag;
/* 授权成功面板 */
static volatile int authorizeOkPanelHandle;
/* 授权成功面板加载标志 */
static volatile int loadAuthorizeOkPanelFlag;
/* 取消授权授权确认面板 */
static int cancelAuthorizePanelHandle;
/* 取消授权授权确认面板加载标志 */
static volatile int loadCancelAuthorizePanelFlag;
/* 启动页 */
static int startPagePanelHandle;
/* 退出时的二次确认界面 */
static int quitConfirmPanelHandle;
static volatile int quitConfirmPanelHandleFlag;
/* 是否授权 */
static volatile int authorizeFlag;

//==============================================================================
// Global variables
/* 主面板 */
int panelHandle;
/* 左侧连接、配置面板 */
int configPanelHandle;
/* 左侧连接、配置面板加载标志 */
volatile int configPanelHandleFlag;
/* 右侧波形面板 */
int wavePanelHandle;
/* 右侧波形面板加载标志 */
volatile int wavePanelHandleFlag;
/* 连接失败面板 */
int connectFaildPanelHandle;
/* 连接失败面板加载标志 */
volatile int connectFaildPanelHandleFlag;
/* 采样间隔（s） */
double samplingSpace;
/* 测试时长（s） */
double testTime;
/* 自动存储标志 */
volatile int autoSaveFlag;
/* 启用测试时长标志 */
volatile int useFlag;
/* 工程路径 */
char projectDir[1024];
/* 测试结果保存路径 */
char dirPath[1024];
/* 当前测试项目 */
int testItem;
/* 量程 */
double range;
/* 位数索引 */
int digit;
/* 厂家_型号 */
char mader_type[1204];
/* VISA资源管器 */
ViSession defaultRM;
/* 设备句柄 */
ViSession viDev;
/* 当前连接类型 */
int connectType;
/* 设备名称 */
char viDevName[1024];
/* 波特率 */
ViUInt32 baudRate;

/*------------------------------------------------ 函数声明 ------------------------------------------------*/
void LoadCancelAuthorPanel();		   // 加载授权取消界面
void LoadAuthorOkPanel();			   // 加载授权成功面板
void LoadAboutUsPanel();			   // 加载关于我们面板
void LoadQuitConfirmPanel();		   // 加载退出时二次确认界面
void LoadStartPage();				   // 加载起始页面
void LoadConfigPanel();				   // 加载并显示配置面板
void LoadWavePanel();				   // 加载并显示波形面板
void LoadAuthorPanel();				   // 加载授权面板
int IsAuthorize();					   // 判断用户是否已获得授权
void InitSys();						   // 系统初始化
void LoadQuitConfirmPanel();		   // 退出软件时的二次确认界面

//========================================== main ==========================================//
int main(int argc, char *argv[])
{
	if (InitCVIRTE(0, argv, 0) == 0)
		return -1; /* out of memory */

	// 加载启动页
	LoadStartPage();

	if ((panelHandle = LoadPanel(0, "NS-Multimeter.uir", PANEL)) < 0)
		return -1;
	DisplayPanel(panelHandle);

	// 初始化
	InitSys();

	// 检查用户是否获得授权
	IsAuthorize();

	RunUserInterface();
	DiscardPanel(panelHandle);
	return 0;
}

// 加载授权取消界面
void LoadCancelAuthorPanel()
{
	if (loadCancelAuthorizePanelFlag == 0)
	{
		// 取消授权确认面板加载标志置1
		loadCancelAuthorizePanelFlag = 1;
		// 单击按钮，弹出 取消授权确认面板
		cancelAuthorizePanelHandle = LoadPanel(panelHandle, "NS-Multimeter.uir", PANEL_CNAU);
	}
	// 设置面板位置
	SetPanelPos(cancelAuthorizePanelHandle, 205, 422);
	// 显示 取消授权确认面板面板
	DisplayPanel(cancelAuthorizePanelHandle);
}

// 加载授权成功面板
void LoadAuthorOkPanel()
{
	if (loadAuthorizeOkPanelFlag == 0)
	{
		loadAuthorizeOkPanelFlag = 1;
		// 加载授权成功面板
		authorizeOkPanelHandle = LoadPanel(panelHandle, "NS-Multimeter.uir", PANEL_AUOK);
	}
	// 设置面板位置
	SetPanelPos(authorizeOkPanelHandle, 205, 422);
	// 显示授权成功面板
	DisplayPanel(authorizeOkPanelHandle);
}

/* 加载并显示波形面板 */
void LoadWavePanel()
{
	if (wavePanelHandleFlag == 0)
	{
		wavePanelHandleFlag = 1;
		// 加载右侧波形面板
		wavePanelHandle = LoadPanel(panelHandle, "NS-Multimeter.uir", PANEL_WAVE);
	}

	// 设置面板位置
	SetPanelPos(wavePanelHandle, 52, 252);
	// 显示左侧 配置面板
	DisplayPanel(wavePanelHandle);
}

/* 加载并显示配置面板 */
void LoadConfigPanel()
{
	if (configPanelHandleFlag == 0)
	{
		configPanelHandleFlag = 1;

		// 加载左侧 配置面板
		configPanelHandle = LoadPanel(panelHandle, "NS-Multimeter.uir", PANEL_CONF);
	}
	// 设置面板位置
	SetPanelPos(configPanelHandle, 41, 0);
	// 显示左侧 配置面板
	DisplayPanel(configPanelHandle);
}

// 加载关于我们面板
void LoadAboutUsPanel()
{
	// 如果未加载过面板
	if (loadAboutUsPanelFlag == 0)
	{
		// 标志置1
		loadAboutUsPanelFlag = 1;
		// 加载面板
		aboutUsPanelHandle = LoadPanel(panelHandle, "NS-Multimeter.uir", PANEL_ABUS);
	}
	// 显示面板
	DisplayPanel(aboutUsPanelHandle);
}

/* 加载退出时二次确认界面 */
void LoadQuitConfirmPanel()
{
	if (quitConfirmPanelHandleFlag == 0)
	{
		quitConfirmPanelHandleFlag = 1;
		quitConfirmPanelHandle = LoadPanel(panelHandle, "NS-Multimeter.uir", PANEL_QUCF);
	}
	DisplayPanel(quitConfirmPanelHandle);
}

// 启动页面板
void LoadStartPage()
{
	startPagePanelHandle = LoadPanel(0, "NS-Multimeter.uir", PANEL_SPAG);
	DisplayPanel(startPagePanelHandle);
	Delay(3);
	DiscardPanel(startPagePanelHandle);
}

/* 判断用户是否已获得授权 1获得授权 0未获得授权 */
int IsAuthorize()
{
	char message[1024] = {0}; 		// 消息
	int surplus;			  		// 剩余天数
	int ret;
	char signatureCode[1024] = {0}; // 电脑特征码
	char remainDay[128] = {0};		// 授权剩余时长

	// 检查用户的license是否可用
	ret = CheckLicense(message, &surplus);
	if (ret != 0) // 用户的license不可使用
	{
		// 判断能否加载授权面板
		if (loadAuthorizePanelFlag == 0)
		{
			// 加载授权面板
			authorizePanelHandle = LoadPanel(panelHandle, "NS-Multimeter.uir", PANEL_AUTH);

			// 授权面板加载标志置1
			loadAuthorizePanelFlag = 1;
		}

		// 设置面板位置
		SetPanelPos(authorizePanelHandle, 167, 368);
		// 显示面板
		DisplayPanel(authorizePanelHandle);
		
		// 获取电脑特征码
		GetSignature(signatureCode);
		// 显示特征码到 授权面板上的String控件
		SetCtrlVal(authorizePanelHandle, PANEL_AUTH_STRING_PC_SIGNATURE, signatureCode);

		// 开始测试按钮不可用，用户完成授权后可用
		SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_START_TEST, ATTR_DIMMED, 1);
		// 仪器连接按钮不可用，用户授权后可用
		SetCtrlAttribute(configPanelHandle, PANEL_CONF_PBTN_CONNECT, ATTR_DIMMED, 1);

		// 用户授权未授权
		authorizeFlag = 0;
	}
	else // ret == 0， 用户的license可以使用，查看授权剩余天数
	{
		if (surplus == -1) // 永久授权
		{
			// 显示永久授权按钮
			SetCtrlAttribute(panelHandle, PANEL_PBTN_FOREVER_AUTHOR, ATTR_VISIBLE, 1);

			// 隐藏暂未授权按钮
			SetCtrlAttribute(panelHandle, PANEL_PBTN_NOT_AUTHOR, ATTR_VISIBLE, 0);

			// 隐藏限时授权图片按钮
			SetCtrlAttribute(panelHandle, PANEL_PBTN_LIMIT_AUTHOR, ATTR_VISIBLE, 0);
		}
		else // 期限授权
		{
			// 隐藏永久授权按钮
			SetCtrlAttribute(panelHandle, PANEL_PBTN_FOREVER_AUTHOR, ATTR_VISIBLE, 0);

			// 隐藏暂未授权按钮
			SetCtrlAttribute(panelHandle, PANEL_PBTN_NOT_AUTHOR, ATTR_VISIBLE, 0);

			// 获取授权剩余时长
			sprintf(remainDay, "有效期%d天", surplus);
			// 更新限时授权按钮的标签
			SetCtrlAttribute(panelHandle, PANEL_PBTN_LIMIT_AUTHOR, ATTR_LABEL_TEXT, remainDay);
			// 显示限时授权图片按钮
			SetCtrlAttribute(panelHandle, PANEL_PBTN_LIMIT_AUTHOR, ATTR_VISIBLE, 1);
		}
		// 授权标志置1
		authorizeFlag = 1;

		// 用户完成授权，开始测试按钮可用
		SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_START_TEST, ATTR_DIMMED, 0);
		// 用户完成授权，仪器连接按钮可用
		SetCtrlAttribute(configPanelHandle, PANEL_CONF_PBTN_CONNECT, ATTR_DIMMED, 0);
	}
	return 0;
}

/* 系统初始化 */
void InitSys()
{
	InstrConfig config = {0}; 	// 上次仪器配置
	
	// 打开VISA资源管理器
	viOpenDefaultRM(&defaultRM);

	// 加载并显示配置面板
	LoadConfigPanel();

	// 加载并显示波形面板
	LoadWavePanel();

	// 打开数据库
	OpenSqliteDB("ns.db");

	// 获取工程路径
	GetDir(projectDir);
	
	// 加载配置
	LoadConfigFromIniFile(configPanelHandle, &config);
	
	// 设置连接方式
	int connectTypeIndex = 0;
	connectType = config.connectType;
	GetIndexFromValue(configPanelHandle, PANEL_CONF_RING_CONN_TYPE, &connectTypeIndex, connectType);
	if (connectTypeIndex < 0)
		connectTypeIndex = 0;
	SetCtrlIndex(configPanelHandle, PANEL_CONF_RING_CONN_TYPE, connectTypeIndex);
	CallCtrlCallback(configPanelHandle, PANEL_CONF_RING_CONN_TYPE, EVENT_COMMIT, 0, 0, 0);
	
	// 设置资源地址
	int srcAddrIndex = 0;
	strcpy(viDevName, config.srcAddr);
	GetIndexFromValue(configPanelHandle, PANEL_CONF_RING_SRC_ADDR, &srcAddrIndex, viDevName);
	if (srcAddrIndex < 0)
		srcAddrIndex = 0;
	SetCtrlIndex(configPanelHandle, PANEL_CONF_RING_SRC_ADDR, srcAddrIndex);
	CallCtrlCallback(configPanelHandle, PANEL_CONF_RING_SRC_ADDR, EVENT_COMMIT, 0, 0, 0);
}

/* 加载并显示授权界面 */
void LoadAuthorPanel()
{
	/* 电脑特征码 */
	char signatureCode[1024] = {0};

	// 判断能否加载面板
	if (loadAuthorizePanelFlag == 0)
	{
		// 授权面板加载标志置1
		loadAuthorizePanelFlag = 1;
		// 加载授权面板
		authorizePanelHandle = LoadPanel(panelHandle, "NS-Multimeter.uir", PANEL_AUTH);
	}

	// 获取电脑特征码
	GetSignature(signatureCode);
	// 显示特征码到 授权面板上的String控件
	SetCtrlVal(authorizePanelHandle, PANEL_AUTH_STRING_PC_SIGNATURE, signatureCode);
	// 设置面板位置
	SetPanelPos(authorizePanelHandle, 167, 368);
	// 显示面板
	DisplayPanel(authorizePanelHandle);
}

//====================================== 控件回调 ======================================//

/* 右上角的退出按钮 */
int CVICALLBACK PBtnQuitCallback(int panel, int control, int event,
								 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// 加载退出二次确认界面
			LoadQuitConfirmPanel();
			break;
	}
	return 0;
}

/* 右上角的最小化按钮 */
int CVICALLBACK PBtnMiniMize(int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// 最小化窗口
			SetPanelAttribute(panel, ATTR_WINDOW_ZOOM, VAL_MINIMIZE);
			break;
	}
	return 0;
}

/* 用户界面顶部条形幕布回调 */
int CVICALLBACK CanvasMoveCB(int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{

	switch (event)
	{
		case EVENT_LEFT_CLICK:
			// 拖动面板移动
			DragPanelMove(panel, control);
			break;

		default:
			break;
	}
	return 0;
}

/* 右上角的关于我们按钮 */
int CVICALLBACK PBtnAboutUsCB(int panel, int control, int event,
							  void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_LEFT_CLICK:
			// 加载关于我们面板
			LoadAboutUsPanel();
			break;

		default:
			break;
	}
	return 0;
}

/* 关于我们面板右上角的关闭按钮 */
int CVICALLBACK PBtnAboutUsQuitCB(int panel, int control, int event,
								  void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_LEFT_CLICK:
			// 注销面板
			DiscardPanel(aboutUsPanelHandle);

			// 标志置0
			loadAboutUsPanelFlag = 0;
			break;
	}
	return 0;
}

/* 右上角的用户帮助按钮 */
int CVICALLBACK PBtnUserHelpCB(int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_LEFT_CLICK:
			// 鼠标单击
			/*=====================================================================================================================*/
			break;
	}
	return 0;
}

/* 永久授权按钮 */
int CVICALLBACK PBtnForeverAuthorCB(int panel, int control, int event,
									void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_RIGHT_CLICK: // 用户鼠标右击
			// 右键 显示取消授权按钮
			SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_CANCEL_AUTHOR, ATTR_VISIBLE, 1);
			break;
	}
	return 0;
}

/* 用户界面顶部的暂未授权按钮 */
int CVICALLBACK PBtnAuthorizeCB(int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// 加载并显示授权界面
			LoadAuthorPanel();
			break;
	}
	return 0;
}

/* 授权页面的 关闭/退出 按钮 */
int CVICALLBACK PBtnAuthorizeQuitCB(int panel, int control, int event,
									void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// 注销面板
			DiscardPanel(authorizePanelHandle);
			// 授权面板加载标志置0
			loadAuthorizePanelFlag = 0;
			break;
	}
	return 0;
}

/* 授权页面的 确认按钮 */
int CVICALLBACK PBtnAuthorConfirmCB(int panel, int control, int event,
									void *callbackData, int eventData1, int eventData2)
{
	/* 用户输入的license */
	char license[1024] = {0};

	/* 消息 */
	char message[1024] = {0};

	/* 授权码验证结果（检测结果 0:通过 1:license无效 2:期限注册成功） */
	int result = -1;

	switch (event)
	{
		case EVENT_COMMIT:
			// 获取用户输入的授权码
			GetCtrlVal(authorizePanelHandle, PANEL_AUTH_STRING_AUTHOR_CODE, license);

			// 验证授权码
			result = CheckRegister(license, message);
			if (result == 0 || result == 2) // 注册成功
			{
				// 授权标志置1
				authorizeFlag = 1;

				// 关闭授权面板
				DiscardPanel(authorizePanelHandle);
				// 授权面板加载标志置0
				loadAuthorizePanelFlag = 0;

				// 用户完成授权，开始测试按钮可用
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_START_TEST, ATTR_DIMMED, 0);

				// 用户完成授权，仪器连接按钮可用
				SetCtrlAttribute(configPanelHandle, PANEL_CONF_PBTN_CONNECT, ATTR_DIMMED, 0);

				// 隐藏暂未授权按钮
				SetCtrlAttribute(panelHandle, PANEL_PBTN_NOT_AUTHOR, ATTR_VISIBLE, 0);
				// 判断 永久授权 还是 限期授权
				if (result == 0) // 永久
				{
					// 显示永久授权按钮
					SetCtrlAttribute(panelHandle, PANEL_PBTN_FOREVER_AUTHOR, ATTR_VISIBLE, 1);

					// 隐藏限时授权按钮
					SetCtrlAttribute(panelHandle, PANEL_PBTN_LIMIT_AUTHOR, ATTR_VISIBLE, 0);
				}
				else // 限期
				{
					// 隐藏永久授权按钮
					SetCtrlAttribute(panelHandle, PANEL_PBTN_FOREVER_AUTHOR, ATTR_VISIBLE, 0);

					// 显示限时授权按钮
					SetCtrlAttribute(panelHandle, PANEL_PBTN_LIMIT_AUTHOR, ATTR_VISIBLE, 1);

					// 显示显示时间
					IsAuthorize();
				}

				// 加载授权成功面板
				LoadAuthorOkPanel();
			}
			else // license无效
			{
				// 显示授权码错误
				SetCtrlAttribute(authorizePanelHandle, PANEL_AUTH_TEXTMSG_LIC_ERR, ATTR_TEXT_COLOR, 0xEB4747); // 更换文字颜色
				SetCtrlVal(authorizePanelHandle, PANEL_AUTH_TEXTMSG_LIC_ERR, "授权码错误！");
			}
			break;
	}
	return 0;
}

/* 授权成功后，加载授权成功面板，面板上有开始测试按钮 */
int CVICALLBACK PBtnStartTestAfterAuthorCB(int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// 点击开始测试按钮，注销授权成功面板
			DiscardPanel(authorizeOkPanelHandle);
			authorizeOkPanelHandle = 0;

			// 授权成功面板加载标志置0
			loadAuthorizeOkPanelFlag = 0;
			break;
	}
	return 0;
}

/* 取消授权按钮 */
int CVICALLBACK PBtnCancelAuthorCB(int panel, int control, int event,
								   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_LEFT_CLICK:
			// 加载取消授权确认面板面板
			LoadCancelAuthorPanel();
			// 隐藏 取消授权按钮
			SetCtrlAttribute(panel, PANEL_WAVE_PBTN_CANCEL_AUTHOR, ATTR_VISIBLE, 0);
			break;
	}
	return 0;
}

/* 取消授权面板 上的 取消按钮 */
int CVICALLBACK PBtnCancelAuthorCancelCB(int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_LEFT_CLICK:
			// 注销 取消授权面板
			DiscardPanel(cancelAuthorizePanelHandle);

			// 取消授权确认面板加载标志置1
			loadCancelAuthorizePanelFlag = 0;
			break;
	}
	return 0;
}

/* 取消授权面板 上的 确认按钮 */
int CVICALLBACK PBtnCancelAuthorOkCB(int panel, int control, int event,
									 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// 取消授权
			CancelLicense();

			// 授权标志置0
			authorizeFlag = 0;

			// 注销 取消授权面板
			DiscardPanel(cancelAuthorizePanelHandle);

			// 取消授权确认面板加载标志置1
			loadCancelAuthorizePanelFlag = 0;

			// 隐藏永久授权按钮
			SetCtrlAttribute(panelHandle, PANEL_PBTN_FOREVER_AUTHOR, ATTR_VISIBLE, 0);
			// 隐藏限时授权按钮
			SetCtrlAttribute(panelHandle, PANEL_PBTN_LIMIT_AUTHOR, ATTR_VISIBLE, 0);
			// 显示暂未授权按钮
			SetCtrlAttribute(panelHandle, PANEL_PBTN_NOT_AUTHOR, ATTR_VISIBLE, 1);
			
			// 开始测试、仪器连接和数据存储按钮不可用
			SetCtrlAttribute(configPanelHandle, PANEL_CONF_PBTN_CONNECT, ATTR_DIMMED, 1);
			SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_START_TEST, ATTR_DIMMED, 1);
			SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_SAVE_DATA, ATTR_DIMMED, 1);
			break;
	}
	return 0;
}

/* 主面板回调 */
int CVICALLBACK PanelCB(int panel, int event, void *callbackData,
						int eventData1, int eventData2)
{
	int btnX, btnY; // 按钮当前的位置
	int x, y;		// 鼠标 当前相对 取消授权按钮 的 位置
	int state;		// 取消授权按钮 当前的 状态（是否被隐藏）

	switch (event)
	{
		case EVENT_LEFT_CLICK: // 单击面板 取消授权按钮 以外的地方，隐藏 取消授权按钮
			// 获取 取消授权按钮 当前的 状态（是否被隐藏）
			GetCtrlAttribute(panel, PANEL_WAVE_PBTN_CANCEL_AUTHOR, ATTR_VISIBLE, &state);
			if (state) // 取消授权按钮 当前没有被隐藏
			{
				// 获取 鼠标 当前相对 取消授权按钮 的 位置
				GetRelativeMouseState(panel, PANEL_WAVE_PBTN_CANCEL_AUTHOR, &x, &y, NULL, NULL, NULL);

				// 获取按钮当前位置
				GetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_CANCEL_AUTHOR, ATTR_LEFT, &btnX);
				GetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_CANCEL_AUTHOR, ATTR_TOP, &btnY);

				// 鼠标 在 取消授权按钮 以外的地方
				if (x < btnX || x > 123 + btnX || y < btnY || y > 58 + btnY)
				{
					// 隐藏 取消授权按钮
					SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_CANCEL_AUTHOR, ATTR_VISIBLE, 0);
				}
			}
			break;
	}
	return 0;
}

/* 展开收起按钮 */
int CVICALLBACK PBtnUnfoldCB(int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	char tmpPath[1024] = {0};

	switch (event)
	{
		case EVENT_COMMIT:
			// 判断面板隐藏状态
			if (unfoldFlag) // 配置面板处于隐藏状态
			{
				// 显示 配置面板
				DisplayPanel(configPanelHandle);

				// 更换展开收起按钮的图片
				strcpy(tmpPath, projectDir);
				strcat(tmpPath, "\\Icon\\unfold_0.png");
				SetCtrlAttribute(panelHandle, PANEL_PBTN_UNFOLD, ATTR_IMAGE_FILE, tmpPath);
				
				// 移动展开收起按钮 的位置
				SetCtrlAttribute(panel, PANEL_PBTN_UNFOLD, ATTR_LEFT, 226);

				// 恢复波形面板大小
				SetPanelSize(wavePanelHandle, 619, 1018);

				// 恢复波形面板位置
				SetPanelPos(wavePanelHandle, 51, 252);

				// 恢复波形显示控件大小
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_GRAPH, ATTR_WIDTH, 1008);

				// 恢复 波形面板右上角的 开始测试 和 数据存储 按钮位置
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_START_TEST, ATTR_LEFT, 800);
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_SAVE_DATA, ATTR_LEFT, 910);

				// 配置面板展开
				unfoldFlag = 0;
			}
			else // 配置面板处于显示状态
			{
				// 隐藏 配置面板
				HidePanel(configPanelHandle);
				
				// 更换展开收起按钮的图片
				strcpy(tmpPath, projectDir);
				strcat(tmpPath, "\\Icon\\unfold_1.png");
				SetCtrlAttribute(panelHandle, PANEL_PBTN_UNFOLD, ATTR_IMAGE_FILE, tmpPath);

				// 移动展开收起按钮 的位置
				SetCtrlAttribute(panel, PANEL_PBTN_UNFOLD, ATTR_LEFT, 0);

				// 放大波形面板
				SetPanelSize(wavePanelHandle, 619, 1243);

				// 设置波形面板位置
				SetPanelPos(wavePanelHandle, 51, 27);

				// 放大波形显示控件
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_GRAPH, ATTR_WIDTH, 1230);

				// 设置 波形面板右上角的 开始测试 和 数据存储 按钮位置
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_START_TEST, ATTR_LEFT, 1025);
				SetCtrlAttribute(wavePanelHandle, PANEL_WAVE_PBTN_SAVE_DATA, ATTR_LEFT, 1135);

				// 配置面板收起
				unfoldFlag = 1;
			}
			break;
	}
	return 0;
}

// 退出时二次确认取消按钮
int CVICALLBACK PBtnQuitConfirmNoCB (int panel, int control, int event,
									 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			// 注销退出时二次确认界面
			quitConfirmPanelHandleFlag = 0;
			DiscardPanel(quitConfirmPanelHandle);
			break;
	}
	return 0;
}

// 退出时二次确认确认按钮
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
			// 如果仪器句柄没关闭，则关闭
			if (viDev)
				viClose(viDev);

			// 关闭VISA资源管理器
			viClose(defaultRM);
			
			// 关闭数据库
			CloseSqliteDB();
			
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
			
			// 保存配置数据到ini文件
			SaveConfigToIniFile(&config);

			// 注销退出时二次确认界面
			if (quitConfirmPanelHandleFlag)
				DiscardPanel(quitConfirmPanelHandle);
			
			// 注销授权面板
			if (authorizePanelHandle)
				DiscardPanel(authorizePanelHandle);
			
			// 注销左侧 配置面板
			if (configPanelHandleFlag)
				DiscardPanel(configPanelHandle);

			// 注销右侧 波形面板
			if (wavePanelHandleFlag)
				DiscardPanel(wavePanelHandle);

			// 注销连接失败面板
			if (connectFaildPanelHandleFlag)
				DiscardPanel(connectFaildPanelHandle);

			// 程序结束
			QuitUserInterface(0);
			break;
	}
	return 0;
}
