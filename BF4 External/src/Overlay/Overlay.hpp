#pragma once
#include<windows.h>
#include<d2d1.h>
#include<dwrite.h>
#include<dwmapi.h>
#include<string>
#include<fstream>
#include <d3dx9math.h>


class Direct2DOverlay;

typedef void(*DrawCallback)(Direct2DOverlay* dis);

struct checkbox_t;

enum alignment
{
	FONT_LEFT = 0,
	FONT_CENTER = 1,
	FONT_RIGHT = 2
};

enum font_type
{
	ESP_FONT = 0,
	MENU_FONT = 1
};

class Direct2DOverlay
{
public:
	Direct2DOverlay(HINSTANCE hInstance, DrawCallback callbacker);
	~Direct2DOverlay();
	bool Initialize(unsigned int process_id, std::string overlayWindowName);
	bool Initialize(std::string gameWindowName, std::string overlayWindowName);
	bool IsGood();
	void StartLoop();
	void BeginDraw();
	void ClearScreen();
	void ClearScreen(D2D1::ColorF colour);
	void DrawBox(D3DXVECTOR2 pos1, D3DXVECTOR2 pos2, float thickness, D2D1::ColorF colour, bool filled);
	void DrawBoxWithString(std::wstring str, bool screen_lock, D3DXVECTOR2 pos, float fontSize, D2D1::ColorF colour_of_text, int font_type, D2D1::ColorF colour_of_box);
	void DrawTab(std::wstring str, bool screen_lock, D3DXVECTOR2 pos, float fontSize, D2D1::ColorF colour_of_text, int font_type, D2D1::ColorF colour_of_box);
	void DrawCircle(D3DXVECTOR2 pos, float radius, float thicc, D2D1::ColorF colour, bool filled);
	void DrawLine(D3DXVECTOR2 pos1, D3DXVECTOR2 pos2, float thicc, D2D1::ColorF colour);
	//void DrawLine(D3DXVECTOR4 pos, float thicc, D2D1::ColorF colour);
	D3DXVECTOR2 DrawString(std::wstring str, float fontSize, D3DXVECTOR2 pos, font_type font_type, alignment alignment, D2D1::ColorF colour);
	D3DXVECTOR2 DrawCheckBoxString(std::wstring str, float fontSize, D3DXVECTOR2 pos, font_type font_type, alignment alignment, D2D1::ColorF colour);
	void DrawCrosshair(D2D1::ColorF colour, float thicc, float len, float xoffset, float yoffset);
	void EndDraw();
	RECT GetOverlayRect();

private:
	ID2D1Factory* factory;
	ID2D1HwndRenderTarget* target;
	ID2D1SolidColorBrush* solid_brush;
	IDWriteFactory* w_factory;
	IDWriteTextFormat* esp_font;
	IDWriteTextFormat* menu_font;
	IDWriteTextLayout* w_layout;
	bool good;
	bool loopRunning;
	bool drawing;
	unsigned int process_id;
	HWND overlayWindow;
	HINSTANCE appInstance;
	DrawCallback callback;

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uiMessage, WPARAM wParam, LPARAM lParam);
};