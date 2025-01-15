/*
	Windows Unicode UTF-16 Dictionary
	Version 2.0

	A tool to translate text to numbers and numbers to text.

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "globldef.h"
#include "strdef.h"
#include "strdef.hpp"

#define MAINWND_WNDCLASS_NAME TEXT("__MAINWNDCLASS__")
#define MAINWND_CAPTION TEXT("UTF-16 Numeric Dictionary")

#define RUNTIME_STATUS_INIT 0
#define RUNTIME_STATUS_IDLE 1
#define RUNTIME_STATUS_MAIN 2
#define RUNTIME_STATUS_ENTERTEXT 3
#define RUNTIME_STATUS_ENTERNUM 4
#define RUNTIME_STATUS_RESULT 5

#define CUSTOMCOLOR_BLACK 0x00000000
#define CUSTOMCOLOR_WHITE 0x00ffffff
#define CUSTOMCOLOR_LTGRAY 0x00c0c0c0

#define CUSTOMBRUSHINDEX_TRANSPARENT 0U
#define CUSTOMBRUSHINDEX_SOLID_BLACK 1U
#define CUSTOMBRUSHINDEX_SOLID_WHITE 2U
#define CUSTOMBRUSHINDEX_SOLID_LTGRAY 3U

#define PP_CUSTOMBRUSH_LENGTH 4U
#define PP_CUSTOMBRUSH_SIZE (PP_CUSTOMBRUSH_LENGTH*sizeof(VOID*))

#define CUSTOMFONT_TITLE_CHARSET DEFAULT_CHARSET
#define CUSTOMFONT_TITLE_WIDTH 20
#define CUSTOMFONT_TITLE_HEIGHT 35
#define CUSTOMFONT_TITLE_WEIGHT FW_NORMAL

#define CUSTOMFONT_TEXT_CHARSET DEFAULT_CHARSET
#define CUSTOMFONT_TEXT_WIDTH 10
#define CUSTOMFONT_TEXT_HEIGHT 20
#define CUSTOMFONT_TEXT_WEIGHT FW_NORMAL

#define CUSTOMFONTINDEX_TITLE 0U
#define CUSTOMFONTINDEX_TEXT 1U

#define PP_CUSTOMFONT_LENGTH 2U
#define PP_CUSTOMFONT_SIZE (PP_CUSTOMFONT_LENGTH*sizeof(VOID*))

//GUI DEFINITION MACROS=================================================

#define MAINWND_BKCOLOR CUSTOMCOLOR_LTGRAY
#define MAINWND_BRUSHINDEX CUSTOMBRUSHINDEX_SOLID_LTGRAY

#define TEXTWND_TEXTCOLOR CUSTOMCOLOR_BLACK
#define TEXTWND_BKCOLOR MAINWND_BKCOLOR
#define TEXTWND_BRUSHINDEX CUSTOMBRUSHINDEX_TRANSPARENT

#define TEXTBOXWND_TEXTCOLOR CUSTOMCOLOR_BLACK
#define TEXTBOXWND_BKCOLOR CUSTOMCOLOR_WHITE
#define TEXTBOXWND_BRUSHINDEX CUSTOMBRUSHINDEX_SOLID_WHITE

//=====================================================================

INT runtime_status = -1;
INT prev_status = -1;

VOID **pp_custombrush = nullptr;
VOID **pp_customfont = nullptr;

HWND p_mainwnd = nullptr;
HWND p_text1 = nullptr;
HWND p_text2 = nullptr;
HWND p_textbox1 = nullptr;
HWND p_button1 = nullptr;
HWND p_button2 = nullptr;
HWND p_button3 = nullptr;

WORD mainwnd_wndclass_id = 0u;

extern BOOL WINAPI app_init(VOID);
extern VOID WINAPI app_deinit(VOID);

extern BOOL WINAPI custom_gdiobj_init(VOID);
extern VOID WINAPI custom_gdiobj_deinit(VOID);

extern BOOL WINAPI register_mainwnd_wndclass(VOID);
extern BOOL WINAPI create_mainwnd(VOID);
extern BOOL WINAPI create_ctrls(VOID);

extern INT WINAPI app_get_ref_status(VOID);

extern VOID WINAPI runtime_loop(VOID);

extern VOID WINAPI paintscreen_main(VOID);
extern VOID WINAPI paintscreen_input(VOID);
extern VOID WINAPI paintscreen_result(VOID);

extern VOID WINAPI text_align(VOID);
extern VOID WINAPI textbox_align(VOID);
extern VOID WINAPI button_align(VOID);
extern VOID WINAPI ctrls_setup(VOID);

extern BOOL WINAPI window_get_dimensions(HWND p_wnd, INT *p_xpos, INT *p_ypos, INT *p_width, INT *p_height, INT *p_centerx, INT *p_centery);
extern BOOL WINAPI translate_texttonum(BOOL dec);
extern BOOL WINAPI translate_hextotext(VOID);
extern BOOL WINAPI translate_dectotext(VOID);

extern BOOL WINAPI catch_messages(VOID);

extern LRESULT CALLBACK mainwnd_wndproc(HWND p_wnd, UINT msg, WPARAM wparam, LPARAM lparam);

extern LRESULT CALLBACK mainwnd_event_wmcommand(HWND p_wnd, WPARAM wparam, LPARAM lparam);
extern LRESULT CALLBACK mainwnd_event_wmpaint(HWND p_wnd, WPARAM wparam, LPARAM lparam);
extern LRESULT CALLBACK mainwnd_event_wmctlcolorstatic(HWND p_wnd, WPARAM wparam, LPARAM lparam);
extern LRESULT CALLBACK mainwnd_event_wmctlcoloredit(HWND p_wnd, WPARAM wparam, LPARAM lparam);
extern LRESULT CALLBACK mainwnd_event_wmsize(HWND p_wnd, WPARAM wparam, LPARAM lparam);
extern LRESULT CALLBACK mainwnd_event_wmdestroy(HWND p_wnd, WPARAM wparam, LPARAM lparam);

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
	p_instance = hInstance;

	if(!app_init())
	{
		app_deinit();
		return 1;
	}

	runtime_loop();

	app_deinit();
	return 0;
}

BOOL WINAPI app_init(VOID)
{
	if(p_instance == nullptr)
	{
		MessageBox(NULL, TEXT("Error: Invalid Instance."), TEXT("INIT ERROR"), (MB_ICONSTOP | MB_OK));
		return FALSE;
	}

	p_processheap = GetProcessHeap();
	if(p_processheap == nullptr)
	{
		MessageBox(NULL, TEXT("Error: Invalid Process Heap."), TEXT("INIT ERROR"), (MB_ICONSTOP | MB_OK));
		return FALSE;
	}

	if(!custom_gdiobj_init())
	{
		MessageBox(NULL, TEXT("Error: GDIOBJ Init Failed."), TEXT("INIT ERROR"), (MB_ICONSTOP | MB_OK));
		return FALSE;
	}

	if(!register_mainwnd_wndclass())
	{
		MessageBox(NULL, TEXT("Error: Register WNDCLASS Failed."), TEXT("INIT ERROR"), (MB_ICONSTOP | MB_OK));
		return FALSE;
	}

	if(!create_mainwnd())
	{
		MessageBox(NULL, TEXT("Error: Create MAINWND Failed."), TEXT("INIT ERROR"), (MB_ICONSTOP | MB_OK));
		return FALSE;
	}

	if(!create_ctrls())
	{
		MessageBox(NULL, TEXT("Error: Create CTRLS Failed."), TEXT("INIT ERROR"), (MB_ICONSTOP | MB_OK));
		return FALSE;
	}

	runtime_status = RUNTIME_STATUS_INIT;
	return TRUE;
}

VOID WINAPI app_deinit(VOID)
{
	if(p_mainwnd != nullptr) DestroyWindow(p_mainwnd);

	if(mainwnd_wndclass_id)
	{
		UnregisterClass(MAINWND_WNDCLASS_NAME, p_instance);
		mainwnd_wndclass_id = 0u;
	}

	custom_gdiobj_deinit();
	return;
}

BOOL WINAPI custom_gdiobj_init(VOID)
{
	SIZE_T n_obj = 0u;
	LOGFONT logfont;

	//Init custom brush

	pp_custombrush = (VOID**) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, PP_CUSTOMBRUSH_SIZE);
	if(pp_custombrush == nullptr) return FALSE;

	pp_custombrush[CUSTOMBRUSHINDEX_TRANSPARENT] = GetStockObject(HOLLOW_BRUSH);
	pp_custombrush[CUSTOMBRUSHINDEX_SOLID_BLACK] = CreateSolidBrush(CUSTOMCOLOR_BLACK);
	pp_custombrush[CUSTOMBRUSHINDEX_SOLID_WHITE] = CreateSolidBrush(CUSTOMCOLOR_WHITE);
	pp_custombrush[CUSTOMBRUSHINDEX_SOLID_LTGRAY] = CreateSolidBrush(CUSTOMCOLOR_LTGRAY);

	for(n_obj = 0u; n_obj < PP_CUSTOMBRUSH_LENGTH; n_obj++) if(pp_custombrush[n_obj] == nullptr) return FALSE;

	//Init custom font

	pp_customfont = (VOID**) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, PP_CUSTOMFONT_SIZE);
	if(pp_customfont == nullptr) return FALSE;

	ZeroMemory(&logfont, sizeof(LOGFONT));

	logfont.lfCharSet = CUSTOMFONT_TITLE_CHARSET;
	logfont.lfWidth = CUSTOMFONT_TITLE_WIDTH;
	logfont.lfHeight = CUSTOMFONT_TITLE_HEIGHT;
	logfont.lfWeight = CUSTOMFONT_TITLE_WEIGHT;

	pp_customfont[CUSTOMFONTINDEX_TITLE] = CreateFontIndirect(&logfont);

	logfont.lfCharSet = CUSTOMFONT_TEXT_CHARSET;
	logfont.lfWidth = CUSTOMFONT_TEXT_WIDTH;
	logfont.lfHeight = CUSTOMFONT_TEXT_HEIGHT;
	logfont.lfWeight = CUSTOMFONT_TEXT_WEIGHT;

	pp_customfont[CUSTOMFONTINDEX_TEXT] = CreateFontIndirect(&logfont);

	for(n_obj = 0u; n_obj < PP_CUSTOMFONT_LENGTH; n_obj++) if(pp_customfont[n_obj] == nullptr) return FALSE;

	return TRUE;
}

VOID WINAPI custom_gdiobj_deinit(VOID)
{
	SIZE_T n_obj = 0u;

	if(p_processheap == nullptr) return;

	if(pp_custombrush != nullptr)
	{
		for(n_obj = 0u; n_obj < PP_CUSTOMBRUSH_LENGTH; n_obj++)
		{
			if(pp_custombrush[n_obj] != nullptr)
			{
				DeleteObject(pp_custombrush[n_obj]);
				pp_custombrush[n_obj] = nullptr;
			}
		}

		HeapFree(p_processheap, 0, pp_custombrush);
		pp_custombrush = nullptr;
	}

	if(pp_customfont != nullptr)
	{
		for(n_obj = 0u; n_obj < PP_CUSTOMFONT_LENGTH; n_obj++)
		{
			if(pp_customfont[n_obj] != nullptr)
			{
				DeleteObject(pp_customfont[n_obj]);
				pp_customfont[n_obj] = nullptr;
			}
		}

		HeapFree(p_processheap, 0, pp_customfont);
		pp_customfont = nullptr;
	}

	return;
}

BOOL WINAPI register_mainwnd_wndclass(VOID)
{
	WNDCLASS wndclass;
	ZeroMemory(&wndclass, sizeof(WNDCLASS));

	wndclass.style = CS_OWNDC;
	wndclass.lpfnWndProc = &mainwnd_wndproc;
	wndclass.hInstance = p_instance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH) pp_custombrush[MAINWND_BRUSHINDEX];
	wndclass.lpszClassName = MAINWND_WNDCLASS_NAME;

	mainwnd_wndclass_id = RegisterClass(&wndclass);

	return (BOOL) mainwnd_wndclass_id;
}

BOOL WINAPI create_mainwnd(VOID)
{
	DWORD style = (WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX);

	p_mainwnd = CreateWindow(MAINWND_WNDCLASS_NAME, MAINWND_CAPTION, style, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, NULL, NULL, p_instance, NULL);

	return (p_mainwnd != nullptr);
}

BOOL WINAPI create_ctrls(VOID)
{
	DWORD style = 0u;

	style = (WS_CHILD | SS_CENTER);
	p_text1 = CreateWindow(TEXT("STATIC"), NULL, style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);
	p_text2 = CreateWindow(TEXT("STATIC"), NULL, style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);

	style = (WS_CHILD | WS_TABSTOP | WS_HSCROLL | ES_LEFT);
	p_textbox1 = CreateWindow(TEXT("EDIT"), NULL, style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);

	style = (WS_CHILD | WS_TABSTOP | BS_CENTER | BS_VCENTER | BS_TEXT | BS_PUSHBUTTON);
	p_button1 = CreateWindow(TEXT("BUTTON"), NULL, style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);
	p_button2 = CreateWindow(TEXT("BUTTON"), NULL, style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);
	p_button3 = CreateWindow(TEXT("BUTTON"), NULL, style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);

	if(p_text1 == nullptr) return FALSE;
	if(p_text2 == nullptr) return FALSE;
	if(p_textbox1 == nullptr) return FALSE;
	if(p_button1 == nullptr) return FALSE;
	if(p_button2 == nullptr) return FALSE;
	if(p_button3 == nullptr) return FALSE;

	SendMessage(p_text1, WM_SETFONT, (WPARAM) pp_customfont[CUSTOMFONTINDEX_TITLE], (LPARAM) TRUE);
	SendMessage(p_text2, WM_SETFONT, (WPARAM) pp_customfont[CUSTOMFONTINDEX_TEXT], (LPARAM) TRUE);
	SendMessage(p_textbox1, WM_SETFONT, (WPARAM) pp_customfont[CUSTOMFONTINDEX_TEXT], (LPARAM) TRUE);

	return TRUE;
}

INT WINAPI app_get_ref_status(VOID)
{
	if(runtime_status == RUNTIME_STATUS_IDLE) return prev_status;

	return runtime_status;
}

VOID WINAPI runtime_loop(VOID)
{
	while(catch_messages())
	{
		switch(runtime_status)
		{
			case RUNTIME_STATUS_IDLE:
				Sleep(10);
				break;

			case RUNTIME_STATUS_INIT:
				text_align();
				textbox_align();
				runtime_status = RUNTIME_STATUS_MAIN;

			case RUNTIME_STATUS_MAIN:
				paintscreen_main();
				break;

			case RUNTIME_STATUS_ENTERTEXT:
			case RUNTIME_STATUS_ENTERNUM:

				paintscreen_input();
				break;

			case RUNTIME_STATUS_RESULT:
				paintscreen_result();
				break;
		}

		if(runtime_status != RUNTIME_STATUS_IDLE)
		{
			prev_status = runtime_status;
			runtime_status = RUNTIME_STATUS_IDLE;
		}
	}

	return;
}

VOID WINAPI paintscreen_main(VOID)
{
	ShowWindow(p_text1, SW_HIDE);
	ShowWindow(p_text2, SW_HIDE);
	ShowWindow(p_textbox1, SW_HIDE);
	ShowWindow(p_button1, SW_HIDE);
	ShowWindow(p_button2, SW_HIDE);
	ShowWindow(p_button3, SW_HIDE);

	button_align();

	SendMessage(p_text1, WM_SETTEXT, 0, (LPARAM) TEXT("Choose Conversion Type"));
	SendMessage(p_button1, WM_SETTEXT, 0, (LPARAM) TEXT("Text To Numbers"));
	SendMessage(p_button2, WM_SETTEXT, 0, (LPARAM) TEXT("Numbers To Text"));

	ShowWindow(p_text1, SW_SHOW);
	ShowWindow(p_button1, SW_SHOW);
	ShowWindow(p_button2, SW_SHOW);
	return;
}

VOID WINAPI paintscreen_input(VOID)
{
	ShowWindow(p_text1, SW_HIDE);
	ShowWindow(p_text2, SW_HIDE);
	ShowWindow(p_textbox1, SW_HIDE);
	ShowWindow(p_button1, SW_HIDE);
	ShowWindow(p_button2, SW_HIDE);
	ShowWindow(p_button3, SW_HIDE);

	text_align();
	button_align();

	SendMessage(p_textbox1, WM_SETTEXT, 0, (LPARAM) TEXT(""));
	SendMessage(p_button1, WM_SETTEXT, 0, (LPARAM) TEXT("Return"));

	switch(runtime_status)
	{
		case RUNTIME_STATUS_ENTERTEXT:
			SendMessage(p_text1, WM_SETTEXT, 0, (LPARAM) TEXT("Enter Input Text"));
			SendMessage(p_button2, WM_SETTEXT, 0, (LPARAM) TEXT("Text To Hex"));
			SendMessage(p_button3, WM_SETTEXT, 0, (LPARAM) TEXT("Text To Dec"));
			break;

		case RUNTIME_STATUS_ENTERNUM:
			SendMessage(p_text1, WM_SETTEXT, 0, (LPARAM) TEXT("Enter Input Numbers"));
			SendMessage(p_text2, WM_SETTEXT, 0, (LPARAM) TEXT("Separate numbers using white spaces or symbols"));
			SendMessage(p_button2, WM_SETTEXT, 0, (LPARAM) TEXT("Hex To Text"));
			SendMessage(p_button3, WM_SETTEXT, 0, (LPARAM) TEXT("Dec To Text"));

			ShowWindow(p_text2, SW_SHOW);
			break;
	}

	ShowWindow(p_text1, SW_SHOW);
	ShowWindow(p_textbox1, SW_SHOW);
	ShowWindow(p_button1, SW_SHOW);
	ShowWindow(p_button2, SW_SHOW);
	ShowWindow(p_button3, SW_SHOW);

	return;
}

VOID WINAPI paintscreen_result(VOID)
{
	ShowWindow(p_text1, SW_HIDE);
	ShowWindow(p_text2, SW_HIDE);
	ShowWindow(p_textbox1, SW_HIDE);
	ShowWindow(p_button1, SW_HIDE);
	ShowWindow(p_button2, SW_HIDE);
	ShowWindow(p_button3, SW_HIDE);

	text_align();
	button_align();

	SendMessage(p_text1, WM_SETTEXT, 0, (LPARAM) TEXT("Result:"));
	SendMessage(p_text2, WM_SETTEXT, 0, (LPARAM) textbuf);
	SendMessage(p_button1, WM_SETTEXT, 0, (LPARAM) TEXT("Return"));

	ShowWindow(p_text1, SW_SHOW);
	ShowWindow(p_text2, SW_SHOW);
	ShowWindow(p_button1, SW_SHOW);
	return;
}

VOID WINAPI text_align(VOID)
{
	const INT TEXT_MARGIN_LEFT = 40;

	INT mainwnd_width = 0;

	INT text2_ypos = 0;

	INT text_width = 0;
	INT text2_height = 0;

	INT ref_status = -1;

	ref_status = app_get_ref_status();
	window_get_dimensions(p_mainwnd, nullptr, nullptr, &mainwnd_width, nullptr, nullptr, nullptr);

	text_width = mainwnd_width - 2*TEXT_MARGIN_LEFT;

	switch(ref_status)
	{
		case RUNTIME_STATUS_ENTERTEXT:
		case RUNTIME_STATUS_ENTERNUM:

			text2_ypos = 60;
			text2_height = CUSTOMFONT_TEXT_HEIGHT;
			break;

		case RUNTIME_STATUS_RESULT:
			text2_ypos = 100;
			text2_height = 8*CUSTOMFONT_TEXT_HEIGHT;
			break;
	}

	SetWindowPos(p_text1, NULL, TEXT_MARGIN_LEFT, 20, text_width, CUSTOMFONT_TITLE_HEIGHT, 0);
	SetWindowPos(p_text2, NULL, TEXT_MARGIN_LEFT, text2_ypos, text_width, text2_height, 0);
	return;
}

VOID WINAPI textbox_align(VOID)
{
	const INT TEXTBOX_MARGIN_LEFT = 40;
	const INT TEXTBOX_MARGIN_TOP = 100;
	const INT TEXTBOX_HEIGHT = 8*CUSTOMFONT_TEXT_HEIGHT;

	INT mainwnd_width = 0;

	INT textbox_width = 0;

	window_get_dimensions(p_mainwnd, nullptr, nullptr, &mainwnd_width, nullptr, nullptr, nullptr);

	textbox_width = mainwnd_width - 2*TEXTBOX_MARGIN_LEFT;

	SetWindowPos(p_textbox1, NULL, TEXTBOX_MARGIN_LEFT, TEXTBOX_MARGIN_TOP, textbox_width, TEXTBOX_HEIGHT, 0);
	return;
}

VOID WINAPI button_align(VOID)
{
	const INT BUTTON_WIDTH = 120;
	const INT BUTTON_HEIGHT = 20;

	INT ref_status = -1;
	INT mainwnd_height = 0;
	INT mainwnd_centerx = 0;

	INT button1_xpos = 0;
	INT button2_xpos = 0;
	INT button3_xpos = 0;

	INT button_ypos = 0;

	ref_status = app_get_ref_status();
	window_get_dimensions(p_mainwnd, nullptr, nullptr, nullptr, &mainwnd_height, &mainwnd_centerx, nullptr);

	button_ypos = mainwnd_height - BUTTON_HEIGHT - 60;

	switch(ref_status)
	{
		case RUNTIME_STATUS_MAIN:
			button1_xpos = mainwnd_centerx - BUTTON_WIDTH - 10;
			button2_xpos = mainwnd_centerx + 10;
			break;

		case RUNTIME_STATUS_ENTERTEXT:
		case RUNTIME_STATUS_ENTERNUM:

			button2_xpos = mainwnd_centerx - BUTTON_WIDTH/2;

			button1_xpos = button2_xpos - BUTTON_WIDTH - 20;
			button3_xpos = mainwnd_centerx + BUTTON_WIDTH/2 + 20;
			break;

		case RUNTIME_STATUS_RESULT:
			button1_xpos = mainwnd_centerx - BUTTON_WIDTH/2;
			break;
	}

	SetWindowPos(p_button1, NULL, button1_xpos, button_ypos, BUTTON_WIDTH, BUTTON_HEIGHT, 0);
	SetWindowPos(p_button2, NULL, button2_xpos, button_ypos, BUTTON_WIDTH, BUTTON_HEIGHT, 0);
	SetWindowPos(p_button3, NULL, button3_xpos, button_ypos, BUTTON_WIDTH, BUTTON_HEIGHT, 0);
	return;
}

VOID WINAPI ctrls_setup(VOID)
{
	text_align();
	textbox_align();
	button_align();
	return;
}

BOOL WINAPI window_get_dimensions(HWND p_wnd, INT *p_xpos, INT *p_ypos, INT *p_width, INT *p_height, INT *p_centerx, INT *p_centery)
{
	RECT rect;

	if(p_wnd == nullptr) return FALSE;
	if(!GetWindowRect(p_wnd, &rect)) return FALSE;

	if(p_xpos != nullptr) *p_xpos = rect.left;
	if(p_ypos != nullptr) *p_ypos = rect.top;
	if(p_width != nullptr) *p_width = rect.right - rect.left;
	if(p_height != nullptr) *p_height = rect.bottom - rect.top;
	if(p_centerx != nullptr) *p_centerx = (rect.right - rect.left)/2;
	if(p_centery != nullptr) *p_centery = (rect.bottom - rect.top)/2;

	return TRUE;
}

BOOL WINAPI translate_texttonum(BOOL dec)
{
	SIZE_T input_len = 0u;
	SIZE_T n_char = 0u;
	__string input = TEXT("");
	__string output = TEXT("");

	SendMessage(p_textbox1, WM_GETTEXT, (WPARAM) TEXTBUF_SIZE_CHARS, (LPARAM) textbuf);
	textbuf[TEXTBUF_SIZE_CHARS - 1u] = (TCHAR) '\0';

	input_len = (SIZE_T) cstr_getlength(textbuf);
	input = textbuf;

	if(dec) output = TEXT("DEC OUT: ");
	else output = TEXT("HEX OUT: ");

	for(n_char = 0u; n_char < input_len; n_char++)
	{
		if(dec) __SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("%u"), (UINT) input[n_char]);
		else __SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("%x"), (UINT) input[n_char]);

		output += textbuf;
		output += TEXT(", ");
	}

	output += TEXT("NULL");

	cstr_copy(output.c_str(), textbuf, TEXTBUF_SIZE_CHARS);
	return TRUE;
}

BOOL WINAPI translate_hextotext(VOID)
{
	SIZE_T input_len = 0u;
	SIZE_T n_char = 0u;
	INT ch = 0;
	__string strnum = TEXT("");
	__string output = TEXT("TEXT OUT: ");

	SendMessage(p_textbox1, WM_GETTEXT, (WPARAM) TEXTBUF_SIZE_CHARS, (LPARAM) textbuf);
	textbuf[TEXTBUF_SIZE_CHARS - 1u] = (TCHAR) '\0';

	input_len = (SIZE_T) cstr_getlength(textbuf);
	if(input_len < 1u) return FALSE;

	for(n_char = 0u; n_char < input_len; n_char++)
	{
		if(std::isdigit(textbuf[n_char]) || std::isalpha(textbuf[n_char])) strnum += textbuf[n_char];
		else if(strnum.length() > 0u)
		{
			try
			{
				ch = std::stoi(strnum, nullptr, 16);
			}
			catch(...)
			{
				return FALSE;
			}

			output += (TCHAR) ch;
			strnum = TEXT("");
		}
	}

	if(strnum.length() > 0u)
	{
		try
		{
			ch = std::stoi(strnum, nullptr, 16);
		}
		catch(...)
		{
			return FALSE;
		}

		output += (TCHAR) ch;
		strnum = TEXT("");
	}

	cstr_copy(output.c_str(), textbuf, TEXTBUF_SIZE_CHARS);
	return TRUE;
}

BOOL WINAPI translate_dectotext(VOID)
{
	SIZE_T input_len = 0u;
	SIZE_T n_char = 0u;
	INT ch = 0;
	__string strnum = TEXT("");
	__string output = TEXT("TEXT OUT: ");

	SendMessage(p_textbox1, WM_GETTEXT, (WPARAM) TEXTBUF_SIZE_CHARS, (LPARAM) textbuf);
	textbuf[TEXTBUF_SIZE_CHARS - 1u] = (TCHAR) '\0';

	input_len = (SIZE_T) cstr_getlength(textbuf);
	if(input_len < 1u) return FALSE;

	for(n_char = 0u; n_char < input_len; n_char++)
	{
		if(std::isdigit(textbuf[n_char])) strnum += textbuf[n_char];
		else if(strnum.length() > 0u)
		{
			try
			{
				ch = std::stoi(strnum);
			}
			catch(...)
			{
				return FALSE;
			}

			output += (TCHAR) ch;
			strnum = TEXT("");
		}
	}

	if(strnum.length() > 0u)
	{
		try
		{
			ch = std::stoi(strnum);
		}
		catch(...)
		{
			return FALSE;
		}

		output += (TCHAR) ch;
		strnum = TEXT("");
	}

	cstr_copy(output.c_str(), textbuf, TEXTBUF_SIZE_CHARS);
	return TRUE;
}

BOOL WINAPI catch_messages(VOID)
{
	MSG msg;

	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if(msg.message == WM_QUIT) return FALSE;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return TRUE;
}

LRESULT CALLBACK mainwnd_wndproc(HWND p_wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg)
	{
		case WM_COMMAND:
			return mainwnd_event_wmcommand(p_wnd, wparam, lparam);

		case WM_PAINT:
			return mainwnd_event_wmpaint(p_wnd, wparam, lparam);

		case WM_CTLCOLORSTATIC:
			return mainwnd_event_wmctlcolorstatic(p_wnd, wparam, lparam);

		case WM_CTLCOLOREDIT:
			return mainwnd_event_wmctlcoloredit(p_wnd, wparam, lparam);

		case WM_SIZE:
			return mainwnd_event_wmsize(p_wnd, wparam, lparam);

		case WM_DESTROY:
			return mainwnd_event_wmdestroy(p_wnd, wparam, lparam);
	}

	return DefWindowProc(p_wnd, msg, wparam, lparam);
}

LRESULT CALLBACK mainwnd_event_wmcommand(HWND p_wnd, WPARAM wparam, LPARAM lparam)
{
	if(p_wnd == nullptr) return 0;
	if(lparam == 0) return 0;

	if(((ULONG_PTR) lparam) == ((ULONG_PTR) p_button1))
	{
		switch(prev_status)
		{
			case RUNTIME_STATUS_MAIN:
				runtime_status = RUNTIME_STATUS_ENTERTEXT;
				break;

			case RUNTIME_STATUS_ENTERTEXT:
			case RUNTIME_STATUS_ENTERNUM:
			case RUNTIME_STATUS_RESULT:

				runtime_status = RUNTIME_STATUS_MAIN;
				break;
		}
	}
	else if(((ULONG_PTR) lparam) == ((ULONG_PTR) p_button2))
	{
		switch(prev_status)
		{
			case RUNTIME_STATUS_MAIN:
				runtime_status = RUNTIME_STATUS_ENTERNUM;
				break;

			case RUNTIME_STATUS_ENTERTEXT:
				if(translate_texttonum(FALSE)) runtime_status = RUNTIME_STATUS_RESULT;
				else
				{
					MessageBox(NULL, TEXT("Something Went Wrong.\r\nMake sure to enter valid UTF-16 characters."), TEXT("Conversion Error"), (MB_ICONEXCLAMATION | MB_OK));
					runtime_status = RUNTIME_STATUS_ENTERTEXT;
				}
				break;

			case RUNTIME_STATUS_ENTERNUM:
				if(translate_hextotext()) runtime_status = RUNTIME_STATUS_RESULT;
				else
				{
					MessageBox(NULL, TEXT("Something Went Wrong.\r\nMake sure to enter valid hexadecimal values."), TEXT("Conversion Error"), (MB_ICONEXCLAMATION | MB_OK));
					runtime_status = RUNTIME_STATUS_ENTERNUM;
				}
				break;
		}
	}
	else if(((ULONG_PTR) lparam) == ((ULONG_PTR) p_button3))
	{
		switch(prev_status)
		{
			case RUNTIME_STATUS_ENTERTEXT:
				if(translate_texttonum(TRUE)) runtime_status = RUNTIME_STATUS_RESULT;
				else
				{
					MessageBox(NULL, TEXT("Something Went Wrong.\r\nMake sure to enter valid UTF-16 characters."), TEXT("Conversion Error"), (MB_ICONEXCLAMATION | MB_OK));
					runtime_status = RUNTIME_STATUS_ENTERTEXT;
				}
				break;

			case RUNTIME_STATUS_ENTERNUM:
				if(translate_dectotext()) runtime_status = RUNTIME_STATUS_RESULT;
				else
				{
					MessageBox(NULL, TEXT("Something Went Wrong.\r\nMake sure to enter valid integer values."), TEXT("Conversion Error"), (MB_ICONEXCLAMATION | MB_OK));
					runtime_status = RUNTIME_STATUS_ENTERNUM;
				}
				break;
		}
	}

	return 0;
}

LRESULT CALLBACK mainwnd_event_wmpaint(HWND p_wnd, WPARAM wparam, LPARAM lparam)
{
	HDC p_wnddc = nullptr;
	PAINTSTRUCT paintstruct;

	if(p_wnd == nullptr) return 0;

	p_wnddc = BeginPaint(p_wnd, &paintstruct);
	if(p_wnddc == nullptr) return 0;

	FillRect(paintstruct.hdc, &paintstruct.rcPaint, (HBRUSH) pp_custombrush[MAINWND_BRUSHINDEX]);
	EndPaint(p_wnd, &paintstruct);

	return 0;
}

LRESULT CALLBACK mainwnd_event_wmctlcolorstatic(HWND p_wnd, WPARAM wparam, LPARAM lparam)
{
	if(p_wnd == nullptr) return 0;
	if(wparam == 0) return 0;
	if(lparam == 0) return 0;

	SetTextColor((HDC) wparam, TEXTWND_TEXTCOLOR);
	SetBkColor((HDC) wparam, TEXTWND_BKCOLOR);

	return (LRESULT) pp_custombrush[TEXTWND_BRUSHINDEX];
}

LRESULT CALLBACK mainwnd_event_wmctlcoloredit(HWND p_wnd, WPARAM wparam, LPARAM lparam)
{
	if(p_wnd == nullptr) return 0;
	if(wparam == 0) return 0;
	if(lparam == 0) return 0;

	SetTextColor((HDC) wparam, TEXTBOXWND_TEXTCOLOR);
	SetBkColor((HDC) wparam, TEXTBOXWND_BKCOLOR);

	return (LRESULT) pp_custombrush[TEXTBOXWND_BRUSHINDEX];
}

LRESULT CALLBACK mainwnd_event_wmsize(HWND p_wnd, WPARAM wparam, LPARAM lparam)
{
	if(p_wnd == nullptr) return 0;

	ctrls_setup();
	RedrawWindow(p_wnd, NULL, NULL, (RDW_ERASE | RDW_FRAME | RDW_INTERNALPAINT | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN));
	return 0;
}

LRESULT CALLBACK mainwnd_event_wmdestroy(HWND p_wnd, WPARAM wparam, LPARAM lparam)
{
	p_mainwnd = nullptr;
	p_text1 = nullptr;
	p_text2 = nullptr;
	p_textbox1 = nullptr;
	p_button1 = nullptr;
	p_button2 = nullptr;
	p_button3 = nullptr;

	PostQuitMessage(0);
	return 0;
}
