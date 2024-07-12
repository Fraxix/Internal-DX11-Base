#pragma once
#include "helper.h"

namespace DX11Base 
{
	class Engine {
	public:

		HWND	pGameWindow{ 0 };

		bool bShowMenu{ false };

		Engine();
		~Engine();
	};
	inline std::unique_ptr<Engine> g_Engine;

	class D3D11Window
	{
	public:
		enum DXGI : int
		{
			IDXGI_PRESENT = 8,
			IDXGI_DRAW_INDEXED = 12,
			IDXGI_RESIZE_BUFFERS = 13,
		};

	public:
		bool								bInit{ false };
		bool								bInitImGui{ false };
		WNDPROC								m_OldWndProc{};
		ImGuiContext* pImGui;
		ImGuiViewport* pViewport;

	public:
		bool								GetD3DContext();
		bool								HookD3D();
		void								HookWndProc(HWND hWnd);
		void								UnhookD3D();
		bool								InitWindow();
		bool								DeleteWindow();
		bool								InitImGui(IDXGISwapChain* swapChain);
		void								Overlay(IDXGISwapChain* pSwapChain);

	public:
		explicit D3D11Window();
		~D3D11Window() noexcept;

	private:
		typedef HRESULT(WINAPI* IDXGISwapChainPresent)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
		typedef void(APIENTRY* ID3D11DeviceContextDrawIndexed)(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);
		typedef HRESULT(WINAPI* IDXGISwapChainResizeBuffers)(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
		IDXGISwapChainPresent				IDXGISwapChain_Present_stub = 0;
		ID3D11DeviceContextDrawIndexed		ID3D11DeviceContext_DrawIndexed_stub = 0;
		IDXGISwapChainResizeBuffers			IDXGISwapChain_ResizeBuffers_stub = 0;

		typedef BOOL(WINAPI* hk_SetCursorPos)(int, int);
		hk_SetCursorPos origSetCursorPos = 0;



		WNDCLASSEX							WindowClass;
		HWND								WindowHwnd;
		ID3D11Device* m_Device{};
		ID3D11DeviceContext* m_DeviceContext{};
		ID3D11RenderTargetView* m_RenderTargetView{};
		IDXGISwapChain* m_pSwapChain{};


	private:
		static LRESULT APIENTRY				WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		static HRESULT APIENTRY				SwapChain_Present_hook(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
		static void APIENTRY				DeviceContext_DrawIndexed_hook(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);
		static HRESULT WINAPI				SwapChain_ResizeBuffers_hook(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
	};
	inline std::unique_ptr<D3D11Window> g_D3D11Window;

	class Hooking
	{
	public:
		void								Initialize();
		void								Shutdown();
		static bool							CreateHook(LPVOID pTarget, LPVOID pDetour, LPVOID* pOrig);
		static void							EnableHook(LPVOID pTarget);
		static void							EnableAllHooks();
		static void							DisableHook(LPVOID pTarget);
		static void							RemoveHook(LPVOID pTarget);
		static void							DisableAllHooks();
		static void							RemoveAllHooks();


	public:
		explicit Hooking();
		~Hooking() noexcept;
	};
	inline std::unique_ptr<Hooking> g_Hooking;
}