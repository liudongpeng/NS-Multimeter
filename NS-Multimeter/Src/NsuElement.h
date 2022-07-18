int __stdcall SetButtonState (int panel, int control, int state, char *target);
int __stdcall GetButtonState (int panel, int control, int *state);
int __stdcall DragPanelMove (int panel, int control);
int __stdcall CreatProgressBar (int panel, int control);
int __stdcall SetProgressBarAttibute (int panel, int control, int mode, double percent);
int __stdcall SetProgressBarColor (int panel, int control, unsigned int color);
int __stdcall CreatPanelScrollBar (int panel, int scrollControl, int backControl, int direction, int start, int area);
int __stdcall DragPanelScrollBar (int panel, int control);
int __stdcall FreshPanelScrollBar (int panel, int scrollControl);
int __stdcall DragTableScrollBar (int panel, int table, int control);
int __stdcall FreshTableScrollBar (int panel, int table, int scrollControl);
int __stdcall CreatTableScrollBar (int panel, int table, int scrollControl, int direction, int start, int area);
void __stdcall InsertTableRowsWithDefaultAttributte (int panel, int control, int start, int num, int hot);
void __stdcall CreatNsCalendarControl (int panel, int control, int language, char *format); 
void __stdcall InitNsCalendarControlVal (int panel, int control);
void __stdcall SetNsCalendarControlVal (int panel, int control, int year, int month, int day);
void __stdcall CreatNsPictureCommandButton (int panel, int control, char *picture, int height, int width, int dynamic);   
void __stdcall CreatNsPictureToggleButton (int panel, int control, char *picture, int height, int width);
void __stdcall CreatNsDateControl (int panel, int yearCtrl, int monthCtrl, int dayCtrl);
int __stdcall DragPanelScroll (int panel, int mode);
#define DEFAULTS_STATE 0//默认状态
#define SELECT_STATE 1 	//选择控件被选择状态  
#define FOCUS_STATE 2  	//鼠标移动到控件上时变色状态
#define DIMMED_STATE 3	//控件禁用状态
#define DIMMED_SELECT_STATE 4	//选择控件禁用状态

#define PROGRESS_OFF 0//静态进度条模式 
#define PROGRESS_ON 1//动态进度条模式  

#define HOR_SCROLL_BAR 0//水平进度条 
#define VERT_SCROLL_BAR 1//纵向进度条

#define HOR_PANEL_DRAG 0//只允许水平拖动 
#define VERT_PANEL_DRAG 1//只允许垂直拖动
#define BOTH_PANEL_DRAG 2//双方向拖动
