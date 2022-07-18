/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  PANEL                            1       /* callback function: PanelCB */
#define  PANEL_PBTN_UNFOLD                2       /* control type: pictButton, callback function: PBtnUnfoldCB */
#define  PANEL_CANVAS_DOWN                3       /* control type: canvas, callback function: (none) */
#define  PANEL_CANVAS_TOP                 4       /* control type: canvas, callback function: CanvasMoveCB */
#define  PANEL_PBTN_ABOUT_US              5       /* control type: pictButton, callback function: PBtnAboutUsCB */
#define  PANEL_PBTN_USER_HELP             6       /* control type: pictButton, callback function: PBtnUserHelpCB */
#define  PANEL_PBTN_MINIMIZE              7       /* control type: pictButton, callback function: PBtnMiniMize */
#define  PANEL_PBTN_QUIT                  8       /* control type: pictButton, callback function: PBtnQuitCallback */
#define  PANEL_PICTURE_ICON               9       /* control type: picture, callback function: (none) */
#define  PANEL_PICTURE_LINE               10      /* control type: picture, callback function: (none) */
#define  PANEL_PBTN_NOT_AUTHOR            11      /* control type: pictButton, callback function: PBtnAuthorizeCB */
#define  PANEL_PICTURE_AUTHOR             12      /* control type: picture, callback function: (none) */
#define  PANEL_PBTN_FOREVER_AUTHOR        13      /* control type: pictButton, callback function: PBtnForeverAuthorCB */
#define  PANEL_PBTN_LIMIT_AUTHOR          14      /* control type: pictButton, callback function: PBtnForeverAuthorCB */
#define  PANEL_TEXTMSG_TITLE              15      /* control type: textMsg, callback function: (none) */

#define  PANEL_ABUS                       2
#define  PANEL_ABUS_PBTN_ABOUTUS_QUIT     2       /* control type: pictButton, callback function: PBtnAboutUsQuitCB */
#define  PANEL_ABUS_PICTURE_ABOUTUS       3       /* control type: picture, callback function: (none) */

#define  PANEL_AUOK                       3
#define  PANEL_AUOK_PICTURE_AUTH_OK       2       /* control type: picture, callback function: (none) */
#define  PANEL_AUOK_PBT_TEST_AFTER_AUTHOR 3       /* control type: pictButton, callback function: PBtnStartTestAfterAuthorCB */
#define  PANEL_AUOK_TEXTMSG_AUTH_OK       4       /* control type: textMsg, callback function: (none) */

#define  PANEL_AUTH                       4
#define  PANEL_AUTH_PICTURE_QUTHOR_ICON   2       /* control type: picture, callback function: (none) */
#define  PANEL_AUTH_PICTURE_AUTHOR        3       /* control type: picture, callback function: (none) */
#define  PANEL_AUTH_TEXTMSG_AUTHOR_CODE   4       /* control type: textMsg, callback function: (none) */
#define  PANEL_AUTH_TEXTMSG_PC_SIGNATURE  5       /* control type: textMsg, callback function: (none) */
#define  PANEL_AUTH_PICTURE_AUTHOR_CODE   6       /* control type: picture, callback function: (none) */
#define  PANEL_AUTH_PICTURE_PC_SIGNATURE  7       /* control type: picture, callback function: (none) */
#define  PANEL_AUTH_STRING_AUTHOR_CODE    8       /* control type: string, callback function: (none) */
#define  PANEL_AUTH_STRING_PC_SIGNATURE   9       /* control type: string, callback function: (none) */
#define  PANEL_AUTH_PBT_AUTHOR_QUIT       10      /* control type: pictButton, callback function: PBtnAuthorizeQuitCB */
#define  PANEL_AUTH_PBTN_AUTHOR_CONFIRM   11      /* control type: pictButton, callback function: PBtnAuthorConfirmCB */
#define  PANEL_AUTH_PBTN_AUTHOR_CANCEL    12      /* control type: pictButton, callback function: PBtnAuthorizeQuitCB */
#define  PANEL_AUTH_TEXTMSG_LIC_ERR       13      /* control type: textMsg, callback function: (none) */

#define  PANEL_CERR                       5
#define  PANEL_CERR_PICTURE_WARNING       2       /* control type: picture, callback function: (none) */
#define  PANEL_CERR_TEXTMSG_TIP           3       /* control type: textMsg, callback function: (none) */
#define  PANEL_CERR_PBTN_IKNOW            4       /* control type: pictButton, callback function: PBtnIKnowCB */

#define  PANEL_CNAU                       6
#define  PANEL_CNAU_PICTURE_CNAU_TIP      2       /* control type: picture, callback function: (none) */
#define  PANEL_CNAU_PBTN_CNAU_CANCEL      3       /* control type: pictButton, callback function: PBtnCancelAuthorCancelCB */
#define  PANEL_CNAU_PBTN_CNAU_OK          4       /* control type: pictButton, callback function: PBtnCancelAuthorOkCB */
#define  PANEL_CNAU_TEXTMSG_CNAU_CONFIRM  5       /* control type: textMsg, callback function: (none) */

#define  PANEL_CONF                       7
#define  PANEL_CONF_TEXTMSG_CONN_TYPE     2       /* control type: textMsg, callback function: (none) */
#define  PANEL_CONF_TEXTMSG_MULTIMETER    3       /* control type: textMsg, callback function: (none) */
#define  PANEL_CONF_TEXTMSG_CONN_STATE    4       /* control type: textMsg, callback function: (none) */
#define  PANEL_CONF_TEXTMSG_SRC_ADDR      5       /* control type: textMsg, callback function: (none) */
#define  PANEL_CONF_TEXTMSG_BAUDRATE      6       /* control type: textMsg, callback function: (none) */
#define  PANEL_CONF_TEXTMSG_PAR_CONF      7       /* control type: textMsg, callback function: (none) */
#define  PANEL_CONF_TEXTMSG_TEST_ITEM     8       /* control type: textMsg, callback function: (none) */
#define  PANEL_CONF_TEXTMSG_RANGE         9       /* control type: textMsg, callback function: (none) */
#define  PANEL_CONF_TEXTMSG_SAMPLE_SPACE  10      /* control type: textMsg, callback function: (none) */
#define  PANEL_CONF_TEXTMSG_DATA_WIDTH    11      /* control type: textMsg, callback function: (none) */
#define  PANEL_CONF_TEXTMSG_SAMPLE_TIME   12      /* control type: textMsg, callback function: (none) */
#define  PANEL_CONF_PICTURE_186           13      /* control type: picture, callback function: (none) */
#define  PANEL_CONF_TEXTBOX_DATA_PATH     14      /* control type: textBox, callback function: (none) */
#define  PANEL_CONF_PBTN_CONNECT          15      /* control type: pictButton, callback function: PBtnConnectCB */
#define  PANEL_CONF_PBTN_DATA_SAVE_PATH   16      /* control type: pictButton, callback function: PBtnDataSavePathCB */
#define  PANEL_CONF_PICTURE_CONNECT       17      /* control type: picture, callback function: (none) */
#define  PANEL_CONF_RING_TEST_ITEM        18      /* control type: ring, callback function: RingTestItemCB */
#define  PANEL_CONF_PICTURE_TEST_ITEM     19      /* control type: picture, callback function: (none) */
#define  PANEL_CONF_PICTURE_113_CONN_TYPE 20      /* control type: picture, callback function: (none) */
#define  PANEL_CONF_PICTURE_TEST_RANGE    21      /* control type: picture, callback function: (none) */
#define  PANEL_CONF_PICTURE_113_BAUDRATE  22      /* control type: picture, callback function: (none) */
#define  PANEL_CONF_PICTURE_TEST_SPACE    23      /* control type: picture, callback function: (none) */
#define  PANEL_CONF_STRING_TIME_SPACE     24      /* control type: string, callback function: (none) */
#define  PANEL_CONF_RING_SRC_ADDR         25      /* control type: ring, callback function: RingSrcAddrCB */
#define  PANEL_CONF_STRING_TEST_TIME      26      /* control type: string, callback function: (none) */
#define  PANEL_CONF_RING_BAUDRATE         27      /* control type: ring, callback function: RingBaudRateCB */
#define  PANEL_CONF_PICTURE_TEST_TIME     28      /* control type: picture, callback function: (none) */
#define  PANEL_CONF_RING_CONN_TYPE        29      /* control type: ring, callback function: RingConnectTypeCB */
#define  PANEL_CONF_PICTURE_SRC_ADDR      30      /* control type: picture, callback function: (none) */
#define  PANEL_CONF_RING_DATA_WIDTH       31      /* control type: ring, callback function: RingDataWidthCB */
#define  PANEL_CONF_RING_TEST_RANGE       32      /* control type: ring, callback function: RingTestRangeCB */
#define  PANEL_CONF_PICTURE_DATA_WIDTH    33      /* control type: picture, callback function: (none) */
#define  PANEL_CONF_STRING_IP             34      /* control type: string, callback function: (none) */
#define  PANEL_CONF_PTBTN_AUTOSAVE        35      /* control type: pictButton, callback function: PTBtnAutoSaveCB */
#define  PANEL_CONF_PTBTN_USE_TESTTIME    36      /* control type: pictButton, callback function: PTBtnUseTestTimeCB */

#define  PANEL_QUCF                       8
#define  PANEL_QUCF_PICTURE_QUIT_TIP      2       /* control type: picture, callback function: (none) */
#define  PANEL_QUCF_PBTN_QUITCONFIRM_NO   3       /* control type: pictButton, callback function: PBtnQuitConfirmNoCB */
#define  PANEL_QUCF_PBTN_QUITCONFIRM_YES  4       /* control type: pictButton, callback function: PBtnQuitConfirmYesCB */
#define  PANEL_QUCF_TEXTMSG_QUIT_CONFIRM  5       /* control type: textMsg, callback function: (none) */

#define  PANEL_SPAG                       9
#define  PANEL_SPAG_PICTURE_STARTPAGE     2       /* control type: picture, callback function: (none) */

#define  PANEL_WAVE                       10      /* callback function: WavePanelCB */
#define  PANEL_WAVE_PBTN_SAVE_DATA        2       /* control type: pictButton, callback function: PBtnSaveData */
#define  PANEL_WAVE_PBTN_START_TEST       3       /* control type: pictButton, callback function: PBtnStartTest */
#define  PANEL_WAVE_PBTN_CANCEL_AUTHOR    4       /* control type: pictButton, callback function: PBtnCancelAuthorCB */
#define  PANEL_WAVE_PICTURE_VECTOR_0      5       /* control type: picture, callback function: (none) */
#define  PANEL_WAVE_TEXTMSG_WAVE_TIP      6       /* control type: textMsg, callback function: (none) */
#define  PANEL_WAVE_PBTN_RECOVER_IMG      7       /* control type: pictButton, callback function: PBtnRecoverImage */
#define  PANEL_WAVE_GRAPH                 8       /* control type: graph, callback function: GraphCB */
#define  PANEL_WAVE_TEXTMSG_CURSORS       9       /* control type: textMsg, callback function: (none) */


     /* Control Arrays: */

          /* (no control arrays in the resource file) */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK CanvasMoveCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GraphCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PanelCB(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PBtnAboutUsCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PBtnAboutUsQuitCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PBtnAuthorConfirmCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PBtnAuthorizeCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PBtnAuthorizeQuitCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PBtnCancelAuthorCancelCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PBtnCancelAuthorCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PBtnCancelAuthorOkCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PBtnConnectCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PBtnDataSavePathCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PBtnForeverAuthorCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PBtnIKnowCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PBtnMiniMize(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PBtnQuitCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PBtnQuitConfirmNoCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PBtnQuitConfirmYesCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PBtnRecoverImage(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PBtnSaveData(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PBtnStartTest(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PBtnStartTestAfterAuthorCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PBtnUnfoldCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PBtnUserHelpCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PTBtnAutoSaveCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PTBtnUseTestTimeCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK RingBaudRateCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK RingConnectTypeCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK RingDataWidthCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK RingSrcAddrCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK RingTestItemCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK RingTestRangeCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WavePanelCB(int panel, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif