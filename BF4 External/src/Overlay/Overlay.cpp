#include "Overlay.hpp"
#include <iostream>
#include <utility>

Direct2DOverlay::Direct2DOverlay(HINSTANCE hInstance, DrawCallback callbacker)
{
	this->factory = NULL;
	this->solid_brush = NULL;
	this->target = NULL;
	this->w_factory = NULL;
	this->esp_font = NULL;
	this->menu_font = NULL;
	this->w_layout = NULL;
	this->good = false;
	this->loopRunning = false;
	this->drawing = false;
	this->overlayWindow = NULL;
	this->appInstance = hInstance;
	this->callback = callbacker;
}

Direct2DOverlay::~Direct2DOverlay()
{
	this->good = false;
	this->loopRunning = false;
	if (this->factory != NULL)
	{
		this->factory->Release();
		this->factory = NULL;
	}
	if (this->target != NULL)
	{
		this->target->Release();
		this->target = NULL;
	}
	if (this->solid_brush != NULL)
	{
		this->solid_brush->Release();
		this->solid_brush = NULL;
	}
	if (this->w_factory != NULL)
	{
		this->w_factory->Release();
		this->w_factory = NULL;
	}

	if (this->w_layout != NULL)
	{
		this->w_layout->Release();
		this->w_layout = NULL;
	}

	if (this->menu_font != NULL)
	{
		this->menu_font->Release();
		this->menu_font = NULL;
	}

	if (this->esp_font != NULL)
	{
		this->esp_font->Release();
		this->esp_font = NULL;
	}


}

bool Direct2DOverlay::Initialize(unsigned int process_id, std::string overlayWindowName)
{
	if (!this->good && !this->loopRunning)
	{
		if (!overlayWindowName.empty() && process_id != 0)
		{
			this->process_id = process_id;

			WNDCLASSEX wClass;
			ZeroMemory(&wClass, sizeof(wClass));
			wClass.cbSize = sizeof(wClass);
			wClass.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
			wClass.hCursor = LoadCursor(NULL, IDC_CROSS);
			wClass.lpszClassName = "pimpov";
			wClass.style = 0;
			wClass.lpfnWndProc = WindowProc;
			wClass.hInstance = this->appInstance;

			if (RegisterClassEx(&wClass))
			{
				RECT rekt = { 0,0,200,200 };
				AdjustWindowRectEx(&rekt, WS_POPUP, FALSE, WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_COMPOSITED);

				this->overlayWindow = CreateWindowEx(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST, "pimpov", overlayWindowName.c_str(), WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, rekt.right - rekt.left, rekt.bottom - rekt.top, NULL, NULL, this->appInstance, NULL);
				//this->overlayWindow = CreateWindowEx(WS_EX_COMPOSITED | WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_NOACTIVATE, CLASS_NAME, overlayWindowName.c_str(), WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, rekt.right - rekt.left, rekt.bottom - rekt.top, NULL, NULL, this->appInstance, NULL);
				if (this->overlayWindow != NULL)
				{
					MARGINS mar = { -1 };
					DwmExtendFrameIntoClientArea(this->overlayWindow, &mar);

					HRESULT res = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &this->factory);
					HRESULT res1 = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &this->factory);
					if (SUCCEEDED(res) && SUCCEEDED(res1))
					{
						res = this->factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)), D2D1::HwndRenderTargetProperties(this->overlayWindow, D2D1::SizeU(200, 200), D2D1_PRESENT_OPTIONS_IMMEDIATELY), &this->target);
						res1 = this->factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)), D2D1::HwndRenderTargetProperties(this->overlayWindow, D2D1::SizeU(200, 200), D2D1_PRESENT_OPTIONS_IMMEDIATELY), &this->target);
						if (SUCCEEDED(res) && SUCCEEDED(res1))
						{
							res = this->target->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f), &this->solid_brush);
							res1 = this->target->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f), &this->solid_brush);
							if (SUCCEEDED(res) && SUCCEEDED(res1))
							{
								res = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&this->w_factory));
								res1 = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&this->w_factory));
								if (SUCCEEDED(res) && SUCCEEDED(res1))
								{
									res = this->w_factory->CreateTextFormat(L"Roboto", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_SEMI_EXPANDED, 10.0f, L"en-us", &this->esp_font);
									res1 = this->w_factory->CreateTextFormat(L"Cousine", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_SEMI_EXPANDED, 10.0f, L"en-us", &this->menu_font);
									if (SUCCEEDED(res) && SUCCEEDED(res1))
									{
										this->good = true;

									}
								}
							}
						}
					}

				}
			}
		}
	}
	return this->good;
}

bool Direct2DOverlay::IsGood()
{
	return this->good;
}

HWND FindTopWindow(DWORD pid)
{
	std::pair<HWND, DWORD> params = { 0, pid };

	// Enumerate the windows using a lambda to process each window
	BOOL bResult = EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL
		{
			auto pParams = (std::pair<HWND, DWORD>*)(lParam);

			DWORD processId;
			if (GetWindowThreadProcessId(hwnd, &processId) && processId == pParams->second)
			{
				// Stop enumerating
				SetLastError(-1);
				pParams->first = hwnd;
				return FALSE;
			}

			// Continue enumerating
			return TRUE;
		}, (LPARAM)&params);

	if (!bResult && GetLastError() == -1 && params.first)
	{
		return params.first;
	}

	return 0;
}

void Direct2DOverlay::StartLoop()
{
	if (this->good && !this->loopRunning && this->callback)
	{
		this->loopRunning = true;

		MSG message;
		message.message = WM_NULL;

		ShowWindow(this->overlayWindow, 1);
		UpdateWindow(this->overlayWindow);
		//	SetLayeredWindowAttributes(this->overlayWindow, RGB(0, 0, 0), 255, LWA_COLORKEY);
		SetLayeredWindowAttributes(this->overlayWindow, RGB(0, 0, 0), 255, /*ULW_COLORKEY | */LWA_ALPHA);
		while (message.message != WM_QUIT)
		{
			if (PeekMessage(&message, this->overlayWindow, NULL, NULL, PM_REMOVE))
			{
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
			else
			{
				//HWND gamewindow = FindWindow(NULL, this->gameWindowName.c_str());
				HWND gamewindow = FindTopWindow(this->process_id);

				if (gamewindow != NULL)
				{
					WINDOWINFO info;
					ZeroMemory(&info, sizeof(info));
					info.cbSize = sizeof(info);
					GetWindowInfo(gamewindow, &info);

					D2D1_SIZE_U siz;
					siz.height = ((info.rcClient.bottom) - (info.rcClient.top));
					siz.width = ((info.rcClient.right) - (info.rcClient.left));

					if (!IsIconic(this->overlayWindow)) {
						SetWindowPos(this->overlayWindow, NULL, info.rcClient.left, info.rcClient.top, siz.width, siz.height, SWP_SHOWWINDOW);
						this->target->Resize(&siz);
					}
					HWND foreground = GetForegroundWindow();
					if (foreground == gamewindow)
					{
						this->callback(this);
					}
					else
					{
						this->BeginDraw();
						this->ClearScreen();
						this->EndDraw();
					}
				}
				else
				{
					this->BeginDraw();
					this->ClearScreen();
					this->EndDraw();
				}
			}
			Sleep(1);
		}
	}
}

void Direct2DOverlay::BeginDraw()
{
	if (this->good && !this->drawing)
	{
		this->drawing = true;
		this->target->BeginDraw();
	}
}

void Direct2DOverlay::ClearScreen()
{
	this->ClearScreen(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
}

void Direct2DOverlay::ClearScreen(D2D1::ColorF colour)
{
	if (this->good && this->drawing)
	{
		this->target->Clear(D2D1::ColorF(colour.r / 255, colour.g / 255, colour.b / 255, colour.a / 255));
	}
}


void Direct2DOverlay::DrawBox(D3DXVECTOR2 pos1 , D3DXVECTOR2 pos2, float thickness, D2D1::ColorF colour, bool filled)
{
	if (this->good && this->drawing)
	{
		this->solid_brush->SetColor(D2D1::ColorF(colour.r / 255, colour.g / 255, colour.b / 255, colour.a / 255));
		if (filled)
		{
			this->target->FillRectangle(D2D1::RectF(pos1.x, pos1.y, pos2.x, pos2.y), this->solid_brush);
		}
		else
		{
			this->target->DrawRectangle(D2D1::RectF(pos1.x, pos1.y, pos2.x, pos2.y), this->solid_brush, thickness);
		}

	}
}
void Direct2DOverlay::DrawBoxWithString(std::wstring str, bool screen_lock, D3DXVECTOR2 pos, float fontSize, D2D1::ColorF colour_of_text, int font_type, D2D1::ColorF colour_of_box)
{
	if (this->good && this->drawing)
	{
		RECT rect = this->GetOverlayRect();
		float dpix, dpiy;
		dpix = static_cast<float>(rect.right - rect.left);
		dpiy = static_cast<float>(rect.bottom - rect.top);
		HRESULT res;
		if (font_type == 0)//esp font
			res = this->w_factory->CreateTextLayout(str.c_str(), str.length() + 1, this->esp_font, dpix, dpiy, &this->w_layout);
		else if (font_type == 1) //menu font
			res = this->w_factory->CreateTextLayout(str.c_str(), str.length() + 1, this->menu_font, dpix, dpiy, &this->w_layout);

		if (SUCCEEDED(res))
		{
			DWRITE_TEXT_RANGE range = { 0, str.length() };
			this->w_layout->SetFontSize(fontSize, range);

			DWRITE_TEXT_METRICS TextInfo = {};
			this->w_layout->GetMetrics(&TextInfo);

			D3DXVECTOR2 text_data = D3DXVECTOR2(TextInfo.width / 2.0f, TextInfo.height / 2.0f);

			text_data = D3DXVECTOR2(TextInfo.width / 2.0f, TextInfo.height / 2.0f);

			this->solid_brush->SetColor(D2D1::ColorF(colour_of_box.r / 255, colour_of_box.g / 255, colour_of_box.b / 255, colour_of_box.a / 255));                    //set box colour
			this->target->FillRectangle(D2D1::RectF(pos.x - text_data.x - 2, pos.y - text_data.y, pos.x + text_data.x, pos.y + text_data.y), this->solid_brush);      //draw box

			this->solid_brush->SetColor(D2D1::ColorF(colour_of_text.r, colour_of_text.g, colour_of_text.b, colour_of_text.a));                                        //set text colour
			this->target->DrawTextLayout(D2D1::Point2F(pos.x - text_data.x, pos.y - text_data.y), this->w_layout, this->solid_brush, D2D1_DRAW_TEXT_OPTIONS_NO_SNAP); //draw text

			this->w_layout->Release();
			this->w_layout = NULL;
			TextInfo = {};
		}
	}
}

void Direct2DOverlay::DrawTab(std::wstring str, bool screen_lock, D3DXVECTOR2 pos, float fontSize, D2D1::ColorF colour_of_text, int font_type, D2D1::ColorF colour_of_box)
{
	if (this->good && this->drawing)
	{
		RECT rect = this->GetOverlayRect();
		float dpix, dpiy;
		dpix = static_cast<float>(rect.right - rect.left);
		dpiy = static_cast<float>(rect.bottom - rect.top);
		HRESULT res;
		if (font_type == 0)//esp font
			res = this->w_factory->CreateTextLayout(str.c_str(), str.length() + 1, this->esp_font, dpix, dpiy, &this->w_layout);
		else if (font_type == 1) //menu font
			res = this->w_factory->CreateTextLayout(str.c_str(), str.length() + 1, this->menu_font, dpix, dpiy, &this->w_layout);

		if (SUCCEEDED(res))
		{
			DWRITE_TEXT_RANGE range = { 0, str.length() };
			this->w_layout->SetFontSize(fontSize, range);

			DWRITE_TEXT_METRICS TextInfo = {};
			this->w_layout->GetMetrics(&TextInfo);

			D3DXVECTOR2 text_data = D3DXVECTOR2(TextInfo.width / 2.0f, TextInfo.height / 2.0f);

			text_data = D3DXVECTOR2(TextInfo.width / 2.0f, TextInfo.height / 2.0f);

			this->solid_brush->SetColor(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f));
			this->target->DrawRectangle(D2D1::RectF(pos.x, pos.y, pos.x + 25.0f, pos.y + 10.0f), this->solid_brush);
			this->solid_brush->SetColor(D2D1::ColorF(colour_of_box.r / 255, colour_of_box.g / 255, colour_of_box.b / 255, colour_of_box.a / 255));
			this->target->FillRectangle(D2D1::RectF(pos.x - text_data.x - 2, pos.y - text_data.y, pos.x + text_data.x, pos.y + text_data.y), this->solid_brush);             //draw box


			this->solid_brush->SetColor(D2D1::ColorF(colour_of_text.r, colour_of_text.g, colour_of_text.b, colour_of_text.a));                                        //set text colour
			this->target->DrawTextLayout(D2D1::Point2F(pos.x - text_data.x, pos.y - text_data.y), this->w_layout, this->solid_brush, D2D1_DRAW_TEXT_OPTIONS_NO_SNAP); //draw text

			this->w_layout->Release();
			this->w_layout = NULL;
			TextInfo = {};
		}
	}
}

void Direct2DOverlay::DrawCircle(D3DXVECTOR2 pos, float radius, float thicc, D2D1::ColorF colour, bool filled)
{
	if (this->good && this->drawing)
	{
		this->solid_brush->SetColor(D2D1::ColorF(colour.r / 255, colour.g / 255, colour.b / 255, colour.a / 255));
		if (filled)
		{
			this->target->FillEllipse(D2D1::Ellipse(D2D1::Point2F(pos.x, pos.y), radius, radius), this->solid_brush);
		}
		else
		{
			this->target->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(pos.x, pos.y), radius, radius), this->solid_brush, thicc);
		}
	}
}

void Direct2DOverlay::DrawLine(D3DXVECTOR2 pos1, D3DXVECTOR2 pos2 , float thicc, D2D1::ColorF colour)
{
	if (this->good && this->drawing)
	{
		this->solid_brush->SetColor(D2D1::ColorF(colour.r / 255, colour.g / 255, colour.b / 255, colour.a / 255));
		this->target->DrawLine(D2D1::Point2F(pos1.x, pos1.y), D2D1::Point2F(pos2.x, pos2.y), this->solid_brush, thicc);
	}
}

D3DXVECTOR2 Direct2DOverlay::DrawString(std::wstring str, float fontSize, D3DXVECTOR2 pos, font_type font_type, alignment alignment, D2D1::ColorF colour)
{

	if (this->good && this->drawing)
	{
		D3DXVECTOR2 string_return;
		RECT rect = this->GetOverlayRect();

		float dpix = static_cast<float>(rect.right - rect.left);
		float dpiy = static_cast<float>(rect.bottom - rect.top);

		HRESULT res;

		if (font_type == 0)//esp font
			res = this->w_factory->CreateTextLayout(str.c_str(), str.length() + 1, this->esp_font, dpix, dpiy, &this->w_layout);
		else if (font_type == 1) //menu font
			res = this->w_factory->CreateTextLayout(str.c_str(), str.length() + 1, this->menu_font, dpix, dpiy, &this->w_layout);

		if (SUCCEEDED(res))
		{
			DWRITE_TEXT_RANGE range = { 0, str.length() };
			this->w_layout->SetFontSize(fontSize, range);
			this->solid_brush->SetColor(D2D1::ColorF(colour.r / 255, colour.g / 255, colour.b / 255, colour.a / 255));

			DWRITE_TEXT_METRICS TextInfo = {};
			this->w_layout->GetMetrics(&TextInfo);
			string_return = D3DXVECTOR2(TextInfo.width, TextInfo.height);

			D3DXVECTOR2 text_data;

			if (alignment == FONT_LEFT)
				text_data = D3DXVECTOR2(TextInfo.width, TextInfo.height / 2);

			else if (alignment == FONT_RIGHT)
				text_data = D3DXVECTOR2(TextInfo.width / 4, TextInfo.height / 2);

			else if (alignment == FONT_CENTER)
				text_data = D3DXVECTOR2(TextInfo.width / 2.0f, TextInfo.height / 2);

			this->target->DrawTextLayout(D2D1::Point2F(pos.x - text_data.x, pos.y - text_data.y), this->w_layout, this->solid_brush, D2D1_DRAW_TEXT_OPTIONS_NO_SNAP);

			this->w_layout->Release();
			this->w_layout = NULL;
			TextInfo = {};
			return string_return;

		}
	}
}

D3DXVECTOR2 Direct2DOverlay::DrawCheckBoxString(std::wstring str, float fontSize, D3DXVECTOR2 pos, font_type font_type, alignment alignment, D2D1::ColorF colour)
{

	if (this->good && this->drawing)
	{
		D3DXVECTOR2 string_return;
		RECT rect = this->GetOverlayRect();

		float dpix = static_cast<float>(rect.right - rect.left);
		float dpiy = static_cast<float>(rect.bottom - rect.top);

		HRESULT res, selected;

		if (font_type == 0) //esp font
		{
			res = this->w_factory->CreateTextLayout(str.c_str(), str.length() + 1, this->esp_font, dpix, dpiy, &this->w_layout);
		}
		else if (font_type == 1) //menu font
		{
			res = this->w_factory->CreateTextLayout(str.c_str(), str.length() + 1, this->menu_font, dpix, dpiy, &this->w_layout);
		}


		if (SUCCEEDED(res))
		{
			DWRITE_TEXT_RANGE range = { 0, str.length() };
			this->w_layout->SetFontSize(fontSize, range);
			this->solid_brush->SetColor(D2D1::ColorF(colour.r / 255, colour.g / 255, colour.b / 255, colour.a / 255));

			DWRITE_TEXT_METRICS TextInfo = {};
			this->w_layout->GetMetrics(&TextInfo);
			string_return = D3DXVECTOR2(TextInfo.width, TextInfo.height);

			D3DXVECTOR2 text_data;

			if (alignment == FONT_LEFT)
				text_data = D3DXVECTOR2(TextInfo.width, TextInfo.height / 2);

			else if (alignment == FONT_RIGHT)
				text_data = D3DXVECTOR2(TextInfo.width / 4, TextInfo.height / 2);

			else if (alignment == FONT_CENTER)
				text_data = D3DXVECTOR2(TextInfo.width / 2.0f, TextInfo.height / 2);


			this->target->DrawTextLayout(D2D1::Point2F(pos.x + 9.0f - text_data.x + text_data.x + 4.0f, pos.y - text_data.y + 1), this->w_layout, this->solid_brush, D2D1_DRAW_TEXT_OPTIONS_NO_SNAP);

			this->w_layout->Release();
			this->w_layout = NULL;
			TextInfo = {};
			return string_return;

		}
	}
}
void Direct2DOverlay::DrawCrosshair(D2D1::ColorF colour, float thicc, float len, float xoffset, float yoffset)
{
	if (this->good && this->drawing)
	{
		RECT rect = this->GetOverlayRect();
		float width = static_cast<float>(rect.right - rect.left);
		float height = static_cast<float>(rect.bottom - rect.top);

		D3DXVECTOR2 pos(width, height);

		//this->DrawLine(D3DXVECTOR4((pos.x / 2.0f) - (len / 2.0f) + xoffset, (pos.y / 2.0f) + yoffset, (pos.x / 2.0f) + (len / 2.0f) + xoffset, (pos.x / 2.0f) + yoffset), thicc, colour);
		//this->DrawLine(D3DXVECTOR4((pos.x / 2.0f) + xoffset, (pos.y / 2.0f) - (len / 2.0f) + yoffset, (pos.x / 2.0f) + xoffset, (pos.x / 2.0f) + (len / 2.0f) + yoffset), thicc, colour);
	}
}

void Direct2DOverlay::EndDraw()
{
	if (this->good && this->drawing)
	{
		this->drawing = false;
		this->target->EndDraw();
	}
}

RECT Direct2DOverlay::GetOverlayRect()
{
	RECT rekt;
	GetClientRect(this->overlayWindow, &rekt);
	return rekt;
}

LRESULT CALLBACK Direct2DOverlay::WindowProc(HWND hWnd, UINT uiMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uiMessage)
	{
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, uiMessage, wParam, lParam);
	}
	return 0;
}