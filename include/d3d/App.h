#ifndef D3DAPP_H
#define D3DAPP_H

#include <string>
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

#include <ctime>
#include <algorithm>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>


namespace byhj  
{

namespace d3d 
{


class App
{
public:
	App() :m_AppName(L"DirectX12: "), m_WndClassName(L"D3DWindow")
	{

	}
	virtual ~App() {}

	void InitApp();
	int Run();
	LRESULT CALLBACK MessageHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual	void v_Init()     = 0;
	virtual void v_Update()   = 0;
	virtual void v_Render()   = 0;
	virtual void v_Shutdown() = 0;

	// Convenience overrides for handling mouse input.
	virtual void v_OnMouseDown(WPARAM btnState, int x, int y){ }
	virtual void v_OnMouseUp(WPARAM btnState, int x, int y)  { }
	virtual void v_OnMouseMove(WPARAM btnState, int x, int y){ }
	virtual void v_OnMouseWheel(WPARAM btnState, int x, int y) { }

protected:

	int   m_ScreenWidth;
	int   m_ScreenHeight;
	float m_ScreenFar;
	float m_ScreenNear;
	int   m_PosX;
	int   m_PosY;

	LPCTSTR m_AppName;
	LPCTSTR m_WndClassName;

	//void      GetVideoCardInfo(char &, int &);
	HINSTANCE GetAppInst() const { return m_hInstance; }
	HWND      GetHwnd()    const { return m_hWnd; }
	float     GetAspect()  const { return static_cast<float>(m_ScreenWidth)
		                                  / static_cast<float>(m_ScreenHeight); }

private:
	bool init_window();

private:
	HINSTANCE m_hInstance;
	HWND      m_hWnd;
};

}

}
#endif