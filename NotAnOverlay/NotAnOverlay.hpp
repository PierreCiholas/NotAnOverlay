#pragma once
#include <Windows.h>
#include <string>
// For screen capture
#include <ole2.h>
#include <olectl.h>
#include "PerfMgr.hpp"

#define DEFAULT_WNDCLASSNAME L"Control"
#define DEFAULT_WNDTITLE L""

class NotAnOverlay {
public:
	NotAnOverlay();
	~NotAnOverlay();

	// Start-up (Required to have a thread on a private method)
	static void NotAnOverlay::Start(void* notAnOverlayInstance);
	HANDLE NotAnOverlay::StartThread();

	// Window management
	static void TreatWindowMessageQueue();
	static LRESULT CALLBACK NotAnOverlay::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	// Getters
	HWND NotAnOverlay::GetWindow();
	HANDLE NotAnOverlay::GetThread();

private:
	void NotAnOverlay::InitOverlay();
	void NotAnOverlay::CloneArea(int x, int y, int w, int h);
	// Window management
	HWND NotAnOverlay::SpawnOverlayWindow(std::wstring windowClassName = DEFAULT_WNDCLASSNAME, std::wstring windowTitle = DEFAULT_WNDTITLE);
	bool NotAnOverlay::RegisterWindowClass(std::wstring windowClassName = DEFAULT_WNDCLASSNAME);
	bool NotAnOverlay::GetScreenResolution();
	// Screen capture
	void NotAnOverlay::CaptureScreenArea(int x, int y, int w, int h, HBITMAP* hBitmap);
	bool NotAnOverlay::SaveBitmap(LPCSTR filename, HBITMAP bmp, HPALETTE pal = NULL);

protected:
	HANDLE m_hThreadNotAnOverlay = NULL;
	HWND m_hwndNotOverlay = NULL;
	POINT m_screenResolution;
	RECT m_windowSize;

	PAINTSTRUCT m_paintStruct;

	HDC m_hdcSource = NULL;
	HDC m_hdcDestination = NULL;
	HDC m_hdcSrcTemp = NULL;
	HDC m_hdcDstTemp = NULL;

	HBITMAP m_hBitmapSource = NULL;
	HBITMAP m_hBitmapTemp = NULL;

	PerfManager perfs;
};

NotAnOverlay::NotAnOverlay() {
	m_hdcSource = GetDC(NULL); // TODO: Use HWND to capture specific window
	m_hdcSrcTemp = CreateCompatibleDC(m_hdcSource);
	Start(this);
}

NotAnOverlay::~NotAnOverlay() {
	// TODO: on destruction, make the thread terminate properly, join, then destroy.
	DeleteObject(m_hBitmapSource);
	DeleteObject(m_hBitmapTemp);
	DeleteDC(m_hdcDestination);
	DeleteDC(m_hdcDstTemp);
	DeleteDC(m_hdcSrcTemp);
	ReleaseDC(NULL, m_hdcSource);
}

// Getters
HWND NotAnOverlay::GetWindow() { return m_hwndNotOverlay; }
HANDLE NotAnOverlay::GetThread() { return m_hThreadNotAnOverlay; }

void NotAnOverlay::Start(void* notAnOverlayInstance) {
	return ((NotAnOverlay*)notAnOverlayInstance)->InitOverlay();
}

HANDLE NotAnOverlay::StartThread() {
	m_hThreadNotAnOverlay = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Start, this, NULL, NULL);
}

void NotAnOverlay::CloneArea(int x, int y, int w, int h) {
	NotAnOverlay::CaptureScreenArea(x, y, w, h, &m_hBitmapSource);

	InvalidateRect(m_hwndNotOverlay, NULL, TRUE); // Tells that the window should be repainted
	m_hdcDestination = BeginPaint(m_hwndNotOverlay, &m_paintStruct);
	m_hdcDstTemp = CreateCompatibleDC(m_hdcDestination);
	m_hBitmapTemp = (HBITMAP)SelectObject(m_hdcDstTemp, m_hBitmapSource);
	
	// No resizing (ideal when same resolution)
	//BitBlt(hdc, 0, 0, w, h, hdcMem, 0, 0, SRCCOPY);

	// Resizing (ideal when resolutions are different)
	GetClientRect(m_hwndNotOverlay, &m_windowSize); // TODO: Move to WindowProc and refresh only on msg WM_SIZE (require WindowProc to be method)
	SetStretchBltMode(m_hdcDestination, COLORONCOLOR); // Use HALFTONE for better image quality (but slower)
	StretchBlt(m_hdcDestination, 0, 0, m_windowSize.right, m_windowSize.bottom, m_hdcDstTemp, x, y, w, h, SRCCOPY);
	EndPaint(m_hwndNotOverlay, &m_paintStruct);

	UpdateWindow(m_hwndNotOverlay); // Force direct repaint window
}

void NotAnOverlay::CaptureScreenArea(int x, int y, int w, int h, HBITMAP* hBitmap) {
	*hBitmap = CreateCompatibleBitmap(m_hdcSource, w, h);
	m_hBitmapTemp = (HBITMAP)SelectObject(m_hdcSrcTemp, *hBitmap);

	BitBlt(m_hdcSrcTemp, 0, 0, w, h, m_hdcSource, x, y, SRCCOPY);
	*hBitmap = (HBITMAP)SelectObject(m_hdcSrcTemp, m_hBitmapTemp);
}

void NotAnOverlay::InitOverlay() {
	NotAnOverlay::GetScreenResolution();
	m_hwndNotOverlay = SpawnOverlayWindow();

	while (true) {
		perfs.AddTick();

		NotAnOverlay::CloneArea(0, 0, m_screenResolution.x, m_screenResolution.y);
		TreatWindowMessageQueue();
	}
}

HWND NotAnOverlay::SpawnOverlayWindow(std::wstring windowClassName, std::wstring windowTitle) {
	NotAnOverlay::RegisterWindowClass(windowClassName);
	return CreateWindowEx(NULL, windowClassName.c_str(), windowTitle.c_str(), WS_VISIBLE | WS_TILEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, NULL, NULL);
}

bool NotAnOverlay::RegisterWindowClass(std::wstring windowClassName) {
	WNDCLASSEX overlayWindowClass;
	SecureZeroMemory(&overlayWindowClass, sizeof(WNDCLASSEX));
	overlayWindowClass.cbClsExtra = NULL;
	overlayWindowClass.cbWndExtra = NULL;
	overlayWindowClass.cbSize = sizeof(WNDCLASSEX);
	overlayWindowClass.style = CS_HREDRAW | CS_VREDRAW;
	overlayWindowClass.lpfnWndProc = WindowProc; // Function that will be executed when the window receives a "message" (input). Required! (crashes if set to NULL)
	overlayWindowClass.hInstance = NULL;
	overlayWindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	overlayWindowClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	overlayWindowClass.hIconSm = LoadIcon(0, IDI_APPLICATION);
	overlayWindowClass.hbrBackground = NULL; // (HBRUSH)RGB(0, 0, 0)
	overlayWindowClass.lpszClassName = windowClassName.c_str(); // Class name to use with the Windows API function CreateWindow(Ex) to create the window
	overlayWindowClass.lpszMenuName = windowClassName.c_str();
	if (!RegisterClassEx(&overlayWindowClass))
		return false;
	else
		return true;
}

void NotAnOverlay::TreatWindowMessageQueue() {
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

LRESULT CALLBACK NotAnOverlay::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		case WM_DESTROY: {
			PostQuitMessage(0); // Exit the program if the window gets closed.
			exit(EXIT_SUCCESS);
			return EXIT_SUCCESS;
		} break;
		case WM_SIZE: {
			// GetClientRect(m_hwndNotOverlay, &m_windowSize);
		} break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam); // Handle any messages the switch statement didn't
}

bool NotAnOverlay::SaveBitmap(LPCSTR filename, HBITMAP bmp, HPALETTE pal) {
	bool result = false;
	PICTDESC pd;

	pd.cbSizeofstruct = sizeof(PICTDESC);
	pd.picType = PICTYPE_BITMAP;
	pd.bmp.hbitmap = bmp;
	pd.bmp.hpal = pal;

	LPPICTURE picture;
	HRESULT res = OleCreatePictureIndirect(&pd, IID_IPicture, false,
		reinterpret_cast<void**>(&picture));

	if (!SUCCEEDED(res))
		return false;

	LPSTREAM stream;
	res = CreateStreamOnHGlobal(0, true, &stream);

	if (!SUCCEEDED(res))
	{
		picture->Release();
		return false;
	}

	LONG bytes_streamed;
	res = picture->SaveAsFile(stream, true, &bytes_streamed);

	HANDLE file = CreateFileA(filename, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

	if (!SUCCEEDED(res) || !file) {
		stream->Release();
		picture->Release();
		return false;
	}

	HGLOBAL mem = 0;
	GetHGlobalFromStream(stream, &mem);
	LPVOID data = GlobalLock(mem);

	DWORD bytes_written;

	result = !!WriteFile(file, data, bytes_streamed, &bytes_written, 0);
	result &= (bytes_written == static_cast<DWORD>(bytes_streamed));

	GlobalUnlock(mem);
	CloseHandle(file);

	stream->Release();
	picture->Release();

	return result;
}

bool NotAnOverlay::GetScreenResolution() {
	RECT desktop;
	const HWND hDesktop = GetDesktopWindow();
	if (!GetWindowRect(hDesktop, &desktop))
		return false;
	m_screenResolution.x = desktop.right;
	m_screenResolution.y = desktop.bottom;
	return true;
}